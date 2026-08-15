# meshoptimizer (vendored subset)

Upstream: https://github.com/zeux/meshoptimizer
Version: 1.2 (`MESHOPTIMIZER_VERSION 1020`)
License: MIT (see LICENSE.md)

Only the files needed for mesh simplification are vendored:

- `src/meshoptimizer.h`    - public API
- `src/simplifier.cpp`     - `meshopt_simplify`, `meshopt_simplifyScale`
- `src/indexgenerator.cpp` - `meshopt_generateVertexRemap`, `meshopt_remapVertexBuffer`,
                             `meshopt_remapIndexBuffer`

These two translation units link on their own; the rest of the library
(vertex cache optimisation, compression, meshlets) is not used here.

Used by `AutoRemesher::resample` as a decimation pre-pass for inputs that are
too dense for the isotropic remesher to coarsen directly.
