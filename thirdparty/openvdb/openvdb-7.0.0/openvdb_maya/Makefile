# Copyright Contributors to the OpenVDB Project
# SPDX-License-Identifier: MPL-2.0
#
# Makefile for OpenVDB Maya plugins
#

# Targets:
#   all                 [default target] all Maya plugins
#   install             install plugins, scripts, etc. into MAYA_DESTDIR
#   clean               delete generated files from the local directory
#   depend              recompute source file header dependencies
#   header_test         check for missing or indirectly included headers
#
# Options:
#   abi=N               build for compatibility with version N of the
#                       OpenVDB Grid ABI, where N is 2, 3, 4, etc.
#                       (some newer features will be disabled)
#   bits=32|64          build for 32-bit or 64-bit environments (default: 64)
#   debug=yes           build with debugging symbols and without optimization
#   verbose=yes         run commands in verbose mode
#   strict=yes          Enable a collection of pre defined compiler warnings
#                       for GCC and clang


#
# The following variables must be defined, either here or on the command line
# (e.g., "make install DESTDIR=/usr/local"):
#
# Note that the OpenVDB library and these Maya plugins must be built
# against compatible versions of the Boost, OpenEXR and TBB libraries.
#

# MAYA_LOCATION should be set to the directory where Maya is installed.
ifeq (,$(MAYA_LOCATION))
$(warning Warning: $$(MAYA_LOCATION) is not set)
endif


# The directory into which to install libraries and header files
# (it is recommended that this match the DESTDIR in the OpenVDB
# library makefile)
DESTDIR := /tmp/OpenVDB

# The directory into which to install libraries (e.g., for Linux multiarch support)
DESTDIR_LIB_DIR := $(DESTDIR)/lib

# The directory into which to install Maya plugins, scripts, etc.
MAYA_DESTDIR := $(DESTDIR)/maya

# The parent directory of the openvdb/ header directory
OPENVDB_INCL_DIR := $(DESTDIR)/include
# The directory containing libopenvdb
OPENVDB_LIB_DIR := $(DESTDIR_LIB_DIR)
OPENVDB_LIB := -lopenvdb

# The parent directory of the boost/ header directory
BOOST_INCL_DIR :=
# The directory containing libboost_iostreams, libboost_system, etc.
BOOST_LIB_DIR :=
BOOST_LIB := -lboost_iostreams -lboost_system

# The parent directory of the OpenEXR/ header directory (which contains half.h)
ILMBASE_INCL_DIR :=
# The directory containing libHalf
ILMBASE_LIB_DIR :=
HALF_LIB := -lHalf

# The parent directory of the tbb/ header directory
TBB_INCL_DIR := $(MAYA_LOCATION)/include
# The directory containing libtbb
TBB_LIB_DIR := $(MAYA_LOCATION)/lib
TBB_LIB := -ltbb


#
# Ideally, users shouldn't need to change anything below this line.
#

SHELL = /bin/bash

# Turn off implicit rules for speed
.SUFFIXES:

ifneq (,$(INSTALL_DIR))
    $(warning Warning: $$(INSTALL_DIR) is no longer used; set $$(DESTDIR) instead.)
endif

# Determine the platform.
ifeq ("$(OS)","Windows_NT")
    WINDOWS_NT := 1
else
    UNAME_S := $(shell uname -s)
    ifeq ("$(UNAME_S)","Linux")
        LINUX := 1
    else
        ifeq ("$(UNAME_S)","Darwin")
            MBSD := 1
        endif
    endif
endif

MAYA_INCL_DIR := $(MAYA_LOCATION)/include

ifeq (yes,$(strip $(debug)))
    OPTIMIZE := -g
else
    OPTIMIZE := -O3 -DNDEBUG
endif

ifeq (yes,$(strip $(verbose)))
    QUIET :=
else
    QUIET := > /dev/null
endif

INCLDIRS := \
    -I . \
    -I $(OPENVDB_INCL_DIR) \
    -isystem $(BOOST_INCL_DIR) \
    -isystem $(ILMBASE_INCL_DIR) \
    -isystem $(TBB_INCL_DIR) \
    -isystem $(MAYA_INCL_DIR) \
#

CXXFLAGS += -std=c++11

CXXFLAGS += -pthread $(OPTIMIZE) $(INCLDIRS)
abi := $(strip $(abi))
ifneq (,$(abi))
    CXXFLAGS += -DOPENVDB_3_ABI_COMPATIBLE  # TODO: deprecated
    CXXFLAGS += -DOPENVDB_ABI_VERSION_NUMBER=$(abi)
endif
# From $(MAYA_LOCATION)/devkit/plug-ins/buildconfig:
CXXFLAGS += \
    -D_BOOL \
    -DFUNCPROTO \
    -DGL_GLEXT_PROTOTYPES=1 \
    -DREQUIRE_IOSTREAM \
    -DUNIX \
    -fno-gnu-keywords \
    -fno-omit-frame-pointer \
    -fno-strict-aliasing \
    -funsigned-char \
    -Wno-comment \
    -Wno-multichar \
    -Wno-strict-aliasing \
#

ifeq (32,$(strip $(bits)))
    CXXFLAGS += -m32 -DBits32_
else
    CXXFLAGS += -m64 -DBits64_
endif

ifdef LINUX
    CXXFLAGS += -DLINUX
    ifeq (32,$(strip $(bits)))
        CXXFLAGS += -DLINUX_32
    else
        CXXFLAGS += -DLINUX_64
    endif
endif
ifdef MBSD
    # Darwin ld treats undefined symbols as errors by default;
    # change to runtime resolution, like Linux.
    CXXFLAGS += -undefined dynamic_lookup
endif

ifeq (yes,$(strip $(strict)))
    USING_CLANG=$(shell ${CXX} --version | grep clang)
    USING_GCC=$(shell ${CXX} --version | grep GCC)
    ifneq (,$(USING_CLANG))
        CXXFLAGS += \
            -Werror \
            -Wall \
            -Wextra \
            -Wconversion \
            -Wno-sign-conversion \
        #
    else ifneq (,$(USING_GCC))
        CXXFLAGS += \
            -Werror \
            -Wall \
            -Wextra \
            -pedantic \
            -Wcast-align \
            -Wcast-qual \
            -Wconversion \
            -Wdisabled-optimization \
            -Woverloaded-virtual \
        #
    endif
endif

MAYA_LIBS := \
    -lOpenMayaFX \
    -lOpenMaya \
    -lFoundation \
#

LIBS := \
    -Wl,-rpath,$(OPENVDB_LIB_DIR) -L$(OPENVDB_LIB_DIR) $(OPENVDB_LIB) \
    -Wl,-rpath,$(ILMBASE_LIB_DIR) -L$(ILMBASE_LIB_DIR) $(HALF_LIB) \
    -Wl,-rpath,$(TBB_LIB_DIR) -L$(TBB_LIB_DIR) $(TBB_LIB) \
    -Wl,-rpath,$(BOOST_LIB_DIR) -L$(BOOST_LIB_DIR) $(BOOST_LIB) \
    -Wl,-rpath,$(MAYA_LOCATION)/lib -L$(MAYA_LOCATION)/lib $(MAYA_LIBS) \
    -ldl -lm -lz \
#
ifdef LINUX
    LIBS += -lrt
endif

MAYA_SRC_NAMES := \
    OpenVDBCopyNode.cc \
    OpenVDBData.cc \
    OpenVDBFilterNode.cc \
    OpenVDBFromMayaFluidNode.cc \
    OpenVDBFromPolygonsNode.cc \
    OpenVDBPlugin.cc \
    OpenVDBReadNode.cc \
    OpenVDBToPolygonsNode.cc \
    OpenVDBTransformNode.cc \
    OpenVDBUtil.cc \
    OpenVDBVisualizeNode.cc \
    OpenVDBWriteNode.cc \
#
MAYA_INCLUDE_NAMES := \
    OpenVDBData.h \
    OpenVDBPlugin.h \
    OpenVDBUtil.h \
#

MAYA_SCRIPTS := \
    AEOpenVDBCopyTemplate.mel \
    AEOpenVDBFilterTemplate.mel \
    AEOpenVDBFromMayaFluidTemplate.mel \
    AEOpenVDBFromPolygonsTemplate.mel \
    AEOpenVDBReadTemplate.mel \
    AEOpenVDBToPolygonsTemplate.mel \
    AEOpenVDBTransformTemplate.mel \
    AEOpenVDBVisualizeTemplate.mel \
    AEOpenVDBWriteTemplate.mel \
#

ALL_INCLUDE_FILES := \
    $(MAYA_INCLUDE_NAMES) \
#
MAYA_SRC_FILES := \
    $(MAYA_SRC_NAMES) \
#
ALL_SRC_FILES := $(MAYA_SRC_FILES)

MAYA_OBJ_NAMES := $(MAYA_SRC_NAMES:.cc=.o)
MAYA_SO_NAME := OpenVDB.so

DEPEND := maya_deps

# Get the list of dependencies that are newer than the current target,
# but limit the list to at most three entries.
list_deps = $(if $(wordlist 4,5,$(?F)),$(firstword $(?F)) and others,$(wordlist 1,3,$(?F)))

ALL_TARGETS := \
    $(DEPEND) \
    $(MAYA_SO_NAME) \
#

.SUFFIXES: .o .cc

.PHONY: all clean depend header_test install

.cc.o:
	@echo "Building $@ because of $(call list_deps)"
	$(CXX) -c $(CXXFLAGS) -fPIC -o $@ $<

all: $(ALL_TARGETS)

# Create an openvdb_maya/ symlink to this directory, to mirror the DWA directory structure.
openvdb_maya:
	ln -f -s . openvdb_maya
openvdb_maya/OpenVDBData.h openvdb_maya/OpenVDBUtil.h: openvdb_maya

$(MAYA_OBJ_NAMES): openvdb_maya/OpenVDBData.h openvdb_maya/OpenVDBUtil.h

$(MAYA_SO_NAME): $(MAYA_OBJ_NAMES)
	@echo "Building $@ because of $(call list_deps)"
	$(CXX) $(CXXFLAGS) -fPIC -Wl,-Bsymbolic -shared -o $@ $^ $(LIBS)

install: $(MAYA_SO_NAME)
	mkdir -p $(MAYA_DESTDIR)/plugins
	@echo "Created $(MAYA_DESTDIR)/plugins"
	cp -f $(MAYA_SO_NAME) $(MAYA_DESTDIR)/plugins/
	@echo "Copied $(MAYA_SO_NAME) to $(MAYA_DESTDIR)/plugins/"
	mkdir -p $(MAYA_DESTDIR)/scripts
	@echo "Created $(MAYA_DESTDIR)/scripts"
	cp -f $(MAYA_SCRIPTS) $(MAYA_DESTDIR)/scripts/
	@echo "Copied MEL scripts to $(MAYA_DESTDIR)/scripts/"

# TODO: This accumulates all source file dependencies into a single file
# containing a rule for each *.o file.  Consider generating a separate
# dependency file for each *.o file instead.
$(DEPEND): $(ALL_INCLUDE_FILES) $(ALL_SRC_FILES)
	@echo "Generating dependencies because of $(call list_deps)"
	$(RM) $(DEPEND)
	for f in $(ALL_SRC_FILES); \
		do $(CXX) $(CXXFLAGS) -O0 -MM $$f -MT `echo $$f | sed 's%\.[^.]*%.o%'` \
			-isystem $(MAYA_INCL_DIR) >> $(DEPEND); \
	done

depend: $(DEPEND)

# Compile an implicit translation unit for each header to identify any indirect includes
HEADER_TEST_FILES := $(addprefix header_test-,$(MAYA_INCLUDE_NAMES))
$(HEADER_TEST_FILES): header_test-%:
	echo "#include \"$*\"" | $(CXX) -c -x c++ $(CXXFLAGS) \
	    -isystem $(MAYA_INCL_DIR) -fPIC -o /dev/null -
echo_header_test:
	@echo "Checking for missing or indirectly included headers"
header_test: echo_header_test $(HEADER_TEST_FILES)

clean:
	$(RM) $(ALL_TARGETS) $(DEPEND) $(MAYA_OBJ_NAMES)

ifneq (,$(strip $(wildcard $(DEPEND))))
    include $(DEPEND)
endif
