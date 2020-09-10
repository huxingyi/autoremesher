// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file Utils.h
/// @author FX R&D Simulation team
/// @brief Utility classes and functions for OpenVDB plugins

#ifndef OPENVDB_HOUDINI_UTILS_HAS_BEEN_INCLUDED
#define OPENVDB_HOUDINI_UTILS_HAS_BEEN_INCLUDED

#include "GU_PrimVDB.h"
#include <OP/OP_Node.h> // for OP_OpTypeId
#include <UT/UT_SharedPtr.h>
#include <UT/UT_Interrupt.h>
#include <openvdb/openvdb.h>
#include <functional>
#include <type_traits>


#ifdef SESI_OPENVDB
#ifdef OPENVDB_HOUDINI_API
    #undef OPENVDB_HOUDINI_API
    #define OPENVDB_HOUDINI_API
#endif
#endif

class GEO_PrimVDB;
class GU_Detail;
class UT_String;

namespace openvdb_houdini {

using Grid = openvdb::GridBase;
using GridPtr = openvdb::GridBase::Ptr;
using GridCPtr = openvdb::GridBase::ConstPtr;
using GridRef = openvdb::GridBase&;
using GridCRef = const openvdb::GridBase&;


/// @brief Iterator over const VDB primitives on a geometry detail
///
/// @details At least until @c GEO_PrimVDB becomes a built-in primitive type
/// (that can be used as the mask for a @c GA_GBPrimitiveIterator), use this
/// iterator to iterate over all VDB grids belonging to a gdp and, optionally,
/// belonging to a particular group.
class OPENVDB_HOUDINI_API VdbPrimCIterator
{
public:
    using FilterFunc = std::function<bool (const GU_PrimVDB&)>;

    /// @param gdp
    ///     the geometry detail over which to iterate
    /// @param group
    ///     a group in the detail over which to iterate (if @c nullptr,
    ///     iterate over all VDB primitives)
    /// @param filter
    ///     an optional function or functor that takes a const reference
    ///     to a GU_PrimVDB and returns a boolean specifying whether
    ///     that primitive should be visited (@c true) or not (@c false)
    explicit VdbPrimCIterator(const GEO_Detail* gdp, const GA_PrimitiveGroup* group = nullptr,
        FilterFunc filter = FilterFunc());

    VdbPrimCIterator(const VdbPrimCIterator&);
    VdbPrimCIterator& operator=(const VdbPrimCIterator&);

    //@{
    /// Advance to the next VDB primitive.
    void advance();
    VdbPrimCIterator& operator++() { advance(); return *this; }
    //@}

    //@{
    /// Return a pointer to the current VDB primitive (@c nullptr if at end).
    const GU_PrimVDB* getPrimitive() const;
    const GU_PrimVDB* operator*() const { return getPrimitive(); }
    const GU_PrimVDB* operator->() const { return getPrimitive(); }
    //@}

    //@{
    GA_Offset getOffset() const { return getPrimitive()->getMapOffset(); }
    GA_Index getIndex() const { return getPrimitive()->getMapIndex(); }
    //@}

    /// Return @c false if there are no more VDB primitives.
    operator bool() const { return getPrimitive() != nullptr; }

    /// @brief Return the value of the current VDB primitive's @c name attribute.
    /// @param defaultName
    ///     if the current primitive has no @c name attribute
    ///     or its name is empty, return this name instead
    UT_String getPrimitiveName(const UT_String& defaultName = "") const;

    /// @brief Return the value of the current VDB primitive's @c name attribute
    /// or, if the name is empty, the primitive's index (as a UT_String).
    UT_String getPrimitiveNameOrIndex() const;

    /// @brief Return a string of the form "N (NAME)", where @e N is
    /// the current VDB primitive's index and @e NAME is the value
    /// of the primitive's @c name attribute.
    /// @param keepEmptyName  if the current primitive has no @c name attribute
    ///     or its name is empty, then if this flag is @c true, return a string
    ///     "N ()", otherwise return a string "N" omitting the empty name
    UT_String getPrimitiveIndexAndName(bool keepEmptyName = true) const;

protected:
    /// Allow primitives to be deleted during iteration.
    VdbPrimCIterator(const GEO_Detail*, GA_Range::safedeletions,
        const GA_PrimitiveGroup* = nullptr, FilterFunc = FilterFunc());

    UT_SharedPtr<GA_GBPrimitiveIterator> mIter;
    FilterFunc mFilter;
}; // class VdbPrimCIterator


/// @brief Iterator over non-const VDB primitives on a geometry detail
///
/// @details At least until @c GEO_PrimVDB becomes a built-in primitive type
/// (that can be used as the mask for a @c GA_GBPrimitiveIterator), use this
/// iterator to iterate over all VDB grids belonging to a gdp and, optionally,
/// belonging to a particular group.
class OPENVDB_HOUDINI_API VdbPrimIterator: public VdbPrimCIterator
{
public:
    /// @param gdp
    ///     the geometry detail over which to iterate
    /// @param group
    ///     a group in the detail over which to iterate (if @c nullptr,
    ///     iterate over all VDB primitives)
    /// @param filter
    ///     an optional function or functor that takes a @c const reference
    ///     to a GU_PrimVDB and returns a boolean specifying whether
    ///     that primitive should be visited (@c true) or not (@c false)
    explicit VdbPrimIterator(GEO_Detail* gdp, const GA_PrimitiveGroup* group = nullptr,
        FilterFunc filter = FilterFunc()):
        VdbPrimCIterator(gdp, group, filter) {}
    /// @brief Allow primitives to be deleted during iteration.
    /// @param gdp
    ///     the geometry detail over which to iterate
    /// @param group
    ///     a group in the detail over which to iterate (if @c nullptr,
    ///     iterate over all VDB primitives)
    /// @param filter
    ///     an optional function or functor that takes a @c const reference
    ///     to a GU_PrimVDB and returns a boolean specifying whether
    ///     that primitive should be visited (@c true) or not (@c false)
    VdbPrimIterator(GEO_Detail* gdp, GA_Range::safedeletions,
        const GA_PrimitiveGroup* group = nullptr, FilterFunc filter = FilterFunc()):
        VdbPrimCIterator(gdp, GA_Range::safedeletions(), group, filter) {}

    VdbPrimIterator(const VdbPrimIterator&);
    VdbPrimIterator& operator=(const VdbPrimIterator&);

    /// Advance to the next VDB primitive.
    VdbPrimIterator& operator++() { advance(); return *this; }

    //@{
    /// Return a pointer to the current VDB primitive (@c nullptr if at end).
    GU_PrimVDB* getPrimitive() const {
        return const_cast<GU_PrimVDB*>(VdbPrimCIterator::getPrimitive());
    }
    GU_PrimVDB* operator*() const { return getPrimitive(); }
    GU_PrimVDB* operator->() const { return getPrimitive(); }
    //@}
}; // class VdbPrimIterator


////////////////////////////////////////


/// @brief Wrapper class that adapts a Houdini @c UT_Interrupt object
/// for use with OpenVDB library routines
/// @sa openvdb/util/NullInterrupter.h
class Interrupter
{
public:
    explicit Interrupter(const char* title = nullptr):
        mUTI{UTgetInterrupt()}, mRunning{false}, mTitle{title ? title : ""}
    {}
    ~Interrupter() { if (mRunning) this->end(); }

    Interrupter(const Interrupter&) = default;
    Interrupter& operator=(const Interrupter&) = default;

    /// @brief Signal the start of an interruptible operation.
    /// @param name  an optional descriptive name for the operation
    void start(const char* name = nullptr) {
        if (!mRunning) { mRunning = true; mUTI->opStart(name ? name : mTitle.c_str()); }
    }
    /// Signal the end of an interruptible operation.
    void end() { if (mRunning) { mUTI->opEnd(); mRunning = false; } }

    /// @brief Check if an interruptible operation should be aborted.
    /// @param percent  an optional (when >= 0) percentage indicating
    ///     the fraction of the operation that has been completed
    bool wasInterrupted(int percent=-1) { return mUTI->opInterrupt(percent); }

private:
    UT_Interrupt* mUTI;
    bool mRunning;
    std::string mTitle;
};


////////////////////////////////////////


// Utility methods

/// @brief Store a VDB grid in a new VDB primitive and add the primitive
/// to a geometry detail.
/// @return the newly-created VDB primitive.
/// @param gdp        the detail to which to add the primitive
/// @param grid       the VDB grid to be added
/// @param name       if non-null, set the new primitive's @c name attribute to this string
/// @note This operation clears the input grid's metadata.
OPENVDB_HOUDINI_API
GU_PrimVDB* createVdbPrimitive(GU_Detail& gdp, GridPtr grid, const char* name = nullptr);


/// @brief Replace an existing VDB primitive with a new primitive that contains
/// the given grid.
/// @return the newly-created VDB primitive.
/// @param gdp        the detail to which to add the primitive
/// @param grid       the VDB grid to be added
/// @param src        replace this primitive with the newly-created primitive
/// @param copyAttrs  if @c true, copy attributes and group membership from the @a src primitive
/// @param name       if non-null, set the new primitive's @c name attribute to this string;
///                   otherwise, if @a copyAttrs is @c true, copy the name from @a src
/// @note This operation clears the input grid's metadata.
OPENVDB_HOUDINI_API
GU_PrimVDB* replaceVdbPrimitive(GU_Detail& gdp, GridPtr grid, GEO_PrimVDB& src,
    const bool copyAttrs = true, const char* name = nullptr);


/// @brief Return in @a corners the corners of the given grid's active voxel bounding box.
/// @return @c false if the grid has no active voxels.
OPENVDB_HOUDINI_API
bool evalGridBBox(GridCRef grid, UT_Vector3 corners[8], bool expandHalfVoxel = false);


/// Construct an index-space CoordBBox from a UT_BoundingBox.
OPENVDB_HOUDINI_API
openvdb::CoordBBox makeCoordBBox(const UT_BoundingBox&, const openvdb::math::Transform&);


/// @{
/// @brief Start forwarding OpenVDB log messages to the Houdini error manager
/// for all operators of the given type.
/// @details Typically, log forwarding is enabled for specific operator types
/// during initialization of the openvdb_houdini library, and there's no need
/// for client code to call this function.
/// @details This function has no effect unless OpenVDB was built with
/// <A HREF="http://log4cplus.sourceforge.net/">log4cplus</A>.
/// @note OpenVDB messages are typically logged to the console as well.
/// This function has no effect on console logging.
/// @sa stopLogForwarding(), isLogForwarding()
OPENVDB_HOUDINI_API
void startLogForwarding(OP_OpTypeId);

/// @brief Stop forwarding OpenVDB log messages to the Houdini error manager
/// for all operators of the given type.
/// @details Typically, log forwarding is enabled for specific operator types
/// during initialization of the openvdb_houdini library, and there's no need
/// for client code to disable it.
/// @details This function has no effect unless OpenVDB was built with
/// <A HREF="http://log4cplus.sourceforge.net/">log4cplus</A>.
/// @note OpenVDB messages are typically logged to the console as well.
/// This function has no effect on console logging.
/// @sa startLogForwarding(), isLogForwarding()
OPENVDB_HOUDINI_API
void stopLogForwarding(OP_OpTypeId);

/// @brief Return @c true if OpenVDB messages logged by operators
/// of the given type are forwarded to the Houdini error manager.
/// @sa startLogForwarding(), stopLogForwarding()
OPENVDB_HOUDINI_API
bool isLogForwarding(OP_OpTypeId);
/// @}


////////////////////////////////////////


// Grid type lists, for use with GEO_PrimVDB::apply(), GEOvdbApply(),
// or openvdb::GridBase::apply()

using ScalarGridTypes = openvdb::TypeList<
    openvdb::BoolGrid,
    openvdb::FloatGrid,
    openvdb::DoubleGrid,
    openvdb::Int32Grid,
    openvdb::Int64Grid>;

using NumericGridTypes = openvdb::TypeList<
    openvdb::FloatGrid,
    openvdb::DoubleGrid,
    openvdb::Int32Grid,
    openvdb::Int64Grid>;

using RealGridTypes = openvdb::TypeList<
    openvdb::FloatGrid,
    openvdb::DoubleGrid>;

using Vec3GridTypes = openvdb::TypeList<
    openvdb::Vec3SGrid,
    openvdb::Vec3DGrid,
    openvdb::Vec3IGrid>;

using PointGridTypes = openvdb::TypeList<
    openvdb::points::PointDataGrid>;

using VolumeGridTypes = ScalarGridTypes::Append<Vec3GridTypes>;

using AllGridTypes = VolumeGridTypes::Append<PointGridTypes>;


/// @brief If the given primitive's grid resolves to one of the listed grid types,
/// invoke the functor @a op on the resolved grid.
/// @return @c true if the functor was invoked, @c false otherwise
template<typename GridTypeListT, typename OpT>
inline bool
GEOvdbApply(const GEO_PrimVDB& vdb, OpT& op)
{
    if (auto gridPtr = vdb.getConstGridPtr()) {
        return gridPtr->apply<GridTypeListT>(op);
    }
    return false;
}

/// @brief If the given primitive's grid resolves to one of the listed grid types,
/// invoke the functor @a op on the resolved grid.
/// @return @c true if the functor was invoked, @c false otherwise
/// @details If @a makeUnique is true, deep copy the grid's tree before
/// invoking the functor if the tree is shared with other grids.
template<typename GridTypeListT, typename OpT>
inline bool
GEOvdbApply(GEO_PrimVDB& vdb, OpT& op, bool makeUnique = true)
{
    if (vdb.hasGrid()) {
        auto gridPtr = vdb.getGridPtr();
        if (makeUnique) {
            auto treePtr = gridPtr->baseTreePtr();
            if (treePtr.use_count() > 2) { // grid + treePtr = 2
                // If the grid resolves to one of the listed types and its tree
                // is shared with other grids, replace the tree with a deep copy.
                gridPtr->apply<GridTypeListT>(
                    [](Grid& baseGrid) { baseGrid.setTree(baseGrid.constBaseTree().copy()); });
            }
        }
        return gridPtr->apply<GridTypeListT>(op);
    }
    return false;
}

} // namespace openvdb_houdini

#endif // OPENVDB_HOUDINI_UTILS_HAS_BEEN_INCLUDED
