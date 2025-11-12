//
//  DeZipper.cpp
//
//  PARAMETER SMOOTHING
//
//  MyFirstPlug2_VST
//
//  Created by Matteo Desantis on 22/10/2020.
//

#include "DeZipper.hpp"
#include <cmath>
//  #define is_almost_denormal(f) (fabs(f) < 3.e-34) would do the same job do solve denormal problem.

DeZipper::DeZipper()
{
    m_DZMM = 0.0;
    m_DZFB = 0.5;         //fb coefficient
    m_DZFF = 1.0 - m_DZFB;  //ff coefficient
}
//--------------------------------------------------

double DeZipper::smooth(double sample, double amount)
{
    m_DZFB = amount;
    m_DZFF = 1.0 - m_DZFB;
    double temp = m_DZFF*sample + m_DZFB*m_DZMM;
    
    double abstemp = fabs(temp);    //solve denormalisation problem
    if (abstemp < 3.e-34)
    {
        temp = 0.0;
    }
    m_DZMM = temp;
    
    return temp;
}

