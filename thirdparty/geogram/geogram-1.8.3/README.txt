# geogram

[![License](https://img.shields.io/badge/License-BSD_3--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)
[![Release](https://github.com/BrunoLevy/geogram/actions/workflows/make_release.yml/badge.svg)](https://github.com/BrunoLevy/geogram/actions/workflows/make_release.yml)
[![Emscripten](https://github.com/BrunoLevy/geogram/actions/workflows/emscripten.yml/badge.svg)](https://github.com/BrunoLevy/geogram/actions/workflows/emscripten.yml)
[![Doxygen](https://github.com/BrunoLevy/geogram/actions/workflows/doxygen.yml/badge.svg)](https://github.com/BrunoLevy/geogram/actions/workflows/doxygen.yml)

[![Continuous](https://github.com/BrunoLevy/geogram/actions/workflows/continuous.yml/badge.svg)](https://github.com/BrunoLevy/geogram/actions/workflows/continuous.yml)
[![Continuous](https://custom-icon-badges.demolab.com/badge/CI-Continuous-lightblue?logo=tasklist&logoColor=white)](https://brunolevy.github.io/geogram/reports/smoke/)

[![Nightly](https://github.com/BrunoLevy/geogram/actions/workflows/nightly.yml/badge.svg)](https://github.com/BrunoLevy/geogram/actions/workflows/nightly.yml)
[![Nightly](https://custom-icon-badges.demolab.com/badge/CI-Nightly-lightblue?logo=tasklist&logoColor=white)](https://brunolevy.github.io/geogram/reports/nightly/)



![](https://github.com/BrunoLevy/geogram/wiki/geogram_banner_2022.png)

Geogram is a programming library with geometric algorithms. It has
geometry-processing functionalities:
- [surface reconstruction](https://github.com/BrunoLevy/geogram/wiki/Reconstruction)
- [remeshing](https://github.com/BrunoLevy/geogram/wiki/Remeshing)
- [parameterization and texturing](https://github.com/BrunoLevy/geogram/wiki/Texturing)

It also has lower-level algorithm:
- [Exact numbers / exact predicates](https://brunolevy.github.io/geogram/multi__precision_8h.html)
- [Delaunay triangulations in 2D](https://github.com/BrunoLevy/geogram/wiki/Delaunay2D)
  and highly efficient parallel [Delaunay triangulations in 3D](https://github.com/BrunoLevy/geogram/wiki/Delaunay3D)
- Memory efficient surfacic/volumetric/hybrid [mesh data structure](https://github.com/BrunoLevy/geogram/wiki/Mesh)
- Efficient [geometric search data structures](https://github.com/BrunoLevy/geogram/wiki/Raytrace) for
  intersection and raytracing (AABBs, KdTrees, ...)
- [Spectral mesh processing](https://github.com/BrunoLevy/geogram/wiki/ManifoldHarmonics)
- [Linear solver on CPU and GPU](https://github.com/BrunoLevy/geogram/wiki/OpenNL)

Geogram contains the main results in Geometry Processing from the former
ALICE Inria project, that is, more than 30 research articles published
in ACM SIGGRAPH, ACM Transactions on Graphics, Symposium on Geometry 
Processing and Eurographics. It was supported by two grants from the
European Research Council (ERC): GOODSHAPE and VORPALINE.

Links
-----
  - [Documentation, how to compile, tutorials....](https://github.com/BrunoLevy/geogram/wiki)
  - [Programmer's reference manuals...](https://brunolevy.github.io/geogram/)  
  - [Releases](https://github.com/BrunoLevy/geogram/releases)
  - [Projects with geogram](https://github.com/BrunoLevy/geogram/wiki/Publications)
  - [Graphite](https://github.com/BrunoLevy/GraphiteThree), an experimental 3D modeler built around geogram.
  - [Geogram in-browser demos](https://github.com/BrunoLevy/geogram/wiki/compiling_Emscripten#examples)
    (How is it possible ? _more on this [here](https://github.com/BrunoLevy/geogram/wiki/compiling_Emscripten)_)

How does it compare to other geometry-processing libraries ?
------------------------------------------------------------

There exists other libraries for geometric computing / geometry processing, such as:
- [CGAL](https://www.cgal.org/)
- [libIGL](https://libigl.github.io/)

It is difficult to compare different "swiss army knives" that have a completely different set of blades.
The specificity of Geogram as compared to these **generic** libraries is that it is targeted towards some
**specific** scenari with no compromise concerning performance and memory consumption. For instance,
Geogram's 3D triangulation can be applied to pointsets with hundred millions points that we use in
[our research in cosmology](https://physics.aps.org/articles/v15/75), where the data structures used
by other library would not fit in memory. This comes at the expense of less genericity
(for instance, CGAL has a function to remove a point from a 3D Delaunay triangulation that Geogram does not have).

Another difference is the programming style and the design choices: Geogram is not a header-only library 
and makes a moderate use of modern C++ programming. This difference is rather a question of programmer's taste.

Geogram comes "with batteries installed" (nearly no external dependancies) and is easy to compile. It does not
depend on BOOST (that we find too heavy, again it is a question of taste).

Geogram works under Linux/Windows/Mac/Android/Emscripten. Emscripten lets you compile programs and include them
in webpages, see [examples here](https://github.com/BrunoLevy/geogram/wiki/compiling_Emscripten#examples).
