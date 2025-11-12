//
//  AllPass.cpp
//
//  Created by Matteo Desantis on 2/01/2021.
//

#include "AllPass.hpp"
#include <cmath>


// Constructor
AllPass::AllPass()
{
    // Init
    m_maxdelay = 10000;     // max delay = 1 sec if Fs = 48000
    if(dline)
        {
            delete [] dline;                    // contructor checks whether dline exists
            dline = nullptr;                    // if it does, dline is deleted
        }
    dline = new double[m_maxdelay];             // dynamically allocate 48000 samples
    memset(dline,0,m_maxdelay*sizeof(double));  // initialise array to 0
    m_delay = 0;            // delay time is initialised to 0, changed with filter method
    m_alpha = 0.5;          // initialised at 0.5, can be changed using the filter method
    reset();
}

AllPass::~AllPass()
{
    if(dline)
        {
            delete [] dline;            // deconstructor checks whether dline exists.
            dline = nullptr;            // if it does, it is deleted
         }
    if(sine_array)                      // chechks whether sine_array exists
    {
        delete [] sine_array;
        sine_array = nullptr;
    }
}
//--------------------------------------------------

void AllPass::reset()       // reset should be called when stop/play button is pressed
{
    rptr = wptr = 0;
    memset(dline, 0, m_maxdelay*sizeof(double));
}

double AllPass::processaudio(double in, double fb_coeff, int delay_time)
{
    // Processing
    m_alpha = fb_coeff;         // m_alpha is passed when the method is called
    // assign delay time
    m_delay = delay_time;
    // Calculate read pointer position
    rptr = wptr - m_delay;
    if(rptr < 0)
    {
        rptr += m_maxdelay;
    }
    // Actual Processing
    double delay_out = dline[rptr];             // output given by the memory line
    double delay_in = in - delay_out*m_alpha;   // delay line input
    double temp = delay_in*m_alpha + delay_out; // allpass output
    dline[wptr] = delay_in;                     // actually writes in the delay array
    
    // wptr update
    wptr++;
    if (wptr >= m_maxdelay)
    {
        wptr = 0;                   // wrap back of the write pointer -> circular                                 buffer
    }
    
    double abstemp = fabs(temp);    //solve denormalisation problem
    if (abstemp < 3.e-34)
    {
        temp = 0.0;
    }
    
    return temp;
}
void AllPass::set_sine_array(double Fs, double lfo_frequency)
{
    // set array length
    N = (int) round(Fs/lfo_frequency);
    // contrsuctor checks whether the array exists and initialises it
    if(sine_array)
        {
            delete [] sine_array;
            sine_array = nullptr;
        }
    sine_array = new double[Fs/lfo_frequency];             // dynamically allocate 48000 samples
    memset(dline,0,m_maxdelay*sizeof(double));  // initialise array to 0
    // calculate sin
    for (int i = 0; i < N; i++ )
    {
        sine_array[i] = sin(2*M_PI*i/N);
    }
    j = 0;                      // set array counter for processaudio_mod
}

double AllPass::processaudio_mod(double in, double fb_coeff, int delay_time, int amplitude)
{
    // Processing
    m_alpha = fb_coeff;         // m_alpha is passed when the method is called
    // assign delay time
    m_delay = delay_time;
    // Calculate read pointer position
    rptr = wptr - m_delay;
    // delay line modulation
    if (j < N)
    {
        rptr = (int)round(amplitude*sine_array[j] + rptr);
        j++;
        if(j >= N) j = 0;
    }
    if(rptr < 0)
    {
        rptr += m_maxdelay;
    }
    // Actual Processing
    double delay_out = dline[rptr];             // output given by the memory line
    double delay_in = in - delay_out*m_alpha;   // delay line input
    double temp = delay_in*m_alpha + delay_out; // allpass output
    dline[wptr] = delay_in;                     // actually writes in the delay array
    
    // wptr update
    wptr++;
    if (wptr >= m_maxdelay)
    {
        wptr = 0;                   // wrap back of the write pointer -> circular                                 buffer
    }
    
    double abstemp = fabs(temp);    //solve denormalisation problem
    if (abstemp < 3.e-34)
    {
        temp = 0.0;
    }
    
    return temp;
}
