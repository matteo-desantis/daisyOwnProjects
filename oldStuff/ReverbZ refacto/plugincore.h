// -----------------------------------------------------------------------------
//    ASPiK Plugin Kernel File:  plugincore.h
//
/**
    \file   plugincore.h
    \author Will Pirkle
    \date   17-September-2018
    \brief  base class interface file for ASPiK plugincore object
            - http://www.aspikplugins.com
            - http://www.willpirkle.com
*/
// MD 8/10/2025 - Cleaned up everything not related to the core ReverbZ algorithm
// -----------------------------------------------------------------------------
#ifndef __pluginCore_h__
#define __pluginCore_h__

/* #include "pluginbase.h" */
#include "../dspLib/AllPass.hpp"
#include "../dspLib/DelayLine.hpp"  
#include "../dspLib/DeZipper.hpp"           
#include "../dspLib/OnePoleFilter.hpp"
#include "../dspLib/Saturator.hpp"
#include "../dspLib/mathUtils.hpp"
// **--0x7F1F--**

/* ------------------- Dummy definitions for intellisense ------------------- */
// --- Dummy base class and dependencies for Intellisense ---
#ifndef __pluginbase_h__
#define __pluginbase_h__
class PluginBase {};
#endif

/* -------------------------------------------------------------------------- */

// **--0x0F1F--**

// GUI control IDs
enum controlID
{
    predelayTime,           // 0
    inputLowpassFc,         // 1
    inputHighpassFc,        // 2
    inputDiffusion,         // 3
    decay,                  // 4
    drive,                  // 5
    hfDamping,              // 6
    lfDamping,              // 7
    mixPercentage,          // 8
    smooth                  // 9
};

/**
\class PluginCore
\ingroup ASPiK-Core
\brief
The PluginCore object is the default PluginBase derived object for ASPiK projects.
Note that you are fre to change the name of this object (as long as you change it in the compiler settings, etc...)


PluginCore Operations:
- overrides the main processing functions from the base class
- performs reset operation on sub-modules
- processes audio
- processes messages for custom views
- performs pre and post processing functions on parameters and audio (if needed)

\author Will Pirkle http://www.willpirkle.com
\remark This object is included in Designing Audio Effects Plugins in C++ 2nd Ed. by Will Pirkle
\version Revision : 1.0
\date Date : 2018 / 09 / 7
*/
class PluginCore : public PluginBase
{
public:
    PluginCore();

    /** Destructor: empty in default version */
    virtual ~PluginCore(){}

    // --- PluginBase Overrides ---
    //
    /** this is the creation function for all plugin parameters */
    bool initPluginParameters();

    /** process frames of data */
    virtual bool processAudioFrame(ProcessFrameInfo& processFrameInfo);

    /** this can be called: 1) after bound variable has been updated or 2) after smoothing occurs  */
    virtual bool postUpdatePluginParameter(int32_t controlID, double controlValue, ParameterUpdateInfo& paramInfo);

    /*
    MD define dummy getSampleRate()
    */
    double getSampleRate();

    // Additinal initialiation method called in constructor and reset.
    bool initialize();


private:
    //  **--0x07FD--**
    /*
    Dummy input channel settings
    */
    bool mIsMonoInMonoOut = false;      // mono in, mono out
    bool mIsMonoInStereoOut = true;     // mono in, stereo out
    bool mIsStereoInStereoOut = false;  // stereo in, stereo out

    double mFs;                                     // Project's sampling frequency
    double mFsDattorro = 29761;                     // Dattorro's original sampling frequency

    // DeZippers for changing-parameters
    dspLib::DeZipper mDeZipperPredelaySamples;
    dspLib::DeZipper mDeZipperInputLowpassNormWc;
    dspLib::DeZipper mDeZipperInputHighpassNormWc;
    dspLib::DeZipper mDeZipperInputDiffusionAllpass1;
    dspLib::DeZipper mDeZipperInputDiffusionAllpass3;
    dspLib::DeZipper mDeZipperDecay;
    dspLib::DeZipper mDeZipperSaturationDrive;
    dspLib::DeZipper mDeZipperTankLowpassNormWc;
    dspLib::DeZipper mDeZipperTankHighpassNormWc;
    dspLib::DeZipper mDeZipperTankAllpassDiffusion;
    dspLib::DeZipper mDeZipperDryWetMix;

    /* ---------------------------- INPUT SECTION --------------------------- */
    // Predelay - DelayLine Object
    dspLib::DelayLine mPredelay;                           
    double mPredelayTime = 0.000000;
    double mPredelayOut;                            // delay line out
    
    // Input Lowpass Filter    
    dspLib::OnePoleFilter mInputLowpass;                          
    double mInputLowpassFc = 22000.000000;          // cutoff frequency
    double mInputLowpassOut;                        // filter output
    
    // Input Highpass Filter
    dspLib::OnePoleFilter mInputHighpass;           // input highpass variables
    double mInputHighpassFc = 10.000000;            // cutoff frequency
    double mInputHighpassOut;                       // filter output
    
    // Input Diffusers - AllPass Objects
    dspLib::AllPass mInputAllpass1;                 // input diffusion all-passes variables
    double mInputAllpass1DiffusionCtrl = 0.750000;  // fb coefficient
    double mInputAllpass1Out;                       // filter output
   
    dspLib::AllPass mInputAllpass2;
    double mInputAllpass2Out;
    
    dspLib::AllPass mInputAllpass3;
    double mInputAllpass3Out;
    
    dspLib::AllPass mInputAllpass4;
    double mInputAllPass4Out;
   
    /* ---------------------------- TANK SECTION --------------------------- */
    // Tank Inputs, Accumulators and Parameters
    double mTankInput1 = 0.0;                       // tank inputs initialised to 0.0
    double mTankInput2 = 0.0;
    double mTankAccumulator1 = 0.0;                 // tank accumulators initialised to 0.0
    double mTankAccumulator2 = 0.0;
    double mTankDecayCtrl = 0.500000;
    double mTankDecay;
    
    // Tank Allpasses with delayline modulation
    dspLib::AllPass mModAllpass1;                   // modulated tank allpass filters
    double mModAllpass1Diffusion = 0.70;
    double mModAllpass1Out;
    
    dspLib::AllPass mModAllpass2;
    double mModAllpass2Out;
    
    dspLib::DelayLine mTankDelay1;                  // tank delaylines 1 and 3
    double mTankDelay1Out;
    
    dspLib::DelayLine mTankDelay3;
    double mTankDelay3Out;
    
    dspLib::Saturator mSaturator;
    double mSaturatorDriveCtrl = 0.100000;
    double mSaturator1Out;
    double mSaturator2Out;
    
    dspLib::OnePoleFilter mTankLowpass1;            // tank hf damping
    double mTankLowpassFc = 5000.000000;
    double mTankLowpass1Out;
    
    dspLib::OnePoleFilter mTankLowpass2;
    double mTankLowpass2Out;
    
    dspLib::OnePoleFilter mTankHighpass1;           // tank highpass
    double mTankHighpassFc = 0.000000;
    double mTankHighpass1Out;
    
    dspLib::OnePoleFilter mTankHighpass2;
    double mTankHighpass2Out;
    
    dspLib::AllPass mTankAllpass5;
    double mTankAllpass5Out;
    
    dspLib::AllPass mTankAllpass6;
    double mTankAllpass6Out;
    
    dspLib::DelayLine mTankDelay2;
    double mTankDelay2Out;
    
    dspLib::DelayLine mTankDelay4;
    double mTankDelay4Out;

    /* ----------------------- SMOOTH TANK SECTION ----------------------- */
    int mIsSmoothed = 0;
    enum smoothEnum {On, Off};      // on = 0, off = 1 TODO: Seems unused
    
    dspLib::AllPass mTankAllpass7;
    double mTankAllpass7Out;
    
    dspLib::AllPass mTankAllpass8;
    double mTankAllpass8Out;
    
    dspLib::AllPass mTankAllpass9;
    double mTankAllpass9Out;
    
    dspLib::AllPass mTankAllpass10;
    double mTankAllpass10Out;
    
    /* ------------------------------ DRY / WET ----------------------------- */
    double mDryWetMix;
    double mDryWetMixPercentage = 100.000000;
    
    // **--0x1A7F--**
    // --- end member variables
};

#endif /* define __pluginCore_h__ */

