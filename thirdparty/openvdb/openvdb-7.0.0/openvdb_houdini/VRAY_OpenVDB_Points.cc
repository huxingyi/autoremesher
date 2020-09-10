// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

/// @file VRAY_OpenVDB_Points.cc
///
/// @authors Dan Bailey, Richard Kwok
///
/// @brief The Delayed Load Mantra Procedural for OpenVDB Points.

#include <UT/UT_Version.h>

#include <UT/UT_DSOVersion.h>
#include <GU/GU_Detail.h>
#include <OP/OP_OperatorTable.h>
#include <UT/UT_BoundingBox.h>
#include <UT/UT_Ramp.h>
#include <VRAY/VRAY_Procedural.h>
#include <VRAY/VRAY_ProceduralFactory.h>

#include <openvdb/openvdb.h>
#include <openvdb/io/File.h>
#include <openvdb/openvdb.h>
#include <openvdb/points/PointDataGrid.h>
#include <openvdb/points/PointGroup.h>
#include <openvdb_houdini/PointUtils.h>

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>


using namespace openvdb;
using namespace openvdb::points;
namespace hvdb = openvdb_houdini;


// mantra renders points with a world-space radius of 0.05 by default
static const float DEFAULT_PSCALE = 0.05f;

class VRAY_OpenVDB_Points : public VRAY_Procedural {
public:
    using GridVecPtr = std::vector<PointDataGrid::Ptr>;

    VRAY_OpenVDB_Points();
    ~VRAY_OpenVDB_Points() override = default;

    const char* className() const override;

    int initialize(const UT_BoundingBox*) override;
    void getBoundingBox(UT_BoundingBox&) override;
    void render() override;

private:
    UT_BoundingBox      mBox;
    UT_StringHolder     mFilename;
    std::vector<Name>   mIncludeGroups;
    std::vector<Name>   mExcludeGroups;
    UT_StringHolder     mAttrStr;
    GridVecPtr          mGridPtrs;
    float               mPreBlur;
    float               mPostBlur;
    bool                mSpeedToColor;
    float               mMaxSpeed;
    UT_Ramp             mFunctionRamp;

}; // class VRAY_OpenVDB_Points

////////////////////////////////////////

template <typename PointDataTreeT>
struct GenerateBBoxOp {

    using PointDataLeaf = typename PointDataTreeT::LeafNodeType;
    using LeafRangeT    = typename tree::LeafManager<const PointDataTreeT>::LeafRange;

    GenerateBBoxOp( const math::Transform& transform,
                    const std::vector<Name>& includeGroups,
                    const std::vector<Name>& excludeGroups)
        : mTransform(transform)
        , mIncludeGroups(includeGroups)
        , mExcludeGroups(excludeGroups) { }

    GenerateBBoxOp(const GenerateBBoxOp& parent, tbb::split)
        : mTransform(parent.mTransform)
        , mBbox(parent.mBbox)
        , mIncludeGroups(parent.mIncludeGroups)
        , mExcludeGroups(parent.mExcludeGroups) { }

    void operator()(const LeafRangeT& range) {

        for (auto leafIter = range.begin(); leafIter; ++leafIter) {

            const AttributeSet::Descriptor& descriptor = leafIter->attributeSet().descriptor();

            size_t pscaleIndex = descriptor.find("pscale");
            if (pscaleIndex != AttributeSet::INVALID_POS) {

                std::string pscaleType = descriptor.type(pscaleIndex).first;

                if (pscaleType == typeNameAsString<float>()) {
                    expandBBox<float>(*leafIter, pscaleIndex);
                } else if (pscaleType == typeNameAsString<half>()) {
                    expandBBox<half>(*leafIter, pscaleIndex);
                } else {
                    throw TypeError("Unsupported pscale type - " + pscaleType);
                }
            }
            else {
                // use default pscale value
                expandBBox<float>(*leafIter, pscaleIndex);
            }
        }
    }

    void join(GenerateBBoxOp& rhs) {
        mBbox.expand(rhs.mBbox);
    }

    template <typename PscaleType>
    void expandBBox(const PointDataLeaf& leaf, size_t pscaleIndex) {

        auto positionHandle =
            points::AttributeHandle<Vec3f>::create(leaf.constAttributeArray("P"));

        // expandBBox will not pick up a pscale handle unless
        // the attribute type matches the template type

        typename AttributeHandle<PscaleType>::Ptr pscaleHandle;
        if (pscaleIndex != AttributeSet::INVALID_POS) {
            if (leaf.attributeSet().descriptor().type(pscaleIndex).first
                == typeNameAsString<PscaleType>())
            {
                pscaleHandle =
                    AttributeHandle<PscaleType>::create(leaf.constAttributeArray(pscaleIndex));
            }
        }

        // uniform value is in world space
        bool pscaleIsUniform = true;
        PscaleType uniformPscale(DEFAULT_PSCALE);

        if (pscaleHandle) {
            pscaleIsUniform = pscaleHandle->isUniform();
            uniformPscale = pscaleHandle->get(0);
        }

        // combine the bounds of every point on this leaf into an index-space bbox

        if (!mIncludeGroups.empty() || !mExcludeGroups.empty()) {

            points::MultiGroupFilter filter(mIncludeGroups, mExcludeGroups, leaf.attributeSet());
            auto iter = leaf.beginIndexOn(filter);

            for (; iter; ++iter) {

                double pscale = double(pscaleIsUniform ? uniformPscale : pscaleHandle->get(*iter));

                // pscale needs to be transformed to index space
                Vec3d radius = mTransform.worldToIndex(Vec3d(pscale));
                Vec3d position = iter.getCoord().asVec3d() + positionHandle->get(*iter);

                mBbox.expand(position - radius);
                mBbox.expand(position + radius);
            }
        }
        else {

            auto iter = leaf.beginIndexOn();

            for (; iter; ++iter) {

                double pscale = double(pscaleIsUniform ? uniformPscale : pscaleHandle->get(*iter));

                // pscale needs to be transformed to index space
                Vec3d radius = mTransform.worldToIndex(Vec3d(pscale));
                Vec3d position = iter.getCoord().asVec3d() + positionHandle->get(*iter);

                mBbox.expand(position - radius);
                mBbox.expand(position + radius);

            }
        }
    }

    /////////////

    const math::Transform&      mTransform;
    BBoxd                       mBbox;
    const std::vector<Name>&    mIncludeGroups;
    const std::vector<Name>&    mExcludeGroups;

}; // GenerateBBoxOp

//////////////////////////////////////


template <typename PointDataTreeT>
struct PopulateColorFromVelocityOp {

    using LeafNode          = typename PointDataTreeT::LeafNodeType;
    using IndexOnIter       = typename LeafNode::IndexOnIter;
    using LeafManagerT      = typename tree::LeafManager<PointDataTreeT>;
    using LeafRangeT        = typename LeafManagerT::LeafRange;
    using MultiGroupFilter  = points::MultiGroupFilter;

    PopulateColorFromVelocityOp(    const size_t colorIndex,
                                    const size_t velocityIndex,
                                    const UT_Ramp& ramp,
                                    const float maxSpeed,
                                    const std::vector<Name>& includeGroups,
                                    const std::vector<Name>& excludeGroups)
        : mColorIndex(colorIndex)
        , mVelocityIndex(velocityIndex)
        , mRamp(ramp)
        , mMaxSpeed(maxSpeed)
        , mIncludeGroups(includeGroups)
        , mExcludeGroups(excludeGroups) { }

    Vec3f getColorFromRamp(const Vec3f& velocity) const{

        float proportionalSpeed = (mMaxSpeed == 0.0f ? 0.0f : velocity.length()/mMaxSpeed);

        if (proportionalSpeed > 1.0f)   proportionalSpeed = 1.0f;
        if (proportionalSpeed < 0.0f)   proportionalSpeed = 0.0f;

        float rampVal[4];
        mRamp.rampLookup(proportionalSpeed, rampVal);
        return Vec3f(rampVal[0], rampVal[1], rampVal[2]);
    }

    void operator()(LeafRangeT& range) const{

        for (auto leafIter = range.begin(); leafIter; ++leafIter) {

            auto& leaf = *leafIter;

            auto colorHandle =
                points::AttributeWriteHandle<Vec3f>::create(leaf.attributeArray(mColorIndex));

            auto velocityHandle =
                points::AttributeHandle<Vec3f>::create(leaf.constAttributeArray(mVelocityIndex));

            const bool uniform = velocityHandle->isUniform();
            const Vec3f uniformColor = getColorFromRamp(velocityHandle->get(0));

            MultiGroupFilter filter(mIncludeGroups, mExcludeGroups, leaf.attributeSet());
            if (filter.state() == points::index::ALL) {
                for (auto iter = leaf.beginIndexOn(); iter; ++iter) {
                    Vec3f color = uniform ?
                        uniformColor : getColorFromRamp(velocityHandle->get(*iter));
                    colorHandle->set(*iter, color);
                }
            }
            else {
                for (auto iter = leaf.beginIndexOn(filter); iter; ++iter) {
                    Vec3f color = uniform ?
                        uniformColor : getColorFromRamp(velocityHandle->get(*iter));
                    colorHandle->set(*iter, color);
                }
            }
        }
    }

    //////////////////////////////////////////////

    const size_t                            mColorIndex;
    const size_t                            mVelocityIndex;
    const UT_Ramp&                          mRamp;
    const float                             mMaxSpeed;
    const std::vector<Name>&                mIncludeGroups;
    const std::vector<Name>&                mExcludeGroups;
};

////////////////////////////////////////////


namespace {

template <typename PointDataGridT>
inline BBoxd
getBoundingBox( const std::vector<typename PointDataGridT::Ptr>& gridPtrs,
                const std::vector<Name>& includeGroups,
                const std::vector<Name>& excludeGroups)
{
    using PointDataTree     = typename PointDataGridT::TreeType;

    BBoxd worldBounds;

    for (const auto& grid : gridPtrs) {

        tree::LeafManager<const PointDataTree> leafManager(grid->tree());

        // size and combine the boxes for each leaf in the tree via a reduction
        GenerateBBoxOp<PointDataTree> generateBbox(grid->transform(), includeGroups, excludeGroups);
        tbb::parallel_reduce(leafManager.leafRange(), generateBbox);

        if (generateBbox.mBbox.empty())     continue;

        // all the bounds must be unioned in world space
        BBoxd gridBounds = grid->transform().indexToWorld(generateBbox.mBbox);
        worldBounds.expand(gridBounds);
    }

    return worldBounds;
}

} // namespace

static VRAY_ProceduralArg   theArgs[] = {
    VRAY_ProceduralArg("file", "string", ""),
    VRAY_ProceduralArg("streamdata", "int", "1"),
    VRAY_ProceduralArg("groupmask", "string", ""),
    VRAY_ProceduralArg("attrmask", "string", ""),
    VRAY_ProceduralArg("speedtocolor", "int", "0"),
    VRAY_ProceduralArg("maxspeed", "real", "1.0"),
    VRAY_ProceduralArg("ramp", "string", ""),
    VRAY_ProceduralArg()
};

class ProcDef : public VRAY_ProceduralFactory::ProcDefinition
{
public:
    ProcDef()
    : VRAY_ProceduralFactory::ProcDefinition("openvdb_points")
    {
    }
    virtual VRAY_Procedural *create() const { return new VRAY_OpenVDB_Points(); }
    virtual VRAY_ProceduralArg  *arguments() const { return theArgs; }
};

void
registerProcedural(VRAY_ProceduralFactory *factory)
{
    factory->insert(new ProcDef);
}

VRAY_OpenVDB_Points::VRAY_OpenVDB_Points()
{
    openvdb::initialize();
}

const char *
VRAY_OpenVDB_Points::className() const
{
    return "VRAY_OpenVDB_Points";
}

int
VRAY_OpenVDB_Points::initialize(const UT_BoundingBox *)
{
    struct Local
    {
        static GridVecPtr loadGrids(const std::string& filename, const bool stream)
        {
            GridVecPtr grids;

            // save the grids so that we only read the file once
            try
            {
                io::File file(filename);
                file.open();

                for (auto iter=file.beginName(), endIter=file.endName(); iter != endIter; ++iter) {

                    GridBase::Ptr baseGrid = file.readGridMetadata(*iter);
                    if (baseGrid->isType<points::PointDataGrid>()) {
                        auto grid = StaticPtrCast<points::PointDataGrid>(file.readGrid(*iter));
                        assert(grid);
                        if (stream) {
                            // enable streaming mode to auto-collapse attributes
                            // on read for improved memory efficiency
                            points::setStreamingMode(grid->tree(), /*on=*/true);
                        }
                        grids.push_back(grid);
                    }
                }

                file.close();
            }
            catch (const IoError& e)
            {
                OPENVDB_LOG_ERROR(e.what() << " (" << filename << ")");
            }

            return grids;
        }
    };

    import("file", mFilename);

    int streamData;
    import("streamdata", &streamData, 1);
    import("attrmask", mAttrStr);

    float fps;
    import("global:fps", &fps, 1);

    float shutter[2];
    import("camera:shutter", shutter, 2);

    int velocityBlur;
    import("object:velocityblur", &velocityBlur, 1);

    mPreBlur = velocityBlur ? -shutter[0]/fps : 0;
    mPostBlur = velocityBlur ? shutter[1]/fps : 0;

    int speedToColorInt = 0;
    import("speedtocolor", &speedToColorInt, 1);
    mSpeedToColor = bool(speedToColorInt);

    // if speed-to-color is enabled we need to build a ramp object
    if (mSpeedToColor) {

        import("maxspeed", &mMaxSpeed, 1);

        UT_StringHolder rampStr;
        import("ramp", rampStr);

        std::stringstream rampStream(rampStr.toStdString());
        std::istream_iterator<float> begin(rampStream);
        std::istream_iterator<float> end;
        std::vector<float> rampVals(begin, end);

        for (size_t n = 4, N = rampVals.size(); n < N; n += 5) {
            mFunctionRamp.addNode(rampVals[n-4],
                UT_FRGBA(rampVals[n-3], rampVals[n-2], rampVals[n-1], 1.0f),
                static_cast<UT_SPLINE_BASIS>(rampVals[n]));
        }
    }

    mGridPtrs = Local::loadGrids(mFilename.toStdString(), streamData ? true : false);

    // extract which groups to include and exclude
    UT_StringHolder groupStr;
    import("groupmask", groupStr);
    AttributeSet::Descriptor::parseNames(mIncludeGroups, mExcludeGroups, groupStr.toStdString());

    // get openvdb bounds and convert to houdini bounds
    BBoxd vdbBox = ::getBoundingBox<PointDataGrid>(mGridPtrs, mIncludeGroups, mExcludeGroups);
    mBox.setBounds(
        static_cast<float>(vdbBox.min().x()),
        static_cast<float>(vdbBox.min().y()),
        static_cast<float>(vdbBox.min().z()),
        static_cast<float>(vdbBox.max().x()),
        static_cast<float>(vdbBox.max().y()),
        static_cast<float>(vdbBox.max().z()));

    // if streaming the data, re-open the file now that the bounding box has been computed
    if (streamData) {
        mGridPtrs = Local::loadGrids(mFilename.toStdString(), true);
    }

    return 1;
}

void
VRAY_OpenVDB_Points::getBoundingBox(UT_BoundingBox &box)
{
    box = mBox;
}

void
VRAY_OpenVDB_Points::render()
{
    using PointDataTree     = points::PointDataGrid::TreeType;
    using AttributeSet      = points::AttributeSet;
    using Descriptor        = AttributeSet::Descriptor;

    /// Allocate geometry and extract the GU_Detail
    VRAY_ProceduralGeo  geo = createGeometry();

    GU_Detail* gdp = geo.get();

    // extract which attributes to include and exclude
    std::vector<Name> includeAttributes;
    std::vector<Name> excludeAttributes;
    AttributeSet::Descriptor::parseNames(
        includeAttributes, excludeAttributes, mAttrStr.toStdString());

    // if nothing was included or excluded: "all attributes" is implied with an empty vector
    // if nothing was included but something was explicitly excluded:
    //     add all attributes but then remove the excluded
    // if something was included:
    //     add only explicitly included attributes and then removed any excluded

    if (includeAttributes.empty() && !excludeAttributes.empty()) {

        // add all attributes
        for (const auto& grid : mGridPtrs) {

            auto leafIter = grid->tree().cbeginLeaf();
            if (!leafIter) continue;

            const AttributeSet& attributeSet = leafIter->attributeSet();
            const Descriptor& descriptor = attributeSet.descriptor();
            const Descriptor::NameToPosMap& nameToPosMap = descriptor.map();

            for (const auto& namePos : nameToPosMap) {

                includeAttributes.push_back(namePos.first);
            }
        }
    }

    // sort, and then remove any duplicates
    std::sort(includeAttributes.begin(), includeAttributes.end());
    std::sort(excludeAttributes.begin(), excludeAttributes.end());
    includeAttributes.erase(
        std::unique(includeAttributes.begin(), includeAttributes.end()), includeAttributes.end());
    excludeAttributes.erase(
        std::unique(excludeAttributes.begin(), excludeAttributes.end()), excludeAttributes.end());

    // make a vector (validAttributes) of all elements that are in includeAttributes
    // but are NOT in excludeAttributes
    std::vector<Name> validAttributes(includeAttributes.size());
    auto pastEndIter = std::set_difference(includeAttributes.begin(), includeAttributes.end(),
        excludeAttributes.begin(), excludeAttributes.end(), validAttributes.begin());
    validAttributes.resize(pastEndIter - validAttributes.begin());

    // if any of the grids are going to add a pscale, set the default here
    if (std::binary_search(validAttributes.begin(), validAttributes.end(), "pscale")) {
        gdp->addTuple(GA_STORE_REAL32, GA_ATTRIB_POINT, "pscale", 1, GA_Defaults(DEFAULT_PSCALE));
    }

    // map speed to color if requested
    if (mSpeedToColor) {
        for (const auto& grid : mGridPtrs) {

            PointDataTree& tree = grid->tree();

            auto leafIter = tree.beginLeaf();
            if (!leafIter) continue;

            size_t velocityIndex = leafIter->attributeSet().find("v");
            if (velocityIndex != AttributeSet::INVALID_POS) {

                const std::string velocityType =
                    leafIter->attributeSet().descriptor().type(velocityIndex).first;

                // keep existing Cd attribute only if it is a supported type (float or half)
                size_t colorIndex = leafIter->attributeSet().find("Cd");
                std::string colorType = "";
                if (colorIndex != AttributeSet::INVALID_POS) {
                    colorType = leafIter->attributeSet().descriptor().type(colorIndex).first;
                    if (colorType != typeNameAsString<Vec3f>()
                        && colorType != typeNameAsString<Vec3H>())
                    {
                        dropAttribute(tree, "Cd");
                        colorIndex = AttributeSet::INVALID_POS;
                    }
                }

                // create new Cd attribute if one did not previously exist
                if (colorIndex == AttributeSet::INVALID_POS) {
                    openvdb::points::appendAttribute<Vec3f, FixedPointCodec<false, UnitRange>>(
                        tree, "Cd");
                    colorIndex = leafIter->attributeSet().find("Cd");
                }

                tree::LeafManager<PointDataTree> leafManager(tree);

                PopulateColorFromVelocityOp<PointDataTree> populateColor(colorIndex, velocityIndex,
                    mFunctionRamp, mMaxSpeed, mIncludeGroups, mExcludeGroups);
                tbb::parallel_for(leafManager.leafRange(), populateColor);
            }
        }
    }

    for (const auto& grid : mGridPtrs) {

        hvdb::convertPointDataGridToHoudini(
            *gdp, *grid, validAttributes, mIncludeGroups, mExcludeGroups);
    }

    geo.addVelocityBlur(mPreBlur, mPostBlur);

    // Create a geometry object in mantra
    VRAY_ProceduralChildPtr obj = createChild();
    obj->addGeometry(geo);

    // Override the renderpoints setting to always enable points only rendering
    int one = 1;
    obj->changeSetting("renderpoints", 1, &one);
}

