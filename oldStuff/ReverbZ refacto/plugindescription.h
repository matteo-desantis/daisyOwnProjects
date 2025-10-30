#pragma once

// Dummy plugin description header for Intellisense

namespace controlID {
	enum {
		predelayTime = 0,
		inputLowpassFc,
		inputHighpassFc,
		inputDiffusion,
		decay,
		drive,
		hfDamping,
		lfDamping,
		mixPercentage,
		smooth
	};
}

enum controlVariableType {
	kDouble,
	kInt
};

enum boundVariableType {
	kDouble,
	kInt
};

namespace taper {
	enum {
		kLinearTaper,
		kAntiLogTaper,
		kLogTaper
	};
}

struct PluginParameter {
	PluginParameter(int, const char*, const char*, controlVariableType, double, double, double, int) {}
	PluginParameter(int, const char*, const char*, const char*) {}
	void setBoundVariable(double*, boundVariableType) {}
	void setBoundVariable(int*, boundVariableType) {}
};

inline void addPluginParameter(PluginParameter*) {}

namespace smoothEnum {
	enum {
		Off = 0,
		On = 1
	};
}
// --- CMAKE generated variables for your plugin

#include "pluginstructures.h"

// --- Dummy pluginstructures.h dependency ---
#ifndef __pluginstructures_h__
#define __pluginstructures_h__
// Add any dummy types needed for Intellisense
struct ResetInfo {};
struct ProcessFrameInfo {
	double audioInputFrame[2];
	double audioOutputFrame[2];
};
struct ParameterUpdateInfo {};
#endif

// --- Dummy readerwriterqueue.h dependency ---
#ifndef __readerwriterqueue_h__
#define __readerwriterqueue_h__
// Empty dummy for Intellisense
#endif

#ifndef _plugindescription_h
#define _plugindescription_h

#define QUOTE(name) #name
#define STR(macro) QUOTE(macro)
#define AU_COCOA_VIEWFACTORY_STRING STR(AU_COCOA_VIEWFACTORY_NAME)
#define AU_COCOA_VIEW_STRING STR(AU_COCOA_VIEW_NAME)

// --- AU Plugin Cocoa View Names (flat namespace) 
#define AU_COCOA_VIEWFACTORY_NAME AUCocoaViewFactory_5B184885C555363A9401CFDF950C86EF
#define AU_COCOA_VIEW_NAME AUCocoaView_5B184885C555363A9401CFDF950C86EF

// --- BUNDLE IDs (MacOS Only) 
const char* kAAXBundleID = "developer.aax.reverbz.bundleID";
const char* kAUBundleID = "developer.au.reverbz.bundleID";
const char* kVST3BundleID = "developer.vst3.reverbz.bundleID";

// --- Plugin Names 
const char* kPluginName = "ReverbZ";
const char* kShortPluginName = "ReverbZ";
const char* kAUBundleName = "ReverbZ_AU";


// --- VST3 UUID 
const char* kVSTFUID = "{5b184885-c555-363a-9401-cfdf950c86ef}";

// --- 4-char codes 
const int32_t kFourCharCode = 'RVBZ';
const int32_t kAAXProductID = 'RVBZ';
const int32_t kManufacturerID = 'DesP';

// --- Vendor information 
const char* kVendorName = "DesPlugZ";
const char* kVendorURL = "";
const char* kVendorEmail = "";

// --- Plugin Options 
const bool kWantSidechain = false;
const uint32_t kLatencyInSamples = 0;
const double kTailTimeMsec = 0;
const bool kVSTInfiniteTail = false;
const bool kVSTSAA = false;
const uint32_t kVST3SAAGranularity = 1;
const uint32_t kAAXCategory = 0;

#endif
