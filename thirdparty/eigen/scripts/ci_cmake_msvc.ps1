# Powershell script to set up MSVC cmake builds that mirror the CI.  Useful for reproducing issues.

param ($EIGEN_CI_ROOTDIR,
       $EIGEN_CI_BUILDDIR,
       $EIGEN_CI_BUILD_TARGET,
       $EIGEN_CI_ADDITIONAL_ARGS,
       $EIGEN_CI_BEFORE_SCRIPT,
       $EIGEN_CI_CMAKE_GENERATOR,
       $EIGEN_CI_MSVC_ARCH,
       $EIGEN_CI_MSVC_VER,
       $EIGEN_CI_TEST_CUSTOM_CXX_FLAGS
       )

function Get-ScriptDirectory { Split-Path $MyInvocation.ScriptName }
       
# Set defaults if not already set.
IF (!$EIGEN_CI_ROOTDIR)               { $EIGEN_CI_ROOTDIR               = Join-Path (Get-ScriptDirectory) '..' }
IF (!$EIGEN_CI_BUILDDIR)              { $EIGEN_CI_BUILDDIR              = ".build" }
IF (!$EIGEN_CI_BUILD_TARGET)          { $EIGEN_CI_BUILD_TARGET          = "buildtests" }
IF (!$EIGEN_CI_ADDITIONAL_ARGS)       { $EIGEN_CI_ADDITIONAL_ARGS       = "" }
IF (!$EIGEN_CI_BEFORE_SCRIPT)         { $EIGEN_CI_BEFORE_SCRIPT         = "" }
IF (!$EIGEN_CI_CMAKE_GENERATOR)       { $EIGEN_CI_CMAKE_GENERATOR       = "Ninja" }
IF (!$EIGEN_CI_MSVC_ARCH)             { $EIGEN_CI_MSVC_ARCH             = "x64" }
IF (!$EIGEN_CI_MSVC_VER)              { $EIGEN_CI_MSVC_VER              = "14.29" }
IF (!$EIGEN_CI_TEST_CUSTOM_CXX_FLAGS) { $EIGEN_CI_TEST_CUSTOM_CXX_FLAGS = "/d2ReducedOptimizeHugeFunctions /DEIGEN_STRONG_INLINE=inline /Os" }

# Export variables into the global scope
$global:EIGEN_CI_ROOTDIR               = $EIGEN_CI_ROOTDIR
$global:EIGEN_CI_BUILDDIR              = $EIGEN_CI_BUILDDIR
$global:EIGEN_CI_BUILD_TARGET          = $EIGEN_CI_BUILD_TARGET
$global:EIGEN_CI_ADDITIONAL_ARGS       = $EIGEN_CI_ADDITIONAL_ARGS
$global:EIGEN_CI_BEFORE_SCRIPT         = $EIGEN_CI_BEFORE_SCRIPT
$global:EIGEN_CI_CMAKE_GENERATOR       = $EIGEN_CI_CMAKE_GENERATOR
$global:EIGEN_CI_MSVC_ARCH             = $EIGEN_CI_MSVC_ARCH
$global:EIGEN_CI_MSVC_VER              = $EIGEN_CI_MSVC_VER
$global:EIGEN_CI_TEST_CUSTOM_CXX_FLAGS = $EIGEN_CI_TEST_CUSTOM_CXX_FLAGS

# Print configuration variables.
Get-Variable | findstr EIGEN


# Run any setup scripts.
if ("${EIGEN_CI_BEFORE_SCRIPT}") { Invoke-Expression -Command "${EIGEN_CI_BEFORE_SCRIPT}" }

# Find Visual Studio installation directory.
$global:VS_INSTALL_DIR = &"${Env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath
# Run VCVarsAll.bat incitialization script and extract environment variables.
# http://allen-mack.blogspot.com/2008/03/replace-visual-studio-command-prompt.html
cmd.exe /c "`"${VS_INSTALL_DIR}\VC\Auxiliary\Build\vcvarsall.bat`" $EIGEN_CI_MSVC_ARCH -vcvars_ver=$EIGEN_CI_MSVC_VER & set" | foreach { if ($_ -match "=") { $v = $_.split("="); set-item -force -path "ENV:\$($v[0])" -value "$($v[1])" } }

# Create and change to the build directory.
IF (-Not (Test-Path -Path $EIGEN_CI_BUILDDIR) ) { mkdir $EIGEN_CI_BUILDDIR }
cd $EIGEN_CI_BUILDDIR

# We need to split EIGEN_CI_ADDITIONAL_ARGS, otherwise they are interpreted
# as a single argument.  Split by space, unless double-quoted.
$split_args = [regex]::Split(${EIGEN_CI_ADDITIONAL_ARGS}, ' (?=(?:[^"]|"[^"]*")*$)' )
cmake -G "${EIGEN_CI_CMAKE_GENERATOR}" -DCMAKE_BUILD_TYPE=MinSizeRel -DEIGEN_TEST_CUSTOM_CXX_FLAGS="${EIGEN_CI_TEST_CUSTOM_CXX_FLAGS}" ${split_args} "${EIGEN_CI_ROOTDIR}"

IF ($EIGEN_CI_BUILD_TARGET) {
  # Windows builds sometimes fail due heap errors. In that case, try
  # building the rest, then try to build again with a single thread.
  cmake --build . --target $EIGEN_CI_BUILD_TARGET -- -k0 || cmake --build . --target $EIGEN_CI_BUILD_TARGET -- -k0 -j1
}

