**Deprecation Strategy for OpenVDB**

OpenVDB is committed to supporting three years of
[VFX Reference Platform](http://www.vfxplatform.com/) and all releases of
Houdini and Maya based on those versions of the platform. The latest supported
year is that in which the VDB version listed matches the major version.

For example, version 6.1.0 of OpenVDB supports VFX Reference Platform years
2019, 2018 and 2017.

This infers the following support:

* OpenVDB ABI=4, ABI=5 and ABI=6
* C++11 and C++14
* Houdini 16.5, 17.0 and 17.5

When version 7.0.0 is released, OpenVDB will support VFX Reference Platform
years 2020, 2019 and 2018. Support for Houdini 16.5 and C++11 will be dropped.

Support for obsolete ABIs will not be dropped until the first minor release
after the introduction of a new ABI. For example, the latest version to retain
support for ABI=4 will be the release prior to 7.1.0.
