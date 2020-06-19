[![appveyor status](https://ci.appveyor.com/api/projects/status/github/huxingyi/autoremesher?branch=master&svg=true)](https://ci.appveyor.com/project/huxingyi/autoremesher) [![travis status](https://travis-ci.org/huxingyi/autoremesher.svg?branch=master)](https://travis-ci.org/huxingyi/autoremesher)  

Buy me a coffee for staying up late coding :-) [![](https://www.paypalobjects.com/en_US/i/btn/btn_donate_SM.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=GHALWLWXYGCU6&item_name=Support+me+coding+in+my+spare+time&currency_code=AUD&source=url)  

# Overview
Automatic quad remeshing tool.  
https://blogs.dust3d.org/2020/06/19/auto-remesher/

![](https://blogs.dust3d.org/public/attachments/2020-06-19-auto-remesher/auto-remesher-480.png)   

# Quick Start

## Windows
Download autoremesher-unstable.zip from [releases](https://github.com/huxingyi/autoremesher/releases)
Extract it and run,
```
> autoremesher.exe <input.obj> -o <output.obj> [-s <gradient size>]
```
*If you are running 32bit instead of 64bit, please download autoremesher-unstable-x86.zip*

## macOS
Download autoremesher-unstable.dmg file from [releases](https://github.com/huxingyi/autoremesher/releases)
Double click the dmg file and drag the app to your folder, open a command line window, cd to the folder and run,
```
$ ./autoremesher-unstable.app/Contents/MacOS/autoremesher <input.obj> -o <output.obj> [-s <gradient size>]
```
*For the first time, Apple will reject to run and popup something like "can't be opened because its integrity cannot be verified", Goto System preferences > Security & Privacy > General and under "Allow apps downloaded from" click the button to allow it*

## Linux
Download autoremesher-unstable.AppImage file from [releases](https://github.com/huxingyi/autoremesher/releases)
```
$ chmod a+x ./autoremesher-unstable.AppImage
$ ./autoremesher-unstable.AppImage <input.obj> -o <output.obj> [-s <gradient size>]
```

# Build

## Windows (64bit)
Prerequisites: Visual Studio 2017, CMake
```
> git clone https://github.com/huxingyi/autoremesher.git
> cd autoremesher
> call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"

> cd thirdparty/OpenMesh/OpenMesh-8.1
> mkdir build
> cd build
> cmake -G "Visual Studio 15 2017" -A x64 -D "BUILD_APPS=OFF" ../
> cmake --build . --config Release
> cd ../../../../

> mkdir build
> cd build
> cmake -G "Visual Studio 15 2017" -A x64 ../
> cmake --build . --config Release
```

## Windows (32bit)
Prerequisites: Visual Studio 2017, CMake
```
> git clone https://github.com/huxingyi/autoremesher.git
> cd autoremesher
> call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars32.bat"

> cd thirdparty/OpenMesh/OpenMesh-8.1
> mkdir build
> cd build
> cmake -G "Visual Studio 15 2017" -D "BUILD_APPS=OFF" ../
> cmake --build . --config Release
> cd ../../../../

> mkdir build
> cd build
> cmake -G "Visual Studio 15 2017" ../
> cmake --build . --config Release
```

## Ubuntu
```
$ sudo add-apt-repository --yes ppa:beineri/opt-qt591-trusty
$ sudo apt-get update
$ sudo apt install cmake
$ sudo apt-get install qt59base qt59tools --force-yes
$ source /opt/qt59/bin/qt59-env.sh

$ git clone https://github.com/huxingyi/autoremesher.git
$ cd thirdparty/OpenMesh/OpenMesh-8.1
$ mkdir build
$ cd build
$ cmake -D "BUILD_APPS=OFF" -D "OPENMESH_BUILD_SHARED=OFF" ../
$ cmake --build . --config Release
$ rm -rf Build/lib/libOpenMeshCore.so
$ cd ../../../../

$ qmake -config release
$ make
```

## macOS
```
$ HOMEBREW_VERBOSE_USING_DOTS=1 brew reinstall --verbose qt
$ export PATH="/usr/local/opt/qt/bin:$(brew --prefix)/bin:$PATH

$ git clone https://github.com/huxingyi/autoremesher.git
$ cd thirdparty/OpenMesh/OpenMesh-8.1
$ mkdir build
$ cd build
$ cmake -D "BUILD_APPS=OFF" -D "OPENMESH_BUILD_SHARED=OFF" ../
$ cmake --build . --config Release
$ rm -rf Build/lib/libOpenMeshCore.dylib
$ cd ../../../../

$ qmake -config release
$ make
```

## Acknowledgements
- [CoMISo](https://www.graphics.rwth-aachen.de/software/comiso/)  
- [libQEx](https://github.com/hcebke/libQEx)  
- [libigl](https://github.com/libigl/libigl)  
- [eigen](https://gitlab.com/libeigen/eigen)  
- [OpenMesh](https://graphics.rwth-aachen.de:9000/OpenMesh)  
- [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader)  
- [clapack](http://www.netlib.org/clapack/)  
