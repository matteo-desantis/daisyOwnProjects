# Project Name
TARGET = ReverbZpatch

# Sources
CPP_SOURCES = ReverbZpatch.cpp \
			  ../_projLib/ReverbZ.cpp

# Library Locations
LIBDAISY_DIR = ../../libDaisy/
DAISYSP_DIR = ../../DaisySP/

# Optional: ask the core Makefile to build dspLib as a static library
# by defining DSPLIB_DIR before including the core makefile.
DSPLIB_DIR = ../../dspLib

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

