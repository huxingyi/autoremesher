:: Get the Visual Studio installation dir
set MSVCDIR=%VS100COMNTOOLS%..\..\VC

:: Configure environment for Visual Studio
call "%MSVCDIR%\VCVARSALL.BAT" x64

:: Set the generator to use
set CMAKE_VS_GENERATOR=Visual Studio 10 Win64

