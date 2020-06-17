#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>
#include <iostream>
#include <list>

#include <stdint.h>

namespace {

class OpenMeshVectorTest : public testing::Test {

    protected:

        // This function is called before each test is run
        virtual void SetUp() {
            
            // Do some initial stuff with the member data here...
        }

        // This function is called after all tests are through
        virtual void TearDown() {

            // Do some final stuff with the member data here...
        }

};



/*
 * ====================================================================
 * Define tests below
 * ====================================================================
 */

/* Compute surface area via cross product
 */
TEST_F(OpenMeshVectorTest, ComputeTriangleSurfaceWithCrossProduct) {


  //
  // vec1
  //  x
  //  |
  //  |
  //  |
  //  x------>x vec2
  //

  OpenMesh::Vec3d vec1(0.0,1.0,0.0);
  OpenMesh::Vec3d vec2(1.0,0.0,0.0);

  double area = 0.5 * cross(vec1,vec2).norm();
  EXPECT_EQ(0.5f , area ) << "Wrong area in cross product function";

  area = 0.5 * ( vec1 % vec2 ).norm();
  EXPECT_EQ(0.5f , area ) << "Wrong area in cross product operator";

}

/* Check OpenMesh Vector type abs function
 */
TEST_F(OpenMeshVectorTest, AbsTest) {

  OpenMesh::Vec3d vec1(0.5,0.5,-0.5);

  EXPECT_EQ( vec1.l8_norm() , 0.5f ) << "Wrong l8norm computation";

}

/* Compute surface area via cross product
 */
TEST_F(OpenMeshVectorTest, VectorCasting) {

  OpenMesh::Vec3d vecd(1.0,2.0,3.0);
  OpenMesh::Vec3f vecf = OpenMesh::vector_cast<OpenMesh::Vec3f>(vecd);
  EXPECT_EQ(1.f, vecf[0]) << "vector type cast failed on component 0";
  EXPECT_EQ(2.f, vecf[1]) << "vector type cast failed on component 1";
  EXPECT_EQ(3.f, vecf[2]) << "vector type cast failed on component 2";

  OpenMesh::Vec4d vecd4(40.0,30.0,20.0,10.0);
  vecd = OpenMesh::vector_cast<OpenMesh::Vec3d>(vecd4);
  EXPECT_EQ(40.0, vecd[0]) << "vector dimension cast failed on component 0";
  EXPECT_EQ(30.0, vecd[1]) << "vector dimension cast failed on component 1";
  EXPECT_EQ(20.0, vecd[2]) << "vector dimension cast failed on component 2";

}

#if (defined(_MSC_VER) && (_MSC_VER >= 1900)) || __cplusplus > 199711L || defined(__GXX_EXPERIMENTAL_CXX0X__)
TEST_F(OpenMeshVectorTest, cpp11_constructors) {
    OpenMesh::Vec3d vec1 { 1.2, 2.0, 3.0 };

    EXPECT_EQ(1.2, vec1[0]);
    EXPECT_EQ(2.0, vec1[1]);
    EXPECT_EQ(3.0, vec1[2]);

    OpenMesh::Vec4f vec2 { 1.2f, 3.5f, 1.0f, 0.0f };

    EXPECT_EQ(1.2f, vec2[0]);
    EXPECT_EQ(3.5f, vec2[1]);
    EXPECT_EQ(1.0f, vec2[2]);
    EXPECT_EQ(0.0f, vec2[3]);

    OpenMesh::Vec4f vec2b { vec2 };

    EXPECT_EQ(1.2f, vec2b[0]);
    EXPECT_EQ(3.5f, vec2b[1]);
    EXPECT_EQ(1.0f, vec2b[2]);
    EXPECT_EQ(0.0f, vec2b[3]);

    OpenMesh::Vec4d vec4d { 1.23 };
    EXPECT_EQ(1.23, vec4d[0]);
    EXPECT_EQ(1.23, vec4d[1]);
    EXPECT_EQ(1.23, vec4d[2]);
    EXPECT_EQ(1.23, vec4d[3]);
}

TEST_F(OpenMeshVectorTest, cpp11_htmlColorLiteral) {
    static constexpr OpenMesh::Vec4f rose = 0xFFC7F1FF_htmlColor;

    EXPECT_EQ(0xFFC7F1FF_htmlColor, rose);

    const OpenMesh::Vec4f light_blue = 0x1FCFFFFF_htmlColor;
    EXPECT_LE((OpenMesh::Vec4f(0.1215686274f, 0.8117647058f, 1.0f, 1.0f)
        - light_blue).sqrnorm(), 1e-10);

    const auto light_blue_2 = 0x1FCFFFFF_htmlColor;
    // Check whether auto type deduction works as expected.
    static_assert(std::is_same<decltype(light_blue_2), decltype(light_blue)>
        ::value, "Bad type deduced from _htmlColor literal.");
    EXPECT_EQ(light_blue, light_blue_2);
}


namespace {
class C {
    public:
        C() {}
        C(const C &rhs) { ADD_FAILURE() << "Copy constructor used."; }
        C(C &&rhs) { ++copy_con; }
        C &operator= (const C &rhs) {
            ADD_FAILURE() << "Copy assignemnt used.";
            return *this;
        }
        C &operator= (C &&rhs) { ++copy_ass; return *this; }

        static int copy_con;
        static int copy_ass;
};

int C::copy_con = 0;
int C::copy_ass = 0;
}

/**
 * Checks two things:
 *   1) Whether VectorT works with a non-arithmetic type.
 *   2) Whether move construction and assignment works.
 */
TEST_F(OpenMeshVectorTest, move_constructor_assignment) {

    C::copy_con = 0;
    C::copy_ass = 0;

    // Test move assigning.
    OpenMesh::VectorT<C, 3> x, y;
    x = std::move(y);
    EXPECT_EQ(3, C::copy_ass);
    EXPECT_EQ(0, C::copy_con);

    // Test move constructing.
    OpenMesh::VectorT<C, 3> z(std::move(x));
    EXPECT_EQ(3, C::copy_ass);
    EXPECT_EQ(3, C::copy_con);
}

TEST_F(OpenMeshVectorTest, iterator_init) {
    std::list<float> a;
    a.push_back(1.0);
    a.push_back(2.0);
    a.push_back(3.0);
    OpenMesh::Vec3f v(a.begin());
    EXPECT_EQ(OpenMesh::Vec3f(1.0, 2.0, 3.0), v);
}

#endif // C++11


TEST_F(OpenMeshVectorTest, BasicArithmeticInPlace) {
    OpenMesh::Vec3d v1(1, 2, 3);
    double s1 = 2;
    const double epsilon = 1e-6;

    OpenMesh::Vec3d v2add (3, 4, 6); v2add  += v1;
    OpenMesh::Vec3d v2sub (3, 4, 6); v2sub  -= v1;
    OpenMesh::Vec3d v2cmul(3, 4, 6); v2cmul *= v1;
    OpenMesh::Vec3d v2cdiv(3, 4, 6); v2cdiv /= v1;
    OpenMesh::Vec3d v2smul(3, 4, 6); v2smul *= s1;
    OpenMesh::Vec3d v2sdiv(3, 4, 6); v2sdiv /= s1;

    EXPECT_NEAR(4, v2add[0], epsilon);
    EXPECT_NEAR(6, v2add[1], epsilon);
    EXPECT_NEAR(9, v2add[2], epsilon);

    EXPECT_NEAR(2, v2sub[0], epsilon);
    EXPECT_NEAR(2, v2sub[1], epsilon);
    EXPECT_NEAR(3, v2sub[2], epsilon);

    EXPECT_NEAR( 3, v2cmul[0], epsilon);
    EXPECT_NEAR( 8, v2cmul[1], epsilon);
    EXPECT_NEAR(18, v2cmul[2], epsilon);

    EXPECT_NEAR(3, v2cdiv[0], epsilon);
    EXPECT_NEAR(2, v2cdiv[1], epsilon);
    EXPECT_NEAR(2, v2cdiv[2], epsilon);

    EXPECT_NEAR( 6, v2smul[0], epsilon);
    EXPECT_NEAR( 8, v2smul[1], epsilon);
    EXPECT_NEAR(12, v2smul[2], epsilon);

    EXPECT_NEAR(1.5, v2sdiv[0], epsilon);
    EXPECT_NEAR(2.0, v2sdiv[1], epsilon);
    EXPECT_NEAR(3.0, v2sdiv[2], epsilon);
}

TEST_F(OpenMeshVectorTest, BasicArithmeticImmutable) {
    OpenMesh::Vec3d v1(1, 2, 3);
    const double epsilon = 1e-6;

    OpenMesh::Vec3d v2add  = v1 + OpenMesh::Vec3d(2, 4, 6);
    OpenMesh::Vec3d v2sub  = v1 - OpenMesh::Vec3d(2, 4, 6);
    OpenMesh::Vec3d v2cmul = v1 * OpenMesh::Vec3d(2, 4, 6);
    OpenMesh::Vec3d v2cdiv = v1 / OpenMesh::Vec3d(2, 4, 6);
    OpenMesh::Vec3d v2smul = v1 * 2.0;
    OpenMesh::Vec3d v2sdiv = v1 / 2.0;
    OpenMesh::Vec3d v2neg  = -v1;

    EXPECT_NEAR(3, v2add[0], epsilon);
    EXPECT_NEAR(6, v2add[1], epsilon);
    EXPECT_NEAR(9, v2add[2], epsilon);

    EXPECT_NEAR(-1, v2sub[0], epsilon);
    EXPECT_NEAR(-2, v2sub[1], epsilon);
    EXPECT_NEAR(-3, v2sub[2], epsilon);

    EXPECT_NEAR( 2, v2cmul[0], epsilon);
    EXPECT_NEAR( 8, v2cmul[1], epsilon);
    EXPECT_NEAR(18, v2cmul[2], epsilon);

    EXPECT_NEAR(0.5, v2cdiv[0], epsilon);
    EXPECT_NEAR(0.5, v2cdiv[1], epsilon);
    EXPECT_NEAR(0.5, v2cdiv[2], epsilon);

    EXPECT_NEAR(2, v2smul[0], epsilon);
    EXPECT_NEAR(4, v2smul[1], epsilon);
    EXPECT_NEAR(6, v2smul[2], epsilon);

    EXPECT_NEAR(0.5, v2sdiv[0], epsilon);
    EXPECT_NEAR(1.0, v2sdiv[1], epsilon);
    EXPECT_NEAR(1.5, v2sdiv[2], epsilon);

    EXPECT_NEAR(-1, v2neg[0], epsilon);
    EXPECT_NEAR(-2, v2neg[1], epsilon);
    EXPECT_NEAR(-3, v2neg[2], epsilon);
}
template <typename V1, typename V2, typename S>
void test_dot(const V1 &v1, const V2 &v2, const S&s) {
    EXPECT_NEAR(s, v1 | v2, 1e-6);
    EXPECT_NEAR(s, v1.dot(v2), 1e-6);
    EXPECT_NEAR(-s, (-v1) | v2, 1e-6);
    EXPECT_NEAR(-s, (-v1).dot(v2), 1e-6);
    EXPECT_NEAR(s, v2 | v1, 1e-6);
    EXPECT_NEAR(s, v2.dot(v1), 1e-6);
}

template <typename V1, typename V2, typename V3>
void test_cross(const V1 &v1, const V2 &v2, const V3 &res) {
    EXPECT_NEAR(0, (v1.cross(v2) - res).norm(), 1e-6);
    EXPECT_NEAR(0, (v1 % v2 - res).norm(), 1e-6);
    EXPECT_NEAR(0, (v2.cross(v1) + res).norm(), 1e-6);
    EXPECT_NEAR(0, (v2 % v1 + res).norm(), 1e-6);
}

TEST_F(OpenMeshVectorTest, BasicLinearAlgebra) {
    OpenMesh::Vec3d v(1, 2, 3);
    EXPECT_EQ(v[0], 1.0);
    EXPECT_EQ(v[1], 2.0);
    EXPECT_EQ(v[2], 3.0);

    EXPECT_EQ(OpenMesh::Vec3d(-1, -2, -3), -v);
    EXPECT_EQ(3, OpenMesh::Vec3d(1, 3, 2).max());
    EXPECT_EQ(3, OpenMesh::Vec3d(1, 2, 3).max());
    EXPECT_EQ(3, OpenMesh::Vec3d(1, 3, -4).max());
    EXPECT_EQ(3, OpenMesh::Vec3d(-4, 2, 3).max());
    EXPECT_EQ(4, OpenMesh::Vec3d(1, 3, -4).max_abs());
    EXPECT_EQ(4, OpenMesh::Vec3d(-4, 2, 3).max_abs());

    EXPECT_EQ(1, OpenMesh::Vec3d(1, 3, 2).min());
    EXPECT_EQ(1, OpenMesh::Vec3d(1, 2, 3).min());
    EXPECT_EQ(-4, OpenMesh::Vec3d(1, 3, -4).min());
    EXPECT_EQ(-4, OpenMesh::Vec3d(-4, 2, 3).min());
    EXPECT_EQ(1, OpenMesh::Vec3d(1, 3, -4).min_abs());
    EXPECT_EQ(2, OpenMesh::Vec3d(-4, 2, 3).min_abs());


    test_dot(OpenMesh::Vec3d(1, 2, 3), OpenMesh::Vec3d(1, 2, 3), 14.);
    test_dot(OpenMesh::Vec3d(1, 2, 3), OpenMesh::Vec3d(-1, -2, -3), -14.);
    test_dot(OpenMesh::Vec3d(1, 2, 3), OpenMesh::Vec3i(1, 2, 3), 14);
    test_dot(OpenMesh::Vec3i(1, 2, 3), OpenMesh::Vec3i(1, 2, 3), 14);

    test_cross(OpenMesh::Vec3i(2, 0, 0), OpenMesh::Vec3i(0, 3, 0), OpenMesh::Vec3i(0, 0, 6));
    test_cross(OpenMesh::Vec3d(2, 0, 0), OpenMesh::Vec3d(0, 3, 0), OpenMesh::Vec3d(0, 0, 6));
    test_cross(OpenMesh::Vec3d(2, 0, 0), OpenMesh::Vec3i(0, 3, 0), OpenMesh::Vec3d(0, 0, 6));
}

TEST_F(OpenMeshVectorTest, array_init) {
    float a[3]; a[0] = 1.0; a[1] = 2.0; a[2] = 3.0;
    OpenMesh::Vec3f v(a);
    EXPECT_EQ(OpenMesh::Vec3f(1.0, 2.0, 3.0), v);

    // This should not invoke the array constructor.
    OpenMesh::Vec3d v2(3.0f);
}

TEST_F(OpenMeshVectorTest, normalized_cond) {
    OpenMesh::Vec3d v1(1, -2, 3), v2(0, 0, 0);
    EXPECT_EQ(OpenMesh::Vec3d(0, 0, 0), v2.normalize_cond());
    const OpenMesh::Vec3d r1 =
            OpenMesh::Vec3d(
                     0.2672612419124244,
                    -0.5345224838248488,
                     0.8017837257372732) - v1.normalize_cond();
    EXPECT_NEAR(r1[0], 0.0, 1e-12);
    EXPECT_NEAR(r1[1], 0.0, 1e-12);
    EXPECT_NEAR(r1[2], 0.0, 1e-12);
}

TEST_F(OpenMeshVectorTest, size_dim) {
    OpenMesh::Vec3d v3d(1, 2, 3);
    OpenMesh::Vec3f v3f(1, 2, 3);
    OpenMesh::Vec2i v2i(1, 2);

    EXPECT_EQ(3u, v3d.size());
    EXPECT_EQ(3, v3d.dim());
    EXPECT_EQ(3u, v3f.size());
    EXPECT_EQ(3, v3f.dim());
    EXPECT_EQ(2u, v2i.size());
    EXPECT_EQ(2, v2i.dim());
}

class OpenMeshVectorGCCBugTest;
void trigger_alignment_bug(OpenMeshVectorGCCBugTest &obj);

/**
 * This is a regression test for a GCC compiler bug.
 * @see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66598
 * @see https://www.graphics.rwth-aachen.de:9000/OpenMesh/OpenMesh/issues/32
 */
class OpenMeshVectorGCCBugTest : public testing::Test {

    protected:

        virtual void SetUp() {
            /*
             * WARNING: DO NOT CHANGE ANYTHGIN! Every single line, as
             * pointless as it may look, is carefully crafted to provoke
             * the GCC optimizer bug mentioned above.
             */
            testfn = trigger_alignment_bug;
            vec1 = OpenMesh::Vec4f(1.0f, 2.0f, 3.0f, 4.0f);
            vec2 = OpenMesh::Vec4f(5.0f, 6.0f, 7.0f, 8.0f);
            padding = 42;
        }

        virtual void TearDown() {
            // Do some final stuff with the member data here...
        }

    public:
        /*
         * WARNING: DO NOT CHANGE ANYTHGIN! Every single line, as
         * pointless as it may look, is carefully crafted to provoke
         * the GCC optimizer bug mentioned above.
         */
        int32_t padding;
        OpenMesh::Vec4f vec1, vec2;
        void (*testfn)(OpenMeshVectorGCCBugTest &obj);

        OpenMesh::Vec4f &get_vec1() { return vec1; }
        OpenMesh::Vec4f &get_vec2() { return vec2; }
};

void trigger_alignment_bug(OpenMeshVectorGCCBugTest &obj) {
    /*
     * WARNING: DO NOT CHANGE ANYTHGIN! Every single line, as
     * pointless as it may look, is carefully crafted to provoke
     * the GCC optimizer bug mentioned above.
     */
    int x1 = 1;

    OpenMesh::Vec4f vec3 = obj.get_vec1();
    OpenMesh::Vec4f vec4 = obj.get_vec2();
    vec3 += vec4;

    EXPECT_EQ(1, x1);
    EXPECT_EQ(42, obj.padding);
    EXPECT_EQ(6.0f, vec3[0]);
    EXPECT_EQ(8.0f, vec3[1]);
    EXPECT_EQ(10.0f, vec3[2]);
    EXPECT_EQ(12.0f, vec3[3]);
}

TEST_F(OpenMeshVectorGCCBugTest, alignment_bug) {
    /*
     * WARNING: DO NOT CHANGE ANYTHGIN! Every single line, as
     * pointless as it may look, is carefully crafted to provoke
     * the GCC optimizer bug mentioned above.
     */
    (*testfn)(*this);
}


TEST_F(OpenMeshVectorTest, Test_simple_0_constructor) {

  // Create a test vector with zeroes from one parameter
  OpenMesh::Vec3d testVec = OpenMesh::Vec3d(0);
  
  EXPECT_EQ(0.0f, testVec[0]) << "Wrong Value after construction!";
  EXPECT_EQ(0.0f, testVec[1]) << "Wrong Value after construction!";
  EXPECT_EQ(0.0f, testVec[2]) << "Wrong Value after construction!";



}



}
