//
//  AllPass.hpp
//
//  Created by Matteo Desantis on 22/10/2020.
//

#ifndef AllPass_hpp
#define AllPass_hpp

#include <stdio.h>
#include <string.h>
//#include "fxobjects.hpp"

class AllPass
{
public:
    AllPass();          // constructor
    ~AllPass();         // destructor
    void reset();
    double processaudio(double sample, double fb_coeff, int delay_time);
    double processaudio_mod(double sample, double fb_coeff, int delay_time, int amplitude);
    void set_sine_array(double Fs, double lfo_requency);
private:
    int m_delay;        // delay samples
    int m_maxdelay;     // circular buffer size = max delay samples
    int rptr, wptr;     // read position index, write position index
    double m_alpha;     // feedback coefficient value
    double *dline = nullptr;      // pointer to memory
    int N;              // LFO array length
    int sampling_rate;
    double lfo_frequency;
    int amplitude;
    double *sine_array = nullptr;       // array to store sin values
    int j;              // sine array counter
};

#endif //AllPass_hpp

