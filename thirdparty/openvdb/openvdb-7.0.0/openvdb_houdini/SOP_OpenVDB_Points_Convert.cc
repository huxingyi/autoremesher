// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

/// @file SOP_OpenVDB_Points_Convert.cc
///
/// @authors Dan Bailey, Nick Avramoussis, James Bird
///
/// @brief Converts points to OpenVDB points.

#include <openvdb/openvdb.h>
#include <openvdb/points/PointDataGrid.h>
#include <openvdb/points/PointCount.h>
#include <openvdb/points/PointMask.h>

#include <openvdb_houdini/Utils.h>
#include <openvdb_houdini/PointUtils.h>
#include <openvdb_houdini/SOP_NodeVDB.h>

#include <houdini_utils/geometry.h>
#include <houdini_utils/ParmFactory.h>

#include <CH/CH_Manager.h> // for CHgetEvalTime
#include <GU/GU_DetailHandle.h>
#include <GU/GU_PackedContext.h>
#include <GU/GU_PackedGeometry.h>
#include <GU/GU_PackedFragment.h>
#include <GU/GU_PrimPacked.h>
#include <UT/UT_Version.h>

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>



using namespace openvdb;
using namespace openvdb::points;
using namespace openvdb::math;


namespace hvdb = openvdb_houdini;
namespace hutil = houdini_utils;

namespace {

enum TRANSFORM_MODE {
    TRANSFORM_TARGET_POINTS = 0,
    TRANSFORM_VOXEL_SIZE,
    TRANSFORM_REF_GRID
};

enum CONVERSION_MODE {
    MODE_CONVERT_TO_VDB = 0,
    MODE_CONVERT_FROM_VDB,
    MODE_GENERATE_MASK,
    MODE_COUNT_POINTS,
};

enum OUTPUT_NAME_MODE {
    NAME_KEEP = 0,
    NAME_APPEND,
    NAME_REPLACE
};

} // anonymous namespace


////////////////////////////////////////


class SOP_OpenVDB_Points_Convert: public hvdb::SOP_NodeVDB
{
public:
    SOP_OpenVDB_Points_Convert(OP_Network*, const char* name, OP_Operator*);
    ~SOP_OpenVDB_Points_Convert() override = default;

    static OP_Node* factory(OP_Network*, const char* name, OP_Operator*);

    int isRefInput(unsigned i) const override { return (i == 1); }

    static OUTPUT_NAME_MODE getOutputNameMode(const std::string& modeName);

    class Cache: public SOP_VDBCacheOptions { OP_ERROR cookVDBSop(OP_Context&) override; };

protected:
    bool updateParmsFlags() override;
};


////////////////////////////////////////


namespace {

inline int
lookupAttrInput(const PRM_SpareData* spare)
{
    const char  *istring;
    if (!spare) return 0;
    istring = spare->getValue("sop_input");
    return istring ? atoi(istring) : 0;
}

inline void
sopBuildAttrMenu(void* data, PRM_Name* menuEntries, int themenusize,
    const PRM_SpareData* spare, const PRM_Parm*)
{
    if (data == nullptr || menuEntries == nullptr || spare == nullptr) return;

    SOP_Node* sop = CAST_SOPNODE(static_cast<OP_Node*>(data));

    if (sop == nullptr) {
        // terminate and quit
        menuEntries[0].setToken(0);
        menuEntries[0].setLabel(0);
        return;
    }


    int inputIndex = lookupAttrInput(spare);
    const GU_Detail* gdp = sop->getInputLastGeo(inputIndex, CHgetEvalTime());

    size_t menuIdx = 0, menuEnd(themenusize - 2);

    // null object
    menuEntries[menuIdx].setToken("0");
    menuEntries[menuIdx++].setLabel("- no attribute selected -");

    if (gdp) {

        // point attribute names
        auto iter = gdp->pointAttribs().begin(GA_SCOPE_PUBLIC);

        if (!iter.atEnd() && menuIdx != menuEnd) {

            if (menuIdx > 0) {
                menuEntries[menuIdx].setToken(PRM_Name::mySeparator);
                menuEntries[menuIdx++].setLabel(PRM_Name::mySeparator);
            }

            for (; !iter.atEnd() && menuIdx != menuEnd; ++iter) {

                const char* str = (*iter)->getName();

                if (str) {
                    Name name = str;
                    if (name != "P") {
                        menuEntries[menuIdx].setToken(name.c_str());
                        menuEntries[menuIdx++].setLabel(name.c_str());
                    }
                }
            }
        }
    }

    // terminator
    menuEntries[menuIdx].setToken(0);
    menuEntries[menuIdx].setLabel(0);
}

const PRM_ChoiceList PrimAttrMenu(
    PRM_ChoiceListType(PRM_CHOICELIST_REPLACE), sopBuildAttrMenu);

} // unnamed namespace


////////////////////////////////////////


// Build UI and register this operator.
void
newSopOperator(OP_OperatorTable* table)
{
    openvdb::initialize();

    // Force the building of the unit vector codec as it isn't threadsafe.
    const uint16_t data = 0;
    auto SYS_UNUSED_VAR_ATTRIB ignoredResult = openvdb::math::QuantizedUnitVec::unpack(data);

    if (table == nullptr) return;

    hutil::ParmList parms;

    parms.add(hutil::ParmFactory(PRM_ORD, "conversion", "Conversion")
        .setDefault(PRMzeroDefaults)
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, {
            "vdb", "Pack Points into VDB Points",
            "hdk", "Extract Points from VDB Points",
            "mask", "Generate Mask from VDB Points",
            "count", "Points/Voxel Count from VDB Points"
        })
        .setTooltip("The conversion method for the expected input types.")
        .setDocumentation(
            "Whether to pack points into a VDB Points primitive"
            " or to extract points from such a primitive or to generate"
            " a mask from the primitive or to count the number of"
            " points-per-voxel in the primitive"));

    parms.add(hutil::ParmFactory(PRM_STRING, "group", "Group")
        .setChoiceList(&hutil::PrimGroupMenu)
        .setTooltip("Specify a subset of the input point data grids to convert.")
        .setDocumentation(
            "A subset of the input VDB Points primitives to be processed"
            " (see [specifying volumes|/model/volumes#group])"));

    parms.add(hutil::ParmFactory(PRM_STRING, "vdbpointsgroup", "VDB Points Group")
        .setChoiceList(&hvdb::VDBPointsGroupMenuInput1)
        .setTooltip("Specify VDB Points Groups to use as an input.")
        .setDocumentation(
            "The point group inside the VDB Points primitive to extract\n\n"
            "This may be a normal point group that was collapsed into the"
            " VDB Points primitive when it was created, or a new group created"
            " with the [OpenVDB Points Group node|Node:sop/DW_OpenVDBPointsGroup]."));

    //  point grid name
    parms.add(hutil::ParmFactory(PRM_STRING, "name", "VDB Name")
        .setDefault("points")
        .setTooltip("The name of the VDB Points primitive to be created")
        .setDocumentation(nullptr));

    // VDB points grid name
    parms.add(hutil::ParmFactory(PRM_STRING, "outputname", "Output Name")
        .setDefault("keep")
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, {
            "keep",     "Keep Original Name",
            "append",   "Add Suffix",
            "replace",  "Custom Name",
        })
        .setTooltip("Output VDB naming scheme")
        .setDocumentation(
            "Give the output VDB Points the same name as the input VDB,"
            " or add a suffix to the input name, or use a custom name."));

    parms.add(hutil::ParmFactory(PRM_STRING, "countname", "VDB Name")
        .setDefault("count")
        .setTooltip("The name of the VDB count primitive to be created")
        .setDocumentation(nullptr));

    parms.add(hutil::ParmFactory(PRM_STRING, "maskname", "VDB Name")
        .setDefault("mask")
        .setTooltip("The name of the VDB mask primitive to be created")
        .setDocumentation("The name of the VDB primitive to be created"));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "keep", "Keep Original Geometry")
        .setDefault(PRMzeroDefaults)
        .setTooltip("The incoming geometry will not be deleted if this is set.")
        .setDocumentation("The incoming geometry will not be deleted if this is set."));

    // Transform
    parms.add(hutil::ParmFactory(PRM_ORD, "transform", "Define Transform")
        .setDefault(PRMzeroDefaults)
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, {
            "targetpointspervoxel",  "Using Target Points Per Voxel",
            "voxelsizeonly",         "Using Voxel Size Only",
            "userefvdb",             "To Match Reference VDB"
        })
        .setTooltip(
            "Specify how to construct the PointDataGrid transform. If\n"
            "an optional transform input is provided for the first two\n"
            "options, the rotate and translate components are preserved.\n"
            "Using Target Points Per Voxel:\n"
            "    Automatically calculates a voxel size based off the input\n"
            "    point set and a target amount of points per voxel.\n"
            "Using Voxel Size Only:\n"
            "    Explicitly sets a voxel size.\n"
            "To Match Reference VDB:\n"
            "    Uses the complete transform provided from the second input.")
        .setDocumentation("\
How to construct the VDB Points primitive's transform\n\n\
An important consideration is how big to make the grid cells\n\
that contain the points.  Too large and there are too many points\n\
per cell and little optimization occurs.  Too small and the cost\n\
of the cells outweighs the points.\n\
\n\
Using Target Points Per Voxel:\n\
    Automatically calculate a voxel size so that the given number\n\
    of points ends up in each voxel.  This will assume uniform\n\
    distribution of points.\n\
    \n\
    If an optional transform input is provided, use its rotation\n\
    and translation.\n\
Using Voxel Size Only:\n\
    Provide an explicit voxel size, and if an optional transform input\n\
    is provided, use its rotation and translation.\n\
To Match Reference VDB:\n\
    Use the complete transform provided from the second input.\n"));

    parms.add(hutil::ParmFactory(PRM_FLT_J, "voxelsize", "Voxel Size")
        .setDefault(PRMpointOneDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 1e-5, PRM_RANGE_UI, 5)
        .setTooltip("The desired voxel size of the new VDB Points grid"));

    parms.add(hutil::ParmFactory(PRM_INT_J, "pointspervoxel", "Points per Voxel")
        .setDefault(8)
        .setRange(PRM_RANGE_RESTRICTED, 1, PRM_RANGE_UI, 16)
        .setTooltip(
            "The number of points per voxel to use as the target for "
            "automatic voxel size computation"));

    // Group name (Transform reference)
    parms.add(hutil::ParmFactory(PRM_STRING, "refvdb", "Reference VDB")
        .setChoiceList(&hutil::PrimGroupMenu)
        .setSpareData(&SOP_Node::theSecondInput)
        .setTooltip("References the first/selected grid's transform.")
        .setDocumentation(
            "Which VDB in the second input to use as the reference for the transform\n\n"
            "If this is not set, use the first VDB found."));

    //////////

    // Point attribute transfer

    parms.add(hutil::ParmFactory(PRM_ORD, "poscompression", "Position Compression")
        .setDefault(PRMoneDefaults)
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, {
            "none",   "None",
            "int16",  "16-bit Fixed Point",
            "int8",   "8-bit Fixed Point"
        })
        .setTooltip("The position attribute compression setting.")
        .setDocumentation(
            "The position can be stored relative to the center of the voxel.\n"
            "This means it does not require the full 32-bit float representation,\n"
            "but can be quantized to a smaller fixed-point value."));

    parms.add(hutil::ParmFactory(PRM_HEADING, "transferheading", "Attribute Transfer"));

     // Mode. Either convert all or convert specifc attributes
    parms.add(hutil::ParmFactory(PRM_ORD, "mode", "Mode")
        .setDefault(PRMzeroDefaults)
        .setTooltip("Whether to transfer only specific attributes or all attributes found")
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, {
            "all", "All Attributes",
            "spec", "Specific Attributes"
        }));


    hutil::ParmList attrParms;

    // Attribute name
    attrParms.add(hutil::ParmFactory(PRM_STRING, "attribute#", "Attribute")
        .setChoiceList(&PrimAttrMenu)
        .setSpareData(&SOP_Node::theFirstInput)
        .setTooltip("Select a point attribute to transfer.\n\n"
            "Supports integer and floating-point attributes of "
            "arbitrary precisions and tuple sizes."));

    {
        char const * const items[] = {
            "none", "None",
            "truncate", "16-bit Truncate",
            UnitVecCodec::name(), "Unit Vector",
            FixedPointCodec<true, UnitRange>::name(), "8-bit Unit",
            FixedPointCodec<false, UnitRange>::name(), "16-bit Unit",
            nullptr
        };

        attrParms.add(hutil::ParmFactory(PRM_ORD, "valuecompression#", "Value Compression")
            .setDefault(PRMzeroDefaults)
            .setChoiceListItems(PRM_CHOICELIST_SINGLE, items)
            .setTooltip("Value compression to use for specific attributes.")
            .setDocumentation("\
How to compress attribute values\n\
\n\
None:\n\
    Values are stored with their full precision.\n\
\n\
16-bit Truncate:\n\
    Values are stored at half precision, truncating lower-order bits.\n\
\n\
Unit Vector:\n\
    Values are treated as unit vectors, so that if two components\n\
    are known, the third is implied and need not be stored.\n\
\n\
8-bit Unit:\n\
    Values are treated as lying in the 0..1 range and are quantized to 8 bits.\n\
\n\
16-bit Unit:\n\
    Values are treated as lying in the 0..1 range and are quantized to 16 bits.\n"));
    }

    attrParms.add(hutil::ParmFactory(PRM_TOGGLE, "blosccompression#", "Blosc Compression")
        .setInvisible() // this parm is now a no-op as in-memory blosc compression is deprecated
        .setDefault(PRMzeroDefaults));

    // Add multi parm
    parms.add(hutil::ParmFactory(PRM_MULTITYPE_LIST, "attrList", "Point Attributes")
        .setTooltip("Transfer point attributes to each voxel in the level set's narrow band")
        .setMultiparms(attrParms)
        .setDefault(PRMzeroDefaults));

    parms.add(hutil::ParmFactory(PRM_LABEL, "attributespacer", ""));

    {
        char const * const items[] = {
            "none", "None",
            UnitVecCodec::name(), "Unit Vector",
            "truncate", "16-bit Truncate",
            nullptr
    };

    parms.add(hutil::ParmFactory(PRM_ORD, "normalcompression", "Normal Compression")
        .setDefault(PRMzeroDefaults)
        .setTooltip("All normal attributes will use this compression codec.")
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, items));
    }

    {
        char const * const items[] = {
            "none", "None",
            FixedPointCodec<false, UnitRange>::name(), "16-bit Unit",
            FixedPointCodec<true, UnitRange>::name(), "8-bit Unit",
            "truncate", "16-bit Truncate",
            nullptr
    };

    parms.add(hutil::ParmFactory(PRM_ORD, "colorcompression", "Color Compression")
        .setDefault(PRMzeroDefaults)
        .setTooltip("All color attributes will use this compression codec.")
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, items));
    }


    hutil::ParmList obsoleteParms;
    obsoleteParms.add(hutil::ParmFactory(PRM_HEADING, "transferHeading", "Attribute Transfer"));


    //////////
    // Register this operator.

    hvdb::OpenVDBOpFactory("Convert VDB Points",
        SOP_OpenVDB_Points_Convert::factory, parms, *table)
#ifndef SESI_OPENVDB
        .setInternalName("DW_OpenVDBPointsConvert")
#endif
        .addInput("Points to Convert")
        .addOptionalInput("Optional Reference VDB (for transform)")
        .setObsoleteParms(obsoleteParms)
        .setVerb(SOP_NodeVerb::COOK_GENERIC,
            []() { return new SOP_OpenVDB_Points_Convert::Cache; })
        .setDocumentation("\
#icon: COMMON/openvdb\n\
#tags: vdb\n\
\n\
\"\"\"Convert a point cloud into a VDB Points primitive, or vice versa.\"\"\"\n\
\n\
@overview\n\
\n\
This node converts an unstructured cloud of points to and from a single\n\
[VDB Points|http://www.openvdb.org/documentation/doxygen/points.html] primitive.\n\
The resulting primitive will reorder the points to place spatially\n\
close points close together.\n\
It is then able to efficiently unpack regions of interest within that primitive.\n\
The [OpenVDB Points Group node|Node:sop/DW_OpenVDBPointsGroup] can be used\n\
to create regions of interest.\n\
\n\
Because nearby points often have similar data, there is the possibility\n\
of aggressively compressing attribute data to minimize data size.\n\
\n\
@related\n\
- [OpenVDB Points Group|Node:sop/DW_OpenVDBPointsGroup]\n\
\n\
@examples\n\
\n\
See [openvdb.org|http://www.openvdb.org/download/] for source code\n\
and usage examples.\n");
}


////////////////////////////////////////


OP_Node*
SOP_OpenVDB_Points_Convert::factory(OP_Network* net,
    const char* name, OP_Operator* op)
{
    return new SOP_OpenVDB_Points_Convert(net, name, op);
}


SOP_OpenVDB_Points_Convert::SOP_OpenVDB_Points_Convert(OP_Network* net,
    const char* name, OP_Operator* op)
    : hvdb::SOP_NodeVDB(net, name, op)
{
}


////////////////////////////////////////


// Enable or disable parameters in the UI.
bool
SOP_OpenVDB_Points_Convert::updateParmsFlags()
{
    bool changed = false;

    const bool toVdbPoints = evalInt("conversion", 0, 0) == 0;
    const bool toMask = evalInt("conversion", 0, 0) == 2;
    const bool toCount = evalInt("conversion", 0, 0) == 3;
    const bool convertAll = evalInt("mode", 0, 0) == 0;
    const auto transform = evalInt("transform", 0, 0);

    changed |= enableParm("group", !toVdbPoints);
    changed |= setVisibleState("group", !toVdbPoints);

    changed |= enableParm("vdbpointsgroup", !toVdbPoints);
    changed |= setVisibleState("vdbpointsgroup", !toVdbPoints);

    changed |= enableParm("name", toVdbPoints);
    changed |= setVisibleState("name", toVdbPoints);

    changed |= enableParm("outputname", toCount || toMask);
    changed |= setVisibleState("outputname", toCount || toMask);

    const bool useCustomName =
        (getOutputNameMode(evalStdString("outputname", 0.0)) != NAME_KEEP);

    changed |= enableParm("countname", useCustomName && toCount);
    changed |= setVisibleState("countname", toCount);

    changed |= enableParm("maskname", useCustomName && toMask);
    changed |= setVisibleState("maskname", toMask);

    changed |= enableParm("keep", !toVdbPoints);
    changed |= setVisibleState("keep", !toVdbPoints);

    const int refexists = (this->nInputs() == 2);

    changed |= enableParm("transform", toVdbPoints);
    changed |= setVisibleState("transform", toVdbPoints);

    changed |= enableParm("refvdb", refexists);
    changed |= setVisibleState("refvdb", toVdbPoints);

    changed |= enableParm("voxelsize", toVdbPoints && transform == TRANSFORM_VOXEL_SIZE);
    changed |= setVisibleState("voxelsize", toVdbPoints && transform == TRANSFORM_VOXEL_SIZE);

    changed |= enableParm("pointspervoxel", toVdbPoints && transform == TRANSFORM_TARGET_POINTS);
    changed |= setVisibleState("pointspervoxel",
        toVdbPoints && transform == TRANSFORM_TARGET_POINTS);

    changed |= setVisibleState("transferheading", toVdbPoints);

    changed |= enableParm("poscompression", toVdbPoints);
    changed |= setVisibleState("poscompression", toVdbPoints);

    changed |= enableParm("mode", toVdbPoints);
    changed |= setVisibleState("mode", toVdbPoints);

    changed |= enableParm("attrList", toVdbPoints && !convertAll);
    changed |= setVisibleState("attrList", toVdbPoints && !convertAll);

    changed |= enableParm("normalcompression", toVdbPoints && convertAll);
    changed |= setVisibleState("normalcompression", toVdbPoints && convertAll);

    changed |= enableParm("colorcompression", toVdbPoints && convertAll);
    changed |= setVisibleState("colorcompression", toVdbPoints && convertAll);

    return changed;
}


////////////////////////////////////////


OUTPUT_NAME_MODE
SOP_OpenVDB_Points_Convert::getOutputNameMode(const std::string& modeName)
{
    if (modeName == "append") return NAME_APPEND;
    if (modeName == "replace") return NAME_REPLACE;
    return NAME_KEEP;
}


////////////////////////////////////////


OP_ERROR
SOP_OpenVDB_Points_Convert::Cache::cookVDBSop(OP_Context& context)
{
    try {
        hvdb::Interrupter boss{"Converting points"};

        hvdb::WarnFunc warnFunction = [this](const std::string& msg) {
            this->addWarning(SOP_MESSAGE, msg.c_str());
        };

        const fpreal time = context.getTime();

        const int conversion = static_cast<int>(evalInt("conversion", 0, time));
        const bool keepOriginalGeo = evalInt("keep", 0, time) == 1;

        const GA_PrimitiveGroup* group = (conversion != MODE_CONVERT_TO_VDB) ?
            matchGroup(*inputGeo(0), evalStdString("group", time)) : nullptr;

        // Extract VDB Point groups to filter

        const std::string pointsGroup = evalStdString("vdbpointsgroup", time);

        std::vector<std::string> includeGroups;
        std::vector<std::string> excludeGroups;
        if (conversion != MODE_CONVERT_TO_VDB) {
            openvdb::points::AttributeSet::Descriptor::parseNames(
                includeGroups, excludeGroups, pointsGroup);
        }

        // Optionally copy transform parameters from reference grid (if not converting from VDB).

        Transform::Ptr transform;

        if (conversion != MODE_CONVERT_FROM_VDB) {
            if (const GU_Detail* refGeo = inputGeo(1, context)) {

                const GA_PrimitiveGroup* refGroup =
                    matchGroup(*refGeo, evalStdString("refvdb", time));

                hvdb::VdbPrimCIterator it(refGeo, refGroup);
                const hvdb::GU_PrimVDB* refPrim = *it;

                if (!refPrim) {
                    addError(SOP_MESSAGE, "Second input has no VDB primitives.");
                    return error();
                }

                transform = refPrim->getGrid().transform().copy();
            }
        }

        // handle to VDB, count and mask conversion options

        if (conversion != MODE_CONVERT_TO_VDB) {

            UT_Array<GEO_Primitive*> primsToDelete;
            primsToDelete.clear();

            if (keepOriginalGeo) {
                // Duplicate primary (left) input geometry

                if (const auto* input0 = inputGeo(0)) {
                    gdp->replaceWith(*input0);
                } else {
                    gdp->stashAll();
                }

                // Extract VDB primitives to delete

                for (hvdb::VdbPrimIterator vdbIt(gdp, group); vdbIt; ++vdbIt) {
                    openvdb::GridBase::ConstPtr gridBase = vdbIt->getConstGridPtr();
                    PointDataGrid::ConstPtr points =
                        openvdb::GridBase::constGrid<PointDataGrid>(gridBase);
                    if (!points)    continue;

                    primsToDelete.append(*vdbIt);
                }
            } else {
                gdp->stashAll();
            }

            // Extract point grids and names for conversion

            std::vector<PointDataGrid::ConstPtr> pointGrids;
            std::vector<std::string> pointNames;

            const GU_Detail* sourceGdp = keepOriginalGeo ? gdp : inputGeo(0, context);

            for (hvdb::VdbPrimCIterator vdbIt(sourceGdp, group); vdbIt; ++vdbIt) {
                openvdb::GridBase::ConstPtr gridBase = vdbIt->getConstGridPtr();
                PointDataGrid::ConstPtr points =
                    openvdb::GridBase::constGrid<PointDataGrid>(gridBase);
                if (!points)    continue;

                pointGrids.push_back(points);

                if (conversion != MODE_CONVERT_FROM_VDB) {
                    const std::string gridName = vdbIt.getPrimitiveName().toStdString();

                    pointNames.push_back(gridName);
                }
            }

            if (keepOriginalGeo) {
                gdp->deletePrimitives(primsToDelete, true);
            }

            if (conversion == MODE_CONVERT_FROM_VDB) {

                // passing an empty vector of attribute names implies that
                // all attributes should be converted
                const std::vector<std::string> emptyNameVector;

                // if all point data is being converted, sequentially pre-fetch any out-of-core
                // data for faster performance when using delayed-loading

                const bool allData =    emptyNameVector.empty() &&
                                        includeGroups.empty() &&
                                        excludeGroups.empty();

                for (PointDataGrid::ConstPtr grid : pointGrids) {

                    GU_Detail geo;

                    // if all the data is being loaded, prefetch it for faster load performance

                    if (allData) {
                        prefetch(grid->tree());
                    }

                    // perform conversion

                    hvdb::convertPointDataGridToHoudini(
                        geo, *grid, emptyNameVector, includeGroups, excludeGroups);

                    const MetaMap& metaMap = *grid;

                    hvdb::convertMetadataToHoudini(geo, metaMap, warnFunction);

                    gdp->merge(geo);
                }

                return error();
            }
            else {

                const auto outputName = getOutputNameMode(evalStdString("outputname", time));

                size_t i = 0;

                for (PointDataGrid::ConstPtr grid : pointGrids) {

                    assert(i < pointNames.size());
                    const std::string gridName = pointNames[i++];

                    GU_Detail geo;

                    if (conversion == MODE_GENERATE_MASK) {
                        openvdb::BoolGrid::Ptr maskGrid;
                        auto leaf = grid->tree().cbeginLeaf();
                        if (leaf) {
                            MultiGroupFilter filter(includeGroups, excludeGroups, leaf->attributeSet());
                            if (transform) {
                                maskGrid = openvdb::points::convertPointsToMask(
                                    *grid, *transform, filter);
                            }
                            else {
                                maskGrid = openvdb::points::convertPointsToMask(
                                    *grid, filter);
                            }
                        } else {
                            maskGrid = openvdb::BoolGrid::create();
                        }

                        const std::string customName = evalStdString("maskname", time);

                        std::string vdbName;
                        switch (outputName) {
                            case NAME_KEEP:    vdbName = gridName; break;
                            case NAME_APPEND:  vdbName = gridName + customName; break;
                            case NAME_REPLACE: vdbName = customName; break;
                        }
                        hvdb::createVdbPrimitive(*gdp, maskGrid, vdbName.c_str());
                    }
                    else {
                        openvdb::Int32Grid::Ptr countGrid;
                        auto leaf = grid->tree().cbeginLeaf();
                        if (leaf) {
                            MultiGroupFilter filter(includeGroups, excludeGroups, leaf->attributeSet());
                            if (transform) {
                                countGrid = openvdb::points::pointCountGrid(
                                    *grid, *transform, filter);
                            }
                            else {
                                countGrid = openvdb::points::pointCountGrid(
                                    *grid, filter);
                            }
                        }
                        else {
                            countGrid = openvdb::Int32Grid::create();
                        }

                        const std::string customName = evalStdString("maskname", time);

                        std::string vdbName;
                        switch (outputName) {
                            case NAME_KEEP:    vdbName = gridName; break;
                            case NAME_APPEND:  vdbName = gridName + customName; break;
                            case NAME_REPLACE: vdbName = customName; break;
                        }
                        hvdb::createVdbPrimitive(*gdp, countGrid, vdbName.c_str());
                    }
                }

                return error();
            }
        }

        // if we're here, we're converting Houdini points to OpenVDB. Clear gdp entirely
        // before proceeding, then check for particles in the primary (left) input port

        gdp->clearAndDestroy();

        const GU_Detail* ptGeo = inputGeo(0, context);

        const auto transformMode = evalInt("transform", 0, time);

        math::Mat4d matrix(math::Mat4d::identity());

        if (transform && transformMode != TRANSFORM_REF_GRID) {
            const math::AffineMap::ConstPtr affineMap = transform->baseMap()->getAffineMap();
            matrix = affineMap->getMat4();
        }
        else if (!transform && transformMode == TRANSFORM_REF_GRID) {
            addError(SOP_MESSAGE, "No target VDB transform found on second input.");
            return error();
        }

        if (transformMode == TRANSFORM_TARGET_POINTS) {
            const int pointsPerVoxel = static_cast<int>(evalInt("pointspervoxel", 0, time));
            const float voxelSize =
                hvdb::computeVoxelSizeFromHoudini(*ptGeo, pointsPerVoxel,
                    matrix, /*rounding*/ 5, boss);

            matrix.preScale(Vec3d(voxelSize) / math::getScale(matrix));
            transform = Transform::createLinearTransform(matrix);
        } else if (transformMode == TRANSFORM_VOXEL_SIZE) {
            const auto voxelSize = evalFloat("voxelsize", 0, time);
            matrix.preScale(Vec3d(voxelSize) / math::getScale(matrix));
            transform = Transform::createLinearTransform(matrix);
        }

        UT_String attrName;
        openvdb_houdini::AttributeInfoMap attributes;

        GU_Detail nonConstDetail;
        const GU_Detail* detail;

        // unpack any packed primitives

        boss.start();

        for (GA_Iterator it(ptGeo->getPrimitiveRange()); !it.atEnd(); ++it)
        {
            GA_Offset offset = *it;

            const GA_Primitive* primitive = ptGeo->getPrimitive(offset);
            if (!primitive || !GU_PrimPacked::isPackedPrimitive(*primitive)) continue;

            const GU_PrimPacked* packedPrimitive = static_cast<const GU_PrimPacked*>(primitive);

            packedPrimitive->unpack(nonConstDetail);
        }

        if (ptGeo->getNumPoints() > 0 && nonConstDetail.getNumPoints() == 0) {
            // only unpacked points exist so just use the input gdp

            detail = ptGeo;
        }
        else {
            // merge unpacked and packed point data

            nonConstDetail.mergePoints(*ptGeo);
            detail = &nonConstDetail;
        }

        if (evalInt("mode", 0, time) != 0) {
            // Transfer point attributes.
            if (evalInt("attrList", 0, time) > 0) {
                for (int i = 1, N = static_cast<int>(evalInt("attrList", 0, 0)); i <= N; ++i) {
                    evalStringInst("attribute#", &i, attrName, 0, 0);
                    const Name attributeName = Name(attrName);

                    const GA_ROAttributeRef attrRef =
                        detail->findPointAttribute(attributeName.c_str());

                    if (!attrRef.isValid()) continue;

                    const GA_Attribute* const attribute = attrRef.getAttribute();

                    if (!attribute) continue;

                    const GA_Storage storage(hvdb::attributeStorageType(attribute));

                    // only tuple and string tuple attributes are supported

                    if (storage == GA_STORE_INVALID) {
                        throw std::runtime_error{"Invalid attribute type - " + attributeName};
                    }

                    const int16_t width(hvdb::attributeTupleSize(attribute));
                    assert(width > 0);

                    const GA_TypeInfo typeInfo(attribute->getOptions().typeInfo());

                    const bool isVector = width == 3 && (typeInfo == GA_TYPE_VECTOR ||
                                                         typeInfo == GA_TYPE_NORMAL ||
                                                         typeInfo == GA_TYPE_COLOR);
                    const bool isQuaternion = width == 4 && (typeInfo == GA_TYPE_QUATERNION);
                    const bool isMatrix = width == 16 && (typeInfo == GA_TYPE_TRANSFORM);

                    int valueCompression = static_cast<int>(
                        evalIntInst("valuecompression#", &i, 0, 0));

                    // check value compression compatibility with attribute type

                    if (valueCompression != hvdb::COMPRESSION_NONE) {
                        if (storage == GA_STORE_STRING) {
                            // disable value compression for strings and add a SOP warning
                            valueCompression = hvdb::COMPRESSION_NONE;
                            warnFunction("Value compression not supported on string attributes."
                                " Disabling compression for attribute \""
                                + attributeName + "\".");
                        } else {
                            // disable value compression for incompatible types
                            // and add a SOP warning

                            if (valueCompression == hvdb::COMPRESSION_TRUNCATE &&
                                (storage != GA_STORE_REAL32 || isQuaternion || isMatrix))
                            {
                                valueCompression = hvdb::COMPRESSION_NONE;
                                warnFunction("Truncate value compression only supported for 32-bit"
                                    " floating-point attributes. Disabling compression for"
                                    " attribute \"" + attributeName + "\".");
                            }

                            if (valueCompression == hvdb::COMPRESSION_UNIT_VECTOR &&
                                (storage != GA_STORE_REAL32 || !isVector))
                            {
                                valueCompression = hvdb::COMPRESSION_NONE;
                                warnFunction("Unit Vector value compression only supported for"
                                    " vector 3 x 32-bit floating-point attributes. "
                                    "Disabling compression for attribute \""
                                    + attributeName + "\".");
                            }

                            const bool isUnit =
                                (valueCompression == hvdb::COMPRESSION_UNIT_FIXED_POINT_8
                              || valueCompression == hvdb::COMPRESSION_UNIT_FIXED_POINT_16);
                            if (isUnit && (storage != GA_STORE_REAL32 || (width != 1 && !isVector)))
                            {
                                valueCompression = hvdb::COMPRESSION_NONE;
                                warnFunction("Unit compression only supported for scalar and vector"
                                    " 3 x 32-bit floating-point attributes. "
                                    "Disabling compression for attribute \""
                                    + attributeName + "\".");
                            }
                        }
                    }

                    attributes[attributeName] = std::pair<int, bool>(valueCompression, false);
                }
            }
        } else {

            // point attribute names
            auto iter = detail->pointAttribs().begin(GA_SCOPE_PUBLIC);

            const auto normalCompression = evalInt("normalcompression", 0, time);
            const auto colorCompression = evalInt("colorcompression", 0, time);

            if (!iter.atEnd()) {
                for (; !iter.atEnd(); ++iter) {
                    const char* str = (*iter)->getName();
                    if (!str) continue;

                    const Name attributeName = str;

                    if (attributeName == "P") continue;

                    const GA_ROAttributeRef attrRef =
                        detail->findPointAttribute(attributeName.c_str());

                    if (!attrRef.isValid()) continue;

                    const GA_Attribute* const attribute = attrRef.getAttribute();

                    if (!attribute) continue;

                    const GA_Storage storage(hvdb::attributeStorageType(attribute));

                    // only tuple and string tuple attributes are supported

                    if (storage == GA_STORE_INVALID) {
                        throw std::runtime_error{"Invalid attribute type - " + attributeName};
                    }

                    const int16_t width(hvdb::attributeTupleSize(attribute));
                    assert(width > 0);

                    const GA_TypeInfo typeInfo(attribute->getOptions().typeInfo());

                    const bool isNormal = width == 3 && typeInfo == GA_TYPE_NORMAL;
                    const bool isColor = width == 3 && typeInfo == GA_TYPE_COLOR;

                    int valueCompression = hvdb::COMPRESSION_NONE;

                    if (isNormal) {
                        if (normalCompression == 1) {
                            valueCompression = hvdb::COMPRESSION_UNIT_VECTOR;
                        } else if (normalCompression == 2) {
                            valueCompression = hvdb::COMPRESSION_TRUNCATE;
                        }
                    }
                    else if (isColor) {
                        if (colorCompression == 1) {
                            valueCompression = hvdb::COMPRESSION_UNIT_FIXED_POINT_16;
                        } else if (colorCompression == 2) {
                            valueCompression = hvdb::COMPRESSION_UNIT_FIXED_POINT_8;
                        } else if (colorCompression == 3) {
                            valueCompression = hvdb::COMPRESSION_TRUNCATE;
                        }
                    }

                    attributes[attributeName] = std::pair<int, bool>(valueCompression, false);
                }
            }
        }

        // Determine position compression

        const int positionCompression = static_cast<int>(evalInt("poscompression", 0, time));

        PointDataGrid::Ptr pointDataGrid = hvdb::convertHoudiniToPointDataGrid(
            *detail, positionCompression, attributes, *transform, warnFunction);

        hvdb::populateMetadataFromHoudini(*pointDataGrid, *detail, warnFunction);

        hvdb::createVdbPrimitive(*gdp, pointDataGrid, evalStdString("name", time).c_str());

        boss.end();

    } catch (const std::exception& e) {
        addError(SOP_MESSAGE, e.what());
    }
    return error();
}
