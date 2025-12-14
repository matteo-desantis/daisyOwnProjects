//
//  LowPass.cpp
//
//
//  Created by Matteo Desantis on 22/10/2020.
//

#include "LowPass.hpp"
#include <cmath>

LowPass::LowPass()
{
    //init
    m_LPMM = 0.0;
    m_LPFF = 1.0;
    m_LPFB = 1.0;
}
//--------------------------------------------------

double LowPass::LP(double sample, float norm_cutoff_freq)
{
    // filter coefficients calculation
    m_LPFB = exp(-norm_cutoff_freq);             // -norm_cutoff_freq = wc = 2*pi*f/Fs
    m_LPFF = 1 - m_LPFB;                         // output gain in the passband is kept to 0dB
    
    double temp = m_LPFF*sample + m_LPFB*m_LPMM; //filter output
    double abstemp = fabs(temp);    //solve denormalisation problem
    if (abstemp < 3.e-34)
    {
        temp = 0.0;
    }
    
    m_LPMM = temp;                               //memory calculation 
    
    return temp;
}


/* m_LPFB coefficient calculation from https://dsp.stackexchange.com/questions/54086/single-pole-iir-low-pass-filter-which-is-the-correct-formula-for-the-decay-coe
*/
