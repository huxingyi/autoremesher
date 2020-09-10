# OpenEXR Release Notes

* [Version 2.4.1](#version-241-february-11-2020)
* [Version 2.4.0](#version-240-september-19-2019)
* [Version 2.3.0](#version-230-august-13-2018)
* [Version 2.2.1](#version-221-november-30-2017)
* [Version 2.2.0](#version-220-august-10-2014)
* [Version 2.1.0](#version-210-november-25-2013)
* [Version 2.0.1](#version-201-july-11-2013)
* [Version 2.0.0](#version-200-april-9-2013)
* [Version 1.7.1](#version-171-july-31-2012)
* [Version 1.7.0](#version-170-july-23-2010)
* [Version 1.6.1](#version-161-october-22-2007)
* [Version 1.6.0](#version-160-august-3,2007)
* [Version 1.5.0](#version-150-december-15-2006)
* [Version 1.4.0a](#version-140a-august-9-2006)
* [Version 1.4.0](#version-140-august-2,2006)
* [Version 1.3.1](#version-131-june-14-2006)
* [Version 1.3.0](#version-130-june-8,2006)
* [Version 1.2.2](#version-122-march-15-2005)
* [Version 1.2.1](#version-121-june-6,2004)
* [Version 1.2.0](#version-120-may-11-2004)
* [Version 1.1.1](#version-111-march-27-2004)
* [Version 1.1.0](#version-110-february-6-2004)
* [Version 1.0.7](#version-107-january-7-2004)
* [Version 1.0.6](#version-106)
* [Version 1.0.5](#version-105-april-3-2003)
* [Version 1.0.4](#version-104)
* [Version 1.0.3](#version-103)
* [Version 1.0.2](#version-102)
* [Version 1.0.1](#version-101)
* [Version 1.0](#version-10)

## Version 2.4.1 (February 11, 2020)

Patch release with minor bug fixes.

### Summary

* Various fixes for memory leaks and invalid memory accesses
* Various fixes for integer overflow with large images.
* Various cmake fixes for build/install of python modules.
* ImfMisc.h is no longer installed, since it's a private header.

### Merged Pull Requests

* [659](https://github.com/AcademySoftwareFoundation/openexr/pull/659) fix memory leaks and invalid memory accesses

* [609](https://github.com/AcademySoftwareFoundation/openexr/pull/609) Fixes #593, others - issues with pyilmbase install 

* [605](https://github.com/AcademySoftwareFoundation/openexr/pull/605) No longer install ImfMisc.h 

* [603](https://github.com/openexr/openexr/pull/603) Update Azure build to work with new RB-2.4 branch. 

* [596](https://github.com/AcademySoftwareFoundation/openexr/pull/596) Add Boost::Python to Python modules link libraries

* [592](https://github.com/AcademySoftwareFoundation/openexr/pull/592) Take DESTDIR into account when creating library symlinks

* [589](https://github.com/openexr/openexr/pull/589) Fix int32 overflow bugs with deep images 

### Commits \[ git log v2.4.0...v2.4.1\]

* [fix memory leaks and invalid memory accesses](https://github.com/AcademySoftwareFoundation/openexr/commit/e79d2296496a50826a15c667bf92bdc5a05518b4) ([Peter Hillman](@peterh@wetafx.co.nz) 2020-02-08)

* [Fix overzealous removal of if statements breaking all builds except win32](https://github.com/openexr/openexr/commit/031199cd4fc062dd7bfe902c6552cf22f6bfbbdb) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-11-07)

* [Handle python2 not being installed, but python3 being present](https://github.com/openexr/openexr/commit/8228578da6f86d17b9a2a3f8c6053f8b4ee3fb71) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-11-07)

* [Fix issue with defines not being set correctly for win32](https://github.com/openexr/openexr/commit/d10895ef0ad25dd60e68a2ab00bab7c0592f8c5b) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-11-07)

* [Re-enable Boost_NO_BOOST_CMAKE by default, document, clean up status messages](https://github.com/openexr/openexr/commit/b303f6788a434fd61e52c1bacb93a96c4c3440ea) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-11-07)

* [Set CMP0074 such that people who set Boost_ROOT won't get warnings](https://github.com/openexr/openexr/commit/8ec1440cbd999f17457be605150bc53395fbb334) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-11-07)

* [ensure paths are canonicalized by get_filename_component prior to comparing](https://github.com/openexr/openexr/commit/28d1cb256f1b46f120adb131e606b2699acc72d7) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-11-07)

* [Fix issue with drive letter under windows](https://github.com/openexr/openexr/commit/34ce16c2653d02fcef6a297a2a61112dbf693922) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-11-06)

* [Extract to function, protect against infinite loop](https://github.com/openexr/openexr/commit/650da0d63410d863c4a0aed15a6bee1b46b559cb) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-11-06)

* [Fixes #593, others - issues with pyilmbase install](https://github.com/openexr/openexr/commit/df768ec8a97adb82947fc4b92a199db9a38c044c) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-11-05)

* [Take DESTDIR into account when creating library symlinks](https://github.com/openexr/openexr/commit/ed4807b9e4dc8d94ce79d0b2ed36acc548bee57e) ([Antonio Rojas](@arojas@archlinux.org) 2019-10-19)

* [No longer install ImfMisc.h](https://github.com/openexr/openexr/commit/f1b017c8029b529c5c5ed01b6ad1b10a0e48036c) ([Cary Phillips](@cary@ilm.com) 2019-10-31)

* [add boost to python module link library](https://github.com/openexr/openexr/commit/a571bdfe42866a1f1c579114e2fcae8318172c21) ([Jens Lindgren](@lindgren_jens@hotmail.com) 2019-10-22)

* [Update Azure build to work with new branch.](https://github.com/openexr/openexr/commit/4273e84f86fe27392dec53a5cef900caf6727154) ([Christina Tempelaar-Lietz](@xlietz@gmail.com) 2019-10-26)

* [Fix int32 overflow bugs with deep images](https://github.com/openexr/openexr/commit/e53ebd3ef677ab983f83f927f6525efcb5dcb995) ([Larry Gritz](@lg@larrygritz.com) 2019-10-17)

* [Prepare 2.4 release branch](https://github.com/openexr/openexr/commit/486ff10547d034530c5190bbef6181324b42c209) ([Larry Gritz](@lg@larrygritz.com) 2019-10-24)

## Version 2.4.0 (September 19, 2019)

### Summary

* Completely re-written CMake configuration files
* Improved support for building on Windows, via CMake
* Improved support for building on macOS, via CMake
* All code compiles without warnings on gcc, clang, msvc
* Cleanup of license and copyright notices
* floating-point exception handling is disabled by default
* New Slice::Make method to reliably compute base pointer for a slice.
* Miscellaneous bug fixes

### Security Vulnerabilities

This version fixes the following security vulnerabilities:

* [CVE-2018-18444](https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2018-18444) [Issue #351](https://github.com/openexr/openexr/issues/351) Out of Memory
* [CVE-2018-18443](https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2018-18443) [Issue #350](https://github.com/openexr/openexr/issues/350) heap-buffer-overflow
* [CVE-2017-12596](https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2017-12596) [Issue #238](https://github.com/openexr/openexr/issues/238) heap-based buffer overflow in exrmaketiled

### Closed Issues

* [529](https://github.com/openexr/openexr/issues/529) The OpenEXR_viewer can't be installed successfully due to the Cg support
* [511](https://github.com/openexr/openexr/issues/511) A confused problem in the EXR to JPEG
* [494](https://github.com/openexr/openexr/issues/494) SEGV exrmakepreview in ImfTiledOutputFile.cpp:458
* [493](https://github.com/openexr/openexr/issues/493) SEGV exrmakepreview in makePreview.cpp:132
* [491](https://github.com/openexr/openexr/issues/491) SEGV exrheader in ImfMultiPartInputFile.cpp:579
* [488](https://github.com/openexr/openexr/issues/488) Wiki has outdated info
* [462](https://github.com/openexr/openexr/issues/462) Inconsistent line terminators (CRLF)
* [461](https://github.com/openexr/openexr/issues/461) Wrong LC_RPATH after make install (cmake setup on macos)
* [457](https://github.com/openexr/openexr/issues/457) New CMake setup fails on cmake 3.12
* [455](https://github.com/openexr/openexr/issues/455) Build for mac using cmake to Xcode fails to compile
* [449](https://github.com/openexr/openexr/issues/449) OpenEXR.cpp:36:10: fatal error: 'ImathBox.h' file not found
* [424](https://github.com/openexr/openexr/issues/424) Integrating with OSS-Fuzz
* [421](https://github.com/openexr/openexr/issues/421) How to normalize multi-channel exr image?
* [400](https://github.com/openexr/openexr/issues/400) Create security@openexr.com and info@openexr.com addresses
* [398](https://github.com/openexr/openexr/issues/398) Document CVE's in CHANGES.md release notes file
* [396](https://github.com/openexr/openexr/issues/396) Set up a CREDITS.md file
* [395](https://github.com/openexr/openexr/issues/395) Migrate CLA's from openexr.com to the GitHub repo
* [394](https://github.com/openexr/openexr/issues/394) Properly document the OpenEXR coding style
* [393](https://github.com/openexr/openexr/issues/393) Set up CODEOWNERS file
* [389](https://github.com/openexr/openexr/issues/389) fix -Wall compiler warnings
* [388](https://github.com/openexr/openexr/issues/388) OpenEXR build fails with multiple errors
* [381](https://github.com/openexr/openexr/issues/381) Replace deprecated FindPythonLibs in CMakeLists.txt
* [380](https://github.com/openexr/openexr/issues/380) undefined symbol: _ZTIN7Iex_2_27BaseExcE
* [379](https://github.com/openexr/openexr/issues/379) ZLIB_LIBRARY ZLIB_INCLUDE_DIR being ignored (LNK2019 errors) in OpenEXR\IlmImf\IlmImf.vcxproj
* [377](https://github.com/openexr/openexr/issues/377) 2.3.0: test suite is failing
* [364](https://github.com/openexr/openexr/issues/364) Standalone build of openexr on windows - (with already installed ilmbase)
* [363](https://github.com/openexr/openexr/issues/363) `OpenEXRSettings.cmake` is missing from the release tarball
* [362](https://github.com/openexr/openexr/issues/362) Cmake macro `SET_ILMBASE_INCLUDE_DIRS` assumes
* [360](https://github.com/openexr/openexr/issues/360) Specified Boost.Python not found on Boost versions < 1.67
* [359](https://github.com/openexr/openexr/issues/359) [VS2015] Compile error C2782: 'ssize_t' in PyImathFixedMatrix
* [357](https://github.com/openexr/openexr/issues/357) Move ILMBASE_HAVE_CONTROL_REGISTER_SUPPORT to a private header
* [353](https://github.com/openexr/openexr/issues/353) Add --with-cg-libdir option to support arch dependant Cg library paths
* [352](https://github.com/openexr/openexr/issues/352) buffer-overflow
* [351](https://github.com/openexr/openexr/issues/351) Out of Memory 
* [350](https://github.com/openexr/openexr/issues/350) heap-buffer-overflow
* [348](https://github.com/openexr/openexr/issues/348) Possible compile/install issues in PyIlmBase with multiple jobs
* [343](https://github.com/openexr/openexr/issues/343) CMake issues on Windows
* [342](https://github.com/openexr/openexr/issues/342) IlmImf CMake dependency issue
* [340](https://github.com/openexr/openexr/issues/340) Cannot figure out how to build OpenEXR under mingw64 with v2.3.0
* [333](https://github.com/openexr/openexr/issues/333) openexr 2.3.0 static cmake build broken.
* [302](https://github.com/openexr/openexr/issues/302) Error when linking Half project: unresolved external symbol "private: static union half::uif const * const half::_toFloat" (?_toFloat@half@@0QBTuif@1@B)
* [301](https://github.com/openexr/openexr/issues/301) How to link different IlmBase library names according to Debug/Release configuration, when building OpenEXR with CMake + VS2015?
* [294](https://github.com/openexr/openexr/issues/294) Problem building OpenEXR-2.2.1 in Visual Studio 2015 x64
* [290](https://github.com/openexr/openexr/issues/290) Out Of Memory in Pxr24Compressor (79678745)
* [288](https://github.com/openexr/openexr/issues/288) Out of Memory in B44Compressor (79258415)
* [282](https://github.com/openexr/openexr/issues/282) IlmBase should link pthread
* [281](https://github.com/openexr/openexr/issues/281) Error in installing OpenEXR
* [276](https://github.com/openexr/openexr/issues/276) The savanah.nongnu.org tar.gz hosting
* [274](https://github.com/openexr/openexr/issues/274) Cmake installation of ilmbase places .dll files under `/lib` instead of `/bin`
* [271](https://github.com/openexr/openexr/issues/271) heap-buffer-overflow
* [270](https://github.com/openexr/openexr/issues/270) Out of Memory in TileOffsets (73566621)
* [268](https://github.com/openexr/openexr/issues/268) Invalid Shift at FastHufDecoder (72367575)
* [267](https://github.com/openexr/openexr/issues/267) Cast Overflow at FastHufDecoder (72375479)
* [266](https://github.com/openexr/openexr/issues/266) Divide by Zero at calculateNumTiles (72239767)
* [265](https://github.com/openexr/openexr/issues/265) Signed Integer Overflow in getTiledChunkOffsetTableSize (72377177)
* [264](https://github.com/openexr/openexr/issues/264) Signed Integer Overflow in calculateNumTiles (73181093)
* [263](https://github.com/openexr/openexr/issues/263) Signed Integer Overflow in chunkOffsetReconstruction (72873449, 73090589)
* [262](https://github.com/openexr/openexr/issues/262) Heap Out-of-Bounds write in Imf_2_2::copyIntoFrameBuffer (72940266)
* [261](https://github.com/openexr/openexr/issues/261) Heap Out of Bounds Read in TiledInputFile (72228841)
* [259](https://github.com/openexr/openexr/issues/259) Heap Out of Bounds Access (72839282)
* [257](https://github.com/openexr/openexr/issues/257) Out of Memory / Invalid allocation in lmfArray resizeErase (72828572, 72837441)
* [255](https://github.com/openexr/openexr/issues/255) Process for reporting security bugs
* [254](https://github.com/openexr/openexr/issues/254) [VS 2015] Can't run tests and OpenVDB compile errors
* [253](https://github.com/openexr/openexr/issues/253) C++11-style compile-time type information for `half`.
* [252](https://github.com/openexr/openexr/issues/252) `std::numeric_limits<half>::digits10` value is wrong.
* [250](https://github.com/openexr/openexr/issues/250) SO version change in 2.2.1
* [246](https://github.com/openexr/openexr/issues/246) half.h default user-provided constructor breaks c++ semantics (value/zero initialization vs default initialization)
* [244](https://github.com/openexr/openexr/issues/244) Cannot write to Z channel
* [240](https://github.com/openexr/openexr/issues/240) CpuId' was not declared in this scope
* [239](https://github.com/openexr/openexr/issues/239) pyilmbase error vs2015 with boost1.61 and python27 please help ，alse error
* [238](https://github.com/openexr/openexr/issues/238)  heap-based buffer overflow in exrmaketiled 
* [237](https://github.com/openexr/openexr/issues/237) Can RgbaOutputFile use 32-bit float?
* [234](https://github.com/openexr/openexr/issues/234) How to link compress2, uncompress and compress on 64 bit Windows 7 & Visual Studio 2015 when building openexr?
* [232](https://github.com/openexr/openexr/issues/232) Multiple segmentation faults CVE-2017-9110 to CVE-2017-9116
* [231](https://github.com/openexr/openexr/issues/231) Half.h stops OpenEXR from compiling
* [230](https://github.com/openexr/openexr/issues/230) Imf::OutputFile Produce binary different files
* [226](https://github.com/openexr/openexr/issues/226) IMathExc - multiple definitions on linking.
* [224](https://github.com/openexr/openexr/issues/224) Make PyIlmBase compatible with Python 3.x
* [217](https://github.com/openexr/openexr/issues/217) Issue with optimized build compiled with Intel C/C++ compiler (ICC)
* [213](https://github.com/openexr/openexr/issues/213) AddressSanitizer CHECK failed in ImageMagick fuzz test.  
* [208](https://github.com/openexr/openexr/issues/208) build issues on OSX: ImfDwaCompressorSimd.h:483:no such instruction: `vmovaps (%rsi), %ymm0'
* [205](https://github.com/openexr/openexr/issues/205) Building with VS 2015
* [202](https://github.com/openexr/openexr/issues/202) Documentation error: File Layout "Verson Field" lists wrong bits
* [199](https://github.com/openexr/openexr/issues/199) Unexpected rpaths on macOS
* [194](https://github.com/openexr/openexr/issues/194) RLE Broken for 32-bit formats
* [191](https://github.com/openexr/openexr/issues/191) PyIlmBase Cmake unable to find Boost
* [189](https://github.com/openexr/openexr/issues/189) store to misaligned address / for type 'int64_t', which requires 8 byte alignment
* [188](https://github.com/openexr/openexr/issues/188) iex_debugTrap link error
* [182](https://github.com/openexr/openexr/issues/182) Many C4275 warning compiling on Windows
* [176](https://github.com/openexr/openexr/issues/176) Implement a canonical FindIlmbase.cmake
* [166](https://github.com/openexr/openexr/issues/166) CMake static build of OpenEXR 2.2 fails to link dwaLookups on Linux
* [165](https://github.com/openexr/openexr/issues/165) Clang compilation warnings
* [164](https://github.com/openexr/openexr/issues/164) OpenEXR.pc is not created during "configure" stage.
* [163](https://github.com/openexr/openexr/issues/163) Problems building the OpenEXR-2.2.0
* [160](https://github.com/openexr/openexr/issues/160) Visual Studio 2013 not linking properly with IlmThread
* [158](https://github.com/openexr/openexr/issues/158) Python3 support
* [150](https://github.com/openexr/openexr/issues/150) build issue, debian 7.0 x64
* [139](https://github.com/openexr/openexr/issues/139) configure scripts contain bashisms
* [134](https://github.com/openexr/openexr/issues/134) DWA compressor fails to compile on Win/Mac for some compiler versions
* [132](https://github.com/openexr/openexr/issues/132) Wrong namespaces used in DWA Compressor.
* [125](https://github.com/openexr/openexr/issues/125) cmake: cannot link against static ilmbase libraries
* [123](https://github.com/openexr/openexr/issues/123) cmake: allow building of static and dynamic libs at the same time
* [105](https://github.com/openexr/openexr/issues/105) Building pyilmbase 1.0.0 issues
* [098](https://github.com/openexr/openexr/issues/98) Race condition in creation of LockedTypeMap and registerAttributeTypes 
* [095](https://github.com/openexr/openexr/issues/95) Compile fail with MinGW-w64 on Windows
* [094](https://github.com/openexr/openexr/issues/94) CMake does not generate "toFloat.h" with Ninja
* [092](https://github.com/openexr/openexr/issues/92) MultiPartOutputFile API fails when single part has no type
* [089](https://github.com/openexr/openexr/issues/89) gcc 4.8 compilation issues
* [086](https://github.com/openexr/openexr/issues/86) VS 2010 broken: exporting std::string subclass crashes
* [079](https://github.com/openexr/openexr/issues/79) compile openexr with mingw 64 bit
* [067](https://github.com/openexr/openexr/issues/67) testBox failure on i386
* [050](https://github.com/openexr/openexr/issues/50) Recommended way of opening an EXR file in python?
* [015](https://github.com/openexr/openexr/issues/15) IlmImf Thread should report an 'optimal' number ofthreads to use.

### Merged Pull Requests

* [541](https://github.com/openexr/openexr/pull/541) TSC meeting notes Aug 22, 2019
* [540](https://github.com/openexr/openexr/pull/540) Fix exports when compiling DLLs enabled with mingw
* [539](https://github.com/openexr/openexr/pull/539) Force exception handling / unwind disposition under msvc
* [538](https://github.com/openexr/openexr/pull/538) Add option to control whether pyimath uses the fp exception mechanism
* [537](https://github.com/openexr/openexr/pull/537) Set default value for buildSharedLibs
* [536](https://github.com/openexr/openexr/pull/536) Force the python binding libraries to shared
* [535](https://github.com/openexr/openexr/pull/535) Fix cmake warnings, fix check for numpy
* [534](https://github.com/openexr/openexr/pull/534) Create a "holder" object to fix stale reference to array
* [533](https://github.com/openexr/openexr/pull/533) Disable the debug postfix for the python modules
* [532](https://github.com/openexr/openexr/pull/532) explicitly add the boost includes to the target
* [531](https://github.com/openexr/openexr/pull/531) Update license for DreamWorks Lossy Compression
* [530](https://github.com/openexr/openexr/pull/530) Azure updates for MacOS/Windows/Linux
* [528](https://github.com/openexr/openexr/pull/528) brief notes of TSC meeting 2019-08-16
* [526](https://github.com/openexr/openexr/pull/526) Fix compile warnings from the latest merges
* [525](https://github.com/openexr/openexr/pull/525) Rework boost python search logic to be simpler and more robust
* [524](https://github.com/openexr/openexr/pull/524) Fix #268, issue with right shift in fast huf decoder
* [523](https://github.com/openexr/openexr/pull/523) Address issues with mingw and win32 wide filenames
* [522](https://github.com/openexr/openexr/pull/522) 2.4.0 release notes
* [520](https://github.com/openexr/openexr/pull/520) Add missing symbol export to Slice::Make
* [519](https://github.com/openexr/openexr/pull/519) TSC meeting notes August 8, 2019
* [518](https://github.com/openexr/openexr/pull/518) Makes building of fuzz test optional
* [517](https://github.com/openexr/openexr/pull/517) Added defines for DWAA and DWAB compression.
* [516](https://github.com/openexr/openexr/pull/516) changed AP_CPPFLAGS to AM_CPPFLAGS in PyImathNumpy/Makefile.am.
* [515](https://github.com/openexr/openexr/pull/515) add the files generated by bootstrap/configure to .gitignore.
* [514](https://github.com/openexr/openexr/pull/514) suppress SonarCloud warnings about unhandled exceptions
* [512](https://github.com/openexr/openexr/pull/512) Project documentation edits
* [510](https://github.com/openexr/openexr/pull/510) Added MacOS jobs to Azure pipeline
* [509](https://github.com/openexr/openexr/pull/509) Contrib cleanup
* [503](https://github.com/openexr/openexr/pull/503) TSC meeting notes from 7/25/2019
* [501](https://github.com/openexr/openexr/pull/501) license and copyright fixes
* [500](https://github.com/openexr/openexr/pull/500) Fix another set of warnings that crept in during previous fix merges
* [498](https://github.com/openexr/openexr/pull/498) Fix #491, issue with part number range check reconstructing chunk off…
* [497](https://github.com/openexr/openexr/pull/497) Fix logic for 1 pixel high/wide preview images (Fixes #493)
* [495](https://github.com/openexr/openexr/pull/495) Fix for #494: validate tile coordinates when doing copyPixels
* [490](https://github.com/openexr/openexr/pull/490) Normalize library naming between cmake and autoconf
* [489](https://github.com/openexr/openexr/pull/489) Refresh of README's
* [487](https://github.com/openexr/openexr/pull/487) Azure: updated docker containers, added windows install scripts.
* [486](https://github.com/openexr/openexr/pull/486) Fix #246, add type traits check
* [483](https://github.com/openexr/openexr/pull/483) Large dataWindow Offset test: for discussion
* [482](https://github.com/openexr/openexr/pull/482) Update Azure Linux/SonarCloud jobs to work with new build
* [481](https://github.com/openexr/openexr/pull/481) rewrite of build and installation documentation in INSTALL.md
* [480](https://github.com/openexr/openexr/pull/480) Put all runtime artefacts in a single folder to help win32 find dlls
* [479](https://github.com/openexr/openexr/pull/479) Fix compile warnings
* [478](https://github.com/openexr/openexr/pull/478) Fixes #353, support for overriding Cg libdir
* [477](https://github.com/openexr/openexr/pull/477) Fix #224, imath python code such that tests pass under python3
* [476](https://github.com/openexr/openexr/pull/476) Fix dos files to unix, part of #462
* [475](https://github.com/openexr/openexr/pull/475) Fixes #252, incorrect math computing half digits
* [474](https://github.com/openexr/openexr/pull/474) Fixes #139
* [473](https://github.com/openexr/openexr/pull/473) Fix missing #include <cmath> for std::isnormal
* [472](https://github.com/openexr/openexr/pull/472) Add viewers library to default build
* [471](https://github.com/openexr/openexr/pull/471) Warn the user, but make PyIlmBase not fail a build by default
* [470](https://github.com/openexr/openexr/pull/470) Fix #352, issue with aspect ratio
* [468](https://github.com/openexr/openexr/pull/468) Fix #455 by not using object libraries under apple
* [467](https://github.com/openexr/openexr/pull/467) NumPy lookup logic is only in newer versions of cmake than our minimum
* [466](https://github.com/openexr/openexr/pull/466) Remove last vestiges of old ifdef for windows
* [465](https://github.com/openexr/openexr/pull/465) Fix #461, issue with macos rpath support
* [463](https://github.com/openexr/openexr/pull/463) Fix #457, (unused) policy tag only in 3.13+ of cmake, no longer needed
* [460](https://github.com/openexr/openexr/pull/460) TSC meeting notes 7/18/2019
* [459](https://github.com/openexr/openexr/pull/459) added missing copyright notices
* [458](https://github.com/openexr/openexr/pull/458) fix for failing PyIlmBase/configure because it can't run the IlmBase test program.
* [456](https://github.com/openexr/openexr/pull/456) fix incorrect license identifier
* [450](https://github.com/openexr/openexr/pull/450) change INCLUDES to AM_CPPFLAGS, upon the recommendation of automake warnings
* [448](https://github.com/openexr/openexr/pull/448) Fixes #95, compilation issue with mingw
* [447](https://github.com/openexr/openexr/pull/447) Implements #15, request for hardware concurrency utility function
* [446](https://github.com/openexr/openexr/pull/446) Fixes #282, missing link against pthread
* [444](https://github.com/openexr/openexr/pull/444) added missing files in autoconf setup
* [443](https://github.com/openexr/openexr/pull/443) don't index empty array in testMultiPartSharedAttributes
* [442](https://github.com/openexr/openexr/pull/442) TiledInputFile only supports regular TILEDIMAGE types, not DEEPTILE...
* [441](https://github.com/openexr/openexr/pull/441) TSC meeting notes, July 7, 2019
* [440](https://github.com/openexr/openexr/pull/440) security policy
* [439](https://github.com/openexr/openexr/pull/439) code of conduct
* [438](https://github.com/openexr/openexr/pull/438) Azure and SonarCloud setup
* [437](https://github.com/openexr/openexr/pull/437) address #271: catch scanlines with negative sizes
* [436](https://github.com/openexr/openexr/pull/436) specific check for bad size field in header attributes (related to #248)
* [435](https://github.com/openexr/openexr/pull/435) Refactor cmake
* [434](https://github.com/openexr/openexr/pull/434) Issue #262
* [433](https://github.com/openexr/openexr/pull/433) Fix for #263: prevent overflow in multipart chunk offset reconstruction
* [432](https://github.com/openexr/openexr/pull/432) Fix for #378, bswap on read on big-endian architectures
* [431](https://github.com/openexr/openexr/pull/431) Fixed column labels in OpenEXRFileLayout document
* [429](https://github.com/openexr/openexr/pull/429) change OpaqueAttribute's _typeName field to be std::string
* [428](https://github.com/openexr/openexr/pull/428) Added Coding Style section on Type Casting.
* [427](https://github.com/openexr/openexr/pull/427) adding source .odt files for the .pdf's on the documentation page
* [425](https://github.com/openexr/openexr/pull/425) Handle exceptions, per SonarCloud rules
* [423](https://github.com/openexr/openexr/pull/423) Address #270: limit Tiled images to INT_MAX total number of tiles
* [422](https://github.com/openexr/openexr/pull/422) Add exr2aces to autoconf build script
* [420](https://github.com/openexr/openexr/pull/420) updated references to CVE's in release notes.
* [417](https://github.com/openexr/openexr/pull/417) TSC meeting notes June 27, 2019
* [416](https://github.com/openexr/openexr/pull/416) Fix #342, copy paste bug with dependencies
* [415](https://github.com/openexr/openexr/pull/415) convert_index returns Py_ssize_t
* [414](https://github.com/openexr/openexr/pull/414) Fix part of #232, issue with pointer overflows
* [413](https://github.com/openexr/openexr/pull/413) Fix library suffix issue in cmake file for exr2aces
* [412](https://github.com/openexr/openexr/pull/412) Fix #350 - memory leak on exit
* [411](https://github.com/openexr/openexr/pull/411) Fixes the rpath setting to have the correct variable name
* [410](https://github.com/openexr/openexr/pull/410) Fixed the 2.3.0 release notes to mention that CVE-2017-12596 is fixed.
* [409](https://github.com/openexr/openexr/pull/409) Add initial rules for running clang-format on the code base
* [408](https://github.com/openexr/openexr/pull/408) Add ImfFloatVectorAttribute.h to the automake install
* [406](https://github.com/openexr/openexr/pull/406) New CI with aswfstaging/ci-base image
* [405](https://github.com/openexr/openexr/pull/405) June 20, 2019 TSC meeting notes
* [404](https://github.com/openexr/openexr/pull/404) Miscellaneous documentation improvements
* [403](https://github.com/openexr/openexr/pull/403) Added CLA forms
* [402](https://github.com/openexr/openexr/pull/402) TSC Meeting notes June 13, 2019
* [397](https://github.com/openexr/openexr/pull/397) Updates to README.md, and initial CONTRIBUTING.md, GOVERNANCE.md, INSTALL.md
* [383](https://github.com/openexr/openexr/pull/383) Fixed formatting
* [382](https://github.com/openexr/openexr/pull/382) TSC meeting notes 2019-5-2
* [339](https://github.com/openexr/openexr/pull/339) fix standalone and combined cmake

### Commits \[ git log v2.3.0...v2.4.0\]

* [Add missing include](https://github.com/openexr/openexr/commit/cd1b068ab1d2e2b40cb81c79e997fecfe31dfa11) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-08-24)

* [Add option to control whether pyimath uses the fp exception mechanism](https://github.com/openexr/openexr/commit/be0df7b76106ba4b33efca289641fdeb59adb3a2) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-08-24)

* [Update license for DreamWorks Lossy Compression](https://github.com/openexr/openexr/commit/5b64c63cef71f4542ef4e2452077f62755b66252) ([jbradley](@jbradley@dreamworks.com) 2019-08-19)

* [Added defines for DWAA and DWAB compression.](https://github.com/openexr/openexr/commit/1b88251b8d955124d7a5da9716ec287ef78440e5) ([Dirk Lemstra](@dirk@lemstra.org) 2019-08-08)

* [TSC meeting notes Aug 22, 2019](https://github.com/openexr/openexr/commit/9307279963b44d31152441bbe771de044329f356) ([Cary Phillips](@cary@ilm.com) 2019-08-26)

* [2.4.0 release notes * Added commit history * Added table of contents Signed-off-by: Cary Phillips <cary@ilm.com>](https://github.com/openexr/openexr/commit/9fe66510bb5c353bb855b6a5bdbb6be8d3762778) ([Cary Phillips](@cary@ilm.com) 2019-08-10)

* [Fix vtable insertion for win32, use new macro everywhere](https://github.com/openexr/openexr/commit/54d46dacb88fbfa41608c7e347cffa5552742bc4) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-08-25)

* [Use unique id, not typeid reference which may differ](https://github.com/openexr/openexr/commit/728c26ccbd9f0700633c89c94b8328ee78f40cec) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-08-25)

* [Force vtable into a translation unit](https://github.com/openexr/openexr/commit/7678a9d09c45cc9ae2b9f591f3565d10a503aadd) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-08-25)

* [Fix exports when compiling DLLs enabled with mingw](https://github.com/openexr/openexr/commit/3674dd27ce45c1f2cc11993957dccee4bdd840dd) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-08-25)

* [Force exception handling / unwind disposition under msvc](https://github.com/openexr/openexr/commit/b4d5d867a49029e93b4b3aa6708d1fc0093613cc) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-08-25)

* [Force the python binding libraries to shared](https://github.com/openexr/openexr/commit/39c17b9ceef2ec05b1ebd25a9ee3f15e5fe17181) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-08-24)

* [Fix cmake warnings, fix check for numpy](https://github.com/openexr/openexr/commit/85bde2ea9afbddffc6ffbfa597f8bb1d25b42859) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-08-24)

* [Remove unused typedef from previous failed attempt at boost python usage](https://github.com/openexr/openexr/commit/6d5b23a258b562c29012953e13d67012a66322f0) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-08-24)

* [Create a "holder" object to fix stale reference to array](https://github.com/openexr/openexr/commit/d2a9dec4d37143feb3b9daeb646b9e93632c5d8a) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-08-24)

* [Disable the debug postfix for the python modules](https://github.com/openexr/openexr/commit/311ebb0485a253445c7324b3d42eaadd01ceb8b4) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-08-24)

* [explicitly add the boost includes to the target as Boost::headers does not seem to](https://github.com/openexr/openexr/commit/bdedcc6361da71e7512f978d4017a1fbb25ace92) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-08-24)

* [Set default value for buildSharedLibs](https://github.com/openexr/openexr/commit/62427d2dc3d3ee147e01e6d0e3b2119f37dfa689) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-08-24)

* [Azure updates for MacOS/Windows/Linux](https://github.com/openexr/openexr/commit/3a49e9fe3f3d586a57d25265335752380cbe1b31) ([Christina Tempelaar-Lietz](@xlietz@gmail.com) 2019-08-18)

* [brief notes of TSC meeting 2019-08-16](https://github.com/openexr/openexr/commit/36fb144da1110232bf416d5e1c4abde263056d17) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-08-16)

* [Fix compile warnings from the latest merges](https://github.com/openexr/openexr/commit/181add33e9391372e76abb6bfc654f37d3788e4a) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-08-14)

* [Fix boost checks when a versioned python is not found](https://github.com/openexr/openexr/commit/d6c176718595415e7b17e7a6c77af0df75cc36de) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-08-14)

* [Rework boost python search logic to be simpler and more robust](https://github.com/openexr/openexr/commit/c21272230b30562d219d41d00cdcbc98be602c37) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-08-14)

* [Fix spacing](https://github.com/openexr/openexr/commit/4f8137070fa257557f7b474c41b9b9c260b7f3cd) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-08-14)

* [Fix #268, issue with right shift in fast huf decoder](https://github.com/openexr/openexr/commit/2f33f0ff08cf66286fda5cf60ee6f995821bde0d) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-08-14)

* [Add mechanism for test programs to use win32 wide filename fix when manually creating std::fstreams](https://github.com/openexr/openexr/commit/e0ac10e045b6d932c221c9223d88940b14e12b8b) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-08-12)

* [Use temp directory for tests under win32, properly cleanup files from util tests](https://github.com/openexr/openexr/commit/1d0b240557a230cf704c8797f97ce373a3ca5474) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-08-12)

* [Fix issue with mingw gcc and wide / utf8 filenames](https://github.com/openexr/openexr/commit/02fbde4e1942e2ffcf652eb99e32fb15530cc93d) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-08-12)

* [Remove unused using statements](https://github.com/openexr/openexr/commit/ce09ee004050ec2c1c0fff72b28d1d69a98dfaea) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-08-12)

* [Add missing exports for ImfAcesFile](https://github.com/openexr/openexr/commit/631d5d49bab5ef0194983a0e15471102b5acacd9) ([Nick Porcino](@meshula@hotmail.com) 2019-08-10)

* [Add missing symbol export to Slice::Make](https://github.com/openexr/openexr/commit/efb5d10f6001e165149bf0dc17f96b4671d213c3) ([Nick Porcino](@meshula@hotmail.com) 2019-08-09)

* [TSC meeting notes August 8, 2019](https://github.com/openexr/openexr/commit/ee8830f108e7a930f6326175f444ed026e504f27) ([Cary Phillips](@cary@ilm.com) 2019-08-08) Signed-off-by: Cary Phillips <cary@ilm.com>

* [changed AP_CPPFLAGS to AM_CPPFLAGS in PyImathNumpy/Makefile.am.](https://github.com/openexr/openexr/commit/859017261d4401ebdb965f268d88b10455984719) ([Cary Phillips](@cary@ilm.com) 2019-08-07) What this a typo? The automake-generated Makefiles expect 'AM', which
was leading to a failure to find PyImath.h. Signed-off-by: Cary Phillips <cary@ilm.com>

* [Removed the d_exr Renderman plugin from Contrib. It was hopelessly outdated, not updated since 2003, and no longer of benefit.](https://github.com/openexr/openexr/commit/6999eb39465d99d5fbb01eff9f1acfdb424d9f82) ([Cary Phillips](@cary@ilm.com) 2019-07-27) Signed-off-by: Cary Phillips <cary@ilm.com>

* [Removed the Photoshop plugin from Contrib. It was hopelessly outdated and no longer of benefit.](https://github.com/openexr/openexr/commit/e84040bde6259777035b3032337aee4a24f34548) ([Cary Phillips](@cary@ilm.com) 2019-07-27) Signed-off-by: Cary Phillips <cary@ilm.com>

* [added SPDX license identifier.](https://github.com/openexr/openexr/commit/e9e4f34616460b3a3c179a7bcc2be2e8f4e79ae8) ([Cary Phillips](@cary@ilm.com) 2019-07-27) Signed-off-by: Cary Phillips <cary@ilm.com>

* [Upon the request of the ASWF Governing Board and the advice of Pixar/Lucasfilm attorney Richard Guo, changed the license on the DtexToExr source code to BSD-3-Clause, to bring in line with the standard OpenEXR license. Also, removed COPYING, as it only contained license info; remoted INSTALL because it was only a copy of the boilerplate bootstrap/config documentation; remove NEWS because we're not using that file any more.](https://github.com/openexr/openexr/commit/a73956bfd4809769bcb8fe2229f7d888c7deccff) ([Cary Phillips](@cary@ilm.com) 2019-07-27) Signed-off-by: Cary Phillips <cary@ilm.com>

* [TSC meeting notes from 7/25/2019](https://github.com/openexr/openexr/commit/2ebd7ade2f392fc3da50c0227e3ff11a7a2f4d8e) ([Cary Phillips](@cary@ilm.com) 2019-07-26) Signed-off-by: Cary Phillips <cary@ilm.com>

* [Match variable style syntax per Cary](https://github.com/openexr/openexr/commit/f5ab8176637d8ea1decc83929950aa3864c87141) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-08-10) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Add headers to build so programs that can parse and display that will do so](https://github.com/openexr/openexr/commit/19557bfaf1b6b38a2407a6a261ee8f3b376c0bd6) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-25) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [First pass of describing versioning and naming of library names](https://github.com/openexr/openexr/commit/eeae20a72f596589b6429ba43bff69281b801015) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-25) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Normalize library naming between cmake and autoconf](https://github.com/openexr/openexr/commit/c3ebd44bdb64c5bfe0065f3d0ac898387a0fbb63) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-25) installed libraries should follow the following basic pattern: (-> indicates a symlink)

    libFoo.so -> libFoo-LIB_SUFFIX.so
    libFoo-LIB_SUFFIX.so -> libFoo-LIB_SUFFIX.so.MAJ_SO_VERSION
    libFoo-LIB_SUFFIX.so.MAJ_SO_VERSION ->
    libFoo-LIB_SUFFIX.so.FULL_SO_VERSION

    so with a concrete example of 2.3 lib w/ so version of 24

    libFoo.so -> libFoo-2_3.so
    libFoo-2_3.so -> libFoo-2_3.so.24
    libFoo-2_3.so.24 -> libFoo-2_3.so.24.0.0
    libFoo-2_3.so.24.0.0.0 <--- actual file

    (there may be slight variations in the link destinations based on
    differences in libtool and cmake, but the file names available should
    all be there) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [only perform check in c++14 to avoid old c++11 standards deficient compilers](https://github.com/openexr/openexr/commit/1aeba79984bef35cead1da540550441f2b8244af) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-25) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Fix #246, add type traits check](https://github.com/openexr/openexr/commit/5323c345361dcf01d012fd8f40e8c6c975b9cb83) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-23) previous cleanup did most of the work, but add an explicit test that
half is now trivial and default constructible.  Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [remove sanityCheck for 32 bit overflow. Add test for large offsets](https://github.com/openexr/openexr/commit/b0acdd7bcbd006ff93972cc3c6d66c617280c557) ([Peter Hillman](@peterh@wetafx.co.nz) 2019-07-23) 

* [Makes building of fuzz test optional](https://github.com/openexr/openexr/commit/73d5676079d77b4241719f57d0219a3287503b8b) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-08-09) This further makes the fuzz test compilation dependent on whether you
want to include the fuzz test in the ctest "make test" rule. This is
mostly for sonar cloud such that it doesn't complain that the fuzz test
code isn't being run as a false positive (because it isn't included in
the test) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Added MacOS jobs to Azure pipeline](https://github.com/openexr/openexr/commit/29eab92cdee9130b7d1cc6adb801966d0bc87c94) ([Christina Tempelaar-Lietz](@xlietz@gmail.com) 2019-07-27) 

* [initial draft of release notes for 2.3.1](https://github.com/openexr/openexr/commit/4fa4251dc1cce417a7832478f6d05421561e2fd2) ([Cary Phillips](@cary@ilm.com) 2019-08-06) Signed-off-by: Cary Phillips <cary@ilm.com>

* [Add //NOSONAR to the "unhandled exception" catches that SonarCloud identifies as vulnerabilities, to suppress the warning. In each of these cases, a comment explains that no action is called for in the catch, so it should not, in fact, be regarded as a bug or vulnerability.](https://github.com/openexr/openexr/commit/c46428acaca50e824403403ebdaec45b97d92bca) ([Cary Phillips](@cary@ilm.com) 2019-07-28) Signed-off-by: Cary Phillips <cary@ilm.com>

* [explicitly name the path for the autoconf-generated files in .gitignore.](https://github.com/openexr/openexr/commit/220cfcdd7e08d28098bf13c992d48df4b0ab191d) ([Cary Phillips](@cary@ilm.com) 2019-08-04) 

* [add the file generated by bootstrap/configure to .gitignore.](https://github.com/openexr/openexr/commit/81af15fd5ea58c33cfa18c60797daaba55126c1b) ([Cary Phillips](@cary@ilm.com) 2019-08-04) Signed-off-by: Cary Phillips <cary@ilm.com>

* [Fixes #353, support for overriding Cg libdir](https://github.com/openexr/openexr/commit/63924fd0f47e428b63c82579e8b03a1eeb4e4ca1) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-22) to handle systems where it isn't lib, but lib64, as needed
Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [more documentation tweaks](https://github.com/openexr/openexr/commit/b6c006aafc500816e42909491437bf9af79bb03c) ([Cary Phillips](@cary@ilm.com) 2019-07-28) Signed-off-by: Cary Phillips <cary@ilm.com>

* [Updates to README, CONTRIBUTING, GOVERNANCE: better introduction, removed some of the TSC process descriptions that are redudant in the charter.](https://github.com/openexr/openexr/commit/1cd03756bbf22a65f84eb42c9d83b78be2902c02) ([Cary Phillips](@cary@ilm.com) 2019-07-28) Signed-off-by: Cary Phillips <cary@ilm.com>

* [update to the template copyright notice.](https://github.com/openexr/openexr/commit/21c307aaf054f304f52bb488258f81d68e38385f) ([Cary Phillips](@cary@ilm.com) 2019-07-25) Signed-off-by: Cary Phillips <cary@ilm.com>

* [Updates to LICENSE and CONTRIBUTORS.](https://github.com/openexr/openexr/commit/559186e6c638190ec1db122ec5f1a0890c056a16) ([Cary Phillips](@cary@ilm.com) 2019-07-25) Signed-off-by: Cary Phillips <cary@ilm.com>

* [Fix another set of warnings that crept in during previous fix merges](https://github.com/openexr/openexr/commit/e07ef34af508b7ce9115ebc5454edeaacb35fb8c) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-25) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Fix logic for 1 pixel high/wide preview images (Fixes #493)](https://github.com/openexr/openexr/commit/74504503cff86e986bac441213c403b0ba28d58f) ([Peter Hillman](@peterh@wetafx.co.nz) 2019-07-25) 

* [Fix for #494: validate tile coordinates when doing copyPixels](https://github.com/openexr/openexr/commit/6bb36714528a9563dd3b92720c5063a1284b86f8) ([Peter Hillman](@peterh@wetafx.co.nz) 2019-07-25) 

* [add test for filled channels in DeepScanlines](https://github.com/openexr/openexr/commit/c04673810a86ba050d809da42339aeb7129fc910) ([Peter Hillman](@peterh@wetafx.co.nz) 2019-07-18) 

* [add test for skipped and filled channels in DeepTiles](https://github.com/openexr/openexr/commit/b1a5c8ca1921a3fc573952c8034fddd8fdac214b) ([Peter Hillman](@peterh@wetafx.co.nz) 2019-07-18) 

* [slightly rearrange test for filled channels](https://github.com/openexr/openexr/commit/3c9d0b244ec31ab5e5849e1b6020c55096707ab5) ([Peter Hillman](@peterh@wetafx.co.nz) 2019-07-18) 

* [Make sure to skip over slices that will only be filled when computing the uncompressed pixel size. Otherwise chunks that compressed to larger sizes than the original will fail to load.](https://github.com/openexr/openexr/commit/14905ee6d802b27752890d39880cd05338337e39) ([Halfdan Ingvarsson](@halfdan@sidefx.com) 2013-04-25) 

* [Fix #491, issue with part number range check reconstructing chunk offset table](https://github.com/openexr/openexr/commit/8b5370c688a7362673c3a5256d93695617a4cd9a) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-25) The chunk offset was incorrectly testing for a part number that was the
same size (i.e. an invalid index)
Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [removed logo, that didn't work.](https://github.com/openexr/openexr/commit/d5800c14296527b3540da7aefd28b5937158d2cc) ([Cary Phillips](@cary@ilm.com) 2019-07-23) Signed-off-by: Cary Phillips <cary@ilm.com>

* [added logo](https://github.com/openexr/openexr/commit/70435d286a0fe1a022ba26f00a1fd6eb37505a32) ([Cary Phillips](@cary@ilm.com) 2019-07-23) Signed-off-by: Cary Phillips <cary@ilm.com>

* [OpenEXR logo](https://github.com/openexr/openexr/commit/d6eeb1432bc626709f934da7428561d4aeb8c5a5) ([Cary Phillips](@cary@ilm.com) 2019-07-23) Signed-off-by: Cary Phillips <cary@ilm.com>

* [smaller window image](https://github.com/openexr/openexr/commit/fcedcad366988a24fb9c756510488f8fb83dc2ac) ([Cary Phillips](@cary@ilm.com) 2019-07-23) Signed-off-by: Cary Phillips <cary@ilm.com>

* [fixed image references in README.md](https://github.com/openexr/openexr/commit/6def338579442d0fe1e3fbed0d458db3c5cf2a42) ([Cary Phillips](@cary@ilm.com) 2019-07-23) Signed-off-by: Cary Phillips <cary@ilm.com>

* [Revised the overview information in README.md, and condensed the information in the module README.md's, and removed the local AUTHORS, NEWS, ChangeLog files.](https://github.com/openexr/openexr/commit/0c04c734d1a7ba3f3f85577ec56388238c9202c6) ([Cary Phillips](@cary@ilm.com) 2019-07-23) Signed-off-by: Cary Phillips <cary@ilm.com>

* [Azure: updated docker containers, added windows install scripts.](https://github.com/openexr/openexr/commit/941082379a49a1aecafe2b9e84f3403314d910a9) ([Christina Tempelaar-Lietz](@xlietz@gmail.com) 2019-07-22) 

* [rewrite of build and installation documentation in INSTALL.md](https://github.com/openexr/openexr/commit/591b671ba549bccca1e41ad457f569107242565d) ([Cary Phillips](@cary@ilm.com) 2019-07-22) Signed-off-by: Cary Phillips <cary@ilm.com>

* [Convert constructor casts to static_cast](https://github.com/openexr/openexr/commit/625b95fa026c3b78e537e9bb6a39fcd51920ad13) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-23) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Convert constructor casts to static_cast, remove dead code](https://github.com/openexr/openexr/commit/5cbf3cb368cd7013a119c3f08555a69fe33a932b) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-23) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Fix issues and warnings compiling in optimized using gcc -Wall](https://github.com/openexr/openexr/commit/6d4e118cebbb7adf8ed29d846bb6f7fb0fb198eb) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-23) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Ensure tests have assert when building in a release mode](https://github.com/openexr/openexr/commit/fe93c2c1ade319a7bc9a733cbeaad3c625a31d0d) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-23) Fixes warnings and makes sure tests are ... testing
Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Cleanup warnings for clang -Wall](https://github.com/openexr/openexr/commit/a5fbf7d669ca6b2b402f4fdf9022b43e5eea616f) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-23) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [First pass of warning cleanup](https://github.com/openexr/openexr/commit/c1501ec2b29c95501c8fc324f4ec91bd93f0c1d3) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-23) This fixes g++ -Wall to compile warning free
Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Update Azure Linux/SonarCloud jobs to work with new build](https://github.com/openexr/openexr/commit/b19c8d221976bc6c0debc77431b0fe40dfeb8887) ([¨Christina Tempelaar-Lietz¨](@xlietz@gmail.com) 2019-07-21) Signed-off-by: Christina Tempelaar-Lietz <xlietz@gmail.com>

* [Fix dos files to unix, part of #462](https://github.com/openexr/openexr/commit/0f97a86349b377e0f380d2782326844bef652820) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-22) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Put all runtime artefacts in a single folder to help win32 find dlls](https://github.com/openexr/openexr/commit/e2e8b53e267c373971f3e6da700670679a46403d) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-23) This will (hopefully) fix issues with compiling ilmbase as a dll and
using that to generate and compile openexr
Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Fix #224, imath python code such that tests pass under python3](https://github.com/openexr/openexr/commit/ab50d774e91a6448443e6cdb303bd040105cfaf8) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-22) Previously had fixed print functions, this pass converts the following:
- integer division changed in python3 3/2 -> 1.5, have to use 3//2 to
get an int
- xrange is no more, just use range
- integer type coersion for division not working, force type constructor
Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Fixes #252, incorrect math computing half digits](https://github.com/openexr/openexr/commit/bca0bc002b222d64712b748a733d9c9a0701f834) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-22) Based on float / double math for base 10 digits, with 1 bit of rounding
error, the equation should be floor( mantissa_digits - 1 ) * log10(2) ),
which in the case of half becomes floor( 10 * log10(2) ) or 3
Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Fixes #139](https://github.com/openexr/openexr/commit/ba329cba788d4f320e6fc455919233222c27a0dd) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-21) Removes bash-isms from the autoconf bootstrap / configure.ac files
Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Add viewers library to default build](https://github.com/openexr/openexr/commit/f52164dcc92c98775c3503aa9827fbd5d1e69b63) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-21) If libraries can't be found, will warn and not build
Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Warn the user, but make PyIlmBase not fail a build by default](https://github.com/openexr/openexr/commit/a0dcd35c51fc7811bc17b766ded17622f91e3fd0) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-21) By default, many people won't have the dependencies to build PyIlmBase.
Make it such that the build will warn, but continue to build without the
python extension
Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Fix missing #include <cmath> for std::isnormal](https://github.com/openexr/openexr/commit/9aa10cfac3209ac398b12c14eec2611420f20985) ([Axel Waggershauser](@awagger@gmail.com) 2019-07-21) fixes compile regression on macos + clang-6

* [further cleanup and remove old mworks checks that had been copied around](https://github.com/openexr/openexr/commit/351ad1897e3b84bd5b1e29835c7e68bb09f1f914) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-21) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Remove last vestiges of old ifdef for windows previously removed elsewhere](https://github.com/openexr/openexr/commit/b3651854491afa8b6c98e9078a5f4a33178c1a66) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-21) Previously PLATFORM_WINDOWS was used to conditionally include things,
but that had been removed elsewhere, and a few spots missed.
Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Fix #352, issue with aspect ratio](https://github.com/openexr/openexr/commit/34e2e78f205c49eafb49b7589701746f748194ad) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-21) If a file is contructed with an abnormal aspect ratio, tools like make
preview will fail. This adds an extra check to the creation / reading of
ImfHeader to avoid this issue
Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Fix #455 by not using object libraries under apple](https://github.com/openexr/openexr/commit/0451df8f7986ff5ab37c26d2aa6a7aeb115c8948) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-21) Per the docs, add_library calls with only object library dependencies
are not yet handled properly by Xcode and similar. Disable the use of
object libraries as a compilation speedup mechanism as a result.
Similarly, disable under win32 when building both types of libs to avoid
exported symbols in the static libs. Finally, use same mechanism to
avoid extra layer of libs in generated exports when only building one
config on all platforms
Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [NumPy lookup logic is only in newer versions of cmake than our minimum](https://github.com/openexr/openexr/commit/5b4b23d1cf49ee89132251bc7987d65b7a11efe6) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-21) We are doing the numpy lookup manually for now
Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Fix #461, issue with macos rpath support, remove half-baked framework support](https://github.com/openexr/openexr/commit/9aa52c8c0c96b24c8d645d7850dae77f4bf64620) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-21) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Refactor origin function to a Slice factory and Rgba custom utility](https://github.com/openexr/openexr/commit/119eb2d4672e5c77a79929758f7e4c566f47c794) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-21) Instead of a general templated routine, have a Slice factory function
and then a custom Rgba utility function to clarify and avoid missing
strides, etc. when dealing with slices
Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [merges common fixes and move bounds check to central location](https://github.com/openexr/openexr/commit/6a41400b47d574a5fc6133b9a7139bcd7b59d585) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-01) PR #401 had conflicts, and some of the checks were not in a central
location. This incorporates those changes, moving the extra range checks
to the central sanityCheck already in ImfHeader. Then adds a new utility
function for computing the pointer offsets that can prevent simple
overflow when there are large offsets from origin or widths with
subsampling.
Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>
Co-Authored-By: pgajdos <pgajdos@suse.cz>

* [Fix part of #232, issue with pointer overflows](https://github.com/openexr/openexr/commit/4aa6a4e0fcd52b220c71807307b9139966c3644c) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-06-27) This addresses pointer overflow in exr2aces with large datawindow
offsets. It also fixes similar issues in exrenvmap and exrmakepreview.
This addresses the crashes in CVE-2017-9111, CVE-2017-9113,
CVE-2017-9115
Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Fix portion of #462](https://github.com/openexr/openexr/commit/2309b42be084939e8593e036b814049f98eb7888) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-21) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Fix copyright notice, clarify version requirement comment](https://github.com/openexr/openexr/commit/688b50d1982854b1a2be63160eae03472cf4820e) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-20) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Fix copyright notice, clarify version requirement comment](https://github.com/openexr/openexr/commit/bbf1f5ed9814f35f953c5b28349ca8dd59a3ed87) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-20) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Fix #457, (unused) policy tag only in 3.13+ of cmake, no longer needed](https://github.com/openexr/openexr/commit/e69dc2131791a42d5e0618506a4846ec7d53b997) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-20) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [TSC meeting notes 7/18/2019](https://github.com/openexr/openexr/commit/04e21585d01c36790dad186a34c4c64c8e0a1dae) ([Cary Phillips](@cary@ilm.com) 2019-07-18) Signed-off-by: Cary Phillips <cary@ilm.com>

* [Typo in Makefile.am, AM_CPPFLAGS should append to the previous value.](https://github.com/openexr/openexr/commit/97626390f86007fcff2d33c68919389e211983e1) ([Cary Phillips](@cary@ilm.com) 2019-07-18) Signed-off-by: Cary Phillips <cary@ilm.com>

* [changed INCLUDE to AM_CPPFLAGS, upon the recommendation of automake warnings.](https://github.com/openexr/openexr/commit/f91edef414e319235959a537e0ef62c49dddcde3) ([Cary Phillips](@cary@ilm.com) 2019-07-17) Signed-off-by: Cary Phillips <cary@ilm.com>

* [added missing copyright notices](https://github.com/openexr/openexr/commit/76cb1ef869a23ab49f4313fee16a4d5750e91485) ([Cary Phillips](@cary@ilm.com) 2019-07-18) Signed-off-by: Cary Phillips <cary@ilm.com>

* [in PyIlmBase/configure.ac, set LD_LIBRARY_PATH explicitly for the ilmbase test program,so that it finds the libraries when it executes.](https://github.com/openexr/openexr/commit/0bd322d424781f20750141ddc829fc9e16f7e305) ([Cary Phillips](@cary@ilm.com) 2019-07-18) Signed-off-by: Cary Phillips <cary@ilm.com>

* [remove the reference to the LICENSE file in the copyright notice template.](https://github.com/openexr/openexr/commit/1aedb3ceec973e9bc0bad88fc151b2504884e84c) ([Cary Phillips](@cary@ilm.com) 2019-07-18) Signed-off-by: Cary Phillips <cary@ilm.com>

* [fix incorrect license identifier](https://github.com/openexr/openexr/commit/02f1e3d876a784cfd0ab8d0581bafe1fd0d98df2) ([Cary Phillips](@cary@ilm.com) 2019-07-18) Signed-off-by: Cary Phillips <cary@ilm.com>

* [Rename new function and clarify purpose](https://github.com/openexr/openexr/commit/e8dc4326383540ef4a4e2a388cb176da72c120fb) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-18) After discussion with phillman, renamed to give this routine a purpose
beyond some soon to be deleted legacy support, and clarified this in the
comment documenting the function.
Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Implements #15, request for hardware concurrency utility function](https://github.com/openexr/openexr/commit/23eaf0f45ff531ba0ab3fb1540d5c7d31b4bfe94) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-18) new static member of ThreadPool, call as
ThreadPool::hardwareConcurrency, so no abi breakage or api change
Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [use headers.data() instead of &headers[0]](https://github.com/openexr/openexr/commit/42665b55f4062f1492156c7bc9482318c7b49cda) ([Peter Hillman](@peterh@wetafx.co.nz) 2019-07-17) 

* [don't index empty array in testMultiPartSharedAttributes](https://github.com/openexr/openexr/commit/bb5aad9b793b1113cae42d80fea8925503607de1) ([Peter Hillman](@peterh@wetafx.co.nz) 2019-07-16) 

* [Added IlmThreadSemaphoreOSX to IlmBase/IlmThread/Makefile.am and added PyIlmBase/PyIlmBase.pc.in back in, looks like it got inadvertently removed by a previous commit.](https://github.com/openexr/openexr/commit/c580d3531c36ed1de35fbfe359eed5f74c2de6dc) ([Cary Phillips](@cary@ilm.com) 2019-07-16) Signed-off-by: Cary Phillips <cary@ilm.com>

* [Azure and SonarCloud setup](https://github.com/openexr/openexr/commit/9d053e4871e721144ad25ac04437646cf4f16d66) ([¨Christina Tempelaar-Lietz¨](@xlietz@gmail.com) 2019-07-12) Signed-off-by: ¨Christina Tempelaar-Lietz¨ <xlietz@gmail.com>

* [Fixes #95, compilation issue with mingw](https://github.com/openexr/openexr/commit/2cf0560dd8eb469680d2281e6d80348dad9ad500) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-18) The tree now compiles using mingw to compile, tested by cross compiling
for windows from linux
Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Fixes #282, missing link against pthread](https://github.com/openexr/openexr/commit/e90f1b0ed19cb05821c7351ce8d5d9a22fb094eb) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-18) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Remove removed file, add CMakeLists.txt file](https://github.com/openexr/openexr/commit/9683c48479ed2372d26eb51ed91d89b01c495dfd) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-18) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [PyIlmBase finished refactor, misc cleanup](https://github.com/openexr/openexr/commit/4d97270c6ce0916483c1aff5b1f77846cfff11a0) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-18) - add extra dist to automake for make dist
- finish numpy lookup
- add sample vfx 15 toolchain file for doc purposes
- merge cxx standard, pay attention to global setting if set
- merge clang tidy option
- add default build type if not set
Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Remove un-needed files now that cmake can provide correct values](https://github.com/openexr/openexr/commit/08332041bb46b45e93855c9843a2aa916ec4ebef) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-18) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Fix issues with rpath, message cleanup, checkpoint better python layer](https://github.com/openexr/openexr/commit/0eff97241f495027021b54978028475f0b2459dd) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-17) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Start to integrate python module using FindPython and FindBoost from modern cmake](https://github.com/openexr/openexr/commit/c236ed81b7146947999b75fd93aedc5d54d78f64) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-16) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Attempt to set rpath for more common scenarios when people are building custom versions](https://github.com/openexr/openexr/commit/10adf360120898c6ad3a0be2838056948bf22233) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-16) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Documentation pass](https://github.com/openexr/openexr/commit/ba22a8e0a366c87677c53bab72af72dbc378b0dd) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-16) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Enable custom install subfolder for headers](https://github.com/openexr/openexr/commit/9067b792c6f178bd2ff1d15e7b4d898fc1677495) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-13) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Generate version file to ensure proper version check](https://github.com/openexr/openexr/commit/edb6938738462009990086fb7081a860412ec0d4) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-13) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Properly include additional cmake files in "make dist" under autoconf](https://github.com/openexr/openexr/commit/ae54f3d656f8c6336c22385ee5d5ab1f35324c37) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-13) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [First pass updating the documentation for cmake builds](https://github.com/openexr/openexr/commit/120b93ecf33c45284dff68eaf0ee779fa1cb6747) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-12) 

* [Switch testing control to use standard ctest setting option](https://github.com/openexr/openexr/commit/fe6bf4c585723ff8851dfe965343a2adb0f1c1f4) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-12) 

* [First pass making cross compile work, cross compiling windows using mingw on linux](https://github.com/openexr/openexr/commit/f44721e0c504b0b400a71513600295fc5e00f014) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-12) This currently works for building using static libraries, but not yet
tested with dlls.
Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Fix new (WIP) cmake setup to work on OS/X](https://github.com/openexr/openexr/commit/2fe5a26d7ef36276ba4aa354178b81fc6612868d) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-12) This includes a fix for the semaphore configure check as well as a
couple of compile warnings
Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Add missing file, remove unused exrbuild tool cmake](https://github.com/openexr/openexr/commit/9a1ca7579b1ac793ae2d7bbee667e498d9bc8322) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-12) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Refactor cmake](https://github.com/openexr/openexr/commit/df41027db50bd52a0b797444f02d5907b756652e) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-07-12) This refactors the cmake setup, modernizing it to a current flavor of
cmake and cleaning up the definitions. This also makes the top level
folder a "super project", meaning it is including what should be
distinct / standalone sub-projects with their own finds that should
work.
Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [TiledInputFile only supports regular TILEDIMAGE types, not DEEPTILE or unknown tiled types. Enforce for both InputFile and InputPart API. Fixes #266, Related to #70](https://github.com/openexr/openexr/commit/ece555214a63aaf0917ad9df26be7e17451fefb9) ([Peter Hillman](@peterh@wetafx.co.nz) 2019-07-15) 

* [address #271: catch scanlines with negative sizes](https://github.com/openexr/openexr/commit/849c616e0c96665559341451a08fe730534d3cec) ([Peter Hillman](@peterh@wetafx.co.nz) 2019-07-12) 

* [TSC meeting notes, July 7, 2019](https://github.com/openexr/openexr/commit/960a56f58da13be6c97c59eae1f57bd8882c4588) ([Cary Phillips](@cary@ilm.com) 2019-07-12) Signed-off-by: Cary Phillips <cary@ilm.com>

* [securty policy](https://github.com/openexr/openexr/commit/8f483c2552070f3d9dd2df98f6500dfa1c051dcc) ([Cary Phillips](@cary@ilm.com) 2019-07-12) Signed-off-by: Cary Phillips <cary@ilm.com>

* [code of conduct](https://github.com/openexr/openexr/commit/f31407518aa361263c77eae13f1eef46999ca01f) ([Cary Phillips](@cary@ilm.com) 2019-07-12) Signed-off-by: Cary Phillips <cary@ilm.com>

* [bswap_32 to correct endianness on read, to address #81.](https://github.com/openexr/openexr/commit/225ddb8777e75978b88c2d6311bb0cf94c0b6f22) ([Cary Phillips](@cary@ilm.com) 2019-07-02) Signed-off-by: Cary Phillips <cary@ilm.com>

* [fix reading files](https://github.com/openexr/openexr/commit/5350d10ffc03c774e5cd574062297fc91001064d) ([Dan Horák](@dan@danny.cz) 2019-04-15) testFutureProofing and testMultiPartFileMixingBasic both use fread(&length,4,f) to get a 4 byte
integer value from input file. The value read is not converted from the little endian format to
the machine format causing problems (eg. test didn't finish after 24 hours).
fixes issue #81

* [SonarCloud considers strcpy() a vulernability. It was used only in OpaqueAttribute, whose type name was stored as Array<char>.  I changed the type to std::string. I suspect this simply dates to a time before std::string was commonly used.](https://github.com/openexr/openexr/commit/29d18b70bf542ef9ec6e8861c015d2e7b3d3ec58) ([Cary Phillips](@cary@ilm.com) 2019-07-09) Also, it appears that nothing in the test suite validated opaque attributes, which hold values read from a file when the attribute type is not known. I added a test to validate the behavior, which also validates that the typeName() works when implemented with std::string instead of Array<char>.
Signed-off-by: Cary Phillips <cary@ilm.com>

* [Updated pdf with fixes for file version bits on page 7.](https://github.com/openexr/openexr/commit/8da36708caaf0591f72538bfa414d8af20af90e9) ([Cary Phillips](@cary@ilm.com) 2019-07-11) Signed-off-by: Cary Phillips <cary@ilm.com>

* [Fixed column labels in table on page 7; bit 11 is "deep", bit 12 is "multi-part". Bit 9 is long names, and is not in the table.](https://github.com/openexr/openexr/commit/a3198419f7593564747337e763083492c0470f45) ([Cary Phillips](@cary@ilm.com) 2019-07-09) Signed-off-by: Cary Phillips <cary@ilm.com>

* [New CI with aswfstaging/ci-base image](https://github.com/openexr/openexr/commit/5e7cde5c082881009516aa57a711a19e3eb92f64) ([aloysb](@aloysb@al.com.au) 2019-06-17) Signed-off-by: Aloys Baillet <aloys.baillet@gmail.com>
Conflicts:
	azure-pipelines.yml

* [use static_cast in error test](https://github.com/openexr/openexr/commit/700e4996ce619743d5bebe07b4158ccc4547e9ad) ([Peter Hillman](@peterh@wetafx.co.nz) 2019-07-11) 

* [throw better exceptions in multipart chunk reconstruction](https://github.com/openexr/openexr/commit/001a852cca078c23d98c6a550c65268cc160042a) ([Peter Hillman](@peterh@wetafx.co.nz) 2019-07-11) 

* [Fix for #263: prevent overflow in multipart chunk offset table reconstruction](https://github.com/openexr/openexr/commit/6e4b6ac0b5223f6e813e025532b3f0fc4e02f541) ([Peter Hillman](@peterh@wetafx.co.nz) 2019-07-09) 

* [protect against negative sized tiles](https://github.com/openexr/openexr/commit/395aa4cbcaf91ce37aeb5e9876c44291bed4d1f9) ([Peter Hillman](@peterh@wetafx.co.nz) 2019-07-11) 

* [apply suggested for for #262](https://github.com/openexr/openexr/commit/9e9e4616f60891a8b27ee9cdeac930e5686dca4f) ([Peter Hillman](@peterh@wetafx.co.nz) 2019-07-10) 

* [specific check for bad size field in header attributes (related to #248)](https://github.com/openexr/openexr/commit/4c146c50e952655bc193567224c2a081c7da5e98) ([Peter Hillman](@peterh@wetafx.co.nz) 2019-07-12) 

* [use static_cast and numeric_limits as suggested](https://github.com/openexr/openexr/commit/eda733c5880e226873116ba66ce9069dbc844bdd) ([Peter Hillman](@peterh@wetafx.co.nz) 2019-07-09) 

* [Address #270: limit to INT_MAX tiles total](https://github.com/openexr/openexr/commit/7f438ffac4f6feb46383f66cb7e83ab41074943d) ([Peter Hillman](@peterh@wetafx.co.nz) 2019-07-05) 

* [exr2aces wasn't built via the configure script](https://github.com/openexr/openexr/commit/1959f74ee7f47948038a1ecb16c8ba8b84d4eb89) ([Peter Hillman](@peterh@wetafx.co.nz) 2019-07-05) 

* [added links for CVE's](https://github.com/openexr/openexr/commit/afd9beac8b7e114def78793b6810cbad8764a477) ([Cary Phillips](@cary@ilm.com) 2019-07-02) Signed-off-by: Cary Phillips <cary@ilm.com>

* [added "Test Policy" section to CONTRIBUTING.](https://github.com/openexr/openexr/commit/695019e4b98b55ed583d1455a9219e55fc777d1a) ([Cary Phillips](@cary@ilm.com) 2019-07-02) Signed-off-by: Cary Phillips <cary@ilm.com>

* [updated references to CVE's in release notes.](https://github.com/openexr/openexr/commit/2a0226b4c99c057ab7f3b038dafd92543ade3e6f) ([Cary Phillips](@cary@ilm.com) 2019-07-02) Signed-off-by: Cary Phillips <cary@ilm.com>

* [Fixed the 2.3.0 release notes to mention that CVE-2017-12596 is fixed.](https://github.com/openexr/openexr/commit/9da28302194b413b57da757ab69eb33373407f51) ([Cary Phillips](@cary@ilm.com) 2019-06-26) Signed-off-by: Cary Phillips <cary@ilm.com>

* [Added Coding Style section on Type Casting.](https://github.com/openexr/openexr/commit/7790ad78bb4e2b6f4bf22a7c1703af1e352004a4) ([Cary Phillips](@cary@ilm.com) 2019-07-08) Signed-off-by: Cary Phillips <cary@ilm.com>

* [adding source .odt files for the .pdf's on the documention page on openexr.com](https://github.com/openexr/openexr/commit/2f7847e3faf7146f2be8c1c0c3053c50b7ee9d97) ([Cary Phillips](@cary@ilm.com) 2019-07-03) Signed-off-by: Cary Phillips <cary@ilm.com>

* [fix readme typo](https://github.com/openexr/openexr/commit/67c1d4d2fc62f1bbc94202e49e65bd92de2e580f) ([Nick Porcino](@meshula@hotmail.com) 2019-07-08) 

* [Handle exceptions, per SonarCloud rules; all catch blocks must do something to indicate the exception isn't ignored.](https://github.com/openexr/openexr/commit/fbce9002eff631b3feeeb18d45419c1fba4204ea) ([Cary Phillips](@cary@ilm.com) 2019-07-07) Signed-off-by: Cary Phillips <cary@ilm.com>

* [TSC meeting notes June 27, 2019](https://github.com/openexr/openexr/commit/4093d0fbb16ad687779ec6cc7b44308596d5579f) ([Cary Phillips](@cary@ilm.com) 2019-06-28) Signed-off-by: Cary Phillips <cary@ilm.com>

* [Implement semaphore for osx](https://github.com/openexr/openexr/commit/fbb912c3c8b13a9581ffde445e390c1603bae35d) ([oleksii.vorobiov](@oleksii.vorobiov@globallogic.com) 2018-11-01) 

* [Various fixes to address compiler warnings: - removed unused variables and functions - added default cases to switch statements - member initialization order in class constructors - lots of signed/unsigned comparisons fixed either by changing a loop iterator from int to size_t, or by selective type casting.](https://github.com/openexr/openexr/commit/c8a7f6a5ebce9a6d5bd9a3320bc746221789f407) ([Cary Phillips](@cary@ilm.com) 2019-06-24) Signed-off-by: Cary Phillips <cary@ilm.com>

* [convert_index returns Py_ssize_t](https://github.com/openexr/openexr/commit/ce886b87336ba04a12eb631ecfcc71da0c9b74bf) ([Cary Phillips](@cary@ilm.com) 2019-06-27) Signed-off-by: Cary Phillips <cary@ilm.com>

* [Fix #342, copy paste bug with dependencies](https://github.com/openexr/openexr/commit/2b28d90bc5e329c989dc44c1d5fdcdf715d225d7) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-06-28) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Fixes the rpath setting to have the correct variable name](https://github.com/openexr/openexr/commit/5093aaa05278030d07304588fa52466538794fe7) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-06-27) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Add ImfFloatVectorAttribute.h to the automake install](https://github.com/openexr/openexr/commit/d61c0967cb7cd8fa255de64e4e79894d59c0f82d) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-06-26) The CMake file was previously updated to include this file on install,
but was missing from the automake side.
Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Fix #350 - memory leak on exit](https://github.com/openexr/openexr/commit/adbc1900cb9d25fcc4df008d4008b781cf2fa4f8) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-06-27) This fixes CVE-2018-18443, the last thread pool provider set into the
pool was not being correctly cleaned up at shutdown of the thread pool.
Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Fix library suffix issue in cmake file for exr2aces](https://github.com/openexr/openexr/commit/e4099a673e3348d4836c79a760e07b28b1912083) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-06-27) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Iterate on formatting, add script to run the formatting](https://github.com/openexr/openexr/commit/969305c5731aef054e170e776086e3747eb20ee0) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-06-27) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [Add initial rules for running clang-format on the code base](https://github.com/openexr/openexr/commit/6513fcf2e25ebd92c8f80f18e8cd7718ba7c4a41) ([Kimball Thurston](@kdt3rd@gmail.com) 2019-06-27) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [find Boost.Python 3 on older Boost versions](https://github.com/openexr/openexr/commit/9b58cf0fc197947dc5798854de639233bb35c6cb) ([Jens Lindgren](@lindgren_jens@hotmail.com) 2018-11-19) 

* [MSYS support](https://github.com/openexr/openexr/commit/a19c806a7b52cdf74bfa6966b720efd8b24a2590) ([Harry Mallon](@hjmallon@gmail.com) 2019-01-30) 

* [Only find_package ZLIB when required](https://github.com/openexr/openexr/commit/ab357b0a7a6d7e0ee761bf8ee5846688626d9236) ([Harry Mallon](@hjmallon@gmail.com) 2019-02-06) 

* [Remove unused headers](https://github.com/openexr/openexr/commit/db9fcdc9c448a9f0d0da78010492398a394c87e7) ([Grant Kim](@6302240+enpinion@users.noreply.github.com) 2019-06-13) 

* [WIN32 to _WIN32 for Compiler portability](https://github.com/openexr/openexr/commit/6e2a73ed8721da899a5bd844397444d5b15a5c71) ([Grant Kim](@6302240+enpinion@users.noreply.github.com) 2019-06-11) https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros?view=vs-2019
_WIN32 is the standard according to the official documentation from Microsoft and also this fixes MinGW compile error.

* [Update README.md](https://github.com/openexr/openexr/commit/45e9910be6009ac4ddf4db51c3c505daafc942a3) ([Huibean Luo](@huibean.luo@gmail.com) 2019-04-08) 

* [Added a few people to CREDITS.](https://github.com/openexr/openexr/commit/db512f5de8f4cc0f6ff81a67bf1bb7e8e7f0cc53) ([Cary Phillips](@cary@ilm.com) 2019-06-20) Signed-off-by: Cary Phillips <cary@ilm.com>

* [added release note summary information for all old releases from the "Announcements" section of openexr.com to CHANGES.md, so the repo's release notes are complete.](https://github.com/openexr/openexr/commit/61bbd0df59494cc2fa0e508506f32526acf2bf51) ([Cary Phillips](@cary@ilm.com) 2019-06-20) Signed-off-by: Cary Phillips <cary@ilm.com>

* [first real draft of coding style, and steps in the release process.](https://github.com/openexr/openexr/commit/1d514e66313cac0440b80c290b35cfa6b8f89b51) ([Cary Phillips](@cary@ilm.com) 2019-06-20) Signed-off-by: Cary Phillips <cary@ilm.com>

* [- added CREDITS.md (generated from "git log") - added CODEOWNERS (mostly a placeholder, everything is currently owned by TSC members) - the Release Process section of CONTRIBUTING gives the git log arguments to generate release notes. - remove stray meeting minutes file at the root level.](https://github.com/openexr/openexr/commit/050048c72ef4c32119d21cdb499e23418429f529) ([Cary Phillips](@cary@ilm.com) 2019-06-19) Signed-off-by: Cary Phillips <cary@ilm.com>

* [fixed references to renamed ASWF folder](https://github.com/openexr/openexr/commit/bd4c36cf07db310bb8350a4e5f575d86f1c7f8cb) ([Cary Phillips](@cary@ilm.com) 2019-06-19) Signed-off-by: Cary Phillips <cary@ilm.com>

* [June 20, 2019 TSC meeting notes](https://github.com/openexr/openexr/commit/82134840a001c2692ee762b0a767ab1b43cb64db) ([Cary Phillips](@cary@ilm.com) 2019-06-20) Signed-off-by: Cary Phillips <cary@ilm.com>

* [CLA's Renamed aswf-tsc to ASWF](https://github.com/openexr/openexr/commit/7ebb766d7540ae9a2caea80b9f1c9799d7c8d8af) ([Cary Phillips](@cary@ilm.com) 2019-06-15) Signed-off-by: Cary Phillips <cary@ilm.com>

* [2019-06-13.md](https://github.com/openexr/openexr/commit/9b2719c68635879421805ed3b602ea19aae68a77) ([seabeepea](@seabeepea@gmail.com) 2019-06-14) Signed-off-by: seabeepea <seabeepea@gmail.com>

* [Missed John on the attendee list.](https://github.com/openexr/openexr/commit/0035649cc6d7f4d86be8609758b927b01b8c110c) ([Cary Phillips](@cary@ilm.com) 2019-06-13) Signed-off-by: Cary Phillips <cary@ilm.com>

* [TSC Meeting notes June 13, 2019](https://github.com/openexr/openexr/commit/79857214aec3d81f73f2e9613a4b44caa21751c8) ([Cary Phillips](@cary@ilm.com) 2019-06-13) Signed-off-by: Cary Phillips <cary@ilm.com>

* [- Formatting section is TBD - fixed references to license - removed references to CI - added section on GitHub labels](https://github.com/openexr/openexr/commit/0045a12d20112b253895d88b4e2bce3ffcff0d90) ([Cary Phillips](@cary@ilm.com) 2019-06-14) Signed-off-by: Cary Phillips <cary@ilm.com>

* [fixing minor typos](https://github.com/openexr/openexr/commit/f62e9c0f9903e03c1d0d80e68e29ffba573c7f8d) ([xlietz](@31363633+xlietz@users.noreply.github.com) 2019-06-12) 

* [Edits to README.md and CONTRIBUTING.md](https://github.com/openexr/openexr/commit/55a674bde7ee63c1badacbe061d3cb222927c68e) ([Cary Phillips](@cary@ilm.com) 2019-06-11) 

* [Add initial Azure pipeline setup file](https://github.com/openexr/openexr/commit/9ed83bd964008c4ff19958b0e2824e08bdf6e610) ([seabeepea](@seabeepea@gmail.com) 2019-06-12) 

* [typos](https://github.com/openexr/openexr/commit/10e33e334df9202cd8c8a940c7cd3ec36548d7d8) ([seabeepea](@seabeepea@gmail.com) 2019-06-09) 

* [Contributing and Goverance sections](https://github.com/openexr/openexr/commit/ce9f05fbcc4c47330c43815cc40fc164e2ad53d3) ([seabeepea](@seabeepea@gmail.com) 2019-06-09) 

* [meeting notes](https://github.com/openexr/openexr/commit/eed7c0aa972cf8b5f5641ca9946b27a3a054155f) ([Cary Phillips](@cary@ilm.com) 2019-05-09) 

* [Fixed formatting](https://github.com/openexr/openexr/commit/b10e1015e349313b589f4c0b5b4bddefd3da64f7) ([John Mertic](@jmertic@linuxfoundation.org) 2019-05-08) Signed-off-by: John Mertic <jmertic@linuxfoundation.org>

* [moved charter to charter subfolder.](https://github.com/openexr/openexr/commit/db49dcfdfcfaca5a60a84f65ced11df97d0df1ec) ([Cary Phillips](@cary@ilm.com) 2019-05-08) 

* [OpenEXR-Technical-Charter.md](https://github.com/openexr/openexr/commit/2a33b9a4ca520490c5f368d6028decb9c76f8837) ([Cary Phillips](@cary@ilm.com) 2019-05-08) 

* [OpenEXR-Adoption-Proposal.md](https://github.com/openexr/openexr/commit/3e22cab39663b5c97ba3fd20df02ae634e21fc84) ([Cary Phillips](@cary@ilm.com) 2019-05-08) 

* [Meeting notes 2019-5-2](https://github.com/openexr/openexr/commit/c33d52f6c5a7d453d4b969224ab33852e47fe084) ([Cary Phillips](@cary@ilm.com) 2019-05-05) 

* [Remove unused cmake variable](https://github.com/openexr/openexr/commit/c3a1da6f47279d34c23d29f6e2f264cf2126a4f8) ([Nick Porcino](@nick.porcino@oculus.com) 2019-03-29) 

* [add build-win/, build-nuget/, and *~ to .gitignore.](https://github.com/openexr/openexr/commit/94ab55d8d4103881324ec15b8a41b3298ca7e467) ([Cary Phillips](@cary@ilm.com) 2018-09-22) 

* [Update the README files with instructions for building on Windows, specifically calling out the proper Visual Studio version.](https://github.com/openexr/openexr/commit/ab742b86a37a7eb93f0312d98fc47f7526ddd65a) ([Cary Phillips](@cary@ilm.com) 2018-09-22) 

* [Removed OpenEXRViewers.pc.in and PyIlmBase.pc.in. Since these modules are binaries, not libraries, there is no need to support pkgconfig for them.](https://github.com/openexr/openexr/commit/999a49d721604bb88178b596675deda4dc25cf1b) ([Cary Phillips](@cary@ilm.com) 2018-09-22) 

* [Rebuild OpenEXR NuGet with 2.3 source and enable exrviewer for testing purposes](https://github.com/openexr/openexr/commit/c0d0a637a25e1741f528999a2556eda39102ddac) ([mancoast](@RobertPancoast77@gmail.com) 2018-09-15) 

* [fix standalone and combined cmake](https://github.com/openexr/openexr/commit/017d027cc27ac0a7b2af90196fe3e49c4afe1aab) ([Kimball Thurston](@kdt3rd@gmail.com) 2018-09-08) This puts the version numbers into one file, and the settings and
variables for building into another, that is then replicated and
conditionally included when building a standalone package.
Signed-off-by: Kimball Thurston <kdt3rd@gmail.com>

* [CONTRIBUTING.md, INSTALL.md, and changes README.md and INSTALL.md](https://github.com/openexr/openexr/commit/d1d9f19475c858e66c1260fcc2be9e26dcddfc03) ([seabeepea](@seabeepea@gmail.com) 2019-06-09) 

* [added GOVERNANCE.md](https://github.com/openexr/openexr/commit/09a11a92b149f0e7d51a62086572050ad4fdc4fe) ([seabeepea](@seabeepea@gmail.com) 2019-06-09) 



## Version 2.3.0 (August 13, 2018)

### Features/Improvements:

* ThreadPool overhead improvements, enable custom thread pool to be registered via ThreadPoolProvider class
* Fixes to enable custom namespaces for Iex, Imf
* Improve read performance for deep/zipped data, and SIMD-accelerated uncompress support
* Added rawPixelDataToBuffer() function for access to compressed scanlines
* Iex::BaseExc no longer derived from std::string.
* Imath throw() specifiers removed
* Initial Support for Python 3

### Bugs:

* 25+ various bug fixes (see detailed Release Notes for the full list)

* This release addresses vulnerability [CVE-2017-12596](https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2017-12596).

### Build Fixes:

* Various fixes to the cmake and autoconf build infrastructures
* Various changes to support compiling for C++11 / C++14 / C++17 and GCC 6.3.1
* Various fixes to address Windows build issues
* 60+ total build-related fixes (see detailed Release Notes for the full list)

### Diff Stats \[git diff --stat v2.2.1\]

    CHANGES.md                                         |  1487 +++
    CMakeLists.txt                                     |   194 +
    Contrib/DtexToExr/bootstrap                        |     2 +-
    Contrib/DtexToExr/configure.ac                     |     2 +-
    IlmBase/CMakeLists.txt                             |   214 +-
    IlmBase/COPYING                                    |    34 -
    IlmBase/Half/CMakeLists.txt                        |   107 +-
    IlmBase/Half/half.cpp                              |     6 +-
    IlmBase/Half/half.h                                |     8 +-
    IlmBase/Half/halfExport.h                          |    44 +-
    IlmBase/Half/halfLimits.h                          |     9 +
    IlmBase/HalfTest/CMakeLists.txt                    |     4 +-
    IlmBase/HalfTest/testLimits.cpp                    |    13 +-
    IlmBase/INSTALL                                    |     2 -
    IlmBase/Iex/CMakeLists.txt                         |    81 +-
    IlmBase/Iex/IexBaseExc.cpp                         |    71 +-
    IlmBase/Iex/IexBaseExc.h                           |    87 +-
    IlmBase/Iex/IexMacros.h                            |    62 +-
    IlmBase/IexMath/CMakeLists.txt                     |    76 +-
    IlmBase/IexMath/IexMathFloatExc.cpp                |    18 +
    IlmBase/IexMath/IexMathFloatExc.h                  |    36 +-
    IlmBase/IexTest/CMakeLists.txt                     |     4 +-
    IlmBase/IexTest/testBaseExc.cpp                    |     2 +-
    IlmBase/IlmThread/CMakeLists.txt                   |    78 +-
    IlmBase/IlmThread/IlmThread.cpp                    |    48 +-
    IlmBase/IlmThread/IlmThread.h                      |    48 +-
    IlmBase/IlmThread/IlmThreadForward.h               |     8 +
    IlmBase/IlmThread/IlmThreadMutex.cpp               |     7 +-
    IlmBase/IlmThread/IlmThreadMutex.h                 |    48 +-
    IlmBase/IlmThread/IlmThreadMutexPosix.cpp          |    10 +-
    IlmBase/IlmThread/IlmThreadMutexWin32.cpp          |     9 +-
    IlmBase/IlmThread/IlmThreadPool.cpp                |   720 +-
    IlmBase/IlmThread/IlmThreadPool.h                  |    64 +-
    IlmBase/IlmThread/IlmThreadPosix.cpp               |     2 +
    IlmBase/IlmThread/IlmThreadSemaphore.h             |    49 +-
    .../IlmThread/IlmThreadSemaphorePosixCompat.cpp    |    78 +-
    IlmBase/IlmThread/IlmThreadWin32.cpp               |     6 +
    IlmBase/Imath/CMakeLists.txt                       |   130 +-
    IlmBase/Imath/ImathBox.cpp                         |    37 -
    IlmBase/Imath/ImathEuler.h                         |     7 +-
    IlmBase/Imath/ImathInt64.h                         |     3 +
    IlmBase/Imath/ImathMatrix.h                        |    56 +-
    IlmBase/Imath/ImathShear.cpp                       |    54 -
    IlmBase/Imath/ImathVec.cpp                         |    24 +-
    IlmBase/Imath/ImathVec.h                           |    48 +-
    IlmBase/Imath/Makefile.am                          |     4 +-
    IlmBase/ImathTest/CMakeLists.txt                   |     6 +-
    IlmBase/Makefile.am                                |     5 +-
    IlmBase/README                                     |    70 -
    IlmBase/README.CVS                                 |    16 -
    IlmBase/README.OSX                                 |   101 -
    IlmBase/README.cmake.txt                           |    65 -
    IlmBase/README.git                                 |    16 -
    IlmBase/README.md                                  |   277 +
    IlmBase/README.namespacing                         |    83 -
    IlmBase/bootstrap                                  |     4 +-
    IlmBase/config.windows/IlmBaseConfig.h             |     1 +
    IlmBase/config/IlmBaseConfig.h.in                  |     7 +
    IlmBase/configure.ac                               |    50 +-
    IlmBase/m4/ax_cxx_compile_stdcxx.m4                |   982 ++
    LICENSE                                            |    34 +
    OpenEXR/AUTHORS                                    |     2 +
    OpenEXR/CMakeLists.txt                             |   272 +-
    OpenEXR/COPYING                                    |    34 -
    OpenEXR/INSTALL                                    |     2 -
    OpenEXR/IlmImf/CMakeLists.txt                      |   396 +-
    OpenEXR/IlmImf/ImfAcesFile.h                       |    38 +-
    OpenEXR/IlmImf/ImfAttribute.cpp                    |     6 +
    OpenEXR/IlmImf/ImfAttribute.h                      |     8 +-
    OpenEXR/IlmImf/ImfB44Compressor.h                  |    10 +-
    OpenEXR/IlmImf/ImfCRgbaFile.h                      |     2 +-
    OpenEXR/IlmImf/ImfChannelList.h                    |    45 +-
    OpenEXR/IlmImf/ImfChromaticities.h                 |     5 +-
    OpenEXR/IlmImf/ImfCompositeDeepScanLine.h          |    14 +-
    OpenEXR/IlmImf/ImfCompressionAttribute.h           |     6 +-
    OpenEXR/IlmImf/ImfCompressor.h                     |    14 +-
    OpenEXR/IlmImf/ImfDeepCompositing.h                |     6 +-
    OpenEXR/IlmImf/ImfDeepFrameBuffer.h                |    38 +-
    OpenEXR/IlmImf/ImfDeepScanLineInputFile.cpp        |     6 +-
    OpenEXR/IlmImf/ImfDeepScanLineInputFile.h          |    20 +-
    OpenEXR/IlmImf/ImfDeepScanLineInputPart.h          |    18 +-
    OpenEXR/IlmImf/ImfDeepScanLineOutputFile.cpp       |    14 +-
    OpenEXR/IlmImf/ImfDeepScanLineOutputFile.h         |    14 +-
    OpenEXR/IlmImf/ImfDeepScanLineOutputPart.h         |    12 +-
    OpenEXR/IlmImf/ImfDeepTiledInputFile.cpp           |    16 +-
    OpenEXR/IlmImf/ImfDeepTiledInputFile.h             |    37 +-
    OpenEXR/IlmImf/ImfDeepTiledInputPart.cpp           |     2 +-
    OpenEXR/IlmImf/ImfDeepTiledInputPart.h             |    34 +-
    OpenEXR/IlmImf/ImfDeepTiledOutputFile.cpp          |    18 +-
    OpenEXR/IlmImf/ImfDeepTiledOutputFile.h            |    33 +-
    OpenEXR/IlmImf/ImfDeepTiledOutputPart.h            |    31 +-
    OpenEXR/IlmImf/ImfDwaCompressor.cpp                |   232 +-
    OpenEXR/IlmImf/ImfDwaCompressor.h                  |    43 +-
    OpenEXR/IlmImf/ImfDwaCompressorSimd.h              |    67 +-
    OpenEXR/IlmImf/ImfFastHuf.cpp                      |    62 +-
    OpenEXR/IlmImf/ImfFastHuf.h                        |     5 +
    OpenEXR/IlmImf/ImfFrameBuffer.h                    |    36 +-
    OpenEXR/IlmImf/ImfGenericInputFile.h               |     5 +-
    OpenEXR/IlmImf/ImfGenericOutputFile.h              |     6 +-
    OpenEXR/IlmImf/ImfHeader.h                         |    90 +-
    OpenEXR/IlmImf/ImfIO.h                             |    13 +-
    OpenEXR/IlmImf/ImfInputFile.cpp                    |    41 +-
    OpenEXR/IlmImf/ImfInputFile.h                      |    42 +-
    OpenEXR/IlmImf/ImfInputPart.cpp                    |     8 +
    OpenEXR/IlmImf/ImfInputPart.h                      |    22 +-
    OpenEXR/IlmImf/ImfInputPartData.h                  |     1 +
    OpenEXR/IlmImf/ImfInt64.h                          |     1 +
    OpenEXR/IlmImf/ImfKeyCode.h                        |    19 +-
    OpenEXR/IlmImf/ImfLut.h                            |     8 +-
    OpenEXR/IlmImf/ImfMisc.cpp                         |    55 +-
    OpenEXR/IlmImf/ImfMisc.h                           |    20 +-
    OpenEXR/IlmImf/ImfMultiPartInputFile.cpp           |     4 +-
    OpenEXR/IlmImf/ImfMultiPartInputFile.h             |    10 +-
    OpenEXR/IlmImf/ImfMultiPartOutputFile.cpp          |     4 +-
    OpenEXR/IlmImf/ImfMultiPartOutputFile.h            |    10 +-
    OpenEXR/IlmImf/ImfName.h                           |     9 +
    OpenEXR/IlmImf/ImfOpaqueAttribute.h                |    10 +-
    OpenEXR/IlmImf/ImfOptimizedPixelReading.h          |     4 +-
    OpenEXR/IlmImf/ImfOutputFile.cpp                   |    95 +-
    OpenEXR/IlmImf/ImfOutputFile.h                     |    15 +-
    OpenEXR/IlmImf/ImfOutputPart.h                     |    13 +-
    OpenEXR/IlmImf/ImfOutputPartData.h                 |    23 +-
    OpenEXR/IlmImf/ImfPizCompressor.h                  |    10 +-
    OpenEXR/IlmImf/ImfPreviewImage.h                   |    14 +-
    OpenEXR/IlmImf/ImfPxr24Compressor.h                |    10 +-
    OpenEXR/IlmImf/ImfRational.h                       |     3 +-
    OpenEXR/IlmImf/ImfRgbaFile.h                       |    47 +-
    OpenEXR/IlmImf/ImfRleCompressor.h                  |     7 +-
    OpenEXR/IlmImf/ImfScanLineInputFile.cpp            |    42 +-
    OpenEXR/IlmImf/ImfScanLineInputFile.h              |    37 +-
    OpenEXR/IlmImf/ImfSimd.h                           |    11 +-
    OpenEXR/IlmImf/ImfStdIO.cpp                        |    36 +-
    OpenEXR/IlmImf/ImfStdIO.h                          |    24 +-
    OpenEXR/IlmImf/ImfSystemSpecific.h                 |    15 +-
    OpenEXR/IlmImf/ImfTileOffsets.h                    |    16 +-
    OpenEXR/IlmImf/ImfTiledInputFile.cpp               |    16 +-
    OpenEXR/IlmImf/ImfTiledInputFile.h                 |    32 +-
    OpenEXR/IlmImf/ImfTiledInputPart.h                 |    30 +-
    OpenEXR/IlmImf/ImfTiledOutputFile.cpp              |    66 +-
    OpenEXR/IlmImf/ImfTiledOutputFile.h                |    39 +-
    OpenEXR/IlmImf/ImfTiledOutputPart.h                |    33 +-
    OpenEXR/IlmImf/ImfTiledRgbaFile.h                  |    83 +-
    OpenEXR/IlmImf/ImfTimeCode.h                       |    35 +-
    OpenEXR/IlmImf/ImfVersion.h                        |     4 +-
    OpenEXR/IlmImf/ImfZip.cpp                          |   191 +-
    OpenEXR/IlmImf/ImfZip.h                            |     8 +
    OpenEXR/IlmImf/ImfZipCompressor.h                  |     5 +
    OpenEXR/IlmImf/Makefile.am                         |    12 +-
    OpenEXR/IlmImf/dwaLookups.cpp                      |    10 +-
    OpenEXR/IlmImfExamples/CMakeLists.txt              |    18 +-
    OpenEXR/IlmImfExamples/Makefile.am                 |     8 +-
    OpenEXR/IlmImfExamples/previewImageExamples.cpp    |     6 +-
    OpenEXR/IlmImfFuzzTest/CMakeLists.txt              |    27 +-
    OpenEXR/IlmImfFuzzTest/Makefile.am                 |     6 +-
    OpenEXR/IlmImfTest/CMakeLists.txt                  |    18 +-
    OpenEXR/IlmImfTest/Makefile.am                     |     6 +-
    OpenEXR/IlmImfTest/compareDwa.h                    |     4 +-
    OpenEXR/IlmImfTest/testDwaCompressorSimd.cpp       |    47 +-
    OpenEXR/IlmImfUtil/CMakeLists.txt                  |   113 +-
    OpenEXR/IlmImfUtil/ImfDeepImage.h                  |    33 +-
    OpenEXR/IlmImfUtil/ImfDeepImageChannel.h           |    35 +-
    OpenEXR/IlmImfUtil/ImfDeepImageIO.h                |    26 +-
    OpenEXR/IlmImfUtil/ImfDeepImageLevel.cpp           |     2 +-
    OpenEXR/IlmImfUtil/ImfDeepImageLevel.h             |    44 +-
    OpenEXR/IlmImfUtil/ImfFlatImage.h                  |    29 +-
    OpenEXR/IlmImfUtil/ImfFlatImageChannel.h           |    10 +-
    OpenEXR/IlmImfUtil/ImfFlatImageIO.h                |    26 +-
    OpenEXR/IlmImfUtil/ImfFlatImageLevel.cpp           |     2 +-
    OpenEXR/IlmImfUtil/ImfFlatImageLevel.h             |    31 +-
    OpenEXR/IlmImfUtil/ImfImage.cpp                    |     4 +-
    OpenEXR/IlmImfUtil/ImfImage.h                      |    31 +-
    OpenEXR/IlmImfUtil/ImfImageChannel.h               |    10 +-
    OpenEXR/IlmImfUtil/ImfImageDataWindow.cpp          |     3 +-
    OpenEXR/IlmImfUtil/ImfImageDataWindow.h            |     2 +
    OpenEXR/IlmImfUtil/ImfImageIO.h                    |    10 +-
    OpenEXR/IlmImfUtil/ImfImageLevel.cpp               |     2 +-
    OpenEXR/IlmImfUtil/ImfImageLevel.h                 |    20 +-
    OpenEXR/IlmImfUtil/ImfSampleCountChannel.h         |    23 +-
    OpenEXR/IlmImfUtil/ImfUtilExport.h                 |    46 +
    OpenEXR/IlmImfUtil/Makefile.am                     |    16 +-
    OpenEXR/IlmImfUtilTest/CMakeLists.txt              |    20 +-
    OpenEXR/IlmImfUtilTest/Makefile.am                 |     6 +-
    OpenEXR/Makefile.am                                |     5 +-
    OpenEXR/README                                     |    77 -
    OpenEXR/README.CVS                                 |    16 -
    OpenEXR/README.OSX                                 |    57 -
    OpenEXR/README.cmake.txt                           |    54 -
    OpenEXR/README.git                                 |    16 -
    OpenEXR/README.md                                  |   132 +
    OpenEXR/README.namespacing                         |    83 -
    OpenEXR/bootstrap                                  |     4 +-
    OpenEXR/build.log                                  | 11993 -------------------
    OpenEXR/configure.ac                               |   284 +-
    OpenEXR/doc/Makefile.am                            |     1 -
    OpenEXR/doc/TheoryDeepPixels.pdf                   |   Bin 331719 -> 334777 bytes
    OpenEXR/exr2aces/CMakeLists.txt                    |    10 +-
    OpenEXR/exrbuild/CMakeLists.txt                    |    13 +-
    OpenEXR/exrenvmap/CMakeLists.txt                   |    10 +-
    OpenEXR/exrenvmap/Makefile.am                      |     6 +-
    OpenEXR/exrheader/CMakeLists.txt                   |    15 +-
    OpenEXR/exrheader/Makefile.am                      |     6 +-
    OpenEXR/exrmakepreview/CMakeLists.txt              |    10 +-
    OpenEXR/exrmakepreview/Makefile.am                 |     6 +-
    OpenEXR/exrmakepreview/makePreview.cpp             |     6 +-
    OpenEXR/exrmaketiled/CMakeLists.txt                |     9 +-
    OpenEXR/exrmaketiled/Makefile.am                   |     6 +-
    OpenEXR/exrmaketiled/makeTiled.cpp                 |     8 +-
    OpenEXR/exrmultipart/CMakeLists.txt                |    13 +-
    OpenEXR/exrmultipart/Makefile.am                   |     8 +-
    OpenEXR/exrmultiview/CMakeLists.txt                |    12 +-
    OpenEXR/exrmultiview/Makefile.am                   |     6 +-
    OpenEXR/exrstdattr/CMakeLists.txt                  |    13 +-
    OpenEXR/exrstdattr/Makefile.am                     |     6 +-
    OpenEXR/m4/ax_cxx_compile_stdcxx.m4                |   982 ++
    OpenEXR/m4/path.pkgconfig.m4                       |    63 +-
    OpenEXR_Viewers/AUTHORS                            |    12 -
    OpenEXR_Viewers/CMakeLists.txt                     |    71 +-
    OpenEXR_Viewers/COPYING                            |    34 -
    OpenEXR_Viewers/INSTALL                            |     2 -
    OpenEXR_Viewers/Makefile.am                        |     6 +-
    OpenEXR_Viewers/NEWS                               |     2 -
    OpenEXR_Viewers/README                             |    95 -
    OpenEXR_Viewers/README.CVS                         |    16 -
    OpenEXR_Viewers/README.OSX                         |    18 -
    OpenEXR_Viewers/README.md                          |   278 +
    OpenEXR_Viewers/README.win32                       |   196 -
    OpenEXR_Viewers/bootstrap                          |     4 +-
    OpenEXR_Viewers/configure.ac                       |    47 +-
    OpenEXR_Viewers/exrdisplay/CMakeLists.txt          |    15 +-
    OpenEXR_Viewers/exrdisplay/GlWindow3d.h            |     5 +
    OpenEXR_Viewers/m4/ax_cxx_compile_stdcxx.m4        |   982 ++
    OpenEXR_Viewers/playexr/CMakeLists.txt             |     8 +-
    PyIlmBase/AUTHORS                                  |    10 -
    PyIlmBase/CMakeLists.txt                           |   128 +-
    PyIlmBase/COPYING                                  |    34 -
    PyIlmBase/INSTALL                                  |     2 -
    PyIlmBase/Makefile.am                              |     7 +-
    PyIlmBase/NEWS                                     |     2 -
    PyIlmBase/PyIex/CMakeLists.txt                     |    52 +-
    PyIlmBase/PyIex/PyIex.cpp                          |     4 +-
    PyIlmBase/PyIex/PyIex.h                            |     4 +-
    PyIlmBase/PyIex/PyIexExport.h                      |    45 +-
    PyIlmBase/PyIex/iexmodule.cpp                      |     5 +-
    PyIlmBase/PyIexTest/CMakeLists.txt                 |     4 +-
    PyIlmBase/PyImath/CMakeLists.txt                   |    53 +-
    PyIlmBase/PyImath/PyImath.cpp                      |     5 +-
    PyIlmBase/PyImath/PyImath.h                        |     8 +-
    PyIlmBase/PyImath/PyImathAutovectorize.cpp         |     2 +-
    PyIlmBase/PyImath/PyImathAutovectorize.h           |     6 +-
    PyIlmBase/PyImath/PyImathBasicTypes.cpp            |     9 +-
    PyIlmBase/PyImath/PyImathBasicTypes.h              |     4 +-
    PyIlmBase/PyImath/PyImathBox.cpp                   |    18 +-
    PyIlmBase/PyImath/PyImathBox.h                     |     4 +-
    PyIlmBase/PyImath/PyImathBox2Array.cpp             |     4 +-
    PyIlmBase/PyImath/PyImathBox3Array.cpp             |     4 +-
    PyIlmBase/PyImath/PyImathBoxArrayImpl.h            |    10 +-
    PyIlmBase/PyImath/PyImathColor.h                   |     3 +-
    PyIlmBase/PyImath/PyImathColor3.cpp                |     8 +-
    PyIlmBase/PyImath/PyImathColor3ArrayImpl.h         |     4 +-
    PyIlmBase/PyImath/PyImathColor4.cpp                |     6 +-
    PyIlmBase/PyImath/PyImathColor4Array2DImpl.h       |     7 +-
    PyIlmBase/PyImath/PyImathColor4ArrayImpl.h         |     4 +-
    PyIlmBase/PyImath/PyImathEuler.cpp                 |     8 +-
    PyIlmBase/PyImath/PyImathEuler.h                   |     3 +-
    PyIlmBase/PyImath/PyImathExport.h                  |    52 +-
    PyIlmBase/PyImath/PyImathFixedArray.cpp            |     2 +-
    PyIlmBase/PyImath/PyImathFixedArray.h              |    11 +-
    PyIlmBase/PyImath/PyImathFixedArray2D.h            |     9 +
    PyIlmBase/PyImath/PyImathFixedMatrix.h             |     9 +
    PyIlmBase/PyImath/PyImathFixedVArray.cpp           |    14 +-
    PyIlmBase/PyImath/PyImathFixedVArray.h             |     2 +-
    PyIlmBase/PyImath/PyImathFrustum.cpp               |     8 +-
    PyIlmBase/PyImath/PyImathFrustum.h                 |     3 +-
    PyIlmBase/PyImath/PyImathFun.cpp                   |     8 +-
    PyIlmBase/PyImath/PyImathFun.h                     |     2 +-
    PyIlmBase/PyImath/PyImathLine.cpp                  |    16 +-
    PyIlmBase/PyImath/PyImathLine.h                    |     2 +-
    PyIlmBase/PyImath/PyImathM44Array.cpp              |     6 +-
    PyIlmBase/PyImath/PyImathM44Array.h                |     2 +-
    PyIlmBase/PyImath/PyImathMatrix.h                  |     3 +-
    PyIlmBase/PyImath/PyImathMatrix33.cpp              |     8 +-
    PyIlmBase/PyImath/PyImathMatrix44.cpp              |    10 +-
    PyIlmBase/PyImath/PyImathOperators.h               |     4 +-
    PyIlmBase/PyImath/PyImathPlane.cpp                 |    20 +-
    PyIlmBase/PyImath/PyImathPlane.h                   |     2 +-
    PyIlmBase/PyImath/PyImathQuat.cpp                  |    10 +-
    PyIlmBase/PyImath/PyImathQuat.h                    |     3 +-
    PyIlmBase/PyImath/PyImathRandom.cpp                |    10 +-
    PyIlmBase/PyImath/PyImathShear.cpp                 |     8 +-
    PyIlmBase/PyImath/PyImathStringArray.cpp           |     6 +-
    PyIlmBase/PyImath/PyImathStringArray.h             |     4 +-
    PyIlmBase/PyImath/PyImathStringArrayRegister.h     |     2 +-
    PyIlmBase/PyImath/PyImathStringTable.cpp           |     4 +-
    PyIlmBase/PyImath/PyImathTask.cpp                  |    10 +-
    PyIlmBase/PyImath/PyImathTask.h                    |    34 +-
    PyIlmBase/PyImath/PyImathUtil.cpp                  |     6 +-
    PyIlmBase/PyImath/PyImathUtil.h                    |    14 +-
    PyIlmBase/PyImath/PyImathVec.h                     |     4 +-
    PyIlmBase/PyImath/PyImathVec2Impl.h                |    12 +-
    PyIlmBase/PyImath/PyImathVec3ArrayImpl.h           |    12 +-
    PyIlmBase/PyImath/PyImathVec3Impl.h                |     6 +-
    PyIlmBase/PyImath/PyImathVec4ArrayImpl.h           |    10 +-
    PyIlmBase/PyImath/PyImathVec4Impl.h                |     6 +-
    PyIlmBase/PyImath/imathmodule.cpp                  |    38 +-
    PyIlmBase/PyImathNumpy/CMakeLists.txt              |    25 +-
    PyIlmBase/PyImathNumpy/imathnumpymodule.cpp        |    14 +-
    PyIlmBase/PyImathNumpyTest/CMakeLists.txt          |     6 +-
    PyIlmBase/PyImathNumpyTest/pyImathNumpyTest.in     |    81 +-
    PyIlmBase/PyImathTest/CMakeLists.txt               |     2 +
    PyIlmBase/PyImathTest/pyImathTest.in               |  1090 +-
    PyIlmBase/README                                   |    51 -
    PyIlmBase/README.OSX                               |    21 -
    PyIlmBase/README.md                                |    99 +
    PyIlmBase/bootstrap                                |     4 +-
    PyIlmBase/configure.ac                             |    64 +-
    PyIlmBase/m4/ax_cxx_compile_stdcxx.m4              |   982 ++
    README                                             |    68 -
    README.md                                          |   202 +
    cmake/FindIlmBase.cmake                            |   192 +
    cmake/FindNumPy.cmake                              |    51 +
    cmake/FindOpenEXR.cmake                            |   198 +
    321 files changed, 12796 insertions(+), 16398 deletions(-)
   
### Commits \[ git log v2.2.1...v.2.3.0\]

*  [Reverted python library -l line logic to go back to the old PYTHON_VERSION based logic.](https://github.com/openexr/openexr/commit/02310c624547fd765cd6e08abe459755d4ecebcc) ([Nick Rasmussen](@nick@ilm.com), 2018-08-09) 

*  [Updated build system to use local copies of the ax_cxx_copmile_stdcxx.m4 macro.](https://github.com/openexr/openexr/commit/3d6c9302b3d7f394a90ac3c95d12b1db1c183812) ([Nick Rasmussen](@nick@ilm.com), 2018-08-09) 

*  [accidentally commited Makefile instead of Makefile.am](https://github.com/openexr/openexr/commit/46dda162ef2b3defceaa25e6bdd2b71b98844685) ([Cary Phillips](@cary@ilm.com), 2018-08-09) 

*  [update CHANGES.md](https://github.com/openexr/openexr/commit/ea46c15be9572f81549eaa76a1bdf8dbe364f780) ([Cary Phillips](@cary@ilm.com), 2018-08-08) 

*  [Added FindNumPy.cmake](https://github.com/openexr/openexr/commit/63870bb10415ca7ea76ecfdafdfe70f5894f66f2) ([Nick Porcino](@meshula@hotmail.com), 2018-08-08) 

*  [Add PyImathNumpyTest to Makefile and configure.ac](https://github.com/openexr/openexr/commit/36abd2b728e8759b010ceffe94363d5f473fe6dc) ([Cary Phillips](@cary@ilm.com), 2018-08-08) 

*  [Add ImfExportUtil.h to Makefile.am](https://github.com/openexr/openexr/commit/82f78f4a895e29b42d2ccc0d66be08948203f507) ([Cary Phillips](@cary@ilm.com), 2018-08-08) 

*  [fix pyilmbase tests, static compilation](https://github.com/openexr/openexr/commit/75c918b65c2394c7f7a9f769fee87572d06e81b5) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-08-09) - python extensions must be shared, so can not follow the overall lib type for the library. - the code should be compiled fPIC when building a static library such that it can be linked into a .so - remove the dependency on the particle python extension in the numpy test - add environment variables such that the python tests will work in the build tree without a "make install" (win32 doesn't neede ld_library_path, but it doesn't hurt, but may need path?) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com> 

*  [fix OPENEXR_VERSION and OPENEXR_SOVERSION](https://github.com/openexr/openexr/commit/4481442b467e492a3a515b0992391dc160282786) ([Cary Phillips](@cary@ilm.com), 2018-08-08) 

*  [update readme documentation for new cmake option](https://github.com/openexr/openexr/commit/081c9f9f9f26afc6943f1b2e63d171802895bee5) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-08-08) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com> 

*  [fix compile errors under c++17](https://github.com/openexr/openexr/commit/6d9e3f6e2a9545e9d060f599967868d228d9a56a) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-08-08) Fixes errors with collisions due to the addition of clamp to the std namespace Signed-off-by: Kimball Thurston <kdt3rd@gmail.com> 

*  [add last ditch effort for numpy](https://github.com/openexr/openexr/commit/af5fa2d84acf74e411d6592201890b1e489978c4) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-08-08) Apparently not all distributions include a FindNumPy.cmake or similar, even if numpy is indeed installed. This makes a second effort to find using python itself Signed-off-by: Kimball Thurston <kdt3rd@gmail.com> 

*  [make pyilmbase tests conditional](https://github.com/openexr/openexr/commit/07951c8bdf6164e34f37c3d88799e4e98e46d1ee) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-08-08) This makes the PyIlmBase tests conditional in the same manner as OpenEXR and IlmBase Signed-off-by: Kimball Thurston <kdt3rd@gmail.com> 

*  [optimize regeneration of config files](https://github.com/openexr/openexr/commit/b610ff33e827c38ac3693d3e43ad973c891d808c) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-08-08) This makes the config files write to a temporary file, then use cmake's configure_file command with copyonly to compare the contents and not copy if they are the same. Incremental builds are much faster as a result when working on new features and adding files to the cmakelists.txt Signed-off-by: Kimball Thurston <kdt3rd@gmail.com> 

*  [make fuzz test optional like autoconf](https://github.com/openexr/openexr/commit/79a50ea7eb869a94bb226841aebad9d46ecc3836) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-08-08) This makes running the fuzz tests as part of the "make test" rule optional. Even with this off by default, if building tests is enabled, the fuzz test will still be compiled, and is available to run via "make fuzz". This should enable a weekly jenkins build config to run the fuzz tests, given that it takes a long time to run. Signed-off-by: Kimball Thurston <kdt3rd@gmail.com> 

*  [Fix SO version](https://github.com/openexr/openexr/commit/f4055c33bb128bd4544d265b167337c584364716) ([Nick Porcino](@meshula@hotmail.com), 2018-08-07) 

*  [CHANGES.md formatting](https://github.com/openexr/openexr/commit/8cd1b9210855fa4f6923c1b94df8a86166be19b1) ([Cary Phillips](@cary@ilm.com), 2018-08-07) 

*  [format old release notes](https://github.com/openexr/openexr/commit/3c5b5f894def68cf5240e8f427147c867f745912) ([Cary Phillips](@cary@ilm.com), 2018-08-07) 

*  [release notes upates](https://github.com/openexr/openexr/commit/534e4bcde71ce34b9f8fa9fc39e9df1a58aa3f80) ([Cary Phillips](@cary@ilm.com), 2018-08-07) 

*  [CHANGES.md](https://github.com/openexr/openexr/commit/471d7bd1c558c54ecc3cbbb2a65932f1e448a370) ([Cary Phillips](@cary@ilm.com), 2018-08-07) 

*  [OpenEXR_Viewers/README.md formatting](https://github.com/openexr/openexr/commit/806db743cf0bcb7710d08f56ee6f2ece10e31367) ([Cary Phillips](@cary@ilm.com), 2018-08-07) 

*  [more README fixes.](https://github.com/openexr/openexr/commit/82bc701e605e092ae5f31d142450d921c293ded1) ([Cary Phillips](@cary@ilm.com), 2018-08-07) 

*  [README.md cleanup](https://github.com/openexr/openexr/commit/d1d9760b084f460cf21de2b8e273e8d6adcfb4f6) ([Cary Phillips](@cary@ilm.com), 2018-08-07) 

*  [fix dependencies when building static](https://github.com/openexr/openexr/commit/03329c8d34c93ecafb4a35a8cc645cd3bea14217) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-08-08) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com> 

*  [fix exrdisplay compile under cmake](https://github.com/openexr/openexr/commit/a617dc1a9cc8c7b85df040f5587f1727dec31caf) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-08-07) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com> 

*  [PyIlmBase README.md cleanup](https://github.com/openexr/openexr/commit/a385fd4f09ab5dd1163fab6870393f1b71e163eb) ([Cary Phillips](@cary@ilm.com), 2018-08-07) 

*  [Updates to README's](https://github.com/openexr/openexr/commit/0690e762bb45afadd89e94838270080447998a48) ([Cary Phillips](@cary@ilm.com), 2018-08-07) 

*  [added --foreign to automake in bootstrap](https://github.com/openexr/openexr/commit/4a74696f2066dd4bb58433bbcb706fdf526a7770) ([Cary Phillips](@cary@ilm.com), 2018-08-06) 

*  [Remove obsolete README files from Makefile.am](https://github.com/openexr/openexr/commit/57259b7811f3adce23a1e4c99411d686c55fefed) ([Cary Phillips](@cary@ilm.com), 2018-08-06) 

*  [Removed COPYING, INSTALL, README.cmake.txt](https://github.com/openexr/openexr/commit/54d3bbcfef10a367591cced99f759b89e8478b07) ([Cary Phillips](@cary@ilm.com), 2018-08-05) 

*  [cleaned up README files for root and IlmBase](https://github.com/openexr/openexr/commit/54e6ae149addd5b9673d1ee0f2954759b5ed073d) ([Cary Phillips](@cary@ilm.com), 2018-08-05) 

*  [LIBTOOL_CURRENT=24](https://github.com/openexr/openexr/commit/7b7ea9c86bbf8744cb41df6fa7e5f7dd270294a5) ([Cary Phillips](@cary@ilm.com), 2018-08-06) 

*  [bump version to 2.3](https://github.com/openexr/openexr/commit/8a7b4ad263103e725fda4e624962cc0f559c4faa) ([Cary Phillips](@cary@ilm.com), 2018-08-05) 

*  [folding in internal ILM changes - conditional delete in exception catch block.](https://github.com/openexr/openexr/commit/656f898dff3ab7d06c4d35219385251f7948437b) ([Cary Phillips](@cary@ilm.com), 2018-08-05) 

*  [Removed COPYING, INSTALL, README.cmake.txt](https://github.com/openexr/openexr/commit/94ece7ca86ffccb3ec2bf4138f4ad47e3f496167) ([Cary Phillips](@cary@ilm.com), 2018-08-05) 

*  [edits to READMEs](https://github.com/openexr/openexr/commit/405fa911ad974eeaf3c3769820b7c4a0c59f0099) ([Cary Phillips](@cary@ilm.com), 2018-08-05) 

*  [README fixes.](https://github.com/openexr/openexr/commit/c612d8276a5d9e28ae6bdc39b770cbc083e21cf4) ([Cary Phillips](@cary@ilm.com), 2018-08-05) 

*  [cleaned up README files for root and IlmBase](https://github.com/openexr/openexr/commit/cda04c6451b0b196c887b03e68d8a80863f58832) ([Cary Phillips](@cary@ilm.com), 2018-08-05) 

*  [Fallback default system provided Boost Python](https://github.com/openexr/openexr/commit/a174497d1fd84378423f733053f1a058608d81f0) ([Thanh Ha](@thanh.ha@linuxfoundation.org), 2018-08-03) User provided Python version via OPENEXR_PYTHON_MAJOR and OPENEXR_PYTHON_MINOR parameters, failing that fallback onto the system's default "python" whichever that may be. Signed-off-by: Thanh Ha <thanh.ha@linuxfoundation.org> 

*  [fix double delete in multipart files, check logic in others](https://github.com/openexr/openexr/commit/da96e3759758c1fcac5963e07eab8e1f58a674e7) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-08-04) Multipart files have a Data object that automatically cleans up it's stream if appropriate, the other file objects have the destructor of the file object perform the delete (instead of Data). This causes a double delete to happen in MultiPart objects when unable to open a stream. Additionally, fix tabs / spaces to just be spaces Signed-off-by: Kimball Thurston <kdt3rd@gmail.com> 

*  [fix scenario where ilmimf is being compiled from parent directory](https://github.com/openexr/openexr/commit/c246315fe392815399aee224f38bafd01585594b) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-08-04) need to use current source dir so test images can be found Signed-off-by: Kimball Thurston <kdt3rd@gmail.com> 

*  [Fix logic errors with BUILD_DWALOOKUPS](https://github.com/openexr/openexr/commit/dc7cb41c4e8a3abd60dec46d0bcb6a1c9ef31452) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-08-04) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com> 

*  [remove debug print](https://github.com/openexr/openexr/commit/8e16aa8930a85f1ef3f1f6ba454af275aabc205d) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-08-04) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com> 

*  [fully set variables as pkg config does not seem to by default](https://github.com/openexr/openexr/commit/f478511f796e5d05dada28f9841dcf9ebd9730ac) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-08-04) 

*  [add check with error message for zlib, fix defaults, restore old thread check](https://github.com/openexr/openexr/commit/788956537282cfcca712c1e9690d72cd19978ce0) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-08-04) 

*  [PR #187 CMake enhancements to speed up dependency builds of OpenEXR.](https://github.com/openexr/openexr/commit/17e10ab10ddf937bc2809bda858bf17af6fb3448) ([Nick Porcino](@meshula@hotmail.com), 2018-08-02) 

*  [restore prefix, update to use PKG_CHECK_MODULES](https://github.com/openexr/openexr/commit/fb9d1be5c07779c90e7744ccbf27201fcafcdfdb) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-08-03) previous commit from dracwyrm had made it such that pkg-config must be used and ilmbase must be installed in the default pkg-config path by default. restore the original behaviour by which a prefix could be provided, yet still retain use of PKG_CHECK_MODULES to find IlmBase if the prefix is not specified, and continue to use pkg-config to find zlib instead of assuming -lz Signed-off-by: Kimball Thurston <kdt3rd@gmail.com> 

*  [restore original API for Lock since we can't use typedef to unique_lock](https://github.com/openexr/openexr/commit/e7fc2258a16ab7fe17d24855d16d4e56b80c172e) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-08-02) 

*  [fixes #292, issue with utf-8 filenames](https://github.com/openexr/openexr/commit/846fe64c584ebb89434aaa02f5d431fbd3ca6165) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-08-01) windows needs to widen the string to properly open files, this implements a solution for compiling with MSVC anyway using the extension for fstream to take a wchar Signed-off-by: Kimball Thurston <kdt3rd@gmail.com> 

*  [fix maintainer mode issue, extra line in paste](https://github.com/openexr/openexr/commit/772ff9ad045032fc338af1b684cb50983191bc0d) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-08-02) 

*  [Default the python bindings to on](https://github.com/openexr/openexr/commit/dc5e26136b1c5edce911ff0eccc17cda40388b54) ([Nick Porcino](@meshula@hotmail.com), 2018-08-01) 

*  [Add Find scripts, and ability to build OpenEXR with pre-existing IlmBase](https://github.com/openexr/openexr/commit/34ee51e9118097f784653f08c9482c886f83d2ef) ([Nick Porcino](@meshula@hotmail.com), 2018-08-01) 

*  [fix names, disable rules when not building shared](https://github.com/openexr/openexr/commit/dbd3b34baf4104e844c273b682e7b133304294f2) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-07-31) 

*  [add suffix variable for target names to enable static-only build](https://github.com/openexr/openexr/commit/7b1ed10e241e793db9d8933df30dd305a93835dd) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-07-31) 

*  [The string field is now called _message.](https://github.com/openexr/openexr/commit/bd32e84632da4754cfe6db47f2e72c29f4d7df27) ([Cary Phillips](@cary@ilm.com), 2018-08-01) 

*  [C++11 support for numeric_limits<Half>::max_digits10() and lowest()](https://github.com/openexr/openexr/commit/2d931bab38840ab3cdf9c6322767a862aae4037d) ([Cary Phillips](@cary@ilm.com), 2018-07-31) 

*  [fixes for GCC 6.3.1 (courtesy of Will Harrower): - renamed local variables in THROW macros to avoid warnings - cast to bool](https://github.com/openexr/openexr/commit/7fda69a377ee41979284137795cb338bb3c6d147) ([Cary Phillips](@cary@rnd-build7-sf-38.lucasfilm.com), 2018-07-31) 

*  [renames name to message and removes implicit cast](https://github.com/openexr/openexr/commit/54105e3c292c6884e7870ecfddb561deda7a3458) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-07-31) This removes the implicit cast, which is arguably more "standard", and also less surprising. Further, renames the name function to message to match internal ILM changes, and message makes more sense as a function name than ... name. 

*  [Remove IEX_THROW_SPEC](https://github.com/openexr/openexr/commit/02c896501da244ec6345d7ee5ef825d71ba1f0a2) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-07-31) This removes the macro and uses therein. We changed the API with removing the subclass from std::string of Iex::BaseExc, so there is no reason to retain this compatibility as well, especially since it isn't really meaningful anyway in (modern) C++ 

*  [CMake3 port. Various Windows fixes](https://github.com/openexr/openexr/commit/b2d37be8b874b300be1907f10339cac47e39170b) ([Nick Porcino](@meshula@hotmail.com), 2018-07-29) 

*  [changes to enable custom namespace defines to be used](https://github.com/openexr/openexr/commit/acd76e16276b54186096b04b06bd118eb32a1bcf) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-07-29) 

*  [fix extraneous unsigned compare accidentally merged](https://github.com/openexr/openexr/commit/a56773bd7a1f9a8bb10afe5fb36c4e03f622eff6) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-07-29) 

*  [Use proper definition of namespaces instead of default values.](https://github.com/openexr/openexr/commit/c6978f9fd998df32b2c56a7b25bbbd52005bbf9e) ([Juri Abramov](@gabramov@nvidia.com), 2014-08-18) 

*  [fixes #260, out of bounds vector access](https://github.com/openexr/openexr/commit/efc360fc17935453e95f62939dd5d7caacce4bf7) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-07-29) noticed by Google Autofuzz Signed-off-by: Kimball Thurston <kdt3rd@gmail.com> 

*  [fix potential io streams leak in case of exceptions during 'initialize' function](https://github.com/openexr/openexr/commit/19bac86f27bab8649858ef79658224e9a54cb4cf) ([CAHEK7](@ghosts.in.a.box@gmail.com), 2016-02-12) 

*  [OpenEXR: Fix build system and change doc install](https://github.com/openexr/openexr/commit/60cc8b711ab402c5526ca1f872de5209ad15ec7d) ([dracwyrm](@j.scruggs@gmail.com), 2017-08-11) The build sysem for the OpenEXR sub-module is has issues. This patch is being used on Gentoo Linux with great success. It also adresses the issue of linking to previously installed versions. Signed-off by: Jonathan Scruggs (j.scruggs@gmail.com) Signed-off by: David Seifert (soap@gentoo.org) 

*  [Note that numpy is required to build PyIlmBase](https://github.com/openexr/openexr/commit/76935a912a8e365ed4fe8c7a54b60561790dafd5) ([Thanh Ha](@thanh.ha@linuxfoundation.org), 2018-07-20) Signed-off-by: Thanh Ha <thanh.ha@linuxfoundation.org> 

*  [Fixed exports on DeepImageChannel and FlatImageChannel. If the whole class isn't exported, the typeinfo doesn't get exported, and so dynamic casting into those classes will not work.](https://github.com/openexr/openexr/commit/942ff971d30cba1b237c91e9f448376d279dc5ee) ([Halfdan Ingvarsson](@halfdan@sidefx.com), 2014-10-06) Also fixed angle-bracket include to a quoted include. 

*  [Fixed angle bracket vs quote includes.](https://github.com/openexr/openexr/commit/fd8570927a7124ff2990f5f38556b2ec03d77a44) ([Halfdan Ingvarsson](@halfdan@sidefx.com), 2014-03-18) 

*  [Change IexBaseExc to no longer derive from std::string, but instead include it as a member variable. This resolves a problem with MSVC 2012 and dllexport-ing template classes.](https://github.com/openexr/openexr/commit/fa59776fd83a8f35ed5418b83bbc9975ba0ef3bc) ([Halfdan Ingvarsson](@halfdan@sidefx.com), 2014-03-03) 

*  [make code more amenable to compiling with mingw for cross-compiling](https://github.com/openexr/openexr/commit/dd867668c4c63d23c034cc2ea8f2352451e8554d) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-07-29) Signed-off-by: Kimball Thurston <kdt3rd@gmail.com> 

*  [Fix shebang line to bash](https://github.com/openexr/openexr/commit/d3512e07a5af5053397ed62bd0d306b10357358c) ([Thanh Ha](@thanh.ha@linuxfoundation.org), 2018-07-19) Depending on the distro running the script the following error might appear: ./bootstrap: 4: [: Linux: unexpected operator This is because #!/bin/sh is not the same on every distro and this script is actually expecting bash. So update the shebang line to be bash. Signed-off-by: Thanh Ha <thanh.ha@linuxfoundation.org> 

*  [Visual Studio and Windows fixes](https://github.com/openexr/openexr/commit/4cfefeab4be94b8c46d604075367b6496d29dcb5) ([Liam Fernandez](@liam@utexas.edu), 2018-06-20) IlmBase: Fix IF/ELSEIF clause (WIN32 only) PyImath: Install *.h in 'include' dir PyImathNumpy: Change python library filename to 'imathnumpy.pyd' (WIN32 only) 

*  [Fix probable typo for static builds.](https://github.com/openexr/openexr/commit/31e1ae8acad3126a63044dfb8518d70390131c7b) ([Simon Otter](@skurmedel@gmail.com), 2018-06-18) 

*  [Must also export protected methods](https://github.com/openexr/openexr/commit/17384ee01e5fa842f282c833ab2bc2aa33e07125) ([Nick Porcino](@meshula@hotmail.com), 2018-06-10) 

*  [IlmImfUtilTest compiles successfully](https://github.com/openexr/openexr/commit/6093789bc7b7c543f128ab2b055987808ec15167) ([Nick Porcino](@meshula@hotmail.com), 2018-06-09) 

*  [IlmImfUtil now builds on Windows](https://github.com/openexr/openexr/commit/d7328287d1ea363ab7839201e90d7d7f4deb635f) ([Nick Porcino](@meshula@hotmail.com), 2018-06-09) 

*  [Set python module suffix per platform](https://github.com/openexr/openexr/commit/39b9edfdfcad5e77601d4462a6f9ba93bef83835) ([Nick Porcino](@meshula@hotmail.com), 2018-06-05) 

*  [fix include ifdef](https://github.com/openexr/openexr/commit/32723d8112d1addf0064e8295b824faab60f0162) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-05-26) 

*  [switch from shared pointer to a manually counted object as gcc 4.8 and 4.9 do not provide proper shared_ptr atomic functions](https://github.com/openexr/openexr/commit/3f532a7ab81c33f61dc6786a8c7ce6e0c09acc07) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-05-26) 

*  [Fix typos to TheoryDeepPixels document](https://github.com/openexr/openexr/commit/655f96032e0eddd868a122fee80bd558e0cbf17d) ([peterhillman](@peter@peterhillman.org.uk), 2018-05-17) Equations 6 and 7 were incorrect. 

*  [initial port of PyIlmBase to python 3](https://github.com/openexr/openexr/commit/84dbf637c5c3ac4296181dd93de4fb5ffdc4b582) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-05-04) 

*  [replicate configure / cmake changes from ilmbase](https://github.com/openexr/openexr/commit/00df2c72ca1b7cb148e19a9bdc44651a6c74c9e4) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-05-04) This propagates the same chnages to configure.ac and cmakelists.txt to enable compiling with c++11/14. Additionally, adds some minor changes to configure to enable python 3 to be configured (source code changes tbd) 

*  [add move constructor and assignment operator](https://github.com/openexr/openexr/commit/cfebcc24e1a1cc307678ea757ec38bff02a5dc51) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-05-03) 

*  [Fix Windows Python binding builds. Does not address PyImath runtime issues, but does allow build to succeed](https://github.com/openexr/openexr/commit/15ce54ca02fdfa16c4a99f45a30c7a54826c6ac3) ([Nick Porcino](@meshula@hotmail.com), 2018-04-30) 

*  [Fix c++11 detection issue on windows. Fix ilmbase DLL export warnings](https://github.com/openexr/openexr/commit/7376f9b736f9503a9d34b67c99bc48ce826a6334) ([Nick Porcino](@meshula@hotmail.com), 2018-04-27) 

*  [enable different c++ standards to be selected instead of just c++14](https://github.com/openexr/openexr/commit/99ecfcabbc2b95acb40283f04ab358b3db9cc0f9) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-04-15) 

*  [Incorporate review feedback](https://github.com/openexr/openexr/commit/99b367d963ba0892e7ab830458b6a990aa3033ce) ([Nick Porcino](@meshula@hotmail.com), 2018-04-04) 

*  [add compatibility std::condition_variable semaphore when posix semaphores not available](https://github.com/openexr/openexr/commit/b6dc2a6b71f9373640d988979f9ae1929640397a) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-04-04) 

*  [fix error overwriting beginning of config file](https://github.com/openexr/openexr/commit/01680dc4d90c9f7fd64e498e57588f630a52a214) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-04-04) 

*  [remove the dynamic exception for all versions of c++ unless FORCE_CXX03 is on](https://github.com/openexr/openexr/commit/45cb2c8fb2418afaa3900c553e26ad3886cd5acf) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-04-04) 

*  [ThreadPool improvements](https://github.com/openexr/openexr/commit/bf0cb8cdce32fce36017107c9982e1e5db2fb3fa) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-04-04) - switch to use c++11 features - Add API to enable replacement of the thread pool - Add custom, low-latency handling when threads is 0 - Lower lock boundary when adding tasks (or eliminate in c++11 mode) 

*  [switch mutex to be based on std::mutex when available](https://github.com/openexr/openexr/commit/848c8c329b16aeee0d3773e827d506a2a53d4840) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-04-04) 

*  [switch IlmThread to use c++11 threads when enabled](https://github.com/openexr/openexr/commit/eea1e607177e339e05daa1a2ec969a9dd12f2497) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-04-04) 

*  [use dynamic exception macro to avoid warnings in c++14 mode](https://github.com/openexr/openexr/commit/610179cbe3ffc2db206252343e75a16221d162b4) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-04-04) 

*  [add #define to manage dynamic exception deprecation in c++11/14](https://github.com/openexr/openexr/commit/b133b769aaee98566e695191476f59f32eece591) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-04-04) 

*  [configuration changes to enable c++14](https://github.com/openexr/openexr/commit/5f58c94aea83d44e27afd1f65e4defc0f523f6be) ([Kimball Thurston](@kdt3rd@gmail.com), 2018-04-04) 

*  [Cmake now building OpenEXR successfully for Windows](https://github.com/openexr/openexr/commit/ac055a9e50c974f4cd58c28a5a0bb96011812072) ([Nick Porcino](@meshula@hotmail.com), 2018-03-28) 

*  [Missing symbols on Windows due to missing IMF_EXPORT](https://github.com/openexr/openexr/commit/965c1eb6513ad80c71b425c8a1b04a70b3bae291) ([Ibraheem Alhashim](@ibraheem.alhashim@gmail.com), 2018-03-05) 

*  [Implement SIMD-accelerated ImfZip::uncompress](https://github.com/openexr/openexr/commit/32f2aa58fe4f6f6691eef322fdfbbc9aa8363f80) ([John Loy](@jloy@pixar.com), 2017-04-12) The main bottleneck in ImfZip::uncompress appears not to be zlib but the predictor & interleaving loops that run after zlib's decompression. Fortunately, throughput in both of these loops can be improved with SIMD operations. Even though each trip of the predictor loop has data dependencies on all previous values, the usual SIMD prefix-sum construction is able to provide a significant speedup. While the uses of SSSE3 and SSE4.1 are minor in this change and could maybe be replaced with some slightly more complicated SSE2, SSE4.1 was released in 2007, so it doesn't seem unreasonable to require it in 2017. 

*  [Compute sample locations directly in Imf::readPerDeepLineTable.](https://github.com/openexr/openexr/commit/e64095257a29f9bc423298ee8dbc09a317f22046) ([John Loy](@jloy@pixar.com), 2017-04-06) By changing the function to iterate over sample locations directly instead of discarding unsampled pixel positions, we can avoid computing a lot of modulos (more than one per pixel.) Even on modern x86 processors, idiv is a relatively expensive instruction. Though it may appear like this optimization could be performed by a sufficiently sophisticated compiler, gcc 4.8 does not get there (even at -O3.) 

*  [Manually hoist loop invariants in Imf::bytesPerDeepLineTable.](https://github.com/openexr/openexr/commit/71b8109a4ad123ef0d5783f01922463a16d2ca59) ([John Loy](@jloy@pixar.com), 2017-04-05) This is primarily done to avoid a call to pixelTypeSize within the inner loop. In particular, gcc makes the call to pixelTypeSize via PLT indirection so it may have arbitrary side-effects (i.e. ELF symbol interposition strikes again) and may not be moved out of the loop by the compiler. 

*  [Inline Imf::sampleCount; this is an ABI-breaking change.](https://github.com/openexr/openexr/commit/5aa0afd5a4f8df9e09d6461f115e6e0cec5cbe46) ([John Loy](@jloy@pixar.com), 2017-03-29) gcc generates calls to sampleCount via PLT indirection even within libIlmImf. As such, they are not inlined and must be treated as having arbitrary side effects (because of ELF symbol interposition.) Making addressing computations visible at call sites allows a much wider range of optimizations by the compiler beyond simply eliminating the function call overhead. 

*  [Delete build.log](https://github.com/openexr/openexr/commit/148f1c230b5ecd94d795ca172a8246785c7caca7) ([Arkady Shapkin](@arkady.shapkin@gmail.com), 2017-02-18) 

*  [fix defect in semaphore implementation which caused application hang at exit time, because not all worker threads get woken up when task semaphore is repeatedly posted (to wake them up) after setting the stopping flag in the thread pool](https://github.com/openexr/openexr/commit/4706d615e942462a532381a8a86bc5fe820c6816) ([Richard Goedeken](@Richard@fascinationsoftware.com), 2016-11-22) 

*  [fix comparison of unsigned expression < 0 (Issue #165)](https://github.com/openexr/openexr/commit/9e3913c94c55549640c732f549d2912fbd85c336) ([CAHEK7](@ghosts.in.a.box@gmail.com), 2016-02-15) 

*  [Added Iex library once more for linker dependency](https://github.com/openexr/openexr/commit/b0b50791b5b36fddb010b5ad630dd429f947a080) ([Eric Sommerlade](@es0m@users.noreply.github.com), 2015-02-20) 

*  [windows/cmake: Commands depend on Half.dll which needs to be in path. Running commands in Half.dll's directory addresses this and the commands run on first invocation](https://github.com/openexr/openexr/commit/1a23716fd7e9ae167f53c7f2099651ede1279fbb) ([E Sommerlade](@es0m@users.noreply.github.com), 2015-02-10) 

*  [Fixed memory corruption / actual crashes on Window](https://github.com/openexr/openexr/commit/c330c40e1962257b0e59328fdceaa9cdcde3041b) ([JuriAbramov](@openexr@dr-abramov.de), 2015-01-19) Fixed memory corruption caused by missing assignment operator with non-trivial copy constructor logic. FIxes crashes on Windows when "dwaa" or "dwab" codecs are used for saving files. 

*  [std namespace should be specified for transform](https://github.com/openexr/openexr/commit/4a00a9bc6c92b20443c61f5e9877123e7fef16e6) ([JuriAbramov](@openexr@dr-abramov.de), 2014-08-20) Fixes build with some VS and clang version. 

*  [m4/path.pkgconfig.m4: use PKG_PROG_PKG_CONFIG to find correct pkg-config](https://github.com/openexr/openexr/commit/056cb9f09efa9116c7f5fb8bc0717a260ad23744) ([Michael Thomas (malinka)](@malinka@entropy-development.com), 2016-05-24) pkg-config supplies this macro and prefers it to be used to allow for cross-compilation scenarios where target-prefixed binaries are prefered to pkg-config 

*  [Updated list of EXTRA_DIST files to reflect the updated test images and prior removal of README.win32](https://github.com/openexr/openexr/commit/165dceaeee86e0f8ce1ed1db3e3030c609a49f17) ([Nick Rasmussen](@nick@ilm.com), 2017-11-17) 

*  [Updated list of EXTRA_DIST files to reflect the updated test images and prior removal of README.win32](https://github.com/openexr/openexr/commit/dcaf5fdb4d1244d8e60a58832cfe9c54734a2257) ([Nick Rasmussen](@nick@ilm.com), 2017-11-17) 

*  [Updated openexr version to 2.2.1, resynced the .so version number to 23 across all projects.](https://github.com/openexr/openexr/commit/e69de40ddbb6bd58341618a506b2e913e5ac1797) ([Nick Rasmussen](@nick@ilm.com), 2017-11-17) 

*  [Add additional input validation in an attempt to resolve issue #232](https://github.com/openexr/openexr/commit/49db4a4192482eec9c27669f75db144cf5434804) ([Shawn Walker-Salas](@shawn.walker@oracle.com), 2017-05-30) 

*  [Add additional input validation in an attempt to resolve issue #232](https://github.com/openexr/openexr/commit/f09f5f26c1924c4f7e183428ca79c9881afaf53c) ([Shawn Walker-Salas](@shawn.walker@oracle.com), 2017-05-30) 

*  [root level LICENSE](https://github.com/openexr/openexr/commit/a774d643b566d56314f26695f2bf9b75f88e64f6) ([cary-ilm](@cary@ilm.com), 2017-10-23) 

*  [Fix copyright/license notice in halfExport.h](https://github.com/openexr/openexr/commit/20d043d017d4b752356bb76946ffdffaa9c15c72) ([Ed Hanway](@ehanway@ilm.com), 2017-01-09) 

*  [Merge branch 'jkingsman-cleanup-readme' into develop](https://github.com/openexr/openexr/commit/6f6d9cea513ea409d4b65da40ac096eab9a549b0) ([Ed Hanway](@ehanway@ilm.com), 2016-10-28) 

*  [README edits.](https://github.com/openexr/openexr/commit/098a4893910d522b867082ed38d7388e6265bee0) ([Ed Hanway](@ehanway@ilm.com), 2016-10-28) 

*  [Merge branch 'cleanup-readme' of https://github.com/jkingsman/openexr into jkingsman-cleanup-readme](https://github.com/openexr/openexr/commit/43e50ed5dca1ddfb3ca2cb4c38c7752497db6e50) ([Ed Hanway](@ehanway@ilm.com), 2016-10-28) 

*  [Install ImfStdIO.h](https://github.com/openexr/openexr/commit/2872d3b230a7920696510f80a50d9ce36b6cc94e) ([Ed Hanway](@ehanway@ilm.com), 2016-10-28) This was originally intended to be an internal class only, but its use has become the de facto way to handle UTF-8 filenames on Windows. 

*  [Merge pull request #204 from dlemstra/IMF_HAVE_SSE2](https://github.com/openexr/openexr/commit/cbb01bf286a2e04df95fb51458d1c2cbdc08935b) ([Ed Hanway](@ehanway-ilm@users.noreply.github.com), 2016-10-19) Consistent check for IMF_HAVE_SSE2. 

*  [Remove fixed-length line breaks](https://github.com/openexr/openexr/commit/0ea6b8c7d077a18fb849c2b2ff532cd952d06a38) ([Jack Kingsman](@jack.kingsman@gmail.com), 2016-10-19) 

*  [Update README to markdown](https://github.com/openexr/openexr/commit/9c6d22e23a25d761f5456e08623b8d77c0f8930a) ([Jack Kingsman](@jack.kingsman@gmail.com), 2016-10-18) 

*  [Merge pull request #206 from lgritz/lg-register](https://github.com/openexr/openexr/commit/6788745398594d479e8cf91a6c301fea0537108b) ([Ed Hanway](@ehanway-ilm@users.noreply.github.com), 2016-09-30) Remove 'register' keyword. 

*  [Remove 'register' keyword.](https://github.com/openexr/openexr/commit/6d297f35c5dbfacc8a5e94f33b986db7ab468db9) ([Larry Gritz](@lg@larrygritz.com), 2016-09-30) 'register' is a relic of K&R-era C, it's utterly useless in modern compilers. It's been deprecated in C++11, and therefore will generate warnings when encountered -- and many packages that use OpenEXR's public headers use -Werr to turn warnings into errors. Starting in C++17, the keyword is removed entirely, and thus will certainly be a build break for that version of the standard. So it's time for it to go. 

*  [Consistent check for IMF_HAVE_SSE2.](https://github.com/openexr/openexr/commit/7403524c8fed971383c724d85913b2d52672caf3) ([dirk](@dirk@git.imagemagick.org), 2016-09-17) 

*  [Merge pull request #141 from lucywilkes/develop](https://github.com/openexr/openexr/commit/c23f5345a6cc89627cc416b3e0e6b182cd427479) ([Ed Hanway](@ehanway-ilm@users.noreply.github.com), 2016-09-16) Adding rawPixelDataToBuffer() function for access to compressed scanlines 

*  [Merge pull request #198 from ZeroCrunch/develop](https://github.com/openexr/openexr/commit/891437f74805f6c8ebc897932091cbe0bb7e1163) ([Ed Hanway](@ehanway-ilm@users.noreply.github.com), 2016-08-02) Windows compile fix 

*  [Windows compile fix](https://github.com/openexr/openexr/commit/77faf005b50e8f77a8080676738ef9b9c807bf53) ([Jamie Kenyon](@jamie.kenyon@thefoundry.co.uk), 2016-07-29) std::min wasn't found due to <algorithm> not being included. 

*  [Merge pull request #179 from CAHEK7/NullptrBug](https://github.com/openexr/openexr/commit/a0a68393a4d3b622251fb7c490ee9d59e080b776) ([Ed Hanway](@ehanway-ilm@users.noreply.github.com), 2016-07-26) fix potential memory leak 

*  [Merge branch 'develop' of https://github.com/r-potter/openexr into r-potter-develop](https://github.com/openexr/openexr/commit/b206a243a03724650b04efcdf863c7761d5d5d5b) ([Ed Hanway](@ehanway@ilm.com), 2016-07-26) 

*  [Merge pull request #154 into develop](https://github.com/openexr/openexr/commit/bc372d47186db31d104e84e4eb9e84850819db8d) ([Ed Hanway](@ehanway@ilm.com), 2016-07-25) 

*  [Merge pull request #168 into develop](https://github.com/openexr/openexr/commit/44d077672f558bc63d907891bb88d741b334d807) ([Ed Hanway](@ehanway@ilm.com), 2016-07-25) 

*  [Merge pull request #175 into develop](https://github.com/openexr/openexr/commit/7513fd847cf38af89572cc209b03e5b548e6bfc8) ([Ed Hanway](@ehanway@ilm.com), 2016-07-25) 

*  [Merge pull request #174 into develop](https://github.com/openexr/openexr/commit/b16664a2ee4627c235b9ce798f4fc911e9c5694f) ([Ed Hanway](@ehanway@ilm.com), 2016-07-25) 

*  [Merge branch pull request 172 into develop: fix copy and paste bug in ImfDeepTiledInputPart.cpp](https://github.com/openexr/openexr/commit/ef7b78d5988d37dbbc74c21ad245ed5c80927223) ([Ed Hanway](@ehanway@ilm.com), 2016-07-25) 

*  [Merge pull request #195 from openexr/master](https://github.com/openexr/openexr/commit/bc234de193bd9cd32d94648e2936270aa4406e91) ([Ed Hanway](@ehanway-ilm@users.noreply.github.com), 2016-07-25) Catch develop branch up with commits in master. 

*  [fix potential memory leak](https://github.com/openexr/openexr/commit/d2f10c784d52f841b85e382620100cdbf0d3b1e5) ([CAHEK7](@ghosts.in.a.box@gmail.com), 2016-02-05) 

*  [Fix warnings when compiled with MSVC 2013.](https://github.com/openexr/openexr/commit/3aabef263083024db9e563007d0d76609ac8d585) ([Xo Wang](@xow@google.com), 2016-01-06) Similar fix to that from a27e048451ba3084559634e5e045a92a613b1455. 

*  [Fix typo in C bindings (Close #140)](https://github.com/openexr/openexr/commit/c229dfe63380f41dfae1e977b10dfc7c49c7efc7) ([Edward Kmett](@ekmett@gmail.com), 2015-12-09) IMF_RAMDOM_Y should be IMF_RANDOM_Y 

*  [Fix copy and paste bug](https://github.com/openexr/openexr/commit/501b654d851e2da1d9e5ca010a1e13fe34ae24ab) ([Christopher Kulla](@fpsunflower@users.noreply.github.com), 2015-11-19) The implementation of DeepTiledInputPart::tileXSize was copy and pasted from the function above but not changed. This causes it tor return incorrect values. 

*  [Switch AVX detection asm to not use an empty clobber list for use with older gcc versions](https://github.com/openexr/openexr/commit/51073d1aa8f96963fc6a3ecad8f844ce70c90991) ([Kevin Wheatley](@kevin.wheatley@framestore.com), 2015-10-14) 

*  [Merge pull request #145 from karlrasche/DWAx_clamp_float32](https://github.com/openexr/openexr/commit/521b25df787b460e57d5c1e831b232152b93a6ee) ([Ed Hanway](@ehanway-ilm@users.noreply.github.com), 2015-10-23) Clamp, don't cast, float inputs with DWAx compression 

*  [Merge pull request #143 from karlrasche/DWAx_bad_zigzag_order](https://github.com/openexr/openexr/commit/9547d38199f5db2712c06ccdda9195badbecccaa) ([Ed Hanway](@ehanway-ilm@users.noreply.github.com), 2015-10-23) Wrong zig-zag ordering used for DWAx decode optimization 

*  [Merge pull request #157 from karlrasche/DWAx_compress_bound](https://github.com/openexr/openexr/commit/de27156b77896aeef5b1c99edbca2bc4fa784b51) ([Ed Hanway](@ehanway-ilm@users.noreply.github.com), 2015-10-23) Switch over to use compressBound() instead of manually computing headroom for compress() 

*  [Switch over to use compressBound() instead of manually computing headroom for compress()](https://github.com/openexr/openexr/commit/c9a2e193ce243c66177ddec6be43bc6f655ff78a) ([Karl Rasche](@karl.rasche@dreamworks.com), 2015-02-18) 

*  [Fix a linker error when compiling OpenEXR statically on Linux](https://github.com/openexr/openexr/commit/caa09c1b361e2b152786d9e8b2b90261c9d9a3aa) ([Wenzel Jakob](@wenzel@inf.ethz.ch), 2015-02-02) Linking OpenEXR and IlmBase statically on Linux failed due to interdependencies between Iex and IlmThread. Simply reversing their order in CMakeLists.txt fixes the issue (which only arises on Linux since the GNU linker is particularly sensitive to the order of static libraries) 

*  [Clamp incoming float values to half, instead of simply casting, on encode.](https://github.com/openexr/openexr/commit/cb172eea58b8be078b88eca35f246e12df2de620) ([Karl Rasche](@karl.rasche@dreamworks.com), 2014-11-24) Casting can introduce Infs, which are zero'ed later on, prior to the forward DCT step. This can have the nasty side effect of forcing bright values to zero, instead of clamping them to 65k. 

*  [Remove errant whitespace](https://github.com/openexr/openexr/commit/fc67c8245dbff48e546abae027cc9c80c98b3db1) ([Karl Rasche](@karl.rasche@dreamworks.com), 2014-11-20) 

*  [Use the correct zig-zag ordering when finding choosing between fast-path inverse DCT versions (computing which rows are all zero)](https://github.com/openexr/openexr/commit/b0d0d47b65c5ebcb8c6493aa2238b9f890c4d7fe) ([Karl Rasche](@karl.rasche@dreamworks.com), 2014-11-19) 

*  [Resolve dependency issue building eLut.h/toFloat.h with CMake/Ninja.](https://github.com/openexr/openexr/commit/8eed7012c10f1a835385d750fd55f228d1d35df9) ([Ralph Potter](@r.potter@bath.ac.uk), 2014-11-05) 

*  [Adding rawPixelDataToBuffer() function for access to compressed data read from scanline input files.](https://github.com/openexr/openexr/commit/1f6eddeea176ce773dacd5cdee0cbad0ab549bae) ([Lucy Wilkes](@lucywilkes@users.noreply.github.com), 2014-10-22) Changes from The Foundry to add rawPixelDataToBuffer(...) function to the OpenEXR library. This allows you to read raw scan lines into an external buffer. It's similar to the existing function rawPixelData, but unlike this existing function it allows the user to control where the data will be stored instead of reading it into a local buffer. This means you can store multiple raw scan lines at once and enables the decompression of these scan lines to be done in parallel using an application's own threads. (cherry picked from commit ca76ebb40a3c5a5c8e055f0c8d8be03ca52e91c8) 

*  [Merge pull request #137 from karlrasche/interleaveByte2_sse_bug](https://github.com/openexr/openexr/commit/f4a6d3b9fabd82a11b63abf938e9e32f42d2d6d7) ([Ed Hanway](@ehanway-ilm@users.noreply.github.com), 2014-10-15) Fixing SSE2 byte interleaving path to work with short runs 

*  [Fixing SSE2 byte interleaving path to work with short runs](https://github.com/openexr/openexr/commit/da28ad8cd54dfa3becfdac33872c5b1401a9cc3c) ([Karl Rasche](@karl.rasche@dreamworks.com), 2014-09-08) 

*  [Merge pull request #126 from fnordware/LL_literal](https://github.com/openexr/openexr/commit/91015147e5a6a1914bcb16b12886aede9e1ed065) ([Ed Hanway](@ehanway-ilm@users.noreply.github.com), 2014-08-14) Use LL for 64-bit literals 

*  [Change suffixes to ULL because Int64 is unsigned](https://github.com/openexr/openexr/commit/353cbc2e89c582e07796f01bce8f203e84c8ae46) ([Brendan Bolles](@brendan@fnordware.com), 2014-08-14) As discusses in pull request #126 

*  [Merge pull request #127 from openexr/tarball_contents_fix](https://github.com/openexr/openexr/commit/699b4a62d5de9592d26f581a9cade89fdada7e6a) ([Ed Hanway](@ehanway-ilm@users.noreply.github.com), 2014-08-14) Tarball contents fix 

*  [Add dwa test images to dist (tarball) manifest. Also drop README.win32 from tarball. (Already removed from repo.)](https://github.com/openexr/openexr/commit/cbac202a84b0b0bac0fcd92e5b5c8d634085329e) ([Ed Hanway](@ehanway@ilm.com), 2014-08-14) [New Cmake-centric instructions covering builds for Windows and other platforms to follow.] 

*  [Use LL for 64-bit literals](https://github.com/openexr/openexr/commit/57ecf581d053f5cacf2e8fc3c024490e0bbe536f) ([Brendan Bolles](@brendan@fnordware.com), 2014-08-13) On a 32-bit architecture, these literals are too big for just a long, they need to be long long, otherwise I get an error in GCC.

## Version 2.2.1 (November 30, 2017)

This maintenance release addresses the reported OpenEXR security
vulnerabilities, specifically:

* [CVE-2017-9110](https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2017-9110)
* [CVE-2017-9111](https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2017-9111)
* [CVE-2017-9112](https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2017-9112)
* [CVE-2017-9113](https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2017-9113)
* [CVE-2017-9114](https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2017-9114)
* [CVE-2017-9115](https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2017-9115)
* [CVE-2017-9116](https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2017-9116)

## Version 2.2.0 (August 10, 2014)

This release includes the following components:

* OpenEXR: v2.2.0
* IlmBase: v2.2.0
* PyIlmBase: v2.2.0
* OpenEXR_Viewers: v2.2.0

This significant new features of this release include:

* **DreamWorks Lossy Compression** A new high quality, high performance
  lossy compression codec contributed by DreamWorks Animation. This
  codec allows control over variable lossiness to balance visual
  quality and file size. This contribution also includes performance
  improvements that speed up the PIZ codec.

* **IlmImfUtil** A new library intended to aid in development of image
  file manipulation utilities that support the many types of OpenEXR
  images.

This release also includes improvements to cross-platform build
support using CMake.

## Version 2.1.0 (November 25, 2013)

This release includes the following components (version locked):
* OpenEXR: v2.1.0
* IlmBase: v2.1.0
* PyIlmBase: v2.1.0
* OpenEXR_Viewers: v2.1.0

This release includes a refactoring of the optimised read paths for
RGBA data, optimisations for some of the python bindings to Imath,
improvements to the cmake build environment as well as additional
documentation describing deep data in more detail.

## Version 2.0.1 (July 11, 2013)

### Detailed Changes:

* Temporarily turning off optimisation code path (Piotr Stanczyk)
          
* Added additional tests for future optimisation refactoring (Piotr
	  Stanczyk / Peter Hillman)

* Fixes for StringVectors (Peter Hillman)

* Additional checks for type mismatches (Peter Hillman)
          
* Fix for Composite Deep Scanline (Brendan Bolles)

## Version 2.0 (April 9, 2013)

Industrial Light & Magic (ILM) and Weta Digital announce the release
of OpenEXR 2.0, the major version update of the open source high
dynamic range file format first introduced by ILM and maintained and
expanded by a number of key industry leaders including Weta Digital,
Pixar Animation Studios, Autodesk and others.

The release includes a number of new features that align with the
major version number increase. Amongst the major improvements are:

* **Deep Data support** - Pixels can now store a variable-length list of
  samples. The main rationale behind deep images is to enable the
  storage of multiple values at different depths for each
  pixel. OpenEXR 2.0 supports both hard-surface and volumetric
  representations for Deep Compositing workflows.

* **Multi-part Image Files** - With OpenEXR 2.0, files can now contain
  a number of separate, but related, data parts in one file. Access to
  any part is independent of the others, pixels from parts that are
  not required in the current operation don't need to be accessed,
  resulting in quicker read times when accessing only a subset of
  channels. The multipart interface also incorporates support for
  Stereo images where views are stored in separate parts. This makes
  stereo OpenEXR 2.0 files significantly faster to work with than the
  previous multiview support in OpenEXR.

* **Optimized pixel reading** - decoding RGB(A) scanline images has
  been accelerated on SSE processors providing a significant speedup
  when reading both old and new format images, including multipart and
  multiview files.

* **Namespacing** - The library introduces versioned namespaces to
  avoid conflicts between packages compiled with different versions of
  the library.

Although OpenEXR 2.0 is a major version update, files created by the
new library that don't exercise the new feature set are completely
backwards compatible with previous versions of the library. By using
the OpenEXR 2.0 library, performance improvements, namespace versions
and basic multi-part/deep reading support should be available to
applications without code modifications.

This code is designed to support Deep Compositing - a revolutionary
compositing workflow developed at Weta Digital that detached the
rendering of different elements in scene. In particular, changes in
one layer could be rendered separately without the need to re-render
other layers that would be required to handle holdouts in a
traditional comp workflow or sorting of layers in complex scenes with
elements moving in depth. Deep Compositing became the primary
compositing workflow on Avatar and has seen wide industry
adoption. The technique allows depth and color value to be stored for
every pixel in a scene allowing for much more efficient handling of
large complex scenes and greater freedom for artists to iterate.

True to the open source ethos, a number of companies contributed to
support the format and encourage adoption. Amongst others, Pixar
Animation Studios has contributed its DtexToExr converter to the
OpenEXR repository under a Microsoft Public License, which clears any
concerns about existing patents in the area, and Autodesk provided
performance optimizations geared towards real-time post-production
workflows.

Extensive effort has been put in ensuring all requirements were met to
help a wide adoption, staying true to the wide success of
OpenEXR. Many software companies were involved in the beta cycle to
insure support amongst a number of industry leading
applications. Numerous packages like SideFX's Houdini, Autodesk's
Maya, Solid Angle's Arnold renderer, Sony Pictures Imageworks' Open
Image IO have already announced their support of the format.

Open EXR 2.0 is an important step in the adoption of deep compositing
as it provides a consistent file format for deep data that is easy to
read and work with throughout a visual effects pipeline. The Foundry
has build OpenEXR 2.0 support into its Nuke Compositing application as
the base for the Deep Compositing workflows.

OpenEXR 2.0 is already in use at both Weta Digital and Industrial
Light & Magic. ILM took advantage of the new format on Marvel's The
Avengers and two highly anticipated summer 2013 releases, Pacific Rim
and The Lone Ranger. Recent examples of Weta Digital's use of the
format also include Marvel's Avengers as well as Prometheus and The
Hobbit. In addition, a large number of visual effects studios have
already integrated a deep workflow into their compositing pipelines or
are in the process of doing so including:, Sony Pictures Imageworks,
Pixar Animation Studios, Rhythm & Hues, Fuel and MPC.

In addition to visual effects, the new additions to the format, means
that depth data can also be assigned to two-dimensional data for a use
in many design fields including, architecture, graphic design,
automotive and product prototyping.

### Detailed Changes:

* Updated Documentation
	   (Peter Hillman)
* Updated Namespacing mechanism
	   (Piotr Stanczyk)
* Fixes for succd & predd
	   (Peter Hillman)
* Fixes for FPE control registers
	   (Piotr Stanczyk)
* Additional checks and tests on DeepImages, scanlines and tiles
	   (Peter Hillman)
* Folded in Autodesk read optimisations for RGB(A) files
	  (Pascal Jette, Peter Hillman)
* Updated the bootstrap scripts to use libtoolize if glibtoolize isn't available on darwin. 
	  (Nick Rasmussen)
* Numerous minor fixes, missing includes etc

## Version 2.0.0.beta.1 (June 15, 2012)

Development of OpenEXR v2 has been undertaken in a collaborative
environment (cf. previous github announcement) comprised of Industrial
Light & Magic, Weta Digital as well as a number of other contributors.

Some of the new features included in the Beta.1 release of OpenEXR v2
are:

* **Deep Data** Pixels can now store a variable length list of
  samples. The main rationale behind deep-images is to have multiple
  values at different depths for each pixel. OpenEXR v2 supports both
  hard surface and volumetric representation requirements for deep
  compositing workflows.

* **Multi-part image files** With OpenEXR v2, files can now contain a
    number of separate, but related, images in one file. Access to any
    part is independent of the others; in particular, no access of
    data need take place for unrequested parts.

In addition, OpenEXR v2 also contains platform independent mechanisms
for handling co-existing library version conflicts in the same process
space. (Currently implemented in IlmImf)

Finally, a reminder that this is a Beta release and potentially
incompatible changes may be introduced in future releases prior to the
v2.0.0 production version.

Please read the separate file for v2 additions and changes.

### Detailed Changes:

* Added git specific files 
	  (Piotr Stanczyk)
* Updated the so verison to 20
	  (Piotr Stanczyk)
* Initial use of the CMake build system 
	  (Nicholas Yue)

## Version 1.7.1 (July 31, 2012)

This release includes the following components:

* OpenEXR: v1.7.1
* IlmBase: v1.0.3
* PyIlmBase: v1.0.0 (introduces a Boost dependency)
* OpenEXR_Viewers: v1.0.2

Of particular note is the introduction of PyIlmBase. This module forms
a comprehensive set of python bindings to the IlmBase module.

In addition, contained in this release is a number of additions to
Imath as well as a minor tweak to Imath::Frustrum (for better support
for Windows platforms) as well as other minor fixes, including
correction for soname version of IlmImf.

## Version 1.7.0 (July 23, 2010)

This release includes support for stereoscopic images, please see the
adjoining documentation in the ``MultiViewOpenEXR.pdf``. (Many thanks
to Weta Digital for their contribution.) In addition, we added support
for targeting 64 bit Windows, fixes for buffer overruns and a number
of other minor fixes, additions and optimisations. Please see the
Changelog files for more detailed information.

### Bugs

This release addresses the following security vulnerabilities:

* [CVE-2009-1720](https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2009-1720)
* [CVE-2009-1721](https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2009-1721)
* [CVE-2009-1722](https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2009-1722)

### Detailed Changes:

* Added support for targetting builds on 64bit Windows and minimising
  number of compiler warnings on Windows. Thanks to Ger Hobbelt for
  his contributions to CreateDLL.  (Ji Hun Yu)
          
* Added new atttribute types (Florian Kainz):
  * **M33dAttribute** 3x3 double-precision matrix
  * **M44dAttribute** 4x4 double-precision matrix
  * **V2d** 2D double-precision vector
  * **V3d** 3D double-precision vector  
	  
* Bug fix: crash when reading a damaged image file (found by Apple).
  An exception thrown inside the PIZ Huffman decoder bypasses
  initialization of an array of pointers.  The uninitialized pointers
  are later passed to operator delete.  (Florian Kainz)

* Bug fix: crash when reading a damaged image file (found by Apple).
  Computing the size of input certain buffers may overflow and wrap
  around to a small number, later causing writes beyond the end of the
  buffer.  (Florian Kainz)

* In the "Technical Introduction" document, added Premultiplied
  vs. Un-Premulitiplied Color section: states explicitly that pixels
  with zero alpha and non-zero RGB are allowed, points out that
  preserving such a pixel can be a problem in application programs
  with un-premultiplied internal image representations.  (Florian
  Kainz)

* exrenvmap improvements:

  - New command line flags set the type of the input image to
    latitude-longitude map or cube-face map, overriding the envmap
    attribute in the input file header.

  - Cube-face maps can now be assembled from or split into six
    square sub-images.

  - Converting a cube-face map into a new cube-face map with the same
    face size copies the image instead of resampling it.  This avoids
    blurring when a cube-face map is assembled from or split into
    sub-images.  (Florian Kainz)

* Updated standard chromaticities in ImfAcesFile.cpp to match final
  ACES (Academy Color Encoding Specification) document.  (Florian
  Kainz)

* Added worldToCamera and worldToNDC matrices to
  ImfStandardAttributes.h (Florian Kainz)

* Increased the maximum length of attribute and channel names from 31
  to 255 characters.  For files that do contain names longer than 31
  characters, a new LONG_NAMES_FLAG in the fil version number is set.
  This flag causes older versions of the IlmImf library (1.6.1 and
  earlier) to reject files with long names.  Without the flag, older
  library versions would mis-interpret files with long names as
  broken.  (Florian Kainz)

* Reading luminance/chroma-encoded files via the RGBA interface is
  faster: buffer padding avoids cache thrashing for certain image
  sizes, redundant calls to saturation() have been eliminated.  (Mike
  Wall)

* Added "hemispherical blur" option to exrenvmap.  (Florian Kainz)

* Added experimental version of I/O classes for ACES file format
  (restricted OpenEXR format with special primaries and white point);
  added exr2aces file converter.  (Florian Kainz)

* Added new constructors to classes Imf::RgbaInputFile and
  Imf::TiledRgbaInputFile.  The new constructors have a layerName
  parameter, which allows the caller to specify which layer of a
  multi-layer or multi-view image will be read.  (Florian Kainz)

* A number of member functions in classes Imf::Header,
  Imf::ChannelList and Imf::FrameBuffer have parameters of type "const
  char *".  Added equivalent functions that take "const std::string &"
  parameters.  (Florian Kainz)

* Added library support for Weta Digital multi-view images:
  StringVector attribute type, multiView standard attribute of type
  StringVector, utility functions related to grouping channels into
  separate views.  (Peter Hillman, Florian Kainz)

## Version 1.6.1 (October 22, 2007)

This release fixes a buffer overrun in OpenEXR and a Windows build
problem in CTL, and it removes a few unnecessary files from the
.tar.gz packages.

### Detailed Changes:

* Removed Windows .suo files from distribution.  (Eric Wimmer)

* Bug fix: crashes, memory leaks and file descriptor leaks when
  reading damaged image files (some reported by Apple, others found by
  running IlmImfFuzzTest).  (Florian Kainz)
          
* Added new IlmImfFuzzTest program to test how resilient the IlmImf
  library is with respect broken input files: the program first
  damages OpenEXR files by partially overwriting them with random
  data; then it tries to read the damaged files.  If all goes well,
  the program doesn't crash.  (Florian Kainz)

## Version 1.6.0 (August 3, 2007)

OpenEXR 1.6.0:

* Reduced generational loss in B44- and B44A-compressed images.

* Added B44A compression. This is a variation of B44, but with a
  better compression ratio for images with large uniform areas, such
  as in an alpha channel.

* Bug fixes.

CTL 1.4.0:

* Added new functions to the CTL standard library: 3x3 matrix support,
  1D lookup tables with cubic interpolation.

* Added new "ctlversion" statement to the language.

* Bug fixes.

OpenEXR_CTL 1.0.0:

* Applying CTL transforms to a frame buffer is multi-threaded.
Bug fixes.

OpenEXR_Viewers 1.0.0:

* Implemented new naming conventions for CTL parameters.

IlmBase 1.0.0:

* Half now implements "round to nearest even" mode.

### Detailed Changes:

* Bumped DSO version number to 6.0 (Florian Kainz)

* Added new standard attributes related to color rendering with CTL
  (Color Transformation Language): renderingTransform,
  lookModTransform and adoptedNeutral.  (Florian Kainz)

* Bug fix: for pixels with luminance near HALF_MIN, conversion from
  RGB to luminance/chroma produces NaNs and infinities (Florian Kainz)
          
* Bug fix: excessive desaturation of small details with certain colors
  after repeatedly loading and saving luminance/chroma encoded images
  with B44 compression.  (Florian Kainz)

* Added B44A compression, a minor variation of B44: in most cases, the
  compression ratio is 2.28:1, the same as with B44, but in uniform
  image areas where all pixels have the same value, the compression
  ratio increases to 10.66:1.  Uniform areas occur, for example, in an
  image's alpha channel, which typically contains large patches that
  are solid black or white, or in computer- generated images with a
  black background.  (Florian Kainz)

* Added flag to configure.ac to enable or disable use of large auto
  arrays in the IlmImf library.  Default is "enable" for Linux,
  "disable" for everything else.  (Darby Johnston, Florian Kainz)

* corrected version number on dso's (libtool) - now 5.0

* Separated ILMBASE_LDFLAGS and ILMBASE_LIBS so that test programs can
  link with static libraries properly

* eliminated some warning messages during install (Andrew Kunz)
	
## Version 1.5.0 (December 15, 2006)

The new version includes several significant changes:

* OpenEXR supports a new image compression method, called B44. It has
  a fixed compression rate of 2.28:1, or 4.57:1 if used in combination
  with luminance/chroma encoding. B44-compressed images can be
  uncompressed fast enough to support real-time playback of image
  sequences.

* The new playexr program plays back moving image sequences. Playexr
  is multi-threaded and utilizes the threading capabilities of the
  IlmImf library that were introduced in OpenEXR 1.3.0. The program
  plays back B44-compressed images with fairly high-resolution in real
  time on commodity hardware.

* The playexr program and a new version of the existing exrdisplay
  image viewer both support color rendering via color transforms
  written in the new Color Transformation Language or CTL. CTL is not
  part of OpenEXR; it will be released separately. CTL support in
  playexr and exrdisplay is optional; the programs can be built and
  will run without CTL.

* In preparation for the release of CTL, OpenEXR has been split into
  three separate packages:

  * IlmBase 0.9.0 includes the Half, Iex, Imath and IlmThread libraries

  * OpenEXR 1.5.0 includes the IlmImf library, programming examples and utility programs such as exrheader or exrenvmap

  * OpenEXRViewers 0.9.0 includes the playexr and exrdisplay programs

* The "Technical Introduction to OpenEXR" document now includes a
  recommendation for storing CIE XYZ pixel data in OpenEXR files.

* A new "OpenEXR Image Viewing Software" document describes the
  playexr and exrdisplay programs. It briefly explains real-time
  playback and color rendering, and includes recommendations for
  testing if other image viewing software displays OpenEXR images
  correctly.

* The OpenEXR sample image set now includes B44-compressed files and
  files with CIE XYZ pixel data.

### Detailed Changes:  

* reorganized packaging of OpenEXR libraries to facilitate integration
  with CTL.  Now this library depends on the library IlmBase.  Some
  functionality has been moved into OpenEXR_Viewers, which depends on
  two other libraries, CTL and OpenEXR_CTL.  Note: previously there
  were separate releases of OpenEXR-related plugins for Renderman,
  Shake and Photoshop.  OpenEXR is supported natively by Rendermand
  and Photoshop, so these plugins will not be supported for this or
  future versions of OpenEXR.  (Andrew Kunz)

* New build scripts for Linux/Unix (Andrew Kunz)

* New Windows project files and build scripts (Kimball Thurston)

* float-to-half conversion now preserves the sign of float zeroes and
  of floats that are so small that they become half zeroes.  (Florian
  Kainz)

* Bug fix: Imath::Frustum<T>::planes() returns incorrect planes if the
  frustum is orthogonal.  (Philip Hubbard)

* added new framesPerSecond optional standard attribute (Florian
  Kainz)

* Imath cleanup:

  - Rewrote function Imath::Quat<T>::setRotation() to make it
    numerically more accurate, added confidence tests

  - Rewrote function Imath::Quat<T>::slerp() using Don Hatch's method,
    which is numerically more accurate, added confidence tests.

  - Rewrote functions Imath::closestPoints(), Imath::intersect(),
    added confidence tests.

  - Removed broken function Imath::nearestPointOnTriangle().

  - Rewrote Imath::drand48(), Imath::lrand48(), etc. to make them
    functionally identical with the Unix/Linux versions of drand48(),
    lrand48() and friends.

  - Replaced redundant definitions of Int64 in Imath and IlmImf with a
    single definition in ImathInt64.h.  (Florian Kainz)

* exrdisplay: if the file's and the display's RGB chromaticities
  differ, the pixels RGB values are transformed from the file's to the
  display's RGB space.  (Florian Kainz)

* Added new lossy B44 compression method.  HALF channels are
  compressed with a fixed ratio of 2.28:1.  UINT and FLOAT channels
  are stored verbatim, without compression.  (Florian Kainz)

## Version 1.4.0a (August 9, 2006)

* Fixed the ReleaseDLL targets for Visual Studio 2003.  (Barnaby Robson)
	
## Version 1.4.0 (August 2, 2006)	

 This is the next major production-ready release of OpenEXR and offers
 full compatibility with our last production release, which was
 1.2.2. This version obsoletes versions 1.3.x, which were test
 versions for 1.4.0. If you have been using 1.3.x, please upgrade to
 1.4.0.

* Production release.

* Bug Fix: calling setFrameBuffer() for every scan line while reading
  a tiled file through the scan line API returns bad pixel data. (Paul
  Schneider, Florian Kainz)

## Version 1.3.1 (June 14, 2006)

* Fixed the ReleaseDLL targets for Visual Studio 2005.  (Nick Porcino, Drew Hess)

* Fixes/enhancements for createDLL.  (Nick Porcino)
	
## Version 1.3.0 (June 8, 2006)

This is a test release. The major new feature in this version is
support for multithreaded file I/O. We've been testing the threaded
code internally at ILM for a few months, and we have not encountered
any bugs, but we'd like to get some feedback from others before we
release the production version.

Here's a summary of the changes since version 1.2.2:

* Support for multithreaded file reading and writing.

* Support for Intel-based OS X systems.

* Support for Visual Studio 2005.

* Better handling of **PLATFORM_** and **HAVE_** macros.

* Updated documentation.

* Bug fixes related to handling of incomplete and damaged files.

* Numerous bug fixes and cleanups to the autoconf-based build system.

* Removed support for the following configurations that were
  previously supported. Some of these configurations may happen to
  continue to function, but we can't help you if they don't, largely
  because we don't have any way to test them:

  * IRIX
  * OSF1
  * SunOS
  * OS X versions prior to 10.3.
  * gcc on any platform prior to version 3.3

### Detailed Changes:

* Removed openexr.spec file, it's out of date and broken to boot.
 (Drew Hess)
          
* Support for Visual Studio 2005.  (Drew Hess, Nick Porcino)

* When compiling against OpenEXR headers on Windows, you no longer
  need to define any **HAVE_** or **PLATFORM_** macros in your
  projects.  If you are using any OpenEXR DLLs, however, you must
  define OPENEXR_DLL in your project's preprocessor directives.  (Drew
  Hess)

* Many fixes to the Windows VC7 build system.  (Drew Hess, Nick
  Porcino)

* Support for building universal binaries on OS X 10.4.  (Drew Hess,
Paul Schneider)
          
* Minor configure.ac fix to accomodate OS X's automake.  (Drew Hess)
          
* Removed CPU-specific optimizations from configure.ac, autoconf's
	  guess at the CPU type isn't very useful, anyway.  Closes
	  #13429.  (Drew Hess)
          
* Fixed quoting for tests in configure.ac.  Closes #13428.  (Drew
  Hess)
          
* Use host specification instead of target in configure.ac.  Closes
  #13427.  (Drew Hess)

* Fix use of AC_ARG_ENABLE in configure.ac.  Closes #13426.  (Drew
Hess)

* Removed workaround for OS X istream::read bug.  (Drew Hess)
          
* Added pthread support to OpenEXR pkg-config file.  (Drew Hess)
          
* Added -no-undefined to LDFLAGS and required libs to LIBADD for
  library projects with other library dependencies, per Rex Dieter's
  patch.  (Drew Hess)
          
* **HAVE_** macros are now defined in the OpenEXRConfig.h header file
  instead of via compiler flags.  There are a handful of public
  headers which rely on the value of these macros, and projects
  including these headers have previously needed to define the same
  macros and values as used by OpenEXR's 'configure', which is bad
  form.  Now 'configure' writes these values to the OpenEXRConfig.h
  header file, which is included by any OpenEXR source files that need
  these macros.  This method of specifying **HAVE_** macros guarantees
  that projects will get the proper settings without needing to add
  compile- time flags to accomodate OpenEXR.  Note that this isn't
  implemented properly for Windows yet.  (Drew Hess)

* Platform cleanups:

  - No more support for IRIX or OSF1.

  - No more explicit support for SunOS, because we have no way to
    verify that it's working.  I suspect that newish versions of SunOS
    will just work out of the box, but let me know if not.

  - No more **PLATFORM_** macros (vestiges of the ILM internal build
    system).  PLATFORM_DARWIN_PPC is replaced by HAVE_DARWIN.
    PLATFORM_REDHAT_IA32 (which was only used in IlmImfTest) is
    replaced by HAVE_LINUX_PROCFS.

  - OS X 10.4, which is the minimum version we're going to support
    with this version, appears to have support for nrand48 and
    friends, so no need to use the Imath-supplied version of them
    anymore.  (Drew Hess)

* No more PLATFORM_WINDOWS or PLATFORM_WIN32, replace with proper
  standard Windows macros.  (Drew Hess)

* Remove support for gcc 2.95, no longer supported.  (Drew Hess)

* Eliminate HAVE_IOS_BASE macro, OpenEXR now requires support for
  ios_base.  (Drew Hess)

* Eliminate HAVE_STL_LIMITS macro, OpenEXR now requires the ISO C++
  <limits> header.  (Drew Hess)

* Use double quote-style include dirctives for OpenEXR
  includes.  (Drew Hess)

* Added a document that gives an overview of the on-disk
  layout of OpenEXR files (Florian Kainz)

* Added sections on layers and on memory-mapped file input
  to the documentation.  (Florian Kainz)

* Bug fix: reading an incomplete file causes a deadlock while
  waiting on a semaphore.  (Florian Kainz)

* Updated documentation (ReadingAndWritingImageFiles.sxw) and sample
  code (IlmImfExamples): Added a section about multi-threading,
  updated section on thread-safety, changed documentation and sample
  code to use readTiles()/writeTiles() instead of
  readTile()/writeTile() where possible, mentioned that environment
  maps contain redundant pixels, updated section on testing if a file
  is an OpenEXR file.  (Florian Kainz)

* Multi-threading bug fixes (exceptions could be thrown multiple
  times, some operations were not thread safe), updated some comments,
  added comments, more multithreaded testing.  (Florian Kainz)

* Added multi-threading support: multiple threads
  cooperate to read or write a single OpenEXR file.
  (Wojciech Jarosz)

* Added operator== and operator!= to Imath::Frustum. (Andre Mazzone)

* Bug fix: Reading a PIZ-compressed file with an invalid Huffman code
  table caused crashes by indexing off the end of an array.  (Florian
  Kainz)

## Version 1.2.2 (March 15, 2005)

This is a relatively minor update to the project, with the following changes:

* New build system for Windows; support for DLLs.

* Switched documentation from HTML to PDF format.

* IlmImf: support for image layers in ChannelList.

* IlmImf: added isComplete() method to file classes to check whether a file is complete.

* IlmImf: exposed staticInitialize() in ImfHeader.h in order to allow
  thread-safe library initialization in multithreaded applications.

* IlmImf: New "time code" standard attribute.

* exrdisplay: support for displaying wrap-around texture map images.

* exrmaketiled: can now specify wrap mode.

* IlmImf: New "wrapmodes" standard attribute to indicate extrapolation
  mode for mipmaps and ripmaps.

* IlmImf: New "key code" standard attribute to identify motion picture
  film frames.

* Imath: Removed TMatrix<T> classes; these classes are still under
  development and are too difficult to keep in sync with OpenEXR CVS.

### Detailed Changes:


* Updated README to remove option for building with Visual C++ 6.0.
	  (Drew Hess)

* Some older versions of gcc don't support a full iomanip
	  implemenation; check for this during configuration. 
	  (Drew Hess)

* Install PDF versions of documentation, remove old/out-of-date
	  HTML documentation.  (Florian Kainz)

* Removed vc/vc6 directory; Visual C++ 6.0 is no longer
	  supported.  (Drew Hess)

* Updated README.win32 with details of new build system.
	  (Florian Kainz, Drew Hess)

* New build system for Windows / Visual C++ 7 builds both
	  static libraries and DLLs.
	  (Nick Porcino)

* Removed Imath::TMatrix<T> and related classes, which are not
	  used anywhere in OpenEXR.
	  (Florian Kainz)

* Added minimal support for "image layers" to class Imf::ChannelList
	  (Florian Kainz)

* Added new isComplete() method to InputFile, TiledInputFile
	  etc., that checks if a file is complete or if any pixels
	  are missing (for example, because writing the file was
	  aborted prematurely).
	  (Florian Kainz)

* Exposed staticInitialize() function in ImfHeader.h in order
	  to allow thread-safe library initialization in multithreaded
	  programs.
	  (Florian Kainz)

* Added a new "time code" attribute
	  (Florian Kainz)

* exrmaketiled: when a MIPMAP_LEVELS or RIPMAP_LEVELS image
	  is produced, low-pass filtering takes samples outside the
	  image's data window.  This requires extrapolating the image.
	  The user can now specify how the image is extrapolated
	  horizontally and vertically (image is surrounded by black /
	  outermost row of pixels repeats / entire image repeats /
	  entire image repeats, every other copy is a mirror image).
	  exrdisplay: added option to swap the top and botton half,
	  and the left and right half of an image, so that the image's
	  four corners end up in the center.  This is useful for checking
	  the seams of wrap-around texture map images.
	  IlmImf library: Added new "wrapmodes" standard attribute
	  to indicate the extrapolation mode for MIPMAP_LEVELS and
	  RIPMAP_LEVELS images.
	  (Florian Kainz)

* Added a new "key code" attribute to identify motion picture
	  film frames.
	  (Florian Kainz)

* Removed #include <Iex.h> from ImfAttribute.h, ImfHeader.h
	  and ImfXdr.h so that including header files such as
	  ImfInputFile.h no longer defines ASSERT and THROW macros,
	  which may conflict with similar macros defined by
	  application programs.
	  (Florian Kainz)

* Converted HTML documentation to OpenOffice format to
	  make maintaining the documents easier:
	      api.html -> ReadingAndWritingImageFiles.sxw
	      details.html -> TechnicalIntroduction.sxw
	  (Florian Kainz)

## Version 1.2.1 (June 6, 2004)

This is a fairly minor release, mostly just a few tweaks, a few bug
fixes, and some new documentation. Here are the most important
changes:

* reduced memory footprint of exrenvmap and exrmaketiled utilities.

* IlmImf: new helper functions to determine whether a file is an OpenEXR file, and whether it's scanline- or tile-based.

* IlmImf: bug fix for PXR24 compression with ySampling != 1.

* Better support for gcc 3.4.

* Warning cleanups in Visual C++.

### Detailed Changes:

* exrenvmap and exrmaketiled use slightly less memory
	  (Florian Kainz)

* Added functions to IlmImf for quickly testing if a file
	  is an OpenEXR file, and whether the file is scan-line
	  based or tiled. (Florian Kainz)

* Added preview image examples to IlmImfExamples.  Added
	  description of preview images and environment maps to
	  docs/api.html (Florian Kainz)

* Bug fix: PXR24 compression did not work properly for channels
	  with ySampling != 1.
	  (Florian Kainz)

* Made ``template <class T>`` become ``template <class S, class T>`` for 
          the ``transform(ObjectS, ObjectT)`` methods. This was done to allow
          for differing templated objects to be passed in e.g.  say a 
          ``Box<Vec3<S>>`` and a ``Matrix44<T>``, where S=float and T=double.
          (Jeff Yost, Arkell Rasiah)

* New method Matrix44::setTheMatrix(). Used for assigning a 
          M44f to a M44d. (Jeff Yost, Arkell Rasiah)

* Added convenience Color typedefs for half versions of Color3
          and Color4. Note the Makefile.am for both Imath and ImathTest
          have been updated with -I and/or -L pathing to Half.
          (Max Chen, Arkell Rasiah)

* Methods equalWithAbsError() and equalWithRelError() are now
          declared as const. (Colette Mullenhoff, Arkell Rasiah)

* Fixes for gcc34. Mainly typename/template/using/this syntax
          correctness changes. (Nick Ramussen, Arkell Rasiah)

* Added Custom low-level file I/O examples to IlmImfExamples
	  and to the docs/api.html document.  (Florian Kainz)

* Eliminated most warnings messages when OpenEXR is compiled
	  with Visual C++.  The OpenEXR code uses lots of (intentional
	  and unintended) implicit type conversions.  By default, Visual
	  C++ warns about almost all of them.  Most implicit conversions
	  have been removed from the .h files, so that including them
	  should not generate warnings even at warning level 3.  Most
	  .cpp files are now compiled with warning level 1.
	  (Florian Kainz)

## Version 1.2.0 (May 11, 2004)

OpenEXR 1.2.0 is now available. This is the first official,
production-ready release since OpenEXR 1.0.7. If you have been using
the development 1.1 series, please switch to 1.2.0 as soon as
possible. We believe that OpenEXR 1.2.0 is ready for use in shipping
applications. We have been using it in production at ILM for several
months now with no problems. There are quite a few major new features
in the 1.2 series as compared to the original 1.0 series:

* Support for tiled images, including mipmaps and ripmaps. Note that
  software based on the 1.0 series cannot read or write tiled
  images. However, simply by recompiling your software against the 1.2
  release, any code that reads scanline images can read tiled images,
  too.

* A new Pxr24 compressor, contributed by Pixar Animation
  Studios. Values produced by the Pxr24 compressor provide the same
  range as 32-bit floating-point numbers with slightly less precision,
  and compress quite a bit better. The Pxr24 compressor stores UINT
  and HALF channels losslessly, and for these data types performs
  similarly to the ZIP compressor.

* OpenEXR now supports high dynamic-range YCA (luminance/chroma/alpha)
  images with subsampled chroma channels. These files are supported
  via the RGBA convenience interface, so that data is presented to the
  application as RGB(A) but stored in the file as YC(A). OpenEXR also
  supports Y and YA (black-and-white/black-and-white with alpha)
  images.

* An abstracted file I/O interface, so that you can use OpenEXR with
  interfaces other than C++'s iostreams.

* Several new utilities for manipulating tiled image files.

### Detailed Changes:

* Production-ready release.

* Disable long double warnings on OS X.  (Drew Hess)

* Add new source files to VC7 IlmImfDll target.  (Drew Hess)

* Iex: change the way that APPEND_EXC and REPLACE_EXC modify
	  their what() string to work around an issue with Visual C++
	  7.1.  (Florian Kainz, Nick Porcino)

* Bumped OpenEXR version to 1.2 and .so versions to 2.0.0 in
	  preparation for the release.  (Drew Hess)

* Imath: fixed ImathTMatrix.h to work with gcc 3.4.  (Drew Hess)

* Another quoting fix in openexr.m4.  (Drew Hess)

* Quoting fix in acinclude.m4 for automake 1.8.  (Brad Hards)

* Imath: put inline at beginning of declaration in ImathMatrix.h
	  to fix a warning.  (Ken McGaugh)

* Imath: made Vec equalWithError () methods const.

* Cleaned up compile-time Win32 support.  (Florian Kainz)

* Bug fix: Reading a particular broken PIZ-compressed file
	  caused crashes by indexing off the end of an array.
	  (Florian Kainz)

## Version 1.1.1 (March 27, 2004)

OpenEXR 1.1.1 is now available. This another development release. We
expect to release a stable version, 1.2, around the end of
April. Version 1.1.1 includes support for PXR24 compression, and for
high-dynamic-range luminance/chroma images with subsampled chroma
channels. Version 1.1.1 also fixes a bug in the 1.1.0 tiled file
format.

### Detailed Changes:

* Half: operator= and variants now return by reference rather
	  than by value.  This brings half into conformance with
	  built-in types.  (Drew Hess)

* Half: remove copy constructor, let compiler supply its
	  own.  This improves performance up to 25% on some
	  expressions using half.  (Drew Hess)

* configure: don't try to be fancy with CXXFLAGS, just use
	  what the user supplies or let configure choose a sensible
	  default if CXXFLAGS is not defined.

* IlmImf: fixed a bug in reading scanline files on big-endian
          architectures.  (Drew Hess)

* exrmaketiled: Added an option to select compression type.
	  (Florian Kainz)

* exrenvmap: Added an option to select compression type.
	  (Florian Kainz)

* exrdisplay: Added some new command-line options.  (Florian Kainz)

* IlmImf: Added Pixar's new "slightly lossy" image compression
	  method.  The new method, named PXR24, preserves HALF and
	  UINT data without loss, but FLOAT pixels are converted to
	  a 24-bit representation.  PXR24 appears to compress
	  FLOAT depth buffers very well without losing much accuracy.
	  (Loren Carpenter, Florian Kainz)

* Changed top-level LICENSE file to allow for other copyright
	  holders for individual files.

* IlmImf: TILED FILE FORMAT CHANGE.  TiledOutputFile was
	  incorrectly interleaving channels and scanlines before
	  passing pixel data to a compressor.  The lossless compressors
	  still work, but lossy compressors do not.  Fix the bug by
	  interleaving channels and scanlines in tiled files in the
	  same way as ScanLineOutputFile does.  Programs compiled with
	  the new version of IlmImf cannot read tiled images produced
	  with version 1.1.0.  (Florian Kainz)

* IlmImf: ImfXdr.h fix for 64-bit architectures.  (Florian Kainz)

* IlmImf: OpenEXR now supports YCA (luminance/chroma/alpha)
	  images with subsampled chroma channels.  When an image
	  is written with the RGBA convenience interface, selecting
	  WRITE_YCA instead of WRITE_RGBA causes the library to
	  convert the pixels to YCA format.  If WRITE_Y is selected,
	  only luminance is stored in the file (for black and white
	  images).  When an image file is read with the RGBA convenience
	  interface, YCA data are automatically converted back to RGBA.
	  (Florian Kainz)

* IlmImf: speed up reading tiled files as scan lines.
	  (Florian Kainz)

* Half:  Fixed subtle bug in Half where signaling float NaNs
	  were being converted to inf in half.  (Florian Kainz)

* gcc 3.3 compiler warning cleanups.  (various)

* Imath: ImathEuler.h fixes for gcc 3.4.  (Garrick Meeker)
	
## Version 1.1.0 (February 6, 2004)

 OpenEXR 1.1.0 is now available. This is a major new release with
 support for tiled images, multi-resolution files (mip/ripmaps),
 environment maps, and abstracted file I/O. We've also released a new
 set of images that demonstrate these features, and updated the
 CodeWarrior project and Photoshop plugins for this release. See the
 downloads section for the source code and the new images.

### Detailed Changes:

* Added new targets to Visual C++ .NET 2003 project
	  for exrmaketiled, exrenvmap, exrmakepreview, and exrstdattr.
	  (Drew Hess)

* A few assorted Win32 fixes for Imath.  (Drew Hess)

* GNU autoconf builds now produce versioned libraries.
	  This release is 1:0:0.  (Drew Hess)

* Fixes for Visual C++ .NET 2003.  (Paul Schneider)

* Updated Visual C++ zlib project file to zlib 1.2.1.
	  (Drew Hess)

* exrdisplay: Fixed fragment shader version.  (Drew Hess)

* *Test: Fixed some compiler issues.  (Drew Hess)

* Imath: Handle "restrict" keyword properly.  (Drew Hess)

* IlmImfExamples: Updated to latest versions of example
	  source code, includes tiling and multi-res images.
	  (Florian Kainz)

* exrmakepreview: A new utility to create preview images.
	  (Florian Kainz)

* exrenvmap: A new utility to create OpenEXR environment
	  maps.  (Florian Kainz)

* exrstdattr: A new utility to modify standard 
	  attributes.  (Florian Kainz)

* Updated exrheader to print level rounding mode and
	  preview image size.  (Florian Kainz)

* Updated exrmaketiled to use level rounding mode.
	  (Florian Kainz)

* IlmImf: Changed the orientation of lat-long envmaps to
	  match typical panoramic camera setups.  (Florian Kainz)

* IlmImf: Fixed a bug where partially-completed files with
	  DECREASING_Y could not be read.  (Florian Kainz)

* IlmImf: Added support for selectable rounding mode (up/down)
	  when generating multiresolution files.  (Florian Kainz)

* exrdisplay: Support for tiled images, mip/ripmaps, preview
	  images, and display windows.  (Florian Kainz, Drew Hess)

* exrmaketiled: A new utility which generates tiled
	  versions of OpenEXR images.  (Florian Kainz)

* IlmImf: Changed Imf::VERSION to Imf::EXR_VERSION to
	  work around problems with autoconf VERSION macro
	  conflict.  (Drew Hess)

* exrheader: Support for tiles, mipmaps, environment
	  maps.  (Florian Kainz)

* IlmImf: Environment map support.  (Florian Kainz)

* IlmImf: Abstracted stream I/O support.  (Florian Kainz)

* IlmImf: Support for tiled and mip/ripmapped files;
	  requires new file format.  (Wojciech Jarosz, Florian Kainz)

* Imath: **TMatrix**, generic 2D matricies and algorithms.
	  (Francesco Callari)

* Imath: major quaternions cleanup.  (Cary Phillips)

* Imath: added GLBegin, GLPushAttrib, GLPushMatrix objects
	  for automatic cleanup on exceptions.  (Cary Phillips)

* Imath: removed implicit scalar->vector promotions and vector
	  comparisons.  (Nick Rasmussen)
	
## Version 1.0.7 (January 7, 2004)

OpenEXR 1.0.7 is now available. In addition to some bug fixes, this
version adds support for some new standard attributes, such as primary
and white point chromaticities, lens aperture, film speed, image
acquisition time and place, and more. If you want to use these new
attributes in your applications, see the ImfStandardAttributes.h
header file for documentation.

Our project hosting site, Savannah, is still recovering from a
compromise last month, so in the meantime, we're hosting file
downloads here. Some of the files are not currently available, but
we're working to restore them.

### Detailed Changes:

* Fixed a typo in one of the IlmImfTest tests. (Paul Schneider)

* Fixed a bug in exrdisplay that causes the image to display
	  as all black if there's a NaN or infinity in an OpenEXR
	  image. (Florian Kainz)

* Updated exrheader per recent changes to IlmImf library.
	  (Florian Kainz)

* Changed an errant float to a T in ImathFrame.h nextFrame().
	  (Cary Phillips)

* Support for new "optional standard" attributes
	  (chromaticities, luminance, comments, etc.).
	  (Florian Kainz, Greg Ward, Joseph Goldstone)

* Fixed a buffer overrun in ImfOpaqueAttribute. (Paul Schneider)

* Added new function, isImfMagic (). (Florian Kainz)
	
## Version 1.0.6:

* Added README.win32 to disted files.

* Fixed OpenEXR.pc.in pkg-config file, OpenEXR now works
	  with pkg-config.

* Random fixes to readme files for new release.

* Fixed openexr.m4, now looks in /usr by default.

* Added Visual Studio .NET 2003 "solution."

* Fixes for Visual Studio .NET 2003 w/ Microsoft C++ compiler.
	  (Various)

* Random Imath fixes and enhancements.  Note that 
	  extractSHRT now takes an additional optional
          argument, see ImathMatrixAlgo.h for details.  (Various)

* Added Wojciech Jarosz to AUTHORS file.

* Added test cases for uncompressed case, preview images,
	  frame buffer type conversion.  (Wojciech Jarosz,
	  Florian Kainz)

* Fix a bug in IlmImf where uncompressed data doesn't get
	  read/written correctly.  (Wojciech Jarosz)

* Added support for preview images and preview image
	  attributes (thumbnail images) in IlmImf.  (Florian Kainz)

* Added support for automatic frame buffer type conversion
	  in IlmImf.  (Florian Kainz)

* Cleaned up some compile-time checks.

* Added HalfTest unit tests.

* [exrdisplay] Download half framebuffer to texture memory 
	  instead of converting to float first.  Requires latest
	  Nvidia drivers.

## Version 1.0.5 (April 3, 2003)

Industrial Light & Magic has released the source code for an OpenEXR
display driver for Pixar's Renderman. This display driver is covered
under the OpenEXR free software license. See the downloads section for
the source code.

### Detailed Changes:

* Fixed IlmImf.dll to use static runtime libs (Andreas).

* Added exrheader project to Visual Studio 6.0 workspace.

* Added some example code showing how to use the IlmImf library.
	  (Florian)

* Use DLL runtime libs for Win32 libraries rather than static
	  runtime libs.

* Add an exrdisplay_fragshader project to the Visual Studio 6.0
	  workspace to enable fragment shaders in Win32.

* Add an IlmImfDll project to the Visual Studio 6.0 workspace.

* In Win32, export the ImfCRgbaFile C interface via a DLL so
	  that Visual C++ 6.0 users can link against an Intel-compiled
	  IlmImf.  (Andreas Kahler)

* Use auto_ptr in ImfAutoArray on Win32, it doesn't like large 
	  automatic stacks.

* Performance improvements in PIZ decoding, between
	  20 and 60% speedup on Athlon and Pentium 4 systems.
          (Florian)

* Updated the README with various information, made
	  some cosmetic changes for readability.

* Added fragment shader support to exrdisplay.

* Bumped the version to 1.0.5 in prep for release.

* Updated README and README.OSX to talk about CodeWarrior 
          project files.

* Incorporated Rodrigo Damazio's patch for an openexr.m4
	  macro file and an openexr.spec file for building RPMs.

* Small change in ImfAttribute.h to make IlmImf compile with gcc 2.95.

* Updated ImfDoubleAttribute.h for Codewarrior on MacOS.

* Added exrheader utility.

* Update to AUTHORS file.

* Added a README.win32 file.

* Added project files for Visual Studio 6.0.

* Initial Win32 port.  Requires Visual Studio 6.0 and Intel C++
	  compiler version 7.0.

* Added new intersectT method in ImathSphere.h

* Fixed some bugs in ImathQuat.h

* Proper use of fltk-config to get platform-specific FLTK
	  compile- and link-time flags.

* exrdisplay uses Imath::Math<T>::pow instead of powf now.
	  powf is not availble on all platforms.

* Roll OS X "hack" into the source until Apple fixes their
	  istream implementation.
	
## Version 1.0.4

### Detailed Changes:

* OpenEXR is now covered by a modified BSD license.  See LICENSE
	  for the new terms.

## Version 1.0.3:

### Detailed Changes:

* OpenEXR is now in sf.net CVS.

* Imf::Xdr namespace cleanups.

* Some IlmImfTest cleanups for OS X.

* Use .cpp extension in exrdisplay sources.

* Iex cleanups.

* Make IlmImf compile with Metrowerks Codewarrior.

* Change large automatic stacks in ImfHuf.C to auto_ptrs allocated
	  off the heap.  MacOS X default stack size isn't large enough.

* std::ios fix for MacOS X in ImfInputFile.C.

* Added new FP predecessor/successor functions to Imath, added
	  tests to ImathTest

* Fixed a bug in Imath::extractSHRT for 3x3 matricies when
	  exactly one of the original scaling factors is negative, updated
	  ImathTest to check this case.

* Install include files when 'make install' is run.

* exrdisplay requires fltk 1.1+ now in an effort to support
	  a MacOS X display program (fltk 1.1 runs on OS X), though this
	  is untested.

* renamed configure.in to configure.ac

* Removed some tests from IexTest that are no longer used.

* Removed ImfHalfXdr.h, it's not used anymore.

* Revamped the autoconf system, added some compile-time 
          optimizations, a pkgconfig target, and some maintainer-specific
          stuff.

## Version 1.0.2

### Detailed Changes:


* More OS X fixes in Imath, IlmImf and IlmImfTest.

* Imath updates.

* Fixed a rotation bug in Imath

## Version 1.0.1

### Detailed Changes:

* Used autoconf 2.53 and automake 1.6 to generate build environment.

* Makefile.am cleanups.

* OS X fixes.

* removed images directory (now distributed separately).

## Version 1.0

### Detailed Changes:

* first official release.

* added some high-level documentation, removed the old OpenEXR.html
          documentation.

* fixed a few nagging build problems.

* bumped IMV_VERSION_NUMBER to 2

## Version 0.9

### Detailed Changes:

* added exrdisplay viewer application.

* cleanup _data in Imf::InputFile and Imf::OutputFile constructors.

* removed old ILM copyright notices.

## Version 0.8

### Detailed Changes:

* Initial release.
