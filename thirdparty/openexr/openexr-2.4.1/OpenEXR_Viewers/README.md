# The OpenEXR_Viewers Programs

The OpenEXR_Viewers library provides code for simple programs that
view EXR images.

* **exrdisplay** - exrdisplay is a simple still image viewer that
optionally applies color transforms to OpenEXR images, using ctl as
explained in this document:

    doc/OpenEXRViewers.pdf

* **playexr** - playexr is a program that plays back OpenEXR image
sequences, optionally with CTL support. The playexr directory contains
the source code for the playback program, and two sample CTL
transforms (one rendering and one display transform).

## Dependencies

Building OpenEXR_Viewers requires the OpenEXR library.

**exrdisplay** requires [FLTK 1.1](http://www.fltk.org/index.php) or
 greater and OpenGL.

**playexr** requires OpenGL, GLUT and the
 [Cg](https://developer.nvidia.com/cg-toolkit) hardware shading
 language.

## License

These libraries are covered under the
[BSD-3-Clause](https://www.openexr.com/license.html) license of the
OpenEXR project.

## Building and Installation

See the top-level [INSTALL](../INSTALL.md) file for information about
building and installing the OpenEXR libraries.

