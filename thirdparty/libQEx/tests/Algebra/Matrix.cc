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

using QEx::Matrix;
using QEx::Vec3d;

class Test_Matrix: public testing::Test {
};

TEST_F(Test_Matrix, stream_operator) {
    Matrix<2, 3> m;
    m << 1, 2, 3,
         4, 5, 6;

    EXPECT_EQ(1, m.v[0]);
    EXPECT_EQ(2, m.v[1]);
    EXPECT_EQ(3, m.v[2]);
    EXPECT_EQ(4, m.v[3]);
    EXPECT_EQ(5, m.v[4]);
    EXPECT_EQ(6, m.v[5]);

    m << 7, 8, 9;

    EXPECT_EQ(7, m.v[0]);
    EXPECT_EQ(8, m.v[1]);
    EXPECT_EQ(9, m.v[2]);
    EXPECT_EQ(4, m.v[3]);
    EXPECT_EQ(5, m.v[4]);
    EXPECT_EQ(6, m.v[5]);
}

TEST_F(Test_Matrix, braces_operator) {
    Matrix<2, 3> m;
    m << 1, 2, 3,
         4, 5, 6;
    EXPECT_EQ(1, m(0, 0));
    EXPECT_EQ(2, m(0, 1));
    EXPECT_EQ(3, m(0, 2));
    EXPECT_EQ(4, m(1, 0));
    EXPECT_EQ(5, m(1, 1));
    EXPECT_EQ(6, m(1, 2));

    /*
     * Test const version, too.
     */
    const Matrix<2, 3> m1(m);
    EXPECT_EQ(1, m1(0, 0));
    EXPECT_EQ(2, m1(0, 1));
    EXPECT_EQ(3, m1(0, 2));
    EXPECT_EQ(4, m1(1, 0));
    EXPECT_EQ(5, m1(1, 1));
    EXPECT_EQ(6, m1(1, 2));

    /*
     * Test assignment.
     */
    m(0, 2) = 42;

    EXPECT_EQ(1, m(0, 0));
    EXPECT_EQ(2, m(0, 1));
    EXPECT_EQ(42, m(0, 2));
    EXPECT_EQ(4, m(1, 0));
    EXPECT_EQ(5, m(1, 1));
    EXPECT_EQ(6, m(1, 2));
}

TEST_F(Test_Matrix, eq) {
    Matrix<3, 3> ma;
    ma << 1, 2, 3,
          4, 5, 6,
          7, 8, 9;

    Matrix<3, 3> mb;
    mb << 0, 2, 3,
          4, 5, 6,
          7, 8, 9;


    Matrix<3, 3> mc;
    mc << 1, 2, 3,
          4, 5, 6,
          7, 8, 0;

    EXPECT_EQ(ma, ma);
    EXPECT_FALSE(ma == mb);
    EXPECT_FALSE(ma == mc);
    EXPECT_FALSE(mb == mc);
}

TEST_F(Test_Matrix, inverse) {
    {
        Matrix<3, 3> m;
        m << 1, 3, 3,
             1, 4, 3,
             1, 3, 4;

        Matrix<3, 3> inv;
        inv <<  7, -3, -3,
               -1,  1,  0,
               -1,  0,  1;

        EXPECT_EQ(inv, m.inverse());
    }
}

TEST_F(Test_Matrix, matrix_mult) {
    Matrix<2, 3> ma;
    ma << 1, 2, 3,
          4, 5, 6;
    Matrix<3, 4> mb;
    mb <<  7,  8,  9, 10,
          11, 12, 13, 14,
          15, 16, 17, 18;

    Matrix<2, 4> result;
    result <<  74,  80,  86,  92,
              173, 188, 203, 218;

    EXPECT_EQ(result, ma * mb);
}

TEST_F(Test_Matrix, col) {
    Matrix<3, 2> ma;
    ma << 1, 2,
          3, 4,
          5, 6;
    ma.col(0) = Vec3d(11, 12, 13);
    EXPECT_EQ(11, ma(0, 0));
    EXPECT_EQ(12, ma(1, 0));
    EXPECT_EQ(13, ma(2, 0));

    EXPECT_EQ(2, ma(0, 1));
    EXPECT_EQ(4, ma(1, 1));
    EXPECT_EQ(6, ma(2, 1));

    ma.col(1) = Vec3d(14, 15, 16);
    EXPECT_EQ(11, ma(0, 0));
    EXPECT_EQ(12, ma(1, 0));
    EXPECT_EQ(13, ma(2, 0));

    EXPECT_EQ(14, ma(0, 1));
    EXPECT_EQ(15, ma(1, 1));
    EXPECT_EQ(16, ma(2, 1));
}

TEST_F(Test_Matrix, block) {
    Matrix<3, 3> ma;
    ma << 1, 2, 3,
          4, 5, 6,
          7, 8, 9;

    Matrix<2, 2> mb;
    mb << 10, 11,
          12, 13;

    ma.block<2, 2>(1, 1) = mb;

    EXPECT_EQ(1, ma(0, 0));
    EXPECT_EQ(2, ma(0, 1));
    EXPECT_EQ(3, ma(0, 2));

    EXPECT_EQ(4, ma(1, 0));
    EXPECT_EQ(10, ma(1, 1));
    EXPECT_EQ(11, ma(1, 2));

    EXPECT_EQ(7, ma(2, 0));
    EXPECT_EQ(12, ma(2, 1));
    EXPECT_EQ(13, ma(2, 2));
}

TEST_F(Test_Matrix, mul_eq) {
    Matrix<3, 3> ma;
    ma << 1, 2, 3,
          4, 5, 6,
          7, 8, 9;

    ma *= 2.5;

    Matrix<3, 3> mb;
    mb <<  2.5,  5.0,  7.5,
          10.0, 12.5, 15.0,
          17.5, 20.0, 22.5;

    EXPECT_EQ(mb, ma);
}

TEST_F(Test_Matrix, div_eq) {
    Matrix<3, 3> ma;
    ma << 1, 2, 3,
          4, 5, 6,
          7, 8, 9;

    Matrix<3, 3> mb;
    mb <<  2.5,  5.0,  7.5,
          10.0, 12.5, 15.0,
          17.5, 20.0, 22.5;

    mb /= 2.5;

    EXPECT_EQ(ma, mb);
}

TEST_F(Test_Matrix, vec_mul_matrix) {
    Vec3d vec(1, 2, 3);
    Matrix<1, 2> mat;
    mat << 4, 5;
    Matrix<3, 2> res;
    res <<  4,  5,
            8, 10,
           12, 15;
    EXPECT_EQ(res, vec * mat);
}

} /* anonymous namespace */
