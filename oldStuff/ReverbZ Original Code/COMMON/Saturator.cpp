//
//  Saturator.cpp
//  Reverbz_VST
//
//  Created by Matteo Desantis on 05/01/2021.
//

#include "Saturator.hpp"
#include <cmath>

Saturator::Saturator()
{
    // init
    drive = 0.1;
}

//-------------------------------------------

double Saturator::processaudio_atan(double sample, double drive_db)
{
    double temp = 0.0;
    double input = sample;
    // take the drive value passed as a db value and converts it to linear
    drive = pow(10.0, drive_db/20.0);
    if (drive < 1.0)
    {
        temp = atan(input*drive)/atan(drive);
    }
    else
    {
        // normalise to avoid volume increase - empirical derivation
        temp = atan(input*drive)/((0.9 + 0.1*drive)*atan(drive));
    }
    return temp;
}
double Saturator::processaudio_tanh(double sample, double drive_db)
{
    double input = sample;
    double temp = 0.0;
    // take the drive value passed as a db value and converts it to linear
    drive = pow(10.0, drive_db/20.0);
    if (drive < 1.0)
    {
        temp = tanh(input*drive)/tanh(drive);
    }
    else
    {
        // normalise to avoid volume increase - empirical derivation
        temp = tanh(input*drive)/((0.7 + 0.3 *drive)*tanh(drive));
    return temp;
    }
}

