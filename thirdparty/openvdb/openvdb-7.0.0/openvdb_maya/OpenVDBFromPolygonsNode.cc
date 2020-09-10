// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

/// @author FX R&D OpenVDB team

#include "OpenVDBPlugin.h"
#include <openvdb_maya/OpenVDBData.h>
#include <openvdb_maya/OpenVDBUtil.h>

#include <openvdb/tools/MeshToVolume.h>
#include <openvdb/tools/LevelSetUtil.h>

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

#include <memory> // std::auto_ptr

namespace mvdb = openvdb_maya;


////////////////////////////////////////


struct OpenVDBFromPolygonsNode : public MPxNode
{
    OpenVDBFromPolygonsNode() {}
    virtual ~OpenVDBFromPolygonsNode() {}

    virtual MStatus compute(const MPlug& plug, MDataBlock& data);

    static void* creator();
    static MStatus initialize();

    static MTypeId id;
    static MObject aMeshInput;
    static MObject aVdbOutput;
    static MObject aExportDistanceGrid;
    static MObject aDistanceGridName;
    static MObject aExportDensityGrid;
    static MObject aDensityGridName;
    static MObject aVoxelSize;
    static MObject aExteriorBandWidth;
    static MObject aInteriorBandWidth;
    static MObject aFillInterior;
    static MObject aUnsignedDistanceField;
    static MObject aEstimatedGridResolution;
    static MObject aNodeInfo;
};


MTypeId OpenVDBFromPolygonsNode::id(0x00108A54);
MObject OpenVDBFromPolygonsNode::aMeshInput;
MObject OpenVDBFromPolygonsNode::aVdbOutput;
MObject OpenVDBFromPolygonsNode::aExportDistanceGrid;
MObject OpenVDBFromPolygonsNode::aDistanceGridName;
MObject OpenVDBFromPolygonsNode::aExportDensityGrid;
MObject OpenVDBFromPolygonsNode::aDensityGridName;
MObject OpenVDBFromPolygonsNode::aVoxelSize;
MObject OpenVDBFromPolygonsNode::aExteriorBandWidth;
MObject OpenVDBFromPolygonsNode::aInteriorBandWidth;
MObject OpenVDBFromPolygonsNode::aFillInterior;
MObject OpenVDBFromPolygonsNode::aUnsignedDistanceField;
MObject OpenVDBFromPolygonsNode::aEstimatedGridResolution;
MObject OpenVDBFromPolygonsNode::aNodeInfo;


namespace {
    mvdb::NodeRegistry registerNode("OpenVDBFromPolygons", OpenVDBFromPolygonsNode::id,
        OpenVDBFromPolygonsNode::creator, OpenVDBFromPolygonsNode::initialize);
}


////////////////////////////////////////


void*
OpenVDBFromPolygonsNode::creator()
{
    return new OpenVDBFromPolygonsNode();
}


MStatus
OpenVDBFromPolygonsNode::initialize()
{
    MStatus stat;
    MFnTypedAttribute tAttr;
    MFnNumericAttribute nAttr;

    // Setup the input mesh attribute

    MFnMeshData meshCreator;
    MObject emptyMesh = meshCreator.create(&stat);
    if (stat != MS::kSuccess) return stat;


    MFnStringData fnStringData;
    MObject distName = fnStringData.create("surface");
    MObject densName = fnStringData.create("density");
    MObject emptyStr = fnStringData.create("");

    aMeshInput = tAttr.create("MeshInput", "mesh", MFnData::kMesh, emptyMesh, &stat);
    if (stat != MS::kSuccess) return stat;

    stat = addAttribute(aMeshInput);
    if (stat != MS::kSuccess) return stat;


    // Conversion settings

    aExportDistanceGrid = nAttr.create(
        "ExportDistanceVDB", "exportdistance", MFnNumericData::kBoolean);
    nAttr.setDefault(true);
    nAttr.setConnectable(false);

    stat = addAttribute(aExportDistanceGrid);
    if (stat != MS::kSuccess) return stat;

    aDistanceGridName = tAttr.create(
        "DistanceGridName", "distancename", MFnData::kString, distName, &stat);
    if (stat != MS::kSuccess) return stat;

    tAttr.setConnectable(false);
    stat = addAttribute(aDistanceGridName);
    if (stat != MS::kSuccess) return stat;


    aExportDensityGrid = nAttr.create(
        "ExportDensityVDB", "exportdensity", MFnNumericData::kBoolean);
    nAttr.setDefault(false);
    nAttr.setConnectable(false);

    stat = addAttribute(aExportDensityGrid);
    if (stat != MS::kSuccess) return stat;

    aDensityGridName = tAttr.create(
        "DensityGridName", "densityname", MFnData::kString, densName, &stat);
    if (stat != MS::kSuccess) return stat;

    tAttr.setConnectable(false);
    stat = addAttribute(aDensityGridName);
    if (stat != MS::kSuccess) return stat;


    aVoxelSize = nAttr.create("VoxelSize", "voxelsize", MFnNumericData::kFloat);
    nAttr.setDefault(1.0);
    nAttr.setMin(1e-5);
    nAttr.setSoftMin(0.0);
    nAttr.setSoftMax(10.0);

    stat = addAttribute(aVoxelSize);
    if (stat != MS::kSuccess) return stat;


    aExteriorBandWidth = nAttr.create(
        "ExteriorBandWidth", "exteriorbandwidth", MFnNumericData::kFloat);
    nAttr.setDefault(3.0);
    nAttr.setMin(1.0);
    nAttr.setSoftMin(1.0);
    nAttr.setSoftMax(10.0);

    stat = addAttribute(aExteriorBandWidth);
    if (stat != MS::kSuccess) return stat;

    aInteriorBandWidth = nAttr.create(
        "InteriorBandWidth", "interiorbandwidth", MFnNumericData::kFloat);
    nAttr.setDefault(3.0);
    nAttr.setMin(1.0);
    nAttr.setSoftMin(1.0);
    nAttr.setSoftMax(10.0);

    stat = addAttribute(aInteriorBandWidth);
    if (stat != MS::kSuccess) return stat;


    aFillInterior = nAttr.create("FillInterior", "fillinterior", MFnNumericData::kBoolean);
    nAttr.setDefault(false);
    nAttr.setConnectable(false);

    stat = addAttribute(aFillInterior);
    if (stat != MS::kSuccess) return stat;


    aUnsignedDistanceField = nAttr.create(
        "UnsignedDistanceField", "udf", MFnNumericData::kBoolean);
    nAttr.setDefault(false);
    nAttr.setConnectable(false);

    stat = addAttribute(aUnsignedDistanceField);
    if (stat != MS::kSuccess) return stat;


    // Setup the output attributes

    aVdbOutput = tAttr.create("VdbOutput", "vdb", OpenVDBData::id, MObject::kNullObj, &stat);
    if (stat != MS::kSuccess) return stat;

    tAttr.setWritable(false);
    tAttr.setStorable(false);
    stat = addAttribute(aVdbOutput);
    if (stat != MS::kSuccess) return stat;

    // VDB Info
    aEstimatedGridResolution = tAttr.create(
        "EstimatedGridResolution", "res", MFnData::kString, emptyStr, &stat);
    if (stat != MS::kSuccess) return stat;
    tAttr.setConnectable(false);
    tAttr.setWritable(false);
    stat = addAttribute(aEstimatedGridResolution);
    if (stat != MS::kSuccess) return stat;

    aNodeInfo = tAttr.create("NodeInfo", "info", MFnData::kString, emptyStr, &stat);
    if (stat != MS::kSuccess) return stat;
    tAttr.setConnectable(false);
    tAttr.setWritable(false);
    stat = addAttribute(aNodeInfo);
    if (stat != MS::kSuccess) return stat;

    // Set the attribute dependencies

    attributeAffects(aMeshInput, aVdbOutput);
    attributeAffects(aExportDistanceGrid, aVdbOutput);
    attributeAffects(aDistanceGridName, aVdbOutput);
    attributeAffects(aExportDensityGrid, aVdbOutput);
    attributeAffects(aDensityGridName, aVdbOutput);
    attributeAffects(aVoxelSize, aVdbOutput);
    attributeAffects(aExteriorBandWidth, aVdbOutput);
    attributeAffects(aInteriorBandWidth, aVdbOutput);
    attributeAffects(aFillInterior, aVdbOutput);
    attributeAffects(aUnsignedDistanceField, aVdbOutput);

    attributeAffects(aMeshInput, aEstimatedGridResolution);
    attributeAffects(aVoxelSize, aEstimatedGridResolution);

    attributeAffects(aMeshInput, aNodeInfo);
    attributeAffects(aExportDistanceGrid, aNodeInfo);
    attributeAffects(aDistanceGridName, aNodeInfo);
    attributeAffects(aExportDensityGrid, aNodeInfo);
    attributeAffects(aDensityGridName, aNodeInfo);
    attributeAffects(aVoxelSize, aNodeInfo);
    attributeAffects(aExteriorBandWidth, aNodeInfo);
    attributeAffects(aInteriorBandWidth, aNodeInfo);
    attributeAffects(aFillInterior, aNodeInfo);
    attributeAffects(aUnsignedDistanceField, aNodeInfo);

    return MS::kSuccess;
}


////////////////////////////////////////


MStatus
OpenVDBFromPolygonsNode::compute(const MPlug& plug, MDataBlock& data)
{
    MStatus status;

    if (plug == aEstimatedGridResolution) {

        const float voxelSize = data.inputValue(aVoxelSize, &status).asFloat();
        if (status != MS::kSuccess) return status;
        if (!(voxelSize > 0.0)) return MS::kFailure;

        MDataHandle meshHandle = data.inputValue(aMeshInput, &status);
        if (status != MS::kSuccess) return status;

        MObject tmpObj = meshHandle.asMesh();
        if (tmpObj == MObject::kNullObj) return MS::kFailure;

        MObject obj = meshHandle.asMeshTransformed();
        if (obj == MObject::kNullObj) return MS::kFailure;

        MFnMesh mesh(obj);

        MFloatPointArray vertexArray;
        status = mesh.getPoints(vertexArray, MSpace::kWorld);
        if (status != MS::kSuccess) return status;

        openvdb::Vec3s pmin(std::numeric_limits<float>::max()),
            pmax(-std::numeric_limits<float>::max());

        for(unsigned i = 0, I = vertexArray.length(); i < I; ++i) {
            pmin[0] = std::min(pmin[0], vertexArray[i].x);
            pmin[1] = std::min(pmin[1], vertexArray[i].y);
            pmin[2] = std::min(pmin[2], vertexArray[i].z);
            pmax[0] = std::max(pmax[0], vertexArray[i].x);
            pmax[1] = std::max(pmax[1], vertexArray[i].y);
            pmax[2] = std::max(pmax[2], vertexArray[i].z);
        }

        pmax = (pmax - pmin) / voxelSize;

        int xres = int(std::ceil(pmax[0]));
        int yres = int(std::ceil(pmax[1]));
        int zres = int(std::ceil(pmax[2]));

        std::stringstream txt;
        txt << xres << " x " << yres << " x " << zres << " voxels";

        MString str = txt.str().c_str();
        MDataHandle strHandle = data.outputValue(aEstimatedGridResolution);
        strHandle.set(str);

        return data.setClean(plug);

    } else if (plug == aVdbOutput || plug == aNodeInfo) {

        MDataHandle meshHandle = data.inputValue(aMeshInput, &status);
        if (status != MS::kSuccess) return status;

        {
            MObject obj = meshHandle.asMesh();
            if (obj == MObject::kNullObj) return MS::kFailure;
        }

        MObject obj = meshHandle.asMeshTransformed();
        if (obj == MObject::kNullObj) return MS::kFailure;

        MFnMesh mesh(obj);

        mvdb::Timer computeTimer;
        std::stringstream infoStr;

        const bool exportDistanceGrid = data.inputValue(aExportDistanceGrid, &status).asBool();
        const bool exportDensityGrid = data.inputValue(aExportDensityGrid, &status).asBool();

        MFnPluginData pluginData;
        pluginData.create(OpenVDBData::id, &status);

        if (status != MS::kSuccess) {
            MGlobal::displayError("Failed to create a new OpenVDBData object.");
            return MS::kFailure;
        }

        OpenVDBData* vdb = static_cast<OpenVDBData*>(pluginData.data(&status));
        if (status != MS::kSuccess || !vdb) return MS::kFailure;

        MDataHandle outHandle = data.outputValue(aVdbOutput);

        float voxelSize = data.inputValue(aVoxelSize, &status).asFloat();
        if (status != MS::kSuccess) return status;
        if (!(voxelSize > 0.0)) return MS::kFailure;

        openvdb::math::Transform::Ptr transform;
        try {
            transform = openvdb::math::Transform::createLinearTransform(voxelSize);
        } catch (openvdb::ArithmeticError) {
            MGlobal::displayError("Invalid voxel size.");
            return MS::kFailure;
        }

        MFloatPointArray vertexArray;
        status = mesh.getPoints(vertexArray, MSpace::kWorld);
        if (status != MS::kSuccess) return status;


        openvdb::Vec3d pos;
        std::vector<openvdb::Vec3s> pointList(vertexArray.length());
        for(unsigned i = 0, I = vertexArray.length(); i < I; ++i) {
            pos[0] = double(vertexArray[i].x);
            pos[1] = double(vertexArray[i].y);
            pos[2] = double(vertexArray[i].z);

            pos = transform->worldToIndex(pos);

            pointList[i][0] = float(pos[0]);
            pointList[i][1] = float(pos[1]);
            pointList[i][2] = float(pos[2]);
        }

        std::vector<openvdb::Vec4I> primList;

        MIntArray vertices;
        for (MItMeshPolygon mIt(obj); !mIt.isDone(); mIt.next()) {

            mIt.getVertices(vertices);

            if (vertices.length() < 3) {

                MGlobal::displayWarning(
                    "Skipped unsupported geometry, single point or line primitive.");

            } else if (vertices.length() > 4) {

                MPointArray points;
                MIntArray triangleVerts;
                mIt.getTriangles(points, triangleVerts);

                for(unsigned idx = 0; idx < triangleVerts.length(); idx+=3) {

                    openvdb::Vec4I prim(
                        triangleVerts[idx],
                        triangleVerts[idx+1],
                        triangleVerts[idx+2],
                        openvdb::util::INVALID_IDX);

                    primList.push_back(prim);
                }

            } else {
                mIt.getVertices(vertices);
                openvdb::Vec4I prim(vertices[0], vertices[1], vertices[2],
                    (vertices.length() < 4) ? openvdb::util::INVALID_IDX : vertices[3]);

                primList.push_back(prim);
            }
        }


        infoStr << "Input Mesh\n";
        infoStr << "  nr of points: " << vertexArray.length() << "\n";
        infoStr << "  nr of primitives: " << primList.size() << "\n";



        if (exportDistanceGrid || exportDensityGrid) {

            float exteriorBandWidth = data.inputValue(aExteriorBandWidth, &status).asFloat();
            float interiorBandWidth = exteriorBandWidth;

            int conversionFlags = 0;

            // convert to SDF
            if (!data.inputValue(aUnsignedDistanceField, &status).asBool()) {

                if (!data.inputValue(aFillInterior, &status).asBool()) {
                    interiorBandWidth = data.inputValue(aInteriorBandWidth, &status).asFloat();
                } else {
                    interiorBandWidth = std::numeric_limits<float>::max();
                }

            } else {
                conversionFlags = openvdb::tools::UNSIGNED_DISTANCE_FIELD;
            }

            openvdb::tools::QuadAndTriangleDataAdapter<openvdb::Vec3s, openvdb::Vec4I>
                theMesh(pointList, primList);

            openvdb::FloatGrid::Ptr grid = openvdb::tools::meshToVolume<openvdb::FloatGrid>(
                theMesh, *transform, exteriorBandWidth, interiorBandWidth, conversionFlags);

            // export distance grid
            if (exportDistanceGrid) {
                std::string name = data.inputValue(aDistanceGridName, &status).asString().asChar();
                if (!name.empty()) grid->setName(name);
                vdb->insert(grid);
            }

            // export density grid
            if (exportDensityGrid) {

                std::string name = data.inputValue(aDensityGridName, &status).asString().asChar();
                openvdb::FloatGrid::Ptr densityGrid;

                if (exportDistanceGrid) {
                    densityGrid = grid->deepCopy();
                } else {
                    densityGrid = grid;
                }

                openvdb::tools::sdfToFogVolume(*densityGrid);

                if (!name.empty()) densityGrid->setName(name);
                vdb->insert(densityGrid);
            }

        }

        std::string elapsedTime = computeTimer.elapsedTime();
        mvdb::printGridInfo(infoStr, *vdb);
        infoStr << "Compute Time: " << elapsedTime << "\n";
        mvdb::updateNodeInfo(infoStr, data, aNodeInfo);

        outHandle.set(vdb);
        return data.setClean(plug);
    }

    return MS::kUnknownParameter;
}
