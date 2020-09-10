///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) Ken Museth
//
// All rights reserved. This software is distributed under the
// Mozilla Public License 2.0 ( http://www.mozilla.org/MPL/2.0/ )
//
// Redistributions of source code must retain the above copyright
// and license notice and the following restrictions and disclaimer.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// IN NO EVENT SHALL THE COPYRIGHT HOLDERS' AND CONTRIBUTORS' AGGREGATE
// LIABILITY FOR ALL CLAIMS REGARDLESS OF THEIR BASIS EXCEED US$250.00.
//
///////////////////////////////////////////////////////////////////////////
//
/// @file FindActiveValues.h
///
/// @brief Finds the active values in a tree which intersects a bounding box.
///        Two methods are provided, one that counts the number of active values
///        and one that simply tests if any active values intersect the bbox.
///
/// @warning For repeated calls to the free-standing functions defined below
///          consider instead creating an instance of FindActiveValues
///          and then repeatedly call its member methods. This assumes the tree
///          to be constant between calls but is sightly faster.
///
/// @author Ken Museth

#ifndef OPENVDB_TOOLS_FINDACTIVEVALUES_HAS_BEEN_INCLUDED
#define OPENVDB_TOOLS_FINDACTIVEVALUES_HAS_BEEN_INCLUDED

#include <vector>
#include <openvdb/version.h> // for OPENVDB_VERSION_NAME
#include <openvdb/Types.h>
#include <openvdb/tree/ValueAccessor.h>

#include <tbb/blocked_range.h>
#include <tbb/parallel_reduce.h>

namespace openvdb {
OPENVDB_USE_VERSION_NAMESPACE
namespace OPENVDB_VERSION_NAME {
namespace tools {

/// @brief Returns true if the bounding box intersects any of the active
///        values in a tree, i.e. either active voxels or active tiles.
///
/// @warning For repeated calls to this method consider instead creating an instance of
///          FindActiveValues and then repeatedly call any(). This assumes the tree
///          to be constant between calls but is slightly faster.
///
/// @param tree   const tree to be tested for active values.
/// @param bbox   index bounding box which is intersected against the active values.
template<typename TreeT>
inline bool
anyActiveValues(const TreeT& tree, const CoordBBox &bbox);

/// @brief Returns true if the bounding box intersects none of the active
///        values in a tree, i.e. neither active voxels or active tiles.
///
/// @warning For repeated calls to this method consider instead creating an instance of
///          FindActiveValues and then repeatedly call none(). This assumes the tree
///          to be constant between calls but is slightly faster.
///
/// @param tree   const tree to be tested for active values.
/// @param bbox   index bounding box which is intersected against the active values.
template<typename TreeT>
inline bool
noActiveValues(const TreeT& tree, const CoordBBox &bbox);

/// @brief Returns the number of active values that intersects a bounding box intersects,
///        i.e. the count includes both active voxels and virtual voxels in active tiles.
///
/// @warning For repeated calls to this method consider instead creating an instance of
///          FindActiveValues and then repeatedly call count(). This assumes the tree
///          to be constant between calls but is slightly faster.
///
/// @param tree   const tree to be tested for active values.
/// @param bbox   index bounding box which is intersected against the active values.
template<typename TreeT>
inline Index64
countActiveValues(const TreeT& tree, const CoordBBox &bbox);

//////////////////////////////////////////////////////////////////////////////////////////

/// @brief   Finds the active values in a tree which intersects a bounding box.
///
/// @details Two methods are provided, one that count the number of active values
///          and one that simply tests if any active values intersect the bbox.
///
/// @warning Tree nodes are cached by this class so it's important that the tree is not
///          modified after this class is instantiated and before its methods are called.
template<typename TreeT>
class FindActiveValues
{
public:

    /// @brief Constructor from a const tree, which is assumed not to be modified after construction.
    FindActiveValues(const TreeT& tree);

    /// @brief Default destructor
    ~FindActiveValues();

    /// @brief Initiate this class with a new (or modified) tree.
    void update(const TreeT& tree);

    /// @brief Returns true if the specified bounding box intersects any active values.
    ///
    /// @warning Using a ValueAccessor (i.e. useAccessor = true) can improve performance for especially
    ///          small bounding boxes, but at the cost of no thread-safety. So if multiple threads are
    ///          calling this method concurrently use the default setting, useAccessor = false.
    bool any(const CoordBBox &bbox, bool useAccessor = false) const;

    /// @brief Returns true if the specified bounding box does not intersect any active values.
    ///
    /// @warning Using a ValueAccessor (i.e. useAccessor = true) can improve performance for especially
    ///          small bounding boxes, but at the cost of no thread-safety. So if multiple threads are
    ///          calling this method concurrently use the default setting, useAccessor = false.
    bool none(const CoordBBox &bbox, bool useAccessor = false) const { return !this->any(bbox, useAccessor); }

    /// @brief Returns the number of active voxels intersected by the specified bounding box.
    Index64 count(const CoordBBox &bbox) const;

private:

    // Cleans up internal data structures
    void clear();

    // builds internal data structures
    void init(const TreeT &tree);

    template<typename NodeT>
    typename NodeT::NodeMaskType getBBoxMask(const CoordBBox &bbox, const NodeT* node) const;

    // process leaf node
    inline bool any(const typename TreeT::LeafNodeType* leaf, const CoordBBox &bbox ) const;

    // process leaf node
    inline Index64 count(const typename TreeT::LeafNodeType* leaf, const CoordBBox &bbox ) const;

    // process internal node
    template<typename NodeT>
    bool any(const NodeT* node, const CoordBBox &bbox) const;

    // process internal node
    template<typename NodeT>
    Index64 count(const NodeT* node, const CoordBBox &bbox) const;

    using AccT = tree::ValueAccessor<const TreeT, false/* IsSafe */>;
    using RootChildT = typename TreeT::RootNodeType::ChildNodeType;

    struct NodePairT;

    AccT mAcc;
    std::vector<CoordBBox> mRootTiles;// cache bbox of child nodes (faster to cache than access RootNode)
    std::vector<NodePairT> mRootNodes;// cache bbox of acive tiles (faster to cache than access RootNode)

};// FindActiveValues class

//////////////////////////////////////////////////////////////////////////////////////////

template<typename TreeT>
FindActiveValues<TreeT>::FindActiveValues(const TreeT& tree) : mAcc(tree), mRootTiles(), mRootNodes()
{
    this->init(tree);
}

template<typename TreeT>
FindActiveValues<TreeT>::~FindActiveValues()
{
    this->clear();
}

template<typename TreeT>
void FindActiveValues<TreeT>::update(const TreeT& tree)
{
    this->clear();
    mAcc = AccT(tree);
    this->init(tree);
}

template<typename TreeT>
void FindActiveValues<TreeT>::clear()
{
    mRootNodes.clear();
    mRootTiles.clear();
}

template<typename TreeT>
void FindActiveValues<TreeT>::init(const TreeT& tree)
{
    for (auto i = tree.root().cbeginChildOn(); i; ++i) {
        mRootNodes.emplace_back(i.getCoord(), &*i);
    }
    for (auto i = tree.root().cbeginValueOn(); i; ++i) {
        mRootTiles.emplace_back(CoordBBox::createCube(i.getCoord(), RootChildT::DIM));
    }
}

template<typename TreeT>
bool FindActiveValues<TreeT>::any(const CoordBBox &bbox, bool useAccessor) const
{
    if (useAccessor) {
        if (mAcc.isValueOn( (bbox.min() + bbox.max())>>1 )) return true;
    } else {
        if (mAcc.tree().isValueOn( (bbox.min() + bbox.max())>>1 )) return true;
    }

    for (auto& tile : mRootTiles) {
        if (tile.hasOverlap(bbox)) return true;
    }
    for (auto& node : mRootNodes) {
        if (!node.bbox.hasOverlap(bbox)) {
            continue;
        } else if (node.bbox.isInside(bbox)) {
            return this->any(node.child, bbox);
        } else if (this->any(node.child, bbox)) {
            return true;
        }
    }
    return false;
}

template<typename TreeT>
Index64 FindActiveValues<TreeT>::count(const CoordBBox &bbox) const
{
    Index64 count = 0;
    for (auto& tile : mRootTiles) {//loop over active tiles only
        if (!tile.hasOverlap(bbox)) {
            continue;//ignore non-overlapping tiles
        } else if (tile.isInside(bbox)) {
            return bbox.volume();// bbox is completely inside the active tile
        } else if (bbox.isInside(tile)) {
            count += RootChildT::NUM_VOXELS;
        } else {
            auto tmp = tile;
            tmp.intersect(bbox);
            count += tmp.volume();
        }
    }
    for (auto &node : mRootNodes) {//loop over child nodes of the root node only
        if ( !node.bbox.hasOverlap(bbox) ) {
            continue;//ignore non-overlapping child nodes
        } else if ( node.bbox.isInside(bbox) ) {
            return this->count(node.child, bbox);// bbox is completely inside the child node
        } else {
            count += this->count(node.child, bbox);
        }
    }
    return count;
}

template<typename TreeT>
template<typename NodeT>
typename NodeT::NodeMaskType FindActiveValues<TreeT>::getBBoxMask(const CoordBBox &bbox, const NodeT* node) const
{
    typename NodeT::NodeMaskType mask;
    auto b = node->getNodeBoundingBox();
    assert( bbox.hasOverlap(b) );
    if ( bbox.isInside(b) ) {
        mask.setOn();//node is completely inside the bbox so early out
    } else {
        b.intersect(bbox);
        b.min() &=  NodeT::DIM-1u;
        b.min() >>= NodeT::ChildNodeType::TOTAL;
        b.max() &=  NodeT::DIM-1u;
        b.max() >>= NodeT::ChildNodeType::TOTAL;
        assert( b.hasVolume() );
        auto it = b.begin();
        for (const Coord& x = *it; it; ++it) {
            mask.setOn(x[2] + (x[1] << NodeT::LOG2DIM) + (x[0] << 2*NodeT::LOG2DIM));
        }
    }
    return mask;
}

template<typename TreeT>
template<typename NodeT>
bool FindActiveValues<TreeT>::any(const NodeT* node, const CoordBBox &bbox) const
{
    // Generate a bit mask of the bbox coverage
    auto mask = this->getBBoxMask(bbox, node);

    // Check active tiles
    const auto tmp = mask & node->getValueMask();// prune active the tile mask with the bbox mask
    if (!tmp.isOff()) return true;

    // Check child nodes
    mask &= node->getChildMask();// prune the child mask with the bbox mask
    const auto* table = node->getTable();
    bool test = false;
    for (auto i = mask.beginOn(); !test && i; ++i) {
        test = this->any(table[i.pos()].getChild(), bbox);
    }
    return test;
}

template<typename TreeT>
inline bool FindActiveValues<TreeT>::any(const typename TreeT::LeafNodeType* leaf, const CoordBBox &bbox ) const
{
    bool test = leaf->getValueMask().isOn();

    for (auto i = leaf->cbeginValueOn(); !test && i; ++i) {
        test = bbox.isInside(i.getCoord());
    }
    return test;
}

template<typename TreeT>
inline Index64 FindActiveValues<TreeT>::count(const typename TreeT::LeafNodeType* leaf, const CoordBBox &bbox ) const
{
    Index64 count = 0;
    if (leaf->getValueMask().isOn()) {
        auto b = leaf->getNodeBoundingBox();
        b.intersect(bbox);
        count = b.volume();
    } else {
        for (auto i = leaf->cbeginValueOn(); i; ++i) {
            if (bbox.isInside(i.getCoord())) ++count;
        }
    }
    return count;
}

template<typename TreeT>
template<typename NodeT>
Index64 FindActiveValues<TreeT>::count(const NodeT* node, const CoordBBox &bbox) const
{
    Index64 count = 0;

    // Generate a bit masks
    auto mask = this->getBBoxMask(bbox, node);
    const auto childMask = mask & node->getChildMask();// prune the child mask with the bbox mask
    mask &= node->getValueMask();// prune active tile mask with the bbox mask
    const auto* table = node->getTable();

    {// Check child nodes
        using ChildT = typename NodeT::ChildNodeType;
        using RangeT = tbb::blocked_range<typename std::vector<const ChildT*>::iterator>;
        std::vector<const ChildT*> childNodes(childMask.countOn());
        int j=0;
        for (auto i = childMask.beginOn(); i; ++i, ++j) childNodes[j] = table[i.pos()].getChild();
        count += tbb::parallel_reduce( RangeT(childNodes.begin(), childNodes.end()), 0,
            [&](const RangeT& r, Index64 sum)->Index64 {
                for ( auto i = r.begin(); i != r.end(); ++i ) sum += this->count(*i, bbox);
                return sum;
            }, []( Index64 a, Index64 b )->Index64 { return a+b; }
        );
    }

    {// Check active tiles
        std::vector<Coord> coords(mask.countOn());
        using RangeT = tbb::blocked_range<typename std::vector<Coord>::iterator>;
        int j=0;
        for (auto i = mask.beginOn(); i; ++i, ++j) coords[j] = node->offsetToGlobalCoord(i.pos());
        count += tbb::parallel_reduce( RangeT(coords.begin(), coords.end()), 0,
            [&bbox](const RangeT& r, Index64 sum)->Index64 {
                for ( auto i = r.begin(); i != r.end(); ++i ) {
                    auto b = CoordBBox::createCube(*i, NodeT::ChildNodeType::DIM);
                    b.intersect(bbox);
                    sum += b.volume();
                }
                return sum;
            }, []( Index64 a, Index64 b )->Index64 { return a+b; }
        );
    }

    return count;
}

template<typename TreeT>
struct FindActiveValues<TreeT>::NodePairT
{
    const RootChildT* child;
    const CoordBBox   bbox;
    NodePairT(const Coord& c = Coord(), const RootChildT* p = nullptr)
        : child(p), bbox(CoordBBox::createCube(c, RootChildT::DIM))
    {
    }
};// NodePairT struct

//////////////////////////////////////////////////////////////////////////////////////////

// Implementation of stand-alone function
template<typename TreeT>
inline bool
anyActiveValues(const TreeT& tree, const CoordBBox &bbox)
{
    FindActiveValues<TreeT> op(tree);
    return op.any(bbox);
}

// Implementation of stand-alone function
template<typename TreeT>
inline bool
noActiveValues(const TreeT& tree, const CoordBBox &bbox)
{
    FindActiveValues<TreeT> op(tree);
    return op.none(bbox);
}

// Implementation of stand-alone function
template<typename TreeT>
inline bool
countActiveValues(const TreeT& tree, const CoordBBox &bbox)
{
    FindActiveValues<TreeT> op(tree);
    return op.count(bbox);
}

} // namespace tools
} // namespace OPENVDB_VERSION_NAME
} // namespace openvdb

#endif // OPENVDB_TOOLS_FINDACTIVEVALUES_HAS_BEEN_INCLUDED

// Copyright (c) Ken Museth
// All rights reserved. This software is distributed under the
// Mozilla Public License 2.0 ( http://www.mozilla.org/MPL/2.0/ )
