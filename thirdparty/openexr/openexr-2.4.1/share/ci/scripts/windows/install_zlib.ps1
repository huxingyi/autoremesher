$zlibVersion = $Args[0]
$zlibMajorMinor = [io.path]::GetFileNameWithoutExtension("$zlibVersion")
$zlibVersionConcise = $zlibVersion -replace '[.]',''
$zlibArchive = "https://www.zlib.net/zlib${zlibVersionConcise}.zip"

$zlibRoot = "C:\"
$zlibBuildPath = "${zlibRoot}\zlib-${zlibVersion}"
$zlibInstallPath = "C:\_zlib"
$msbuild = "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\MSBuild\15.0\Bin\msbuild.exe"

Write-Host "Retrieving ${zlibArchive}"
Invoke-WebRequest "${zlibArchive}" -OutFile "${zlibRoot}\zlib-${zlibVersion}.zip"
Write-Host "Expanding archive ${zlibRoot}\zlib-${zlibVersion}.zip"
Expand-Archive "${zlibRoot}\zlib-${zlibVersion}.zip" -DestinationPath "${zlibRoot}"

cd $zlibBuildPath
mkdir _build
cd _build
cmake .. -G"Visual Studio 15 2017" -DCMAKE_INSTALL_PREFIX="${zlibInstallPath}"

Write-Host "Building ${zlibBuildPath}\_build\INSTALL.vcxproj" -foregroundcolor green 
& "${msbuild}" "${zlibBuildPath}\_build\INSTALL.vcxproj" /P:Configuration=Release

cd $zlibInstallPath
Write-Host "ls ${zlibInstallPath}"
& ls

cd 
& ls

Write-Host "##vso[task.prependpath]${zlibInstallPath}"
Write-Host "##vso[task.prependpath]${zlibInstallPath}\bin"

