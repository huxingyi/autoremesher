// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

/// @file OpenVDBTransformNode.cc
/// @author FX R&D OpenVDB team

#include "OpenVDBPlugin.h"
#include <openvdb_maya/OpenVDBData.h>
#include <openvdb_maya/OpenVDBUtil.h>

#include <maya/MFnTypedAttribute.h>
#include <maya/MFnStringData.h>
#include <maya/MFnPluginData.h>
#include <maya/MGlobal.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFloatVector.h>

#include <boost/math/constants/constants.hpp> // boost::math::constants::pi


namespace mvdb = openvdb_maya;


////////////////////////////////////////


struct OpenVDBTransformNode : public MPxNode
{
    OpenVDBTransformNode() {}
    virtual ~OpenVDBTransformNode() {}

    virtual MStatus compute(const MPlug& plug, MDataBlock& data);

    static void* creator();
    static MStatus initialize();

    static MTypeId id;
    static MObject aVdbInput;
    static MObject aVdbOutput;
    static MObject aVdbSelectedGridNames;
    static MObject aTranslate;
    static MObject aRotate;
    static MObject aScale;
    static MObject aPivot;
    static MObject aUniformScale;
    static MObject aInvert;
};


MTypeId OpenVDBTransformNode::id(0x00108A57);
MObject OpenVDBTransformNode::aVdbOutput;
MObject OpenVDBTransformNode::aVdbInput;
MObject OpenVDBTransformNode::aVdbSelectedGridNames;
MObject OpenVDBTransformNode::aTranslate;
MObject OpenVDBTransformNode::aRotate;
MObject OpenVDBTransformNode::aScale;
MObject OpenVDBTransformNode::aPivot;
MObject OpenVDBTransformNode::aUniformScale;
MObject OpenVDBTransformNode::aInvert;


namespace {
    mvdb::NodeRegistry registerNode("OpenVDBTransform", OpenVDBTransformNode::id,
        OpenVDBTransformNode::creator, OpenVDBTransformNode::initialize);
}


////////////////////////////////////////


void* OpenVDBTransformNode::creator()
{
    return new OpenVDBTransformNode();
}


MStatus OpenVDBTransformNode::initialize()
{
    MStatus stat;

    // attributes

    MFnTypedAttribute tAttr;
    MFnStringData strData;

    aVdbSelectedGridNames = tAttr.create("SelectedGridNames", "grids", MFnData::kString, strData.create("*"), &stat);
    if (stat != MS::kSuccess) return stat;

    tAttr.setConnectable(false);
    stat = addAttribute(aVdbSelectedGridNames);
    if (stat != MS::kSuccess) return stat;


    MFnNumericAttribute nAttr;

    aTranslate = nAttr.createPoint("Translate", "t", &stat);
    if (stat != MS::kSuccess) return stat;
    nAttr.setDefault(0.0, 0.0, 0.0);
    stat = addAttribute(aTranslate);
    if (stat != MS::kSuccess) return stat;

    aRotate = nAttr.createPoint("Rotate", "r", &stat);
    if (stat != MS::kSuccess) return stat;
    nAttr.setDefault(0.0, 0.0, 0.0);
    stat = addAttribute(aRotate);
    if (stat != MS::kSuccess) return stat;

    aScale = nAttr.createPoint("Scale", "s", &stat);
    if (stat != MS::kSuccess) return stat;
    nAttr.setDefault(1.0, 1.0, 1.0);
    stat = addAttribute(aScale);
    if (stat != MS::kSuccess) return stat;

    aPivot = nAttr.createPoint("Pivot", "p", &stat);
    if (stat != MS::kSuccess) return stat;
    nAttr.setDefault(0.0, 0.0, 0.0);
    stat = addAttribute(aPivot);
    if (stat != MS::kSuccess) return stat;

    aUniformScale = nAttr.create("UniformScale", "us", MFnNumericData::kFloat);
    nAttr.setDefault(1.0);
    nAttr.setMin(1e-7);
    nAttr.setSoftMax(10.0);

    stat = addAttribute(aUniformScale);
    if (stat != MS::kSuccess) return stat;


    aInvert = nAttr.create("invert", "InvertTransformation", MFnNumericData::kBoolean);
    nAttr.setDefault(false);
    stat = addAttribute(aInvert);
    if (stat != MS::kSuccess) return stat;


    // input / output

    aVdbInput = tAttr.create("VdbInput", "vdbinput", OpenVDBData::id, MObject::kNullObj, &stat);
    if (stat != MS::kSuccess) return stat;

    tAttr.setConnectable(true);
    stat = addAttribute(aVdbInput);
    if (stat != MS::kSuccess) return stat;


    aVdbOutput = tAttr.create("VdbOutput", "vdboutput", OpenVDBData::id, MObject::kNullObj, &stat);
    if (stat != MS::kSuccess) return stat;

    tAttr.setWritable(false);
    tAttr.setStorable(false);
    stat = addAttribute(aVdbOutput);
    if (stat != MS::kSuccess) return stat;


    // attribute dependencies

    stat = attributeAffects(aVdbInput, aVdbOutput);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aVdbSelectedGridNames, aVdbOutput);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aTranslate, aVdbOutput);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aRotate, aVdbOutput);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aScale, aVdbOutput);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aPivot, aVdbOutput);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aUniformScale, aVdbOutput);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aInvert, aVdbOutput);
    if (stat != MS::kSuccess) return stat;

    return MS::kSuccess;
}


////////////////////////////////////////


MStatus OpenVDBTransformNode::compute(const MPlug& plug, MDataBlock& data)
{

    if (plug == aVdbOutput) {

        const OpenVDBData* inputVdb = mvdb::getInputVDB(aVdbInput, data);

        MStatus status;
        MFnPluginData pluginData;
        pluginData.create(OpenVDBData::id, &status);

        if (status != MS::kSuccess) {
            MGlobal::displayError("Failed to create a new OpenVDBData object.");
            return MS::kFailure;
        }

        OpenVDBData* outputVdb = static_cast<OpenVDBData*>(pluginData.data(&status));

        if (inputVdb && outputVdb) {

            const MFloatVector t = data.inputValue(aTranslate, &status).asFloatVector();
            const MFloatVector r = data.inputValue(aRotate, &status).asFloatVector();
            const MFloatVector p = data.inputValue(aPivot, &status).asFloatVector();
            const MFloatVector s = data.inputValue(aScale, &status).asFloatVector() *
                  data.inputValue(aUniformScale, &status).asFloat();

            // Construct new transform

            openvdb::Mat4R mat(openvdb::Mat4R::identity());

            mat.preTranslate(openvdb::Vec3R(p[0], p[1], p[2]));

            const double deg2rad = boost::math::constants::pi<double>() / 180.0;
            mat.preRotate(openvdb::math::X_AXIS, deg2rad*r[0]);
            mat.preRotate(openvdb::math::Y_AXIS, deg2rad*r[1]);
            mat.preRotate(openvdb::math::Z_AXIS, deg2rad*r[2]);

            mat.preScale(openvdb::Vec3R(s[0], s[1], s[2]));
            mat.preTranslate(openvdb::Vec3R(-p[0], -p[1], -p[2]));
            mat.preTranslate(openvdb::Vec3R(t[0], t[1], t[2]));

            typedef openvdb::math::AffineMap AffineMap;
            typedef openvdb::math::Transform Transform;

            if (data.inputValue(aInvert, &status).asBool()) {
                mat = mat.inverse();
            }

            AffineMap map(mat);

            const std::string selectionStr =
                data.inputValue(aVdbSelectedGridNames, &status).asString().asChar();

            mvdb::GridCPtrVec grids;
            if (!mvdb::getSelectedGrids(grids, selectionStr, *inputVdb, *outputVdb)) {
                MGlobal::displayWarning("No grids are selected.");
            }

            for (mvdb::GridCPtrVecIter it = grids.begin(); it != grids.end(); ++it) {

                openvdb::GridBase::ConstPtr grid = (*it)->copyGrid(); // shallow copy, shares tree

                // Merge the transform's current affine representation with the new affine map.
                AffineMap::Ptr compound(
                    new AffineMap(*grid->transform().baseMap()->getAffineMap(), map));

                // Simplify the affine map and replace the transform.
                openvdb::ConstPtrCast<openvdb::GridBase>(grid)->setTransform(
                    Transform::Ptr(new Transform(openvdb::math::simplify(compound))));

                outputVdb->insert(grid);
            }

            MDataHandle output = data.outputValue(aVdbOutput);
            output.set(outputVdb);

            return data.setClean(plug);
        }
    }

    return MS::kUnknownParameter;
}
