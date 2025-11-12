//
//  Delay.cpp
//
//
//  Created by Matteo Deantis on 17/12/2020
//

#include "DelayClass.hpp"


// Constructor
DelayClass::DelayClass()
{
    // This is needed for initialisation
    m_maxdelay = 48000;             // 1 sec max delay having Fs = 48000. might be                                           changed to Fs dependent.
    if(dline)
        {
            delete [] dline;                    // contructor checks whether dline exists
            dline = nullptr;                    // if it does, dline is deleted
        }
    dline = new double[m_maxdelay];             // dynamically allocate 48000 samples
    memset(dline,0,m_maxdelay*sizeof(double));   // initialise array to 0
    m_delay = 0;                    // if no delay time is set later then delay is set                                to 0.
    reset();
}

// Deconstructor
DelayClass::~DelayClass()
{
    if(dline)
        {
            delete [] dline;            // deconstructor checks whether dline exists.
            dline = nullptr;            // if it does, it is deleted
        }
}

void DelayClass::reset()
{
    rptr = wptr = 0;                    // re-inits array indexes
    m_delay = 0;                        // no delay
    memset(dline,0,m_maxdelay*sizeof(double));      // clears memory
}

double DelayClass::processaudio(double sample, int delay_samples)
{
    // Processing
    // Use new delay time
    m_delay = delay_samples;
    // Calculate Read Index Position
    rptr = wptr - m_delay;
    if (rptr < 0)
    {
        rptr += m_maxdelay;
    }
    
    // Delay Algorithm
    double temp = dline[rptr];
    dline[wptr] = sample;
    
    // Increment write pointer
    wptr++;
     if (wptr >= m_maxdelay)
     {
         wptr = 0;
     }
    // it actually reads before writing, so as to avoid to get the old samples the new one is forced to the output when delay time is set to 0
    if (m_delay == 0) temp = sample;
    return temp;
}

    

