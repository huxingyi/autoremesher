// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#include <cppunit/extensions/HelperMacros.h>
#include <openvdb/Exceptions.h>
#include <openvdb/util/NodeMasks.h>
#include <openvdb/io/Compression.h>

using openvdb::Index;

template<typename MaskType> void TestAll();

class TestNodeMask: public CppUnit::TestCase
{
public:
    CPPUNIT_TEST_SUITE(TestNodeMask);
    CPPUNIT_TEST(testAll4);
    CPPUNIT_TEST(testAll3);
    CPPUNIT_TEST(testAll2);
    CPPUNIT_TEST(testAll1);
    CPPUNIT_TEST(testCompress);
    CPPUNIT_TEST_SUITE_END();

    void testAll4() { TestAll<openvdb::util::NodeMask<4> >(); }
    void testAll3() { TestAll<openvdb::util::NodeMask<3> >(); }
    void testAll2() { TestAll<openvdb::util::NodeMask<2> >(); }
    void testAll1() { TestAll<openvdb::util::NodeMask<1> >(); }

    void testCompress();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestNodeMask);

template<typename MaskType>
void TestAll()
{
    CPPUNIT_ASSERT(MaskType::memUsage() == MaskType::SIZE/8);
    const Index SIZE = MaskType::SIZE > 512 ? 512 : MaskType::SIZE;

    {// default constructor
        MaskType m;//all bits are off
        for (Index i=0; i<SIZE; ++i) CPPUNIT_ASSERT(m.isOff(i));
        for (Index i=0; i<SIZE; ++i) CPPUNIT_ASSERT(!m.isOn(i));
        CPPUNIT_ASSERT(m.isOff());
        CPPUNIT_ASSERT(!m.isOn());
        CPPUNIT_ASSERT(m.countOn() == 0);
        CPPUNIT_ASSERT(m.countOff()== MaskType::SIZE);
        m.toggle();//all bits are on
        CPPUNIT_ASSERT(m.isOn());
        CPPUNIT_ASSERT(!m.isOff());
        CPPUNIT_ASSERT(m.countOn() == MaskType::SIZE);
        CPPUNIT_ASSERT(m.countOff()== 0);
        for (Index i=0; i<SIZE; ++i) CPPUNIT_ASSERT(!m.isOff(i));
        for (Index i=0; i<SIZE; ++i) CPPUNIT_ASSERT(m.isOn(i));
    }
    {// On constructor
        MaskType m(true);//all bits are on
        CPPUNIT_ASSERT(m.isOn());
        CPPUNIT_ASSERT(!m.isOff());
        CPPUNIT_ASSERT(m.countOn() == MaskType::SIZE);
        CPPUNIT_ASSERT(m.countOff()== 0);
        for (Index i=0; i<SIZE; ++i) CPPUNIT_ASSERT(!m.isOff(i));
        for (Index i=0; i<SIZE; ++i) CPPUNIT_ASSERT(m.isOn(i));
        m.toggle();
        for (Index i=0; i<SIZE; ++i) CPPUNIT_ASSERT(m.isOff(i));
        for (Index i=0; i<SIZE; ++i) CPPUNIT_ASSERT(!m.isOn(i));
        CPPUNIT_ASSERT(m.isOff());
        CPPUNIT_ASSERT(!m.isOn());
        CPPUNIT_ASSERT(m.countOn() == 0);
        CPPUNIT_ASSERT(m.countOff()== MaskType::SIZE);
    }
    {// Off constructor
        MaskType m(false);
        CPPUNIT_ASSERT(m.isOff());
        CPPUNIT_ASSERT(!m.isOn());
        CPPUNIT_ASSERT(m.countOn() == 0);
        CPPUNIT_ASSERT(m.countOff()== MaskType::SIZE);
        m.setOn();
        CPPUNIT_ASSERT(m.isOn());
        CPPUNIT_ASSERT(!m.isOff());
        CPPUNIT_ASSERT(m.countOn() == MaskType::SIZE);
        CPPUNIT_ASSERT(m.countOff()== 0);
        m = MaskType();//copy asignment
        CPPUNIT_ASSERT(m.isOff());
        CPPUNIT_ASSERT(!m.isOn());
        CPPUNIT_ASSERT(m.countOn() == 0);
        CPPUNIT_ASSERT(m.countOff()== MaskType::SIZE);
    }
    {// test setOn, setOff, findFirstOn and findFiratOff
        MaskType m;
        for (Index i=0; i<SIZE; ++i) {
            m.setOn(i);
            CPPUNIT_ASSERT(m.countOn() == 1);
            CPPUNIT_ASSERT(m.findFirstOn() == i);
            CPPUNIT_ASSERT(m.findFirstOff() == (i==0 ? 1 : 0));
            for (Index j=0; j<SIZE; ++j) {
                CPPUNIT_ASSERT( i==j ? m.isOn(j) : m.isOff(j) );
            }
            m.setOff(i);
            CPPUNIT_ASSERT(m.countOn() == 0);
            CPPUNIT_ASSERT(m.findFirstOn() == MaskType::SIZE);
        }
    }
    {// OnIterator
        MaskType m;
        for (Index i=0; i<SIZE; ++i) {
            m.setOn(i);
            for (typename MaskType::OnIterator iter=m.beginOn(); iter; ++iter) {
                CPPUNIT_ASSERT( iter.pos() == i );
            }
            CPPUNIT_ASSERT(m.countOn() == 1);
            m.setOff(i);
            CPPUNIT_ASSERT(m.countOn() == 0);
        }
    }
    {// OffIterator
        MaskType m(true);
        for (Index i=0; i<SIZE; ++i) {
            m.setOff(i);
            CPPUNIT_ASSERT(m.countOff() == 1);
            for (typename MaskType::OffIterator iter=m.beginOff(); iter; ++iter) {
                CPPUNIT_ASSERT( iter.pos() == i );
            }
            CPPUNIT_ASSERT(m.countOn() == MaskType::SIZE-1);
            m.setOn(i);
            CPPUNIT_ASSERT(m.countOff() == 0);
            CPPUNIT_ASSERT(m.countOn() == MaskType::SIZE);
        }
    }
    {// isConstant
        MaskType m(true);//all bits are on
        bool isOn = false;
        CPPUNIT_ASSERT(!m.isOff());
        CPPUNIT_ASSERT(m.isOn());
        CPPUNIT_ASSERT(m.isConstant(isOn));
        CPPUNIT_ASSERT(isOn);
        m.setOff(MaskType::SIZE-1);//sets last bit off
        CPPUNIT_ASSERT(!m.isOff());
        CPPUNIT_ASSERT(!m.isOn());
        CPPUNIT_ASSERT(!m.isConstant(isOn));
        m.setOff();//sets all bits off
        CPPUNIT_ASSERT(m.isOff());
        CPPUNIT_ASSERT(!m.isOn());
        CPPUNIT_ASSERT(m.isConstant(isOn));
        CPPUNIT_ASSERT(!isOn);
    }
    {// DenseIterator
        MaskType m(false);
        for (Index i=0; i<SIZE; ++i) {
            m.setOn(i);
            CPPUNIT_ASSERT(m.countOn() == 1);
            for (typename MaskType::DenseIterator iter=m.beginDense(); iter; ++iter) {
                CPPUNIT_ASSERT( iter.pos()==i ? *iter : !*iter );
            }
            m.setOff(i);
            CPPUNIT_ASSERT(m.countOn() == 0);
        }
    }
}

void
TestNodeMask::testCompress()
{
    using namespace openvdb;

    using ValueT = int;
    using MaskT = openvdb::util::NodeMask<1>;

    { // no inactive values
        MaskT valueMask(true);
        MaskT childMask;
        std::vector<int> values = {0,1,2,3,4,5,6,7};
        int background = 0;

        CPPUNIT_ASSERT_EQUAL(valueMask.countOn(), Index32(8));
        CPPUNIT_ASSERT_EQUAL(childMask.countOn(), Index32(0));

        openvdb::io::MaskCompress<ValueT, MaskT> maskCompress(
            valueMask, childMask, values.data(), background);

        CPPUNIT_ASSERT_EQUAL(maskCompress.metadata, int8_t(openvdb::io::NO_MASK_OR_INACTIVE_VALS));
        CPPUNIT_ASSERT_EQUAL(maskCompress.inactiveVal[0], background);
        CPPUNIT_ASSERT_EQUAL(maskCompress.inactiveVal[1], background);
    }

    { // all inactive values are +background
        MaskT valueMask;
        MaskT childMask;
        std::vector<int> values = {10,10,10,10,10,10,10,10};
        int background = 10;

        CPPUNIT_ASSERT_EQUAL(valueMask.countOn(), Index32(0));

        openvdb::io::MaskCompress<ValueT, MaskT> maskCompress(
            valueMask, childMask, values.data(), background);

        CPPUNIT_ASSERT_EQUAL(maskCompress.metadata, int8_t(openvdb::io::NO_MASK_OR_INACTIVE_VALS));
        CPPUNIT_ASSERT_EQUAL(maskCompress.inactiveVal[0], background);
        CPPUNIT_ASSERT_EQUAL(maskCompress.inactiveVal[1], background);
    }

    { // all inactive values are -background
        MaskT valueMask;
        MaskT childMask;
        std::vector<int> values = {-10,-10,-10,-10,-10,-10,-10,-10};
        int background = 10;

        CPPUNIT_ASSERT_EQUAL(valueMask.countOn(), Index32(0));

        openvdb::io::MaskCompress<ValueT, MaskT> maskCompress(
            valueMask, childMask, values.data(), background);

        CPPUNIT_ASSERT_EQUAL(maskCompress.metadata, int8_t(openvdb::io::NO_MASK_AND_MINUS_BG));
        CPPUNIT_ASSERT_EQUAL(maskCompress.inactiveVal[0], -background);
        CPPUNIT_ASSERT_EQUAL(maskCompress.inactiveVal[1], background);
    }

    { // all inactive vals have the same non-background val
        MaskT valueMask(true);
        MaskT childMask;
        std::vector<int> values = {0,1,500,500,4,500,500,7};
        int background = 10;

        valueMask.setOff(2);
        valueMask.setOff(3);
        valueMask.setOff(5);
        valueMask.setOff(6);
        CPPUNIT_ASSERT_EQUAL(valueMask.countOn(), Index32(4));

        openvdb::io::MaskCompress<ValueT, MaskT> maskCompress(
            valueMask, childMask, values.data(), background);

        CPPUNIT_ASSERT_EQUAL(int(maskCompress.metadata), int(openvdb::io::NO_MASK_AND_ONE_INACTIVE_VAL));
        CPPUNIT_ASSERT_EQUAL(maskCompress.inactiveVal[0], 500);
        CPPUNIT_ASSERT_EQUAL(maskCompress.inactiveVal[1], background);
    }

    { // mask selects between -background and +background
        MaskT valueMask;
        MaskT childMask;
        std::vector<int> values = {0,10,10,-10,4,10,-10,10};
        int background = 10;

        valueMask.setOn(0);
        valueMask.setOn(4);
        CPPUNIT_ASSERT_EQUAL(valueMask.countOn(), Index32(2));

        openvdb::io::MaskCompress<ValueT, MaskT> maskCompress(
            valueMask, childMask, values.data(), background);

        CPPUNIT_ASSERT_EQUAL(int(maskCompress.metadata), int(openvdb::io::MASK_AND_NO_INACTIVE_VALS));
        CPPUNIT_ASSERT_EQUAL(maskCompress.inactiveVal[0], -background);
        CPPUNIT_ASSERT_EQUAL(maskCompress.inactiveVal[1], background);
    }

    { // mask selects between -background and +background
        MaskT valueMask;
        MaskT childMask;
        std::vector<int> values = {0,-10,-10,10,4,-10,10,-10};
        int background = 10;

        valueMask.setOn(0);
        valueMask.setOn(4);
        CPPUNIT_ASSERT_EQUAL(valueMask.countOn(), Index32(2));

        openvdb::io::MaskCompress<ValueT, MaskT> maskCompress(
            valueMask, childMask, values.data(), background);

        CPPUNIT_ASSERT_EQUAL(int(maskCompress.metadata), int(openvdb::io::MASK_AND_NO_INACTIVE_VALS));
        CPPUNIT_ASSERT_EQUAL(maskCompress.inactiveVal[0], -background);
        CPPUNIT_ASSERT_EQUAL(maskCompress.inactiveVal[1], background);
    }

    { // mask selects between backgd and one other inactive val
        MaskT valueMask;
        MaskT childMask;
        std::vector<int> values = {0,500,500,10,4,500,10,500};
        int background = 10;

        valueMask.setOn(0);
        valueMask.setOn(4);
        CPPUNIT_ASSERT_EQUAL(valueMask.countOn(), Index32(2));

        openvdb::io::MaskCompress<ValueT, MaskT> maskCompress(
            valueMask, childMask, values.data(), background);

        CPPUNIT_ASSERT_EQUAL(int(maskCompress.metadata), int(openvdb::io::MASK_AND_ONE_INACTIVE_VAL));
        CPPUNIT_ASSERT_EQUAL(maskCompress.inactiveVal[0], 500);
        CPPUNIT_ASSERT_EQUAL(maskCompress.inactiveVal[1], background);
    }

    { // mask selects between two non-background inactive vals
        MaskT valueMask;
        MaskT childMask;
        std::vector<int> values = {0,500,500,2000,4,500,2000,500};
        int background = 10;

        valueMask.setOn(0);
        valueMask.setOn(4);
        CPPUNIT_ASSERT_EQUAL(valueMask.countOn(), Index32(2));

        openvdb::io::MaskCompress<ValueT, MaskT> maskCompress(
            valueMask, childMask, values.data(), background);

        CPPUNIT_ASSERT_EQUAL(int(maskCompress.metadata), int(openvdb::io::MASK_AND_TWO_INACTIVE_VALS));
        CPPUNIT_ASSERT_EQUAL(maskCompress.inactiveVal[0], 500); // first unique value
        CPPUNIT_ASSERT_EQUAL(maskCompress.inactiveVal[1], 2000); // second unique value
    }

    { // mask selects between two non-background inactive vals
        MaskT valueMask;
        MaskT childMask;
        std::vector<int> values = {0,2000,2000,500,4,2000,500,2000};
        int background = 10;

        valueMask.setOn(0);
        valueMask.setOn(4);
        CPPUNIT_ASSERT_EQUAL(valueMask.countOn(), Index32(2));

        openvdb::io::MaskCompress<ValueT, MaskT> maskCompress(
            valueMask, childMask, values.data(), background);

        CPPUNIT_ASSERT_EQUAL(int(maskCompress.metadata), int(openvdb::io::MASK_AND_TWO_INACTIVE_VALS));
        CPPUNIT_ASSERT_EQUAL(maskCompress.inactiveVal[0], 2000); // first unique value
        CPPUNIT_ASSERT_EQUAL(maskCompress.inactiveVal[1], 500); // second unique value
    }

    { // > 2 inactive vals, so no mask compression at all
        MaskT valueMask;
        MaskT childMask;
        std::vector<int> values = {0,1000,2000,3000,4,2000,500,2000};
        int background = 10;

        valueMask.setOn(0);
        valueMask.setOn(4);
        CPPUNIT_ASSERT_EQUAL(valueMask.countOn(), Index32(2));

        openvdb::io::MaskCompress<ValueT, MaskT> maskCompress(
            valueMask, childMask, values.data(), background);

        CPPUNIT_ASSERT_EQUAL(int(maskCompress.metadata), int(openvdb::io::NO_MASK_AND_ALL_VALS));
        CPPUNIT_ASSERT_EQUAL(maskCompress.inactiveVal[0], 1000); // first unique value
        CPPUNIT_ASSERT_EQUAL(maskCompress.inactiveVal[1], 2000); // second unique value
    }

    { // mask selects between two non-background inactive vals (selective child mask)
        MaskT valueMask;
        MaskT childMask;
        std::vector<int> values = {0,1000,2000,3000,4,2000,500,2000};
        int background = 0;

        valueMask.setOn(0);
        valueMask.setOn(4);
        CPPUNIT_ASSERT_EQUAL(valueMask.countOn(), Index32(2));

        childMask.setOn(3);
        childMask.setOn(6);
        CPPUNIT_ASSERT_EQUAL(childMask.countOn(), Index32(2));

        openvdb::io::MaskCompress<ValueT, MaskT> maskCompress(
            valueMask, childMask, values.data(), background);

        CPPUNIT_ASSERT_EQUAL(int(maskCompress.metadata), int(openvdb::io::MASK_AND_TWO_INACTIVE_VALS));
        CPPUNIT_ASSERT_EQUAL(maskCompress.inactiveVal[0], 1000); // first unique value
        CPPUNIT_ASSERT_EQUAL(maskCompress.inactiveVal[1], 2000); // secone unique value
    }
}




