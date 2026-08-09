# Find Visual Studio installation directory.
$VS_INSTALL_DIR = &"${Env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath

# Run VCVarsAll.bat initialization script and extract environment variables.
# http://allen-mack.blogspot.com/2008/03/replace-visual-studio-command-prompt.html
cmd.exe /c "`"${VS_INSTALL_DIR}\VC\Auxiliary\Build\vcvarsall.bat`" $EIGEN_CI_MSVC_ARCH -vcvars_ver=$EIGEN_CI_MSVC_VER & set" |
  foreach {
    if ($_ -match "=") {
      $v = $_.split("="); set-item -force -path "ENV:\$($v[0])" -value "$($v[1])"
    }
  }

# Create and enter build directory.
$rootdir = Get-Location
if (-Not (Test-Path ${EIGEN_CI_BUILDDIR})) {
    mkdir $EIGEN_CI_BUILDDIR
}
cd $EIGEN_CI_BUILDDIR

# We need to split EIGEN_CI_ADDITIONAL_ARGS, otherwise they are interpreted
# as a single argument.  Split by space, unless double-quoted.
$split_args = [regex]::Split(${EIGEN_CI_ADDITIONAL_ARGS}, ' (?=(?:[^"]|"[^"]*")*$)' )

# Configure build.
cmake -G Ninja -DCMAKE_BUILD_TYPE=MinSizeRel `
      -DEIGEN_TEST_CUSTOM_CXX_FLAGS="${EIGEN_CI_TEST_CUSTOM_CXX_FLAGS}" `
      ${split_args} "${rootdir}"

$target = ""
if (${EIGEN_CI_BUILD_TARGET}) {
  $target = "--target ${EIGEN_CI_BUILD_TARGET}"
}

# Windows builds sometimes fail due heap errors. In that case, try
# building the rest, then try to build again with a single thread.
cmake --build . ${target} -- -k0 || cmake --build . ${target} -- -k0 -j1

$success = $LASTEXITCODE

# Return to root directory.
cd ${rootdir}

# Explicitly propagate exit code to indicate pass/failure of build command.
if($success -ne 0) { Exit $success }
