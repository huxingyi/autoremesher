// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#include <cppunit/extensions/HelperMacros.h>
#include <openvdb/Types.h>
#include <functional> // for std::ref()
#include <string>


using namespace openvdb;

class TestTypes: public CppUnit::TestCase
{
public:
    CPPUNIT_TEST_SUITE(TestTypes);
    CPPUNIT_TEST(testVecTraits);
    CPPUNIT_TEST(testQuatTraits);
    CPPUNIT_TEST(testMatTraits);
    CPPUNIT_TEST(testValueTraits);
    CPPUNIT_TEST(testTypeList);
    CPPUNIT_TEST_SUITE_END();

    void testVecTraits();
    void testQuatTraits();
    void testMatTraits();
    void testValueTraits();
    void testTypeList();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestTypes);


namespace { struct Dummy {}; }


void
TestTypes::testVecTraits()
{
    { // VecTraits - IsVec

        // standard types (Vec3s, etc)
        CPPUNIT_ASSERT(VecTraits<Vec3s>::IsVec);
        CPPUNIT_ASSERT(VecTraits<Vec3d>::IsVec);
        CPPUNIT_ASSERT(VecTraits<Vec3i>::IsVec);
        CPPUNIT_ASSERT(VecTraits<Vec2i>::IsVec);
        CPPUNIT_ASSERT(VecTraits<Vec2s>::IsVec);
        CPPUNIT_ASSERT(VecTraits<Vec2d>::IsVec);
        CPPUNIT_ASSERT(VecTraits<Vec4i>::IsVec);
        CPPUNIT_ASSERT(VecTraits<Vec4s>::IsVec);
        CPPUNIT_ASSERT(VecTraits<Vec4d>::IsVec);

        // some less common types (Vec3U16, etc)
        CPPUNIT_ASSERT(VecTraits<Vec2R>::IsVec);
        CPPUNIT_ASSERT(VecTraits<Vec3U16>::IsVec);
        CPPUNIT_ASSERT(VecTraits<Vec4H>::IsVec);

        // some non-vector types
        CPPUNIT_ASSERT(!VecTraits<int>::IsVec);
        CPPUNIT_ASSERT(!VecTraits<double>::IsVec);
        CPPUNIT_ASSERT(!VecTraits<bool>::IsVec);
        CPPUNIT_ASSERT(!VecTraits<Quats>::IsVec);
        CPPUNIT_ASSERT(!VecTraits<Mat4d>::IsVec);
        CPPUNIT_ASSERT(!VecTraits<ValueMask>::IsVec);
        CPPUNIT_ASSERT(!VecTraits<Dummy>::IsVec);
        CPPUNIT_ASSERT(!VecTraits<Byte>::IsVec);
    }

    { // VecTraits - Size

        // standard types (Vec3s, etc)
        CPPUNIT_ASSERT(VecTraits<Vec3s>::Size == 3);
        CPPUNIT_ASSERT(VecTraits<Vec3d>::Size == 3);
        CPPUNIT_ASSERT(VecTraits<Vec3i>::Size == 3);
        CPPUNIT_ASSERT(VecTraits<Vec2i>::Size == 2);
        CPPUNIT_ASSERT(VecTraits<Vec2s>::Size == 2);
        CPPUNIT_ASSERT(VecTraits<Vec2d>::Size == 2);
        CPPUNIT_ASSERT(VecTraits<Vec4i>::Size == 4);
        CPPUNIT_ASSERT(VecTraits<Vec4s>::Size == 4);
        CPPUNIT_ASSERT(VecTraits<Vec4d>::Size == 4);

        // some less common types (Vec3U16, etc)
        CPPUNIT_ASSERT(VecTraits<Vec2R>::Size == 2);
        CPPUNIT_ASSERT(VecTraits<Vec3U16>::Size == 3);
        CPPUNIT_ASSERT(VecTraits<Vec4H>::Size == 4);

        // some non-vector types
        CPPUNIT_ASSERT(VecTraits<int>::Size == 1);
        CPPUNIT_ASSERT(VecTraits<double>::Size == 1);
        CPPUNIT_ASSERT(VecTraits<bool>::Size == 1);
        CPPUNIT_ASSERT(VecTraits<Quats>::Size == 1);
        CPPUNIT_ASSERT(VecTraits<Mat4d>::Size == 1);
        CPPUNIT_ASSERT(VecTraits<ValueMask>::Size == 1);
        CPPUNIT_ASSERT(VecTraits<Dummy>::Size == 1);
        CPPUNIT_ASSERT(VecTraits<Byte>::Size == 1);
    }

    { // VecTraits - ElementType

        // standard types (Vec3s, etc)
        CPPUNIT_ASSERT(bool(std::is_same<VecTraits<Vec3s>::ElementType, float>::value));
        CPPUNIT_ASSERT(bool(std::is_same<VecTraits<Vec3d>::ElementType, double>::value));
        CPPUNIT_ASSERT(bool(std::is_same<VecTraits<Vec3i>::ElementType, int>::value));
        CPPUNIT_ASSERT(bool(std::is_same<VecTraits<Vec2i>::ElementType, int>::value));
        CPPUNIT_ASSERT(bool(std::is_same<VecTraits<Vec2s>::ElementType, float>::value));
        CPPUNIT_ASSERT(bool(std::is_same<VecTraits<Vec2d>::ElementType, double>::value));
        CPPUNIT_ASSERT(bool(std::is_same<VecTraits<Vec4i>::ElementType, int>::value));
        CPPUNIT_ASSERT(bool(std::is_same<VecTraits<Vec4s>::ElementType, float>::value));
        CPPUNIT_ASSERT(bool(std::is_same<VecTraits<Vec4d>::ElementType, double>::value));

        // some less common types (Vec3U16, etc)
        CPPUNIT_ASSERT(bool(std::is_same<VecTraits<Vec2R>::ElementType, double>::value));
        CPPUNIT_ASSERT(bool(std::is_same<VecTraits<Vec3U16>::ElementType, uint16_t>::value));
        CPPUNIT_ASSERT(bool(std::is_same<VecTraits<Vec4H>::ElementType, half>::value));

        // some non-vector types
        CPPUNIT_ASSERT(bool(std::is_same<VecTraits<int>::ElementType, int>::value));
        CPPUNIT_ASSERT(bool(std::is_same<VecTraits<double>::ElementType, double>::value));
        CPPUNIT_ASSERT(bool(std::is_same<VecTraits<bool>::ElementType, bool>::value));
        CPPUNIT_ASSERT(bool(std::is_same<VecTraits<Quats>::ElementType, Quats>::value));
        CPPUNIT_ASSERT(bool(std::is_same<VecTraits<Mat4d>::ElementType, Mat4d>::value));
        CPPUNIT_ASSERT(bool(std::is_same<VecTraits<ValueMask>::ElementType, ValueMask>::value));
        CPPUNIT_ASSERT(bool(std::is_same<VecTraits<Dummy>::ElementType, Dummy>::value));
        CPPUNIT_ASSERT(bool(std::is_same<VecTraits<Byte>::ElementType, Byte>::value));
    }
}


void
TestTypes::testQuatTraits()
{
    { // QuatTraits - IsQuat

        // standard types (Quats, etc)
        CPPUNIT_ASSERT(QuatTraits<Quats>::IsQuat);
        CPPUNIT_ASSERT(QuatTraits<Quatd>::IsQuat);

        // some non-quaternion types
        CPPUNIT_ASSERT(!QuatTraits<Vec3s>::IsQuat);
        CPPUNIT_ASSERT(!QuatTraits<Vec4d>::IsQuat);
        CPPUNIT_ASSERT(!QuatTraits<Vec2i>::IsQuat);
        CPPUNIT_ASSERT(!QuatTraits<Vec3U16>::IsQuat);
        CPPUNIT_ASSERT(!QuatTraits<int>::IsQuat);
        CPPUNIT_ASSERT(!QuatTraits<double>::IsQuat);
        CPPUNIT_ASSERT(!QuatTraits<bool>::IsQuat);
        CPPUNIT_ASSERT(!QuatTraits<Mat4s>::IsQuat);
        CPPUNIT_ASSERT(!QuatTraits<ValueMask>::IsQuat);
        CPPUNIT_ASSERT(!QuatTraits<Dummy>::IsQuat);
        CPPUNIT_ASSERT(!QuatTraits<Byte>::IsQuat);
    }

    { // QuatTraits - Size

        // standard types (Quats, etc)
        CPPUNIT_ASSERT(QuatTraits<Quats>::Size == 4);
        CPPUNIT_ASSERT(QuatTraits<Quatd>::Size == 4);

        // some non-quaternion types
        CPPUNIT_ASSERT(QuatTraits<Vec3s>::Size == 1);
        CPPUNIT_ASSERT(QuatTraits<Vec4d>::Size == 1);
        CPPUNIT_ASSERT(QuatTraits<Vec2i>::Size == 1);
        CPPUNIT_ASSERT(QuatTraits<Vec3U16>::Size == 1);
        CPPUNIT_ASSERT(QuatTraits<int>::Size == 1);
        CPPUNIT_ASSERT(QuatTraits<double>::Size == 1);
        CPPUNIT_ASSERT(QuatTraits<bool>::Size == 1);
        CPPUNIT_ASSERT(QuatTraits<Mat4s>::Size == 1);
        CPPUNIT_ASSERT(QuatTraits<ValueMask>::Size == 1);
        CPPUNIT_ASSERT(QuatTraits<Dummy>::Size == 1);
        CPPUNIT_ASSERT(QuatTraits<Byte>::Size == 1);
    }

    { // QuatTraits - ElementType

        // standard types (Quats, etc)
        CPPUNIT_ASSERT(bool(std::is_same<QuatTraits<Quats>::ElementType, float>::value));
        CPPUNIT_ASSERT(bool(std::is_same<QuatTraits<Quatd>::ElementType, double>::value));

        // some non-matrix types
        CPPUNIT_ASSERT(bool(std::is_same<QuatTraits<Vec3s>::ElementType, Vec3s>::value));
        CPPUNIT_ASSERT(bool(std::is_same<QuatTraits<Vec4d>::ElementType, Vec4d>::value));
        CPPUNIT_ASSERT(bool(std::is_same<QuatTraits<Vec2i>::ElementType, Vec2i>::value));
        CPPUNIT_ASSERT(bool(std::is_same<QuatTraits<Vec3U16>::ElementType, Vec3U16>::value));
        CPPUNIT_ASSERT(bool(std::is_same<QuatTraits<int>::ElementType, int>::value));
        CPPUNIT_ASSERT(bool(std::is_same<QuatTraits<double>::ElementType, double>::value));
        CPPUNIT_ASSERT(bool(std::is_same<QuatTraits<bool>::ElementType, bool>::value));
        CPPUNIT_ASSERT(bool(std::is_same<QuatTraits<Mat4s>::ElementType, Mat4s>::value));
        CPPUNIT_ASSERT(bool(std::is_same<QuatTraits<ValueMask>::ElementType, ValueMask>::value));
        CPPUNIT_ASSERT(bool(std::is_same<QuatTraits<Dummy>::ElementType, Dummy>::value));
        CPPUNIT_ASSERT(bool(std::is_same<QuatTraits<Byte>::ElementType, Byte>::value));
    }
}


void
TestTypes::testMatTraits()
{
    { // MatTraits - IsMat

        // standard types (Mat4d, etc)
        CPPUNIT_ASSERT(MatTraits<Mat3s>::IsMat);
        CPPUNIT_ASSERT(MatTraits<Mat3d>::IsMat);
        CPPUNIT_ASSERT(MatTraits<Mat4s>::IsMat);
        CPPUNIT_ASSERT(MatTraits<Mat4d>::IsMat);

        // some non-matrix types
        CPPUNIT_ASSERT(!MatTraits<Vec3s>::IsMat);
        CPPUNIT_ASSERT(!MatTraits<Vec4d>::IsMat);
        CPPUNIT_ASSERT(!MatTraits<Vec2i>::IsMat);
        CPPUNIT_ASSERT(!MatTraits<Vec3U16>::IsMat);
        CPPUNIT_ASSERT(!MatTraits<int>::IsMat);
        CPPUNIT_ASSERT(!MatTraits<double>::IsMat);
        CPPUNIT_ASSERT(!MatTraits<bool>::IsMat);
        CPPUNIT_ASSERT(!MatTraits<Quats>::IsMat);
        CPPUNIT_ASSERT(!MatTraits<ValueMask>::IsMat);
        CPPUNIT_ASSERT(!MatTraits<Dummy>::IsMat);
        CPPUNIT_ASSERT(!MatTraits<Byte>::IsMat);
    }

    { // MatTraits - Size

        // standard types (Mat4d, etc)
        CPPUNIT_ASSERT(MatTraits<Mat3s>::Size == 3);
        CPPUNIT_ASSERT(MatTraits<Mat3d>::Size == 3);
        CPPUNIT_ASSERT(MatTraits<Mat4s>::Size == 4);
        CPPUNIT_ASSERT(MatTraits<Mat4d>::Size == 4);

        // some non-matrix types
        CPPUNIT_ASSERT(MatTraits<Vec3s>::Size == 1);
        CPPUNIT_ASSERT(MatTraits<Vec4d>::Size == 1);
        CPPUNIT_ASSERT(MatTraits<Vec2i>::Size == 1);
        CPPUNIT_ASSERT(MatTraits<Vec3U16>::Size == 1);
        CPPUNIT_ASSERT(MatTraits<int>::Size == 1);
        CPPUNIT_ASSERT(MatTraits<double>::Size == 1);
        CPPUNIT_ASSERT(MatTraits<bool>::Size == 1);
        CPPUNIT_ASSERT(MatTraits<Quats>::Size == 1);
        CPPUNIT_ASSERT(MatTraits<ValueMask>::Size == 1);
        CPPUNIT_ASSERT(MatTraits<Dummy>::Size == 1);
        CPPUNIT_ASSERT(MatTraits<Byte>::Size == 1);
    }

    { // MatTraits - ElementType

        // standard types (Mat4d, etc)
        CPPUNIT_ASSERT(bool(std::is_same<MatTraits<Mat3s>::ElementType, float>::value));
        CPPUNIT_ASSERT(bool(std::is_same<MatTraits<Mat3d>::ElementType, double>::value));
        CPPUNIT_ASSERT(bool(std::is_same<MatTraits<Mat4s>::ElementType, float>::value));
        CPPUNIT_ASSERT(bool(std::is_same<MatTraits<Mat4d>::ElementType, double>::value));

        // some non-matrix types
        CPPUNIT_ASSERT(bool(std::is_same<MatTraits<Vec3s>::ElementType, Vec3s>::value));
        CPPUNIT_ASSERT(bool(std::is_same<MatTraits<Vec4d>::ElementType, Vec4d>::value));
        CPPUNIT_ASSERT(bool(std::is_same<MatTraits<Vec2i>::ElementType, Vec2i>::value));
        CPPUNIT_ASSERT(bool(std::is_same<MatTraits<Vec3U16>::ElementType, Vec3U16>::value));
        CPPUNIT_ASSERT(bool(std::is_same<MatTraits<int>::ElementType, int>::value));
        CPPUNIT_ASSERT(bool(std::is_same<MatTraits<double>::ElementType, double>::value));
        CPPUNIT_ASSERT(bool(std::is_same<MatTraits<bool>::ElementType, bool>::value));
        CPPUNIT_ASSERT(bool(std::is_same<MatTraits<Quats>::ElementType, Quats>::value));
        CPPUNIT_ASSERT(bool(std::is_same<MatTraits<ValueMask>::ElementType, ValueMask>::value));
        CPPUNIT_ASSERT(bool(std::is_same<MatTraits<Dummy>::ElementType, Dummy>::value));
        CPPUNIT_ASSERT(bool(std::is_same<MatTraits<Byte>::ElementType, Byte>::value));
    }
}


void
TestTypes::testValueTraits()
{
    { // ValueTraits - IsVec, IsQuat, IsMat, IsScalar

        // vector types
        CPPUNIT_ASSERT(ValueTraits<Vec3s>::IsVec);
        CPPUNIT_ASSERT(!ValueTraits<Vec3s>::IsQuat);
        CPPUNIT_ASSERT(!ValueTraits<Vec3s>::IsMat);
        CPPUNIT_ASSERT(!ValueTraits<Vec3s>::IsScalar);
        CPPUNIT_ASSERT(ValueTraits<Vec4d>::IsVec);
        CPPUNIT_ASSERT(!ValueTraits<Vec4d>::IsQuat);
        CPPUNIT_ASSERT(!ValueTraits<Vec4d>::IsMat);
        CPPUNIT_ASSERT(!ValueTraits<Vec4d>::IsScalar);
        CPPUNIT_ASSERT(ValueTraits<Vec3U16>::IsVec);
        CPPUNIT_ASSERT(!ValueTraits<Vec3U16>::IsQuat);
        CPPUNIT_ASSERT(!ValueTraits<Vec3U16>::IsMat);
        CPPUNIT_ASSERT(!ValueTraits<Vec3U16>::IsScalar);

        // quaternion types
        CPPUNIT_ASSERT(!ValueTraits<Quats>::IsVec);
        CPPUNIT_ASSERT(ValueTraits<Quats>::IsQuat);
        CPPUNIT_ASSERT(!ValueTraits<Quats>::IsMat);
        CPPUNIT_ASSERT(!ValueTraits<Quats>::IsScalar);
        CPPUNIT_ASSERT(!ValueTraits<Quatd>::IsVec);
        CPPUNIT_ASSERT(ValueTraits<Quatd>::IsQuat);
        CPPUNIT_ASSERT(!ValueTraits<Quatd>::IsMat);
        CPPUNIT_ASSERT(!ValueTraits<Quatd>::IsScalar);

        // matrix types
        CPPUNIT_ASSERT(!ValueTraits<Mat3s>::IsVec);
        CPPUNIT_ASSERT(!ValueTraits<Mat3s>::IsQuat);
        CPPUNIT_ASSERT(ValueTraits<Mat3s>::IsMat);
        CPPUNIT_ASSERT(!ValueTraits<Mat3s>::IsScalar);
        CPPUNIT_ASSERT(!ValueTraits<Mat4d>::IsVec);
        CPPUNIT_ASSERT(!ValueTraits<Mat4d>::IsQuat);
        CPPUNIT_ASSERT(ValueTraits<Mat4d>::IsMat);
        CPPUNIT_ASSERT(!ValueTraits<Mat4d>::IsScalar);

        // scalar types
        CPPUNIT_ASSERT(!ValueTraits<double>::IsVec);
        CPPUNIT_ASSERT(!ValueTraits<double>::IsQuat);
        CPPUNIT_ASSERT(!ValueTraits<double>::IsMat);
        CPPUNIT_ASSERT(ValueTraits<double>::IsScalar);
        CPPUNIT_ASSERT(!ValueTraits<bool>::IsVec);
        CPPUNIT_ASSERT(!ValueTraits<bool>::IsQuat);
        CPPUNIT_ASSERT(!ValueTraits<bool>::IsMat);
        CPPUNIT_ASSERT(ValueTraits<bool>::IsScalar);
        CPPUNIT_ASSERT(!ValueTraits<ValueMask>::IsVec);
        CPPUNIT_ASSERT(!ValueTraits<ValueMask>::IsQuat);
        CPPUNIT_ASSERT(!ValueTraits<ValueMask>::IsMat);
        CPPUNIT_ASSERT(ValueTraits<ValueMask>::IsScalar);
        CPPUNIT_ASSERT(!ValueTraits<Dummy>::IsVec);
        CPPUNIT_ASSERT(!ValueTraits<Dummy>::IsQuat);
        CPPUNIT_ASSERT(!ValueTraits<Dummy>::IsMat);
        CPPUNIT_ASSERT(ValueTraits<Dummy>::IsScalar);
    }

    { // ValueTraits - Size

        // vector types
        CPPUNIT_ASSERT(ValueTraits<Vec3s>::Size == 3);
        CPPUNIT_ASSERT(ValueTraits<Vec4d>::Size == 4);
        CPPUNIT_ASSERT(ValueTraits<Vec3U16>::Size == 3);

        // quaternion types
        CPPUNIT_ASSERT(ValueTraits<Quats>::Size == 4);
        CPPUNIT_ASSERT(ValueTraits<Quatd>::Size == 4);

        // matrix types
        CPPUNIT_ASSERT(ValueTraits<Mat3s>::Size == 3);
        CPPUNIT_ASSERT(ValueTraits<Mat4d>::Size == 4);

        // scalar types
        CPPUNIT_ASSERT(ValueTraits<double>::Size == 1);
        CPPUNIT_ASSERT(ValueTraits<bool>::Size == 1);
        CPPUNIT_ASSERT(ValueTraits<ValueMask>::Size == 1);
        CPPUNIT_ASSERT(ValueTraits<Dummy>::Size == 1);
    }

    { // ValueTraits - Elements

        // vector types
        CPPUNIT_ASSERT(ValueTraits<Vec3s>::Elements == 3);
        CPPUNIT_ASSERT(ValueTraits<Vec4d>::Elements == 4);
        CPPUNIT_ASSERT(ValueTraits<Vec3U16>::Elements == 3);

        // quaternion types
        CPPUNIT_ASSERT(ValueTraits<Quats>::Elements == 4);
        CPPUNIT_ASSERT(ValueTraits<Quatd>::Elements == 4);

        // matrix types
        CPPUNIT_ASSERT(ValueTraits<Mat3s>::Elements == 3*3);
        CPPUNIT_ASSERT(ValueTraits<Mat4d>::Elements == 4*4);

        // scalar types
        CPPUNIT_ASSERT(ValueTraits<double>::Elements == 1);
        CPPUNIT_ASSERT(ValueTraits<bool>::Elements == 1);
        CPPUNIT_ASSERT(ValueTraits<ValueMask>::Elements == 1);
        CPPUNIT_ASSERT(ValueTraits<Dummy>::Elements == 1);
    }

    { // ValueTraits - ElementType

        // vector types
        CPPUNIT_ASSERT(bool(std::is_same<ValueTraits<Vec3s>::ElementType, float>::value));
        CPPUNIT_ASSERT(bool(std::is_same<ValueTraits<Vec4d>::ElementType, double>::value));
        CPPUNIT_ASSERT(bool(std::is_same<ValueTraits<Vec3U16>::ElementType, uint16_t>::value));

        // quaternion types
        CPPUNIT_ASSERT(bool(std::is_same<ValueTraits<Quats>::ElementType, float>::value));
        CPPUNIT_ASSERT(bool(std::is_same<ValueTraits<Quatd>::ElementType, double>::value));

        // matrix types
        CPPUNIT_ASSERT(bool(std::is_same<ValueTraits<Mat3s>::ElementType, float>::value));
        CPPUNIT_ASSERT(bool(std::is_same<ValueTraits<Mat4d>::ElementType, double>::value));

        // scalar types
        CPPUNIT_ASSERT(bool(std::is_same<ValueTraits<double>::ElementType, double>::value));
        CPPUNIT_ASSERT(bool(std::is_same<ValueTraits<bool>::ElementType, bool>::value));
        CPPUNIT_ASSERT(bool(std::is_same<ValueTraits<ValueMask>::ElementType, ValueMask>::value));
        CPPUNIT_ASSERT(bool(std::is_same<ValueTraits<Dummy>::ElementType, Dummy>::value));
    }
}


////////////////////////////////////////


namespace {

template<typename T> char typeCode() { return '.'; }
template<> char typeCode<bool>()   { return 'b'; }
template<> char typeCode<char>()   { return 'c'; }
template<> char typeCode<double>() { return 'd'; }
template<> char typeCode<float>()  { return 'f'; }
template<> char typeCode<int>()    { return 'i'; }
template<> char typeCode<long>()   { return 'l'; }


struct TypeCodeOp
{
    std::string codes;
    template<typename T> void operator()(const T&) { codes.push_back(typeCode<T>()); }
};


template<typename TSet>
inline std::string
typeSetAsString()
{
    TypeCodeOp op;
    TSet::foreach(std::ref(op));
    return op.codes;
}

} // anonymous namespace


void
TestTypes::testTypeList()
{
    using T0 = TypeList<>;
    CPPUNIT_ASSERT_EQUAL(std::string(), typeSetAsString<T0>());

    using T1 = TypeList<int>;
    CPPUNIT_ASSERT_EQUAL(std::string("i"), typeSetAsString<T1>());

    using T2 = TypeList<float>;
    CPPUNIT_ASSERT_EQUAL(std::string("f"), typeSetAsString<T2>());

    using T3 = TypeList<bool, double>;
    CPPUNIT_ASSERT_EQUAL(std::string("bd"), typeSetAsString<T3>());

    using T4 = T1::Append<T2>;
    CPPUNIT_ASSERT_EQUAL(std::string("if"), typeSetAsString<T4>());
    CPPUNIT_ASSERT_EQUAL(std::string("fi"), typeSetAsString<T2::Append<T1>>());

    using T5 = T3::Append<T4>;
    CPPUNIT_ASSERT_EQUAL(std::string("bdif"), typeSetAsString<T5>());

    using T6 = T5::Append<T5>;
    CPPUNIT_ASSERT_EQUAL(std::string("bdifbdif"), typeSetAsString<T6>());

    using T7 = T5::Append<char, long>;
    CPPUNIT_ASSERT_EQUAL(std::string("bdifcl"), typeSetAsString<T7>());

    using T8 = T5::Append<char>::Append<long>;
    CPPUNIT_ASSERT_EQUAL(std::string("bdifcl"), typeSetAsString<T8>());

    using T9 = T8::Remove<TypeList<>>;
    CPPUNIT_ASSERT_EQUAL(std::string("bdifcl"), typeSetAsString<T9>());

    using T10 = T8::Remove<std::string>;
    CPPUNIT_ASSERT_EQUAL(std::string("bdifcl"), typeSetAsString<T10>());

    using T11 = T8::Remove<char>::Remove<int>;
    CPPUNIT_ASSERT_EQUAL(std::string("bdfl"), typeSetAsString<T11>());

    using T12 = T8::Remove<char, int>;
    CPPUNIT_ASSERT_EQUAL(std::string("bdfl"), typeSetAsString<T12>());

    using T13 = T8::Remove<TypeList<char, int>>;
    CPPUNIT_ASSERT_EQUAL(std::string("bdfl"), typeSetAsString<T13>());
}
