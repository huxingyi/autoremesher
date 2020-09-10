# The PyIlmBase Libraries

The PyIlmBase libraries provides python bindings for the IlmBase
libraries:

* **PyIex** - bindings for Iex
* **PyImath** - bindings for Imath
* **PyImathNumpy** - bindings that convert between numpy and Imath arrays

In addition, the distribution also includes confidence tests:

* **PyIexTest**
* **PyImathTest**
* **PyImathNumpyTest**

## Dependencies

PyIlmBase requires **numpy** to be available to the builder. Install with
your favorite package manager or use a Python virtualenv:

    virtualenv numpy
    source numpy/bin/activate
    pip install numpy

## License

These libraries are covered under the
[BSD-3-Clause](https://www.openexr.com/license.html) license of the
OpenEXR project.

## Building and Installation

See the top-level [INSTALL](../INSTALL.md) file for information about
building and installing the OpenEXR libraries.


