// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file SOP_OpenVDB_Visualize.cc
///
/// @author FX R&D OpenVDB team
///
/// @brief Visualize VDB grids and their tree topology

#include <houdini_utils/ParmFactory.h>
#include <houdini_utils/geometry.h>
#include <openvdb_houdini/GeometryUtil.h>
#include <openvdb_houdini/Utils.h>
#include <openvdb_houdini/SOP_NodeVDB.h>
#include <openvdb/tools/PointIndexGrid.h>
#include <openvdb/points/PointDataGrid.h>

#ifdef DWA_OPENVDB
#include <openvdb_houdini/DW_VDBUtils.h>
#endif

#include <GA/GA_Handle.h>
#include <GA/GA_Types.h>
#include <GU/GU_ConvertParms.h>
#include <GU/GU_Detail.h>
#include <GU/GU_PolyReduce.h>
#include <GU/GU_Surfacer.h>
#include <PRM/PRM_Parm.h>
#include <UT/UT_Interrupt.h>
#include <UT/UT_VectorTypes.h> // for UT_Vector3i
#include <UT/UT_Version.h>
#include <UT/UT_UniquePtr.h>

#include <algorithm>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>


namespace std {
template<> struct is_integral<openvdb::PointIndex32>: public true_type {};
template<> struct is_integral<openvdb::PointIndex64>: public true_type {};
template<> struct is_integral<openvdb::PointDataIndex32>: public true_type {};
template<> struct is_integral<openvdb::PointDataIndex64>: public true_type {};
}

namespace hvdb = openvdb_houdini;
namespace hutil = houdini_utils;

// HAVE_SURFACING_PARM is disabled in H12.5
#ifdef SESI_OPENVDB
#define HAVE_SURFACING_PARM 0
#else
#define HAVE_SURFACING_PARM 1
#endif


enum RenderStyle { STYLE_NONE = 0, STYLE_POINTS, STYLE_WIRE_BOX, STYLE_SOLID_BOX };

enum MeshMode { MESH_NONE = 0, MESH_OPENVDB, MESH_HOUDINI };


class SOP_OpenVDB_Visualize: public hvdb::SOP_NodeVDB
{
public:
    SOP_OpenVDB_Visualize(OP_Network*, const char* name, OP_Operator*);
    ~SOP_OpenVDB_Visualize() override = default;

    static OP_Node* factory(OP_Network*, const char* name, OP_Operator*);

    int isRefInput(unsigned i) const override { return (i == 1); }

    static UT_Vector3 colorLevel(int level) { return sColors[std::max(3-level,0)]; }
    static const UT_Vector3& colorSign(bool negative) { return sColors[negative ? 5 : 4]; }

    class Cache: public SOP_VDBCacheOptions { OP_ERROR cookVDBSop(OP_Context&) override; };

protected:
    bool updateParmsFlags() override;
    void resolveObsoleteParms(PRM_ParmList*) override;

private:
    static const UT_Vector3 sColors[];
};


// Same color scheme as the VDB TOG paper.
const UT_Vector3 SOP_OpenVDB_Visualize::sColors[] = {
    UT_Vector3(0.045f, 0.045f, 0.045f),         // 0. Root
    UT_Vector3(0.0432f, 0.33f, 0.0411023f),     // 1. First internal node level
    UT_Vector3(0.871f, 0.394f, 0.01916f),       // 2. Intermediate internal node levels
    UT_Vector3(0.00608299f, 0.279541f, 0.625f), // 3. Leaf level
    UT_Vector3(0.523f, 0.0325175f, 0.0325175f), // 4. Value >= ZeroVal (for voxels or tiles)
    UT_Vector3(0.92f, 0.92f, 0.92f)             // 5. Value < ZeroVal (for voxels or tiles)
};


////////////////////////////////////////


void
newSopOperator(OP_OperatorTable* table)
{
    if (table == nullptr) return;

    hutil::ParmList parms;

    // Group pattern
    parms.add(hutil::ParmFactory(PRM_STRING, "group", "Group")
        .setChoiceList(&hutil::PrimGroupMenuInput1)
        .setTooltip("Specify a subset of the input VDBs to be processed.")
        .setDocumentation(
            "The VDBs to be visualized (see [specifying volumes|/model/volumes#group])"));

#if HAVE_SURFACING_PARM
    // Surfacing
    parms.add(hutil::ParmFactory(PRM_HEADING, "surfacing", "Surfacing"));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "drawsurface", "")
        .setTypeExtended(PRM_TYPE_TOGGLE_JOIN));

    {   // Meshing scheme
        char const * const items[] = {
            "openvdb",  "OpenVDB Mesher",
            "houdini",  "Houdini Surfacer",
            nullptr
        };
        parms.add(hutil::ParmFactory(PRM_ORD, "mesher", "Mesher")
            .setChoiceListItems(PRM_CHOICELIST_SINGLE, items)
            .setTooltip("Select a meshing scheme.")
            .setDocumentation("The meshing scheme to be used to visualize scalar volumes"));
    }

    parms.add(hutil::ParmFactory(PRM_FLT_J, "adaptivity", "Adaptivity")
        .setRange(PRM_RANGE_RESTRICTED, 0.0, PRM_RANGE_RESTRICTED, 1.0)
        .setDocumentation(
            "How closely to match the surface\n\n"
            "Higher adaptivity allows for more variation in polygon size,"
            " so that fewer polygons are used to represent the surface."));

    //parms.add(hutil::ParmFactory(PRM_TOGGLE, "computeNormals", "Compute Point Normals"));

    parms.add(hutil::ParmFactory(PRM_FLT_J, "isoValue", "Isovalue")
        .setRange(PRM_RANGE_FREE, -2.0, PRM_RANGE_FREE, 2.0)
        .setDocumentation("The isovalue of the surface to be meshed"));
    parms.add(
        hutil::ParmFactory(PRM_RGB_J, "surfaceColor", "Surface Color")
        .setDefault(std::vector<PRM_Default>(3, PRM_Default(0.84))) // RGB = (0.84, 0.84, 0.84)
        .setVectorSize(3)
        .setDocumentation("The color of the surface mesh"));

    // Tree Topology
    parms.add(hutil::ParmFactory(PRM_HEADING,"treeTopology", "Tree Topology"));
#endif // HAVE_SURFACING_PARM

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "addcolor", "Color")
        .setDefault(PRMoneDefaults)
        .setTooltip("Specify whether to draw in color.")
        .setDocumentation(
            "Specify whether to generate geometry with the `Cd` color attribute."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "previewfrustum", "Frustum")
        .setTooltip(
            "Specify whether to draw the camera frustums\n"
            "of VDBs with frustum transforms.")
        .setDocumentation(
            "For VDBs with [frustum transforms|http://www.openvdb.org/documentation/"
            "doxygen/transformsAndMaps.html#sFrustumTransforms],"
            " generate geometry representing the frustum bounding box."));

#ifdef DWA_OPENVDB
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "previewroi", "Region of Interest")
        .setDocumentation(
            "If enabled, generate geometry representing the region of interest"
            " (for VDBs with ROI metadata)."));
#endif

    char const * const boxItems[] = {
        "wirebox",  "Wireframe Boxes",
        "box",      "Solid Boxes",
        nullptr
    };
    char const * const pointAndBoxItems[] = {
        "points",   "Points",
        "wirebox",  "Wireframe Boxes",
        "box",      "Solid Boxes",
        nullptr
    };

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "drawleafnodes", "")
        .setDefault(PRMoneDefaults)
        .setTypeExtended(PRM_TYPE_TOGGLE_JOIN));
    parms.add(hutil::ParmFactory(PRM_STRING, "leafstyle", "Leaf Nodes")
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, pointAndBoxItems)
        .setDefault("wirebox")
        .setDocumentation(
            "Specify whether to render the leaf nodes of VDB trees"
            " as wireframe boxes, as solid boxes, or as a single point"
            " in the middle of each node.\n\n"
            "If __Color__ is enabled, leaf nodes will be blue."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "drawinternalnodes", "")
        .setTypeExtended(PRM_TYPE_TOGGLE_JOIN));
    parms.add(hutil::ParmFactory(PRM_STRING, "internalstyle", "Internal Nodes")
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, boxItems)
        .setDefault("wirebox")
        .setDocumentation(
            "Specify whether to render the internal nodes of VDB trees"
            " as wireframe boxes or as solid boxes.\n\n"
            "If __Color__ is enabled, the lowest-level internal nodes will be green"
            " and higher-level internal nodes will be orange."));

    // Active tiles
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "drawtiles", "")
        .setDefault(PRMoneDefaults)
        .setTypeExtended(PRM_TYPE_TOGGLE_JOIN));

    parms.add(hutil::ParmFactory(PRM_STRING, "tilestyle", "Active Tiles")
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, pointAndBoxItems)
        .setDefault("wirebox")
        .setDocumentation(
            "Specify whether to render the active tiles of VDB trees"
            " as wireframe boxes, as solid boxes, or as a single point"
            " in the middle of each tile.\n\n"
            "If __Color__ is enabled, negative-valued tiles will be white"
            " and nonnegative tiles will be red."));

    // Active voxels
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "drawvoxels", "")
        .setTypeExtended(PRM_TYPE_TOGGLE_JOIN));

    parms.add(hutil::ParmFactory(PRM_ORD, "voxelstyle", "Active Voxels")
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, pointAndBoxItems)
        .setDefault("points")
        .setDocumentation(
            "Specify whether to render the active voxels of VDB trees"
            " as wireframe boxes, as solid boxes, or as a single point"
            " in the middle of each voxel.\n\n"
            "If __Color__ is enabled, negative-valued voxels will be white"
            " and nonnegative voxels will be red.\n\n"
            "WARNING:\n"
            "    Rendering active voxels as boxes can generate large amounts of geometry."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "ignorestaggered", "Ignore Staggered Vectors")
        .setTooltip("Draw staggered vectors as if they were collocated."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "addindexcoord", "Points with Index Coordinates")
        .setTooltip("Add a voxel/tile index coordinate attribute to points.")
        .setDocumentation(
            "For voxels, tiles, and leaf nodes rendered as points, add an attribute to"
            " the points that gives the coordinates of the points in the VDB's [index space|"
            "http://www.openvdb.org/documentation/doxygen/overview.html#secSpaceAndTrans]."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "addvalue", "Points with Values")
        .setTooltip("Add a voxel/tile value attribute to points.")
        .setDocumentation(
            "For voxels and tiles rendered as points, add an attribute to the points"
            " that gives the voxel and tile values."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "usegridname", "Name Point Attributes After VDBs")
        .setTooltip(
            "If enabled, use the VDB name as the attribute name when\n"
            "displaying points with values.\n"
            "If disabled or if a VDB has no name, use either \"vdb_int\",\n"
            "\"vdb_float\" or \"vdb_vec3f\" as the attribute name.")
        .setDocumentation(
            "If enabled, name the attribute added by __Points with Values__ after"
            " the VDB primitive.  If disabled or if a VDB has no name, name the point"
            " attribute according to its type: `vdb_int`, `vdb_float`, `vdb_vec3f`, etc."));

    // Obsolete parameters
    hutil::ParmList obsoleteParms;
    obsoleteParms.beginSwitcher("tabMenu");
    obsoleteParms.addFolder("Tree Topology");
    obsoleteParms.endSwitcher();
    obsoleteParms.add(hutil::ParmFactory(PRM_HEADING,"renderOptions", "Render Options"));
    obsoleteParms.add(hutil::ParmFactory(PRM_TOGGLE, "extractMesh", "Extract Mesh"));
    obsoleteParms.add(hutil::ParmFactory(PRM_TOGGLE, "computeNormals", "Compute Point Normals"));
    obsoleteParms.add(hutil::ParmFactory(PRM_TOGGLE, "reverse", "Reverse Faces"));
    obsoleteParms.add(hutil::ParmFactory(PRM_HEADING, "optionsHeading", "Options"));
    obsoleteParms.add(hutil::ParmFactory(PRM_TOGGLE, "verbose", "Verbose"));
    obsoleteParms.add(hutil::ParmFactory(PRM_HEADING, "Other", "Other"));
    {
        char const * const items[] = {
            "none",     "Disabled",
            "opevdb",   "OpenVDB Mesher", // note the misspelling
            "houdini",  "Houdini Surfacer",
            nullptr
        };
        obsoleteParms.add(hutil::ParmFactory(PRM_ORD, "meshing", "Meshing")
            .setChoiceListItems(PRM_CHOICELIST_SINGLE, items));
    }
    {
        char const * const items[] = {
            "none",     "Disabled",
            "leaf",     "Leaf Nodes and Active Tiles",
            "nonconst", "Leaf and Internal Nodes",
            nullptr
        };
        obsoleteParms.add(hutil::ParmFactory(PRM_ORD, "nodes", "Tree Nodes")
            .setDefault(PRMoneDefaults)
            .setChoiceListItems(PRM_CHOICELIST_SINGLE, items));
    }
    {
        char const * const items[] = {
            "none",     "Disabled",
            "points",   "Points",
            "pvalue",   "Points with Values",
            "wirebox",  "Wireframe Box",
            "box",      "Solid Box",
            nullptr
        };
        obsoleteParms.add(hutil::ParmFactory(PRM_ORD, "tiles", "Active Constant Tiles")
            .setChoiceListItems(PRM_CHOICELIST_SINGLE, items));
        obsoleteParms.add(hutil::ParmFactory(PRM_ORD, "voxels", "Active Voxels")
            .setChoiceListItems(PRM_CHOICELIST_SINGLE, items));
    }
    obsoleteParms.add(hutil::ParmFactory(PRM_TOGGLE, "previewFrustum", "Frustum"));
    obsoleteParms.add(hutil::ParmFactory(PRM_ORD, "leafmode", "Leaf Nodes")
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, boxItems));
    obsoleteParms.add(hutil::ParmFactory(PRM_ORD, "internalmode", "Internal Nodes")
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, boxItems));
    obsoleteParms.add(hutil::ParmFactory(PRM_ORD, "tilemode", "Active Tiles")
        .setDefault(PRMoneDefaults)
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, pointAndBoxItems));
    obsoleteParms.add(hutil::ParmFactory(PRM_ORD, "voxelmode", "Active Voxels")
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, pointAndBoxItems));

#ifndef DWA_OPENVDB
    // We probably need this to share hip files.
    obsoleteParms.add(hutil::ParmFactory(PRM_TOGGLE, "previewroi", ""));
#endif

    // Register this operator.
    hvdb::OpenVDBOpFactory("VDB Visualize Tree",
        SOP_OpenVDB_Visualize::factory, parms, *table)
#ifndef SESI_OPENVDB
        .setInternalName("DW_OpenVDBVisualize")
#endif
        .setObsoleteParms(obsoleteParms)
        .addInput("Input with VDBs to visualize")
        .setVerb(SOP_NodeVerb::COOK_GENERATOR, []() { return new SOP_OpenVDB_Visualize::Cache; })
        .setDocumentation("\
#icon: COMMON/openvdb\n\
#tags: vdb\n\
\n\
\"\"\"Generate geometry to visualize the internal\n\
[tree structure|http://www.openvdb.org/documentation/doxygen/overview.html#secTree]\n\
of a VDB volume.\"\"\"\n\
\n\
@overview\n\
\n\
This node can be a useful troubleshooting tool.\n\
Among other things, it allows one to evaluate the\n\
[sparseness|http://www.openvdb.org/documentation/doxygen/overview.html#secSparsity]\n\
of VDB volumes as well as to examine their extents and the values of individual voxels.\n\
\n\
@related\n\
- [Node:sop/vdbvisualizetree]\n\
\n\
@examples\n\
\n\
See [openvdb.org|http://www.openvdb.org/download/] for source code\n\
and usage examples.\n");
}


////////////////////////////////////////


OP_Node*
SOP_OpenVDB_Visualize::factory(OP_Network* net,
    const char* name, OP_Operator* op)
{
    return new SOP_OpenVDB_Visualize(net, name, op);
}


SOP_OpenVDB_Visualize::SOP_OpenVDB_Visualize(OP_Network* net,
    const char* name, OP_Operator* op):
    hvdb::SOP_NodeVDB(net, name, op)
{
}


////////////////////////////////////////


void
SOP_OpenVDB_Visualize::resolveObsoleteParms(PRM_ParmList* obsoleteParms)
{
    if (!obsoleteParms) return;

    const fpreal time = 0.0;

    // The "extractMesh" toggle switched Houdini surfacing on or off.
    PRM_Parm* parm = obsoleteParms->getParmPtr("extractMesh");
    if (parm && !parm->isFactoryDefault()) {
        const bool extractMesh = obsoleteParms->evalInt("extractMesh", 0, time);
        setInt("drawsurface", 0, time, extractMesh);
        if (extractMesh) setString(UT_String("houdini"), CH_STRING_LITERAL, "mesher", 0, time);
    }

    // The "meshing" menu included a "Disabled" option, which is now handled with a toggle.
    parm = obsoleteParms->getParmPtr("meshing");
    if (parm && !parm->isFactoryDefault()) {
        // 0: disabled, 1: OpenVDB mesher, 2: Houdini surfacer
        const int meshing = obsoleteParms->evalInt("meshing", 0, time);
        setInt("drawsurface", 0, time, meshing > 0);
        if (meshing) {
            setString(UT_String(meshing == 2 ? "houdini" : "openvdb"),
                CH_STRING_LITERAL, "mesher", 0, time);
        }
    }

    // The "nodes", "tiles" and "voxels" menus all included "Disabled" options,
    // which are now handled with toggles.  The old scheme also had two conflicting
    // ways to enable display of tiles, which the following attempts to reconcile.

    const UT_String pointStr("points"), boxStr("box"), wireStr("wirebox");

    parm = obsoleteParms->getParmPtr("nodes");
    if (parm && !parm->isFactoryDefault()) {
        // 0: disabled, 1: leaf nodes and active tiles, 2: leaf and internal nodes
        const int mode = obsoleteParms->evalInt("nodes", 0, time);
        // Enable leaf nodes if the old mode was not "Disabled".
        setInt("drawleafnodes", 0, time, mode > 0);
        // Set the leaf style to wire box, but only if leaf nodes are displayed.
        if (mode > 0) setString(wireStr, CH_STRING_LITERAL, "leafstyle", 0, time);
        // Enable internal nodes if the old mode was "Leaf and Internal Nodes".
        setInt("drawinternalnodes", 0, time, mode == 2);
        // Set the internal node style to wire box, but only if internal nodes are displayed.
        if (mode == 2) {
            setString(wireStr, CH_STRING_LITERAL, "internalstyle", 0, time);
        }
        // Disable tiles if the old mode was not "Leaf Nodes and Active Tiles".
        setInt("drawtiles", 0, time, mode == 1);
        if (mode == 1) {
            // Display tiles as wire boxes if the old mode was "Leaf Nodes and Active Tiles".
            // (This setting took precedence over the tile mode, below.)
            setString(wireStr, CH_STRING_LITERAL, "tilestyle", 0, time);
        }
    }

    parm = obsoleteParms->getParmPtr("tiles");
    if (parm && !parm->isFactoryDefault()) {
        // 0: disabled, 1: points, 2: points with values, 3: wire boxes, 4: solid boxes
        const int mode = obsoleteParms->evalInt("tiles", 0, time);
        if (mode > 0) setInt("drawtiles", 0, time, true);
        switch (mode) {
            case 1:
                setString(pointStr, CH_STRING_LITERAL, "tilestyle", 0, time);
                setInt("addvalue", 0, time, false);
                break;
            case 2:
                setString(pointStr, CH_STRING_LITERAL, "tilestyle", 0, time);
                setInt("addvalue", 0, time, true);
                break;
            case 3: setString(wireStr, CH_STRING_LITERAL, "tilestyle", 0, time); break;
            case 4: setString(boxStr, CH_STRING_LITERAL, "tilestyle", 0, time); break;
        }
    }

    parm = obsoleteParms->getParmPtr("voxels");
    if (parm && !parm->isFactoryDefault()) {
        // 0: disabled, 1: points, 2: points with values, 3: wire boxes, 4: solid boxes
        const int mode = obsoleteParms->evalInt("voxels", 0, time);
        setInt("drawvoxels", 0, time, mode > 0);
        switch (mode) {
            case 1:
                setString(pointStr, CH_STRING_LITERAL, "voxelstyle", 0, time);
                setInt("addvalue", 0, time, false);
                break;
            case 2:
                setString(pointStr, CH_STRING_LITERAL, "voxelstyle", 0, time);
                setInt("addvalue", 0, time, true);
                break;
            case 3: setString(wireStr, CH_STRING_LITERAL, "voxelstyle", 0, time); break;
            case 4: setString(boxStr, CH_STRING_LITERAL, "voxelstyle", 0, time); break;
        }
    }

    for (const auto* name: {"leaf", "internal"}) {
        const auto oldName = std::string(name) + "mode";
        const auto newName = std::string(name) + "style";
        parm = obsoleteParms->getParmPtr(oldName.c_str());
        if (parm && !parm->isFactoryDefault()) {
            const int mode = obsoleteParms->evalInt(oldName.c_str(), 0, time);
            setString(mode == 0 ? wireStr : boxStr, CH_STRING_LITERAL, newName.c_str(), 0, time);
        }
    }

    for (const auto* name: {"tile", "voxel"}) {
        const auto oldName = std::string(name) + "mode";
        const auto newName = std::string(name) + "style";
        parm = obsoleteParms->getParmPtr(oldName.c_str());
        if (parm && !parm->isFactoryDefault()) {
            switch (obsoleteParms->evalInt(oldName.c_str(), 0, time)) {
                case 0: setString(pointStr, CH_STRING_LITERAL, newName.c_str(), 0, time); break;
                case 1: setString(wireStr, CH_STRING_LITERAL, newName.c_str(), 0, time); break;
                case 2: setString(boxStr, CH_STRING_LITERAL, newName.c_str(), 0, time); break;
            }
        }
    }

    resolveRenamedParm(*obsoleteParms, "previewFrustum", "previewfrustum");

    // Delegate to the base class.
    hvdb::SOP_NodeVDB::resolveObsoleteParms(obsoleteParms);
}


// Update UI parm display
bool
SOP_OpenVDB_Visualize::updateParmsFlags()
{
    bool changed = false;

    const fpreal time = 0.0;

#if HAVE_SURFACING_PARM
    const bool extractMesh = bool(evalInt("drawsurface", 0, time));
    changed |= enableParm("mesher", extractMesh);
    //changed += enableParm("computeNormals", extractMesh);
    changed |= enableParm("adaptivity", extractMesh);
    changed |= enableParm("surfaceColor", extractMesh);
    changed |= enableParm("isoValue", extractMesh);
#endif

    const std::string
        leafMode = evalStdString("leafstyle", time),
        tileMode = evalStdString("tilestyle", time),
        voxelMode = evalStdString("voxelstyle", time);
    const bool
        drawLeafNodes = bool(evalInt("drawleafnodes", 0, time)),
        drawVoxels = bool(evalInt("drawvoxels", 0, time)),
        drawTiles = bool(evalInt("drawtiles", 0, time)),
        drawPoints = (drawTiles && tileMode == "points") || (drawVoxels && voxelMode == "points");

    changed |= enableParm("leafstyle", drawLeafNodes);
    changed |= enableParm("internalstyle", bool(evalInt("drawinternalnodes", 0, time)));
    changed |= enableParm("tilestyle", drawTiles);
    changed |= enableParm("voxelstyle", drawVoxels);
    changed |= enableParm("ignorestaggered", drawVoxels);
    changed |= enableParm("addvalue", drawPoints);
    changed |= enableParm("addindexcoord", drawPoints || (drawLeafNodes && leafMode == "points"));
    changed |= enableParm("usegridname", drawPoints);

    return changed;
}


////////////////////////////////////////


template<typename OpType>
inline RenderStyle
evalRenderStyle(OpType& op, const char* toggleName, const char* modeName, fpreal time)
{
    RenderStyle style = STYLE_NONE;
    if (op.evalInt(toggleName, 0, time)) {
        const std::string mode = op.evalStdString(modeName, time);
        if (mode == "points") {
            style = STYLE_POINTS;
        } else if (mode == "wirebox") {
            style = STYLE_WIRE_BOX;
        } else if (mode == "box") {
            style = STYLE_SOLID_BOX;
        }
    }
    return style;
}


////////////////////////////////////////


inline void
createBox(GU_Detail& geo, const openvdb::math::Transform& xform,
    const openvdb::CoordBBox& bbox, const UT_Vector3* color = nullptr, bool solid = false)
{
    struct Local {
        static inline UT_Vector3 Vec3dToUTV3(const openvdb::Vec3d& v) {
            return UT_Vector3(float(v.x()), float(v.y()), float(v.z()));
        }
    };

    UT_Vector3 corners[8];

#if 1
    // Nodes are rendered as cell-centered (0.5 voxel dilated) AABBox in world space
    const openvdb::Vec3d min(bbox.min().x()-0.5, bbox.min().y()-0.5, bbox.min().z()-0.5);
    const openvdb::Vec3d max(bbox.max().x()+0.5, bbox.max().y()+0.5, bbox.max().z()+0.5);
#else
    // Render as node-centered (used for debugging)
    const openvdb::Vec3d min(bbox.min().x(), bbox.min().y(), bbox.min().z());
    const openvdb::Vec3d max(bbox.max().x()+1.0, bbox.max().y()+1.0, bbox.max().z()+1.0);
#endif

    openvdb::Vec3d ptn = xform.indexToWorld(min);
    corners[0] = Local::Vec3dToUTV3(ptn);

    ptn = openvdb::Vec3d(min.x(), min.y(), max.z());
    ptn = xform.indexToWorld(ptn);
    corners[1] = Local::Vec3dToUTV3(ptn);

    ptn = openvdb::Vec3d(max.x(), min.y(), max.z());
    ptn = xform.indexToWorld(ptn);
    corners[2] = Local::Vec3dToUTV3(ptn);

    ptn = openvdb::Vec3d(max.x(), min.y(), min.z());
    ptn = xform.indexToWorld(ptn);
    corners[3] = Local::Vec3dToUTV3(ptn);

    ptn = openvdb::Vec3d(min.x(), max.y(), min.z());
    ptn = xform.indexToWorld(ptn);
    corners[4] = Local::Vec3dToUTV3(ptn);

    ptn = openvdb::Vec3d(min.x(), max.y(), max.z());
    ptn = xform.indexToWorld(ptn);
    corners[5] = Local::Vec3dToUTV3(ptn);

    ptn = xform.indexToWorld(max);
    corners[6] = Local::Vec3dToUTV3(ptn);

    ptn = openvdb::Vec3d(max.x(), max.y(), min.z());
    ptn = xform.indexToWorld(ptn);
    corners[7] = Local::Vec3dToUTV3(ptn);

    hutil::createBox(geo, corners, color, solid);
}


////////////////////////////////////////


struct TreeParms
{
    RenderStyle internalStyle = STYLE_NONE;
    RenderStyle tileStyle = STYLE_NONE;
    RenderStyle leafStyle = STYLE_NONE;
    RenderStyle voxelStyle = STYLE_NONE;
    bool addColor = true;
    bool ignoreStaggeredVectors = false;
    bool addValue = false;
    bool addIndexCoord = false;
    bool useGridName = false;
};


class TreeVisualizer
{
public:
    TreeVisualizer(GU_Detail&, const TreeParms&, hvdb::Interrupter* = nullptr);

    template<typename GridType>
    void operator()(const GridType&);

private:
    /// @param pos position in index coordinates
    GA_Offset createPoint(const openvdb::Vec3d& pos);

    GA_Offset createPoint(const openvdb::CoordBBox&, const UT_Vector3& color);

    template<typename ValType>
    typename std::enable_if<std::is_integral<ValType>::value>::type
    addPoint(const openvdb::CoordBBox&, const UT_Vector3& color, ValType s, bool);

    template<typename ValType>
    typename std::enable_if<std::is_floating_point<ValType>::value>::type
    addPoint(const openvdb::CoordBBox&, const UT_Vector3& color, ValType s, bool);

    template<typename ValType>
    typename std::enable_if<!std::is_arithmetic<ValType>::value>::type
    addPoint(const openvdb::CoordBBox&, const UT_Vector3& color, ValType v, bool staggered);

    void addPoint(const openvdb::CoordBBox&, const UT_Vector3& color, bool staggered);

    void addBox(const openvdb::CoordBBox&, const UT_Vector3& color, bool solid);

    bool wasInterrupted(int percent = -1) const {
        return mInterrupter && mInterrupter->wasInterrupted(percent);
    }


    TreeParms mParms;
    GU_Detail* mGeo;
    hvdb::Interrupter* mInterrupter;
    const openvdb::math::Transform* mXform;
    GA_RWHandleF  mFloatHandle;
    GA_RWHandleI  mInt32Handle;
    GA_RWHandleV3 mVec3fHandle;
    GA_RWHandleV3 mCdHandle;
    GA_RWHandleT<UT_Vector3i> mIndexCoordHandle;
};


////////////////////////////////////////


TreeVisualizer::TreeVisualizer(GU_Detail& geo, const TreeParms& parms,
    hvdb::Interrupter* interrupter)
    : mParms(parms)
    , mGeo(&geo)
    , mInterrupter(interrupter)
    , mXform(nullptr)
{
}


template<typename GridType>
void
TreeVisualizer::operator()(const GridType& grid)
{
    using TreeType = typename GridType::TreeType;

    mXform = &grid.transform();

    const bool staggered = !mParms.ignoreStaggeredVectors &&
        (grid.getGridClass() == openvdb::GRID_STAGGERED);

    //{
    // Create point attributes.

    if (mParms.addColor) {
        mCdHandle.bind(mGeo->findDiffuseAttribute(GA_ATTRIB_POINT));
        if (!mCdHandle.isValid()) {
            mCdHandle.bind(mGeo->addDiffuseAttribute(GA_ATTRIB_POINT));
        }
    }

    if (mParms.addIndexCoord &&
        ((mParms.tileStyle == STYLE_POINTS)
        || (mParms.voxelStyle == STYLE_POINTS)
        || (mParms.leafStyle == STYLE_POINTS)))
    {
        const UT_String attrName = "vdb_ijk";

        GA_RWAttributeRef attribHandle = mGeo->findIntTuple(GA_ATTRIB_POINT, attrName, 3);
        if (!attribHandle.isValid()) {
            attribHandle = mGeo->addIntTuple(GA_ATTRIB_POINT, attrName, 3, GA_Defaults(0));
        }

        mIndexCoordHandle = attribHandle.getAttribute();

        UT_String varName = attrName;
        varName.toUpper();
        mGeo->addVariableName(attrName, varName);
    }

    if (mParms.addValue &&
        ((mParms.tileStyle == STYLE_POINTS) || (mParms.voxelStyle == STYLE_POINTS)))
    {
        const std::string valueType = grid.valueType();

        UT_String attrName;
        if (mParms.useGridName) {
            attrName = grid.getName();
            attrName.forceValidVariableName();
        }

        if (valueType == openvdb::typeNameAsString<float>() ||
            valueType == openvdb::typeNameAsString<double>())
        {
            if (!attrName.isstring()) attrName = "vdb_float";
            UT_String varName = attrName;
            varName.toUpper();

            GA_RWAttributeRef attribHandle =
                mGeo->findFloatTuple(GA_ATTRIB_POINT, attrName, 1);

            if (!attribHandle.isValid()) {
                attribHandle = mGeo->addFloatTuple(
                    GA_ATTRIB_POINT, attrName, 1, GA_Defaults(0));
            }

            mFloatHandle = attribHandle.getAttribute();
            mGeo->addVariableName(attrName, varName);

        } else if (valueType == openvdb::typeNameAsString<int32_t>() ||
            valueType == openvdb::typeNameAsString<int64_t>() ||
            valueType == openvdb::typeNameAsString<bool>())
        {
            if (!attrName.isstring()) attrName = "vdb_int";
            UT_String varName = attrName;
            varName.toUpper();

            GA_RWAttributeRef attribHandle =
                mGeo->findIntTuple(GA_ATTRIB_POINT, attrName, 1);

            if (!attribHandle.isValid()) {
                attribHandle = mGeo->addIntTuple(
                    GA_ATTRIB_POINT, attrName, 1, GA_Defaults(0));
            }

            mInt32Handle = attribHandle.getAttribute();
            mGeo->addVariableName(attrName, varName);

        } else if (valueType == openvdb::typeNameAsString<openvdb::Vec3s>() ||
            valueType == openvdb::typeNameAsString<openvdb::Vec3d>())
        {
            if (!attrName.isstring()) attrName = "vdb_vec3f";
            UT_String varName = attrName;
            varName.toUpper();

            GA_RWAttributeRef attribHandle =
                mGeo->findFloatTuple(GA_ATTRIB_POINT, attrName, 3);

            if (!attribHandle.isValid()) {
                attribHandle = mGeo->addFloatTuple(
                    GA_ATTRIB_POINT, attrName, 3, GA_Defaults(0));
            }

            mVec3fHandle = attribHandle.getAttribute();
            mGeo->addVariableName(attrName, varName);

        } else {
            throw std::runtime_error(
                "value attributes are not supported for values of type " + valueType);
        }
    }

    //}

    // Render nodes.
    if (mParms.internalStyle || mParms.leafStyle) {
        openvdb::CoordBBox bbox;

        for (typename TreeType::NodeCIter iter(grid.tree()); iter; ++iter) {
            if (iter.getDepth() == 0) continue; // don't draw the root node

            const bool isLeaf = (iter.getLevel() == 0);
            if (isLeaf && !mParms.leafStyle) continue;
            if (!isLeaf && !mParms.internalStyle) continue;

            const bool solid = (isLeaf ? mParms.leafStyle == STYLE_SOLID_BOX
                : mParms.internalStyle == STYLE_SOLID_BOX);

            const auto color = SOP_OpenVDB_Visualize::colorLevel(iter.getLevel());

            iter.getBoundingBox(bbox);
            if (isLeaf && mParms.leafStyle == STYLE_POINTS) {
                addPoint(bbox, color, staggered);
            } else {
                addBox(bbox, color, solid);
            }
        }
    }

    if (!mParms.tileStyle && !mParms.voxelStyle) return;

    // Render tiles and voxels.
    openvdb::CoordBBox bbox;
    for (auto iter = grid.cbeginValueOn(); iter; ++iter) {
        if (wasInterrupted()) break;

        const int style = iter.isVoxelValue() ? mParms.voxelStyle : mParms.tileStyle;
        if (style == STYLE_NONE) continue;

        const bool negative = openvdb::math::isNegative(iter.getValue());
        const UT_Vector3& color = SOP_OpenVDB_Visualize::colorSign(negative);
        iter.getBoundingBox(bbox);

        if (style == STYLE_POINTS) {
            if (mParms.addValue) {
                addPoint(bbox, color, iter.getValue(), staggered);
            } else {
                addPoint(bbox, color, staggered);
            }
        } else {
            addBox(bbox, color, style == STYLE_SOLID_BOX);
        }
    }
}


inline GA_Offset
TreeVisualizer::createPoint(const openvdb::Vec3d& pos)
{
    openvdb::Vec3d wpos = mXform->indexToWorld(pos);
    GA_Offset offset = mGeo->appendPointOffset();
    mGeo->setPos3(offset, wpos[0], wpos[1], wpos[2]);
    if (mIndexCoordHandle.isValid()) {
        // Attach the (integer) index coordinates of the voxel at the given pos.
        openvdb::Coord idxPos = openvdb::Coord::floor(pos);
        mIndexCoordHandle.set(offset, UT_Vector3i(idxPos[0], idxPos[1], idxPos[2]));
    }
    return offset;
}


inline GA_Offset
TreeVisualizer::createPoint(const openvdb::CoordBBox& bbox,
    const UT_Vector3& color)
{
    openvdb::Vec3d pos = openvdb::Vec3d(0.5*(bbox.min().x()+bbox.max().x()),
                                        0.5*(bbox.min().y()+bbox.max().y()),
                                        0.5*(bbox.min().z()+bbox.max().z()));
    GA_Offset offset = createPoint(pos);
    if (mCdHandle.isValid()) mCdHandle.set(offset, color);
    return offset;
}


template<typename ValType>
typename std::enable_if<std::is_integral<ValType>::value>::type
TreeVisualizer::addPoint(const openvdb::CoordBBox& bbox,
    const UT_Vector3& color, ValType s, bool)
{
    mInt32Handle.set(createPoint(bbox, color), int(s));
}


template<typename ValType>
typename std::enable_if<std::is_floating_point<ValType>::value>::type
TreeVisualizer::addPoint(const openvdb::CoordBBox& bbox,
    const UT_Vector3& color, ValType s, bool)
{
    mFloatHandle.set(createPoint(bbox, color), float(s));
}


template<typename ValType>
typename std::enable_if<!std::is_arithmetic<ValType>::value>::type
TreeVisualizer::addPoint(const openvdb::CoordBBox& bbox,
    const UT_Vector3& color, ValType v, bool staggered)
{
    if (!staggered) {
        mVec3fHandle.set(createPoint(bbox, color),
            UT_Vector3(float(v[0]), float(v[1]), float(v[2])));
    } else {
        openvdb::Vec3d pos = openvdb::Vec3d(0.5*(bbox.min().x()+bbox.max().x()),
                                            0.5*(bbox.min().y()+bbox.max().y()),
                                            0.5*(bbox.min().z()+bbox.max().z()));
        pos[0] -= 0.5; // -x
        GA_Offset offset = createPoint(pos);
        if (mCdHandle.isValid()) mCdHandle.set(offset, UT_Vector3(1.0, 0.0, 0.0)); // r
        mVec3fHandle.set(offset, UT_Vector3(float(v[0]), 0.0, 0.0));

        pos[0] += 0.5;
        pos[1] -= 0.5; // -y
        offset = createPoint(pos);
        if (mCdHandle.isValid()) mCdHandle.set(offset, UT_Vector3(0.0, 1.0, 0.0)); // g
        mVec3fHandle.set(offset, UT_Vector3(0.0, float(v[1]), 0.0));

        pos[1] += 0.5;
        pos[2] -= 0.5; // -z
        offset = createPoint(pos);
        if (mCdHandle.isValid()) mCdHandle.set(offset, UT_Vector3(0.0, 0.0, 1.0)); // b
        mVec3fHandle.set(offset, UT_Vector3(0.0, 0.0, float(v[2])));
    }
}


void
TreeVisualizer::addPoint(const openvdb::CoordBBox& bbox,
    const UT_Vector3& color, bool staggered)
{
    if (!staggered) {
        createPoint(bbox, color);
    } else {
        openvdb::Vec3d pos = openvdb::Vec3d(0.5*(bbox.min().x()+bbox.max().x()),
                                            0.5*(bbox.min().y()+bbox.max().y()),
                                            0.5*(bbox.min().z()+bbox.max().z()));
        pos[0] -= 0.5; // -x
        GA_Offset offset = createPoint(pos);
        if (mCdHandle.isValid()) mCdHandle.set(offset, color);

        pos[0] += 0.5;
        pos[1] -= 0.5; // -y
        offset = createPoint(pos);
        if (mCdHandle.isValid()) mCdHandle.set(offset, color);

        pos[1] += 0.5;
        pos[2] -= 0.5; // -z
        offset = createPoint(pos);
        if (mCdHandle.isValid()) mCdHandle.set(offset, color);
    }
}


void
TreeVisualizer::addBox(const openvdb::CoordBBox& bbox,
    const UT_Vector3& color, bool solid)
{
    createBox(*mGeo, *mXform, bbox, mParms.addColor ? &color : nullptr, solid);
}


////////////////////////////////////////


#if HAVE_SURFACING_PARM

class GridSurfacer
{
public:
    GridSurfacer(GU_Detail& geo, float iso = 0.0, float adaptivityThreshold = 0.0,
        bool generateNormals = false, hvdb::Interrupter* interrupter = nullptr);

    template<typename GridType>
    void operator()(const GridType&);

private:
    bool wasInterrupted(int percent = -1) const {
        return mInterrupter && mInterrupter->wasInterrupted(percent);
    }

    GU_Detail* mGeo;
    const float mIso, mAdaptivityThreshold;
    const bool mGenerateNormals;
    hvdb::Interrupter* mInterrupter;
};


GridSurfacer::GridSurfacer(GU_Detail& geo, float iso,
    float adaptivityThreshold, bool generateNormals, hvdb::Interrupter* interrupter)
    : mGeo(&geo)
    , mIso(iso)
    , mAdaptivityThreshold(adaptivityThreshold)
    , mGenerateNormals(generateNormals)
    , mInterrupter(interrupter)
{
}


template<typename GridType>
void
GridSurfacer::operator()(const GridType& grid)
{
    using TreeType = typename GridType::TreeType;
    using LeafNodeType = typename TreeType::LeafNodeType;
    openvdb::CoordBBox bbox;

    // Gets min & max and checks if the grid is empty
    if (grid.tree().evalLeafBoundingBox(bbox)) {

        openvdb::Coord dim(bbox.max() - bbox.min());

        GU_Detail tmpGeo;

        GU_Surfacer surfacer(tmpGeo,
            UT_Vector3(float(bbox.min().x()), float(bbox.min().y()), float(bbox.min().z())),
            UT_Vector3(float(dim[0]), float(dim[1]), float(dim[2])),
            dim[0], dim[1], dim[2], mGenerateNormals);

        typename GridType::ConstAccessor accessor = grid.getConstAccessor();

        openvdb::Coord xyz;
        fpreal density[8];

        // for each leaf..
        for (typename TreeType::LeafCIter iter = grid.tree().cbeginLeaf(); iter; iter.next()) {

            if (wasInterrupted()) break;

            bool isLess = false, isMore = false;

            // for each active voxel..
            typename LeafNodeType::ValueOnCIter it = iter.getLeaf()->cbeginValueOn();
            for ( ; it; ++it) {
                xyz = it.getCoord();

                // Sample values at each corner of the voxel
                for (unsigned int d = 0; d < 8; ++d) {

                    openvdb::Coord valueCoord(
                        xyz.x() +  (d & 1),
                        xyz.y() + ((d & 2) >> 1),
                        xyz.z() + ((d & 4) >> 2));

                    // Houdini uses the inverse sign convention for level sets!
                    density[d] = mIso - float(accessor.getValue(valueCoord));
                    density[d] <= 0.0f ? isLess = true : isMore = true;
                }

                // If there is a crossing, surface this voxel
                if (isLess && isMore) {
                    surfacer.addCell(
                        xyz.x() - bbox.min().x(),
                        xyz.y() - bbox.min().y(),
                        xyz.z() - bbox.min().z(),
                        density, 0);
                }
            } // end active voxel traversal
        } // end leaf traversal

        if (wasInterrupted()) return;

        if (mAdaptivityThreshold > 1e-6) {
            GU_PolyReduceParms parms;
            parms.percentage =
                static_cast<float>(100.0 * (1.0 - std::min(mAdaptivityThreshold, 0.99f)));
            parms.usepercent = 1;
            tmpGeo.polyReduce(parms);
        }

        // world space transform
        for (GA_Iterator it(tmpGeo.getPointRange()); !it.atEnd(); it.advance()) {
            GA_Offset ptOffset = it.getOffset();

            UT_Vector3 pos = tmpGeo.getPos3(ptOffset);
            openvdb::Vec3d vPos(pos.x(), pos.y(), pos.z());
            openvdb::Vec3d wPos = grid.indexToWorld(vPos);

            tmpGeo.setPos3(ptOffset, UT_Vector3(
                static_cast<float>(wPos.x()),
                static_cast<float>(wPos.y()),
                static_cast<float>(wPos.z())));
        }

        mGeo->merge(tmpGeo);
    }
}

#endif // HAVE_SURFACING_PARM


////////////////////////////////////////


OP_ERROR
SOP_OpenVDB_Visualize::Cache::cookVDBSop(OP_Context& context)
{
    try {
        const fpreal time = context.getTime();

        hvdb::Interrupter boss("Visualizing VDBs");

        const GU_Detail* refGdp = inputGeo(0);
        if (refGdp == nullptr) return error();

        // Get the group of grids to visualize.
        const GA_PrimitiveGroup* group = matchGroup(*refGdp, evalStdString("group", time));

        // Evaluate the UI parameters.
        MeshMode meshing = MESH_NONE;
#if HAVE_SURFACING_PARM
        if (evalInt("drawsurface", 0, time)) {
            std::string s = evalStdString("mesher", time);
            meshing = (s == "houdini") ? MESH_HOUDINI : MESH_OPENVDB;
        }
        const double adaptivity = evalFloat("adaptivity", 0, time);
        const double iso = double(evalFloat("isoValue", 0, time));
#endif

        TreeParms treeParms;
        treeParms.internalStyle =
            evalRenderStyle(*this, "drawinternalnodes", "internalstyle", time);
        treeParms.tileStyle = evalRenderStyle(*this, "drawtiles", "tilestyle", time);
        treeParms.leafStyle = evalRenderStyle(*this, "drawleafnodes", "leafstyle", time);
        treeParms.voxelStyle = evalRenderStyle(*this, "drawvoxels", "voxelstyle", time);
        treeParms.addColor = bool(evalInt("addcolor", 0, time));
        treeParms.addValue = bool(evalInt("addvalue", 0, time));
        treeParms.addIndexCoord = bool(evalInt("addindexcoord", 0, time));
        treeParms.useGridName = bool(evalInt("usegridname", 0, time));
        treeParms.ignoreStaggeredVectors = bool(evalInt("ignorestaggered", 0, time));

        const bool drawTree = (treeParms.internalStyle || treeParms.tileStyle
            || treeParms.leafStyle || treeParms.voxelStyle);
        const bool showFrustum = bool(evalInt("previewfrustum", 0, time));
#ifdef DWA_OPENVDB
        const bool showROI = bool(evalInt("previewroi", 0, time));
#else
        const bool showROI = false;
#endif

#if HAVE_SURFACING_PARM
        if (meshing != MESH_NONE) {
            fpreal values[3] = {
                evalFloat("surfaceColor", 0, time),
                evalFloat("surfaceColor", 1, time),
                evalFloat("surfaceColor", 2, time)};

            GA_Defaults color;
            color.set(values, 3);
            gdp->addFloatTuple(GA_ATTRIB_POINT, "Cd", 3, color);
        }

        // mesh using OpenVDB mesher
        if (meshing == MESH_OPENVDB) {
            GU_ConvertParms parms;
            parms.setToType(GEO_PrimTypeCompat::GEOPRIMPOLY);
            parms.myOffset = static_cast<float>(iso);
            parms.preserveGroups = false;
            UT_UniquePtr<GA_PrimitiveGroup> groupDeleter;
            if (!group) {
                parms.primGroup = nullptr;
            } else {
                // parms.primGroup might be modified, so make a copy.
                parms.primGroup = new GA_PrimitiveGroup(*refGdp);
                groupDeleter.reset(parms.primGroup);
                parms.primGroup->copyMembership(*group);
            }
            GU_PrimVDB::convertVDBs(*gdp, *refGdp, parms, adaptivity, /*keep_original*/true);
        }
#endif // HAVE_SURFACING_PARM

        if (!boss.wasInterrupted()
            && (meshing == MESH_HOUDINI || drawTree || showFrustum || showROI))
        {
            // for each VDB primitive...
            for (hvdb::VdbPrimCIterator it(refGdp, group); it; ++it) {

                if (boss.wasInterrupted()) break;

                const GU_PrimVDB *vdb = *it;

#if HAVE_SURFACING_PARM
                // mesh using houdini surfacer
                if (meshing == MESH_HOUDINI) {
                    GridSurfacer surfacer(*gdp, static_cast<float>(iso),
                        static_cast<float>(adaptivity), false, &boss);
                    GEOvdbProcessTypedGridScalar(*vdb, surfacer);
                }
#endif

                // draw tree topology
                if (drawTree) {
                    TreeVisualizer draw(*gdp, treeParms, &boss);

                    if (!GEOvdbProcessTypedGridTopology(*vdb, draw)) {
                        GEOvdbProcessTypedGridPoint(*vdb, draw);
                    }
                }

                if (showFrustum) {
                    UT_Vector3 box_color(0.6f, 0.6f, 0.6f);
                    UT_Vector3 tick_color(0.0f, 0.0f, 0.0f);
                    hvdb::drawFrustum(*gdp, vdb->getGrid().transform(),
                        &box_color, &tick_color, /*shaded*/true);
                }

#ifdef DWA_OPENVDB
                if (showROI) {
                    const openvdb::GridBase& grid = vdb->getConstGrid();
                    openvdb::Vec3IMetadata::ConstPtr metaMin =
                        grid.getMetadata<openvdb::Vec3IMetadata>(
                            openvdb::Name(openvdb_houdini::METADATA_ROI_MIN));
                    openvdb::Vec3IMetadata::ConstPtr metaMax =
                        grid.getMetadata<openvdb::Vec3IMetadata>(
                            openvdb::Name(openvdb_houdini::METADATA_ROI_MAX));

                    if (metaMin && metaMax) {
                        const UT_Vector3 roiColor(1.0, 0.0, 0.0);
                        openvdb::CoordBBox roi(
                            openvdb::Coord(metaMin->value()), openvdb::Coord(metaMax->value()));
                        createBox(*gdp, grid.transform(), roi, &roiColor);
                    }
                }
#endif
            }
        }

        if (boss.wasInterrupted()) {
            addWarning(SOP_MESSAGE, "Process was interrupted");
        }

        boss.end();

    } catch (std::exception& e) {
        addError(SOP_MESSAGE, e.what());
    }

    return error();
}
