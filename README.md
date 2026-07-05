# AutoRemesher

AutoRemesher is a cross-platform automatic quad remeshing tool that converts high-polygon meshes into clean quad-based topology. It is built on top of libraries: [Geogram](https://github.com/BrunoLevy/geogram), [libigl](https://github.com/libigl), [isotropicremesher](https://github.com/huxingyi/isotropicremesher) and [others](https://github.com/huxingyi/autoremesher/blob/master/ACKNOWLEDGEMENTS.html).

Buy me a coffee for staying up late coding :-) [![](https://www.paypalobjects.com/en_US/i/btn/btn_donate_SM.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=GHALWLWXYGCU6&item_name=Support+me+coding+in+my+spare+time&currency_code=AUD&source=url)

## Getting Started

These instructions will get you a copy of AutoRemesher up and running on your local machine for development and testing purposes.

### Prerequisites

- C++ compiler with C++14 support
- CMake 3.12 or later
- TBB (Intel Threading Building Blocks)

### Building

1. Clone the repository
```
git clone https://github.com/huxingyi/autoremesher.git
```

2. Build using CMake
```
cd autoremesher
mkdir build && cd build
cmake ..
cmake --build .
```

#### Windows

1. Install Visual Studio 2022 with C++ development tools
2. Install CMake from [cmake.org](https://cmake.org/)
3. Open a command prompt and follow the build steps above, or open the project folder in Visual Studio and let it handle CMake configuration

#### macOS

1. Install Xcode Command Line Tools: `xcode-select --install`
2. Install CMake via Homebrew: `brew install cmake`
3. Follow the build steps above

#### Linux

1. Install build dependencies using your distribution's package manager:
   - Ubuntu/Debian: `sudo apt install build-essential cmake libtbb-dev`
   - Fedora: `sudo dnf install gcc-c++ cmake tbb-devel`

### Quick Start

#### Windows

Download `autoremesher-<version>-win32-x86_64.zip` from [releases](https://github.com/huxingyi/autoremesher/releases), extract it and run `autoremesher.exe`.

#### macOS

Download `autoremesher-<version>.dmg` from [releases](https://github.com/huxingyi/autoremesher/releases).

*For the first time, Apple will reject to run and popup something like "can't be opened because its integrity cannot be verified". Go to System Preferences > Security & Privacy > General and under "Allow apps downloaded from" click the button to allow it.*

#### Linux

Download `autoremesher-<version>.AppImage` from [releases](https://github.com/huxingyi/autoremesher/releases).

```
$ chmod a+x ./autoremesher-<version>.AppImage
$ ./autoremesher-<version>.AppImage
```

### Links

- [Check out open-source auto-retopology tool AutoRemesher](http://www.cgchannel.com/2020/08/check-out-open-source-auto-retopology-tool-autoremesher/) **cgchannel.com**
- [A New Open-Source Auto-Retopology Tool](https://80.lv/articles/a-new-open-source-auto-retopology-tool/) **80.lv**
- [[Non-Blender] Autoremesher auto-retopology tool released](https://www.blendernation.com/2020/08/18/non-blender-autoremesher-auto-retopology-tool-released/) **blendernation.com**
- [オープンソースの新しいオートリメッシュツール Auto Remesher](https://cginterest.com/2020/08/20/%e3%82%aa%e3%83%bc%e3%83%97%e3%83%b3%e3%82%bd%e3%83%bc%e3%82%b9%e3%81%ae%e6%96%b0%e3%81%97%e3%81%84%e3%82%aa%e3%83%bc%e3%83%88%e3%83%aa%e3%83%a1%e3%83%83%e3%82%b7%e3%83%a5%e3%83%84%e3%83%bc%e3%83%ab-a/) **cginterest.com**
- [AutoRemesher 1.0.0-alpha - 超高速で高品質のクワッドポリゴン生成！Dust3D開発者によるオープンソースの自動リメッシュツール！](https://3dnchu.com/archives/autoremesher-1-0-0-alpha/) **3dnchu.com**
- [Open Source AutoRemesher released](https://cgpress.org/archives/open-source-remesher.html) **cgpress.org**
- [「autoremesher」多角形を自動でリトポしてれる無料トポロジーツール](https://modelinghappy.com/archives/30339) **modelinghappy.com**
- [Open Source Auto Remesher](https://blender-addons.org/open-source-auto-remesher/) **blender-addons.org**
- [AutoRemesher | Auto-Retopology-Tool](https://www.digitalproduction.com/2020/08/05/autoremesher-auto-retopology-tool/) **digitalproduction.com**
- [Autoremesher open source auto-retopology tool](https://blenderartists.org/t/autoremesher-open-source-auto-retopology-tool/1245131/126) **blenderartists.org**

## License

AutoRemesher is licensed under the MIT License - see the [LICENSE](https://github.com/huxingyi/autoremesher/blob/master/LICENSE) file for details.

## Acknowledgements

See the full [ACKNOWLEDGEMENTS](https://github.com/huxingyi/autoremesher/blob/master/ACKNOWLEDGEMENTS.html) for a list of libraries and resources used in this project.

<!-- Sponsors begin --><!-- Sponsors end -->