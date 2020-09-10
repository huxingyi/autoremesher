$boostVersion = $Args[0]
$boostMajorMinor = [io.path]::GetFileNameWithoutExtension("$boostVersion")
$boostVersionConcise = $boostVersion -replace '[.]',''
$boostArray = $boostVersion -split "\."
$boostMajor = $boostArray[0]
$boostMinor = $boostArray[1]
$boostPatch = $boostArray[2];
$boostVersionU = "boost_${boostMajor}_${boostMinor}_${boostPatch}"
$boostArchive = "http://dl.bintray.com/boostorg/release/${boostVersion}/source/${boostVersionU}.zip"

$homeDir = (pwd)
$boostRoot = "${homeDir}\_boost"
$boostInstallDir = "${boostRoot}\boost-${boostVersion}\${boostVersionU}"

Write-Host "boostRoot ${boostRoot}"
Write-Host "boostInstallDir ${boostInstallDir}"

mkdir $boostRoot
cd $boostRoot

# Retrieve/expand archive 
Write-Host "Retrieving ${boostArchive}"
Invoke-WebRequest $boostArchive -OutFile "boost-${boostVersion}.zip"
Write-Host "Expanding archive boost-${boostVersion}.zip"
Expand-Archive "boost-${boostVersion}.zip" 

# Configure and install boost 
cd "${boostInstallDir}\tools\build"
& .\bootstrap.bat 
& .\b2 install -j4 variant=release toolset=msvc `
      --prefix=$boostInstallDir `
      --address-model=64

$env:Path = "${boostInstallDir}\bin;${boostInstallDir};$env:Path"

cd ..\..

# Build boost-python2 libs 
& b2 --with-python `
     --buildid=2 `
     --address-model=64 `
     --prefix=$boostInstallDir `
     toolset=msvc

# Build boost-python3 libs 
# & b2 --with-python --clean
# & bootstrap.bat --with-python=$python3Dir
# & b2 --with-python `
#      --buildid=3 `
#      --address-model=64 `
#      --prefix=$boostInstallDir `
#      toolset=msvc

cd ..\..\..

$newBoostRoot = "c:\Program Files\Boost\${boostVersion}"
New-Item -Path "c:\Program Files\Boost" -ItemType "directory" -Name $boostVersion
Move-Item -Path "${boostInstallDir}\boost" -Destination $newBoostRoot
Move-Item -Path "${boostInstallDir}\stage\lib" -Destination $newBoostRoot
ls $newBoostRoot

$env:BOOST_ROOT = $newBoostRoot
echo "BOOST_ROOT = ${env:BOOST_ROOT}"

