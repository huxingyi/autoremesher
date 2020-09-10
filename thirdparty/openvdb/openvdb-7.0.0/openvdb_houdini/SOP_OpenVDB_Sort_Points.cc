// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file SOP_OpenVDB_Sort_Points.cc
///
/// @author Mihai Alden

#include <houdini_utils/ParmFactory.h>

#include <openvdb_houdini/SOP_NodeVDB.h>
#include <openvdb_houdini/GU_VDBPointTools.h>

#include <openvdb/tools/PointPartitioner.h>

#include <GA/GA_AttributeFilter.h>
#include <GA/GA_ElementWrangler.h>
#include <GA/GA_PageIterator.h>
#include <GA/GA_SplittableRange.h>
#include <GU/GU_Detail.h>
#include <PRM/PRM_Parm.h>
#include <UT/UT_Version.h>
#include <UT/UT_UniquePtr.h>

#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>

#include <memory>
#include <stdexcept>


namespace hvdb = openvdb_houdini;
namespace hutil = houdini_utils;


////////////////////////////////////////

// Local Utility Methods

namespace {

struct CopyElements {

    CopyElements(GA_PointWrangler& wrangler, const GA_Offset* offsetArray)
        : mWrangler(&wrangler), mOffsetArray(offsetArray) { }

    void operator()(const GA_SplittableRange& range) const {
        GA_Offset start, end;
        for (GA_PageIterator pageIt = range.beginPages(); !pageIt.atEnd(); ++pageIt) {
            for (GA_Iterator blockIt(pageIt.begin()); blockIt.blockAdvance(start, end); ) {
                for (GA_Offset i = start; i < end; ++i) {
                    mWrangler->copyAttributeValues(i, mOffsetArray[i]);
                }
            }
        }
    }

    GA_PointWrangler          * const mWrangler;
    GA_Offset           const * const mOffsetArray;
}; // struct CopyElements


struct SetOffsets
{
    using PointPartitioner = openvdb::tools::UInt32PointPartitioner;

    SetOffsets(const GU_Detail& srcGeo, const PointPartitioner& partitioner, GA_Offset* offsetArray)
        : mSrcGeo(&srcGeo), mPartitioner(&partitioner), mOffsetArray(offsetArray) { }

    void operator()(const tbb::blocked_range<size_t>& range) const {

        size_t idx = 0;
        for (size_t n = 0, N = range.begin(); n != N; ++n) {
            idx += mPartitioner->indices(n).size(); // increment to start index
        }

        for (size_t n = range.begin(), N = range.end(); n != N; ++n) {
            for (PointPartitioner::IndexIterator it = mPartitioner->indices(n); it; ++it) {
                mOffsetArray[idx++] = mSrcGeo->pointOffset(*it);
            }
        }
    }

    GU_Detail           const * const mSrcGeo;
    PointPartitioner    const * const mPartitioner;
    GA_Offset                 * const mOffsetArray;
}; // struct SetOffsets

} // unnamed namespace


////////////////////////////////////////

// SOP Implementation

struct SOP_OpenVDB_Sort_Points: public hvdb::SOP_NodeVDB
{
    SOP_OpenVDB_Sort_Points(OP_Network*, const char* name, OP_Operator*);
    static OP_Node* factory(OP_Network*, const char* name, OP_Operator*);

    class Cache: public SOP_VDBCacheOptions { OP_ERROR cookVDBSop(OP_Context&) override; };
};


void
newSopOperator(OP_OperatorTable* table)
{
    if (table == nullptr) return;

    hutil::ParmList parms;

    parms.add(hutil::ParmFactory(PRM_STRING, "pointgroup", "Point Group")
        .setChoiceList(&SOP_Node::pointGroupMenu)
        .setTooltip("A group of points to rasterize."));

    parms.add(hutil::ParmFactory(PRM_FLT_J, "binsize", "Bin Size")
        .setDefault(PRMpointOneDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_UI, 5)
        .setTooltip("The size (length of a side) of the cubic bin, in world units."));

    hvdb::OpenVDBOpFactory("VDB Sort Points",
        SOP_OpenVDB_Sort_Points::factory, parms, *table)
        .setNativeName("")
        .addInput("points")
        .setVerb(SOP_NodeVerb::COOK_GENERATOR, []() { return new SOP_OpenVDB_Sort_Points::Cache; })
        .setDocumentation("\
#icon: COMMON/openvdb\n\
#tags: vdb\n\
\n\
\"\"\"Reorder points into spatially-organized bins.\"\"\"\n\
\n\
@overview\n\
\n\
This node reorders Houdini points so that they are sorted into\n\
three-dimensional spatial bins.\n\
By increasing CPU cache locality of point data, sorting can improve the\n\
performance of algorithms such as rasterization that rely on neighbor access.\n\
\n\
@examples\n\
\n\
See [openvdb.org|http://www.openvdb.org/download/] for source code\n\
and usage examples.\n");
}


OP_Node*
SOP_OpenVDB_Sort_Points::factory(OP_Network* net, const char* name, OP_Operator* op)
{
    return new SOP_OpenVDB_Sort_Points(net, name, op);
}


SOP_OpenVDB_Sort_Points::SOP_OpenVDB_Sort_Points(OP_Network* net, const char* name, OP_Operator* op)
    : hvdb::SOP_NodeVDB(net, name, op)
{
}


OP_ERROR
SOP_OpenVDB_Sort_Points::Cache::cookVDBSop(OP_Context& context)
{
    try {
        const fpreal time = context.getTime();
        const GU_Detail* srcGeo = inputGeo(0);

        UT_UniquePtr<GA_Offset[]> srcOffsetArray;
        size_t numPoints = 0;

        { // partition points and construct ordered offset list
            const GA_PointGroup* pointGroup = parsePointGroups(
                evalStdString("pointgroup", time).c_str(), GroupCreator(srcGeo));

            const fpreal voxelSize = evalFloat("binsize", 0, time);
            const openvdb::math::Transform::Ptr transform =
                openvdb::math::Transform::createLinearTransform(voxelSize);

            GU_VDBPointList<openvdb::Vec3s> points(*srcGeo, pointGroup);

            openvdb::tools::UInt32PointPartitioner partitioner;
            partitioner.construct(points, *transform, /*voxel order=*/true);

            numPoints = points.size();
            srcOffsetArray.reset(new GA_Offset[numPoints]);

            tbb::parallel_for(tbb::blocked_range<size_t>(0, partitioner.size()),
                SetOffsets(*srcGeo, partitioner, srcOffsetArray.get()));
        }

        // order point attributes

        gdp->appendPointBlock(numPoints);

        gdp->cloneMissingAttributes(*srcGeo, GA_ATTRIB_POINT, GA_AttributeFilter::selectPublic());

        GA_PointWrangler ptWrangler(*gdp, *srcGeo,  GA_PointWrangler::INCLUDE_P);

        UTparallelFor(GA_SplittableRange(gdp->getPointRange()),
            CopyElements(ptWrangler, srcOffsetArray.get()));

    } catch (std::exception& e) {
        addError(SOP_MESSAGE, e.what());
    }

    return error();
}
