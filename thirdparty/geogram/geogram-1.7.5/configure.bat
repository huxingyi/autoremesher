:: This file is for Windows users, 
:: it launches CMake and creates configuration for Visual Studio
:: for Release and Debug modes.

@echo off
setlocal ENABLEDELAYEDEXPANSION 

:: Check for options: [ --build_name_suffix suffix ]
set buildNameSuffix=""
if "%1" ==  "--build_name_suffix" (
   set buildNameSuffix=%2
   SHIFT & SHIFT
)

:: Read platform
set opsys=%1

:: Checking for CMake

echo.
echo ============= Checking for CMake ============
echo.

cmake --version
if %errorlevel% == 0 (
    echo Found CMake
) else (
    echo Error: CMake not found, please install it - see http://www.cmake.org/
    exit /B 1
)

:: Checking the current OS

if "%opsys%" == "" (
    if "%PROCESSOR_ARCHITECTURE%" == "x86" (
        set opsys=Win32-vs2012
    ) else if "%PROCESSOR_ARCHITECTURE%" == "AMD64" (
        set opsys=Win64-vs2012
    ) else (
        echo Error: OS not supported
        exit /B 1
    )
)

if not exist "cmake\platforms\%opsys%" (
    echo Error: unsupported platform: %opsys%
    exit /B 1
)


:: Import the platform specific configuration

echo.
echo ============= Checking for Visual Studio ============
echo.

call "cmake\platforms\%opsys%\setvars.bat" || exit /B 1

:: Generate build tree

echo.
echo ============= Creating build system for %opsys% ============
echo.

if not exist build\%opsys%%buildNameSuffix% (
    mkdir build\%opsys%%buildNameSuffix%
)

echo Using cmake generator %CMAKE_VS_GENERATOR%
set cmake_generator_options=-G "%CMAKE_VS_GENERATOR%"

if "%CMAKE_VS_GENERATOR_TOOLSET%" neq "" (
    echo Using cmake generator toolset %CMAKE_VS_GENERATOR_TOOLSET%
    set cmake_generator_options=%cmake_generator_options% -T "%CMAKE_VS_GENERATOR_TOOLSET%"
)


::set cmake_debug_options=--trace --debug-output
pushd build\%opsys%%buildNameSuffix%
cmake ..\.. %cmake_debug_options% %cmake_generator_options% -DVORPALINE_PLATFORM:STRING=%opsys% || exit /B 1
popd

echo.
echo ============== Vorpaline build configured ==================
echo.
echo To build vorpaline:
echo - go to build/%opsys%%buildNameSuffix%
echo - run 'cmake --build . --config=Release(or Debug) [--target=target_to_build]'
echo.
echo Note: local configuration can be specified in CMakeOptions.txt
echo See CMakeOptions.txt.sample for an example
echo You'll need to re-run configure.bat if you create or modify CMakeOptions.txt
echo.

:: Clear globals

set buildNameSuffix=
set opsys=
exit /B 0
