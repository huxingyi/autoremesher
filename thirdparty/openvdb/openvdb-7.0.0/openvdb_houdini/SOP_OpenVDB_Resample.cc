// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
///
/// @file SOP_OpenVDB_Resample.cc
//
/// @author FX R&D OpenVDB team
///
/// @class SOP_OpenVDB_Resample
/// This node resamples voxels from input VDB grids into new grids
/// (of the same type) through a sampling transform that is either
/// specified by user-supplied translation, rotation, scale and pivot
/// parameters or taken from an optional reference grid.

#include <houdini_utils/ParmFactory.h>
#include <openvdb_houdini/Utils.h>
#include <openvdb_houdini/UT_VDBTools.h> // for GridTransformOp, et al.
#include <openvdb_houdini/UT_VDBUtils.h> // for UTvdbProcessTypedGridReal()
#include <openvdb_houdini/SOP_NodeVDB.h>
#include <openvdb/openvdb.h>
#include <openvdb/tools/GridTransformer.h>
#include <openvdb/tools/LevelSetRebuild.h>
#include <openvdb/tools/VectorTransformer.h> // for transformVectors()
#include <UT/UT_Interrupt.h>
#include <UT/UT_Version.h>
#include <functional>
#include <stdexcept>
#include <string>



namespace hvdb = openvdb_houdini;
namespace hutil = houdini_utils;

namespace {
enum { MODE_PARMS = 0, MODE_REF_GRID, MODE_VOXEL_SIZE, MODE_VOXEL_SCALE };
}


class SOP_OpenVDB_Resample: public hvdb::SOP_NodeVDB
{
public:
    SOP_OpenVDB_Resample(OP_Network*, const char* name, OP_Operator*);
    ~SOP_OpenVDB_Resample() override {}

    static OP_Node* factory(OP_Network*, const char* name, OP_Operator*);

    int isRefInput(unsigned i) const override { return (i == 1); }

    class Cache: public SOP_VDBCacheOptions { OP_ERROR cookVDBSop(OP_Context&) override; };

protected:
    void resolveObsoleteParms(PRM_ParmList*) override;
    bool updateParmsFlags() override;
};


////////////////////////////////////////


// Build UI and register this operator.
void
newSopOperator(OP_OperatorTable* table)
{
    if (table == nullptr) return;

    hutil::ParmList parms;

    // Group pattern
    parms.add(hutil::ParmFactory(PRM_STRING, "group", "Group")
        .setChoiceList(&hutil::PrimGroupMenuInput1)
        .setTooltip("Specify a subset of the input VDBs to be resampled")
        .setDocumentation(
            "A subset of the input VDBs to be resampled"
            " (see [specifying volumes|/model/volumes#group])"));

    // Reference grid group
    parms.add(hutil::ParmFactory(PRM_STRING, "reference", "Reference")
        .setChoiceList(&hutil::PrimGroupMenuInput2)
        .setTooltip(
            "Specify a single reference VDB from the\n"
            "first input whose transform is to be matched.\n"
            "Alternatively, connect the reference VDB\n"
            "to the second input."));

    parms.add(hutil::ParmFactory(PRM_ORD, "order", "Interpolation")
        .setDefault(PRMoneDefaults)
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, {
            "point",     "Nearest",
            "linear",    "Linear",
            "quadratic", "Quadratic"
        })
        .setDocumentation("\
How to interpolate values at fractional voxel positions\n\
\n\
Nearest:\n\
    Use the value from the nearest voxel.\n\n\
    This is fast but can cause aliasing artifacts.\n\
Linear:\n\
    Interpolate trilinearly between the values of immediate neighbors.\n\n\
    This matches what [Node:sop/volumemix] and [Vex:volumesample] do.\n\
Quadratic:\n\
    Interpolate triquadratically between the values of neighbors.\n\n\
    This produces smoother results than trilinear interpolation but is slower.\n"));

    // Transform source
    parms.add(hutil::ParmFactory(PRM_ORD, "mode", "Define Transform")
        .setDefault(PRMoneDefaults)
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, {
            "explicit",       "Explicitly",
            "refvdb",         "To Match Reference VDB",
            "voxelsizeonly",  "Using Voxel Size Only",
            "voxelscaleonly", "Using Voxel Scale Only"
        })
        .setTooltip(
            "Specify how to define the relative transform\n"
            "between an input and an output VDB.")
        .setDocumentation("\
How to generate the new VDB's transform\n\
\n\
Explicitly:\n\
    Use the values of the transform parameters below.\n\
To Match Reference VDB:\n\
    Match the transform and voxel size of a reference VDB.\n\n\
    The resulting volume is a copy of the input VDB,\n\
    aligned to the reference VDB.\n\
Using Voxel Size Only:\n\
    Keep the transform of the input VDB but set a new voxel size,\n\
    increasing or decreasing the resolution.\n\
Using Voxel Scale Only:\n\
    Keep the transform of the input VDB but scale the voxel size,\n\
    increasing or decreasing the resolution.\n"));

    parms.add(hutil::ParmFactory(PRM_ORD, "xOrd", "Transform Order")
        .setDefault(0, "tsr")
        .setChoiceList(&PRMtrsMenu)
        .setTypeExtended(PRM_TYPE_JOIN_PAIR)
        .setTooltip(
            "When __Define Transform__ is Explicitly, the order of operations"
            " for the new transform"));

    parms.add(hutil::ParmFactory(
        PRM_ORD | PRM_Type(PRM_Type::PRM_INTERFACE_LABEL_NONE), "rOrd", "")
        .setDefault(0, "zyx")
        .setChoiceList(&PRMxyzMenu)
        .setTooltip(
            "When __Define Transform__ is Explicitly, the order of rotations"
            " for the new transform"));

    // Translation
    parms.add(hutil::ParmFactory(PRM_XYZ_J, "t", "Translate Voxels")
        .setDefault(PRMzeroDefaults)
        .setVectorSize(3)
        .setTooltip(
            "When __Define Transform__ is Explicitly, the shift in voxels for the new transform"));

    // Rotation
    parms.add(hutil::ParmFactory(PRM_XYZ_J, "r", "Rotate")
        .setDefault(PRMzeroDefaults)
        .setVectorSize(3)
        .setTooltip(
            "When __Define Transform__ is Explicitly, the rotation for the new transform"));

    // Scale
    parms.add(hutil::ParmFactory(PRM_XYZ_J, "s", "Scale")
        .setDefault(PRMoneDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 0.000001f, PRM_RANGE_UI, 10)
        .setVectorSize(3)
        .setTooltip(
            "When __Define Transform__ is Explicitly, the scale for the new transform"));

    // Pivot
    parms.add(hutil::ParmFactory(PRM_XYZ_J, "p", "Pivot")
        .setDefault(PRMzeroDefaults)
        .setVectorSize(3)
        .setTooltip(
            "When __Define Transform__ is Explicitly, the world-space pivot point"
            " for scaling and rotation in the new transform"));

    // Voxel size
    parms.add(hutil::ParmFactory(PRM_FLT_J, "voxelsize", "Voxel Size")
        .setDefault(PRMoneDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 0.000001f, PRM_RANGE_UI, 1)
        .setTooltip(
            "The desired absolute voxel size for all output VDBs\n\n"
            "Larger voxels correspond to lower resolution.\n"));

    // Voxel scale
    parms.add(hutil::ParmFactory(PRM_FLT_J, "voxelscale", "Voxel Scale")
        .setDefault(PRMoneDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 0.000001f, PRM_RANGE_UI, 1)
        .setTooltip(
            "The amount by which to scale the voxel size for each output VDB\n\n"
            "Larger voxels correspond to lower resolution.\n"));

    // Toggle to apply transform to vector values
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "xformvectors", "Transform Vectors")
        .setDefault(PRMzeroDefaults)
        .setTooltip(
            "Apply the resampling transform to the voxel values of vector-valued VDBs,"
            " in accordance with those VDBs'"
            " [Vector Type|http://www.openvdb.org/documentation/doxygen/overview.html#secGrid]"
            " attributes."));

    // Level set rebuild toggle
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "rebuild", "Rebuild SDF")
        .setDefault(PRMoneDefaults)
        .setTooltip(
            "Transforming (especially scaling) a level set might invalidate\n"
            "signed distances, necessitating reconstruction of the SDF.\n\n"
            "This option affects only level set volumes, and it should\n"
            "almost always be enabled."));

    // Prune toggle
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "prune", "Prune Tolerance")
        .setDefault(PRMoneDefaults)
        .setTypeExtended(PRM_TYPE_TOGGLE_JOIN)
        .setTooltip(
            "Reduce the memory footprint of output VDBs that have"
            " (sufficiently large) regions of voxels with the same value.\n\n"
            "Voxel values are considered equal if they differ by less than"
            " the specified threshold.\n\n"
            "NOTE:\n"
            "    Pruning affects only the memory usage of a grid.\n"
            "    It does not remove voxels, apart from inactive voxels\n"
            "    whose value is equal to the background."));

    // Pruning tolerance slider
    parms.add(hutil::ParmFactory(
        PRM_FLT_J, "tolerance", "Prune Tolerance")
        .setDefault(PRMzeroDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_UI, 1)
        .setDocumentation(nullptr));

    // Obsolete parameters
    hutil::ParmList obsoleteParms;
    obsoleteParms.add(hutil::ParmFactory(PRM_SEPARATOR, "sep2", "separator"));
    obsoleteParms.add(hutil::ParmFactory(PRM_SEPARATOR, "sep3", "separator"));
    obsoleteParms.add(hutil::ParmFactory(PRM_SEPARATOR, "sep4", "separator"));
    obsoleteParms.add(hutil::ParmFactory(PRM_STRING, "reference_grid", "Reference"));
    obsoleteParms.add(hutil::ParmFactory(PRM_XYZ_J, "translate", "Translate")
        .setDefault(PRMzeroDefaults)
        .setVectorSize(3));
    obsoleteParms.add(hutil::ParmFactory(PRM_XYZ_J, "rotate", "Rotate")
        .setDefault(PRMzeroDefaults)
        .setVectorSize(3));
    obsoleteParms.add(hutil::ParmFactory(PRM_XYZ_J, "scale", "Scale")
        .setDefault(PRMoneDefaults)
        .setVectorSize(3));
    obsoleteParms.add(hutil::ParmFactory(PRM_XYZ_J, "pivot", "Pivot")
        .setDefault(PRMzeroDefaults)
        .setVectorSize(3));
    obsoleteParms.add(hutil::ParmFactory(PRM_FLT_J, "voxel_size", "Voxel Size")
        .setDefault(PRMoneDefaults));

    // Register this operator.
    hvdb::OpenVDBOpFactory("VDB Resample", SOP_OpenVDB_Resample::factory, parms, *table)
        .setObsoleteParms(obsoleteParms)
        .addInput("Source VDB grids to resample")
        .addOptionalInput("Optional transform reference VDB grid")
        .setVerb(SOP_NodeVerb::COOK_INPLACE, []() { return new SOP_OpenVDB_Resample::Cache; })
        .setDocumentation("\
#icon: COMMON/openvdb\n\
#tags: vdb\n\
\n\
\"\"\"Resample a VDB volume into a new orientation and/or voxel size.\"\"\"\n\
\n\
@overview\n\
\n\
This node resamples voxels from input VDBs into new VDBs (of the same type)\n\
through a sampling transform that is either specified by user-supplied\n\
translation, rotation, scale and pivot parameters or taken from\n\
an optional reference VDB.\n\
\n\
@related\n\
- [OpenVDB Combine|Node:sop/DW_OpenVDBCombine]\n\
- [Node:sop/vdbresample]\n\
- [Node:sop/xform]\n\
\n\
@examples\n\
\n\
See [openvdb.org|http://www.openvdb.org/download/] for source code\n\
and usage examples.\n");
}


void
SOP_OpenVDB_Resample::resolveObsoleteParms(PRM_ParmList* obsoleteParms)
{
    if (!obsoleteParms) return;

    resolveRenamedParm(*obsoleteParms, "reference_grid", "reference");
    resolveRenamedParm(*obsoleteParms, "voxel_size", "voxelsize");
    resolveRenamedParm(*obsoleteParms, "translate", "t");
    resolveRenamedParm(*obsoleteParms, "rotate", "r");
    resolveRenamedParm(*obsoleteParms, "scale", "s");
    resolveRenamedParm(*obsoleteParms, "pivot", "p");

    // Delegate to the base class.
    hvdb::SOP_NodeVDB::resolveObsoleteParms(obsoleteParms);
}


// Disable UI Parms.
bool
SOP_OpenVDB_Resample::updateParmsFlags()
{
    bool changed = false;

    const auto mode = evalInt("mode", 0, 0);
    changed |= enableParm("t", mode == MODE_PARMS);
    changed |= enableParm("r", mode == MODE_PARMS);
    changed |= enableParm("s", mode == MODE_PARMS);
    changed |= enableParm("p", mode == MODE_PARMS);
    changed |= enableParm("xOrd", mode == MODE_PARMS);
    changed |= enableParm("rOrd", mode == MODE_PARMS);
    changed |= enableParm("xformvectors", mode == MODE_PARMS);
    changed |= enableParm("reference", mode == MODE_REF_GRID);
    changed |= enableParm("voxelsize", mode == MODE_VOXEL_SIZE);
    changed |= enableParm("voxelscale", mode == MODE_VOXEL_SCALE);
    // Show either the voxel size or the voxel scale parm, but not both.
    changed |= setVisibleState("voxelsize", mode != MODE_VOXEL_SCALE);
    changed |= setVisibleState("voxelscale", mode == MODE_VOXEL_SCALE);

    changed |= enableParm("tolerance", bool(evalInt("prune", 0, 0)));

    return changed;
}


////////////////////////////////////////


OP_Node*
SOP_OpenVDB_Resample::factory(OP_Network* net, const char* name, OP_Operator* op)
{
    return new SOP_OpenVDB_Resample(net, name, op);
}


SOP_OpenVDB_Resample::SOP_OpenVDB_Resample(OP_Network* net, const char* name, OP_Operator* op):
    hvdb::SOP_NodeVDB(net, name, op)
{
}


////////////////////////////////////////


namespace {

// Helper class for use with UTvdbProcessTypedGrid()
struct RebuildOp
{
    std::function<void (const std::string&)> addWarning;
    openvdb::math::Transform xform;
    hvdb::GridPtr outGrid;

    template<typename GridT>
    void operator()(const GridT& grid)
    {
        using ValueT = typename GridT::ValueType;

        const ValueT halfWidth = ValueT(grid.background() * (1.0 / grid.voxelSize()[0]));

        hvdb::Interrupter interrupter;
        try {
            outGrid = openvdb::tools::doLevelSetRebuild(grid,
                /*isovalue=*/openvdb::zeroVal<ValueT>(),
                /*exWidth=*/halfWidth, /*inWidth=*/halfWidth, &xform, &interrupter);
        } catch (openvdb::TypeError&) {
            addWarning("skipped rebuild of level set grid " + grid.getName()
                + " of type " + grid.type());
            outGrid = openvdb::ConstPtrCast<GridT>(grid.copy());
        }
    }
}; // struct RebuildOp


// Functor for use with UTvdbProcessTypedGridVec3() to apply a transform
// to the voxel values of vector-valued grids
struct VecXformOp
{
    openvdb::Mat4d mat;
    VecXformOp(const openvdb::Mat4d& _mat): mat(_mat) {}
    template<typename GridT> void operator()(GridT& grid) const
    {
        openvdb::tools::transformVectors(grid, mat);
    }
};

} // unnamed namespace


////////////////////////////////////////


OP_ERROR
SOP_OpenVDB_Resample::Cache::cookVDBSop(OP_Context& context)
{
    try {
        auto addWarningCB = [this](const std::string& s) { addWarning(SOP_MESSAGE, s.c_str()); };

        const fpreal time = context.getTime();

        // Get parameters.
        const int samplingOrder = static_cast<int>(evalInt("order", 0, time));
        if (samplingOrder < 0 || samplingOrder > 2) {
            throw std::runtime_error{"expected interpolation order between 0 and 2, got "
                + std::to_string(samplingOrder)};
        }

        char const* const xOrdMenu[] = { "srt", "str", "rst", "rts", "tsr", "trs" };
        char const* const rOrdMenu[] = { "xyz", "xzy", "yxz", "yzx", "zxy", "zyx" };
        const UT_String
            xformOrder = xOrdMenu[evalInt("xOrd", 0, time)],
            rotOrder = rOrdMenu[evalInt("rOrd", 0, time)];

        const int mode = static_cast<int>(evalInt("mode", 0, time));
        if (mode < MODE_PARMS || mode > MODE_VOXEL_SCALE) {
            throw std::runtime_error{"expected mode between " + std::to_string(int(MODE_PARMS))
                + " and " + std::to_string(int(MODE_VOXEL_SCALE))
                + ", got " + std::to_string(mode)};
        }

        const openvdb::Vec3R
            translate = evalVec3R("t", time),
            rotate = (M_PI / 180.0) * evalVec3R("r", time),
            scale = evalVec3R("s", time),
            pivot = evalVec3R("p", time);
        const float
            voxelSize = static_cast<float>(evalFloat("voxelsize", 0, time)),
            voxelScale = static_cast<float>(evalFloat("voxelscale", 0, time));

        const bool
            prune = evalInt("prune", 0, time),
            rebuild = evalInt("rebuild", 0, time),
            xformVec = evalInt("xformvectors", 0, time);
        const float tolerance = static_cast<float>(evalFloat("tolerance", 0, time));

        // Get the group of grids to be resampled.
        const GA_PrimitiveGroup* group = matchGroup(*gdp, evalStdString("group", time));

        hvdb::GridCPtr refGrid;
        if (mode == MODE_VOXEL_SIZE) {
            // Create a dummy reference grid whose (linear) transform specifies
            // the desired voxel size.
            hvdb::GridPtr grid = openvdb::FloatGrid::create();
            grid->setTransform(openvdb::math::Transform::createLinearTransform(voxelSize));
            refGrid = grid;
        } else if (mode == MODE_VOXEL_SCALE) {
            // Create a dummy reference grid with a default (linear) transform.
            refGrid = openvdb::FloatGrid::create();
        } else if (mode == MODE_REF_GRID) {
            // Get the user-specified reference grid from the second input,
            // if it is connected, or else from the first input.
            const GU_Detail* refGdp = inputGeo(1, context);
            if (!refGdp) { refGdp = gdp; }
            if (auto it = hvdb::VdbPrimCIterator(refGdp,
                matchGroup(*refGdp, evalStdString("reference", time))))
            {
                refGrid = it->getConstGridPtr();
                if (++it) { addWarning(SOP_MESSAGE, "more than one reference grid was found"); }
            } else {
                throw std::runtime_error("no reference grid was found");
            }
        }

        UT_AutoInterrupt progress("Resampling VDB grids");

        // Iterate over the input grids.
        for (hvdb::VdbPrimIterator it(gdp, GA_Range::safedeletions(), group); it; ++it) {
            if (progress.wasInterrupted()) throw std::runtime_error("Was Interrupted");

            GU_PrimVDB* vdb = *it;

            const UT_VDBType valueType = vdb->getStorageType();

            const bool isLevelSet = ((vdb->getGrid().getGridClass() == openvdb::GRID_LEVEL_SET)
                && (valueType == UT_VDB_FLOAT || valueType == UT_VDB_DOUBLE));

            if (isLevelSet && !rebuild) {
                // If the input grid is a level set but level set rebuild is disabled,
                // set the grid's class to "unknown", to prevent the resample tool
                // from triggering a rebuild.
                vdb->getGrid().setGridClass(openvdb::GRID_UNKNOWN);
            }

            const hvdb::Grid& grid = vdb->getGrid();

            // Override the sampling order for boolean grids.
            int curOrder = samplingOrder;
            if (valueType == UT_VDB_BOOL && (samplingOrder != 0)) {
                addWarning(SOP_MESSAGE,
                    ("a boolean VDB grid can't be order-" + std::to_string(samplingOrder)
                    + " sampled; using nearest neighbor sampling instead").c_str());
                curOrder = 0;
            }

            // Create a new, empty output grid of the same type as the input grid
            // and with the same metadata.
            hvdb::GridPtr outGrid = grid.copyGridWithNewTree();

            UT_AutoInterrupt scopedInterrupt(
                ("Resampling " + it.getPrimitiveName().toStdString()).c_str());

            if (refGrid) {
                // If a reference grid was provided, then after resampling, the
                // output grid's transform will be the same as the reference grid's.

                openvdb::math::Transform::Ptr refXform = refGrid->transform().copy();
                if (mode == MODE_VOXEL_SCALE) {
                    openvdb::Vec3d scaledVoxelSize = grid.voxelSize() * voxelScale;
                    refXform->preScale(scaledVoxelSize);
                }

                if (isLevelSet && rebuild) {
                    // Use the level set rebuild tool to both resample and rebuild.
                    RebuildOp op;
                    op.addWarning = addWarningCB;
                    op.xform = *refXform;
                    UTvdbProcessTypedGridReal(valueType, grid, op);
                    outGrid = op.outGrid;

                } else {
                    // Use the resample tool to sample the input grid into the output grid.

                    // Set the correct transform on the output grid.
                    outGrid->setTransform(refXform);

                    if (curOrder == 0) {
                        hvdb::GridResampleToMatchOp<openvdb::tools::PointSampler> op(outGrid);
                        GEOvdbProcessTypedGridTopology(*vdb, op);
                    } else if (curOrder == 1) {
                        hvdb::GridResampleToMatchOp<openvdb::tools::BoxSampler> op(outGrid);
                        GEOvdbProcessTypedGridTopology(*vdb, op);
                    } else if (curOrder == 2) {
                        hvdb::GridResampleToMatchOp<openvdb::tools::QuadraticSampler> op(outGrid);
                        GEOvdbProcessTypedGridTopology(*vdb, op);
                    }

#ifdef SESI_OPENVDB
                    if (isLevelSet) {
                        auto tempgrid = UTvdbGridCast<openvdb::FloatGrid>(outGrid);
                        openvdb::tools::pruneLevelSet(tempgrid->tree());
                        openvdb::tools::signedFloodFill(tempgrid->tree());
                    }
#endif
                }

            } else {
                // Resample into the output grid using the user-supplied transform.
                // The output grid's transform will be the same as the input grid's.

                openvdb::tools::GridTransformer xform(pivot, scale, rotate, translate,
                    xformOrder.toStdString(), rotOrder.toStdString());

                if (isLevelSet && rebuild) {
                    // Use the level set rebuild tool to both resample and rebuild.
                    RebuildOp op;
                    op.addWarning = addWarningCB;

                    // Compose the input grid's transform with the user-supplied transform.
                    // (The latter is retrieved from the GridTransformer, so that the
                    // order of operations and the rotation order are respected.)
                    op.xform = grid.constTransform();
                    op.xform.preMult(xform.getTransform().inverse());

                    UTvdbProcessTypedGridReal(valueType, grid, op);
                    outGrid = op.outGrid;
                    outGrid->setTransform(grid.constTransform().copy());

                } else {
                    // Use the resample tool to sample the input grid into the output grid.

                    hvdb::Interrupter interrupter;
                    xform.setInterrupter(interrupter);

                    if (curOrder == 0) {
                        hvdb::GridTransformOp<openvdb::tools::PointSampler> op(outGrid, xform);
                        GEOvdbProcessTypedGridTopology(*vdb, op);
                    } else if (curOrder == 1) {
                        hvdb::GridTransformOp<openvdb::tools::BoxSampler> op(outGrid, xform);
                        GEOvdbProcessTypedGridTopology(*vdb, op);
                    } else if (curOrder == 2) {
                        hvdb::GridTransformOp<openvdb::tools::QuadraticSampler> op(outGrid, xform);
                        GEOvdbProcessTypedGridTopology(*vdb, op);
                    }

#ifdef SESI_OPENVDB
                    if (isLevelSet) {
                        auto tempgrid = UTvdbGridCast<openvdb::FloatGrid>(outGrid);
                        openvdb::tools::pruneLevelSet(tempgrid->tree());
                        openvdb::tools::signedFloodFill(tempgrid->tree());
                    }
#endif
                }

                if (xformVec && outGrid->isInWorldSpace()
                    && outGrid->getVectorType() != openvdb::VEC_INVARIANT)
                {
                    // If (and only if) the grid is vector-valued, apply the transform
                    // to each voxel's value.
                    VecXformOp op(xform.getTransform());
                    UTvdbProcessTypedGridVec3(valueType, *outGrid, op);
                }
            }

            if (prune) outGrid->pruneGrid(tolerance);

            // Replace the original VDB primitive with a new primitive that contains
            // the output grid and has the same attributes and group membership.
            hvdb::replaceVdbPrimitive(*gdp, outGrid, *vdb);
        }
    } catch (std::exception& e) {
        addError(SOP_MESSAGE, e.what());
    }
    return error();
}
