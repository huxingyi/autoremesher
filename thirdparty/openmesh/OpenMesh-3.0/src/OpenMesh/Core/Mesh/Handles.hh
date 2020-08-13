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

#ifndef OPENMESH_HANDLES_HH
#define OPENMESH_HANDLES_HH


//== INCLUDES =================================================================

#include <OpenMesh/Core/System/config.h>
#include <iostream>


//== NAMESPACES ===============================================================

namespace OpenMesh {

//== CLASS DEFINITION =========================================================


/// Base class for all handle types
class BaseHandle
{ 
public:
  
  explicit BaseHandle(int _idx=-1) : idx_(_idx) {}

  /// Get the underlying index of this handle
  int idx() const { return idx_; }

  /// The handle is valid iff the index is not equal to -1.
  bool is_valid() const { return idx_ != -1; }

  /// reset handle to be invalid
  void reset() { idx_=-1; }
  /// reset handle to be invalid
  void invalidate() { idx_ = -1; }

  bool operator==(const BaseHandle& _rhs) const { 
    return (this->idx_ == _rhs.idx_); 
  }

  bool operator!=(const BaseHandle& _rhs) const { 
    return (this->idx_ != _rhs.idx_); 
  }

  bool operator<(const BaseHandle& _rhs) const { 
    return (this->idx_ < _rhs.idx_); 
  }


  // this is to be used only by the iterators
  void __increment() { ++idx_; }
  void __decrement() { --idx_; }


private:

  int idx_; 
};


//-----------------------------------------------------------------------------

/// Write handle \c _hnd to stream \c _os
inline std::ostream& operator<<(std::ostream& _os, const BaseHandle& _hnd) 
{
  return (_os << _hnd.idx());
}


//-----------------------------------------------------------------------------


/// Handle for a vertex entity
struct VertexHandle : public BaseHandle
{
  explicit VertexHandle(int _idx=-1) : BaseHandle(_idx) {}
};


/// Handle for a halfedge entity
struct HalfedgeHandle : public BaseHandle
{
  explicit HalfedgeHandle(int _idx=-1) : BaseHandle(_idx) {}
};


/// Handle for a edge entity
struct EdgeHandle : public BaseHandle
{
  explicit EdgeHandle(int _idx=-1) : BaseHandle(_idx) {}
};


/// Handle for a face entity
struct FaceHandle : public BaseHandle
{
  explicit FaceHandle(int _idx=-1) : BaseHandle(_idx) {}
};


//=============================================================================
} // namespace OpenMesh
//=============================================================================
#endif // OPENMESH_HANDLES_HH
//=============================================================================
