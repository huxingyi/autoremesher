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

using QEx::Point_2;
using QEx::Vector_2;

class Test_Point_2: public testing::Test {
};

TEST_F(Test_Point_2, plus) {
    Point_2 point(1, 2);
    Vector_2 vec(-3, 4.2);
    EXPECT_EQ(Point_2(-2, 6.2), point + vec);
}

TEST_F(Test_Point_2, minus_infix_vector) {
    Point_2 point(1, 2);
    Vector_2 vec(-3, 4.2);
    EXPECT_EQ(Point_2(4, -2.2), point - vec);
}

TEST_F(Test_Point_2, minus_infix_point) {
    Point_2 pa(1, 2);
    Point_2 pb(-3, 4.2);
    EXPECT_EQ(Vector_2(4, -2.2), pa - pb);
}

} /* anonymous namespace */
