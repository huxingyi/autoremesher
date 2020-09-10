// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

/// @file OpenVDBToPolygonsNode.cc
///
/// @author Fredrik Salomonsson (fredriks@d2.com)

#include "OpenVDBPlugin.h"
#include <openvdb_maya/OpenVDBData.h>
#include <openvdb_maya/OpenVDBUtil.h>

#include <openvdb/tools/VolumeToMesh.h>

#include <maya/MFnTypedAttribute.h>
#include <maya/MFloatPointArray.h>
#include <maya/MPointArray.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MFnMeshData.h>
#include <maya/MFnStringData.h>
#include <maya/MFnPluginData.h>
#include <maya/MGlobal.h>
#include <maya/MFnMesh.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MArrayDataHandle.h>
#include <maya/MArrayDataBuilder.h>

#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>

#include <memory>
#include <string>
#include <vector>


namespace mvdb = openvdb_maya;


////////////////////////////////////////


struct OpenVDBToPolygonsNode : public MPxNode
{
    OpenVDBToPolygonsNode() {}
    ~OpenVDBToPolygonsNode() override = default;

    MStatus compute(const MPlug& plug, MDataBlock& data) override;

    static void * creator();
    static MStatus initialize();

    static MTypeId id;
    static MObject aVdbInput;
    static MObject aIsovalue;
    static MObject aAdaptivity;
    static MObject aVdbAllGridNames;
    static MObject aVdbSelectedGridNames;
    static MObject aMeshOutput;
};


MTypeId OpenVDBToPolygonsNode::id(0x00108A59);
MObject OpenVDBToPolygonsNode::aVdbInput;
MObject OpenVDBToPolygonsNode::aIsovalue;
MObject OpenVDBToPolygonsNode::aAdaptivity;
MObject OpenVDBToPolygonsNode::aVdbAllGridNames;
MObject OpenVDBToPolygonsNode::aVdbSelectedGridNames;
MObject OpenVDBToPolygonsNode::aMeshOutput;


////////////////////////////////////////


namespace {

mvdb::NodeRegistry registerNode("OpenVDBToPolygons", OpenVDBToPolygonsNode::id,
    OpenVDBToPolygonsNode::creator, OpenVDBToPolygonsNode::initialize);

// Internal utility methods

class VDBToMayaMesh
{
public:
    MObject mesh;

    VDBToMayaMesh(openvdb::tools::VolumeToMesh& mesher): mesh(), mMesher(&mesher) { }

    template<typename GridType>
    inline void operator()(typename GridType::ConstPtr);

private:
    openvdb::tools::VolumeToMesh * const mMesher;
    struct PointCopyOp;
    struct FaceCopyOp;
}; // VDBToMayaMesh


struct VDBToMayaMesh::PointCopyOp
{
    PointCopyOp(MFloatPointArray& mayaPoints, const openvdb::tools::PointList& vdbPoints)
        : mMayaPoints(&mayaPoints) , mVdbPoints(&vdbPoints) { }

    void operator()(const tbb::blocked_range<size_t>& range) const {
        for (size_t n = range.begin(),  N = range.end(); n < N; ++n) {
            const openvdb::Vec3s& p_vdb = (*mVdbPoints)[n];
            MFloatPoint& p_maya = (*mMayaPoints)[static_cast<unsigned int>(n)];
            p_maya[0] = p_vdb[0];
            p_maya[1] = p_vdb[1];
            p_maya[2] = p_vdb[2];
        }
    }

private:
    MFloatPointArray * const mMayaPoints;
    openvdb::tools::PointList const * const mVdbPoints;
};


struct VDBToMayaMesh::FaceCopyOp
{
    using UInt32Array = std::unique_ptr<uint32_t[]>;

    FaceCopyOp(MIntArray& indices, MIntArray& polyCount,
        const UInt32Array& numQuadsPrefix, const UInt32Array& numTrianglesPrefix,
        const openvdb::tools::PolygonPoolList& polygonPoolList)
        : mIndices(&indices)
        , mPolyCount(&polyCount)
        , mNumQuadsPrefix(numQuadsPrefix.get())
        , mNumTrianglesPrefix(numTrianglesPrefix.get())
        , mPolygonPoolList(&polygonPoolList)
    {
    }

    void operator()(const tbb::blocked_range<size_t>& range) const {
        const uint32_t numQuads = mNumQuadsPrefix[range.begin()];
        const uint32_t numTriangles = mNumTrianglesPrefix[range.begin()];

        uint32_t face = numQuads + numTriangles;
        uint32_t vertex = 4*numQuads + 3*numTriangles;

        MIntArray& indices = *mIndices;
        MIntArray& polyCount = *mPolyCount;

        // Setup the polygon count and polygon indices
        for (size_t n = range.begin(), N = range.end(); n < N; ++n) {
            const openvdb::tools::PolygonPool& polygons = (*mPolygonPoolList)[n];
            // Add all quads in the polygon pool
            for (size_t i = 0, I = polygons.numQuads(); i < I; ++i) {
                polyCount[face++] = 4;
                const openvdb::Vec4I& quad = polygons.quad(i);
                indices[vertex++] = quad[0];
                indices[vertex++] = quad[1];
                indices[vertex++] = quad[2];
                indices[vertex++] = quad[3];
            }
            // Add all triangles in the polygon pool
            for (size_t i = 0, I = polygons.numTriangles(); i < I; ++i) {
                polyCount[face++] = 3;
                const openvdb::Vec3I& triangle = polygons.triangle(i);
                indices[vertex++] = triangle[0];
                indices[vertex++] = triangle[1];
                indices[vertex++] = triangle[2];
            }
        }
    }

private:
    MIntArray * const mIndices;
    MIntArray * const mPolyCount;
    uint32_t const * const mNumQuadsPrefix;
    uint32_t const * const mNumTrianglesPrefix;
    openvdb::tools::PolygonPoolList const * const mPolygonPoolList;
};


template<typename GridType>
inline void
VDBToMayaMesh::operator()(typename GridType::ConstPtr grid)
{
    // extract polygonal surface
    (*mMesher)(*grid);

    // transfer quads and triangles
    MIntArray polygonCounts, polygonConnects;
    {
        const size_t polygonPoolListSize = mMesher->polygonPoolListSize();
        std::unique_ptr<uint32_t[]> numQuadsPrefix(new uint32_t[polygonPoolListSize]);
        std::unique_ptr<uint32_t[]> numTrianglesPrefix(new uint32_t[polygonPoolListSize]);
        uint32_t numQuads = 0, numTriangles = 0;

        openvdb::tools::PolygonPoolList& polygonPoolList = mMesher->polygonPoolList();
        for (size_t n = 0; n < polygonPoolListSize; ++n) {
            numQuadsPrefix[n]     = numQuads;
            numTrianglesPrefix[n] = numTriangles;
            numQuads     += uint32_t(polygonPoolList[n].numQuads());
            numTriangles += uint32_t(polygonPoolList[n].numTriangles());
        }

        polygonCounts.setLength(numQuads + numTriangles);
        polygonConnects.setLength(4*numQuads + 3*numTriangles);

        tbb::parallel_for(tbb::blocked_range<size_t>(0, polygonPoolListSize),
            FaceCopyOp(polygonConnects, polygonCounts,
                numQuadsPrefix, numTrianglesPrefix, polygonPoolList));

        polygonPoolList.reset();  // delete polygons
    }

    // transfer points
    const size_t numPoints = mMesher->pointListSize();
    MFloatPointArray vertexArray(static_cast<unsigned int>(numPoints));

    tbb::parallel_for(tbb::blocked_range<size_t>(0, numPoints),
        PointCopyOp(vertexArray, mMesher->pointList()));

    mMesher->pointList().reset(); // delete points

    mesh = MFnMeshData().create();

    MFnMesh().create(vertexArray.length(), polygonCounts.length(),
        vertexArray, polygonCounts, polygonConnects, mesh);
}

} // unnamed namespace


////////////////////////////////////////


void* OpenVDBToPolygonsNode::creator()
{
    return new OpenVDBToPolygonsNode();
}


MStatus OpenVDBToPolygonsNode::initialize()
{
    MStatus stat;
    MFnNumericAttribute nAttr;
    MFnTypedAttribute tAttr;
    // Setup input / output attributes

    aVdbInput = tAttr.create( "vdbInput", "input", OpenVDBData::id, MObject::kNullObj,
                              &stat );

    if (stat != MS::kSuccess) return stat;
    tAttr.setReadable(false);

    stat = addAttribute(aVdbInput);
    if (stat != MS::kSuccess) return stat;

    aMeshOutput = tAttr.create("meshOutput", "mesh", MFnData::kMesh, MObject::kNullObj, &stat);
    if (stat != MS::kSuccess) return stat;

    tAttr.setReadable(true);
    tAttr.setWritable(false);
    tAttr.setStorable(false);
    tAttr.setArray(true);
    tAttr.setUsesArrayDataBuilder(true);

    stat = addAttribute(aMeshOutput);
    if (stat != MS::kSuccess) return stat;

    // Setup UI attributes
    aIsovalue = nAttr.create("isovalue", "iso", MFnNumericData::kFloat);
    nAttr.setDefault(0.0);
    nAttr.setSoftMin(-1.0);
    nAttr.setSoftMax( 1.0);
    nAttr.setConnectable(false);

    stat = addAttribute(aIsovalue);
    if (stat != MS::kSuccess) return stat;

    aAdaptivity = nAttr.create("adaptivity", "adapt", MFnNumericData::kFloat);
    nAttr.setDefault(0.0);
    nAttr.setMin(0.0);
    nAttr.setMax( 1.0);
    nAttr.setConnectable(false);

    stat = addAttribute(aAdaptivity);
    if (stat != MS::kSuccess) return stat;

    MFnStringData fnStringData;
    MObject defaultStringData = fnStringData.create("");

    aVdbAllGridNames = tAttr.create("vdbAllGridNames", "allgrids",
        MFnData::kString, defaultStringData, &stat);
    if (stat != MS::kSuccess) return stat;
    tAttr.setConnectable(false);
    tAttr.setWritable(false);
    tAttr.setReadable(false);
    tAttr.setHidden(true);

    stat = addAttribute(aVdbAllGridNames);
    if (stat != MS::kSuccess) return stat;

    aVdbSelectedGridNames = tAttr.create("vdbSelectedGridNames", "selectedgrids",
        MFnData::kString, defaultStringData, &stat);
    if (stat != MS::kSuccess) return stat;
    tAttr.setConnectable(false);
    tAttr.setWritable(false);
    tAttr.setReadable(false);
    tAttr.setHidden(true);

    stat = addAttribute(aVdbSelectedGridNames);
    if (stat != MS::kSuccess) return stat;

    // Setup dependencies
    attributeAffects(aVdbInput, aVdbAllGridNames);
    attributeAffects(aVdbInput, aMeshOutput);
    attributeAffects(aIsovalue, aMeshOutput);
    attributeAffects(aAdaptivity, aMeshOutput);
    attributeAffects(aVdbSelectedGridNames, aMeshOutput);

    return MS::kSuccess;
}


////////////////////////////////////////


MStatus OpenVDBToPolygonsNode::compute(const MPlug& plug, MDataBlock& data)
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
        return MS::kUnknownParameter;
    }

    // Convert Openvdbs to meshes
    if (plug == aMeshOutput) {
        MDataHandle isoHandle = data.inputValue(aIsovalue, &status);
        if (status != MS::kSuccess) return status;

        MDataHandle adaptHandle = data.inputValue(aAdaptivity, &status);
        if (status != MS::kSuccess) return status;

        openvdb::tools::VolumeToMesh mesher(isoHandle.asFloat(), adaptHandle.asFloat());

        MArrayDataHandle outArrayHandle = data.outputArrayValue(aMeshOutput, &status);
        if (status != MS::kSuccess) return status;

        MArrayDataBuilder builder(aMeshOutput, static_cast<unsigned int>(grids.size()), &status);
        for (size_t n = 0, N = grids.size(); n < N; ++n) {
            VDBToMayaMesh converter(mesher);
            if (mvdb::processTypedScalarGrid(grids[n], converter)) {
                MDataHandle outHandle = builder.addElement(static_cast<unsigned int>(n));
                outHandle.set(converter.mesh);
            }
        }

        status = outArrayHandle.set(builder);
        if (status != MS::kSuccess) return status;

        status = outArrayHandle.setAllClean();
        if (status != MS::kSuccess) return status;
    } else {
        return MS::kUnknownParameter;
    }

    return data.setClean(plug);
}
