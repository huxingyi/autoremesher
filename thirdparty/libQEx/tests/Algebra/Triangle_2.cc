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

using QEx::Triangle_2;
using QEx::Point_2;
using QEx::Bbox_2;
using QEx::Vec2d;
using QEx::INS_INSIDE;
using QEx::INS_OUTSIDE;
using QEx::INS_BOUNDARY;
using QEx::BND_ON_BOUNDED_SIDE;
using QEx::BND_ON_UNBOUNDED_SIDE;
using QEx::BND_ON_BOUNDARY;

class Test_Triangle_2: public testing::Test {
    private:
        ::testing::AssertionResult insBndSubtest(const Point_2 &a, const Point_2 &b, const Point_2 &c, const Point_2 &tpt, QEx::BOUNDEDNESS expected_boundedness, QEx::INSIDENESS expected_insideness) const {
            QEx::INSIDENESS actual_insideness;
            QEx::BOUNDEDNESS actual_boundedness;
            Triangle_2 current_tri(a, b, c);

            if (expected_boundedness != (actual_boundedness = (current_tri = Triangle_2(a, b, c)).boundedness(tpt)))
                return ::testing::AssertionFailure() << current_tri << ".boundedness(" << tpt << ") was expected to return " << expected_boundedness << ", but did return " << actual_boundedness;
            if (expected_insideness != (actual_insideness = (current_tri = Triangle_2(a, b, c)).insideness(tpt)))
                return ::testing::AssertionFailure() << current_tri << ".insideness(" << tpt << ") was expected to return " << expected_insideness << ", but did return " << actual_insideness;

            if (expected_boundedness != (actual_boundedness = (current_tri = Triangle_2(b, c, a)).boundedness(tpt)))
                return ::testing::AssertionFailure() << current_tri << ".boundedness(" << tpt << ") was expected to return " << expected_boundedness << ", but did return " << actual_boundedness;
            if (expected_insideness != (actual_insideness = (current_tri = Triangle_2(b, c, a)).insideness(tpt)))
                return ::testing::AssertionFailure() << current_tri << ".insideness(" << tpt << ") was expected to return " << expected_insideness << ", but did return " << actual_insideness;

            if (expected_boundedness != (actual_boundedness = (current_tri = Triangle_2(c, a, b)).boundedness(tpt)))
                return ::testing::AssertionFailure() << current_tri << ".boundedness(" << tpt << ") was expected to return " << expected_boundedness << ", but did return " << actual_boundedness;
            if (expected_insideness != (actual_insideness = (current_tri = Triangle_2(c, a, b)).insideness(tpt)))
                return ::testing::AssertionFailure() << current_tri << ".insideness(" << tpt << ") was expected to return " << expected_insideness << ", but did return " << actual_insideness;

            return ::testing::AssertionSuccess();
        }

        void rot90(Point_2 &in) const {
            in = Point_2(-in[1], in[0]);
        }
        void rot90(Point_2 &a, Point_2 &b, Point_2 &c, Point_2 &d) const {
            rot90(a);
            rot90(b);
            rot90(c);
            rot90(d);
        }

    protected:
        ::testing::AssertionResult insBndTest(const Triangle_2 &tri, const Point_2 &pt, QEx::BOUNDEDNESS expected_boundedness, QEx::INSIDENESS expected_insideness) const {
            Point_2 tpt(pt);
            Point_2 a(tri.a), b(tri.b), c(tri.c);

            {
            ::testing::AssertionResult result = insBndSubtest(a, b, c, tpt, expected_boundedness, expected_insideness);
            if (!result) return result;
            }

            rot90(tpt, a, b, c);

            {
            ::testing::AssertionResult result = insBndSubtest(a, b, c, tpt, expected_boundedness, expected_insideness);
            if (!result) return result;
            }

            rot90(tpt, a, b, c);

            {
            ::testing::AssertionResult result = insBndSubtest(a, b, c, tpt, expected_boundedness, expected_insideness);
            if (!result) return result;
            }

            rot90(tpt, a, b, c);

            {
            ::testing::AssertionResult result = insBndSubtest(a, b, c, tpt, expected_boundedness, expected_insideness);
            if (!result) return result;
            }

            return ::testing::AssertionSuccess();
        }

};

TEST_F(Test_Triangle_2, insideness) {
    {
        Triangle_2 tri(Point_2(0, 0), Point_2(5, 6.0), Point_2(0, 4));
        EXPECT_TRUE(insBndTest(tri, Point_2(1, 2), BND_ON_BOUNDED_SIDE, INS_INSIDE));

        EXPECT_TRUE(insBndTest(tri, Point_2(0, 0), BND_ON_BOUNDARY, INS_BOUNDARY));
        EXPECT_TRUE(insBndTest(tri, Point_2(5, 6), BND_ON_BOUNDARY, INS_BOUNDARY));
        EXPECT_TRUE(insBndTest(tri, Point_2(0, 4), BND_ON_BOUNDARY, INS_BOUNDARY));

        EXPECT_TRUE(insBndTest(tri, Point_2(0, 1.2), BND_ON_BOUNDARY, INS_BOUNDARY));
        EXPECT_TRUE(insBndTest(tri, Point_2(2.5, 3.0), BND_ON_BOUNDARY, INS_BOUNDARY));

        /*
         * Outside due to numerical issues.
         */
        EXPECT_TRUE(insBndTest(tri, Point_2(1, 1.2), BND_ON_UNBOUNDED_SIDE, INS_OUTSIDE));
        EXPECT_TRUE(insBndTest(tri, Point_2(3, 3.6), BND_ON_BOUNDED_SIDE, INS_INSIDE));
    }
    {
        Triangle_2 tri(Point_2(0, 0), Point_2(6, 5), Point_2(0, 4));
        EXPECT_TRUE(insBndTest(tri, Point_2(1, 2), BND_ON_BOUNDED_SIDE, INS_INSIDE));

        EXPECT_TRUE(insBndTest(tri, Point_2(0, 0), BND_ON_BOUNDARY, INS_BOUNDARY));
        EXPECT_TRUE(insBndTest(tri, Point_2(6, 5), BND_ON_BOUNDARY, INS_BOUNDARY));
        EXPECT_TRUE(insBndTest(tri, Point_2(0, 4), BND_ON_BOUNDARY, INS_BOUNDARY));

        EXPECT_TRUE(insBndTest(tri, Point_2(0, 1.2), BND_ON_BOUNDARY, INS_BOUNDARY));
        EXPECT_TRUE(insBndTest(tri, Point_2(3.0, 2.5), BND_ON_BOUNDARY, INS_BOUNDARY));

        EXPECT_TRUE(insBndTest(tri, Point_2(0, -1), BND_ON_UNBOUNDED_SIDE, INS_OUTSIDE));
        EXPECT_TRUE(insBndTest(tri, Point_2(0, 5), BND_ON_UNBOUNDED_SIDE, INS_OUTSIDE));

        /*
         * Outside due to numerical issues.
         */
        EXPECT_TRUE(insBndTest(tri, Point_2(1.2, 1), BND_ON_BOUNDED_SIDE, INS_INSIDE));
        EXPECT_TRUE(insBndTest(tri, Point_2(3.6, 3), BND_ON_UNBOUNDED_SIDE, INS_OUTSIDE));
    }
    {
        Triangle_2 tri(Point_2(8, 8), Point_2(13, 14.0), Point_2(8, 12));
        EXPECT_TRUE(insBndTest(tri, Point_2(9, 10), BND_ON_BOUNDED_SIDE, INS_INSIDE));

        EXPECT_TRUE(insBndTest(tri, Point_2(8, 8), BND_ON_BOUNDARY, INS_BOUNDARY));
        EXPECT_TRUE(insBndTest(tri, Point_2(13, 14), BND_ON_BOUNDARY, INS_BOUNDARY));
        EXPECT_TRUE(insBndTest(tri, Point_2(8, 12), BND_ON_BOUNDARY, INS_BOUNDARY));

        EXPECT_TRUE(insBndTest(tri, Point_2(8, 9.2), BND_ON_BOUNDARY, INS_BOUNDARY));
        EXPECT_TRUE(insBndTest(tri, Point_2(10.5, 11.0), BND_ON_BOUNDARY, INS_BOUNDARY));

        EXPECT_TRUE(insBndTest(tri, Point_2(9, 9.2), BND_ON_UNBOUNDED_SIDE, INS_OUTSIDE));
        EXPECT_TRUE(insBndTest(tri, Point_2(11, 11.6), BND_ON_UNBOUNDED_SIDE, INS_OUTSIDE));
    }

    /*
     * Degenerated to a line.
     */
    {
        Triangle_2 tri(Point_2(8, 8), Point_2(9, 9), Point_2(10, 10));
        EXPECT_TRUE(insBndTest(tri, Point_2(8, 8), BND_ON_BOUNDARY, INS_BOUNDARY));
        EXPECT_TRUE(insBndTest(tri, Point_2(8.5, 8.5), BND_ON_BOUNDARY, INS_BOUNDARY));

        EXPECT_TRUE(insBndTest(tri, Point_2(7, 7), BND_ON_UNBOUNDED_SIDE, INS_OUTSIDE));
        EXPECT_TRUE(insBndTest(tri, Point_2(11, 11), BND_ON_UNBOUNDED_SIDE, INS_OUTSIDE));
    }

    /*
     * Degenerated to a line, different ordering, same results.
     */
    {
        Triangle_2 tri(Point_2(8, 8), Point_2(10, 10), Point_2(9, 9));
        EXPECT_TRUE(insBndTest(tri, Point_2(8, 8), BND_ON_BOUNDARY, INS_BOUNDARY));
        EXPECT_TRUE(insBndTest(tri, Point_2(8.5, 8.5), BND_ON_BOUNDARY, INS_BOUNDARY));

        EXPECT_TRUE(insBndTest(tri, Point_2(7, 7), BND_ON_UNBOUNDED_SIDE, INS_OUTSIDE));
        EXPECT_TRUE(insBndTest(tri, Point_2(11, 11), BND_ON_UNBOUNDED_SIDE, INS_OUTSIDE));
    }

    {
        Triangle_2 tri(Point_2(1, 1), Point_2(2, 1), Point_2(1, 2));
        EXPECT_TRUE(insBndTest(tri, Point_2(1, 1), BND_ON_BOUNDARY, INS_BOUNDARY));
        EXPECT_TRUE(insBndTest(tri, Point_2(1.5, 1), BND_ON_BOUNDARY, INS_BOUNDARY));

        EXPECT_TRUE(insBndTest(tri, Point_2(1.2, 1.2), BND_ON_BOUNDED_SIDE, INS_INSIDE));
        EXPECT_TRUE(insBndTest(tri, Point_2(11, 11), BND_ON_UNBOUNDED_SIDE, INS_OUTSIDE));
    }

    /*
     * Inverted triangle.
     */
    {
        Triangle_2 tri(Point_2(1, 1), Point_2(1, 2), Point_2(2, 1));
        EXPECT_TRUE(insBndTest(tri, Point_2(1, 1), BND_ON_BOUNDARY, INS_BOUNDARY));
        EXPECT_TRUE(insBndTest(tri, Point_2(1.5, 1), BND_ON_BOUNDARY, INS_BOUNDARY));

        EXPECT_TRUE(insBndTest(tri, Point_2(1.2, 1.2), BND_ON_BOUNDED_SIDE, INS_OUTSIDE));
        EXPECT_TRUE(insBndTest(tri, Point_2(11, 11), BND_ON_UNBOUNDED_SIDE, INS_INSIDE));
    }

    /*
     * Degenerated to a point.
     */
    {
        Triangle_2 tri(Point_2(3, 3), Point_2(3, 3), Point_2(3, 3));
        EXPECT_TRUE(insBndTest(tri, Point_2(3, 3), BND_ON_BOUNDARY, INS_BOUNDARY));
        EXPECT_TRUE(insBndTest(tri, Point_2(3, 3.1), BND_ON_UNBOUNDED_SIDE, INS_OUTSIDE));
        EXPECT_TRUE(insBndTest(tri, Point_2(3, 2.9), BND_ON_UNBOUNDED_SIDE, INS_OUTSIDE));
        EXPECT_TRUE(insBndTest(tri, Point_2(3.1, 3), BND_ON_UNBOUNDED_SIDE, INS_OUTSIDE));
        EXPECT_TRUE(insBndTest(tri, Point_2(2.9, 3), BND_ON_UNBOUNDED_SIDE, INS_OUTSIDE));
    }

    {
        Triangle_2 tri(Point_2(2, 2), Point_2(5, 2), Point_2(3.5, 4));
        // All six outside sectors.
        EXPECT_TRUE(insBndTest(tri, Point_2(1, 1.5), BND_ON_UNBOUNDED_SIDE, INS_OUTSIDE));
        EXPECT_TRUE(insBndTest(tri, Point_2(3.5, 1), BND_ON_UNBOUNDED_SIDE, INS_OUTSIDE));
        EXPECT_TRUE(insBndTest(tri, Point_2(6, 1.5), BND_ON_UNBOUNDED_SIDE, INS_OUTSIDE));
        EXPECT_TRUE(insBndTest(tri, Point_2(6, 3), BND_ON_UNBOUNDED_SIDE, INS_OUTSIDE));
        EXPECT_TRUE(insBndTest(tri, Point_2(3.5, 4.5), BND_ON_UNBOUNDED_SIDE, INS_OUTSIDE));
        EXPECT_TRUE(insBndTest(tri, Point_2(1, 3), BND_ON_UNBOUNDED_SIDE, INS_OUTSIDE));
        // Corners.
        EXPECT_TRUE(insBndTest(tri, Point_2(2, 2), BND_ON_BOUNDARY, INS_BOUNDARY));
        EXPECT_TRUE(insBndTest(tri, Point_2(5, 2), BND_ON_BOUNDARY, INS_BOUNDARY));
        EXPECT_TRUE(insBndTest(tri, Point_2(3.5, 4), BND_ON_BOUNDARY, INS_BOUNDARY));
        // Edges.
        EXPECT_TRUE(insBndTest(tri, Point_2(3, 2), BND_ON_BOUNDARY, INS_BOUNDARY));
        EXPECT_TRUE(insBndTest(tri, Point_2(4.25, 3), BND_ON_BOUNDARY, INS_BOUNDARY));
        EXPECT_TRUE(insBndTest(tri, Point_2(2.75, 3), BND_ON_BOUNDARY, INS_BOUNDARY));
        // Inside.
        EXPECT_TRUE(insBndTest(tri, Point_2(3.5, 3), BND_ON_BOUNDED_SIDE, INS_INSIDE));
    }

    {
        Triangle_2 tri(Point_2(2, 2), Point_2(3.5, 4), Point_2(5, 2));
        // All six outside sectors.
        EXPECT_TRUE(insBndTest(tri, Point_2(1, 1.5), BND_ON_UNBOUNDED_SIDE, INS_INSIDE));
        EXPECT_TRUE(insBndTest(tri, Point_2(3.5, 1), BND_ON_UNBOUNDED_SIDE, INS_INSIDE));
        EXPECT_TRUE(insBndTest(tri, Point_2(6, 1.5), BND_ON_UNBOUNDED_SIDE, INS_INSIDE));
        EXPECT_TRUE(insBndTest(tri, Point_2(6, 3), BND_ON_UNBOUNDED_SIDE, INS_INSIDE));
        EXPECT_TRUE(insBndTest(tri, Point_2(3.5, 4.5), BND_ON_UNBOUNDED_SIDE, INS_INSIDE));
        EXPECT_TRUE(insBndTest(tri, Point_2(1, 3), BND_ON_UNBOUNDED_SIDE, INS_INSIDE));
        // Corners.
        EXPECT_TRUE(insBndTest(tri, Point_2(2, 2), BND_ON_BOUNDARY, INS_BOUNDARY));
        EXPECT_TRUE(insBndTest(tri, Point_2(5, 2), BND_ON_BOUNDARY, INS_BOUNDARY));
        EXPECT_TRUE(insBndTest(tri, Point_2(3.5, 4), BND_ON_BOUNDARY, INS_BOUNDARY));
        // Edges.
        EXPECT_TRUE(insBndTest(tri, Point_2(3, 2), BND_ON_BOUNDARY, INS_BOUNDARY));
        EXPECT_TRUE(insBndTest(tri, Point_2(4.25, 3), BND_ON_BOUNDARY, INS_BOUNDARY));
        EXPECT_TRUE(insBndTest(tri, Point_2(2.75, 3), BND_ON_BOUNDARY, INS_BOUNDARY));
        // Inside.
        EXPECT_TRUE(insBndTest(tri, Point_2(3.5, 3), BND_ON_BOUNDED_SIDE, INS_OUTSIDE));
    }
}

/*
 * We don't test the has_on_* functions thoroughly
 * because they are just minimal wrappers around boundedness()
 * which we tested extensively.
 */

TEST_F(Test_Triangle_2, has_on_bounded_side) {
    EXPECT_FALSE(
            Triangle_2(Point_2(1, 1), Point_2(2, 1), Point_2(2, 2))
            .has_on_bounded_side(Point_2(1.2, 1.2)));

    EXPECT_FALSE(
            Triangle_2(Point_2(1, 1), Point_2(2, 1), Point_2(2, 2))
            .has_on_bounded_side(Point_2(1.2, 1)));

    EXPECT_TRUE(
            Triangle_2(Point_2(1, 1), Point_2(2, 1), Point_2(2, 2))
            .has_on_bounded_side(Point_2(1.5, 1.2)));

    EXPECT_FALSE(
            Triangle_2(Point_2(1, 1), Point_2(2, 1), Point_2(2, 2))
            .has_on_bounded_side(Point_2(1.2, .9)));
}

TEST_F(Test_Triangle_2, has_on_boundary) {
    EXPECT_TRUE(
            Triangle_2(Point_2(1, 1), Point_2(2, 1), Point_2(2, 2))
            .has_on_boundary(Point_2(1.2, 1)));


    EXPECT_FALSE(
            Triangle_2(Point_2(1, 1), Point_2(2, 1), Point_2(1, 2))
            .has_on_boundary(Point_2(1.2, 1.2)));

    EXPECT_TRUE(
            Triangle_2(Point_2(1, 1), Point_2(2, 1), Point_2(1, 2))
            .has_on_boundary(Point_2(1.2, 1)));

    EXPECT_TRUE(
            Triangle_2(Point_2(1, 1), Point_2(2, 1), Point_2(1, 2))
            .has_on_boundary(Point_2(2, 1)));

    EXPECT_FALSE(
            Triangle_2(Point_2(1, 1), Point_2(2, 1), Point_2(1, 2))
            .has_on_boundary(Point_2(1.2, .9)));
}

TEST_F(Test_Triangle_2, is_degenerate) {
    EXPECT_FALSE(
            Triangle_2(Point_2(1, 1), Point_2(2, 1), Point_2(1, 2))
            .is_degenerate());

    EXPECT_TRUE(
            Triangle_2(Point_2(1, 1), Point_2(2, 1), Point_2(3, 1))
            .is_degenerate());

    EXPECT_TRUE(
            Triangle_2(Point_2(1, 1), Point_2(2, 1), Point_2(2, 1))
            .is_degenerate());

    EXPECT_TRUE(
            Triangle_2(Point_2(1, 1), Point_2(1, 1), Point_2(1, 1))
            .is_degenerate());
}


TEST_F(Test_Triangle_2, bbox) {
    EXPECT_EQ(
            Triangle_2(Point_2(1, 1), Point_2(2, 1), Point_2(1, 2)).bbox(),
            Bbox_2(Vec2d(1, 1), Vec2d(2, 2)));

    EXPECT_EQ(
            Triangle_2(Point_2(1, 1), Point_2(2, 1), Point_2(3, 1)).bbox(),
            Bbox_2(Vec2d(1, 1), Vec2d(3, 1)));

    EXPECT_EQ(
            Triangle_2(Point_2(1, 1), Point_2(2, 1), Point_2(2, 1)).bbox(),
            Bbox_2(Vec2d(1, 1), Vec2d(2, 1)));

    EXPECT_EQ(
            Triangle_2(Point_2(1, 1), Point_2(1, 1), Point_2(1, 1)).bbox(),
            Bbox_2(Vec2d(1, 1), Vec2d(1, 1)));
}

TEST_F(Test_Triangle_2, brackets_operator) {
    Triangle_2 tri(Point_2(8, 8), Point_2(9, 9), Point_2(10, 10));
    EXPECT_EQ(Point_2(8, 8), tri[0]);
    EXPECT_EQ(Point_2(9, 9), tri[1]);
    EXPECT_EQ(Point_2(10, 10), tri[2]);
}

} /* anonymous namespace */
