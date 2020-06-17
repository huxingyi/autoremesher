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

#include <limits>
#include <gtest/gtest.h>
#include "../../src/Algebra.hh"

namespace {

using QEx::Segment_2;
using QEx::Triangle_2;
using QEx::Bbox_2;
using QEx::Point_2;
using QEx::Vec2d;

class Test_Segment_2: public testing::Test {
    protected:
        void rot90(Point_2 &in) {
            in = Point_2(-in[1], in[0]);
        }

        void has_on(bool expected, const Point_2 &a, const Point_2 &b, const Point_2 &c) {
            EXPECT_EQ(expected, Segment_2(a, b).has_on(c))
                    << "Segment_2(" << a << ", " << b << ").has_on(" << c << ") != " << expected;
            EXPECT_EQ(expected, Segment_2(b, a).has_on(c))
                    << "Segment_2(" << b << ", " << a << ").has_on(" << c << ") != " << expected;
        }
        void has_on_all_perms(bool expected, Point_2 a, Point_2 b, Point_2 c) {
            has_on(expected, a, b, c);
            rot90(a); rot90(b); rot90(c);
            has_on(expected, a, b, c);
            rot90(a); rot90(b); rot90(c);
            has_on(expected, a, b, c);
            rot90(a); rot90(b); rot90(c);
            has_on(expected, a, b, c);
        }

        void intersects_segment(bool expected, const Point_2 &a1, const Point_2 &b1, const Point_2 &a2, const Point_2 &b2) {
            EXPECT_EQ(expected, Segment_2(a1, b1).intersects(Segment_2(a2, b2)))
                << "Segment_2(" << a1 << ", " << b1 << ").intersects(Segment_2(" << a2 << ", " << b2 << "))";
            EXPECT_EQ(expected, Segment_2(a2, b2).intersects(Segment_2(a1, b1)))
                << "Segment_2(" << a2 << ", " << b2 << ").intersects(Segment_2(" << a1 << ", " << b1 << "))";
            EXPECT_EQ(expected, Segment_2(b1, a1).intersects(Segment_2(b2, a2)))
                << "Segment_2(" << b1 << ", " << a1 << ").intersects(Segment_2(" << b2 << ", " << a2 << "))";
            EXPECT_EQ(expected, Segment_2(b2, a2).intersects(Segment_2(b1, a1)))
                << "Segment_2(" << b2 << ", " << a2 << ").intersects(Segment_2(" << b1 << ", " << a1 << "))";
        }

        void intersects_segment_all_perms(bool expected, Point_2 a1, Point_2 b1, Point_2 a2, Point_2 b2) {
            intersects_segment(expected, a1, b1, a2, b2);
            rot90(a1); rot90(b1); rot90(a2); rot90(b2);
            intersects_segment(expected, a1, b1, a2, b2);
            rot90(a1); rot90(b1); rot90(a2); rot90(b2);
            intersects_segment(expected, a1, b1, a2, b2);
            rot90(a1); rot90(b1); rot90(a2); rot90(b2);
            intersects_segment(expected, a1, b1, a2, b2);
        }

        void intersects_triangle(bool expected, const Point_2 &a1, const Point_2 &b1, const Point_2 &a2, const Point_2 &b2, const Point_2 &c2) {
            EXPECT_EQ(expected, Segment_2(a1, b1).intersects(Triangle_2(a2, b2, c2)))
                << "Segment_2(" << a1 << ", " << b1 << ").intersects(Triangle_2(" << a2 << ", " << b2 << ", " << c2 << "))";
            EXPECT_EQ(expected, Segment_2(b1, a1).intersects(Triangle_2(a2, b2, c2)))
                << "Segment_2(" << a1 << ", " << b1 << ").intersects(Triangle_2(" << a2 << ", " << b2 << ", " << c2 << "))";
            EXPECT_EQ(expected, Segment_2(a1, b1).intersects(Triangle_2(b2, c2, a2)))
                << "Segment_2(" << a1 << ", " << b1 << ").intersects(Triangle_2(" << a2 << ", " << b2 << ", " << c2 << "))";
            EXPECT_EQ(expected, Segment_2(b1, a1).intersects(Triangle_2(b2, c2, a2)))
                << "Segment_2(" << a1 << ", " << b1 << ").intersects(Triangle_2(" << a2 << ", " << b2 << ", " << c2 << "))";
            EXPECT_EQ(expected, Segment_2(a1, b1).intersects(Triangle_2(c2, a2, b2)))
                << "Segment_2(" << a1 << ", " << b1 << ").intersects(Triangle_2(" << a2 << ", " << b2 << ", " << c2 << "))";
            EXPECT_EQ(expected, Segment_2(b1, a1).intersects(Triangle_2(c2, a2, b2)))
                << "Segment_2(" << a1 << ", " << b1 << ").intersects(Triangle_2(" << a2 << ", " << b2 << ", " << c2 << "))";
        }

        void intersects_triangle_all_perms(bool expected, Point_2 a1, Point_2 b1, Point_2 a2, Point_2 b2, Point_2 c2) {
            intersects_triangle(expected, a1, b1, a2, b2, c2);
            rot90(a1); rot90(b1); rot90(a2); rot90(b2); rot90(c2);
            intersects_triangle(expected, a1, b1, a2, b2, c2);
            rot90(a1); rot90(b1); rot90(a2); rot90(b2); rot90(c2);
            intersects_triangle(expected, a1, b1, a2, b2, c2);
            rot90(a1); rot90(b1); rot90(a2); rot90(b2); rot90(c2);
            intersects_triangle(expected, a1, b1, a2, b2, c2);
        }
};

TEST_F(Test_Segment_2, is_degenerate) {
    EXPECT_TRUE(Segment_2(Point_2(1.2, 2.5), Point_2(1.2, 2.5)).is_degenerate());
    EXPECT_TRUE(Segment_2(Point_2(1, 2.5), Point_2(1, 2.5)).is_degenerate());
    EXPECT_FALSE(Segment_2(Point_2(1 + std::numeric_limits<double>::epsilon(), 2.5), Point_2(1, 2.5)).is_degenerate());
    EXPECT_FALSE(Segment_2(Point_2(1, 2.5), Point_2(1 + std::numeric_limits<double>::epsilon(), 2.5)).is_degenerate());
    EXPECT_FALSE(Segment_2(Point_2(0, 0), Point_2(1.5, 2.5)).is_degenerate());
}

TEST_F(Test_Segment_2, bbox) {
    EXPECT_EQ(Bbox_2(Vec2d(1.2, 2.5), Vec2d(1.2, 2.5)),
            Segment_2(Point_2(1.2, 2.5), Point_2(1.2, 2.5)).bbox());
    EXPECT_EQ(Bbox_2(Vec2d(0.2, 2.5), Vec2d(1.2, 2.5)),
            Segment_2(Point_2(1.2, 2.5), Point_2(0.2, 2.5)).bbox());
    EXPECT_EQ(Bbox_2(Vec2d(1.2, 2.0), Vec2d(1.2, 2.5)),
            Segment_2(Point_2(1.2, 2.0), Point_2(1.2, 2.5)).bbox());
    EXPECT_EQ(Bbox_2(Vec2d(-1.2, 2.0), Vec2d(1.2, 2.5)),
            Segment_2(Point_2(-1.2, 2.0), Point_2(1.2, 2.5)).bbox());
}

TEST_F(Test_Segment_2, has_on) {
    has_on_all_perms(false, Point_2(0, 0), Point_2(5, 6.0), Point_2(1, 1.2));
    has_on_all_perms(false, Point_2(8, 8), Point_2(13, 14.0), Point_2(9, 9.2));
    has_on_all_perms(false, Point_2(8.0, 8.0), Point_2(8.7, 11.5), Point_2(8.25, 9.25));
    has_on_all_perms(false, Point_2(0.0, 0.0), Point_2(.7, 3.5), Point_2(.25, 1.25));

    has_on_all_perms(false, Point_2(0, 0), Point_2(5, 6.0), Point_2(10, 12));
    has_on_all_perms(true, Point_2(0, 0), Point_2(10, 12), Point_2(5, 6.0));
    has_on_all_perms(true, Point_2(0, 0), Point_2(4, 8.0), Point_2(2, 4));

    has_on_all_perms(false, Point_2(8, 5), Point_2(13, 11.0), Point_2(18, 17));
    has_on_all_perms(true, Point_2(8, 5), Point_2(12, 13.0), Point_2(10, 9));

    // Test w/ degenerated segment.
    has_on_all_perms(true, Point_2(3.2, 2.5), Point_2(3.2, 2.5), Point_2(3.2, 2.5));
    has_on_all_perms(false, Point_2(3.2, 2.5), Point_2(3.2, 2.5), Point_2(3.2, 2.6));
}

TEST_F(Test_Segment_2, intersects_segment) {
    intersects_segment_all_perms(true, Point_2(1, 1), Point_2(2, 2), Point_2(1.2, 1.2), Point_2(2.2, 2.2));
    intersects_segment_all_perms(false, Point_2(0, 0), Point_2(2, 6), Point_2(1.2, 3.6), Point_2(3, 9));

    intersects_segment_all_perms(true, Point_2(1, 1), Point_2(2, 2), Point_2(1.3, 1.3), Point_2(2.2, 2.2));
    intersects_segment_all_perms(true, Point_2(1, 1), Point_2(2, 2), Point_2(1.5, 1.5), Point_2(2.5, 2.5));
    intersects_segment_all_perms(true, Point_2(1, 1), Point_2(2, 2), Point_2(1.5, 1.5), Point_2(2.2, 2.2));
    intersects_segment_all_perms(false, Point_2(1, 1), Point_2(2, 2), Point_2(2.2, 2.2), Point_2(3, 3));
    intersects_segment_all_perms(true, Point_2(1, 1), Point_2(2, 2), Point_2(1, 1), Point_2(.3, .3));
    intersects_segment_all_perms(false, Point_2(1 + std::numeric_limits<double>::epsilon(), 1), Point_2(2, 2), Point_2(1, 1), Point_2(.3, .3));

    intersects_segment_all_perms(false, Point_2(0, 3), Point_2(0, 4), Point_2(0, 5), Point_2(0, 6));
    intersects_segment_all_perms(true, Point_2(0, 3), Point_2(0, 4), Point_2(0, 4), Point_2(0, 6));

    intersects_segment_all_perms(true, Point_2(1.2, 2), Point_2(3, 4), Point_2(2.4, 2), Point_2(1.2, 4));
}

TEST_F(Test_Segment_2, intersects_triangle) {
    intersects_triangle_all_perms(true, Point_2(2.5, 2), Point_2(2.5, 4), Point_2(1, 1), Point_2(4, 2), Point_2(3, 3));
    intersects_triangle_all_perms(true, Point_2(2.5, 2), Point_2(3.5, 4), Point_2(1, 1), Point_2(4, 2), Point_2(3, 3));
    intersects_triangle_all_perms(true, Point_2(1.5, 0.5), Point_2(1.5, 0.5), Point_2(0, 0), Point_2(3, 0), Point_2(1.5, 2));
    intersects_triangle_all_perms(true, Point_2(0.0, 2.0), Point_2(3.0, 2.0), Point_2(0, 0), Point_2(3, 0), Point_2(1.5, 2));
    intersects_triangle_all_perms(true, Point_2(0.0, 2.0), Point_2(1.5, 2.0), Point_2(0, 0), Point_2(3, 0), Point_2(1.5, 2));
    intersects_triangle_all_perms(true, Point_2(1.5, 2.0), Point_2(1.5, 2.0), Point_2(0, 0), Point_2(3, 0), Point_2(1.5, 2));
    intersects_triangle_all_perms(true, Point_2(1.5, 0), Point_2(1.5, -.5), Point_2(0, 0), Point_2(3, 0), Point_2(1.5, 2));
    intersects_triangle_all_perms(true, Point_2(0, 2), Point_2(2, -1), Point_2(0, 0), Point_2(3, 0), Point_2(1.5, 2));
    intersects_triangle_all_perms(true, Point_2(1.5, 3), Point_2(1.5, -1), Point_2(0, 0), Point_2(3, 0), Point_2(1.5, 2));
    intersects_triangle_all_perms(true, Point_2(3, 4), Point_2(4, 5), Point_2(3.5, 4.5), Point_2(3.5, 4.5), Point_2(3.5, 4.5));
    intersects_triangle_all_perms(true, Point_2(-1.5, -2), Point_2(3, 4), Point_2(0, 0), Point_2(3, 0), Point_2(1.5, 2));
    intersects_triangle_all_perms(false, Point_2(-1.5001, -2), Point_2(3, 4), Point_2(0, 0), Point_2(3, 0), Point_2(1.5, 2));
    intersects_triangle_all_perms(false, Point_2(-1.5, -2 + std::numeric_limits<double>::epsilon()), Point_2(3, 4), Point_2(0, 0), Point_2(3, 0), Point_2(1.5, 2));
    const double
        dmin = std::numeric_limits<double>::min(),
        dmax = std::numeric_limits<double>::max(),
        eps = std::numeric_limits<double>::epsilon();
    intersects_triangle_all_perms(true, Point_2(dmin, dmin), Point_2(dmax, dmax), Point_2(-eps, 0), Point_2(eps, 0), Point_2(0, eps));
    intersects_triangle_all_perms(true, Point_2(-1, -1), Point_2(1, 1), Point_2(-eps, 0), Point_2(eps, 0), Point_2(0, eps));
    intersects_triangle_all_perms(true, Point_2(-1-eps, -1), Point_2(1, 1), Point_2(-eps, 0), Point_2(eps, 0), Point_2(0, eps));
    intersects_triangle_all_perms(true, Point_2(-1-2.0 * eps, -1), Point_2(1, 1), Point_2(-eps, 0), Point_2(eps, 0), Point_2(0, eps));
    intersects_triangle_all_perms(false, Point_2(-1-3.0 * eps, -1), Point_2(1, 1), Point_2(-eps, 0), Point_2(eps, 0), Point_2(0, eps));
}

TEST_F(Test_Segment_2, bracket_operator) {
    Segment_2 s(Point_2(1.23, 4.56), Point_2(7.89, 10.1112));
    EXPECT_EQ(Point_2(1.23, 4.56), s[0]);
    EXPECT_EQ(Point_2(7.89, 10.1112), s[1]);
}

} /* anonymous namespace */
