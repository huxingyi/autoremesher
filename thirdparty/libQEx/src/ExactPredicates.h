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

/** @file
 * @brief Defines the the exact predicate ::orient2d and
 * enums, types and functions it needs.
 */

#ifndef QEX_EXACTPREDICATES_H_
#define QEX_EXACTPREDICATES_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the exact predicates.
 *
 * This function has to be called before calling ::orient2d.
 */
void exactinit();

/**
 * @brief Computes the orientation of the supplied points.
 *
 * This function only returns correct values if exactinit() was called before.
 *
 * @param pa, pb, pc Arrays containing the two coordinates of a point, each.
 * @return A value the sign of which reflects the orientation of the three points.
 */
double orient2d(const double *pa, const double *pb, const double *pc);

/**
 * @brief Specifies the orientation of a tuple of three 2D points.
 *
 * There is a bijection between the orientation of points (a, b, c)
 * and the sign of the determinant of the matrix [b-a c-a] which
 * is reflected by the aliases for ::ORI_CW, ::ORI_CCW and ::ORI_COLLINEAR.
 */
typedef enum {
    ORI_CW = -1,      //!< The points are ordered in clockwise order.
    ORI_CCW = 1,      //!< The points are ordered in counterclockwise order.
    ORI_COLLINEAR = 0,//!< The points are collinear.
    ORI_NEGATIVE = -1,//!< The determinant/area of the triangle is negative, equivalent to ::ORI_CW.
    ORI_ZERO = 0,     //!< The determinant/area of the triangle is zero, equivalent to ::ORI_COLLINEAR.
    ORI_POSITIVE = 1, //!< The determinant/area of the triangle is positive, equivalent to ::ORI_CCW.
    ORI_RIGHT = -1,   //!< Connecting the points one has to make a right, equivalent to ::ORI_CW.
    ORI_LEFT = 1      //!< Connecting the points one has to make a left, equivalent to ::ORI_CCW.
} ORIENTATION;

/// Maps an ::ORIENTATION to a string. Useful for debugging output.
inline const char *Orientation2Str(ORIENTATION value) {
    static const char *(strs[]) = {
        "ORI_NEGATIVE", "ORI_ZERO", "ORI_POSITIVE"
    };

    assert(value >= -1 && value <= 1);

    return strs[value + 1];
}

/// Wrapper around ::orient2d. Returns the result as an ::ORIENTATION.
static inline ORIENTATION sign_orient2d(const double *pa, const double *pb, const double *pc) {
    const double result = orient2d(pa, pb, pc);
    // A little convoluted but branchless.
    return (ORIENTATION) ((result > 0.0) - (result < 0.0));
}

#ifdef __cplusplus
} // extern "C"
#endif


#endif /* QEX_EXACTPREDICATES_H_ */
