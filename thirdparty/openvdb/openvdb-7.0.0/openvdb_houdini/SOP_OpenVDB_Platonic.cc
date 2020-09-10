// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file SOP_OpenVDB_Platonic.cc
/// @author FX R&D OpenVDB team

#include <houdini_utils/ParmFactory.h>
#include <openvdb_houdini/Utils.h>
#include <openvdb_houdini/SOP_NodeVDB.h>
#include <UT/UT_Interrupt.h>
#include <openvdb/tools/LevelSetSphere.h>
#include <openvdb/tools/LevelSetPlatonic.h>
#include <openvdb/tools/LevelSetUtil.h>

namespace hvdb = openvdb_houdini;
namespace hutil = houdini_utils;


class SOP_OpenVDB_Platonic: public hvdb::SOP_NodeVDB
{
public:
    SOP_OpenVDB_Platonic(OP_Network*, const char* name, OP_Operator*);
    ~SOP_OpenVDB_Platonic() override {}

    static OP_Node* factory(OP_Network*, const char* name, OP_Operator*);

protected:
    OP_ERROR cookVDBSop(OP_Context&) override;
    bool updateParmsFlags() override;
};


void
newSopOperator(OP_OperatorTable* table)
{
    if (table == nullptr) return;

    hutil::ParmList parms;

    // Shapes
    parms.add(hutil::ParmFactory(PRM_ORD, "solidType", "Solid Type")
        .setTooltip("Select a sphere or one of the five platonic solids")
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, {
            "sphere",       "Sphere",
            "tetrahedron",  "Tetrahedron",
            "cube",         "Cube",
            "octahedron",   "Octahedron",
            "dodecahedron", "Dodecahedron",
            "icosahedron",  "Icosahedron"
        }));

    { // Grid Class
        const std::vector<std::string> items{
            openvdb::GridBase::gridClassToString(openvdb::GRID_LEVEL_SET), // token
            openvdb::GridBase::gridClassToMenuName(openvdb::GRID_LEVEL_SET), // label
            "sdf", "Signed Distance Field"
        };

        parms.add(hutil::ParmFactory(PRM_STRING, "gridclass", "Grid Class")
            .setDefault(openvdb::GridBase::gridClassToString(openvdb::GRID_LEVEL_SET))
            .setChoiceListItems(PRM_CHOICELIST_SINGLE, items)
            .setDocumentation("\
The type of volume to generate\n\
\n\
Level Set:\n\
    Generate a narrow-band signed distance field level set, in which\n\
    the values define positive and negative distances to the surface\n\
    of the solid up to a certain band width.\n\
\n\
Signed Distance Field:\n\
    Generate a narrow-band unsigned distance field level set, in which\n\
    the values define positive distances to the surface of the solid\n\
    up to a certain band width.\n"));
    }

    // Radius
    parms.add(hutil::ParmFactory(PRM_FLT_J, "scalarRadius", "Radius/Size")
        .setDefault(PRMoneDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_FREE, 10)
        .setTooltip("The size of the platonic solid or the radius of the sphere"));

    // Center
    parms.add(hutil::ParmFactory(PRM_XYZ_J, "center", "Center")
        .setVectorSize(3)
        .setDefault(PRMzeroDefaults)
        .setTooltip("The world-space center of the level set"));

    // Voxel size
    parms.add(hutil::ParmFactory(PRM_FLT_J, "voxelSize", "Voxel Size")
        .setDefault(0.1f)
        .setRange(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_FREE, 10)
        .setTooltip("The size of a voxel in world units"));

    // Narrow-band half-width
    parms.add(hutil::ParmFactory(PRM_FLT_J, "halfWidth", "Half-Band Width")
        .setDefault(PRMthreeDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 1.0, PRM_RANGE_UI, 10)
        .setTooltip(
            "Half the width of the narrow band in voxel units\n\n"
            "For proper operation, many nodes expect this to be at least three."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "fogVolume", "Convert to Fog Volume")
        .setTooltip("If enabled, generate a fog volume instead of a level set"));

    // Register this operator.
    hvdb::OpenVDBOpFactory("VDB Platonic",
        SOP_OpenVDB_Platonic::factory, parms, *table)
        .setNativeName("")
        .setDocumentation("\
#icon: COMMON/openvdb\n\
#tags: vdb\n\
\n\
\"\"\"Generate a platonic solid as a level set or a fog volume VDB.\"\"\"\n\
\n\
@overview\n\
\n\
This node generates a VDB representing a platonic solid as either a level set or fog volume.\n\
\n\
@related\n\
- [OpenVDB Create|Node:sop/DW_OpenVDBCreate]\n\
- [Node:sop/platonic]\n\
\n\
@examples\n\
\n\
See [openvdb.org|http://www.openvdb.org/download/] for source code\n\
and usage examples.\n");
}


OP_Node*
SOP_OpenVDB_Platonic::factory(OP_Network* net,
    const char* name, OP_Operator* op)
{
    return new SOP_OpenVDB_Platonic(net, name, op);
}


SOP_OpenVDB_Platonic::SOP_OpenVDB_Platonic(OP_Network* net,
    const char* name, OP_Operator* op):
    hvdb::SOP_NodeVDB(net, name, op)
{
}


bool
SOP_OpenVDB_Platonic::updateParmsFlags()
{
    bool changed = false;

    const bool sdfGrid = (evalStdString("gridclass", 0) == "sdf");

    changed |= enableParm("halfWidth", sdfGrid);

    return changed;
}


OP_ERROR
SOP_OpenVDB_Platonic::cookVDBSop(OP_Context& context)
{
    try {
        hutil::ScopedInputLock lock(*this, context);
        gdp->clearAndDestroy();

        const fpreal time = context.getTime();

        hvdb::Interrupter boss("Creating VDB platonic solid");

        // Read GUI parameters and generate narrow-band level set of sphere
        const float radius = static_cast<float>(evalFloat("scalarRadius", 0, time));
        const openvdb::Vec3f center = evalVec3f("center", time);
        const float voxelSize = static_cast<float>(evalFloat("voxelSize", 0, time));
        const float halfWidth = ((evalStdString("gridclass", 0) != "sdf") ?
            3.0f : static_cast<float>(evalFloat("halfWidth", 0, time)));

        openvdb::FloatGrid::Ptr grid;
        switch (evalInt("solidType", 0, time)) {
        case 0://Sphere
            grid = openvdb::tools::createLevelSetSphere<openvdb::FloatGrid, hvdb::Interrupter>
                (radius, center, voxelSize, halfWidth, &boss);
            break;
        case 1:// Tetrahedraon
            grid = openvdb::tools::createLevelSetTetrahedron<openvdb::FloatGrid, hvdb::Interrupter>
                (radius, center, voxelSize, halfWidth, &boss);
            break;
        case 2:// Cube
            grid = openvdb::tools::createLevelSetCube<openvdb::FloatGrid, hvdb::Interrupter>
                (radius, center, voxelSize, halfWidth, &boss);
            break;
        case 3:// Octahedron
            grid = openvdb::tools::createLevelSetOctahedron<openvdb::FloatGrid, hvdb::Interrupter>
                (radius, center, voxelSize, halfWidth, &boss);
            break;
        case 4:// Dodecahedron
            grid = openvdb::tools::createLevelSetDodecahedron<openvdb::FloatGrid, hvdb::Interrupter>
                (radius, center, voxelSize, halfWidth, &boss);
            break;
        case 5:// Icosahedron
            grid = openvdb::tools::createLevelSetIcosahedron<openvdb::FloatGrid, hvdb::Interrupter>
                (radius, center, voxelSize, halfWidth, &boss);
            break;
        default:
            addError(SOP_MESSAGE, "Illegal shape.");
            return error();
        }

        // Fog volume conversion
        if (bool(evalInt("fogVolume", 0, time))) {
            openvdb::tools::sdfToFogVolume(*grid);
        }

        // Store the grid in a new VDB primitive and add the primitive to the output gdp
        hvdb::createVdbPrimitive(*gdp, grid, "PlatonicSolid");

    } catch (std::exception& e) {
        addError(SOP_MESSAGE, e.what());
    }
    return error();
}
