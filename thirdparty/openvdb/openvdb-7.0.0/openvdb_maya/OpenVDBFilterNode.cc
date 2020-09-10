// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

/// @author FX R&D OpenVDB team

#include "OpenVDBPlugin.h"
#include <openvdb_maya/OpenVDBData.h>
#include <openvdb_maya/OpenVDBUtil.h>

#include <openvdb/tools/Filter.h>

#include <maya/MFnTypedAttribute.h>
#include <maya/MFnStringData.h>
#include <maya/MFnPluginData.h>
#include <maya/MGlobal.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnNumericAttribute.h>


namespace mvdb = openvdb_maya;


////////////////////////////////////////


struct OpenVDBFilterNode : public MPxNode
{
    OpenVDBFilterNode() {}
    virtual ~OpenVDBFilterNode() {}

    virtual MStatus compute(const MPlug& plug, MDataBlock& data);

    static void* creator();
    static MStatus initialize();

    static MTypeId id;
    static MObject aVdbInput;
    static MObject aVdbOutput;
    static MObject aVdbSelectedGridNames;
    static MObject aFilter;
    static MObject aRadius;
    static MObject aOffset;
    static MObject aIterations;
};


MTypeId OpenVDBFilterNode::id(0x00108A56);
MObject OpenVDBFilterNode::aVdbOutput;
MObject OpenVDBFilterNode::aVdbInput;
MObject OpenVDBFilterNode::aVdbSelectedGridNames;
MObject OpenVDBFilterNode::aFilter;
MObject OpenVDBFilterNode::aRadius;
MObject OpenVDBFilterNode::aOffset;
MObject OpenVDBFilterNode::aIterations;


namespace {
    mvdb::NodeRegistry registerNode("OpenVDBFilter", OpenVDBFilterNode::id,
        OpenVDBFilterNode::creator, OpenVDBFilterNode::initialize);
}


////////////////////////////////////////


void* OpenVDBFilterNode::creator()
{
    return new OpenVDBFilterNode();
}


MStatus OpenVDBFilterNode::initialize()
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


    MFnEnumAttribute eAttr;
    aFilter = eAttr.create("Filter", "filter", 0, &stat);
    if (stat != MS::kSuccess) return stat;

    eAttr.addField("Mean", 0);
    eAttr.addField("Gauss", 1);
    eAttr.addField("Median", 2);
    eAttr.addField("Offset", 3);

    eAttr.setConnectable(false);
    stat = addAttribute(aFilter);
    if (stat != MS::kSuccess) return stat;


    MFnNumericAttribute nAttr;

    aRadius = nAttr.create("FilterVoxelRadius", "r", MFnNumericData::kInt);
    nAttr.setDefault(1);
    nAttr.setMin(1);
    nAttr.setSoftMin(1);
    nAttr.setSoftMax(5);

    stat = addAttribute(aRadius);
    if (stat != MS::kSuccess) return stat;

    aIterations = nAttr.create("Iterations", "it", MFnNumericData::kInt);
    nAttr.setDefault(1);
    nAttr.setMin(1);
    nAttr.setSoftMin(1);
    nAttr.setSoftMax(10);

    stat = addAttribute(aIterations);
    if (stat != MS::kSuccess) return stat;


    aOffset = nAttr.create("Offset", "o", MFnNumericData::kFloat);
    nAttr.setDefault(0.0);
    nAttr.setSoftMin(-1.0);
    nAttr.setSoftMax(1.0);

    stat = addAttribute(aOffset);
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

    stat = attributeAffects(aFilter, aVdbOutput);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aVdbSelectedGridNames, aVdbOutput);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aRadius, aVdbOutput);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aOffset, aVdbOutput);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aIterations, aVdbOutput);
    if (stat != MS::kSuccess) return stat;

    return MS::kSuccess;
}


////////////////////////////////////////

namespace internal {

template <typename GridT>
void filterGrid(openvdb::GridBase& grid, int operation, int radius, int iterations, float offset = 0.0)
{
    GridT& gridRef = static_cast<GridT&>(grid);

    openvdb::tools::Filter<GridT> filter(gridRef);

    switch (operation) {
    case 0:
        filter.mean(radius, iterations);
        break;
    case 1:
        filter.gaussian(radius, iterations);
        break;
    case 2:
        filter.median(radius, iterations);
        break;
    case 3:
        filter.offset(typename GridT::ValueType(offset));
        break;
    }
}

}; // namespace internal

////////////////////////////////////////


MStatus OpenVDBFilterNode::compute(const MPlug& plug, MDataBlock& data)
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


            const int operation = data.inputValue(aFilter, &status).asInt();
            const int radius = data.inputValue(aRadius, &status).asInt();
            const int iterations = data.inputValue(aIterations, &status).asInt();
            const float offset = data.inputValue(aOffset, &status).asFloat();
            const std::string selectionStr =
                data.inputValue(aVdbSelectedGridNames, &status).asString().asChar();


            mvdb::GridCPtrVec grids;
            if (!mvdb::getSelectedGrids(grids, selectionStr, *inputVdb, *outputVdb)) {
                MGlobal::displayWarning("No grids are selected.");
            }

            for (mvdb::GridCPtrVecIter it = grids.begin(); it != grids.end(); ++it) {

                const openvdb::GridBase& gridRef = **it;

                if (gridRef.type() == openvdb::FloatGrid::gridType()) {
                    openvdb::GridBase::Ptr grid = gridRef.deepCopyGrid(); // modifiable copy
                    internal::filterGrid<openvdb::FloatGrid>(*grid, operation, radius, iterations, offset);
                    outputVdb->insert(grid);
                } else if (gridRef.type() == openvdb::DoubleGrid::gridType()) {
                    openvdb::GridBase::Ptr grid = gridRef.deepCopyGrid(); // modifiable copy
                    internal::filterGrid<openvdb::DoubleGrid>(*grid, operation, radius, iterations, offset);
                    outputVdb->insert(grid);
                } else {
                    const std::string msg = "Skipped '" + gridRef.getName() + "', unsupported type.";
                    MGlobal::displayWarning(msg.c_str());
                    outputVdb->insert(gridRef);
                }
            }

            MDataHandle output = data.outputValue(aVdbOutput);
            output.set(outputVdb);

            return data.setClean(plug);
        }
    }

    return MS::kUnknownParameter;
}
