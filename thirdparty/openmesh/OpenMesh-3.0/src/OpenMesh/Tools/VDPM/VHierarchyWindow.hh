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

//=============================================================================
//
//  CLASS newClass
//
//=============================================================================

#ifndef OPENMESH_VDPROGMESH_VHIERARCHYWINDOWS_HH
#define OPENMESH_VDPROGMESH_VHIERARCHYWINDOWS_HH


//== INCLUDES =================================================================

#include <OpenMesh/Tools/VDPM/VHierarchy.hh>
#include <algorithm>

//== FORWARDDECLARATIONS ======================================================


//== NAMESPACES ===============================================================

namespace OpenMesh {
namespace VDPM {

//== CLASS DEFINITION =========================================================

	      
/** \todo VHierarchyWindow documentation
*/
class VHierarchyWindow
{
private:

  // reference of vertex hierarchy
  VHierarchy    *vhierarchy_;

  // bits buffer (byte units)
  unsigned char *buffer_;
  int           buffer_min_;
  size_t        buffer_max_;
  int           current_pos_;

  // window (byte units)
  int           window_min_;
  int           window_max_;
  

  // # of right shift (bit units)
  unsigned char n_shift_;           // [0, 7]

  unsigned char flag8(unsigned char n_shift) const
  { return 0x80 >> n_shift; }  

  unsigned char flag8(VHierarchyNodeHandle _node_handle) const
  {
    assert(_node_handle.idx() >= 0);
    return  0x80 >> (unsigned int) (_node_handle.idx() % 8);
  }
  int byte_idx(VHierarchyNodeHandle _node_handle) const
  {
    assert(_node_handle.idx() >= 0);
    return  _node_handle.idx() / 8;
  }
  int buffer_idx(VHierarchyNodeHandle _node_handle) const
  { return  byte_idx(_node_handle) - buffer_min_; } 

  bool before_window(VHierarchyNodeHandle _node_handle) const
  { return (_node_handle.idx()/8 < window_min_) ? true : false; }

  bool after_window(VHierarchyNodeHandle _node_handle) const
  { return (_node_handle.idx()/8 < window_max_) ? false : true; }

  bool underflow(VHierarchyNodeHandle _node_handle) const
  { return (_node_handle.idx()/8 < buffer_min_) ? true : false; }

  bool overflow(VHierarchyNodeHandle _node_handle) const
  { return (_node_handle.idx()/8 < int(buffer_max_) ) ? false : true; }

  bool update_buffer(VHierarchyNodeHandle _node_handle);

public:
  VHierarchyWindow();
  VHierarchyWindow(VHierarchy &_vhierarchy);
  ~VHierarchyWindow(void);
  
  void set_vertex_hierarchy(VHierarchy &_vhierarchy)
  { vhierarchy_ = &_vhierarchy; }

  void begin()
  {
    int new_window_min = window_min_;
    for (current_pos_=window_min_-buffer_min_; 
	 current_pos_ < window_size(); ++current_pos_)
    {
      if (buffer_[current_pos_] == 0)   
	++new_window_min;
      else
      {
        n_shift_ = 0;
        while ((buffer_[current_pos_] & flag8(n_shift_)) == 0)
          ++n_shift_;
        break;
      }
    }
    window_min_ = new_window_min;
  }

  void next()
  {
    ++n_shift_;
    if (n_shift_ == 8)
    {
      n_shift_ = 0;
      ++current_pos_;
    }

    while (current_pos_ < window_max_-buffer_min_)
    {
      if (buffer_[current_pos_] != 0) // if the current byte has non-zero bits
      {
        while (n_shift_ != 8)
        {
          if ((buffer_[current_pos_] & flag8(n_shift_)) != 0)
            return;                     // find 1 bit in the current byte
          ++n_shift_;
        }
      }
      n_shift_ = 0;
      ++current_pos_;
    }
  }
  bool end() { return !(current_pos_ < window_max_-buffer_min_); }

  int window_size() const      { return  window_max_ - window_min_; }
  size_t buffer_size() const      { return  buffer_max_ - buffer_min_; }

  VHierarchyNodeHandle node_handle()
  {
    return  VHierarchyNodeHandle(8*(buffer_min_+current_pos_) + (int)n_shift_);
  }

  void activate(VHierarchyNodeHandle _node_handle)
  {
    update_buffer(_node_handle);
    buffer_[buffer_idx(_node_handle)] |= flag8(_node_handle);
    window_min_ = std::min(window_min_, byte_idx(_node_handle));
    window_max_ = std::max(window_max_, 1+byte_idx(_node_handle));
  }


  void inactivate(VHierarchyNodeHandle _node_handle)
  {
    if (is_active(_node_handle) != true)  return;
    buffer_[buffer_idx(_node_handle)] ^= flag8(_node_handle);
  }


  bool is_active(VHierarchyNodeHandle _node_handle) const
  {
    if (before_window(_node_handle) == true ||
	after_window(_node_handle) == true)
      return  false;
    return ((buffer_[buffer_idx(_node_handle)] & flag8(_node_handle)) > 0);
  }

  void init(VHierarchyNodeHandleContainer &_roots);
  void update_with_vsplit(VHierarchyNodeHandle _parent_handle);
  void update_with_ecol(VHierarchyNodeHandle _parent_handle);
};

//=============================================================================
} // namespace VDPM
} // namespace OpenMesh
//=============================================================================
#endif // OPENMESH_VDPROGMESH_VHIERARCHYWINDOWS_HH
//=============================================================================

