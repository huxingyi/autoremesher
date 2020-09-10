// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#include <cppunit/extensions/HelperMacros.h>
#include <openvdb/Exceptions.h>
#include <openvdb/Metadata.h>


class TestFloatMetadata : public CppUnit::TestCase
{
public:
    CPPUNIT_TEST_SUITE(TestFloatMetadata);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();

    void test();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestFloatMetadata);

void
TestFloatMetadata::test()
{
    using namespace openvdb;

    Metadata::Ptr m(new FloatMetadata(1.0));
    Metadata::Ptr m2 = m->copy();

    CPPUNIT_ASSERT(dynamic_cast<FloatMetadata*>(m.get()) != 0);
    CPPUNIT_ASSERT(dynamic_cast<FloatMetadata*>(m2.get()) != 0);

    CPPUNIT_ASSERT(m->typeName().compare("float") == 0);
    CPPUNIT_ASSERT(m2->typeName().compare("float") == 0);

    FloatMetadata *s = dynamic_cast<FloatMetadata*>(m.get());
    //CPPUNIT_ASSERT(s->value() == 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f,s->value(),0);
    s->value() = 2.0;
    //CPPUNIT_ASSERT(s->value() == 2.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0f,s->value(),0);
    m2->copy(*s);

    s = dynamic_cast<FloatMetadata*>(m2.get());
    //CPPUNIT_ASSERT(s->value() == 2.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0f,s->value(),0);
}
