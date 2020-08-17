:: Get the Visual Studio installation dir
set MSVCDIR=%VS120COMNTOOLS%..\..\VC

:: Configure environment for Visual Studio
call "%MSVCDIR%\VCVARSALL.BAT" x64

:: Set the generator to use
set CMAKE_VS_GENERATOR=Visual Studio 12 Win64
