// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#include <cppunit/extensions/HelperMacros.h>
#include <openvdb/Exceptions.h>
#include <openvdb/Metadata.h>

class TestMat4Metadata : public CppUnit::TestCase
{
public:
    CPPUNIT_TEST_SUITE(TestMat4Metadata);
    CPPUNIT_TEST(testMat4s);
    CPPUNIT_TEST(testMat4d);
    CPPUNIT_TEST_SUITE_END();

    void testMat4s();
    void testMat4d();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestMat4Metadata);

void
TestMat4Metadata::testMat4s()
{
    using namespace openvdb;

    Metadata::Ptr m(new Mat4SMetadata(openvdb::math::Mat4s(1.0f, 1.0f, 1.0f, 1.0f,
                                                           1.0f, 1.0f, 1.0f, 1.0f,
                                                           1.0f, 1.0f, 1.0f, 1.0f,
                                                           1.0f, 1.0f, 1.0f, 1.0f)));
    Metadata::Ptr m3 = m->copy();

    CPPUNIT_ASSERT(dynamic_cast<Mat4SMetadata*>( m.get()) != 0);
    CPPUNIT_ASSERT(dynamic_cast<Mat4SMetadata*>(m3.get()) != 0);

    CPPUNIT_ASSERT( m->typeName().compare("mat4s") == 0);
    CPPUNIT_ASSERT(m3->typeName().compare("mat4s") == 0);

    Mat4SMetadata *s = dynamic_cast<Mat4SMetadata*>(m.get());
    CPPUNIT_ASSERT(s->value() == openvdb::math::Mat4s(1.0f, 1.0f, 1.0f, 1.0f,
                                                      1.0f, 1.0f, 1.0f, 1.0f,
                                                      1.0f, 1.0f, 1.0f, 1.0f,
                                                      1.0f, 1.0f, 1.0f, 1.0f));
    s->value() = openvdb::math::Mat4s(3.0f, 3.0f, 3.0f, 3.0f,
                                      3.0f, 3.0f, 3.0f, 3.0f,
                                      3.0f, 3.0f, 3.0f, 3.0f,
                                      3.0f, 3.0f, 3.0f, 3.0f);
    CPPUNIT_ASSERT(s->value() == openvdb::math::Mat4s(3.0f, 3.0f, 3.0f, 3.0f,
                                                      3.0f, 3.0f, 3.0f, 3.0f,
                                                      3.0f, 3.0f, 3.0f, 3.0f,
                                                      3.0f, 3.0f, 3.0f, 3.0f));

    m3->copy(*s);

    s = dynamic_cast<Mat4SMetadata*>(m3.get());
    CPPUNIT_ASSERT(s->value() == openvdb::math::Mat4s(3.0f, 3.0f, 3.0f, 3.0f,
                                                      3.0f, 3.0f, 3.0f, 3.0f,
                                                      3.0f, 3.0f, 3.0f, 3.0f,
                                                      3.0f, 3.0f, 3.0f, 3.0f));
}

void
TestMat4Metadata::testMat4d()
{
    using namespace openvdb;

    Metadata::Ptr m(new Mat4DMetadata(openvdb::math::Mat4d(1.0, 1.0, 1.0, 1.0,
                                                           1.0, 1.0, 1.0, 1.0,
                                                           1.0, 1.0, 1.0, 1.0,
                                                           1.0, 1.0, 1.0, 1.0)));
    Metadata::Ptr m3 = m->copy();

    CPPUNIT_ASSERT(dynamic_cast<Mat4DMetadata*>( m.get()) != 0);
    CPPUNIT_ASSERT(dynamic_cast<Mat4DMetadata*>(m3.get()) != 0);

    CPPUNIT_ASSERT( m->typeName().compare("mat4d") == 0);
    CPPUNIT_ASSERT(m3->typeName().compare("mat4d") == 0);

    Mat4DMetadata *s = dynamic_cast<Mat4DMetadata*>(m.get());
    CPPUNIT_ASSERT(s->value() == openvdb::math::Mat4d(1.0, 1.0, 1.0, 1.0,
                                                      1.0, 1.0, 1.0, 1.0,
                                                      1.0, 1.0, 1.0, 1.0,
                                                      1.0, 1.0, 1.0, 1.0));
    s->value() = openvdb::math::Mat4d(3.0, 3.0, 3.0, 3.0,
                                      3.0, 3.0, 3.0, 3.0,
                                      3.0, 3.0, 3.0, 3.0,
                                      3.0, 3.0, 3.0, 3.0);
    CPPUNIT_ASSERT(s->value() == openvdb::math::Mat4d(3.0, 3.0, 3.0, 3.0,
                                                      3.0, 3.0, 3.0, 3.0,
                                                      3.0, 3.0, 3.0, 3.0,
                                                      3.0, 3.0, 3.0, 3.0));

    m3->copy(*s);

    s = dynamic_cast<Mat4DMetadata*>(m3.get());
    CPPUNIT_ASSERT(s->value() == openvdb::math::Mat4d(3.0, 3.0, 3.0, 3.0,
                                                      3.0, 3.0, 3.0, 3.0,
                                                      3.0, 3.0, 3.0, 3.0,
                                                      3.0, 3.0, 3.0, 3.0));
}
