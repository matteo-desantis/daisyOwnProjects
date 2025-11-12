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
// -----------------------------------------------------------------------------
#include "plugincore.h"
#include "plugindescription.h"
#include <cmath>

/**
\brief PluginCore constructor is launching pad for object initialization

Operations:
- initialize the plugin description (strings, codes, numbers, see initPluginDescriptors())
- setup the plugin's audio I/O channel support
- create the PluginParameter objects that represent the plugin parameters (see FX book if needed)
- create the presets
*/
PluginCore::PluginCore()
{
    // --- describe the plugin; call the helper to init the static parts you setup in plugindescription.h
    initPluginDescriptors();

    // --- default I/O combinations
    // --- for FX plugins
    if (getPluginType() == kFXPlugin)
    {
        addSupportedIOCombination({ kCFMono, kCFMono });
        addSupportedIOCombination({ kCFMono, kCFStereo });
        addSupportedIOCombination({ kCFStereo, kCFStereo });
    }
    else // --- synth plugins have no input, only output
    {
        addSupportedIOCombination({ kCFNone, kCFMono });
        addSupportedIOCombination({ kCFNone, kCFStereo });
    }

    // --- for sidechaining, we support mono and stereo inputs; auxOutputs reserved for future use
    addSupportedAuxIOCombination({ kCFMono, kCFNone });
    addSupportedAuxIOCombination({ kCFStereo, kCFNone });

    // --- create the parameters
    initPluginParameters();

    // --- create the presets
    initPluginPresets();
}

/**
\brief create all of your plugin parameters here

\return true if parameters were created, false if they already existed
*/
bool PluginCore::initPluginParameters()
{
    if (pluginParameterMap.size() > 0)
        return false;

    // --- Add your plugin parameter instantiation code bewtween these hex codes
    // **--0xDEA7--**
    PluginParameter* piParam = nullptr;
    
    // Predelay Time
    piParam = new PluginParameter(controlID::predelay_time, "Predelay Time", "ms", controlVariableType::kDouble, 0.000000, 100.000000, 0.00000, taper::kLinearTaper);
    piParam->setBoundVariable(&predelay_time, boundVariableType::kDouble);
    addPluginParameter(piParam);
    
    // Input lowpass filter cutoff frequency
    piParam = new PluginParameter(controlID::input_lp_fc, "Input LowPass Fc", "Hz", controlVariableType::kDouble, 500.000000, 22000.000000, 22000.000000, taper::kAntiLogTaper);
    piParam->setBoundVariable(&input_lp_fc, boundVariableType::kDouble);
    addPluginParameter(piParam);
    
    // Input highpass filter cutoff frequency
    piParam = new PluginParameter(controlID::input_hp_fc, "Input HighPass Fc", "Hz", controlVariableType::kDouble, 10.000000, 3000.000000, 10.000000, taper::kAntiLogTaper);
    piParam->setBoundVariable(&input_hp_fc, boundVariableType::kDouble);
    addPluginParameter(piParam);

    // Input diffusion
    piParam = new PluginParameter(controlID::input_diffusion, "Input Diffusion", "", controlVariableType::kDouble, 0.000000, 0.999000, 0.750000, taper::kLinearTaper);
    piParam->setBoundVariable(&input_diffusion, boundVariableType::kDouble);
    addPluginParameter(piParam);
    
    // Tank decay control
        piParam = new PluginParameter(controlID::decay, "Decay", "", controlVariableType::kDouble, 0.000000, 0.850000, 0.500000, taper::kLinearTaper);
        piParam->setBoundVariable(&decay, boundVariableType::kDouble);
        addPluginParameter(piParam);
   
    // Saturation amount control
    piParam = new PluginParameter(controlID::drive, "Drive", "dB", controlVariableType::kDouble, 0.100000, 20.000000, 0.100000, taper::kLinearTaper);
    piParam->setBoundVariable(&drive, boundVariableType::kDouble);
    addPluginParameter(piParam);

    // HF Damping
    /// set the opposite way round to get a the desired behaviour in the GUI
    piParam = new PluginParameter(controlID::hf_damping, "HF Damping", "Hz", controlVariableType::kDouble, -20000.000000, -400.000000, -5000.000000, taper::kLogTaper);
    piParam->setBoundVariable(&hf_damping, boundVariableType::kDouble);
    addPluginParameter(piParam);

    // HF Damping
    piParam = new PluginParameter(controlID::lf_damping, "LF Damping", "Hz", controlVariableType::kDouble, 0.000000, 3000.000000, 0.000000, taper::kAntiLogTaper);
    piParam->setBoundVariable(&lf_damping, boundVariableType::kDouble);
    addPluginParameter(piParam);

    // Dry/Wet control
    piParam = new PluginParameter(controlID::mix_perc, "Dry/Wet ", "%", controlVariableType::kDouble, 0.000000, 100.000000, 100.000000, taper::kLinearTaper);
    piParam->setBoundVariable(&mix_perc, boundVariableType::kDouble);
    addPluginParameter(piParam);

    // Smooth control - includes allpasses 7 and 8 in the tank to get less of an echoing reverb
    piParam = new PluginParameter(controlID::smooth, "Smooth", "On, Of", "On");
    piParam->setBoundVariable(&smooth, boundVariableType::kInt);
    addPluginParameter(piParam);

    
    // **--0xEDA5--**
   
    
    // --- BONUS Parameter
    // --- SCALE_GUI_SIZE
    PluginParameter* piParamBonus = new PluginParameter(SCALE_GUI_SIZE, "Scale GUI", "tiny,small,medium,normal,large,giant", "normal");
    addPluginParameter(piParamBonus);

    // --- create the super fast access array
    initPluginParameterArray();

    return true;
}

/**
\brief initialize object for a new run of audio; called just before audio streams

Operation:
- store sample rate and bit depth on audioProcDescriptor - this information is globally available to all core functions
- reset your member objects here

\param resetInfo structure of information about current audio format

\return true if operation succeeds, false otherwise
*/
bool PluginCore::reset(ResetInfo& resetInfo)
{
    // --- save for audio processing
    audioProcDescriptor.sampleRate = resetInfo.sampleRate;
    audioProcDescriptor.bitDepth = resetInfo.bitDepth;
    
    Fs = getSampleRate();
    
    // convert Dattorro's delay times based on the current sampling rate
    delay_samples_allpass1 = (int) round(Fs/Fs_D * 142);
    delay_samples_allpass2 = (int) round(Fs/Fs_D * 107);
    delay_samples_allpass3 = (int) round(Fs/Fs_D * 379);
    delay_samples_allpass4 = (int) round(Fs/Fs_D * 277);
    delay_samples_mod_allpass1 = (int) round(Fs/Fs_D * 672);
    delay_samples_mod_allpass2 = (int) round(Fs/Fs_D * 908);
    delay_samples_delay1_tank = (int) round(Fs/Fs_D * 4453);
    delay_samples_delay3_tank = (int) round(Fs/Fs_D * 3720);
    delay_samples_allpass5 = (int) round(Fs/Fs_D * 1800);
    delay_samples_allpass6 = (int) round(Fs/Fs_D * 2656);
    delay_samples_delay2_tank = (int) round(Fs/Fs_D * 4217);
    delay_samples_delay4_tank = (int) round(Fs/Fs_D * 3163);
    // delay times for allpass 7-10 are prime numbers
    delay_samples_allpass7 = (int) round(Fs/Fs_D * 1511);
    delay_samples_allpass8 = (int) round(Fs/Fs_D * 2003);
    delay_samples_allpass9 = (int) round(Fs/Fs_D * 1709);
    delay_samples_allpass10 = (int) round(Fs/Fs_D * 2411);

    // init tank inputs
    tank_input1 = 0.0;
    tank_input2 = 0.0;
    
    // clear out and re-init delay lines
    predelay.reset();
    allpass1.reset();
    allpass2.reset();
    allpass3.reset();
    allpass4.reset();
    mod_allpass1.reset();
    mod_allpass2.reset();
    delay1_tank.reset();
    delay3_tank.reset();
    allpass5.reset();
    allpass6.reset();
    delay2_tank.reset();
    delay4_tank.reset();
    allpass7.reset();
    allpass8.reset();
    allpass9.reset();
    allpass10.reset();
    
    // set LFO rate for the delay line modulation. The argument passed is the actual frequency [Hz], which is then normalised to f/Fs within the class method.
    mod_allpass1.set_sine_array(Fs, 0.6);
    mod_allpass2.set_sine_array(Fs, 0.8);
    
    // --- other reset inits
    return PluginBase::reset(resetInfo);
}

/**
\brief one-time initialize function called after object creation and before the first reset( ) call

Operation:
- saves structure for the plugin to use; you can also load WAV files or state information here
*/
bool PluginCore::initialize(PluginInfo& pluginInfo)
{
    // --- add one-time init stuff here
    return true;
}

/**
\brief do anything needed prior to arrival of audio buffers

Operation:
- syncInBoundVariables when preProcessAudioBuffers is called, it is *guaranteed* that all GUI control change information
  has been applied to plugin parameters; this binds parameter changes to your underlying variables
- NOTE: postUpdatePluginParameter( ) will be called for all bound variables that are acutally updated; if you need to process
  them individually, do so in that function
- use this function to bulk-transfer the bound variable data into your plugin's member object variables

\param processInfo structure of information about *buffer* processing

\return true if operation succeeds, false otherwise
*/
bool PluginCore::preProcessAudioBuffers(ProcessBufferInfo& processInfo)
{
    // --- sync internal variables to GUI parameters; you can also do this manually if you don't
    //     want to use the auto-variable-binding
    syncInBoundVariables();

    return true;
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
    // --- fire any MIDI events for this sample interval
    processFrameInfo.midiEventQueue->fireMidiEvents(processFrameInfo.currentFrame);

    // --- do per-frame updates; VST automation and parameter smoothing
    doSampleAccurateParameterUpdates();

    // --- decode the channelIOConfiguration and process accordingly
    //
    // --- Synth Plugin:
    // --- Synth Plugin --- remove for FX plugins
    double inL = processFrameInfo.audioInputFrame[0];
    double inR = processFrameInfo.audioInputFrame[1];
    double input;
    double outL_wet, outR_wet;
    double outL, outR;
    
    // --- FX Plugin:
    if(processFrameInfo.channelIOConfig.inputChannelFormat == kCFMono &&
       processFrameInfo.channelIOConfig.outputChannelFormat == kCFMono)
    {
        input = inL;        // set input to the reverberator
    }

    // --- Mono-In/Stereo-Out
    else if(processFrameInfo.channelIOConfig.inputChannelFormat == kCFMono &&
       processFrameInfo.channelIOConfig.outputChannelFormat == kCFStereo)
    {
        input = inL;        // set input to the reverberator
    }

    // --- Stereo-In/Stereo-Out
    else if(processFrameInfo.channelIOConfig.inputChannelFormat == kCFStereo &&
       processFrameInfo.channelIOConfig.outputChannelFormat == kCFStereo)
    {
        input = (inL + inR)/2;      // convert stereo to mono to feed the reverberator
    }
    
    // Dezipping
    /// input lowpass fc
    input_lp_norm_wc_dz = dz_input_lp_norm_wc.smooth(input_lp_norm_wc, 0.95);
    /// input highpass fc
    input_hp_norm_wc_dz = dz_input_hp_norm_wc.smooth(input_hp_norm_wc, 0.95);
    /// input diffusion fc
    input_diffusion_dz = dz_input_diffusion.smooth(input_diffusion, 0.999);
    input_diffusion2_dz = dz_input_diffusion2.smooth(input_diffusion2, 0.95);
    /// decay
    decay_dz = dz_decay.smooth(decay, 0.95);
    /// tank diffusion
    tank_diffusion2_dz = dz_tank_diffusion2.smooth(tank_diffusion2, 0.95);
    /// drive
    drive_dz = dz_drive.smooth(drive, 0.95);
    /// HF damp
    tank_lp_norm_wc_dz = dz_tank_lp_norm_wc.smooth(tank_lp_norm_wc, 0.95);
    /// LF damp
    tank_hp_norm_wc_dz = dz_tank_hp_norm_wc.smooth(tank_hp_norm_wc, 0.99);
    /// dry/wet
    mix_dz = dz_mix.smooth(mix, 0.95);
    
    // PRE-DELAY
    predelay_out = predelay.processaudio(input, predelay_samples);
    
    // INPUT LOWPASS FILTER
    input_lp_out = input_lowpass.LP(predelay_out, input_lp_norm_wc_dz);
    
    // INPUT HIGHPASS FILTER
    input_hp_out = input_highpass.HP(input_lp_out, input_hp_norm_wc_dz);
    
    // INPUT DIFFUSERS
    allpass1_out = allpass1.processaudio(input_hp_out, input_diffusion_dz, delay_samples_allpass1);
    allpass2_out = allpass2.processaudio(allpass1_out, input_diffusion_dz, delay_samples_allpass2);
    allpass3_out = allpass3.processaudio(allpass2_out, input_diffusion2_dz, delay_samples_allpass3);
    allpass4_out = allpass4.processaudio(allpass3_out, input_diffusion2_dz, delay_samples_allpass4);
    
    // TANK
    
    // tank input accumulator summed with input diffusers' output
    tank_input1 = allpass4_out + tank_accumulator2;
    tank_input2 = allpass4_out + tank_accumulator1;
    
    // Modulated tank all-passes
    // smooth controls whether there's amplitude modulation and the number of all pass filters in the tank
    if (smooth == 0) // smooth is on
    {
        // max delay samples modulation
        mod_depth1 = 24;
        mod_depth2 = 48;
    }
    else if (smooth == 1) // smooth is off
    {
        mod_depth1 = mod_depth2 = 0;
    }
    mod_allpass1_out = mod_allpass1.processaudio_mod(tank_input1, tank_diffusion1, delay_samples_mod_allpass1, mod_depth1);
    mod_allpass2_out = mod_allpass2.processaudio_mod(tank_input2, tank_diffusion1, delay_samples_mod_allpass2, mod_depth2);
    
    // Delay lines (1 and 3)
    delay1_tank_out = delay1_tank.processaudio(mod_allpass1_out, delay_samples_delay1_tank);
    delay3_tank_out = delay3_tank.processaudio(mod_allpass2_out, delay_samples_delay3_tank);
    
    // Saturation
    // 2 different saturation curves, one for each leg of the tank
    saturator1_out = saturator1.processaudio_atan(delay1_tank_out, drive_dz);
    saturator2_out = saturator1.processaudio_tanh(delay3_tank_out, drive_dz);
    
    // Tank Lowpass Filtering (Damping)
    tank_lp1_out = tank_lp1.LP(saturator1_out, tank_lp_norm_wc_dz);
    tank_lp2_out = tank_lp2.LP(saturator2_out, tank_lp_norm_wc_dz);
    
    // Tank HighPass
    tank_hp1_out = tank_hp1.HP(tank_lp1_out, tank_hp_norm_wc_dz);
    tank_hp2_out = tank_hp2.HP(tank_lp2_out, tank_hp_norm_wc_dz);
    
    // Tank AllPass filters
    allpass5_out = allpass5.processaudio(tank_hp1_out, tank_diffusion2_dz, delay_samples_allpass5);
    allpass6_out = allpass6.processaudio(tank_hp2_out, tank_diffusion2_dz, delay_samples_allpass6);
    
    // Add decay control between the allpass filters and the last delay lines
    allpass5_out = allpass5_out*decay_dz;
    allpass6_out = allpass6_out*decay_dz;
    
    // Dealy lines (2 and 4)
    delay2_tank_out = delay2_tank.processaudio(allpass5_out, delay_samples_delay2_tank);
    delay4_tank_out = delay4_tank.processaudio(allpass6_out, delay_samples_delay4_tank);
    
    // if smooth == on allpass 7 - 10 are included
    if (smooth == 0)
    {
        // Added Allpasses 7 - 10
        allpass7_out = allpass7.processaudio(delay2_tank_out, tank_diffusion2_dz, delay_samples_allpass7);
        allpass8_out = allpass8.processaudio(delay4_tank_out, tank_diffusion2_dz, delay_samples_allpass8);
        allpass9_out = allpass9.processaudio(allpass7_out, tank_diffusion2_dz, delay_samples_allpass9);
        allpass10_out = allpass10.processaudio(allpass8_out, tank_diffusion2_dz, delay_samples_allpass10);
        
        // Compute the accumulators as the outputs from the last tank nodes scaled by decay control
        tank_accumulator1 = decay_dz*(allpass9_out);
        tank_accumulator2 = decay_dz*(allpass10_out);
        
        // Simplified wet output computation compared to dattorro's
        
        outL_wet = 0.6*(delay3_tank_out - allpass5_out + delay2_tank_out - allpass8_out + allpass10_out);
        outR_wet = 0.6*(delay1_tank_out - allpass6_out + delay4_tank_out - allpass7_out + allpass9_out);
        
    }
    
    // if smooth == off then allpasses 7 - 10 are bypassed
    if (smooth == 1)
    {
        // Compute the accumulators as the outputs from the last tank nodes scaled by decay control
        tank_accumulator1 = decay_dz*(delay2_tank_out);
        tank_accumulator2 = decay_dz*(delay4_tank_out);
        
        // Simplified wet output computation compared to dattorro's
        outL_wet = 0.7*(delay3_tank_out - allpass5_out + delay2_tank_out);
        outR_wet = 0.7*(delay1_tank_out - allpass6_out + delay4_tank_out);
    }
    
    
    // Dry/Wet
    outL = inL*(1 - mix_dz) + outL_wet*mix_dz;
    outR = inR*(1 - mix_dz) + outR_wet*mix_dz;
    
    processFrameInfo.audioOutputFrame[0] = outL;
    processFrameInfo.audioOutputFrame[1] = outR;
    
    return true; /// processed
    // return false; /// NOT processed
}


/**
\brief do anything needed prior to arrival of audio buffers

Operation:
- updateOutBoundVariables sends metering data to the GUI meters

\param processInfo structure of information about *buffer* processing

\return true if operation succeeds, false otherwise
*/
bool PluginCore::postProcessAudioBuffers(ProcessBufferInfo& processInfo)
{
    // --- update outbound variables; currently this is meter data only, but could be extended
    //     in the future
    updateOutBoundVariables();

    return true;
}

/**
\brief update the PluginParameter's value based on GUI control, preset, or data smoothing (thread-safe)

Operation:
- update the parameter's value (with smoothing this initiates another smoothing process)
- call postUpdatePluginParameter to do any further processing

\param controlID the control ID value of the parameter being updated
\param controlValue the new control value
\param paramInfo structure of information about why this value is being udpated (e.g as a result of a preset being loaded vs. the top of a buffer process cycle)

\return true if operation succeeds, false otherwise
*/
bool PluginCore::updatePluginParameter(int32_t controlID, double controlValue, ParameterUpdateInfo& paramInfo)
{
    // --- use base class helper
    setPIParamValue(controlID, controlValue);

    // --- do any post-processing
    postUpdatePluginParameter(controlID, controlValue, paramInfo);

    return true; /// handled
}

/**
\brief update the PluginParameter's value based on *normlaized* GUI control, preset, or data smoothing (thread-safe)

Operation:
- update the parameter's value (with smoothing this initiates another smoothing process)
- call postUpdatePluginParameter to do any further processing

\param controlID the control ID value of the parameter being updated
\param normalizedValue the new control value in normalized form
\param paramInfo structure of information about why this value is being udpated (e.g as a result of a preset being loaded vs. the top of a buffer process cycle)

\return true if operation succeeds, false otherwise
*/
bool PluginCore::updatePluginParameterNormalized(int32_t controlID, double normalizedValue, ParameterUpdateInfo& paramInfo)
{
    // --- use base class helper, returns actual value
    double controlValue = setPIParamValueNormalized(controlID, normalizedValue, paramInfo.applyTaper);

    // --- do any post-processing
    postUpdatePluginParameter(controlID, controlValue, paramInfo);

    return true; /// handled
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
    // --- now do any post update cooking; be careful with VST Sample Accurate automation
    //     If enabled, then make sure the cooking functions are short and efficient otherwise disable it
    //     for the Parameter involved
    Fs = getSampleRate();
    switch(controlID)
    {
        case controlID::predelay_time:
        {
            predelay_samples = (int) round((predelay_time*Fs)/1000.0); // force conversion to int. the actual delay value for the rptr is obatined from the predelay_time variable.
            return true;    /// handled
        }
        case controlID::input_lp_fc:
        {
            input_lp_norm_wc = 2*M_PI*input_lp_fc/Fs;
            return true;    /// handled
        }
        case controlID::input_hp_fc:
        {
            input_hp_norm_wc = 2*M_PI*input_hp_fc/Fs;
            return true;    /// handled
        }
        case controlID::input_diffusion:
        {
            // input diffusion 2 gets varied along with diffusion 1
            // might change to /6?
            input_diffusion2 = 0.625 + (input_diffusion - 0.5)/6.0;
            return true;    /// handled
        }
        case controlID::decay:
        {
            // decay also affects the tank_diffusion2 (values tanken from dattorro's )
            tank_diffusion2 = decay + 0.15;
            if (tank_diffusion2 < 0.15) tank_diffusion2 = 0.15;
            if (tank_diffusion2 > 0.50) tank_diffusion2 = 0.50;
            return true;    /// handled
        }
        case controlID::drive:
        {
            return true;    /// handled
        }
        case controlID::hf_damping:
        {
            hf_abs_damping = fabs(hf_damping);
            // hard clip the value to avoid it going to +inf.
            if (hf_abs_damping > 21999)
            {
                hf_abs_damping = 21999;
            }
            if (hf_abs_damping < 400)
            {
                hf_abs_damping = 400;
            }
            // Updating
            tank_lp_norm_wc = 2*M_PI*hf_abs_damping/Fs;
            return true;    /// handled
        }
        case controlID::lf_damping:
        {
            // Updating
            tank_hp_norm_wc = 2*M_PI*lf_damping/Fs;
            return true;    /// handled
        }
        case controlID::mix_perc:
        {
            mix = mix_perc/100.0;
            return true;    /// handled
        }
        /*case controlID:1
        {
            return true;    /// handled
        }*/
        default:
            return false;   /// not handled
    }

    return false;
}

/**
\brief has nothing to do with actual variable or updated variable (binding)

CAUTION:
- DO NOT update underlying variables here - this is only for sending GUI updates or letting you
  know that a parameter was changed; it should not change the state of your plugin.

WARNING:
- THIS IS NOT THE PREFERRED WAY TO LINK OR COMBINE CONTROLS TOGETHER. THE PROPER METHOD IS
  TO USE A CUSTOM SUB-CONTROLLER THAT IS PART OF THE GUI OBJECT AND CODE.
  SEE http://www.willpirkle.com for more information

\param controlID the control ID value of the parameter being updated
\param actualValue the new control value

\return true if operation succeeds, false otherwise
*/
bool PluginCore::guiParameterChanged(int32_t controlID, double actualValue)
{
    /*
    switch (controlID)
    {
        case controlID::<your control here>
        {

            return true; // handled
        }

        default:
            break;
    }*/

    return false; /// not handled
}

/**
\brief For Custom View and Custom Sub-Controller Operations

NOTES:
- this is for advanced users only to implement custom view and custom sub-controllers
- see the SDK for examples of use

\param messageInfo a structure containing information about the incoming message

\return true if operation succeeds, false otherwise
*/
bool PluginCore::processMessage(MessageInfo& messageInfo)
{
    // --- decode message
    switch (messageInfo.message)
    {
        // --- add customization appearance here
    case PLUGINGUI_DIDOPEN:
    {
        return false;
    }

    // --- NULL pointers so that we don't accidentally use them
    case PLUGINGUI_WILLCLOSE:
    {
        return false;
    }

    // --- update view; this will only be called if the GUI is actually open
    case PLUGINGUI_TIMERPING:
    {
        return false;
    }

    // --- register the custom view, grab the ICustomView interface
    case PLUGINGUI_REGISTER_CUSTOMVIEW:
    {

        return false;
    }

    case PLUGINGUI_REGISTER_SUBCONTROLLER:
    case PLUGINGUI_QUERY_HASUSERCUSTOM:
    case PLUGINGUI_USER_CUSTOMOPEN:
    case PLUGINGUI_USER_CUSTOMCLOSE:
    case PLUGINGUI_EXTERNAL_SET_NORMVALUE:
    case PLUGINGUI_EXTERNAL_SET_ACTUALVALUE:
    {

        return false;
    }

    default:
        break;
    }

    return false; /// not handled
}


/**
\brief process a MIDI event

NOTES:
- MIDI events are 100% sample accurate; this function will be called repeatedly for every MIDI message
- see the SDK for examples of use

\param event a structure containing the MIDI event data

\return true if operation succeeds, false otherwise
*/
bool PluginCore::processMIDIEvent(midiEvent& event)
{
    return true;
}

/**
\brief (for future use)

NOTES:
- MIDI events are 100% sample accurate; this function will be called repeatedly for every MIDI message
- see the SDK for examples of use

\param vectorJoysickData a structure containing joystick data

\return true if operation succeeds, false otherwise
*/
bool PluginCore::setVectorJoystickParameters(const VectorJoystickData& vectorJoysickData)
{
    return true;
}

/**
\brief use this method to add new presets to the list

NOTES:
- see the SDK for examples of use
- for non RackAFX users that have large paramter counts, there is a secret GUI control you
  can enable to write C++ code into text files, one per preset. See the SDK or http://www.willpirkle.com for details

\return true if operation succeeds, false otherwise
*/
bool PluginCore::initPluginPresets()
{
    // **--0xFF7A--**

    // **--0xA7FF--**

    return true;
}

/**
\brief setup the plugin description strings, flags and codes; this is ordinarily done through the ASPiKreator or CMake

\return true if operation succeeds, false otherwise
*/
bool PluginCore::initPluginDescriptors()
{
    pluginDescriptor.pluginName = PluginCore::getPluginName();
    pluginDescriptor.shortPluginName = PluginCore::getShortPluginName();
    pluginDescriptor.vendorName = PluginCore::getVendorName();
    pluginDescriptor.pluginTypeCode = PluginCore::getPluginType();

    // --- describe the plugin attributes; set according to your needs
    pluginDescriptor.hasSidechain = kWantSidechain;
    pluginDescriptor.latencyInSamples = kLatencyInSamples;
    pluginDescriptor.tailTimeInMSec = kTailTimeMsec;
    pluginDescriptor.infiniteTailVST3 = kVSTInfiniteTail;

    // --- AAX
    apiSpecificInfo.aaxManufacturerID = kManufacturerID;
    apiSpecificInfo.aaxProductID = kAAXProductID;
    apiSpecificInfo.aaxBundleID = kAAXBundleID;  /* MacOS only: this MUST match the bundle identifier in your info.plist file */
    apiSpecificInfo.aaxEffectID = "aaxDeveloper.";
    apiSpecificInfo.aaxEffectID.append(PluginCore::getPluginName());
    apiSpecificInfo.aaxPluginCategoryCode = kAAXCategory;

    // --- AU
    apiSpecificInfo.auBundleID = kAUBundleID;   /* MacOS only: this MUST match the bundle identifier in your info.plist file */
    apiSpecificInfo.auBundleName = kAUBundleName;

    // --- VST3
    apiSpecificInfo.vst3FUID = PluginCore::getVSTFUID(); // OLE string format
    apiSpecificInfo.vst3BundleID = kVST3BundleID;/* MacOS only: this MUST match the bundle identifier in your info.plist file */
    apiSpecificInfo.enableVST3SampleAccurateAutomation = kVSTSAA;
    apiSpecificInfo.vst3SampleAccurateGranularity = kVST3SAAGranularity;

    // --- AU and AAX
    apiSpecificInfo.fourCharCode = PluginCore::getFourCharCode();

    return true;
}

// --- static functions required for VST3/AU only --------------------------------------------- //
const char* PluginCore::getPluginBundleName() { return kAUBundleName; }
const char* PluginCore::getPluginName(){ return kPluginName; }
const char* PluginCore::getShortPluginName(){ return kShortPluginName; }
const char* PluginCore::getVendorName(){ return kVendorName; }
const char* PluginCore::getVendorURL(){ return kVendorURL; }
const char* PluginCore::getVendorEmail(){ return kVendorEmail; }
const char* PluginCore::getAUCocoaViewFactoryName(){ return AU_COCOA_VIEWFACTORY_STRING; }
pluginType PluginCore::getPluginType(){ return kPluginType; }
const char* PluginCore::getVSTFUID(){ return kVSTFUID; }
int32_t PluginCore::getFourCharCode(){ return kFourCharCode; }

