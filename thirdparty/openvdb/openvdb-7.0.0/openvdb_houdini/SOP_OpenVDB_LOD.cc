// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file SOP_OpenVDB_LOD.cc
///
/// @author FX R&D OpenVDB team
///
/// @brief Generate one or more levels of a volume mipmap.

#include <houdini_utils/ParmFactory.h>
#include <openvdb_houdini/SOP_NodeVDB.h>
#include <openvdb_houdini/Utils.h>
#include <openvdb/tools/MultiResGrid.h>

#include <hboost/algorithm/string/join.hpp>

#include <string>
#include <vector>


namespace hvdb = openvdb_houdini;
namespace hutil = houdini_utils;


class SOP_OpenVDB_LOD: public hvdb::SOP_NodeVDB
{
public:
    SOP_OpenVDB_LOD(OP_Network*, const char* name, OP_Operator*);
    ~SOP_OpenVDB_LOD() override {}

    static OP_Node* factory(OP_Network*, const char* name, OP_Operator*);

    int isRefInput(unsigned input) const override { return (input > 0); }

    class Cache: public SOP_VDBCacheOptions { OP_ERROR cookVDBSop(OP_Context&) override; };

protected:
    bool updateParmsFlags() override;
};


////////////////////////////////////////


void
newSopOperator(OP_OperatorTable* table)
{
    if (table == nullptr) return;

    hutil::ParmList parms;

    parms.add(hutil::ParmFactory(PRM_STRING, "group", "Group")
        .setChoiceList(&hutil::PrimGroupMenuInput1)
        .setTooltip("Specify a subset of the input VDB grids to be processed.")
        .setDocumentation(
            "A subset of the input VDB grids to be processed"
            " (see [specifying volumes|/model/volumes#group])"));

    parms.add(hutil::ParmFactory(PRM_ORD, "lod", "LOD Mode")
        .setDefault(PRMzeroDefaults)
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, {
            "single", "Single Level",
            "range",  "Level Range",
            "mipmaps","LOD Pyramid"
        })
        .setDocumentation(
            "How to build the LOD pyramid\n\n"
            "Single Level:\n"
            "    Build a single, filtered VDB.\n\n"
            "Level Range:\n"
            "    Build a series of VDBs of progressively lower resolution\n"
            "    within a given range of scales.\n\n"
            "LOD Pyramid:\n"
            "    Build a standard pyramid of VDBs of decreasing resolution.\n"
            "    Each level of the pyramid is half the resolution in each\n"
            "    dimension of the previous level, starting with the input VDB.\n"));

    parms.add(hutil::ParmFactory(PRM_FLT_J, "level", "Level")
        .setDefault(PRMoneDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 0.0, PRM_RANGE_UI, 10.0)
        .setTooltip("Specify which single level to produce.\n"
            "Level 0, the highest-resolution level, is the input VDB."));

    {
        const std::vector<fpreal> defaultRange{
            fpreal(0.0), // start
            fpreal(2.0), // end
            fpreal(1.0)  // step
        };

        parms.add(hutil::ParmFactory(PRM_FLT_J, "range", "Range")
            .setDefault(defaultRange)
            .setVectorSize(3)
            .setRange(PRM_RANGE_RESTRICTED, 0.0, PRM_RANGE_UI, 10.0)
            .setTooltip(
                "In Level Range mode, specify the (inclusive) starting and ending levels"
                " and the level step. Level 0, the highest-resolution level, is the input VDB;"
                " fractional levels are allowed."));
    }

    parms.add(hutil::ParmFactory(PRM_INT_J, "count", "Count")
        .setDefault(PRMtwoDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 2, PRM_RANGE_UI, 10)
        .setTooltip(
            "In LOD Pyramid mode, specify the number of pyramid levels to generate."
            " Each level is half the resolution of the previous level."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "reuse", "Preserve Grid Names")
        .setDefault(PRMzeroDefaults)
        .setTooltip(
            "In Single Level mode, give the output VDB the same name as the input VDB."));

    hvdb::OpenVDBOpFactory("VDB LOD", SOP_OpenVDB_LOD::factory, parms, *table)
        .addInput("VDBs")
        .setVerb(SOP_NodeVerb::COOK_INPLACE, []() { return new SOP_OpenVDB_LOD::Cache; })
        .setDocumentation("\
#icon: COMMON/openvdb\n\
#tags: vdb\n\
\n\
\"\"\"Build an LOD pyramid from a VDB volume.\"\"\"\n\
\n\
@overview\n\
\n\
This node creates filtered versions of a VDB volume at multiple resolutions,\n\
providing mipmap-like levels of detail.\n\
The low-resolution versions can be used both as thumbnails for fast processing\n\
and for constant-time, filtered lookups over large areas of a volume.\n\
\n\
@related\n\
- [OpenVDB Resample|Node:sop/DW_OpenVDBResample]\n\
\n\
@examples\n\
\n\
See [openvdb.org|http://www.openvdb.org/download/] for source code\n\
and usage examples.\n");
}


bool
SOP_OpenVDB_LOD::updateParmsFlags()
{
    bool changed = false;

    const auto lodMode = evalInt("lod", 0, 0);

    changed |= enableParm("level", lodMode == 0);
    changed |= enableParm("reuse", lodMode == 0);
    changed |= enableParm("range", lodMode == 1);
    changed |= enableParm("count", lodMode == 2);

    return changed;
}


////////////////////////////////////////


OP_Node*
SOP_OpenVDB_LOD::factory(OP_Network* net,
    const char* name, OP_Operator* op)
{
    return new SOP_OpenVDB_LOD(net, name, op);
}


SOP_OpenVDB_LOD::SOP_OpenVDB_LOD(OP_Network* net,
    const char* name, OP_Operator* op):
    hvdb::SOP_NodeVDB(net, name, op)
{
}


////////////////////////////////////////


namespace {

template<openvdb::Index Order>
struct MultiResGridFractionalOp
{
    MultiResGridFractionalOp(float f) : level(f) {}
    template<typename GridType>
    void operator()(const GridType& grid)
    {
        if ( level <= 0.0f ) {
            outputGrid = typename GridType::Ptr( new GridType(grid) );
        } else {
            const size_t levels = openvdb::math::Ceil(level) + 1;
            using TreeT = typename GridType::TreeType;
            openvdb::tools::MultiResGrid<TreeT> mrg( levels, grid );
            outputGrid = mrg.template createGrid<Order>( level );
        }
    }
    const float level;
    hvdb::GridPtr outputGrid;
};

template<openvdb::Index Order>
struct MultiResGridRangeOp
{
    MultiResGridRangeOp(float start_, float end_, float step_, hvdb::Interrupter& boss_)
        : start(start_), end(end_), step(step_), outputGrids(), boss(&boss_)
    {}

    template<typename GridType>
    void operator()(const GridType& grid)
    {
        if ( end > 0.0f ) {
            const size_t levels = openvdb::math::Ceil(end) + 1;
            using TreeT = typename GridType::TreeType;
            openvdb::tools::MultiResGrid<TreeT> mrg( levels, grid );

            // inclusive range
            for (float level = start; !(level > end); level += step) {

                if (boss->wasInterrupted()) break;

                outputGrids.push_back( mrg.template createGrid<Order>( level ) );
            }
        }
    }
    const float start, end, step;
    std::vector<hvdb::GridPtr> outputGrids;
    hvdb::Interrupter * const boss;
};

struct MultiResGridIntegerOp
{
    MultiResGridIntegerOp(size_t n) : levels(n) {}
    template<typename GridType>
    void operator()(const GridType& grid)
    {
        using TreeT = typename GridType::TreeType;
        openvdb::tools::MultiResGrid<TreeT> mrg( levels, grid );
        outputGrids = mrg.grids();
    }
    const size_t levels;
    openvdb::GridPtrVecPtr outputGrids;
};


inline bool
isValidRange(float start, float end, float step)
{
    if (start < 0.0f || !(step > 0.0f) || end < 0.0f) {
        return false;
    }

    return !(start > end);
}

}//unnamed namespace


////////////////////////////////////////


OP_ERROR
SOP_OpenVDB_LOD::Cache::cookVDBSop(OP_Context& context)
{
    try {
        const fpreal time = context.getTime();

        // Get the group of grids to process.
        const GA_PrimitiveGroup* group = matchGroup(*gdp, evalStdString("group", time));

        std::vector<std::string> skipped;

        hvdb::Interrupter boss("Creating VDB LoD pyramid");
        GA_RWHandleS name_h(gdp, GA_ATTRIB_PRIMITIVE, "name");

        const auto lodMode = evalInt("lod", 0, 0);
        if (lodMode == 0) {

            const bool reuseName = evalInt("reuse", 0, 0) > 0;
            MultiResGridFractionalOp<1> op( float(evalFloat("level", 0, time)) );
            for (hvdb::VdbPrimIterator it(gdp, group); it; ++it) {

                if (boss.wasInterrupted()) return error();

                if (name_h.isValid())
                    it->getGrid().setName(static_cast<const char *>(name_h.get(it->getMapOffset())));

                if (!it->getGrid().transform().isLinear()) {
                    skipped.push_back(it->getGrid().getName());
                    continue;
                }

                GEOvdbProcessTypedGridTopology(**it, op);

                if (boss.wasInterrupted()) return error();

                if (reuseName) op.outputGrid->setName( it->getGrid().getName() );

                hvdb::createVdbPrimitive(*gdp, op.outputGrid);

                gdp->destroyPrimitiveOffset(it->getMapOffset(), /*and_points=*/true);
            }

        } else if (lodMode == 1) {

            const float start = float(evalFloat("range", 0, time));
            const float end = float(evalFloat("range", 1, time));
            const float step = float(evalFloat("range", 2, time));

            if (!isValidRange(start, end, step)) {
                addError(SOP_MESSAGE, "Invalid range, make sure that "
                    "start <= end and the step size is a positive number.");
                return error();
            }

            MultiResGridRangeOp<1> op( start, end, step, boss );
            for (hvdb::VdbPrimIterator it(gdp, group); it; ++it) {

                if (boss.wasInterrupted()) return error();

                if (name_h.isValid())
                    it->getGrid().setName(static_cast<const char *>(name_h.get(it->getMapOffset())));

                if (!it->getGrid().transform().isLinear()) {
                    skipped.push_back(it->getGrid().getName());
                    continue;
                }

                GEOvdbProcessTypedGridTopology(**it, op);

                if (boss.wasInterrupted()) return error();

                for (size_t i=0; i< op.outputGrids.size(); ++i) {
                    hvdb::createVdbPrimitive(*gdp, op.outputGrids[i]);
                }

                gdp->destroyPrimitiveOffset(it->getMapOffset(), /*and_points=*/true);
            }
        } else if (lodMode == 2) {

            MultiResGridIntegerOp op( evalInt("count", 0, time) );
            for (hvdb::VdbPrimIterator it(gdp, group); it; ++it) {

                if (boss.wasInterrupted()) return error();

                if (name_h.isValid())
                    it->getGrid().setName(static_cast<const char *>(name_h.get(it->getMapOffset())));

                if (!it->getGrid().transform().isLinear()) {
                    skipped.push_back(it->getGrid().getName());
                    continue;
                }

                GEOvdbProcessTypedGridTopology(**it, op);

                if (boss.wasInterrupted()) return error();

                for (size_t i=0; i< op.outputGrids->size(); ++i) {
                    hvdb::createVdbPrimitive(*gdp, op.outputGrids->at(i));
                }

                gdp->destroyPrimitiveOffset(it->getMapOffset(), /*and_points=*/true);
            }

        } else {
            addError(SOP_MESSAGE, "Invalid LOD option.");
        }

        if (!skipped.empty()) {
            addWarning(SOP_MESSAGE, ("Unable to process grid(s): " +
                hboost::algorithm::join(skipped, ", ")).c_str());
        }

    } catch (std::exception& e) {
        addError(SOP_MESSAGE, e.what());
    }

    return error();
}
