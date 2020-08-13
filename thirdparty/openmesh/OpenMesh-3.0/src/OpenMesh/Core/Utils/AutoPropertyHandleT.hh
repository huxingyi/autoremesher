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

#ifndef OPENMESH_AutoPropertyHandleT_HH
#define OPENMESH_AutoPropertyHandleT_HH

//== INCLUDES =================================================================
#include <assert.h>
#include <string>

//== NAMESPACES ===============================================================

namespace OpenMesh {

//== CLASS DEFINITION =========================================================

template <class Mesh_, class PropertyHandle_>
class AutoPropertyHandleT : public PropertyHandle_
{
public:
  typedef Mesh_                             Mesh;
  typedef PropertyHandle_                   PropertyHandle;
  typedef PropertyHandle                    Base;
  typedef typename PropertyHandle::Value    Value;
  typedef AutoPropertyHandleT<Mesh, PropertyHandle>
                                            Self;
protected:
  Mesh*                                     m_;
  bool                                      own_property_;//ref counting?

public:
  AutoPropertyHandleT()
  : m_(NULL), own_property_(false)
  {}
  
  AutoPropertyHandleT(const Self& _other)
  : Base(_other.idx()), m_(_other.m_), own_property_(false)
  {}
  
  explicit AutoPropertyHandleT(Mesh& _m, const std::string& _pp_name = std::string())
  { add_property(_m, _pp_name); }

  AutoPropertyHandleT(Mesh& _m, PropertyHandle _pph)
  : Base(_pph.idx()), m_(&_m), own_property_(false)
  {}

  ~AutoPropertyHandleT()
  {
    if (own_property_)
    {
      m_->remove_property(*this);
    }
  }

  inline void                               add_property(Mesh& _m, const std::string& _pp_name = std::string())
  {
    assert(!is_valid());
    m_ = &_m;
    own_property_ = _pp_name.empty() || !m_->get_property_handle(*this, _pp_name);
    if (own_property_)
    {
      m_->add_property(*this, _pp_name);
    }
  }
  
  inline void                               remove_property()
  {
    assert(own_property_);//only the owner can delete the property
    m_->remove_property(*this);
    own_property_ = false;
    invalidate();
  }
  
  template <class _Handle>
  inline Value&                             operator [] (_Handle _hnd)
  { return m_->property(*this, _hnd); }

  template <class _Handle>
  inline const Value&                       operator [] (_Handle _hnd) const
  { return m_->property(*this, _hnd); }

  inline bool                               own_property() const
  { return own_property_; }

  inline void                               free_property()
  { own_property_ = false; }
};

//=============================================================================
} // namespace OpenMesh
//=============================================================================
#endif // OPENMESH_AutoPropertyHandleT_HH defined
//=============================================================================
