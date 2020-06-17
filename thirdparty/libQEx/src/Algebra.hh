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
 * @brief Defines a bunch of algebraic classes and operators needed by QEx.
 *
 * Note that the algebraic classes and operators defined in this file
 * are specifically targeted at QEx. They are NOT intended as a general
 * purpose algebra library. Only those classes and operations needed
 * for QEx are implemented and if you use these classes in different
 * settings you might find that a lot of common operators are missing.
 */

#ifndef QEX_AGEBRA_HH
#define QEX_AGEBRA_HH

#include "Vector.hh"
#include "ExactPredicates.h"

#ifdef _WIN32
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
#else
#include <stdint.h>
#endif

/// QEx namespace.
namespace QEx {

/*
 * Forward declarations
 */
template<unsigned int R, unsigned int C> class Matrix;

/**
 * @brief A 2D vector class.
 *
 * It is derived from ::Vec2d but the base class is
 * private so that no accidental mixup of data types
 * can occur and legacy code cannot accidentally use
 * unintended operations.
 */
class Vector_2 : private Vec2d {
    public:
        Vector_2() {}

        Vector_2(double x0, double x1) : Vec2d(x0, x1) {
        }

        explicit Vector_2(const Vec2d &v) : Vec2d(v) {}

        /// Component wise addition.
        Vector_2 operator+(const Vector_2 &rhs) const {
            return Vector_2(static_cast<const Vec2d*>(this)->operator+(rhs));
        }

        /// Component wise subtraction.
        Vector_2 operator-(const Vector_2 &rhs) const {
            return Vector_2(static_cast<const Vec2d*>(this)->operator-(rhs));
        }

        /// Component wise negation.
        Vector_2 operator-() const {
            return Vector_2(static_cast<const Vec2d*>(this)->operator-());
        }

        /// Scale by the inverse of a scalar.
        Vector_2 &operator/=(double rhs) {
            static_cast<Vec2d*>(this)->operator/=(rhs);
            return *this;
        }

        /// Scale by a scalar.
        Vector_2 &operator*=(double rhs) {
            static_cast<Vec2d*>(this)->operator*=(rhs);
            return *this;
        }

        double norm() const {
            return static_cast<const Vec2d*>(this)->norm();
        }

        /// Conversion to 1x2 matrix.
        Matrix<1, 2> transpose() const;

        /// Dot product.
        double dot(const Vector_2 &rhs) const {
            return (*this)[0] * rhs[0] + (*this)[1] * rhs[1];
        }

        /// Read-only access to the individual components.
        const double &operator[] (size_t i) const {
            return static_cast<const Vec2d*>(this)->operator[](i);
        }

#if 0
        /// Non-const version of data() const.
        double *data() {
            return static_cast<Vec2d*>(this)->data();
        }
#endif

        /// Access the raw data the vector.
        const double *data() const {
            return static_cast<const Vec2d*>(this)->data();
        }

        /// Non-const version of toVec2d() const.
        Vec2d &toVec2d() {
            return static_cast<Vec2d&>(*this);
        }

        /**
         * @brief Explicit conversion to ::Vec2d.
         *
         * We don't offer an implicit cast so that no
         * accidental mixup between the different
         * data types is possible.
         */
        const Vec2d &toVec2d() const {
            return static_cast<const Vec2d&>(*this);
        }

        /// Equality
        bool operator== (const Vector_2 &rhs) const {
            return static_cast<const Vec2d&>(*this) == static_cast<const Vec2d&>(rhs);
        }

        /// Inequality
        bool operator!= (const Vector_2 &rhs) const {
            return static_cast<const Vec2d&>(*this) != static_cast<const Vec2d&>(rhs);
        }

        /// C++ iostreams support.
        friend
        std::ostream &operator<< (std::ostream &s, const Vector_2 &rhs) {
            return s << "Vector_2(" << rhs[0] << ", " << rhs[1] << ")";
        }
};

/// A 2D point class.
class Point_2 : private Vec2d {
    public:
        Point_2() {}

        Point_2(double x0, double x1) : Vec2d(x0, x1) {
        }

        explicit Point_2(const Vec2d &v) : Vec2d(v) {}

        /// Subtraction of two points yields vector.
        Vector_2 operator-(const Point_2 &rhs) const {
            return Vector_2(static_cast<const Vec2d*>(this)->operator-(rhs));
        }

        /// Point plus vector yields point.
        Point_2 operator+(const Vector_2 &rhs) const {
            return Point_2((*this)[0] + rhs[0], (*this)[1] + rhs[1]);
        }

        /// Point minus vector yields point.
        Point_2 operator-(const Vector_2 &rhs) const {
            return Point_2((*this)[0] - rhs[0], (*this)[1] - rhs[1]);
        }

        /// Read-only access to individual components.
        const double &operator[] (size_t i) const {
            return static_cast<const Vec2d*>(this)->operator[](i);
        }

#if 0
        /// Non-const version of data() const.
        double *data() {
            return static_cast<Vec2d*>(this)->data();
        }
#endif

        /// Access the raw data the vector.
        const double *data() const {
            return static_cast<const Vec2d*>(this)->data();
        }

        /// Non-const version of toVec2d() const.
        Vec2d &toVec2d() {
            return static_cast<Vec2d&>(*this);
        }

        /**
         * @brief Explicit conversion to ::Vec2d.
         *
         * We don't offer an implicit cast so that no
         * accidental mixup between the different
         * data types is possible.
         */
        const Vec2d &toVec2d() const {
            return static_cast<const Vec2d&>(*this);
        }

        /// Equality
        bool operator== (const Point_2 &rhs) const {
            return static_cast<const Vec2d&>(*this) == static_cast<const Vec2d&>(rhs);
        }

        /// Inequality
        bool operator!= (const Point_2 &rhs) const {
            return static_cast<const Vec2d&>(*this) != static_cast<const Vec2d&>(rhs);
        }

        /// C++ iostreams support.
        friend
        std::ostream &operator<< (std::ostream &s, const Point_2 &rhs) {
            return s << "Point_2(" << rhs[0] << ", " << rhs[1] << ")";
        }
};

/// orient2d convenience wrapper for Point_2.
inline ORIENTATION orient2d(const Point_2 &a, const Point_2 &b, const Point_2 &c) {
    return sign_orient2d(a.data(), b.data(), c.data());
}

/// orient2d convenience wrapper for Vec2d.
inline ORIENTATION orient2d(const Vec2d &a, const Vec2d &b) {
    static const double o[] = { 0.0, 0.0 };
    return sign_orient2d(o, a.data(), b.data());
}

/// orient2d convenience wrapper for Vector_2.
inline ORIENTATION orient2d(const Vector_2 &a, const Vector_2 &b) {
    static const double o[] = { 0.0, 0.0 };
    return sign_orient2d(o, a.data(), b.data());
}

/// Syntactically more concise convenience wrapper around orient2d.
inline bool is_collinear(const Point_2 &a, const Point_2 &b, const Point_2 &c) {
    return orient2d(a, b, c) == ORI_COLLINEAR;
}

/** @brief Specifies the location of a point relative to a triangle dependant on its orientation
 *
 * Note that for a triangle with positive orientation, ::INS_INSIDE is
 * equivalent to the bounded side whereas ::INS_OUTSIDE is equivalent to
 * the unbounded side. <em>For a triangle with negative orientation,
 * it is the other way round</em>, i.e. the ::INS_INSIDE is equivalent
 * to the \em unbounded side.
 *
 * @see BOUNDEDNESS
 */
enum INSIDENESS {
    INS_OUTSIDE = -1,//!< Point is outside the triangle, i.e. neither inside nor on the boundary.
    INS_BOUNDARY = 0,//!< Point is on the triangle of the polygon, i.e. neither inside nor outside.
    INS_INSIDE = 1   //!< Point is inside the triangle, i.e. neither outside nor on the boundary.
};

/** @brief Specifies the location of a point relative to a triangle invariant in its orientation.
 *
 * @see INSIDENESS
 */
enum BOUNDEDNESS {
    BND_ON_UNBOUNDED_SIDE = -1, //!< Point is on the unbounded side of the triangle.
    BND_ON_BOUNDARY       = 0,  //!< Point is on the boundary of the triangle.
    BND_ON_BOUNDED_SIDE   = 1,  //!< Point is on the bounded side of the triangle.
    BND_INVALID = 2             //!< Marker value to identify certain implementation errors.
};

/// A 2D bounding box class.
class Bbox_2 {
    public:
        Bbox_2(const Vec2d &vmin, const Vec2d &vmax) : vmin(vmin), vmax(vmax) {}

        double xmin() const { return vmin[0]; }
        double xmax() const { return vmax[0]; }
        double ymin() const { return vmin[1]; }
        double ymax() const { return vmax[1]; }

        bool operator==(const Bbox_2 &rhs) const {
            return vmin == rhs.vmin && vmax == rhs.vmax;
        }

        const Vec2d vmin, vmax;
};

/**
 * @brief Given the supplied points are on a line, is the third
 * one on the line segment defined by the other two?
 *
 * Assuming a, b and c are collinear, this function returns
 * true if c falls onto [a, b] (including c == a and c == b),
 * false otherwise.
 */
static inline bool has_on_assume_collinear(const Point_2 &a, const Point_2 &b, const Point_2 &c) {
    return (std::min(a[0], b[0]) <= c[0] && std::max(a[0], b[0]) >= c[0]) &&
            (std::min(a[1], b[1]) <= c[1] && std::max(a[1], b[1]) >= c[1]);
}

/// A 2D triangle class.
class Triangle_2 {
    public:
        Triangle_2(const Point_2 &a, const Point_2 &b, const Point_2 &c) : a(a), b(b), c(c) {}

        /** @brief Computes the location of the supplied point relative to this triangle <em>dependant on its orientation</em>.
         *
         * <em>This member is more inefficient than boundedness().
         * Prefer the use of boundedness() over insideness() wherever possible.</em>
         *
         * Note that it depends on the orientation of the triangle's vertices
         * whether the "inside" of the triangle is defined to be the bounded or
         * the unbounded side. If the points are ordered in counter-clockwise order
         * the inside is equivalent to the bounded side of the triangle, if the
         * points are ordered in clockwise order, the inside is equivalent to the unbounded
         * side of the triangle.
         *
         * If the triangle is degenerated to a line or a point then there
         * is no bounded side. In this case the only possible return values
         * are ::INS_OUTSIDE and ::INS_BOUNDARY.
         *
         * See the documentation of the ::INSIDENESS enum for the exact meaning
         * of the possible return values.
         *
         * @return The location of the supplied point relative to this triangle.
         * @see boundedness
         */
        INSIDENESS insideness(const Point_2 &pt) const {
            BOUNDEDNESS bnd = boundedness(pt);
            ORIENTATION ori = orient2d(a, b, c);

            // If the orientation is inverse (== -1) the bounded side (== 1) is the outside (== -1).
            return static_cast<INSIDENESS>(static_cast<char>(bnd) * (ori == ORI_CW ? -1 : 1));
        }

        /** @brief Computes the location of the supplied point relative to this triangle <em>invariant in its orientation</em>.
         *
         * @return The location of the supplied point relative to this triangle.
         *
         * @see insideness
         */
        BOUNDEDNESS boundedness(const Point_2 &pt) const {
            /*
             * We determine the orientations of the three triangles
             * (a, b, pt), (b, c, pt) and (c, a, pt). We count the
             * number of times ORI_CW and ORI_COLLINEAR occurs. Both
             * of these numbers are <= 3. We stuff both two bit numbers
             * into one four bit integer idx. We then use idx to
             * address the idx-th pair of bits in lookup_table. This
             * two bit number specifies the boundedness of the
             * present configuration.
             *
             * Invalid indices are set to binary 11 which translates
             * to ::BND_INVALID.
             */
            static const uint32_t lookup_table = 0xfff5d182;

            const int ori_a = static_cast<int>(orient2d(b, c, pt));
            const int ori_b = static_cast<int>(orient2d(c, a, pt));
            const int ori_c = static_cast<int>(orient2d(a, b, pt));
            const unsigned char idx = ((1 << ((ori_a + 1) << 1)) + (1 << ((ori_b + 1) << 1)) + (1 << ((ori_c + 1) << 1))) & 0xF;

            /*
             * If idx == 12 (meaning ori_a = ori_b = ori_c = 0) we have
             * a degenerate triangle and have to use has_on to decide
             * whether pt is on the boundary or outside.
             */
            if (idx == 12) {
                if (has_on_assume_collinear(a, b, pt) || has_on_assume_collinear(b, c, pt))
                    return BND_ON_BOUNDARY;
                else
                    return BND_ON_UNBOUNDED_SIDE;
            }

            assert(idx <= 9);

            return static_cast<BOUNDEDNESS>(((lookup_table >> (idx << 1)) & 0x3) - 1);
        }

        /** @brief Compute the orientation of this triangle.
         *
         * @return the orientation (i.e. CW, CCW, degenerate) of the triangle.
         */
        ORIENTATION orientation() const {
            return orient2d(a, b, c);
        }

        /// Syntactically more concise wrapper around boundedness().
        bool has_on_bounded_side(const Point_2 &pt) const {
            return boundedness(pt) == BND_ON_BOUNDED_SIDE;
        }

        /// Syntactically more concise wrapper around boundedness().
        bool has_on_boundary(const Point_2 &pt) const {
            return boundedness(pt) == BND_ON_BOUNDARY;
        }

        /// Syntactically more concise wrapper around boundedness().
        bool has_on_unbounded_side(const Point_2 &pt) const {
            return boundedness(pt) == BND_ON_UNBOUNDED_SIDE;
        }

        /// Syntactically more concise wrapper around orientation().
        bool is_degenerate() const {
            return orientation() == ORI_ZERO;
        }

        /// Compute the bounding box of this triangle.
        Bbox_2 bbox() const {
            Vec2d vmin(a.toVec2d()), vmax(a.toVec2d());
            vmin.minimize(b.toVec2d()); vmax.maximize(b.toVec2d());
            vmin.minimize(c.toVec2d()); vmax.maximize(c.toVec2d());
            return Bbox_2(vmin, vmax);
        }

        /// Access to the corner points of this triangle.
        const Point_2 &operator[] (const int i) const {
            switch (i%3) {
                case 0:
                    return a;
                case 1:
                    return b;
                case 2:
                default:
                    return c;
            }
        }

        Point_2 a, b, c;

        friend std::ostream &operator<<(std::ostream &s, const Triangle_2 &t) {
            return s << "Triangle_2(" << t.a << ", " << t.b << ", " << t.c << ")";
        }
};

/** @brief A row-major matrix class.
 *
 * Reminder: Row-major means the rows are stored one after another.
 * Indices/memory ordering of a 2x2 matrix would be:
 * @f[
 * \left(\begin{array}{ll}0&1\\2&3\end{array}\right)
 * @f]
 */
template<unsigned int R, unsigned int C>
class Matrix {
    public:
        /** @brief Helper class to enable bulk assignment of values to a matrix.
         *
         * This class enables bulk assignment of values using Matrix::operator<<.
         */
        class Inserter {
            public:
                Inserter(Matrix &m, int i) : m(m), i(i) {}

                Inserter &operator, (double value) {
                    assert(i < R * C);
                    m.v[i] = value;
                    ++i;
                    return *this;
                }


                Matrix &m;
                unsigned int i;
        };

        /** @brief Helper class to enable access to a column of a matrix.
         *
         * Note: This class is coined for matrices with 3 rows. It's
         * very specific to QEx' needs.
         */
        class Column {
            public:
                Column(Matrix &m, int c) : m(m), c(c) {};

                Column &operator= (const Vec3d &v) {
                    for (unsigned int i = 0; i < R; ++i)
                        m(i, c) = v[i];
                    return *this;
                }

                Matrix &m;
                unsigned int c;
        };

        /** @brief Helper class to address a block inside a Matrix conveniently.
         */
        template<unsigned int BR, unsigned int BC>
        class Block {
            public:
                Block(Matrix &m, unsigned int ofs_r, unsigned int ofs_c) :
                    m(m), ofs_r(ofs_r), ofs_c(ofs_c) {}

                Block &operator=(const Matrix<BR, BC> &rhs) {
                    for (unsigned int r = 0; r < BR; ++r) {
                        for (unsigned int c = 0; c < BC; ++c) {
                            m(ofs_r + r, ofs_c + c) = rhs(r, c);
                        }
                    }

                    return *this;
                }

                Matrix &m;
                unsigned int ofs_r, ofs_c;
        };

    public:
        /** @brief Element access through zero-based row and column index.
         *
         * @see operator() (unsigned int r, unsigned int c) const
         */
        double &operator() (unsigned int r, unsigned int c) {
            assert(r * C + c < R * C);
            return v[r * C + c];
        }

        /** @brief Element access through zero-based row and column index.
         *
         * @param r Zero-based row index.
         * @param c Zero-based column index.
         * @return The addressed element.
         */
        const double &operator() (unsigned int r, unsigned int c) const {
            return const_cast<Matrix<R, C>&>(*this)(r, c);
        }

        /** @brief Convenient bulk assignment of values to the matrix.
         *
         * This operator together with the Inserter helper class
         * enables syntax like so:
         *
         *     Matrix<2,2> m;
         *     m << 1, 2,
         *          3, 4;
         */
        Inserter operator<< (double value) {
            v[0] = value;
            return Inserter(*this, 1);
        }

        /** @brief Returns the (pseudo-)inverse of this matrix.
         *
         * This member is undefined for generic matrices.
         * Only a specialization for 3x3 matrices is implemented
         * as nothing else is needed for QEx.
         */
        Matrix<C, R> inverse() const;

        template<unsigned int RHS_C>
        Matrix<R, RHS_C> operator* (const Matrix<C, RHS_C> &rhs) const {
            Matrix<R, RHS_C> result;
            for (unsigned int r = 0; r < R; ++r) {
                for (unsigned int c = 0; c < RHS_C; ++c) {
                    double v = 0;
                    for (unsigned int i = 0; i < C; ++i) {
                        v += (*this)(r, i) * rhs(i, c);
                    }
                    result(r, c) = v;
                }
            }
            return result;
        }

        Column col(unsigned int i) {
            return Column(*this, i);
        }

        template<unsigned int BR, unsigned int BC>
        Block<BR, BC> block(unsigned int ofs_r, unsigned int ofs_c) {
            return Block<BR, BC>(*this, ofs_r, ofs_c);
        }

        Matrix &operator /= (double rhs) {
            for (unsigned int i = 0; i < R * C; ++i) v[i] /= rhs;
            return *this;
        }

        Matrix &operator *= (double rhs) {
            for (unsigned int i = 0; i < R * C; ++i) v[i] *= rhs;
            return *this;
        }

        bool operator== (const Matrix &rhs) const {
            for (unsigned int i = 0; i < R * C; ++i) if (v[i] != rhs.v[i]) return false;
            return true;
        }

        double v[R * C];

        friend
        std::ostream &operator<< (std::ostream &s, const Matrix &rhs) {
            s << "Matrix<" << R << ", " << C << ">(" << std::endl;
            for (unsigned int r = 0; r < R; ++r) {
                for (unsigned int c = 0; c < C; ++c) {
                    if (c > 0) s << ", ";
                    s << rhs(r, c);
                }
                s << std::endl;
            }
            s << ")" << std::endl;
            return s;
        }
};

template<>
inline Matrix<3, 3> Matrix<3, 3>::inverse() const {
    Matrix<3, 3> result;
    result <<
              (*this)(2, 2)*(*this)(1, 1)-(*this)(2, 1)*(*this)(1, 2),  -((*this)(2, 2)*(*this)(0, 1)-(*this)(2, 1)*(*this)(0, 2)),   (*this)(1, 2)*(*this)(0, 1)-(*this)(1, 1)*(*this)(0, 2),
            -((*this)(2, 2)*(*this)(1, 0)-(*this)(2, 0)*(*this)(1, 2)),   (*this)(2, 2)*(*this)(0, 0)-(*this)(2, 0)*(*this)(0, 2),  -((*this)(1, 2)*(*this)(0, 0)-(*this)(1, 0)*(*this)(0, 2)),
              (*this)(2, 1)*(*this)(1, 0)-(*this)(2, 0)*(*this)(1, 1),  -((*this)(2, 1)*(*this)(0, 0)-(*this)(2, 0)*(*this)(0, 1)),   (*this)(1, 1)*(*this)(0, 0)-(*this)(1, 0)*(*this)(0, 1);
    result /= (*this)(0, 0)*((*this)(2, 2)*(*this)(1, 1)-(*this)(2, 1)*(*this)(1, 2))-(*this)(1, 0)*((*this)(2, 2)*(*this)(0, 1)-(*this)(2, 1)*(*this)(0, 2))+(*this)(2, 0)*((*this)(1, 2)*(*this)(0, 1)-(*this)(1, 1)*(*this)(0, 2));
    return result;
}

/// Short name for a 3x3 matrix.
typedef Matrix<3, 3> Matrix_3;

inline Matrix<1, 2> Vector_2::transpose() const {
    Matrix<1, 2> m;
    m << (*this)[0], (*this)[1];
    return m;
}

/** @brief Multiplication of a 3-dimensional vector with a 1x2 matrix.
 *
 * @param lhs A 3-dimensional vector.
 * @param rhs A 1x2 matrix.
 * @return The vector-matrix-product of lhs and rhs.
 */
inline Matrix<3, 2> operator*(const Vec3d &lhs, const Matrix<1, 2> &rhs) {
    Matrix<3, 2> m;
    m <<
            lhs[0] * rhs(0, 0), lhs[0] * rhs(0, 1),
            lhs[1] * rhs(0, 0), lhs[1] * rhs(0, 1),
            lhs[2] * rhs(0, 0), lhs[2] * rhs(0, 1);
    return m;
}

/// A 2D line segment class.
class Segment_2 {
    public:
        Segment_2(const Point_2 &a, const Point_2 &b) : a(a), b(b) { }

        /** @brief Checks whether the segment is degenerated to a point.
         *
         * @returns true if the segment is degenerated to a point.
         */
        bool is_degenerate() const {
            return a == b;
        }

        /** @brief Computes the bounding box of the segment.
         *
         * @returns the bounding box of the segment. The boundary
         * points lie on the boundary of the bounding box.
         */
        Bbox_2 bbox() const {
            Vec2d vmin(a.toVec2d()), vmax(a.toVec2d());
            vmin.minimize(b.toVec2d()); vmax.maximize(b.toVec2d());
            return Bbox_2(vmin, vmax);
        }

        /** @brief Checks whether the supplied point falls onto this segment.
         *
         * @returns true if the supplied point lies on this segment.
         * The boundary points this->{a,b} are seen as belonging to
         * the segment and thus true is returned if on or both of them
         * are identical to the supplied point.
         */
        bool has_on(const Point_2 &c) const {
            if (orient2d(a, b, c) != ORI_COLLINEAR) return false;

            return has_on_assume_collinear(c);
        }

        /** @brief Checks whether this segment and the supplied one intersect.
         *
         * @returns true if both segments have at least
         * one point in common. The boundary points this->{a, b}
         * and rhs.{a,b} belong to the respective segment and thus
         * cause a true return value if intersected.
         */
        bool intersects(const Segment_2 &rhs) const {
            const ORIENTATION ori1a = orient2d(a, b, rhs.a);
            if (ori1a == ORI_COLLINEAR && has_on_assume_collinear(rhs.a)) return true;
            const ORIENTATION ori1b = orient2d(a, b, rhs.b);
            if (ori1b == ORI_COLLINEAR && has_on_assume_collinear(rhs.b)) return true;

            if (ori1a == ori1b) return false;

            const ORIENTATION ori2a = orient2d(rhs.a, rhs.b, a);
            if (ori2a == ORI_COLLINEAR && rhs.has_on_assume_collinear(a)) return true;
            const ORIENTATION ori2b = orient2d(rhs.a, rhs.b, b);
            if (ori2b == ORI_COLLINEAR && rhs.has_on_assume_collinear(b)) return true;

            if (ori2a == ori2b) return false;

            return true;
        }

        /** @brief Checks whether this segment and the supplied triangle intersect.
         *
         * @returns true if this segment and the
         * triangle have at least one point in common. The boundary
         * points of the segment this->{a,b} as well as the boundary
         * polygon of the triangle (including its corners) do belong
         * to the respective entity and thus cause a true return value
         * if intersected.
         */
        bool intersects(const Triangle_2 &rhs) const {
            if (rhs.has_on_bounded_side(a) || rhs.has_on_bounded_side(b)) return true;

            return intersects(Segment_2(rhs.a, rhs.b)) || intersects(Segment_2(rhs.b, rhs.c)) || intersects(Segment_2(rhs.c, rhs.a));
        }

        /// Access the boundary points of the segment.
        const Point_2 &operator[] (int i) const {
            if (i % 2) return b; else return a;
        }

        /// A little convenience function intended for debugging output.
        const std::string toTikz() const {
            std::ostringstream result;

            result << "\\draw[black,->,>=latex] "
                    << "(" << a[0] << ", " << a[1] << ") -- "
                    << "(" << b[0] << ", " << b[1] << ");";

            return result.str();
        }

    private:
        /**
         * Assuming a, b and c are collinear, this function returns
         * true if c falls onto [a, b] (including c == a and c == b),
         * false otherwise.
         */
        bool has_on_assume_collinear(const Point_2 &c) const {
            return QEx::has_on_assume_collinear(a, b, c);
        }

    private:
        const Point_2 a, b;

        /// Operator to support C++ iostream output.
        template<typename STREAM>
        friend STREAM &operator<<(STREAM &s, const Segment_2 &t) {
            return s << "Segment_2(" << t.a << ", " << t.b << ")";
        }
};

//typedef Vec2d Vector_2;

} // namespace QEx

#endif // QEX_AGEBRA_HH
