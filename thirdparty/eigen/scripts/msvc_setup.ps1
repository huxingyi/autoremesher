# Powershell script to set up MSVC environment.

param ($EIGEN_CI_MSVC_ARCH, $EIGEN_CI_MSVC_VER)
function Get-ScriptDirectory { Split-Path $MyInvocation.ScriptName }
       
# Set defaults if not already set.
IF (!$EIGEN_CI_MSVC_ARCH)             { $EIGEN_CI_MSVC_ARCH             = "x64" }
IF (!$EIGEN_CI_MSVC_VER)              { $EIGEN_CI_MSVC_VER              = "14.29" }

# Export variables into the global scope
$global:EIGEN_CI_MSVC_ARCH             = $EIGEN_CI_MSVC_ARCH
$global:EIGEN_CI_MSVC_VER              = $EIGEN_CI_MSVC_VER

# Find Visual Studio installation directory.
$global:VS_INSTALL_DIR = &"${Env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath
# Run VCVarsAll.bat incitialization script and extract environment variables.
# http://allen-mack.blogspot.com/2008/03/replace-visual-studio-command-prompt.html
cmd.exe /c "`"${VS_INSTALL_DIR}\VC\Auxiliary\Build\vcvarsall.bat`" $EIGEN_CI_MSVC_ARCH -vcvars_ver=$EIGEN_CI_MSVC_VER & set" | foreach { if ($_ -match "=") { $v = $_.split("="); set-item -force -path "ENV:\$($v[0])" -value "$($v[1])" } }