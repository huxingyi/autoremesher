/*===========================================================================*\
 *                                                                           *
 *                               OpenMesh                                    *
 *      Copyright (C) 2001-2014 by Computer Graphics Group, RWTH Aachen      *
 *                           www.openmesh.org                                *
 *                                                                           *
 *---------------------------------------------------------------------------* 
 *  This file is part of OpenMesh.                                           *
 *                                                                           *
 *  OpenMesh is free software: you can redistribute it and/or modify         * 
 *  it under the terms of the GNU Lesser General Public License as           *
 *  published by the Free Software Foundation, either version 3 of           *
 *  the License, or (at your option) any later version with the              *
 *  following exceptions:                                                    *
 *                                                                           *
 *  If other files instantiate templates or use macros                       *
 *  or inline functions from this file, or you compile this file and         *
 *  link it with other files to produce an executable, this file does        *
 *  not by itself cause the resulting executable to be covered by the        *
 *  GNU Lesser General Public License. This exception does not however       *
 *  invalidate any other reasons why the executable file might be            *
 *  covered by the GNU Lesser General Public License.                        *
 *                                                                           *
 *  OpenMesh is distributed in the hope that it will be useful,              *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU Lesser General Public License for more details.                      *
 *                                                                           *
 *  You should have received a copy of the GNU LesserGeneral Public          *
 *  License along with OpenMesh.  If not,                                    *
 *  see <http://www.gnu.org/licenses/>.                                      *
 *                                                                           *
\*===========================================================================*/ 

/*===========================================================================*\
 *                                                                           *             
 *   $Revision$                                                         *
 *   $Date$                   *
 *                                                                           *
\*===========================================================================*/

#ifndef MATHDEFS_HH
#define MATHDEFS_HH

#include <cmath>
#include <cfloat>

#ifndef M_PI
  #define M_PI      3.14159265359
#endif

namespace OpenMesh
{

/** comparison operators with user-selected precision control
*/
template <class T, typename Real>
inline bool is_zero(const T& _a, Real _eps)
{ return fabs(_a) < _eps; }

template <class T1, class T2, typename Real>
inline bool is_eq(const T1& a, const T2& b, Real _eps)
{ return is_zero(a-b, _eps); }

template <class T1, class T2, typename Real>
inline bool is_gt(const T1& a, const T2& b, Real _eps)
{ return (a > b) && !is_eq(a,b,_eps); }

template <class T1, class T2, typename Real>
inline bool is_ge(const T1& a, const T2& b, Real _eps)
{ return (a > b) || is_eq(a,b,_eps); }

template <class T1, class T2, typename Real>
inline bool is_lt(const T1& a, const T2& b, Real _eps)
{ return (a < b) && !is_eq(a,b,_eps); }

template <class T1, class T2, typename Real>
inline bool is_le(const T1& a, const T2& b, Real _eps)
{ return (a < b) || is_eq(a,b,_eps); }

/*const float flt_eps__ = 10*FLT_EPSILON;
const double dbl_eps__ = 10*DBL_EPSILON;*/
const float flt_eps__ = (float)1e-05;
const double dbl_eps__ = 1e-09;

inline float eps__(float) 
{ return flt_eps__; }

inline double eps__(double)
{ return dbl_eps__; }

template <class T>
inline bool is_zero(const T& a)
{ return is_zero(a, eps__(a)); }

template <class T1, class T2>
inline bool is_eq(const T1& a, const T2& b)
{ return is_zero(a-b); }

template <class T1, class T2>
inline bool is_gt(const T1& a, const T2& b)
{ return (a > b) && !is_eq(a,b); }

template <class T1, class T2>
inline bool is_ge(const T1& a, const T2& b)
{ return (a > b) || is_eq(a,b); }

template <class T1, class T2>
inline bool is_lt(const T1& a, const T2& b)
{ return (a < b) && !is_eq(a,b); }

template <class T1, class T2>
inline bool is_le(const T1& a, const T2& b)
{ return (a < b) || is_eq(a,b); }

/// Trigonometry/angles - related

template <class T>
inline T sane_aarg(T _aarg)
{
  if (_aarg < -1)
  {
    _aarg = -1;
  }
  else if (_aarg >  1)
  {
    _aarg = 1;
  }
  return _aarg;
}

/** returns the angle determined by its cos and the sign of its sin
    result is positive if the angle is in [0:pi]
    and negative if it is in [pi:2pi]
*/
template <class T>
T angle(T _cos_angle, T _sin_angle)
{//sanity checks - otherwise acos will return nan
  _cos_angle = sane_aarg(_cos_angle);
  return (T) _sin_angle >= 0 ? acos(_cos_angle) : -acos(_cos_angle);
}

template <class T>
inline T positive_angle(T _angle)
{ return _angle < 0 ? (2*M_PI + _angle) : _angle; }

template <class T>
inline T positive_angle(T _cos_angle, T _sin_angle)
{ return positive_angle(angle(_cos_angle, _sin_angle)); }

template <class T>
inline T deg_to_rad(const T& _angle)
{ return M_PI*(_angle/180); }

template <class T>
inline T rad_to_deg(const T& _angle)
{ return 180*(_angle/M_PI); }

inline double log_(double _value)
{ return log(_value); }

}//namespace OpenMesh

#endif//MATHDEFS_HH
