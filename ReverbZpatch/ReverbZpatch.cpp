#include "daisy_patch_sm.h"
#include "daisysp.h"
//#include "../../libDaisy/src/dev/sdram.h"
#include "dspConfig.hpp"
#include "../_projLib/ReverbZ.hpp"
#include "../_helperUtils/ctrlUtils.hpp"
#include "../../dspLib/Utils/sdramArena.h"
#include <cstdio>

// Reserve SDRAM space for arena allocator
// This forces the linker to allocate space in .sdram_bss section
#define SDRAM_BUFFER_SIZE (16 * 1024 * 1024)  // 16MB reserve
volatile uint8_t DSY_SDRAM_BSS sdramPool[SDRAM_BUFFER_SIZE]; // volatile prevents optimization; __attribute__((used)) prevents GC

using namespace daisy;
using namespace patch_sm;
//using namespace daisysp;
using namespace projLib;

using ReverbZ_t = projLib::ReverbZ<DSPLIB_MAX_BUFFER_SIZE>;

/** Our hardware board class handles the interface to the actual DaisyPatchSM
 * hardware. */
DaisyPatchSM patch;
/* HW Objects */
Switch button;
Switch toggle;

/* Environment Constants */
const int FS_REVERBZ = 48000;  // ReverbZ fixed sample rate

/** ReverbZ reverb processor instance */
ReverbZ_t reverbz(FS_REVERBZ); // Object allocated on stack, buffers in SDRAM via init().

/* Control Parameters for ReverbZ */
double predelayTimeCtrl = 00.0;          // in ms
double inputLowpassFcCtrl = 22000.0;     // in Hz
double inputHighpassFcCtrl = 10.0;       // in Hz
double inputDiffusionCtrl = 0.75;        // normalized [0.0 - 1.0]
double decayCtrl = 0.5;                  // normalized [0.0 - 1.0]
double driveCtrl = 0.1;                  // normalized [0.0 - 1.0]
double hfDampingFcCtrl = 5000.0;         // in Hz
double lfDampingFcCtrl = 0.0;            // in Hz
double mixPercentageCtrl = 100.0;        // in percentage [0.0 - 100.0]
int    smoothCtrl = 0;                   // smoothing + modulation on/off

/* ------------------------------------------------------------------------------------------ */
/** DEFINE UI PARAMETERS MAPPING [TODO:]
 * 
 * Page 0: Most crucial parameters
 *  - Pot1 (CV_1) Decay
 *  - Pot2 (CV_2) Drive
 *  - Pot3 (CV_3) Diffusion (input diffusion maybe add some global allpass-delay time scaling?)
 *  - Pot4 (CV_4) Mix Percentage
 * 
 * Page 1: Filtering
 *  - Pot1 (CV_1) Input Lowpass Fc
 *  - Pot2 (CV_2) Input Highpass Fc
 *  - Pot3 (CV_3) Tank HF Damping Fc
 *  - Pot4 (CV_4) Tank LF Damping Fc 
 * 
 * Page 2: Leftover / Misc
 *  - Pot1 (CV_1) Predelay Time
 * 
 * 
 * Smooth Ctrl: Toggle switch (B8) ON/OFF, page-independent
 */


/** Callback for processing and synthesizing audio
 *
 *  The audio buffers are arranged as arrays of samples for each channel.
 *  For example, to access the left input you would use:
 *    in[0][n]
 *  where n is the specific sample.
 *  There are "size" samples in each array.
 *
 *  The default size is very small (just 4 samples per channel). This means the
 * callback is being called at 16kHz.
 *
 *  This size is acceptable for many applications, and provides an extremely low
 * latency from input to output. However, you can change this size by calling
 * patch.SetAudioBlockSize(desired_size). When running complex DSP it can be more
 * efficient to do the processing on larger chunks at a time.
 *
 */
void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    for(size_t i = 0; i < size; i++)
    {
        /* Process Audio */
        reverbz.processAudioStereo(in[0][i], in[1][i]);  // Process stereo input
        out[0][i] = reverbz.mOutL;          // Left output
        out[1][i] = reverbz.mOutR;          // Right output
    }
}

int main(void)
{
    /* -------------- Hardware + board configuration variables -------------- */
    // Parameters for LED blinking pattern
    const float maxCvOut = 5.0f;
    bool ledState = false;
    bool buttonState = false;
    bool isShortBlinking = true;
    int ledWaitTimeShort = 100;      // in ms
    int ledWaitTimeLong = 500;       // in ms
    int nBlinksMax = 1;              // Max short blinks per params page
    int nBlinks = 0;                 // Counter of short blinks
    // HW controls
    float pot1Value = 0.0f;
    float pot2Value = 0.0f;
    float pot3Value = 0.0f;
    float pot4Value = 0.0f;
    bool toggleSwitchState = false;
    float cvIn1Value = 0.0f;
    float cvIn2Value = 0.0f;
    float cvIn3Value = 0.0f;
    float cvIn4Value = 0.0f;

    /* Main loop timing */
    int mainLoopWaitTime = 1;   // in ms
    int mainLoopCounter = 0;

    /* Control pages */
    int paramsPage = 0;
    int nParamsPages = 3;

    /* -------------------------- ReverbZ Controls -------------------------- */
    float predelayTimeCtrlNorm = 0.0f;      // normalized [0.0 - 1.0]
    float predelayTimeCtrl = 0.0f;          // in ms

    float inputLowpassFcCtrlNorm = 1.0f;    // normalized [0.0 - 1.0]   
    float inputLowpassFcCtrl = 22000.0f;    // in Hz

    float inputHighpassFcCtrlNorm = 0.0f;   // normalized [0.0 - 1.0]
    float inputHighpassFcCtrl = 10.0f;      // in Hz

    float inputDiffusionCtrl = 0.75f;       // normalized [0.0 - 1.0]
    float inputDiffusionCtrlStored = 0.75f; // stored value for CV control while on pages 1-2 and soft-takeover

    float decayCtrl = 0.5f;                 // normalized [0.0 - 1.0]
    float decayCtrlStored = 0.5f;           // stored value for CV control while on pages 1-2 and soft-takeover

    float driveCtrlNorm = 0.0f;             // normalized [0.0 - 1.0]
    float driveCtrl = 0.1f;                 // dB 
    float driveCtrlStored = 0.1f;           // dB - stored value for CV control while on pages 1-2 and soft-takeover

    float hfDampingFcCtrlNorm = 0.39f;      // normalized [0.0 - 1.0] (0.39f corresponds to 5kHz)
    float hfDampingFcCtrl = 5000.0f;        // in Hz-

    float lfDampingFcCtrlNorm = 0.0f;       // normalized [0.0 - 1.0]
    float lfDampingFcCtrl = 0.0f;           // in Hz

    float mixPercentageCtrlNorm = 0.0f;     // normalized [0.0 - 1.0]
    float mixPercentageCtrl = 100.0f;       // in percentage [0.0 - 100.0]
    float mixPercentageCtrlStored = 100.0f; // stored value for CV control while on pages 1-2 and soft-takeover

    int   smoothCtrl = 0;                   // smoothing + modulation on/off

    /* ----------------------- Hardware Initialization ---------------------- */
    /* Initialize the Daisy Patch hardware */
    patch.Init();
    button.Init(patch.B7);          // Setup momentary switch on pin B7
    toggle.Init(patch.B8);          // Setup toggle switch on pin B8
    // Don't need to initialize pots. 

    // Touch sdramPool to prevent linker garbage collection
    sdramPool[0] = 0;

    // Wait a bit for everything to settle
    System::Delay(100);

    /** Initialize SDRAM arena allocator (must be after patch.Init()) */
    sdramArenaInit();

    // Wait a bit for everything to settle
    System::Delay(100);

    /** Init ReverbZ buffers in SDRAM (must be after sdramArenaInit()) */
    reverbz.init(); // Allocate buffers on SDRAM

    /* Set Audio parameters */
    patch.SetAudioSampleRate(FS_REVERBZ); // Set sample rate to 48kHz
    patch.SetAudioBlockSize(4);           // Set block size to 4 samples

    /** Start Processing the audio */
    patch.StartAudio(AudioCallback);

    /* ------------------------------ Main Loop ----------------------------- */
    while(1) 
    {
        /** Set parameters page based on momentary switch */
        /* Debounce the momentary button */
        button.Debounce();
        buttonState = button.FallingEdge();
        if(buttonState && !button.Pressed())
        {
            paramsPage = (paramsPage + 1) % nParamsPages; // Cycle through 3 pages

            // Reset LED blinking pattern
            nBlinks = 0; // Reset blink counter
            isShortBlinking = true;
            ledState = false;

            // Reset main loop counter
            mainLoopCounter = 0;

            // TODO: Get pot values for soft-takeover on page switch

            // TODO: long press to switch between VU-mode led or blinking-mode led
        }

        /* ----------------- Parameters Cooking and Mapping ----------------- */
        /* Process All controls - Hardware layer */
        patch.ProcessAllControls();
        toggle.Debounce();
        toggleSwitchState = toggle.Pressed() ? 1 : 0;
        pot1Value = patch.GetAdcValue(CV_1);
        pot2Value = patch.GetAdcValue(CV_2);
        pot3Value = patch.GetAdcValue(CV_3);
        pot4Value = patch.GetAdcValue(CV_4);
        cvIn1Value = patch.GetAdcValue(CV_5);
        cvIn2Value = patch.GetAdcValue(CV_6);
        cvIn3Value = patch.GetAdcValue(CV_7);
        cvIn4Value = patch.GetAdcValue(CV_8);  


        // ---- For debugging -----
        decayCtrl = pot1Value;
        driveCtrlNorm = pot2Value;
        inputDiffusionCtrl = pot3Value;
        mixPercentageCtrlNorm = pot4Value;
        // ------------------------

        if (paramsPage == 0) 
        {       
            // // Page 0: Most crucial parameters - Controls with pot+cv 
            // decayCtrl = softTakeoverWithCv(pot1Value, cvIn1Value, decayCtrlStored);
            // decayCtrlStored = decayCtrl;  // Store for pages 1-2

            // driveCtrlNorm = softTakeoverWithCv(pot2Value, cvIn2Value, driveCtrlStored);
            // driveCtrlStored = driveCtrlNorm;  // Store for pages 1-2

            // inputDiffusionCtrl = softTakeoverWithCv(pot3Value, cvIn3Value, inputDiffusionCtrlStored);
            // inputDiffusionCtrlStored = inputDiffusionCtrl;  // Store for pages 1-2
            
            // mixPercentageCtrlNorm = softTakeoverWithCv(pot4Value, cvIn4Value, mixPercentageCtrlStored);
            // mixPercentageCtrlStored = mixPercentageCtrlNorm;  // Store for pages 1-2

            // // Page 1-2: parameters retain their previous values
        }
        else if (paramsPage == 1) 
        {
            // // Page 1: Filtering - Controls with pot
            // inputLowpassFcCtrl = softTakeover(pot1Value, inputLowpassFcCtrl);
            // inputHighpassFcCtrl = softTakeover(pot2Value, inputHighpassFcCtrl);
            // hfDampingFcCtrlNorm = softTakeover(pot3Value, hfDampingFcCtrlNorm);
            // lfDampingFcCtrlNorm = softTakeover(pot4Value, lfDampingFcCtrlNorm);

            // // Page 0: CV sets page 0 parameters
            // decayCtrl = limitPotAndCv(decayCtrlStored, cvIn1Value);
            // driveCtrlNorm = limitPotAndCv(driveCtrlStored, cvIn2Value);
            // inputDiffusionCtrl = limitPotAndCv(inputDiffusionCtrlStored, cvIn3Value);
            // mixPercentageCtrlNorm = limitPotAndCv(mixPercentageCtrlStored, cvIn4Value);

            // // Page 2: parameters retain their previous values
        }
        else if (paramsPage == 2) 
        {
            // // Page 2: Leftover / Misc - Controls with pot
            // predelayTimeCtrlNorm = softTakeover(pot1Value, predelayTimeCtrlNorm);

            // // Page 0: CV sets page 0 parameters
            // decayCtrl = limitPotAndCv(decayCtrlStored, cvIn1Value);
            // driveCtrlNorm = limitPotAndCv(driveCtrlStored, cvIn2Value);
            // inputDiffusionCtrl = limitPotAndCv(inputDiffusionCtrlStored, cvIn3Value);
            // mixPercentageCtrlNorm = limitPotAndCv(mixPercentageCtrlStored, cvIn4Value);

            // // Page 1: parameters retain their previous values
        }

        /* Map normalized control values to actual parameter ranges */
        predelayTimeCtrl = mapAntiLog(predelayTimeCtrlNorm, 0.0, 100.0);                // 0.0ms - 100.0ms.
        inputLowpassFcCtrl = mapAntiLog(inputLowpassFcCtrlNorm, 10.0, 22000.0);         // 10.0Hz - 22.0kHz
        inputHighpassFcCtrl = mapAntiLog(inputHighpassFcCtrlNorm, 10.0, 22000.0);       // 10.0Hz - 22.0kHz
        driveCtrl = mapLinear(driveCtrlNorm, 0.0, 20.0);                                // 0.0 - 20.0dB
        hfDampingFcCtrl = mapLog(hfDampingFcCtrlNorm, 20000.0, 400.0);                  // 400Hz - 20.0kHz, inverse mapping
        lfDampingFcCtrl = mapAntiLog(lfDampingFcCtrlNorm, 10.0, 3000.0);                // 10.0Hz - 3.0kHz
        mixPercentageCtrl = mapLinear(mixPercentageCtrlNorm, 0.0, 100.0);               // 0.0% - 100.0%
        
        // Set smoothing control
        smoothCtrl = toggleSwitchState;

        /* ------ Update parameters in ReverbZ object ------ */
        reverbz.setControlParameters(predelayTimeCtrl,
                                    inputLowpassFcCtrl,
                                    inputHighpassFcCtrl,
                                    inputDiffusionCtrl,
                                    decayCtrl,
                                    driveCtrl,
                                    hfDampingFcCtrl,
                                    lfDampingFcCtrl,
                                    mixPercentageCtrl,
                                    smoothCtrl);

        /* ------ LED Blinking based on params page ------ */
        if(paramsPage == 0)      {nBlinksMax = 1;}
        else if(paramsPage == 1) {nBlinksMax = 2;}
        else if(paramsPage == 2) {nBlinksMax = 3;}

        // Short and long blinking pattern
        if (isShortBlinking) {
            // Short blink
            if (nBlinks < nBlinksMax*2) {
                if (mainLoopCounter == 0) {
                    // Set the CV_OUT_2 to set the front panel the LED state (0V for off, 5V for on)
                    patch.WriteCvOut(CV_OUT_2, ledState ? 0.0f : maxCvOut);
                }
                else if (mainLoopCounter >= ledWaitTimeShort) {
                    // If the wait time has elapsed, toggle the LED and reset the counters
                    mainLoopCounter = -1; // will be incremented to 0 at end of loop
                    nBlinks++;
                    ledState = !ledState;
                }
            }
            else {
                // Switch to long blink after max short blinks
                isShortBlinking = false;
                nBlinks = 0; 
            }
        }
        else {
            // Final wait time (LED off)
            if (mainLoopCounter == 0) {
                // Set the CV_OUT_2 to set the front panel the LED state (0.0V for off)
                patch.WriteCvOut(CV_OUT_2, maxCvOut);
            }
            else if (mainLoopCounter >= ledWaitTimeLong) {
                // If the wait time has elapsed, toggle the LED and reset the counter
                mainLoopCounter = -1;   // 

                // Reset for next time
                isShortBlinking = true;
                ledState = false;
            }
        }

        // Wait
        System::Delay(mainLoopWaitTime);
        mainLoopCounter++;
    }
}