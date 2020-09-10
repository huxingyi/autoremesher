// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file SOP_OpenVDB_Convert.cc
///
/// @author SESI and FX R&D OpenVDB team

#include <houdini_utils/ParmFactory.h>
#include <openvdb_houdini/SOP_NodeVDB.h>
#include <openvdb_houdini/GeometryUtil.h>
#include <openvdb_houdini/AttributeTransferUtil.h>
#include <openvdb_houdini/Utils.h>

#include <openvdb/tools/LevelSetUtil.h>
#include <openvdb/tools/Mask.h> // for tools::interiorMask()
#include <openvdb/tools/MeshToVolume.h>
#include <openvdb/tools/Morphology.h>
#include <openvdb/tools/VolumeToMesh.h>
#include <openvdb/tools/Prune.h>
#include <openvdb/tree/ValueAccessor.h>

#include <CH/CH_Manager.h>
#include <GA/GA_PageIterator.h>
#include <GU/GU_ConvertParms.h>
#include <GU/GU_PrimPoly.h>
#include <GU/GU_PrimPolySoup.h>
#include <SYS/SYS_Math.h>
#include <UT/UT_Interrupt.h>
#include <UT/UT_Version.h>
#include <UT/UT_VoxelArray.h>
#include <UT/UT_UniquePtr.h>

#include <hboost/algorithm/string/join.hpp>

#include <limits>
#include <list>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>


namespace hvdb = openvdb_houdini;
namespace hutil = houdini_utils;


namespace {
enum ConvertTo { HVOLUME, OPENVDB, POLYGONS, POLYSOUP };
enum ConvertClass { CLASS_NO_CHANGE, CLASS_SDF, CLASS_FOG_VOLUME };
}


class SOP_OpenVDB_Convert: public hvdb::SOP_NodeVDB
{
public:
    SOP_OpenVDB_Convert(OP_Network*, const char* name, OP_Operator*);
    ~SOP_OpenVDB_Convert() override {}

    static OP_Node* factory(OP_Network*, const char* name, OP_Operator*);

    // Return true for a given input if the connector to the input
    // should be drawn dashed rather than solid.
    int isRefInput(unsigned idx) const override { return (idx == 1); }

protected:
    bool updateParmsFlags() override;

public:
    class Cache: public SOP_VDBCacheOptions
    {
    protected:
        OP_ERROR cookVDBSop(OP_Context&) override;

    private:
        void convertVDBType(
            GU_Detail&,
            GA_PrimitiveGroup*,
            const UT_String& newTypeStr,
            const UT_String& newPrecisionStr,
            hvdb::Interrupter&);

        void convertToPoly(
            fpreal time,
            GA_PrimitiveGroup*,
            bool buildpolysoup,
            hvdb::Interrupter&);

        template<class GridType>
        void referenceMeshing(
            std::list<openvdb::GridBase::ConstPtr>& grids,
            std::list<const GU_PrimVDB*> vdbs,
            GA_PrimitiveGroup *group,
            openvdb::tools::VolumeToMesh& mesher,
            const GU_Detail* refGeo,
            bool computeNormals,
            hvdb::Interrupter& boss,
            const fpreal time);
    }; // class Cache
};


// Build UI and register this operator.
void
newSopOperator(OP_OperatorTable* table)
{
    if (table == nullptr) return;

    hutil::ParmList parms;

    parms.add(hutil::ParmFactory(PRM_STRING, "group", "Group")
        .setChoiceList(&hutil::PrimGroupMenuInput1)
        .setTooltip("Specify a subset of the input primitives to convert.")
        .setDocumentation(
            "A subset of the input primitives to be converted"
            " (see [specifying volumes|/model/volumes#group])"));


    // Convert To Menu
    parms.add(hutil::ParmFactory(PRM_ORD, "conversion", "Convert To")
        .setDefault(PRMzeroDefaults)
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, {
            "volume",   "Volume",
            "vdb",      "VDB",
            "poly",     "Polygons",
            "polysoup", "Polygon Soup"
        })
    .setDocumentation("\
The type of conversion to perform\n\
\n\
Volume:\n\
    Convert a VDB volume into a dense Houdini volume.\n\
\n\
    This allows legacy tools to operate on the primitive,\n\
    however the memory requirements of dense volumes with effective\n\
    resolutions over 1000<sup>3</sup> might be prohibitive.\n\
    Consider using the __Split Disjoint Volumes__ option.\n\
\n\
VDB:\n\
    Convert a Houdini volume into a VDB volume.\n\
\n\
    By default, the resulting VDB will be of the same class as the input,\n\
    so a fog volume becomes a fog VDB and an SDF volume becomes an SDF VDB.\n\
\n\
Polygons:\n\
    Generate a polygonal mesh representing an isosurface of a VDB volume.\n\
\n\
Polygon Soup:\n\
    Generate a polygonal mesh representing an isosurface of a VDB volume.\n\
\n\
    The mesh is stored as a polygon soup, which is more compact than\n\
    an ordinary mesh but does not support most editing operations.\n"));

    // Grid Class Menu
    parms.add(hutil::ParmFactory(PRM_ORD, "vdbclass", "VDB Class")
        .setDefault(PRMzeroDefaults)
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, {
            "none", "No Change",
            "sdf",  "Convert Fog to SDF",
            "fog",  "Convert SDF to Fog"
        })
        .setTooltip("Convert fog volumes to signed distance fields or vice versa."));

    parms.add(hutil::ParmFactory(PRM_STRING, "vdbtype", "VDB Type")
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, {
            "none",   "No Change",
            "float",  "Float",
            "int",    "Integer",
            "bool",   "Bool",
            "vec3f",  "Vector Float",
            "vec3i",  "Vector Integer"
        })
        .setDefault("none")
        .setTooltip("Change the type of value stored at each voxel.")
        .setDocumentation(
            "Change the type of value stored at each voxel.\n\n"
            "When converting from a scalar type to a vector type, the scalar value\n"
            "is copied to each vector component.\n\n"
            "When converting from a vector type to a scalar type, voxel values are\n"
            "lost&mdash;only voxel topology is preserved.\n\n"
            "This option is not available when VDB class conversion is enabled,\n"
            "since SDFs and fog volumes always have scalar, floating-point values.\n"));

    parms.add(hutil::ParmFactory(PRM_STRING, "vdbprecision", "VDB Precision")
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, {
            "none", "No Change",
            "32",   "32-bit",
            "64",   "64-bit"
        })
        .setDefault("none")
        .setTooltip("Change the numerical precision of the value stored at each voxel."));

    //////////

    // Parms for converting to volumes

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "splitdisjointvolumes", "Split Disjoint Volumes")
        .setTooltip(
            "When converting to volumes, where possible create a separate"
            " volume primitive for each connected component of a VDB."
            " This allows very large and sparse VDBs to be converted"
            " with a reduced memory footprint."));

    //////////

    // Parms for converting to polygons

    parms.add(hutil::ParmFactory(PRM_FLT_J, "isoValue", "Isovalue")
        .setRange(PRM_RANGE_UI, -1.0, PRM_RANGE_UI, 1.0)
        .setTooltip("The crossing point of the VDB values that is considered "
            "the surface when converting to polygons"));

    parms.add(hutil::ParmFactory(PRM_FLT_J, "fogisovalue", "Fog Isovalue")
        .setRange(PRM_RANGE_UI, 0.0, PRM_RANGE_UI, 1.0)
        .setDefault(PRMpointFiveDefaults)
        .setTooltip("The crossing point of the VDB values that is considered "
            "the surface when converting to level sets from fog volumes"));

    parms.add(hutil::ParmFactory(PRM_FLT_J, "adaptivity", "Adaptivity")
        .setRange(PRM_RANGE_RESTRICTED, 0.0, PRM_RANGE_RESTRICTED, 2.0)
        .setTooltip("When converting to polygons, the adaptivity threshold determines "
            "how closely the isosurface is matched by the resulting mesh. Higher "
            "thresholds will allow more variation in polygon size, using fewer "
            "polygons to express the surface."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "computenormals", "Compute Vertex Normals")
        .setTooltip("Compute edge-preserving vertex normals."));

    //////////

    // Reference input options

    parms.add(hutil::ParmFactory(PRM_FLT_J, "internaladaptivity", "Internal Adaptivity")
        .setRange(PRM_RANGE_RESTRICTED, 0.0, PRM_RANGE_RESTRICTED, 1.0)
        .setTooltip("When converting to polygons with a second input, this overrides "
            "the adaptivity threshold for all internal surfaces."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "transferattributes", "Transfer Surface Attributes")
        .setTooltip(
            "Transfer all attributes (primitive, vertex and point) from the reference surface.")
        .setDocumentation(
            "When a reference surface is provided, this option transfers all attributes\n"
            "(primitive, vertex and point) from the reference surface to the output geometry.\n"
            "\n"
            "NOTE:\n"
            "    Primitive attribute values can't meaningfully be transferred to a\n"
            "    polygon soup, because the entire polygon soup is a single primitive.\n"
            "\n"
            "NOTE:\n"
            "    Computed vertex normals for primitives in the surface group\n"
            "    will be overridden.\n"));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "sharpenfeatures", "Sharpen Features")
        .setTooltip("Sharpen edges and corners."));

    parms.add(hutil::ParmFactory(PRM_FLT_J, "edgetolerance", "Edge Tolerance")
        .setDefault(0.5)
        .setRange(PRM_RANGE_RESTRICTED, 0.0, PRM_RANGE_RESTRICTED, 1.0)
        .setTooltip("Controls the edge adaptivity mask"));

    parms.add(hutil::ParmFactory(PRM_STRING, "surfacegroup", "Surface Group")
        .setDefault("surface_polygons")
        .setTooltip("When converting to polygons with a second input, this "
            "specifies a group for all polygons that are coincident with the "
            "reference surface. This group is useful for transferring attributes such "
            "as uv coordinates, normals etc. from the reference surface."));

    parms.add(hutil::ParmFactory(PRM_STRING, "interiorgroup", "Interior Group")
        .setDefault("interior_polygons")
        .setTooltip("When converting to polygons with a second input, this "
            "specifies a group for all polygons that are interior to the "
            "reference surface. This group can be used to identify surface regions "
            "that might require projected uv coordinates or new materials."));

    parms.add(hutil::ParmFactory(PRM_STRING, "seamlinegroup", "Seam Line Group")
        .setDefault("seam_polygons")
        .setTooltip("When converting to polygons with a second input, this "
            "specifies a group for all polygons that are in proximity to "
            "the seam lines. This group can be used to drive secondary elements such "
            "as debris and dust."));

    parms.add(hutil::ParmFactory(PRM_STRING, "seampoints", "Seam Points")
        .setDefault("seam_points")
        .setTooltip(
            "When converting to polygons with a second input, this specifies"
            " a group of the fracture seam points. This can be used to drive"
            " local pre-fracture dynamics such as local surface buckling."));

    //////////

    // Mask input options

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "surfacemask", "")
        .setDefault(PRMoneDefaults)
        .setTypeExtended(PRM_TYPE_TOGGLE_JOIN)
        .setTooltip("Enable / disable the surface mask"));

    parms.add(hutil::ParmFactory(PRM_STRING, "surfacemaskname", "Surface Mask")
        .setChoiceList(&hutil::PrimGroupMenuInput3)
        .setTooltip(
            "A single VDB whose active voxels or (if the VDB is a level set or SDF)\n"
            "interior voxels define the region to be meshed"));

    parms.add(hutil::ParmFactory(PRM_FLT_J, "surfacemaskoffset", "Mask Offset")
        .setDefault(PRMzeroDefaults)
        .setRange(PRM_RANGE_UI, -1.0, PRM_RANGE_UI, 1.0)
        .setTooltip(
            "Isovalue that determines the interior of the surface mask\n"
            "when the mask is a level set or SDF"));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "invertmask", "Invert Surface Mask")
        .setTooltip("If enabled, mesh the complement of the mask."));


    parms.add(hutil::ParmFactory(PRM_TOGGLE, "adaptivityfield", "")
        .setTypeExtended(PRM_TYPE_TOGGLE_JOIN)
        .setTooltip("Enable / disable the the adaptivity field"));

    parms.add(hutil::ParmFactory(PRM_STRING, "adaptivityfieldname", "Adaptivity Field")
        .setTooltip(
            "A single scalar grid used as a spatial multiplier for the adaptivity threshold")
        .setChoiceList(&hutil::PrimGroupMenuInput3));

    //////////

    // Parms for converting to volumes

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "prune", "")
        .setDefault(PRMoneDefaults)
        .setTypeExtended(PRM_TYPE_TOGGLE_JOIN)
        .setTooltip("Collapse regions of constant value in output grids. "
            "Voxel values are considered equal if they differ "
            "by less than the specified threshold.")
        .setDocumentation(nullptr));

    parms.add(hutil::ParmFactory(PRM_FLT_J, "tolerance", "Prune Tolerance")
        .setDefault(PRMzeroDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_UI, 1)
        .setTooltip(
            "When pruning is enabled, voxel values are considered equal"
            " if they differ by less than the specified tolerance.")
        .setDocumentation(
            "If enabled, reduce the memory footprint of output grids that have"
            " (sufficiently large) regions of voxels with the same value,"
            " where values are considered equal if they differ by less than"
            " the specified threshold.\n\n"
            "NOTE:\n"
            "    Pruning affects only the memory usage of a grid.\n"
            "    It does not remove voxels, apart from inactive voxels\n"
            "    whose value is equal to the background."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "flood", "Signed-Flood Fill Output")
        .setDefault(PRMoneDefaults)
        .setTooltip("Reclassify inactive output voxels as either inside or outside.")
        .setDocumentation(
            "Test inactive voxels to determine if they are inside or outside of an SDF"
            " and hence whether they should have negative or positive sign.\n\n"
            "NOTE:\n"
            "    This option is ignored when converting native fog volumes to VDBs.\n"));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "activateinsidesdf", "Activate Interior Voxels")
        .setDefault(PRMoneDefaults)
        .setTooltip("Activate all voxels inside a converted level set.")
        .setDocumentation(
            "Activate all voxels inside an SDF, even if they match the background value.\n\n"
            "This option is useful if processing the resulting VDB with VEX,\n"
            "which operates only on active voxels of a VDB.\n"
            "However, disabling this option will retain only the narrow active internal\n"
            "band of an incoming SDF if it has one, saving memory and downstream processing.\n\n"
            "This toggle has no effect for non-SDF volumes, or if\n"
            "__Signed-Flood Fill Output__ is disabled."));

    //////////

    // Obsolete parameters
    hutil::ParmList obsoleteParms;
    obsoleteParms.add(hutil::ParmFactory(PRM_SEPARATOR,"sep1", ""));
    obsoleteParms.add(hutil::ParmFactory(PRM_TOGGLE, "smoothseams", "Smooth Seams"));
    obsoleteParms.add(hutil::ParmFactory(PRM_INT_J, "automaticpartitions", ""));
    obsoleteParms.add(hutil::ParmFactory(PRM_INT_J, "activepart", ""));

    // Register this operator.
    hvdb::OpenVDBOpFactory("Convert VDB",
        SOP_OpenVDB_Convert::factory, parms, *table)
#ifndef SESI_OPENVDB
        .setInternalName("DW_OpenVDBConvert")
#endif
        .setObsoleteParms(obsoleteParms)
        .addInput("VDBs to convert")
        .addOptionalInput("Optional reference surface. Can be used "
            "to transfer attributes, sharpen features and to "
            "eliminate seams from fractured pieces.")
        .addOptionalInput("Optional VDB masks")
        .setVerb(SOP_NodeVerb::COOK_DUPLICATE, []() { return new SOP_OpenVDB_Convert::Cache; })
        .setDocumentation("\
#icon: COMMON/openvdb\n\
#tags: vdb\n\
\n\
\"\"\"Convert VDB volumes into other primitive types.\"\"\"\n\
\n\
@overview\n\
\n\
This node converts sparse volumes, or VDBs, into other primitive types,\n\
including Houdini volumes.\n\
It offers some options not available through the [Convert|Node:sop/convert] node.\n\
\n\
When converting to polygons, the second and third inputs can be optionally\n\
supplied.\n\
The second input provides a reference polygon surface that is useful\n\
for preserving features of [fractured|Node:sop/DW_OpenVDBFracture] VDBs.\n\
The third provides additional VDB fields that can be used for\n\
masking (which voxels to convert to polygons) and/or for specifying\n\
an adaptivity multiplier.\n\
\n\
@related\n\
- [OpenVDB To Polygons|Node:sop/DW_OpenVDBToPolygons]\n\
- [OpenVDB To Spheres|Node:sop/DW_OpenVDBToSpheres]\n\
- [Node:sop/convert]\n\
- [Node:sop/convertvolume]\n\
\n\
@examples\n\
\n\
See [openvdb.org|http://www.openvdb.org/download/] for source code\n\
and usage examples.\n");
}


////////////////////////////////////////


OP_Node*
SOP_OpenVDB_Convert::factory(OP_Network* net,
    const char* name, OP_Operator* op)
{
    return new SOP_OpenVDB_Convert(net, name, op);
}


SOP_OpenVDB_Convert::SOP_OpenVDB_Convert(OP_Network* net,
    const char* name, OP_Operator* op):
    hvdb::SOP_NodeVDB(net, name, op)
{
}


////////////////////////////////////////


namespace {

/// @brief Convert a collection of OpenVDB grids into Houdini volumes.
/// @return @c true if all grids were successfully converted, @c false if one
/// or more grids were skipped due to unrecognized grid types.
void
convertToVolumes(GU_Detail& dst, GA_PrimitiveGroup* group, bool split_disjoint = false)
{
    GU_ConvertParms parms;
    parms.setToType(GEO_PrimTypeCompat::GEOPRIMVOLUME);
    parms.primGroup = group;
    parms.preserveGroups = true;
    GU_PrimVDB::convertVDBs(dst, dst, parms,
        /*adaptivity=*/0, /*keep_original*/false , split_disjoint);
}


////////////////////////////////////////


void
convertToOpenVDB(
    GU_Detail& dst,
    GA_PrimitiveGroup* group,
    bool flood,
    bool prune,
    fpreal tolerance,
    bool activateinsidesdf)
{
    GU_ConvertParms parms;
    parms.primGroup = group;
    parms.preserveGroups = true;
    GU_PrimVDB::convertVolumesToVDBs(dst, dst, parms, flood, prune, tolerance,
        /*keep_original*/false, activateinsidesdf);
}


////////////////////////////////////////


void
convertVDBClass(
    GU_Detail& dst,
    GA_PrimitiveGroup* group,
    openvdb::GridClass newClass,
    float isovalue)
{
    using namespace openvdb;

    for (hvdb::VdbPrimIterator it(&dst, group); it; ++it) {
        const auto typ = it->getStorageType();
        if ((typ != UT_VDB_FLOAT) && (typ != UT_VDB_DOUBLE)) continue;

        auto& grid = it->getGrid();
        if (grid.getGridClass() == newClass) continue;

        if (newClass == GRID_FOG_VOLUME) { // convert a level set to a fog volume
            it->makeGridUnique();
            if (typ == UT_VDB_FLOAT) {
                FloatGrid& fogGrid = UTvdbGridCast<FloatGrid>(grid);
                tools::sdfToFogVolume(fogGrid, std::numeric_limits<float>::max());
            } else if (typ == UT_VDB_DOUBLE) {
                DoubleGrid& fogGrid = UTvdbGridCast<DoubleGrid>(grid);
                tools::sdfToFogVolume(fogGrid, std::numeric_limits<double>::max());
            }
            it->setVisualization(GEO_VOLUMEVIS_SMOKE, it->getVisIso(), it->getVisDensity());

        } else if (newClass == GRID_LEVEL_SET) { // convert a fog volume to a level set
            // *** FIXME:TODO: Hack until we have a good method ***
            // Convert to polygons
            tools::VolumeToMesh mesher(isovalue);
            if (typ == UT_VDB_FLOAT) {
                mesher(UTvdbGridCast<FloatGrid>(grid));
            } else if (typ == UT_VDB_DOUBLE) {
                mesher(UTvdbGridCast<DoubleGrid>(grid));
            }

            // Convert to SDF
            math::Transform::Ptr transform = grid.transformPtr();
            std::vector<Vec3s> points;
            points.reserve(mesher.pointListSize());
            for (size_t i = 0, n = mesher.pointListSize(); i < n; i++) {
                // The MeshToVolume conversion, further down, requires the
                // points to be in grid index space.
                points.push_back(transform->worldToIndex(mesher.pointList()[i]));
            }

            openvdb::tools::PolygonPoolList& polygonPoolList = mesher.polygonPoolList();

            std::vector<Vec4I> primitives;
            size_t numPrimitives = 0;
            for (size_t n = 0, N = mesher.polygonPoolListSize(); n < N; ++n) {
                const openvdb::tools::PolygonPool& polygons = polygonPoolList[n];
                numPrimitives += polygons.numQuads();
                numPrimitives += polygons.numTriangles();
            }
            primitives.reserve(numPrimitives);

            for (size_t n = 0, N = mesher.polygonPoolListSize(); n < N; ++n) {

                const openvdb::tools::PolygonPool& polygons = polygonPoolList[n];

                // Copy quads
                for (size_t i = 0, I = polygons.numQuads(); i < I; ++i) {
                    primitives.push_back(polygons.quad(i));
                }

                // Copy triangles (adaptive mesh)
                if (polygons.numTriangles() != 0) {
                    openvdb::Vec4I quad;
                    quad[3] = openvdb::util::INVALID_IDX;
                    for (size_t i = 0, I = polygons.numTriangles(); i < I; ++i) {
                        const openvdb::Vec3I& triangle = polygons.triangle(i);
                        quad[0] = triangle[0];
                        quad[1] = triangle[1];
                        quad[2] = triangle[2];
                        primitives.push_back(quad);
                    }
                }
            }

            openvdb::tools::QuadAndTriangleDataAdapter<openvdb::Vec3s, openvdb::Vec4I>
                mesh(points, primitives);

            // Set grid and visualization
            if (it->getStorageType() == UT_VDB_FLOAT) {
                if (auto sdfGridPtr = tools::meshToVolume<FloatGrid>(mesh, *transform)) {
                    it->setGrid(*sdfGridPtr);
                }
            } else if (it->getStorageType() == UT_VDB_DOUBLE) {
                if (auto sdfGridPtr = tools::meshToVolume<DoubleGrid>(mesh, *transform)) {
                    it->setGrid(*sdfGridPtr);
                }
            }
            it->setVisualization(GEO_VOLUMEVIS_ISO, it->getVisIso(), it->getVisDensity());
        }
    }
}


////////////////////////////////////////


void
copyMesh(
    GU_Detail& detail,
    const GU_PrimVDB* srcvdb,
    GA_PrimitiveGroup* delgroup,
    openvdb::tools::VolumeToMesh& mesher,
    bool toPolySoup,
    GA_PrimitiveGroup* surfaceGroup = nullptr,
    GA_PrimitiveGroup* interiorGroup = nullptr,
    GA_PrimitiveGroup* seamGroup = nullptr,
    GA_PointGroup* seamPointGroup = nullptr)
{
    const openvdb::tools::PointList& points = mesher.pointList();
    openvdb::tools::PolygonPoolList& polygonPoolList = mesher.polygonPoolList();

    const char exteriorFlag = char(openvdb::tools::POLYFLAG_EXTERIOR);
    const char seamLineFlag = char(openvdb::tools::POLYFLAG_FRACTURE_SEAM);

    // Disable adding to seamPointGroup if we don't have pointFlags()
    if (mesher.pointFlags().size() != mesher.pointListSize()) {
        seamPointGroup = nullptr;
    }

    GA_Size npoints = mesher.pointListSize();
    const GA_Offset startpt = detail.appendPointBlock(npoints);
    UT_ASSERT_COMPILETIME(sizeof(openvdb::tools::PointList::element_type) == sizeof(UT_Vector3));
    GA_RWHandleV3 pthandle(detail.getP());
    pthandle.setBlock(startpt, npoints, reinterpret_cast<UT_Vector3*>(points.get()));

    // group fracture seam points
    if (seamPointGroup && GA_Size(mesher.pointFlags().size()) == npoints) {
        GA_Offset ptoff = startpt;
        for (GA_Size i = 0; i < npoints; ++i) {

            if (mesher.pointFlags()[i]) {
                seamPointGroup->addOffset(ptoff);
            }
            ++ptoff;
        }
    }

    // index 0 --> interior, not on seam
    // index 1 --> interior, on seam
    // index 2 --> surface,  not on seam
    // index 3 --> surface,  on seam
    GA_Size nquads[4] = {0, 0, 0, 0};
    GA_Size ntris[4]  = {0, 0, 0, 0};
    for (size_t n = 0, N = mesher.polygonPoolListSize(); n < N; ++n) {
        const openvdb::tools::PolygonPool& polygons = polygonPoolList[n];
        for (size_t i = 0, I = polygons.numQuads(); i < I; ++i) {
            int flags = (((polygons.quadFlags(i) & exteriorFlag)!=0) << 1)
                       | ((polygons.quadFlags(i) & seamLineFlag)!=0);
            ++nquads[flags];
        }
        for (size_t i = 0, I = polygons.numTriangles(); i < I; ++i) {
            int flags = (((polygons.triangleFlags(i) & exteriorFlag)!=0) << 1)
                       | ((polygons.triangleFlags(i) & seamLineFlag)!=0);
            ++ntris[flags];
        }
    }

    GA_Size nverts[4] = {
        nquads[0]*4 + ntris[0]*3,
        nquads[1]*4 + ntris[1]*3,
        nquads[2]*4 + ntris[2]*3,
        nquads[3]*4 + ntris[3]*3
    };
    UT_IntArray verts[4];
    for (int flags = 0; flags < 4; ++flags) {
        verts[flags].setCapacity(nverts[flags]);
        verts[flags].entries(nverts[flags]);
    }

    GA_Size iquad[4] = {0, 0, 0, 0};
    GA_Size itri[4]  = {nquads[0]*4, nquads[1]*4, nquads[2]*4, nquads[3]*4};

    for (size_t n = 0, N = mesher.polygonPoolListSize(); n < N; ++n) {
        const openvdb::tools::PolygonPool& polygons = polygonPoolList[n];

        // Copy quads
        for (size_t i = 0, I = polygons.numQuads(); i < I; ++i) {
            const openvdb::Vec4I& quad = polygons.quad(i);
            int flags = (((polygons.quadFlags(i) & exteriorFlag)!=0) << 1)
                       | ((polygons.quadFlags(i) & seamLineFlag)!=0);
            verts[flags](iquad[flags]++) = quad[0];
            verts[flags](iquad[flags]++) = quad[1];
            verts[flags](iquad[flags]++) = quad[2];
            verts[flags](iquad[flags]++) = quad[3];
        }

        // Copy triangles (adaptive mesh)
        for (size_t i = 0, I = polygons.numTriangles(); i < I; ++i) {
            const openvdb::Vec3I& triangle = polygons.triangle(i);
            int flags = (((polygons.triangleFlags(i) & exteriorFlag)!=0) << 1)
                       | ((polygons.triangleFlags(i) & seamLineFlag)!=0);
            verts[flags](itri[flags]++) = triangle[0];
            verts[flags](itri[flags]++) = triangle[1];
            verts[flags](itri[flags]++) = triangle[2];
        }
    }

    bool shared_vertices = true;
    if (toPolySoup) {
        // NOTE: Since we could be using the same points for multiple
        //       polysoups, and the shared vertices option assumes that
        //       the points are only used by this polysoup, we have to
        //       use the unique vertices option.
        int num_prims = 0;
        for (int flags = 0; flags < 4; ++flags) {
            if (!nquads[flags] && !ntris[flags]) continue;
            num_prims++;
        }
        shared_vertices = (num_prims <= 1);
    }

    for (int flags = 0; flags < 4; ++flags) {
        if (!nquads[flags] && !ntris[flags]) continue;

        GEO_PolyCounts sizelist;
        if (nquads[flags]) sizelist.append(4, nquads[flags]);
        if (ntris[flags])  sizelist.append(3, ntris[flags]);

        GA_Detail::OffsetMarker marker(detail);

        if (toPolySoup) {
            GU_PrimPolySoup::build(
                &detail, startpt, npoints, sizelist, verts[flags].array(), shared_vertices);
        } else {
            GU_PrimPoly::buildBlock(&detail, startpt, npoints, sizelist, verts[flags].array());
        }

        GA_Range range(marker.primitiveRange());
        GA_Range pntRange(marker.pointRange());
        GU_ConvertParms parms;
        parms.preserveGroups = true;
        GUconvertCopySingleVertexPrimAttribsAndGroups(parms,
            *srcvdb->getParent(), srcvdb->getMapOffset(), detail,
            range, pntRange);

        if (delgroup)                       delgroup->removeRange(range);
        if (seamGroup && (flags & 1))       seamGroup->addRange(range);
        if (surfaceGroup && (flags & 2))    surfaceGroup->addRange(range);
        if (interiorGroup && !(flags & 2))  interiorGroup->addRange(range);
    }
}


////////////////////////////////////////


int
getVDBPrecision(UT_VDBType typ)
{
    switch (typ) {
        case UT_VDB_BOOL:    return 1;
        case UT_VDB_FLOAT:
        case UT_VDB_INT32:
        case UT_VDB_VEC3F:
        case UT_VDB_VEC3I:   return 32;
        case UT_VDB_DOUBLE:
        case UT_VDB_INT64:
        case UT_VDB_VEC3D:   return 64;
        default: break;
    }
    return 0;
}


const char*
getVDBTypeName(UT_VDBType typ)
{
    switch (typ) {
        case UT_VDB_BOOL:    return "bool";
        case UT_VDB_FLOAT:
        case UT_VDB_DOUBLE:  return "float";
        case UT_VDB_INT32:
        case UT_VDB_INT64:   return "int";
        case UT_VDB_VEC3F:
        case UT_VDB_VEC3D:   return "vec3f";
        case UT_VDB_VEC3I:   return "vec3i";
        default: break;
    }
    return "none";
}


UT_VDBType
getVDBTypeFromNameAndPrecision(const UT_String& name, int bits)
{
    if (name == "float") {
        return ((bits == 64) ? UT_VDB_DOUBLE : UT_VDB_FLOAT);
    } else if (name == "vec3f") {
        return ((bits == 64) ? UT_VDB_VEC3D : UT_VDB_VEC3F);
    } else if (name == "bool") {
        return UT_VDB_BOOL;
    } else if (name == "int") {
        return ((bits == 64) ? UT_VDB_INT64 : UT_VDB_INT32);
    } else if (name == "vec3i") {
        return UT_VDB_VEC3I;
    }
    return UT_VDB_INVALID;
}


////////////////////////////////////////


// Functor for use with GEOvdbProcessTypedGrid*() to create a copy of a grid,
// but with a new value type
struct GridCopyOp
{
    UT_VDBType outType = UT_VDB_INVALID;
    hvdb::GridPtr outGrid;

    template<typename OutGridT, typename InGridT>
    typename OutGridT::Ptr copyGrid(const InGridT& inGrid)
    {
        using OutValueT = typename OutGridT::ValueType;
        using OutGridPtrT = typename OutGridT::Ptr;
        using OutTreeT = typename OutGridT::TreeType;
        using OutTreePtrT = typename OutTreeT::Ptr;

        OutTreePtrT newTree;

        try {
            // Deep copy the input grid's tree, casting its values to the output grid's ValueType.
            newTree.reset(new OutTreeT{inGrid.constTree()});
        } catch (openvdb::TypeError&) {
            try {
                // If the value copy fails (due to incompatible value types),
                // try a topology copy instead.
                newTree.reset(new OutTreeT{inGrid.constTree(),
                    openvdb::zeroVal<OutValueT>(), openvdb::TopologyCopy{}});
            } catch (openvdb::TypeError&) {
                // If the topology copy fails, give up.
                return OutGridPtrT{};
            }
        }
        auto newGrid = OutGridT::create(newTree);
        newGrid->insertMeta(*inGrid.copyMeta());
        newGrid->setTransform(inGrid.transform().copy());
        if ((outType != UT_VDB_FLOAT) && (outType != UT_VDB_DOUBLE)
            && (newGrid->getGridClass() == openvdb::GRID_LEVEL_SET))
        {
            // If the output grid is not floating-point scalar, then it can't be a level set.
            newGrid->setGridClass(openvdb::GRID_UNKNOWN);
        }
        if ((UTvdbGetGridTupleSize(outType) != 1)
            && (newGrid->getGridClass() == openvdb::GRID_FOG_VOLUME))
        {
            // If the output grid is not scalar, then it can't be a fog volume.
            newGrid->setGridClass(openvdb::GRID_UNKNOWN);
        }
        return newGrid;
    }

    template<typename GridT>
    void operator()(const GridT& inGrid)
    {
        outGrid.reset();
        if (UTvdbGetGridType(inGrid) == outType) return;

        switch (outType) {
            case UT_VDB_BOOL:    outGrid = copyGrid<openvdb::BoolGrid>(inGrid); break;
            case UT_VDB_FLOAT:   outGrid = copyGrid<openvdb::FloatGrid>(inGrid); break;
            case UT_VDB_INT32:   outGrid = copyGrid<openvdb::Int32Grid>(inGrid); break;
            case UT_VDB_VEC3F:   outGrid = copyGrid<openvdb::Vec3fGrid>(inGrid); break;
            case UT_VDB_VEC3I:   outGrid = copyGrid<openvdb::Vec3IGrid>(inGrid); break;
            case UT_VDB_DOUBLE:  outGrid = copyGrid<openvdb::DoubleGrid>(inGrid); break;
            case UT_VDB_INT64:   outGrid = copyGrid<openvdb::Int64Grid>(inGrid); break;
            case UT_VDB_VEC3D:   outGrid = copyGrid<openvdb::Vec3dGrid>(inGrid); break;
            default: break;
        }
    }
}; // struct GridCopyOp


////////////////////////////////////////


struct InteriorMaskOp
{
    InteriorMaskOp(double iso = 0.0): inIsovalue(iso) {}

    template<typename GridType>
    void operator()(const GridType& grid)
    {
        outGridPtr = openvdb::tools::interiorMask(grid, inIsovalue);
    }

    const double inIsovalue;
    openvdb::BoolGrid::Ptr outGridPtr;
};


// Extract a boolean mask from a grid of any type.
inline hvdb::GridCPtr
getMaskFromGrid(const hvdb::GridCPtr& gridPtr, double isovalue = 0.0)
{
    hvdb::GridCPtr maskGridPtr;
    if (gridPtr) {
        if (gridPtr->isType<openvdb::BoolGrid>()) {
            // If the input grid is already boolean, return it.
            maskGridPtr = gridPtr;
        } else {
            InteriorMaskOp op{isovalue};
            UTvdbProcessTypedGridTopology(UTvdbGetGridType(*gridPtr), *gridPtr, op);
            maskGridPtr = op.outGridPtr;
        }
    }
    return maskGridPtr;
}

} // unnamed namespace


////////////////////////////////////////


// Enable or disable parameters in the UI.
bool
SOP_OpenVDB_Convert::updateParmsFlags()
{
    bool changed = false;
    const fpreal time = CHgetEvalTime();

    ConvertTo target = static_cast<ConvertTo>(evalInt("conversion", 0, time));
    const bool toVolume = (target == HVOLUME);
    const bool toOpenVDB = (target == OPENVDB);
    const bool toPolySoup = (target == POLYSOUP);
    const bool toPoly = toPolySoup || (target == POLYGONS);
    const bool toSDF = (evalInt("vdbclass", 0, time) == CLASS_SDF);
    const bool toFog = (evalInt("vdbclass", 0, time) == CLASS_FOG_VOLUME);

    UT_String vdbTypeStr;
    evalString(vdbTypeStr, "vdbtype", 0, time);
    const bool toFixedPrecision = ((vdbTypeStr == "bool")
        || (vdbTypeStr == "vec3i")); // bool and vec3i grids have fixed precision

    //
    // Enable/disable
    //
    changed |= enableParm("adaptivity", toPoly);
    changed |= enableParm("isoValue", toPoly || (toOpenVDB && toSDF));
    changed |= enableParm("fogisovalue", toOpenVDB && toSDF);

    if (toOpenVDB) {
        changed |= enableParm("tolerance", bool(evalInt("prune",  0, time)));
    }

    bool refexists = (nInputs() == 2);
    changed |= enableParm("computenormals", toPoly && !toPolySoup);
    changed |= enableParm("internaladaptivity", toPoly && refexists);
    changed |= enableParm("surfacegroup", toPoly && refexists);
    changed |= enableParm("interiorgroup", toPoly && refexists);
    changed |= enableParm("seamlinegroup", toPoly && refexists);
    changed |= enableParm("seampoints", toPoly && refexists);
    changed |= enableParm("transferattributes", toPoly && refexists);
    changed |= enableParm("sharpenfeatures", toPoly && refexists);
    changed |= enableParm("edgetolerance", toPoly && refexists);

    const bool maskexists = (nInputs() == 3);

    changed |= enableParm("surfacemask", toPoly && maskexists);
    changed |= enableParm("adaptivityfield", toPoly && maskexists);

    const bool surfacemask = bool(evalInt("surfacemask", 0, 0));
    changed |= enableParm("surfacemaskname", toPoly && maskexists && surfacemask);
    changed |= enableParm("surfacemaskoffset", toPoly && maskexists && surfacemask);
    changed |= enableParm("invertmask", toPoly && maskexists && surfacemask);

    changed |= enableParm("adaptivityfield", toPoly && maskexists);

    const bool adaptivityfield = bool(evalInt("adaptivityfield", 0, 0));
    changed |= enableParm("adaptivityfieldname", toPoly && maskexists && adaptivityfield);

    //
    // Show/hide
    //
    changed |= setVisibleState("splitdisjointvolumes", toVolume);

    changed |= setVisibleState("adaptivity", toPoly);
    changed |= setVisibleState("isoValue", toPoly);
    changed |= setVisibleState("fogisovalue", toOpenVDB);
    changed |= setVisibleState("computenormals", toPoly);

    changed |= setVisibleState("internaladaptivity", toPoly);
    changed |= setVisibleState("transferattributes", toPoly);
    changed |= setVisibleState("sharpenfeatures", toPoly);
    changed |= setVisibleState("edgetolerance", toPoly);
    changed |= setVisibleState("surfacegroup", toPoly);
    changed |= setVisibleState("interiorgroup", toPoly);
    changed |= setVisibleState("seamlinegroup", toPoly);
    changed |= setVisibleState("seampoints", toPoly);

    changed |= setVisibleState("surfacemask", toPoly);
    changed |= setVisibleState("surfacemaskname", toPoly);
    changed |= setVisibleState("surfacemaskoffset", toPoly);
    changed |= setVisibleState("invertmask", toPoly);
    changed |= setVisibleState("adaptivityfield", toPoly);
    changed |= setVisibleState("adaptivityfieldname", toPoly);

    changed |= setVisibleState("flood", toOpenVDB);
    changed |= setVisibleState("prune", toOpenVDB);
    changed |= setVisibleState("tolerance", toOpenVDB);
    changed |= setVisibleState("vdbclass", toOpenVDB);
    changed |= setVisibleState("vdbtype", toOpenVDB && !(toSDF || toFog));
    changed |= setVisibleState("vdbprecision", toOpenVDB && !toFixedPrecision);

    changed |= setVisibleState("activateinsidesdf", toOpenVDB);
    if (toOpenVDB) {
        changed |= enableParm("activateinsidesdf", bool(evalInt("flood",  0, time)));
    }

    return changed;
}


////////////////////////////////////////


// Convert all VDB primitives in the given group to have a new storage type (where possible).
void
SOP_OpenVDB_Convert::Cache::convertVDBType(
    GU_Detail& dst,
    GA_PrimitiveGroup* group,
    const UT_String& outTypeStr,
    const UT_String& outPrecStr,
    hvdb::Interrupter& boss)
{
    for (hvdb::VdbPrimIterator it(&dst, group); it; ++it) {
        if (boss.wasInterrupted()) return;

        const UT_VDBType inType = it->getStorageType();
        const UT_String inTypeName = getVDBTypeName(inType);
        const int inBits = getVDBPrecision(inType);

        const UT_VDBType outType = getVDBTypeFromNameAndPrecision(
            ((outTypeStr == "none") ? inTypeName : outTypeStr),
            ((outPrecStr == "none") ? inBits : ((outPrecStr == "32") ? 32 : 64)));

        if (outType != inType) {
            GridCopyOp op;
            op.outType = outType;
            // Create a copy of the grid, but with a different value type.
            // Store the copy as op.outGrid.
            GEOvdbProcessTypedGridTopology(*it.getPrimitive(), op);
            if (op.outGrid) {
                auto& grid = *op.outGrid;
                grid.removeMeta("value_type");
                grid.insertMeta("value_type", openvdb::StringMetadata(grid.valueType()));
                it->setGrid(grid);
                it->syncAttrsFromMetadata();
            }
        }
    }
}


template <class GridType>
void
SOP_OpenVDB_Convert::Cache::referenceMeshing(
    std::list<openvdb::GridBase::ConstPtr>& grids,
    std::list<const GU_PrimVDB*> vdbs,
    GA_PrimitiveGroup* delgroup,
    openvdb::tools::VolumeToMesh& mesher,
    const GU_Detail* refGeo,
    bool computeNormals,
    hvdb::Interrupter& boss,
    const fpreal time)
{
    if (refGeo == nullptr) return;

    using TreeType = typename GridType::TreeType;
    using ValueType = typename GridType::ValueType;

    const bool transferAttributes = evalInt("transferattributes", 0, time);
    const bool sharpenFeatures = evalInt("sharpenfeatures", 0, time);

    // Get the first grid's transform and background value.
    openvdb::math::Transform::Ptr transform = grids.front()->transform().copy();

    typename GridType::ConstPtr firstGrid = openvdb::gridConstPtrCast<GridType>(grids.front());

    if (!firstGrid) {
        addError(SOP_MESSAGE, "Unsupported grid type.");
        return;
    }

    const ValueType backgroundValue = firstGrid->background();
    const openvdb::GridClass gridClass = firstGrid->getGridClass();

    typename GridType::ConstPtr refGrid;

    using IntGridT = typename GridType::template ValueConverter<openvdb::Int32>::Type;
    typename IntGridT::Ptr indexGrid;

    openvdb::tools::MeshToVoxelEdgeData edgeData;

    UT_UniquePtr<GU_Detail> geoPtr;
    if (!refGrid) {
        std::string warningStr;
        geoPtr = hvdb::convertGeometry(*refGeo, warningStr, &boss);

        if (geoPtr) {
            refGeo = geoPtr.get();
            if (!warningStr.empty()) addWarning(SOP_MESSAGE, warningStr.c_str());
        }

        std::vector<openvdb::Vec3s> pointList;
        std::vector<openvdb::Vec4I> primList;

        pointList.resize(refGeo->getNumPoints());
        primList.resize(refGeo->getNumPrimitives());

        UTparallelFor(GA_SplittableRange(refGeo->getPointRange()),
            hvdb::TransformOp(refGeo, *transform, pointList));

        UTparallelFor(GA_SplittableRange(refGeo->getPrimitiveRange()),
            hvdb::PrimCpyOp(refGeo, primList));

        if (boss.wasInterrupted()) return;

        openvdb::tools::QuadAndTriangleDataAdapter<openvdb::Vec3s, openvdb::Vec4I>
            mesh(pointList, primList);

        float bandWidth = 3.0;

        if (gridClass != openvdb::GRID_LEVEL_SET) {
            bandWidth = float(backgroundValue) / float(transform->voxelSize()[0]);
        }

        indexGrid.reset(new IntGridT(0));

        refGrid = openvdb::tools::meshToVolume<GridType>(boss,
            mesh, *transform, bandWidth, bandWidth, 0, indexGrid.get());

        if (sharpenFeatures) edgeData.convert(pointList, primList);
    }

    if (boss.wasInterrupted()) return;

    using BoolTreeType = typename TreeType::template ValueConverter<bool>::Type;
    typename BoolTreeType::Ptr maskTree;

    if (sharpenFeatures) {

        const float edgetolerance = static_cast<float>(evalFloat("edgetolerance", 0, time));

        maskTree = typename BoolTreeType::Ptr(new BoolTreeType(false));
        maskTree->topologyUnion(indexGrid->tree());
        openvdb::tree::LeafManager<BoolTreeType> maskLeafs(*maskTree);

        hvdb::GenAdaptivityMaskOp<typename IntGridT::TreeType, BoolTreeType>
            op(*refGeo, indexGrid->tree(), maskLeafs, edgetolerance);
        op.run();

        openvdb::tools::pruneInactive(*maskTree);

        openvdb::tools::dilateVoxels(*maskTree, 2);

        mesher.setAdaptivityMask(maskTree);
    }


    if (boss.wasInterrupted()) return;

    const double iadaptivity = double(evalFloat("internaladaptivity", 0, time));
    mesher.setRefGrid(refGrid, iadaptivity);


    std::vector<std::string> badTransformList, badBackgroundList, badTypeList;

    GA_PrimitiveGroup *surfaceGroup = nullptr, *interiorGroup = nullptr, *seamGroup = nullptr;
    GA_PointGroup* seamPointGroup = nullptr;

    {
        UT_String newGroupStr;
        evalString(newGroupStr, "surfacegroup", 0, time);
        if (newGroupStr.length() > 0) {
            surfaceGroup = gdp->findPrimitiveGroup(newGroupStr);
            if (!surfaceGroup) surfaceGroup = gdp->newPrimitiveGroup(newGroupStr);
        }

        evalString(newGroupStr, "interiorgroup", 0, time);
        if (newGroupStr.length() > 0) {
            interiorGroup = gdp->findPrimitiveGroup(newGroupStr);
            if (!interiorGroup) interiorGroup = gdp->newPrimitiveGroup(newGroupStr);
        }

        evalString(newGroupStr, "seamlinegroup", 0, time);
        if (newGroupStr.length() > 0) {
            seamGroup = gdp->findPrimitiveGroup(newGroupStr);
            if (!seamGroup) seamGroup = gdp->newPrimitiveGroup(newGroupStr);
        }

        evalString(newGroupStr, "seampoints", 0, time);
        if (newGroupStr.length() > 0) {
            seamPointGroup = gdp->findPointGroup(newGroupStr);
            if (!seamPointGroup) seamPointGroup = gdp->newPointGroup(newGroupStr);
        }
    }

    std::vector<typename GridType::ConstPtr> fragments;
    std::vector<const GU_PrimVDB*> fragment_vdbs;
    std::list<openvdb::GridBase::ConstPtr>::iterator it = grids.begin();
    std::list<const GU_PrimVDB*>::iterator vdbit = vdbs.begin();

    for (; it != grids.end(); ++it, ++vdbit) {

        if (boss.wasInterrupted()) break;

        typename GridType::ConstPtr grid = openvdb::gridConstPtrCast<GridType>(*it);

        if (!grid) {
            badTypeList.push_back(grid->getName());
            continue;
        }

        if (grid->transform() != *transform) {
            badTransformList.push_back(grid->getName());
            continue;
        }

        if (!openvdb::math::isApproxEqual(grid->background(), backgroundValue)) {
            badBackgroundList.push_back(grid->getName());
            continue;
        }

        fragments.push_back(grid);
        fragment_vdbs.push_back(*vdbit);
    }

    grids.clear();

    for (size_t i = 0, I = fragments.size(); i < I; ++i) {
        mesher(*fragments[i]);
        ConvertTo target = static_cast<ConvertTo>(evalInt("conversion", 0, time));
        bool toPolySoup = (target == POLYSOUP);
        copyMesh(*gdp, fragment_vdbs[i], delgroup, mesher, toPolySoup,
            surfaceGroup, interiorGroup, seamGroup, seamPointGroup);
    }

    // Sharpen Features
    if (!boss.wasInterrupted() && sharpenFeatures) {
        UTparallelFor(GA_SplittableRange(gdp->getPointRange()),
            hvdb::SharpenFeaturesOp(*gdp, *refGeo, edgeData, *transform,
                surfaceGroup, maskTree.get()));
    }

    // Compute vertex normals
    if (!boss.wasInterrupted() && computeNormals) {

        UTparallelFor(GA_SplittableRange(gdp->getPrimitiveRange()),
            hvdb::VertexNormalOp(*gdp, interiorGroup, (transferAttributes ? -1.0f : 0.7f) ));

        if (!interiorGroup) {
            addWarning(SOP_MESSAGE, "More accurate vertex normals can be generated "
                "if the interior polygon group is enabled.");
        }
    }

    // Transfer Primitive Attributes
    if (!boss.wasInterrupted() && transferAttributes && refGeo && indexGrid) {
        hvdb::transferPrimitiveAttributes(*refGeo, *gdp, *indexGrid, boss, surfaceGroup);
    }


    if (!badTransformList.empty()) {
        std::string s = "The following grids were skipped: '" +
            hboost::algorithm::join(badTransformList, ", ") +
            "' because they don't match the transform of the first grid.";
        addWarning(SOP_MESSAGE, s.c_str());
    }

    if (!badBackgroundList.empty()) {
        std::string s = "The following grids were skipped: '" +
            hboost::algorithm::join(badBackgroundList, ", ") +
            "' because they don't match the background value of the first grid.";
        addWarning(SOP_MESSAGE, s.c_str());
    }

    if (!badTypeList.empty()) {
        std::string s = "The following grids were skipped: '" +
            hboost::algorithm::join(badTypeList, ", ") +
            "' because they don't have the same data type as the first grid.";
        addWarning(SOP_MESSAGE, s.c_str());
    }
}


void
SOP_OpenVDB_Convert::Cache::convertToPoly(
    fpreal time,
    GA_PrimitiveGroup *group,
    bool buildpolysoup,
    hvdb::Interrupter &boss)
{
    hvdb::VdbPrimCIterator vdbIt(gdp, group);
    if (!vdbIt) {
        addWarning(SOP_MESSAGE, "No VDB primitives found.");
        return;
    }

    const bool      computeNormals = !buildpolysoup && (evalInt("computenormals", 0, time) != 0);
    const bool      invertmask = evalInt("invertmask", 0, time);
    const fpreal    adaptivity = evalFloat("adaptivity", 0, time);
    const fpreal    iso = evalFloat("isoValue", 0, time);
    const fpreal    maskoffset = evalFloat("surfacemaskoffset", 0, time);

    openvdb::tools::VolumeToMesh mesher(iso, adaptivity);

    // Check mask input
    const GU_Detail* maskGeo = inputGeo(2);
    if (maskGeo) {

        if (evalInt("surfacemask", 0, time)) {
            const auto maskStr = evalStdString("surfacemaskname", time);
            const GA_PrimitiveGroup* maskGroup =
                parsePrimitiveGroups(maskStr.c_str(), GroupCreator(maskGeo));
            if (!maskGroup && !maskStr.empty()) {
                addWarning(SOP_MESSAGE, "Surface mask not found.");
            } else {
                hvdb::VdbPrimCIterator maskIt(maskGeo, maskGroup);
                if (maskIt) {
                    if (auto maskGridPtr = getMaskFromGrid(maskIt->getGridPtr(), maskoffset)) {
                        mesher.setSurfaceMask(maskGridPtr, invertmask);
                    } else {
                        std::string mesg = "Surface mask "
                            + maskIt.getPrimitiveNameOrIndex().toStdString()
                            + " of type " + maskIt->getGrid().type() + " is not supported.";
                        addWarning(SOP_MESSAGE, mesg.c_str());
                    }
                }
            }
        }

        if (evalInt("adaptivityfield", 0, time)) {
            const auto maskStr = evalStdString("adaptivityfieldname", time);
            const GA_PrimitiveGroup* maskGroup = matchGroup(*maskGeo, maskStr);
            if (!maskGroup && !maskStr.empty()) {
                addWarning(SOP_MESSAGE, "Adaptivity field not found.");
            } else {
                hvdb::VdbPrimCIterator maskIt(maskGeo, maskGroup);
                if (maskIt) {
                    openvdb::FloatGrid::ConstPtr grid =
                        openvdb::gridConstPtrCast<openvdb::FloatGrid>(maskIt->getGridPtr());

                    mesher.setSpatialAdaptivity(grid);
                }
            }
        }
    }


    const GU_Detail* refGeo = inputGeo(1);
    GU_ConvertParms parms;
    GA_PrimitiveGroup *delGroup = parms.getDeletePrimitives(gdp);

    if (refGeo) {
        // Collect all level set grids.
        std::list<openvdb::GridBase::ConstPtr> grids;
        std::list<const GU_PrimVDB*> vdbs;
        std::vector<std::string> nonLevelSetList, nonLinearList;
        for (; vdbIt; ++vdbIt) {
            if (boss.wasInterrupted()) break;

            const openvdb::GridClass gridClass = vdbIt->getGrid().getGridClass();
            if (gridClass != openvdb::GRID_LEVEL_SET) {
                nonLevelSetList.push_back(vdbIt.getPrimitiveNameOrIndex().toStdString());
                continue;
            }

            if (!vdbIt->getGrid().transform().isLinear()) {
                nonLinearList.push_back(vdbIt.getPrimitiveNameOrIndex().toStdString());
                continue;
            }

            delGroup->addOffset(vdbIt.getOffset());
            grids.push_back(vdbIt->getConstGridPtr());
            vdbs.push_back(*vdbIt);
        }

        if (!nonLevelSetList.empty()) {
            std::string s = "Reference meshing is only supported for "
                "Level Set grids, the following grids were skipped: '" +
                hboost::algorithm::join(nonLevelSetList, ", ") + "'.";
            addWarning(SOP_MESSAGE, s.c_str());
        }

        if (!nonLinearList.empty()) {
            std::string s = "The following grids were skipped: '" +
                hboost::algorithm::join(nonLinearList, ", ") +
                "' because they don't have a linear/affine transform.";
            addWarning(SOP_MESSAGE, s.c_str());
        }

        // Mesh using a reference surface
        if (!grids.empty() && !boss.wasInterrupted()) {

            if (grids.front()->isType<openvdb::FloatGrid>()) {
                referenceMeshing<openvdb::FloatGrid>(
                    grids, vdbs, delGroup, mesher, refGeo, computeNormals, boss, time);
            } else if (grids.front()->isType<openvdb::DoubleGrid>()) {
                referenceMeshing<openvdb::DoubleGrid>(
                    grids, vdbs, delGroup, mesher, refGeo, computeNormals, boss, time);
            } else {
                addError(SOP_MESSAGE, "Unsupported grid type.");
            }

            // Delete old VDB primitives
            if (error() < UT_ERROR_ABORT)
                gdp->destroyPrimitives(gdp->getPrimitiveRange(delGroup), /*and_points*/true);
        }

        if (delGroup) gdp->destroyGroup(delGroup);

    } else {

        ConvertTo target = static_cast<ConvertTo>(evalInt("conversion", 0, time));
        bool toPolySoup = (target == POLYSOUP);

        // Mesh each VDB primitive independently
        for (; vdbIt; ++vdbIt) {

            if (boss.wasInterrupted()) break;
            GEOvdbProcessTypedGridScalar(*vdbIt.getPrimitive(), mesher);

            delGroup->addOffset(vdbIt.getOffset());

            copyMesh(*gdp, *vdbIt, delGroup, mesher, toPolySoup);

        }

        // Delete old VDB primitives
        if (error() < UT_ERROR_ABORT)
            gdp->destroyPrimitives(gdp->getPrimitiveRange(delGroup), /*and_points*/true);

        if (!boss.wasInterrupted() && computeNormals) {
            UTparallelFor(GA_SplittableRange(gdp->getPrimitiveRange()),
                hvdb::VertexNormalOp(*gdp));
        }

        if (delGroup) gdp->destroyGroup(delGroup);
    }
}


////////////////////////////////////////


OP_ERROR
SOP_OpenVDB_Convert::Cache::cookVDBSop(OP_Context& context)
{
    try {
        const fpreal t = context.getTime();

        GA_PrimitiveGroup* group = parsePrimitiveGroupsCopy(
            evalStdString("group", t).c_str(), GroupCreator(gdp));

        hvdb::Interrupter interrupter("Converting VDBs");

        switch (evalInt("conversion",  0, t))
        {
            case HVOLUME: {
                const bool splitDisjointVols = (evalInt("splitdisjointvolumes", 0, t) != 0);
                convertToVolumes(*gdp, group, splitDisjointVols);
                break;
            }
            case OPENVDB: {
                const bool activateinside = (evalInt("activateinsidesdf", 0, t) != 0);
                convertToOpenVDB(*gdp, group,
                    (evalInt("flood", 0, t) != 0),
                    (evalInt("prune", 0, t) != 0),
                    evalFloat("tolerance", 0, t),
                    activateinside);

                UT_String newTypeStr, newPrecStr;
                evalString(newTypeStr, "vdbtype", 0, t);
                evalString(newPrecStr, "vdbprecision", 0, t);

                switch (evalInt("vdbclass", 0, t)) {
                    case CLASS_SDF:
                        convertVDBClass(*gdp, group, openvdb::GRID_LEVEL_SET,
                            static_cast<float>(evalFloat("fogisovalue", 0, t)));
                        newTypeStr = "none"; // SDFs are always floating-point
                        break;
                    case CLASS_FOG_VOLUME:
                        convertVDBClass(*gdp, group, openvdb::GRID_FOG_VOLUME, /*unused*/0);
                        newTypeStr = "none"; // fog volumes are always floating-point
                        break;
                    default:
                        // ignore
                        break;
                }

                if ((newTypeStr != "none") || (newPrecStr != "none")) {
                    convertVDBType(*gdp, group, newTypeStr, newPrecStr, interrupter);
                }
                break;
            }
            case POLYGONS: {
                convertToPoly(t, group, false, interrupter);
                break;
            }
            case POLYSOUP: {
                convertToPoly(t, group, true, interrupter);
                break;
            }
            default: {
                addWarning(SOP_MESSAGE, "Unrecognized conversion type");
                break;
            }
        }

        if (interrupter.wasInterrupted()) {
            addWarning(SOP_MESSAGE, "Process was interrupted");
        }

        interrupter.end();

    } catch (std::exception& e) {
        addError(SOP_MESSAGE, e.what());
    }
    return error();
}
