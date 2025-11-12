#include "daisy_patch_sm.h"
#include "daisysp.h"
#include "../_projLib/ReverbZ.hpp"

/** These are namespaces for the daisy libraries.
 *  These lines allow us to omit the "daisy::" and "daisysp::" before
 * referencing modules, and functions within the daisy libraries.
 */
using namespace daisy;
using namespace patch_sm;
using namespace daisysp;
using namespace projLib;

#ifndef FS_REVERBZ
#define FS_REVERBZ 48000
#endif

/** Our hardware board class handles the interface to the actual DaisyPatchSM
 * hardware. */
DaisyPatchSM patch;
/* HW Objects */
Switch button;

/** ReverbZ reverb processor instance */
ReverbZ reverbz(FS_REVERBZ);  // Initialize with fixed sample rate of 48kHz

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
 *  - Pot3 (CV_3) HF Damping Fc
 *  - Pot4 (CV_4) LF Damping Fc 
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
    /** The easiest way to do pass thru is to simply copy the input to the output
     * In C++ the standard way of doing this is with std::copy. However, those
     * familliar with C can use memcpy. A simple loop is also a good way to do
     * this.
     *
     * Since you'll most likely want to be doing something between the input,
     *  and the output, and not just passing it through we'll demonstrate doing
     *  so with a for loop.
     */
    for(size_t i = 0; i < size; i++)
    {
        /* TODO: Retrieve, compute and set ReverbZ parameters */

        /* Process Audio */
        reverbz.processAudioStereo(in[0][i], in[1][i]);  // Process stereo input
        out[0][i] = reverbz.mOutL;          // Left output
        out[1][i] = reverbz.mOutR;          // Right output
    }
}

int main(void)
{
    /** Initialize main variables */
    bool ledState = false;
    bool buttonState = false;
    int ledWaitTime = 500;      // in ms
    int mainLoopWaitTime = 1;   // in ms
    int mainLoopCounter = 0;

    /* Control pages */
    int paramsPage = 0;
    int nParamsPages = 3;

    /** Initialize the hardware */
    const float maxCvOut = 5.0f;
    patch.Init();
    // TODO: init all hw
    /* Initialize the switch on pin B7 */
    button.Init(patch.B7);


    /* Set Audio parameters */
    patch.SetAudioSampleRate(FS_REVERBZ); // Set sample rate to 48kHz
    patch.SetAudioBlockSize(4);           // Set block size to 4 samples

    /** Start Processing the audio */
    patch.StartAudio(AudioCallback);

    /** Main loop */
    while(1) 
    {
        /** Set Parameters page based on momentary switch */
        /* Debounce the switch */
        button.Debounce();
        buttonState = button.FallingEdge();
        if(buttonState && !button.Pressed())
        {
            paramsPage = (paramsPage + 1) % nParamsPages; // Cycle through 3 pages
        }

        // TODO: Read potentiometers and map to control parameters based on paramsPage

        /* ------ LED Blinking based on params page ------ */
        if(paramsPage == 0)
        {
            ledWaitTime = 500; // Slow blink
        }
        else if(paramsPage == 1)
        {
            ledWaitTime = 300; // Mid blink
        }
        else if(paramsPage == 2)
        {
            ledWaitTime = 100; // Fast blink
        }

        // Toggle the LED state for the next time around.
        if (mainLoopCounter >= ledWaitTime)
        {
            // If the wait time has elapsed, toggle the LED and reset the counter
            mainLoopCounter = 0;
            ledState = !ledState;

            // // Set the onboard LED
            // patch.SetLed(ledState);

        }

        // Set the CV_OUT_2 to set the front panel the LED state (0V for off, 5V for on)
        patch.WriteCvOut(CV_OUT_2, ledState ? maxCvOut : 0.0f);

        // Wait
        System::Delay(mainLoopWaitTime);
        mainLoopCounter++;
    }
}
