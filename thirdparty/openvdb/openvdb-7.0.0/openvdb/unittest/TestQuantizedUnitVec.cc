// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#include <cppunit/extensions/HelperMacros.h>
#include <openvdb/Exceptions.h>
#include <openvdb/math/QuantizedUnitVec.h>
#include <openvdb/math/Math.h>
#include <openvdb/math/Vec3.h>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <ctime>


class TestQuantizedUnitVec: public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(TestQuantizedUnitVec);
    CPPUNIT_TEST(testQuantization);
    CPPUNIT_TEST_SUITE_END();

    void testQuantization();

private:
    // Generate a random number in the range [0, 1].
    double randNumber() { return double(rand()) / (double(RAND_MAX) + 1.0); }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestQuantizedUnitVec);


////////////////////////////////////////


namespace {
const uint16_t
    MASK_XSIGN = 0x8000, // 1000000000000000
    MASK_YSIGN = 0x4000, // 0100000000000000
    MASK_ZSIGN = 0x2000; // 0010000000000000
}


////////////////////////////////////////


void
TestQuantizedUnitVec::testQuantization()
{
    using namespace openvdb;
    using namespace openvdb::math;

    //
    // Check sign bits
    //
    Vec3s unitVec = Vec3s(-1.0, -1.0, -1.0);
    unitVec.normalize();

    uint16_t quantizedVec = QuantizedUnitVec::pack(unitVec);

    CPPUNIT_ASSERT((quantizedVec & MASK_XSIGN));
    CPPUNIT_ASSERT((quantizedVec & MASK_YSIGN));
    CPPUNIT_ASSERT((quantizedVec & MASK_ZSIGN));

    unitVec[0] = -unitVec[0];
    unitVec[2] = -unitVec[2];
    quantizedVec = QuantizedUnitVec::pack(unitVec);

    CPPUNIT_ASSERT(!(quantizedVec & MASK_XSIGN));
    CPPUNIT_ASSERT((quantizedVec & MASK_YSIGN));
    CPPUNIT_ASSERT(!(quantizedVec & MASK_ZSIGN));

    unitVec[1] = -unitVec[1];
    quantizedVec = QuantizedUnitVec::pack(unitVec);

    CPPUNIT_ASSERT(!(quantizedVec & MASK_XSIGN));
    CPPUNIT_ASSERT(!(quantizedVec & MASK_YSIGN));
    CPPUNIT_ASSERT(!(quantizedVec & MASK_ZSIGN));

    QuantizedUnitVec::flipSignBits(quantizedVec);

    CPPUNIT_ASSERT((quantizedVec & MASK_XSIGN));
    CPPUNIT_ASSERT((quantizedVec & MASK_YSIGN));
    CPPUNIT_ASSERT((quantizedVec & MASK_ZSIGN));

    unitVec[2] = -unitVec[2];
    quantizedVec = QuantizedUnitVec::pack(unitVec);
    QuantizedUnitVec::flipSignBits(quantizedVec);

    CPPUNIT_ASSERT((quantizedVec & MASK_XSIGN));
    CPPUNIT_ASSERT((quantizedVec & MASK_YSIGN));
    CPPUNIT_ASSERT(!(quantizedVec & MASK_ZSIGN));

    //
    // Check conversion error
    //
    const double tol = 0.05; // component error tolerance

    const int numNormals = 40000;


    // init
    srand(0);
    const int n = int(std::sqrt(double(numNormals)));
    const double xScale = (2.0 * M_PI) / double(n);
    const double yScale = M_PI / double(n);

    double x, y, theta, phi;
    Vec3s n0, n1;

    // generate random normals, by uniformly distributing points on a unit-sphere.

    // loop over a [0 to n) x [0 to n) grid.
    for (int a = 0; a < n; ++a) {
        for (int b = 0; b < n; ++b) {

            // jitter, move to random pos. inside the current cell
            x = double(a) + randNumber();
            y = double(b) + randNumber();

            // remap to a lat/long map
            theta = y * yScale; // [0 to PI]
            phi   = x * xScale; // [0 to 2PI]

            // convert to cartesian coordinates on a unit sphere.
            // spherical coordinate triplet (r=1, theta, phi)
            n0[0] = float(std::sin(theta)*std::cos(phi));
            n0[1] = float(std::sin(theta)*std::sin(phi));
            n0[2] = float(std::cos(theta));

            CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, n0.length(), 1e-6);

            n1 = QuantizedUnitVec::unpack(QuantizedUnitVec::pack(n0));

            CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, n1.length(), 1e-6);

            CPPUNIT_ASSERT_DOUBLES_EQUAL(n0[0], n1[0], tol);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(n0[1], n1[1], tol);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(n0[2], n1[2], tol);

            float sumDiff = std::abs(n0[0] - n1[0]) + std::abs(n0[1] - n1[1])
                + std::abs(n0[2] - n1[2]);

            CPPUNIT_ASSERT(sumDiff < (2.0 * tol));
        }
    }
}
