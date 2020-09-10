// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#include <cppunit/extensions/HelperMacros.h>
#include <openvdb/tools/ParticleAtlas.h>
#include <openvdb/math/Math.h>

#include <vector>
#include <algorithm>
#include <cmath>
#include "util.h" // for genPoints


struct TestParticleAtlas: public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(TestParticleAtlas);
    CPPUNIT_TEST(testParticleAtlas);
    CPPUNIT_TEST_SUITE_END();

    void testParticleAtlas();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestParticleAtlas);

////////////////////////////////////////

namespace {

class ParticleList
{
public:
    typedef openvdb::Vec3R          PosType;
    typedef PosType::value_type     ScalarType;

    ParticleList(const std::vector<PosType>& points,
        const std::vector<ScalarType>& radius)
        : mPoints(&points)
        , mRadius(&radius)
    {
    }

    // Return the number of points in the array
    size_t size() const {
        return mPoints->size();
    }

    // Return the world-space position for the nth particle.
    void getPos(size_t n, PosType& xyz) const {
        xyz = (*mPoints)[n];
    }

    // Return the world-space radius for the nth particle.
    void getRadius(size_t n, ScalarType& radius) const {
        radius = (*mRadius)[n];
    }

protected:
    std::vector<PosType>    const * const mPoints;
    std::vector<ScalarType> const * const mRadius;
}; // ParticleList


template<typename T>
bool hasDuplicates(const std::vector<T>& items)
{
    std::vector<T> vec(items);
    std::sort(vec.begin(), vec.end());

    size_t duplicates = 0;
    for (size_t n = 1, N = vec.size(); n < N; ++n) {
        if (vec[n] == vec[n-1]) ++duplicates;
    }
    return duplicates != 0;
}

} // namespace



////////////////////////////////////////


void
TestParticleAtlas::testParticleAtlas()
{
    // generate points

    const size_t numParticle = 40000;
    const double minVoxelSize = 0.01;

    std::vector<openvdb::Vec3R> points;
    unittest_util::genPoints(numParticle, points);

    std::vector<double> radius;
    for (size_t n = 0, N = points.size() / 2; n < N; ++n) {
        radius.push_back(minVoxelSize);
    }

    for (size_t n = points.size() / 2, N = points.size(); n < N; ++n) {
        radius.push_back(minVoxelSize * 2.0);
    }

    ParticleList particles(points, radius);

    // construct data structure

    typedef openvdb::tools::ParticleAtlas<> ParticleAtlas;

    ParticleAtlas atlas;

    CPPUNIT_ASSERT(atlas.empty());
    CPPUNIT_ASSERT(atlas.levels() == 0);

    atlas.construct(particles, minVoxelSize);

    CPPUNIT_ASSERT(!atlas.empty());
    CPPUNIT_ASSERT(atlas.levels() == 2);

    CPPUNIT_ASSERT(
        openvdb::math::isApproxEqual(atlas.minRadius(0), minVoxelSize));

    CPPUNIT_ASSERT(
        openvdb::math::isApproxEqual(atlas.minRadius(1), minVoxelSize * 2.0));

    typedef openvdb::tools::ParticleAtlas<>::Iterator ParticleAtlasIterator;

    ParticleAtlasIterator it(atlas);

    CPPUNIT_ASSERT(atlas.levels() == 2);

    std::vector<uint32_t> indices;
    indices.reserve(numParticle);

    it.updateFromLevel(0);

    CPPUNIT_ASSERT(it);
    CPPUNIT_ASSERT_EQUAL(it.size(), numParticle - (points.size() / 2));


    for (; it; ++it) {
        indices.push_back(*it);
    }

    it.updateFromLevel(1);

    CPPUNIT_ASSERT(it);
    CPPUNIT_ASSERT_EQUAL(it.size(), (points.size() / 2));


    for (; it; ++it) {
        indices.push_back(*it);
    }

    CPPUNIT_ASSERT_EQUAL(numParticle, indices.size());

    CPPUNIT_ASSERT(!hasDuplicates(indices));


    openvdb::Vec3R center = points[0];
    double searchRadius = minVoxelSize * 10.0;

    it.worldSpaceSearchAndUpdate(center, searchRadius, particles);
    CPPUNIT_ASSERT(it);

    indices.clear();
    for (; it; ++it) {
        indices.push_back(*it);
    }

    CPPUNIT_ASSERT_EQUAL(it.size(), indices.size());
    CPPUNIT_ASSERT(!hasDuplicates(indices));


    openvdb::BBoxd bbox;
    for (size_t n = 0, N = points.size() / 2; n < N; ++n) {
        bbox.expand(points[n]);
    }

    it.worldSpaceSearchAndUpdate(bbox, particles);
    CPPUNIT_ASSERT(it);

    indices.clear();
    for (; it; ++it) {
        indices.push_back(*it);
    }

    CPPUNIT_ASSERT_EQUAL(it.size(), indices.size());
    CPPUNIT_ASSERT(!hasDuplicates(indices));
}

