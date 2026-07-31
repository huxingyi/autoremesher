@echo off
rem Incremental release rebuild. See build-env.bat for QTDIR / VCVARS overrides.
setlocal

call "%~dp0build-env.bat"
if errorlevel 1 exit /b 1

set CL=/MP
rem The qmake-generated Makefile never creates DESTDIR, so rcc fails without this.
if not exist "release" mkdir "release"
if not exist "Makefile.Release" (
    echo Makefile.Release not found, running qmake...
    qmake -spec win32-msvc
    if errorlevel 1 exit /b 1
)
nmake -f Makefile.Release
exit /b %errorlevel%
