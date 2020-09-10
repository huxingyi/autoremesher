// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

/// @author FX R&D OpenVDB team

#include "OpenVDBPlugin.h"
#include <openvdb_maya/OpenVDBData.h>
#include <openvdb_maya/OpenVDBUtil.h>

#include <openvdb/io/Stream.h>
#include <openvdb/math/Math.h>

#include <maya/MFnNumericAttribute.h>
#include <maya/MFnPluginData.h>
#include <maya/MFnStringData.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnEnumAttribute.h>

#include <sstream> // std::stringstream

namespace mvdb = openvdb_maya;


////////////////////////////////////////


struct OpenVDBWriteNode : public MPxNode
{
    OpenVDBWriteNode() {}
    virtual ~OpenVDBWriteNode() {}

    virtual MStatus compute(const MPlug& plug, MDataBlock& data);

    static void* creator();
    static MStatus initialize();
    static MTypeId id;
    static MObject aVdbFilePath;
    static MObject aFrameNumbering;
    static MObject aInputTime;
    static MObject aVdbInput;
    static MObject aVdbOutput;
    static MObject aNodeInfo;
};


MTypeId OpenVDBWriteNode::id(0x00108A52);
MObject OpenVDBWriteNode::aVdbFilePath;
MObject OpenVDBWriteNode::aFrameNumbering;
MObject OpenVDBWriteNode::aInputTime;
MObject OpenVDBWriteNode::aVdbInput;
MObject OpenVDBWriteNode::aVdbOutput;
MObject OpenVDBWriteNode::aNodeInfo;


namespace {
    mvdb::NodeRegistry registerNode("OpenVDBWrite", OpenVDBWriteNode::id,
        OpenVDBWriteNode::creator, OpenVDBWriteNode::initialize);
} // unnamed namespace


////////////////////////////////////////


void* OpenVDBWriteNode::creator()
{
        return new OpenVDBWriteNode();
}


MStatus OpenVDBWriteNode::initialize()
{
    MStatus stat;
    MFnTypedAttribute tAttr;
    MFnEnumAttribute eAttr;
    MFnUnitAttribute unitAttr;

    MFnStringData fnStringData;
    MObject defaultStringData = fnStringData.create("volume.vdb");
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

    aVdbInput = tAttr.create("VdbInput", "vdbinput", OpenVDBData::id, MObject::kNullObj, &stat);
    if (stat != MS::kSuccess) return stat;

    tAttr.setConnectable(true);
    stat = addAttribute(aVdbInput);
    if (stat != MS::kSuccess) return stat;

    // Setup the output attributes

    aVdbOutput = tAttr.create("VdbOutput", "vdboutput", OpenVDBData::id, MObject::kNullObj, &stat);
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

    stat = attributeAffects(aVdbInput, aVdbOutput);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aVdbFilePath, aNodeInfo);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aFrameNumbering, aNodeInfo);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aInputTime, aNodeInfo);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aVdbInput, aNodeInfo);
    if (stat != MS::kSuccess) return stat;

    return MS::kSuccess;
}


////////////////////////////////////////


MStatus OpenVDBWriteNode::compute(const MPlug& plug, MDataBlock& data)
{

    if (plug == aVdbOutput || plug == aNodeInfo) {

        MStatus status;

        const int numberingScheme = data.inputValue(aFrameNumbering , &status).asInt();

        MDataHandle filePathHandle = data.inputValue(aVdbFilePath, &status);
        if (status != MS::kSuccess) return status;

        std::string filename = filePathHandle.asString().asChar();
        if (filename.empty()) {
            return MS::kUnknownParameter;
        }

        MTime time = data.inputValue(aInputTime).asTime();
        mvdb::insertFrameNumber(filename, time, numberingScheme);

        std::stringstream infoStr;
        infoStr << "File: " << filename << "\n";

        MDataHandle inputVdbHandle = data.inputValue(aVdbInput, &status);
        if (status != MS::kSuccess) return status;

        MFnPluginData fnData(inputVdbHandle.data());
        MPxData * pxData = fnData.data();

        if (pxData) {
            OpenVDBData* vdb = dynamic_cast<OpenVDBData*>(pxData);

            if (vdb) {

                // Add file-level metadata.
                openvdb::MetaMap outMeta;
                outMeta.insertMeta("creator",
                    openvdb::StringMetadata("Maya/OpenVDB_Write_Node"));

                const MTime dummy(1.0, MTime::kSeconds);
                const double fps = dummy.as(MTime::uiUnit());
                const double tpf = 6000.0 / fps;
                const double frame = time.as(MTime::uiUnit());

                outMeta.insertMeta("frame", openvdb::DoubleMetadata(frame));
                outMeta.insertMeta("tick", openvdb::Int32Metadata(int(openvdb::math::Round(frame * tpf))));

                outMeta.insertMeta("frames_per_second", openvdb::Int32Metadata(int(fps)));
                outMeta.insertMeta("ticks_per_frame", openvdb::Int32Metadata(int(tpf)));
                outMeta.insertMeta("ticks_per_second", openvdb::Int32Metadata(6000));

                // Create a VDB file object.
                openvdb::io::File file(filename);

                vdb->write(file, outMeta);

                //file.write(vdb->grids(), outMeta);
                file.close();

                // Output
                MFnPluginData outputDataCreators;
                outputDataCreators.create(OpenVDBData::id, &status);
                if (status != MS::kSuccess) return status;

                OpenVDBData* outputVdb = static_cast<OpenVDBData*>(outputDataCreators.data(&status));
                if (status != MS::kSuccess) return status;

                outputVdb = vdb;

                MDataHandle outHandle = data.outputValue(aVdbOutput);
                outHandle.set(outputVdb);

                infoStr << "Frame: " << frame << "\n";
                mvdb::printGridInfo(infoStr, *vdb);
            }

            mvdb::updateNodeInfo(infoStr, data, aNodeInfo);
            return data.setClean(plug);
        }
    }

    return MS::kUnknownParameter;
}
