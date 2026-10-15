# AutoRemesher (fork)

AutoRemesher is a cross-platform automatic quad remeshing tool that converts high-polygon meshes into clean quad-based topology.

This repository is a fork of [AutoRemesher](https://github.com/huxingyi/autoremesher) by Jeremy Hu (huxingyi). The fork works on remeshing quality, speed and Blender integration, and is a work in progress. The original project and its releases live at [huxingyi/autoremesher](https://github.com/huxingyi/autoremesher).

It is built on [Eigen](https://eigen.tuxfamily.org/), [TBB](https://github.com/oneapi-src/oneTBB), [isotropicremesher](https://github.com/huxingyi/isotropicremesher), [meshoptimizer](https://github.com/zeux/meshoptimizer) and [others](ACKNOWLEDGEMENTS.html).

<img width="3644" height="2202" alt="autoremesher-1 0-screenshot" src="https://github.com/user-attachments/assets/47851f1e-127c-49af-81b7-0c8ac06fb3ad" />

## Getting Started

These instructions will get you a copy of **AutoRemesher** up and running on your local machine for development.

### Prerequisites

- C++ compiler with C++14 support (GCC, Clang, or MSVC)
- Qt 5.15.2
- TBB (Intel Threading Building Blocks)
- CMake 3.12 or later (only needed on Windows to build TBB from source)

### Building

#### Linux (Ubuntu/Debian)

```bash
# Install Qt and build tools
sudo apt install build-essential qt5-qmake qtbase5-dev qttools5-dev-tools libqt5svg5-dev

# Install TBB and OpenGL
sudo apt install libtbb-dev libgl1-mesa-dev

# Clone and build
git clone https://github.com/KajetanMuczynski/autoremesher.git
cd autoremesher
qmake
make -j$(nproc)
```

> **Fedora:** `sudo dnf install gcc-c++ qt5-qtbase-devel qt5-qttools-devel tbb-devel mesa-libGL-devel`

#### Windows (Visual Studio 2022)

1. Install [Visual Studio 2022](https://visualstudio.microsoft.com/) with **Desktop development with C++** workload.
2. Install [CMake](https://cmake.org/download/) (required to build TBB from source).
3. Install Qt 5.15.2 with the [online installer](https://www.qt.io/download-open-source) — select the `msvc2019_64` archive.
4. Open a **x64 Native Tools Command Prompt for VS 2022** and run:

```cmd
:: Build TBB from the bundled third-party source
cd thirdparty\tbb
cmake -B build2 ^
    -DTBB_BUILD_SHARED=ON ^
    -DTBB_BUILD_STATIC=OFF ^
    -DTBB_BUILD_TBBMALLOC=OFF ^
    -DTBB_BUILD_TBBMALLOC_PROXY=OFF ^
    -DTBB_BUILD_TESTS=OFF
cmake --build build2 --config Release
cd ..\..

:: Build AutoRemesher
qmake -spec win32-msvc
set CL=/MP
nmake -f Makefile.Release
```

The release binary will be at `release\autoremesher.exe`.

#### Windows (MSYS2 / MinGW-w64)

An alternative to the MSVC toolchain that builds against Qt 6 from MSYS2.

1. Install [MSYS2](https://www.msys2.org/).
2. From the **MSYS2 MINGW64** shell, install the toolchain and dependencies:

```bash
pacman -S --needed \
    mingw-w64-x86_64-gcc \
    mingw-w64-x86_64-qt6-base \
    mingw-w64-x86_64-tbb \
    make
```

3. Build from the same shell:

```bash
cd /c/path/to/autoremesher
qmake
make -j$(nproc)
```

The binary will be at `release/autoremesher.exe`. Run it from the MINGW64 shell so the Qt 6 and oneTBB DLLs on `/mingw64/bin` are found, or bundle the dependencies next to the binary with `windeployqt-qt6 release/autoremesher.exe`.

#### macOS

```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install dependencies via Homebrew
brew install qt@5 tbb cmake

# Build
export PATH="/usr/local/opt/qt@5/bin:$PATH"
git clone https://github.com/KajetanMuczynski/autoremesher.git
cd autoremesher
qmake CONFIG+=sdk_no_version_check
make -j$(sysctl -n hw.logicalcpu)
```

### Running a quick test

AutoRemesher has a CLI mode for headless processing. Try it with one of the [common-3d-test-models](https://github.com/alecjacobson/common-3d-test-models):

```bash
./autoremesher \
    --input armadillo.obj \
    --output remeshed.obj \
    --report remeshed_report.txt \
    --target-quads 50000 \
    --edge-scaling 1.0 \
    --sharp-edge 90.0 \
    --smooth-normal 0.0 \
    --adaptivity 1.0 \
    --anisotropy 1.0
```

### Prebuilt binaries

This fork has no releases yet, so build it from source as above. Prebuilt binaries of the original AutoRemesher are on the [upstream releases page](https://github.com/huxingyi/autoremesher/releases).

## License

GPL-3.0-or-later, see [LICENSE](LICENSE). Portions derived from AutoRemesher by Jeremy Hu (huxingyi), MIT, see [LICENSE-MIT-upstream](LICENSE-MIT-upstream). Libraries under `thirdparty/` keep their own licenses.

## Acknowledgements

AutoRemesher was created by Jeremy Hu; see [AUTHORS](AUTHORS) and [CONTRIBUTORS](CONTRIBUTORS) for everyone who worked on the original. See [ACKNOWLEDGEMENTS](ACKNOWLEDGEMENTS.html) for the libraries and resources it uses.
