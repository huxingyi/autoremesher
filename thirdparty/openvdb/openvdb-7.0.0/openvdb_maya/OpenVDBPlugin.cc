// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

/// @author FX R&D OpenVDB team

#include <openvdb_maya/OpenVDBData.h>
#include "OpenVDBPlugin.h"

#include <openvdb/Platform.h>
#include <openvdb/openvdb.h>
#include <openvdb/Types.h> // compiler pragmas

#include <maya/MIOStream.h>
#include <maya/MFnPlugin.h>
#include <maya/MString.h>
#include <maya/MArgList.h>
#include <maya/MGlobal.h>
#include <maya/MItSelectionList.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MPxData.h>
#include <maya/MTypeId.h>
#include <maya/MPlug.h>
#include <maya/MFnPluginData.h>

#include <tbb/mutex.h>

#include <vector>
#include <sstream>
#include <string>

////////////////////////////////////////


namespace openvdb_maya {


namespace {

struct NodeInfo {
    MString typeName;
    MTypeId typeId;
    MCreatorFunction creatorFunction;
    MInitializeFunction initFunction;
    MPxNode::Type type;
    const MString* classification;
};

typedef std::vector<NodeInfo> NodeList;

typedef tbb::mutex Mutex;
typedef Mutex::scoped_lock Lock;

// Declare this at file scope to ensure thread-safe initialization.
Mutex sRegistryMutex;

NodeList * gNodes = NULL;

} // unnamed namespace


NodeRegistry::NodeRegistry(const MString& typeName, const MTypeId& typeId,
    MCreatorFunction creatorFunction, MInitializeFunction initFunction,
    MPxNode::Type type, const MString* classification)
{
    NodeInfo node;
    node.typeName           = typeName;
    node.typeId             = typeId;
    node.creatorFunction    = creatorFunction;
    node.initFunction       = initFunction;
    node.type               = type;
    node.classification     = classification;

    Lock lock(sRegistryMutex);

    if (!gNodes) {
        OPENVDB_START_THREADSAFE_STATIC_WRITE
        gNodes = new NodeList();
        OPENVDB_FINISH_THREADSAFE_STATIC_WRITE
    }

    gNodes->push_back(node);
}


void
NodeRegistry::registerNodes(MFnPlugin& plugin, MStatus& status)
{
    Lock lock(sRegistryMutex);

    if (gNodes) {
        for (size_t n = 0, N = gNodes->size(); n < N; ++n) {

            const NodeInfo& node = (*gNodes)[n];

            status = plugin.registerNode(node.typeName, node.typeId,
                node.creatorFunction, node.initFunction, node.type, node.classification);

            if (!status) {
                const std::string msg = "Failed to register '" +
                    std::string(node.typeName.asChar()) + "'";
                status.perror(msg.c_str());
                break;
            }
        }
    }
}


void
NodeRegistry::deregisterNodes(MFnPlugin& plugin, MStatus& status)
{
    Lock lock(sRegistryMutex);

    if (gNodes) {
        for (size_t n = 0, N = gNodes->size(); n < N; ++n) {

            const NodeInfo& node = (*gNodes)[n];

            status = plugin.deregisterNode(node.typeId);

            if (!status) {
                const std::string msg = "Failed to deregister '" +
                    std::string(node.typeName.asChar()) + "'";
                status.perror(msg.c_str());
                break;
            }
        }
    }
}

} // namespace openvdb_maya


////////////////////////////////////////


MStatus initializePlugin(MObject);
MStatus uninitializePlugin(MObject);


MStatus
initializePlugin(MObject obj)
{
    openvdb::initialize();

    MStatus status;
    MFnPlugin plugin(obj, "DreamWorks Animation", "0.5", "Any");

    status = plugin.registerData("OpenVDBData", OpenVDBData::id, OpenVDBData::creator);
    if (!status) {
        status.perror("Failed to register 'OpenVDBData'");
        return status;
    }

    openvdb_maya::NodeRegistry::registerNodes(plugin, status);

    return status;
}


MStatus
uninitializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin(obj);

    openvdb_maya::NodeRegistry::deregisterNodes(plugin, status);

    status = plugin.deregisterData(OpenVDBData::id);
    if (!status) {
        status.perror("Failed to deregister 'OpenVDBData'");
        return status;
    }

    return status;
}


////////////////////////////////////////

