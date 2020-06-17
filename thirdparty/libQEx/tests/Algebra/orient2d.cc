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
using QEx::Vec2d;

class Test_orient2d: public testing::Test {
    protected:
        static void SetUpTestCase() {
            exactinit();
        }

        void all_perms(ORIENTATION expected,
                const Point_2 &a, const Point_2 &b, const Point_2 &c) {

            ORIENTATION actual = QEx::orient2d(a, b, c);
            EXPECT_EQ(expected, actual)
                << std::setprecision(30)
                << "orient2d(" << a << ", " << b << ", " << c
                << ") yields " << Orientation2Str(actual)
                << " but expected " << Orientation2Str(expected);

            actual = orient2d(b, c, a);
            EXPECT_EQ(expected, QEx::orient2d(b, c, a))
                << std::setprecision(30)
                << "orient2d(" << b << ", " << c << ", " << a
                << ") yields " << Orientation2Str(actual)
                << " but expected " << Orientation2Str(expected);

            actual = orient2d(c, a, b);
            EXPECT_EQ(expected, QEx::orient2d(c, a, b))
                << std::setprecision(30)
                << "orient2d(" << c << ", " << a << ", " << b
                << ") yields " << Orientation2Str(actual)
                << " but expected " << Orientation2Str(expected);

            if (expected == ORI_COLLINEAR) {
                EXPECT_TRUE(QEx::is_collinear(a, b, c));
                EXPECT_TRUE(QEx::is_collinear(a, c, b));
                EXPECT_TRUE(QEx::is_collinear(b, a, c));
                EXPECT_TRUE(QEx::is_collinear(b, c, a));
                EXPECT_TRUE(QEx::is_collinear(c, a, b));
                EXPECT_TRUE(QEx::is_collinear(c, b, a));
            } else {
                EXPECT_FALSE(QEx::is_collinear(a, b, c));
                EXPECT_FALSE(QEx::is_collinear(a, c, b));
                EXPECT_FALSE(QEx::is_collinear(b, a, c));
                EXPECT_FALSE(QEx::is_collinear(b, c, a));
                EXPECT_FALSE(QEx::is_collinear(c, a, b));
                EXPECT_FALSE(QEx::is_collinear(c, b, a));
            }

            EXPECT_EQ(expected, QEx::Triangle_2(a, b, c).orientation());
            EXPECT_EQ(expected, QEx::Triangle_2(b, c, a).orientation());
            EXPECT_EQ(expected, QEx::Triangle_2(c, a, b).orientation());
        }

};

TEST_F(Test_orient2d, Point_2_ccw) {
    all_perms(ORI_CCW, Point_2(0, .5), Point_2(.5, 0), Point_2(1, 1));
    all_perms(ORI_CCW, Point_2(-1, -1), Point_2(1, -1), Point_2(0, 1.5));

    /*
     * These are not collinear since double(1.2) != 1.2, etc.
     */
    all_perms(ORI_CCW, Point_2(0, 0), Point_2(1, 1.2), Point_2(5, 6.0));
    all_perms(ORI_CCW, Point_2(8, 8), Point_2(9, 9.2), Point_2(13, 14.0));
    all_perms(ORI_CCW, Point_2(8.0, 8.0), Point_2(8.25, 9.25), Point_2(8.7, 11.5));
    all_perms(ORI_CCW, Point_2(0.0, 0.0), Point_2(.25, 1.25), Point_2(.7, 3.5));
}

TEST_F(Test_orient2d, Point_2_cw) {
    all_perms(ORI_CW, Point_2(.5, 0), Point_2(0, .5), Point_2(1, 1));
    all_perms(ORI_CW, Point_2(1, -1), Point_2(-1, -1), Point_2(0, 1.5));

    /*
     * These are not collinear since double(1.2) != 1.2, etc.
     */
    all_perms(ORI_CW, Point_2(0, 0), Point_2(1.2, 1), Point_2(6.0, 5));
    all_perms(ORI_CW, Point_2(8, 8), Point_2(9.2, 9), Point_2(14.0, 13));
    all_perms(ORI_CW, Point_2(8.0, 8.0), Point_2(9.25, 8.25), Point_2(11.5, 8.7));
    all_perms(ORI_CW, Point_2(0.0, 0.0), Point_2(1.25, .25), Point_2(3.5, .7));
}

TEST_F(Test_orient2d, Point_2_collinear) {
    all_perms(ORI_COLLINEAR, Point_2(-.1, 0), Point_2(.5, 0), Point_2(.8, 0));
    all_perms(ORI_COLLINEAR, Point_2(-.3, -.3), Point_2(.5, .5), Point_2(.8, .8));
    all_perms(ORI_COLLINEAR, Point_2(-.3, -.3), Point_2(0, 0), Point_2(.8, .8));
    all_perms(ORI_COLLINEAR, Point_2(1, 1), Point_2(3, 2), Point_2(5, 3));
    all_perms(ORI_COLLINEAR, Point_2(0, 0), Point_2(1.5, 1), Point_2(7.5, 5));
    all_perms(ORI_COLLINEAR, Point_2(0, 0), Point_2(1, 1.5), Point_2(5, 7.5));
}

TEST_F(Test_orient2d, Vec2d_ccw) {
    EXPECT_EQ(ORI_CCW, QEx::orient2d(Vec2d(1.0, 0), Vec2d(0, 1.0)));

    /*
     * Beware the epsilons.
     */
    EXPECT_EQ(ORI_CCW, QEx::orient2d(Vec2d(1.0, 1.2), Vec2d(5.0, 6.0)));
    EXPECT_EQ(ORI_CCW, QEx::orient2d(Vec2d(.25, 1.25), Vec2d(.7, 3.5)));
}

TEST_F(Test_orient2d, Vec2d_cw) {
    EXPECT_EQ(ORI_CW, QEx::orient2d(Vec2d(0, 1.0), Vec2d(1.0, 0)));

    /*
     * Beware the epsilons.
     */
    EXPECT_EQ(ORI_CW, QEx::orient2d(Vec2d(1.2, 1.0), Vec2d(6.0, 5.0)));
    EXPECT_EQ(ORI_CW, QEx::orient2d(Vec2d(1.25, .25), Vec2d(3.5, .7)));
}

TEST_F(Test_orient2d, Vec2d_collinear) {
    EXPECT_EQ(ORI_COLLINEAR, QEx::orient2d(Vec2d(1.5, 1), Vec2d(7.5, 5)));
    EXPECT_EQ(ORI_COLLINEAR, QEx::orient2d(Vec2d(1, 1.5), Vec2d(5, 7.5)));
    EXPECT_EQ(ORI_COLLINEAR, QEx::orient2d(Vec2d(1.5, .2), Vec2d(3, .4)));

    EXPECT_EQ(ORI_COLLINEAR, QEx::orient2d(Vec2d(-1.5, 1), Vec2d(-7.5, 5)));
    EXPECT_EQ(ORI_COLLINEAR, QEx::orient2d(Vec2d(-1, 1.5), Vec2d(-5, 7.5)));
    EXPECT_EQ(ORI_COLLINEAR, QEx::orient2d(Vec2d(-1.5, .2), Vec2d(-3, .4)));

    EXPECT_EQ(ORI_COLLINEAR, QEx::orient2d(Vec2d(1.5, -1), Vec2d(7.5, -5)));
    EXPECT_EQ(ORI_COLLINEAR, QEx::orient2d(Vec2d(1, -1.5), Vec2d(5, -7.5)));
    EXPECT_EQ(ORI_COLLINEAR, QEx::orient2d(Vec2d(1.5, -.2), Vec2d(3, -.4)));
}

} /* anonymous namespace */
