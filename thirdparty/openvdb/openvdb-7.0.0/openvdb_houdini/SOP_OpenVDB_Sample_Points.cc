// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file SOP_OpenVDB_Sample_Points.cc
///
/// @author FX R&D OpenVDB team
///
/// @brief Samples OpenVDB grid values as attributes on spatially located particles.
/// Currently the grid values can be scalar (float, double) or vec3 (float, double)
/// but the attributes on the particles are single precision scalar or vec3

#include <houdini_utils/ParmFactory.h>

#include <openvdb_houdini/Utils.h>
#include <openvdb_houdini/PointUtils.h>
#include <openvdb_houdini/SOP_NodeVDB.h>

#include <openvdb/tools/Interpolation.h>  // for box sampler
#include <openvdb/points/PointCount.h>
#include <openvdb/points/PointSample.h>
#include <openvdb/points/IndexFilter.h>   // for MultiGroupFilter

#include <tbb/tick_count.h>                 // for timing
#include <tbb/task.h>                       // for cancel

#include <UT/UT_Interrupt.h>
#include <GA/GA_PageHandle.h>
#include <GA/GA_PageIterator.h>

#include <hboost/algorithm/string/join.hpp>

#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>




namespace hvdb = openvdb_houdini;
namespace hutil = houdini_utils;
namespace cvdb = openvdb;


class SOP_OpenVDB_Sample_Points: public hvdb::SOP_NodeVDB
{
public:
    SOP_OpenVDB_Sample_Points(OP_Network*, const char* name, OP_Operator*);
    ~SOP_OpenVDB_Sample_Points() override {}

    static OP_Node* factory(OP_Network*, const char*, OP_Operator*);

    // The VDB port holds read-only VDBs.
    int isRefInput(unsigned input) const override { return (input == 1); }

    class Cache: public SOP_VDBCacheOptions { OP_ERROR cookVDBSop(OP_Context&) override; };
};


////////////////////////////////////////


// Build UI and register this operator.
void
newSopOperator(OP_OperatorTable* table)
{
    if (table == nullptr) return;

    hutil::ParmList parms;

    parms.add(hutil::ParmFactory(PRM_STRING, "group", "Group")
        .setChoiceList(&hutil::PrimGroupMenuInput2)
        .setTooltip("A subset of the input VDBs to sample")
        .setDocumentation("A subset of the input VDBs to sample"
            " (see [specifying volumes|/model/volumes#group])"));

    parms.add(hutil::ParmFactory(PRM_STRING, "vdbpointsgroups", "VDB Points Groups")
        .setTooltip("Subsets of VDB points to sample onto")
        .setDocumentation(
            "Subsets of VDB points to sample onto\n\n"
            "See [Node:sop/vdbpointsgroup] for details on grouping VDB points.\n\n"
            "This parameter has no effect if there are no input VDB point data primitives.")
        .setChoiceList(&hvdb::VDBPointsGroupMenuInput1));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "renamevel", "Rename Vel to V")
        .setDefault(PRMzeroDefaults)
        .setDocumentation("If an input VDB's name is \"`vel`\", name the point attribute \"`v`\".")
        .setTooltip("If an input VDB's name is \"vel\", name the point attribute \"v\"."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "attributeexists", "Report Existing Attributes")
        .setDefault(PRMzeroDefaults)
        .setTooltip("Display a warning if a point attribute being sampled into already exists."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "verbose", "Verbose")
        .setDefault(PRMzeroDefaults)
        .setTooltip("Print the sequence of operations to the terminal."));

    // Obsolete parameters
    hutil::ParmList obsoleteParms;
    obsoleteParms.add(hutil::ParmFactory(PRM_TOGGLE, "threaded", "Multi-threading"));
    obsoleteParms.add(hutil::ParmFactory(PRM_SEPARATOR, "sep2", "Separator"));

    // Register the SOP
    hvdb::OpenVDBOpFactory("VDB Sample Points",
        SOP_OpenVDB_Sample_Points::factory, parms, *table)
        .setNativeName("")
        .setObsoleteParms(obsoleteParms)
        .addInput("Points")
        .addInput("VDBs")
        .setVerb(SOP_NodeVerb::COOK_INPLACE, []() { return new SOP_OpenVDB_Sample_Points::Cache; })
        .setDocumentation("\
#icon: COMMON/openvdb\n\
#tags: vdb\n\
\n\
\"\"\"Sample VDB voxel values onto points.\"\"\"\n\
\n\
@overview\n\
\n\
This node samples VDB voxel values into point attributes, where the points\n\
may be either standard Houdini points or points stored in VDB point data grids.\n\
Currently, the voxel values can be single- or double-precision scalars or vectors,\n\
but the attributes on the points will be single-precision only.\n\
\n\
Point attributes are given the same names as the VDBs from which they are sampled.\n\
\n\
@related\n\
- [OpenVDB From Particles|Node:sop/DW_OpenVDBFromParticles]\n\
- [Node:sop/vdbfromparticles]\n\
- [Node:sop/convertvdbpoints]\n\
\n\
@examples\n\
\n\
See [openvdb.org|http://www.openvdb.org/download/] for source code\n\
and usage examples.\n");

}


////////////////////////////////////////


OP_Node*
SOP_OpenVDB_Sample_Points::factory(OP_Network* net, const char* name, OP_Operator *op)
{
    return new SOP_OpenVDB_Sample_Points(net, name, op);
}


SOP_OpenVDB_Sample_Points::SOP_OpenVDB_Sample_Points(
    OP_Network* net, const char* name, OP_Operator* op):
    hvdb::SOP_NodeVDB(net, name, op)
{
}


////////////////////////////////////////


namespace {

using StringSet = std::set<std::string>;
using StringVec = std::vector<std::string>;
using AttrNameMap = std::map<std::string /*gridName*/, StringSet /*attrNames*/>;
using PointGridPtrVec = std::vector<openvdb::points::PointDataGrid::Ptr>;


struct VDBPointsSampler
{
    VDBPointsSampler(PointGridPtrVec& points,
                     const StringVec& includeGroups,
                     const StringVec& excludeGroups,
                     AttrNameMap& existingAttrs)
        : mPointGrids(points)
        , mIncludeGroups(includeGroups)
        , mExcludeGroups(excludeGroups)
        , mExistingAttrs(existingAttrs) {}

    template <typename GridType>
    inline void
    pointSample(const hvdb::Grid& sourceGrid,
                const std::string& attributeName,
                hvdb::Interrupter* interrupter)
    {
        warnOnExisting(attributeName);
        const GridType& grid = UTvdbGridCast<GridType>(sourceGrid);
        for (auto& pointGrid : mPointGrids) {
            auto leaf = pointGrid->tree().cbeginLeaf();
            if (!leaf)  continue;
            cvdb::points::MultiGroupFilter filter(
                mIncludeGroups, mExcludeGroups, leaf->attributeSet());
            cvdb::points::pointSample(*pointGrid, grid, attributeName, filter, interrupter);
        }
    }

    template <typename GridType>
    inline void
    boxSample(const hvdb::Grid& sourceGrid,
              const std::string& attributeName,
              hvdb::Interrupter* interrupter)
    {
        warnOnExisting(attributeName);
        const GridType& grid = UTvdbGridCast<GridType>(sourceGrid);
        for (auto& pointGrid : mPointGrids) {
            auto leaf = pointGrid->tree().cbeginLeaf();
            if (!leaf) continue;
            cvdb::points::MultiGroupFilter filter(
                mIncludeGroups, mExcludeGroups, leaf->attributeSet());
            cvdb::points::boxSample(*pointGrid, grid, attributeName, filter, interrupter);
        }
    }

private:
    inline void
    warnOnExisting(const std::string& attributeName) const
    {
        for (const auto& pointGrid : mPointGrids) {
            assert(pointGrid);
            const auto leaf = pointGrid->tree().cbeginLeaf();
            if (!leaf) continue;
            if (leaf->hasAttribute(attributeName)) {
                mExistingAttrs[pointGrid->getName()].insert(attributeName);
            }
        }
    }

    const PointGridPtrVec& mPointGrids;
    const StringVec& mIncludeGroups;
    const StringVec& mExcludeGroups;
    AttrNameMap& mExistingAttrs;
};


template <bool staggered = false>
struct BoxSampler {
    template <class Accessor>
    static bool sample(const Accessor& in, const cvdb::Vec3R& inCoord,
        typename Accessor::ValueType& result)
    {
        return cvdb::tools::BoxSampler::sample<Accessor>(in, inCoord, result);
    }
};

template<>
struct BoxSampler<true> {
    template <class Accessor>
    static bool sample(const Accessor& in, const cvdb::Vec3R& inCoord,
        typename Accessor::ValueType& result)
    {
        return cvdb::tools::StaggeredBoxSampler::sample<Accessor>(in, inCoord, result);
    }
};


template <bool staggered = false>
struct NearestNeighborSampler {
    template <class Accessor>
    static bool sample(const Accessor& in, const cvdb::Vec3R& inCoord,
        typename Accessor::ValueType& result)
    {
        return cvdb::tools::PointSampler::sample<Accessor>(in, inCoord, result);
    }
};

template<>
struct NearestNeighborSampler<true> {
    template <class Accessor>
    static bool sample(const Accessor& in, const cvdb::Vec3R& inCoord,
        typename Accessor::ValueType& result)
    {
        return cvdb::tools::StaggeredPointSampler::sample<Accessor>(in, inCoord, result);
    }
};


template<
    typename GridType,
    typename GA_RWPageHandleType,
    bool staggered = false,
    bool NearestNeighbor = false>
class PointSampler
{
public:
    using Accessor = typename GridType::ConstAccessor;

    // constructor. from grid and GU_Detail*
    PointSampler(const hvdb::Grid& grid, const bool threaded,
                 GU_Detail* gdp, GA_RWAttributeRef& handle,
                 hvdb::Interrupter* interrupter):
        mGrid(grid),
        mThreaded(threaded),
        mGdp(gdp),
        mAttribPageHandle(handle.getAttribute()),
        mInterrupter(interrupter)
    {
    }

    // constructor.  from other
    PointSampler(const PointSampler<GridType, GA_RWPageHandleType, staggered>& other):
        mGrid(other.mGrid),
        mThreaded(other.mThreaded),
        mGdp(other.mGdp),
        mAttribPageHandle(other.mAttribPageHandle),
        mInterrupter(other.mInterrupter)
    {
    }

    void sample()
    {
        mInterrupter->start();
        if (mThreaded) {
            // multi-threaded
            UTparallelFor(GA_SplittableRange(mGdp->getPointRange()), *this);
        } else {
            // single-threaded
            (*this)(GA_SplittableRange(mGdp->getPointRange()));
        }
        mInterrupter->end();
    }

    // only the supported versions don't throw
    void operator() (const GA_SplittableRange& range) const
    {

        if (mInterrupter->wasInterrupted()) {
            tbb::task::self().cancel_group_execution();
        }
        const GridType& grid = UTvdbGridCast<GridType>(mGrid);
        // task local grid accessor
        Accessor accessor = grid.getAccessor();
        // sample scalar data onto points
        typename GridType::ValueType value;
        cvdb::Vec3R point;

        GA_ROPageHandleV3   p_ph(mGdp->getP());
        GA_RWPageHandleType v_ph = mAttribPageHandle;

        if(!v_ph.isValid()) {
            throw std::runtime_error("new attribute not valid");
        }

        // iterate over pages in the range
        for (GA_PageIterator pit = range.beginPages(); !pit.atEnd(); ++pit) {
            GA_Offset start;
            GA_Offset end;

            // per-page setup
            p_ph.setPage(*pit);
            v_ph.setPage(*pit);
            // iterate over elements in the page
            for (GA_Iterator it(pit.begin()); it.blockAdvance(start, end); ) {
                for (GA_Offset offset = start; offset < end; ++offset ) {
                    // get the pos.
                    UT_Vector3 pos = p_ph.get(offset);
                    // find the interpolated value
                    point = mGrid.worldToIndex(cvdb::Vec3R(pos[0], pos[1], pos[2]));

                    if (NearestNeighbor) {
                        NearestNeighborSampler<staggered>::template sample<Accessor>(
                            accessor, point, value);
                    } else {
                        BoxSampler<staggered>::template sample<Accessor>(accessor, point, value);
                    }
                    // set the value
                    v_ph.value(offset) = translateValue(value);
                }
            }
        }
    }
    template<typename T> inline static float translateValue(const T& vdb_value) {
        return static_cast<float>(vdb_value);
    }
    inline static UT_Vector3 translateValue(cvdb::Vec3f& vdb_value) {
        return UT_Vector3(vdb_value[0], vdb_value[1], vdb_value[2]);
    }
    inline static UT_Vector3 translateValue(cvdb::Vec3d& vdb_value) {
        return UT_Vector3(
            static_cast<float>(vdb_value[0]),
            static_cast<float>(vdb_value[1]),
            static_cast<float>(vdb_value[2]));
    }

private:
    // member data
    const hvdb::Grid&    mGrid;
    bool                 mThreaded;
    GU_Detail*           mGdp;
    GA_RWPageHandleType  mAttribPageHandle;
    hvdb::Interrupter*   mInterrupter;
}; // class PointSampler

} // anonymous namespace


////////////////////////////////////////


OP_ERROR
SOP_OpenVDB_Sample_Points::Cache::cookVDBSop(OP_Context& context)
{
    try {
        const fpreal time = context.getTime();

        GU_Detail* aGdp = gdp; // where the points live
        const GU_Detail* bGdp = inputGeo(1, context); // where the grids live

        // extract UI data
        const bool verbose = evalInt("verbose", 0, time) != 0;
        const bool threaded = true; /*evalInt("threaded", 0, time);*/

        // total number of points in vdb grids - this is used when verbose execution is
        // requested, but will otherwise remain 0
        size_t nVDBPoints = 0;
        StringVec includeGroups, excludeGroups;
        UT_String vdbPointsGroups;

        // obtain names of vdb point groups to include / exclude

        evalString(vdbPointsGroups, "vdbpointsgroups", 0, time);

        cvdb::points::AttributeSet::Descriptor::parseNames(includeGroups, excludeGroups,
            vdbPointsGroups.toStdString());

        // extract VDB points grids

        PointGridPtrVec pointGrids;

        for (openvdb_houdini::VdbPrimIterator it(gdp); it; ++it) {
            GU_PrimVDB* vdb = *it;
            if (!vdb || !vdb->getConstGridPtr()->isType<cvdb::points::PointDataGrid>()) continue;

            vdb->makeGridUnique();

            cvdb::GridBase::Ptr grid = vdb->getGridPtr();
            cvdb::points::PointDataGrid::Ptr pointDataGrid =
                cvdb::gridPtrCast<cvdb::points::PointDataGrid>(grid);

            if (verbose) {
                if (auto leaf = pointDataGrid->tree().cbeginLeaf()) {
                    cvdb::points::MultiGroupFilter filter(includeGroups, excludeGroups,
                        leaf->attributeSet());
                    nVDBPoints += cvdb::points::pointCount<cvdb::points::PointDataTree,
                        cvdb::points::MultiGroupFilter>(pointDataGrid->tree(), filter);
                }
            }

            pointGrids.emplace_back(pointDataGrid);
        }

        const GA_Size nPoints = aGdp->getNumPoints();

        // sanity checks - warn if there are no points on first input port.  Note that
        // each VDB primitive should have a single point associated with it so that we could
        // theoretically only check if nPoints == 0, but we explictly check for 0 pointGrids
        // for the sake of clarity
        if (nPoints == 0 && pointGrids.empty()) {
            const std::string msg = "Input 1 contains no points.";
            addWarning(SOP_MESSAGE, msg.c_str());
            if (verbose) std::cout << msg << std::endl;
        }

        // Get the group of grids to process
        const GA_PrimitiveGroup* group = matchGroup(*bGdp, evalStdString("group", time));

        // These lists are used to keep track of names of already-existing point attributes.
        StringSet existingPointAttrs;
        AttrNameMap existingVdbPointAttrs;

        VDBPointsSampler vdbPointsSampler(pointGrids, includeGroups, excludeGroups,
            existingVdbPointAttrs);

        // scratch variables used in the loop
        GA_Defaults defaultFloat(0.0), defaultInt(0);

        int numScalarGrids  = 0;
        int numVectorGrids  = 0;
        int numUnnamedGrids = 0;

        // start time
        tbb::tick_count time_start = tbb::tick_count::now();
        UT_AutoInterrupt progress("Sampling from VDB grids");

        for (hvdb::VdbPrimCIterator it(bGdp, group); it; ++it) {
            if (progress.wasInterrupted()) {
                throw std::runtime_error("was interrupted");
            }

            const GU_PrimVDB* vdb = *it;
            UT_VDBType gridType = vdb->getStorageType();
            const hvdb::Grid& grid = vdb->getGrid();

            std::string gridName = it.getPrimitiveName().toStdString();
            if (gridName.empty()) {
                std::stringstream ss;
                ss << "VDB_" << numUnnamedGrids++;
                gridName = ss.str();
            }

            // remove any dot "." characters, attribute names can't contain this.
            std::replace(gridName.begin(), gridName.end(), '.', '_');

            std::string attributeName;

            if (gridName == "vel" && evalInt("renamevel", 0, time)) {
                attributeName = "v";
            } else {
                attributeName = gridName;
            }

            //convert gridName to uppercase so we can use it as a local variable name
            std::string attributeVariableName = attributeName;
            std::transform(attributeVariableName.begin(), attributeVariableName.end(),
                attributeVariableName.begin(), ::toupper);

            if (gridType == UT_VDB_FLOAT || gridType == UT_VDB_DOUBLE) {
                // a grid that holds a scalar field (as either float or double type)
                // count
                numScalarGrids++;

                //find or create float attribute
                GA_RWAttributeRef attribHandle =
                    aGdp->findFloatTuple(GA_ATTRIB_POINT, attributeName.c_str(), 1);
                if (!attribHandle.isValid()) {
                    attribHandle = aGdp->addFloatTuple(
                        GA_ATTRIB_POINT, attributeName.c_str(), 1, defaultFloat);
                } else {
                    existingPointAttrs.insert(attributeName);
                }
                aGdp->addVariableName(attributeName.c_str(), attributeVariableName.c_str());

                // user feedback
                if (verbose) {
                    std::cout << "Sampling grid " << gridName << " of type "
                        << grid.valueType() << std::endl;
                }

                hvdb::Interrupter scalarInterrupt("Sampling from VDB floating-type grids");
                // do the sampling
                if (gridType == UT_VDB_FLOAT) {
                    // float scalar
                    PointSampler<cvdb::FloatGrid, GA_RWPageHandleF> theSampler(
                        grid, threaded, aGdp, attribHandle, &scalarInterrupt);
                    theSampler.sample();

                    vdbPointsSampler.boxSample<cvdb::FloatGrid>(
                        grid, attributeName, &scalarInterrupt);
                } else {
                    // double scalar
                    PointSampler<cvdb::DoubleGrid, GA_RWPageHandleF> theSampler(
                        grid, threaded, aGdp, attribHandle, &scalarInterrupt);
                    theSampler.sample();

                    vdbPointsSampler.boxSample<cvdb::DoubleGrid>(
                        grid, attributeName, &scalarInterrupt);
                }

            } else if (gridType == UT_VDB_INT32 || gridType == UT_VDB_INT64) {
                numScalarGrids++;

                //find or create integer attribute
                GA_RWAttributeRef attribHandle =
                    aGdp->findIntTuple(GA_ATTRIB_POINT, attributeName.c_str(), 1);
                if (!attribHandle.isValid()) {
                    attribHandle =
                        aGdp->addIntTuple(GA_ATTRIB_POINT, attributeName.c_str(), 1, defaultInt);
                } else {
                    existingPointAttrs.insert(attributeName);
                }
                aGdp->addVariableName(attributeName.c_str(), attributeVariableName.c_str());

                 // user feedback
                if (verbose) {
                    std::cout << "Sampling grid " << gridName << " of type "
                        << grid.valueType() << std::endl;
                }

                hvdb::Interrupter scalarInterrupt("Sampling from VDB integer-type grids");
                if (gridType == UT_VDB_INT32) {

                    PointSampler<cvdb::Int32Grid, GA_RWPageHandleF, false, true>
                        theSampler(grid, threaded, aGdp, attribHandle, &scalarInterrupt);
                    theSampler.sample();

                    vdbPointsSampler.pointSample<cvdb::Int32Grid>(
                        grid, attributeName, &scalarInterrupt);

                } else {
                    PointSampler<cvdb::Int64Grid, GA_RWPageHandleF, false, true>
                        theSampler(grid, threaded, aGdp, attribHandle, &scalarInterrupt);
                    theSampler.sample();

                    vdbPointsSampler.pointSample<cvdb::Int64Grid>(
                        grid, attributeName, &scalarInterrupt);
                }

            } else if (gridType == UT_VDB_VEC3F || gridType == UT_VDB_VEC3D) {
                // a grid that holds Vec3 data (as either float or double)
                // count
                numVectorGrids++;

                // find or create create vector attribute
                GA_RWAttributeRef attribHandle =
                    aGdp->findFloatTuple(GA_ATTRIB_POINT, attributeName.c_str(), 3);
                if (!attribHandle.isValid()) {
                    attribHandle = aGdp->addFloatTuple(
                        GA_ATTRIB_POINT, attributeName.c_str(), 3, defaultFloat);
                } else {
                    existingPointAttrs.insert(attributeName);
                }
                aGdp->addVariableName(attributeName.c_str(), attributeVariableName.c_str());

                std::unique_ptr<hvdb::Interrupter> interrupter;

                // user feedback
                if (grid.getGridClass() != cvdb::GRID_STAGGERED) {
                    // regular (non-staggered) vec3 grid
                    if (verbose) {
                        std::cout << "Sampling grid " << gridName << " of type "
                            << grid.valueType() << std::endl;
                    }

                    interrupter.reset(new hvdb::Interrupter("Sampling from VDB vector-type grids"));

                    // do the sampling
                    if (gridType == UT_VDB_VEC3F) {
                        // Vec3f
                        PointSampler<cvdb::Vec3fGrid, GA_RWPageHandleV3> theSampler(
                            grid, threaded, aGdp, attribHandle, interrupter.get());
                        theSampler.sample();
                    } else {
                        // Vec3d
                        PointSampler<cvdb::Vec3dGrid, GA_RWPageHandleV3> theSampler(
                            grid, threaded, aGdp, attribHandle, interrupter.get());
                        theSampler.sample();
                    }
                } else {
                    // staggered grid case
                    if (verbose) {
                        std::cout << "Sampling staggered grid " << gridName << " of type "
                            << grid.valueType() << std::endl;
                    }

                    interrupter.reset(new hvdb::Interrupter(
                        "Sampling from VDB vector-type staggered grids"));

                    // do the sampling
                    if (grid.isType<cvdb::Vec3fGrid>()) {
                        // Vec3f
                        PointSampler<cvdb::Vec3fGrid, GA_RWPageHandleV3, true> theSampler(
                            grid, threaded, aGdp, attribHandle, interrupter.get());
                        theSampler.sample();
                    } else {
                        // Vec3d
                        PointSampler<cvdb::Vec3dGrid, GA_RWPageHandleV3, true> theSampler(
                            grid, threaded, aGdp, attribHandle, interrupter.get());
                        theSampler.sample();
                    }
                }

                // staggered vector sampling is handled within the core library for vdb points

                if (gridType == UT_VDB_VEC3F) {
                    vdbPointsSampler.boxSample<cvdb::Vec3fGrid>(
                        grid, attributeName, interrupter.get());
                } else {
                    vdbPointsSampler.boxSample<cvdb::Vec3dGrid>(
                        grid, attributeName, interrupter.get());
                }
            } else {
                addWarning(SOP_MESSAGE, ("Skipped VDB \"" + gridName
                    + "\" of unsupported type " + grid.valueType()).c_str());
            }
        }//end iter

        if (0 != evalInt("attributeexists", 0, time)) {
            // Report existing Houdini point attributes.
            existingPointAttrs.erase("");
            if (existingPointAttrs.size() == 1) {
                addWarning(SOP_MESSAGE, ("Point attribute \"" + *existingPointAttrs.begin()
                    + "\" already exists").c_str());
            } else if (!existingPointAttrs.empty()) {
                const StringVec attrNames(existingPointAttrs.begin(), existingPointAttrs.end());
                const std::string s = "These point attributes already exist: " +
                    hboost::algorithm::join(attrNames, ", ");
                addWarning(SOP_MESSAGE, s.c_str());
            }

            // Report existing VDB Points attributes and the grids in which they appear.
            for (auto& attrs: existingVdbPointAttrs) {
                auto& attrSet = attrs.second;
                attrSet.erase("");
                if (attrSet.size() == 1) {
                    addWarning(SOP_MESSAGE, ("Attribute \"" + *attrSet.begin()
                        + "\" already exists in VDB point data grid \""
                        + attrs.first + "\".").c_str());
                } else if (!attrSet.empty()) {
                    const StringVec attrNames(attrSet.begin(), attrSet.end());
                    const std::string s = "These attributes already exist in VDB point data grid \""
                        + attrs.first + "\": " + hboost::algorithm::join(attrNames, ", ");
                    addWarning(SOP_MESSAGE, s.c_str());
                }
            }
        }

        // timing: end time
        tbb::tick_count time_end = tbb::tick_count::now();

        if (verbose) {
            std::cout << "Sampling " << nPoints + nVDBPoints << " points in "
                      << numVectorGrids << " vector grid" << (numVectorGrids == 1 ? "" : "s")
                      << " and " << numScalarGrids << " scalar grid"
                          << (numScalarGrids == 1 ? "" : "s")
                      << " took " << (time_end - time_start).seconds() << " seconds\n "
                      << (threaded ? "threaded" : "non-threaded") << std::endl;
        }

    } catch (std::exception& e) {
        addError(SOP_MESSAGE, e.what());
    }

    return error();
}
