// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file SOP_OpenVDB_Fracture.cc
///
/// @author FX R&D OpenVDB team
///
/// @brief Level set fracturing

#include <houdini_utils/ParmFactory.h>
#include <openvdb_houdini/Utils.h>
#include <openvdb_houdini/GeometryUtil.h>
#include <openvdb_houdini/SOP_NodeVDB.h>

#include <openvdb/tools/MeshToVolume.h>
#include <openvdb/tools/LevelSetFracture.h>

#include <openvdb/util/Util.h>

#include <GA/GA_ElementGroupTable.h>
#include <GA/GA_PageHandle.h>
#include <GA/GA_PageIterator.h>
#include <GA/GA_AttributeInstanceMatrix.h>
#include <GEO/GEO_PrimClassifier.h>
#include <GEO/GEO_PointClassifier.h>
#include <GU/GU_ConvertParms.h>
#include <UT/UT_Quaternion.h>
#include <UT/UT_ValArray.h>
#include <UT/UT_Version.h>

#include <hboost/algorithm/string/join.hpp>
#include <hboost/math/constants/constants.hpp>

#include <cmath>
#include <iostream>
#include <limits>
#include <list>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>


namespace hvdb = openvdb_houdini;
namespace hutil = houdini_utils;


////////////////////////////////////////


class SOP_OpenVDB_Fracture: public hvdb::SOP_NodeVDB
{
public:
    SOP_OpenVDB_Fracture(OP_Network*, const char* name, OP_Operator*);
    ~SOP_OpenVDB_Fracture() override {}

    static OP_Node* factory(OP_Network*, const char* name, OP_Operator*);

    int isRefInput(unsigned i ) const override { return (i > 0); }

    class Cache: public SOP_VDBCacheOptions
    {
    protected:
        OP_ERROR cookVDBSop(OP_Context&) override;

        template<class GridType>
        void process(
            std::list<openvdb::GridBase::Ptr>& grids,
            const GU_Detail* cutterGeo,
            const GU_Detail* pointGeo,
            hvdb::Interrupter&,
            const fpreal time);
    }; // class Cache

protected:
    bool updateParmsFlags() override;
    void resolveObsoleteParms(PRM_ParmList*) override;
}; // class SOP_OpenVDB_Fracture


////////////////////////////////////////


// Build UI and register this operator.
void
newSopOperator(OP_OperatorTable* table)
{
    if (table == nullptr) return;

    hutil::ParmList parms;

    //////////
    // Input options

    parms.add(hutil::ParmFactory(PRM_STRING, "group", "Group")
        .setChoiceList(&hutil::PrimGroupMenuInput1)
        .setTooltip("Select a subset of the input OpenVDB grids to fracture.")
        .setDocumentation(
            "A subset of the input VDBs to fracture"
            " (see [specifying volumes|/model/volumes#group])"));


    //////////
    // Fracture options
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "separatecutters", "Separate Cutters by Connectivity")
        .setTooltip(
            "The cutter geometry will be classified by point connectivity"
            " and each connected component will be cut separately.\n"
            "Use this if an individual piece of cutting geometry has overlapping components.\n\n"
            "This option is not available when cutter instance points are provided."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "cutteroverlap", "Allow Cutter Overlap")
        .setTooltip(
            "Allow consecutive cutter instances to fracture previously generated fragments."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "centercutter", "Center Cutter Geometry")
#ifndef SESI_OPENVDB
        .setDefault(PRMoneDefaults)
#else
        .setDefault(PRMzeroDefaults)
#endif
        .setTooltip(
            "Center the cutter geometry around its point position centroid before instancing."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "randomizerotation", "Randomize Cutter Rotation")
        .setTooltip(
            "Apply a random rotation to the cutter as it is instanced onto each point.\n\n"
            "This option is only available when cutter instance points are provided."));

    parms.add(hutil::ParmFactory(PRM_INT_J, "seed", "Random Seed")
        .setDefault(PRMoneDefaults)
        .setTooltip("The random number seed for cutter rotations"));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "segmentfragments",
        "Split Input Fragments into Primitives")
        .setTooltip(
            "Split input VDBs with disjoint fragments into multiple primitives,"
            " one per fragment.\nIn a chain of fracture nodes this operation"
            " is typically applied only to the last node.")
        .setDocumentation(
            "Split input VDBs with disjoint fragments into multiple primitives,"
            " one per fragment.\n\n"
"If you have a tube and cut out the middle, the two ends might end up in the\n"
"same VDB.  This option will detect that and split the ends into two VDBs.\n\n"
"NOTE:\n"
"    This operation only needs to be performed if you plan on using the\n"
"    output fragments for collision detection. If you use multiple fracture\n"
"    nodes, then it is most efficient to only enable it on the very last\n"
"    fracture node.\n"));

    parms.add(hutil::ParmFactory(PRM_STRING, "fragmentgroup", "Fragment Group")
        .setTooltip("Specify a group name with which to associate all fragments generated "
            "by this fracture. The residual fragments of the input grids are excluded "
            "from this group."));

    {
        char const * const visnames[] = {
            "none", "None",
            "all",  "Pieces",
            "new",  "New Fragments",
            nullptr
        };
        parms.add(hutil::ParmFactory(PRM_ORD, "visualizepieces", "Visualization")
            .setDefault(PRMzeroDefaults)
            .setChoiceListItems(PRM_CHOICELIST_SINGLE, visnames)
            .setTooltip("Randomize output primitive colors.")
            .setDocumentation(
                "The generated VDBs can be colored uniquely for ease of identification."
                " The New Fragments option will leave the original pieces with their"
                " original coloring and assign colors only to newly-created pieces."));
    }

    //////////

    hutil::ParmList obsoleteParms;
    obsoleteParms.add(hutil::ParmFactory(PRM_STRING, "inputgroup", "Group"));

    //////////

    hvdb::OpenVDBOpFactory("VDB Fracture", SOP_OpenVDB_Fracture::factory, parms, *table)
        .addInput("OpenVDB grids to fracture\n"
            "(Required to have matching transforms and narrow band widths)")
        .addInput("Cutter objects (geometry).")
        .addOptionalInput("Optional points to instance the cutter object onto\n"
            "(The cutter object is used in place if no points are provided.)")
        .setObsoleteParms(obsoleteParms)
        .setVerb(SOP_NodeVerb::COOK_INPLACE, []() { return new SOP_OpenVDB_Fracture::Cache; })
        .setDocumentation("\
#icon: COMMON/openvdb\n\
#tags: vdb\n\
\n\
\"\"\"Split level set VDB volumes into pieces.\"\"\"\n\
\n\
@overview\n\
\n\
This node splits level set VDB volumes into multiple fragments.\n\
\n\
The _cutter_ geometry supplied in the second input determines\n\
where cuts are made in the source volumes.\n\
The optional third input specifies points onto which the cutter geometry\n\
will be instanced, so that even simple geometry can produce complex cuts.\n\
\n\
Typically, the input volume is the output of an [OpenVDB from\n\
Polygons|Node:sop/DW_OpenVDBFromPolygons] node.\n\
When that is the case, the fractured result can be converted back\n\
to polygons seamlessly using the\n\
[OpenVDB Convert|Node:sop/DW_OpenVDBConvert] node\n\
with the original polygons as the second input.\n\
\n\
NOTE:\n\
    The cutter geometry must be a closed surface but does not need to be\n\
    manifold. The cutter geometry can contain self intersections and\n\
    degenerate faces. Normals on the cutter geometry are ignored.\n\
\n\
NOTE:\n\
    The reference points supplied in the optional third input can have\n\
    attributes that control how the cutter is transformed onto them. This\n\
    follows the same rules that the [Node:sop/copy] node uses, except\n\
    for scaling. Scaling an SDF correctly requires that the level set\n\
    be rebuilt at the same time. Thus you must scale your cutter geometry\n\
    appropriately first.\n\
\n\
@related\n\
- [OpenVDB Convert|Node:sop/DW_OpenVDBConvert]\n\
- [OpenVDB From Polygons|Node:sop/DW_OpenVDBFromPolygons]\n\
- [Node:sop/vdbfracture]\n\
\n\
@examples\n\
\n\
See [openvdb.org|http://www.openvdb.org/download/] for source code\n\
and usage examples.\n");
}


////////////////////////////////////////


OP_Node*
SOP_OpenVDB_Fracture::factory(OP_Network* net,
    const char* name, OP_Operator* op)
{
    return new SOP_OpenVDB_Fracture(net, name, op);
}


SOP_OpenVDB_Fracture::SOP_OpenVDB_Fracture(OP_Network* net,
    const char* name, OP_Operator* op): hvdb::SOP_NodeVDB(net, name, op)
{
}


////////////////////////////////////////


void
SOP_OpenVDB_Fracture::resolveObsoleteParms(PRM_ParmList* obsoleteParms)
{
    if (!obsoleteParms) return;

    resolveRenamedParm(*obsoleteParms, "inputgroup", "group");

    // Delegate to the base class.
    hvdb::SOP_NodeVDB::resolveObsoleteParms(obsoleteParms);
}


// Enable or disable parameters in the UI.
bool
SOP_OpenVDB_Fracture::updateParmsFlags()
{
    bool changed = false;

    const bool instancePointsExist = (nInputs() == 3);
    const bool multipleCutters = bool(evalInt("separatecutters", 0, 0));
    const bool randomizeRotation = bool(evalInt("randomizerotation", 0, 0));

    changed |= enableParm("separatecutters", !instancePointsExist);
    changed |= enableParm("centercutter", instancePointsExist);
    changed |= enableParm("randomizerotation", instancePointsExist);
    changed |= enableParm("seed", randomizeRotation);
    changed |= enableParm("cutteroverlap", instancePointsExist || multipleCutters);

    return changed;
}


////////////////////////////////////////


OP_ERROR
SOP_OpenVDB_Fracture::Cache::cookVDBSop(OP_Context& context)
{
    try {
        const fpreal time = context.getTime();

        hvdb::Interrupter boss("Converting geometry to volume");

        //////////
        // Validate inputs

        const GU_Detail* cutterGeo = inputGeo(1);
        if (!cutterGeo || !cutterGeo->getNumPrimitives()) {
            // All good, nothing to worry about with no cutting objects!
            return error();
        }

        std::string warningStr;
        auto geoPtr = hvdb::convertGeometry(*cutterGeo, warningStr, &boss);

        if (geoPtr) {
            cutterGeo = geoPtr.get();
            if (!warningStr.empty()) addWarning(SOP_MESSAGE, warningStr.c_str());
        }

        const GU_Detail* pointGeo = inputGeo(2);

        const GA_PrimitiveGroup* group = matchGroup(*gdp, evalStdString("group", time));

        std::list<openvdb::GridBase::Ptr> grids;
        std::vector<GU_PrimVDB*> origvdbs;

        std::vector<std::string> nonLevelSetList, nonLinearList;

        for (hvdb::VdbPrimIterator vdbIter(gdp, group); vdbIter; ++vdbIter) {

            if (boss.wasInterrupted()) break;

            const openvdb::GridClass gridClass = vdbIter->getGrid().getGridClass();
            if (gridClass != openvdb::GRID_LEVEL_SET) {
                nonLevelSetList.push_back(vdbIter.getPrimitiveNameOrIndex().toStdString());
                continue;
            }

            if (!vdbIter->getGrid().transform().isLinear()) {
                nonLinearList.push_back(vdbIter.getPrimitiveNameOrIndex().toStdString());
                continue;
            }

            GU_PrimVDB* vdb = vdbIter.getPrimitive();

            vdb->makeGridUnique();

            grids.push_back(vdb->getGrid().copyGrid());
            grids.back()->setName(vdb->getGridName());

            grids.back()->insertMeta("houdiniorigoffset",
                openvdb::Int64Metadata( vdb->getMapOffset() ) );

            origvdbs.push_back(vdb);
        }

        if (!nonLevelSetList.empty()) {
            std::string s = "The following non level set grids were skipped: '" +
                hboost::algorithm::join(nonLevelSetList, ", ") + "'.";
            addWarning(SOP_MESSAGE, s.c_str());
        }

        if (!nonLinearList.empty()) {
            std::string s = "The following grids were skipped: '" +
                hboost::algorithm::join(nonLinearList, ", ") +
                "' because they don't have a linear/affine transform.";
            addWarning(SOP_MESSAGE, s.c_str());
        }

        if (!grids.empty() && !boss.wasInterrupted()) {

            if (grids.front()->isType<openvdb::FloatGrid>()) {
                process<openvdb::FloatGrid>(grids, cutterGeo, pointGeo, boss, time);
            } else if (grids.front()->isType<openvdb::DoubleGrid>()) {
                process<openvdb::DoubleGrid>(grids, cutterGeo, pointGeo, boss, time);
            } else {
                addError(SOP_MESSAGE, "Unsupported grid type");
            }

            for (std::vector<GU_PrimVDB*>::iterator it = origvdbs.begin();
                it != origvdbs.end(); ++it)
            {
                gdp->destroyPrimitive(**it, /*andPoints=*/true);
            }
        } else {
             addWarning(SOP_MESSAGE, "No VDB grids to fracture");
        }

        if (boss.wasInterrupted()) {
            addWarning(SOP_MESSAGE, "Process was interrupted");
        }

        boss.end();

    } catch (std::exception& e) {
        addError(SOP_MESSAGE, e.what());
    }

    return error();
}


////////////////////////////////////////


template<class GridType>
void
SOP_OpenVDB_Fracture::Cache::process(
    std::list<openvdb::GridBase::Ptr>& grids,
    const GU_Detail* cutterGeo,
    const GU_Detail* pointGeo,
    hvdb::Interrupter& boss,
    const fpreal time)
{
    GA_PrimitiveGroup* group = nullptr;

    // Evaluate the UI parameters.

    {
        UT_String newGropStr;
        evalString(newGropStr, "fragmentgroup", 0, time);
        if(newGropStr.length() > 0) {
            group = gdp->findPrimitiveGroup(newGropStr);
            if (!group) group = gdp->newPrimitiveGroup(newGropStr);
        }
    }

    const bool randomizeRotation = bool(evalInt("randomizerotation", 0, time));
    const bool cutterOverlap = bool(evalInt("cutteroverlap", 0, time));
    const exint visualization = evalInt("visualizepieces", 0, time);
    const bool segmentFragments = bool(evalInt("segmentfragments", 0, time));

    using ValueType = typename GridType::ValueType;

    typename GridType::Ptr firstGrid = openvdb::gridPtrCast<GridType>(grids.front());
    if (!firstGrid) {
        addError(SOP_MESSAGE, "Unsupported grid type.");
        return;
    }

    // Get the first grid's transform and background value.
    openvdb::math::Transform::Ptr transform = firstGrid->transformPtr();
    const ValueType backgroundValue = firstGrid->background();

    std::vector<openvdb::Vec3s> instancePoints;
    std::vector<openvdb::math::Quats> instanceRotations;

    if (pointGeo != nullptr) {
        instancePoints.resize(pointGeo->getNumPoints());

        GA_Range range(pointGeo->getPointRange());
        GA_AttributeInstanceMatrix instanceMatrix;
        instanceMatrix.initialize(pointGeo->pointAttribs(), "N", "v");
        // Ignore any scaling until levelset fracture supports it.
        instanceMatrix.resetScales();

        // If we're randomizing or there are *any* valid transformation
        // attributes, we need to create an instance matrix.
        if (randomizeRotation || instanceMatrix.hasAnyAttribs()) {
            instanceRotations.resize(instancePoints.size());
            using RandGen = std::mt19937;
            RandGen rng(RandGen::result_type(evalInt("seed", 0, time)));
            std::uniform_real_distribution<float> uniform01;
            const float two_pi = 2.0f * hboost::math::constants::pi<float>();
            UT_DMatrix4 xform;
            UT_Vector3 trans;
            UT_DMatrix3 rotmat;
            UT_QuaternionD quat;

            for (GA_Iterator it(range); !it.atEnd(); it.advance()) {
                UT_Vector3 pos = pointGeo->getPos3(*it);

                if (randomizeRotation) {
                    // Generate uniform random rotations by picking random
                    // points in the unit cube and forming the unit quaternion.

                    const float u  = uniform01(rng);
                    const float c1 = std::sqrt(1-u);
                    const float c2 = std::sqrt(u);
                    const float s1 = two_pi * uniform01(rng);
                    const float s2 = two_pi * uniform01(rng);

                    UT_Quaternion  orient(c1 * std::sin(s1), c1 * std::cos(s1),
                                          c2 * std::sin(s2), c2 * std::cos(s2));
                    instanceMatrix.getMatrix(xform, pos, orient, *it);
                }
                else {
                    instanceMatrix.getMatrix(xform, pos, *it);
                }
                GA_Index i = pointGeo->pointIndex(*it);
                xform.getTranslates(trans);
                xform.extractRotate(rotmat);
                quat.updateFromRotationMatrix(rotmat);
                instancePoints[i] = openvdb::Vec3s(trans.x(), trans.y(), trans.z());
                instanceRotations[i].init(
                    static_cast<float>(quat.x()),
                    static_cast<float>(quat.y()),
                    static_cast<float>(quat.z()),
                    static_cast<float>(quat.w()));
            }
        }
        else
        {
            // No randomization or valid instance attributes, just use P.
            for (GA_Iterator it(range); !it.atEnd(); it.advance()) {
                UT_Vector3 pos = pointGeo->getPos3(*it);
                instancePoints[pointGeo->pointIndex(*it)] =
                    openvdb::Vec3s(pos.x(), pos.y(), pos.z());
            }
        }
    }
    if (boss.wasInterrupted()) return;

    std::list<typename GridType::Ptr> residuals;

    {
        std::list<openvdb::GridBase::Ptr>::iterator it = grids.begin();

        std::vector<std::string> badTransformList, badBackgroundList, badTypeList;

        for (; it != grids.end(); ++it) {
            typename GridType::Ptr residual = openvdb::gridPtrCast<GridType>(*it);

            if (residual) {

                if (residual->transform() != *transform) {
                    badTransformList.push_back(residual->getName());
                    continue;
                }

                if (!openvdb::math::isApproxEqual(residual->background(), backgroundValue)) {
                    badBackgroundList.push_back(residual->getName());
                    continue;
                }

                residuals.push_back(residual);

            } else {
                badTypeList.push_back(residual->getName());
                continue;
            }

        }
        grids.clear();


        if (!badTransformList.empty()) {
            std::string s = "The following grids were skipped: '" +
                hboost::algorithm::join(badTransformList, ", ") +
                "' because they don't match the transform of the first grid.";
            addWarning(SOP_MESSAGE, s.c_str());
        }

        if (!badBackgroundList.empty()) {
            std::string s = "The following grids were skipped: '" +
                hboost::algorithm::join(badBackgroundList, ", ") +
                "' because they don't match the background value of the first grid.";
            addWarning(SOP_MESSAGE, s.c_str());
        }

        if (!badTypeList.empty()) {
            std::string s = "The following grids were skipped: '" +
                hboost::algorithm::join(badTypeList, ", ") +
                "' because they don't have the same data type as the first grid.";
            addWarning(SOP_MESSAGE, s.c_str());
        }
    }

    // Setup fracture tool
    openvdb::tools::LevelSetFracture<GridType, hvdb::Interrupter> lsFracture(&boss);

    const bool separatecutters = (pointGeo == nullptr) && bool(evalInt("separatecutters", 0, time));

    std::vector<openvdb::Vec3s> pointList;

    {
        pointList.resize(cutterGeo->getNumPoints());
        openvdb::math::Transform::Ptr xform = transform->copy();

        if (!instancePoints.empty() && !separatecutters && bool(evalInt("centercutter", 0, time))) {
            UT_BoundingBox pointBBox;
            cutterGeo->getPointBBox(&pointBBox);
            UT_Vector3 center = pointBBox.center();
            xform->postTranslate(openvdb::Vec3s(center.x(), center.y(), center.z()));
        }

        UTparallelFor(GA_SplittableRange(cutterGeo->getPointRange()),
                hvdb::TransformOp(cutterGeo, *xform, pointList));
    }

    // Check for multiple cutter objects
    GEO_PrimClassifier primClassifier;
    if (separatecutters) {
        primClassifier.classifyBySharedPoints(*cutterGeo);
    }

    const int cutterObjects = separatecutters ? primClassifier.getNumClass() : 1;
    const float bandWidth = float(backgroundValue / transform->voxelSize()[0]);

    if (cutterObjects > 1) {
        GA_Offset start, end;
        GA_SplittableRange range(cutterGeo->getPrimitiveRange());

        for (int classId = 0; classId < cutterObjects; ++classId) {

            if (boss.wasInterrupted()) break;

            size_t numPrims = 0;
            for (GA_PageIterator pageIt = range.beginPages(); !pageIt.atEnd(); ++pageIt) {
                for (GA_Iterator blockIt(pageIt.begin()); blockIt.blockAdvance(start, end); ) {
                    for (GA_Offset i = start; i < end; ++i) {
                        if (classId == primClassifier.getClass(
                            static_cast<int>(cutterGeo->primitiveIndex(i))))
                        {
                            ++numPrims;
                        }
                    }
                }
            }

            typename GridType::Ptr cutterGrid;

            if (numPrims == 0) continue;

            {
                std::vector<openvdb::Vec4I> primList;
                primList.reserve(numPrims);

                openvdb::Vec4I prim;
                using Vec4IValueType = openvdb::Vec4I::ValueType;

                for (GA_PageIterator pageIt = range.beginPages(); !pageIt.atEnd(); ++pageIt) {
                    for (GA_Iterator blockIt(pageIt.begin()); blockIt.blockAdvance(start, end); ) {
                        for (GA_Offset i = start; i < end; ++i) {
                            if (classId == primClassifier.getClass(
                                static_cast<int>(cutterGeo->primitiveIndex(i))))
                            {
                                const GA_Primitive* primRef = cutterGeo->getPrimitiveList().get(i);
                                const GA_Size vtxn = primRef->getVertexCount();

                                if ((primRef->getTypeId() == GEO_PRIMPOLY) &&
                                    (3 == vtxn || 4 == vtxn))
                                {
                                    for (GA_Size vtx = 0; vtx < vtxn; ++vtx) {
                                        prim[int(vtx)] = static_cast<Vec4IValueType>(
                                            cutterGeo->pointIndex(primRef->getPointOffset(vtx)));
                                    }

                                    if (vtxn != 4) prim[3] = openvdb::util::INVALID_IDX;

                                    primList.push_back(prim);
                                }
                            }
                        }
                    }
                }

                openvdb::tools::QuadAndTriangleDataAdapter<openvdb::Vec3s, openvdb::Vec4I>
                    mesh(pointList, primList);

                cutterGrid = openvdb::tools::meshToVolume<GridType>(
                    boss, mesh, *transform, bandWidth, bandWidth);
            }

            if (!cutterGrid || cutterGrid->activeVoxelCount() == 0) continue;

            lsFracture.fracture(residuals, *cutterGrid, segmentFragments,
                nullptr, nullptr, cutterOverlap);
        }
    } else {

        // Convert cutter object mesh to level-set
        typename GridType::Ptr cutterGrid;

        {
            std::vector<openvdb::Vec4I> primList;
            primList.resize(cutterGeo->getNumPrimitives());

            UTparallelFor(GA_SplittableRange(cutterGeo->getPrimitiveRange()),
                hvdb::PrimCpyOp(cutterGeo, primList));


            openvdb::tools::QuadAndTriangleDataAdapter<openvdb::Vec3s, openvdb::Vec4I>
                mesh(pointList, primList);

            cutterGrid = openvdb::tools::meshToVolume<GridType>(
                boss, mesh, *transform, bandWidth, bandWidth);
        }

        if (!cutterGrid || cutterGrid->activeVoxelCount() == 0 || boss.wasInterrupted()) return;


        lsFracture.fracture(residuals, *cutterGrid, segmentFragments, &instancePoints,
            &instanceRotations, cutterOverlap);

    }


    if (boss.wasInterrupted()) return;

    typename std::list<typename GridType::Ptr>::iterator it;

    // Primitive Color
    GA_RWHandleV3 color;
    if (visualization) {
        GA_RWAttributeRef attrRef = gdp->findDiffuseAttribute(GA_ATTRIB_PRIMITIVE);
        if (!attrRef.isValid()) attrRef = gdp->addDiffuseAttribute(GA_ATTRIB_PRIMITIVE);
        color.bind(attrRef.getAttribute());
    }

    UT_IntArray piececount;
    UT_IntArray totalpiececount;

    piececount.entries(gdp->getNumPrimitiveOffsets());
    totalpiececount.entries(gdp->getNumPrimitiveOffsets());

    GU_ConvertParms parms;
    parms.preserveGroups = true;

    // Export residual fragments
    exint coloridx = 0;
    GA_RWHandleS name_h(gdp, GA_ATTRIB_PRIMITIVE, "name");

    // We have to do a pre-pass over all pieces to compute the total
    // number of pieces from each original object.  This way
    // we can tell if we need to do renaming or not.
    for (it = residuals.begin(); it != residuals.end(); ++it) {

        GA_Offset origvdboff = GA_INVALID_OFFSET;

        typename GridType::Ptr grid = *it;
        openvdb::Int64Metadata::Ptr offmeta =
            grid->template getMetadata<openvdb::Int64Metadata>("houdiniorigoffset");
        if (offmeta) {
            origvdboff = static_cast<GA_Offset>(offmeta->value());
        }
        if (origvdboff != GA_INVALID_OFFSET) {
            totalpiececount(origvdboff)++;
        }
    }
    for (it = lsFracture.fragments().begin(); it != lsFracture.fragments().end(); ++it) {

        GA_Offset origvdboff = GA_INVALID_OFFSET;

        typename GridType::Ptr grid = *it;
        openvdb::Int64Metadata::Ptr offmeta =
            grid->template getMetadata<openvdb::Int64Metadata>("houdiniorigoffset");
        if (offmeta) {
            origvdboff = static_cast<GA_Offset>(offmeta->value());
        }
        if (origvdboff != GA_INVALID_OFFSET) {
            totalpiececount(origvdboff)++;
        }
    }

    for (it = residuals.begin(); it != residuals.end(); ++it) {
        if (boss.wasInterrupted()) break;

        typename GridType::Ptr grid = *it;
        GA_Offset origvdboff = GA_INVALID_OFFSET;

        openvdb::Int64Metadata::Ptr offmeta =
            grid->template getMetadata<openvdb::Int64Metadata>("houdiniorigoffset");
        if (offmeta) {
            origvdboff = static_cast<GA_Offset>(offmeta->value());
            grid->removeMeta("houdiniorigoffset");
        }

        std::string gridname = grid->getName();
        UT_String name;

        name.harden(gridname.c_str());

        // Suffix our name.
        if (name.isstring() && origvdboff != GA_INVALID_OFFSET
            && totalpiececount(origvdboff) > 1)
        {
            UT_WorkBuffer buf;
            buf.sprintf("%s_%d", static_cast<const char*>(name), piececount(origvdboff));
            piececount(origvdboff)++;
            name.harden(buf.buffer());
        }

        GU_PrimVDB* vdb = hvdb::createVdbPrimitive(*gdp, grid, static_cast<const char*>(name));

        if (origvdboff != GA_INVALID_OFFSET)
        {
            GU_PrimVDB* origvdb =
                dynamic_cast<GU_PrimVDB*>(gdp->getGEOPrimitive(origvdboff));

            if (origvdb)
            {
                GA_Offset newvdbpt;

                newvdbpt = vdb->getPointOffset(0);
                GUconvertCopySingleVertexPrimAttribsAndGroups(
                    parms,
                    *origvdb->getParent(),
                    origvdb->getMapOffset(),
                    *gdp,
                    GA_Range(gdp->getPrimitiveMap(), vdb->getMapOffset(), vdb->getMapOffset()+1),
                    GA_Range(gdp->getPointMap(), newvdbpt, newvdbpt+1));
            }
        }

        if (visualization == 1 && color.isValid())
        {
            float r, g, b;
            UT_Color::getUniqueColor(coloridx, &r, &g, &b);
            color.set(vdb->getMapOffset(), UT_Vector3(r, g, b));
        }
        coloridx++;

        if (name.isstring() && name_h.isValid())
        {
            name_h.set(vdb->getMapOffset(), static_cast<const char*>(name));
        }
    }

    if (boss.wasInterrupted()) return;

    // Export new fragments
    for (it = lsFracture.fragments().begin(); it != lsFracture.fragments().end(); ++it) {

        if (boss.wasInterrupted()) break;

        typename GridType::Ptr grid = *it;

        GA_Offset origvdboff = GA_INVALID_OFFSET;

        openvdb::Int64Metadata::Ptr offmeta =
            grid->template getMetadata<openvdb::Int64Metadata>("houdiniorigoffset");
        if (offmeta) {
            origvdboff = static_cast<GA_Offset>(offmeta->value());
            grid->removeMeta("houdiniorigoffset");
        }

        std::string gridname = grid->getName();
        UT_String name;

        name.harden(gridname.c_str());

        // Suffix our name.
        if (name.isstring() && origvdboff != GA_INVALID_OFFSET)
        {
            UT_WorkBuffer buf;
            buf.sprintf("%s_%d", static_cast<const char*>(name), piececount(origvdboff));
            piececount(origvdboff)++;
            name.harden(buf.buffer());
        }
        GU_PrimVDB* vdb = hvdb::createVdbPrimitive(*gdp, grid, static_cast<const char*>(name));
        if (origvdboff != GA_INVALID_OFFSET)
        {
            GU_PrimVDB* origvdb =
                dynamic_cast<GU_PrimVDB*>(gdp->getGEOPrimitive(origvdboff));

            if (origvdb)
            {
                GA_Offset newvdbpt;

                newvdbpt = vdb->getPointOffset(0);
                GUconvertCopySingleVertexPrimAttribsAndGroups(
                    parms,
                    *origvdb->getParent(),
                    origvdb->getMapOffset(),
                    *gdp,
                    GA_Range(gdp->getPrimitiveMap(), vdb->getMapOffset(), vdb->getMapOffset()+1),
                    GA_Range(gdp->getPointMap(), newvdbpt, newvdbpt+1));
            }
        }
        if (name.isstring() && name_h.isValid())
        {
            name_h.set(vdb->getMapOffset(), static_cast<const char*>(name));
        }

        if (group) group->add(vdb);

        if (visualization && color.isValid())
        {
            float r, g, b;
            UT_Color::getUniqueColor(coloridx++, &r, &g, &b);
            color.set(vdb->getMapOffset(), UT_Vector3(r, g, b));
        }
        coloridx++;
    }
}
