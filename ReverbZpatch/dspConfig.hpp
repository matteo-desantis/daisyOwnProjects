#ifndef REVERBZPATCH_CONFIG_HPP
#define REVERBZPATCH_CONFIG_HPP

#pragma once
#include <cstddef>


// Define the maximum buffer size for all dspLib templates used in ReverbZpatch.
// This is used by RingBuffer, DelayLine, LFO, and AllPass classes
constexpr std::size_t DSPLIB_MAX_BUFFER_SIZE = 8192;

// TODO: Add sample rate and buffer scale factors?

#endif // REVERBZPATCH_CONFIG_HPP