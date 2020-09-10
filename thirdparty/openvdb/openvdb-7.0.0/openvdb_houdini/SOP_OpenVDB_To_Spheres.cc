// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file SOP_OpenVDB_To_Spheres.cc
///
/// @author FX R&D OpenVDB team
///
/// @brief Fills a volume with adaptively sized overlapping or nonoverlapping spheres.

#include <houdini_utils/ParmFactory.h>
#include <openvdb_houdini/SOP_NodeVDB.h>
#include <openvdb_houdini/GeometryUtil.h>
#include <openvdb_houdini/Utils.h>

#include <openvdb/tools/VolumeToSpheres.h>

#include <GU/GU_ConvertParms.h>
#include <GU/GU_Detail.h>
#include <GU/GU_PrimSphere.h>
#include <PRM/PRM_Parm.h>
#include <GA/GA_PageIterator.h>
#include <UT/UT_Interrupt.h>
#include <UT/UT_Version.h>

#include <hboost/algorithm/string/join.hpp>

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>



namespace hvdb = openvdb_houdini;
namespace hutil = houdini_utils;


////////////////////////////////////////


class SOP_OpenVDB_To_Spheres: public hvdb::SOP_NodeVDB
{
public:
    SOP_OpenVDB_To_Spheres(OP_Network*, const char* name, OP_Operator*);
    ~SOP_OpenVDB_To_Spheres() override {}

    static OP_Node* factory(OP_Network*, const char* name, OP_Operator*);

    int isRefInput(unsigned i) const override { return (i > 0); }

    void checkActivePart(float time);

    class Cache: public SOP_VDBCacheOptions { OP_ERROR cookVDBSop(OP_Context&) override; };

protected:
    void resolveObsoleteParms(PRM_ParmList*) override;
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
        .setTooltip("A subset of the input VDBs to be processed")
        .setDocumentation(
            "A subset of the input VDB grids to be processed"
            " (see [specifying volumes|/model/volumes#group])"));

    parms.add(hutil::ParmFactory(PRM_FLT_J, "isovalue", "Isovalue")
        .setDefault(PRMzeroDefaults)
        .setRange(PRM_RANGE_UI, -1.0, PRM_RANGE_UI, 1.0)
        .setTooltip(
            "The voxel value that determines the surface of the volume\n\n"
            "Zero works for signed distance fields, while fog volumes"
            " require a small positive value (0.5 is a good initial guess)."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "worldunits", "Use World Space Units")
        .setDocumentation(
            "If enabled, specify sphere radii in world units, otherwise in voxels."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "useradiusmin", "")
        .setDefault(PRMoneDefaults)
        .setTypeExtended(PRM_TYPE_TOGGLE_JOIN));

    parms.add(hutil::ParmFactory(PRM_FLT_J, "radiusmin", "Min Radius")
        .setDefault(PRMoneDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 1e-5, PRM_RANGE_UI, 2.0)
        .setTooltip("The radius of the smallest sphere allowed")
        .setDocumentation(
            "The radius of the smallest sphere allowed\n\n"
            "If disabled, allow spheres of any radius greater than zero."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "useradiusmax", "")
        .setDefault(PRMzeroDefaults)
        .setTypeExtended(PRM_TYPE_TOGGLE_JOIN));

    parms.add(hutil::ParmFactory(PRM_FLT_J, "radiusmax", "Max Radius")
        .setDefault(100.0)
        .setRange(PRM_RANGE_RESTRICTED, 1e-5, PRM_RANGE_UI, 100.0)
        .setTooltip("The radius of the largest sphere allowed")
        .setDocumentation(
            "The radius of the largest sphere allowed\n\n"
            "If disabled, allow arbitrarily large spheres."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "usespheresmin", "")
        .setDefault(PRMoneDefaults)
        .setTypeExtended(PRM_TYPE_TOGGLE_JOIN));

    parms.add(hutil::ParmFactory(PRM_INT_J, "spheresmin", "Min Spheres")
        .setRange(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_UI, 100)
        .setDefault(1)
        .setTooltip("The minimum number of spheres to be generated")
        .setDocumentation(
            "The minimum number of spheres to be generated\n\n"
            "If disabled, allow very small VDBs to not generate any spheres.\n\n"
            "NOTE:\n"
            "    __Min Spheres__ takes precedence over __Min Radius__.\n"
            "    Spheres smaller than __Min Radius__ might be generated\n"
            "    in order to ensure that the minimum sphere count is satisfied."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "usespheresmax", "")
        .setDefault(PRMoneDefaults)
        .setTypeExtended(PRM_TYPE_TOGGLE_JOIN));

    parms.add(hutil::ParmFactory(PRM_INT_J, "spheresmax", "Max Spheres")
        .setRange(PRM_RANGE_RESTRICTED, 1, PRM_RANGE_UI, 100)
        .setDefault(50)
        .setTooltip("The maximum number of spheres to be generated")
        .setDocumentation(
            "The maximum number of spheres to be generated\n\n"
            "If disabled, allow for up to __Point Count__ spheres to be generated."));

    parms.add(hutil::ParmFactory(PRM_INT_J, "scatter", "Point Count")
        .setRange(PRM_RANGE_RESTRICTED, 1000, PRM_RANGE_UI, 50000)
        .setDefault(10000)
        .setTooltip(
            "The number of candidate sphere centers to consider\n\n"
            "Increasing this count increases the chances of finding optimal sphere sizes."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "overlapping", "Overlapping")
#ifndef SESI_OPENVDB
        .setDefault(PRMzeroDefaults)
#else
        .setDefault(PRMoneDefaults)
#endif
        .setTooltip("If enabled, allow spheres to overlap/intersect."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "preserve", "Preserve Attributes and Groups")
#ifndef SESI_OPENVDB
        .setDefault(PRMzeroDefaults)
#else
        .setDefault(PRMoneDefaults)
#endif
        .setTooltip("If enabled, copy attributes and groups from the input."));

    // The "doid" parameter name comes from the standard in POPs
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "doid", "Add ID Attribute")
#ifndef SESI_OPENVDB
        .setDefault(PRMoneDefaults)
#else
        .setDefault(PRMzeroDefaults)
#endif
        .setTooltip("If enabled, add an id point attribute that denotes the source VDB.")
        .setDocumentation(
            "If enabled, add an `id` point attribute that denotes the source VDB"
            " for each sphere."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "dopscale", "Add PScale Attribute")
        .setDefault(PRMzeroDefaults)
        .setTooltip("If enabled, add a pscale point attribute to each sphere.")
        .setDocumentation(
            "If enabled, add a `pscale` point attribute that indicates"
            " the radius of each sphere."));

    //////////

    const float fmax = std::numeric_limits<float>::max();

    hutil::ParmList obsoleteParms;
    obsoleteParms.add(hutil::ParmFactory(PRM_INT_J, "spheres", "Max Spheres").setDefault(50));
    obsoleteParms.add(hutil::ParmFactory(PRM_FLT_J, "minradius", "Min Radius").setDefault(1.0));
    obsoleteParms.add(hutil::ParmFactory(PRM_FLT_J, "maxradius", "Max Radius").setDefault(fmax));
    obsoleteParms.add(hutil::ParmFactory(PRM_FLT_J, "minradiusworld", "").setDefault(0.1));
    obsoleteParms.add(hutil::ParmFactory(PRM_FLT_J, "maxradiusworld", "").setDefault(fmax));

    //////////

    hvdb::OpenVDBOpFactory("VDB to Spheres", SOP_OpenVDB_To_Spheres::factory, parms, *table)
#ifndef SESI_OPENVDB
        .setInternalName("DW_OpenVDBToSpheres")
#endif
        .addInput("VDBs to convert")
        .setObsoleteParms(obsoleteParms)
        .setVerb(SOP_NodeVerb::COOK_GENERATOR, []() { return new SOP_OpenVDB_To_Spheres::Cache; })
        .setDocumentation("\
#icon: COMMON/openvdb\n\
#tags: vdb\n\
\n\
\"\"\"Fill a VDB volume with adaptively-sized spheres.\"\"\"\n\
\n\
@overview\n\
\n\
This node is useful for generating proxy geometry for RBD simulations,\n\
since approximating nonconvex geometry with sphere compounds\n\
drastically improves the simulation time.\n\
This can be used, for example, on the output of an\n\
[OpenVDB Fracture node|Node:sop/DW_OpenVDBFracture].\n\
\n\
Another use is to produce the initial density volume for cloud modeling.\n\
\n\
@related\n\
- [OpenVDB Fracture|Node:sop/DW_OpenVDBFracture]\n\
- [Node:sop/cloud]\n\
- [Node:sop/vdbtospheres]\n\
\n\
@examples\n\
\n\
See [openvdb.org|http://www.openvdb.org/download/] for source code\n\
and usage examples.\n");
}


void
SOP_OpenVDB_To_Spheres::resolveObsoleteParms(PRM_ParmList* obsoleteParms)
{
    if (!obsoleteParms) return;

    const fpreal time = 0.0;
    const bool worldUnits = (0 != evalInt("worldunits", 0, time));

    resolveRenamedParm(*obsoleteParms, "spheres", "spheresmax");
    resolveRenamedParm(*obsoleteParms, "minradius", "radiusmin");

    // If world units are enabled, use the old world-space radius bounds if they exist.
    if (worldUnits && obsoleteParms->getParmPtr("minradiusworld")) {
        setFloat("radiusmin", 0, time, obsoleteParms->evalFloat("minradiusworld", 0, time));
    }
    {
        // The old "maxradius" and "maxradiusworld" parameters had default values
        // of numeric_limits<float>::max(), indicating no upper bound.
        // That state is now represented by the "useradiusmax" toggle, which defaults to Off.
        // If "maxradius" (or "maxradiusworld" in world-space mode) had a non-default value,
        // transfer that value to "radiusmax" and toggle "useradiusmax" on.
        char const * const oldName = (worldUnits ? "maxradiusworld" : "maxradius");
        PRM_Parm* parm = obsoleteParms->getParmPtr(oldName);
        if (parm && !parm->isFactoryDefault()) {
            setFloat("radiusmax", 0, time, obsoleteParms->evalFloat(oldName, 0, time));
            setInt("useradiusmax", 0, time, true);
        }
    }

    hvdb::SOP_NodeVDB::resolveObsoleteParms(obsoleteParms);
}


OP_Node*
SOP_OpenVDB_To_Spheres::factory(OP_Network* net,
    const char* name, OP_Operator* op)
{
    return new SOP_OpenVDB_To_Spheres(net, name, op);
}


SOP_OpenVDB_To_Spheres::SOP_OpenVDB_To_Spheres(OP_Network* net,
    const char* name, OP_Operator* op):
    hvdb::SOP_NodeVDB(net, name, op)
{
}


bool
SOP_OpenVDB_To_Spheres::updateParmsFlags()
{
    bool changed = false;
    changed |= enableParm("radiusmin", (0 != evalInt("useradiusmin", 0, 0)));
    changed |= enableParm("radiusmax", (0 != evalInt("useradiusmax", 0, 0)));
    changed |= enableParm("spheresmin", (0 != evalInt("usespheresmin", 0, 0)));
    changed |= enableParm("spheresmax", (0 != evalInt("usespheresmax", 0, 0)));
    return changed;
}


////////////////////////////////////////


OP_ERROR
SOP_OpenVDB_To_Spheres::Cache::cookVDBSop(OP_Context& context)
{
    try {
        const fpreal time = context.getTime();

        hvdb::Interrupter boss("Filling VDBs with spheres");

        const GU_Detail* vdbGeo = inputGeo(0);
        if (vdbGeo == nullptr) return error();

        const GA_PrimitiveGroup* group = matchGroup(*vdbGeo, evalStdString("group", time));
        hvdb::VdbPrimCIterator vdbIt(vdbGeo, group);

        if (!vdbIt) {
            addWarning(SOP_MESSAGE, "No VDBs found.");
            return error();
        }

        // Eval attributes
        const bool
            addID = (0 != evalInt("doid", 0, time)),
            addPScale = (0 != evalInt("dopscale", 0, time)),
            overlapping = (0 != evalInt("overlapping", 0, time)),
            preserve = (0 != evalInt("preserve", 0, time)),
            useMinRadius = (0 != evalInt("useradiusmin", 0, time)),
            useMaxRadius = (0 != evalInt("useradiusmax", 0, time)),
            useMinSpheres = (0 != evalInt("usespheresmin", 0, time)),
            useMaxSpheres = (0 != evalInt("usespheresmax", 0, time)),
            worldUnits = (0 != evalInt("worldunits", 0, time));

        const float
            fmin = std::numeric_limits<float>::min(),
            fmax = std::numeric_limits<float>::max(),
            isovalue = static_cast<float>(evalFloat("isovalue", 0, time)),
            minRadius = !useMinRadius ? fmin : static_cast<float>(evalFloat("radiusmin", 0, time)),
            maxRadius = !useMaxRadius ? fmax : static_cast<float>(evalFloat("radiusmax", 0, time));

        const int scatter = static_cast<int>(evalInt("scatter", 0, time));
        const openvdb::Vec2i sphereCount(
            !useMinSpheres ? 0 : static_cast<int>(evalInt("spheresmin", 0, time)),
            !useMaxSpheres ? scatter : static_cast<int>(evalInt("spheresmax", 0, time)));

        GA_RWHandleI idAttr;
        if (addID) {
            GA_RWAttributeRef aRef = gdp->findPointAttribute("id");
            if (!aRef.isValid()) {
                aRef = gdp->addIntTuple(GA_ATTRIB_POINT, "id", 1, GA_Defaults(0));
            }
            idAttr = aRef.getAttribute();
            if(!idAttr.isValid()) {
                addWarning(SOP_MESSAGE, "Failed to create the point ID attribute.");
                return error();
            }
        }

        GA_RWHandleF pscaleAttr;
        if (addPScale) {
            GA_RWAttributeRef aRef = gdp->findFloatTuple(GA_ATTRIB_POINT, GEO_STD_ATTRIB_PSCALE);
            if (!aRef.isValid()) {
                aRef = gdp->addFloatTuple(
                    GA_ATTRIB_POINT, GEO_STD_ATTRIB_PSCALE, 1, GA_Defaults(0));
            }
            pscaleAttr = aRef.getAttribute();
            if(!pscaleAttr.isValid()) {
                addWarning(SOP_MESSAGE, "Failed to create the point pscale attribute.");
                return error();
            }
        }

        int idNumber = 1;

        GU_ConvertParms parms;
        parms.setKeepGroups(true);

        std::vector<std::string> skippedGrids;

        for (; vdbIt; ++vdbIt) {
            if (boss.wasInterrupted()) break;

            openvdb::Vec2s radiusRange(minRadius, maxRadius);
            if (worldUnits) {
                const float voxelScale = float(1.0 / vdbIt->getGrid().voxelSize()[0]);
                radiusRange *= voxelScale;
            }

            radiusRange[1] = std::max(radiusRange[1], radiusRange[0] + float(1e-5));

            std::vector<openvdb::Vec4s> spheres;

            if (vdbIt->getGrid().type() == openvdb::FloatGrid::gridType()) {

                openvdb::FloatGrid::ConstPtr gridPtr =
                    openvdb::gridConstPtrCast<openvdb::FloatGrid>(vdbIt->getGridPtr());

                openvdb::tools::fillWithSpheres(*gridPtr, spheres, sphereCount, overlapping,
                    radiusRange[0], radiusRange[1], isovalue, scatter, &boss);


            } else if (vdbIt->getGrid().type() == openvdb::DoubleGrid::gridType()) {

                openvdb::DoubleGrid::ConstPtr gridPtr =
                    openvdb::gridConstPtrCast<openvdb::DoubleGrid>(vdbIt->getGridPtr());

                openvdb::tools::fillWithSpheres(*gridPtr, spheres, sphereCount, overlapping,
                    radiusRange[0], radiusRange[1], isovalue, scatter, &boss);

            } else {
                skippedGrids.push_back(vdbIt.getPrimitiveNameOrIndex().toStdString());
                continue;
            }

            GA_Detail::OffsetMarker marker(*gdp);

            // copy spheres to Houdini
            for (size_t n = 0, N = spheres.size(); n < N; ++n) {

                const openvdb::Vec4s& sphere = spheres[n];

                GA_Offset ptoff = gdp->appendPointOffset();

                gdp->setPos3(ptoff, sphere.x(), sphere.y(), sphere.z());

                if (addID) {
                    idAttr.set(ptoff, idNumber);
                }

                if (addPScale) {
                    pscaleAttr.set(ptoff, sphere[3]);
                }

                UT_Matrix4 mat = UT_Matrix4::getIdentityMatrix();
                mat.scale(sphere[3],sphere[3],sphere[3]);

                GU_PrimSphereParms sphereParms(gdp, ptoff);
                sphereParms.xform = mat;
                GU_PrimSphere::build(sphereParms);
            }

            if (preserve) {
                GUconvertCopySingleVertexPrimAttribsAndGroups(
                    parms, *vdbGeo, vdbIt.getOffset(),
                    *gdp, marker.primitiveRange(), marker.pointRange());
            }
            ++idNumber;
        }

        if (!skippedGrids.empty()) {
            std::string s = "Only scalar (float/double) grids are supported, the following "
                "were skipped: '" + hboost::algorithm::join(skippedGrids, ", ") + "'.";
            addWarning(SOP_MESSAGE, s.c_str());
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
