/*
 * Copyright 2013 Computer Graphics Group, RWTH Aachen University
 * Author: Hans-Christian Ebke <ebke@cs.rwth-aachen.de>
 *
 * This file is part of QEx.
 *
 * QEx is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * QEx is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with QEx.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>
#include "../../src/Algebra.hh"

namespace {

using QEx::Vector_2;
using QEx::Matrix;

class Test_Vector_2: public testing::Test {
//    protected:
//        virtual void SetUp() {}
//        virtual void TearDown() {}
};

TEST_F(Test_Vector_2, eq) {
    EXPECT_EQ(Vector_2(1, 2), Vector_2(1, 2));
    EXPECT_EQ(Vector_2(1.4, 2.5), Vector_2(1.4, 2.5));
    EXPECT_EQ(Vector_2(-1, -2), Vector_2(-1, -2));
    EXPECT_EQ(Vector_2(0, 0), Vector_2(0, 0));

    EXPECT_NE(Vector_2(1, 2), Vector_2(1, 2.0001));

    EXPECT_NE(Vector_2(1, 2), Vector_2(-1, 2));
    EXPECT_NE(Vector_2(1, 2), Vector_2(1, -2));

    EXPECT_NE(Vector_2(-1, 2), Vector_2(1, 2));
    EXPECT_NE(Vector_2(1, -2), Vector_2(1, 2));
}

TEST_F(Test_Vector_2, plus) {
    Vector_2 va(1, 2);
    Vector_2 vb(-3, 4.2);
    EXPECT_EQ(Vector_2(-2, 6.2), va + vb);
}

TEST_F(Test_Vector_2, minus_infix) {
    Vector_2 va(1, 2);
    Vector_2 vb(-3, 4.2);
    EXPECT_EQ(Vector_2(4, -2.2), va - vb);
}

TEST_F(Test_Vector_2, minus_prefix) {
    EXPECT_EQ(Vector_2(3, -2), -Vector_2(-3, 2));
}

TEST_F(Test_Vector_2, div_eq) {
    Vector_2 va(4, 3);
    va /= 2.0;
    EXPECT_EQ(Vector_2(2, 1.5), va);
}

TEST_F(Test_Vector_2, mul_eq) {
    Vector_2 va(4.2, 3);
    va *= 2.0;
    EXPECT_EQ(Vector_2(8.4, 6), va);
}

TEST_F(Test_Vector_2, transpose) {
    Vector_2 va(4.2, 3);
    Matrix<1, 2> expected; expected << 4.2, 3.0;
    Matrix<1, 2> actual = va.transpose();
    EXPECT_EQ(expected, actual);
}

} /* anonymous namespace */
