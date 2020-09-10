// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#include "openvdb.h"
#include "io/DelayedLoadMetadata.h"
//#ifdef OPENVDB_ENABLE_POINTS
#include "points/PointDataGrid.h"
//#endif
#include "tools/PointIndexGrid.h"
#include "util/logging.h"
#include <tbb/mutex.h>
#ifdef OPENVDB_USE_BLOSC
#include <blosc.h>
#endif

// If using an OPENVDB_ABI_VERSION_NUMBER that has been deprecated, issue an error
// directive. This can be optionally suppressed by defining OPENVDB_USE_DEPRECATED_ABI.
// Do this in the .cc of openvdb.cc to ensure this decision is made at the time of
// building the core library
#ifndef OPENVDB_USE_DEPRECATED_ABI
    #if OPENVDB_ABI_VERSION_NUMBER == 4
        #error ABI = 4 is deprecated, define OPENVDB_USE_DEPRECATED_ABI to suppress this error
    #elif OPENVDB_ABI_VERSION_NUMBER < 4
        #error ABI <= 3 is no longer supported
    #endif
#endif

namespace openvdb {
OPENVDB_USE_VERSION_NAMESPACE
namespace OPENVDB_VERSION_NAME {

typedef tbb::mutex Mutex;
typedef Mutex::scoped_lock Lock;

namespace {
// Declare this at file scope to ensure thread-safe initialization.
Mutex sInitMutex;
bool sIsInitialized = false;
}

void
initialize()
{
    Lock lock(sInitMutex);
    if (sIsInitialized) return;

    logging::initialize();

    // Register metadata.
    Metadata::clearRegistry();
    BoolMetadata::registerType();
    DoubleMetadata::registerType();
    FloatMetadata::registerType();
    Int32Metadata::registerType();
    Int64Metadata::registerType();
    StringMetadata::registerType();
    Vec2IMetadata::registerType();
    Vec2SMetadata::registerType();
    Vec2DMetadata::registerType();
    Vec3IMetadata::registerType();
    Vec3SMetadata::registerType();
    Vec3DMetadata::registerType();
    Vec4IMetadata::registerType();
    Vec4SMetadata::registerType();
    Vec4DMetadata::registerType();
    Mat4SMetadata::registerType();
    Mat4DMetadata::registerType();

    // Register maps
    math::MapRegistry::clear();
    math::AffineMap::registerMap();
    math::UnitaryMap::registerMap();
    math::ScaleMap::registerMap();
    math::UniformScaleMap::registerMap();
    math::TranslationMap::registerMap();
    math::ScaleTranslateMap::registerMap();
    math::UniformScaleTranslateMap::registerMap();
    math::NonlinearFrustumMap::registerMap();

    // Register common grid types.
    GridBase::clearRegistry();
    BoolGrid::registerGrid();
    MaskGrid::registerGrid();
    FloatGrid::registerGrid();
    DoubleGrid::registerGrid();
    Int32Grid::registerGrid();
    Int64Grid::registerGrid();
    StringGrid::registerGrid();
    Vec3IGrid::registerGrid();
    Vec3SGrid::registerGrid();
    Vec3DGrid::registerGrid();

    // Register types associated with point index grids.
    Metadata::registerType(typeNameAsString<PointIndex32>(), Int32Metadata::createMetadata);
    Metadata::registerType(typeNameAsString<PointIndex64>(), Int64Metadata::createMetadata);
    tools::PointIndexGrid::registerGrid();

    // Register types associated with point data grids.
//#ifdef OPENVDB_ENABLE_POINTS
    points::internal::initialize();
//#endif

    // Register delay load metadata
    io::DelayedLoadMetadata::registerType();

#ifdef OPENVDB_USE_BLOSC
    blosc_init();
    if (blosc_set_compressor("lz4") < 0) {
        OPENVDB_LOG_WARN("Blosc LZ4 compressor is unavailable");
    }
    /// @todo blosc_set_nthreads(int nthreads);
#endif

#ifdef __ICC
// Disable ICC "assignment to statically allocated variable" warning.
// This assignment is mutex-protected and therefore thread-safe.
__pragma(warning(disable:1711))
#endif

    sIsInitialized = true;

#ifdef __ICC
__pragma(warning(default:1711))
#endif
}


void
uninitialize()
{
    Lock lock(sInitMutex);

#ifdef __ICC
// Disable ICC "assignment to statically allocated variable" warning.
// This assignment is mutex-protected and therefore thread-safe.
__pragma(warning(disable:1711))
#endif

    sIsInitialized = false;

#ifdef __ICC
__pragma(warning(default:1711))
#endif

    Metadata::clearRegistry();
    GridBase::clearRegistry();
    math::MapRegistry::clear();

//#ifdef OPENVDB_ENABLE_POINTS
    points::internal::uninitialize();
//#endif

#ifdef OPENVDB_USE_BLOSC
    // We don't want to destroy Blosc, because it might have been
    // initialized by some other library.
    //blosc_destroy();
#endif
}

} // namespace OPENVDB_VERSION_NAME
} // namespace openvdb
