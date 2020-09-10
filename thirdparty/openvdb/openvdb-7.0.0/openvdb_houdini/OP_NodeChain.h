// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file OP_NodeChain.h
//
/// @author FX R&D Simulation team
//
/// @brief Utilities to collect a chain of adjacent nodes of a particular type
/// so that they can be cooked in a single step
/// @details For example, adjacent xform SOPs could be collapsed
/// by composing their transformation matrices into a single matrix.

#ifndef HOUDINI_UTILS_OP_NODECHAIN_HAS_BEEN_INCLUDED
#define HOUDINI_UTILS_OP_NODECHAIN_HAS_BEEN_INCLUDED

#include <OP/OP_Context.h>
#include <OP/OP_Channels.h> // for CH_AutoEvaluateTime
#include <OP/OP_Director.h>
#include <OP/OP_Node.h>
#include <SYS/SYS_Types.h> // for fpreal
#include <UT/UT_String.h>
#include <UT/UT_Thread.h>
#include <algorithm>
#include <string>
#include <vector>
#if defined(PRODDEV_BUILD) || defined(DWREAL_IS_DOUBLE)
  // OPENVDB_HOUDINI_API, which has no meaning in a DWA build environment but
  // must at least exist, is normally defined by including openvdb/Platform.h.
  // For DWA builds (i.e., if either PRODDEV_BUILD or DWREAL_IS_DOUBLE exists),
  // that introduces an unwanted and unnecessary library dependency.
  #ifndef OPENVDB_HOUDINI_API
    #define OPENVDB_HOUDINI_API
  #endif
#else
  #include <openvdb/Platform.h>
#endif


namespace houdini_utils {

/// @brief Return a list of adjacent, uncooked nodes of the given @c NodeType,
/// starting from @a startNode and traversing the network upstream
/// along input 0 connections.
/// @details The list is ordered from the topmost node to @a startNode.
/// @note Lock the inputs of the topmost node before cooking the chain.
/// @deprecated This functionality has been marked as deprecated as OpenVDB
/// is no longer using this due to changes relating to verbification. Please get
/// in touch with the Technical Steering Committee if you *are* still relying on
/// this code and would like it to remain in the houdini_utils library.
template<typename NodeType>
OPENVDB_DEPRECATED
inline std::vector<NodeType*>
getNodeChain(OP_Context& context, NodeType* startNode, bool addInterest = true)
{
    struct Local {
        /// Return the nearest upstream node to the given node, traversing
        /// only input 0 connections and omitting bypassed nodes.
        static inline OP_Node* nextInput(
            fpreal now,
            OP_Node* node)
        {
            OP_Node* input = node->getInput(0, /*mark_used=*/true);
            while (input) {
                OP_Node* passThrough = input->getPassThroughNode(now, /*mark_used=*/true);
                if (!passThrough) break;
                input = passThrough;
            }
            return input;
        }
    }; // struct Local

    const fpreal now = context.getTime();

    std::vector<NodeType*> nodes;
    for (OP_Node* node = startNode; node != NULL; node = Local::nextInput(now, node)) {
        // Stop if the node does not need to cook.
        if (!node->needToCook(context, /*query_only=*/true)) break;

        if (NodeType* candidate = dynamic_cast<NodeType*>(node)) {
            nodes.push_back(candidate);
        } else {
            // Stop if the node is not of the requested type, unless it is a SOP_NULL.
            std::string opname = node->getName().toStdString().substr(0, 4);
            if (opname == "null") continue;
            break; // stop for all other node types
        }
    }
    std::reverse(nodes.begin(), nodes.end());

    if (addInterest && startNode != nodes[0] && nodes[0]->getInput(0)) {
        startNode->addExtraInput(nodes[0]->getInput(0), OP_INTEREST_DATA);
    }

    return nodes;
}


////////////////////////////////////////


/// @brief Constructing an OP_EvalScope object allows one to temporarily
/// (for the duration of the current scope) set the evaluation context
/// and time for a node other than the one that is currently being cooked.
/// @internal Entire class is defined in header, so do *NOT* use *_API
/// @deprecated This functionality has been marked as deprecated as OpenVDB
/// is no longer using this due to changes relating to verbification. Please get
/// in touch with the Technical Steering Committee if you *are* still relying on
/// this code and would like it to remain in the houdini_utils library.
class OP_EvalScope
{
public:
    OPENVDB_DEPRECATED
    OP_EvalScope(OP_Node& node, OP_Context& context):
        mAutoEvaluator(
            *OPgetDirector()->getCommandManager(),
            context.getThread(),
            context.getTime(),
            node.getChannels()),
        mDirector(OPgetDirector()),
        mThread(context.getThread())
    {
        mDirector->pushCwd(mThread, &node);
    }

    OPENVDB_DEPRECATED
    ~OP_EvalScope() { mDirector->popCwd(mThread); }

private:
    CH_AutoEvaluateTime mAutoEvaluator;
    OP_Director* mDirector;
    int mThread;
};

} // namespace houdini_utils

#endif // HOUDINI_UTILS_OP_NODECHAIN_HAS_BEEN_INCLUDED
