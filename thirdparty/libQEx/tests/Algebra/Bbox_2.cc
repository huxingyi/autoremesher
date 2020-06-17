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

using QEx::Bbox_2;
using QEx::Vec2d;

class Test_Bbox_2: public testing::Test {
};

TEST_F(Test_Bbox_2, xyminmax) {
    {
        Bbox_2 bb(Vec2d(1, 2), Vec2d(3, 4));
        EXPECT_EQ(1, bb.xmin());
        EXPECT_EQ(2, bb.ymin());
        EXPECT_EQ(3, bb.xmax());
        EXPECT_EQ(4, bb.ymax());
    }
    {
        Bbox_2 bb(Vec2d(1, -2), Vec2d(3, -1));
        EXPECT_EQ(1, bb.xmin());
        EXPECT_EQ(-2, bb.ymin());
        EXPECT_EQ(3, bb.xmax());
        EXPECT_EQ(-1, bb.ymax());
    }
}

} /* anonymous namespace */
