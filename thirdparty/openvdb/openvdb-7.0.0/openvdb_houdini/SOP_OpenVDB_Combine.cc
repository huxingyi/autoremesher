// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file SOP_OpenVDB_Combine.cc
///
/// @author FX R&D OpenVDB team

#include <houdini_utils/ParmFactory.h>
#include <openvdb_houdini/Utils.h>
#include <openvdb_houdini/SOP_NodeVDB.h>
#include <openvdb/math/Math.h> // for isFinite()
#include <openvdb/tools/ChangeBackground.h>
#include <openvdb/tools/Composite.h>
#include <openvdb/tools/GridTransformer.h> // for resampleToMatch()
#include <openvdb/tools/LevelSetRebuild.h> // for levelSetRebuild()
#include <openvdb/tools/Morphology.h> // for deactivate()
#include <openvdb/tools/Prune.h>
#include <openvdb/tools/SignedFloodFill.h>
#include <openvdb/util/NullInterrupter.h>
#include <PRM/PRM_Parm.h>
#include <UT/UT_Interrupt.h>
#include <UT/UT_Version.h>
#include <algorithm> // for std::min()
#include <cctype> // for isspace()
#include <iomanip>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>



namespace hvdb = openvdb_houdini;
namespace hutil = houdini_utils;

namespace {

//
// Operations
//

enum Operation {
    OP_COPY_A,            // A
    OP_COPY_B,            // B
    OP_INVERT,            // 1 - A
    OP_ADD,               // A + B
    OP_SUBTRACT,          // A - B
    OP_MULTIPLY,          // A * B
    OP_DIVIDE,            // A / B
    OP_MAXIMUM,           // max(A, B)
    OP_MINIMUM,           // min(A, B)
    OP_BLEND1,            // (1 - A) * B
    OP_BLEND2,            // A + (1 - A) * B
    OP_UNION,             // CSG A u B
    OP_INTERSECTION,      // CSG A n B
    OP_DIFFERENCE,        // CSG A / B
    OP_REPLACE,           // replace A with B
    OP_TOPO_UNION,        // A u active(B)
    OP_TOPO_INTERSECTION, // A n active(B)
    OP_TOPO_DIFFERENCE    // A / active(B)
};
enum { OP_FIRST = OP_COPY_A, OP_LAST = OP_TOPO_DIFFERENCE };

//#define TIMES " \xd7 " // ISO-8859 multiplication symbol
#define TIMES " * "
const char* const sOpMenuItems[] = {
    "copya",                "Copy A",
    "copyb",                "Copy B",
    "inverta",              "Invert A",
    "add",                  "Add",
    "subtract",             "Subtract",
    "multiply",             "Multiply",
    "divide",               "Divide",
    "maximum",              "Maximum",
    "minimum",              "Minimum",
    "compatimesb",          "(1 - A)" TIMES "B",
    "apluscompatimesb",     "A + (1 - A)" TIMES "B",
    "sdfunion",             "SDF Union",
    "sdfintersect",         "SDF Intersection",
    "sdfdifference",        "SDF Difference",
    "replacewithactive",    "Replace A with Active B",
    "topounion",            "Activity Union",
    "topointersect",        "Activity Intersection",
    "topodifference",       "Activity Difference",
    nullptr
};
#undef TIMES

inline Operation
asOp(int i, Operation defaultOp = OP_COPY_A)
{
    return (i >= OP_FIRST && i <= OP_LAST)
        ? static_cast<Operation>(i) : defaultOp;
}

inline bool needAGrid(Operation op) { return (op != OP_COPY_B); }
inline bool needBGrid(Operation op) { return (op != OP_COPY_A && op != OP_INVERT); }
inline bool needLevelSets(Operation op)
{
    return (op == OP_UNION || op == OP_INTERSECTION || op == OP_DIFFERENCE);
}

//
// Resampling options
//

enum ResampleMode {
    RESAMPLE_OFF,    // don't auto-resample grids
    RESAMPLE_B,      // resample B to match A
    RESAMPLE_A,      // resample A to match B
    RESAMPLE_HI_RES, // resample higher-res grid to match lower-res
    RESAMPLE_LO_RES  // resample lower-res grid to match higher-res
};
enum { RESAMPLE_MODE_FIRST = RESAMPLE_OFF, RESAMPLE_MODE_LAST = RESAMPLE_LO_RES };

const char* const sResampleModeMenuItems[] = {
    "off",      "Off",
    "btoa",     "B to Match A",
    "atob",     "A to Match B",
    "hitolo",   "Higher-res to Match Lower-res",
    "lotohi",   "Lower-res to Match Higher-res",
    nullptr
};

inline ResampleMode
asResampleMode(exint i, ResampleMode defaultMode = RESAMPLE_B)
{
    return (i >= RESAMPLE_MODE_FIRST && i <= RESAMPLE_MODE_LAST)
        ? static_cast<ResampleMode>(i) : defaultMode;
}


//
// Collation options
//

enum CollationMode {
    COLL_PAIRS = 0,
    COLL_A_WITH_1ST_B,
    COLL_FLATTEN_A,
    COLL_FLATTEN_B_TO_A,
    COLL_FLATTEN_A_GROUPS
};

inline CollationMode
asCollation(const std::string& str)
{
    if (str == "pairs")          return COLL_PAIRS;
    if (str == "awithfirstb")    return COLL_A_WITH_1ST_B;
    if (str == "flattena")       return COLL_FLATTEN_A;
    if (str == "flattenbtoa")    return COLL_FLATTEN_B_TO_A;
    if (str == "flattenagroups") return COLL_FLATTEN_A_GROUPS;

    throw std::runtime_error{"invalid collation mode \"" + str + "\""};
}

} // anonymous namespace


/// @brief SOP to combine two VDB grids via various arithmetic operations
class SOP_OpenVDB_Combine: public hvdb::SOP_NodeVDB
{
public:
    SOP_OpenVDB_Combine(OP_Network*, const char* name, OP_Operator*);
    ~SOP_OpenVDB_Combine() override {}

    static OP_Node* factory(OP_Network*, const char*, OP_Operator*);

    class Cache: public SOP_VDBCacheOptions
    {
    public:
        fpreal getTime() const { return mTime; }
    protected:
        OP_ERROR cookVDBSop(OP_Context&) override;
    private:
        hvdb::GridPtr combineGrids(Operation,
            hvdb::GridCPtr aGrid, hvdb::GridCPtr bGrid,
            const UT_String& aGridName, const UT_String& bGridName,
            ResampleMode resample);

        fpreal mTime = 0.0;
    }; // class Cache

protected:
    bool updateParmsFlags() override;
    void resolveObsoleteParms(PRM_ParmList*) override;

private:
    template<typename> struct DispatchOp;
    struct CombineOp;
};


////////////////////////////////////////


void
newSopOperator(OP_OperatorTable* table)
{
    if (table == nullptr) return;

    hutil::ParmList parms;

    // Group A
    parms.add(hutil::ParmFactory(PRM_STRING, "agroup", "Group A")
        .setChoiceList(&hutil::PrimGroupMenuInput1)
        .setTooltip("Use a subset of the first input as the A VDB(s).")
        .setDocumentation(
            "The VDBs to be used from the first input"
            " (see [specifying volumes|/model/volumes#group])"));

    // Group B
    parms.add(hutil::ParmFactory(PRM_STRING, "bgroup", "Group B")
        .setChoiceList(&hutil::PrimGroupMenuInput2)
        .setTooltip("Use a subset of the second input as the B VDB(s).")
        .setDocumentation(
            "The VDBs to be used from the second input"
            " (see [specifying volumes|/model/volumes#group])"));

    parms.add(hutil::ParmFactory(PRM_STRING, "collation", "Collation")
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, {
            "pairs",          "Combine A/B Pairs",
            "awithfirstb",    "Combine Each A With First B",
            "flattena",       "Flatten All A",
            "flattenbtoa",    "Flatten All B Into First A",
            "flattenagroups", "Flatten A Groups"
        })
        .setDefault("pairs")
        .setTooltip("Specify the order in which to combine VDBs from the A and/or B groups.")
        .setDocumentation("\
The order in which to combine VDBs from the _A_ and/or _B_ groups\n\
\n\
Combine _A_/_B_ Pairs:\n\
    Combine pairs of _A_ and _B_ VDBs, in the order in which they appear\n\
    in their respective groups.\n\
Combine Each _A_ With First _B_:\n\
    Combine each _A_ VDB with the first _B_ VDB.\n\
Flatten All _A_:\n\
    Collapse all of the _A_ VDBs into a single output VDB.\n\
Flatten All _B_ Into First _A_:\n\
    Accumulate each _B_ VDB into the first _A_ VDB, producing a single output VDB.\n\
Flatten _A_ Groups:\n\
    Collapse VDBs within each _A_ group, producing one output VDB for each group.\n\
\n\
    Space-separated group patterns are treated as distinct groups in this mode.\n\
    For example, \"`@name=x* @name=y*`\" results in two output VDBs\n\
    (provided that there is at least one _A_ VDB whose name starts with `x`\n\
    and at least one whose name starts with `y`).\n\
"));
    // Menu of available operations
    parms.add(hutil::ParmFactory(PRM_ORD, "operation", "Operation")
        .setDefault(PRMzeroDefaults)
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, sOpMenuItems)
        .setDocumentation("\
Each voxel that is active in either of the input VDBs\n\
will be processed with this operation.\n\
\n\
Copy _A_:\n\
    Use _A_, ignore _B_.\n\
\n\
Copy _B_:\n\
    Use _B_, ignore _A_.\n\
\n\
Invert _A_:\n\
    Use 0 &minus; _A_.\n\
\n\
Add:\n\
    Add the values of _A_ and _B_.\n\
\n\
NOTE:\n\
    Using this for fog volumes, which have density values between 0 and 1,\n\
    will push densities over 1 and cause a bright interface between the\n\
    input volumes when rendered.  To avoid this problem, try using the\n\
    _A_&nbsp;+&nbsp;(1&nbsp;&minus;&nbsp;_A_)&nbsp;&times;&nbsp;_B_\n\
    operation.\n\
\n\
Subtract:\n\
    Subtract the values of _B_ from the values of _A_.\n\
\n\
Multiply:\n\
    Multiply the values of _A_ and _B_.\n\
\n\
Divide:\n\
    Divide the values of _A_ by _B_.\n\
\n\
Maximum:\n\
    Use the maximum of each corresponding value from _A_ and _B_.\n\
\n\
NOTE:\n\
    Using this for fog volumes, which have density values between 0 and 1,\n\
    can produce a dark interface between the inputs when rendered, due to\n\
    the binary nature of choosing a value from either from _A_ or _B_.\n\
    To avoid this problem, try using the\n\
    (1&nbsp;&minus;&nbsp;_A_)&nbsp;&times;&nbsp;_B_ operation.\n\
\n\
Minimum:\n\
    Use the minimum of each corresponding value from _A_ and _B_.\n\
\n\
(1&nbsp;&minus;&nbsp;_A_)&nbsp;&times;&nbsp;_B_:\n\
    This is similar to SDF Difference, except for fog volumes,\n\
    and can also be viewed as \"soft cutout\" operation.\n\
    It is typically used to clear out an area around characters\n\
    in a dust simulation or some other environmental volume.\n\
\n\
_A_&nbsp;+&nbsp;(1&nbsp;&minus;&nbsp;_A_)&nbsp;&times;&nbsp;_B_:\n\
    This is similar to SDF Union, except for fog volumes, and\n\
    can also be viewed as a \"soft union\" or \"merge\" operation.\n\
    Consider using this over the Maximum or Add operations\n\
    for fog volumes.\n\
\n\
SDF Union:\n\
    Generate the union of signed distance fields _A_ and _B_.\n\
\n\
SDF Intersection:\n\
    Generate the intersection of signed distance fields _A_ and _B_.\n\
\n\
SDF Difference:\n\
    Remove signed distance field _B_ from signed distance field _A_.\n\
\n\
Replace _A_ with Active _B_:\n\
    Copy the active voxels of _B_ into _A_.\n\
\n\
Activity Union:\n\
    Make voxels active if they are active in either _A_ or _B_.\n\
\n\
Activity Intersection:\n\
    Make voxels active if they are active in both _A_ and _B_.\n\
\n\
    It is recommended to enable pruning when using this operation.\n\
\n\
Activity Difference:\n\
    Make voxels active if they are active in _A_ but not in _B_.\n\
\n\
    It is recommended to enable pruning when using this operation.\n"));

    // Scalar multiplier on the A grid
    parms.add(hutil::ParmFactory(PRM_FLT_J, "amult", "A Multiplier")
        .setDefault(PRMoneDefaults)
        .setRange(PRM_RANGE_UI, -10, PRM_RANGE_UI, 10)
        .setTooltip(
            "Multiply voxel values in the A VDB by a scalar\n"
            "before combining the A VDB with the B VDB."));

    // Scalar multiplier on the B grid
    parms.add(hutil::ParmFactory(PRM_FLT_J, "bmult", "B Multiplier")
        .setDefault(PRMoneDefaults)
        .setRange(PRM_RANGE_UI, -10, PRM_RANGE_UI, 10)
        .setTooltip(
            "Multiply voxel values in the B VDB by a scalar\n"
            "before combining the A VDB with the B VDB."));

    // Menu of resampling options
    parms.add(hutil::ParmFactory(PRM_ORD, "resample", "Resample")
        .setDefault(PRMoneDefaults)
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, sResampleModeMenuItems)
        .setTooltip(
            "If the A and B VDBs have different transforms, one VDB should\n"
            "be resampled to match the other before the two are combined.\n"
            "Also, level set VDBs should have matching background values\n"
            "(i.e., matching narrow band widths)."));

    // Menu of resampling interpolation order options
    parms.add(hutil::ParmFactory(PRM_ORD, "resampleinterp", "Interpolation")
        .setDefault(PRMoneDefaults)
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, {
            "point",     "Nearest",
            "linear",    "Linear",
            "quadratic", "Quadratic"
        })
        .setTooltip(
            "Specify the type of interpolation to be used when\n"
            "resampling one VDB to match the other's transform.")
        .setDocumentation(
            "The type of interpolation to be used when resampling one VDB"
            " to match the other's transform\n\n"
            "Nearest neighbor interpolation is fast but can introduce noticeable"
            " sampling artifacts.  Quadratic interpolation is slow but high-quality."
            " Linear interpolation is intermediate in speed and quality."));

    // Deactivate background value toggle
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "deactivate", "Deactivate Background Voxels")
        .setDefault(PRMzeroDefaults)
        .setTypeExtended(PRM_TYPE_TOGGLE_JOIN)
        .setDocumentation(
            "Deactivate active output voxels whose values equal"
            " the output VDB's background value."));

    // Deactivation tolerance slider
    parms.add(hutil::ParmFactory(PRM_FLT_J, "bgtolerance", "Deactivate Tolerance")
        .setDefault(PRMzeroDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_UI, 1)
        .setTooltip(
            "Deactivate active output voxels whose values\n"
            "equal the output VDB's background value.\n"
            "Voxel values are considered equal if they differ\n"
            "by less than the specified tolerance.")
        .setDocumentation(
            "When deactivation of background voxels is enabled,"
            " voxel values are considered equal to the background"
            " if they differ by less than this tolerance."));

    // Prune toggle
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "prune", "Prune")
        .setDefault(PRMoneDefaults)
        .setTypeExtended(PRM_TYPE_TOGGLE_JOIN)
        .setDocumentation(
            "Reduce the memory footprint of output VDBs that have"
            " (sufficiently large) regions of voxels with the same value.\n\n"
            "NOTE:\n"
            "    Pruning affects only the memory usage of a VDB.\n"
            "    It does not remove voxels, apart from inactive voxels\n"
            "    whose value is equal to the background."));

    // Pruning tolerance slider
    parms.add(hutil::ParmFactory(PRM_FLT_J, "tolerance", "Prune Tolerance")
        .setDefault(PRMzeroDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_UI, 1)
        .setTooltip(
            "Collapse regions of constant value in output VDBs.\n"
            "Voxel values are considered equal if they differ\n"
            "by less than the specified tolerance.")
        .setDocumentation(
            "When pruning is enabled, voxel values are considered equal"
            " if they differ by less than the specified tolerance."));

    // Flood fill toggle
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "flood", "Signed-Flood-Fill Output SDFs")
        .setDefault(PRMzeroDefaults)
        .setTooltip(
            "Reclassify inactive voxels of level set VDBs as either inside or outside.")
        .setDocumentation(
            "Test inactive voxels to determine if they are inside or outside of an SDF"
            " and hence whether they should have negative or positive sign."));


    // Obsolete parameters
    hutil::ParmList obsoleteParms;
    obsoleteParms.add(hutil::ParmFactory(PRM_ORD, "combination", "Operation")
        .setDefault(-2));
    obsoleteParms.add(hutil::ParmFactory(PRM_SEPARATOR, "sep1", ""));
    obsoleteParms.add(hutil::ParmFactory(PRM_SEPARATOR, "sep2", ""));
    obsoleteParms.add(hutil::ParmFactory(PRM_TOGGLE, "flatten", "Flatten All B into A")
        .setDefault(PRMzeroDefaults));
    obsoleteParms.add(hutil::ParmFactory(PRM_TOGGLE, "pairs", "Combine A/B Pairs")
        .setDefault(PRMoneDefaults));
    obsoleteParms.add(hutil::ParmFactory(PRM_STRING, "groupA", "Group A"));
    obsoleteParms.add(hutil::ParmFactory(PRM_STRING, "groupB", "Group B"));
    obsoleteParms.add(hutil::ParmFactory(PRM_FLT_J, "mult_a", "A Multiplier")
        .setDefault(PRMoneDefaults));
    obsoleteParms.add(hutil::ParmFactory(PRM_FLT_J, "mult_b", "B Multiplier")
        .setDefault(PRMoneDefaults));


    // Register SOP
    hvdb::OpenVDBOpFactory("VDB Combine", SOP_OpenVDB_Combine::factory, parms, *table)
        .addInput("A VDBs")
        .addOptionalInput("B VDBs")
        .setObsoleteParms(obsoleteParms)
        .setVerb(SOP_NodeVerb::COOK_INPLACE, []() { return new SOP_OpenVDB_Combine::Cache; })
        .setDocumentation("\
#icon: COMMON/openvdb\n\
#tags: vdb\n\
\n\
\"\"\"Combine the values of VDB volumes in various ways.\"\"\"\n\
\n\
@related\n\
\n\
- [Node:sop/vdbcombine]\n\
- [Node:sop/volumevop]\n\
- [Node:sop/volumemix]\n\
\n\
@examples\n\
\n\
See [openvdb.org|http://www.openvdb.org/download/] for source code\n\
and usage examples.\n");
}


////////////////////////////////////////


OP_Node*
SOP_OpenVDB_Combine::factory(OP_Network* net,
    const char* name, OP_Operator* op)
{
    return new SOP_OpenVDB_Combine(net, name, op);
}


SOP_OpenVDB_Combine::SOP_OpenVDB_Combine(OP_Network* net, const char* name, OP_Operator* op)
    : SOP_NodeVDB(net, name, op)
{
}


////////////////////////////////////////


void
SOP_OpenVDB_Combine::resolveObsoleteParms(PRM_ParmList* obsoleteParms)
{
    if (!obsoleteParms) return;

    const fpreal time = 0.0;

    if (PRM_Parm* parm = obsoleteParms->getParmPtr("combination")) {
        if (!parm->isFactoryDefault()) {
            // The "combination" choices (union, intersection, difference) from
            // the old CSG SOP were appended to this SOP's "operation" list.
            switch (obsoleteParms->evalInt("combination", 0, time)) {
                case 0: setInt("operation", 0, 0.0, OP_UNION); break;
                case 1: setInt("operation", 0, 0.0, OP_INTERSECTION); break;
                case 2: setInt("operation", 0, 0.0, OP_DIFFERENCE); break;
            }
        }
    }
    {
        PRM_Parm
            *flatten = obsoleteParms->getParmPtr("flatten"),
            *pairs = obsoleteParms->getParmPtr("pairs");
        if (flatten && !flatten->isFactoryDefault()) { // factory default was Off
            setString("flattenbtoa", CH_STRING_LITERAL, "collation", 0, time);
        } else if (pairs && !pairs->isFactoryDefault()) { // factory default was On
            setString("awithfirstb", CH_STRING_LITERAL, "collation", 0, time);
        }
    }

    resolveRenamedParm(*obsoleteParms, "groupA", "agroup");
    resolveRenamedParm(*obsoleteParms, "groupB", "bgroup");
    resolveRenamedParm(*obsoleteParms, "mult_a", "amult");
    resolveRenamedParm(*obsoleteParms, "mult_b", "bmult");

    // Delegate to the base class.
    hvdb::SOP_NodeVDB::resolveObsoleteParms(obsoleteParms);
}


// Enable or disable parameters in the UI.
bool
SOP_OpenVDB_Combine::updateParmsFlags()
{
    bool changed = false;

    changed |= enableParm("resampleinterp", evalInt("resample", 0, 0) != 0);
    changed |= enableParm("bgtolerance", evalInt("deactivate", 0, 0) != 0);
    changed |= enableParm("tolerance", evalInt("prune", 0, 0) != 0);

    return changed;
}


////////////////////////////////////////


namespace {

using StringVec = std::vector<std::string>;

// Split a string into group patterns separated by whitespace.
// For example, given '@name=d* @id="1 2" {grp1 grp2}', return
// ['@name=d*', '@id="1 2"', '{grp1 grp2}'].
// (This is nonstandard.  Normally, multiple patterns are unioned
// to define a single group.)
// Nesting of quotes and braces is not supported.
inline StringVec
splitPatterns(const std::string& str)
{
    StringVec patterns;
    bool quoted = false, braced = false;
    std::string pattern;
    for (const auto c: str) {
        if (isspace(c)) {
            if (pattern.empty()) continue; // skip whitespace between patterns
            if (quoted || braced) {
                pattern.push_back(c); // keep whitespace within quotes or braces
            } else {
                // At the end of a pattern.  Start a new pattern.
                patterns.push_back(pattern);
                pattern.clear();
                quoted = braced = false;
            }
        } else {
            switch (c) {
                case '"': quoted = !quoted; break;
                case '{': braced = true; break;
                case '}': braced = false; break;
                default: break;
            }
            pattern.push_back(c);
        }
    }
    if (!pattern.empty()) { patterns.push_back(pattern); } // add the final pattern

    // If no patterns were found, add an empty pattern, which matches everything.
    if (patterns.empty()) { patterns.push_back(""); }

    return patterns;
}


inline UT_String
getGridName(const GU_PrimVDB* vdb, const UT_String& defaultName = "")
{
    UT_String name{UT_String::ALWAYS_DEEP};
    if (vdb != nullptr) {
        name = vdb->getGridName();
        if (!name.isstring()) name = defaultName;
    }
    return name;
}

} // anonymous namespace


OP_ERROR
SOP_OpenVDB_Combine::Cache::cookVDBSop(OP_Context& context)
{
    try {
        UT_AutoInterrupt progress{"Combining VDBs"};

        mTime = context.getTime();

        const Operation op = asOp(static_cast<int>(evalInt("operation", 0, getTime())));
        const ResampleMode resample = asResampleMode(evalInt("resample", 0, getTime()));
        const CollationMode collation = asCollation(evalStdString("collation", getTime()));

        const bool
            flattenA = ((collation == COLL_FLATTEN_A) || (collation == COLL_FLATTEN_A_GROUPS)),
            flatten = (flattenA || (collation == COLL_FLATTEN_B_TO_A)),
            needA = needAGrid(op),
            needB = (needBGrid(op) && !flattenA);

        GU_Detail* aGdp = gdp;
        const GU_Detail* bGdp = inputGeo(1, context);

        const auto aGroupStr = evalStdString("agroup", getTime());
        const auto bGroupStr = evalStdString("bgroup", getTime());

        const auto* bGroup = (!bGdp ?  nullptr : matchGroup(*bGdp, bGroupStr));

        // In Flatten A Groups mode, treat space-separated subpatterns
        // as specifying distinct groups to be processed independently.
        // (In all other modes, subpatterns are unioned into a single group.)
        std::vector<const GA_PrimitiveGroup*> aGroupVec;
        if (collation != COLL_FLATTEN_A_GROUPS) {
            aGroupVec.push_back(matchGroup(*aGdp, aGroupStr));
        } else {
            for (const auto& pattern: splitPatterns(aGroupStr)) {
                aGroupVec.push_back(matchGroup(*aGdp, pattern));
            }
        }

        // For diagnostic purposes, keep track of whether any input grids are left unused.
        bool unusedA = false, unusedB = false;

        // Iterate over one or more A groups.
        for (const auto* aGroup: aGroupVec) {
            hvdb::VdbPrimIterator aIt{aGdp, GA_Range::safedeletions{}, aGroup};
            hvdb::VdbPrimCIterator bIt{bGdp, bGroup};

            // Populate two vectors of primitives, one comprising the A grids
            // and the other the B grids.  (In the case of flattening operations,
            // these grids might be taken from the same input.)
            // Note: the following relies on exhausted iterators returning nullptr
            // and on incrementing an exhausted iterator being a no-op.
            std::vector<GU_PrimVDB*> aVdbVec;
            std::vector<const GU_PrimVDB*> bVdbVec;
            switch (collation) {
                case COLL_PAIRS:
                    for ( ; (!needA || aIt) && (!needB || bIt); ++aIt, ++bIt) {
                        aVdbVec.push_back(*aIt);
                        bVdbVec.push_back(*bIt);
                    }
                    unusedA = unusedA || (needA && bool(aIt));
                    unusedB = unusedB || (needB && bool(bIt));
                    break;
                case COLL_A_WITH_1ST_B:
                    for ( ; aIt && (!needB || bIt); ++aIt) {
                        aVdbVec.push_back(*aIt);
                        bVdbVec.push_back(*bIt);
                    }
                    break;
                case COLL_FLATTEN_B_TO_A:
                    aVdbVec.push_back(*aIt);
                    bVdbVec.push_back(*bIt);
                    for (++bIt; bIt; ++bIt) {
                        aVdbVec.push_back(nullptr);
                        bVdbVec.push_back(*bIt);
                    }
                    break;
                case COLL_FLATTEN_A:
                case COLL_FLATTEN_A_GROUPS:
                    aVdbVec.push_back(*aIt);
                    for (++aIt; aIt; ++aIt) { bVdbVec.push_back(*aIt); }
                    break;
            }
            if ((needA && aVdbVec.empty()) || (needB && bVdbVec.empty())) continue;

            std::set<GU_PrimVDB*> vdbsToRemove;

            // Combine grids.
            if (!flatten) {
                // Iterate over A and, optionally, B grids.
                for (size_t i = 0, N = std::min(aVdbVec.size(), bVdbVec.size()); i < N; ++i) {
                    if (progress.wasInterrupted()) { throw std::runtime_error{"interrupted"}; }

                    // Note: even if needA is false, we still need to delete A grids.
                    GU_PrimVDB* aVdb = aVdbVec[i];
                    const GU_PrimVDB* bVdb = bVdbVec[i];

                    hvdb::GridPtr aGrid;
                    hvdb::GridCPtr bGrid;
                    if (aVdb) aGrid = aVdb->getGridPtr();
                    if (bVdb) bGrid = bVdb->getConstGridPtr();

                    // For error reporting, get the names of the A and B grids.
                    const UT_String
                        aGridName = getGridName(aVdb, /*default=*/"A"),
                        bGridName = getGridName(bVdb, /*default=*/"B");

                    if (hvdb::GridPtr outGrid =
                        combineGrids(op, aGrid, bGrid, aGridName, bGridName, resample))
                    {
                        // Name the output grid after the A grid if the A grid is used,
                        // or after the B grid otherwise.
                        UT_String outGridName = needA ? getGridName(aVdb) : getGridName(bVdb);
                        // Add a new VDB primitive for the output grid to the output gdp.
                        GU_PrimVDB::buildFromGrid(*gdp, outGrid,
                            /*copyAttrsFrom=*/needA ? aVdb : bVdb, outGridName);
                        vdbsToRemove.insert(aVdb);
                    }
                }

            // Flatten grids (i.e., combine all B grids into the first A grid).
            } else {
                GU_PrimVDB* aVdb = aVdbVec[0];
                hvdb::GridPtr aGrid;
                if (aVdb) aGrid = aVdb->getGridPtr();

                hvdb::GridPtr outGrid;
                UT_String outGridName;

                // Iterate over B grids.
                const GU_PrimVDB* bVdb = nullptr;
                for (const GU_PrimVDB* theBVdb: bVdbVec) {
                    if (progress.wasInterrupted()) { throw std::runtime_error{"interrupted"}; }

                    bVdb = theBVdb;

                    hvdb::GridCPtr bGrid;
                    if (bVdb) {
                        bGrid = bVdb->getConstGridPtr();
                        if (flattenA) {
                            // When flattening within the A group, remove B grids,
                            // since they're actually copies of grids from input 0.
                            vdbsToRemove.insert(const_cast<GU_PrimVDB*>(bVdb));
                        }
                    }

                    const UT_String
                        aGridName = getGridName(aVdb, /*default=*/"A"),
                        bGridName = getGridName(bVdb, /*default=*/"B");

                    // Name the output grid after the A grid if the A grid is used,
                    // or after the B grid otherwise.
                    outGridName = (needA ? getGridName(aVdb) : getGridName(bVdb));

                    outGrid = combineGrids(op, aGrid, bGrid, aGridName, bGridName, resample);

                    aGrid = outGrid;
                }
                if (outGrid) {
                    // Add a new VDB primitive for the output grid to the output gdp.
                    GU_PrimVDB::buildFromGrid(*gdp, outGrid,
                        /*copyAttrsFrom=*/needA ? aVdb : bVdb, outGridName);
                    vdbsToRemove.insert(aVdb);
                }
            }

            // Remove primitives that were copied from input 0.
            for (GU_PrimVDB* vdb: vdbsToRemove) {
                if (vdb)
                    gdp->destroyPrimitive(*vdb, /*andPoints=*/true);
            }
        } // for each A group

        if (unusedA || unusedB) {
            std::ostringstream ostr;
            ostr << "some grids were not processed because there were more "
                << (unusedA ? "A" : "B") << " grids than "
                << (unusedA ? "B" : "A") << " grids";
            addWarning(SOP_MESSAGE, ostr.str().c_str());
        }

    } catch (std::exception& e) {
        addError(SOP_MESSAGE, e.what());
    }
    return error();
}


////////////////////////////////////////


namespace {

/// Functor to compute scale * grid + offset, for scalars scale and offset
template<typename GridT>
struct MulAdd
{
    using ValueT = typename GridT::ValueType;
    using GridPtrT = typename GridT::Ptr;

    float scale, offset;

    explicit MulAdd(float s, float t = 0.0): scale(s), offset(t) {}

    void operator()(const ValueT& a, const ValueT&, ValueT& out) const
    {
        OPENVDB_NO_TYPE_CONVERSION_WARNING_BEGIN
        out = ValueT(a * scale + offset);
        OPENVDB_NO_TYPE_CONVERSION_WARNING_END
    }

    /// @return true if the scale is 1 and the offset is 0
    bool isIdentity() const
    {
        return (openvdb::math::isApproxEqual(scale, 1.f, 1.0e-6f)
            && openvdb::math::isApproxEqual(offset, 0.f, 1.0e-6f));
    }

    /// Compute dest = src * scale + offset
    void process(const GridT& src, GridPtrT& dest) const
    {
        if (isIdentity()) {
            dest = src.deepCopy();
        } else {
            if (!dest) dest = GridT::create(src); // same transform, new tree
            ValueT bg;
            (*this)(src.background(), ValueT(), bg);
            openvdb::tools::changeBackground(dest->tree(), bg);
            dest->tree().combine2(src.tree(), src.tree(), *this, /*prune=*/false);
        }
    }
};


////////////////////////////////////////


/// Functor to compute (1 - A) * B for grids A and B
template<typename ValueT>
struct Blend1
{
    float aMult, bMult;
    const ValueT ONE;
    explicit Blend1(float a = 1.0, float b = 1.0):
        aMult(a), bMult(b), ONE(openvdb::zeroVal<ValueT>() + 1) {}
    void operator()(const ValueT& a, const ValueT& b, ValueT& out) const
    {
        OPENVDB_NO_TYPE_CONVERSION_WARNING_BEGIN
        out = ValueT((ONE - aMult * a) * bMult * b);
        OPENVDB_NO_TYPE_CONVERSION_WARNING_END
    }
};


////////////////////////////////////////


/// Functor to compute A + (1 - A) * B for grids A and B
template<typename ValueT>
struct Blend2
{
    float aMult, bMult;
    const ValueT ONE;
    explicit Blend2(float a = 1.0, float b = 1.0):
        aMult(a), bMult(b), ONE(openvdb::zeroVal<ValueT>() + 1) {}
    void operator()(const ValueT& a, const ValueT& b, ValueT& out) const
    {
        OPENVDB_NO_TYPE_CONVERSION_WARNING_BEGIN
        out = ValueT(a*aMult); out = out + ValueT((ONE - out) * bMult*b);
        OPENVDB_NO_TYPE_CONVERSION_WARNING_END
    }
};


////////////////////////////////////////


// Helper class to compare both scalar and vector values
template<typename ValueT>
struct ApproxEq
{
    const ValueT &a, &b;
    ApproxEq(const ValueT& _a, const ValueT& _b): a(_a), b(_b) {}
    operator bool() const {
        return openvdb::math::isRelOrApproxEqual(
            a, b, /*rel*/ValueT(1e-6f), /*abs*/ValueT(1e-8f));
    }
};


// Specialization for Vec2
template<typename T>
struct ApproxEq<openvdb::math::Vec2<T> >
{
    using VecT = openvdb::math::Vec2<T>;
    using ValueT = typename VecT::value_type;
    const VecT &a, &b;
    ApproxEq(const VecT& _a, const VecT& _b): a(_a), b(_b) {}
    operator bool() const { return a.eq(b, /*abs=*/ValueT(1e-8f)); }
};


// Specialization for Vec3
template<typename T>
struct ApproxEq<openvdb::math::Vec3<T> >
{
    using VecT = openvdb::math::Vec3<T>;
    using ValueT = typename VecT::value_type;
    const VecT &a, &b;
    ApproxEq(const VecT& _a, const VecT& _b): a(_a), b(_b) {}
    operator bool() const { return a.eq(b, /*abs=*/ValueT(1e-8f)); }
};


// Specialization for Vec4
template<typename T>
struct ApproxEq<openvdb::math::Vec4<T> >
{
    using VecT = openvdb::math::Vec4<T>;
    using ValueT = typename VecT::value_type;
    const VecT &a, &b;
    ApproxEq(const VecT& _a, const VecT& _b): a(_a), b(_b) {}
    operator bool() const { return a.eq(b, /*abs=*/ValueT(1e-8f)); }
};

} // unnamed namespace


////////////////////////////////////////


template<typename AGridT>
struct SOP_OpenVDB_Combine::DispatchOp
{
    SOP_OpenVDB_Combine::CombineOp* combineOp;

    DispatchOp(SOP_OpenVDB_Combine::CombineOp& op): combineOp(&op) {}

    template<typename BGridT> void operator()(typename BGridT::ConstPtr);
}; // struct DispatchOp


// Helper class for use with UTvdbProcessTypedGrid()
struct SOP_OpenVDB_Combine::CombineOp
{
    SOP_OpenVDB_Combine::Cache* self;
    Operation op;
    ResampleMode resample;
    UT_String aGridName, bGridName;
    hvdb::GridCPtr aBaseGrid, bBaseGrid;
    hvdb::GridPtr outGrid;
    hvdb::Interrupter interrupt;

    CombineOp(): self(nullptr) {}

    // Functor for use with UTvdbProcessTypedGridScalar() to return
    // a scalar grid's background value as a floating-point quantity
    struct BackgroundOp {
        double value;
        BackgroundOp(): value(0.0) {}
        template<typename GridT> void operator()(const GridT& grid) {
            value = static_cast<double>(grid.background());
        }
    };
    static double getScalarBackgroundValue(const hvdb::Grid& baseGrid)
    {
        BackgroundOp bgOp;
        UTvdbProcessTypedGridScalar(UTvdbGetGridType(baseGrid), baseGrid, bgOp);
        return bgOp.value;
    }

    template<typename GridT>
    typename GridT::Ptr resampleToMatch(const GridT& src, const hvdb::Grid& ref, int order)
    {
        using ValueT = typename GridT::ValueType;
        const ValueT ZERO = openvdb::zeroVal<ValueT>();

        const openvdb::math::Transform& refXform = ref.constTransform();

        typename GridT::Ptr dest;
        if (src.getGridClass() == openvdb::GRID_LEVEL_SET) {
            // For level set grids, use the level set rebuild tool to both resample the
            // source grid to match the reference grid and to rebuild the resulting level set.
            const bool refIsLevelSet = ref.getGridClass() == openvdb::GRID_LEVEL_SET;
            OPENVDB_NO_TYPE_CONVERSION_WARNING_BEGIN
            const ValueT halfWidth = refIsLevelSet
                ? ValueT(ZERO + this->getScalarBackgroundValue(ref) * (1.0 / ref.voxelSize()[0]))
                : ValueT(src.background() * (1.0 / src.voxelSize()[0]));
            OPENVDB_NO_TYPE_CONVERSION_WARNING_END

            if (!openvdb::math::isFinite(halfWidth)) {
                std::stringstream msg;
                msg << "Resample to match: Illegal narrow band width = " << halfWidth
                    << ", caused by grid '" << src.getName() << "' with background "
                    << this->getScalarBackgroundValue(ref);
                throw std::invalid_argument(msg.str());
            }

            try {
                dest = openvdb::tools::doLevelSetRebuild(src, /*iso=*/ZERO,
                    /*exWidth=*/halfWidth, /*inWidth=*/halfWidth, &refXform, &interrupt);
            } catch (openvdb::TypeError&) {
                self->addWarning(SOP_MESSAGE, ("skipped rebuild of level set grid "
                    + src.getName() + " of type " + src.type()).c_str());
                dest.reset();
            }
        }
        if (!dest && src.constTransform() != refXform) {
            // For non-level set grids or if level set rebuild failed due to an unsupported
            // grid type, use the grid transformer tool to resample the source grid to match
            // the reference grid.
            dest = src.copyWithNewTree();
            dest->setTransform(refXform.copy());
            using namespace openvdb;
            switch (order) {
            case 0: tools::resampleToMatch<tools::PointSampler>(src, *dest, interrupt); break;
            case 1: tools::resampleToMatch<tools::BoxSampler>(src, *dest, interrupt); break;
            case 2: tools::resampleToMatch<tools::QuadraticSampler>(src, *dest, interrupt); break;
            }
        }
        return dest;
    }

    // If necessary, resample one grid so that its index space registers
    // with the other grid's.
    // Note that one of the grid pointers might change as a result.
    template<typename AGridT, typename BGridT>
    void resampleGrids(const AGridT*& aGrid, const BGridT*& bGrid)
    {
        if (!aGrid || !bGrid) return;

        const bool
            needA = needAGrid(op),
            needB = needBGrid(op),
            needBoth = needA && needB;
        const int samplingOrder = static_cast<int>(
            self->evalInt("resampleinterp", 0, self->getTime()));

        // One of RESAMPLE_A, RESAMPLE_B or RESAMPLE_OFF, specifying whether
        // grid A, grid B or neither grid was resampled
        int resampleWhich = RESAMPLE_OFF;

        // Determine which of the two grids should be resampled.
        if (resample == RESAMPLE_HI_RES || resample == RESAMPLE_LO_RES) {
            const openvdb::Vec3d
                aVoxSize = aGrid->voxelSize(),
                bVoxSize = bGrid->voxelSize();
            const double
                aVoxVol = aVoxSize[0] * aVoxSize[1] * aVoxSize[2],
                bVoxVol = bVoxSize[0] * bVoxSize[1] * bVoxSize[2];
            resampleWhich = ((aVoxVol > bVoxVol && resample == RESAMPLE_LO_RES)
                || (aVoxVol < bVoxVol && resample == RESAMPLE_HI_RES))
                ? RESAMPLE_A : RESAMPLE_B;
        } else {
            resampleWhich = resample;
        }

        if (aGrid->constTransform() != bGrid->constTransform()) {
            // If the A and B grid transforms don't match, one of the grids
            // should be resampled into the other's index space.
            if (resample == RESAMPLE_OFF) {
                if (needBoth) {
                    // Resampling is disabled.  Just log a warning.
                    std::ostringstream ostr;
                    ostr << aGridName << " and " << bGridName << " transforms don't match";
                    self->addWarning(SOP_MESSAGE, ostr.str().c_str());
                }
            } else {
                if (needA && resampleWhich == RESAMPLE_A) {
                    // Resample grid A into grid B's index space.
                    aBaseGrid = this->resampleToMatch(*aGrid, *bGrid, samplingOrder);
                    aGrid = static_cast<const AGridT*>(aBaseGrid.get());
                } else if (needB && resampleWhich == RESAMPLE_B) {
                    // Resample grid B into grid A's index space.
                    bBaseGrid = this->resampleToMatch(*bGrid, *aGrid, samplingOrder);
                    bGrid = static_cast<const BGridT*>(bBaseGrid.get());
                }
            }
        }

        if (aGrid->getGridClass() == openvdb::GRID_LEVEL_SET &&
            bGrid->getGridClass() == openvdb::GRID_LEVEL_SET)
        {
            // If both grids are level sets, ensure that their background values match.
            // (If one of the grids was resampled, then the background values should
            // already match.)
            const double
                a = this->getScalarBackgroundValue(*aGrid),
                b = this->getScalarBackgroundValue(*bGrid);
            if (!ApproxEq<double>(a, b)) {
                if (resample == RESAMPLE_OFF) {
                    if (needBoth) {
                        // Resampling/rebuilding is disabled.  Just log a warning.
                        std::ostringstream ostr;
                        ostr << aGridName << " and " << bGridName
                            << " background values don't match ("
                            << std::setprecision(3) << a << " vs. " << b << ");\n"
                            << "                 the output grid will not be a valid level set";
                        self->addWarning(SOP_MESSAGE, ostr.str().c_str());
                    }
                } else {
                    // One of the two grids needs a level set rebuild.
                    if (needA && resampleWhich == RESAMPLE_A) {
                        // Rebuild A to match B's background value.
                        aBaseGrid = this->resampleToMatch(*aGrid, *bGrid, samplingOrder);
                        aGrid = static_cast<const AGridT*>(aBaseGrid.get());
                    } else if (needB && resampleWhich == RESAMPLE_B) {
                        // Rebuild B to match A's background value.
                        bBaseGrid = this->resampleToMatch(*bGrid, *aGrid, samplingOrder);
                        bGrid = static_cast<const BGridT*>(bBaseGrid.get());
                    }
                }
            }
        }
    }

    void checkVectorTypes(const hvdb::Grid* aGrid, const hvdb::Grid* bGrid)
    {
        if (!aGrid || !bGrid || !needAGrid(op) || !needBGrid(op)) return;

        switch (op) {
            case OP_TOPO_UNION:
            case OP_TOPO_INTERSECTION:
            case OP_TOPO_DIFFERENCE:
                // No need to warn about different vector types for topology-only operations.
                break;

            default:
            {
                const openvdb::VecType
                    aVecType = aGrid->getVectorType(),
                    bVecType = bGrid->getVectorType();
                if (aVecType != bVecType) {
                    std::ostringstream ostr;
                    ostr << aGridName << " and " << bGridName
                        << " have different vector types\n"
                        << "                 (" << hvdb::Grid::vecTypeToString(aVecType)
                        << " vs. " << hvdb::Grid::vecTypeToString(bVecType) << ")";
                    self->addWarning(SOP_MESSAGE, ostr.str().c_str());
                }
                break;
            }
        }
    }

    // Combine two grids of the same type.
    template<typename GridT>
    void combineSameType()
    {
        using ValueT = typename GridT::ValueType;

        const bool
            needA = needAGrid(op),
            needB = needBGrid(op);
        const float
            aMult = float(self->evalFloat("amult", 0, self->getTime())),
            bMult = float(self->evalFloat("bmult", 0, self->getTime()));

        const GridT *aGrid = nullptr, *bGrid = nullptr;
        if (aBaseGrid) aGrid = UTvdbGridCast<GridT>(aBaseGrid).get();
        if (bBaseGrid) bGrid = UTvdbGridCast<GridT>(bBaseGrid).get();
        if (needA && !aGrid) throw std::runtime_error("missing A grid");
        if (needB && !bGrid) throw std::runtime_error("missing B grid");

        // Warn if combining vector grids with different vector types.
        if (needA && needB && openvdb::VecTraits<ValueT>::IsVec) {
            this->checkVectorTypes(aGrid, bGrid);
        }

        // If necessary, resample one grid so that its index space
        // registers with the other grid's.
        if (aGrid && bGrid) this->resampleGrids(aGrid, bGrid);

        const ValueT ZERO = openvdb::zeroVal<ValueT>();

        // A temporary grid is needed for binary operations, because they
        // cannibalize the B grid.
        typename GridT::Ptr resultGrid, tempGrid;

        switch (op) {
            case OP_COPY_A:
                MulAdd<GridT>(aMult).process(*aGrid, resultGrid);
                break;

            case OP_COPY_B:
                MulAdd<GridT>(bMult).process(*bGrid, resultGrid);
                break;

            case OP_INVERT:
                MulAdd<GridT>(-aMult, 1.0).process(*aGrid, resultGrid);
                break;

            case OP_ADD:
                MulAdd<GridT>(aMult).process(*aGrid, resultGrid);
                MulAdd<GridT>(bMult).process(*bGrid, tempGrid);
                openvdb::tools::compSum(*resultGrid, *tempGrid);
                break;

            case OP_SUBTRACT:
                MulAdd<GridT>(aMult).process(*aGrid, resultGrid);
                MulAdd<GridT>(-bMult).process(*bGrid, tempGrid);
                openvdb::tools::compSum(*resultGrid, *tempGrid);
                break;

            case OP_MULTIPLY:
                MulAdd<GridT>(aMult).process(*aGrid, resultGrid);
                MulAdd<GridT>(bMult).process(*bGrid, tempGrid);
                openvdb::tools::compMul(*resultGrid, *tempGrid);
                break;

            case OP_DIVIDE:
                MulAdd<GridT>(aMult).process(*aGrid, resultGrid);
                MulAdd<GridT>(bMult).process(*bGrid, tempGrid);
                openvdb::tools::compDiv(*resultGrid, *tempGrid);
                break;

            case OP_MAXIMUM:
                MulAdd<GridT>(aMult).process(*aGrid, resultGrid);
                MulAdd<GridT>(bMult).process(*bGrid, tempGrid);
                openvdb::tools::compMax(*resultGrid, *tempGrid);
                break;

            case OP_MINIMUM:
                MulAdd<GridT>(aMult).process(*aGrid, resultGrid);
                MulAdd<GridT>(bMult).process(*bGrid, tempGrid);
                openvdb::tools::compMin(*resultGrid, *tempGrid);
                break;

            case OP_BLEND1: // (1 - A) * B
            {
                const Blend1<ValueT> comp(aMult, bMult);
                ValueT bg;
                comp(aGrid->background(), ZERO, bg);
                resultGrid = aGrid->copyWithNewTree();
                openvdb::tools::changeBackground(resultGrid->tree(), bg);
                resultGrid->tree().combine2(aGrid->tree(), bGrid->tree(), comp, /*prune=*/false);
                break;
            }
            case OP_BLEND2: // A + (1 - A) * B
            {
                const Blend2<ValueT> comp(aMult, bMult);
                ValueT bg;
                comp(aGrid->background(), ZERO, bg);
                resultGrid = aGrid->copyWithNewTree();
                openvdb::tools::changeBackground(resultGrid->tree(), bg);
                resultGrid->tree().combine2(aGrid->tree(), bGrid->tree(), comp, /*prune=*/false);
                break;
            }

            case OP_UNION:
                MulAdd<GridT>(aMult).process(*aGrid, resultGrid);
                MulAdd<GridT>(bMult).process(*bGrid, tempGrid);
                openvdb::tools::csgUnion(*resultGrid, *tempGrid);
                break;

            case OP_INTERSECTION:
                MulAdd<GridT>(aMult).process(*aGrid, resultGrid);
                MulAdd<GridT>(bMult).process(*bGrid, tempGrid);
                openvdb::tools::csgIntersection(*resultGrid, *tempGrid);
                break;

            case OP_DIFFERENCE:
                MulAdd<GridT>(aMult).process(*aGrid, resultGrid);
                MulAdd<GridT>(bMult).process(*bGrid, tempGrid);
                openvdb::tools::csgDifference(*resultGrid, *tempGrid);
                break;

            case OP_REPLACE:
                MulAdd<GridT>(aMult).process(*aGrid, resultGrid);
                MulAdd<GridT>(bMult).process(*bGrid, tempGrid);
                openvdb::tools::compReplace(*resultGrid, *tempGrid);
                break;

            case OP_TOPO_UNION:
                MulAdd<GridT>(aMult).process(*aGrid, resultGrid);
                // Note: no need to scale the B grid for topology-only operations.
                resultGrid->topologyUnion(*bGrid);
                break;

            case OP_TOPO_INTERSECTION:
                MulAdd<GridT>(aMult).process(*aGrid, resultGrid);
                resultGrid->topologyIntersection(*bGrid);
                break;

            case OP_TOPO_DIFFERENCE:
                MulAdd<GridT>(aMult).process(*aGrid, resultGrid);
                resultGrid->topologyDifference(*bGrid);
                break;
        }

        outGrid = this->postprocess<GridT>(resultGrid);
    }

    // Combine two grids of different types.
    /// @todo Currently, only topology operations can be performed on grids of different types.
    template<typename AGridT, typename BGridT>
    void combineDifferentTypes()
    {
        const bool
            needA = needAGrid(op),
            needB = needBGrid(op);

        const AGridT* aGrid = nullptr;
        const BGridT* bGrid = nullptr;
        if (aBaseGrid) aGrid = UTvdbGridCast<AGridT>(aBaseGrid).get();
        if (bBaseGrid) bGrid = UTvdbGridCast<BGridT>(bBaseGrid).get();
        if (needA && !aGrid) throw std::runtime_error("missing A grid");
        if (needB && !bGrid) throw std::runtime_error("missing B grid");

        // Warn if combining vector grids with different vector types.
        if (needA && needB && openvdb::VecTraits<typename AGridT::ValueType>::IsVec
            && openvdb::VecTraits<typename BGridT::ValueType>::IsVec)
        {
            this->checkVectorTypes(aGrid, bGrid);
        }

        // If necessary, resample one grid so that its index space
        // registers with the other grid's.
        if (aGrid && bGrid) this->resampleGrids(aGrid, bGrid);

        const float aMult = float(self->evalFloat("amult", 0, self->getTime()));

        typename AGridT::Ptr resultGrid;

        switch (op) {
            case OP_TOPO_UNION:
                MulAdd<AGridT>(aMult).process(*aGrid, resultGrid);
                // Note: no need to scale the B grid for topology-only operations.
                resultGrid->topologyUnion(*bGrid);
                break;

            case OP_TOPO_INTERSECTION:
                MulAdd<AGridT>(aMult).process(*aGrid, resultGrid);
                resultGrid->topologyIntersection(*bGrid);
                break;

            case OP_TOPO_DIFFERENCE:
                MulAdd<AGridT>(aMult).process(*aGrid, resultGrid);
                resultGrid->topologyDifference(*bGrid);
                break;

            default:
            {
                std::ostringstream ostr;
                ostr << "can't combine grid " << aGridName << " of type " << aGrid->type()
                    << "\n                 with grid " << bGridName
                    << " of type " << bGrid->type();
                throw std::runtime_error(ostr.str());
                break;
            }
        }

        outGrid = this->postprocess<AGridT>(resultGrid);
    }

    template<typename GridT>
    typename GridT::Ptr postprocess(typename GridT::Ptr resultGrid)
    {
        using ValueT = typename GridT::ValueType;
        const ValueT ZERO = openvdb::zeroVal<ValueT>();

        const bool
            prune = self->evalInt("prune", 0, self->getTime()),
            flood = self->evalInt("flood", 0, self->getTime()),
            deactivate = self->evalInt("deactivate", 0, self->getTime());

        if (deactivate) {
            const float deactivationTolerance =
                float(self->evalFloat("bgtolerance", 0, self->getTime()));
            OPENVDB_NO_TYPE_CONVERSION_WARNING_BEGIN
            const ValueT tolerance(ZERO + deactivationTolerance);
            OPENVDB_NO_TYPE_CONVERSION_WARNING_END
            // Mark active output tiles and voxels as inactive if their
            // values match the output grid's background value.
            // Do this first to facilitate pruning.
            openvdb::tools::deactivate(*resultGrid, resultGrid->background(), tolerance);
        }

        if (flood && resultGrid->getGridClass() == openvdb::GRID_LEVEL_SET) {
            openvdb::tools::signedFloodFill(resultGrid->tree());
        }
        if (prune) {
            const float pruneTolerance = float(self->evalFloat("tolerance", 0, self->getTime()));
            OPENVDB_NO_TYPE_CONVERSION_WARNING_BEGIN
            const ValueT tolerance(ZERO + pruneTolerance);
            OPENVDB_NO_TYPE_CONVERSION_WARNING_END
            openvdb::tools::prune(resultGrid->tree(), tolerance);
        }

        return resultGrid;
    }

    template<typename AGridT>
    void operator()(typename AGridT::ConstPtr)
    {
        const bool
            needA = needAGrid(op),
            needB = needBGrid(op),
            needBoth = needA && needB;

        if (!needBoth || !aBaseGrid || !bBaseGrid || aBaseGrid->type() == bBaseGrid->type()) {
            this->combineSameType<AGridT>();
        } else {
            DispatchOp<AGridT> dispatcher(*this);
            // Dispatch on the B grid's type.
            int success = UTvdbProcessTypedGridTopology(
                UTvdbGetGridType(*bBaseGrid), bBaseGrid, dispatcher);
            if (!success) {
                std::ostringstream ostr;
                ostr << "grid " << bGridName << " has unsupported type " << bBaseGrid->type();
                self->addWarning(SOP_MESSAGE, ostr.str().c_str());
            }
        }
    }
}; // struct CombineOp


template<typename AGridT>
template<typename BGridT>
void
SOP_OpenVDB_Combine::DispatchOp<AGridT>::operator()(typename BGridT::ConstPtr)
{
    combineOp->combineDifferentTypes<AGridT, BGridT>();
}


////////////////////////////////////////


hvdb::GridPtr
SOP_OpenVDB_Combine::Cache::combineGrids(
    Operation op,
    hvdb::GridCPtr aGrid,
    hvdb::GridCPtr bGrid,
    const UT_String& aGridName,
    const UT_String& bGridName,
    ResampleMode resample)
{
    hvdb::GridPtr outGrid;

    const bool
        needA = needAGrid(op),
        needB = needBGrid(op),
        needLS = needLevelSets(op);

    if (!needA && !needB) throw std::runtime_error("nothing to do");
    if (needA && !aGrid) throw std::runtime_error("missing A grid");
    if (needB && !bGrid) throw std::runtime_error("missing B grid");

    if (needLS &&
        ((aGrid && aGrid->getGridClass() != openvdb::GRID_LEVEL_SET) ||
         (bGrid && bGrid->getGridClass() != openvdb::GRID_LEVEL_SET)))
    {
        std::ostringstream ostr;
        ostr << "expected level set grids for the " << sOpMenuItems[op*2+1]
            << " operation,\n                 found "
            << hvdb::Grid::gridClassToString(aGrid->getGridClass()) << " (" << aGridName << ") and "
            << hvdb::Grid::gridClassToString(bGrid->getGridClass()) << " (" << bGridName
            << ");\n                 the output grid will not be a valid level set";
        addWarning(SOP_MESSAGE, ostr.str().c_str());
    }

    if (needA && needB && aGrid->type() != bGrid->type()
        && op != OP_TOPO_UNION && op != OP_TOPO_INTERSECTION && op != OP_TOPO_DIFFERENCE)
    {
        std::ostringstream ostr;
        ostr << "can't combine grid " << aGridName << " of type " << aGrid->type()
            << "\n                 with grid " << bGridName << " of type " << bGrid->type();
        addWarning(SOP_MESSAGE, ostr.str().c_str());
        return outGrid;
    }

    CombineOp compOp;
    compOp.self = this;
    compOp.op = op;
    compOp.resample = resample;
    compOp.aBaseGrid = aGrid;
    compOp.bBaseGrid = bGrid;
    compOp.aGridName = aGridName;
    compOp.bGridName = bGridName;
    compOp.interrupt = hvdb::Interrupter();

    int success = UTvdbProcessTypedGridTopology(
        UTvdbGetGridType(needA ? *aGrid : *bGrid), aGrid, compOp);
    if (!success || !compOp.outGrid) {
        std::ostringstream ostr;
        if (aGrid->type() == bGrid->type()) {
            ostr << "grids " << aGridName << " and " << bGridName
                << " have unsupported type " << aGrid->type();
        } else {
            ostr << "grid " << (needA ? aGridName : bGridName)
                << " has unsupported type " << (needA ? aGrid->type() : bGrid->type());
        }
        addWarning(SOP_MESSAGE, ostr.str().c_str());
    }
    return compOp.outGrid;
}
