// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

/*
 * Copyright (c) Side Effects Software Inc.
 *
 * Produced by:
 *      Side Effects Software Inc
 *      477 Richmond Street West
 *      Toronto, Ontario
 *      Canada   M5V 3E7
 *      416-504-9876
 *
 * NAME:        GEO_PrimVDB.h ( GEO Library, C++)
 *
 * COMMENTS:    Custom VDB primitive.
 */

#include <UT/UT_Version.h>

// Using the native OpenVDB Primitive shipped with Houdini is strongly recommended,
// as there is no guarantee that this code will be kept in sync with Houdini.
// However, for debugging it can be useful, so supply -DSESI_OPENVDB_PRIM to
// the compiler to build this custom primitive.

#if !defined(SESI_OPENVDB) && !defined(SESI_OPENVDB_PRIM)

#include <GEO/GEO_PrimVDB.h>

namespace openvdb_houdini {
using ::GEO_VolumeOptions;
using ::GEO_PrimVDB;
}

#else // SESI_OPENVDB || SESI_OPENVDB_PRIM

#ifndef __HDK_GEO_PrimVDB__
#define __HDK_GEO_PrimVDB__

#include <GEO/GEO_Primitive.h>
#include <GEO/GEO_Vertex.h>
#include <GEO/GEO_VolumeOptions.h>
#include <GA/GA_Defines.h>

#include <SYS/SYS_AtomicInt.h> // for SYS_AtomicCounter

#include <UT/UT_BoundingBox.h>
#include "UT_VDBUtils.h"
#include <UT/UT_Version.h>

#include <openvdb/Platform.h>
#include <openvdb/openvdb.h>


class   GEO_Detail;
class   GEO_PrimVolume;
class   GEO_PrimVolumeXform;
class   UT_MemoryCounter;


class OPENVDB_HOUDINI_API GEO_PrimVDB : public GEO_Primitive
{
public:
    typedef uint64      UniqueId;

protected:
    /// NOTE: The constructor should only be called from subclass
    ///       constructors.
    GEO_PrimVDB(GEO_Detail *d, GA_Offset offset = GA_INVALID_OFFSET);

public:
    static GA_PrimitiveFamilyMask       buildFamilyMask()
                                            { return GA_FAMILY_NONE; }

    /// @{
    /// Required interface methods
    virtual bool        isDegenerate() const;
    virtual int         getBBox(UT_BoundingBox *bbox) const;
    virtual void        reverse();
    virtual UT_Vector3  computeNormal() const;
    virtual void        copyPrimitive(const GEO_Primitive *src);
    virtual void        copyUnwiredForMerge(const GA_Primitive *src,
                                            const GA_MergeMap &map);

    using GEO_Primitive::getVertexOffset;
    using GEO_Primitive::getPointOffset;
    using GEO_Primitive::setPointOffset;
    using GEO_Primitive::getPos3;
    using GEO_Primitive::setPos3;
    SYS_FORCE_INLINE
    GA_Offset getVertexOffset() const
    { return getVertexOffset(0); }
    SYS_FORCE_INLINE
    GA_Offset getPointOffset() const
    { return getPointOffset(0); }
    SYS_FORCE_INLINE
    void setPointOffset(GA_Offset pt)
    { setPointOffset(0, pt); }
    SYS_FORCE_INLINE
    UT_Vector3 getPos3() const
    { return getPos3(0); }
    SYS_FORCE_INLINE
    void setPos3(const UT_Vector3 &pos)
    { setPos3(0, pos); }

    /// Convert an index in the voxel array into the corresponding worldspace
    /// location
    void                indexToPos(int x, int y, int z, UT_Vector3 &pos) const;
    void                findexToPos(UT_Vector3 index, UT_Vector3 &pos) const;

    /// Convert a 3d position into the closest index value.
    void                posToIndex(UT_Vector3 pos, int &x, int &y, int &z) const;
    void                posToIndex(UT_Vector3 pos, UT_Vector3 &index) const;

    /// Evaluate the voxel value at the given world space position.
    /// Note that depending on the underlying VDB type, this may not
    /// be sensible, in which case a zero will silently be returned
    fpreal              getValueF(const UT_Vector3 &pos) const;
    fpreal              getValueAtIndexF(int ix, int iy, int iz) const;
    UT_Vector3D         getValueV3(const UT_Vector3 &pos) const;
    UT_Vector3D         getValueAtIndexV3(int ix, int iy, int iz) const;
    void                getValues(float *f, int stride, const UT_Vector3 *pos, int num) const;
    void                getValues(int *f, int stride, const UT_Vector3 *pos, int num) const;
    void                getValuesAtIndices(float *f, int stride, const int *ix, const int *iy, const int *iz, int num) const;
    void                getValuesAtIndices(int *f, int stride, const int *ix, const int *iy, const int *iz, int num) const;

    /// Vector grid variants.
    void                getValues(UT_Vector3 *f, int stride, const UT_Vector3 *pos, int num) const;
    void                getValuesAtIndices(UT_Vector3 *f, int stride, const int *ix, const int *iy, const int *iz, int num) const;

    // Worldspace gradient at the given position
    UT_Vector3           getGradient(const UT_Vector3 &pos) const;

    /// Evaluate this grid's gradients at the given world space positions.
    /// Does nothing and returns false if grid is non-scalar.
    /// If normalize is true, then the gradients will be normalized to be unit
    /// length.
    bool                evalGradients(
                            UT_Vector3 *gradients,
                            int gradients_stride,
                            const UT_Vector3 *positions,
                            int num_positions,
                            bool normalize = false) const;

    /// Get the storage type of the grid
    UT_VDBType          getStorageType() const
                                { return myGridAccessor.getStorageType(); }
    /// Get the tuple size, usually 1 or 3
    int                 getTupleSize() const
                            { return UTvdbGetGridTupleSize(getStorageType()); }

    bool                 isSDF() const;

    /// True if the two volumes map the same indices to the same positions.
    bool                 isAligned(const GEO_PrimVDB *vdb) const;
    /// True if the two volumes have the same active regions
    bool                 isActiveRegionMatched(const GEO_PrimVDB *vdb) const;

    /// True if we are aligned with the world axes.  Ie, all our
    /// off diagonals are zero and our diagonal is positive.
    bool                 isWorldAxisAligned() const;

    // Transform the matrix associated with this primitive. Translate is
    // ignored.
    virtual void        transform(const UT_Matrix4 &mat);

    /// Accessors for the 4x4 matrix representing the affine transform that
    /// converts from index space voxel coordinates to world space. For frustum
    /// maps, this will be transform as if the taper value is set to 1.
    /// @{
    void                setTransform4(const UT_DMatrix4 &xform4);
    void                setTransform4(const UT_Matrix4 &xform4);
    UT_Matrix4D         getTransform4() const;
    /// @}

    // Take the whole set of points into consideration when applying the
    // point removal operation to this primitive. The method returns 0 if
    // successful, -1 if it failed because it would have become degenerate,
    // and -2 if it failed because it would have had to remove the primitive
    // altogether.
    virtual int          detachPoints(GA_PointGroup &grp);
    /// Before a point is deleted, all primitives using the point will be
    /// notified.  The method should return "false" if it's impossible to
    /// delete the point.  Otherwise, the vertices should be removed.
    virtual GA_DereferenceStatus        dereferencePoint(GA_Offset point,
                                                bool dry_run=false);
    virtual GA_DereferenceStatus        dereferencePoints(
                                                const GA_RangeMemberQuery &pt_q,
                                                bool dry_run=false);
    virtual const GA_PrimitiveJSON      *getJSON() const;

    /// This method assigns a preallocated vertex to the quadric, optionally
    /// creating the topological link between the primitive and new vertex.
    void                 assignVertex(GA_Offset new_vtx, bool update_topology);

    /// Evalaute a point given a u,v coordinate (with derivatives)
    virtual bool        evaluatePointRefMap(GA_Offset result_vtx,
                                GA_AttributeRefMap &hlist,
                                fpreal u, fpreal v, uint du, uint dv) const;
    /// Evalaute position given a u,v coordinate (with derivatives)
    virtual int         evaluatePointV4(UT_Vector4 &pos, float u, float v = 0,
                                        unsigned du=0, unsigned dv=0) const
                        {
                           return GEO_Primitive::evaluatePointV4(pos, u, v,
                                       du, dv);
                        }
    /// @}

    /// Convert transforms between native volumes and VDBs
    /// @{

    /// Get a GEO_PrimVolumeXform which represent's the grid's full transform.
    /// The returned space's fromVoxelSpace() method will convert index space
    /// voxel coordinates to world space positions (and the vice versa for
    /// toVoxelSpace()).
    GEO_PrimVolumeXform getIndexSpaceTransform() const;

    /// Equivalent to getSpaceTransform(getGrid().evalActiveVoxelBoundingBox()).
    /// The returned space's fromVoxelSpace() method will convert 0-1
    /// coordinates over the active voxel bounding box to world space (and vice
    /// versa for toVoxelSpace()).
    GEO_PrimVolumeXform getSpaceTransform() const;

    /// Gives the equivalent to GEO_PrimVolume's getSpaceTransform() by using
    /// the given bounding box to determine the bounds of the transform.
    /// The resulting world space sample points will be offset by half a voxel
    /// so that they match GEO_PrimVolume.
    /// The returned space's fromVoxelSpace() method will convert 0-1
    /// coordinates over the bbox extents to world space (and vice versa for
    /// toVoxelSpace()).
    GEO_PrimVolumeXform getSpaceTransform(const UT_BoundingBoxD &bbox) const;

    /// Sets the transform from a GEO_PrimVolume's getSpaceTransform() by using
    /// the index space [(0,0,0), resolution] bbox. If force_taper is true,
    /// then the resulting transform will always be a NonlinearFrustumMap even
    /// if there is no tapering.
    void                setSpaceTransform(const GEO_PrimVolumeXform &space,
                                          const UT_Vector3R &resolution,
                                          bool force_taper = false);

    /// @}

    fpreal              getTaper() const;

    /// Returns the resolution of the active voxel array.
    /// Does *not* mean the indices go from 0..rx, however!
    void                getRes(int &rx, int &ry, int &rz) const;

    /// Computes the voxel diameter by taking a step in x, y, and z
    /// converting to world space and taking the length of that vector.
    fpreal               getVoxelDiameter() const;

    /// Returns the length of the voxel when you take an x, y, and z step
    UT_Vector3           getVoxelSize() const;

    /// VDBs may either be unbounded, or created with a specific frustum
    /// range.  The latter is important for tapered VDBs that otherwise
    /// have a singularity at the camera location.  Tools can use the
    /// presence of an idxbox as a clipping box in index space.
    /// This does *NOT* relate to getRes - it may be much larger or
    /// even in some cases smaller.
    bool                getFrustumBounds(UT_BoundingBox &idxbox) const;

    enum ActivateOperation
    {
        ACTIVATE_UNION,         // Activate anything in source
        ACTIVATE_INTERSECT,     // Deactivate anything not in source
        ACTIVATE_SUBTRACT,      // Deactivate anything in source
        ACTIVATE_COPY           // Set our activation to match source
    };

    /// Activates voxels given an *index* space bounding box.  This
    /// is an inclusive box.
    /// If this is Frustum VDB, the activation will be clipped by that.
    /// Setting the value only takes effect if the voxels are activated,
    /// deactivated voxels are set to the background.
    void                activateIndexBBox(
                            const openvdb::CoordBBox& bbox,
                            ActivateOperation operation,
                            bool setvalue, fpreal value)
                        {
                            activateIndexBBoxAdapter(
                                &bbox, operation, setvalue, value);
                        }

    /// Activates all of the voxels in this VDB that are touched
    /// by active voxels in the source.
    /// If ignore_transform is true, voxels will be activated
    /// by grid index instead of world space position.
    void                activateByVDB(const GEO_PrimVDB *vdb,
                                    ActivateOperation operation,
                                    bool setvalue, fpreal value,
                                    bool ignore_transform=false);

    /// @{
    /// Though not strictly required (i.e. not pure virtual), these methods
    /// should be implemented for proper behaviour.
    virtual GEO_Primitive       *copy(int preserve_shared_pts = 0) const;

    // Have we been deactivated and stashed?
    virtual void        stashed(bool beingstashed, GA_Offset offset=GA_INVALID_OFFSET);

    /// @}

    /// @{
    /// Optional interface methods.  Though not required, implementing these
    /// will give better behaviour for the new primitive.
    virtual UT_Vector3  baryCenter() const;
    virtual fpreal      calcVolume(const UT_Vector3 &refpt) const;
    /// Calculate the surface area of the active voxels where
    /// a voxel face contributes if it borders an inactive voxel.
    virtual fpreal      calcArea() const;
    /// @}

    /// @{
    /// Enlarge a bounding box by the bounding box of the primitive.  A
    /// return value of false indicates an error in the operation, most
    /// likely an invalid P.  For any attribute other than the position
    /// these methods simply enlarge the bounding box based on the vertex.
    virtual bool         enlargeBoundingBox(UT_BoundingRect &b,
                                        const GA_Attribute *P) const;
    virtual bool         enlargeBoundingBox(UT_BoundingBox &b,
                                        const GA_Attribute *P) const;
    virtual void         enlargePointBounds(UT_BoundingBox &e) const;
    /// @}
    /// Enlarge a bounding sphere to encompass the primitive.  A return value
    /// of false indicates an error in the operation, most likely an invalid
    /// P.  For any attribute other than the position this method simply
    /// enlarges the sphere based on the vertex.
    virtual bool         enlargeBoundingSphere(UT_BoundingSphere &b,
                                        const GA_Attribute *P) const;

    /// Accessor for the local 3x3 affine transform matrix for the primitive.
    /// For frustum maps, this will be transform as if the taper value is set
    /// to 1.
    /// @{
    virtual void        getLocalTransform(UT_Matrix3D &result) const;
    virtual void        setLocalTransform(const UT_Matrix3D &new_mat3);
    /// @}

    /// @internal Hack to condition 4x4 matrices that we avoid creating what
    /// OpenVDB erroneously thinks are singular matrices. Returns true if mat4
    /// was modified.
    static bool         conditionMatrix(UT_Matrix4D &mat4);

    /// Visualization accessors
    /// @{
    const GEO_VolumeOptions &getVisOptions() const  { return myVis; }

    void                setVisualization(
                            GEO_VolumeVis vismode,
                            fpreal iso,
                            fpreal density)
                        {
                            myVis.myMode = vismode;
                            myVis.myIso = iso;
                            myVis.myDensity = density;
                        }
    GEO_VolumeVis       getVisualization() const    { return myVis.myMode; }
    fpreal              getVisIso() const           { return myVis.myIso; }
    fpreal              getVisDensity() const       { return myVis.myDensity; }
    /// @}

    /// Load the order from a JSON value
    bool                loadOrder(const UT_JSONValue &p);

    /// @{
    /// Save/Load vdb to a JSON stream
    bool                saveVDB(UT_JSONWriter &w) const;
    bool                loadVDB(UT_JSONParser &p);
    /// @}

    bool                saveVisualization(
                            UT_JSONWriter &w,
                            const GA_SaveMap &map) const;
    bool                loadVisualization(
                            UT_JSONParser &p,
                            const GA_LoadMap &map);

    /// Method to perform quick lookup of vertex without the virtual call
    GA_Offset           fastVertexOffset(GA_Size UT_IF_ASSERT_P(index)) const
                        {
                            UT_ASSERT_P(index < 1);
                            return getVertexOffset();
                        }

    void        setVertexPoint(int i, GA_Offset pt)
                {
                    if (i == 0)
                        setPointOffset(pt);
                }

    /// @brief Computes the total density of the volume, scaled by
    /// the volume's size. Negative values will be ignored.
    fpreal      calcPositiveDensity() const;

    bool        hasGrid() const { return myGridAccessor.hasGrid(); }

    /// @brief If this primitive's grid's voxel data (i.e., its tree)
    /// is shared, replace the tree with a deep copy of itself that is
    /// not shared with anyone else.
    void                        makeGridUnique()
                                    { myGridAccessor.makeGridUnique(); }

    /// @brief Returns true if the tree is not shared.  If it is not shared,
    /// one can make destructive edits without makeGridUnique.
    bool                        isGridUnique() const
                                    { return myGridAccessor.isGridUnique(); }

    /// @brief Return a reference to this primitive's grid.
    /// @note Calling setGrid() invalidates all references previously returned.
    const openvdb::GridBase &   getConstGrid() const
                                    { return myGridAccessor.getConstGrid(*this); }
    /// @brief Return a reference to this primitive's grid.
    /// @note Calling setGrid() invalidates all references previously returned.
    const openvdb::GridBase &   getGrid() const
                                    { return getConstGrid(); }
    /// @brief Return a reference to this primitive's grid.
    /// @note Calling setGrid() invalidates all references previously returned.
    /// @warning Call makeGridUnique() before modifying the grid's voxel data.
    openvdb::GridBase &         getGrid()
                                {
                                    incrGridUniqueIds();
                                    return myGridAccessor.getGrid(*this);
                                }

    /// @brief Return a shared pointer to this primitive's grid.
    /// @note Calling setGrid() causes the grid to which the shared pointer
    /// refers to be disassociated with this primitive.
    openvdb::GridBase::ConstPtr getConstGridPtr() const
                                    { return myGridAccessor.getConstGridPtr(*this); }
    /// @brief Return a shared pointer to this primitive's grid.
    /// @note Calling setGrid() causes the grid to which the shared pointer
    /// refers to be disassociated with this primitive.
    openvdb::GridBase::ConstPtr getGridPtr() const
                                    { return getConstGridPtr(); }
    /// @brief Return a shared pointer to this primitive's grid.
    /// @note Calling setGrid() causes the grid to which the shared pointer
    /// refers to be disassociated with this primitive.
    /// @warning Call makeGridUnique() before modifying the grid's voxel data.
    openvdb::GridBase::Ptr      getGridPtr()
                                {
                                    incrGridUniqueIds();
                                    return myGridAccessor.getGridPtr(*this);
                                }

    /// @brief Set this primitive's grid to a shallow copy of the given grid.
    /// @note Invalidates all previous getGrid() and getConstGrid() references
    void                        setGrid(const openvdb::GridBase &grid)
                                {
                                    incrGridUniqueIds();
                                    myGridAccessor.setGrid(grid, *this);
                                }

    /// @brief Return a reference to this primitive's grid metadata.
    /// @note Calling setGrid() invalidates all references previously returned.
    const openvdb::MetaMap&     getConstMetadata() const
                                    { return getConstGrid(); }
    /// @brief Return a reference to this primitive's grid metadata.
    /// @note Calling setGrid() invalidates all references previously returned.
    const openvdb::MetaMap&     getMetadata() const
                                    { return getConstGrid(); }
    /// @brief Return a reference to this primitive's grid metadata.
    /// @note Calling setGrid() invalidates all references previously returned.
    openvdb::MetaMap&           getMetadata()
                                {
                                    incrMetadataUniqueId();
                                    return myGridAccessor.getGrid(*this);
                                }

    /// @brief Return the value of this primitive's "name" attribute
    /// in the given detail.
    const char *    getGridName() const;

    /// @brief Return this primitive's serial number.
    /// @details A primitive's serial number never changes.
    UniqueId        getUniqueId() const
                        { return static_cast<UniqueId>(myUniqueId.relaxedLoad()); }

    /// @brief Return the serial number of this primitive's voxel data.
    /// @details The serial number is incremented whenever a non-const
    /// reference or pointer to this primitive's grid is requested
    /// (whether or not the voxel data is ultimately modified).
    UniqueId        getTreeUniqueId() const
                        { return static_cast<UniqueId>(myTreeUniqueId.relaxedLoad()); }
    /// @brief Return the serial number of this primitive's grid metadata.
    /// @details The serial number is incremented whenever a non-const
    /// reference to the metadata or non-const access to the grid is requested
    /// (whether or not the metadata is ultimately modified).
    UniqueId        getMetadataUniqueId() const
                        { return static_cast<UniqueId>(myMetadataUniqueId.relaxedLoad()); }
    /// @brief Return the serial number of this primitive's transform.
    /// @details The serial number is incremented whenever the transform
    /// is modified or non-const access to this primitive's grid is requested
    /// (whether or not the transform is ultimately modified).
    UniqueId        getTransformUniqueId() const
                        { return static_cast<UniqueId>(myTransformUniqueId.relaxedLoad()); }


    /// @brief If this primitive's grid resolves to one of the listed grid types,
    /// invoke the functor @a op on the resolved grid.
    /// @return @c true if the functor was invoked, @c false otherwise
    ///
    /// @par Example:
    /// @code
    /// auto printOp = [](const openvdb::GridBase& grid) { grid.print(); };
    /// const GEO_PrimVDB* prim = ...;
    /// using RealGridTypes = openvdb::TypeList<openvdb::FloatGrid, openvdb::DoubleGrid>;
    /// // Print info about the primitive's grid if it is a floating-point grid.
    /// prim->apply<RealGridTypes>(printOp);
    /// @endcode
    template<typename GridTypeListT, typename OpT>
    bool    apply(OpT& op) const
                { return hasGrid() ? getConstGrid.apply<GridTypeListT>(op) : false; }

    /// @brief If this primitive's grid resolves to one of the listed grid types,
    /// invoke the functor @a op on the resolved grid.
    /// @return @c true if the functor was invoked, @c false otherwise
    /// @details If @a makeUnique is true, deep copy the grid's tree before
    /// invoking the functor if the tree is shared with other grids.
    ///
    /// @par Example:
    /// @code
    /// auto fillOp = [](const auto& grid) { // C++14
    ///     // Convert voxels in the given bounding box into background voxels.
    ///     grid.fill(openvdb::CoordBBox(openvdb::Coord(0), openvdb::Coord(99)),
    ///         grid.background(), /*active=*/false);
    /// };
    /// GEO_PrimVDB* prim = ...;
    /// // Set background voxels in the primitive's grid if it is a floating-point grid.
    /// using RealGridTypes = openvdb::TypeList<openvdb::FloatGrid, openvdb::DoubleGrid>;
    /// prim->apply<RealGridTypes>(fillOp);
    /// @endcode
    template<typename GridTypeListT, typename OpT>
    bool    apply(OpT& op, bool makeUnique = true)
            {
                if (hasGrid()) {
                    auto& grid = myGridAccessor.getGrid(*this);
                    if (makeUnique) {
                        auto treePtr = grid.baseTreePtr();
                        if (treePtr.use_count() > 2) { // grid + treePtr = 2
                            // If the grid resolves to one of the listed types and its tree
                            // is shared with other grids, replace the tree with a deep copy.
                            grid.apply<GridTypeListT>([this](openvdb::GridBase& baseGrid) {
                                baseGrid.setTree(baseGrid.constBaseTree().copy());
                                this->incrTreeUniqueId();
                            });
                        }
                    }
                    if (grid.apply<GridTypeListT>(op)) {
                        incrGridUniqueIds();
                        return true;
                    }
                }
                return false;
            }

protected:
    typedef SYS_AtomicCounter AtomicUniqueId; // 64-bit

    /// Register intrinsic attributes
    GA_DECLARE_INTRINSICS(GA_NO_OVERRIDE)

    /// Return true if the given metadata token is an intrinsic
    static bool         isIntrinsicMetadata(const char *name);

    /// @warning vertexPoint() doesn't check the bounds.  Use with caution.
    GA_Offset           vertexPoint(GA_Size) const
    { return getPointOffset(); }

    /// Report approximate memory usage, excluding sizeof(*this),
    /// because the subclass doesn't have access to myGridAccessor.
    int64               getBaseMemoryUsage() const;

    // This is called by the subclasses to count the
    // memory used by this, excluding sizeof(*this).
    void countBaseMemory(UT_MemoryCounter &counter) const;

    /// @brief Return an ID number that is guaranteed to be unique across
    /// all VDB primitives.
    static UniqueId     nextUniqueId();

    void                incrTreeUniqueId()
                            { myTreeUniqueId.maximum(nextUniqueId()); }
    void                incrMetadataUniqueId()
                            { myMetadataUniqueId.maximum(nextUniqueId()); }
    void                incrTransformUniqueId()
                            { myTransformUniqueId.maximum(nextUniqueId()); }
    void                incrGridUniqueIds()
                        {
                            incrTreeUniqueId();
                            incrMetadataUniqueId();
                            incrTransformUniqueId();
                        }

    /// @brief Replace this primitive's grid with a shallow copy
    /// of another primitive's grid.
    void                copyGridFrom(const GEO_PrimVDB&);

    /// @brief GridAccessor manages access to a GEO_PrimVDB's grid.
    /// @details In keeping with OpenVDB library conventions, the grid
    /// is stored internally by shared pointer.  However, grid objects
    /// are never shared among primitives, though their voxel data
    /// (i.e., their trees) may be shared.
    /// <p>Among other things, GridAccessor
    /// - ensures that each primitive's transform and metadata are unique
    ///   (i.e., not shared with anyone else)
    /// - allows primitives to share voxel data but, via makeGridUnique(),
    ///   provides a way to break the connection
    /// - ensures that the primitive's transform and the grid's transform
    ///   are in sync (specifically, the translation component, which is
    ///   stored independently as a vertex offset).
    class GridAccessor
    {
    public:
        GridAccessor() : myStorageType(UT_VDB_INVALID)
            { }

        void clear()
        {
            myGrid.reset();
            myStorageType = UT_VDB_INVALID;
        }

        openvdb::GridBase &
        getGrid(const GEO_PrimVDB &prim)
            { updateGridTranslates(prim); return *myGrid; }

        const openvdb::GridBase &
        getConstGrid(const GEO_PrimVDB &prim) const
            { updateGridTranslates(prim); return *myGrid; }

        openvdb::GridBase::Ptr
        getGridPtr(const GEO_PrimVDB &prim)
            { updateGridTranslates(prim); return myGrid; }

        openvdb::GridBase::ConstPtr
        getConstGridPtr(const GEO_PrimVDB &prim) const
            { updateGridTranslates(prim); return myGrid; }

        // These accessors will ensure the transform's translate is set into
        // the vertex position.
        void        setGrid(const openvdb::GridBase& grid, GEO_PrimVDB& prim)
                        { setGridAdapter(&grid, prim); }
        void        setTransform(
                        const openvdb::math::Transform &xform,
                        GEO_PrimVDB &prim)
                        { setTransformAdapter(&xform, prim); }

        void            makeGridUnique();
        bool            isGridUnique() const;

        UT_VDBType      getStorageType() const { return myStorageType; }
        bool            hasGrid() const { return myGrid != 0; }

    private:
        void        updateGridTranslates(const GEO_PrimVDB &prim) const;
        void        setVertexPosition(
                        const openvdb::math::Transform &xform,
                        GEO_PrimVDB &prim)
                        { setVertexPositionAdapter(&xform, prim); }

        void        setGridAdapter(const void* grid, GEO_PrimVDB&);
        void        setTransformAdapter(const void* xform, GEO_PrimVDB&);
        void        setVertexPositionAdapter(const void* xform, GEO_PrimVDB&);

    private:
        openvdb::GridBase::Ptr  myGrid;
        UT_VDBType              myStorageType;
    };

private:
    void                activateIndexBBoxAdapter(
                            const void* bbox,
                            ActivateOperation,
                            bool setvalue, fpreal value);


    GridAccessor            myGridAccessor;

    GEO_VolumeOptions       myVis;

    AtomicUniqueId          myUniqueId;
    AtomicUniqueId          myTreeUniqueId;
    AtomicUniqueId          myMetadataUniqueId;
    AtomicUniqueId          myTransformUniqueId;

}; // class GEO_PrimVDB


#ifndef SESI_OPENVDB
namespace openvdb_houdini {
using ::GEO_VolumeOptions;
using ::GEO_PrimVDB;
}
#endif


////////////////////////////////////////


namespace UT_VDBUtils {

// This overload of UT_VDBUtils::callTypedGrid(), for GridBaseType = GEO_PrimVDB,
// calls makeGridUnique() on the primitive just before instantiating and
// invoking the functor on the primitive's grid.  This delays the call
// to makeGridUnique() until it is known to be necessary and thus avoids
// making deep copies of grids of types that won't be processed.
template<typename GridType, typename OpType>
inline void
callTypedGrid(GEO_PrimVDB& prim, OpType& op)
{
    prim.makeGridUnique();

#ifdef _MSC_VER
    op.operator()<GridType>(*(UTverify_cast<GridType*>(&prim.getGrid())));
#else
    op.template operator()<GridType>(*(UTverify_cast<GridType*>(&prim.getGrid())));
#endif
}

// Overload of callTypedGrid() for GridBaseType = const GEO_PrimVDB
template<typename GridType, typename OpType>
inline void
callTypedGrid(const GEO_PrimVDB& prim, OpType& op)
{
#ifdef _MSC_VER
    op.operator()<GridType>(*(UTverify_cast<const GridType*>(&prim.getConstGrid())));
#else
    op.template operator()<GridType>(*(UTverify_cast<const GridType*>(&prim.getConstGrid())));
#endif
}

} // namespace UT_VDBUtils

// Define UTvdbProcessTypedGrid*() (see UT_VDBUtils.h) for grids
// belonging to primitives, for various subsets of grid types.
UT_VDB_DECL_PROCESS_TYPED_GRID(GEO_PrimVDB&)
UT_VDB_DECL_PROCESS_TYPED_GRID(const GEO_PrimVDB&)


////////////////////////////////////////


/// @brief Utility function to process the grid of a const primitive using functor @a op.
/// @details It will invoke @code op.operator()<GridT>(const GridT &grid) @endcode
/// @{
template <typename OpT>
inline bool GEOvdbProcessTypedGrid(const GEO_PrimVDB &vdb, OpT &op)
{
    return UTvdbProcessTypedGrid(vdb.getStorageType(), vdb.getGrid(), op);
}

template <typename OpT>
inline bool GEOvdbProcessTypedGridReal(const GEO_PrimVDB &vdb, OpT &op)
{
    return UTvdbProcessTypedGridReal(vdb.getStorageType(), vdb.getGrid(), op);
}

template <typename OpT>
inline bool GEOvdbProcessTypedGridScalar(const GEO_PrimVDB &vdb, OpT &op)
{
    return UTvdbProcessTypedGridScalar(vdb.getStorageType(), vdb.getGrid(), op);
}

template <typename OpT>
inline bool GEOvdbProcessTypedGridTopology(const GEO_PrimVDB &vdb, OpT &op)
{
    return UTvdbProcessTypedGridTopology(vdb.getStorageType(), vdb.getGrid(), op);
}

template <typename OpT>
inline bool GEOvdbProcessTypedGridVec3(const GEO_PrimVDB &vdb, OpT &op)
{
    return UTvdbProcessTypedGridVec3(vdb.getStorageType(), vdb.getGrid(), op);
}

template <typename OpT>
inline bool GEOvdbProcessTypedGridPoint(const GEO_PrimVDB &vdb, OpT &op)
{
    return UTvdbProcessTypedGridPoint(vdb.getStorageType(), vdb.getGrid(), op);
}
/// @}

/// @brief Utility function to process the grid of a primitive using functor @a op.
/// @param vdb  the primitive whose grid is to be processed
/// @param op  a functor with a call operator of the form
///     @code op.operator()<GridT>(GridT &grid) @endcode
/// @param makeUnique  if @c true, call <tt>vdb.makeGridUnique()</tt> before
///     invoking the functor
/// @{
template <typename OpT>
inline bool GEOvdbProcessTypedGrid(GEO_PrimVDB &vdb, OpT &op, bool makeUnique = true)
{
    if (makeUnique) return UTvdbProcessTypedGrid(vdb.getStorageType(), vdb, op);
    return UTvdbProcessTypedGrid(vdb.getStorageType(), vdb.getGrid(), op);
}

template <typename OpT>
inline bool GEOvdbProcessTypedGridReal(GEO_PrimVDB &vdb, OpT &op, bool makeUnique = true)
{
    if (makeUnique) return UTvdbProcessTypedGridReal(vdb.getStorageType(), vdb, op);
    return UTvdbProcessTypedGridReal(vdb.getStorageType(), vdb.getGrid(), op);
}

template <typename OpT>
inline bool GEOvdbProcessTypedGridScalar(GEO_PrimVDB &vdb, OpT &op, bool makeUnique = true)
{
    if (makeUnique) return UTvdbProcessTypedGridScalar(vdb.getStorageType(), vdb, op);
    return UTvdbProcessTypedGridScalar(vdb.getStorageType(), vdb.getGrid(), op);
}

template <typename OpT>
inline bool GEOvdbProcessTypedGridTopology(GEO_PrimVDB &vdb, OpT &op, bool makeUnique = true)
{
    if (makeUnique) return UTvdbProcessTypedGridTopology(vdb.getStorageType(), vdb, op);
    return UTvdbProcessTypedGridTopology(vdb.getStorageType(), vdb.getGrid(), op);
}

template <typename OpT>
inline bool GEOvdbProcessTypedGridVec3(GEO_PrimVDB &vdb, OpT &op, bool makeUnique = true)
{
    if (makeUnique) return UTvdbProcessTypedGridVec3(vdb.getStorageType(), vdb, op);
    return UTvdbProcessTypedGridVec3(vdb.getStorageType(), vdb.getGrid(), op);
}

template <typename OpT>
inline bool GEOvdbProcessTypedGridPoint(GEO_PrimVDB &vdb, OpT &op, bool makeUnique = true)
{
    if (makeUnique) return UTvdbProcessTypedGridPoint(vdb.getStorageType(), vdb, op);
    return UTvdbProcessTypedGridPoint(vdb.getStorageType(), vdb.getGrid(), op);
}
/// @}

#endif // __HDK_GEO_PrimVDB__

#endif // SESI_OPENVDB || SESI_OPENVDB_PRIM
