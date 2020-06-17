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

using QEx::Point_2;

class Test_has_on_assume_collinear: public testing::Test {
    protected:
        Point_2 rot90(const Point_2 &in) {
            return Point_2(-in[1], in[0]);
        }

        Point_2 rot180(const Point_2 &in) {
            return Point_2(-in[0], -in[1]);
        }

        Point_2 rot270(const Point_2 &in) {
            return Point_2(in[1], -in[0]);
        }

        void all_perms(bool expected,
                const Point_2 &a, const Point_2 &b, const Point_2 &c) {

            EXPECT_EQ(expected, QEx::has_on_assume_collinear(a, b, c))
                    << "QEx::has_on_assume_collinear(" << a << ", " << b << ", " << c << ") != " << expected;
            EXPECT_EQ(expected, QEx::has_on_assume_collinear(rot90(a), rot90(b), rot90(c)))
                    << "QEx::has_on_assume_collinear[rot90](" << a << ", " << b << ", " << c << ") != " << expected;
            EXPECT_EQ(expected, QEx::has_on_assume_collinear(rot180(a), rot180(b), rot180(c)))
                    << "QEx::has_on_assume_collinear[rot180](" << a << ", " << b << ", " << c << ") != " << expected;
            EXPECT_EQ(expected, QEx::has_on_assume_collinear(rot270(a), rot270(b), rot270(c)))
                    << "QEx::has_on_assume_collinear[rot270](" << a << ", " << b << ", " << c << ") != " << expected;

            if (expected == true && a != b && a != c && b != c) {
                all_perms(false, a, c, b);
                all_perms(false, b, c, a);
            }
        }
};

TEST_F(Test_has_on_assume_collinear, axis_aligned) {
    all_perms(true, Point_2(0, 0), Point_2(1, 0), Point_2(.5, 0));
    all_perms(true, Point_2(-1, 0), Point_2(1, 0), Point_2(.5, 0));
    all_perms(true, Point_2(3.4, 2), Point_2(4.7, 2), Point_2(4.1, 2));

    all_perms(false, Point_2(0, 0), Point_2(1, 0), Point_2(1.2, 0));
    all_perms(false, Point_2(-1, 0), Point_2(1, 0), Point_2(2, 0));
    all_perms(false, Point_2(3.4, 2), Point_2(4.7, 2), Point_2(4.8, 2));
    all_perms(false, Point_2(3.4, 2), Point_2(4.7, 2), Point_2(-200, 2));
}

TEST_F(Test_has_on_assume_collinear, boundary) {
    all_perms(true, Point_2(0, 0), Point_2(1, 0), Point_2(1, 0));
    all_perms(true, Point_2(-1, 0), Point_2(1, 0), Point_2(-1, 0));
    all_perms(true, Point_2(.123, 17.4), Point_2(19.3, -7), Point_2(19.3, -7));
    all_perms(true, Point_2(.123, 17.4), Point_2(19.3, -7), Point_2(.123, 17.4));
}

TEST_F(Test_has_on_assume_collinear, arbitrary) {
    all_perms(false, Point_2(.123, 17.4), Point_2(19.3, -7), Point_2(19.9, -7));
    all_perms(false, Point_2(.123, 17.4), Point_2(19.3, -7), Point_2(19.3, -7.2));
    all_perms(false, Point_2(.123, 17.4), Point_2(19.3, -7), Point_2(123, 17.4));
    all_perms(false, Point_2(.123, 17.4), Point_2(19.3, -7), Point_2(.123, 174));

    all_perms(true, Point_2(-.1, 0), Point_2(.8, 0), Point_2(.5, 0));
    all_perms(true, Point_2(-.3, -.3), Point_2(.8, .8), Point_2(.5, .5));
    all_perms(true, Point_2(-.3, -.3), Point_2(.8, .8), Point_2(0, 0));
    all_perms(true, Point_2(1, 1), Point_2(5, 3), Point_2(3, 2));
    all_perms(true, Point_2(0, 0), Point_2(7.5, 5), Point_2(1.5, 1));
    all_perms(true, Point_2(0, 0), Point_2(5, 7.5), Point_2(1, 1.5));
}

TEST_F(Test_has_on_assume_collinear, degenerate) {
    all_perms(true, Point_2(3, 7), Point_2(3, 7), Point_2(3, 7));
    all_perms(true, Point_2(0, 0), Point_2(0, 0), Point_2(0, 0));
    all_perms(false, Point_2(3, 7), Point_2(3, 7), Point_2(4, 7));
    all_perms(false, Point_2(0, 0), Point_2(0, 0), Point_2(1, 0));
    all_perms(false, Point_2(1, 1), Point_2(1, 1), Point_2(1 + std::numeric_limits<double>::epsilon(), 1));
}

TEST_F(Test_has_on_assume_collinear, epsilon) {
    all_perms(true, Point_2(0, 0), Point_2(1, 0), Point_2(1 - std::numeric_limits<double>::epsilon(), 0));
    all_perms(false, Point_2(0, 0), Point_2(1, 0), Point_2(1 + std::numeric_limits<double>::epsilon(), 0));
    all_perms(false, Point_2(-1, 0), Point_2(1, 0), Point_2(1 + std::numeric_limits<double>::epsilon(), 0));

    all_perms(true, Point_2(1, 1), Point_2(1 + std::numeric_limits<double>::epsilon(), 1), Point_2(1, 1));
    all_perms(true, Point_2(1 - std::numeric_limits<double>::epsilon(), 1), Point_2(1 + std::numeric_limits<double>::epsilon(), 1), Point_2(1, 1));
    all_perms(false, Point_2(1, 1), Point_2(1 + std::numeric_limits<double>::epsilon(), 1), Point_2(1 - std::numeric_limits<double>::epsilon(), 1));
}

} /* anonymous namespace */
