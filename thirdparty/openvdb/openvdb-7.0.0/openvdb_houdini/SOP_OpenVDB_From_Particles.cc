// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file SOP_OpenVDB_From_Particles.cc
///
/// @author FX R&D OpenVDB team
///
/// @brief Converts points into signed distance / level set volumes.
///
/// @note The world space narrow band half-width is encoded in the
///       background value of a level set grid.

#include <houdini_utils/ParmFactory.h>
#include <openvdb_houdini/AttributeTransferUtil.h>
#include <openvdb_houdini/GU_VDBPointTools.h>
#include <openvdb_houdini/SOP_NodeVDB.h>
#include <openvdb_houdini/Utils.h>

#include <openvdb/Grid.h>
#include <openvdb/tools/LevelSetUtil.h>
#include <openvdb/tools/Morphology.h>
#include <openvdb/tools/ParticlesToLevelSet.h>
#include <openvdb/tools/TopologyToLevelSet.h>

#include <CH/CH_Manager.h>
#include <GA/GA_Types.h> // for GA_ATTRIB_POINT
#include <PRM/PRM_Parm.h>
#include <UT/UT_Assert.h>
#include <UT/UT_Version.h>

#include <algorithm>
#include <cmath>
#include <cstdlib> // for std::atoi()
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>



namespace hvdb = openvdb_houdini;
namespace hutil = houdini_utils;


namespace { class ParticleList; }


class SOP_OpenVDB_From_Particles: public hvdb::SOP_NodeVDB
{
public:
    SOP_OpenVDB_From_Particles(OP_Network* net, const char* name, OP_Operator* op):
        hvdb::SOP_NodeVDB(net, name, op) {}

    static OP_Node* factory(OP_Network* net, const char* name, OP_Operator* op)
    {
        return new SOP_OpenVDB_From_Particles(net, name, op);
    }

    int isRefInput(unsigned i) const override { return (i > 0); }

    int convertUnits();

    static int convertUnitsCB(void* data, int /*idx*/, float /*time*/, const PRM_Template*)
    {
        if (auto* sop = static_cast<SOP_OpenVDB_From_Particles*>(data)) {
            return sop->convertUnits();
        }
        return 0;
    }

    static void buildAttrMenu(void*, PRM_Name*, int, const PRM_SpareData*, const PRM_Parm*);

    static const PRM_ChoiceList sPointAttrMenu;

    class Cache: public SOP_VDBCacheOptions
    {
    public:
        float voxelSize() const { return mVoxelSize; }

    protected:
        OP_ERROR cookVDBSop(OP_Context&) override;

    private:
        void convert(
            fpreal time,
            ParticleList&,
            openvdb::FloatGrid::Ptr,
            openvdb::BoolGrid::Ptr,
            hvdb::Interrupter&);

        void convertWithAttributes(
            fpreal time,
            const GU_Detail&,
            ParticleList&,
            openvdb::FloatGrid::Ptr,
            openvdb::BoolGrid::Ptr,
            hvdb::Interrupter&);

        int constructGenericAtttributeList(
            fpreal time,
            hvdb::AttributeDetailList&,
            const GU_Detail&,
            const openvdb::Int32Grid& closestPtIdxGrid);

        float mVoxelSize = 0.1f;
    }; // class Cache

protected:
    void resolveObsoleteParms(PRM_ParmList*) override;
    bool updateParmsFlags() override;
}; // class SOP_OpenVDB_From_Particles


const PRM_ChoiceList SOP_OpenVDB_From_Particles::sPointAttrMenu(
    PRM_ChoiceListType(PRM_CHOICELIST_EXCLUSIVE | PRM_CHOICELIST_REPLACE),
    SOP_OpenVDB_From_Particles::buildAttrMenu);


////////////////////////////////////////


void
newSopOperator(OP_OperatorTable* table)
{
    if (table == nullptr) return;

    hutil::ParmList parms;

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "builddistance", "")
        .setDefault(PRMoneDefaults)
        .setTypeExtended(PRM_TYPE_TOGGLE_JOIN));

    parms.add(hutil::ParmFactory(PRM_STRING, "distancename", "Distance VDB")
        .setDefault("surface")
        .setTooltip("A name for the output SDF volume")
        .setDocumentation(
            "If enabled, output a narrow-band signed distance field with the given name."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "buildfog", "")
        .setTypeExtended(PRM_TYPE_TOGGLE_JOIN));

    parms.add(hutil::ParmFactory(PRM_STRING, "fogname", "Fog VDB")
        .setDefault("density")
        .setTooltip("A name for the output fog volume")
        .setDocumentation(
            "If enabled, output a fog volume with the given name.\n\n"
            "Voxels inside particles will have value one, and voxels outside"
            " will have value zero.  Within a narrow band centered on particle surfaces,"
            " voxel values will vary linearly from zero to one."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "buildinteriormask", "")
        .setTypeExtended(PRM_TYPE_TOGGLE_JOIN));

    parms.add(hutil::ParmFactory(PRM_STRING, "interiormaskname", "Interior Mask VDB")
        .setDefault("mask")
        .setTooltip("A name for the output interior mask volume")
        .setDocumentation(
            "If enabled, output an interior mask volume with the given name.\n\n"
            "Voxels inside particles will be active, and voxels outside will be inactive."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "buildmask", "")
        .setTypeExtended(PRM_TYPE_TOGGLE_JOIN));

    parms.add(hutil::ParmFactory(PRM_STRING, "maskname", "Bounding Mask VDB")
        .setDefault("boundingvolume")
        .setTooltip("A name for the output bounding mask volume")
        .setDocumentation(
            "If enabled, output an alpha mask volume with the given name.\n\n"
            "The alpha mask is a fog volume derived from the CSG difference"
            " between a level set surface with a maximum radius of the particles"
            " and a level set surface with a minimum radius of the particles."
            " This mask can be used to constrain level set smoothing so as to"
            " prevent surface details from being completely smoothed away."));

    parms.add(hutil::ParmFactory(PRM_FLT_J, "boundinglimit", "Bounding Limit")
        .setDefault(0.25)
        .setRange(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_RESTRICTED, 1)
        .setTooltip(
            "Fraction by which to increase the maximum and decrease the minimum"
            " particle radii used to define the limit surfaces for the alpha mask"));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "usereferencevdb", "")
        .setDefault(PRMoneDefaults)
        .setTypeExtended(PRM_TYPE_TOGGLE_JOIN));

    parms.add(hutil::ParmFactory(PRM_STRING, "referencevdb", "Reference VDB")
        .setChoiceList(&hutil::PrimGroupMenuInput2)
        .setDocumentation(
            "If enabled, give output volumes the same orientation and voxel size as"
            " the selected VDB (see [specifying volumes|/model/volumes#group])"
            " and match the narrow-band width if the reference VDB is a level set."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "merge", "Merge with Reference VDB")
        .setDocumentation(
            "If a reference VDB is provided, union the new particles into it.\n\n"
            "This allows one to use the particles to specify only the surface detail"
            " and use a coarse, offset volume for the main bulk."));

    parms.add(hutil::ParmFactory(PRM_FLT_J, "voxelsize", "Voxel Size")
        .setDefault(PRMpointOneDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 1e-5, PRM_RANGE_UI, 5)
        .setDocumentation(
            "The desired voxel size in world units (smaller corresponds to higher resolution)\n\n"
            "Particles smaller than the voxel size will not be represented in the output VDB."));

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "useworldspace", "Use World Space for Band")
        .setCallbackFunc(&SOP_OpenVDB_From_Particles::convertUnitsCB)
        .setTooltip(
            "If enabled, specify the narrow-band width in world units, otherwise in voxels."));

    parms.add(hutil::ParmFactory(PRM_FLT_J, "halfbandvoxels", "Half-Band Voxels")
        .setDefault(PRMthreeDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 1, PRM_RANGE_UI, 10)
        .setTooltip(
            "Half the width of the narrow band in voxels\n"
            "Many level set operations require a minimum of three voxels.")
        .setDocumentation(nullptr));

    parms.add(hutil::ParmFactory(PRM_FLT_J, "halfband", "Half-Band")
        .setDefault(PRMoneDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 1e-5, PRM_RANGE_UI, 10)
        .setDocumentation("Half the width of the narrow band in world units"));


    parms.beginExclusiveSwitcher("conversion", "Conversion");
    parms.addFolder("Spheres");

    parms.add(hutil::ParmFactory(PRM_FLT_J, "particlescale", "Particle Scale")
        .setDefault(PRMoneDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 0.0, PRM_RANGE_UI, 2.0)
        .setDocumentation(
            "Multiplier for the `pscale` point attribute, which defines"
            " the world space particle radius\n\n"
            "If the `pscale` attribute is missing, it is assumed to have a value of one."));

    parms.add(hutil::ParmFactory(PRM_FLT_J, "minradius", "Minimum Radius")
        .setDefault(1.5)
        .setRange(PRM_RANGE_RESTRICTED, 0.0, PRM_RANGE_UI, 2.0)
        .setTooltip(
            "Minimum radius in voxel units after scaling\n\n"
            "Particles smaller than this limit are ignored.\n"
            "Particles with radius smaller than 1.5 voxels will likely cause"
            " aliasing artifacts, so this should not be set lower than 1.5."));

     parms.add(hutil::ParmFactory(PRM_TOGGLE, "velocitytrails", "Velocity Trails")
        .setTooltip(
            "Generate multiple spheres for each particle, trailing off"
            " in the direction of the particle's velocity attribute.")
        .setDocumentation(
            "Generate multiple spheres for each particle, trailing off"
            " in the direction opposite the particle's velocity attribute.\n\n"
            "The velocity attribute must be named `v` and be of type 3flt."));

     parms.add(hutil::ParmFactory(PRM_FLT_J, "velocityscale", "Velocity Scale")
        .setDefault(PRMoneDefaults)
        .setRange(PRM_RANGE_UI, 0.0, PRM_RANGE_UI, 1.0)
        .setTooltip(
            "When velocity trails are enabled, scale the lengths of the trails by this amount."));

    parms.add(hutil::ParmFactory(PRM_FLT_J, "trailresolution", "Trail Resolution")
        .setDefault(PRMoneDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 0.2, PRM_RANGE_UI, 2.0)
        .setTooltip(
            "When velocity trails are enabled, separate the component spheres"
            " of each trail by this distance.\n\n"
            "Use this parameter to control aliasing and limit the number"
            " of particle instances."));

    hutil::ParmList transferParms;

    transferParms.add(hutil::ParmFactory(PRM_STRING, "attribute#", "Attribute")
        .setChoiceList(&SOP_OpenVDB_From_Particles::sPointAttrMenu)
        .setSpareData(&SOP_Node::theFirstInput)
        .setTooltip(
            "A point attribute from which to create a VDB\n\n"
            "Integer and floating-point attributes of arbitrary precision"
            " and tuple size are supported."));

    transferParms.add(hutil::ParmFactory(PRM_STRING, "attributeGridName#", "VDB Name")
        .setTooltip("The name for this VDB (leave blank to use the attribute's name)"));

    {
        std::vector<std::string> items;
        for (int i = 0; i < openvdb::NUM_VEC_TYPES ; ++i) {
            items.push_back(openvdb::GridBase::vecTypeToString(openvdb::VecType(i)));
            items.push_back(openvdb::GridBase::vecTypeExamples(openvdb::VecType(i)));
        }

        transferParms.add(hutil::ParmFactory(PRM_ORD, "vecType#", "Vector Type")
            .setDefault(PRMzeroDefaults)
            .setChoiceListItems(PRM_CHOICELIST_SINGLE, items)
            .setTooltip("How vector values should be interpreted"));
    }

    parms.add(hutil::ParmFactory(PRM_TOGGLE, "buildattrs", "Transfer Attributes")
        .setDefault(PRMoneDefaults)
        .setTooltip(
            "Generate additional VDBs that store the values of point attributes.")
        .setDocumentation(
            "Generate additional VDBs that store the values of point"
            " [attributes|/model/attributes].\n\n"
            "When __Interior Mask VDB__ output is selected, attribute values will be set"
            " for voxels inside particles.  Otherwise, attribute values will be set"
            " only for voxels in the narrow band around particle surfaces."));

    parms.add(hutil::ParmFactory(PRM_MULTITYPE_LIST, "attrList", "Attributes")
        .setMultiparms(transferParms)
        .setDefault(PRMzeroDefaults));


    parms.addFolder("Points");

    parms.add(hutil::ParmFactory(PRM_INT_J, "dilation", "Dilation")
        .setDefault(PRMoneDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_UI, 10)
        .setTooltip(
            "Number of morphological dilation iterations "
            "used to expand the active voxel region"));

    parms.add(hutil::ParmFactory(PRM_INT_J, "closing", "Closing")
        .setDefault(PRMoneDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_UI, 10)
        .setTooltip(
            "Number of morphological closing iterations "
            "used to fill gaps in the active voxel region"));

    parms.add(hutil::ParmFactory(PRM_INT_J, "smoothing", "Smoothing")
        .setRange(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_UI, 10)
        .setTooltip("Number of surface smoothing iterations"));

    parms.endSwitcher();


    hutil::ParmList obsoleteParms;
    obsoleteParms.add(hutil::ParmFactory(PRM_HEADING, "optionsHeading", "Options"));
    obsoleteParms.add(hutil::ParmFactory(PRM_FLT_J, "gradientWidth",  "Gradient width"));
    obsoleteParms.add(hutil::ParmFactory(PRM_TOGGLE, "customGradientWidth", ""));
    obsoleteParms.add(hutil::ParmFactory(PRM_STRING, "ptnIndexGridName", ""));
    obsoleteParms.add(hutil::ParmFactory(PRM_TOGGLE, "ptnIndexGrid", ""));
    obsoleteParms.add(hutil::ParmFactory(PRM_HEADING, "transformHeading", "Transform"));
    obsoleteParms.add(hutil::ParmFactory(PRM_HEADING, "outputHeading", "Output grids"));
    obsoleteParms.add(hutil::ParmFactory(PRM_TOGGLE, "threading", "Threading"));
    obsoleteParms.add(hutil::ParmFactory(PRM_TOGGLE, "matchlevelset", ""));
    obsoleteParms.add(hutil::ParmFactory(PRM_TOGGLE, "levelSet", "").setDefault(PRMoneDefaults));
    obsoleteParms.add(hutil::ParmFactory(PRM_STRING, "gridName", "").setDefault("surface"));
    obsoleteParms.add(hutil::ParmFactory(PRM_STRING, "fogVolumeGridName", "")
        .setDefault("density"));
    obsoleteParms.add(hutil::ParmFactory(PRM_TOGGLE, "fogVolume", ""));
    obsoleteParms.add(hutil::ParmFactory(PRM_TOGGLE, "maskVolume", ""));
    obsoleteParms.add(hutil::ParmFactory(PRM_STRING, "maskVolumeGridName", "").setDefault("mask"));
    obsoleteParms.add(hutil::ParmFactory(PRM_FLT_J, "maskWidth", "").setDefault(0.25));
    obsoleteParms.add(hutil::ParmFactory(PRM_STRING, "group", ""));
    obsoleteParms.add(hutil::ParmFactory(PRM_TOGGLE, "writeintoref", ""));
    obsoleteParms.add(hutil::ParmFactory(PRM_FLT_J, "bandWidth", "").setDefault(PRMthreeDefaults));
    obsoleteParms.add(hutil::ParmFactory(PRM_FLT_J, "bandWidthWS", "").setDefault(PRMoneDefaults));
    obsoleteParms.add(hutil::ParmFactory(PRM_TOGGLE, "worldSpaceUnits", ""));
    obsoleteParms.add(hutil::ParmFactory(PRM_FLT_J, "maxWidth", "Max Half-width"));
    obsoleteParms.add(hutil::ParmFactory(PRM_FLT_J, "Rmax", "Max Radius In Voxels"));
    obsoleteParms.add(hutil::ParmFactory(PRM_FLT_J, "voxelSize", "")
        .setDefault(PRMpointOneDefaults));
    obsoleteParms.add(hutil::ParmFactory(PRM_FLT_J, "dR", "").setDefault(PRMoneDefaults));
    obsoleteParms.add(hutil::ParmFactory(PRM_FLT_J, "Rmin", "").setDefault(1.5));
    obsoleteParms.add(hutil::ParmFactory(PRM_FLT_J, "dV", "").setDefault(PRMoneDefaults));
    obsoleteParms.add(hutil::ParmFactory(PRM_FLT_J, "dX", "").setDefault(PRMoneDefaults));
    obsoleteParms.add(hutil::ParmFactory(PRM_HEADING, "transferHeading", "Attribute transfer"));
    obsoleteParms.add(hutil::ParmFactory(PRM_HEADING, "particleHeading", "Conversion settings"));
    obsoleteParms.add(hutil::ParmFactory(PRM_TOGGLE, "prune", "Prune Level Set"));
    obsoleteParms.add(hutil::ParmFactory(PRM_ORD, "footprint", ""));
    // temporary parameters used in the 3.2 beta
    obsoleteParms.add(hutil::ParmFactory(PRM_INT_J, "dilate", "").setDefault(PRMoneDefaults));
    obsoleteParms.add(hutil::ParmFactory(PRM_INT_J, "erode", "").setDefault(PRMoneDefaults));
    obsoleteParms.add(hutil::ParmFactory(PRM_TOGGLE, "distancevdb", "")
        .setDefault(PRMoneDefaults));
    obsoleteParms.add(hutil::ParmFactory(PRM_STRING, "distancevdbname", "").setDefault("surface"));
    obsoleteParms.add(hutil::ParmFactory(PRM_TOGGLE, "fogvdb", ""));
    obsoleteParms.add(hutil::ParmFactory(PRM_STRING, "fogvdbname", "").setDefault("density"));
    obsoleteParms.add(hutil::ParmFactory(PRM_TOGGLE, "maskvdb", ""));
    obsoleteParms.add(hutil::ParmFactory(PRM_STRING, "maskvdbname", "")
        .setDefault("boundingvolume"));

    /// @todo obsoleteAttrParms


    hvdb::OpenVDBOpFactory("VDB from Particles",
        SOP_OpenVDB_From_Particles::factory, parms, *table)
#ifndef SESI_OPENVDB
        .setInternalName("DW_OpenVDBFromParticles")
#endif
        .addInput("Points to convert")
        .addOptionalInput("Optional reference VDB")
        .setObsoleteParms(obsoleteParms)
        .setVerb(SOP_NodeVerb::COOK_GENERATOR,
            []() { return new SOP_OpenVDB_From_Particles::Cache; })
        .setDocumentation("\
#icon: COMMON/openvdb\n\
#tags: vdb\n\
\n\
\"\"\"Convert point clouds and/or point attributes into VDB volumes.\"\"\"\n\
\n\
@overview\n\
\n\
This node can create signed distance fields, density fields (\"fog volumes\"),\n\
and/or boolean mask volumes from point clouds, optionally treating each point\n\
as a sphere whose radius is given by its point scale attribute (`pscale`).\n\
\n\
Since the resulting VDB volumes store only the voxels around each point,\n\
they can have a much a higher effective resolution than a traditional\n\
Houdini volume.\n\
\n\
NOTE:\n\
    The `pscale` attribute is set by the [Attribute|Node:pop/attribute] POP\n\
    or the [Point|Node:sop/point] SOP.\n\
\n\
    Points smaller than 1.5 voxels cannot be resolved and will not appear in output VDBs.\n\
\n\
    The __Particle Scale__ parameter applies uniform scaling to all spheres.\n\
\n\
Connect a VDB to the second input to transfer that VDB's orientation and voxel size\n\
to the output VDBs (see the __Reference VDB__ parameter), and optionally to merge\n\
that VDB's contents into the output VDBs.\n\
\n\
@related\n\
- [Node:sop/scatter]\n\
- [OpenVDB Create|Node:sop/DW_OpenVDBCreate]\n\
- [OpenVDB From Polygons|Node:sop/DW_OpenVDBFromPolygons]\n\
- [Node:sop/isooffset]\n\
- [Node:sop/vdbfromparticles]\n\
\n\
@examples\n\
\n\
See [openvdb.org|http://www.openvdb.org/download/] for source code\n\
and usage examples.\n");
}


////////////////////////////////////////


void
SOP_OpenVDB_From_Particles::resolveObsoleteParms(PRM_ParmList* obsoleteParms)
{
    if (!obsoleteParms || obsoleteParms->allDefaults() != 0) return;

    PRM_Parm* parm = obsoleteParms->getParmPtr("footprint");
    if (parm && !parm->isFactoryDefault()) {
        setInt("velocitytrails", 0, 0.0, 1);
    }

    resolveRenamedParm(*obsoleteParms, "dR", "particlescale");
    resolveRenamedParm(*obsoleteParms, "Rmin", "minradius");
    resolveRenamedParm(*obsoleteParms, "dV", "velocityscale");
    resolveRenamedParm(*obsoleteParms, "dX", "trailresolution");
    resolveRenamedParm(*obsoleteParms, "voxelSize", "voxelsize");
    resolveRenamedParm(*obsoleteParms, "maskWidth", "boundinglimit");
    resolveRenamedParm(*obsoleteParms, "bandWidth", "halfbandvoxels");
    resolveRenamedParm(*obsoleteParms, "bandWidthWS", "halfband");
    resolveRenamedParm(*obsoleteParms, "levelSet", "builddistance");
    resolveRenamedParm(*obsoleteParms, "fogVolume", "buildfog");
    resolveRenamedParm(*obsoleteParms, "maskVolume", "buildmask");
    resolveRenamedParm(*obsoleteParms, "writeintoref", "merge");
    resolveRenamedParm(*obsoleteParms, "worldSpaceUnits", "useworldspace");
    resolveRenamedParm(*obsoleteParms, "gridName", "distancename");
    resolveRenamedParm(*obsoleteParms, "fogVolumeGridName", "fogname");
    resolveRenamedParm(*obsoleteParms, "maskVolumeGridName", "maskname");
    resolveRenamedParm(*obsoleteParms, "group", "referencevdb");

    resolveRenamedParm(*obsoleteParms, "distancevdb", "builddistance");
    resolveRenamedParm(*obsoleteParms, "distancevdbname", "distancename");
    resolveRenamedParm(*obsoleteParms, "fogvdb", "buildfog");
    resolveRenamedParm(*obsoleteParms, "fogvdbname", "fogname");
    resolveRenamedParm(*obsoleteParms, "maskvdb", "buildmask");
    resolveRenamedParm(*obsoleteParms, "maskvdbname", "maskname");

    // Delegate to the base class.
    hvdb::SOP_NodeVDB::resolveObsoleteParms(obsoleteParms);
}


// Enable or disable parameters in the UI.
bool
SOP_OpenVDB_From_Particles::updateParmsFlags()
{
    bool changed = false;

    changed |= enableParm("distancename", bool(evalInt("builddistance", 0, 0)));
    changed |= enableParm("fogname", bool(evalInt("buildfog", 0, 0)));
    changed |= enableParm("interiormaskname", bool(evalInt("buildinteriormask", 0, 0)));

    bool useMask = evalInt("buildmask", 0, 0) == 1;
    changed |= enableParm("boundinglimit", useMask);
    changed |= enableParm("maskname", useMask);

    bool useRef = ((this->nInputs() == 2) && (0 != evalInt("usereferencevdb", 0, 0)));
    changed |= enableParm("referencevdb", useRef);
    changed |= enableParm("merge", useRef);
    changed |= enableParm("voxelsize", !useRef);

    bool useWSUnits = bool(evalInt("useworldspace", 0, 0));
    changed |= setVisibleState("halfbandvoxels", !useWSUnits);
    changed |= setVisibleState("halfband", useWSUnits);

    bool useTrails =  evalInt("velocitytrails", 0, 0) == 1;
    changed |= enableParm("trailresolution", useTrails);
    changed |= enableParm("velocityscale", useTrails);

    changed |= enableParm("attrList", bool(evalInt("buildattrs", 0, 0)));

    // enable / disable vector type menu
    UT_String attrName;
    GA_ROAttributeRef attrRef;
    const GU_Detail* ptGeo = this->getInputLastGeo(0, CHgetEvalTime());
    if (ptGeo) {
        for (int i = 1, N = static_cast<int>(evalInt("attrList", 0, 0)); i <= N; ++i) {

            evalStringInst("attribute#", &i, attrName, 0, 0);
            bool isVector = false;

            if (attrName.length() != 0) {

                attrRef = ptGeo->findPointAttribute(attrName);
                if (attrRef.isValid()) {

                    const GA_Attribute *attr = attrRef.getAttribute();
                    if (attr) {
                        const GA_TypeInfo typeInfo = attr->getTypeInfo();
                        isVector = (typeInfo == GA_TYPE_HPOINT || typeInfo == GA_TYPE_POINT
                            || typeInfo == GA_TYPE_VECTOR || typeInfo == GA_TYPE_NORMAL);

                        if (!isVector) {
                            const GA_AIFTuple *tupleAIF = attr->getAIFTuple();
                            if (tupleAIF) isVector = tupleAIF->getTupleSize(attr) == 3;
                        }
                    }
                }
            }
            changed |= enableParmInst("vecType#", &i, isVector);
            changed |= setVisibleStateInst("vecType#", &i, isVector);
        }
    }

    return changed;
}


// Callback to convert from voxel to world-space units
int
SOP_OpenVDB_From_Particles::convertUnits()
{
    const fpreal time = CHgetEvalTime();

    float voxSize = 0.1f;
    // Attempt to extract the voxel size from our cache.
    if (const auto* cache = dynamic_cast<SOP_OpenVDB_From_Particles::Cache*>(myNodeVerbCache)) {
        voxSize = cache->voxelSize();
    }

    if (evalInt("useworldspace", 0, time) != 0) {
        setFloat("halfband", 0, time, evalFloat("halfbandvoxels", 0, time) * voxSize);
    } else {
        setFloat("halfbandvoxels", 0, time, evalFloat("halfband", 0, time) / voxSize);
    }

    return 1;
}


// This implementation differs somewhat from ParmFactory::setAttrChoiceList().
void
SOP_OpenVDB_From_Particles::buildAttrMenu(void* data, PRM_Name* entries, int maxEntries,
    const PRM_SpareData* spare, const PRM_Parm*)
{
    if (!data || !entries || !spare) return;

    SOP_Node* sop = CAST_SOPNODE(static_cast<OP_Node*>(data));

    if (sop == nullptr) {
        // terminate and quit
        entries[0].setToken(0);
        entries[0].setLabel(0);
        return;
    }

    const int inputIndex = [&]() {
        const char* s = spare->getValue("sop_input");
        return s ? std::atoi(s) : 0;
    }();
    const GU_Detail* gdp = sop->getInputLastGeo(inputIndex, CHgetEvalTime());

    size_t menuIdx = 0, menuEnd(maxEntries - 2);

    // null object
    entries[menuIdx].setToken("0");
    entries[menuIdx++].setLabel("- no attribute selected -");

    if (gdp) {
        // point attribute names
        GA_AttributeDict::iterator iter = gdp->pointAttribs().begin(GA_SCOPE_PUBLIC);
        if (!iter.atEnd() && menuIdx != menuEnd) {
            if (menuIdx > 0) {
                entries[menuIdx].setToken(PRM_Name::mySeparator);
                entries[menuIdx++].setLabel(PRM_Name::mySeparator);
            }
            for ( ; !iter.atEnd() && menuIdx != menuEnd; ++iter) {
                std::ostringstream token;
                token << (*iter)->getName();

                entries[menuIdx].setToken(token.str().c_str());
                entries[menuIdx++].setLabel(token.str().c_str());
            }
            // Special case
            entries[menuIdx].setToken("point_list_index");
            entries[menuIdx++].setLabel("point_list_index");
        }
    }

    // terminator
    entries[menuIdx].setToken(0);
    entries[menuIdx].setLabel(0);
}


////////////////////////////////////////


namespace {

// This class implements the particle access interface required by
// openvdb::tools::ParticlesToLevelSet.
class ParticleList
{
public:
    using Real = openvdb::Real;
    using PosType = openvdb::Vec3R; // required by openvdb::tools::PointPartitioner

    ParticleList(const GU_Detail* gdp, Real radiusMult = 1, Real velocityMult = 1)
        : mGdp(gdp)
        , mScaleHandle(gdp, GA_ATTRIB_POINT, GEO_STD_ATTRIB_PSCALE)
        , mVelHandle(gdp, GA_ATTRIB_POINT, GEO_STD_ATTRIB_VELOCITY)
        , mHasRadius(mScaleHandle.isValid())
        , mHasVelocity(mVelHandle.isValid())
        , mRadiusMult(radiusMult)
        , mVelocityMult(velocityMult)
    {
    }

    // Do the particles have non-constant radius
    bool hasRadius() const { return mHasRadius;}

    // Do the particles have velocity
    bool hasVelocity() const { return mHasVelocity;}

    // Multiplier for the radius
    Real radiusMult() const { return mRadiusMult; }
    void setRadiusMult(Real mult) { mRadiusMult = mult; }

    // The public methods below are the only ones required
    // by tools::ParticlesToLevelSet

    size_t size() const { return mGdp->getNumPoints(); }

    // Position of particle in world space
    // This is required by ParticlesToLevelSet::rasterizeSpheres(*this,radius)
    void getPos(size_t n, PosType& xyz) const
    {
        const UT_Vector3 p = mGdp->getPos3(mGdp->pointOffset(n));
        xyz[0] = p[0], xyz[1] = p[1], xyz[2] = p[2];
    }

    // Position and radius of particle in world space
    // This is required by ParticlesToLevelSet::rasterizeSpheres(*this)
    void getPosRad(size_t n, PosType& xyz, Real& rad) const
    {
        UT_ASSERT(mHasRadius);
        const GA_Offset m = mGdp->pointOffset(n);
        const UT_Vector3 p = mGdp->getPos3(m);
        xyz[0] = p[0], xyz[1] = p[1], xyz[2] = p[2];
        rad = mRadiusMult*mScaleHandle.get(m);
    }

    // Position, radius and velocity of particle in world space
    // This is required by ParticlesToLevelSet::rasterizeTrails
    void getPosRadVel(size_t n, PosType& xyz, Real& rad, PosType& vel) const
    {
        UT_ASSERT(mHasVelocity);
        const GA_Offset m = mGdp->pointOffset(n);
        const UT_Vector3 p = mGdp->getPos3(m);
        xyz[0] = p[0], xyz[1] = p[1], xyz[2] = p[2];
        rad = mHasRadius ? mRadiusMult*mScaleHandle.get(m) : mRadiusMult;
        const UT_Vector3 v = mVelHandle.get(m);
        vel[0] = mVelocityMult*v[0], vel[1] = mVelocityMult*v[1], vel[2] = mVelocityMult*v[2];
    }

    // Required for attribute transfer
    void getAtt(size_t n, openvdb::Int32& att) const { att = openvdb::Int32(n); }

protected:
    const GU_Detail* mGdp;
    GA_ROHandleF     mScaleHandle;
    GA_ROHandleV3    mVelHandle;
    const bool       mHasRadius, mHasVelocity;
    Real             mRadiusMult; // multiplier for radius
    const Real       mVelocityMult; // multiplier for velocity
}; // class ParticleList


////////////////////////////////////////


template<class ValueType>
inline void
addAttributeDetails(
    hvdb::AttributeDetailList& attributeList,
    const GA_Attribute* attribute,
    const GA_AIFTuple* tupleAIF,
    const int attrTupleSize,
    const openvdb::Int32Grid& closestPtIdxGrid,
    std::string& customName,
    int vecType = -1)
{
    // Defines a new type of a tree having the same hierarchy as the incoming
    // Int32Grid's tree but potentially a different value type.
    using TreeType = typename openvdb::Int32Grid::TreeType::ValueConverter<ValueType>::Type;
    using GridType = typename openvdb::Grid<TreeType>;

    if (vecType != -1) { // Vector grid
         // Get the attribute's default value.
         ValueType defValue =
             hvdb::evalAttrDefault<ValueType>(tupleAIF->getDefaults(attribute), 0);

        // Construct a new tree that matches the closestPtIdxGrid's active voxel topology.
        typename TreeType::Ptr tree(
            new TreeType(closestPtIdxGrid.tree(), defValue, openvdb::TopologyCopy()));
        typename GridType::Ptr grid(GridType::create(tree));

        grid->setVectorType(openvdb::VecType(vecType));

        attributeList.push_back(hvdb::AttributeDetailBase::Ptr(
            new hvdb::AttributeDetail<GridType>(grid, attribute, tupleAIF, 0, true)));

        if (customName.size() > 0) {
            attributeList[attributeList.size()-1]->name() = customName;
        }

    } else {
        for (int c = 0; c < attrTupleSize; ++c) {
            // Get the attribute's default value.
            ValueType defValue =
                hvdb::evalAttrDefault<ValueType>(tupleAIF->getDefaults(attribute), c);

            // Construct a new tree that matches the closestPtIdxGrid's active voxel topology.
            typename TreeType::Ptr tree(
                new TreeType(closestPtIdxGrid.tree(), defValue, openvdb::TopologyCopy()));
            typename GridType::Ptr grid(GridType::create(tree));

            attributeList.push_back(hvdb::AttributeDetailBase::Ptr(
                new hvdb::AttributeDetail<GridType>(grid, attribute, tupleAIF, c)));

            if (customName.size() > 0) {
                std::ostringstream name;
                name << customName;
                if(attrTupleSize != 1) name << "_" << c;

                attributeList[attributeList.size()-1]->name() = name.str();
            }
        }
    }
}


inline void
transferAttributes(
    hvdb::AttributeDetailList& pointAttributes,
    const openvdb::Int32Grid& closestPtIdxGrid,
    openvdb::math::Transform::Ptr transform,
    const GU_Detail& ptGeo,
    GU_Detail& outputGeo)
{
    // Threaded attribute transfer.
    hvdb::PointAttrTransfer transferOp(pointAttributes, closestPtIdxGrid, ptGeo);
    transferOp.runParallel();

    // Construct and add VDB primitives to the gdp
    for (size_t i = 0, N = pointAttributes.size(); i < N; ++i) {
        hvdb::AttributeDetailBase::Ptr& attrDetail = pointAttributes[i];
        std::ostringstream gridName;
        gridName << attrDetail->name();
        attrDetail->grid()->setTransform(transform);
        hvdb::createVdbPrimitive(outputGeo, attrDetail->grid(), gridName.str().c_str());
    }
}


template<typename AttrT, typename GridT>
inline openvdb::Int32Grid::Ptr
convertImpl(
    const ParticleList& paList,
    GridT& outGrid,
    float minRadius,
    float maxRadius,
    bool velocityTrails,
    float trailRes,
    hvdb::Interrupter& boss,
    size_t& numTooSmall,
    size_t& numTooLarge)
{
    openvdb::tools::ParticlesToLevelSet<GridT, AttrT, hvdb::Interrupter> raster(outGrid, &boss);

    raster.setRmin(minRadius);
    raster.setRmax(maxRadius);

    if (velocityTrails) {
        raster.rasterizeTrails(paList, trailRes);
    } else if (paList.hasRadius()) {
        raster.rasterizeSpheres(paList);
    } else {
        raster.rasterizeSpheres(paList, paList.radiusMult());
    }
    // Always prune to produce a valid narrow-band level set.
    raster.finalize(/*prune=*/true);

    numTooSmall = raster.getMinCount();
    numTooLarge = raster.getMaxCount();

    return openvdb::gridPtrCast<openvdb::Int32Grid>(raster.attributeGrid());
}


inline std::string
getIgnoredParticleWarning(size_t numTooSmall, size_t numTooLarge)
{
    std::string mesg;
    if (numTooSmall || numTooLarge) {
        std::ostringstream ostr;
        ostr << "Ignored ";
        if (numTooSmall) { ostr << numTooSmall << " small"; }
        if (numTooSmall && numTooLarge) { ostr << " and "; }
        if (numTooLarge) { ostr << numTooLarge << " large"; }
        ostr << " particles (hint: change Minimum Radius in Voxels)";
        mesg = ostr.str();
    }
    return mesg;
}

} // anonymous namespace


void
SOP_OpenVDB_From_Particles::Cache::convert(
    fpreal time,
    ParticleList& paList,
    openvdb::FloatGrid::Ptr sdfGrid,
    openvdb::BoolGrid::Ptr maskGrid,
    hvdb::Interrupter& boss)
{
    using NoAttrs = void;

    const bool velocityTrails = paList.hasVelocity() && (0 != evalInt("velocitytrails", 0, time));
    const float
        minRadius = float(evalFloat("minradius", 0, time)),
        maxRadius = 1e15f,
        trailRes = (!velocityTrails ? 1.f : float(evalFloat("trailresolution", 0, time)));

    size_t numTooSmall = 0, numTooLarge = 0;

    if (sdfGrid) {
        convertImpl<NoAttrs>(paList, *sdfGrid, minRadius, maxRadius,
            velocityTrails, trailRes, boss, numTooSmall, numTooLarge);
    }
    if (maskGrid) {
        convertImpl<NoAttrs>(paList, *maskGrid, minRadius, maxRadius,
            velocityTrails, trailRes, boss, numTooSmall, numTooLarge);
    }
    {
        const auto mesg = getIgnoredParticleWarning(numTooSmall, numTooLarge);
        if (!mesg.empty()) { addWarning(SOP_MESSAGE, mesg.c_str()); }
    }
}


void
SOP_OpenVDB_From_Particles::Cache::convertWithAttributes(
    fpreal time,
    const GU_Detail& ptGeo,
    ParticleList& paList,
    openvdb::FloatGrid::Ptr sdfGrid,
    openvdb::BoolGrid::Ptr maskGrid,
    hvdb::Interrupter& boss)
{
    const bool velocityTrails = paList.hasVelocity() && (0 != evalInt("velocitytrails", 0, time));
    const float
        minRadius = float(evalFloat("minradius", 0, time)),
        maxRadius = 1e15f,
        trailRes = (!velocityTrails ? 1.f : float(evalFloat("trailresolution", 0, time)));

    openvdb::Int32Grid::Ptr closestPtIdxGrid;
    size_t numTooSmall = 0, numTooLarge = 0;

    if (sdfGrid) {
        closestPtIdxGrid = convertImpl<openvdb::Int32>(paList, *sdfGrid, minRadius, maxRadius,
            velocityTrails, trailRes, boss, numTooSmall, numTooLarge);
    }
    if (maskGrid) {
        if (closestPtIdxGrid) {
            // For backward compatibility, the index grid associated with the SDF
            // takes precedence over one associated with the mask.
            using NoAttrs = void;
            convertImpl<NoAttrs>(paList, *maskGrid, minRadius, maxRadius,
                velocityTrails, trailRes, boss, numTooSmall, numTooLarge);
        } else {
            closestPtIdxGrid = convertImpl<openvdb::Int32>(paList, *maskGrid,
                minRadius, maxRadius, velocityTrails, trailRes, boss, numTooSmall, numTooLarge);
        }
    }
    {
        const auto mesg = getIgnoredParticleWarning(numTooSmall, numTooLarge);
        if (!mesg.empty()) { addWarning(SOP_MESSAGE, mesg.c_str()); }
    }

    if (!closestPtIdxGrid || boss.wasInterrupted()) return;

    // Transfer point attributes.
    if ((0 != evalInt("buildattrs", 0, time)) && (evalInt("attrList", 0, time) > 0)) {
        hvdb::AttributeDetailList pointAttributes;

        int closestPointIndexInstance =
            constructGenericAtttributeList(time, pointAttributes, ptGeo, *closestPtIdxGrid);

        auto transform = (sdfGrid ? sdfGrid->transformPtr() : maskGrid->transformPtr());
        transferAttributes(pointAttributes, *closestPtIdxGrid, transform, ptGeo, *gdp);

        // Export the closest-point index grid.
        if (closestPointIndexInstance > -1) {
            UT_String gridNameStr;
            evalStringInst("attributeGridName#", &closestPointIndexInstance, gridNameStr, 0, time);
            if (gridNameStr.length() == 0) gridNameStr = "point_list_index";
            hvdb::createVdbPrimitive(*gdp, closestPtIdxGrid, gridNameStr.toStdString().c_str());
        }
    }
}


// Helper method for point attribute transfer
int
SOP_OpenVDB_From_Particles::Cache::constructGenericAtttributeList(
    fpreal time,
    hvdb::AttributeDetailList &pointAttributes,
    const GU_Detail& ptGeo,
    const openvdb::Int32Grid& closestPtIdxGrid)
{
    UT_String attrName;
    GA_ROAttributeRef attrRef;
    GA_Range range;
    int closestPointIndexInstance = -1;

    // for each selected attribute
    for (int i = 1, N = static_cast<int>(evalInt("attrList", 0, time)); i <= N; ++i) {

        evalStringInst("attribute#", &i, attrName, 0, time);

        if (attrName.length() == 0) continue;

        if (attrName == "point_list_index") {
            // The closest point index grid is a special case,
            // the converter has already generated it for us.
            closestPointIndexInstance = i;
            continue;
        }

        attrRef = ptGeo.findPointAttribute(attrName);

        if (!attrRef.isValid()) {
            std::ostringstream ostr;
            ostr << "Skipped unrecognized attribute: '"<< attrName << "'";
            addWarning(SOP_MESSAGE, ostr.str().c_str());
            continue;
        }

        evalStringInst("attributeGridName#", &i, attrName, 0, time);
        std::string customName = attrName.toStdString();

        int vecType = static_cast<int>(evalIntInst("vecType#", &i, 0, time));

        const GA_Attribute *attr = attrRef.getAttribute();
        if (!attr) {
            addWarning(SOP_MESSAGE, "Skipped unrecognized attribute type");
            continue;
        }

        const GA_AIFTuple *tupleAIF = attr->getAIFTuple();
        if (!tupleAIF) {
            addWarning(SOP_MESSAGE, "Skipped unrecognized attribute type");
            continue;
        }

        const GA_Storage attrStorage = tupleAIF->getStorage(attr);
        const int attrTupleSize = tupleAIF->getTupleSize(attr);
        const GA_TypeInfo typeInfo = attr->getTypeInfo();

        const bool interpertAsVector = (typeInfo == GA_TYPE_HPOINT || typeInfo == GA_TYPE_POINT
            || typeInfo == GA_TYPE_VECTOR || typeInfo == GA_TYPE_NORMAL);

        switch (attrStorage)
        {
            case GA_STORE_INT16:
            case GA_STORE_INT32:
                if (interpertAsVector || attrTupleSize == 3) {
                    addAttributeDetails<openvdb::Vec3i>(pointAttributes, attr, tupleAIF,
                        attrTupleSize, closestPtIdxGrid, customName, vecType);
                } else {
                    addAttributeDetails<openvdb::Int32>(pointAttributes, attr, tupleAIF,
                        attrTupleSize, closestPtIdxGrid, customName);
                }

                break;
            case GA_STORE_INT64:
                addAttributeDetails<openvdb::Int64>(pointAttributes, attr, tupleAIF,
                    attrTupleSize, closestPtIdxGrid, customName);
                break;
            case GA_STORE_REAL16:
            case GA_STORE_REAL32:
                if (interpertAsVector || attrTupleSize == 3) {
                    addAttributeDetails<openvdb::Vec3s>(pointAttributes, attr, tupleAIF,
                        attrTupleSize, closestPtIdxGrid, customName, vecType);
                } else {
                    addAttributeDetails<float>(pointAttributes, attr, tupleAIF,
                        attrTupleSize, closestPtIdxGrid, customName);
                }

                break;
            case GA_STORE_REAL64:
                if (interpertAsVector || attrTupleSize == 3) {
                    addAttributeDetails<openvdb::Vec3d>(pointAttributes, attr, tupleAIF,
                        attrTupleSize, closestPtIdxGrid, customName, vecType);
                } else {
                    addAttributeDetails<double>(pointAttributes, attr, tupleAIF,
                        attrTupleSize, closestPtIdxGrid, customName);
                }

                break;
            default:
                addWarning(SOP_MESSAGE, "Skipped unrecognized attribute type");
                break;
        }
    }
    return closestPointIndexInstance;
}


////////////////////////////////////////


OP_ERROR
SOP_OpenVDB_From_Particles::Cache::cookVDBSop(OP_Context& context)
{
    try {
        hvdb::Interrupter boss("Creating VDBs from particles");

        const GU_Detail* ptGeo = inputGeo(0, context);
        const GU_Detail* refGeo = inputGeo(1, context);

        const fpreal time = context.getTime();
        mVoxelSize = float(evalFloat("voxelsize", 0, time));

        if (mVoxelSize < 1e-5) {
            std::ostringstream ostr;
            ostr << "The voxel size ("<< mVoxelSize << ") is too small.";
            addError(SOP_MESSAGE, ostr.str().c_str());
            return error();
        }

        const bool
            outputLevelSetGrid = (0 != evalInt("builddistance", 0, time)),
            outputFogVolumeGrid = (0 != evalInt("buildfog", 0, time)),
            outputInteriorMaskGrid = (0 != evalInt("buildinteriormask", 0, time)),
            outputBoundingMaskGrid = (0 != evalInt("buildmask", 0, time)),
            outputAttributeGrid =
                ((0 != evalInt("buildattrs", 0, time)) && (evalInt("attrList", 0, time) > 0)),
            needLeveLSet = (outputLevelSetGrid || outputFogVolumeGrid || outputBoundingMaskGrid
                || (outputAttributeGrid && !outputInteriorMaskGrid));

        if (!outputFogVolumeGrid && !outputLevelSetGrid
            && !outputAttributeGrid && !outputInteriorMaskGrid)
        {
             addWarning(SOP_MESSAGE, "No output selected");
             return error();
        }

        ParticleList paList(ptGeo,
            evalFloat("particlescale", 0, time), evalFloat("velocityscale", 0, time));

        float background = 0.0;
        if (evalInt("useworldspace", 0, time) != 0) {
            background = float(evalFloat("halfband", 0, time));
        } else {
            background = mVoxelSize * float(evalFloat("halfbandvoxels", 0, time));
        }
        auto transform = openvdb::math::Transform::createLinearTransform(mVoxelSize);

        openvdb::FloatGrid::Ptr sdfGrid;
        openvdb::BoolGrid::Ptr maskGrid;
        openvdb::MaskGrid::Ptr pointMaskGrid;

        // Optionally copy the reference grid and/or its transform.
        hvdb::GridCPtr refGrid;
        if (refGeo && (0 != evalInt("usereferencevdb", 0, time))) {
            const auto refName = evalStdString("referencevdb", time);
            hvdb::VdbPrimCIterator it(refGeo, matchGroup(*refGeo, refName));
            if (const hvdb::GU_PrimVDB* refPrim = (it ? *it : nullptr)) {
                refGrid = refPrim->getGridPtr();
            } else {
                addError(SOP_MESSAGE,
                    ("No reference VDB matching \"" + refName + "\" was found.").c_str());
                return error();
            }
        }
        if (refGrid) {
            transform = refGrid->transform().copy();
            mVoxelSize = static_cast<float>(transform->voxelSize()[0]);

            // Match the narrow band width.
            const bool isLevelSet = ((refGrid->getGridClass() == openvdb::GRID_LEVEL_SET)
                && refGrid->isType<openvdb::FloatGrid>());
            if (isLevelSet) {
                background = openvdb::gridConstPtrCast<openvdb::FloatGrid>(refGrid)->background();
                addMessage(SOP_MESSAGE, "Matching the reference level set's half-band width "
                    " and background value.  The Half Band setting will be ignored.");
            }

            if (evalInt("merge", 0, time) != 0) {
                if (needLeveLSet && isLevelSet) {
                    sdfGrid = openvdb::gridPtrCast<openvdb::FloatGrid>(refGrid->deepCopyGrid());
                }
                if (outputInteriorMaskGrid && refGrid->isType<openvdb::BoolGrid>()) {
                    maskGrid = openvdb::gridPtrCast<openvdb::BoolGrid>(refGrid->deepCopyGrid());
                }
                if (!sdfGrid && !maskGrid) {
                    if (needLeveLSet) {
                        addWarning(SOP_MESSAGE, "Can only merge with a level set reference VDB.");
                    } else {
                        addWarning(SOP_MESSAGE, "Can only merge with a boolean reference VDB.");
                    }
                }
            }
        }
        if (boss.wasInterrupted()) { return error(); }

        if (needLeveLSet) {
            if (!sdfGrid) { sdfGrid = openvdb::FloatGrid::create(background); }
            sdfGrid->setGridClass(openvdb::GRID_LEVEL_SET);
            sdfGrid->setTransform(transform);
        }
        if (outputInteriorMaskGrid) {
            if (!maskGrid) { maskGrid = openvdb::BoolGrid::create(); }
            maskGrid->setTransform(transform);
        }

        // Perform the particle conversion.

        const bool doSphereConversion = evalInt("conversion",  0, time) == 0;

        // Point topology conversion settings
        int dilation = static_cast<int>(evalInt("dilation", 0, time));
        int closing = static_cast<int>(evalInt("closing", 0, time));
        int smoothing = static_cast<int>(evalInt("smoothing", 0, time));
        int bandWidth = int(std::ceil(background / mVoxelSize));

        if (doSphereConversion) {
            if (evalInt("velocitytrails", 0, time) != 0 && !paList.hasVelocity()) {
                addWarning(SOP_MESSAGE,
                    "Velocity trails require a velocity point attribute"
                    " named 'v' of type 3fv.");
            }
            if (outputAttributeGrid) {
                this->convertWithAttributes(time, *ptGeo, paList, sdfGrid, maskGrid, boss);
            } else {
                this->convert(time, paList, sdfGrid, maskGrid, boss);
            }
        } else {
            pointMaskGrid = GUvdbCreatePointMaskGrid(*transform, *ptGeo);
            if (sdfGrid) {
                openvdb::FloatGrid::Ptr pointSdfGrid = openvdb::tools::topologyToLevelSet(
                    *pointMaskGrid, bandWidth, closing, dilation, smoothing, &boss);
                openvdb::tools::csgUnion(*sdfGrid, *pointSdfGrid);
            }
            if (maskGrid) {
                openvdb::BoolTree::Ptr maskTree(new openvdb::BoolTree(pointMaskGrid->tree(),
                    /*off=*/false, /*on=*/true, openvdb::TopologyCopy()));
                if (dilation > 0) { openvdb::tools::dilateActiveValues(*maskTree, dilation); }
                maskGrid->setTree(maskTree);
            }
        }

        if (outputBoundingMaskGrid) {
            openvdb::Real radiusScale = paList.radiusMult();
            openvdb::Real offset = openvdb::Real(evalFloat("boundinglimit", 0,  time));
            offset = std::min(std::max(offset, 0.0), 1.0); // clamp to zero-one range.

            openvdb::FloatGrid::Ptr maxGrid = openvdb::FloatGrid::create(background);
            maxGrid->setGridClass(openvdb::GRID_LEVEL_SET);
            maxGrid->setTransform(transform->copy());

            openvdb::FloatGrid::Ptr minGrid = openvdb::FloatGrid::create(background);
            minGrid->setGridClass(openvdb::GRID_LEVEL_SET);
            minGrid->setTransform(transform->copy());

            if (offset > 0.0f) {
                if (doSphereConversion) {
                    paList.setRadiusMult(radiusScale * (1.0 + offset));
                    this->convert(time, paList, maxGrid, nullptr, boss);

                    paList.setRadiusMult(radiusScale * (1.0 - offset));
                    this->convert(time, paList, minGrid, nullptr, boss);
                } else {
                    if (!pointMaskGrid) {
                        pointMaskGrid = GUvdbCreatePointMaskGrid(*transform, *ptGeo);
                    }
                    openvdb::Real dx = openvdb::Real(std::min(dilation, 1));
                    int increase = int(std::ceil(dx * (1.0 + offset)));
                    int decrease = int(dx * (1.0 - offset));

                    maxGrid = openvdb::tools::topologyToLevelSet(
                        *pointMaskGrid, bandWidth, closing, increase, smoothing, &boss);

                    minGrid = openvdb::tools::topologyToLevelSet(
                        *pointMaskGrid, bandWidth, closing, decrease, smoothing, &boss);
                }
            }

            openvdb::tools::csgDifference(*maxGrid, *minGrid);
            openvdb::tools::sdfToFogVolume(*maxGrid);

            maxGrid->setName(evalStdString("maskname", time));
            hvdb::createVdbPrimitive(*gdp, maxGrid);
        }

        if (outputLevelSetGrid && sdfGrid) {
            sdfGrid->setName(evalStdString("distancename", time));
            hvdb::createVdbPrimitive(*gdp, sdfGrid);
        }
        if (outputInteriorMaskGrid && maskGrid) {
            maskGrid->setName(evalStdString("interiormaskname", time));
            hvdb::createVdbPrimitive(*gdp, maskGrid);
        }
        if (outputFogVolumeGrid && sdfGrid) {
            // Only duplicate the output grid if both distance and fog volume grids are exported.
            auto fogGrid = (!outputLevelSetGrid ? sdfGrid : sdfGrid->deepCopy());
            openvdb::tools::sdfToFogVolume(*fogGrid);
            fogGrid->setName(evalStdString("fogname", time));
            hvdb::createVdbPrimitive(*gdp, fogGrid);
        }

    } catch (std::exception& e) {
        addError(SOP_MESSAGE, e.what());
    }

    return error();
}
