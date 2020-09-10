REM @echo off

Echo LIB Windows Build NuGet

REM # Build Vars #
set _SCRIPT_DRIVE=%~d0
set _SCRIPT_FOLDER=%~dp0
set INITDIR=%_SCRIPT_FOLDER%
set SRC=%INITDIR%\..\..\
set BUILDTREE=%SRC%\build-win\
SET tbs_arch=x64
SET vcvar_arg=x86_amd64
SET cmake_platform="Visual Studio 15 2017 Win64"

REM # VC Vars #
SET VCVAR="%programfiles(x86)%\Microsoft Visual Studio\2017\Professional\VC\Auxiliary\Build\vcvarsall.bat"
if exist %VCVAR% call %VCVAR% %vcvar_arg%
SET VCVAR="%programfiles(x86)%\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat"
if exist %VCVAR% call %VCVAR% %vcvar_arg%

REM # Clean Build Tree #
rd /s /q %BUILDTREE%
mkdir %BUILDTREE%
mkdir %BUILDTREE%\deps

REM # Change to Build Tree drive #
%_SCRIPT_DRIVE%
REM # Change to Build Tree directory #
cd %BUILDTREE%

:nuget_Dep
REM # packages from nuget #
mkdir %BUILDTREE%\deps
cd %BUILDTREE%\deps
SET VER=1.2.11.8899
set ZLIBDIR=%BUILDTREE%\deps\zlib-msvc-%tbs_arch%.%VER%\build\native
nuget install zlib-msvc-%tbs_arch% -Version %VER%
SET VER=1.3.4.8788
set FLTKDIR=%BUILDTREE%\deps\FLTK-msvc-%tbs_arch%.%VER%\build\native
nuget install FLTK-msvc-%tbs_arch% -Version %VER%

:copy_files
set BINDIR=%SRC%\build-nuget\
rd /s /q %BINDIR%
mkdir %BINDIR%
mkdir %BINDIR%\lib
set PATH=%BINDIR%\lib;%PATH%
echo %BINDIR%

:static_LIB
REM # LIB STATIC #
ECHO %cmake_platform% STATIC

rd /s /q %BUILDTREE%\OpenEXR
mkdir %BUILDTREE%\OpenEXR
cd %BUILDTREE%\OpenEXR
cmake -G %cmake_platform% ^
-DBUILD_ILMBASE_STATIC:BOOL=ON ^
-DOPENEXR_BUILD_ILMBASE:BOOL=ON ^
-DOPENEXR_BUILD_OPENEXR:BOOL=ON ^
-DOPENEXR_BUILD_PYTHON_LIBS:BOOL=OFF ^
-DOPENEXR_BUILD_VIEWERS:BOOL=ON ^
-DOPENEXR_BUILD_TESTS:BOOL=ON ^
-DOPENEXR_RUN_FUZZ_TESTS:BOOL=OFF ^
-DOPENEXR_BUILD_UTILS:BOOL=ON ^
-DOPENEXR_BUILD_SHARED:BOOL=OFF ^
-DOPENEXR_BUILD_STATIC:BOOL=ON ^
-DCMAKE_CXX_FLAGS_RELEASE="/MD" ^
-DCMAKE_CXX_FLAGS_DEBUG="/MDd" ^
-DCMAKE_C_FLAGS_RELEASE="/MD" ^
-DCMAKE_C_FLAGS_DEBUG="/MDd" ^
-DZLIB_LIBRARY=%ZLIBDIR%\lib_release\zlibstatic.lib ^
-DZLIB_INCLUDE_DIR=%ZLIBDIR%\include ^
-DFLTK_BASE_LIBRARY_RELEASE=%FLTKDIR%\lib_release\fltk.lib ^
-DFLTK_GL_LIBRARY_RELEASE=%FLTKDIR%\lib_release\fltk_gl.lib ^
-DFLTK_FORMS_LIBRARY_RELEASE=%FLTKDIR%\lib_release\fltk_forms.lib ^
-DFLTK_IMAGES_LIBRARY_RELEASE=%FLTKDIR%\lib_release\fltk_images.lib ^
-DFLTK_BASE_LIBRARY_DEBUG=%FLTKDIR%\lib_debug\fltkd.lib ^
-DFLTK_GL_LIBRARY_DEBUG=%FLTKDIR%\lib_debug\fltk_gld.lib ^
-DFLTK_FORMS_LIBRARY_DEBUG=%FLTKDIR%\lib_debug\fltk_formsd.lib ^
-DFLTK_IMAGES_LIBRARY_DEBUG=%FLTKDIR%\lib_debug\fltk_imagesd.lib ^
-DFLTK_INCLUDE_DIR=%FLTKDIR%\include ^
-DCMAKE_INSTALL_PREFIX=%BINDIR% ^
-DCMAKE_BUILD_TYPE="Release" %SRC%
cmake --build . --config Release --target install

move %BINDIR%lib %BINDIR%lib_release
move %BINDIR%bin %BINDIR%bin_release

REM # DEBUG #
REM # Clean Build Tree #
rd /s /q %BUILDTREE%\OpenEXR
mkdir %BUILDTREE%\OpenEXR
cd %BUILDTREE%\OpenEXR
cmake -G %cmake_platform% ^
-DBUILD_ILMBASE_STATIC:BOOL=ON ^
-DOPENEXR_BUILD_ILMBASE:BOOL=ON ^
-DOPENEXR_BUILD_OPENEXR:BOOL=ON ^
-DOPENEXR_BUILD_PYTHON_LIBS:BOOL=OFF ^
-DOPENEXR_BUILD_VIEWERS:BOOL=ON ^
-DOPENEXR_BUILD_TESTS:BOOL=ON ^
-DOPENEXR_RUN_FUZZ_TESTS:BOOL=OFF ^
-DOPENEXR_BUILD_UTILS:BOOL=ON ^
-DOPENEXR_BUILD_SHARED:BOOL=OFF ^
-DOPENEXR_BUILD_STATIC:BOOL=ON ^
-DCMAKE_CXX_FLAGS_RELEASE="/MD" ^
-DCMAKE_CXX_FLAGS_DEBUG="/MDd" ^
-DCMAKE_C_FLAGS_RELEASE="/MD" ^
-DCMAKE_C_FLAGS_DEBUG="/MDd" ^
-DZLIB_LIBRARY=%ZLIBDIR%\lib_debug\zlibstaticd.lib ^
-DZLIB_INCLUDE_DIR=%ZLIBDIR%\include ^
-DFLTK_BASE_LIBRARY_RELEASE=%FLTKDIR%\lib_release\fltk.lib ^
-DFLTK_GL_LIBRARY_RELEASE=%FLTKDIR%\lib_release\fltk_gl.lib ^
-DFLTK_FORMS_LIBRARY_RELEASE=%FLTKDIR%\lib_release\fltk_forms.lib ^
-DFLTK_IMAGES_LIBRARY_RELEASE=%FLTKDIR%\lib_release\fltk_images.lib ^
-DFLTK_BASE_LIBRARY_DEBUG=%FLTKDIR%\lib_debug\fltkd.lib ^
-DFLTK_GL_LIBRARY_DEBUG=%FLTKDIR%\lib_debug\fltk_gld.lib ^
-DFLTK_FORMS_LIBRARY_DEBUG=%FLTKDIR%\lib_debug\fltk_formsd.lib ^
-DFLTK_IMAGES_LIBRARY_DEBUG=%FLTKDIR%\lib_debug\fltk_imagesd.lib ^
-DFLTK_INCLUDE_DIR=%FLTKDIR%\include ^
-DCMAKE_INSTALL_PREFIX=%BINDIR% ^
-DCMAKE_BUILD_TYPE="DEBUG" %SRC%
cmake --build . --config DEBUG --target install

move %BINDIR%lib %BINDIR%lib_debug
move %BINDIR%bin %BINDIR%bin_debug

REM # TODO: ENABLE SHARED Build
GOTO:nuget_req
mkdir %BINDIR%\static\
move /Y %BINDIR%\lib %BINDIR%\static\

:shared_LIB
REM # LIB SHARED #
ECHO %cmake_platform% SHARED


:nuget_req
cd %BINDIR%
REM # make nuget packages from binaries #
copy %INITDIR%\OpenEXR-msvc-%tbs_arch%.targets %BINDIR%\OpenEXR-msvc-%tbs_arch%.targets
cd %BUILDTREE%
nuget pack %INITDIR%\OpenEXR-msvc-%tbs_arch%.nuspec
cd %INITDIR%
REM --- exit ----
GOTO:eof
