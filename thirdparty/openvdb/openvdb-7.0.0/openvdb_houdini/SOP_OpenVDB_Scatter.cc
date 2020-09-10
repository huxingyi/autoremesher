// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file SOP_OpenVDB_Scatter.cc
///
/// @author FX R&D OpenVDB team
///
/// @brief Scatter points on a VDB grid, either by fixed count or by
/// global or local point density.

#include <UT/UT_Assert.h>
#include <UT/UT_Version.h>
#include <UT/UT_ParallelUtil.h> // for UTparallelForLightItems()
#include <GA/GA_SplittableRange.h>
#include <houdini_utils/ParmFactory.h>
#include <openvdb_houdini/SOP_NodeVDB.h>
#include <openvdb_houdini/Utils.h>
#include <openvdb/math/Math.h>
#include <openvdb/math/Stencils.h>
#include <openvdb/points/PointDelete.h>
#include <openvdb/points/PointGroup.h>
#include <openvdb/points/PointScatter.h>
#include <openvdb/tools/GridOperators.h> // for tools::cpt()
#include <openvdb/tools/Interpolation.h> // for tools::BoxSampler
#include <openvdb/tools/LevelSetRebuild.h>
#include <openvdb/tools/LevelSetUtil.h>
#include <openvdb/tools/Morphology.h> // for tools::dilateVoxels()
#include <openvdb/tools/PointScatter.h>
#include <openvdb/tree/LeafManager.h>
#include <hboost/algorithm/string/join.hpp>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>



namespace hvdb = openvdb_houdini;
namespace hutil = houdini_utils;


class SOP_OpenVDB_Scatter: public hvdb::SOP_NodeVDB
{
public:
    SOP_OpenVDB_Scatter(OP_Network* net, const char* name, OP_Operator* op);
    ~SOP_OpenVDB_Scatter() override {}

    static OP_Node* factory(OP_Network*, const char*, OP_Operator*);

    class Cache: public SOP_VDBCacheOptions { OP_ERROR cookVDBSop(OP_Context&) override; };

    void syncNodeVersion(const char* oldVersion, const char*, bool*) override;

protected:
    bool updateParmsFlags() override;
    void resolveObsoleteParms(PRM_ParmList*) override;
};


////////////////////////////////////////


// Build UI and register this operator.
void
newSopOperator(OP_OperatorTable* table)
{
    if (table == nullptr) return;

    hutil::ParmList parms;

    parms.add(hutil::ParmFactory(PRM_STRING, "group", "Group")
        .setChoiceList(&hutil::PrimGroupMenuInput1)
        .setTooltip("Specify a subset of the input VDBs over which to scatter points.")
        .setDocumentation(
            "A subset of the input VDBs over which to scatter points"
            " (see [specifying volumes|/model/volumes#group])"));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "keep", "Keep Original Geometry")
        .setDefault(PRMzeroDefaults)
        .setTooltip("If enabled, the incoming geometry will not be deleted."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "vdbpoints", "Scatter VDB Points")
        .setDefault(PRMzeroDefaults)
        .setTooltip("Generate VDB Points instead of Houdini Points."));

    parms.add(hutil::ParmFactory(PRM_ORD, "outputname", "Output Name")
        .setDefault(PRMzeroDefaults)
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, {
            "keep",     "Keep Original Name",
            "append",   "Add Suffix",
            "replace",  "Custom Name",
        })
        .setTooltip(
            "Give the output VDB Points volumes the same names as the input VDBs,\n"
            "or add a suffix to the input name, or use a custom name."));

    parms.add(hutil::ParmFactory(PRM_STRING, "customname", "Custom Name")
        .setDefault("points")
        .setTooltip("The suffix or custom name to be used"));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "dogroup", "")
        .setTypeExtended(PRM_TYPE_TOGGLE_JOIN)
        .setDefault(PRMzeroDefaults));

    parms.add(hutil::ParmFactory(PRM_STRING, "sgroup", "Scatter Group")
        .setDefault(0, "scatter")
        .setTooltip("If enabled, add scattered points to the group with this name."));

    parms.add(hutil::ParmFactory(PRM_INT_J, "seed", "Random Seed")
        .setDefault(PRMzeroDefaults)
        .setTooltip("The random number seed"));

    parms.add(hutil::ParmFactory(PRM_FLT_J, "spread", "Spread")
        .setDefault(PRMoneDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 0.0, PRM_RANGE_RESTRICTED, 1.0)
        .setTooltip(
            "How far each point may be displaced from the center of its voxel or tile,\n"
            "as a fraction of the voxel or tile size\n\n"
            "A value of zero means that the point is placed exactly at the center."
            " A value of one means that the point can be placed randomly anywhere"
            " inside the voxel or tile.\n\n"
            "When the __SDF Domain__ is an __Isosurface__, a value of zero means that the point"
            " is placed exactly on the isosurface, and a value of one means that the point"
            " can be placed randomly anywhere within one voxel of the isosurface.\n"
            "Frustum grids are currently not properly supported, however."));

    parms.add(hutil::ParmFactory(PRM_ORD, "pointmode", "Mode")
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, {
            "count",           "Point Total",
            "density",         "Point Density",
            "pointspervoxel",  "Points Per Voxel",
        })
        .setTooltip(
            "How to determine the number of points to scatter\n\n"
            "Point Total:\n"
            "    Specify a fixed, total point count.\n"
            "Point Density:\n"
            "    Specify the number of points per unit volume.\n"
            "Points Per Voxel:\n"
            "    Specify the number of points per voxel."));

    parms.add(hutil::ParmFactory(PRM_INT_J, "count", "Point Total")
        .setDefault(5000)
        .setRange(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_UI, 10000)
        .setTooltip("The total number of points to scatter"));

    parms.add(hutil::ParmFactory(PRM_FLT_J , "ppv", "Points Per Voxel")
        .setDefault(8)
        .setRange(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_UI, 10)
        .setTooltip("The number of points per voxel"));

    parms.add(hutil::ParmFactory(PRM_FLT_LOG, "density", "Point Density")
        .setDefault(PRMoneDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_UI, 1000000)
        .setTooltip("The number of points per unit volume"));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "multiply", "Scale Density by Voxel Values")
        .setDefault(PRMzeroDefaults)
        .setTooltip(
            "For scalar-valued VDBs other than signed distance fields,"
            " use voxel values as local multipliers for point density."));

    parms.add(hutil::ParmFactory(PRM_ORD, "poscompression", "Position Compression")
        .setDefault(PRMoneDefaults)
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, {
            "none",   "None",
            "int16",  "16-bit Fixed Point",
            "int8",   "8-bit Fixed Point"
        })
        .setTooltip("The position attribute compression setting.")
        .setDocumentation(
            "The position can be stored relative to the center of the voxel.\n"
            "This means it does not require the full 32-bit float representation,\n"
            "but can be quantized to a smaller fixed-point value."));

    parms.add(hutil::ParmFactory(PRM_STRING, "sdfdomain", "SDF Domain")
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, {
            "interior",  "Interior",
            "surface",   "Isosurface",
            "band",      "Narrow Band",
        })
        .setDefault("band")
        .setTooltip(
            "For signed distance field VDBs, the region over which to scatter points\n\n"
            "Interior:\n"
            "    Scatter points inside the specified isosurface.\n"
            "Isosurface:\n"
            "    Scatter points only on the specified isosurface.\n"
            "Narrow Band:\n"
            "    Scatter points only in the narrow band surrounding the zero crossing."));

    parms.add(hutil::ParmFactory(PRM_FLT_J, "isovalue", "Isovalue")
        .setDefault(PRMzeroDefaults)
        .setRange(PRM_RANGE_UI, -1.0, PRM_RANGE_UI, 1.0)
        .setTooltip("The voxel value that determines the isosurface")
        .setDocumentation(
            "The voxel value that determines the isosurface\n\n"
            "For fog volumes, use a value larger than zero."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "cliptoisosurface", "Clip to Isosurface")
        .setDefault(PRMzeroDefaults)
        .setTooltip("When scattering VDB Points, remove points outside the isosurface."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "verbose", "Verbose")
        .setDefault(PRMzeroDefaults)
        .setTooltip("Print the sequence of operations to the terminal."));

    // Obsolete parameters
    hutil::ParmList obsoleteParms;
    obsoleteParms.add(hutil::ParmFactory(PRM_ORD| PRM_TYPE_JOIN_NEXT, "pointMode", "Point"));
    obsoleteParms.add(hutil::ParmFactory(PRM_TOGGLE, "interior", "Scatter Points Inside Level Sets")        .setDefault(PRMzeroDefaults));
    obsoleteParms.add(hutil::ParmFactory(PRM_SEPARATOR, "sep1", ""));
    obsoleteParms.add(hutil::ParmFactory(PRM_SEPARATOR, "sep2", ""));

    // Register the SOP.
    hvdb::OpenVDBOpFactory("VDB Scatter", SOP_OpenVDB_Scatter::factory, parms, *table)
        .setNativeName("")
        .setObsoleteParms(obsoleteParms)
        .addInput("VDBs on which points will be scattered")
        .setVerb(SOP_NodeVerb::COOK_GENERIC, []() { return new SOP_OpenVDB_Scatter::Cache; })
        .setDocumentation("\
#icon: COMMON/openvdb\n\
#tags: vdb\n\
\n\
\"\"\"Scatter Houdini or VDB points on a VDB volume.\"\"\"\n\
\n\
@overview\n\
\n\
This node scatters points randomly on or inside VDB volumes.\n\
The number of points generated can be specified either by fixed count\n\
or by global or local point density.\n\
\n\
Output can be in the form of either Houdini points or VDB Points volumes.\n\
In the latter case, a VDB Points volume is created for each source VDB,\n\
with the same transform and topology as the source.\n\
\n\
For signed distance field or fog volume VDBs, points can be scattered\n\
either throughout the interior of the volume or only on an isosurface.\n\
For level sets, an additional option is to scatter points only in the\n\
[narrow band|http://www.openvdb.org/documentation/doxygen/overview.html#secGrid]\n\
surrounding the zero crossing.\n\
For all other volumes, points are scattered in active voxels.\n\
\n\
@related\n\
- [Node:sop/scatter]\n\
\n\
@examples\n\
\n\
See [openvdb.org|http://www.openvdb.org/download/] for source code\n\
and usage examples.\n");
}


void
SOP_OpenVDB_Scatter::syncNodeVersion(const char* oldVersion, const char*, bool*)
{
    // Since VDB 7.0.0, position compression is now set to 16-bit fixed point
    // by default. Detect if the VDB version that this node was created with
    // was earlier than 7.0.0 and revert back to null compression if so to
    // prevent potentially breaking older scenes.

    // VDB version string prior to 6.2.0 - "17.5.204"
    // VDB version string since 6.2.0 - "vdb6.2.0 houdini17.5.204"

    openvdb::Name oldVersionStr(oldVersion);

    bool disableCompression = false;
    size_t spacePos = oldVersionStr.find_first_of(' ');
    if (spacePos == std::string::npos) {
        // no space in VDB versions prior to 6.2.0
        disableCompression = true;
    } else if (oldVersionStr.size() > 3 && oldVersionStr.substr(0,3) == "vdb") {
        std::string vdbVersion = oldVersionStr.substr(3,spacePos-3);
        // disable compression in VDB version 6.2.1 or earlier
        if (UT_String::compareVersionString(vdbVersion.c_str(), "6.2.1") <= 0) {
            disableCompression = true;
        }
    }

    if (disableCompression) {
        setInt("poscompression", 0, 0, 0);
    }
}


void
SOP_OpenVDB_Scatter::resolveObsoleteParms(PRM_ParmList* obsoleteParms)
{
    if (!obsoleteParms) return;

    PRM_Parm* parm = obsoleteParms->getParmPtr("interior");
    if (parm && !parm->isFactoryDefault()) { // default was to scatter in the narrow band
        setString(UT_String("interior"), CH_STRING_LITERAL, "sdfdomain", 0, 0.0);
    }

    hvdb::SOP_NodeVDB::resolveObsoleteParms(obsoleteParms);
}


bool
SOP_OpenVDB_Scatter::updateParmsFlags()
{
    bool changed = false;

    const fpreal time = 0;

    const auto vdbpoints = evalInt("vdbpoints", /*idx=*/0, time);
    const auto pmode = evalInt("pointmode", /*idx=*/0, time);
    const auto sdfdomain = evalStdString("sdfdomain", time);

    changed |= setVisibleState("count",      (0 == pmode));
    changed |= setVisibleState("density",    (1 == pmode));
    changed |= setVisibleState("multiply",   (1 == pmode));
    changed |= setVisibleState("ppv",        (2 == pmode));
    changed |= setVisibleState("name",       (1 == vdbpoints));
    changed |= setVisibleState("outputname", (1 == vdbpoints));
    changed |= setVisibleState("customname", (1 == vdbpoints));
    changed |= setVisibleState("cliptoisosurface", (1 == vdbpoints));
    changed |= setVisibleState("poscompression", (1 == vdbpoints));

    changed |= enableParm("customname", (0 != evalInt("outputname", 0, time)));
    changed |= enableParm("sgroup", (1 == evalInt("dogroup", 0, time)));
    changed |= enableParm("isovalue", (sdfdomain != "band"));
    changed |= enableParm("cliptoisosurface", (sdfdomain != "band"));

    return changed;
}


////////////////////////////////////////


OP_Node*
SOP_OpenVDB_Scatter::factory(OP_Network* net, const char* name, OP_Operator *op)
{
    return new SOP_OpenVDB_Scatter(net, name, op);
}


SOP_OpenVDB_Scatter::SOP_OpenVDB_Scatter(OP_Network* net, const char* name, OP_Operator* op):
    hvdb::SOP_NodeVDB(net, name, op)
{
}


////////////////////////////////////////


// Simple wrapper class required by openvdb::tools::UniformPointScatter and
// NonUniformPointScatter
class PointAccessor
{
public:
    PointAccessor(GEO_Detail* gdp): mGdp(gdp) {}

    void add(const openvdb::Vec3R& pos)
    {
        const GA_Offset ptoff = mGdp->appendPointOffset();
        mGdp->setPos3(ptoff, pos.x(), pos.y(), pos.z());
    }

protected:
    GEO_Detail* mGdp;
};


////////////////////////////////////////


/// @brief Functor to translate points toward an isosurface
class SnapPointsOp
{
public:
    using Sampler = openvdb::tools::BoxSampler;

    enum class PointType { kInvalid, kHoudini, kVDB };

    // Constructor for Houdini points
    SnapPointsOp(GEO_Detail& detail, const GA_Range& range, float spread, float isovalue,
        bool rebuild, bool dilate, openvdb::BoolGrid::Ptr mask, hvdb::Interrupter* interrupter)
        : mPointType(range.isValid() && !range.empty() ? PointType::kHoudini : PointType::kInvalid)
        , mDetail(&detail)
        , mRange(range)
        , mSpread(spread)
        , mIsovalue(isovalue)
        , mRebuild(rebuild)
        , mDilate(dilate)
        , mMask(mask)
        , mBoss(interrupter)
    {
    }

    // Constructor for VDB points
    SnapPointsOp(openvdb::points::PointDataGrid& vdbpts, float spread, float isovalue,
        bool rebuild, bool dilate, openvdb::BoolGrid::Ptr mask, hvdb::Interrupter* interrupter)
        : mVdbPoints(&vdbpts)
        , mSpread(spread)
        , mIsovalue(isovalue)
        , mRebuild(rebuild)
        , mDilate(dilate)
        , mMask(mask)
        , mBoss(interrupter)
    {
        const auto leafIter = vdbpts.tree().cbeginLeaf();
        const auto descriptor = leafIter->attributeSet().descriptor();
        mAttrIdx = descriptor.find("P");
        mPointType = (mAttrIdx != openvdb::points::AttributeSet::INVALID_POS) ?
            PointType::kVDB : PointType::kInvalid;
    }

    template<typename GridT>
    void operator()(const GridT& aGrid)
    {
        if (mPointType == PointType::kInvalid) return;

        const GridT* grid = &aGrid;

        // Replace the input grid with a rebuilt narrow-band level set, if requested
        // (typically because the isovalue is nonzero).
        typename GridT::Ptr sdf;
        if (mRebuild) {
            const float width = openvdb::LEVEL_SET_HALF_WIDTH;
            sdf = openvdb::tools::levelSetRebuild(*grid, mIsovalue,
                /*exterior=*/width, /*interior=*/width, /*xform=*/nullptr, mBoss);
            if (sdf) {
                grid = sdf.get();
                mMask.reset(); // no need for a mask now that the input is a narrow-band level set
            }
        }

        // Compute the closest point transform of the SDF.
        const auto cpt = [&]() {
            if (!mMask) {
                return openvdb::tools::cpt(*grid, /*threaded=*/true, mBoss);
            } else {
                if (mDilate) {
                    // Dilate the isosurface mask to produce a suitably large CPT mask,
                    // to avoid unnecessary work in case the input is a dense SDF.
                    const int iterations = static_cast<int>(openvdb::LEVEL_SET_HALF_WIDTH);
                    openvdb::tools::dilateVoxels(
                        mMask->tree(), iterations, openvdb::tools::NN_FACE_EDGE);
                }
                return openvdb::tools::cpt(*grid, *mMask, /*threaded=*/true, mBoss);
            }
        }();

        const auto& xform = aGrid.transform();
        if (mPointType == PointType::kHoudini) {
            // Translate Houdini points toward the isosurface.
            UTparallelForLightItems(GA_SplittableRange(mRange), [&](const GA_SplittableRange& r) {
                const auto cptAcc = cpt->getConstAccessor();
                auto start = GA_Offset(GA_INVALID_OFFSET), end = GA_Offset(GA_INVALID_OFFSET);
                for (GA_Iterator it(r); it.blockAdvance(start, end); ) {
                    if (mBoss && mBoss->wasInterrupted()) break;
                    for (auto offset = start; offset < end; ++offset) {
                        openvdb::Vec3d p{UTvdbConvert(mDetail->getPos3(offset))};
                        // Compute the closest surface point by linear interpolation.
                        const auto surfaceP = Sampler::sample(cptAcc, xform.worldToIndex(p));
                        // Translate the input point toward the surface.
                        p = surfaceP + mSpread * (p - surfaceP); // (1-spread)*surfaceP + spread*p
                        mDetail->setPos3(offset, p.x(), p.y(), p.z());
                    }
                }
            });
        } else /*if (mPointType == PointType::kVDB)*/ {
            // Translate VDB points toward the isosurface.
            using LeafMgr = openvdb::tree::LeafManager<openvdb::points::PointDataTree>;
            LeafMgr leafMgr(mVdbPoints->tree());
            UTparallelForLightItems(leafMgr.leafRange(), [&](const LeafMgr::LeafRange& range) {
                const auto cptAcc = cpt->getConstAccessor();
                for (auto leafIter = range.begin(); leafIter; ++leafIter) {
                    if (mBoss && mBoss->wasInterrupted()) break;
                    // Get a handle to this leaf node's point position array.
                    auto& posArray = leafIter->attributeArray(mAttrIdx);
                    openvdb::points::AttributeWriteHandle<openvdb::Vec3f> posHandle(posArray);
                    // For each point in this leaf node...
                    for (auto idxIter = leafIter->beginIndexOn(); idxIter; ++idxIter) {
                        // The point position is in index space and is relative to
                        // the center of the voxel.
                        const auto idxCenter = idxIter.getCoord().asVec3d();
                        const auto idxP = posHandle.get(*idxIter) + idxCenter;
                        // Compute the closest surface point by linear interpolation.
                        const openvdb::Vec3f surfaceP(Sampler::sample(cptAcc, idxP));
                        // Translate the input point toward the surface.
                        auto p = xform.indexToWorld(idxP);
                        p = surfaceP + mSpread * (p - surfaceP); // (1-spread)*surfaceP + spread*p
                        // Transform back to index space relative to the voxel center.
                        posHandle.set(*idxIter, xform.worldToIndex(p) - idxCenter);
                    }
                }
            });
        }
    }

private:
    PointType mPointType = PointType::kInvalid;
    openvdb::points::PointDataGrid* mVdbPoints = nullptr; // VDB points to be processed
    openvdb::Index64 mAttrIdx = openvdb::points::AttributeSet::INVALID_POS;
    GEO_Detail* mDetail = nullptr; // the detail containing Houdini points to be processed
    GA_Range mRange;               // the range of points to be processed
    float mSpread = 1;             // if 0, place points on the isosurface; if 1, don't move them
    float mIsovalue = 0;
    bool mRebuild = false;         // if true, generate a new SDF from the input grid
    bool mDilate = false;          // if true, dilate the isosurface mask
    openvdb::BoolGrid::Ptr mMask;  // an optional isosurface mask
    hvdb::Interrupter* mBoss = nullptr;
}; // class SnapPointsOp


////////////////////////////////////////


struct BaseScatter
{
    using NullCodec = openvdb::points::NullCodec;
    using FixedCodec16 = openvdb::points::FixedPointCodec<false>;
    using FixedCodec8 = openvdb::points::FixedPointCodec<true>;

    using PositionArray = openvdb::points::TypedAttributeArray<openvdb::Vec3f, NullCodec>;
    using PositionArray16 = openvdb::points::TypedAttributeArray<openvdb::Vec3f, FixedCodec16>;
    using PositionArray8 = openvdb::points::TypedAttributeArray<openvdb::Vec3f, FixedCodec8>;

    BaseScatter(const unsigned int seed,
                const float spread,
                hvdb::Interrupter* interrupter)
        : mPoints()
        , mSeed(seed)
        , mSpread(spread)
        , mInterrupter(interrupter) {}
    virtual ~BaseScatter() {}

    /// @brief Print information about the scattered points
    /// @parm name  A name to insert into the printed info
    /// @parm os    The output stream
    virtual void print(const std::string &name, std::ostream& os = std::cout) const
    {
        if (!mPoints) return;
        const openvdb::Index64 points = openvdb::points::pointCount(mPoints->tree());
        const openvdb::Index64 voxels = mPoints->activeVoxelCount();
        os << points << " points into " << voxels << " active voxels in \""
           << name << "\" corresponding to " << (double(points) / double(voxels))
           << " points per voxel." << std::endl;
    }

    inline openvdb::points::PointDataGrid::Ptr points()
    {
        UT_ASSERT(mPoints);
        return mPoints;
    }

protected:
    openvdb::points::PointDataGrid::Ptr mPoints;
    const unsigned int mSeed;
    const float mSpread;
    hvdb::Interrupter* mInterrupter;
}; // BaseScatter


struct VDBUniformScatter : public BaseScatter
{
    VDBUniformScatter(const openvdb::Index64 count,
                      const unsigned int seed,
                      const float spread,
                      const int compression,
                      hvdb::Interrupter* interrupter)
        : BaseScatter(seed, spread, interrupter)
        , mCount(count)
        , mCompression(compression)
    {}

    template <typename PositionT, typename GridT>
    inline void resolveCompression(const GridT& grid)
    {
        using namespace openvdb::points;
        using PointDataGridT =
            openvdb::Grid<typename TreeConverter<typename GridT::TreeType>::Type>;
        mPoints = openvdb::points::uniformPointScatter<
            GridT, std::mt19937, PositionT, PointDataGridT, hvdb::Interrupter>(
                grid, mCount, mSeed, mSpread, mInterrupter);
    }

    template <typename GridT>
    inline void operator()(const GridT& grid)
    {
        if (mCompression == 1) {
            this->resolveCompression<PositionArray16>(grid);
        } else if (mCompression == 2) {
            this->resolveCompression<PositionArray8>(grid);
        } else {
            this->resolveCompression<PositionArray>(grid);
        }
    }

    void print(const std::string &name, std::ostream& os = std::cout) const
    {
        os << "Uniformly scattered ";
        BaseScatter::print(name, os);
    }

    const openvdb::Index64 mCount;
    const int mCompression;
}; // VDBUniformScatter


struct VDBDenseUniformScatter : public BaseScatter
{
    VDBDenseUniformScatter(const float pointsPerVoxel,
                           const unsigned int seed,
                           const float spread,
                           const int compression,
                           hvdb::Interrupter* interrupter)
        : BaseScatter(seed, spread, interrupter)
        , mPointsPerVoxel(pointsPerVoxel)
        , mCompression(compression)
    {}

    template <typename PositionT, typename GridT>
    inline void resolveCompression(const GridT& grid)
    {
        using namespace openvdb::points;
        using PointDataGridT =
            openvdb::Grid<typename TreeConverter<typename GridT::TreeType>::Type>;
        mPoints = denseUniformPointScatter<GridT, std::mt19937, PositionT, PointDataGridT,
                hvdb::Interrupter>(grid, mPointsPerVoxel, mSeed, mSpread, mInterrupter);
    }

    template <typename GridT>
    inline void operator()(const GridT& grid)
    {
        if (mCompression == 1) {
            this->resolveCompression<PositionArray16>(grid);
        } else if (mCompression == 2) {
            this->resolveCompression<PositionArray8>(grid);
        } else {
            this->resolveCompression<PositionArray>(grid);
        }
    }

    void print(const std::string &name, std::ostream& os = std::cout) const
    {
        os << "Dense uniformly scattered ";
        BaseScatter::print(name, os);
    }

    const float mPointsPerVoxel;
    const int mCompression;
}; // VDBDenseUniformScatter


struct VDBNonUniformScatter : public BaseScatter
{
    VDBNonUniformScatter(const float pointsPerVoxel,
                      const unsigned int seed,
                      const float spread,
                      const int compression,
                      hvdb::Interrupter* interrupter)
        : BaseScatter(seed, spread, interrupter)
        , mPointsPerVoxel(pointsPerVoxel)
        , mCompression(compression)
    {}

    template <typename PositionT, typename GridT>
    inline void resolveCompression(const GridT& grid)
    {
        using namespace openvdb::points;
        using PointDataGridT =
            openvdb::Grid<typename TreeConverter<typename GridT::TreeType>::Type>;
        mPoints = nonUniformPointScatter<GridT, std::mt19937, PositionT, PointDataGridT,
                hvdb::Interrupter>(grid, mPointsPerVoxel, mSeed, mSpread, mInterrupter);
    }

    template <typename GridT>
    inline void operator()(const GridT& grid)
    {
        if (mCompression == 1) {
            this->resolveCompression<PositionArray16>(grid);
        } else if (mCompression == 2) {
            this->resolveCompression<PositionArray8>(grid);
        } else {
            this->resolveCompression<PositionArray>(grid);
        }
    }

    void print(const std::string &name, std::ostream& os = std::cout) const
    {
        os << "Non-uniformly scattered ";
        BaseScatter::print(name, os);
    }

    const float mPointsPerVoxel;
    const int mCompression;
}; // VDBNonUniformScatter


template <typename SurfaceGridT>
struct MarkPointsOutsideIso
{
    using GroupIndex = openvdb::points::AttributeSet::Descriptor::GroupIndex;
    using LeafManagerT = openvdb::tree::LeafManager<openvdb::points::PointDataTree>;
    using PositionHandleT =
        openvdb::points::AttributeHandle<openvdb::Vec3f, openvdb::points::NullCodec>;
    using SurfaceValueT = typename SurfaceGridT::ValueType;

    MarkPointsOutsideIso(const SurfaceGridT& grid,
                         const GroupIndex& deadIndex)
        : mGrid(grid)
        , mDeadIndex(deadIndex) {}

    void operator()(const LeafManagerT::LeafRange& range) const {
        openvdb::math::BoxStencil<const SurfaceGridT> stencil(mGrid);
        for (auto leaf = range.begin(); leaf; ++leaf)  {

            PositionHandleT::Ptr positionHandle =
                PositionHandleT::create(leaf->constAttributeArray(0));
            openvdb::points::GroupWriteHandle deadHandle =
                leaf->groupWriteHandle(mDeadIndex);

            for (auto voxel = leaf->cbeginValueOn(); voxel; ++voxel) {

                const openvdb::Coord& ijk = voxel.getCoord();
                const openvdb::Vec3d vec = ijk.asVec3d();

                for (auto iter = leaf->beginIndexVoxel(ijk); iter; ++iter) {
                    const openvdb::Index index = *iter;
                    const openvdb::Vec3d pos = openvdb::Vec3d(positionHandle->get(index)) + vec;

                    stencil.moveTo(pos);
                    if (stencil.interpolation(pos) > openvdb::zeroVal<SurfaceValueT>()) {
                        deadHandle.set(index, true);
                    }
                }
            }
        }
    }

private:
    const SurfaceGridT& mGrid;
    const GroupIndex& mDeadIndex;
}; // MarkPointsOutsideIso


template<typename OpType>
inline bool
process(const UT_VDBType type, const openvdb::GridBase& grid, OpType& op, const std::string* name)
{
    bool success(false);
    success = UTvdbProcessTypedGridTopology(type, grid, op);
    if (!success) {
        success = UTvdbProcessTypedGridPoint(type, grid, op);
    }
    if (name) op.print(*name);
    return success;
}


// Extract an SDF interior mask in which to scatter points.
inline openvdb::BoolGrid::Ptr
extractInteriorMask(const openvdb::GridBase::ConstPtr grid, const float isovalue)
{
    if (grid->isType<openvdb::FloatGrid>()) {
        return openvdb::tools::sdfInteriorMask(
            static_cast<const openvdb::FloatGrid&>(*grid), isovalue);
    } else if (grid->isType<openvdb::DoubleGrid>()) {
        return openvdb::tools::sdfInteriorMask(
            static_cast<const openvdb::DoubleGrid&>(*grid), isovalue);
    }
    return nullptr;
}


// Extract an SDF isosurface mask in which to scatter points.
inline openvdb::BoolGrid::Ptr
extractIsosurfaceMask(const openvdb::GridBase::ConstPtr grid, const float isovalue)
{
    if (grid->isType<openvdb::FloatGrid>()) {
        return openvdb::tools::extractIsosurfaceMask(
            static_cast<const openvdb::FloatGrid&>(*grid), isovalue);
    } else if (grid->isType<openvdb::DoubleGrid>()) {
        return openvdb::tools::extractIsosurfaceMask(
            static_cast<const openvdb::DoubleGrid&>(*grid), double(isovalue));
    }
    return nullptr;
}


// Remove VDB Points scattered outside of a level set
inline void
cullVDBPoints(openvdb::points::PointDataTree& tree,
              const openvdb::GridBase::ConstPtr grid)
{
    const auto leaf = tree.cbeginLeaf();
    if (leaf) {
        using GroupIndex = openvdb::points::AttributeSet::Descriptor::GroupIndex;
        openvdb::points::appendGroup(tree, "dead");
        const GroupIndex idx = leaf->attributeSet().groupIndex("dead");

        openvdb::tree::LeafManager<openvdb::points::PointDataTree>
            leafManager(tree);

        if (grid->isType<openvdb::FloatGrid>()) {
            const openvdb::FloatGrid& typedGrid =
                static_cast<const openvdb::FloatGrid&>(*grid);
            MarkPointsOutsideIso<openvdb::FloatGrid> mark(typedGrid, idx);
            tbb::parallel_for(leafManager.leafRange(), mark);
        }
        else if (grid->isType<openvdb::DoubleGrid>()) {
            const openvdb::DoubleGrid& typedGrid =
                static_cast<const openvdb::DoubleGrid&>(*grid);
            MarkPointsOutsideIso<openvdb::DoubleGrid> mark(typedGrid, idx);
            tbb::parallel_for(leafManager.leafRange(), mark);
        }
        openvdb::points::deleteFromGroup(tree, "dead");
    }
}


////////////////////////////////////////


OP_ERROR
SOP_OpenVDB_Scatter::Cache::cookVDBSop(OP_Context& context)
{
    try {
        hvdb::Interrupter boss("Scattering points on VDBs");

        const fpreal time = context.getTime();
        const bool keepGrids = (0 != evalInt("keep", 0, time));

        const auto* vdbgeo = inputGeo(0);
        if (keepGrids && vdbgeo) {
            gdp->replaceWith(*vdbgeo);
        } else {
            gdp->stashAll();
        }

        const int seed = static_cast<int>(evalInt("seed", 0, time));
        const auto theSpread = static_cast<float>(evalFloat("spread", 0, time));
        const bool verbose = evalInt("verbose", 0, time) != 0;
        const openvdb::Index64 pointCount = evalInt("count", 0, time);
        const float ptsPerVox = static_cast<float>(evalFloat("ppv", 0, time));
        const auto sdfdomain = evalStdString("sdfdomain", time);
        const float density = static_cast<float>(evalFloat("density", 0, time));
        const bool multiplyDensity = evalInt("multiply", 0, time) != 0;
        const auto theIsovalue = static_cast<float>(evalFloat("isovalue", 0, time));
        const int outputName = static_cast<int>(evalInt("outputname", 0, time));
        const std::string customName = evalStdString("customname", time);

        // Get the group of grids to process.
        const GA_PrimitiveGroup* group = matchGroup(*vdbgeo, evalStdString("group", time));

        // Choose a fast random generator with a long period. Drawback here for
        // mt11213b is that it requires 352*sizeof(uint32) bytes.
        using RandGen = std::mersenne_twister_engine<uint32_t, 32, 351, 175, 19,
            0xccab8ee7, 11, 0xffffffff, 7, 0x31b6ab00, 15, 0xffe50000, 17, 1812433253>; // mt11213b
        RandGen mtRand(seed);

        const auto pmode = evalInt("pointmode", 0, time);
        const bool vdbPoints = evalInt("vdbpoints", 0, time) == 1;
        const bool clipPoints = vdbPoints && bool(evalInt("cliptoisosurface", 0, time));
        const int posCompression = vdbPoints ?
            static_cast<int>(evalInt("poscompression", 0, time)) : 0;
        const bool snapPointsToSurface =
            ((sdfdomain == "surface") && !openvdb::math::isApproxEqual(theSpread, 1.0f));

        // If the domain is the isosurface, set the spread to 1 while generating points
        // so that each point ends up snapping to a unique point on the surface.
        const float spread = (snapPointsToSurface ? 1.f : theSpread);

        std::vector<std::string> emptyGrids;
        std::vector<openvdb::points::PointDataGrid::Ptr> pointGrids;
        PointAccessor pointAccessor(gdp);

        const GA_Offset firstOffset = gdp->getNumPointOffsets();

        // Process each VDB primitive (with a non-null grid pointer)
        // that belongs to the selected group.
        for (hvdb::VdbPrimCIterator primIter(vdbgeo, group); primIter; ++primIter) {

            // Retrieve a read-only grid pointer.
            UT_VDBType gridType = primIter->getStorageType();
            openvdb::GridBase::ConstPtr grid = primIter->getConstGridPtr();
            const std::string gridName = primIter.getPrimitiveName().toStdString();

            if (grid->empty()) {
                emptyGrids.push_back(gridName);
                continue;
            }

            const std::string* const name = verbose ? &gridName : nullptr;
            const openvdb::GridClass gridClass = grid->getGridClass();
            const bool isSignedDistance = (gridClass == openvdb::GRID_LEVEL_SET);
            bool performCull = false;

            const auto isovalue = (gridClass != openvdb::GRID_FOG_VOLUME) ? theIsovalue
                : openvdb::math::Clamp(theIsovalue, openvdb::math::Tolerance<float>::value(), 1.f);

            openvdb::BoolGrid::Ptr mask;
            if (sdfdomain != "band") {
                auto iso = isovalue;
                if (clipPoints) {
                    const openvdb::Vec3d voxelSize = grid->voxelSize();
                    const double maxVoxelSize =
                        openvdb::math::Max(voxelSize.x(), voxelSize.y(), voxelSize.z());
                    iso += static_cast<float>(maxVoxelSize / 2.0);
                    performCull = true;
                }

                if (sdfdomain == "interior") {
                    if (isSignedDistance) {
                        // If the input is an SDF, compute a mask of its interior.
                        // (Fog volumes are their own interior masks.)
                        mask = extractInteriorMask(grid, iso);
                    }
                } else if (sdfdomain == "surface") {
                    mask = extractIsosurfaceMask(grid, iso);
                }
                if (mask) {
                    grid = mask;
                    gridType = UT_VDB_BOOL;
                }
            }

            std::string vdbName;
            if (vdbPoints) {
                if (outputName == 0) vdbName = gridName;
                else if (outputName == 1) vdbName = gridName + customName;
                else vdbName = customName;
            }

            openvdb::points::PointDataGrid::Ptr pointGrid;

            const auto postprocessVDBPoints = [&](BaseScatter& scatter, bool cull) {
                pointGrid = scatter.points();
                if (cull) { cullVDBPoints(pointGrid->tree(), primIter->getConstGridPtr()); }
                pointGrid->setName(vdbName);
                pointGrids.push_back(pointGrid);
                if (verbose) scatter.print(gridName);
            };

            using DenseScatterer = openvdb::tools::DenseUniformPointScatter<
                PointAccessor, RandGen, hvdb::Interrupter>;
            using NonuniformScatterer = openvdb::tools::NonUniformPointScatter<
                PointAccessor, RandGen, hvdb::Interrupter>;
            using UniformScatterer = openvdb::tools::UniformPointScatter<
                PointAccessor, RandGen, hvdb::Interrupter>;

            const GA_Offset startOffset = gdp->getNumPointOffsets();

            switch (pmode) {

            case 0: // fixed point count
                if (vdbPoints) { // VDB points
                    VDBUniformScatter scatter(pointCount, seed, spread, posCompression, &boss);
                    if (process(gridType, *grid, scatter, name))  {
                        postprocessVDBPoints(scatter, performCull);
                    }
                } else { // Houdini points
                    UniformScatterer scatter(pointAccessor, pointCount, mtRand, spread, &boss);
                    process(gridType, *grid, scatter, name);
                }
                break;

            case 1: // points per unit volume
                if (multiplyDensity && !isSignedDistance) { // local density
                    if (vdbPoints) { // VDB points
                        const auto dim = grid->transform().voxelSize();
                        VDBNonUniformScatter scatter(static_cast<float>(density * dim.product()),
                            seed, spread, posCompression, &boss);
                        if (!UTvdbProcessTypedGridScalar(gridType, *grid, scatter)) {
                            throw std::runtime_error(
                                "Only scalar grids support voxel scaling of density");
                        }
                        postprocessVDBPoints(scatter, /*cull=*/false);
                    } else { // Houdini points
                        NonuniformScatterer scatter(pointAccessor, density, mtRand, spread, &boss);
                        if (!UTvdbProcessTypedGridScalar(gridType, *grid, scatter)) {
                            throw std::runtime_error(
                                "Only scalar grids support voxel scaling of density");
                        }
                        if (verbose) scatter.print(gridName);
                    }
                } else { // global density
                    if (vdbPoints) { // VDB points
                        const auto dim = grid->transform().voxelSize();
                        const auto totalPointCount = openvdb::Index64(
                            density * dim.product() * double(grid->activeVoxelCount()));
                        VDBUniformScatter scatter(
                            totalPointCount, seed, spread, posCompression, &boss);
                        if (process(gridType, *grid, scatter, name))  {
                            postprocessVDBPoints(scatter, performCull);
                        }
                    } else { // Houdini points
                        UniformScatterer scatter(pointAccessor, density, mtRand, spread, &boss);
                        process(gridType, *grid, scatter, name);
                    }
                }
                break;

            case 2: // points per voxel
                if (vdbPoints) { // VDB points
                    VDBDenseUniformScatter scatter(
                        ptsPerVox, seed, spread, posCompression, &boss);
                    if (process(gridType, *grid, scatter, name))  {
                        postprocessVDBPoints(scatter, performCull);
                    }
                } else { // Houdini points
                    DenseScatterer scatter(pointAccessor, ptsPerVox, mtRand, spread, &boss);
                    process(gridType, *grid, scatter, name);
                }
                break;

            default:
                throw std::runtime_error(
                    "Expected 0, 1 or 2 for \"pointmode\", got " + std::to_string(pmode));
            } // switch pmode

            if (snapPointsToSurface) {
                // Dilate the mask if it is a single-voxel-wide isosurface mask.
                const bool dilate = (mask && (sdfdomain == "surface"));
                // Generate a new SDF if the input is a fog volume or if the isovalue is nonzero.
                const bool rebuild = (!isSignedDistance || !openvdb::math::isApproxZero(isovalue));
                if (!vdbPoints) {
                    const GA_Range range(gdp->getPointMap(),startOffset,gdp->getNumPointOffsets());
                    // Use the original spread value to control how close to the surface points lie.
                    SnapPointsOp op{*gdp, range, theSpread, isovalue, rebuild, dilate, mask, &boss};
                    GEOvdbProcessTypedGridReal(**primIter, op); // process the original input grid
                } else if (vdbPoints && pointGrid) {
                    SnapPointsOp op{*pointGrid, theSpread, isovalue, rebuild, dilate, mask, &boss};
                    GEOvdbProcessTypedGridReal(**primIter, op);
                }
            }
        } // for each grid

        if (!emptyGrids.empty()) {
            std::string s = "The following grids were empty: "
                + hboost::algorithm::join(emptyGrids, ", ");
            addWarning(SOP_MESSAGE, s.c_str());
        }

        // add points to a group if requested
        if (1 == evalInt("dogroup", 0, time)) {
            const std::string groupName = evalStdString("sgroup", time);
            GA_PointGroup* ptgroup = gdp->newPointGroup(groupName.c_str());

            // add the scattered points to this group

            const GA_Offset lastOffset = gdp->getNumPointOffsets();
            ptgroup->addRange(GA_Range(gdp->getPointMap(), firstOffset, lastOffset));

            for (auto& pointGrid: pointGrids) {
                openvdb::points::appendGroup(pointGrid->tree(), groupName);
                openvdb::points::setGroup(pointGrid->tree(), groupName);
            }
        }

        for (auto& pointGrid: pointGrids) {
            hvdb::createVdbPrimitive(*gdp, pointGrid, pointGrid->getName().c_str());
        }
    }
    catch (std::exception& e) {
        addError(SOP_MESSAGE, e.what());
    }
    return error();
}
