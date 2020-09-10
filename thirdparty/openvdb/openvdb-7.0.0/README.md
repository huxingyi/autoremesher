![OpenVDB](https://www.openvdb.org/images/openvdb_logo.png)

[![Azure](https://dev.azure.com/academysoftwarefoundation/Academy%20Software%20Foundation/_apis/build/status/academysoftwarefoundation.openvdb)](https://dev.azure.com/academysoftwarefoundation/Academy%20Software%20Foundation/_build?definitionId=1&_a=summary)
[![CII Best Practices](https://bestpractices.coreinfrastructure.org/projects/2774/badge)](https://bestpractices.coreinfrastructure.org/projects/2774)

[Website](https://www.openvdb.org) |
[Discussion Forum](https://www.openvdb.org/forum) |
[Documentation](https://www.openvdb.org/documentation/)

OpenVDB is an open source C++ library comprising a novel hierarchical data structure and a large suite of tools for the efficient storage and manipulation of sparse volumetric data discretized on three-dimensional grids. It was developed by DreamWorks Animation for use in volumetric applications typically encountered in feature film production.


### Development Repository

This GitHub repository hosts the trunk of the OpenVDB development. This implies that it is the newest public version with the latest features and bug fixes. However, it also means that it has not undergone a lot of testing and is generally less stable than the [production releases](https://github.com/AcademySoftwareFoundation/openvdb/releases).


### License

OpenVDB is released under the [Mozilla Public License Version 2.0](https://www.mozilla.org/MPL/2.0/), which is a free, open source software license developed and maintained by the Mozilla Foundation.

The trademarks of any contributor to this project may not be used in association with the project without the contributor's express permission.


### Contributing

Developers who wish to contribute code to be considered for inclusion in the OpenVDB distribution must first complete the [Contributor License Agreement](https://www.openvdb.org/download/OpenVDBContributorLicenseAgreement.pdf) and submit it to [openvdb@gmail.com](mailto:openvdb@gmail.com).  We prefer code submissions in the form of pull requests to this repository.  All code should adhere to the OpenVDB [coding standards](https://www.openvdb.org/documentation/doxygen/codingStyle.html), and _every commit must be signed off_.  That is, every commit log message must include a “`Signed-off-by`” line (generated, for example, with “`git commit --signoff`”), indicating that the committer wrote the code and has the right to release it under the [MPL 2.0](https://www.mozilla.org/MPL/2.0/) license. See https://developercertificate.org/ for more information on this requirement.


### Developer Quick Start

See the [build documentation](https://www.openvdb.org/documentation/doxygen/build.html) for help with installations.

#### Linux
##### Installing Dependencies (Boost, TBB, OpenEXR, Blosc)

```
apt-get install -y libboost-iostreams-dev
apt-get install -y libboost-system-dev
apt-get install -y libtbb-dev
apt-get install -y libilmbase-dev
apt-get install -y libopenexr-dev
```
```
git clone git@github.com:Blosc/c-blosc.git
cd c-blosc
git checkout tags/v1.5.0 -b v1.5.0
mkdir build
cd build
cmake ..
make -j4
make install
cd ../..
```

##### Building OpenVDB
```
git clone git@github.com:AcademySoftwareFoundation/openvdb.git
cd openvdb
mkdir build
cd build
cmake ..
make -j4
make install
```
#### macOS
##### Installing Dependencies (Boost, TBB, OpenEXR, Blosc)
```
brew install boost
brew install tbb
brew install ilmbase
brew install openexr
```
```
git clone git@github.com:Blosc/c-blosc.git
cd c-blosc
git checkout tags/v1.5.0 -b v1.5.0
mkdir build
cd build
cmake ..
make -j4
make install
cd ../..
```
##### Building OpenVDB
```
git clone git@github.com:AcademySoftwareFoundation/openvdb.git
cd openvdb
mkdir build
cd build
cmake ..
make -j4
make install
```
#### Windows
##### Installing Dependencies (Boost, TBB, OpenEXR, Blosc)

It is recommended to set the VCPKG_DEFAULT_TRIPLET=x64-windows environment
variable to use 64-bit libraries by default.

```
vcpkg install boost zlib blosc openexr tbb
vcpkg integrate install
```
##### Building OpenVDB
```
git clone git@github.com:AcademySoftwareFoundation/openvdb.git
cd openvdb
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=<PATH_TO_VCPKG>\scripts\buildsystems\vcpkg.cmake ..
make -j4
make install
```
