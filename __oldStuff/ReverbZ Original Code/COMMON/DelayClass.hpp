//
//  Delay.hpp
//
//
//  Created by Matteo Deantis on 17/12/2020
//
 
#ifndef DelayClass_hpp
#define DelayClass_hpp
 
#include <stdio.h>
#include <string.h>
 
class DelayClass
{
public:
    DelayClass();                    // constructor
    ~DelayClass();                   // destructor
    void reset();
    double processaudio(double sample, int delay_samples);

private:
    int m_delay;                // actual delay time
    int m_maxdelay;             // maximum delay time
    int rptr, wptr;             // read write indexes
    double *dline = nullptr;    // pointer to memory
};
 
#endif /* Delay_hpp */
