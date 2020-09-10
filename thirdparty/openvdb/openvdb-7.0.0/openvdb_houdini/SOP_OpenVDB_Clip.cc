// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

/// @file SOP_OpenVDB_Clip.cc
///
/// @author FX R&D OpenVDB team
///
/// @brief Clip grids

#include <houdini_utils/ParmFactory.h>
#include <openvdb_houdini/GeometryUtil.h> // for drawFrustum(), frustumTransformFromCamera()
#include <openvdb_houdini/Utils.h>
#include <openvdb_houdini/SOP_NodeVDB.h>
#include <openvdb/tools/Clip.h> // for tools::clip()
#include <openvdb/tools/LevelSetUtil.h> // for tools::sdfInteriorMask()
#include <openvdb/tools/Mask.h> // for tools::interiorMask()
#include <openvdb/tools/Morphology.h> // for tools::dilateActiveValues(), tools::erodeVoxels()
#include <openvdb/points/PointDataGrid.h>
#include <OBJ/OBJ_Camera.h>
#include <cmath> // for std::abs(), std::round()
#include <exception>
#include <string>



namespace hvdb = openvdb_houdini;
namespace hutil = houdini_utils;


class SOP_OpenVDB_Clip: public hvdb::SOP_NodeVDB
{
public:
    SOP_OpenVDB_Clip(OP_Network*, const char* name, OP_Operator*);
    ~SOP_OpenVDB_Clip() override {}

    static OP_Node* factory(OP_Network*, const char* name, OP_Operator*);

    int isRefInput(unsigned input) const override { return (input == 1); }

    class Cache: public SOP_VDBCacheOptions
    {
    public:
        openvdb::math::Transform::Ptr frustum() const { return mFrustum; }
    protected:
        OP_ERROR cookVDBSop(OP_Context&) override;
    private:
        void getFrustum(OP_Context&);

        openvdb::math::Transform::Ptr mFrustum;
    }; // class Cache

protected:
    void resolveObsoleteParms(PRM_ParmList*) override;
    bool updateParmsFlags() override;
    OP_ERROR cookMyGuide1(OP_Context&) override;
};


////////////////////////////////////////


void
newSopOperator(OP_OperatorTable* table)
{
    if (table == nullptr) return;

    hutil::ParmList parms;

    parms.add(hutil::ParmFactory(PRM_STRING, "group", "Group")
        .setChoiceList(&hutil::PrimGroupMenuInput1)
        .setTooltip("Specify a subset of VDBs from the first input to be clipped.")
        .setDocumentation(
            "A subset of VDBs from the first input to be clipped"
            " (see [specifying volumes|/model/volumes#group])"));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "inside", "Keep Inside")
        .setDefault(PRMoneDefaults)
        .setTooltip(
            "If enabled, keep voxels that lie inside the clipping region.\n"
            "If disabled, keep voxels that lie outside the clipping region.")
        .setDocumentation(
            "If enabled, keep voxels that lie inside the clipping region,"
            " otherwise keep voxels that lie outside the clipping region."));

    parms.add(hutil::ParmFactory(PRM_STRING, "clipper", "Clip To")
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, {
            "camera",   "Camera",
            "geometry", "Geometry",
            "mask",     "Mask VDB"
        })
        .setDefault("geometry")
        .setTooltip("Specify how the clipping region should be defined.")
        .setDocumentation("\
How to define the clipping region\n\
\n\
Camera:\n\
    Use a camera frustum as the clipping region.\n\
Geometry:\n\
    Use the bounding box of geometry from the second input as the clipping region.\n\
Mask VDB:\n\
    Use the active voxels of a VDB volume from the second input as a clipping mask.\n"));

    parms.add(hutil::ParmFactory(PRM_STRING, "mask", "Mask VDB")
        .setChoiceList(&hutil::PrimGroupMenuInput2)
        .setTooltip("Specify a VDB whose active voxels are to be used as a clipping mask.")
        .setDocumentation(
            "A VDB from the second input whose active voxels are to be used as a clipping mask"
            " (see [specifying volumes|/model/volumes#group])"));

    parms.add(hutil::ParmFactory(PRM_STRING, "camera", "Camera")
        .setTypeExtended(PRM_TYPE_DYNAMIC_PATH)
        .setSpareData(&PRM_SpareData::objCameraPath)
        .setTooltip("Specify the path to a reference camera")
        .setDocumentation(
            "The path to the camera whose frustum is to be used as a clipping region"
            " (e.g., `/obj/cam1`)"));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "setnear", "")
        .setDefault(PRMzeroDefaults)
        .setTypeExtended(PRM_TYPE_TOGGLE_JOIN)
        .setTooltip("If enabled, override the camera's near clipping plane."));

    parms.add(hutil::ParmFactory(PRM_FLT_E, "near", "Near Clipping")
        .setDefault(0.001)
        .setTooltip("The position of the near clipping plane")
        .setDocumentation(
            "The position of the near clipping plane\n\n"
            "If enabled, this setting overrides the camera's clipping plane."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "setfar", "")
        .setDefault(PRMzeroDefaults)
        .setTypeExtended(PRM_TYPE_TOGGLE_JOIN)
        .setTooltip("If enabled, override the camera's far clipping plane."));

    parms.add(hutil::ParmFactory(PRM_FLT_E, "far", "Far Clipping")
        .setDefault(10000)
        .setTooltip("The position of the far clipping plane")
        .setDocumentation(
            "The position of the far clipping plane\n\n"
            "If enabled, this setting overrides the camera's clipping plane."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "setpadding", "")
        .setDefault(PRMzeroDefaults)
        .setTypeExtended(PRM_TYPE_TOGGLE_JOIN)
        .setTooltip("If enabled, expand or shrink the clipping region."));

    parms.add(hutil::ParmFactory(PRM_FLT_E, "padding", "Padding")
        .setVectorSize(3)
        .setDefault(PRMzeroDefaults)
        .setTooltip("Padding in world units to be added to the clipping region")
        .setDocumentation(
            "Padding in world units to be added to the clipping region\n\n"
            "Negative values shrink the clipping region.\n\n"
            "Nonuniform padding is not supported when clipping to a VDB volume.\n"
            "The mask volume will be dilated or eroded uniformly"
            " by the _x_-axis padding value."));


    // Obsolete parameters
    hutil::ParmList obsoleteParms;
    obsoleteParms.add(hutil::ParmFactory(PRM_TOGGLE, "usemask", "").setDefault(PRMzeroDefaults));


    hvdb::OpenVDBOpFactory("VDB Clip", SOP_OpenVDB_Clip::factory, parms, *table)
        .addInput("VDBs")
        .addOptionalInput("Mask VDB or bounding geometry")
        .setObsoleteParms(obsoleteParms)
        .setVerb(SOP_NodeVerb::COOK_INPLACE, []() { return new SOP_OpenVDB_Clip::Cache; })
        .setDocumentation("\
#icon: COMMON/openvdb\n\
#tags: vdb\n\
\n\
\"\"\"Clip VDB volumes using a camera frustum, a bounding box, or another VDB as a mask.\"\"\"\n\
\n\
@overview\n\
\n\
This node clips VDB volumes, that is, it removes voxels that lie outside\n\
(or, optionally, inside) a given region by deactivating them and setting them\n\
to the background value.\n\
The clipping region may be one of the following:\n\
* the frustum of a camera\n\
* the bounding box of reference geometry\n\
* the active voxels of another VDB.\n\
\n\
When the clipping region is defined by a VDB, the operation\n\
is similar to [activity intersection|Node:sop/DW_OpenVDBCombine],\n\
except that clipped voxels are not only deactivated but also set\n\
to the background value.\n\
\n\
@related\n\
\n\
- [OpenVDB Combine|Node:sop/DW_OpenVDBCombine]\n\
- [OpenVDB Occlusion Mask|Node:sop/DW_OpenVDBOcclusionMask]\n\
- [Node:sop/vdbactivate]\n\
\n\
@examples\n\
\n\
See [openvdb.org|http://www.openvdb.org/download/] for source code\n\
and usage examples.\n");
}


void
SOP_OpenVDB_Clip::resolveObsoleteParms(PRM_ParmList* obsoleteParms)
{
    if (!obsoleteParms) return;

    auto* parm = obsoleteParms->getParmPtr("usemask");
    if (parm && !parm->isFactoryDefault()) { // factory default was Off
        setString("clipper", CH_STRING_LITERAL, "mask", 0, 0.0);
    }

    // Delegate to the base class.
    hvdb::SOP_NodeVDB::resolveObsoleteParms(obsoleteParms);
}


bool
SOP_OpenVDB_Clip::updateParmsFlags()
{
    bool changed = false;

    UT_String clipper;
    evalString(clipper, "clipper", 0, 0.0);

    const bool clipToCamera = (clipper == "camera");

    changed |= enableParm("mask", clipper == "mask");
    changed |= enableParm("camera", clipToCamera);
    changed |= enableParm("setnear", clipToCamera);
    changed |= enableParm("near", clipToCamera && evalInt("setnear", 0, 0.0));
    changed |= enableParm("setfar", clipToCamera);
    changed |= enableParm("far", clipToCamera && evalInt("setfar", 0, 0.0));
    changed |= enableParm("padding", 0 != evalInt("setpadding", 0, 0.0));

    changed |= setVisibleState("mask", clipper == "mask");
    changed |= setVisibleState("camera", clipToCamera);
    changed |= setVisibleState("setnear", clipToCamera);
    changed |= setVisibleState("near", clipToCamera);
    changed |= setVisibleState("setfar", clipToCamera);
    changed |= setVisibleState("far", clipToCamera);

    return changed;
}


////////////////////////////////////////


OP_Node*
SOP_OpenVDB_Clip::factory(OP_Network* net,
    const char* name, OP_Operator* op)
{
    return new SOP_OpenVDB_Clip(net, name, op);
}


SOP_OpenVDB_Clip::SOP_OpenVDB_Clip(OP_Network* net,
    const char* name, OP_Operator* op):
    hvdb::SOP_NodeVDB(net, name, op)
{
}


////////////////////////////////////////


namespace {

// Functor to convert a mask grid of arbitrary type to a BoolGrid
// and to dilate or erode it
struct DilatedMaskOp
{
    DilatedMaskOp(int dilation_): dilation{dilation_} {}

    template<typename GridType>
    void operator()(const GridType& grid)
    {
        if (dilation == 0) return;

        maskGrid = openvdb::BoolGrid::create();
        maskGrid->setTransform(grid.transform().copy());
        maskGrid->topologyUnion(grid);

        if (dilation < 0) {
            // Densify the mask, since tools::erodeVoxels() ignores active tiles.
            /// @todo Remove this once tools::erodeActiveValues() is implemented.
            maskGrid->tree().voxelizeActiveTiles();
        }

        UT_AutoInterrupt progress{
            ((dilation > 0 ? "Dilating" : "Eroding") + std::string{" VDB mask"}).c_str()};

        int numIterations = std::abs(dilation);

        const int kNumIterationsPerPass = 4;
        const int numPasses = numIterations / kNumIterationsPerPass;

        auto morphologyOp = [&](int iterations) {
            if (dilation > 0) {
                openvdb::tools::dilateActiveValues(maskGrid->tree(), iterations);
            } else {
                /// @todo Replace with tools::erodeActiveValues() once it is implemented.
                openvdb::tools::erodeVoxels(maskGrid->tree(), iterations);
            }
        };

        // Since large dilations and erosions can be expensive, apply them
        // in multiple passes and check for interrupts.
        for (int pass = 0; pass < numPasses; ++pass, numIterations -= kNumIterationsPerPass) {
            const bool interrupt = progress.wasInterrupted(
                /*pct=*/int((100.0 * pass * kNumIterationsPerPass) / std::abs(dilation)));
            if (interrupt) {
                maskGrid.reset();
                throw std::runtime_error{"interrupted"};
            }
            morphologyOp(kNumIterationsPerPass);
        }
        if (numIterations > 0) {
            morphologyOp(numIterations);
        }
    }

    int dilation = 0; // positive = dilation, negative = erosion
    openvdb::BoolGrid::Ptr maskGrid;
};


struct LevelSetMaskOp
{
    template<typename GridType>
    void operator()(const GridType& grid)
    {
        outputGrid = openvdb::tools::sdfInteriorMask(grid);
    }

    hvdb::GridPtr outputGrid;
};


struct BBoxClipOp
{
    BBoxClipOp(const openvdb::BBoxd& bbox_, bool inside_ = true):
        bbox(bbox_), inside(inside_)
    {}

    template<typename GridType>
    void operator()(const GridType& grid)
    {
        outputGrid = openvdb::tools::clip(grid, bbox, inside);
    }

    openvdb::BBoxd bbox;
    hvdb::GridPtr outputGrid;
    bool inside = true;
};


struct FrustumClipOp
{
    FrustumClipOp(const openvdb::math::Transform::Ptr& frustum_, bool inside_ = true):
        frustum(frustum_), inside(inside_)
    {}

    template<typename GridType>
    void operator()(const GridType& grid)
    {
        openvdb::math::NonlinearFrustumMap::ConstPtr mapPtr;
        if (frustum) mapPtr = frustum->constMap<openvdb::math::NonlinearFrustumMap>();
        if (mapPtr) {
            outputGrid = openvdb::tools::clip(grid, *mapPtr, inside);
        }
    }

    const openvdb::math::Transform::ConstPtr frustum;
    const bool inside = true;
    hvdb::GridPtr outputGrid;
};


template<typename GridType>
struct MaskClipDispatchOp
{
    MaskClipDispatchOp(const GridType& grid_, bool inside_ = true):
        grid(&grid_), inside(inside_)
    {}

    template<typename MaskGridType>
    void operator()(const MaskGridType& mask)
    {
        outputGrid.reset();
        if (grid) outputGrid = openvdb::tools::clip(*grid, mask, inside);
    }

    const GridType* grid;
    hvdb::GridPtr outputGrid;
    bool inside = true;
};


struct MaskClipOp
{
    MaskClipOp(hvdb::GridCPtr mask_, bool inside_ = true):
        mask(mask_), inside(inside_)
    {}

    template<typename GridType>
    void operator()(const GridType& grid)
    {
        outputGrid.reset();
        if (mask) {
            // Dispatch on the mask grid type, now that the source grid type is resolved.
            MaskClipDispatchOp<GridType> op(grid, inside);
            UTvdbProcessTypedGridTopology(UTvdbGetGridType(*mask), *mask, op);
            outputGrid = op.outputGrid;
        }
    }

    hvdb::GridCPtr mask;
    hvdb::GridPtr outputGrid;
    bool inside = true;
};

} // unnamed namespace


////////////////////////////////////////


/// Get the selected camera's frustum transform.
void
SOP_OpenVDB_Clip::Cache::getFrustum(OP_Context& context)
{
    mFrustum.reset();

    const auto time = context.getTime();

    UT_String cameraPath;
    evalString(cameraPath, "camera", 0, time);
    if (!cameraPath.isstring()) {
        throw std::runtime_error{"no camera path was specified"};
    }

    OBJ_Camera* camera = nullptr;
    if (auto* obj = cookparms()->getCwd()->findOBJNode(cameraPath)) {
        camera = obj->castToOBJCamera();
    }
    OP_Node* self = cookparms()->getCwd();

    if (!camera) {
        throw std::runtime_error{"camera \"" + cameraPath.toStdString() + "\" was not found"};
    }
    self->addExtraInput(camera, OP_INTEREST_DATA);

    OBJ_CameraParms cameraParms;
    camera->getCameraParms(cameraParms, time);
    if (cameraParms.projection != OBJ_PROJ_PERSPECTIVE) {
        throw std::runtime_error{cameraPath.toStdString() + " is not a perspective camera"};
        /// @todo support ortho and other cameras?
    }

    const bool pad = (0 != evalInt("setpadding", 0, time));
    const auto padding = pad ? evalVec3f("padding", time) : openvdb::Vec3f{0};

    const float nearPlane = (evalInt("setnear", 0, time)
        ? static_cast<float>(evalFloat("near", 0, time))
        : static_cast<float>(camera->getNEAR(time))) - padding[2];
    const float farPlane = (evalInt("setfar", 0, time)
        ? static_cast<float>(evalFloat("far", 0, time))
        : static_cast<float>(camera->getFAR(time))) + padding[2];

    mFrustum = hvdb::frustumTransformFromCamera(*self, context, *camera,
        /*offset=*/0.f, nearPlane, farPlane, /*voxelDepth=*/1.f, /*voxelCountX=*/100);

    if (!mFrustum || !mFrustum->constMap<openvdb::math::NonlinearFrustumMap>()) {
        throw std::runtime_error{
            "failed to compute frustum bounds for camera " + cameraPath.toStdString()};
    }

    if (pad) {
        const auto extents =
            mFrustum->constMap<openvdb::math::NonlinearFrustumMap>()->getBBox().extents();
        mFrustum->preScale(openvdb::Vec3d{
            (extents[0] + 2 * padding[0]) / extents[0],
            (extents[1] + 2 * padding[1]) / extents[1],
            1.0});
    }
}


////////////////////////////////////////


OP_ERROR
SOP_OpenVDB_Clip::cookMyGuide1(OP_Context&)
{
    myGuide1->clearAndDestroy();

    openvdb::math::Transform::ConstPtr frustum;
    // Attempt to extract the frustum from our cache.
    if (auto* cache = dynamic_cast<SOP_OpenVDB_Clip::Cache*>(myNodeVerbCache)) {
        frustum = cache->frustum();
    }

    if (frustum) {
        const UT_Vector3 color{0.9f, 0.0f, 0.0f};
        hvdb::drawFrustum(*myGuide1, *frustum, &color,
            /*tickColor=*/nullptr, /*shaded=*/false, /*ticks=*/false);
    }
    return error();
}


OP_ERROR
SOP_OpenVDB_Clip::Cache::cookVDBSop(OP_Context& context)
{
    try {
        const fpreal time = context.getTime();

        UT_AutoInterrupt progress{"Clipping VDBs"};

        const GU_Detail* maskGeo = inputGeo(1);

        UT_String clipper;
        evalString(clipper, "clipper", 0, time);

        const bool
            useCamera = (clipper == "camera"),
            useMask = (clipper == "mask"),
            inside = evalInt("inside", 0, time),
            pad = evalInt("setpadding", 0, time);

        const auto padding = pad ? evalVec3f("padding", time) : openvdb::Vec3f{0};

        mFrustum.reset();

        openvdb::BBoxd clipBox;
        hvdb::GridCPtr maskGrid;

        if (useCamera) {
            getFrustum(context);
        } else if (maskGeo) {
            if (useMask) {
                const GA_PrimitiveGroup* maskGroup = parsePrimitiveGroups(
                    evalStdString("mask", time).c_str(), GroupCreator{maskGeo});
                hvdb::VdbPrimCIterator maskIt{maskGeo, maskGroup};
                if (maskIt) {
                    if (maskIt->getConstGrid().getGridClass() == openvdb::GRID_LEVEL_SET) {
                        // If the mask grid is a level set, extract an interior mask from it.
                        LevelSetMaskOp op;
                        GEOvdbProcessTypedGridScalar(**maskIt, op);
                        maskGrid = op.outputGrid;
                    } else {
                        maskGrid = maskIt->getConstGridPtr();
                    }
                }
                if (!maskGrid) {
                    addError(SOP_MESSAGE, "mask VDB not found");
                    return error();
                }
                if (pad) {
                    // If padding is enabled and nonzero, dilate or erode the mask grid.
                    const auto paddingInVoxels = padding / maskGrid->voxelSize();
                    if (!openvdb::math::isApproxEqual(paddingInVoxels[0], paddingInVoxels[1])
                        || !openvdb::math::isApproxEqual(paddingInVoxels[1], paddingInVoxels[2]))
                    {
                        addWarning(SOP_MESSAGE,
                            "nonuniform padding is not supported for mask clipping");
                    }
                    if (const int dilation = int(std::round(paddingInVoxels[0]))) {
                        const auto maskType = UTvdbGetGridType(*maskGrid);
                        DilatedMaskOp op{dilation};
                        if (!UTvdbProcessTypedGridTopology(maskType, *maskGrid, op)) {
                            UTvdbProcessTypedGridPoint(maskType, *maskGrid, op);
                        }
                        if (op.maskGrid) maskGrid = op.maskGrid;
                    }
                }
            } else {
                UT_BoundingBox box;
                maskGeo->computeQuickBounds(box);

                clipBox.min()[0] = box.xmin();
                clipBox.min()[1] = box.ymin();
                clipBox.min()[2] = box.zmin();
                clipBox.max()[0] = box.xmax();
                clipBox.max()[1] = box.ymax();
                clipBox.max()[2] = box.zmax();
                if (pad) {
                    clipBox.min() -= padding;
                    clipBox.max() += padding;
                }
            }
        } else {
            addError(SOP_MESSAGE, "Not enough sources specified.");
            return error();
        }

        // Get the group of grids to process.
        const GA_PrimitiveGroup* group = matchGroup(*gdp, evalStdString("group", time));

        int numLevelSets = 0;
        for (hvdb::VdbPrimIterator it{gdp, group}; it; ++it) {
            if (progress.wasInterrupted()) { throw std::runtime_error{"interrupted"}; }

            const auto& inGrid = it->getConstGrid();

            hvdb::GridPtr outGrid;

            if (inGrid.getGridClass() == openvdb::GRID_LEVEL_SET) {
                ++numLevelSets;
            }

            progress.getInterrupt()->setAppTitle(
                ("Clipping VDB " + it.getPrimitiveIndexAndName().toStdString()).c_str());

            if (maskGrid) {
                MaskClipOp op{maskGrid, inside};
                if (GEOvdbProcessTypedGridTopology(**it, op)) { // all Houdini-supported grid types
                    outGrid = op.outputGrid;
                } else if (inGrid.isType<openvdb::points::PointDataGrid>()) {
                    addWarning(SOP_MESSAGE,
                        "only bounding box clipping is currently supported for point data grids");
                }
            } else if (useCamera) {
                FrustumClipOp op{mFrustum, inside};
                if (GEOvdbProcessTypedGridTopology(**it, op)) { // all Houdini-supported grid types
                    outGrid = op.outputGrid;
                } else if (inGrid.isType<openvdb::points::PointDataGrid>()) {
                    addWarning(SOP_MESSAGE,
                        "only bounding box clipping is currently supported for point data grids");
                }
            } else {
                BBoxClipOp op{clipBox, inside};
                if (GEOvdbProcessTypedGridTopology(**it, op)) { // all Houdini-supported grid types
                    outGrid = op.outputGrid;
                } else if (inGrid.isType<openvdb::points::PointDataGrid>()) {
                    if (inside) {
                        outGrid = inGrid.deepCopyGrid();
                        outGrid->clipGrid(clipBox);
                    } else {
                        addWarning(SOP_MESSAGE,
                            "only Keep Inside mode is currently supported for point data grids");
                    }
                }
            }

            // Replace the original VDB primitive with a new primitive that contains
            // the output grid and has the same attributes and group membership.
            hvdb::replaceVdbPrimitive(*gdp, outGrid, **it, true);
        }

        if (numLevelSets > 0) {
            if (numLevelSets == 1) {
                addWarning(SOP_MESSAGE, "a level set grid was clipped;"
                    " the resulting grid might not be a valid level set");
            } else {
                addWarning(SOP_MESSAGE, "some level sets were clipped;"
                    " the resulting grids might not be valid level sets");
            }
        }

    } catch (std::exception& e) {
        addError(SOP_MESSAGE, e.what());
    }

    return error();
}
