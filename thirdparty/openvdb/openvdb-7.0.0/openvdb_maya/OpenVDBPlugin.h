// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

/// @author FX R&D OpenVDB team


#ifndef OPENVDB_MAYA_PLUGIN_HAS_BEEN_INCLUDED
#define OPENVDB_MAYA_PLUGIN_HAS_BEEN_INCLUDED

#include <maya/MString.h>
#include <maya/MTypeId.h>
#include <maya/MPxData.h>

#define MNoVersionString
#include <maya/MFnPlugin.h>

////////////////////////////////////////


namespace openvdb_maya {

struct NodeRegistry
{
    NodeRegistry(const MString& typeName, const MTypeId& typeId,
        MCreatorFunction creatorFunction,
        MInitializeFunction initFunction,
        MPxNode::Type type = MPxNode::kDependNode,
        const MString* classification = NULL);

    static void registerNodes(MFnPlugin& plugin, MStatus& status);
    static void deregisterNodes(MFnPlugin& plugin, MStatus& status);
};

} // namespace openvdb_maya


////////////////////////////////////////


#endif // OPENVDB_MAYA_NODE_REGISTRY_HAS_BEEN_INCLUDED
