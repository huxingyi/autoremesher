# Building and Installation

## Download

To build the latest release of OpenEXR, begin by downloading the
source from the Releases page
https://github.com/openexr/openexr/tarball/v2.3.0.

To build from the latest development version, which may not be stable,
download the master branch via
https://github.com/openexr/openexr/tarball/master, and extract the
contents via ``tar``.

You can download the repository tarball file either via a browser, or
on the Linux/macOS via the command line using ``wget`` or ``curl``:

    % curl -L https://github.com/openexr/openexr/tarball/master | tar xv

This will produce a source directory named
``openexr-openexr-<abbreviated-SHA-1-checksum>``.

Alternatively, clone the GitHub repo directly via:

    % git clone https://github.com/openexr/openexr.git

In the instructions that follow, we will refer to the top-level
directory of the source code tree as ``$source_directory``.

## Prerequisites

Make sure these are installed on your system before building OpenEXR:

* OpenEXR requires CMake version 3.10 or newer (or autoconf on Linux systems).
  - NB: CMake 3.12 is required for current PyIlmBase support
* C++ compiler that supports C++11
* Zlib
* Python and boost-python if building the PyIlmBase module.
  - NB: If you have a custom install of boost and have issues, you may
    need to set Boost_ROOT and/or manually disable Boost_NO_BOOST_CMAKE
    in the PyIlmBase cmake file when you run cmake. See the FindBoost
    documentation that is part of cmake for more information.

The instructions that follow describe building OpenEXR with CMake, but
you can also build and install OpenEXR via the autoconf
bootstrap/configure utilities, described below.

## Linux/macOS Quick Start

To build via CMake, first choose a location for the build directory,
which we will refer to as ``$build_directory``.

    % mkdir $build_directory
    % cd $build_directory
    % cmake $source_directory
    % make
    % make install

Note that the CMake configuration prefers to apply an out-of-tree
build process, since there may be multiple build configurations
(i.e. debug and release), one per folder, all pointing at once source
tree, hence the ``$build_directory`` noted above, referred to in CMake
parlance as the *build directory*. You can place this directory
wherever you like.

See the CMake Configuration Options section below for the most common
configuration options especially the install directory. Note that with
no arguments, as above, ``make install`` installs the header files in
``/usr/local/include``, the object libraries in ``/usr/local/lib``, and the
executable programs in ``/usr/local/bin``.

## Windows Quick Start

Under Windows, if you are using a command line-based setup, such as
cygwin, you can of course follow the above. For Visual Studio, cmake
generators are "multiple configuration", so you don't even have to set
the build type, although you will most likely need to specify the
install location.  Install Directory By default, ``make install``
installs the headers, libraries, and programs into ``/usr/local``, but you
can specify a local install directory to cmake via the
``CMAKE_INSTALL_PREFIX`` variable:

    % cmake .. -DCMAKE_INSTALL_PREFIX=$install_directory

## Library Names

Using either cmake or autoconf based configuration mechanisms described
in this document, by default the installed libraries follow a pattern
for how they are named. This is done to enable multiple versions of the
library to be installed and targeted by different builds depending on
the needs of the project. A simple example of this would be to have
different versions of the library installed to allow for applications
targeting different VFX Platform years to co-exist.

If you are building dynamic libraries, once you have configured, built,
and installed the libraries, you should see the following pattern of
symlinks and files in the install lib folder:

    libHalf.so -> libHalf-$LIB_SUFFIX.so
    libHalf-$LIB_SUFFIX.so -> libHalf-$LIB_SUFFIX.so.$SO_MAJOR_VERSION
    libHalf-$LIB_SUFFIX.so.$SO_MAJOR_VERSION -> libHalf-$LIB_SUFFIX.so.$SO_FULL_VERSION
    libHalf-$LIB_SUFFIX.so.$SO_FULL_VERSION (actual file)

You can configure the LIB_SUFFIX, although it defaults to the library
major and minor version, so in the case of a 2.3 library, it would default
to 2_3. You would then link your programs against this versioned library
to have maximum safety (i.e. `-lHalf-2_3`), and the pkg-config and cmake
configuration files included with find_package should set this up.

## Sub-Modules

OpenEXR consists of four separate sub-modules - IlmBase, PyIlmBase,
OpenEXR, OpenEXR_Viewers - which can be built independently. The
repository’s top-level CMakeLists.txt defines a *super-project* that
builds all four modules, and the steps above for running cmake at the
top level of the repo build each of the sub-modules, in parallel.

However you can build each submodule individually. To build and
install individual sub-modules, build and install the IlmBase module
first:

    % mkdir $build_directory
    % cd $build_directory
    % cmake -DCMAKE_INSTALL_PREFIX=$install_directory \ $source_directory/Ilmbase
    % cmake --build . --target install --config Release 

Once IlmBase is installed, then build and install the OpenEXR module,
taking care to set the ``CMAKE_SYSTEM_PREFIX`` to the directory in which
you installed IlmBase and ``CMAKE_INSTALL_PREFIX`` to the directory in
which to install OpenEXR:
 
    % mkdir $build_directory
    % cd $build_directory
    % cmake -DCMAKE_SYSTEM_PREFIX=$install_directory \ 
            -DCMAKE_INSTALL_PREFIX=$install_directory \ 
            $source_directory/OpenEXR
    % cmake --build . --target install --config Release

Optionally, then build and install PyIlmBase, OpenEXR_Viewers, and Contrib.

The libraries in IlmBase and OpenEXR follow the standard cmake setting
of ``BUILD_SHARED_LIBS`` to control whether to build static or shared
libraries. However, they each have separate controls over whether to
build both shared and static libraries as part of one configuration,
as well as other customization options.

## Custom Namespaces

If you are interested in controlling custom namespace declarations or
similar options, you are encouraged to look at the ``CMakeLists.txt``
infrastructure. In particular, there has been an attempt to centralize
the settings into a common place to more easily see all of them in a
text editor. For IlmBase, this is config/IlmBaseSetup.cmake inside the
IlmBase tree. For OpenEXR, the settings will similarly be found in
``config/OpenEXRSetup.cmake``. As per usual, these settings can also be
seen and/or edited using any of the various gui editors for working
with cmake such as ``ccmake``, ``cmake-gui``, as well as some of the
IDEs in common use.

## Cross Compiling / Specifying Specific Compilers

When trying to either cross-compile for a different platform, or for
tasks such as specifying a compiler set to match the VFX reference
platform (https://vfxplatform.com/), cmake provides the idea of a
toolchain which may be useful instead of having to remember a chain of
configuration options. It also means that platform-specific compiler
names and options are out of the main cmake file, providing better
isolation.

A toolchain file is simply just a cmake script that sets all the
compiler and related flags and is run very early in the configuration
step to be able to set all the compiler options and such for the
discovery that cmake performs automatically. These options can be set
on the command line still if that is clearer, but a theoretical
toolchain file for compiling for VFX Platform 2015 is provided in the
source tree at cmake/Toolchain-Linux-VFX_Platform15.cmake which will
hopefully provide a guide how this might work.

For cross-compiling for additional platforms, there is also an
included sample script in cmake/Toolchain-mingw.cmake which shows how
cross compiling from Linux for Windows may work. The compiler names
and paths may need to be changed for your environment.

More documentation:

* Toolchains: https://cmake.org/cmake/help/v3.12/manual/cmake-toolchains.7.html
* Cross compiling: https://gitlab.kitware.com/cmake/community/wikis/doc/cmake/

## CMake Configuration Options

The default CMake configuration options are stored in
``IlmBase/config/IlmBaseSetup.cmake`` and in
``OpenEXR/config/OpenEXRSetup.cmake``. To see a complete set of option
variables, run:

    % cmake -LAH $source_directory

You can customize these options three ways:

1. Modify the ``.cmake`` files in place.
2. Use the UI ``cmake-gui`` or ``ccmake``.
2. Specify them as command-line arguments when you invoke cmake.

### Verbose Output Options:

* **CMAKE\_EXPORT\_COMPILE\_COMMANDS**

  Enable/Disable output of compile commands during generation. Default is OFF.

* **CMAKE\_VERBOSE\_MAKEFILE**

  Echo all compile commands during make. Default is OFF.

### Compiler Options:

* **OPENEXR\_CXX\_STANDARD**

  C++ standard to compile against. This obeys the global ``CMAKE_CXX_STANDARD`` but doesn’t force the global setting to enable sub-project inclusion. Default is ``14``.

### Library Naming Options:

* **ILMBASE\_LIB\_SUFFIX**

  Append the given string to the end of all the IlmBase libraries. Default is ``-<major>_<minor>`` version string. Please see the section on library names

* **OPENEXR\_LIB\_SUFFIX**

  Append the given string to the end of all the OpenEXR libraries. Default is ``-<major>_<minor>`` version string. Please see the section on library names

### Namespace Options:

* **ILMBASE\_IEX\_NAMESPACE**

  Public namespace alias for Iex. Default is ``Iex``.

* **ILMBASE\_ILMTHREAD\_NAMESPACE**

  Public namespace alias for IlmThread. Default is ``IlmThread``.

* **ILMBASE\_IMATH\_NAMESPACE**
 
  Public namespace alias for Imath. Default is ``Imath``.

* **ILMBASE\_INTERNAL\_IEX\_NAMESPACE**
 
  Real namespace for Iex that will end up in compiled symbols. Default is ``Iex\_<major>\_<minor>``.

* **ILMBASE\_INTERNAL\_ILMTHREAD\_NAMESPACE**
 
  Real namespace for IlmThread that will end up in compiled symbols. Default is ``IlmThread\_<major>\_<minor>``.

* **ILMBASE\_INTERNAL\_IMATH\_NAMESPACE**
 
  Real namespace for Imath that will end up in compiled symbols. Default is ``Imath\_<major>\_<minor>``.

* **ILMBASE\_NAMESPACE\_CUSTOM**
 
  Whether the namespace has been customized (so external users know)

* **OPENEXR\_IMF\_NAMESPACE**
 
  Public namespace alias for Imath. Default is ``Imf``.

* **OPENEXR\_INTERNAL\_IMF\_NAMESPACE**
 
  Real namespace for Imath that will end up in compiled symbols. Default is ``Imf\_<major>\_<minor>``.

* **OPENEXR\_NAMESPACE\_CUSTOM**
 
  Whether the namespace has been customized (so external users know)

### Python Options:

* **PyIlmBase\_Python2\_SITEARCH\_REL**

  This will normally be computed based on where the python2 binary and site-packages live and
  then be a relative path based on the root of those. For example, if site-packages is in
  ``/usr/lib/python2.7/site-packages`` and the python binary is ``/usr/bin/python2.7``, this
  will result in the default install path being ``${CMAKE\_INSTALL\_PREFIX}/lib/python2.7/site-packages``

* **PyIlmBase\_Python3\_SITEARCH\_REL**

  Identical logic to PyIlmBase\_Python2\_SITEARCH\_REL path above, but for python 3.x

### Linting Options:

These linting options are experimental, and primarily for developer-only use at this time.

* **ILMBASE\_USE\_CLANG\_TIDY**
 
  Enable clang-tidy for IlmBase libraries, if it is available. Default is OFF.

* **OPENEXR\_USE\_CLANG\_TIDY**
 
  Enable clang-tidy for OpenEXR libraries, if it is available. Default is OFF.

### Testing Options:


* **BUILD\_TESTING**
 
  Build the testing tree. Default is ON.  Note that this causes the test suite to be compiled, but it is not executed.

* **OPENEXR\_RUN\_FUZZ\_TESTS**
 
  Controls whether to include the fuzz tests (very slow). Default is OFF.

### Additional CMake Options:

See the cmake documentation for more information (https://cmake.org/cmake/help/v3.12/)

* **CMAKE\_BUILD\_TYPE**

  For builds when not using a multi-configuration generator. Available values: ``Debug``, ``Release``, ``RelWithDebInfo``, ``MinSizeRel``

* **BUILD\_SHARED\_LIBS**

  This is the primary control whether to build static libraries or
  shared libraries / dlls (side note: technically a convention, hence
  not an official ``CMAKE\_`` variable, it is defined within cmake and
  used everywhere to control this static / shared behavior)

* For forcing particular compilers to match VFX platform requirements

  ** CMAKE\_CXX\_COMPILER**

  ** CMAKE\_C\_COMPILER**

  ** CMAKE\_LINKER**

     All the related cmake compiler flags (i.e. CMAKE\_CXX_FLAGS, CMAKE_CXX_FLAGS_DEBUG)

  ** CMAKE\_INSTALL\_RPATH**

     For non-standard install locations where you don’t want to have to set ``LD_LIBRARY_PATH`` to use them

## Cmake Tips and Tricks:

If you have ninja (https://ninja-build.org/) installed, it is faster than make. You can generate ninja files using cmake when doing the initial generation:

    % cmake -G “Ninja” ..

If you would like to confirm compile flags, you don’t have to specify the verbose configuration up front, you can instead run

    % make VERBOSE=1

## Configuring via Autoconf

As an alternative to CMake on Linux systems, the OpenEXR build can be configured via the provided bootstrap/configure scripts: 

    % cd $source_directory/IlmBase
    % ./bootstrap
    % ./configure --prefix=$install_directory
    % make
    % make install
    
    % cd $source_directory/OpenEXR
    % ./bootstrap
    % ./configure --prefix=$install_directory \ 
     --with-ilmbase-prefix=$install_directory
    % make 
    % make install
    
    % cd $source_directory/PyIlmBase
    % ./bootstrap
    % ./configure --prefix=$install_directory \ 
     --with-ilmbase-prefix=$install_directory
    % make 
    % make install

Run ``./configure --help`` for a complete set of configuration options.
