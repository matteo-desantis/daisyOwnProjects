# --------------------- Makefile for ReverbZpatch project -------------------- #

# Project Name
TARGET = ReverbZpatch

# Build type: release or debug
BUILD_TYPE ?= release

ifeq ($(BUILD_TYPE),debug)
    OPT = -O0 -g3
    LDFLAGS += -specs=rdimon.specs
    C_DEFS += -DDEBUG
else
    OPT = -O2
endif

# Sources
# Only compile .cpp sources.
CPP_SOURCES = ReverbZpatch.cpp

# Library Locations
LIBDAISY_DIR = ../../libDaisy/
DAISYSP_DIR = ../../DaisySP/

# Optional: ask the core Makefile to build dspLib as a static library
# by defining DSPLIB_DIR before including the core makefile.
DSPLIB_DIR = ../../dspLib

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

