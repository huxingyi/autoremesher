/*
 *  Copyright (c) 2012-2014, Bruno Levy
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 *  * Neither the name of the ALICE Project-Team nor the names of its
 *  contributors may be used to endorse or promote products derived from this
 *  software without specific prior written permission.
 * 
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *  If you modify this software, you should include a notice giving the
 *  name of the person performing the modification, the date of modification,
 *  and the reason for such modification.
 *
 *  Contact: Bruno Levy
 *
 *     Bruno.Levy@inria.fr
 *     http://www.loria.fr/~levy
 *
 *     ALICE Project
 *     LORIA, INRIA Lorraine, 
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX 
 *     FRANCE
 *
 */

#ifndef GEOGRAM_BASIC_VECG
#define GEOGRAM_BASIC_VECG

#include <geogram/basic/common.h>
#include <geogram/basic/numeric.h>
#include <geogram/basic/memory.h>
#include <geogram/basic/assert.h>

#include <iostream>
#include <cfloat>
#include <cmath>

/**
 * \file geogram/basic/vecg.h
 * \brief Generic implementation of geometric vectors
 */

namespace GEO {

    /**
     * \brief Generic maths vector
     * \details Vecng implements a maths vector of dimension \p DIM containing
     * coordinates of type \p T and provides operations for manipulating it.
     * Type \p T is expected to be a numeric type.
     * \tparam DIM dimension of the vector
     * \tparam T type of the vector coordinates.
     */
    template <index_t DIM, class T>
    class vecng {
    public:
        /** \brief The dimension of the vector */
        static const index_t dim = DIM;

        /** \brief This vector type */
        typedef vecng<DIM, T> vector_type;

        /** \brief The type of the vector coordinates */
        typedef T value_type;

        /**
         * \brief Default vector constructor
         * \details All coordinates are initialized to 0 (zero).
         */
        vecng() {
            for(index_t i = 0; i < DIM; i++) {
                data_[i] = T(0);
            }
        }

        // This one should never be called :
        // a template constructor cannot be a copy constructor

        /**
         * \brief Constructs a vector by copy
         * \details This copies coordinates of vector \p v to this vector.
         * The type \p T2 of the coordinates in \p v must be convertible to
         * the type \p T of this vector.
         * \param[in] v an vector of same dimension with coordinates of type
         * \p T2
         * \tparam T2 the type of coordinates in vector \p v
         */
        template <class T2>
        explicit vecng(const vecng<DIM, T2>& v) {
            for(index_t i = 0; i < DIM; i++) {
                data_[i] = T(v[i]);
            }
        }

        // to avoid compilation problems
        template <class T2, index_t DIM2>
        explicit vecng(
            const vecng<DIM2, T2>& v
        ) {
            geo_debug_assert(DIM2 == DIM);
            for(index_t i = 0; i < DIM; i++) {
                data_[i] = T(v[i]);
            }
        }

        /**
         * \brief Constructs a vector from an array
         * \details This copies coordinates the first \p DIM coordinates of
         * array \p v to this vector. The type \p T2 of the coordinates in
         * \p v must be convertible to the type \p T of this vector.
         * \param[in] v an array of values of type \p T2
         * \tparam T2 the type of coordinates in vector \p v
         */
        template <class T2>
        explicit vecng(const T2* v) {
            for(index_t i = 0; i < DIM; i++) {
                data_[i] = T(v[i]);
            }
        }

        /**
         * \brief Gets the vector dimension
         * \return the value of \p DIM
         */
        index_t dimension() const {
            return DIM;
        }

        /**
         * \brief Gets modifiable vector data
         * \return a pointer to the first element of the vector
         */
        T* data() {
            return data_;
        }

        /**
         * \brief Gets non-modifiable vector data
         * \return a const pointer to the first element of the vector
         */
        const T* data() const {
            return data_;
        }

        /**
         * \brief Gets a modifiable vector coordinate
         * \param[in] i index of the coordinate
         * \return a reference to coordinate at index \p i
         */
        inline T& operator[] (index_t i) {
            geo_debug_assert(i < DIM);
            return data()[i];
        }

        /**
         * \brief Gets a non-modifiable vector coordinate
         * \param[in] i index of the coordinate
         * \return a const reference to coordinate at index \p i
         */
        inline const T& operator[] (index_t i) const {
            geo_debug_assert(i < DIM);
            return data()[i];
        }

        /**
         * \brief Gets the squared length of the vector
         */
        inline T length2() const {
            T result = T(0);
            for(index_t i = 0; i < DIM; i++) {
                result += data_[i] * data_[i];
            }
            return result;
        }

        /**
         * \brief Gets the length of the vector
         */
        inline T length() const {
            return sqrt(length2());
        }

        /**
         * \brief Gets the squared distance to a vector
         * \param[in] v another vector
         * \return (\p v - \p this).length2()
         */
        inline T distance2(const vector_type& v) const {
            T result(0);
            for(index_t i = 0; i < DIM; i++) {
                result += geo_sqr(v.data_[i] - data_[i]);
            }
            return result;
        }

        /**
         * \brief Gets the distance to a vector
         * \param[in] v another vector
         * \return (\p v - \p this).length()
         */
        inline T distance(const vector_type& v) const {
            return sqrt(distance2(v));
        }

        // operators

        /**
         * \brief Adds a vector in place
         * \details Adds coordinates of vector \p v to this vector
         * coordinates
         * \param[in] v another vector
         * \return a reference to this vector
         */
        inline vector_type& operator+= (const vector_type& v) {
            for(index_t i = 0; i < DIM; i++) {
                data_[i] += v.data_[i];
            }
            return *this;
        }

        /**
         * \brief Subtracts a vector in place
         * \details Subtracts coordinates of vector \p v from this vector
         * coordinates
         * \param[in] v another vector
         * \return a reference to this vector
         */
        inline vector_type& operator-= (const vector_type& v) {
            for(index_t i = 0; i < DIM; i++) {
                data_[i] -= v.data_[i];
            }
            return *this;
        }

        /**
         * \brief Multiplies by a scalar in place
         * \details Multiplies this vector coordinates by value \p s. The type
         * \p T2 of \p s must be convertible to the type \p T of this vector
         * coordinates.
         * \param[in] s a value of type \p T2
         * \tparam T2 the type of value \p s
         * \return a reference to this vector
         */
        template <class T2>
        inline vector_type& operator*= (T2 s) {
            for(index_t i = 0; i < DIM; i++) {
                data_[i] *= T(s);
            }
            return *this;
        }

        /**
         * \brief Divides by a scalar in place
         * \details Divides this vector coordinates by value \p s. The type
         * \p T2 of \p s must be convertible to the type \p T of this vector
         * coordinates.
         * \param[in] s a value of type \p T2
         * \tparam T2 the type of value \p s
         * \return a reference to this vector
         */
        template <class T2>
        inline vector_type& operator/= (T2 s) {
            for(index_t i = 0; i < DIM; i++) {
                data_[i] /= T(s);
            }
            return *this;
        }

        /**
         * \brief Adds 2 vectors
         * \details Builds a vector by adding coordinates of this vector and
         * coordinates of vector \p v.
         * \param[in] v another vector
         * \return the result vector (\p this + \p v)
         */
        inline vector_type operator+ (const vector_type& v) const {
            vector_type result(*this);
            for(index_t i = 0; i < DIM; i++) {
                result.data_[i] += v.data_[i];
            }
            return result;
        }

        /**
         * \brief Subtracts 2 vectors
         * \details Builds a vector by subtracting coordinates of vector \p v
         * to coordinates of this vector.
         * \param[in] v another vector
         * \return the result vector (\p this - \p v)
         */
        inline vector_type operator- (const vector_type& v) const {
            vector_type result(*this);
            for(index_t i = 0; i < DIM; i++) {
                result.data_[i] -= v.data_[i];
            }
            return result;
        }

        /**
         * \brief Multiplies a vector by a scalar
         * \details Builds a vector by multipying this vector coordinates by
         * value \p s. The type \p T2 of \p s must be convertible to the type
         * \p T of this vector coordinates.
         * \param[in] s a value of type \p T2
         * \tparam T2 the type of value \p s
         * \return the result vector (\p this * \p s)
         */
        template <class T2>
        inline vector_type operator* (T2 s) const {
            vector_type result(*this);
            for(index_t i = 0; i < DIM; i++) {
                result.data_[i] *= T(s);
            }
            return result;
        }

        /**
         * \brief Divides a vector by a scalar
         * \details Builds a vector by dividing this vector coordinates by
         * value \p s. The type \p T2 of \p s must be convertible to the type
         * \p T of this vector coordinates.
         * \param[in] s a value of type \p T2
         * \tparam T2 the type of value \p s
         * \return the result vector (\p this / \p s)
         */
        template <class T2>
        inline vector_type operator/ (T2 s) const {
            vector_type result(*this);
            for(index_t i = 0; i < DIM; i++) {
                result.data_[i] /= T(s);
            }
            return result;
        }

        /**
         * \brief Negates a vector
         * \details Builds a vector by negating coordinates of this vector.
         * \return the result vector (-\p this)
         */
        inline vector_type operator- () const {
            vector_type result;
            for(index_t i = 0; i < DIM; i++) {
                result.data_[i] = -data_[i];
            }
            return result;
        }

    private:
        T data_[DIM];
    };

    /**
     * \brief Computes the dot product of 2 vectors
     * \param[in] v1 the first vector
     * \param[in] v2 the second vector
     * \return the dot product (\p v1 . \p v2)
     * \relates vecng
     */
    template <index_t DIM, class T>
    inline T dot(
        const vecng<DIM, T>& v1, const vecng<DIM, T>& v2
    ) {
        T result = 0;
        for(index_t i = 0; i < DIM; i++) {
            result += v1[i] * v2[i];
        }
        return result;
    }

    /**
     * \brief Multiplies a scalar by a vector
     * \details Builds a vector by multipying this vector coordinates by
     * value \p s. The type \p T2 of \p s must be convertible to the type \p
     * T of this vector coordinates.
     * \param[in] s a value of type \p T2
     * \param[in] v the vector to multiply
     * \tparam T2 the type of value \p s
     * \return the result vector (\p s * \p v)
     * \relates vecng
     */
    template <class T2, index_t DIM, class T>
    inline vecng<DIM, T> operator* (
        T2 s, const vecng<DIM, T>& v
    ) {
        vecng<DIM, T> result;
        for(index_t i = 0; i < DIM; i++) {
            result[i] = T(s) * v[i];
        }
        return result;
    }

    // Compatibility with GLSL

    /**
     * \brief Gets the norm of a vector
     * \param[in] v a vector
     * \return the norm of vector \p v
     * \see vecng::length()
     * \relates vecng
     */
    template <index_t DIM, class T>
    inline T length(const vecng<DIM, T>& v) {
        return v.length();
    }

    /**
     * \brief Gets the square norm of a vector
     * \param[in] v a vector
     * \return the square norm of vector \p v
     * \see vecng::length2()
     * \relates vecng
     */
    template <index_t DIM, class T>
    inline T length2(const vecng<DIM, T>& v) {
        return v.length2();
    }

    /**
     * \brief Gets the square distance between 2 vectors
     * \param[in] v1 the first vector
     * \param[in] v2 the second vector
     * \return the square distance between \p v1 and \p v2.
     * \see vecng::distance2()
     * \relates vecng
     */
    template <index_t DIM, class T>
    inline T distance2(
        const vecng<DIM, T>& v1, const vecng<DIM, T>& v2
    ) {
        return v2.distance2(v1);
    }

    /**
     * \brief Gets the distance between 2 vectors
     * \param[in] v1 the first vector
     * \param[in] v2 the second vector
     * \return the distance between \p v1 and \p v2.
     * \see vecng::distance()
     * \relates vecng
     */
    template <index_t DIM, class T>
    inline T distance(
        const vecng<DIM, T>& v1, const vecng<DIM, T>& v2
    ) {
        return v2.distance(v1);
    }

    /**
     * \brief Normalizes a vector
     * \details Returns a normalized vector constructed by dividing
     * coordinates of vector \p v by it norm. If the norm is 0, then the
     * result is undefined.
     * \param[in] v a vector
     * \return the normalized vector
     * \relates vecng
     */
    template <index_t DIM, class T>
    inline vecng<DIM, T> normalize(
        const vecng<DIM, T>& v
    ) {
        T s = length(v);
        if(s > 1e-30) {
            s = T(1) / s;
        }
        return s * v;
    }

    /**
     * \brief Computes a weighted barycenter
     * \details Computes the barycenter of \p v1 and \p v2 weighted by value
     * 1 - \p s and \p s.
     * \param[in] v1 the first vector
     * \param[in] v2 the second vector
     * \param[in] s the weight value
     * \return the barycenter (1 - \p s) * \p v1 + \p s * \p v2
     * \relates vecng
     */
    template <index_t DIM, class T>
    inline vecng<DIM, T> mix(
        const vecng<DIM, T>& v1, const vecng<DIM, T>& v2, T s
    ) {
        return (T(1) - s) * v1 + s * v2;
    }

    /************************************************************************/

    /**
     * \brief Specialization of class vecng for DIM == 2
     * \see vecng
     */
    template <class T>
    class vecng<2, T> {
    public:
        /** \copydoc vecng::dim */
        static const index_t dim = 2;

        /** \copydoc vecng::vector_type */
        typedef vecng<dim, T> vector_type;

        /** \copydoc vecng::value_type */
        typedef T value_type;

        /** \copydoc vecng::vecng() */
        vecng() :
            x(0),
            y(0) {
        }

        /**
         * \brief Constructs a vector from coordinates
         * \param[in] x_in , y_in vector coordinates
         */
        vecng(T x_in, T y_in) :
            x(x_in),
            y(y_in) {
        }

        /** \copydoc vecng::vecng(const vecng<DIM, T2>&) */
        template <class T2>
        explicit vecng(const vecng<dim, T2>& v) :
            x(v.x),
            y(v.y) {
        }

        /** \copydoc vecng::vecng(const T2*) */
        template <class T2>
        explicit vecng(const T2* v) :
            x(v[0]),
            y(v[1]) {
        }

        /** \copydoc vecng::length2() const */
        inline T length2() const {
            return x * x + y * y;
        }

        /** \copydoc vecng::length() const */
        inline T length() const {
            return sqrt(x * x + y * y);
        }

        /** \copydoc vecng::distance2(const vector_type&) const */
        inline T distance2(const vector_type& v) const {
            T dx = v.x - x;
            T dy = v.y - y;
            return dx * dx + dy * dy;
        }

        /** \copydoc vecng::distance(const vector_type&) const */
        inline T distance(const vector_type& v) const {
            return sqrt(distance2(v));
        }

        /** \copydoc vecng::operator+=(const vector_type&) */
        inline vector_type& operator+= (const vector_type& v) {
            x += v.x;
            y += v.y;
            return *this;
        }

        /** \copydoc vecng::operator-=(const vector_type&) */
        inline vector_type& operator-= (const vector_type& v) {
            x -= v.x;
            y -= v.y;
            return *this;
        }

        /** \copydoc vecng::operator*=(T2) */
        template <class T2>
        inline vector_type& operator*= (T2 s) {
            x *= T(s);
            y *= T(s);
            return *this;
        }

        /** \copydoc vecng::operator/=(T2) */
        template <class T2>
        inline vector_type& operator/= (T2 s) {
            x /= T(s);
            y /= T(s);
            return *this;
        }

        /** \copydoc vecng::operator+(const vector_type&) const */
        inline vector_type operator+ (const vector_type& v) const {
            return vector_type(x + v.x, y + v.y);
        }

        /** \copydoc vecng::operator-(const vector_type&) const */
        inline vector_type operator- (const vector_type& v) const {
            return vector_type(x - v.x, y - v.y);
        }

        /** \copydoc vecng::operator*(T2) const */
        template <class T2>
        inline vector_type operator* (T2 s) const {
            return vector_type(x * T(s), y * T(s));
        }

        /** \copydoc vecng::operator/(T2) const */
        template <class T2>
        inline vector_type operator/ (T2 s) const {
            return vector_type(x / T(s), y / T(s));
        }

        /** \copydoc vecng::operator-() const */
        inline vector_type operator- () const {
            return vector_type(-x, -y);
        }

        /** \copydoc vecng::dimension() const */
        index_t dimension() const {
            return dim;
        }

        /** \copydoc vecng::data() */
        T* data() {
            return &x;
        }

        /** \copydoc vecng::data() const */
        const T* data() const {
            return &x;
        }

        /** \copydoc vecng::operator[](index_t) */
        inline T& operator[] (index_t i) {
            geo_debug_assert(i < dim);
            return data()[i];
        }

        /** \copydoc vecng::operator[](index_t) const */
        inline const T& operator[] (index_t i) const {
            geo_debug_assert(i < dim);
            return data()[i];
        }

        /** \brief Vector x coordinate */
        T x;
        /** \brief Vector y coordinate */
        T y;
    };

    /**
     * \copydoc vecng::dot(const vecng<DIM,T>&,const vecng<DIM,T>&)
     * \relates vecng
     */
    template <class T>
    inline T dot(
        const vecng<2, T>& v1, const vecng<2, T>& v2
    ) {
        return v1.x * v2.x + v1.y * v2.y;
    }

    /**
     * \brief Computes the determinant of 2 vectors
     * \param[in] v1 the first vector
     * \param[in] v2 the second vector
     * \return the value of the determinant
     * \relates vecng
     */
    template <class T>
    inline T det(
        const vecng<2, T>& v1, const vecng<2, T>& v2
    ) {
        return v1.x * v2.y - v1.y * v2.x;
    }

    /**
     * \copydoc vecng::operator*(T2,const vecng<DIM,T>&)
     * \relates vecng
     */
    template <class T2, class T>
    inline vecng<2, T> operator* (
        T2 s, const vecng<2, T>& v
    ) {
        return vecng<2, T>(T(s) * v.x, T(s) * v.y);
    }

    /************************************************************************/

    /**
     * \brief Specialization of class vecng for DIM == 3
     * \see vecng
     */
    template <class T>
    class vecng<3, T> {
    public:
        /** \copydoc vecng::dim */
        static const index_t dim = 3;

        /** \copydoc vecng::vector_type */
        typedef vecng<dim, T> vector_type;

        /** \copydoc vecng::value_type */
        typedef T value_type;

        /** \copydoc vecng::vecng() */
        vecng() :
            x(0),
            y(0),
            z(0) {
        }

        /**
         * \brief Constructs a vector from coordinates
         * \param[in] x_in , y_in , z_in vector coordinates
         */
        vecng(T x_in, T y_in, T z_in) :
            x(x_in),
            y(y_in),
            z(z_in) {
        }

        /** \copydoc vecng::vecng(const vecng<DIM, T2>&) */
        template <class T2>
        explicit vecng(const vecng<dim, T2>& v) :
            x(v.x),
            y(v.y),
            z(v.z) {
        }

        /** \copydoc vecng::vecng(const T2*) */
        template <class T2>
        explicit vecng(const T2* v) :
            x(v[0]),
            y(v[1]),
            z(v[2]) {
        }

        /** \copydoc vecng::length2() const */
        inline T length2() const {
            return x * x + y * y + z * z;
        }

        /** \copydoc vecng::length() const */
        inline T length() const {
            return sqrt(x * x + y * y + z * z);
        }

        /** \copydoc vecng::distance2(const vector_type&) const */
        inline T distance2(const vector_type& v) const {
            T dx = v.x - x;
            T dy = v.y - y;
            T dz = v.z - z;
            return dx * dx + dy * dy + dz * dz;
        }

        /** \copydoc vecng::distance(const vector_type&) const */
        inline T distance(const vector_type& v) const {
            return sqrt(distance2(v));
        }

        /** \copydoc vecng::operator+=(const vector_type&) */
        inline vector_type& operator+= (const vector_type& v) {
            x += v.x;
            y += v.y;
            z += v.z;
            return *this;
        }

        /** \copydoc vecng::operator-=(const vector_type&) */
        inline vector_type& operator-= (const vector_type& v) {
            x -= v.x;
            y -= v.y;
            z -= v.z;
            return *this;
        }

        /** \copydoc vecng::operator*=(T2) */
        template <class T2>
        inline vector_type& operator*= (T2 s) {
            x *= T(s);
            y *= T(s);
            z *= T(s);
            return *this;
        }

        /** \copydoc vecng::operator/=(T2) */
        template <class T2>
        inline vector_type& operator/= (T2 s) {
            x /= T(s);
            y /= T(s);
            z /= T(s);
            return *this;
        }

        /** \copydoc vecng::operator+(const vector_type&) const */
        inline vector_type operator+ (const vector_type& v) const {
            return vector_type(x + v.x, y + v.y, z + v.z);
        }

        /** \copydoc vecng::operator-(const vector_type&) const */
        inline vector_type operator- (const vector_type& v) const {
            return vector_type(x - v.x, y - v.y, z - v.z);
        }

        /** \copydoc vecng::operator*(T2) const */
        template <class T2>
        inline vector_type operator* (T2 s) const {
            return vector_type(x * T(s), y * T(s), z * T(s));
        }

        /** \copydoc vecng::operator/(T2) const */
        template <class T2>
        inline vector_type operator/ (T2 s) const {
            return vector_type(x / T(s), y / T(s), z / T(s));
        }

        /** \copydoc vecng::operator-() const */
        inline vector_type operator- () const {
            return vector_type(-x, -y, -z);
        }

        /** \copydoc vecng::dimension() const */
        index_t dimension() const {
            return dim;
        }

        /** \copydoc vecng::data() */
        T* data() {
            return &x;
        }

        /** \copydoc vecng::data() const */
        const T* data() const {
            return &x;
        }

        /** \copydoc vecng::operator[](index_t) */
        inline T& operator[] (index_t i) {
            geo_debug_assert(i < dim);
            return data()[i];
        }

        /** \copydoc vecng::operator[](index_t) const */
        inline const T& operator[] (index_t i) const {
            geo_debug_assert(i < dim);
            return data()[i];
        }

        /** \brief Vector x coordinate */
        T x;
        /** \brief Vector y coordinate */
        T y;
        /** \brief Vector z coordinate */
        T z;
    };

    /**
     * \copydoc vecng::dot(const vecng<DIM,T>&, const vecng<DIM,T>&)\
     * \relates vecng
     */
    template <class T>
    inline T dot(
        const vecng<3, T>& v1, const vecng<3, T>& v2
    ) {
        return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
    }

    /**
     * \brief Computes the cross product of 2 vectors
     * \param[in] v1 the first vector
     * \param[in] v2 the second vector
     * \return the cross product (\p v1 x \p v2)
     * \relates vecng
     */
    template <class T>
    inline vecng<3, T> cross(
        const vecng<3, T>& v1, const vecng<3, T>& v2
    ) {
        return vecng<3, T>(
            v1.y * v2.z - v1.z * v2.y,
            v1.z * v2.x - v1.x * v2.z,
            v1.x * v2.y - v1.y * v2.x
        );
    }

    /**
     * \copydoc vecng::operator*(T2, const vecng<DIM,T>&)
     * \relates vecng
     */
    template <class T2, class T>
    inline vecng<3, T> operator* (
        T2 s, const vecng<3, T>& v
    ) {
        return vecng<3, T>(T(s) * v.x, T(s) * v.y, T(s) * v.z);
    }

    /************************************************************************/

    /**
     * \brief Specialization of class vecn3 for DIM == 4
     * \see vecng
     */
    template <class T>
    class vecng<4, T> {
    public:
        /** \copydoc vecng::dim */
        static const index_t dim = 4;

        /** \copydoc vecng::vector_type */
        typedef vecng<dim, T> vector_type;

        /** \copydoc vecng::value_type */
        typedef T value_type;

        /** \copydoc vecng::vecng() */
        vecng() :
            x(0),
            y(0),
            z(0),
            w(0) {
        }

        /**
         * \brief Constructs a vector from coordinates
         * \param[in] x_in , y_in , z_in , w_in vector coordinates
         */
        vecng(T x_in, T y_in, T z_in, T w_in) :
            x(x_in),
            y(y_in),
            z(z_in),
            w(w_in) {
        }

        /** \copydoc vecng::vecng(const vecng<DIM, T2>&) */
        template <class T2>
        explicit vecng(const vecng<dim, T2>& v) :
            x(v.x),
            y(v.y),
            z(v.z),
            w(v.w) {
        }

        /** \copydoc vecng::vecng(const T2*) */
        template <class T2>
        explicit vecng(const T2* v) :
            x(v[0]),
            y(v[1]),
            z(v[2]),
            w(v[3]) {
        }

        /** \copydoc vecng::length2() const */
        inline T length2() const {
            return x * x + y * y + z * z + w * w;
        }

        /** \copydoc vecng::length() const */
        inline T length() const {
            return sqrt(x * x + y * y + z * z + w * w);
        }

        /** \copydoc vecng::distance2(const vector_type&) const */
        inline T distance2(const vector_type& v) const {
            T dx = v.x - x;
            T dy = v.y - y;
            T dz = v.z - z;
            T dw = v.w - w;
            return dx * dx + dy * dy + dz * dz + dw * dw;
        }

        /** \copydoc vecng::distance(const vector_type&) const */
        inline T distance(const vector_type& v) const {
            return sqrt(distance2(v));
        }

        /** \copydoc vecng::dimension() const */
        index_t dimension() const {
            return dim;
        }

        /** \copydoc vecng::operator+=(const vector_type&) */
        inline vector_type& operator+= (const vector_type& v) {
            x += v.x;
            y += v.y;
            z += v.z;
            w += v.w;
            return *this;
        }

        /** \copydoc vecng::operator-=(const vector_type&) */
        inline vector_type& operator-= (const vector_type& v) {
            x -= v.x;
            y -= v.y;
            z -= v.z;
            w -= v.w;
            return *this;
        }

        /** \copydoc vecng::operator*=(T2) */
        template <class T2>
        inline vector_type& operator*= (T2 s) {
            x *= T(s);
            y *= T(s);
            z *= T(s);
            w *= T(s);
            return *this;
        }

        /** \copydoc vecng::operator/=(T2) */
        template <class T2>
        inline vector_type& operator/= (T2 s) {
            x /= T(s);
            y /= T(s);
            z /= T(s);
            w /= T(s);
            return *this;
        }

        /** \copydoc vecng::operator+(const vector_type&) const */
        inline vector_type operator+ (const vector_type& v) const {
            return vector_type(x + v.x, y + v.y, z + v.z, w + v.w);
        }

        /** \copydoc vecng::operator-(const vector_type&) const */
        inline vector_type operator- (const vector_type& v) const {
            return vector_type(x - v.x, y - v.y, z - v.z, w - v.w);
        }

        /** \copydoc vecng::operator*(T2) const */
        template <class T2>
        inline vector_type operator* (T2 s) const {
            return vector_type(x * T(s), y * T(s), z * T(s), w * T(s));
        }

        /** \copydoc vecng::operator/(T2) const */
        template <class T2>
        inline vector_type operator/ (T2 s) const {
            return vector_type(x / T(s), y / T(s), z / T(s), w / T(s));
        }

        /** \copydoc vecng::operator-() const */
        inline vector_type operator- () const {
            return vector_type(-x, -y, -z, -w);
        }

        /** \copydoc vecng::data() */
        T* data() {
            return &x;
        }

        /** \copydoc vecng::data() const */
        const T* data() const {
            return &x;
        }

        /** \copydoc vecng::operator[](index_t) */
        inline T& operator[] (index_t i) {
            geo_debug_assert(i < dim);
            return data()[i];
        }

        /** \copydoc vecng::operator[](index_t) const */
        inline const T& operator[] (index_t i) const {
            geo_debug_assert(i < dim);
            return data()[i];
        }

        /** \brief Vector x coordinate */
        T x;
        /** \brief Vector y coordinate */
        T y;
        /** \brief Vector z coordinate */
        T z;
        /** \brief Vector w coordinate */
        T w;
    };

    /**
     * \copydoc vecng::dot(const vecng<DIM,T>&, const vecng<DIM,T>&)
     * \relates vecng
     */
    template <class T>
    inline T dot(
        const vecng<4, T>& v1, const vecng<4, T>& v2
    ) {
        return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
    }

    /**
     * \copydoc vecng::operator*(T2, const vecng<DIM,T>&)
     * \relates vecng
     */
    template <class T2, class T>
    inline vecng<4, T> operator* (
        T2 s, const vecng<4, T>& v
    ) {
        return vecng<4, T>(T(s) * v.x, T(s) * v.y, T(s) * v.z, T(s) * v.w);
    }

    /**
     * \brief Writes a vector to a stream
     * \details This writes the coordinates of vector \p v separated by a
     * space character to the output stream \p out.
     * \param[in] out the output stream
     * \param[in] v the vector to write
     * \return a reference to the output stream \p out
     * \relates vecng
     */
    template <index_t DIM, class T>
    inline std::ostream& operator<< (
        std::ostream& out, const GEO::vecng<DIM, T>& v
    ) {
        const char* sep = "";
        for(index_t i = 0; i < DIM; i++) {
            out << sep << v[i];
            sep = " ";
        }
        return out;
    }

    /**
     * \brief Reads a vector from a stream
     * \details This reads \p DIM coordinates from the input stream \p in and
     * stores them in vector \p v
     * \param[in] in the input stream
     * \param[out] v the vector to read
     * \return a reference to the input stream \p in
     * \relates vecng
     */
    template <index_t DIM, class T>
    inline std::istream& operator>> (
        std::istream& in, GEO::vecng<DIM, T>& v
    ) {
        for(index_t i = 0; i < DIM; i++) {
            in >> v[i];
        }
        return in;
    }
}

#endif

