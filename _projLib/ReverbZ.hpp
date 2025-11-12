/** ------------------------------------------------------------------------- 
    ReverbZ.hpp - Header file for ReverbZ class.
    Saturated Reverb algorithm.

    High-level implementation - No hardware-specific code here.


    Matteo Desantis 31-Oct-2025
*/

#pragma once
#ifndef ReverbZ_hpp
#define ReverbZ_hpp

// Include used dspLib components
#include "../../dspLib/AllPass.hpp"
#include "../../dspLib/DelayLine.hpp"  
#include "../../dspLib/DeZipper.hpp"           
#include "../../dspLib/OnePoleFilter.hpp"
#include "../../dspLib/Saturator.hpp"
#include "../../dspLib/mathUtils.hpp"

using namespace dspLib;

namespace projLib {

class ReverbZ {
    public:
        ReverbZ(int sampleRate);
        ~ReverbZ();

        void init();
        void processAudioMono(double inputSample);
        void processAudioStereo(double inputSampleL, double inputSampleR);
        void setControlParameters(double predelayTime,
                                  double inputLowpassFc,
                                  double inputHighpassFc,
                                  double inputDiffusion,
                                  double decay,
                                  double drive,
                                  double hfDamping,
                                  double lfDamping,
                                  double mixPercentage,
                                  int smooth);

        // Dry-Wet Mix outputs
        double mOutL, mOutR, mOutMono;
    private:
        void processAudioPrivate(double inputSample);

        /* ----------------------------- Outputs ---------------------------- */
        // FX 100% wet outputs
        double mOutWetL, mOutWetR;



        /* ------------------------------------------------------------------ */
        /*         All internal dspLib components as member variables         */
        /* ------------------------------------------------------------------ */
        double mFs;                                     // Project's sampling frequency
        const double mFsDattorro = 29761;                     // Dattorro's original sampling frequency
        const double mBufferScaleFactor = 1.0;                // Buffer scale factor for adapting delay lengths to different sample rates

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
        double mInputLowpassOut;                        // filter output
        
        // Input Highpass Filter
        dspLib::OnePoleFilter mInputHighpass;           // input highpass variables
        double mInputHighpassOut;                       // filter output
        
        // Input Diffusers - AllPass Objects
        dspLib::AllPass mInputAllpass1;                 // input diffusion all-passes variables
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
        double mTankDecay;
        
        // Tank Allpasses with delayline modulation
        dspLib::AllPass mModAllpass1;                   // modulated tank allpass filters
        double mModAllpass1Out;
        
        dspLib::AllPass mModAllpass2;
        double mModAllpass2Out;
        
        dspLib::DelayLine mTankDelay1;                  // tank delaylines 1 and 3
        double mTankDelay1Out;
        
        dspLib::DelayLine mTankDelay3;
        double mTankDelay3Out;
        
        dspLib::Saturator mSaturator;
        double mSaturator1Out;
        double mSaturator2Out;
        
        dspLib::OnePoleFilter mTankLowpass1;            // tank hf damping
        double mTankLowpass1Out;
        
        dspLib::OnePoleFilter mTankLowpass2;
        double mTankLowpass2Out;
        
        dspLib::OnePoleFilter mTankHighpass1;           // tank highpass
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
};

}   // namespace projLib

#endif /* ReverbZ_hpp */
