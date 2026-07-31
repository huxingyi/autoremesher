# AutoRemesher

AutoRemesher is a cross-platform automatic quad remeshing tool. It converts dense, high-polygon
triangle meshes into clean quad-dominant topology suitable for animation, sculpting and further
authoring.

This repository is a fork of [huxingyi/autoremesher](https://github.com/huxingyi/autoremesher).
It keeps the original remeshing core and adds a game-asset oriented I/O layer, symmetry support,
density presets and a hardened command-line mode. See [What's new](#whats-new-in-110) below.

The remeshing core is built on [Geogram](https://github.com/BrunoLevy/geogram),
[libigl](https://github.com/libigl), [isotropicremesher](https://github.com/huxingyi/isotropicremesher)
and [others](ACKNOWLEDGEMENTS.html).

---

## What's new in 1.1.0

| Area | Change |
| --- | --- |
| Mesh I/O | FBX and glTF/GLB import in addition to OBJ; OBJ, GLB and FBX export |
| Symmetry | Optional mirror-symmetric remeshing across the X, Y or Z axis |
| Presets | Low / Medium / High density presets, scaled to the loaded model's triangle count |
| Parameters | Every parameter accepts direct numeric entry as well as slider input |
| Workflow | Drag and drop a model onto the window to load it |
| CLI | All arguments are range-validated; the process exits non-zero on bad input or an empty result |
| Versioning | A single `version.pri` feeds the build, the About dialog and the Windows file properties |
| Build | `build.bat` / `quickbuild.bat` discover Qt and MSVC automatically, with env-var overrides |

---

## Supported formats

| Format | Import | Export | Notes |
| --- | --- | --- | --- |
| Wavefront OBJ (`.obj`) | Yes | Yes | Quads are written as quads |
| Autodesk FBX (`.fbx`) | Yes | Yes | Binary FBX; quads are preserved |
| glTF / GLB (`.gltf`, `.glb`) | Yes | Yes | Export always writes binary GLB and triangulates, because glTF has no quad primitive. A `.gltf` target is written as `.glb` alongside it |

Import reads geometry only. Materials, UVs and textures on the input model are not carried over to
the remeshed output.

---

## Getting started

### Prerequisites

- A C++ compiler with C++14 support (GCC, Clang or MSVC)
- Qt 5.15.2
- TBB (Intel Threading Building Blocks)
- CMake 3.12 or later — Windows only, to build the bundled TBB from source

### Windows (Visual Studio 2022)

1. Install [Visual Studio 2022](https://visualstudio.microsoft.com/) with the
   **Desktop development with C++** workload.
2. Install [CMake](https://cmake.org/download/).
3. Install Qt 5.15.2 with the [online installer](https://www.qt.io/download-open-source) and
   select the `msvc2019_64` component.
4. Run `build.bat` from the repository root and choose **[1] Full Build & Deploy**.

`build.bat` locates the MSVC toolchain through `vswhere` and probes the standard Qt install
layout. Override either if your setup is non-standard:

```cmd
set "QTDIR=D:\Qt\5.15.2\msvc2019_64"
set "VCVARS=D:\VisualStudio\VC\Auxiliary\Build\vcvars64.bat"
build.bat
```

The menu also covers incremental rebuilds, cleaning, running the binary and setting the version.
`quickbuild.bat` is the non-interactive equivalent of an incremental release rebuild.

To build without the scripts, from an **x64 Native Tools Command Prompt for VS 2022**:

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

The release binary lands at `release\autoremesher.exe`. It needs `tbb.dll` from
`thirdparty\tbb\build2\Release\` and the Qt runtime next to it —
`windeployqt release\autoremesher.exe` handles the latter.

### Linux (Ubuntu/Debian)

```bash
sudo apt install build-essential qt5-qmake qtbase5-dev qttools5-dev-tools \
    libqt5svg5-dev libqt5multimedia5-dev libtbb-dev libgl1-mesa-dev

qmake
make -j$(nproc)
```

On Fedora: `sudo dnf install gcc-c++ qt5-qtbase-devel qt5-qttools-devel tbb-devel mesa-libGL-devel`

### macOS

```bash
xcode-select --install
brew install qt@5 tbb cmake

export PATH="/usr/local/opt/qt@5/bin:$PATH"
qmake CONFIG+=sdk_no_version_check
make -j$(sysctl -n hw.logicalcpu)
```

---

## Using the application

Open a model with the **Open** button or by dropping the file onto the window, adjust the
parameters, then start the remesh.

### Presets

The three preset buttons fill in a full parameter set at once. Quad targets are derived from the
loaded model rather than fixed, so the same preset behaves consistently across models of different
density:

| Preset | Target quads | Sharp edge | Smooth normal | Adaptivity |
| --- | --- | --- | --- | --- |
| Low | 10% of input triangles | 120° | 60° | 0.25 |
| Medium | 25% of input triangles | 105° | 45° | 0.25 |
| High | 50% of input triangles | 90° | 30° | 0.25 |

Targets are clamped to the range 1,000 – 1,000,000 quads. Editing any parameter by hand clears the
active preset selection.

### Parameters

| Parameter | Range | Meaning |
| --- | --- | --- |
| Target quads | ≥ 100 | Approximate quad count of the result |
| Edge scaling | > 0.0 | Multiplier on the computed target edge length |
| Sharp edge | 0.0 – 180.0 | Dihedral angle above which an edge is preserved as a feature edge |
| Smooth normal | 0.0 – 180.0 | Normal blending threshold. 0 keeps the surface faceted; higher values follow the original vertex normals more closely |
| Adaptivity | 0.0 – 1.0 | Curvature-adaptive quad density. 0 is uniform, 1 puts finer quads in high-curvature regions |

### Symmetry

Enabling symmetry cuts the model at the chosen axis plane, remeshes one half, then mirrors and
welds the result, so the output topology is symmetric rather than merely close to symmetric. That
matters for characters and other mirrored assets. Symmetry is available in the graphical interface
only; the command-line mode does not expose it.

---

## Command-line mode

Passing `--input` switches AutoRemesher to headless operation. `--output` is then required.

```bash
autoremesher \
    --input armadillo.obj \
    --output remeshed.obj \
    --report remeshed_report.txt \
    --target-quads 50000 \
    --edge-scaling 1.0 \
    --sharp-edge 90.0 \
    --smooth-normal 0.0 \
    --adaptivity 1.0
```

| Option | Default | Accepted values |
| --- | --- | --- |
| `--input`, `-i` | — | Path to an OBJ, FBX, glTF or GLB file. Presence of this flag selects headless mode |
| `--output`, `-o` | — | Output path. Required in headless mode. The extension selects the format |
| `--report` | not written | Path for a plain-text run report (quads, non-quads, vertices, elapsed time) |
| `--target-quads` | 50000 | Integer ≥ 100 |
| `--edge-scaling` | 1.0 | Number > 0.0 |
| `--sharp-edge` | 90.0 | 0.0 – 180.0 |
| `--smooth-normal` | 0.0 | 0.0 – 180.0 |
| `--adaptivity` | 1.0 | 0.0 – 1.0 |

Every value is validated before any work starts; an out-of-range or non-numeric argument is
reported on stderr and the process exits immediately.

### Exit codes

| Code | Meaning |
| --- | --- |
| `0` | Remeshing succeeded and produced at least one quad |
| `1` | Invalid or out-of-range argument, missing `--output`, or the run produced no quads |

The non-zero exit on an empty result is what makes the tool usable from a batch pipeline: a run
that silently produced nothing is reported as a failure rather than a success. A report path that
could not be written is a warning on stderr and does not by itself fail the run.

---

## Versioning

`version.pri` is the single source of truth:

```
HUMAN_VERSION = 1.1.0
VERSION = 1.1.0.0
```

`autoremesher.pro` includes it and turns the values into compile-time defines, so the About dialog
and — on Windows — the executable's file properties always match. `build.bat` option
**[5] Set Version** rewrites the file and deletes the object files that bake the version in, so the
next build actually picks the change up. A value passed on the qmake command line still wins over
the file.

---

## Repository layout

```
src/                  Application sources
  AutoRemesher/       Remeshing core (parameterization, quad extraction, symmetry)
  meshio.cpp/.h       OBJ / FBX / glTF import and export
thirdparty/           Vendored dependencies (geogram, tbb, eigen, ufbx, cgltf, ...)
Docs/research/        Design research notes on UV unwrapping and texture transfer
shaders/, resources/  Assets compiled into the binary
build.bat             Interactive Windows build manager
quickbuild.bat        Non-interactive incremental release rebuild
build-env.bat         Shared Qt / MSVC toolchain discovery used by both scripts
version.pri           Application version
```

---

## License

AutoRemesher is licensed under the MIT License — see [LICENSE](LICENSE).

## Acknowledgements

The original AutoRemesher was created by Jeremy Hu and contributors; see
[AUTHORS](AUTHORS), [CONTRIBUTORS](CONTRIBUTORS) and [SUPPORTERS](SUPPORTERS).
The full list of libraries and resources used is in [ACKNOWLEDGEMENTS.html](ACKNOWLEDGEMENTS.html).

Added in this fork: FBX import/export via [ufbx](https://github.com/ufbx/ufbx) and glTF/GLB import
via [cgltf](https://github.com/jkuhlmann/cgltf).
