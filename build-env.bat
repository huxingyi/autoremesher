@echo off
rem ---------------------------------------------------------------
rem  Shared toolchain discovery for build.bat and quickbuild.bat.
rem
rem  Nothing here is tied to one machine. Override either half by
rem  setting the variable before calling a build script:
rem
rem      set "QTDIR=D:\Qt\5.15.2\msvc2019_64"
rem      set "VCVARS=D:\VS\VC\Auxiliary\Build\vcvars64.bat"
rem
rem  On success QTDIR is set and %QTDIR%\bin is on PATH. On failure the
rem  script returns 1 after printing which variable to set by hand.
rem
rem  Written with goto-based flow rather than if-blocks on purpose: the
rem  discovered paths contain "Program Files (x86)", and those parentheses
rem  break cmd's parser once they are expanded inside a parenthesised block.
rem ---------------------------------------------------------------

rem --- MSVC ------------------------------------------------------
rem VSCMD_ARG_TGT_ARCH is set by vcvars64; skip if already in a dev prompt.
if not "%VSCMD_ARG_TGT_ARCH%"=="" goto QT
if defined VCVARS goto CHECK_VCVARS

rem vswhere ships with every VS 2017+ installer and knows where VS lives.
set "VSWHERE_DIR=%SystemDrive%\Program Files (x86)\Microsoft Visual Studio\Installer"
if not exist "%VSWHERE_DIR%\vswhere.exe" set "VSWHERE_DIR=%ProgramFiles%\Microsoft Visual Studio\Installer"
if not exist "%VSWHERE_DIR%\vswhere.exe" goto PROBE_VCVARS

rem vswhere writes to a temp file instead of being read through backquotes:
rem its path contains "(x86)", and those parentheses break the parser inside
rem the for /f parentheses no matter how the command is quoted.
set "VSWHERE_OUT=%TEMP%\autoremesher_vspath.txt"
"%VSWHERE_DIR%\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath > "%VSWHERE_OUT%" 2>nul
for /f "usebackq tokens=* delims=" %%i in ("%VSWHERE_OUT%") do set "VCVARS=%%i\VC\Auxiliary\Build\vcvars64.bat"
del "%VSWHERE_OUT%" >nul 2>&1
if defined VCVARS goto CHECK_VCVARS

:PROBE_VCVARS
rem Fallback for machines without vswhere: walk the default install layout.
rem The "if not defined" guard keeps the first hit, so newer VS versions win.
for %%y in (2022 2019 2017) do for %%e in (Enterprise Professional Community BuildTools) do if not defined VCVARS if exist "%ProgramFiles%\Microsoft Visual Studio\%%y\%%e\VC\Auxiliary\Build\vcvars64.bat" set "VCVARS=%ProgramFiles%\Microsoft Visual Studio\%%y\%%e\VC\Auxiliary\Build\vcvars64.bat"
for %%y in (2022 2019 2017) do for %%e in (Enterprise Professional Community BuildTools) do if not defined VCVARS if exist "%SystemDrive%\Program Files (x86)\Microsoft Visual Studio\%%y\%%e\VC\Auxiliary\Build\vcvars64.bat" set "VCVARS=%SystemDrive%\Program Files (x86)\Microsoft Visual Studio\%%y\%%e\VC\Auxiliary\Build\vcvars64.bat"
if not defined VCVARS goto NO_VCVARS

:CHECK_VCVARS
if not exist "%VCVARS%" goto BAD_VCVARS
echo Setting up MSVC environment...
call "%VCVARS%"
if errorlevel 1 goto VCVARS_FAILED
goto QT

:NO_VCVARS
echo Error: could not locate the MSVC build tools.
echo        Install Visual Studio with the "Desktop development with C++"
echo        workload, or set VCVARS to the full path of vcvars64.bat.
exit /b 1

:BAD_VCVARS
echo Error: VCVARS points at a file that does not exist:
echo        %VCVARS%
exit /b 1

:VCVARS_FAILED
echo Error: vcvars64.bat failed.
exit /b 1

rem --- Qt --------------------------------------------------------
:QT
if defined QTDIR goto CHECK_QT

rem Probe the default Qt install layout: <root>\5.x.y\msvc*_64.
for %%r in ("%SystemDrive%\Qt" "C:\Qt" "D:\Qt") do for /d %%v in ("%%~r\5.*") do for /d %%m in ("%%~v\msvc*_64") do if exist "%%~m\bin\qmake.exe" set "QTDIR=%%~m"

:CHECK_QT
if not defined QTDIR goto NO_QT
if not exist "%QTDIR%\bin\qmake.exe" goto BAD_QT

set "PATH=%QTDIR%\bin;%PATH%"
exit /b 0

:NO_QT
echo Error: could not locate a Qt 5 msvc_64 installation.
echo        Set QTDIR to your Qt kit directory, for example:
echo            set "QTDIR=C:\Qt\5.15.2\msvc2019_64"
exit /b 1

:BAD_QT
echo Error: no qmake.exe under %QTDIR%\bin
echo        QTDIR must point at a Qt kit directory, not the Qt install root.
exit /b 1
