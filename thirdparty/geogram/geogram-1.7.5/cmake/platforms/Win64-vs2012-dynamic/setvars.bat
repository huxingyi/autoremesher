:: Get the Visual Studio installation dir
set MSVCDIR=%VS110COMNTOOLS%..\..\VC

:: Remove quotes in PATH (workaround to a bug in Jenkins)
set PATH=%PATH:"=%

:: Configure environment for Visual Studio
call "%MSVCDIR%\VCVARSALL.BAT" x64

echo :: Set the generator to use
set CMAKE_VS_GENERATOR=Visual Studio 11 Win64
