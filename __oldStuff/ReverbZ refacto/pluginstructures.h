#pragma once

// Dummy pluginstructures.h for Intellisense

#ifndef __pluginstructures_h__
#define __pluginstructures_h__

struct ResetInfo {};
struct ProcessFrameInfo {
	double audioInputFrame[2];
	double audioOutputFrame[2];
};
struct ParameterUpdateInfo {};

#endif // __pluginstructures_h__

// Dummy readerwriterqueue.h for Intellisense
#ifndef READERWRITERQUEUE_H_INCLUDED
#define READERWRITERQUEUE_H_INCLUDED
// Add minimal dummy types/classes if needed
namespace moodycamel {
	template<typename T>
	class ReaderWriterQueue {
	public:
		ReaderWriterQueue() {}
		bool enqueue(const T&) { return true; }
		bool try_dequeue(T&) { return true; }
	};
}
#endif

// Dummy atomicops.h for Intellisense
#ifndef ATOMICOPS_H_INCLUDED
#define ATOMICOPS_H_INCLUDED
namespace atomicops {
	inline void fence() {}
}
#endif
// -----------------------------------------------------------------------------
//    ASPiK Plugin Kernel File:  pluginstructures.h
//
/**
    \file   pluginstructures.h
    \author Will Pirkle
    \date   17-September-2018
    \brief  globally utilized structures and enumerations
    		- http://www.aspikplugins.com
    		- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
#ifndef _pluginstructures_h
#define _pluginstructures_h

#include <string>
#include <sstream>
#include <vector>
#include <stdint.h>

#endif
