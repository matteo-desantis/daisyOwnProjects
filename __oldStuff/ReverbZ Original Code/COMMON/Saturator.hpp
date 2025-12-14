//
//  Saturator.hpp
//  Reverbz_VST
//
//  Created by Matteo Desantis on 05/01/2021.
//

#ifndef Saturator_hpp
#define Saturator_hpp

#include <stdio.h>

class Saturator
{
public:
    Saturator();
    double processaudio_atan(double sample, double drive_db);
    double processaudio_tanh(double sample, double drive_db);
private:
    double drive;
};


#endif /* Saturator_hpp */
