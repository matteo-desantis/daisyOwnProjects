// -----------------------------------------------------------------------------
//    ASPiK Plugin Kernel File:  plugincore.cpp
//
/**
    \file   plugincore.cpp
    \author Will Pirkle
    \date   17-September-2018
    \brief  Implementation file for PluginCore object
            - http://www.aspikplugins.com
            - http://www.willpirkle.com
*/
// MD 8/10/2025 - Cleaned up everything not related to the core ReverbZ algorithm
// -----------------------------------------------------------------------------
#include "plugincore.h"
#include "plugindescription.h"
#include "../dspLib/AllPass.hpp"
#include "../dspLib/DelayLine.hpp"  
#include "../dspLib/DeZipper.hpp"           
#include "../dspLib/OnePoleFilter.hpp"
#include "../dspLib/Saturator.hpp"
#include "../dspLib/mathUtils.hpp"
#include <cmath>

/**

*/
PluginCore::PluginCore()
    : 
    // Initialize DeZippers with smoothing factor
    mDeZipperPredelaySamples(0.95),
    mDeZipperInputLowpassNormWc(0.95),
    mDeZipperInputHighpassNormWc(0.95),
    mDeZipperInputDiffusionAllpass1(0.999),
    mDeZipperInputDiffusionAllpass3(0.95),
    mDeZipperDecay(0.95),
    mDeZipperTankAllpassDiffusion(0.95),
    mDeZipperSaturationDrive(0.95),
    mDeZipperTankLowpassNormWc(0.95),
    mDeZipperTankHighpassNormWc(0.99),
    mDeZipperDryWetMix(0.95),

    // Init delaylines and allpasses (TODO: might want to increase buffer length with some scale factor, es fs*scaleFactor)
    mPredelay(48000, 0), 
    mInputAllpass1(48000, 0),
    mInputAllpass2(48000, 0),
    mInputAllpass3(48000, 0),
    mInputAllpass4(48000, 0),
    mModAllpass1([&]
    {
        // Init using lambda function to run some calculation before the object is actually initialized.
        int modAllpass1Length = 48000;
        mFs = getSampleRate();

        return dspLib::AllPass(modAllpass1Length, mFs, 0.6, 0.0, dspLib::sine);
    }()),
    mModAllpass2([&]
    {
        int modAllpass2Length = 48000;
        mFs = getSampleRate();

        return dspLib::AllPass(modAllpass2Length, mFs, 0.8, 0.0, dspLib::sine);
    }()),
    mTankAllpass5(48000, 0),
    mTankAllpass6(48000, 0),
    mTankAllpass7(48000, 0),
    mTankAllpass8(48000, 0),
    mTankAllpass9(48000, 0),
    mTankAllpass10(48000, 0)
{
    // --- create the parameters
    initPluginParameters();

    // Initialize member variables
    initialize();
}

/*
MD implement dummy getSampleRate()
*/
double PluginCore::getSampleRate()
{
    mFs = 48000.0; // Default sample rate
    return mFs;
}

/**
\brief create all of your plugin parameters here
*/
bool PluginCore::initPluginParameters()
{
    // --- Add your plugin parameter instantiation code bewtween these hex codes
    // **--0xDEA7--**
    PluginParameter* piParam = nullptr;
    
    // Predelay Time
    piParam = new PluginParameter(controlID::predelayTime, "Predelay Time", "ms", controlVariableType::kDouble, 0.000000, 100.000000, 0.00000, taper::kLinearTaper);
    piParam->setBoundVariable(&mPredelayTime, boundVariableType::kDouble);
    addPluginParameter(piParam);
    
    // Input lowpass filter cutoff frequency
    piParam = new PluginParameter(controlID::inputLowpassFc, "Input LowPass Fc", "Hz", controlVariableType::kDouble, 500.000000, 22000.000000, 22000.000000, taper::kAntiLogTaper);
    piParam->setBoundVariable(&mInputLowpassFc, boundVariableType::kDouble);
    addPluginParameter(piParam);
    
    // Input highpass filter cutoff frequency
    piParam = new PluginParameter(controlID::inputHighpassFc, "Input HighPass Fc", "Hz", controlVariableType::kDouble, 10.000000, 3000.000000, 10.000000, taper::kAntiLogTaper);
    piParam->setBoundVariable(&mInputHighpassFc, boundVariableType::kDouble);
    addPluginParameter(piParam);

    // Input diffusion
    piParam = new PluginParameter(controlID::inputDiffusion, "Input Diffusion", "", controlVariableType::kDouble, 0.000000, 0.999000, 0.750000, taper::kLinearTaper);
    piParam->setBoundVariable(&mInputAllpass1DiffusionCtrl, boundVariableType::kDouble);
    addPluginParameter(piParam);
    
    // Tank decay control
        piParam = new PluginParameter(controlID::decay, "Decay", "", controlVariableType::kDouble, 0.000000, 0.850000, 0.500000, taper::kLinearTaper);
        piParam->setBoundVariable(&mTankDecayCtrl, boundVariableType::kDouble);
        addPluginParameter(piParam);
   
    // Saturation amount control
    piParam = new PluginParameter(controlID::drive, "Drive", "dB", controlVariableType::kDouble, 0.100000, 20.000000, 0.100000, taper::kLinearTaper);
    piParam->setBoundVariable(&mSaturatorDriveCtrl, boundVariableType::kDouble);
    addPluginParameter(piParam);

    // HF Damping
    // set the opposite way round to get a the desired behaviour in the GUI
    piParam = new PluginParameter(controlID::hfDamping, "HF Damping", "Hz", controlVariableType::kDouble, -20000.000000, -400.000000, -5000.000000, taper::kLogTaper);
    piParam->setBoundVariable(&mTankLowpassFc, boundVariableType::kDouble);
    addPluginParameter(piParam);

    // HF Damping
    piParam = new PluginParameter(controlID::lfDamping, "LF Damping", "Hz", controlVariableType::kDouble, 0.000000, 3000.000000, 0.000000, taper::kAntiLogTaper);
    piParam->setBoundVariable(&mTankHighpassFc, boundVariableType::kDouble);
    addPluginParameter(piParam);

    // Dry/Wet control
    piParam = new PluginParameter(controlID::mixPercentage, "Dry/Wet ", "%", controlVariableType::kDouble, 0.000000, 100.000000, 100.000000, taper::kLinearTaper);
    piParam->setBoundVariable(&mDryWetMixPercentage, boundVariableType::kDouble);
    addPluginParameter(piParam);

    // Smooth control - includes allpasses 7-10 in the tank to get less of an echoing reverb
    piParam = new PluginParameter(controlID::smooth, "Smooth", "On, Off", "On");
    piParam->setBoundVariable(&mIsSmoothed, boundVariableType::kInt);
    addPluginParameter(piParam);

    
    // **--0xEDA5--**
    return true;
}

/**
\brief Additional initialization method called in constructor and reset.
*/
bool PluginCore::initialize()
{
    // Base initialization, refactor from reset()
    mFs = getSampleRate();
    
    /* -------------------- Set static object parameters -------------------- */
    /* -- convert Dattorro's delay times based on the current sampling rate - */
    // Input Allpasses
    mInputAllpass1.setDelaySamples((int) round(mFs/mFsDattorro * 142));
    mInputAllpass2.setDelaySamples((int) round(mFs/mFsDattorro * 107));
    mInputAllpass3.setDelaySamples((int) round(mFs/mFsDattorro * 379));
    mInputAllpass4.setDelaySamples((int) round(mFs/mFsDattorro * 277));
    // Tank modulated allpasses
    mModAllpass1.setDelaySamples((int) round(mFs/mFsDattorro * 672));
    mModAllpass2.setDelaySamples((int) round(mFs/mFsDattorro * 908));
    mModAllpass1.setFeedbackCoefficient(mModAllpass1Diffusion);
    mModAllpass2.setFeedbackCoefficient(mModAllpass1Diffusion);
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

    return true; // handled
}


/**
\brief frame-processing method

Operation:
- decode the plugin type - for synth plugins, fill in the rendering code; for FX plugins, delete the if(synth) portion and add your processing code
- note that MIDI events are fired for each sample interval so that MIDI is tightly sunk with audio
- doSampleAccurateParameterUpdates will perform per-sample interval smoothing

\param processFrameInfo structure of information about *frame* processing

\return true if operation succeeds, false otherwise
*/
bool PluginCore::processAudioFrame(ProcessFrameInfo& processFrameInfo)
{
    // --- decode the channelIOConfiguration and process accordingly
    double inL = processFrameInfo.audioInputFrame[0];
    double inR = processFrameInfo.audioInputFrame[1];
    double input;
    double outWetL, outWetR;
    double outL, outR;
    
    // --- FX Plugin:
    if(mIsMonoInMonoOut)
    {
        input = inL;        // set input to the reverberator
    }

    // --- Mono-In/Stereo-Out
    else if(mIsMonoInStereoOut)
    {
        input = inL;        // set input to the reverberator
    }

    // --- Stereo-In/Stereo-Out
    else if(mIsStereoInStereoOut)
    {
        input = (inL + inR)/2;      // convert stereo to mono to feed the reverberator
    }
    
    /* ---------------------------------------------------------------------- */
    /*                              INPUT SECTION                             */
    /* ---------------------------------------------------------------------- */
    // Pre-delay (pre-delay time can be user controlled)
    mPredelayOut = mPredelay.processAudio(input);
    
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
        outWetL = 0.6*(mTankDelay3Out - mTankAllpass5Out + mTankDelay2Out - mTankAllpass8Out + mTankAllpass10Out);
        outWetR = 0.6*(mTankDelay1Out - mTankAllpass6Out + mTankDelay4Out - mTankAllpass7Out + mTankAllpass9Out);   
    }
    
    // If Smooth == off then allpasses 7 - 10 are bypassed
    if (mIsSmoothed == 0)
    {
        // Compute the accumulators as the outputs from the last tank nodes scaled by decay control
        mTankAccumulator1 = mTankDecay*(mTankDelay2Out);
        mTankAccumulator2 = mTankDecay*(mTankDelay4Out);
        
        // Simplified wet output computation compared to dattorro's
        outWetL = 0.7*(mTankDelay3Out - mTankAllpass5Out + mTankDelay2Out);
        outWetR = 0.7*(mTankDelay1Out - mTankAllpass6Out + mTankDelay4Out);
    }
    
    
    // Dry/Wet
    outL = inL*(1 - mDryWetMix) + outWetL*mDryWetMix;
    outR = inR*(1 - mDryWetMix) + outWetR*mDryWetMix;
    
    processFrameInfo.audioOutputFrame[0] = outL;
    processFrameInfo.audioOutputFrame[1] = outR;
    
    return true; // processed
}

/**
\brief perform any operations after the plugin parameter has been updated; this is one paradigm for
       transferring control information into vital plugin variables or member objects. If you use this
       method you can decode the control ID and then do any cooking that is needed. NOTE: do not
       overwrite bound variables here - this is ONLY for any extra cooking that is required to convert
       the GUI data to meaninful coefficients or other specific modifiers.

\param controlID the control ID value of the parameter being updated
\param controlValue the new control value
\param paramInfo structure of information about why this value is being udpated (e.g as a result of a preset being loaded vs. the top of a buffer process cycle)

\return true if operation succeeds, false otherwise
*/
bool PluginCore::postUpdatePluginParameter(int32_t controlID, double controlValue, ParameterUpdateInfo& paramInfo)
{
    // --- Cooking code for plugin parameters after controls update. Added DeZipper processing for control smoothing ---
    mFs = getSampleRate();
    switch(controlID)
    {
        case controlID::predelayTime:
        {
            // predelay can be user-changed
            int predelaySamplesRaw = (int) round((mPredelayTime*mFs)/1000.0); // force conversion to int. the actual delay value for the rptr is obatined from the predelayTime variable.
            double predelaySamples = (int) round(mDeZipperPredelaySamples.processDeZipper(predelaySamplesRaw));
            mPredelay.setDelaySamples(predelaySamples); 
            return true;    /// handled
        }
        case controlID::inputLowpassFc:
        {
            double normFreq = dspLib::normalizeFreq(mInputLowpassFc, mFs);
            double inputLowpassNormWc = mDeZipperInputLowpassNormWc.processDeZipper(normFreq);
            mInputLowpass.setNormalizedCutoffFrequency(inputLowpassNormWc);
            return true;    /// handled
        }
        case controlID::inputHighpassFc:
        {
            double normFreq = dspLib::normalizeFreq(mInputHighpassFc, mFs);
            double inputHighpassNormWc = mDeZipperInputHighpassNormWc.processDeZipper(normFreq);
            mInputHighpass.setNormalizedCutoffFrequency(inputHighpassNormWc);
            return true;    /// handled
        }
        case controlID::inputDiffusion:
        {
            // input diffusion 3 gets varied along with diffusion 1
            // might change to /6?
            double inputAllpass3DiffusionRaw = 0.625 + (mInputAllpass1DiffusionCtrl - 0.5)/6.0;
            double inputAllpass1Diffusion = mDeZipperInputDiffusionAllpass1.processDeZipper(mInputAllpass1DiffusionCtrl);
            double inputAllpass3Diffusion = mDeZipperInputDiffusionAllpass3.processDeZipper(inputAllpass3DiffusionRaw);
            mInputAllpass1.setFeedbackCoefficient(inputAllpass1Diffusion);
            mInputAllpass2.setFeedbackCoefficient(inputAllpass1Diffusion);
            mInputAllpass3.setFeedbackCoefficient(inputAllpass3Diffusion);
            mInputAllpass4.setFeedbackCoefficient(inputAllpass3Diffusion);
            return true;    /// handled
        }
        case controlID::decay:
        {
            // decay also affects the allpasses feedback in the tank (values taken from dattorro's )
            double tankDiffusionRaw = mTankDecayCtrl + 0.15;
            if (tankDiffusionRaw < 0.15) tankDiffusionRaw = 0.15;
            if (tankDiffusionRaw > 0.50) tankDiffusionRaw = 0.50;
            mTankDecay = mDeZipperDecay.processDeZipper(mTankDecayCtrl);
            
            double tankAllpassDiffusion = mDeZipperTankAllpassDiffusion.processDeZipper(tankDiffusionRaw);

            // Update diffusion coefficients of all AllPasses in the tank
            mTankAllpass5.setFeedbackCoefficient(tankAllpassDiffusion);
            mTankAllpass6.setFeedbackCoefficient(tankAllpassDiffusion);    
            mTankAllpass7.setFeedbackCoefficient(tankAllpassDiffusion);
            mTankAllpass8.setFeedbackCoefficient(tankAllpassDiffusion);
            mTankAllpass9.setFeedbackCoefficient(tankAllpassDiffusion);
            mTankAllpass10.setFeedbackCoefficient(tankAllpassDiffusion);

            return true;    /// handled
        }
        case controlID::drive:
        {
            double saturatorDrive = mDeZipperSaturationDrive.processDeZipper(mSaturatorDriveCtrl);
            mSaturator.setDrive(saturatorDrive);
            return true;    /// handled
        }
        case controlID::hfDamping:
        {
            // hard clip the value to avoid it going to +inf.
            if (mTankLowpassFc > 21999)
            {
                mTankLowpassFc = 21999;
            }
            if (mTankLowpassFc < 400)
            {
                mTankLowpassFc = 400;
            }
            // Updating
            double normFreq = dspLib::normalizeFreq(mTankLowpassFc, mFs);
            double tankLowpassNormWc = mDeZipperTankLowpassNormWc.processDeZipper(normFreq);
            mTankLowpass1.setNormalizedCutoffFrequency(tankLowpassNormWc);
            mTankLowpass2.setNormalizedCutoffFrequency(tankLowpassNormWc);
            return true;    /// handled
        }
        case controlID::lfDamping:
        {
            // Updating
            double normFreq = dspLib::normalizeFreq(mTankHighpassFc, mFs);
            double tankHighpassNormWc = mDeZipperTankHighpassNormWc.processDeZipper(normFreq);
            mTankHighpass1.setNormalizedCutoffFrequency(tankHighpassNormWc);
            mTankHighpass2.setNormalizedCutoffFrequency(tankHighpassNormWc);
            return true;    /// handled
        }
        case controlID::mixPercentage:
        {
            double dryWetMix = mDryWetMixPercentage/100.0;
            mDryWetMix = mDeZipperDryWetMix.processDeZipper(dryWetMix);
            return true;    /// handled
        }
        case controlID::smooth:
        {
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
            return true;    /// handled
        }

        default:
            return false;   /// not handled
    }

    return false;
}