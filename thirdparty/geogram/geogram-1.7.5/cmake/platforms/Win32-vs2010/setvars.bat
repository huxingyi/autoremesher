:: Get the Visual Studio installation dir
set MSVCDIR=%VS100COMNTOOLS%..\..\VC

:: Remove quotes in PATH (workaround to a bug in Jenkins)
set PATH=%PATH:"=%

:: Configure environment for Visual Studio
call "%MSVCDIR%\BIN\VCVARS32.BAT"

:: Set the generator to use
set CMAKE_VS_GENERATOR=Visual Studio 10

