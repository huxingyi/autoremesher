// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

/// @file SOP_OpenVDB_Points_Group.cc
///
/// @author Dan Bailey
///
/// @brief Add and remove point groups.

#include <openvdb/openvdb.h>
#include <openvdb/points/PointDataGrid.h>
#include <openvdb/points/PointGroup.h>

#include <openvdb_houdini/SOP_NodeVDB.h>
#include <openvdb_houdini/PointUtils.h>
#include <openvdb_houdini/Utils.h>
#include <houdini_utils/geometry.h>
#include <houdini_utils/ParmFactory.h>

#include <algorithm> // for std::find()
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>



using namespace openvdb;
using namespace openvdb::points;
using namespace openvdb::math;

namespace hvdb = openvdb_houdini;
namespace hutil = houdini_utils;


namespace {

struct GroupParms {
    // global parms
    bool                          mEnable             = false;
    std::string                   mGroupName          = "";
    const GA_PrimitiveGroup *     mGroup              = nullptr;
    // operation flags
    bool                          mOpGroup            = false;
    bool                          mOpLeaf             = false;
    bool                          mOpHashI            = false;
    bool                          mOpHashL            = false;
    bool                          mOpBBox             = false;
    bool                          mOpLS               = false;
    // group parms
    std::vector<std::string>      mIncludeGroups;
    std::vector<std::string>      mExcludeGroups;
    // number parms
    bool                          mCountMode          = false;
    bool                          mHashMode           = false;
    float                         mPercent            = 0.0f;
    long                          mCount              = 0L;
    std::string                   mHashAttribute      = "";
    size_t                        mHashAttributeIndex = openvdb::points::AttributeSet::INVALID_POS;
    // bbox parms
    openvdb::BBoxd                mBBox;
    // level set parms
    openvdb::FloatGrid::ConstPtr  mLevelSetGrid       = FloatGrid::create(0);
    float                         mSDFMin             = 0.0f;
    float                         mSDFMax             = 0.0f;
    // viewport parms
    bool                          mEnableViewport     = false;
    bool                          mAddViewport        = false;
    std::string                   mViewportGroupName  = "";
    // drop groups
    bool                          mDropAllGroups      = false;
    std::vector<std::string>      mDropIncludeGroups;
    std::vector<std::string>      mDropExcludeGroups;

};

} // namespace


////////////////////////////////////////


class SOP_OpenVDB_Points_Group: public hvdb::SOP_NodeVDB
{
public:
    SOP_OpenVDB_Points_Group(OP_Network*, const char* name, OP_Operator*);
    ~SOP_OpenVDB_Points_Group() override = default;

    static OP_Node* factory(OP_Network*, const char* name, OP_Operator*);

    int isRefInput(unsigned i) const override { return (i > 0); }

    class Cache: public SOP_VDBCacheOptions
    {
    public:
        OP_ERROR evalGroupParms(OP_Context&, GroupParms&);
        OP_ERROR evalGridGroupParms(const PointDataGrid&, OP_Context&, GroupParms&);

        void performGroupFiltering(PointDataGrid&, const GroupParms&);
        void setViewportMetadata(PointDataGrid&, const GroupParms&);
        void removeViewportMetadata(PointDataGrid&);
    protected:
        OP_ERROR cookVDBSop(OP_Context&) override;
    }; // class Cache

protected:
    bool updateParmsFlags() override;
    void resolveObsoleteParms(PRM_ParmList*) override;
}; // class SOP_OpenVDB_Points_Group



////////////////////////////////////////


static PRM_Default negPointOneDefault(-0.1);
static PRM_Default fiveThousandDefault(5000);

// Build UI and register this operator.
void
newSopOperator(OP_OperatorTable* table)
{
    openvdb::initialize();

    if (table == nullptr) return;

    hutil::ParmList parms;

    parms.add(hutil::ParmFactory(PRM_STRING, "group", "Group")
        .setChoiceList(&hutil::PrimGroupMenuInput1)
        .setTooltip("Specify a subset of the input VDB grids to be loaded.")
        .setDocumentation(
            "A subset of the input VDB Points primitives to be processed"
            " (see [specifying volumes|/model/volumes#group])"));

    parms.add(hutil::ParmFactory(PRM_STRING, "vdbpointsgroup", "VDB Points Group")
        .setChoiceList(&hvdb::VDBPointsGroupMenuInput1)
        .setTooltip(
            "Create a new VDB points group as a subset of an existing VDB points group(s)."));

    parms.beginSwitcher("tabMenu1");
    parms.addFolder("Create");

    // Toggle to enable creation
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "enablecreate", "Enable")
        .setDefault(PRMoneDefaults)
        .setTooltip("Enable creation of the group."));

    parms.add(hutil::ParmFactory(PRM_STRING, "groupname", "Group Name")
        .setDefault(0, ::strdup("group1"))
        .setTooltip("The name of the internal group to create"));

    parms.beginSwitcher("tabMenu2");
    parms.addFolder("Number");

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "enablenumber", "Enable")
        .setDefault(PRMzeroDefaults)
        .setTooltip("Enable filtering by number."));

    parms.add(hutil::ParmFactory(PRM_ORD, "numbermode", "Mode")
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, {
            "percentage",   "Percentage",
            "total",        "Total"
        })
        .setTooltip("Specify how to filter out a subset of the points inside the VDB Points."));

    parms.add(hutil::ParmFactory(PRM_FLT, "pointpercent", "Percent")
        .setDefault(PRMtenDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_RESTRICTED, 100)
        .setTooltip("The percentage of points to include in the group"));

    parms.add(hutil::ParmFactory(PRM_INT, "pointcount", "Count")
        .setDefault(&fiveThousandDefault)
        .setRange(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_UI, 1000000)
        .setTooltip("The total number of points to include in the group"));

    parms.add(hutil::ParmFactory(PRM_TOGGLE_J, "enablepercentattribute", "")
        .setDefault(PRMzeroDefaults)
        .setTypeExtended(PRM_TYPE_TOGGLE_JOIN));

    parms.add(hutil::ParmFactory(PRM_STRING, "percentattribute", "Attribute Seed")
        .setDefault(0, ::strdup("id"))
        .setTooltip("The point attribute to use as a seed for percent filtering"));

    parms.endSwitcher();

    parms.beginSwitcher("tabMenu3");
    parms.addFolder("Bounding");

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "enableboundingbox", "Enable")
        .setDefault(PRMzeroDefaults)
        .setTooltip("Enable filtering by bounding box."));

    parms.add(hutil::ParmFactory(PRM_ORD, "boundingmode", "Mode")
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, {
            "boundingbox",      "Bounding Box",
            "boundingobject",   "Bounding Object"
        }));

    parms.add(hutil::ParmFactory(PRM_STRING, "boundingname", "Name")
        .setChoiceList(&hutil::PrimGroupMenuInput2)
        .setTooltip("The name of the bounding geometry"));

    parms.add(hutil::ParmFactory(PRM_XYZ, "size", "Size")
        .setDefault(PRMoneDefaults)
        .setVectorSize(3)
        .setTooltip("The size of the bounding box"));

    parms.add(hutil::ParmFactory(PRM_XYZ, "center", "Center")
        .setVectorSize(3)
        .setTooltip("The center of the bounding box"));

    parms.endSwitcher();

    parms.beginSwitcher("tabMenu4");
    parms.addFolder("SDF");

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "enablesdf", "Enable")
        .setDefault(PRMzeroDefaults)
        .setTooltip("Enable filtering by SDF."));

    parms.add(hutil::ParmFactory(PRM_STRING, "sdfname", "Name")
        .setChoiceList(&hutil::PrimGroupMenuInput2)
        .setTooltip("The name of the SDF"));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "enablesdfmin", "Enable")
        .setDefault(PRMoneDefaults)
        .setTypeExtended(PRM_TYPE_TOGGLE_JOIN)
        .setTooltip("Enable SDF minimum."));

    parms.add(hutil::ParmFactory(PRM_FLT, "sdfmin", "SDF Minimum")
        .setDefault(&negPointOneDefault)
        .setRange(PRM_RANGE_UI, -1, PRM_RANGE_UI, 1)
        .setTooltip("SDF minimum value"));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "enablesdfmax", "Enable")
        .setDefault(PRMoneDefaults)
        .setTypeExtended(PRM_TYPE_TOGGLE_JOIN)
        .setTooltip("Enable SDF maximum."));

    parms.add(hutil::ParmFactory(PRM_FLT, "sdfmax", "SDF Maximum")
        .setDefault(PRMpointOneDefaults)
        .setRange(PRM_RANGE_UI, -1, PRM_RANGE_UI, 1)
        .setTooltip("SDF maximum value"));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "sdfinvert", "Invert")
        .setDefault(PRMzeroDefaults)
        .setTooltip("Invert SDF minimum and maximum."));

    parms.endSwitcher();

    parms.addFolder("Delete");

    parms.add(hutil::ParmFactory(PRM_STRING, "deletegroups", "Point Groups")
        .setDefault(0, "")
        .setHelpText(
            "A space-delimited list of groups to delete.\n\n"
            "This will delete the selected groups but will not delete"
            " the points contained in them.")
        .setChoiceList(&hvdb::VDBPointsGroupMenuInput1));

    parms.addFolder("Viewport");

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "enableviewport", "Enable")
        .setDefault(PRMzeroDefaults)
        .setTooltip("Toggle viewport group.")
        .setDocumentation(
            "Enable the viewport group.\n\n"
            "This allows one to specify a subset of points to be displayed in the viewport.\n"
            "This minimizes the data transfer to the viewport without removing the data.\n\n"
            "NOTE:\n"
            "    Only one group can be tagged as a viewport group.\n"));

    parms.add(hutil::ParmFactory(PRM_ORD, "viewportoperation", "Operation")
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, {
            "addviewportgroup",      "Add Viewport Group",
            "removeviewportgroup",   "Remove Viewport Group"
        })
        .setTooltip("Specify whether to add or remove the viewport group."));

    parms.add(hutil::ParmFactory(PRM_STRING, "viewportgroupname", "Name")
        .setDefault("chs(\"groupname\")", CH_OLD_EXPRESSION)
        .setTooltip("Display only this group in the viewport."));

    parms.endSwitcher();


    hutil::ParmList obsoleteParms;
    obsoleteParms.add(houdini_utils::ParmFactory(PRM_LABEL, "spacer1", ""));
    obsoleteParms.add(hutil::ParmFactory(PRM_TOGGLE, "enablelevelset", "Enable")
        .setDefault(PRMzeroDefaults));
    obsoleteParms.add(hutil::ParmFactory(PRM_STRING, "levelsetname", "Name"));


    //////////
    // Register this operator.

    hvdb::OpenVDBOpFactory("VDB Points Group",
        SOP_OpenVDB_Points_Group::factory, parms, *table)
        .addInput("VDB Points")
        .addOptionalInput("Optional bounding geometry or level set")
        .setObsoleteParms(obsoleteParms)
        .setVerb(SOP_NodeVerb::COOK_INPLACE, []() { return new SOP_OpenVDB_Points_Group::Cache; })
        .setDocumentation("\
#icon: COMMON/openvdb\n\
#tags: vdb\n\
\n\
\"\"\"Manipulate the internal groups of a VDB Points primitive.\"\"\"\n\
\n\
@overview\n\
\n\
This node acts like the [Node:sop/group] node, but for the points inside\n\
a VDB Points primitive.\n\
It can create and manipulate the primitive's internal groups.\n\
Generated groups can be used to selectively unpack a subset of the points\n\
with an [OpenVDB Points Convert node|Node:sop/DW_OpenVDBPointsConvert].\n\
\n\
@related\n\
- [OpenVDB Points Convert|Node:sop/DW_OpenVDBPointsConvert]\n\
- [OpenVDB Points Delete|Node:sop/DW_OpenVDBPointsDelete]\n\
\n\
@examples\n\
\n\
See [openvdb.org|http://www.openvdb.org/download/] for source code\n\
and usage examples.\n");
}


void
SOP_OpenVDB_Points_Group::resolveObsoleteParms(PRM_ParmList* obsoleteParms)
{
    if (!obsoleteParms) return;

    resolveRenamedParm(*obsoleteParms, "enablelevelset", "enablesdf");
    resolveRenamedParm(*obsoleteParms, "levelsetname", "sdfname");

    // Delegate to the base class.
    hvdb::SOP_NodeVDB::resolveObsoleteParms(obsoleteParms);
}


bool
SOP_OpenVDB_Points_Group::updateParmsFlags()
{
    bool changed = false;

    const bool creation = evalInt("enablecreate", 0, 0) != 0;
    const bool number = evalInt("enablenumber", 0, 0) != 0;
    const bool total = evalInt("numbermode", 0, 0) == 1;
    const bool percentattribute = evalInt("enablepercentattribute", 0, 0) != 0;
    const bool bounding = evalInt("enableboundingbox", 0, 0) != 0;
    const bool boundingobject = evalInt("boundingmode", 0, 0) == 1;
    const bool viewport = evalInt("enableviewport", 0, 0) != 0;
    const bool levelset = evalInt("enablesdf", 0, 0);
    const bool sdfmin = evalInt("enablesdfmin", 0, 0);
    const bool sdfmax = evalInt("enablesdfmax", 0, 0);
    const bool viewportadd = evalInt("viewportoperation", 0, 0) == 0;

    changed |= enableParm("vdbpointsgroup", creation);
    changed |= enableParm("groupname", creation);
    changed |= enableParm("enablenumber", creation);
    changed |= enableParm("numbermode", creation && number);
    changed |= enableParm("pointpercent", creation && number && !total);
    changed |= enableParm("pointcount", creation && number && total);
    changed |= enableParm("enablepercentattribute", creation && number && !total);
    changed |= enableParm("percentattribute", creation && number && percentattribute && !total);
    changed |= enableParm("enableboundingbox", creation);
    changed |= enableParm("boundingmode", creation && bounding);
    changed |= enableParm("boundingname", creation && bounding && boundingobject);
    changed |= enableParm("size", creation && bounding && !boundingobject);
    changed |= enableParm("center", creation && bounding && !boundingobject);
    changed |= enableParm("viewportoperation", viewport);
    changed |= enableParm("viewportgroupname", viewport && viewportadd);
    changed |= enableParm("sdfname", levelset);
    changed |= enableParm("enablesdfmin", levelset);
    changed |= enableParm("enablesdfmax", levelset);
    changed |= enableParm("sdfmin", levelset && sdfmin);
    changed |= enableParm("sdfmax", levelset && sdfmax);
    changed |= enableParm("sdfinvert", levelset && sdfmin && sdfmax);

    return changed;
}


////////////////////////////////////////


OP_Node*
SOP_OpenVDB_Points_Group::factory(OP_Network* net,
    const char* name, OP_Operator* op)
{
    return new SOP_OpenVDB_Points_Group(net, name, op);
}


SOP_OpenVDB_Points_Group::SOP_OpenVDB_Points_Group(OP_Network* net,
    const char* name, OP_Operator* op)
    : hvdb::SOP_NodeVDB(net, name, op)
{
}


////////////////////////////////////////


OP_ERROR
SOP_OpenVDB_Points_Group::Cache::cookVDBSop(OP_Context& context)
{
    try {
        // Evaluate UI parameters
        GroupParms parms;
        if (evalGroupParms(context, parms) >= UT_ERROR_ABORT) return error();

        UT_AutoInterrupt progress("Processing Points Group");

        hvdb::VdbPrimIterator vdbIt(gdp, parms.mGroup);

        for (; vdbIt; ++vdbIt) {

            if (progress.wasInterrupted()) {
                throw std::runtime_error("processing was interrupted");
            }

            GU_PrimVDB* vdbPrim = *vdbIt;

            // only process if grid is a PointDataGrid with leaves
            if(!openvdb::gridConstPtrCast<PointDataGrid>(vdbPrim->getConstGridPtr())) continue;
            auto&& pointDataGrid = UTvdbGridCast<PointDataGrid>(vdbPrim->getConstGrid());
            auto leafIter = pointDataGrid.tree().cbeginLeaf();
            if (!leafIter) continue;

            // Set viewport metadata if no group being created
            // (copy grid first to ensure metadata is deep copied)
            if (!parms.mEnable) {
                if (parms.mEnableViewport) {
                    auto&& outputGrid = UTvdbGridCast<PointDataGrid>(vdbPrim->getGrid());
                    if (parms.mAddViewport) {
                        setViewportMetadata(outputGrid, parms);
                    } else {
                        removeViewportMetadata(outputGrid);
                    }
                }
            }

            const AttributeSet::Descriptor& descriptor = leafIter->attributeSet().descriptor();

            std::vector<std::string> groupsToDrop;

            bool hasGroupsToDrop = !descriptor.groupMap().empty();
            if (hasGroupsToDrop) {
                // exclude groups mode
                if (!parms.mDropExcludeGroups.empty()) {
                    // if any groups are to be excluded, ignore those to be included
                    // and rebuild them
                    for (const auto& it: descriptor.groupMap()) {
                        if (std::find(  parms.mDropExcludeGroups.begin(),
                                        parms.mDropExcludeGroups.end(), it.first) ==
                                        parms.mDropExcludeGroups.end()) {
                            groupsToDrop.push_back(it.first);
                        }
                    }
                }
                else if (!parms.mDropAllGroups) {
                    // if any groups are to be included, intersect them with groups that exist
                    for (const auto& groupName : parms.mDropIncludeGroups) {
                        if (descriptor.hasGroup(groupName)) {
                            groupsToDrop.push_back(groupName);
                        }
                    }
                }
            }

            if (hasGroupsToDrop)    hasGroupsToDrop = parms.mDropAllGroups || !groupsToDrop.empty();

            // If we are not creating groups and there are no groups to drop (due to an empty list or because none of
            // the chosen ones were actually present), we can continue the loop early here
            if(!parms.mEnable && !hasGroupsToDrop) {
                continue;
            }

            // Evaluate grid-specific UI parameters

            if (evalGridGroupParms(pointDataGrid, context, parms) >= UT_ERROR_ABORT)
                return error();

            // deep copy the VDB tree if it is not already unique
            vdbPrim->makeGridUnique();

            auto&& outputGrid = UTvdbGridCast<PointDataGrid>(vdbPrim->getGrid());

            // filter and create the point group in the grid
            if (parms.mEnable) {
                performGroupFiltering(outputGrid, parms);
            }

            // drop groups
            if (parms.mDropAllGroups) {
                dropGroups(outputGrid.tree());
            }
            else if (!groupsToDrop.empty()) {
                dropGroups(outputGrid.tree(), groupsToDrop);
            }

            // attach group viewport metadata to the grid
            if (parms.mEnableViewport) {
                if (parms.mAddViewport)     setViewportMetadata(outputGrid, parms);
                else                        removeViewportMetadata(outputGrid);
            }

        }

        return error();

    } catch (const std::exception& e) {
        addError(SOP_MESSAGE, e.what());
    }
    return error();
}


////////////////////////////////////////


OP_ERROR
SOP_OpenVDB_Points_Group::Cache::evalGroupParms(
    OP_Context& context, GroupParms& parms)
{
    const fpreal time = context.getTime();

    // evaluate filter mode

    const bool number = evalInt("enablenumber", 0, time);
    const bool countMode = evalInt("numbermode", 0, time) == 1;
    const bool percentAttribute = evalInt("enablepercentattribute", 0, time);
    const bool bounding = evalInt("enableboundingbox", 0, time);
    const bool boundingObject = evalInt("boundingmode", 0, time) == 1;
    const bool levelSet = evalInt("enablesdf", 0, time);

    parms.mCountMode = countMode;
    parms.mHashMode = number && !countMode && percentAttribute;
    parms.mOpLeaf = number;
    parms.mOpBBox = bounding;
    parms.mOpLS = levelSet;

    // Get the grids to group.
    parms.mGroup = matchGroup(*gdp, evalStdString("group", time));

    hvdb::VdbPrimIterator vdbIt(gdp, parms.mGroup);

    // Handle no vdbs
    if (!vdbIt) {
        addError(SOP_MESSAGE, "No VDBs found.");
        return error();
    }

    // Get and parse the vdb points groups
    AttributeSet::Descriptor::parseNames(
        parms.mIncludeGroups, parms.mExcludeGroups, evalStdString("vdbpointsgroup", time));

    if (parms.mIncludeGroups.size() > 0 || parms.mExcludeGroups.size() > 0) {
        parms.mOpGroup = true;
    }

    // reference geometry

    const GU_Detail* refGdp = inputGeo(1);

    // group creation

    parms.mEnable = evalInt("enablecreate", 0, time);

    std::string groupName = evalStdString("groupname", time);
    if (groupName.empty()) {
        addWarning(SOP_MESSAGE, "Cannot create a group with an empty name, changing to _");
        groupName = "_";
    } else if (!AttributeSet::Descriptor::validName(groupName)) {
        addError(SOP_MESSAGE, ("Group name contains invalid characters - " + groupName).c_str());
        return error();
    }

    parms.mGroupName = groupName;

    // number

    if (number) {
        parms.mPercent = static_cast<float>(evalFloat("pointpercent", 0, time));
        parms.mCount = evalInt("pointcount", 0, time);
        parms.mHashAttribute = evalStdString("percentattribute", time);
    }

    // bounds

    if (bounding) {
        if (boundingObject) {
            if (!refGdp) {
                addError(SOP_MESSAGE, "Missing second input");
                return error();
            }

            // retrieve bounding object

            const GA_PrimitiveGroup* boundsGroup = parsePrimitiveGroups(
                evalStdString("boundingname", time).c_str(), GroupCreator(refGdp));

            // compute bounds of bounding object

            UT_BoundingBox box;
            box.initBounds();
            if (boundsGroup) {
                GA_Range range = refGdp->getPrimitiveRange(boundsGroup);
                refGdp->enlargeBoundingBox(box, range);
            }
            else {
                refGdp->computeQuickBounds(box);
            }

            parms.mBBox.min()[0] = box.xmin();
            parms.mBBox.min()[1] = box.ymin();
            parms.mBBox.min()[2] = box.zmin();
            parms.mBBox.max()[0] = box.xmax();
            parms.mBBox.max()[1] = box.ymax();
            parms.mBBox.max()[2] = box.zmax();
        }
        else {
            // store bounding box
            openvdb::BBoxd::ValueType size(
                evalFloat("size", 0, time),
                evalFloat("size", 1, time),
                evalFloat("size", 2, time));
            openvdb::BBoxd::ValueType center(
                evalFloat("center", 0, time),
                evalFloat("center", 1, time),
                evalFloat("center", 2, time));
            parms.mBBox = openvdb::BBoxd(center - size/2, center + size/2);
        }
    }

    // level set

    if (levelSet)
    {
        if (!refGdp) {
            addError(SOP_MESSAGE, "Missing second input");
            return error();
        }

        // retrieve level set grid

        const GA_PrimitiveGroup* levelSetGroup = parsePrimitiveGroups(
            evalStdString("sdfname", time).c_str(), GroupCreator(refGdp));
        for (hvdb::VdbPrimCIterator vdbRefIt(refGdp, levelSetGroup); vdbRefIt; ++vdbRefIt) {
            if (vdbRefIt->getStorageType() == UT_VDB_FLOAT &&
                vdbRefIt->getGrid().getGridClass() == openvdb::GRID_LEVEL_SET) {
                parms.mLevelSetGrid = gridConstPtrCast<FloatGrid>((*vdbRefIt)->getConstGridPtr());
                break;
            }
        }
        if (!parms.mLevelSetGrid) {
            addError(SOP_MESSAGE, "Second input has no float VDB level set");
            return error();
        }

        bool enableSDFMin = evalInt("enablesdfmin", 0, time);
        bool enableSDFMax = evalInt("enablesdfmax", 0, time);

        float sdfMin = enableSDFMin ?
            static_cast<float>(evalFloat("sdfmin", 0, time)) : -std::numeric_limits<float>::max();
        float sdfMax = enableSDFMax ?
            static_cast<float>(evalFloat("sdfmax", 0, time)) : std::numeric_limits<float>::max();

        // check level set min and max values

        if ((enableSDFMin || enableSDFMax) && sdfMin > sdfMax) {
            addWarning(SOP_MESSAGE, "SDF minimum is greater than SDF maximum,"
                " suggest using the invert toggle instead");
        }
        const float background = parms.mLevelSetGrid->background();
        if (enableSDFMin && sdfMin < -background) {
            addWarning(SOP_MESSAGE,
                "SDF minimum value is less than the background value of the level set");
        }
        if (enableSDFMax && sdfMax > background) {
            addWarning(SOP_MESSAGE,
                "SDF maximum value is greater than the background value of the level set");
        }

        const bool sdfInvert = evalInt("sdfinvert", 0, time);
        parms.mSDFMin = sdfInvert ? -sdfMin : sdfMin;
        parms.mSDFMax = sdfInvert ? -sdfMax : sdfMax;
    }

    // viewport

    parms.mEnableViewport = evalInt("enableviewport", 0, time);
    parms.mAddViewport = evalInt("viewportoperation", 0, time) == 0;

    std::string viewportGroupName = evalStdString("viewportgroupname", time);
    if (viewportGroupName == "") {
        addWarning(SOP_MESSAGE, "Cannot create a viewport group with an empty name, changing to _");
        viewportGroupName = "_";
    }

    parms.mViewportGroupName = viewportGroupName;

    // group deletion

    AttributeSet::Descriptor::parseNames(parms.mDropIncludeGroups, parms.mDropExcludeGroups,
        parms.mDropAllGroups, evalStdString("deletegroups", time));

    if (parms.mDropAllGroups) {
        // include groups only apply if not also deleting all groups
        parms.mDropIncludeGroups.clear();
        // if exclude groups is not empty, don't delete all groups
        if (!parms.mDropExcludeGroups.empty()) {
            parms.mDropAllGroups = false;
        }
    }
    else {
        // exclude groups only apply if also deleting all groups
        parms.mDropExcludeGroups.clear();
    }

    return error();
}


////////////////////////////////////////


OP_ERROR
SOP_OpenVDB_Points_Group::Cache::evalGridGroupParms(
    const PointDataGrid& grid, OP_Context&, GroupParms& parms)
{
    auto leafIter = grid.tree().cbeginLeaf();

    if (!leafIter)  return error();

    const AttributeSet::Descriptor& descriptor = leafIter->attributeSet().descriptor();

    // check new group doesn't already exist

    if (parms.mEnable) {

        if (descriptor.hasGroup(parms.mGroupName)) {
            addError(SOP_MESSAGE, ("Cannot create duplicate group - " + parms.mGroupName).c_str());
            return error();
        }

        // group

        if (parms.mOpGroup)
        {
            for (const std::string& name : parms.mIncludeGroups) {
                if (!descriptor.hasGroup(name)) {
                    addError(SOP_MESSAGE, ("Unable to find VDB Points group - " + name).c_str());
                    return error();
                }
            }

            for (const std::string& name : parms.mExcludeGroups) {
                if (!descriptor.hasGroup(name)) {
                    addError(SOP_MESSAGE, ("Unable to find VDB Points group - " + name).c_str());
                    return error();
                }
            }
        }

        // number

        if (parms.mHashMode)
        {
            // retrieve percent attribute type (if it exists)

            const size_t index = descriptor.find(parms.mHashAttribute);

            if (index == AttributeSet::INVALID_POS) {
                addError(SOP_MESSAGE, ("Unable to find attribute - " + parms.mHashAttribute).c_str());
                return error();
            }

            parms.mHashAttributeIndex = index;
            const std::string attributeType = descriptor.valueType(index);

            if (attributeType == "int32")       parms.mOpHashI = true;
            else if (attributeType == "int64")  parms.mOpHashL = true;
            else {
                addError(SOP_MESSAGE, ("Unsupported attribute type for percent attribute filtering - "
                    + attributeType).c_str());
                return error();
            }
        }
    }

    return error();
}


////////////////////////////////////////


void
SOP_OpenVDB_Points_Group::Cache::performGroupFiltering(
    PointDataGrid& outputGrid, const GroupParms& parms)
{
    // filter typedefs

    using HashIFilter = AttributeHashFilter<std::mt19937, int>;
    using HashLFilter = AttributeHashFilter<std::mt19937_64, long>;
    using LeafFilter = RandomLeafFilter<PointDataGrid::TreeType, std::mt19937>;
    using LSFilter = LevelSetFilter<FloatGrid>;

    // composite typedefs (a combination of the above five filters)
    // the group filter is always included because it's cheap to execute

    using GroupHashI = BinaryFilter<MultiGroupFilter, HashIFilter>;
    using GroupHashL = BinaryFilter<MultiGroupFilter, HashLFilter>;
    using GroupLeaf = BinaryFilter<MultiGroupFilter, LeafFilter>;
    using GroupLS = BinaryFilter<MultiGroupFilter, LSFilter>;
    using GroupBBox = BinaryFilter<MultiGroupFilter, BBoxFilter>;
    using LSHashI = BinaryFilter<LSFilter, HashIFilter>;
    using LSHashL = BinaryFilter<LSFilter, HashLFilter>;
    using LSLeaf = BinaryFilter<LSFilter, LeafFilter>;

    using GroupBBoxHashI = BinaryFilter<GroupBBox, HashIFilter>;
    using GroupBBoxHashL = BinaryFilter<GroupBBox, HashLFilter>;
    using GroupBBoxLS = BinaryFilter<GroupBBox, LSFilter>;
    using GroupBBoxLeaf = BinaryFilter<GroupBBox, LeafFilter>;
    using GroupLSHashI = BinaryFilter<GroupLS, HashIFilter>;
    using GroupLSHashL = BinaryFilter<GroupLS, HashLFilter>;
    using GroupLSLeaf = BinaryFilter<GroupLS, LeafFilter>;

    using GroupBBoxLSHashI = BinaryFilter<GroupBBox, LSHashI>;
    using GroupBBoxLSHashL = BinaryFilter<GroupBBox, LSHashL>;
    using GroupBBoxLSLeaf = BinaryFilter<GroupBBox, LSLeaf>;

    // grid data

    PointDataTree& tree = outputGrid.tree();
    if (!tree.beginLeaf())  {
        return;
    }

    openvdb::math::Transform& transform = outputGrid.transform();
    const std::string groupName = parms.mGroupName;

    auto targetPoints = static_cast<int>(parms.mCount);
    if (parms.mOpLeaf && !parms.mCountMode) {
        targetPoints = int(math::Round(
            (parms.mPercent * static_cast<double>(pointCount(tree))) / 100.0));
    }

    const AttributeSet& attributeSet = tree.beginLeaf()->attributeSet();

    // build filter data

    MultiGroupFilter groupFilter(parms.mIncludeGroups, parms.mExcludeGroups, attributeSet);
    BBoxFilter bboxFilter(transform, parms.mBBox);
    HashIFilter hashIFilter(parms.mHashAttributeIndex, parms.mPercent);
    HashLFilter hashLFilter(parms.mHashAttributeIndex, parms.mPercent);
    LeafFilter leafFilter(tree, targetPoints);
    LSFilter lsFilter(*parms.mLevelSetGrid, transform, parms.mSDFMin, parms.mSDFMax);

    // build composite filter data

    GroupHashI groupHashIFilter(groupFilter, hashIFilter);
    GroupHashL groupHashLFilter(groupFilter, hashLFilter);
    GroupLeaf groupLeafFilter(groupFilter, leafFilter);
    GroupLS groupLSFilter(groupFilter, lsFilter);
    GroupBBox groupBBoxFilter(groupFilter, bboxFilter);
    LSHashI lsHashIFilter(lsFilter, hashIFilter);
    LSHashL lsHashLFilter(lsFilter, hashLFilter);
    LSLeaf lsLeafFilter(lsFilter, leafFilter);

    GroupBBoxHashI groupBBoxHashIFilter(groupBBoxFilter, hashIFilter);
    GroupBBoxHashL groupBBoxHashLFilter(groupBBoxFilter, hashLFilter);
    GroupBBoxLS groupBBoxLSFilter(groupBBoxFilter, lsFilter);
    GroupBBoxLeaf groupBBoxLeafFilter(groupBBoxFilter, leafFilter);
    GroupLSHashI groupLSHashIFilter(groupLSFilter, hashIFilter);
    GroupLSHashL groupLSHashLFilter(groupLSFilter, hashLFilter);
    GroupLSLeaf groupLSLeafFilter(groupLSFilter, leafFilter);

    GroupBBoxLSHashI groupBBoxLSHashIFilter(groupBBoxFilter, lsHashIFilter);
    GroupBBoxLSHashL groupBBoxLSHashLFilter(groupBBoxFilter, lsHashLFilter);
    GroupBBoxLSLeaf groupBBoxLSLeafFilter(groupBBoxFilter, lsLeafFilter);

    // append the group

    appendGroup(tree, groupName);

    // perform group filtering

    const GroupParms& p = parms;

    if (p.mOpBBox && p.mOpLS && p.mOpHashI) {
        setGroupByFilter(tree, groupName, groupBBoxLSHashIFilter);
    } else if (p.mOpBBox && p.mOpLS && p.mOpHashL) {
       setGroupByFilter(tree, groupName, groupBBoxLSHashLFilter);
    } else if (p.mOpBBox && p.mOpLS && p.mOpLeaf) {
       setGroupByFilter(tree, groupName, groupBBoxLSLeafFilter);
    } else if (p.mOpBBox && p.mOpHashI) {
       setGroupByFilter(tree, groupName, groupBBoxHashIFilter);
    } else if (p.mOpBBox && p.mOpHashL) {
       setGroupByFilter(tree, groupName, groupBBoxHashLFilter);
    } else if (p.mOpBBox && p.mOpLeaf) {
       setGroupByFilter(tree, groupName, groupBBoxLeafFilter);
    } else if (p.mOpBBox && p.mOpLS) {
       setGroupByFilter(tree, groupName, groupBBoxLSFilter);
    } else if (p.mOpLS && p.mOpHashI) {
       setGroupByFilter(tree, groupName, groupLSHashIFilter);
    } else if (p.mOpLS && p.mOpHashL) {
       setGroupByFilter(tree, groupName, groupLSHashLFilter);
    } else if (p.mOpLS && p.mOpLeaf) {
       setGroupByFilter(tree, groupName, groupLSLeafFilter);
    } else if (p.mOpBBox) {
       setGroupByFilter(tree, groupName, groupBBoxFilter);
    } else if (p.mOpLS) {
       setGroupByFilter(tree, groupName, groupLSFilter);
    } else if (p.mOpHashI) {
       setGroupByFilter(tree, groupName, groupHashIFilter);
    } else if (p.mOpHashL) {
       setGroupByFilter(tree, groupName, groupHashLFilter);
    } else if (p.mOpLeaf) {
       setGroupByFilter(tree, groupName, groupLeafFilter);
    } else if (p.mOpGroup) {
       setGroupByFilter(tree, groupName, groupFilter);
    } else {
        setGroup<PointDataTree>(tree, groupName);
    }
}


////////////////////////////////////////


void
SOP_OpenVDB_Points_Group::Cache::setViewportMetadata(
    PointDataGrid& outputGrid, const GroupParms& parms)
{
    outputGrid.insertMeta(openvdb_houdini::META_GROUP_VIEWPORT,
        StringMetadata(parms.mViewportGroupName));
}


void
SOP_OpenVDB_Points_Group::Cache::removeViewportMetadata(
    PointDataGrid& outputGrid)
{
    outputGrid.removeMeta(openvdb_houdini::META_GROUP_VIEWPORT);
}
