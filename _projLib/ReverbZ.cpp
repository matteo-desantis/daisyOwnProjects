/** ------------------------------------------------------------------------- 
    ReverbZ.cpp - Implementation file for ReverbZ class.
    Saturated Reverb algorithm.

    High-level implementation - No hardware-specific code here.


    Matteo Desantis 31-Oct-2025
*/

#include "ReverbZ.hpp"
#include <cmath>

using namespace dspLib;

namespace projLib {

/* ------------------------------- Constructor ------------------------------ */
ReverbZ::ReverbZ(int sampleRate)
: 
mPredelay(static_cast<int>(sampleRate * mBufferScaleFactor), 0),
mInputAllpass1(static_cast<int>(sampleRate * mBufferScaleFactor), 0),
mInputAllpass2(static_cast<int>(sampleRate * mBufferScaleFactor), 0),
mInputAllpass3(static_cast<int>(sampleRate * mBufferScaleFactor), 0),
mInputAllpass4(static_cast<int>(sampleRate * mBufferScaleFactor), 0),
mModAllpass1([&]{ return dspLib::AllPass(static_cast<int>(sampleRate * mBufferScaleFactor), 0, sampleRate, 0.0, 0.0); }()),
mModAllpass2([&]{ return dspLib::AllPass(static_cast<int>(sampleRate * mBufferScaleFactor), 0, sampleRate, 0.0, 0.0); }()),
mTankDelay1(static_cast<int>(sampleRate * mBufferScaleFactor), 0),
mTankDelay3(static_cast<int>(sampleRate * mBufferScaleFactor), 0),
mTankAllpass5(static_cast<int>(sampleRate * mBufferScaleFactor), 0),
mTankAllpass6(static_cast<int>(sampleRate * mBufferScaleFactor), 0),
mTankAllpass7(static_cast<int>(sampleRate * mBufferScaleFactor), 0),
mTankAllpass8(static_cast<int>(sampleRate * mBufferScaleFactor), 0),
mTankAllpass9(static_cast<int>(sampleRate * mBufferScaleFactor), 0),
mTankAllpass10(static_cast<int>(sampleRate * mBufferScaleFactor), 0),
mTankDelay2(static_cast<int>(sampleRate * mBufferScaleFactor), 0),
mTankDelay4(static_cast<int>(sampleRate * mBufferScaleFactor), 0)
{
    // Set sample rate from external imput.
    mFs = sampleRate;

    // Initialize member variables
    init();
}
/* ------------------------------- Destructor ------------------------------- */
ReverbZ::~ReverbZ(){}
/* -------------------------------------------------------------------------- */
 

/* -------------------------------------------------------------------------- */
/*                               Public Methods                               */
/* -------------------------------------------------------------------------- */
void ReverbZ::init()       
{
    /* -------------------- Set static object parameters -------------------- */
    double modAllpassesFeedbackCoef = 0.70;
    
    /* -- convert Dattorro's delay times based on the current sampling rate - */
    // Input Allpasses
    mInputAllpass1.setDelaySamples((int) round(mFs/mFsDattorro * 142));
    mInputAllpass2.setDelaySamples((int) round(mFs/mFsDattorro * 107));
    mInputAllpass3.setDelaySamples((int) round(mFs/mFsDattorro * 379));
    mInputAllpass4.setDelaySamples((int) round(mFs/mFsDattorro * 277));
    // Tank modulated allpasses
    mModAllpass1.setDelaySamples((int) round(mFs/mFsDattorro * 672));
    mModAllpass2.setDelaySamples((int) round(mFs/mFsDattorro * 908));
    mModAllpass1.setFeedbackCoefficient(modAllpassesFeedbackCoef);
    mModAllpass2.setFeedbackCoefficient(modAllpassesFeedbackCoef);
    // Tank delay lines
    mTankDelay1.setDelaySamples((int) round(mFs/mFsDattorro * 4453));
    mTankDelay3.setDelaySamples((int) round(mFs/mFsDattorro * 3720));
    mTankDelay2.setDelaySamples((int) round(mFs/mFsDattorro * 4217));
    mTankDelay4.setDelaySamples((int) round(mFs/mFsDattorro * 3163));
    // Tank non-modulated allpasses
    mTankAllpass5.setDelaySamples((int) round(mFs/mFsDattorro * 1800));
    mTankAllpass6.setDelaySamples((int) round(mFs/mFsDattorro * 2656));
    // (delay times for allpass 7-10 are prime numbers)
    mTankAllpass7.setDelaySamples((int) round(mFs/mFsDattorro * 1511));
    mTankAllpass8.setDelaySamples((int) round(mFs/mFsDattorro * 2003));
    mTankAllpass9.setDelaySamples((int) round(mFs/mFsDattorro * 1709));
    mTankAllpass10.setDelaySamples((int) round(mFs/mFsDattorro * 2411)); 

    /* Original reverbz GUI control defaults. Not necessary if params are set and updated at runtime
    mPredelayTime = 0.000000;
    mInputLowpassFc = 22000.000000;
    mInputHighpassFc = 10.000000;
    mInputAllpass1DiffusionCtrl = 0.750000;
    mSaturatorDriveCtrl = 0.100000;
    mTankDecayCtrl = 0.500000;
    mTankLowpassFc = 5000.000000;
    mTankHighpassFc = 0.000000;
    mDryWetMixPercentage = 100.000000;
    mIsSmoothed = 0;
    */
}

void ReverbZ::processAudioMono(double inputSample)
{
    /* ------------ Process a single sample here ------------ */
    // Core processing is Mono->Stereo
    processAudioPrivate(inputSample);

    // Dry/Wet -> Stereo to mono 
    double outWetMono = (mOutWetL + mOutWetR)/2;
    mOutMono = inputSample*(1 - mDryWetMix) + outWetMono*mDryWetMix;


}
void ReverbZ::processAudioStereo(double inputSampleL, double inputSampleR)
{
    /* ------------ Process a pair of LR samples here ------------ */
    // Stereo->Mono. Core processing is Mono->Stereo
    double inputSample = (inputSampleL + inputSampleR)/2;
    processAudioPrivate(inputSample);

    // Dry/Wet
    mOutL = inputSampleL*(1 - mDryWetMix) + mOutWetL*mDryWetMix;
    mOutR = inputSampleR*(1 - mDryWetMix) + mOutWetR*mDryWetMix;
}

void ReverbZ::setControlParameters(double predelayTime,
                                    double inputLowpassFc,
                                    double inputHighpassFc,
                                    double inputDiffusion,
                                    double decay,
                                    double drive,
                                    double hfDampingFc,
                                    double lfDampingFc,
                                    double mixPercentage,
                                    int smooth)
{
    /* ------------ PREDELAY range [0,inf] ------------ */
    // Predelay time [input in ms]
    int predelaySamples = (int) (round(predelayTime/1000.0));
    mPredelay.setDelaySamples(predelaySamples);
    // TODO: control not only predelay but the global delays of all allpasses.

    /* ------------ INPUT LP FC range [0Hz, 24kHz] ------------ */
    // Input lowpass cutoff frequency [input in Hz]
    double inputLowpassNormWc = dspLib::normalizeFreq(inputLowpassFc, mFs);
    mInputLowpass.setNormalizedCutoffFrequency(inputLowpassNormWc);

    /* ------------ INPUT HP FC range [0Hz, 24kHz] ------------ */
    // Input highpass cutoff frequency [input in Hz]
    double inputHighpassNormWc = dspLib::normalizeFreq(inputHighpassFc, mFs);
    mInputHighpass.setNormalizedCutoffFrequency(inputHighpassNormWc);

    /* ------------ INPUT DIFFUSION range [0,1] ------------ */
    // input diffusion 3 gets varied along with diffusion 1
    // might change to /6?
    double inputAllpass1Diffusion = inputDiffusion;
    double inputAllpass3Diffusion = 0.625 + (inputDiffusion - 0.5)/6.0;
    mInputAllpass1.setFeedbackCoefficient(inputAllpass1Diffusion);
    mInputAllpass2.setFeedbackCoefficient(inputAllpass1Diffusion);
    mInputAllpass3.setFeedbackCoefficient(inputAllpass3Diffusion);
    mInputAllpass4.setFeedbackCoefficient(inputAllpass3Diffusion);

    /* ------------ TANK DECAY range [0,1] ------------ */
    // decay also affects the allpasses feedback in the tank (values taken from dattorro's )
    double tankAllpassDiffusion = decay + 0.15;
    if (tankAllpassDiffusion < 0.15) tankAllpassDiffusion = 0.15;
    if (tankAllpassDiffusion > 0.50) tankAllpassDiffusion = 0.50;
    
    // Update diffusion coefficients of all AllPasses in the tank
    mTankAllpass5.setFeedbackCoefficient(tankAllpassDiffusion);
    mTankAllpass6.setFeedbackCoefficient(tankAllpassDiffusion);    
    mTankAllpass7.setFeedbackCoefficient(tankAllpassDiffusion);
    mTankAllpass8.setFeedbackCoefficient(tankAllpassDiffusion);
    mTankAllpass9.setFeedbackCoefficient(tankAllpassDiffusion);
    mTankAllpass10.setFeedbackCoefficient(tankAllpassDiffusion);

    /* ------------ TANK DRIVE range [0dB,inf] ------------ */
    mSaturator.setDrive(drive);

    /* ------------ TANK HF DAMPING [0Hz, 24kHz] ------------ */
    double normFreqHfDamping = dspLib::normalizeFreq(hfDampingFc, mFs);
    mTankLowpass1.setNormalizedCutoffFrequency(normFreqHfDamping);
    mTankLowpass2.setNormalizedCutoffFrequency(normFreqHfDamping);

    /* ------------ TANK LF DAMPING [0Hz, 24kHz] ------------ */
    double normFreqLfDamping = dspLib::normalizeFreq(lfDampingFc, mFs);
    mTankHighpass1.setNormalizedCutoffFrequency(normFreqLfDamping);
    mTankHighpass2.setNormalizedCutoffFrequency(normFreqLfDamping);

    /* ------------ DRY-WET MIX [0,100] ------------ */
    mDryWetMix = mixPercentage/100.0;

    /* ------------ SMOOTH ON/OFF [true, false] ------------ */
    mIsSmoothed = smooth;
    if(mIsSmoothed)
    {
        // Set modulation amplitude: max delay samples modulation
        mModAllpass1.setModDepth(24);
        mModAllpass2.setModDepth(48);
    }
    else if(!mIsSmoothed)
    {
        // No delay line modulation - modulation depth set to 0.0 if no arguments are passed.
        mModAllpass1.setModDepth();
        mModAllpass2.setModDepth();
    }
}
/* -------------------------------------------------------------------------- */
/*                               Private Methods                              */
/* -------------------------------------------------------------------------- */
void ReverbZ::processAudioPrivate(double inputSample)
{
    /* ------------ Core processing stereo function ------------ */

    /* ---------------------------------------------------------------------- */
    /*                              INPUT SECTION                             */
    /* ---------------------------------------------------------------------- */
    // Pre-delay (pre-delay time can be user controlled)
    mPredelayOut = mPredelay.processAudio(inputSample);
    
    // Input lowpass filter
    mInputLowpassOut = mInputLowpass.processAudioLP(mPredelayOut);
    
    // Input highpass filter
    mInputHighpassOut = mInputHighpass.processAudioHP(mInputLowpassOut);
    
    // Input Allpass diffusers
    mInputAllpass1Out = mInputAllpass1.processAudio(mInputHighpassOut);
    mInputAllpass2Out = mInputAllpass2.processAudio(mInputAllpass1Out);
    mInputAllpass3Out = mInputAllpass3.processAudio(mInputAllpass2Out);
    mInputAllPass4Out = mInputAllpass4.processAudio(mInputAllpass3Out);
    
    /* ---------------------------------------------------------------------- */
    /*                              TANK SECTION                              */
    /* ---------------------------------------------------------------------- */
    // tank input accumulator summed with input diffusers' output
    mTankInput1 = mInputAllPass4Out + mTankAccumulator2;
    mTankInput2 = mInputAllPass4Out + mTankAccumulator1;
    
    // Modulated tank all-passes
    mModAllpass1Out = mModAllpass1.processAudio(mTankInput1);
    mModAllpass2Out = mModAllpass2.processAudio(mTankInput2);
    
    // Delay lines (1 and 3)
    mTankDelay1Out = mTankDelay1.processAudio(mModAllpass1Out);
    mTankDelay3Out = mTankDelay3.processAudio(mModAllpass2Out);
    
    // Saturation
    // 2 different saturation curves, one for each leg of the tank
    mSaturator1Out = mSaturator.processAudioAtan(mTankDelay1Out);
    mSaturator2Out = mSaturator.processAudioTanh(mTankDelay3Out);
    
    // Tank Lowpass Filtering (Damping)
    mTankLowpass1Out = mTankLowpass1.processAudioLP(mSaturator1Out);
    mTankLowpass2Out = mTankLowpass2.processAudioLP(mSaturator2Out);
    
    // Tank HighPass
    mTankHighpass1Out = mTankHighpass1.processAudioHP(mTankLowpass1Out);
    mTankHighpass2Out = mTankHighpass2.processAudioHP(mTankLowpass2Out);
    
    // Tank AllPass filters
    mTankAllpass5Out = mTankAllpass5.processAudio(mTankHighpass1Out);
    mTankAllpass6Out = mTankAllpass6.processAudio(mTankHighpass2Out);
    
    // Add decay control between the allpass filters and the last delay lines
    mTankAllpass5Out = mTankAllpass5Out*mTankDecay;
    mTankAllpass6Out = mTankAllpass6Out*mTankDecay;
    
    // Delay lines (2 and 4)
    mTankDelay2Out = mTankDelay2.processAudio(mTankAllpass5Out);
    mTankDelay4Out = mTankDelay4.processAudio(mTankAllpass6Out);
    
    // If Smooth == on allpass 7 - 10 are included
    if (mIsSmoothed == 1)
    {
        // Added Allpasses 7 - 10
        mTankAllpass7Out = mTankAllpass7.processAudio(mTankDelay2Out);
        mTankAllpass8Out = mTankAllpass8.processAudio(mTankDelay4Out);
        mTankAllpass9Out = mTankAllpass9.processAudio(mTankAllpass7Out);
        mTankAllpass10Out = mTankAllpass10.processAudio(mTankAllpass8Out);
        
        // Compute the accumulators as the outputs from the last tank nodes scaled by decay control
        mTankAccumulator1 = mTankDecay*(mTankAllpass9Out);
        mTankAccumulator2 = mTankDecay*(mTankAllpass10Out);
        
        // Simplified wet output computation compared to dattorro's
        mOutWetL = 0.6*(mTankDelay3Out - mTankAllpass5Out + mTankDelay2Out - mTankAllpass8Out + mTankAllpass10Out);
        mOutWetR = 0.6*(mTankDelay1Out - mTankAllpass6Out + mTankDelay4Out - mTankAllpass7Out + mTankAllpass9Out);   
    }
    
    // If Smooth == off then allpasses 7 - 10 are bypassed
    if (mIsSmoothed == 0)
    {
        // Compute the accumulators as the outputs from the last tank nodes scaled by decay control
        mTankAccumulator1 = mTankDecay*(mTankDelay2Out);
        mTankAccumulator2 = mTankDecay*(mTankDelay4Out);
        
        // Simplified wet output computation compared to dattorro's
        mOutWetL = 0.7*(mTankDelay3Out - mTankAllpass5Out + mTankDelay2Out);
        mOutWetR = 0.7*(mTankDelay1Out - mTankAllpass6Out + mTankDelay4Out);
    }
}

}   // namespace projLib
