// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

/// @author FX R&D OpenVDB team

#include "OpenVDBPlugin.h"
#include <openvdb_maya/OpenVDBData.h>
#include <openvdb_maya/OpenVDBUtil.h>
#include <openvdb/io/Stream.h>

#include <maya/MFnNumericAttribute.h>
#include <maya/MFnPluginData.h>
#include <maya/MFnStringData.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnEnumAttribute.h>

#include <fstream>
#include <sstream> // std::stringstream

namespace mvdb = openvdb_maya;


////////////////////////////////////////


struct OpenVDBReadNode : public MPxNode
{
    OpenVDBReadNode() {}
    virtual ~OpenVDBReadNode() {}

    virtual MStatus compute(const MPlug& plug, MDataBlock& data);

    static void* creator();
    static MStatus initialize();
    static MTypeId id;
    static MObject aVdbFilePath;
    static MObject aFrameNumbering;
    static MObject aInputTime;
    static MObject aVdbOutput;
    static MObject aNodeInfo;
};


MTypeId OpenVDBReadNode::id(0x00108A51);
MObject OpenVDBReadNode::aVdbFilePath;
MObject OpenVDBReadNode::aFrameNumbering;
MObject OpenVDBReadNode::aInputTime;
MObject OpenVDBReadNode::aVdbOutput;
MObject OpenVDBReadNode::aNodeInfo;


namespace {
    mvdb::NodeRegistry registerNode("OpenVDBRead", OpenVDBReadNode::id,
        OpenVDBReadNode::creator, OpenVDBReadNode::initialize);
}


////////////////////////////////////////


void* OpenVDBReadNode::creator()
{
        return new OpenVDBReadNode();
}


MStatus OpenVDBReadNode::initialize()
{
    MStatus stat;
    MFnTypedAttribute tAttr;
    MFnEnumAttribute eAttr;
    MFnUnitAttribute unitAttr;

    MFnStringData fnStringData;
    MObject defaultStringData = fnStringData.create("");
    MObject emptyStr = fnStringData.create("");

    // Setup the input attributes

    aVdbFilePath = tAttr.create("VdbFilePath", "file", MFnData::kString, defaultStringData, &stat);
    if (stat != MS::kSuccess) return stat;

    tAttr.setConnectable(false);
    stat = addAttribute(aVdbFilePath);
    if (stat != MS::kSuccess) return stat;

    aFrameNumbering = eAttr.create("FrameNumbering", "numbering", 0, &stat);
    if (stat != MS::kSuccess) return stat;

    eAttr.addField("Frame.SubTick", 0);
    eAttr.addField("Fractional frame values", 1);
    eAttr.addField("Global ticks", 2);

    eAttr.setConnectable(false);
    stat = addAttribute(aFrameNumbering);
    if (stat != MS::kSuccess) return stat;

    aInputTime = unitAttr.create("inputTime", "int", MTime(0.0, MTime::kFilm));
    unitAttr.setKeyable(true);
    unitAttr.setReadable(true);
    unitAttr.setWritable(true);
    unitAttr.setStorable(true);
    stat = addAttribute(aInputTime);
    if (stat != MS::kSuccess) return stat;

    // Setup the output attributes

    aVdbOutput = tAttr.create("VdbOutput", "vdb", OpenVDBData::id, MObject::kNullObj, &stat);
    if (stat != MS::kSuccess) return stat;

    tAttr.setWritable(false);
    tAttr.setStorable(false);
    stat = addAttribute(aVdbOutput);
    if (stat != MS::kSuccess) return stat;

    aNodeInfo = tAttr.create("NodeInfo", "info", MFnData::kString, emptyStr, &stat);
    if (stat != MS::kSuccess) return stat;
    tAttr.setConnectable(false);
    tAttr.setWritable(false);
    stat = addAttribute(aNodeInfo);
    if (stat != MS::kSuccess) return stat;

    // Set the attribute dependencies

    stat = attributeAffects(aVdbFilePath, aVdbOutput);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aFrameNumbering, aVdbOutput);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aInputTime, aVdbOutput);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aVdbFilePath, aNodeInfo);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aFrameNumbering, aNodeInfo);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aInputTime, aNodeInfo);
    if (stat != MS::kSuccess) return stat;

    return MS::kSuccess;
}


////////////////////////////////////////


MStatus OpenVDBReadNode::compute(const MPlug& plug, MDataBlock& data)
{
    if (plug == aVdbOutput || plug == aNodeInfo) {

        MStatus status;

        const int numberingScheme = data.inputValue(aFrameNumbering , &status).asInt();

        MDataHandle filePathHandle = data.inputValue (aVdbFilePath, &status);
        if (status != MS::kSuccess) return status;

        std::string filename = filePathHandle.asString().asChar();
        if (filename.empty()) {
            return MS::kUnknownParameter;
        }

        MTime time = data.inputValue(aInputTime).asTime();
        mvdb::insertFrameNumber(filename, time, numberingScheme);

        std::stringstream infoStr;
        infoStr << "File: " << filename << "\n";

        std::ifstream ifile(filename.c_str(), std::ios_base::binary);
        openvdb::GridPtrVecPtr grids = openvdb::io::Stream(ifile).getGrids();

        if (grids && !grids->empty()) {

            MFnPluginData outputDataCreators;
            outputDataCreators.create(OpenVDBData::id, &status);
            if (status != MS::kSuccess) return status;

            OpenVDBData* vdb = static_cast<OpenVDBData*>(outputDataCreators.data(&status));
            if (status != MS::kSuccess) return status;

            vdb->insert(*grids);

            MDataHandle outHandle = data.outputValue(aVdbOutput);
            outHandle.set(vdb);

            infoStr << "Frame: " << time.as(MTime::uiUnit()) << " -> loaded\n";
            mvdb::printGridInfo(infoStr, *vdb);
        } else {
            infoStr << "Frame: " << time.as(MTime::uiUnit()) << " -> no matching file.\n";
        }

        mvdb::updateNodeInfo(infoStr, data, aNodeInfo);
        return data.setClean(plug);
    }

    return MS::kUnknownParameter;
}
