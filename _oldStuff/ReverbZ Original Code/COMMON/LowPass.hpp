//
//  LowPass.hpp
//
//
//  Created by Matteo Desantis on 10/12/2020.
//

#ifndef LowP_hpp
#define LowP_hpp

class LowPass
{
public:
    LowPass();
    double LP(double sample, float norm_cutoff_freq);
private:
    float m_LPMM;   //single sample delay line memory
    float m_LPFB;   //feedback coefficient
    float m_LPFF;   //feedforward coefficient
};

#endif
