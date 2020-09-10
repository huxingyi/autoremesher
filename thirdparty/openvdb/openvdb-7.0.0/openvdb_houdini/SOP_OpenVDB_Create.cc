// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file SOP_OpenVDB_Create.cc
///
/// @author FX R&D OpenVDB team

#include <houdini_utils/ParmFactory.h>
#include <openvdb_houdini/GeometryUtil.h>
#include <openvdb_houdini/SOP_NodeVDB.h>
#include <openvdb_houdini/UT_VDBTools.h> // for GridTransformOp, et al.
#include <openvdb_houdini/Utils.h>
#include <UT/UT_Interrupt.h>
#include <UT/UT_Version.h>
#include <UT/UT_WorkArgs.h>
#include <hboost/algorithm/string/case_conv.hpp>
#include <hboost/algorithm/string/trim.hpp>
#include <OBJ/OBJ_Camera.h>
#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>


namespace hvdb = openvdb_houdini;
namespace hutil = houdini_utils;
namespace cvdb = openvdb;


////////////////////////////////////////


namespace {

// Add new items to the *end* of this list, and update NUM_DATA_TYPES.
enum DataType {
    TYPE_FLOAT = 0,
    TYPE_DOUBLE,
    TYPE_INT,
    TYPE_BOOL,
    TYPE_VEC3S,
    TYPE_VEC3D,
    TYPE_VEC3I
};

enum { NUM_DATA_TYPES = TYPE_VEC3I + 1 };

std::string
dataTypeToString(DataType dts)
{
    std::string ret;
    switch (dts) {
        case TYPE_FLOAT:  ret = "float"; break;
        case TYPE_DOUBLE: ret = "double"; break;
        case TYPE_INT:    ret = "int"; break;
        case TYPE_BOOL:   ret = "bool"; break;
        case TYPE_VEC3S:  ret = "vec3s"; break;
        case TYPE_VEC3D:  ret = "vec3d"; break;
        case TYPE_VEC3I:  ret = "vec3i"; break;
    }
    return ret;
}

std::string
dataTypeToMenuItems(DataType dts)
{
    std::string ret;
    switch (dts) {
        case TYPE_FLOAT:  ret = "float"; break;
        case TYPE_DOUBLE: ret = "double"; break;
        case TYPE_INT:    ret = "int"; break;
        case TYPE_BOOL:   ret = "bool"; break;
        case TYPE_VEC3S:  ret = "vec3s (float)"; break;
        case TYPE_VEC3D:  ret = "vec3d (double)"; break;
        case TYPE_VEC3I:  ret = "vec3i (int)"; break;
    }
    return ret;
}

DataType
stringToDataType(const std::string& s)
{
    DataType ret = TYPE_FLOAT;
    std::string str = s;
    hboost::trim(str);
    hboost::to_lower(str);
    if (str == dataTypeToString(TYPE_FLOAT)) {
        ret = TYPE_FLOAT;
    } else if (str == dataTypeToString(TYPE_DOUBLE)) {
        ret = TYPE_DOUBLE;
    } else if (str == dataTypeToString(TYPE_INT)) {
        ret = TYPE_INT;
    } else if (str == dataTypeToString(TYPE_BOOL)) {
        ret = TYPE_BOOL;
    } else if (str == dataTypeToString(TYPE_VEC3S)) {
        ret = TYPE_VEC3S;
    } else if (str == dataTypeToString(TYPE_VEC3D)) {
        ret = TYPE_VEC3D;
    } else if (str == dataTypeToString(TYPE_VEC3I)) {
        ret = TYPE_VEC3I;
    }
    return ret;
}

} // unnamed namespace


////////////////////////////////////////


class SOP_OpenVDB_Create : public hvdb::SOP_NodeVDB
{
public:
    SOP_OpenVDB_Create(OP_Network *net, const char *name, OP_Operator *op);
    ~SOP_OpenVDB_Create() override {}

    static OP_Node* factory(OP_Network*, const char* name, OP_Operator*);

    int isRefInput(unsigned) const override { return true; }

    int updateNearFar(float time);
    int updateFarPlane(float time);
    int updateNearPlane(float time);

protected:
    OP_ERROR cookVDBSop(OP_Context&) override;
    bool updateParmsFlags() override;
    void resolveObsoleteParms(PRM_ParmList*) override;

private:
    inline cvdb::Vec3i voxelToIndex(const cvdb::Vec3R& V) const
    {
        return cvdb::Vec3i(cvdb::Int32(V[0]), cvdb::Int32(V[1]), cvdb::Int32(V[2]));
    }

    template<typename GridType>
    void createNewGrid(
        const UT_String& gridNameStr,
        const typename GridType::ValueType& background,
        const cvdb::math::Transform::Ptr&,
        const cvdb::MaskGrid::ConstPtr& maskGrid = nullptr,
        GA_PrimitiveGroup* group = nullptr,
        int gridClass = 0,
        int vecType = -1);

    OP_ERROR buildTransform(OP_Context&, openvdb::math::Transform::Ptr&, const GU_PrimVDB*);
    const GU_PrimVDB* getReferenceVdb(OP_Context &context);
    cvdb::MaskGrid::Ptr createMaskGrid(const GU_PrimVDB*, const openvdb::math::Transform::Ptr&);

    bool mNeedsResampling;
};


////////////////////////////////////////


// Callback functions that update the near and far parameters

int updateNearFarCallback(void*, int, float, const PRM_Template*);
int updateNearPlaneCallback(void*, int, float, const PRM_Template*);
int updateFarPlaneCallback(void*, int, float, const PRM_Template*);


int
updateNearFarCallback(void* data, int /*idx*/, float time, const PRM_Template*)
{
   SOP_OpenVDB_Create* sop = static_cast<SOP_OpenVDB_Create*>(data);
   if (sop == nullptr) return 0;
   return sop->updateNearFar(time);
}


int
SOP_OpenVDB_Create::updateNearFar(float time)
{
    const auto cameraPath = evalStdString("camera", time);
    if (cameraPath.empty()) return 1;

    OBJ_Node *camobj = findOBJNode(cameraPath.c_str());
    if (!camobj) return 1;

    OBJ_Camera* cam = camobj->castToOBJCamera();
    if (!cam) return 1;

    fpreal nearPlane = cam->getNEAR(time);
    fpreal farPlane = cam->getFAR(time);

    setFloat("nearPlane", 0, time, nearPlane);
    setFloat("farPlane", 0, time, farPlane);

    return 1;
}


int
updateNearPlaneCallback(void* data, int /*idx*/, float time, const PRM_Template*)
{
   SOP_OpenVDB_Create* sop = static_cast<SOP_OpenVDB_Create*>(data);
   if (sop == nullptr) return 0;
   return sop->updateNearPlane(time);
}


int
SOP_OpenVDB_Create::updateNearPlane(float time)
{
    fpreal
        nearPlane = evalFloat("nearPlane", 0, time),
        farPlane = evalFloat("farPlane", 0, time),
        voxelDepthSize = evalFloat("voxelDepthSize", 0, time);

    if (!(voxelDepthSize > 0.0)) voxelDepthSize = 1e-6;

    farPlane -= voxelDepthSize;

    if (farPlane < nearPlane) {
        setFloat("nearPlane", 0, time, farPlane);
    }

    return 1;
}


int
updateFarPlaneCallback(void* data, int /*idx*/, float time, const PRM_Template*)
{
   SOP_OpenVDB_Create* sop = static_cast<SOP_OpenVDB_Create*>(data);
   if (sop == nullptr) return 0;
   return sop->updateFarPlane(time);
}


int
SOP_OpenVDB_Create::updateFarPlane(float time)
{
    fpreal
        nearPlane = evalFloat("nearPlane", 0, time),
        farPlane = evalFloat("farPlane", 0, time),
        voxelDepthSize = evalFloat("voxelDepthSize", 0, time);

    if (!(voxelDepthSize > 0.0)) voxelDepthSize = 1e-6;

    nearPlane += voxelDepthSize;

    if (farPlane < nearPlane) {
        setFloat("farPlane", 0, time, nearPlane);
    }

    return 1;
}


////////////////////////////////////////


void
newSopOperator(OP_OperatorTable *table)
{
    if (table == nullptr) return;

    hutil::ParmList parms;


    // Group name
    parms.add(hutil::ParmFactory(PRM_STRING, "group", "Group")
        .setTooltip("Specify a name for this group of VDBs."));

    parms.add(hutil::ParmFactory(PRM_SEPARATOR,"sep1", "Sep"));


    // Transform type
    parms.add(hutil::ParmFactory(PRM_ORD | PRM_TYPE_JOIN_NEXT, "transform", "Transform")
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, {
            "linear",   "Linear",
            "frustum",  "Frustum",
            "refVDB",   "Reference VDB"
        })
        .setTooltip(
            "The type of transform to assign to each VDB\n\n"
            "Linear:\n"
            "   Rotation and scale only\n"
            "Frustum:\n"
            "   Perspective projection, with focal length and near and far planes"
            " from a given camera\n"
            "Reference VDB:\n"
            "   Match the transform of an input VDB."));

    // Toggle to preview the frustum
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "previewFrustum", "Preview")
        .setDefault(PRMoneDefaults)
        .setTooltip("Generate geometry indicating the bounds of the camera frustum.")
        .setDocumentation(
            "For a frustum transform, generate geometry indicating"
            " the bounds of the camera frustum."));

    // Uniform voxel size
    parms.add(hutil::ParmFactory(PRM_FLT_J, "voxelSize", "Voxel Size")
        .setDefault(PRMoneDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 1e-5, PRM_RANGE_UI, 5)
        .setTooltip("The size (length of a side) of a cubic voxel in world units")
        .setTooltip(
            "For non-frustum transforms, the size (length of a side)"
            " of a cubic voxel in world units"));

    // Rotation
    parms.add(hutil::ParmFactory(PRM_XYZ_J, "rotation", "Rotation")
        .setVectorSize(3)
        .setDefault(PRMzeroDefaults)
        .setTooltip("Rotation specified in ZYX order"));

    // Frustum settings
    // {

    parms.add(hutil::ParmFactory(PRM_STRING, "camera", "Camera")
        .setTypeExtended(PRM_TYPE_DYNAMIC_PATH)
        .setCallbackFunc(&updateNearFarCallback)
        .setSpareData(&PRM_SpareData::objCameraPath)
        .setTooltip("The path to the reference camera object (e.g., \"/obj/cam1\")")
        .setDocumentation(
            "For a frustum transform, the path to the reference camera object"
            " (for example, `/obj/cam1`)"));

    parms.add(hutil::ParmFactory(PRM_FLT_J | PRM_TYPE_JOIN_NEXT, "nearPlane", "Near/Far Planes")
        .setDefault(PRMzeroDefaults)
        .setCallbackFunc(&updateNearPlaneCallback)
        .setRange(PRM_RANGE_RESTRICTED, 1e-5, PRM_RANGE_UI, 20)
        .setTooltip("The near and far plane distances in world units")
        .setDocumentation(
            "The near and far plane distances in world units\n\n"
            "The near plane distance should always be <= `farPlane` &minus; `voxelDepthSize`,\n"
            "and the far plane distance should always be => `nearPlane` + `voxelDepthSize`."));

    parms.add(hutil::ParmFactory(
        PRM_FLT_J | PRM_Type(PRM_Type::PRM_INTERFACE_LABEL_NONE), "farPlane", "")
        .setDefault(PRMoneDefaults)
        .setCallbackFunc(&updateFarPlaneCallback)
        .setRange(PRM_RANGE_RESTRICTED, 1e-5, PRM_RANGE_UI, 20)
        .setTooltip("Far plane distance, should always be >= nearPlane + voxelDepthSize")
        .setDocumentation(nullptr));

    parms.add(hutil::ParmFactory(PRM_INT_J, "voxelCount", "Voxel Count")
        .setDefault(PRM100Defaults)
        .setRange(PRM_RANGE_RESTRICTED, 1, PRM_RANGE_UI, 200)
        .setTooltip("The desired width of the near plane in voxels"));

    parms.add(hutil::ParmFactory(PRM_FLT_J, "voxelDepthSize", "Voxel Depth")
        .setDefault(PRMoneDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 1e-5, PRM_RANGE_UI, 5)
        .setTooltip("The z dimension of a voxel in world units (all voxels have the same depth)")
        .setTooltip(
            "For a frustum transform, the z dimension of a voxel"
            " in world units (all voxels have the same depth)"));

    parms.add(hutil::ParmFactory(PRM_FLT_J, "cameraOffset", "Camera Offset")
        .setDefault(PRMzeroDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 0.0, PRM_RANGE_FREE, 20.0)
        .setTooltip(
            "Add padding to the frustum without changing the near and far plane positions.\n\n"
            "The camera position is offset in the direction opposite the view."));

    // }

    // Matching settings
    parms.add(hutil::ParmFactory(PRM_STRING, "reference", "Reference")
        .setChoiceList(&hutil::PrimGroupMenuInput2)
        .setTooltip("The VDB to be used as a reference")
        .setDocumentation(
            "A VDB from the second input to be used as reference"
            " (see [specifying volumes|/model/volumes#group])\n\n"
            "If multiple VDBs are selected, only the first one will be used."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "useVoxelSize", "")
        .setTypeExtended(PRM_TYPE_TOGGLE_JOIN)
        .setDefault(PRMzeroDefaults)
        .setTooltip(
            "If enabled, use the given voxel size, otherwise"
            " match the voxel size of the reference VDB."));

    parms.add(hutil::ParmFactory(PRM_FLT_J, "voxelSizeRef", "Voxel Size")
        .setDefault(PRMoneDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 1e-5, PRM_RANGE_UI, 5)
        .setTooltip("The size (length of a side) of a cubic voxel in world units")
        .setDocumentation(nullptr));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "matchTopology", "Match Topology")
        .setDefault(PRMoneDefaults)
        .setTooltip("Match the voxel topology of the reference VDB."));

    // Grids Heading
    parms.add(hutil::ParmFactory(PRM_HEADING, "gridsHeading", ""));

    // Dynamic grid menu
    hutil::ParmList gridParms;
    {
        {   // Grid class menu
            std::vector<std::string> items;
            for (int i = 0; i < openvdb::NUM_GRID_CLASSES; ++i) {
                openvdb::GridClass cls = openvdb::GridClass(i);
                items.push_back(openvdb::GridBase::gridClassToString(cls)); // token
                items.push_back(openvdb::GridBase::gridClassToMenuName(cls)); // label
            }

            gridParms.add(hutil::ParmFactory(PRM_STRING, "gridClass#", "Class")
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

        {   // Element type menu
            std::vector<std::string> items;
            for (int i = 0; i < NUM_DATA_TYPES; ++i) {
                items.push_back(dataTypeToString(DataType(i))); // token
                items.push_back(dataTypeToMenuItems(DataType(i))); // label
            }
            gridParms.add(hutil::ParmFactory(PRM_STRING, "elementType#", "Type")
                .setChoiceListItems(PRM_CHOICELIST_SINGLE, items)
                .setTooltip("The type of value stored at each voxel")
                .setDocumentation(
                    "The type of value stored at each voxel\n\n"
                    "VDB volumes are able to store vector values, unlike Houdini volumes,\n"
                    "which require one scalar volume for each vector component."));
        }

        // Optional grid name string
        gridParms.add(hutil::ParmFactory(PRM_STRING, "gridName#", "Name")
            .setTooltip("A name for this VDB")
            .setDocumentation("A value for the `name` attribute of this VDB primitive"));

        // Default background values
        // {
        const char* bgHelpStr = "The \"default\" value for any voxel not explicitly set";
        gridParms.add(hutil::ParmFactory(PRM_FLT_J, "bgFloat#", "Background Value")
            .setTooltip(bgHelpStr)
            .setDocumentation(bgHelpStr));
        gridParms.add(hutil::ParmFactory(PRM_INT_J, "bgInt#", "Background Value")
            .setDefault(PRMoneDefaults)
            .setTooltip(bgHelpStr)
            .setDocumentation(nullptr));
        gridParms.add(hutil::ParmFactory(PRM_INT_J, "bgBool#", "Background Value")
            .setRange(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_RESTRICTED, 1)
            .setDefault(PRMoneDefaults)
            .setTooltip(bgHelpStr)
            .setDocumentation(nullptr));
        gridParms.add(hutil::ParmFactory(PRM_FLT_J, "bgVec3f#", "Background Value")
            .setVectorSize(3)
            .setTooltip(bgHelpStr)
            .setDocumentation(nullptr));
        gridParms.add(hutil::ParmFactory(PRM_INT_J, "bgVec3i#", "Background Value")
            .setVectorSize(3)
            .setTooltip(bgHelpStr)
            .setDocumentation(nullptr));
        gridParms.add(hutil::ParmFactory(PRM_FLT_J, "width#", "Half-Band Width")
            .setDefault(PRMthreeDefaults)
            .setRange(PRM_RANGE_RESTRICTED, 1.0, PRM_RANGE_UI, 10)
            .setTooltip(
                "Half the width of the narrow band, in voxels\n\n"
                "(Many level set operations require this to be a minimum of three voxels.)"));
        // }

        // Vec type menu
        {
            std::string help =
                "For vector-valued VDBs, specify an interpretation of the vectors"
                " that determines how they are affected by transforms.\n";
            std::vector<std::string> items;
            for (int i = 0; i < openvdb::NUM_VEC_TYPES ; ++i) {
                const auto vectype = static_cast<openvdb::VecType>(i);
                items.push_back(openvdb::GridBase::vecTypeToString(vectype));
                items.push_back(openvdb::GridBase::vecTypeExamples(vectype));
                help += "\n" + openvdb::GridBase::vecTypeExamples(vectype) + "\n    "
                    + openvdb::GridBase::vecTypeDescription(vectype) + ".";
            }

            gridParms.add(hutil::ParmFactory(PRM_ORD, "vecType#", "Vector Type")
                .setDefault(PRMzeroDefaults)
                .setChoiceListItems(PRM_CHOICELIST_SINGLE, items)
                .setTooltip(help.c_str()));
        }
    }

    parms.add(hutil::ParmFactory(PRM_MULTITYPE_LIST, "gridList", "VDBs")
        .setMultiparms(gridParms)
        .setDefault(PRMoneDefaults));


    // Obsolete parameters
    hutil::ParmList obsoleteParms;
    obsoleteParms.add(hutil::ParmFactory(PRM_HEADING,
        "propertiesHeading", "Shared Grid Properties"));
    obsoleteParms.add(hutil::ParmFactory(PRM_HEADING, "frustumHeading", "Frustum Grid Settings"));
    obsoleteParms.add(hutil::ParmFactory(PRM_FLT_J, "padding", "Padding"));
    obsoleteParms.add(hutil::ParmFactory(PRM_TOGGLE, "matchVoxelSize", "Match Voxel Size"));

    // Register this operator.
    hvdb::OpenVDBOpFactory("VDB Create", SOP_OpenVDB_Create::factory, parms, *table)
        .setObsoleteParms(obsoleteParms)
        .addOptionalInput("Optional Input to Merge With")
        .addOptionalInput("Optional Reference VDB")
        .setDocumentation("\
#icon: COMMON/openvdb\n\
#tags: vdb\n\
\n\
\"\"\"Create one or more empty VDB volume primitives.\"\"\"\n\
\n\
@overview\n\
\n\
[Include:volume_types]\n\
\n\
@related\n\
- [OpenVDB From Particles|Node:sop/DW_OpenVDBFromParticles]\n\
- [OpenVDB From Polygons|Node:sop/DW_OpenVDBFromPolygons]\n\
- [OpenVDB Metadata|Node:sop/DW_OpenVDBMetadata]\n\
- [Node:sop/vdb]\n\
\n\
@examples\n\
\n\
See [openvdb.org|http://www.openvdb.org/download/] for source code\n\
and usage examples.\n");
}


////////////////////////////////////////


OP_Node *
SOP_OpenVDB_Create::factory(OP_Network *net, const char *name, OP_Operator *op)
{
    return new SOP_OpenVDB_Create(net, name, op);
}


SOP_OpenVDB_Create::SOP_OpenVDB_Create(OP_Network *net, const char *name, OP_Operator *op)
    : hvdb::SOP_NodeVDB(net, name, op)
    , mNeedsResampling(false)
{
}


////////////////////////////////////////


bool
SOP_OpenVDB_Create::updateParmsFlags()
{
    bool changed = false;
    UT_String tmpStr;

    const auto transformParm = evalInt("transform", 0, 0);
    const bool linear = (transformParm == 0);
    const bool frustum = (transformParm == 1);
    const bool matching = (transformParm == 2);

    for (int i = 1, N = static_cast<int>(evalInt("gridList", 0, 0)); i <= N; ++i) {

        evalStringInst("gridClass#", &i, tmpStr, 0, 0);
        openvdb::GridClass gridClass = openvdb::GridBase::stringToGridClass(tmpStr.toStdString());

        evalStringInst("elementType#", &i, tmpStr, 0, 0);
        DataType eType = stringToDataType(tmpStr.toStdString());
        bool isLevelSet = false;

        // Force a specific data type for some of the grid classes
        if (gridClass == openvdb::GRID_LEVEL_SET) {
            eType = TYPE_FLOAT;
            isLevelSet = true;
        } else if (gridClass == openvdb::GRID_FOG_VOLUME) {
            eType = TYPE_FLOAT;
        } else if (gridClass == openvdb::GRID_STAGGERED) {
            eType = TYPE_VEC3S;
        }

        /// Disbale unused bg value options
        changed |= enableParmInst("bgFloat#", &i,
            !isLevelSet && (eType == TYPE_FLOAT || eType == TYPE_DOUBLE));
        changed |= enableParmInst("width#",   &i, isLevelSet);
        changed |= enableParmInst("bgInt#",   &i, eType == TYPE_INT || eType == TYPE_BOOL);
        changed |= enableParmInst("bgVec3f#", &i, eType == TYPE_VEC3S || eType == TYPE_VEC3D);
        changed |= enableParmInst("bgVec3i#", &i, eType == TYPE_VEC3I);
        changed |= enableParmInst("vecType#", &i, eType >= TYPE_VEC3S);

        // Hide unused bg value options.
        changed |= setVisibleStateInst("bgFloat#", &i,
            !isLevelSet && (eType == TYPE_FLOAT || eType == TYPE_DOUBLE));
        changed |= setVisibleStateInst("width#",   &i, isLevelSet);
        changed |= setVisibleStateInst("bgInt#",   &i, eType == TYPE_INT);
        changed |= setVisibleStateInst("bgBool#",  &i, eType == TYPE_BOOL);
        changed |= setVisibleStateInst("bgVec3f#", &i, eType == TYPE_VEC3S || eType == TYPE_VEC3D);
        changed |= setVisibleStateInst("bgVec3i#", &i, eType == TYPE_VEC3I);
        changed |= setVisibleStateInst("vecType#", &i, eType >= TYPE_VEC3S);

        // Enable different data types
        changed |= enableParmInst("elementType#", &i, gridClass == openvdb::GRID_UNKNOWN);
        changed |= setVisibleStateInst("elementType#", &i, gridClass == openvdb::GRID_UNKNOWN);
    }

    // linear transform and voxel size
    changed |= enableParm("voxelSize", linear);
    changed |= enableParm("rotation", linear);

    changed |= setVisibleState("voxelSize", linear);
    changed |= setVisibleState("rotation", linear);

    // frustum transform
    const auto cameraPath = evalStdString("camera", 0);
    const bool enableFrustumSettings = (!cameraPath.empty() && findOBJNode(cameraPath.c_str()));

    changed |= enableParm("camera", frustum);
    changed |= enableParm("voxelCount", frustum & enableFrustumSettings);
    changed |= enableParm("voxelDepthSize", frustum & enableFrustumSettings);
    changed |= enableParm("offset", frustum & enableFrustumSettings);
    changed |= enableParm("nearPlane", frustum & enableFrustumSettings);
    changed |= enableParm("farPlane", frustum & enableFrustumSettings);
    changed |= enableParm("cameraOffset", frustum & enableFrustumSettings);
    changed |= enableParm("previewFrustum", frustum & enableFrustumSettings);

    changed |= setVisibleState("camera", frustum);
    changed |= setVisibleState("voxelCount", frustum);
    changed |= setVisibleState("voxelDepthSize", frustum);
    changed |= setVisibleState("offset", frustum);
    changed |= setVisibleState("nearPlane", frustum);
    changed |= setVisibleState("farPlane", frustum);
    changed |= setVisibleState("cameraOffset", frustum);
    changed |= setVisibleState("previewFrustum", frustum);

    // matching

    const bool useVoxelSize = evalInt("useVoxelSize", 0, 0);

    changed |= enableParm("reference", matching);
    changed |= enableParm("useVoxelSize", matching);
    changed |= enableParm("voxelSizeRef", matching && useVoxelSize);
    changed |= enableParm("matchTopology", matching);

    changed |= setVisibleState("reference", matching);
    changed |= setVisibleState("useVoxelSize", matching);
    changed |= setVisibleState("voxelSizeRef", matching);
    changed |= setVisibleState("matchTopology", matching);
    changed |= setVisibleState("matchTopologyPlaceholder", false);

    return changed;
}


void
SOP_OpenVDB_Create::resolveObsoleteParms(PRM_ParmList* obsoleteParms)
{
    if (!obsoleteParms) return;
    PRM_Parm* parm = obsoleteParms->getParmPtr("matchVoxelSize");
    if (parm && !parm->isFactoryDefault()) {
        const bool matchVoxelSize = obsoleteParms->evalInt("matchVoxelSize", 0, /*time=*/0.0);
        setInt("useVoxelSize", 0, 0.0, !matchVoxelSize);
    }
    // Delegate to the base class.
    hvdb::SOP_NodeVDB::resolveObsoleteParms(obsoleteParms);
}


////////////////////////////////////////


template<typename GridType>
void
SOP_OpenVDB_Create::createNewGrid(
    const UT_String& gridNameStr,
    const typename GridType::ValueType& background,
    const cvdb::math::Transform::Ptr& transform,
    const cvdb::MaskGrid::ConstPtr& maskGrid,
    GA_PrimitiveGroup* group,
    int gridClass,
    int vecType)
{
    using Tree = typename GridType::TreeType;
    // Create a grid of a pre-registered type and assign it a transform.
    hvdb::GridPtr newGrid;
    if (maskGrid) {
        newGrid = GridType::create(
            typename Tree::Ptr(new Tree(maskGrid->tree(), background, cvdb::TopologyCopy())));
    } else {
        newGrid = GridType::create(background);
    }
    newGrid->setTransform(transform);

    newGrid->setGridClass(openvdb::GridClass(gridClass));
    if (vecType != -1) newGrid->setVectorType(openvdb::VecType(vecType));

    // Store the grid in a new VDB primitive and add the primitive
    // to the output geometry detail.
    GEO_PrimVDB* vdb = hvdb::createVdbPrimitive(*gdp, newGrid,
        gridNameStr.toStdString().c_str());

    // Add the primitive to the group.
    if (group) group->add(vdb);
}


////////////////////////////////////////


OP_ERROR
SOP_OpenVDB_Create::cookVDBSop(OP_Context &context)
{
    try {
        hutil::ScopedInputLock lock(*this, context);

        gdp->clearAndDestroy();
        if (getInput(0)) duplicateSource(0, context);

        fpreal time = context.getTime();

        // Create a group for the grid primitives.
        const auto groupStr = evalStdString("group", time);
        GA_PrimitiveGroup* group = (groupStr.empty() ?
            nullptr : gdp->newPrimitiveGroup(groupStr.c_str()));

        // Get reference VDB, if exists
        const bool matchTransfom = (evalInt("transform", 0, time) == 2);
        const GU_PrimVDB* refVdb = (matchTransfom ? getReferenceVdb(context) : nullptr);

        // Create a shared transform
        cvdb::math::Transform::Ptr transform;
        if (buildTransform(context, transform, refVdb) >= UT_ERROR_ABORT) return error();

        cvdb::MaskGrid::Ptr maskGrid;
        const bool matchTopology = evalInt("matchTopology", 0, time);
        if (matchTransfom && matchTopology)
            maskGrid = createMaskGrid(refVdb, transform);

        // Create the grids
        UT_String gridNameStr, tmpStr;

        for (int i = 1, N = static_cast<int>(evalInt("gridList", 0, 0)); i <= N; ++i) {

            evalStringInst("gridName#", &i, gridNameStr, 0, time);

            evalStringInst("gridClass#", &i, tmpStr, 0, time);
            openvdb::GridClass gridClass =
                openvdb::GridBase::stringToGridClass(tmpStr.toStdString());

            evalStringInst("elementType#", &i, tmpStr, 0, time);
            DataType eType = stringToDataType(tmpStr.toStdString());

            // Force a specific data type for some of the grid classes
            if (gridClass == openvdb::GRID_LEVEL_SET ||
                gridClass == openvdb::GRID_FOG_VOLUME) {
                eType = TYPE_FLOAT;
            } else if (gridClass == openvdb::GRID_STAGGERED) {
                eType = TYPE_VEC3S;
            }

            switch(eType) {
                case TYPE_FLOAT:
                {
                    float voxelSize = float(transform->voxelSize()[0]);
                    float background = 0.0;

                    if (gridClass == openvdb::GRID_LEVEL_SET) {
                        background = float(evalFloatInst("width#", &i, 0, time) * voxelSize);
                    } else {
                        background = float(evalFloatInst("bgFloat#", &i, 0, time));
                    }

                    createNewGrid<cvdb::FloatGrid>(
                        gridNameStr, background, transform, maskGrid, group, gridClass);
                    break;
                }
                case TYPE_DOUBLE:
                {
                    double background = double(evalFloatInst("bgFloat#", &i, 0, time));
                    createNewGrid<cvdb::DoubleGrid>(
                        gridNameStr, background, transform, maskGrid, group, gridClass);
                    break;
                }
                case TYPE_INT:
                {
                    int background = static_cast<int>(evalIntInst("bgInt#", &i, 0, time));
                    createNewGrid<cvdb::Int32Grid>(
                        gridNameStr, background, transform, maskGrid, group, gridClass);
                    break;
                }
                case TYPE_BOOL:
                {
                    bool background = evalIntInst("bgBool#", &i, 0, time);
                    createNewGrid<cvdb::BoolGrid>(
                        gridNameStr, background, transform, maskGrid, group, gridClass);
                    break;
                }
                case TYPE_VEC3S:
                {
                    cvdb::Vec3f background(
                        float(evalFloatInst("bgVec3f#", &i, 0, time)),
                        float(evalFloatInst("bgVec3f#", &i, 1, time)),
                        float(evalFloatInst("bgVec3f#", &i, 2, time)));

                    int vecType = static_cast<int>(evalIntInst("vecType#", &i, 0, time));

                    createNewGrid<cvdb::Vec3SGrid>(
                        gridNameStr, background, transform, maskGrid, group, gridClass, vecType);
                    break;
                }
                case TYPE_VEC3D:
                {
                    cvdb::Vec3d background(
                        double(evalFloatInst("bgVec3f#", &i, 0, time)),
                        double(evalFloatInst("bgVec3f#", &i, 1, time)),
                        double(evalFloatInst("bgVec3f#", &i, 2, time)));

                    int vecType = static_cast<int>(evalIntInst("vecType#", &i, 0, time));

                    createNewGrid<cvdb::Vec3DGrid>(
                        gridNameStr, background, transform, maskGrid, group, gridClass, vecType);
                    break;
                }
                case TYPE_VEC3I:
                {
                    cvdb::Vec3i background(
                        static_cast<cvdb::Int32>(evalIntInst("bgVec3i#", &i, 0, time)),
                        static_cast<cvdb::Int32>(evalIntInst("bgVec3i#", &i, 1, time)),
                        static_cast<cvdb::Int32>(evalIntInst("bgVec3i#", &i, 2, time)));
                    int vecType = static_cast<int>(evalIntInst("vecType#", &i, 0, time));
                    createNewGrid<cvdb::Vec3IGrid>(
                        gridNameStr, background, transform, maskGrid, group, gridClass, vecType);
                    break;
                }
            } // eType switch
        } // grid create loop

    } catch ( std::exception& e) {
        addError(SOP_MESSAGE, e.what());
    }

    return error();
}


////////////////////////////////////////


OP_ERROR
SOP_OpenVDB_Create::buildTransform(OP_Context& context, openvdb::math::Transform::Ptr& transform,
        const GU_PrimVDB* refVdb)
{
    fpreal time = context.getTime();
    const auto transformParm = evalInt("transform", 0, time);
    const bool linear = (transformParm == 0);
    const bool frustum = (transformParm == 1);

    if (frustum) { // nonlinear frustum transform

        const auto cameraPath = evalStdString("camera", time);
        if (cameraPath.empty()) {
            addError(SOP_MESSAGE, "No camera selected");
            return error();
        }

        OBJ_Node *camobj = findOBJNode(cameraPath.c_str());
        if (!camobj) {
            addError(SOP_MESSAGE, "Camera not found");
            return error();
        }

        OBJ_Camera* cam = camobj->castToOBJCamera();
        if (!cam) {
            addError(SOP_MESSAGE, "Camera not found");
            return error();
        }

        // Register
        this->addExtraInput(cam, OP_INTEREST_DATA);

        const float
            offset = static_cast<float>(evalFloat("cameraOffset", 0, time)),
            nearPlane = static_cast<float>(evalFloat("nearPlane", 0, time)),
            farPlane = static_cast<float>(evalFloat("farPlane", 0, time)),
            voxelDepthSize = static_cast<float>(evalFloat("voxelDepthSize", 0, time));
        const int voxelCount = static_cast<int>(evalInt("voxelCount", 0, time));

        transform = hvdb::frustumTransformFromCamera(*this, context, *cam,
            offset, nearPlane, farPlane, voxelDepthSize, voxelCount);

        if (bool(evalInt("previewFrustum", 0, time))) {
            UT_Vector3 boxColor(0.6f, 0.6f, 0.6f);
            UT_Vector3 tickColor(0.0f, 0.0f, 0.0f);
            hvdb::drawFrustum(*gdp, *transform,
                &boxColor, &tickColor, /*shaded*/true);
        }

    } else if (linear) { // linear affine transform

        const double voxelSize = double(evalFloat("voxelSize", 0, time));

        openvdb::Vec3d rotation(
            evalFloat("rotation", 0, time),
            evalFloat("rotation", 1, time),
            evalFloat("rotation", 2, time));

        if (std::abs(rotation.x()) < 0.00001 && std::abs(rotation.y()) < 0.00001
            && std::abs(rotation.z()) < 0.00001) {
            transform = openvdb::math::Transform::createLinearTransform(voxelSize);
        } else {

            openvdb::math::Mat4d xform(openvdb::math::Mat4d::identity());

            xform.preRotate(openvdb::math::X_AXIS, rotation.x());
            xform.preRotate(openvdb::math::Y_AXIS, rotation.y());
            xform.preRotate(openvdb::math::Z_AXIS, rotation.z());
            xform.preScale(openvdb::Vec3d(voxelSize));

            transform = openvdb::math::Transform::createLinearTransform(xform);
        }
    } else { // match reference
        if (refVdb == nullptr) {
            addError(SOP_MESSAGE, "Missing reference grid");
            return error();
        }
        transform = refVdb->getGrid().transform().copy();
        const bool useVoxelSize = evalInt("useVoxelSize", 0, time);
        if (useVoxelSize) { // NOT matching the reference's voxel size
            if (!transform->isLinear()) {
                addError(SOP_MESSAGE, "Cannot change voxel size on a non-linear transform");
                return error();
            }
            const double voxelSize = double(evalFloat("voxelSizeRef", 0, time));
            openvdb::Vec3d relativeVoxelScale = voxelSize / refVdb->getGrid().voxelSize();
            // If the user is changing the voxel size to the original,
            // then there is no need to do anything
            if (!isApproxEqual(openvdb::Vec3d::ones(), relativeVoxelScale)) {
                mNeedsResampling = true;
                transform->preScale(relativeVoxelScale);
            }
        }
    }

    return error();
}


////////////////////////////////////////


const GU_PrimVDB*
SOP_OpenVDB_Create::getReferenceVdb(OP_Context &context)
{
    const GU_Detail* refGdp = inputGeo(1, context);
    if (!refGdp) return nullptr;

    const GA_PrimitiveGroup* refGroup = matchGroup(
        *refGdp, evalStdString("reference", context.getTime()));

    hvdb::VdbPrimCIterator vdbIter(refGdp, refGroup);
    const GU_PrimVDB* refVdb = *vdbIter;
    if (++vdbIter) {
        addWarning(SOP_MESSAGE, "Multiple reference grids were found.\n"
           "Using the first one for reference.");
    }
    return refVdb;
}


////////////////////////////////////////


class GridConvertToMask {
public:
    GridConvertToMask(cvdb::MaskGrid::Ptr& maskGrid) : outGrid(maskGrid) {}

    template<typename GridType>
    void operator()(const GridType& inGrid)
    {
        using MaskTree = cvdb::MaskGrid::TreeType;
        outGrid = cvdb::MaskGrid::create(
                MaskTree::Ptr(new MaskTree(inGrid.tree(), 0, cvdb::TopologyCopy())));
    }
private:
    cvdb::MaskGrid::Ptr& outGrid;
};

cvdb::MaskGrid::Ptr
SOP_OpenVDB_Create::createMaskGrid(const GU_PrimVDB* refVdb,
        const openvdb::math::Transform::Ptr& transform)
{
    if (refVdb == nullptr)
        throw std::runtime_error("Missing reference grid");

    cvdb::MaskGrid::Ptr maskGrid;
    GridConvertToMask op(maskGrid);
    GEOvdbProcessTypedGridTopology(*refVdb, op);
    maskGrid->setTransform(refVdb->getGrid().transform().copy());

    if (!mNeedsResampling)
        return maskGrid;

    cvdb::MaskGrid::Ptr resampledMaskGrid = cvdb::MaskGrid::create();
    resampledMaskGrid->setTransform(transform);

    hvdb::Interrupter interrupter;
    cvdb::tools::resampleToMatch<cvdb::tools::PointSampler>(*maskGrid, *resampledMaskGrid,
            interrupter);

    return resampledMaskGrid;
}
