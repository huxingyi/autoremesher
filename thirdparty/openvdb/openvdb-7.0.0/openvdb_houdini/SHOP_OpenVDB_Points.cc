// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

/// @file SHOP_OpenVDB_Points.cc
///
/// @authors Dan Bailey, Richard Kwok
///
/// @brief The Delayed Load Procedural SHOP for OpenVDB Points.

#include <UT/UT_DSOVersion.h>
#include <UT/UT_Version.h>
#include <UT/UT_Ramp.h>
#include <OP/OP_OperatorTable.h>
#include <SHOP/SHOP_Node.h>
#include <SHOP/SHOP_Operator.h>
#include <PRM/PRM_Include.h>

#include <houdini_utils/ParmFactory.h>

#include <sstream>

namespace hutil = houdini_utils;


class SHOP_OpenVDB_Points : public SHOP_Node
{
public:
    static const char* nodeName() { return "openvdb_points"; }

    SHOP_OpenVDB_Points(OP_Network *parent, const char *name, OP_Operator *entry);
    ~SHOP_OpenVDB_Points() override = default;

    static OP_Node* factory(OP_Network*, const char* name, OP_Operator*);

    bool buildShaderString(UT_String &result, fpreal now, const UT_Options *options,
        OP_Node *obj=0, OP_Node *sop=0, SHOP_TYPE interpretType = SHOP_INVALID) override;

protected:
    OP_ERROR cookMe(OP_Context&) override;
    bool updateParmsFlags() override;
}; // class SHOP_OpenVDB_Points


////////////////////////////////////////


OP_Node*
SHOP_OpenVDB_Points::factory(OP_Network* net, const char* name, OP_Operator* op)
{
    return new SHOP_OpenVDB_Points(net, name, op);
}

SHOP_OpenVDB_Points::SHOP_OpenVDB_Points(OP_Network *parent, const char *name, OP_Operator *entry)
    : SHOP_Node(parent, name, entry, SHOP_GEOMETRY)
{
}

bool
SHOP_OpenVDB_Points::buildShaderString(UT_String &result, fpreal now,
    const UT_Options*, OP_Node*, OP_Node*, SHOP_TYPE)
{
    UT_String fileStr = "";
    evalString(fileStr, "file", 0, now);

    UT_String groupMaskStr = "";
    evalString(groupMaskStr, "groupmask", 0, now);

    UT_String attrMaskStr = "";
    evalString(attrMaskStr, "attrmask", 0, now);

    std::stringstream ss;
    ss << SHOP_OpenVDB_Points::nodeName();
    ss << " file \"" << fileStr.toStdString() << "\"";
    ss << " streamdata " << evalInt("streamdata", 0, now);
    ss << " groupmask \"" << groupMaskStr.toStdString() << "\"";
    ss << " attrmask \"" << attrMaskStr.toStdString() << "\"";
    ss << " speedtocolor " << evalInt("speedtocolor", 0, now);
    ss << " maxspeed " << evalFloat("maxspeed", 0, now);

    // write the speed/color ramp into the ifd
    UT_Ramp ramp;
    updateRampFromMultiParm(now, getParm("function"), ramp);

    ss << " ramp \"";
    for(int n = 0, N = ramp.getNodeCount(); n < N; n++){
        const UT_ColorNode* rampNode = ramp.getNode(n);
        ss << rampNode->t << " ";
        ss << rampNode->rgba.r << " " << rampNode->rgba.g << " " <<  rampNode->rgba.b << " ";
        ss << static_cast<int>(rampNode->basis) << " ";
    }
    ss << "\"";

    result = ss.str();
    return true;
}

OP_ERROR
SHOP_OpenVDB_Points::cookMe(OP_Context& context)
{
    return SHOP_Node::cookMe(context);
}

bool
SHOP_OpenVDB_Points::updateParmsFlags()
{
    bool changed = false;

    const bool speedToColor = evalInt("speedtocolor", 0, 0);

    changed |= enableParm("sep1", speedToColor);
    changed |= setVisibleState("sep1", speedToColor);

    changed |= enableParm("maxspeed", speedToColor);
    changed |= setVisibleState("maxspeed", speedToColor);

    changed |= enableParm("function", speedToColor);
    changed |= setVisibleState("function", speedToColor);

    return changed;
}

////////////////////////////////////////


// Build UI and register this operator.
void
newShopOperator(OP_OperatorTable *table)
{
    if (table == nullptr) return;

    hutil::ParmList parms;

    parms.add(hutil::ParmFactory(PRM_FILE, "file", "File")
        .setDefault("./filename.vdb")
        .setHelpText("File path to the VDB to load."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE,
        "streamdata", "Stream Data for Maximum Memory Efficiency")
        .setDefault(PRMoneDefaults)
        .setHelpText(
            "Stream the data from disk to keep the memory footprint as small as possible."
            " This will make the initial conversion marginally slower because the data"
            " will be loaded twice, once for pre-computation to evaluate the bounding box"
            " and once for the actual conversion."));

    parms.add(hutil::ParmFactory(PRM_STRING, "groupmask", "Group Mask")
        .setDefault("")
        .setHelpText("Specify VDB Points Groups to use. (Default is all groups)"));

    parms.add(hutil::ParmFactory(PRM_STRING, "attrmask", "Attribute Mask")
        .setDefault("")
        .setHelpText("Specify VDB Points Attributes to use. (Default is all attributes)"));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "speedtocolor", "Map Speed To Color")
        .setDefault(PRMzeroDefaults)
        .setHelpText(
            "Replaces the 'Cd' point attribute with colors mapped from the"
            " 'v' point attribute using a ramp."));

    parms.add(hutil::ParmFactory(PRM_SEPARATOR, "sep1", ""));

    parms.add(hutil::ParmFactory(PRM_FLT_J, "maxspeed", "Max Speed")
        .setDefault(1.0f)
        .setHelpText("Reference for 1.0 on the color gradient."));

    parms.add(hutil::ParmFactory(PRM_MULTITYPE_RAMP_RGB, "function", "Speed to Color Function")
        .setDefault(PRMtwoDefaults)
        .setHelpText("Function mapping speeds between 0 and 1 to a color."));

    //////////
    // Register this operator.

    SHOP_Operator* shop = new SHOP_Operator(SHOP_OpenVDB_Points::nodeName(), "OpenVDB Points",
        SHOP_OpenVDB_Points::factory,
        parms.get(),
        /*child_table_name=*/nullptr,
        /*min_sources=*/0, /*max_sources=*/0,
        SHOP_Node::myVariableList,
        OP_FLAG_GENERATOR,
        SHOP_AUTOADD_NONE);
    shop->setIconName("SHOP_geometry");

    table->addOperator(shop);

    //////////
    // Set the SHOP-specific data

    SHOP_OperatorInfo* info = UTverify_cast<SHOP_OperatorInfo*>(shop->getOpSpecificData());
    info->setShaderType(SHOP_GEOMETRY);

    // Set the rendermask to "*" and try to support *all* renderers.
    info->setRenderMask("*");
}
