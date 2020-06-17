/*
 * Copyright 2013 Computer Graphics Group, RWTH Aachen University
 * Authors: David Bommes <bommes@cs.rwth-aachen.de>
 *          Hans-Christian Ebke <ebke@cs.rwth-aachen.de>
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

#ifndef QEX_TRANSITIONFUNCTION_HH
#define QEX_TRANSITIONFUNCTION_HH

#include <sstream>
#include <vector>
#include <complex>
#include <assert.h>

#define ROUND_TF(x) ((x)<0?int((x)-0.5):int((x)+0.5))

namespace QEx {

// Transition Function for Integer-Grid-Mappings
template<class TransitionTypeT>
class TransitionFunctionT {
    public:

        // access value type
        typedef TransitionTypeT TType;
        typedef std::complex<double> Complex;

        // rotation (times PI/2)
        int r;
        // TType translation (double, integer, ...)
        TType tu;
        TType tv;

        explicit TransitionFunctionT(int _r = 0, TType _tu = 0, TType _tv = 0) :
            r(_r), tu(_tu), tv(_tv) {
        }

        static const TransitionFunctionT<TransitionTypeT> IDENTITY;

        // transform uv-coords TF(u,v) = R^r(u,v)
        template<class VType>
        inline void transform_vector(VType& _u, VType& _v) const {
            // r should always be between 0 and 3
            assert(r >= 0 && r < 4);
            // rotate
            switch (r) {
                case 1:
                    std::swap(_u, _v);
                    _u *= -1.0;
                    break;
                case 2:
                    _u *= -1.0;
                    _v *= -1.0;
                    break;
                case 3:
                    std::swap(_u, _v);
                    _v *= -1.0;
                    break;
                default:
                    break;
            }
        }

        // transform uv-coords TF(u,v) = R^r(u,v) + (tu,tv)
        template<class VType>
        inline void transform_point(VType& _u, VType& _v) const {
            // r should always be between 0 and 3
            assert(r >= 0 && r < 4);

            transform_vector(_u, _v);

            // translate
            _u += tu;
            _v += tv;
        }

        template<class VType>
        inline void transform_point(std::complex<VType> &rhs) const {
            /*
             * This is the standard conforming way of accessing the real
             * and imaginary part of a std::complex.
             * See http://en.cppreference.com/w/cpp/numeric/complex
             */
            transform_point(
                    reinterpret_cast<VType(&)[2]>(rhs)[0],
                    reinterpret_cast<VType(&)[2]>(rhs)[1]);
        }

        // transform uv-coords for 2D-type TF(u) = R^r(u)
        template<class VType2>
        inline void transform_vector(VType2& _uv) const {
            double u = _uv[0];
            double v = _uv[1];
            transform_vector(u, v);
            _uv = VType2(u, v);
        }

        // transform uv-coords for 2D-type TF(u) = R^r(u) + t
        template<class VType2>
        inline void transform_point(VType2& _uv) const {
            double u = _uv[0];
            double v = _uv[1];
            transform_point(u, v);
            _uv = VType2(u, v);
        }

        // composition of transition functions
        // T1(T2(*)) = R^r1(R^r2(x)+t2)+t1 = R^(r1+r2)(x) + (R^r1(t2)+t1) = R^(r1+r2)(x) + T1(t2)
        TransitionFunctionT<TType> operator *(const TransitionFunctionT<TType>& _tf2) const {
            TransitionFunctionT tf(_tf2);
            this->transform_point(tf.tu, tf.tv);
            tf.r = (tf.r + this->r) % 4;
            return tf;
        }

        bool operator==(const TransitionFunctionT<TType>& _tf2) const {
            return ((this->r == _tf2.r) && (this->tu == _tf2.tu) && (this->tv == _tf2.tv));
        }

        bool operator!=(const TransitionFunctionT<TType>& _tf2) const {
            return !(*this == _tf2);
        }

        // inverse transition function TF^-1(x) = R^-r(x) - R^-r(t)
        TransitionFunctionT<TType> inverse() const {
            int r_new = (4 - r) % 4;
            TType tu_new = -tu;
            TType tv_new = -tv;
            // r should always be between 0 and 3
            assert(r_new >= 0 && r_new < 4);
            // rotate
            switch (r_new) {
                case 1:
                    std::swap(tu_new, tv_new);
                    tu_new *= -1.0;
                    break;
                case 2:
                    tu_new *= TType(-1);
                    tv_new *= TType(-1);
                    break;
                case 3:
                    std::swap(tu_new, tv_new);
                    tv_new *= -1.0;
                    break;
                default:
                    break;
            }
            return TransitionFunctionT<TType>(r_new, tu_new, tv_new);
        }

        // uniquely compute transition function based on two point pairs
        template<class VType2>
        void estimate_from_point_pair(const VType2& _uv0, const VType2& _uv0_mapped,
                                      const VType2& _uv1, const VType2& _uv1_mapped) {
            Complex l0(_uv0[0], _uv0[1]);
            Complex l1(_uv1[0], _uv1[1]);
            Complex r0(_uv0_mapped[0], _uv0_mapped[1]);
            Complex r1(_uv1_mapped[0], _uv1_mapped[1]);

            // compute rotational part via complex numbers
            r = ROUND_TF(2.0*std::log((r0-r1)/(l0-l1)).imag()/M_PI);
            r = ((r % 4) + 4) % 4; // assure that r is between 0 and 3
            // compute translational part
            Complex t = r0 - std::pow(Complex(0, 1), r) * l0;
            // store translational part
            tu = ROUND_TF(t.real());
            tv = ROUND_TF(t.imag());
        }

        template<typename STREAM>
        friend STREAM &operator<<(STREAM &s, const TransitionFunctionT<TType> &self) {
            return s << "Rotation: " << self.r << ", Translation: " << self.tu << "/" << self.tv;
        }

        std::string toStrCompact() const {
            std::ostringstream res;
            res << r << ": " << tu << ", " << tv;
            return res.str();
        }

        // identity transition function
        static TransitionFunctionT<TType> identity() {
            return TransitionFunctionT<TType>(0, 0, 0);
        }
};

template<typename TransitionTypeT>
const TransitionFunctionT<TransitionTypeT> TransitionFunctionT<TransitionTypeT>::IDENTITY;

typedef TransitionFunctionT<int> TransitionFunctionInt;
typedef TransitionFunctionT<double> TransitionFunctionDouble;

} // namespace QEx

#endif // QEX_TRANSITIONFUNCTION_HH defined

