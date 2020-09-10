// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file SOP_OpenVDB_Metadata.cc
///
/// @author FX R&D OpenVDB team

#include <houdini_utils/ParmFactory.h>
#include <openvdb_houdini/Utils.h>
#include <openvdb_houdini/SOP_NodeVDB.h>
#include <UT/UT_Interrupt.h>
#include <UT/UT_Version.h>
#include <stdexcept>
#include <string>
#include <vector>


namespace hvdb = openvdb_houdini;
namespace hutil = houdini_utils;


class SOP_OpenVDB_Metadata: public hvdb::SOP_NodeVDB
{
public:
    SOP_OpenVDB_Metadata(OP_Network*, const char* name, OP_Operator*);
    ~SOP_OpenVDB_Metadata() override {}

    static OP_Node* factory(OP_Network*, const char* name, OP_Operator*);

    class Cache: public SOP_VDBCacheOptions { OP_ERROR cookVDBSop(OP_Context&) override; };

protected:
    bool updateParmsFlags() override;
};


void
newSopOperator(OP_OperatorTable* table)
{
    if (table == nullptr) return;

    hutil::ParmList parms;

    parms.add(hutil::ParmFactory(PRM_STRING, "group", "Group")
        .setTooltip("Specify a subset of the input VDBs to be modified.")
        .setChoiceList(&hutil::PrimGroupMenuInput1)
        .setDocumentation(
            "A subset of the input VDBs to be modified"
            " (see [specifying volumes|/model/volumes#group])"));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "setname", "")
        .setTypeExtended(PRM_TYPE_TOGGLE_JOIN));
    parms.add(hutil::ParmFactory(PRM_STRING, "name", "Name")
        .setTooltip("The name of the VDB"));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "setclass", "")
        .setTypeExtended(PRM_TYPE_TOGGLE_JOIN));
    {
        std::vector<std::string> items;
        for (int n = 0; n < openvdb::NUM_GRID_CLASSES; ++n) {
            openvdb::GridClass gridclass = static_cast<openvdb::GridClass>(n);
            items.push_back(openvdb::GridBase::gridClassToString(gridclass));
            items.push_back(openvdb::GridBase::gridClassToMenuName(gridclass));
        }
        parms.add(
            hutil::ParmFactory(PRM_STRING, "class", "Class")
            .setChoiceListItems(PRM_CHOICELIST_SINGLE, items)
            .setTooltip("Specify how voxel values should be interpreted.")
            .setDocumentation("\
How voxel values should be interpreted\n\
\n\
Fog Volume:\n\
    The volume represents a density field.  Values should be positive,\n\
    with zero representing empty regions.\n\
Level Set:\n\
    The volume is treated as a narrow-band signed distance field level set.\n\
    The voxels within a certain distance&mdash;the \"narrow band width\"&mdash;of\n\
    an isosurface are expected to define positive (exterior) and negative (interior)\n\
    distances to the surface.  Outside the narrow band, the distance value\n\
    is constant and equal to the band width.\n\
Staggered Vector Field:\n\
    If the volume is vector-valued, the _x_, _y_ and _z_ vector components\n\
    are to be treated as lying on the respective faces of voxels,\n\
    not at their centers.\n\
Other:\n\
    No special meaning is assigned to the volume's data.\n"));
    }

    /// @todo Do we really need to expose this?
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "setcreator", "")
        .setTypeExtended(PRM_TYPE_TOGGLE_JOIN));
    parms.add(hutil::ParmFactory(PRM_STRING, "creator", "Creator")
        .setTooltip("Who (or what node) created the VDB"));

    /// @todo Currently, no SOP pays attention to this setting.
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "setworld", "")
        .setTypeExtended(PRM_TYPE_TOGGLE_JOIN));
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "world", "Transform Values")
        .setDefault(PRMzeroDefaults)
        .setTooltip(
            "For vector-valued VDBs, specify whether voxel values\n"
            "are in world space and should be affected by transforms\n"
            "or in local space and should not be transformed."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "setvectype", "")
        .setTypeExtended(PRM_TYPE_TOGGLE_JOIN));
    {
        std::string help =
            "For vector-valued VDBs, specify an interpretation of the vectors"
            " that determines how they are affected by transforms.\n";
        std::vector<std::string> items;
        for (int n = 0; n < openvdb::NUM_VEC_TYPES; ++n) {
            const auto vectype = static_cast<openvdb::VecType>(n);
            items.push_back(openvdb::GridBase::vecTypeToString(vectype));
            items.push_back(openvdb::GridBase::vecTypeExamples(vectype));
            help += "\n" + openvdb::GridBase::vecTypeExamples(vectype) + "\n    "
                + openvdb::GridBase::vecTypeDescription(vectype) + ".";
        }
        parms.add(hutil::ParmFactory(PRM_STRING, "vectype", "Vector Type")
            .setChoiceListItems(PRM_CHOICELIST_SINGLE, items)
            .setTooltip(::strdup(help.c_str())));
    }

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "setfloat16", "")
        .setTypeExtended(PRM_TYPE_TOGGLE_JOIN));
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "float16", "Write 16-Bit Floats")
        .setDefault(PRMzeroDefaults)
        .setTooltip(
            "When saving the VDB to a file, write floating-point\n"
            "scalar or vector voxel values as 16-bit half floats."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "syncattrs", "Transfer Metadata to Attributes")
        .setDefault(PRMoneDefaults)
        .setTooltip("Transfer all standard metadata values to intrinsic primitive attributes.")
        .setDocumentation(
            "Transfer all standard metadata values to intrinsic primitive attributes,\n"
            "whether or not any of the above values were changed."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "syncmetadata", "Transfer Attributes to Metadata")
        .setDefault(PRMzeroDefaults)
        .setTooltip("Transfer all standard intrinsic primitive attribute values to metadata.")
        .setDocumentation(
            "Transfer all standard intrinsic primitive attribute values to metadata,\n"
            "whether or not any of the above values were changed."));

    // Register this operator.
    hvdb::OpenVDBOpFactory("VDB Metadata", SOP_OpenVDB_Metadata::factory, parms, *table)
        .setNativeName("")
        .addInput("Input with VDBs")
        .setVerb(SOP_NodeVerb::COOK_INPLACE, []() { return new SOP_OpenVDB_Metadata::Cache; })
        .setDocumentation("\
#icon: COMMON/openvdb\n\
#tags: vdb\n\
\n\
\"\"\"Modify the metadata associated with a VDB volume.\"\"\"\n\
\n\
@overview\n\
\n\
This node allows one to create and edit\n\
[metadata|http://www.openvdb.org/documentation/doxygen/codeExamples.html#sHandlingMetadata]\n\
attached to a VDB volume.\n\
Some standard VDB metadata, such as the\n\
[grid class|http://www.openvdb.org/documentation/doxygen/overview.html#secGrid],\n\
is exposed via intrinsic attributes on the primitive and can be viewed\n\
and in some cases edited either from the [geometry spreadsheet|/ref/panes/geosheet]\n\
or with the [Node:sop/attribcreate] node, but changes to attribute values\n\
made through those means are typically not propagated immediately, if at all,\n\
to a VDB's metadata.\n\
This node provides more direct access to the standard VDB metadata.\n\
\n\
@related\n\
- [OpenVDB Create|Node:sop/DW_OpenVDBCreate]\n\
- [Node:sop/attribcreate]\n\
- [Node:sop/name]\n\
\n\
@examples\n\
\n\
See [openvdb.org|http://www.openvdb.org/download/] for source code\n\
and usage examples.\n");
}


bool
SOP_OpenVDB_Metadata::updateParmsFlags()
{
    bool changed = false;
    const fpreal time = 0; // No point using CHgetTime as that is unstable.

    changed |= enableParm("name",    bool(evalInt("setname", 0, time)));
    changed |= enableParm("class",   bool(evalInt("setclass", 0, time)));
    changed |= enableParm("creator", bool(evalInt("setcreator", 0, time)));
    changed |= enableParm("float16", bool(evalInt("setfloat16", 0, time)));
    changed |= enableParm("world",   bool(evalInt("setworld", 0, time)));
    changed |= enableParm("vectype", bool(evalInt("setvectype", 0, time)));

    return changed;
}


OP_Node*
SOP_OpenVDB_Metadata::factory(OP_Network* net,
    const char* name, OP_Operator* op)
{
    return new SOP_OpenVDB_Metadata(net, name, op);
}


SOP_OpenVDB_Metadata::SOP_OpenVDB_Metadata(OP_Network* net,
    const char* name, OP_Operator* op):
    hvdb::SOP_NodeVDB(net, name, op)
{
}


OP_ERROR
SOP_OpenVDB_Metadata::Cache::cookVDBSop(OP_Context& context)
{
    try {
        const fpreal time = context.getTime();

        // Get UI parameter values.
        const bool
            setname = evalInt("setname", 0, time),
            setclass = evalInt("setclass", 0, time),
            setcreator = evalInt("setcreator", 0, time),
            setfloat16 = evalInt("setfloat16", 0, time),
            setvectype = evalInt("setvectype", 0, time),
            setworld = evalInt("setworld", 0, time),
            syncattrs = evalInt("syncattrs", 0, time),
            syncmetadata = evalInt("syncmetadata", 0, time);

        if (!(setname || setclass || setcreator || setfloat16 || setvectype || setworld
            || syncattrs || syncmetadata))
        {
            return error();
        }

        const bool float16 = (!setfloat16 ? false : evalInt("float16", 0, time));
        const bool world = (!setworld ? false : evalInt("world", 0, time));
        const std::string name = (!setname ? std::string{} : evalStdString("name", time));
        const std::string creator = (!setcreator ? std::string{} : evalStdString("creator", time));
        const openvdb::GridClass gridclass = (!setclass ? openvdb::GRID_UNKNOWN
            : openvdb::GridBase::stringToGridClass(evalStdString("class", time)));
        const openvdb::VecType vectype = (!setvectype ? openvdb::VEC_INVARIANT
            : openvdb::GridBase::stringToVecType(evalStdString("vectype", time)));

        // Get the group of grids to be modified.
        const GA_PrimitiveGroup* group = matchGroup(*gdp, evalStdString("group", time));

        UT_AutoInterrupt progress("Set VDB grid metadata");

        // For each VDB primitive in the given group...
        for (hvdb::VdbPrimIterator it(gdp, group); it; ++it) {
            if (progress.wasInterrupted()) throw std::runtime_error("was interrupted");

            GU_PrimVDB* vdb = *it;

            // No need to make the grid unique, since we're not modifying its voxel data.
            hvdb::Grid& grid = vdb->getGrid();

            // Set various grid metadata items.
            if (setname)    grid.setName(name);
            if (setcreator) grid.setCreator(creator);
            if (setfloat16) grid.setSaveFloatAsHalf(float16);
            if (setvectype) grid.setVectorType(vectype);
            if (setworld)   grid.setIsInWorldSpace(world);
            if (setclass) {
                grid.setGridClass(gridclass);

                // Update viewport visualization options.
                switch (gridclass) {
                    case openvdb::GRID_LEVEL_SET:
                    case openvdb::GRID_FOG_VOLUME:
                    {
                        const GEO_VolumeOptions& visOps = vdb->getVisOptions();
                        vdb->setVisualization(
                            ((gridclass == openvdb::GRID_LEVEL_SET) ?
                                GEO_VOLUMEVIS_ISO : GEO_VOLUMEVIS_SMOKE),
                            visOps.myIso, visOps.myDensity);
                        break;
                    }
                    default: break;
                }
            }

            // Optionally transfer metadata to primitive attributes.
            if (syncattrs) vdb->syncAttrsFromMetadata();
            // Optionally transfer primitive attributes to metadata.
            if (syncmetadata) GU_PrimVDB::createMetadataFromGridAttrs(grid, *vdb, *gdp);
        }
    } catch (std::exception& e) {
        addError(SOP_MESSAGE, e.what());
    }
    return error();
}
