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
#include "../../dspLib/OnePoleFilter.hpp"
#include "../../dspLib/Saturator.hpp"
#include "../../dspLib/mathUtils.hpp"

namespace projLib {

template<std::size_t MaxSamples>
class ReverbZ {
    public:
        ReverbZ(int sampleRate);
        ~ReverbZ();

        void init();
        void processAudioMono(float inputSample);
        void processAudioStereo(float inputSampleL, float inputSampleR);
        void setControlParameters(float predelayTime,
                                  float inputLowpassFc,
                                  float inputHighpassFc,
                                  float inputDiffusion,
                                  float decay,
                                  float drive,
                                  float hfDamping,
                                  float lfDamping,
                                  float mixPercentage,
                                  int smooth);

        // Dry-Wet Mix outputs
        float mOutL, mOutR, mOutMono;
    private:
        void processAudioPrivate(float inputSample);

        /* ----------------------------- Outputs ---------------------------- */
        // FX 100% wet outputs
        float mOutWetL_, mOutWetR_;



        /* ------------------------------------------------------------------ */
        /*         All internal dspLib components as member variables         */
        /* ------------------------------------------------------------------ */
        int mFs_;                                       // Project's sampling frequency
        const int mFsDattorro_ = 29761;                 // Dattorro's original sampling frequency

        /* ---------------------------- INPUT SECTION --------------------------- */
        // Predelay - DelayLine Object
        dspLib::DelayLine<MaxSamples> mPredelay_;                           
        float mPredelayTime_ = 0.0f;
        float mPredelayOut_;                            // delay line out
        
        // Input Lowpass Filter    
        dspLib::OnePoleFilter mInputLowpass_;                          
        float mInputLowpassOut_;                        // filter output
        
        // Input Highpass Filter
        dspLib::OnePoleFilter mInputHighpass_;          // input highpass variables
        float mInputHighpassOut_;                       // filter output
        
        // Input Diffusers - AllPass Objects
        dspLib::AllPass<MaxSamples> mInputAllpass1_;    // input diffusion all-passes variables
        float mInputAllpass1Out_;                       // filter output
    
        dspLib::AllPass<MaxSamples> mInputAllpass2_;
        float mInputAllpass2Out_;
        
        dspLib::AllPass<MaxSamples> mInputAllpass3_;
        float mInputAllpass3Out_;
        
        dspLib::AllPass<MaxSamples> mInputAllpass4_;
        float mInputAllPass4Out_;
    
        /* ---------------------------- TANK SECTION --------------------------- */
        // Tank Inputs, Accumulators and Parameters
        float mTankInput1_ = 0.0f;                      // tank inputs initialised to 0.0
        float mTankInput2_ = 0.0f;
        float mTankAccumulator1_ = 0.0f;                // tank accumulators initialised to 0.0
        float mTankAccumulator2_ = 0.0f;
        float mTankDecay_ = 0.5f;                       // tank decay control
        
        // Tank Allpasses with delayline modulation
        dspLib::AllPass<MaxSamples> mModAllpass1_;      // modulated tank allpass filters
        float mModAllpass1Out_;
        
        dspLib::AllPass<MaxSamples> mModAllpass2_;
        float mModAllpass2Out_;
        
        dspLib::DelayLine<MaxSamples> mTankDelay1_;     // tank delaylines 1 and 3
        float mTankDelay1Out_;
        
        dspLib::DelayLine<MaxSamples> mTankDelay3_;
        float mTankDelay3Out_;
        
        dspLib::Saturator mSaturator_;
        float mSaturator1Out_;
        float mSaturator2Out_;
        
        dspLib::OnePoleFilter mTankLowpass1_;           // tank hf damping
        float mTankLowpass1Out_;
        
        dspLib::OnePoleFilter mTankLowpass2_;
        float mTankLowpass2Out_;
        
        dspLib::OnePoleFilter mTankHighpass1_;          // tank highpass
        float mTankHighpass1Out_;
        
        dspLib::OnePoleFilter mTankHighpass2_;
        float mTankHighpass2Out_;
        
        dspLib::AllPass<MaxSamples> mTankAllpass5_;
        float mTankAllpass5Out_;
        
        dspLib::AllPass<MaxSamples> mTankAllpass6_;
        float mTankAllpass6Out_;
        
        dspLib::DelayLine<MaxSamples> mTankDelay2_;
        float mTankDelay2Out_;
        
        dspLib::DelayLine<MaxSamples> mTankDelay4_;
        float mTankDelay4Out_;

        /* ----------------------- SMOOTH TANK SECTION ----------------------- */
        int mIsSmoothed_ = 0;
        
        dspLib::AllPass<MaxSamples> mTankAllpass7_;
        float mTankAllpass7Out_;
        
        dspLib::AllPass<MaxSamples> mTankAllpass8_;
        float mTankAllpass8Out_;
        
        dspLib::AllPass<MaxSamples> mTankAllpass9_;
        float mTankAllpass9Out_;
        
        dspLib::AllPass<MaxSamples> mTankAllpass10_;
        float mTankAllpass10Out_;
        
        /* ------------------------------ DRY / WET ----------------------------- */
        float mDryWetMix_;
};

}   // namespace projLib

/* Include Implentation file */
#include "ReverbZ.tpp"

#endif /* ReverbZ_hpp */
