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
template<std::size_t MaxSamples>
ReverbZ<MaxSamples>::ReverbZ(int sampleRate)
: 
mPredelay_(),
mInputAllpass1_(),
mInputAllpass2_(),
mInputAllpass3_(),
mInputAllpass4_(),
mModAllpass1_(0, 1, 0.0f, 0.0f),    // Fixed Modulated AllPass instantiation, init paramters in init().
mModAllpass2_(0, 1, 0.0f, 0.0f),
mTankDelay1_(),
mTankDelay3_(),
mTankAllpass5_(),
mTankAllpass6_(),
mTankDelay2_(),
mTankDelay4_(),
mTankAllpass7_(),
mTankAllpass8_(),
mTankAllpass9_(),
mTankAllpass10_()
{
    // Set sample rate from external input.
    mFs_ = sampleRate;

    // NOTE: init() must be called manually after hardware/SDRAM initialization
    // DO NOT call init() here - constructor runs during static initialization
    // before SDRAM is ready!
}
/* ------------------------------- Destructor ------------------------------- */
template<std::size_t MaxSamples>
ReverbZ<MaxSamples>::~ReverbZ(){}
/* -------------------------------------------------------------------------- */
 

/* -------------------------------------------------------------------------- */
/*                               Public Methods                               */
/* -------------------------------------------------------------------------- */
template<std::size_t MaxSamples>
void ReverbZ<MaxSamples>::init()       
{
    /* ------------ Allocate Buffers for AllPasses and DelayLines ----------- */
    mPredelay_.init();
    mInputAllpass1_.init();
    mInputAllpass2_.init();
    mInputAllpass3_.init();
    mInputAllpass4_.init();
    mModAllpass1_.init();
    mModAllpass2_.init();
    mTankDelay1_.init();
    mTankDelay3_.init();
    mTankAllpass5_.init();
    mTankAllpass6_.init();
    mTankDelay2_.init();
    mTankDelay4_.init();
    mTankAllpass7_.init();
    mTankAllpass8_.init();
    mTankAllpass9_.init();
    mTankAllpass10_.init();  
    /* -------------------- Set static object parameters -------------------- */
    float modAllpassesFeedbackCoef = 0.70f;
    
    /* -- convert Dattorro's delay times based on the current sampling rate - */
    // Cast integer sampling rates to float to avoid integer division (-> 48000/29761 = 1)
    float mFsFloat = static_cast<float>(mFs_);
    float mFsDattorroFloat = static_cast<float>(mFsDattorro_);
    // Input Allpasses
    mInputAllpass1_.setDelaySamples(static_cast<int>(round(mFsFloat/mFsDattorroFloat * 142)));
    mInputAllpass2_.setDelaySamples(static_cast<int>(round(mFsFloat/mFsDattorroFloat * 107)));
    mInputAllpass3_.setDelaySamples(static_cast<int>(round(mFsFloat/mFsDattorroFloat * 379)));
    mInputAllpass4_.setDelaySamples(static_cast<int>(round(mFsFloat/mFsDattorroFloat * 277)));
    // Tank modulated allpasses
    mModAllpass1_.setDelaySamples(static_cast<int>(round(mFsFloat/mFsDattorroFloat * 672)));
    mModAllpass2_.setDelaySamples(static_cast<int>(round(mFsFloat/mFsDattorroFloat * 908)));
    mModAllpass1_.setFeedbackCoefficient(modAllpassesFeedbackCoef);
    mModAllpass2_.setFeedbackCoefficient(modAllpassesFeedbackCoef);
    // Tank delay lines
    mTankDelay1_.setDelaySamples(static_cast<int>(round(mFsFloat/mFsDattorroFloat * 4453)));
    mTankDelay3_.setDelaySamples(static_cast<int>(round(mFsFloat/mFsDattorroFloat * 3720)));
    mTankDelay2_.setDelaySamples(static_cast<int>(round(mFsFloat/mFsDattorroFloat * 4217)));
    mTankDelay4_.setDelaySamples(static_cast<int>(round(mFsFloat/mFsDattorroFloat * 3163)));
    // Tank non-modulated allpasses
    mTankAllpass5_.setDelaySamples(static_cast<int>(round(mFsFloat/mFsDattorroFloat * 1800)));
    mTankAllpass6_.setDelaySamples(static_cast<int>(round(mFsFloat/mFsDattorroFloat * 2656)));
    // (delay times for allpass 7-10 are prime numbers)
    mTankAllpass7_.setDelaySamples(static_cast<int>(round(mFsFloat/mFsDattorroFloat * 1511)));
    mTankAllpass8_.setDelaySamples(static_cast<int>(round(mFsFloat/mFsDattorroFloat * 2003)));
    mTankAllpass9_.setDelaySamples(static_cast<int>(round(mFsFloat/mFsDattorroFloat * 1709)));
    mTankAllpass10_.setDelaySamples(static_cast<int>(round(mFsFloat/mFsDattorroFloat * 2411))); 

    /* Init Modulated AllPasses' LFOs*/
    mModAllpass1_.mLFO.setSamplingFrequency(mFs_);
    mModAllpass2_.mLFO.setSamplingFrequency(mFs_);
    mModAllpass1_.mLFO.setFrequencyOscillator(0.6f);      // Fixed frequencies
    mModAllpass2_.mLFO.setFrequencyOscillator(0.8f);      // Fixed frequencies
    mModAllpass1_.mLFO.init();
    mModAllpass2_.mLFO.init();


    /* Original reverbz GUI control defaults. Not necessary if params are set and updated at runtime
    mPredelayTime_ = 0.000000;
    mInputLowpassFc = 22000.000000;
    mInputHighpassFc = 10.000000;
    mInputAllpass1DiffusionCtrl = 0.750000;
    mSaturatorDriveCtrl = 0.100000;
    mTankDecayCtrl = 0.500000;
    mTankLowpassFc = 5000.000000;
    mTankHighpassFc = 0.000000;
    mDryWetMixPercentage = 100.000000;
    mIsSmoothed_ = 0;
    */
}

template<std::size_t MaxSamples>
void ReverbZ<MaxSamples>::processAudioMono(float inputSample)
{
    /* ------------ Process a single sample here ------------ */
    // Core processing is Mono->Stereo
    processAudioPrivate(inputSample);

    // Dry/Wet -> Stereo to mono 
    float outWetMono = (mOutWetL_ + mOutWetR_)/2.0f;
    mOutMono = inputSample*(1.0f - mDryWetMix_) + outWetMono*mDryWetMix_;
}

template<std::size_t MaxSamples>
void ReverbZ<MaxSamples>::processAudioStereo(float inputSampleL, float inputSampleR)
{
    /* ------------ Process a pair of LR samples here ------------ */
    // Stereo->Mono. Core processing is Mono->Stereo
    float inputSample = (inputSampleL + inputSampleR)/2.0f;
    processAudioPrivate(inputSample);

    // Dry/Wet
    mOutL = inputSampleL*(1.0f - mDryWetMix_) + mOutWetL_*mDryWetMix_;
    mOutR = inputSampleR*(1.0f - mDryWetMix_) + mOutWetR_*mDryWetMix_;
}

template<std::size_t MaxSamples>
void ReverbZ<MaxSamples>::setControlParameters(float predelayTime,
                                    float inputLowpassFc,
                                    float inputHighpassFc,
                                    float inputDiffusion,
                                    float decay,
                                    float drive,
                                    float hfDampingFc,
                                    float lfDampingFc,
                                    float mixPercentage,
                                    int smooth)
{
    /* ------------ PREDELAY range [0,inf] ------------ */
    // Predelay time [input in ms]
    int predelaySamples = static_cast<int> (round(predelayTime/1000.0f));
    mPredelay_.setDelaySamples(predelaySamples);
    // TODO: control not only predelay but the global delays of all allpasses.

    /* ------------ INPUT LP FC range [0Hz, 24kHz] ------------ */
    // Input lowpass cutoff frequency [input in Hz]
    float inputLowpassNormWc = dspLib::normalizeFreq(inputLowpassFc, mFs_);
    mInputLowpass_.setNormalizedCutoffFrequency(inputLowpassNormWc);

    /* ------------ INPUT HP FC range [0Hz, 24kHz] ------------ */
    // Input highpass cutoff frequency [input in Hz]
    float inputHighpassNormWc = dspLib::normalizeFreq(inputHighpassFc, mFs_);
    mInputHighpass_.setNormalizedCutoffFrequency(inputHighpassNormWc);

    /* ------------ INPUT DIFFUSION range [0,1] ------------ */
    // input diffusion 3 gets varied along with diffusion 1
    // might change to /6?
    float inputAllpass1Diffusion = inputDiffusion;
    float inputAllpass3Diffusion = 0.625f + (inputDiffusion - 0.5f)/6.0f;
    mInputAllpass1_.setFeedbackCoefficient(inputAllpass1Diffusion);
    mInputAllpass2_.setFeedbackCoefficient(inputAllpass1Diffusion);
    mInputAllpass3_.setFeedbackCoefficient(inputAllpass3Diffusion);
    mInputAllpass4_.setFeedbackCoefficient(inputAllpass3Diffusion);

    /* ------------ TANK DECAY range [0,1] ------------ */
    // decay also affects the allpasses feedback in the tank (values taken from dattorro's)
    mTankDecay_ = decay;
    float tankAllpassDiffusion = mTankDecay_ + 0.15f;
    if (tankAllpassDiffusion < 0.15f) tankAllpassDiffusion = 0.15f;
    if (tankAllpassDiffusion > 0.50f) tankAllpassDiffusion = 0.50f;
    
    // Update diffusion coefficients of all AllPasses in the tank
    mTankAllpass5_.setFeedbackCoefficient(tankAllpassDiffusion);
    mTankAllpass6_.setFeedbackCoefficient(tankAllpassDiffusion);    
    mTankAllpass7_.setFeedbackCoefficient(tankAllpassDiffusion);
    mTankAllpass8_.setFeedbackCoefficient(tankAllpassDiffusion);
    mTankAllpass9_.setFeedbackCoefficient(tankAllpassDiffusion);
    mTankAllpass10_.setFeedbackCoefficient(tankAllpassDiffusion);

    /* ------------ TANK DRIVE range [0dB,inf] ------------ */
    mSaturator_.setDrive(drive);

    /* ------------ TANK HF DAMPING [0Hz, 24kHz] ------------ */
    float normFreqHfDamping = dspLib::normalizeFreq(hfDampingFc, mFs_);
    mTankLowpass1_.setNormalizedCutoffFrequency(normFreqHfDamping);
    mTankLowpass2_.setNormalizedCutoffFrequency(normFreqHfDamping);

    /* ------------ TANK LF DAMPING [0Hz, 24kHz] ------------ */
    float normFreqLfDamping = dspLib::normalizeFreq(lfDampingFc, mFs_);
    mTankHighpass1_.setNormalizedCutoffFrequency(normFreqLfDamping);
    mTankHighpass2_.setNormalizedCutoffFrequency(normFreqLfDamping);

    /* ------------ DRY-WET MIX [0,100] ------------ */
    mDryWetMix_ = mixPercentage/100.0f;

    /* ------------ SMOOTH ON/OFF [true, false] ------------ */
    mIsSmoothed_ = smooth;
    if(mIsSmoothed_)
    {
        // Set modulation amplitude: max delay samples modulation
        mModAllpass1_.setModDepth(24.0f);
        mModAllpass2_.setModDepth(48.0f);
    }
    else if(!mIsSmoothed_)
    {
        // No delay line modulation - modulation depth set to 0.0f if no arguments are passed.
        mModAllpass1_.setModDepth();
        mModAllpass2_.setModDepth();
    }
}
/* -------------------------------------------------------------------------- */
/*                               Private Methods                              */
/* -------------------------------------------------------------------------- */
template<std::size_t MaxSamples>
void ReverbZ<MaxSamples>::processAudioPrivate(float inputSample)
{
    /* ------------ Core processing stereo function ------------ */

    /* ---------------------------------------------------------------------- */
    /*                              INPUT SECTION                             */
    /* ---------------------------------------------------------------------- */
    // Pre-delay (pre-delay time can be user controlled)
    mPredelayOut_ = mPredelay_.processAudio(inputSample);
    
    // Input lowpass filter
    mInputLowpassOut_ = mInputLowpass_.processAudioLP(mPredelayOut_);
    
    // Input highpass filter
    mInputHighpassOut_ = mInputHighpass_.processAudioHP(mInputLowpassOut_);
    
    // Input Allpass diffusers
    mInputAllpass1Out_ = mInputAllpass1_.processAudio(mInputHighpassOut_);
    mInputAllpass2Out_ = mInputAllpass2_.processAudio(mInputAllpass1Out_);
    mInputAllpass3Out_ = mInputAllpass3_.processAudio(mInputAllpass2Out_);
    mInputAllPass4Out_ = mInputAllpass4_.processAudio(mInputAllpass3Out_);
    
    /* ---------------------------------------------------------------------- */
    /*                              TANK SECTION                              */
    /* ---------------------------------------------------------------------- */
    // tank input accumulator summed with input diffusers' output
    mTankInput1_ = mInputAllPass4Out_ + mTankAccumulator2_;
    mTankInput2_ = mInputAllPass4Out_ + mTankAccumulator1_;
    
    // Modulated tank all-passes
    mModAllpass1Out_ = mModAllpass1_.processAudio(mTankInput1_);
    mModAllpass2Out_ = mModAllpass2_.processAudio(mTankInput2_);
    
    // Delay lines (1 and 3)
    mTankDelay1Out_ = mTankDelay1_.processAudio(mModAllpass1Out_);
    mTankDelay3Out_ = mTankDelay3_.processAudio(mModAllpass2Out_);
    
    // Saturation
    // 2 different saturation curves, one for each leg of the tank
    mSaturator1Out_ = mSaturator_.processAudioAtan(mTankDelay1Out_);
    mSaturator2Out_ = mSaturator_.processAudioTanh(mTankDelay3Out_);
    
    // Tank Lowpass Filtering (Damping)
    mTankLowpass1Out_ = mTankLowpass1_.processAudioLP(mSaturator1Out_);
    mTankLowpass2Out_ = mTankLowpass2_.processAudioLP(mSaturator2Out_);
    
    // Tank HighPass
    mTankHighpass1Out_ = mTankHighpass1_.processAudioHP(mTankLowpass1Out_);
    mTankHighpass2Out_ = mTankHighpass2_.processAudioHP(mTankLowpass2Out_);
    
    // Tank AllPass filters
    mTankAllpass5Out_ = mTankAllpass5_.processAudio(mTankHighpass1Out_);
    mTankAllpass6Out_ = mTankAllpass6_.processAudio(mTankHighpass2Out_);
    
    // Add decay control between the allpass filters and the last delay lines
    mTankAllpass5Out_ = mTankAllpass5Out_*mTankDecay_;
    mTankAllpass6Out_ = mTankAllpass6Out_*mTankDecay_;
    
    // Delay lines (2 and 4)
    mTankDelay2Out_ = mTankDelay2_.processAudio(mTankAllpass5Out_);
    mTankDelay4Out_ = mTankDelay4_.processAudio(mTankAllpass6Out_);
    
    // If Smooth == on allpass 7 - 10 are included
    if (mIsSmoothed_ == 1)
    {
        // Added Allpasses 7 - 10
        mTankAllpass7Out_ = mTankAllpass7_.processAudio(mTankDelay2Out_);
        mTankAllpass8Out_ = mTankAllpass8_.processAudio(mTankDelay4Out_);
        mTankAllpass9Out_ = mTankAllpass9_.processAudio(mTankAllpass7Out_);
        mTankAllpass10Out_ = mTankAllpass10_.processAudio(mTankAllpass8Out_);
        
        // Compute the accumulators as the outputs from the last tank nodes scaled by decay control
        mTankAccumulator1_ = mTankDecay_*(mTankAllpass9Out_);
        mTankAccumulator2_ = mTankDecay_*(mTankAllpass10Out_);
        
        // Simplified wet output computation compared to dattorro's
        mOutWetL_ = 0.6f*(mTankDelay3Out_ - mTankAllpass5Out_ + mTankDelay2Out_ - mTankAllpass8Out_ + mTankAllpass10Out_);
        mOutWetR_ = 0.6f*(mTankDelay1Out_ - mTankAllpass6Out_ + mTankDelay4Out_ - mTankAllpass7Out_ + mTankAllpass9Out_);   
    }
    
    // If Smooth == off then allpasses 7 - 10 are bypassed
    if (mIsSmoothed_ == 0)
    {
        // Compute the accumulators as the outputs from the last tank nodes scaled by decay control
        mTankAccumulator1_ = mTankDecay_*(mTankDelay2Out_);
        mTankAccumulator2_ = mTankDecay_*(mTankDelay4Out_);
        
        // Simplified wet output computation compared to dattorro's
        mOutWetL_ = 0.7f*(mTankDelay3Out_ - mTankAllpass5Out_ + mTankDelay2Out_);
        mOutWetR_ = 0.7f*(mTankDelay1Out_ - mTankAllpass6Out_ + mTankDelay4Out_);
    }
}

}   // namespace projLib
