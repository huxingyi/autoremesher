@echo off
setlocal

:SETUP_ENV
rem Toolchain discovery lives in build-env.bat. Override with QTDIR / VCVARS.
call "%~dp0build-env.bat"
if errorlevel 1 (
    pause
    exit /b 1
)

:MENU
call :READ_VERSION
cls
echo ====================================================
echo             AutoRemesher Build Manager
echo ====================================================
echo   Version: %CUR_HUMAN%  (build %CUR_NUM%)
echo ====================================================
echo.
echo   [1] Full Build ^& Deploy (Clean Build + Dependencies)
echo   [2] Quick Rebuild (Incremental NMake compilation)
echo   [3] Clean Build Artifacts (Debug/Release/Moc/Obj)
echo   [4] Run AutoRemesher (release\autoremesher.exe)
echo   [5] Set Version
echo   [6] Exit
echo.
echo ====================================================
choice /c 123456 /n /m "Select an option [1-6]: "
if errorlevel 6 goto END
if errorlevel 5 goto SET_VERSION
if errorlevel 4 goto RUN_APP
if errorlevel 3 goto CLEAN_BUILD
if errorlevel 2 goto QUICK_BUILD
if errorlevel 1 goto FULL_BUILD

:FULL_BUILD
rem The qmake-generated Makefile never creates DESTDIR, so rcc fails on a
rem fresh checkout or right after a clean.
if not exist "release" mkdir "release"
echo.
echo ===================================
echo [1/5] Building TBB dependency...
echo ===================================
if not exist "thirdparty\tbb\build2" mkdir "thirdparty\tbb\build2"
pushd "thirdparty\tbb\build2"
if not exist "CMakeCache.txt" (
    cmake -DCMAKE_POLICY_VERSION_MINIMUM=3.5 ..
    if errorlevel 1 (
        echo Error: TBB CMake configuration failed!
        popd
        pause
        goto MENU
    )
)
cmake --build . --config Release
if errorlevel 1 (
    echo Error: TBB build failed!
    popd
    pause
    goto MENU
)
popd

echo.
echo ===================================
echo [2/5] Running qmake...
echo ===================================
qmake -spec win32-msvc
if errorlevel 1 (
    echo Error: qmake failed!
    pause
    goto MENU
)

echo.
echo ===================================
echo [3/5] Compiling AutoRemesher (Release)...
echo ===================================
set CL=/MP
nmake -f Makefile.Release
if errorlevel 1 (
    echo Error: nmake build failed!
    pause
    goto MENU
)

echo.
echo ===================================
echo [4/5] Deploying Qt libraries (windeployqt)...
echo ===================================
windeployqt release\autoremesher.exe --no-translations --release
if errorlevel 1 (
    echo Error: windeployqt failed!
    pause
    goto MENU
)

echo.
echo ===================================
echo [5/5] Copying tbb.dll to release directory...
echo ===================================
copy /Y "thirdparty\tbb\build2\Release\tbb.dll" "release\"
if errorlevel 1 (
    echo Error: Failed to copy tbb.dll!
    pause
    goto MENU
)

echo.
echo ===================================
echo FULL BUILD SUCCESSFUL!
echo ===================================
pause
goto MENU

:QUICK_BUILD
echo.
echo ===================================
echo Quick Rebuild (NMake Release)...
echo ===================================
if not exist "release" mkdir "release"
if not exist "Makefile.Release" (
    echo Makefile.Release not found. Running qmake first...
    qmake -spec win32-msvc
)
set CL=/MP
nmake -f Makefile.Release
if errorlevel 1 (
    echo Error: Quick rebuild failed!
    pause
    goto MENU
)
echo.
echo ===================================
echo QUICK REBUILD SUCCESSFUL!
echo ===================================
pause
goto MENU

:CLEAN_BUILD
echo.
echo ===================================
echo Cleaning build directories and Makefiles...
echo ===================================
if exist "Makefile" del /f /q "Makefile"
if exist "Makefile.Debug" del /f /q "Makefile.Debug"
if exist "Makefile.Release" del /f /q "Makefile.Release"
if exist "debug" rmdir /s /q "debug"
if exist "release" rmdir /s /q "release"
if exist "moc" rmdir /s /q "moc"
if exist "obj" rmdir /s /q "obj"
echo Clean completed!
pause
goto MENU

:RUN_APP
echo.
if exist "release\autoremesher.exe" (
    echo Launching AutoRemesher...
    start "" "release\autoremesher.exe"
) else (
    echo Executable release\autoremesher.exe not found! Please run a build first.
    pause
)
goto MENU

:SET_VERSION
echo.
echo ===================================
echo Set Version
echo ===================================
echo Current: %CUR_HUMAN%  (build %CUR_NUM%)
echo.
echo Enter the new version as MAJOR.MINOR.PATCH (for example 1.2.0).
echo Leave empty to cancel.
set "NEW_HUMAN="
set /p "NEW_HUMAN=New version: "
if "%NEW_HUMAN%"=="" (
    echo Cancelled.
    pause
    goto MENU
)
set "NEW_HUMAN=%NEW_HUMAN: =%"

rem Expand to the 4-part numeric form the Windows executable resource needs.
set "P1=" & set "P2=" & set "P3=" & set "P4="
for /f "tokens=1-4 delims=." %%a in ("%NEW_HUMAN%") do (
    set "P1=%%a"
    set "P2=%%b"
    set "P3=%%c"
    set "P4=%%d"
)
if "%P2%"=="" set "P2=0"
if "%P3%"=="" set "P3=0"
if "%P4%"=="" set "P4=0"
set "NEW_NUM=%P1%.%P2%.%P3%.%P4%"

echo %NEW_NUM%| findstr /r /c:"^[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*$" >nul
if errorlevel 1 (
    echo Error: '%NEW_HUMAN%' is not a numeric version. Every field must be a number.
    pause
    goto MENU
)

> "version.pri" echo # Application version - single source of truth.
>> "version.pri" echo # Written by build.bat option [5]. Safe to edit by hand,
>> "version.pri" echo # but then run a Clean Build so the version is baked in again.
>> "version.pri" echo.
>> "version.pri" echo HUMAN_VERSION = %NEW_HUMAN%
>> "version.pri" echo VERSION = %NEW_NUM%

rem The version is compiled in as a -D define, so the source files never change
rem and nmake would happily keep the stale objects. Drop the ones that bake it in.
rem qmake nests objects under obj\src\, older layouts put them straight in obj\.
for %%f in (main mainwindow aboutwidget logbrowserdialog util) do (
    if exist "obj\%%f.obj" del /f /q "obj\%%f.obj"
    if exist "obj\src\%%f.obj" del /f /q "obj\src\%%f.obj"
)
rem Same story for the Windows version resource: autoremesher.rc does not change,
rem only the defines do, so the compiled .res has to go as well or the executable
rem properties keep reporting the previous version.
if exist "obj\autoremesher.res" del /f /q "obj\autoremesher.res"

echo.
echo Regenerating the Makefile...
qmake -spec win32-msvc
if errorlevel 1 (
    echo Error: qmake failed!
    pause
    goto MENU
)

echo.
echo Version set to %NEW_HUMAN% (build %NEW_NUM%).
echo Run [2] Quick Rebuild to produce a binary carrying it.
pause
goto MENU

:READ_VERSION
set "CUR_HUMAN=1.0.0"
set "CUR_NUM=1.0.0.9"
if not exist "version.pri" goto :eof
for /f "tokens=2 delims==" %%v in ('findstr /b /c:"HUMAN_VERSION" "version.pri"') do set "CUR_HUMAN=%%v"
for /f "tokens=2 delims==" %%v in ('findstr /b /c:"VERSION" "version.pri"') do set "CUR_NUM=%%v"
set "CUR_HUMAN=%CUR_HUMAN: =%"
set "CUR_NUM=%CUR_NUM: =%"
goto :eof

:END
echo Goodbye!
exit /b 0
