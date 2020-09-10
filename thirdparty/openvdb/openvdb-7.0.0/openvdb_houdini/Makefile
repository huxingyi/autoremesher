# Copyright Contributors to the OpenVDB Project
# SPDX-License-Identifier: MPL-2.0
#
# Makefile for OpenVDB Houdini DSOs
#
# This makefile assumes that you are in a Houdini environment,
# i.e., that you have sourced the houdini_setup script.
#

# Targets:
#   houdinilib          utility library for Houdini OpenVDB SOPs
#   houdinisops         Houdini OpenVDB SOPs (installed into HOUDINI_DESTDIR)
#   mantradsos          Mantra OpenVDB procedurals
#                       (installed into HOUDINI_DESTDIR/mantra)
#
#   all                 [default target] all of the above
#   install_lib         install houdinilib into subdirectories of DESTDIR
#   install             install houdinilib into subdirectories of DESTDIR,
#                       and the SOPs into HOUDINI_DESTDIR
#   clean               delete generated files from the local directory
#   depend              recompute source file header dependencies
#   header_test         check for missing or indirectly included headers
#
# Options:
#   abi=N               build for compatibility with version N of the
#                       OpenVDB Grid ABI, where N is 2, 3, 4, etc.
#                       (some newer features will be disabled)
#   shared=no           link executables against static OpenVDB libraries
#                       (default: link against shared libraries)
#   debug=yes           build with debugging symbols and without optimization
#   verbose=yes         run commands (in particular, hcustom and doxygen)
#                       in verbose mode
#   strict=yes          Enable a collection of pre defined compiler warnings
#                       for GCC and clang


#
# The following variables must be defined, either here or on the command line
# (e.g., "make install DESTDIR=/usr/local"):
#
# Note that the OpenVDB library and these Houdini tools must be built
# against compatible versions of the Boost, OpenEXR and TBB libraries.
# Until Houdini 16.5, all three were included in the HDK, so the relevant
# variables below point by default to the HDK library and header directories:
# $(HDSO) and $(HT)/include, respectively.  (Source the houdini_setup script
# to set those two environment variables.)  You must provide your own
# distribution of Boost.
#

ifeq (,$(HFS))
$(warning Warning: $$(HFS) is not set; did you source houdini_setup?)
endif


# The directory into which to install libraries and header files
# (it's recommended that this match the DESTDIR in the OpenVDB
# library makefile)
DESTDIR := /tmp/OpenVDB

# The directory into which to install libraries (e.g., for Linux multiarch support)
DESTDIR_LIB_DIR := $(DESTDIR)/lib

# The directory into which to install Houdini SOPs
HOUDINI_DESTDIR := $(HIH)/dso

# The directory into which to install the Houdini startup script
# (leave blank to not install the script)
HOUDINI_PYTHONRC_DESTDIR := $(HOUDINI_DESTDIR)/python2.7libs

# The parent directory of the openvdb/ header directory
OPENVDB_INCL_DIR := $(DESTDIR)/include
# The directory containing libopenvdb
OPENVDB_LIB_DIR := $(DESTDIR_LIB_DIR)
OPENVDB_LIB := -lopenvdb

# The parent directory of the boost/ header directory
BOOST_INCL_DIR := $(HT)/include
# The directory containing libboost_iostreams, libboost_system, etc.
BOOST_LIB_DIR := $(HDSO)
BOOST_LIB := -lboost_iostreams -lboost_system

# The parent directory of the OpenEXR/ header directory (which contains half.h)
ILMBASE_INCL_DIR := $(HT)/include
# The directory containing libHalf
ILMBASE_LIB_DIR := $(HDSO)
HALF_LIB := -lHalf

# The parent directory of the tbb/ header directory
TBB_INCL_DIR := $(HT)/include
# The directory containing libtbb
TBB_LIB_DIR := $(HDSO)
TBB_LIB := -ltbb

# The parent directory of the log4cplus/ header directory
# (leave blank if log4cplus is unavailable)
LOG4CPLUS_INCL_DIR :=
# The directory containing liblog4cplus
LOG4CPLUS_LIB_DIR :=
LOG4CPLUS_LIB := -llog4cplus


#
# Ideally, users shouldn't need to change anything below this line.
#

SHELL = /bin/bash

# Turn off implicit rules for speed
.SUFFIXES:

ifneq (,$(INSTALL_DIR))
    $(warning Warning: $$(INSTALL_DIR) is no longer used; set $$(DESTDIR) instead.)
endif

# Error if Houdini version is lower than minimum supported version
ifeq (1,$(shell expr "$(HOUDINI_MAJOR_RELEASE).$(HOUDINI_MINOR_RELEASE)" "<" "17.0"))
    $(error Error: Unsupported Houdini Version: $(HOUDINI_VERSION))
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

# TODO: For historical reasons, HOUDINI_DESTDIR specifies the installation
# directory for Houdini DSOs.  It would probably be better if it specified
# the parent directory.
HOUDINI_USER_PREF_DIR := $(abspath $(HOUDINI_DESTDIR)/..)

HOUDINI_INCL_DIR := $(HT)/include
# Flags to be passed to the hcustom build script
HOUDINI_INSTALL := -i $(HOUDINI_DESTDIR)
MANTRA_INSTALL := -i $(HOUDINI_DESTDIR)/mantra
HCUSTOM_FLAGS :=
ifeq (yes,$(strip $(verbose)))
    HCUSTOM_FLAGS += -e
endif
# Extra flags to be passed through hcustom to the compiler or linker
HCUSTOM_EXTRA_CFLAGS := -DIGNORE_HDK_DEPRECATIONS
HCUSTOM_EXTRA_LDFLAGS :=

ifeq (yes,$(strip $(debug)))
    OPTIMIZE := -g
    HCUSTOM_FLAGS += -g
else
    OPTIMIZE := -O3 -DNDEBUG
endif

ifeq (yes,$(strip $(verbose)))
    QUIET :=
else
    QUIET := > /dev/null
endif

has_log4cplus := no
ifneq (,$(and $(LOG4CPLUS_LIB_DIR),$(LOG4CPLUS_INCL_DIR),$(LOG4CPLUS_LIB)))
    has_log4cplus := yes
endif

INCLDIRS := -I . -I .. -isystem $(BOOST_INCL_DIR) -isystem $(ILMBASE_INCL_DIR) -isystem $(TBB_INCL_DIR)
ifeq (yes,$(has_log4cplus))
    INCLDIRS += -isystem $(LOG4CPLUS_INCL_DIR)
endif
HCUSTOM_INCLDIRS := $(subst isystem,I,$(INCLDIRS))

CXXFLAGS += -std=c++11

CXXFLAGS += -pthread $(OPTIMIZE) $(INCLDIRS) -DIGNORE_HDK_DEPRECATIONS
ifeq (yes,$(has_log4cplus))
    CXXFLAGS += -DOPENVDB_USE_LOG4CPLUS
    HCUSTOM_EXTRA_CFLAGS += -DOPENVDB_USE_LOG4CPLUS
endif
abi := $(strip $(abi))
ifneq (,$(abi))
    CXXFLAGS += -DOPENVDB_3_ABI_COMPATIBLE  # TODO: deprecated
    CXXFLAGS += -DOPENVDB_ABI_VERSION_NUMBER=$(abi)
    HCUSTOM_EXTRA_CFLAGS += -DOPENVDB_3_ABI_COMPATIBLE  # TODO: deprecated
    HCUSTOM_EXTRA_CFLAGS += -DOPENVDB_ABI_VERSION_NUMBER=$(abi)
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
        WARNING_FLAGS += \
            -Werror \
            -Wall \
            -Wextra \
            -Wconversion \
            -Wno-sign-conversion \
        #
    else ifneq (,$(USING_GCC))
        WARNING_FLAGS += \
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
    CXXFLAGS += $(WARNING_FLAGS)
    HCUSTOM_EXTRA_CFLAGS += $(WARNING_FLAGS)
endif

ifneq (,$(strip $(wildcard $(HOUDINI_INCL_DIR))))
    HOUDINI_CXXFLAGS := $(shell hcustom -c)
    HOUDINI_LDFLAGS := $(shell hcustom -m)
endif

LIBS := \
    -ldl -lm -lz \
    -L$(ILMBASE_LIB_DIR) $(HALF_LIB) \
    -L$(TBB_LIB_DIR) $(TBB_LIB) \
    -L$(BOOST_LIB_DIR) $(BOOST_LIB) \
#
LIBS_RPATH := \
    -ldl -lm -lz \
    -Wl,-rpath,$(ILMBASE_LIB_DIR) -L$(ILMBASE_LIB_DIR) $(HALF_LIB) \
    -Wl,-rpath,$(TBB_LIB_DIR) -L$(TBB_LIB_DIR) $(TBB_LIB) \
    -Wl,-rpath,$(BOOST_LIB_DIR) -L$(BOOST_LIB_DIR) $(BOOST_LIB) \
#
ifeq (yes,$(has_log4cplus))
    LIBS += -L$(LOG4CPLUS_LIB_DIR) $(LOG4CPLUS_LIB)
    LIBS_RPATH += -Wl,-rpath,$(LOG4CPLUS_LIB_DIR) -L$(LOG4CPLUS_LIB_DIR) $(LOG4CPLUS_LIB)
endif
ifdef LINUX
    LIBS += -lrt
    LIBS_RPATH += -lrt
endif

LIBHOUDINI_INCLUDE_NAMES := \
    AttributeTransferUtil.h \
    GeometryUtil.h \
    GEO_PrimVDB.h \
    GT_GEOPrimCollectVDB.h \
    GU_PrimVDB.h \
    GU_VDBPointTools.h \
    OP_NodeChain.h \
    geometry.h \
    ParmFactory.h \
    PointUtils.h \
    SOP_NodeVDB.h \
    SOP_VDBVerbUtils.h \
    UT_VDBTools.h \
    UT_VDBUtils.h \
    Utils.h \
#
LIBHOUDINI_SRC_NAMES := \
    GeometryUtil.cc \
    GEO_PrimVDB.cc \
    GEO_VDBTranslator.cc \
    GT_GEOPrimCollectVDB.cc \
    GU_PrimVDB.cc \
    GU_VDBPointTools.cc \
    geometry.cc \
    ParmFactory.cc \
    PointUtils.cc \
    SOP_NodeVDB.cc \
    UT_VDBUtils.cc \
    Utils.cc \
#
# Note that creation script names coincide with operator names,
# not (necessarily) with SOP names.
HOUDINI_SOP_SCRIPT_NAMES := \
    DW_OpenVDBRasterizePoints.cmd \
#
HOUDINI_SOP_SRC_NAMES := \
    GR_PrimVDBPoints.cc \
    SHOP_OpenVDB_Points.cc \
    SOP_OpenVDB_Advect.cc \
    SOP_OpenVDB_Advect_Points.cc \
    SOP_OpenVDB_Analysis.cc \
    SOP_OpenVDB_Clip.cc \
    SOP_OpenVDB_Combine.cc \
    SOP_OpenVDB_Convert.cc \
    SOP_OpenVDB_Create.cc \
    SOP_OpenVDB_Densify.cc \
    SOP_OpenVDB_Diagnostics.cc \
    SOP_OpenVDB_Fill.cc \
    SOP_OpenVDB_Filter.cc \
    SOP_OpenVDB_Filter_Level_Set.cc \
    SOP_OpenVDB_Fracture.cc \
    SOP_OpenVDB_From_Particles.cc \
    SOP_OpenVDB_From_Polygons.cc \
    SOP_OpenVDB_LOD.cc \
    SOP_OpenVDB_Metadata.cc \
    SOP_OpenVDB_Morph_Level_Set.cc \
    SOP_OpenVDB_Noise.cc \
    SOP_OpenVDB_Occlusion_Mask.cc \
    SOP_OpenVDB_Platonic.cc \
    SOP_OpenVDB_Points_Convert.cc \
    SOP_OpenVDB_Points_Delete.cc \
    SOP_OpenVDB_Points_Group.cc \
    SOP_OpenVDB_Potential_Flow.cc \
    SOP_OpenVDB_Prune.cc \
    SOP_OpenVDB_Rasterize_Points.cc \
    SOP_OpenVDB_Ray.cc \
    SOP_OpenVDB_Read.cc \
    SOP_OpenVDB_Rebuild_Level_Set.cc \
    SOP_OpenVDB_Remap.cc \
    SOP_OpenVDB_Remove_Divergence.cc \
    SOP_OpenVDB_Resample.cc \
    SOP_OpenVDB_Sample_Points.cc \
    SOP_OpenVDB_Sort_Points.cc \
    SOP_OpenVDB_Scatter.cc \
    SOP_OpenVDB_Segment.cc \
    SOP_OpenVDB_To_Polygons.cc \
    SOP_OpenVDB_To_Spheres.cc \
    SOP_OpenVDB_Topology_To_Level_Set.cc \
    SOP_OpenVDB_Transform.cc \
    SOP_OpenVDB_Vector_Merge.cc \
    SOP_OpenVDB_Vector_Split.cc \
    SOP_OpenVDB_Visualize.cc \
    SOP_OpenVDB_Write.cc \
#
MANTRA_VRAY_SRC_NAMES := \
    VRAY_OpenVDB_Points.cc \
#
HOUDINI_SOP_NAMES := $(basename $(notdir $(HOUDINI_SOP_SRC_NAMES)))
MANTRA_VRAY_NAMES := $(basename $(notdir $(MANTRA_VRAY_SRC_NAMES)))

ALL_INCLUDE_FILES := \
    $(LIBHOUDINI_INCLUDE_NAMES) \
#
HOUDINI_SRC_FILES := \
    $(LIBHOUDINI_SRC_NAMES) \
    $(HOUDINI_SOP_SRC_NAMES) \
#
MANTRA_SRC_FILES := \
    $(MANTRA_VRAY_SRC_NAMES) \
#
ALL_SRC_FILES := $(HOUDINI_SRC_FILES) $(MANTRA_SRC_FILES)

LIBHOUDINI_OBJ_NAMES := $(LIBHOUDINI_SRC_NAMES:.cc=.o)
HOUDINI_SOP_OBJ_NAMES := $(HOUDINI_SOP_SRC_NAMES:.cc=.o)
HOUDINI_SOP_SO_NAMES := $(addprefix $(HOUDINI_DESTDIR)/,\
    $(notdir $(HOUDINI_SOP_SRC_NAMES:.cc=.so)))
MANTRA_VRAY_OBJ_NAMES := $(MANTRA_VRAY_SRC_NAMES:.cc=.o)
MANTRA_VRAY_SO_NAMES := $(addprefix $(HOUDINI_DESTDIR)/mantra/,\
    $(notdir $(MANTRA_VRAY_SRC_NAMES:.cc=.so)))

LIB_MAJOR_VERSION=$(shell grep 'define OPENVDB_LIBRARY_MAJOR_VERSION_NUMBER ' \
    $(OPENVDB_INCL_DIR)/openvdb/version.h | sed 's/[^0-9]*//g')
LIB_MINOR_VERSION=$(shell grep 'define OPENVDB_LIBRARY_MINOR_VERSION_NUMBER ' \
    $(OPENVDB_INCL_DIR)/openvdb/version.h | sed 's/[^0-9]*//g')
LIB_PATCH_VERSION=$(shell grep 'define OPENVDB_LIBRARY_PATCH_VERSION_NUMBER ' \
    $(OPENVDB_INCL_DIR)/openvdb/version.h | sed 's/[^0-9]*//g')

LIB_VERSION=$(LIB_MAJOR_VERSION).$(LIB_MINOR_VERSION).$(LIB_PATCH_VERSION)

LIBHOUDINI_NAME=libopenvdb_houdini
LIBHOUDINI_STATIC := $(LIBHOUDINI_NAME).a
LIBHOUDINI_SHARED := $(LIBHOUDINI_NAME).so.$(LIB_VERSION)

ifeq (no,$(strip $(shared)))
    LIBHOUDINI := $(LIBHOUDINI_STATIC)
else
    LIBHOUDINI := $(LIBHOUDINI_SHARED)
    # Encode the path to libopenvdb.so into plugins, and because some
    # plugins might depend on Houdini libraries that are not linked into
    # hython (among other tools), encode the Houdini library path as well.
    HCUSTOM_EXTRA_LDFLAGS += -Wl,-rpath,$(OPENVDB_LIB_DIR) -Wl,-rpath,$(HDSO)
endif # shared

# Environment variables to pass to hcustom
HCUSTOM_EXPORT :=
ifneq (,$(strip $(HCUSTOM_EXTRA_CFLAGS)$(HCUSTOM_EXTRA_LDFLAGS)))
    ifneq (,$(strip $(HCUSTOM_EXTRA_CFLAGS)))
        HCUSTOM_EXPORT += HCUSTOM_CFLAGS="$(strip $(HCUSTOM_EXTRA_CFLAGS))"
    endif
    ifneq (,$(strip $(HCUSTOM_EXTRA_LDFLAGS)))
        HCUSTOM_EXPORT += HCUSTOM_LDFLAGS="$(strip $(HCUSTOM_EXTRA_LDFLAGS))"
    endif
endif
ifneq (,$(strip $(HCUSTOM_EXPORT)))
    HCUSTOM_EXPORT := export $(HCUSTOM_EXPORT);
endif

DEPEND := houdini_deps

# Get the list of dependencies that are newer than the current target,
# but limit the list to at most three entries.
list_deps = $(if $(wordlist 4,5,$(?F)),$(firstword $(?F)) and others,$(wordlist 1,3,$(?F)))

ALL_TARGETS := \
    $(LIBHOUDINI) \
    $(DEPEND) \
    $(LIBHOUDINI_NAME).so \
#

.SUFFIXES: .o .cc

.PHONY: all clean depend header_test houdinisops mantradsos houdinilib install_lib install

.cc.o:
	@echo "Building $@ because of $(call list_deps)"
	$(CXX) -c $(CXXFLAGS) -fPIC -o $@ $<

all: depend houdinilib houdinisops mantradsos

# Create an openvdb_houdini/ symlink to this directory, to mirror the DWA directory structure.
openvdb_houdini:
	ln -f -s . openvdb_houdini
openvdb_houdini/AttributeTransferUtil.h openvdb_houdini/GeometryUtil.h openvdb_houdini/GEO_PrimVDB.h openvdb_houdini/GT_GEOPrimCollectVDB.h openvdb_houdini/GU_PrimVDB.h openvdb_houdini/GU_VDBPointTools.h openvdb_houdini/SOP_NodeVDB.h openvdb_houdini/UT_VDBTools.h openvdb_houdini/UT_VDBUtils.h openvdb_houdini/Utils.h: openvdb_houdini

# Create a houdini_utils/ symlink to this directory, to mirror the DWA directory structure.
houdini_utils:
	ln -f -s . houdini_utils
houdini_utils/geometry.h houdini_utils/ParmFactory.h houdini_utils/OP_NodeChain.h: houdini_utils

$(LIBHOUDINI_OBJ_NAMES): houdini_utils/geometry.h houdini_utils/ParmFactory.h houdini_utils/OP_NodeChain.h openvdb_houdini/AttributeTransferUtil.h openvdb_houdini/GeometryUtil.h openvdb_houdini/GEO_PrimVDB.h openvdb_houdini/GT_GEOPrimCollectVDB.h openvdb_houdini/GU_PrimVDB.h openvdb_houdini/GU_VDBPointTools.h openvdb_houdini/SOP_NodeVDB.h openvdb_houdini/UT_VDBTools.h openvdb_houdini/UT_VDBUtils.h openvdb_houdini/Utils.h

$(LIBHOUDINI_OBJ_NAMES): %.o: %.cc
	@echo "Building $@ because of $(call list_deps)"
	$(CXX) -c $(CXXFLAGS) $(HOUDINI_CXXFLAGS) \
	    -isystem $(HOUDINI_INCL_DIR) -fPIC -o $@ $<

$(LIBHOUDINI_STATIC): $(LIBHOUDINI_OBJ_NAMES)
	@echo "Building $@ because of $(call list_deps)"
	$(AR) cr $@ $^

$(LIBHOUDINI_SHARED): $(LIBHOUDINI_OBJ_NAMES)
	@echo "Building $@ because of $(call list_deps)"
	$(CXX) $(CXXFLAGS) $(HOUDINI_LDFLAGS) -shared -o $@ $^ $(LIBS_RPATH)

$(LIBHOUDINI_NAME).so: $(LIBHOUDINI_SHARED)
	@# hcustom expects a library named "libname.so", not "libname.so.1.0".
	ln -f -s $< $@

houdinilib: $(LIBHOUDINI)
ifneq (no,$(strip $(shared)))
houdinilib: $(LIBHOUDINI_NAME).so
endif

# hcustom (see below) outputs both a .o and a .so file.
# The .o isn't dependent on $(LIBHOUDINI), but the .so is.
$(HOUDINI_SOP_OBJ_NAMES): $(LIBHOUDINI)
$(MANTRA_VRAY_OBJ_NAMES): $(LIBHOUDINI)

GR_PrimVDBPoints.o: HCUSTOM_FLAGS += -L$(HDSO) -lHoudiniAPPS3
VRAY_OpenVDB_Points.o: HCUSTOM_FLAGS += -L$(HDSO) -lHoudiniRAY

ifeq (no,$(strip $(shared)))
# Link SOPs against static OpenVDB libraries.
$(HOUDINI_SOP_OBJ_NAMES): %.o: %.cc
	@echo "Building $(HOUDINI_DESTDIR)/$(@:.o=.so) because of $(call list_deps)"
	$(HCUSTOM_EXPORT) $(HB)/hcustom $(HOUDINI_INSTALL) $(HCUSTOM_FLAGS) $(HCUSTOM_INCLDIRS) $(LIBS) \
	    -L. -lopenvdb_houdini -L$(OPENVDB_LIB_DIR) $(OPENVDB_LIB) $< $(QUIET)
else
# Link SOPs against shared OpenVDB libraries.
$(HOUDINI_SOP_OBJ_NAMES): %.o: %.cc
	@echo "Building $(HOUDINI_DESTDIR)/$(@:.o=.so) because of $(call list_deps)"
	$(HCUSTOM_EXPORT) $(HB)/hcustom $(HOUDINI_INSTALL) $(HCUSTOM_FLAGS) $(HCUSTOM_INCLDIRS) $(LIBS) \
	    -L. -lopenvdb_houdini -L$(OPENVDB_LIB_DIR) $(OPENVDB_LIB) $< $(QUIET)
endif

ifeq (no,$(strip $(shared)))
# Link SOPs against static OpenVDB libraries.
$(MANTRA_VRAY_OBJ_NAMES): %.o: %.cc
	@echo "Building $(HOUDINI_DESTDIR)/mantra/$(@:.o=.so) because of $(call list_deps)"
	@mkdir -p $(HOUDINI_DESTDIR)/mantra
	$(HCUSTOM_EXPORT) $(HB)/hcustom $(MANTRA_INSTALL) $(HCUSTOM_FLAGS) $(HCUSTOM_INCLDIRS) $(LIBS) \
	    -L. -lopenvdb_houdini -L$(OPENVDB_LIB_DIR) $(OPENVDB_LIB) $< $(QUIET)
else
# Link SOPs against shared OpenVDB libraries.
$(MANTRA_VRAY_OBJ_NAMES): %.o: %.cc
	@echo "Building $(HOUDINI_DESTDIR)/$(@:.o=.so) because of $(call list_deps)"
	@mkdir -p $(HOUDINI_DESTDIR)/mantra
	$(HCUSTOM_EXPORT) $(HB)/hcustom $(MANTRA_INSTALL) $(HCUSTOM_FLAGS) $(HCUSTOM_INCLDIRS) $(LIBS) \
	    -L. -lopenvdb_houdini -L$(OPENVDB_LIB_DIR) $(OPENVDB_LIB) $< $(QUIET)
endif

$(HOUDINI_SOP_SO_NAMES): $(HOUDINI_DESTDIR)/%.so: %.o
$(MANTRA_VRAY_SO_NAMES): $(HOUDINI_DESTDIR)/mantra/%.so: %.o

houdinisops: $(HOUDINI_SOP_SO_NAMES) $(HOUDINI_SOP_SCRIPT_NAMES) SOP_OpenVDB.svg pythonrc.py
	if [ ! -d "$(HOUDINI_USER_PREF_DIR)/config/Icons" ]; \
	then \
	    mkdir -p $(HOUDINI_USER_PREF_DIR)/config/Icons; \
	    echo "Created $(HOUDINI_USER_PREF_DIR)/config/Icons/"; \
	fi
	cp -f SOP_OpenVDB.svg $(HOUDINI_USER_PREF_DIR)/config/Icons/; \
	if [ -n "$(HOUDINI_PYTHONRC_DESTDIR)" ]; \
	then \
	    if [ ! -d "$(HOUDINI_PYTHONRC_DESTDIR)" ]; \
	    then \
	        mkdir -p $(HOUDINI_PYTHONRC_DESTDIR); \
	        echo "Created $(HOUDINI_PYTHONRC_DESTDIR)/"; \
	    fi; \
	    cp -f pythonrc.py $(HOUDINI_PYTHONRC_DESTDIR)/; \
	fi
	if [ ! -d "$(HOUDINI_USER_PREF_DIR)/scripts/sop" ]; \
	then \
	    mkdir -p $(HOUDINI_USER_PREF_DIR)/scripts/sop; \
	    echo "Created $(HOUDINI_USER_PREF_DIR)/scripts/sop/"; \
	fi
	cp -f $(HOUDINI_SOP_SCRIPT_NAMES) $(HOUDINI_USER_PREF_DIR)/scripts/sop/

mantradsos: $(MANTRA_VRAY_SO_NAMES)

install_lib: houdinilib

install: houdinilib houdinisops mantradsos
	mkdir -p $(DESTDIR)/include/openvdb_houdini
	@echo "Created $(DESTDIR)/include/openvdb_houdini"
	mkdir -p $(DESTDIR)/include/houdini_utils
	@echo "Created $(DESTDIR)/include/houdini_utils"
	cp -f $(LIBHOUDINI_INCLUDE_NAMES) $(DESTDIR)/include/openvdb_houdini/
	@# Move headers from openvdb_houdini/ to houdini_utils/.
	mv -f $(DESTDIR)/include/openvdb_houdini/geometry.h \
        $(DESTDIR)/include/openvdb_houdini/ParmFactory.h \
	    $(DESTDIR)/include/openvdb_houdini/OP_NodeChain.h \
	    $(DESTDIR)/include/houdini_utils/
	@echo "Copied header files to $(DESTDIR)/include"
	@#
	mkdir -p $(DESTDIR_LIB_DIR)
	@echo "Created $(DESTDIR_LIB_DIR)"
	cp -f $(LIBHOUDINI) $(DESTDIR_LIB_DIR)
	pushd $(DESTDIR_LIB_DIR) > /dev/null; \
	    if [ -f $(LIBHOUDINI_SHARED) ]; then \
	        ln -f -s $(LIBHOUDINI_SHARED) $(LIBHOUDINI_NAME).so; fi; \
	    popd > /dev/null
	@echo "Copied libopenvdb_houdini to $(DESTDIR_LIB_DIR)"

install: install_lib houdinisops

# TODO: This accumulates all source file dependencies into a single file
# containing a rule for each *.o file.  Consider generating a separate
# dependency file for each *.o file instead.
$(DEPEND): $(ALL_INCLUDE_FILES) $(ALL_SRC_FILES)
	@echo "Generating dependencies because of $(call list_deps)"
	$(RM) $(DEPEND)
	for f in $(HOUDINI_SRC_FILES) $(MANTRA_SRC_FILES); \
	    do $(CXX) $(CXXFLAGS) $(HOUDINI_CXXFLAGS) -O0 \
	        -MM $$f -MT `echo $$f | sed 's%\.[^.]*%.o%'` \
	        -isystem $(HOUDINI_INCL_DIR) >> $(DEPEND); \
	done

depend: $(DEPEND)

# Compile an implicit translation unit for each header to identify any indirect includes
HEADER_TEST_FILES := $(addprefix header_test-,$(LIBHOUDINI_INCLUDE_NAMES))
$(HEADER_TEST_FILES): header_test-%: openvdb_houdini houdini_utils
	echo "#include \"$*\"" | $(CXX) -c -x c++ $(CXXFLAGS) $(HOUDINI_CXXFLAGS) \
	    -isystem $(HOUDINI_INCL_DIR) -fPIC -o /dev/null -
echo_header_test:
	@echo "Checking for missing or indirectly included headers"
header_test: echo_header_test $(HEADER_TEST_FILES)

clean:
	$(RM) $(ALL_TARGETS) $(DEPEND)
	$(RM) $(LIBHOUDINI_STATIC)
	$(RM) $(LIBHOUDINI_SHARED)
	$(RM) $(LIBHOUDINI_OBJ_NAMES)
	$(RM) $(HOUDINI_SOP_OBJ_NAMES) $(HOUDINI_SOP_SO_NAMES)
	$(RM) $(MANTRA_VRAY_OBJ_NAMES) $(MANTRA_VRAY_SO_NAMES)
	$(RM) $(HOUDINI_USER_PREF_DIR)/config/Icons/SOP_OpenVDB.svg
	if [ -n "$(HOUDINI_PYTHONRC_DESTDIR)" ]; \
	then \
	    $(RM) $(HOUDINI_PYTHONRC_DESTDIR)/pythonrc.py; \
	fi
	$(RM) $(addprefix $(HOUDINI_USER_PREF_DIR)/scripts/sop/,\
	    $(notdir $(HOUDINI_SOP_SCRIPT_NAMES)))

ifneq (,$(strip $(wildcard $(DEPEND))))
    include $(DEPEND)
endif
