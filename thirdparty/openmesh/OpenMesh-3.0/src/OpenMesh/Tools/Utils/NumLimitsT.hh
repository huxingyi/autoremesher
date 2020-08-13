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

/** \file Tools/Utils/NumLimitsT.hh
    Temporary solution until std::numeric_limits is standard.
 */

//=============================================================================
//
//  CLASS NumLimitsT
//
//=============================================================================

#ifndef OPENMESH_UTILS_NUMLIMITS_HH
#define OPENMESH_UTILS_NUMLIMITS_HH


//== INCLUDES =================================================================

#include "Config.hh"
#include <limits.h>
#include <float.h>


//== NAMESPEACES ==============================================================

namespace OpenMesh { // BEGIN_NS_OPENMESH
namespace Utils { // BEGIN_NS_UTILS


//== CLASS DEFINITION =========================================================


/** \class NumLimitsT Tools/Utils/NumLimitsT.hh 

    This class provides the maximum and minimum values a certain
    scalar type (\c int, \c float, or \c double) can store. You can
    use it like this:
    \code
    #include <OpenMesh/Utils/NumLimitsT.hh>

    int   float_min   = OpenMesh::NumLimitsT<float>::min();
    float double_max  = OpenMesh::NumLimitsT<double>::max();
    \endcode
    
    \note This functionality should be provided by
    std::numeric_limits.  This template does not exist on gcc <=
    2.95.3. The class template NumLimitsT is just a workaround.
**/
template <typename Scalar>
class NumLimitsT
{
public:
  /// Return the smallest \em absolte value a scalar type can store.
  static inline Scalar min() { return 0; }
  /// Return the maximum \em absolte value a scalar type can store.
  static inline Scalar max() { return 0; }

  static inline bool   is_float()   { return false; }
  static inline bool   is_integer() { return !NumLimitsT<Scalar>::is_float(); }
  static inline bool   is_signed()  { return true; }
};

  // is_float

template<> 
inline bool NumLimitsT<float>::is_float() { return true; }

template<> 
inline bool NumLimitsT<double>::is_float() { return true; }

template<> 
inline bool NumLimitsT<long double>::is_float() { return true; }

  // is_signed

template<> 
inline bool NumLimitsT<unsigned char>::is_signed() { return false; }

template<> 
inline bool NumLimitsT<unsigned short>::is_signed() { return false; }

template<> 
inline bool NumLimitsT<unsigned int>::is_signed() { return false; }

template<> 
inline bool NumLimitsT<unsigned long>::is_signed() { return false; }

template<> 
inline bool NumLimitsT<unsigned long long>::is_signed() { return false; }

  // min/max
template<> inline int  NumLimitsT<int>::min() { return INT_MIN; }
template<> inline int  NumLimitsT<int>::max() { return INT_MAX; }

template<> inline float NumLimitsT<float>::min() { return FLT_MIN; }
template<> inline float NumLimitsT<float>::max() { return FLT_MAX; }

template<> inline double NumLimitsT<double>::min() { return DBL_MIN; }
template<> inline double NumLimitsT<double>::max() { return DBL_MAX; }


//=============================================================================
} // END_NS_UTILS
} // END_NS_OPENMESH
//=============================================================================
#endif // OPENMESH_NUMLIMITS_HH defined
//=============================================================================

