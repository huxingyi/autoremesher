// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file SOP_OpenVDB_Noise.cc
///
/// @author FX R&D OpenVDB team
///
/// @brief Applies noise to level sets represented by VDBs. The noise can
/// optionally be masked by another level set

#include <houdini_utils/ParmFactory.h>
#include <openvdb_houdini/Utils.h>
#include <openvdb_houdini/SOP_NodeVDB.h>
#include <openvdb/math/Operators.h>
#include <openvdb/math/Stencils.h>
#include <openvdb/tools/Interpolation.h> // for box sampler
#include <UT/UT_PNoise.h>
#include <UT/UT_Interrupt.h>
#include <sstream>
#include <stdexcept>



namespace hvdb = openvdb_houdini;
namespace hutil = houdini_utils;
namespace cvdb = openvdb;


namespace { // anon namespace

struct FractalBoltzmannGenerator
{
    FractalBoltzmannGenerator(float freq, float amp, int octaves, float gain,
        float lacunarity, float roughness, int mode):
        mOctaves(octaves), mNoiseMode(mode), mFreq(freq), mAmp(amp), mGain(gain),
        mLacunarity(lacunarity), mRoughness(roughness)
    {}

    // produce the noise as float
    float noise(cvdb::Vec3R point, float freqMult = 1.0f) const
    {
        float signal;
        float result = 0.0f;
        float curamp = mAmp;
        float curfreq = mFreq * freqMult;

        for (int n = 0; n <= mOctaves; n++) {
            point = (point * curfreq);

            // convert to float for UT_PNoise
            float location[3] = { float(point[0]), float(point[1]), float(point[2]) };

            // generate noise in the [-1,1] range
            signal = 2.0f*UT_PNoise::noise3D(location) - 1.0f;

            if (mNoiseMode > 0) {
                signal = cvdb::math::Pow(cvdb::math::Abs(signal), mGain);
            }

            result  += (signal * curamp);
            curfreq = mLacunarity;
            curamp *= mRoughness;
        }
        if (mNoiseMode == 1) {
            result = -result;
        }

        return result;
    }

private:
    // member data
    int mOctaves;
    int mNoiseMode;
    float mFreq;
    float mAmp;
    float mGain;
    float mLacunarity;
    float mRoughness;

};

struct NoiseSettings
{
    NoiseSettings() : mMaskMode(0), mOffset(0.0), mThreshold(0.0),
        mFallOff(0.0), mNOffset(cvdb::Vec3R(0.0, 0.0, 0.0)) {}

    int mMaskMode;
    float mOffset, mThreshold, mFallOff;
    cvdb::Vec3R mNOffset;
};

} // anon namespace


////////////////////////////////////////


class SOP_OpenVDB_Noise: public hvdb::SOP_NodeVDB
{
public:
    SOP_OpenVDB_Noise(OP_Network*, const char* name, OP_Operator*);
    ~SOP_OpenVDB_Noise() override {}

    static OP_Node* factory(OP_Network*, const char*, OP_Operator*);

    int isRefInput(unsigned input) const override { return (input == 1); }

    class Cache: public SOP_VDBCacheOptions
    {
    protected:
        OP_ERROR cookVDBSop(OP_Context&) override;
    private:
        // Process the given grid and return the output grid.
        // Can be applied to FloatGrid or DoubleGrid
        // this contains the majority of the work
        template<typename GridType>
        void applyNoise(hvdb::Grid& grid, const FractalBoltzmannGenerator&,
            const NoiseSettings&, const hvdb::Grid* maskGrid) const;
    }; // class Cache

protected:
    bool updateParmsFlags() override;
};


////////////////////////////////////////


void
newSopOperator(OP_OperatorTable* table)
{
    if (table == nullptr) return;

    hutil::ParmList parms;

    // Define a string-valued group name pattern parameter and add it to the list.
    parms.add(hutil::ParmFactory(PRM_STRING, "group", "Group")
        .setChoiceList(&hutil::PrimGroupMenuInput1)
        .setTooltip("Specify a subset of the input VDB grids to be processed.")
        .setDocumentation(
            "A subset of the input VDBs to be processed"
            " (see [specifying volumes|/model/volumes#group])"));

    // amplitude
    parms.add(hutil::ParmFactory(PRM_FLT_J, "amp", "Amplitude")
        .setDefault(PRMoneDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 0.0, PRM_RANGE_FREE, 10.0)
        .setTooltip("The amplitude of the noise"));

    // frequency
    parms.add(hutil::ParmFactory(PRM_FLT_J, "freq", "Frequency")
        .setDefault(PRMoneDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 0.0, PRM_RANGE_FREE, 1.0)
        .setTooltip("The frequency of the noise"));

    // Octaves
    parms.add(hutil::ParmFactory(PRM_INT_J, "oct", "Octaves")
        .setDefault(PRMzeroDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_FREE, 10)
        .setTooltip("The number of octaves for the noise"));

    // Lacunarity
    parms.add(hutil::ParmFactory(PRM_FLT_J, "lac", "Lacunarity")
        .setDefault(PRMtwoDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 0.0, PRM_RANGE_FREE, 10.0)
        .setTooltip("The lacunarity of the noise"));

    // Gain
    parms.add(hutil::ParmFactory(PRM_FLT_J, "gain", "Gain")
        .setDefault(PRMoneDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 0.0, PRM_RANGE_FREE, 1.0)
        .setTooltip("The gain of the noise"));

    // Roughness
    parms.add(hutil::ParmFactory(PRM_FLT_J, "rough", "Roughness")
        .setDefault(PRMoneDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 0.0, PRM_RANGE_RESTRICTED, 10.0)
        .setTooltip("The roughness of the noise"));

    // SurfaceOffset
    parms.add(hutil::ParmFactory(PRM_FLT_J, "soff", "Surface Offset")
        .setDefault(PRMzeroDefaults)
        .setTooltip("An offset from the isosurface of the level set at which to apply the noise"));

    // Noise Offset
    parms.add(hutil::ParmFactory(PRM_XYZ_J, "noff", "Noise Offset")
        .setVectorSize(3)
        .setDefault(PRMzeroDefaults)
        .setTooltip("An offset for the noise in world units"));

    // Noise Mode
    parms.add(hutil::ParmFactory(PRM_ORD, "mode", "Noise Mode")
        .setDefault(PRMzeroDefaults)
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, {
            "straight", "Straight",
            "abs",      "Absolute",
            "invabs",   "Inverse Absolute"
        })
        .setTooltip("The noise mode: either Straight, Absolute, or Inverse Absolute"));

    // Mask {
    parms.add(hutil::ParmFactory(PRM_HEADING, "maskHeading", "Mask"));

    // Group
    parms.add(hutil::ParmFactory(PRM_STRING, "maskGroup", "Mask Group")
        .setChoiceList(&hutil::PrimGroupMenuInput2)
        .setDocumentation(
            "A scalar VDB from the second input to be used as a mask"
            " (see [specifying volumes|/model/volumes#group])"));

    // Use mask
    parms.add(hutil::ParmFactory(PRM_ORD, "mask", "Mask")
        .setDefault(PRMzeroDefaults)
        .setChoiceListItems(PRM_CHOICELIST_SINGLE, {
            "maskless",       "No noise if mask < threshold",
            "maskgreater",    "No noise if mask > threshold",
            "maskgreaternml", "No noise if mask > threshold & normals align",
            "maskisfreqmult", "Use mask as frequency multiplier"
        })
        .setTooltip("How to interpret the mask")
        .setDocumentation("\
How to interpret the mask\n\
\n\
No noise if mask < threshold:\n\
    Don't add noise to a voxel if the mask value at that voxel\n\
    is less than the __Mask Threshold__.\n\
No noise if mask > threshold:\n\
    Don't add noise to a voxel if the mask value at that voxel\n\
    is greater than the __Mask Threshold__.\n\
No noise if mask > threshold & normals align:\n\
    Don't add noise to a voxel if the mask value at that voxel\n\
    is greater than the __Mask Threshold__ and the surface normal\n\
    of the level set at that voxel aligns with the gradient of the mask.\n\
Use mask as frequency multiplier:\n\
    Add noise to every voxel, but multiply the noise frequency by the mask.\n"));

    // mask threshold
    parms.add(hutil::ParmFactory(PRM_FLT_J, "thres", "Mask Threshold")
        .setDefault(PRMzeroDefaults)
        .setTooltip("The threshold value for mask comparisons"));

    // Fall off
    parms.add(hutil::ParmFactory(PRM_FLT_J, "fall", "Falloff")
        .setDefault(PRMzeroDefaults)
        .setRange(PRM_RANGE_RESTRICTED, 0.0, PRM_RANGE_FREE, 10.0)
        .setTooltip("A falloff value for the threshold"));
    // }

    // Register this operator.
    hvdb::OpenVDBOpFactory("VDB Noise", SOP_OpenVDB_Noise::factory, parms, *table)
        .setNativeName("")
        .addInput("VDB grids to noise")
        .addOptionalInput("Optional VDB grid to use as mask")
        .setVerb(SOP_NodeVerb::COOK_INPLACE, []() { return new SOP_OpenVDB_Noise::Cache; })
        .setDocumentation("\
#icon: COMMON/openvdb\n\
#tags: vdb\n\
\n\
\"\"\"Add noise to VDB level sets.\"\"\"\n\
\n\
@overview\n\
\n\
Using a fractal Boltzmann generator, this node adds surface noise\n\
to VDB level set volumes.\n\
An optional mask grid can be provided to control the amount of noise per voxel.\n\
\n\
@related\n\
- [Node:sop/cloudnoise]\n\
- [Node:sop/volumevop]\n\
\n\
@examples\n\
\n\
See [openvdb.org|http://www.openvdb.org/download/] for source code\n\
and usage examples.\n");
}


////////////////////////////////////////


OP_Node*
SOP_OpenVDB_Noise::factory(OP_Network* net, const char* name, OP_Operator *op)
{
    return new SOP_OpenVDB_Noise(net, name, op);
}


SOP_OpenVDB_Noise::SOP_OpenVDB_Noise(OP_Network* net, const char* name, OP_Operator* op):
    SOP_NodeVDB(net, name, op)
{
    UT_PNoise::initNoise();
}


////////////////////////////////////////


bool
SOP_OpenVDB_Noise::updateParmsFlags()
{

    bool changed = false;

    const GU_Detail* refGdp = this->getInputLastGeo(1, /*time=*/0.0);
    const bool hasSecondInput = (refGdp != nullptr);

    changed |= enableParm("maskGroup", hasSecondInput);
    changed |= enableParm("mask", hasSecondInput);
    changed |= enableParm("thres", hasSecondInput);
    changed |= enableParm("fall", hasSecondInput);

    return changed;
}


////////////////////////////////////////


template<typename GridType>
void
SOP_OpenVDB_Noise::Cache::applyNoise(
    hvdb::Grid& grid,
    const FractalBoltzmannGenerator& fbGenerator,
    const NoiseSettings& settings,
    const hvdb::Grid* mask) const
{
    // Use second order finite difference.
    using Gradient = cvdb::math::Gradient<cvdb::math::GenericMap, cvdb::math::CD_2ND>;
    using CPT = cvdb::math::CPT<cvdb::math::GenericMap, cvdb::math::CD_2ND>;
    using StencilType = cvdb::math::SecondOrderDenseStencil<GridType>;

    using TreeType = typename GridType::TreeType;
    using Vec3Type = cvdb::math::Vec3<typename TreeType::ValueType>;

    // Down cast the generic pointer to the output grid.
    GridType& outGrid = UTvdbGridCast<GridType>(grid);

    const cvdb::math::Transform& xform = grid.transform();

    // Create a stencil.
    StencilType stencil(outGrid); // uses its own grid accessor

    // scratch variables
    typename GridType::ValueType result; // result - use mask as frequency multiplier
    cvdb::Vec3R voxelPt; // voxel coordinates
    cvdb::Vec3R worldPt; // world coordinates
    float noise, alpha;

    // The use of the GenericMap is a performance compromise
    // because the GenericMap holdds a base class pointer.
    // This should be optimized by resolving the acutal map type
    cvdb::math::GenericMap map(grid);

    if (!mask) {
        alpha = 1.0f;
        for (typename GridType::ValueOnIter v = outGrid.beginValueOn(); v; ++v) {
            stencil.moveTo(v);
            worldPt = xform.indexToWorld(CPT::result(map, stencil) + settings.mNOffset);
            noise = fbGenerator.noise(worldPt);
            v.setValue(*v + alpha * (noise - settings.mOffset));
        }
        return;
    }

    // Down cast the generic pointer to the mask grid.
    const GridType* maskGrid = UTvdbGridCast<GridType>(mask);
    const cvdb::math::Transform& maskXform = mask->transform();

    switch (settings.mMaskMode) {
        case 0: //No noise if mask < threshold
        {
            for (typename GridType::ValueOnIter v = outGrid.beginValueOn(); v; ++v) {
                cvdb::Coord ijk = v.getCoord();
                stencil.moveTo(ijk); // in voxel units

                worldPt = xform.indexToWorld(ijk);
                voxelPt = maskXform.worldToIndex(worldPt);
                cvdb::tools::BoxSampler::sample<TreeType>(maskGrid->tree(), voxelPt, result);

                // apply threshold
                if (result < settings.mThreshold) {
                    continue; //next voxel
                }
                alpha = static_cast<float>(result >= settings.mThreshold + settings.mFallOff ?
                    1.0f : (result - settings.mThreshold) / settings.mFallOff);

                worldPt = xform.indexToWorld(CPT::result(map, stencil) + settings.mNOffset);
                noise = fbGenerator.noise(worldPt);
                v.setValue(*v + alpha * (noise - settings.mOffset));
            }
        }
        break;

        case 1: //No noise if mask > threshold
        {
            for (typename GridType::ValueOnIter v = outGrid.beginValueOn(); v; ++v) {
                cvdb::Coord ijk = v.getCoord();
                stencil.moveTo(ijk); // in voxel units

                worldPt = xform.indexToWorld(ijk);
                voxelPt = maskXform.worldToIndex(worldPt);
                cvdb::tools::BoxSampler::sample<TreeType>(maskGrid->tree(), voxelPt, result);

                // apply threshold
                if (result > settings.mThreshold) {
                    continue; //next voxel
                }
                alpha = static_cast<float>(result <= settings.mThreshold - settings.mFallOff ?
                    1.0f : (settings.mThreshold - result) / settings.mFallOff);

                worldPt = xform.indexToWorld(CPT::result(map, stencil) + settings.mNOffset);
                noise = fbGenerator.noise(worldPt);
                v.setValue(*v + alpha * (noise - settings.mOffset));
            }
        }
        break;

        case 2: //No noise if mask < threshold & normals align
        {
            StencilType maskStencil(*maskGrid);
            for (typename GridType::ValueOnIter v = outGrid.beginValueOn(); v; ++v) {
                cvdb::Coord ijk = v.getCoord();
                stencil.moveTo(ijk); // in voxel units

                worldPt = xform.indexToWorld(ijk);
                voxelPt = maskXform.worldToIndex(worldPt);
                cvdb::tools::BoxSampler::sample<TreeType>(maskGrid->tree(), voxelPt, result);

                // for the gradient of the maskGrid
                cvdb::Coord mask_ijk(
                    static_cast<int>(voxelPt[0]),
                    static_cast<int>(voxelPt[1]),
                    static_cast<int>(voxelPt[2]));
                maskStencil.moveTo(mask_ijk);
                // normal alignment
                Vec3Type grid_grad = Gradient::result(map, stencil);
                Vec3Type mask_grad = Gradient::result(map, maskStencil);
                const double c = cvdb::math::Abs(grid_grad.dot(mask_grad));

                if (result > settings.mThreshold && c > 0.9) continue;//next voxel
                alpha = static_cast<float>(result <= settings.mThreshold - settings.mFallOff ?
                    1.0f : (settings.mThreshold - result) / settings.mFallOff);

                worldPt = xform.indexToWorld(CPT::result(map, stencil) + settings.mNOffset);
                noise = fbGenerator.noise(worldPt);
                v.setValue(*v + alpha * (noise - settings.mOffset));
            }
        }
        break;

        case 3: //Use mask as frequency multiplier
        {
            alpha = 1.0f;
            for (typename GridType::ValueOnIter v = outGrid.beginValueOn(); v; ++v) {
                cvdb::Coord ijk = v.getCoord();
                stencil.moveTo(ijk); // in voxel units

                worldPt = xform.indexToWorld(ijk);
                voxelPt = maskXform.worldToIndex(worldPt);
                cvdb::tools::BoxSampler::sample<TreeType>(maskGrid->tree(), voxelPt, result);

                worldPt = xform.indexToWorld(CPT::result(map, stencil) + settings.mNOffset);
                // Use result of sample as frequency multiplier.
                noise = fbGenerator.noise(worldPt, static_cast<float>(result));
                v.setValue(*v + alpha * (noise - settings.mOffset));
            }
        }
        break;

        default: // should never get here
            throw std::runtime_error("internal error in mode selection");
    }// end switch
}


////////////////////////////////////////


OP_ERROR
SOP_OpenVDB_Noise::Cache::cookVDBSop(OP_Context& context)
{
    try {
        const fpreal time = context.getTime();

        // Evaluate the FractalBoltzmann noise parameters from UI
        FractalBoltzmannGenerator fbGenerator(
            static_cast<float>(evalFloat("freq", 0, time)),
            static_cast<float>(evalFloat("amp", 0, time)),
            static_cast<int>(evalInt("oct", 0, time)),
            static_cast<float>(evalFloat("gain", 0, time)),
            static_cast<float>(evalFloat("lac", 0, time)),
            static_cast<float>(evalFloat("rough", 0, time)),
            static_cast<int>(evalInt("mode", 0, time)));

        NoiseSettings settings;

        // evaluate parameter for blending noise
        settings.mOffset = static_cast<float>(evalFloat("soff", 0, time));
        settings.mNOffset = cvdb::Vec3R(
            evalFloat("noff", 0, time),
            evalFloat("noff", 1, time),
            evalFloat("noff", 2, time));

        // Mask
        const openvdb::GridBase* maskGrid = nullptr;
        if (const GU_Detail* refGdp = inputGeo(1)) {
            const GA_PrimitiveGroup* maskGroup =
                matchGroup(*refGdp, evalStdString("maskGroup", time));

            hvdb::VdbPrimCIterator gridIter(refGdp, maskGroup);

            if (gridIter) {
                settings.mMaskMode = static_cast<int>(evalInt("mask", 0, time));
                settings.mThreshold = static_cast<float>(evalFloat("thres", 0, time));
                settings.mFallOff = static_cast<float>(evalFloat("fall", 0, time));

                maskGrid = &((*gridIter)->getGrid());
                ++gridIter;
            }

            if (gridIter) {
                addWarning(SOP_MESSAGE,
                    "Found more than one grid in the mask group; the first grid will be used.");
            }
        }

        // Do the work..
        UT_AutoInterrupt progress("OpenVDB LS Noise");

        // Get the group of grids to process.
        const GA_PrimitiveGroup* group = matchGroup(*gdp, evalStdString("group", time));

        // For each VDB primitive in the selected group.
        for (hvdb::VdbPrimIterator it(gdp, group); it; ++it) {

            if (progress.wasInterrupted()) {
                throw std::runtime_error("processing was interrupted");
            }

            GU_PrimVDB* vdbPrim = *it;

            if (vdbPrim->getStorageType() == UT_VDB_FLOAT) {
                vdbPrim->makeGridUnique();
                applyNoise<cvdb::ScalarGrid>(vdbPrim->getGrid(), fbGenerator, settings, maskGrid);

            } else if (vdbPrim->getStorageType() == UT_VDB_DOUBLE) {
                vdbPrim->makeGridUnique();
                applyNoise<cvdb::DoubleGrid>(vdbPrim->getGrid(), fbGenerator, settings, maskGrid);

            } else {
                std::stringstream ss;
                ss << "VDB primitive " << it.getPrimitiveNameOrIndex()
                    << " was skipped because it is not a scalar grid.";
                addWarning(SOP_MESSAGE, ss.str().c_str());
                continue;
            }
        }

    } catch (std::exception& e) {
        addError(SOP_MESSAGE, e.what());
    }
    return error();
}
