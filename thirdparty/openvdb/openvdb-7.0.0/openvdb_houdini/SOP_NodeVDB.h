// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file SOP_NodeVDB.h
/// @author FX R&D OpenVDB team
/// @brief Base class for OpenVDB plugins

#ifndef OPENVDB_HOUDINI_SOP_NODEVDB_HAS_BEEN_INCLUDED
#define OPENVDB_HOUDINI_SOP_NODEVDB_HAS_BEEN_INCLUDED

#include <houdini_utils/ParmFactory.h>
#include <openvdb/openvdb.h>
#include <openvdb/Platform.h>
#include <SOP/SOP_Node.h>
#ifndef SESI_OPENVDB
#include <UT/UT_DSOVersion.h>
#endif
#include <UT/UT_Version.h>
#include "SOP_VDBVerbUtils.h"
#include <iosfwd>
#include <string>


class GU_Detail;

namespace openvdb_houdini {

/// @brief Use this class to register a new OpenVDB operator (SOP, POP, etc.)
/// @details This class ensures that the operator uses the appropriate OpPolicy.
/// @sa houdini_utils::OpFactory, houdini_utils::OpPolicy
class OPENVDB_HOUDINI_API OpenVDBOpFactory: public houdini_utils::OpFactory
{
public:
    /// Construct an OpFactory that on destruction registers a new OpenVDB operator type.
    OpenVDBOpFactory(const std::string& english, OP_Constructor, houdini_utils::ParmList&,
        OP_OperatorTable&, houdini_utils::OpFactory::OpFlavor = SOP);

    /// @brief Set the name of the equivalent native operator as shipped with Houdini.
    /// @details This is only needed where the native name policy doesn't provide the correct name.
    /// Pass an empty string to indicate that there is no equivalent native operator.
    OpenVDBOpFactory& setNativeName(const std::string& name);

private:
    std::string mNativeName;
};


////////////////////////////////////////


/// @brief Base class from which to derive OpenVDB-related Houdini SOPs
class OPENVDB_HOUDINI_API SOP_NodeVDB: public SOP_Node
{
public:
    SOP_NodeVDB(OP_Network*, const char*, OP_Operator*);
    ~SOP_NodeVDB() override = default;

    void fillInfoTreeNodeSpecific(UT_InfoTree&, const OP_NodeInfoTreeParms&) override;
    void getNodeSpecificInfoText(OP_Context&, OP_NodeInfoParms&) override;

    /// @brief Return this node's registered verb.
    const SOP_NodeVerb* cookVerb() const override;

    /// @brief Retrieve a group from a geometry detail by parsing a pattern
    /// (typically, the value of a Group parameter belonging to this node).
    /// @throw std::runtime_error if the pattern is nonempty but doesn't match any group.
    /// @todo This is a wrapper for SOP_Node::parsePrimitiveGroups(), so it needs access
    /// to a SOP_Node instance.  But it probably doesn't need to be a SOP_NodeVDB method.
    /// @{
    const GA_PrimitiveGroup* matchGroup(GU_Detail&, const std::string& pattern);
    const GA_PrimitiveGroup* matchGroup(const GU_Detail&, const std::string& pattern);
    /// @}

    /// @name Parameter evaluation
    /// @{

    /// @brief Evaluate a vector-valued parameter.
    openvdb::Vec3f evalVec3f(const char* name, fpreal time) const;
    /// @brief Evaluate a vector-valued parameter.
    openvdb::Vec3R evalVec3R(const char* name, fpreal time) const;
    /// @brief Evaluate a vector-valued parameter.
    openvdb::Vec3i evalVec3i(const char* name, fpreal time) const;
    /// @brief Evaluate a vector-valued parameter.
    openvdb::Vec2R evalVec2R(const char* name, fpreal time) const;
    /// @brief Evaluate a vector-valued parameter.
    openvdb::Vec2i evalVec2i(const char* name, fpreal time) const;

    /// @brief Evaluate a string-valued parameter as an STL string.
    /// @details This method facilitates string parameter evaluation in expressions.
    /// For example,
    /// @code
    /// matchGroup(*gdp, evalStdString("group", time));
    /// @endcode
    std::string evalStdString(const char* name, fpreal time, int index = 0) const;

    /// @}

protected:
    /// @{
    /// @brief To facilitate compilable SOPs, cookMySop() is now final.
    /// Instead, either override SOP_NodeVDB::cookVDBSop() (for a non-compilable SOP)
    /// or override SOP_VDBCacheOptions::cookVDBSop() (for a compilable SOP).
    OP_ERROR cookMySop(OP_Context&) override final;

    virtual OP_ERROR cookVDBSop(OP_Context&) { return UT_ERROR_NONE; }
    /// @}

    OP_ERROR cookMyGuide1(OP_Context&) override;
    //OP_ERROR cookMyGuide2(OP_Context&) override;

    /// @brief Transfer the value of an obsolete parameter that was renamed
    /// to the parameter with the new name.
    /// @details This convenience method is intended to be called from
    /// @c resolveObsoleteParms(), when that function is implemented.
    void resolveRenamedParm(PRM_ParmList& obsoleteParms,
        const char* oldName, const char* newName);

    /// @name Input stealing
    /// @{

    /// @brief Steal the geometry on the specified input if possible, instead of copying the data.
    ///
    /// @details In certain cases where a node's input geometry isn't being shared with
    /// other nodes, it is safe for the node to directly modify the geometry.
    /// Normally, input geometry is shared with the upstream node's output cache,
    /// so for stealing to be possible, the "unload" flag must be set on the upstream node
    /// to inhibit caching.  In addition, reference counting of GEO_PrimVDB shared pointers
    /// ensures we cannot steal data that is in use elsewhere.  When stealing is not possible,
    /// this method falls back to copying the shared pointer, effectively performing
    /// a duplicateSource().
    ///
    /// @param index    the index of the input from which to perform this operation
    /// @param context  the current SOP context is used for cook time for network traversal
    /// @param pgdp     pointer to the SOP's gdp
    /// @param gdh      handle to manage input locking
    /// @param clean    (forwarded to duplicateSource())
    ///
    /// @note Prior to Houdini 13.0, this method peforms a duplicateSource() and unlocks the
    /// inputs to the SOP. From Houdini 13.0 on, this method will insert the existing data
    /// into the detail and update the detail handle in the SOP.
    ///
    /// @warning No attempt to call duplicateSource() or inputGeo() should be made after
    /// calling this method, as there will be no data on the input stream if isSourceStealable()
    /// returns @c true.
    /// @deprecated     verbification renders this redundant
    OPENVDB_DEPRECATED
    OP_ERROR duplicateSourceStealable(const unsigned index,
        OP_Context& context, GU_Detail **pgdp, GU_DetailHandle& gdh, bool clean = true);

    /// @brief Steal the geometry on the specified input if possible, instead of copying the data.
    ///
    /// @details In certain cases where a node's input geometry isn't being shared with
    /// other nodes, it is safe for the node to directly modify the geometry.
    /// Normally, input geometry is shared with the upstream node's output cache,
    /// so for stealing to be possible, the "unload" flag must be set on the upstream node
    /// to inhibit caching.  In addition, reference counting of GEO_PrimVDB shared pointers
    /// ensures we cannot steal data that is in use elsewhere.  When stealing is not possible,
    /// this method falls back to copying the shared pointer, effectively performing
    /// a duplicateSource().
    ///
    /// @note Prior to Houdini 13.0, this method peforms a duplicateSource() and unlocks the
    /// inputs to the SOP. From Houdini 13.0 on, this method will insert the existing data
    /// into the detail and update the detail handle in the SOP.
    ///
    /// @param index    the index of the input from which to perform this operation
    /// @param context  the current SOP context is used for cook time for network traversal
    /// @deprecated     verbification renders this redundant
    OPENVDB_DEPRECATED
    OP_ERROR duplicateSourceStealable(const unsigned index, OP_Context& context);

    /// @}

private:
    /// @brief Traverse the upstream network to determine if the source input can be stolen.
    ///
    /// An upstream SOP cannot be stolen if it is implicitly caching the data (no "unload" flag)
    /// or explictly caching the data (using a Cache SOP)
    ///
    /// The traversal ignores pass through nodes such as null SOPs and bypassing.
    ///
    /// @param index    the index of the input from which to perform this operation
    /// @param context  the current SOP context is used for cook time for network traversal
    /// @deprecated     verbification renders this redundant
    OPENVDB_DEPRECATED
    bool isSourceStealable(const unsigned index, OP_Context& context) const;
}; // class SOP_NodeVDB


////////////////////////////////////////


/// @brief Namespace to hold functionality for registering info text callbacks. Whenever
/// getNodeSpecificInfoText() is called, the default info text is added to MMB output unless
/// a valid callback has been registered for the grid type.
///
/// @details Use node_info_text::registerGridSpecificInfoText<> to register a grid type to
/// a function pointer which matches the ApplyGridSpecificInfoText signature.
///
///    void floatGridText(std::ostream&, const openvdb::GridBase&);
///
///    node_info_text::registerGridSpecificInfoText<openvdb::FloatGrid>(&floatGridText);
///
namespace node_info_text
{
    // The function pointer signature expected when registering an grid type text
    // callback. The grid is passed untyped but is guaranteed to match the registered
    // type.
    using ApplyGridSpecificInfoText = void (*)(std::ostream&, const openvdb::GridBase&);

    /// @brief Register an info text callback to a specific grid type.
    /// @note Does not add the callback if the grid type already has a registered callback.
    /// @param gridType   the grid type as a unique string (see templated
    ///                   registerGridSpecificInfoText<>)
    /// @param callback   a pointer to the callback function to execute
    void registerGridSpecificInfoText(const std::string& gridType,
        ApplyGridSpecificInfoText callback);

    /// @brief Register an info text callback to a templated grid type.
    /// @note Does not add the callback if the grid type already has a registered callback.
    /// @param callback   a pointer to the callback function to execute
    template<typename GridType>
    inline void registerGridSpecificInfoText(ApplyGridSpecificInfoText callback)
    {
        registerGridSpecificInfoText(GridType::gridType(), callback);
    }

} // namespace node_info_text


} // namespace openvdb_houdini

#endif // OPENVDB_HOUDINI_SOP_NODEVDB_HAS_BEEN_INCLUDED
