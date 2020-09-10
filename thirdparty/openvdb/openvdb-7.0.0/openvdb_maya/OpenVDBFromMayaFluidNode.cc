// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

/// @author FX R&D OpenVDB team

#include "OpenVDBPlugin.h"
#include <openvdb_maya/OpenVDBData.h>

#include <openvdb/tools/Dense.h>
#include <openvdb/math/Transform.h>

#include <maya/MFnUnitAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MSelectionList.h>
#include <maya/MFnFluid.h>
#include <maya/MFnStringData.h>
#include <maya/MFnPluginData.h>
#include <maya/MGlobal.h>
#include <maya/MMatrix.h>
#include <maya/MPoint.h>
#include <maya/MBoundingBox.h>
#include <maya/MFnNumericAttribute.h>

namespace mvdb = openvdb_maya;


struct OpenVDBFromMayaFluidNode : public MPxNode
{
public:
    OpenVDBFromMayaFluidNode() {}
    virtual ~OpenVDBFromMayaFluidNode() {}

    virtual MStatus compute(const MPlug& plug, MDataBlock& data);

    static void* creator();
    static MStatus initialize();

    static MTypeId id;
    static MObject aFluidNodeName;
    static MObject aVdbOutput;

    static MObject aDensity;
    static MObject aDensityName;
    static MObject aTemperature;
    static MObject aTemperatureName;
    static MObject aPressure;
    static MObject aPressureName;
    static MObject aFuel;
    static MObject aFuelName;
    static MObject aFalloff;
    static MObject aFalloffName;
    static MObject aVelocity;
    static MObject aVelocityName;
    static MObject aColors;
    static MObject aColorsName;
    static MObject aCoordinates;
    static MObject aCoordinatesName;
    static MObject aTime;
};


MTypeId OpenVDBFromMayaFluidNode::id(0x00108A55);
MObject OpenVDBFromMayaFluidNode::aFluidNodeName;
MObject OpenVDBFromMayaFluidNode::aVdbOutput;

MObject OpenVDBFromMayaFluidNode::aDensity;
MObject OpenVDBFromMayaFluidNode::aDensityName;
MObject OpenVDBFromMayaFluidNode::aTemperature;
MObject OpenVDBFromMayaFluidNode::aTemperatureName;
MObject OpenVDBFromMayaFluidNode::aPressure;
MObject OpenVDBFromMayaFluidNode::aPressureName;
MObject OpenVDBFromMayaFluidNode::aFuel;
MObject OpenVDBFromMayaFluidNode::aFuelName;
MObject OpenVDBFromMayaFluidNode::aFalloff;
MObject OpenVDBFromMayaFluidNode::aFalloffName;
MObject OpenVDBFromMayaFluidNode::aVelocity;
MObject OpenVDBFromMayaFluidNode::aVelocityName;
MObject OpenVDBFromMayaFluidNode::aColors;
MObject OpenVDBFromMayaFluidNode::aColorsName;
MObject OpenVDBFromMayaFluidNode::aCoordinates;
MObject OpenVDBFromMayaFluidNode::aCoordinatesName;
MObject OpenVDBFromMayaFluidNode::aTime;

namespace {
    mvdb::NodeRegistry registerNode("OpenVDBFromMayaFluid", OpenVDBFromMayaFluidNode::id,
        OpenVDBFromMayaFluidNode::creator, OpenVDBFromMayaFluidNode::initialize);
}


////////////////////////////////////////


void* OpenVDBFromMayaFluidNode::creator()
{
    return new OpenVDBFromMayaFluidNode();
}


MStatus OpenVDBFromMayaFluidNode::initialize()
{
    MStatus stat;
    MFnTypedAttribute tAttr;
    MFnNumericAttribute nAttr;
    MFnStringData strData;


    aFluidNodeName = tAttr.create("FluidNodeName", "fluid", MFnData::kString,
        strData.create("fluidShape"), &stat);
    if (stat != MS::kSuccess) return stat;
    tAttr.setConnectable(false);
    stat = addAttribute(aFluidNodeName);
    if (stat != MS::kSuccess) return stat;


    aDensity = nAttr.create("Density", "d", MFnNumericData::kBoolean);
    nAttr.setDefault(true);
    nAttr.setConnectable(false);
    stat = addAttribute(aDensity);
    if (stat != MS::kSuccess) return stat;

    aDensityName = tAttr.create("DensityName", "dname", MFnData::kString,
        strData.create("density"), &stat);
    if (stat != MS::kSuccess) return stat;
    tAttr.setConnectable(false);
    stat = addAttribute(aDensityName);
    if (stat != MS::kSuccess) return stat;


    aTemperature = nAttr.create("Temperature", "t", MFnNumericData::kBoolean);
    nAttr.setDefault(false);
    nAttr.setConnectable(false);
    stat = addAttribute(aTemperature);
    if (stat != MS::kSuccess) return stat;

    aTemperatureName = tAttr.create("TemperatureName", "tname", MFnData::kString,
        strData.create("temperature"), &stat);
    if (stat != MS::kSuccess) return stat;
    tAttr.setConnectable(false);
    stat = addAttribute(aTemperatureName);
    if (stat != MS::kSuccess) return stat;


    aPressure = nAttr.create("Pressure", "p", MFnNumericData::kBoolean);
    nAttr.setDefault(false);
    nAttr.setConnectable(false);
    stat = addAttribute(aPressure);
    if (stat != MS::kSuccess) return stat;

    aPressureName = tAttr.create("PressureName", "pname", MFnData::kString,
        strData.create("pressure"), &stat);
    if (stat != MS::kSuccess) return stat;
    tAttr.setConnectable(false);
    stat = addAttribute(aPressureName);
    if (stat != MS::kSuccess) return stat;


    aFuel = nAttr.create("Fuel", "f", MFnNumericData::kBoolean);
    nAttr.setDefault(false);
    nAttr.setConnectable(false);
    stat = addAttribute(aFuel);
    if (stat != MS::kSuccess) return stat;

    aFuelName = tAttr.create("FuelName", "fname", MFnData::kString,
        strData.create("fuel"), &stat);
    if (stat != MS::kSuccess) return stat;
    tAttr.setConnectable(false);
    stat = addAttribute(aFuelName);
    if (stat != MS::kSuccess) return stat;


    aFalloff = nAttr.create("Falloff", "falloff", MFnNumericData::kBoolean);
    nAttr.setDefault(false);
    nAttr.setConnectable(false);
    stat = addAttribute(aFalloff);
    if (stat != MS::kSuccess) return stat;

    aFalloffName = tAttr.create("FalloffName", "falloffname", MFnData::kString,
        strData.create("falloff"), &stat);
    if (stat != MS::kSuccess) return stat;
    tAttr.setConnectable(false);
    stat = addAttribute(aFalloffName);
    if (stat != MS::kSuccess) return stat;


    aVelocity = nAttr.create("Velocity", "v", MFnNumericData::kBoolean);
    nAttr.setDefault(false);
    nAttr.setConnectable(false);
    stat = addAttribute(aVelocity);
    if (stat != MS::kSuccess) return stat;

    aVelocityName = tAttr.create("VelocityName", "vname", MFnData::kString,
        strData.create("velocity"), &stat);
    if (stat != MS::kSuccess) return stat;
    tAttr.setConnectable(false);
    stat = addAttribute(aVelocityName);
    if (stat != MS::kSuccess) return stat;


    aColors = nAttr.create("Color", "cd", MFnNumericData::kBoolean);
    nAttr.setDefault(false);
    nAttr.setConnectable(false);
    stat = addAttribute(aColors);
    if (stat != MS::kSuccess) return stat;

    aColorsName = tAttr.create("ColorsName", "cdname", MFnData::kString,
        strData.create("color"), &stat);
    if (stat != MS::kSuccess) return stat;
    tAttr.setConnectable(false);
    stat = addAttribute(aColorsName);
    if (stat != MS::kSuccess) return stat;


    aCoordinates = nAttr.create("Coordinates", "uvw", MFnNumericData::kBoolean);
    nAttr.setDefault(false);
    nAttr.setConnectable(false);
    stat = addAttribute(aCoordinates);
    if (stat != MS::kSuccess) return stat;

    aCoordinatesName = tAttr.create("CoordinatesName", "uvwname", MFnData::kString,
        strData.create("coordinates"), &stat);
    if (stat != MS::kSuccess) return stat;
    tAttr.setConnectable(false);
    stat = addAttribute(aCoordinatesName);
    if (stat != MS::kSuccess) return stat;

    MFnUnitAttribute uniAttr;
    aTime = uniAttr.create("CurrentTime", "time", MFnUnitAttribute::kTime,  0.0, &stat);
    if (stat != MS::kSuccess) return stat;
    stat = addAttribute(aTime);
    if (stat != MS::kSuccess) return stat;


    // Setup the output vdb
    aVdbOutput = tAttr.create("VdbOutput", "vdb", OpenVDBData::id, MObject::kNullObj, &stat);
    if (stat != MS::kSuccess) return stat;

    tAttr.setWritable(false);
    tAttr.setStorable(false);
    stat = addAttribute(aVdbOutput);
    if (stat != MS::kSuccess) return stat;

    // Set the attribute dependencies
    stat = attributeAffects(message, aVdbOutput);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aFluidNodeName, aVdbOutput);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aDensity, aVdbOutput);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aDensityName, aVdbOutput);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aTemperature, aVdbOutput);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aTemperatureName, aVdbOutput);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aPressure, aVdbOutput);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aPressureName, aVdbOutput);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aFuel, aVdbOutput);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aFuelName, aVdbOutput);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aFalloff, aVdbOutput);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aFalloffName, aVdbOutput);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aVelocity, aVdbOutput);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aVelocityName, aVdbOutput);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aColors, aVdbOutput);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aColorsName, aVdbOutput);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aCoordinates, aVdbOutput);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aCoordinatesName, aVdbOutput);
    if (stat != MS::kSuccess) return stat;


    stat = attributeAffects(aTime, aVdbOutput);
    if (stat != MS::kSuccess) return stat;

    return MS::kSuccess;
}


////////////////////////////////////////


namespace internal {

inline openvdb::math::Transform::Ptr
getTransform(const MFnFluid& fluid)
{
    // Construct local transform
    openvdb::Vec3I res;
    fluid.getResolution(res[0], res[1], res[2]);

    openvdb::Vec3d dim;
    fluid.getDimensions(dim[0], dim[1], dim[2]);

    if (res[0] > 0) dim[0] /= double(res[0]);
    if (res[1] > 0) dim[1] /= double(res[1]);
    if (res[2] > 0) dim[2] /= double(res[2]);

    MBoundingBox bbox = fluid.boundingBox();
    MPoint pos = bbox.min();

    openvdb::Mat4R mat1(dim[0], 0.0,    0.0,    0.0,
                        0.0,    dim[1], 0.0,    0.0,
                        0.0,    0.0,    dim[2], 0.0,
                        pos.x,  pos.y,  pos.z,  1.0);

    // Get node transform
    MMatrix mm;

    MStatus status;
    MObject parent = fluid.parent(0, &status);

    if (status == MS::kSuccess) {
        MFnDagNode parentNode(parent);
        mm = parentNode.transformationMatrix();
    }

    openvdb::Mat4R mat2(mm(0,0), mm(0,1), mm(0,2), mm(0,3),
                        mm(1,0), mm(1,1), mm(1,2), mm(1,3),
                        mm(2,0), mm(2,1), mm(2,2), mm(2,3),
                        mm(3,0), mm(3,1), mm(3,2), mm(3,3));

    return openvdb::math::Transform::createLinearTransform(mat1 * mat2);
}

template<typename value_t>
void copyGrid(OpenVDBData& vdb, const std::string& name, const openvdb::math::Transform& xform,
     const value_t* data, const openvdb::CoordBBox& bbox, value_t bg, value_t tol)
{
    if (data != NULL) {
        openvdb::FloatGrid::Ptr grid = openvdb::FloatGrid::create(bg);
        openvdb::tools::Dense<const value_t, openvdb::tools::LayoutXYZ> dense(bbox, data);
        openvdb::tools::copyFromDense(dense, grid->tree(), tol);

        grid->setName(name);
        grid->setTransform(xform.copy());
        vdb.insert(grid);
    }
}

}; // namespace internal


////////////////////////////////////////


MStatus OpenVDBFromMayaFluidNode::compute(const MPlug& plug, MDataBlock& data)
{
    MStatus status;

    if (plug == aVdbOutput) {

        data.inputValue(aTime, &status);

        MString nodeName = data.inputValue(aFluidNodeName, &status).asString();
        MObject fluidNode;

        MSelectionList selectionList;
        selectionList.add(nodeName);

        if (selectionList.length() != 0) {
            selectionList.getDependNode(0, fluidNode);
        }

        if (fluidNode.isNull()) {
            MGlobal::displayError("There is no fluid node with the given name.");
            return MS::kFailure;
        }

        if (!fluidNode.hasFn(MFn::kFluid)) {
            MGlobal::displayError("The named node is not a fluid.");
            return MS::kFailure;
        }

        MFnFluid fluid(fluidNode);
        if (fluid.object() == MObject::kNullObj) return MS::kFailure;


        openvdb::math::Transform::Ptr xform = internal::getTransform(fluid);

        unsigned int xRes, yRes, zRes;
        fluid.getResolution(xRes, yRes, zRes);

        // inclusive bbox
        openvdb::CoordBBox bbox(openvdb::Coord(0), openvdb::Coord(xRes-1, yRes-1, zRes-1));

        // get output vdb
        MFnPluginData outputDataCreators;
        outputDataCreators.create(OpenVDBData::id, &status);
        if (status != MS::kSuccess) return status;

        OpenVDBData* vdb = static_cast<OpenVDBData*>(outputDataCreators.data(&status));
        if (status != MS::kSuccess) return status;


        // convert fluid data

        if (data.inputValue(aDensity, &status).asBool()) {
            const std::string name = data.inputValue(aDensityName, &status).asString().asChar();
            internal::copyGrid(*vdb, name, *xform, fluid.density(), bbox, 0.0f, 1e-7f);
        }

        if (data.inputValue(aTemperature, &status).asBool()) {
            const std::string name = data.inputValue(aTemperatureName, &status).asString().asChar();
            internal::copyGrid(*vdb, name, *xform, fluid.temperature(), bbox, 0.0f, 1e-7f);
        }

        if (data.inputValue(aPressure, &status).asBool()) {
            const std::string name = data.inputValue(aPressureName, &status).asString().asChar();
            internal::copyGrid(*vdb, name, *xform, fluid.pressure(), bbox, 0.0f, 1e-7f);
        }

        if (data.inputValue(aFuel, &status).asBool()) {
            const std::string name = data.inputValue(aFuelName, &status).asString().asChar();
            internal::copyGrid(*vdb, name, *xform, fluid.fuel(), bbox, 0.0f, 1e-7f);
        }

        if (data.inputValue(aFalloff, &status).asBool()) {
            const std::string name = data.inputValue(aFalloffName, &status).asString().asChar();
            internal::copyGrid(*vdb, name, *xform, fluid.falloff(), bbox, 0.0f, 1e-7f);
        }

        if (data.inputValue(aVelocity, &status).asBool()) {
            const std::string name = data.inputValue(aVelocityName, &status).asString().asChar();

            float *xgrid, *ygrid, *zgrid;
            fluid.getVelocity(xgrid, ygrid, zgrid);

            internal::copyGrid(*vdb, name + "_x", *xform, xgrid, bbox, 0.0f, 1e-7f);
            internal::copyGrid(*vdb, name + "_y", *xform, ygrid, bbox, 0.0f, 1e-7f);
            internal::copyGrid(*vdb, name + "_z", *xform, zgrid, bbox, 0.0f, 1e-7f);
        }

        if (data.inputValue(aColors, &status).asBool()) {
            const std::string name = data.inputValue(aColorsName, &status).asString().asChar();

            float *xgrid, *ygrid, *zgrid;
            fluid.getColors(xgrid, ygrid, zgrid);

            internal::copyGrid(*vdb, name + "_r", *xform, xgrid, bbox, 0.0f, 1e-7f);
            internal::copyGrid(*vdb, name + "_g", *xform, ygrid, bbox, 0.0f, 1e-7f);
            internal::copyGrid(*vdb, name + "_b", *xform, zgrid, bbox, 0.0f, 1e-7f);
        }

        if (data.inputValue(aCoordinates, &status).asBool()) {
            const std::string name = data.inputValue(aCoordinatesName, &status).asString().asChar();

            float *xgrid, *ygrid, *zgrid;
            fluid.getCoordinates(xgrid, ygrid, zgrid);

            internal::copyGrid(*vdb, name + "_u", *xform, xgrid, bbox, 0.0f, 1e-7f);
            internal::copyGrid(*vdb, name + "_v", *xform, ygrid, bbox, 0.0f, 1e-7f);
            internal::copyGrid(*vdb, name + "_w", *xform, zgrid, bbox, 0.0f, 1e-7f);
        }


        // export grids
        MDataHandle outHandle = data.outputValue(aVdbOutput);
        outHandle.set(vdb);

        return data.setClean(plug);
    }

    return MS::kUnknownParameter;
}
