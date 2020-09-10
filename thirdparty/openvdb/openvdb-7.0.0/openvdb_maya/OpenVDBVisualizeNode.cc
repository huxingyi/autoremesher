// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

/// @file OpenVDBVisualizeNode.cc
///
/// @author FX R&D OpenVDB team

#include "OpenVDBPlugin.h"
#include <openvdb_maya/OpenVDBData.h>
#include <openvdb_maya/OpenVDBUtil.h>

#include <openvdb/io/Stream.h>

#include <maya/MFnTypedAttribute.h>
#include <maya/MFnStringData.h>
#include <maya/MFnPluginData.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MPxLocatorNode.h>
#include <maya/MString.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MColor.h>
#include <maya/M3dView.h>


namespace mvdb = openvdb_maya;


////////////////////////////////////////


struct OpenVDBVisualizeNode : public MPxLocatorNode
{
    OpenVDBVisualizeNode();
    virtual ~OpenVDBVisualizeNode();

    virtual MStatus compute(const MPlug& plug, MDataBlock& data);

    virtual void draw(M3dView & view, const MDagPath & path,
        M3dView::DisplayStyle style, M3dView::DisplayStatus status);

    virtual bool isBounded() const;
    virtual MBoundingBox boundingBox() const;

    static void * creator();
    static  MStatus initialize();

    static MObject aVdbInput;
    static MObject aVdbAllGridNames;
    static MObject aVdbSelectedGridNames;

    static MObject aVisualizeBBox;
    static MObject aVisualizeInternalNodes;
    static MObject aVisualizeLeafNodes;
    static MObject aVisualizeActiveTiles;
    static MObject aVisualizeActiveVoxels;
    static MObject aVisualizeSurface;
    static MObject aIsovalue;

    static MObject aCachedBBox;
    static MObject aCachedInternalNodes;
    static MObject aCachedLeafNodes;
    static MObject aCachedActiveTiles;
    static MObject aCachedActiveVoxels;
    static MObject aCachedSurface;


    static MTypeId id;

private:
    std::vector<mvdb::BufferObject> mBBoxBuffers;
    std::vector<mvdb::BufferObject> mNodeBuffers;
    std::vector<mvdb::BufferObject> mLeafBuffers;
    std::vector<mvdb::BufferObject> mTileBuffers;
    std::vector<mvdb::BufferObject> mSurfaceBuffers;
    std::vector<mvdb::BufferObject> mPointBuffers;

    mvdb::ShaderProgram mSurfaceShader, mPointShader;
    MBoundingBox mBBox;
};


////////////////////////////////////////


MObject OpenVDBVisualizeNode::aVdbInput;
MObject OpenVDBVisualizeNode::aVdbAllGridNames;
MObject OpenVDBVisualizeNode::aVdbSelectedGridNames;

MObject OpenVDBVisualizeNode::aVisualizeBBox;
MObject OpenVDBVisualizeNode::aVisualizeInternalNodes;
MObject OpenVDBVisualizeNode::aVisualizeLeafNodes;
MObject OpenVDBVisualizeNode::aVisualizeActiveTiles;
MObject OpenVDBVisualizeNode::aVisualizeActiveVoxels;
MObject OpenVDBVisualizeNode::aVisualizeSurface;
MObject OpenVDBVisualizeNode::aIsovalue;

MObject OpenVDBVisualizeNode::aCachedBBox;
MObject OpenVDBVisualizeNode::aCachedInternalNodes;
MObject OpenVDBVisualizeNode::aCachedLeafNodes;
MObject OpenVDBVisualizeNode::aCachedActiveTiles;
MObject OpenVDBVisualizeNode::aCachedActiveVoxels;
MObject OpenVDBVisualizeNode::aCachedSurface;

MTypeId OpenVDBVisualizeNode::id(0x00108A53);


namespace {
    mvdb::NodeRegistry registerNode("OpenVDBVisualize", OpenVDBVisualizeNode::id,
        OpenVDBVisualizeNode::creator, OpenVDBVisualizeNode::initialize, MPxNode::kLocatorNode);
}


////////////////////////////////////////


OpenVDBVisualizeNode::OpenVDBVisualizeNode()
{
    mSurfaceShader.setVertShader(
        "#version 120\n"
        "varying vec3 normal;\n"
        "void main() {\n"
            "normal = normalize(gl_NormalMatrix * gl_Normal);\n"
            "gl_Position =  ftransform();\n"
            "gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;\n"
        "}\n");

    mSurfaceShader.setFragShader(
        "#version 120\n"
        "varying vec3 normal;\n"
        "const vec4 skyColor = vec4(0.9, 0.9, 1.0, 1.0);\n"
        "const vec4 groundColor = vec4(0.3, 0.3, 0.2, 1.0);\n"
        "void main() {\n"
            "vec3 normalized_normal = normalize(normal);\n"
            "float w = 0.5 * (1.0 + dot(normalized_normal, vec3(0.0, 1.0, 0.0)));\n"
            "vec4 diffuseColor = w * skyColor + (1.0 - w) * groundColor;\n"
            "gl_FragColor = diffuseColor;\n"
        "}\n");

    mSurfaceShader.build();

    mPointShader.setVertShader(
        "#version 120\n"
        "varying vec3 normal;\n"
        "void main() {\n"
            "gl_FrontColor = gl_Color;\n"
            "normal = normalize(gl_NormalMatrix * gl_Normal);\n"
            "gl_Position =  ftransform();\n"
            "gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;\n"
        "}\n");

    mPointShader.setFragShader(
        "#version 120\n"
        "varying vec3 normal;\n"
        "void main() {\n"
            "vec3 normalized_normal = normalize(normal);\n"
            "float w = 0.5 * (1.0 + dot(normalized_normal, vec3(0.0, 1.0, 0.0)));\n"
            "vec4 diffuseColor = w * gl_Color + (1.0 - w) * (gl_Color * 0.3);\n"
            "gl_FragColor = diffuseColor;\n"
        "}\n");

    mPointShader.build();
}

OpenVDBVisualizeNode::~OpenVDBVisualizeNode()
{
}


void* OpenVDBVisualizeNode::creator()
{
    return new OpenVDBVisualizeNode();
}


MStatus OpenVDBVisualizeNode::initialize()
{
    MStatus stat;
    MFnNumericAttribute nAttr;
    MFnTypedAttribute tAttr;

    // Setup input / output attributes

    aVdbInput = tAttr.create("VDBInput", "input", OpenVDBData::id, MObject::kNullObj, &stat);
    if (stat != MS::kSuccess) return stat;
    tAttr.setReadable(false);

    stat = addAttribute(aVdbInput);
    if (stat != MS::kSuccess) return stat;

    // Setup UI attributes

    aVisualizeBBox = nAttr.create("ActiveValueBoundingBox", "bbox", MFnNumericData::kBoolean);
    nAttr.setDefault(true);
    nAttr.setConnectable(false);

    stat = addAttribute(aVisualizeBBox);
    if (stat != MS::kSuccess) return stat;

    aVisualizeInternalNodes = nAttr.create("InternalNodes", "inodes", MFnNumericData::kBoolean);
    nAttr.setDefault(false);
    nAttr.setConnectable(false);

    stat = addAttribute(aVisualizeInternalNodes);
    if (stat != MS::kSuccess) return stat;

    aVisualizeLeafNodes = nAttr.create("LeafNodes", "lnodes", MFnNumericData::kBoolean);
    nAttr.setDefault(true);
    nAttr.setConnectable(false);

    stat = addAttribute(aVisualizeLeafNodes);
    if (stat != MS::kSuccess) return stat;

    aVisualizeActiveTiles = nAttr.create("ActiveTiles", "tiles", MFnNumericData::kBoolean);
    nAttr.setDefault(true);
    nAttr.setConnectable(false);

    stat = addAttribute(aVisualizeActiveTiles);
    if (stat != MS::kSuccess) return stat;

    aVisualizeActiveVoxels = nAttr.create("ActiveVoxels", "voxels", MFnNumericData::kBoolean);
    nAttr.setDefault(false);
    nAttr.setConnectable(false);

    stat = addAttribute(aVisualizeActiveVoxels);
    if (stat != MS::kSuccess) return stat;

    aVisualizeSurface = nAttr.create("Surface", "surface", MFnNumericData::kBoolean);
    nAttr.setDefault(false);
    nAttr.setConnectable(false);

    stat = addAttribute(aVisualizeSurface);
    if (stat != MS::kSuccess) return stat;

    aIsovalue = nAttr.create("Isovalue", "iso", MFnNumericData::kFloat);
    nAttr.setDefault(0.0);
    nAttr.setSoftMin(-1.0);
    nAttr.setSoftMax( 1.0);
    nAttr.setConnectable(false);

    stat = addAttribute(aIsovalue);
    if (stat != MS::kSuccess) return stat;

    // Setup internal attributes

    aCachedBBox = nAttr.create("cachedbbox", "cbb", MFnNumericData::kBoolean);
    nAttr.setDefault(false);
    nAttr.setWritable(false);
    nAttr.setReadable(false);
    nAttr.setHidden(true);

    stat = addAttribute(aCachedBBox);
    if (stat != MS::kSuccess) return stat;

    aCachedInternalNodes = nAttr.create("cachedinternalnodes", "cin", MFnNumericData::kBoolean);
    nAttr.setDefault(false);
    nAttr.setWritable(false);
    nAttr.setReadable(false);
    nAttr.setHidden(true);

    stat = addAttribute(aCachedInternalNodes);
    if (stat != MS::kSuccess) return stat;

    aCachedLeafNodes = nAttr.create("cachedleafnodes", "cln", MFnNumericData::kBoolean);
    nAttr.setDefault(false);
    nAttr.setWritable(false);
    nAttr.setReadable(false);
    nAttr.setHidden(true);

    stat = addAttribute(aCachedLeafNodes);
    if (stat != MS::kSuccess) return stat;

    aCachedActiveTiles = nAttr.create("cachedactivetiles", "cat", MFnNumericData::kBoolean);
    nAttr.setDefault(false);
    nAttr.setWritable(false);
    nAttr.setReadable(false);
    nAttr.setHidden(true);

    stat = addAttribute(aCachedActiveTiles);
    if (stat != MS::kSuccess) return stat;

    aCachedActiveVoxels = nAttr.create("cachedactivevoxels", "cav", MFnNumericData::kBoolean);
    nAttr.setDefault(false);
    nAttr.setWritable(false);
    nAttr.setReadable(false);
    nAttr.setHidden(true);

    stat = addAttribute(aCachedActiveVoxels);
    if (stat != MS::kSuccess) return stat;

    aCachedSurface = nAttr.create("cachedsurface", "cs", MFnNumericData::kBoolean);
    nAttr.setDefault(false);
    nAttr.setWritable(false);
    nAttr.setReadable(false);
    nAttr.setHidden(true);

    stat = addAttribute(aCachedSurface);
    if (stat != MS::kSuccess) return stat;

    MFnStringData fnStringData;
    MObject defaultStringData = fnStringData.create("");

    aVdbAllGridNames = tAttr.create("VdbAllGridNames", "allgrids", MFnData::kString, defaultStringData, &stat);
    if (stat != MS::kSuccess) return stat;
    tAttr.setConnectable(false);
    tAttr.setWritable(false);
    tAttr.setReadable(false);
    tAttr.setHidden(true);

    stat = addAttribute(aVdbAllGridNames);
    if (stat != MS::kSuccess) return stat;

    aVdbSelectedGridNames =
        tAttr.create("VdbSelectedGridNames", "selectedgrids", MFnData::kString, defaultStringData, &stat);
    if (stat != MS::kSuccess) return stat;
    tAttr.setConnectable(false);
    tAttr.setWritable(false);
    tAttr.setReadable(false);
    tAttr.setHidden(true);

    stat = addAttribute(aVdbSelectedGridNames);
    if (stat != MS::kSuccess) return stat;

    // Setup dependencies

    stat = attributeAffects(aVdbInput, aVdbAllGridNames);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aVdbInput, aCachedBBox);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aVdbInput, aCachedInternalNodes);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aVdbInput, aCachedLeafNodes);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aVdbInput, aCachedActiveTiles);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aVdbInput, aCachedActiveVoxels);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aVdbInput, aCachedSurface);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aVdbSelectedGridNames, aCachedBBox);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aVdbSelectedGridNames, aCachedInternalNodes);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aVdbSelectedGridNames, aCachedLeafNodes);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aVdbSelectedGridNames, aCachedActiveTiles);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aVdbSelectedGridNames, aCachedActiveVoxels);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aVdbSelectedGridNames, aCachedSurface);
    if (stat != MS::kSuccess) return stat;

    stat = attributeAffects(aIsovalue, aCachedSurface);
    if (stat != MS::kSuccess) return stat;

    return MS::kSuccess;
}


////////////////////////////////////////


MStatus OpenVDBVisualizeNode::compute(const MPlug& plug, MDataBlock& data)
{
    MStatus status;

    const OpenVDBData* inputVdb = mvdb::getInputVDB(aVdbInput, data);
    if (!inputVdb) return MS::kFailure;

    if (plug == aVdbAllGridNames) {
        MString names = mvdb::getGridNames(*inputVdb).c_str();
        MDataHandle outHandle = data.outputValue(aVdbAllGridNames);
        outHandle.set(names);
        return data.setClean(plug);
    }

    // Get selected grids

    MDataHandle selectionHandle = data.inputValue(aVdbSelectedGridNames, &status);

    if (status != MS::kSuccess) return status;
    std::string names = selectionHandle.asString().asChar();

    std::vector<openvdb::GridBase::ConstPtr> grids;
    mvdb::getGrids(grids, *inputVdb, names);

    if (grids.empty()) {
        mBBoxBuffers.clear();
        mNodeBuffers.clear();
        mLeafBuffers.clear();
        mTileBuffers.clear();
        mSurfaceBuffers.clear();
        mPointBuffers.clear();
        return MS::kUnknownParameter;
    }


    if (plug == aCachedInternalNodes) {
        mNodeBuffers.clear();
        mNodeBuffers.resize(grids.size());

        for (size_t n = 0, N = grids.size(); n < N; ++n) {
            mvdb::InternalNodesGeo drawNodes(mNodeBuffers[n]);
            mvdb::processTypedGrid(grids[n], drawNodes);
        }

        MDataHandle outHandle = data.outputValue(aCachedInternalNodes);
        outHandle.set(true);

    } else if (plug == aCachedLeafNodes) {
        mLeafBuffers.clear();
        mLeafBuffers.resize(grids.size());

        for (size_t n = 0, N = grids.size(); n < N; ++n) {
            mvdb::LeafNodesGeo drawLeafs(mLeafBuffers[n]);
            mvdb::processTypedGrid(grids[n], drawLeafs);
        }

        MDataHandle outHandle = data.outputValue(aCachedLeafNodes);
        outHandle.set(true);

    } else if (plug == aCachedBBox) {
        MPoint pMin, pMax;

        mBBoxBuffers.clear();
        mBBoxBuffers.resize(grids.size());

        for (size_t n = 0, N = grids.size(); n < N; ++n) {
            mvdb::BoundingBoxGeo drawBBox(mBBoxBuffers[n]);
            drawBBox(grids[n]);

            for (int i = 0; i < 3; ++i) {
                pMin[i] = drawBBox.min()[i];
                pMax[i] = drawBBox.max()[i];
            }
        }
        mBBox = MBoundingBox(pMin, pMax);

        MDataHandle outHandle = data.outputValue(aCachedBBox);
        outHandle.set(true);

    } else if (plug == aCachedActiveTiles) {
        mTileBuffers.clear();
        mTileBuffers.resize(grids.size());

        for (size_t n = 0, N = grids.size(); n < N; ++n) {
            mvdb::ActiveTileGeo drawTiles(mTileBuffers[n]);
            mvdb::processTypedGrid(grids[n], drawTiles);
        }

        MDataHandle outHandle = data.outputValue(aCachedActiveTiles);
        outHandle.set(true);

    } else if(plug == aCachedActiveVoxels) {
        mPointBuffers.clear();
        mPointBuffers.resize(grids.size());

        for (size_t n = 0, N = grids.size(); n < N; ++n) {
            mvdb::ActiveVoxelGeo drawVoxels(mPointBuffers[n]);
            mvdb::processTypedScalarGrid(grids[n], drawVoxels);
        }

        MDataHandle outHandle = data.outputValue(aCachedActiveVoxels);
        outHandle.set(true);

    } else if (plug == aCachedSurface) {
        float iso = data.inputValue(aIsovalue, &status).asFloat();
        if (status != MS::kSuccess) return status;

        mSurfaceBuffers.clear();
        mSurfaceBuffers.resize(grids.size());

        for (size_t n = 0, N = grids.size(); n < N; ++n) {
            mvdb::SurfaceGeo drawSurface(mSurfaceBuffers[n], iso);
            mvdb::processTypedScalarGrid(grids[n], drawSurface);
        }

        MDataHandle outHandle = data.outputValue(aCachedSurface);
        outHandle.set(true);

    } else {
        return MS::kUnknownParameter;
    }

    return data.setClean(plug);
}


void
OpenVDBVisualizeNode::draw(M3dView & view, const MDagPath& /*path*/,
        M3dView::DisplayStyle /*style*/, M3dView::DisplayStatus status)
{
    MObject thisNode = thisMObject();

    const bool isSelected = (status == M3dView::kActive) || (status == M3dView::kLead);

    const bool internalNodes    = MPlug(thisNode, aVisualizeInternalNodes).asBool();
    const bool leafNodes        = MPlug(thisNode, aVisualizeLeafNodes).asBool();
    const bool bbox             = MPlug(thisNode, aVisualizeBBox).asBool();
    const bool tiles            = MPlug(thisNode, aVisualizeActiveTiles).asBool();
    const bool voxels           = MPlug(thisNode, aVisualizeActiveVoxels).asBool();
    const bool surface          = MPlug(thisNode, aVisualizeSurface).asBool();

    view.beginGL();

    if (surface && MPlug(thisNode, aCachedSurface).asBool()) {
        if (!view.selectMode()) mSurfaceShader.startShading();
        for (size_t n = 0, N = mSurfaceBuffers.size(); n < N; ++n) {
            mSurfaceBuffers[n].render();
        }
        mSurfaceShader.stopShading();
    }

    if (tiles && MPlug(thisNode, aCachedActiveTiles).asBool()) {
        for (size_t n = 0, N = mTileBuffers.size(); n < N; ++n) {
            mTileBuffers[n].render();
        }
    }

    if (leafNodes && MPlug(thisNode, aCachedLeafNodes).asBool()) {
        for (size_t n = 0, N = mLeafBuffers.size(); n < N; ++n) {
            mLeafBuffers[n].render();
        }
    }

    if (voxels && MPlug(thisNode, aCachedActiveVoxels).asBool()) {
        if (!view.selectMode()) mPointShader.startShading();
        for (size_t n = 0, N = mPointBuffers.size(); n < N; ++n) {
            mPointBuffers[n].render();
        }
        mPointShader.stopShading();
    }

    if (!view.selectMode()) {

        if (internalNodes && MPlug(thisNode, aCachedInternalNodes).asBool()) {

            for (size_t n = 0, N = mNodeBuffers.size(); n < N; ++n) {
                mNodeBuffers[n].render();
            }
        }

        if ((isSelected || bbox) && MPlug(thisNode, aCachedBBox).asBool()) {

            if (isSelected) glColor3f(0.9f, 0.9f, 0.3f);
            else glColor3f(0.045f, 0.045f, 0.045f);

            for (size_t n = 0, N = mBBoxBuffers.size(); n < N; ++n) {
                mBBoxBuffers[n].render();
            }
        }
    }

    view.endGL();
}


bool
OpenVDBVisualizeNode::isBounded() const
{
    return true;
}


MBoundingBox
OpenVDBVisualizeNode::boundingBox() const
{
    bool cachedBBox = false;
    MObject thisNode = thisMObject();
    MPlug(thisNode, aCachedBBox).getValue(cachedBBox);
    if (cachedBBox) return mBBox;

    return MBoundingBox(MPoint(-1.0, -1.0, -1.0), MPoint(1.0, 1.0, 1.0));
}
