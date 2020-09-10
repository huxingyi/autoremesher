// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file geoemetry.h
/// @author FX R&D OpenVDB team
///
/// @brief A collection of Houdini geometry related methods and helper functions.

#ifndef HOUDINI_UTILS_GEOMETRY_HAS_BEEN_INCLUDED
#define HOUDINI_UTILS_GEOMETRY_HAS_BEEN_INCLUDED

#include <UT/UT_VectorTypes.h>
#include <GU/GU_Detail.h>

#if defined(PRODDEV_BUILD) || defined(DWREAL_IS_DOUBLE) || defined(SESI_OPENVDB)
  // OPENVDB_HOUDINI_API, which has no meaning in a DWA build environment but
  // must at least exist, is normally defined by including openvdb/Platform.h.
  // For DWA builds (i.e., if either PRODDEV_BUILD or DWREAL_IS_DOUBLE exists),
  // that introduces an unwanted and unnecessary library dependency.
  #ifndef OPENVDB_HOUDINI_API
    #define OPENVDB_HOUDINI_API
  #endif
#else
  #include <openvdb/Platform.h>
#endif

namespace houdini_utils {

/// @brief Add geometry to the given GU_Detail to create a box with the given corners.
/// @param corners  the eight corners of the box
/// @param color    an optional color for the added geometry
/// @param shaded   if false, generate a wireframe box; otherwise, generate a solid box
/// @param alpha    an optional opacity for the added geometry
OPENVDB_HOUDINI_API void createBox(GU_Detail&, UT_Vector3 corners[8],
    const UT_Vector3* color = nullptr, bool shaded = false, float alpha = 1.0);

} // namespace houdini_utils

#endif // HOUDINI_UTILS_GEOMETRY_HAS_BEEN_INCLUDED
