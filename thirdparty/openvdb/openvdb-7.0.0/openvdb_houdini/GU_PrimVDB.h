// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

/*
 * Copyright (c) Side Effects Software Inc.
 *
 * Produced by:
 *      Side Effects Software Inc
 *      477 Richmond Street West
 *      Toronto, Ontario
 *      Canada   M5V 3E7
 *      416-504-9876
 *
 * NAME:        GU_PrimVDB.h ( GU Library, C++)
 *
 * COMMENTS: Custom VDB primitive.
 */

#include <UT/UT_Version.h>

// Using the native OpenVDB Primitive shipped with Houdini is strongly recommended,
// as there is no guarantee that this code will be kept in sync with Houdini.
// However, for debugging it can be useful, so supply -DSESI_OPENVDB_PRIM to
// the compiler to build this custom primitive.

#if !defined(SESI_OPENVDB) && !defined(SESI_OPENVDB_PRIM)

#include <GU/GU_PrimVDB.h>

namespace openvdb_houdini {
using ::GU_PrimVDB;
}

#else // SESI_OPENVDB || SESI_OPENVDB_PRIM

#ifndef __HDK_GU_PrimVDB__
#define __HDK_GU_PrimVDB__

#include <GA/GA_PrimitiveDefinition.h>
#include "GEO_PrimVDB.h"
#include <GU/GU_Detail.h>
#include <UT/UT_Matrix4.h>
#include <UT/UT_VoxelArray.h>
#include <openvdb/Platform.h>
#include <stddef.h>


class GA_Attribute;
class GEO_PrimVolume;
class UT_MemoryCounter;
class GEO_ConvertParms;
typedef GEO_ConvertParms GU_ConvertParms;


class GU_API GU_PrimVDB : public GEO_PrimVDB
{
protected:
    /// NOTE: Primitives should not be deleted directly.  They are managed
    ///       by the GA_PrimitiveList and the stash.
    virtual ~GU_PrimVDB() {}

public:
    /// NOTE: This constructor should only be called via GU_PrimitiveFactory.
    GU_PrimVDB(GU_Detail *gdp, GA_Offset offset=GA_INVALID_OFFSET)
        : GEO_PrimVDB(gdp, offset)
    {}

    /// Report approximate memory usage.
    virtual int64 getMemoryUsage() const;

    /// Count memory usage using a UT_MemoryCounter in order to count
    /// shared memory correctly.
    /// NOTE: This should always include sizeof(*this).
    virtual void countMemory(UT_MemoryCounter &counter) const;

#ifndef SESI_OPENVDB
    /// Allows you to find out what this primitive type was named.
    static GA_PrimitiveTypeId    theTypeId() { return theDefinition->getId(); }

    /// Must be invoked during the factory callback to add us to the
    /// list of primitives
    static void         registerMyself(GA_PrimitiveFactory *factory);
#endif

    virtual const GA_PrimitiveDefinition &getTypeDef() const
    {
        UT_ASSERT(theDefinition);
        return *theDefinition;
    }

    // Conversion Methods

    virtual GEO_Primitive       *convert(GU_ConvertParms &parms,
                                         GA_PointGroup *usedpts = 0);
    virtual GEO_Primitive       *convertNew(GU_ConvertParms &parms);

    /// Convert all GEO_PrimVolume primitives in geometry to
    /// GEO_PrimVDB, preserving prim/vertex/point attributes (and prim/point
    /// groups if requested).
    static void                 convertVolumesToVDBs(
                                        GU_Detail &dst_geo,
                                        const GU_Detail &src_geo,
                                        GU_ConvertParms &parms,
                                        bool flood_sdf,
                                        bool prune,
                                        fpreal tolerance,
                                        bool keep_original,
                                        bool activate_inside = true);

    /// Convert all GEO_PrimVDB primitives in geometry to parms.toType,
    /// preserving prim/vertex/point attributes (and prim/point groups if
    /// requested).
    /// @{
    static void                 convertVDBs(
                                        GU_Detail &dst_geo,
                                        const GU_Detail &src_geo,
                                        GU_ConvertParms &parms,
                                        fpreal adaptivity,
                                        bool keep_original);
    static void                 convertVDBs(
                                        GU_Detail &dst_geo,
                                        const GU_Detail &src_geo,
                                        GU_ConvertParms &parms,
                                        fpreal adaptivity,
                                        bool keep_original,
                                        bool split_disjoint_volumes);
    /// @}

    // NOTE:  For static member functions please call in the following
    //        manner.  <ptrvalue> = GU_PrimVDB::<functname>
    //        i.e.        partptr = GU_PrimVDB::build(params...);

    // Optional Build Method

    static GU_PrimVDB * build(GU_Detail *gdp, bool append_points = true);

    /// Store a VDB grid in a new VDB primitive and add the primitive
    /// to a geometry detail.
    /// @param gdp   the detail to which to add the new primitive
    /// @param grid  a grid to be associated with the new primitive
    /// @param src   if non-null, copy attributes and groups from this primitive
    /// @param name  if non-null, set the new primitive's @c name attribute to
    ///     this string; otherwise, if @a src is non-null, use its name
    static GU_PrimVDB* buildFromGrid(GU_Detail& gdp, openvdb::GridBase::Ptr grid,
        const GEO_PrimVDB* src = NULL, const char* name = NULL)
    {
        return GU_PrimVDB::buildFromGridAdapter(gdp, &grid, src, name);
    }

    /// Create new VDB primitive from the given native volume primitive
    static GU_PrimVDB * buildFromPrimVolume(
                            GU_Detail &geo,
                            const GEO_PrimVolume &vol,
                            const char *name,
                            const bool flood_sdf = false,
                            const bool prune = false,
                            const float tolerance = 0.0,
                            const bool activate_inside_sdf = true);

    /// A fast method for converting a primitive volume to a polysoup via VDB
    /// into the given gdp. It will _not_ copy attributes because this is a
    /// special case used for display purposes only.
    static void         convertPrimVolumeToPolySoup(
                            GU_Detail &dst_geo,
                            const GEO_PrimVolume &src_vol);

    virtual void        normal(NormalComp &output) const;

    /// @brief Transfer any metadata associated with this primitive's
    /// VDB grid to primitive attributes.
    void syncAttrsFromMetadata();

    /// @brief Transfer any metadata associated with a VDB grid
    /// to primitive attributes on a VDB primitive.
    /// @param prim  the primitive to be populated with attributes
    /// @param grid  the grid whose metadata should be transferred
    /// @param gdp   the detail to which to transfer attributes
    static void createGridAttrsFromMetadata(
        const GEO_PrimVDB& prim,
        const openvdb::GridBase& grid,
        GEO_Detail& gdp)
    {
        GU_PrimVDB::createGridAttrsFromMetadataAdapter(prim, &grid, gdp);
    }

    /// @brief Transfer any metadata associated with the given MetaMap
    /// to attributes on the given element specified by owner.
    /// @param owner    the type of element
    /// @param element  the offset of the element
    /// @param meta_map the metadata that should be transferred
    /// @param gdp      the detail to which to transfer attributes
    static void createAttrsFromMetadata(
        GA_AttributeOwner owner,
        GA_Offset element,
        const openvdb::MetaMap& meta_map,
        GEO_Detail& gdp)
    {
        GU_PrimVDB::createAttrsFromMetadataAdapter(owner, element, &meta_map, gdp);
    }

    /// @brief Transfer a VDB primitive's attributes to a VDB grid as metadata.
    /// @param grid  the grid to be populated with metadata
    /// @param prim  the primitive whose attributes should be transferred
    /// @param gdp   the detail from which to retrieve primitive attributes
    static void createMetadataFromGridAttrs(
        openvdb::GridBase& grid,
        const GEO_PrimVDB& prim,
        const GEO_Detail& gdp)
    {
        GU_PrimVDB::createMetadataFromGridAttrsAdapter(&grid, prim, gdp);
    }

    /// @brief Transfer attributes to VDB metadata.
    /// @param meta_map  the output metadata
    /// @param owner     the type of element
    /// @param element   the offset of the element
    /// @param geo       the detail from which to retrieve primitive attributes
    static void createMetadataFromAttrs(
        openvdb::MetaMap& meta_map,
        GA_AttributeOwner owner,
        GA_Offset element,
        const GEO_Detail& geo)
    {
        GU_PrimVDB::createMetadataFromAttrsAdapter(&meta_map, owner, element, geo);
    }

private: // METHODS

    /// Add a border of the given radius by evaluating from the given volume.
    /// It assumes that the VDB is a float grid and that the voxel array has
    /// the same index space, so this can really only be safely called after
    /// buildFromPrimVolume(). This is used to ensure that non-constant borders
    /// can be converted at the expense of some extra memory.
    void                expandBorderFromPrimVolume(
                            const GEO_PrimVolume &vol,
                            int border_radius);

    GEO_Primitive *     convertToNewPrim(
                            GEO_Detail &dst_geo,
                            GU_ConvertParms &parms,
                            fpreal adaptivity,
                            bool split_disjoint_volumes,
                            bool &success) const;
    GEO_Primitive *     convertToPrimVolume(
                            GEO_Detail &dst_geo,
                            GU_ConvertParms &parms,
                            bool split_disjoint_volumes) const;
    GEO_Primitive *     convertToPoly(
                            GEO_Detail &dst_geo,
                            GU_ConvertParms &parms,
                            fpreal adaptivity,
                            bool buildpolysoup,
                            bool &success) const;

    static GU_PrimVDB*  buildFromGridAdapter(
                            GU_Detail& gdp,
                            void* grid,
                            const GEO_PrimVDB*,
                            const char* name);
    static void         createGridAttrsFromMetadataAdapter(
                            const GEO_PrimVDB& prim,
                            const void* grid,
                            GEO_Detail& gdp);
    static void         createMetadataFromGridAttrsAdapter(
                            void* grid,
                            const GEO_PrimVDB&,
                            const GEO_Detail&);

    static void createAttrsFromMetadataAdapter(
        GA_AttributeOwner owner,
        GA_Offset element,
        const void* meta_map_ptr,
        GEO_Detail& geo);

    static void createMetadataFromAttrsAdapter(
        void* meta_map_ptr,
        GA_AttributeOwner owner,
        GA_Offset element,
        const GEO_Detail& geo);

private: // DATA

    static GA_PrimitiveDefinition       *theDefinition;
    friend class                         GU_PrimitiveFactory;
    SYS_DEPRECATED_PUSH_DISABLE()
};
    SYS_DEPRECATED_POP_DISABLE()


#ifndef SESI_OPENVDB
namespace openvdb_houdini {
using ::GU_PrimVDB;
} // namespace openvdb_houdini
#endif

#endif // __HDK_GU_PrimVDB__

#endif // SESI_OPENVDB || SESI_OPENVDB_PRIM
