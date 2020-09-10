// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file SOP_OpenVDB_Read.cc
///
/// @author FX R&D OpenVDB team

#include <houdini_utils/ParmFactory.h>
#include <openvdb_houdini/Utils.h>
#include <openvdb_houdini/SOP_NodeVDB.h>
#include <openvdb_houdini/GEO_PrimVDB.h>
#include <openvdb_houdini/GU_PrimVDB.h>
#include <UT/UT_Interrupt.h>
#include <cctype>
#include <stdexcept>
#include <string>

namespace hvdb = openvdb_houdini;
namespace hutil = houdini_utils;


class SOP_OpenVDB_Read: public hvdb::SOP_NodeVDB
{
public:
    SOP_OpenVDB_Read(OP_Network*, const char* name, OP_Operator*);
    ~SOP_OpenVDB_Read() override {}

    void getDescriptiveParmName(UT_String& s) const override { s = "file_name"; }

    static void registerSop(OP_OperatorTable*);
    static OP_Node* factory(OP_Network*, const char* name, OP_Operator*);

    int isRefInput(unsigned input) const override { return (input == 0); }

protected:
    OP_ERROR cookVDBSop(OP_Context&) override;
    bool updateParmsFlags() override;
};


////////////////////////////////////////


namespace {

// Populate a choice list with grid names read from a VDB file.
void
populateGridMenu(void* data, PRM_Name* choicenames, int listsize,
    const PRM_SpareData*, const PRM_Parm*)
{
    choicenames[0].setToken(0);
    choicenames[0].setLabel(0);

    hvdb::SOP_NodeVDB* sop = static_cast<hvdb::SOP_NodeVDB*>(data);
    if (sop == nullptr) return;

    // Get the parameters from the GUI
    // The file name of the vdb we would like to load
    const auto file_name = sop->evalStdString("file_name", 0);

     // Keep track of how many names we have entered
    int count = 0;

    // Add the star token to the menu
    choicenames[0].setTokenAndLabel("*", "*");
    ++count;

    try {
        // Open the file and read the header, but don't read in any grids.
        // An exception is thrown if the file is not a valid VDB file.
        openvdb::io::File file(file_name);
        file.open();

        // Loop over the names of all of the grids in the file.
        for (openvdb::io::File::NameIterator nameIter = file.beginName();
            nameIter != file.endName(); ++nameIter)
        {
            // Make sure we don't write more than the listsize,
            // and reserve a spot for the terminating 0.
            if (count > listsize - 2) break;

            std::string gridName = nameIter.gridName(), tokenName = gridName;

            // When a file contains multiple grids with the same name, the names are
            // distinguished with a trailing array index ("grid[0]", "grid[1]", etc.).
            // Escape such names as "grid\[0]", "grid\[1]", etc. to inhibit UT_String's
            // pattern matching.
            if (tokenName.back() == ']') {
                auto start = tokenName.find_last_of('[');
                if (start != std::string::npos && tokenName[start + 1] != ']') {
                    for (auto i = start + 1; i < tokenName.size() - 1; ++i) {
                        // Only digits should appear between the last '[' and the trailing ']'.
                        if (!std::isdigit(tokenName[i])) { start = std::string::npos; break; }
                    }
                    if (start != std::string::npos) tokenName.replace(start, 1, "\\[");
                }
            }

            // Add the grid's name to the list.
            choicenames[count].setTokenAndLabel(tokenName.c_str(), gridName.c_str());
            ++count;
        }

        file.close();
    } catch (...) {}

    // Terminate the list.
    choicenames[count].setTokenAndLabel(nullptr, nullptr);
}


// Callback to trigger a file reload
int
reloadCB(void* data, int /*idx*/, float /*time*/, const PRM_Template*)
{
    SOP_OpenVDB_Read* sop = static_cast<SOP_OpenVDB_Read*>(data);
    if (nullptr != sop) {
        sop->forceRecook();
        return 1; // request a refresh of the parameter pane
    }
    return 0; // no refresh
}

} // unnamed namespace


////////////////////////////////////////


// Build UI and register this operator.
void
newSopOperator(OP_OperatorTable* table)
{
    if (table == nullptr) return;

    hutil::ParmList parms;

    // Metadata-only toggle
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "metadata_only", "Read Metadata Only")
        .setDefault(PRMzeroDefaults)
        .setTooltip(
            "If enabled, output empty VDBs populated with their metadata and transforms only."));

    // Clipping toggle
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "clip", "Clip to Reference Bounds")
        .setDefault(PRMzeroDefaults)
        .setTooltip("Clip VDBs to the bounding box of the reference geometry."));

    // Filename
    parms.add(hutil::ParmFactory(PRM_FILE, "file_name", "File Name")
        .setDefault(0, "./filename.vdb")
        .setTooltip("Select a VDB file."));

    // Grid name mask
    parms.add(hutil::ParmFactory(PRM_STRING, "grids",  "VDB(s)")
        .setDefault(0, "*")
        .setChoiceList(new PRM_ChoiceList(PRM_CHOICELIST_TOGGLE, populateGridMenu))
        .setTooltip("VDB names separated by white space (wildcards allowed)")
        .setDocumentation(
            "VDB names separated by white space (wildcards allowed)\n\n"
            "NOTE:\n"
            "    To distinguish between multiple VDBs with the same name,\n"
            "    append an array index to the name: `density\\[0]`, `density\\[1]`, etc.\n"
            "    Escape the index with a backslash to inhibit wildcard pattern matching.\n"));

    // Toggle to enable/disable grouping
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "enable_grouping", "")
        .setTypeExtended(PRM_TYPE_TOGGLE_JOIN)
        .setDefault(PRMoneDefaults)
        .setTooltip(
            "If enabled, create a group with the given name that comprises the selected VDBs.\n"
            "If disabled, do not group the selected VDBs."));

    // Name for the output group
    parms.add(hutil::ParmFactory(PRM_STRING, "group",  "Group")
        .setDefault(0,
            "import os.path\n"
            "return os.path.splitext(os.path.basename(ch('file_name')))[0]",
            CH_PYTHON_EXPRESSION)
        .setTooltip("Specify a name for this group of VDBs."));

    // Missing Frame menu
    {
        char const * const items[] = {
            "error",    "Report Error",
            "empty",    "No Geometry",
            nullptr
        };

        parms.add(hutil::ParmFactory(PRM_ORD, "missingframe", "Missing Frame")
            .setDefault(PRMzeroDefaults)
            .setChoiceListItems(PRM_CHOICELIST_SINGLE, items)
            .setTooltip(
                "If the specified file does not exist on disk, either report an error"
                " (Report Error) or warn and continue (No Geometry)."));
    }

    // Reload button
    parms.add(hutil::ParmFactory(PRM_CALLBACK, "reload",  "Reload File")
        .setCallbackFunc(&reloadCB)
        .setTooltip("Reread the VDB file."));

    parms.add(hutil::ParmFactory(PRM_SEPARATOR, "sep1", "Sep"));

    // Delayed loading
    parms.add(hutil::ParmFactory(PRM_TOGGLE, "delayload", "Delay Loading")
        .setDefault(PRMoneDefaults)
        .setTooltip(
            "Don't allocate memory for or read voxel values until the values"
            " are actually accessed.\n\n"
            "Delayed loading can significantly lower memory usage, but\n"
            "note that viewport visualization of a volume usually requires\n"
            "the entire volume to be loaded into memory."));

    // Localization file size slider
    parms.add(hutil::ParmFactory(PRM_FLT_J, "copylimit", "Copy If Smaller Than")
        .setTypeExtended(PRM_TYPE_JOIN_PAIR)
        .setDefault(0.5f)
        .setRange(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_UI, 10)
        .setTooltip(
            "When delayed loading is enabled, a file must not be modified on disk before\n"
            "it has been fully read.  For safety, files smaller than the given size (in GB)\n"
            "will be copied to a private, temporary location (either $OPENVDB_TEMP_DIR,\n"
            "$TMPDIR or a system default temp directory).")
        .setDocumentation(
            "When delayed loading is enabled, a file must not be modified on disk before"
            " it has been fully read.  For safety, files smaller than the given size (in GB)"
            " will be copied to a private, temporary location (either `$OPENVDB_TEMP_DIR`,"
            " `$TMPDIR` or a system default temp directory)."));

    parms.add(hutil::ParmFactory(PRM_LABEL, "copylimitlabel", "GB")
        .setDocumentation(nullptr));

    // Register this operator.
    hvdb::OpenVDBOpFactory("VDB Read", SOP_OpenVDB_Read::factory, parms, *table)
        .setNativeName("")
        .addOptionalInput("Optional Bounding Geometry")
        .setDocumentation("\
#icon: COMMON/openvdb\n\
#tags: vdb\n\
\n\
\"\"\"Read a `.vdb` file from disk.\"\"\"\n\
\n\
@overview\n\
\n\
This node reads VDB volumes from a `.vdb` file.\n\
It is usually preferable to use Houdini's native [File|Node:sop/file] node,\n\
however unlike the native node, this node allows one to take advantage of\n\
delayed loading, meaning that only those portions of a volume that are\n\
actually accessed in a scene get loaded into memory.\n\
Delayed loading can significantly reduce memory usage when working\n\
with large volumes (but note that viewport visualization of a volume\n\
usually requires the entire volume to be loaded into memory).\n\
\n\
@related\n\
- [OpenVDB Write|Node:sop/DW_OpenVDBWrite]\n\
- [Node:sop/file]\n\
\n\
@examples\n\
\n\
See [openvdb.org|http://www.openvdb.org/download/] for source code\n\
and usage examples.\n");
}


////////////////////////////////////////


OP_Node*
SOP_OpenVDB_Read::factory(OP_Network* net,
    const char* name, OP_Operator* op)
{
    return new SOP_OpenVDB_Read(net, name, op);
}


SOP_OpenVDB_Read::SOP_OpenVDB_Read(OP_Network* net,
    const char* name, OP_Operator* op):
    hvdb::SOP_NodeVDB(net, name, op)
{
}


////////////////////////////////////////


// Disable parms in the UI.
bool
SOP_OpenVDB_Read::updateParmsFlags()
{
    bool changed = false;
    float t = 0.0;

    changed |= enableParm("group", bool(evalInt("enable_grouping", 0, t)));

    const bool delayedLoad = evalInt("delayload", 0, t);
    changed |= enableParm("copylimit", delayedLoad);
    changed |= enableParm("copylimitlabel", delayedLoad);

    return changed;
}


////////////////////////////////////////


OP_ERROR
SOP_OpenVDB_Read::cookVDBSop(OP_Context& context)
{
    try {
        hutil::ScopedInputLock lock(*this, context);

        gdp->clearAndDestroy();

        const fpreal t = context.getTime();

        const bool
            readMetadataOnly = evalInt("metadata_only", 0, t),
            missingFrameIsError = (0 == evalInt("missingframe", 0, t));

        // Get the file name string from the UI.
        const std::string filename = evalStdString("file_name", t);

        // Get the grid mask string.
        UT_String gridStr;
        evalString(gridStr, "grids", 0, t);

        // Get the group name string.
        UT_String groupStr;
        if (evalInt("enable_grouping", 0, t)) {
            evalString(groupStr, "group", 0, t);
            // If grouping is enabled but no group name was given, derive the group name
            // from the filename (e.g., "bar", given filename "/foo/bar.vdb").
            /// @internal Currently this is done with an expression on the parameter,
            /// but it could alternatively be done as follows:
            //if (!groupStr.isstring()) {
            //    groupStr = filename;
            //    groupStr = UT_String(groupStr.fileName());
            //    groupStr = groupStr.pathUpToExtension();
            //}
        }

        const bool delayedLoad = evalInt("delayload", 0, t);
        const openvdb::Index64 copyMaxBytes =
            openvdb::Index64(1.0e9 * evalFloat("copylimit", 0, t));

        openvdb::BBoxd clipBBox;
        bool clip = evalInt("clip", 0, t);
        if (clip) {
            if (const GU_Detail* clipGeo = inputGeo(0)) {
                UT_BoundingBox box;
                clipGeo->computeQuickBounds(box);
                clipBBox.min()[0] = box.xmin();
                clipBBox.min()[1] = box.ymin();
                clipBBox.min()[2] = box.zmin();
                clipBBox.max()[0] = box.xmax();
                clipBBox.max()[1] = box.ymax();
                clipBBox.max()[2] = box.zmax();
            }
            clip = clipBBox.isSorted();
        }

        UT_AutoInterrupt progress(("Reading " + filename).c_str());

        openvdb::io::File file(filename);
        openvdb::MetaMap::Ptr fileMetadata;
        try {
            // Open the VDB file, but don't read any grids yet.
            file.setCopyMaxBytes(copyMaxBytes);
            file.open(delayedLoad);

            // Read the file-level metadata.
            fileMetadata = file.getMetadata();
            if (!fileMetadata) fileMetadata.reset(new openvdb::MetaMap);

        } catch (std::exception& e) { ///< @todo consider catching only openvdb::IoError
            std::string mesg;
            if (const char* s = e.what()) mesg = s;
            // Strip off the exception name from an openvdb::IoError.
            if (mesg.substr(0, 9) == "IoError: ") mesg = mesg.substr(9);

            if (missingFrameIsError) {
                addError(SOP_MESSAGE, mesg.c_str());
            } else {
                addWarning(SOP_MESSAGE, mesg.c_str());
            }
            return error();
        }

        // Create a group for the grid primitives.
        GA_PrimitiveGroup* group = nullptr;
        if (groupStr.isstring()) {
            group = gdp->newPrimitiveGroup(groupStr.buffer());
        }

        // Loop over all grids in the file.
        for (openvdb::io::File::NameIterator nameIter = file.beginName();
            nameIter != file.endName(); ++nameIter)
        {
            if (progress.wasInterrupted()) throw std::runtime_error("Was Interrupted");

            // Skip grids whose names don't match the user-supplied mask.
            const std::string& gridName = nameIter.gridName();
            if (!UT_String(gridName).multiMatch(gridStr.buffer(), 1, " ")) continue;

            hvdb::GridPtr grid;
            if (readMetadataOnly) {
                grid = file.readGridMetadata(gridName);
            } else if (clip) {
                grid = file.readGrid(gridName, clipBBox);
            } else {
                grid = file.readGrid(gridName);
            }
            if (grid) {
                // Copy file-level metadata into the grid, then create (if necessary)
                // and set a primitive attribute for each metadata item.
                for (openvdb::MetaMap::ConstMetaIterator fileMetaIt = fileMetadata->beginMeta(),
                    end = fileMetadata->endMeta(); fileMetaIt != end; ++fileMetaIt)
                {
                    // Resolve file- and grid-level metadata name conflicts
                    // in favor of the grid-level metadata.
                    if (openvdb::Metadata::Ptr meta = fileMetaIt->second) {
                        const std::string name = fileMetaIt->first;
                        if (!(*grid)[name]) {
                            grid->insertMeta(name, *meta);
                        }
                    }
                }

                // Add a new VDB primitive for this grid.
                // Note: this clears the grid's metadata.
                GEO_PrimVDB* vdb = hvdb::createVdbPrimitive(*gdp, grid);

                // Add the primitive to the group.
                if (group) group->add(vdb);
            }
        }
        file.close();

        // If a group was created but no grids were added to it, delete the group.
        if (group && group->isEmpty()) gdp->destroyPrimitiveGroup(group);

    } catch (std::exception& e) {
        addError(SOP_MESSAGE, e.what());
    }
    return error();
}
