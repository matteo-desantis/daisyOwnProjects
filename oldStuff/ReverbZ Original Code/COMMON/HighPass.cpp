//
//  LowPass.cpp
//
//
//  Created by Matteo Desantis on 22/10/2020.
//

#include "HighPass.hpp"
#include <cmath>

HighPass::HighPass()
{
    //init
    m_HPMM = 0.0;
    m_HPFB = 1.0;
}
//--------------------------------------------------

double HighPass::HP(double sample, float norm_cutoff_freq)
{
    // filter coefficient calculation
    m_HPFB = exp(-norm_cutoff_freq);                // -norm_cutoff_freq = wc = 2*pi*f/Fs
    
    double mid = sample + m_HPFB*m_HPMM;            // Direct Form 1 mid point
    double temp = m_HPFB*(mid - m_HPMM);      // Filter output
    double abstemp = fabs(temp);    //solve denormalisation problem
    if (abstemp < 3.e-34)
    {
        temp = 0.0;
    }
    
    m_HPMM = mid;                                   // memory calculation
    
    // passband gain set to 0dB circa
    temp = temp/m_HPFB;
    
    return temp;
}


/* m_LPFB coefficient calculation from https://dsp.stackexchange.com/questions/54086/single-pole-iir-low-pass-filter-which-is-the-correct-formula-for-the-decay-coe
*/

