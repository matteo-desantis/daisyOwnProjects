//
//  HighPass.hpp
//
//
//  Created by Matteo Desantis on 2/01/2020.
//

#ifndef HighP_hpp
#define HighP_hpp

class HighPass
{
public:
    HighPass();
    double HP(double sample, float norm_cutoff_freq);
private:
    float m_HPMM;   // single sample delay line memory
    float m_HPFB;   // feedback coefficient
};

#endif

