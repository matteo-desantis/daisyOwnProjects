/*
    ctrlUtils.hpp

    Control CV/Hardware utility functions.

    Matteo Desantis 11-Dec-2025
*/


#ifndef ctrlUtils_hpp
#define ctrlUtils_hpp

namespace projLib {

inline float limitPotAndCv(float potValue, float cvValue)
{
    float combined = potValue + cvValue;
    if (combined > 1.0f) return 1.0f;
    if (combined < 0.0f) return 0.0f;
    return combined;
};

inline float softTakeover(float currentPotValue, float storedValue)
{
    // Soft-takeover logic: if the difference between the current pot value and the stored value is above a threshold, return the new value, otherwise return the stored value
    float threshold = 0.025f; // Hardcoded threshold, 2.5% on normalized [0.0 - 1.0] scale
    return (std::abs(currentPotValue - storedValue) > threshold) ? currentPotValue : storedValue;
};

inline float softTakeoverWithCv(float currentPotValue, float cvValue, float storedValue)
{
    // Soft-takeover logic with CV: if the difference between the current pot value and the stored value is above a threshold, return the new combined value, otherwise return the stored value
    return softTakeover(limitPotAndCv(currentPotValue, cvValue), storedValue);
};

}   // namespace projLib

#endif /* ctrlUtils_hpp */