## Release 7.62

1. Corrected two bugs:
  - GmfOpenMesh: could crash with a stack overflow a present some security issues
  - test\_libmeshb\_pipeline.f: the Fortran version of the user's procedure call was crashing

2. New helpers functions system to easily add specific features related to the libMeshb:
  - See the helper's [readme](utilities/libmeshb7_helpers.md) for more information about the new functions to handle polyhedral meshes.
