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
// -----------------------------------------------------------------------------
#ifndef __pluginCore_h__
#define __pluginCore_h__

/* #include "pluginbase.h" */
#include "COMMON/DelayClass.hpp"            // Modified from #include "../../../../COMMON/<ClassName>.hpp" to this form to avoid error in this reference code
#include "COMMON/AllPass.hpp"
#include "COMMON/LowPass.hpp"
#include "COMMON/HighPass.hpp"
#include "COMMON/Saturator.hpp"
#include "COMMON/DeZipper.hpp"
// **--0x7F1F--**


// **--0x0F1F--**

enum controlID
{
    predelay_time,          // 0
    input_lp_fc,            // 1
    input_hp_fc,            // 2
    input_diffusion,        // 3
    decay,                  // 4
    drive,                  // 5
    hf_damping,             // 6
    lf_damping,             // 7
    mix_perc,               // 8
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

    /** called when plugin is loaded, a new audio file is playing or sample rate changes */
    virtual bool reset(ResetInfo& resetInfo);

    /** one-time post creation init function; pluginInfo contains path to this plugin */
    virtual bool initialize(PluginInfo& _pluginInfo);

    // --- preProcess: sync GUI parameters here; override if you don't want to use automatic variable-binding
    virtual bool preProcessAudioBuffers(ProcessBufferInfo& processInfo);

    /** process frames of data */
    virtual bool processAudioFrame(ProcessFrameInfo& processFrameInfo);

    // --- uncomment and override this for buffer processing; see base class implementation for
    //     help on breaking up buffers and getting info from processBufferInfo
    //virtual bool processAudioBuffers(ProcessBufferInfo& processBufferInfo);

    /** preProcess: do any post-buffer processing required; default operation is to send metering data to GUI  */
    virtual bool postProcessAudioBuffers(ProcessBufferInfo& processInfo);

    /** called by host plugin at top of buffer proccess; this alters parameters prior to variable binding operation  */
    virtual bool updatePluginParameter(int32_t controlID, double controlValue, ParameterUpdateInfo& paramInfo);

    /** called by host plugin at top of buffer proccess; this alters parameters prior to variable binding operation  */
    virtual bool updatePluginParameterNormalized(int32_t controlID, double normalizedValue, ParameterUpdateInfo& paramInfo);

    /** this can be called: 1) after bound variable has been updated or 2) after smoothing occurs  */
    virtual bool postUpdatePluginParameter(int32_t controlID, double controlValue, ParameterUpdateInfo& paramInfo);

    /** this is ony called when the user makes a GUI control change */
    virtual bool guiParameterChanged(int32_t controlID, double actualValue);

    /** processMessage: messaging system; currently used for custom/special GUI operations */
    virtual bool processMessage(MessageInfo& messageInfo);

    /** processMIDIEvent: MIDI event processing */
    virtual bool processMIDIEvent(midiEvent& event);

    /** specialized joystick servicing (currently not used) */
    virtual bool setVectorJoystickParameters(const VectorJoystickData& vectorJoysickData);

    /** create the presets */
    bool initPluginPresets();

    // --- BEGIN USER VARIABLES AND FUNCTIONS -------------------------------------- //
    //       Add your variables and methods here

   

    // --- END USER VARIABLES AND FUNCTIONS -------------------------------------- //

private:
    //  **--0x07FD--**
    double Fs;                                      // sampling frequency
    double Fs_D = 29761;                            // Dattorro's original sampling frequency
    
    DelayClass predelay;                            // predelay variables
    double predelay_time = 0.000000;
    int predelay_samples;                           // delay line length (samples)
    double predelay_out;                            // delay line out
    
    LowPass input_lowpass;                          // input lowpass variables
    double input_lp_fc = 22000.000000;              // cutoff frequency
    double input_lp_norm_wc;                        // normalised angular frequency
    DeZipper dz_input_lp_norm_wc;
    double input_lp_norm_wc_dz;
    double input_lp_out;                            // filter output
    
    HighPass input_highpass;                        // input highpass variables
    double input_hp_fc = 10.000000;                 // cutoff frequency
    double input_hp_norm_wc;                        // normalised angular frequency
    DeZipper dz_input_hp_norm_wc;
    double input_hp_norm_wc_dz;
    double input_hp_out;                            // filter output
    
    AllPass allpass1;                               // input diffusion all-passes variables
    int delay_samples_allpass1;                     // delay samples in the delay line
    double input_diffusion = 0.750000;              // fb coefficient
    DeZipper dz_input_diffusion;
    double input_diffusion_dz;
    double allpass1_out;                            // filter output
   
    AllPass allpass2;
    int delay_samples_allpass2;
    double allpass2_out;
    
    AllPass allpass3;
    int delay_samples_allpass3;
    double input_diffusion2 = 0.625;
    DeZipper dz_input_diffusion2;
    double input_diffusion2_dz;
    double allpass3_out;
    
    AllPass allpass4;
    int delay_samples_allpass4;
    double allpass4_out;
   
    double tank_input1 = 0.0;                       // tank inputs initialised to 0.0
    double tank_input2 = 0.0;
    double tank_accumulator1 = 0.0;                 // tank accumulators initialised to 0.0
    double tank_accumulator2 = 0.0;
    double decay = 0.500000;
    DeZipper dz_decay;
    double decay_dz;
    
    AllPass mod_allpass1;                           // modulated tank allpass filters
    int delay_samples_mod_allpass1;
    double tank_diffusion1 = 0.70;
    double mod_allpass1_out;
    int mod_depth1;
    
    AllPass mod_allpass2;
    int delay_samples_mod_allpass2;
    double mod_allpass2_out;
    int mod_depth2;
    
    DelayClass delay1_tank;                         // tank delaylines 1 and 3
    int delay_samples_delay1_tank;
    double delay1_tank_out;
    
    DelayClass delay3_tank;
    int delay_samples_delay3_tank;
    double delay3_tank_out;
    
    Saturator saturator1;
    double drive = 0.100000;
    DeZipper dz_drive;
    double drive_dz;
    double saturator1_out;
    
    Saturator saturator2;                           // drive is the same as above
    double saturator2_out;
    
    LowPass tank_lp1;                               // tank hf damping
    double hf_damping = 5000.000000;
    double hf_abs_damping;
    double tank_lp_norm_wc;
    DeZipper dz_tank_lp_norm_wc;
    double tank_lp_norm_wc_dz;
    double tank_lp1_out;
    
    LowPass tank_lp2;
    double tank_lp2_out;
    
    HighPass tank_hp1;                               // tank highpass
    double lf_damping = 0.000000;
    double tank_hp_norm_wc;
    DeZipper dz_tank_hp_norm_wc;
    double tank_hp_norm_wc_dz;
    double tank_hp1_out;
    
    HighPass tank_hp2;
    double tank_hp2_out;
    
    AllPass allpass5;
    int delay_samples_allpass5;
    double tank_diffusion2 = 0.5;
    DeZipper dz_tank_diffusion2;
    double tank_diffusion2_dz;
    double allpass5_out;
    
    AllPass allpass6;
    int delay_samples_allpass6;
    double allpass6_out;
    
    DelayClass delay2_tank;
    int delay_samples_delay2_tank;
    double delay2_tank_out;
    
    DelayClass delay4_tank;
    int delay_samples_delay4_tank;
    double delay4_tank_out;
    
    // Add four more allpasses to get a smoother response that can be bypassed via smooth control
    int smooth = 0;
    enum class smoothEnum { On, Off };      // on = 0, off = 1
    
    AllPass allpass7;
    int delay_samples_allpass7;
    double allpass7_out;
    
    AllPass allpass8;
    int delay_samples_allpass8;
    double allpass8_out;
    
    AllPass allpass9;
    int delay_samples_allpass9;
    double allpass9_out;
    
    AllPass allpass10;
    int delay_samples_allpass10;
    double allpass10_out;
    
    double mix;
    DeZipper dz_mix;
    double mix_dz;
    double mix_perc = 100.000000;
    
    // **--0x1A7F--**
    // --- end member variables

public:
    /** static description: bundle folder name

    \return bundle folder name as a const char*
    */
    static const char* getPluginBundleName();

    /** static description: name

    \return name as a const char*
    */
    static const char* getPluginName();

    /** static description: short name

    \return short name as a const char*
    */
    static const char* getShortPluginName();

    /** static description: vendor name

    \return vendor name as a const char*
    */
    static const char* getVendorName();

    /** static description: URL

    \return URL as a const char*
    */
    static const char* getVendorURL();

    /** static description: email

    \return email address as a const char*
    */
    static const char* getVendorEmail();

    /** static description: Cocoa View Factory Name

    \return Cocoa View Factory Name as a const char*
    */
    static const char* getAUCocoaViewFactoryName();

    /** static description: plugin type

    \return type (FX or Synth)
    */
    static pluginType getPluginType();

    /** static description: VST3 GUID

    \return VST3 GUID as a const char*
    */
    static const char* getVSTFUID();

    /** static description: 4-char code

    \return 4-char code as int
    */
    static int32_t getFourCharCode();

    /** initalizer */
    bool initPluginDescriptors();

};




#endif /* defined(__pluginCore_h__) */

