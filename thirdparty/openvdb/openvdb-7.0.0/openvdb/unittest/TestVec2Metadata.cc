// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#include <cppunit/extensions/HelperMacros.h>
#include <openvdb/Exceptions.h>
#include <openvdb/Metadata.h>

class TestVec2Metadata : public CppUnit::TestCase
{
public:
    CPPUNIT_TEST_SUITE(TestVec2Metadata);
    CPPUNIT_TEST(testVec2i);
    CPPUNIT_TEST(testVec2s);
    CPPUNIT_TEST(testVec2d);
    CPPUNIT_TEST_SUITE_END();

    void testVec2i();
    void testVec2s();
    void testVec2d();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestVec2Metadata);

void
TestVec2Metadata::testVec2i()
{
    using namespace openvdb;

    Metadata::Ptr m(new Vec2IMetadata(openvdb::Vec2i(1, 1)));
    Metadata::Ptr m2 = m->copy();

    CPPUNIT_ASSERT(dynamic_cast<Vec2IMetadata*>(m.get()) != 0);
    CPPUNIT_ASSERT(dynamic_cast<Vec2IMetadata*>(m2.get()) != 0);

    CPPUNIT_ASSERT(m->typeName().compare("vec2i") == 0);
    CPPUNIT_ASSERT(m2->typeName().compare("vec2i") == 0);

    Vec2IMetadata *s = dynamic_cast<Vec2IMetadata*>(m.get());
    CPPUNIT_ASSERT(s->value() == openvdb::Vec2i(1, 1));
    s->value() = openvdb::Vec2i(2, 2);
    CPPUNIT_ASSERT(s->value() == openvdb::Vec2i(2, 2));

    m2->copy(*s);

    s = dynamic_cast<Vec2IMetadata*>(m2.get());
    CPPUNIT_ASSERT(s->value() == openvdb::Vec2i(2, 2));
}

void
TestVec2Metadata::testVec2s()
{
    using namespace openvdb;

    Metadata::Ptr m(new Vec2SMetadata(openvdb::Vec2s(1, 1)));
    Metadata::Ptr m2 = m->copy();

    CPPUNIT_ASSERT(dynamic_cast<Vec2SMetadata*>(m.get()) != 0);
    CPPUNIT_ASSERT(dynamic_cast<Vec2SMetadata*>(m2.get()) != 0);

    CPPUNIT_ASSERT(m->typeName().compare("vec2s") == 0);
    CPPUNIT_ASSERT(m2->typeName().compare("vec2s") == 0);

    Vec2SMetadata *s = dynamic_cast<Vec2SMetadata*>(m.get());
    CPPUNIT_ASSERT(s->value() == openvdb::Vec2s(1, 1));
    s->value() = openvdb::Vec2s(2, 2);
    CPPUNIT_ASSERT(s->value() == openvdb::Vec2s(2, 2));

    m2->copy(*s);

    s = dynamic_cast<Vec2SMetadata*>(m2.get());
    CPPUNIT_ASSERT(s->value() == openvdb::Vec2s(2, 2));
}

void
TestVec2Metadata::testVec2d()
{
    using namespace openvdb;

    Metadata::Ptr m(new Vec2DMetadata(openvdb::Vec2d(1, 1)));
    Metadata::Ptr m2 = m->copy();

    CPPUNIT_ASSERT(dynamic_cast<Vec2DMetadata*>(m.get()) != 0);
    CPPUNIT_ASSERT(dynamic_cast<Vec2DMetadata*>(m2.get()) != 0);

    CPPUNIT_ASSERT(m->typeName().compare("vec2d") == 0);
    CPPUNIT_ASSERT(m2->typeName().compare("vec2d") == 0);

    Vec2DMetadata *s = dynamic_cast<Vec2DMetadata*>(m.get());
    CPPUNIT_ASSERT(s->value() == openvdb::Vec2d(1, 1));
    s->value() = openvdb::Vec2d(2, 2);
    CPPUNIT_ASSERT(s->value() == openvdb::Vec2d(2, 2));

    m2->copy(*s);

    s = dynamic_cast<Vec2DMetadata*>(m2.get());
    CPPUNIT_ASSERT(s->value() == openvdb::Vec2d(2, 2));
}
