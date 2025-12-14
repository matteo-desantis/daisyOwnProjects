//
//  DeZipper.hpp
//  MyFirstPlug2_VST
//
//  Created by Matteo Desantis on 22/10/2020.
//

#ifndef DeZipper_hpp
#define DeZipper_hpp

#include <stdio.h>

class DeZipper
{
public:
    DeZipper();
    double smooth(double sample, double amount);
private:
    double m_DZMM;   //single sample delay line memory
    double m_DZFB;   //feedback coefficient
    double m_DZFF;   //feedforward coefficient
};

#endif //DeZipper_hpp

