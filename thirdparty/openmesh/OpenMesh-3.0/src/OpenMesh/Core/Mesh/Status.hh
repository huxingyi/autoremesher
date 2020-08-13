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
//  CLASS Status
//
//=============================================================================


#ifndef OPENMESH_ATTRIBUTE_STATUS_HH
#define OPENMESH_ATTRIBUTE_STATUS_HH


//== INCLUDES =================================================================

#include <OpenMesh/Core/System/config.h>


//== NAMESPACES ===============================================================


namespace OpenMesh {
namespace Attributes {

 
//== CLASS DEFINITION  ========================================================
  

/** Status bits used by the Status class. 
 *  \see OpenMesh::Attributes::StatusInfo
 */
enum StatusBits {

  DELETED               = 1,    ///< Item has been deleted
  LOCKED                = 2,    ///< Item is locked.
  SELECTED              = 4,    ///< Item is selected.
  HIDDEN                = 8,    ///< Item is hidden.
  FEATURE               = 16,   ///< Item is a feature or belongs to a feature.
  TAGGED                = 32,   ///< Item is tagged.
  TAGGED2               = 64,   ///< Alternate bit for tagging an item.
  FIXEDNONMANIFOLD      = 128,  ///< Item was non-two-manifold and had to be fixed
  UNUSED                = 256   ///< Unused
};


/** \class StatusInfo Status.hh <OpenMesh/Attributes/Status.hh>
 *
 *   Add status information to a base class.
 *
 *   \see StatusBits
 */
class StatusInfo
{
public:

  typedef unsigned int value_type;
    
  StatusInfo() : status_(0) {}

  /// is deleted ?
  bool deleted() const  { return is_bit_set(DELETED); }
  /// set deleted
  void set_deleted(bool _b) { change_bit(DELETED, _b); }


  /// is locked ?
  bool locked() const  { return is_bit_set(LOCKED); }
  /// set locked
  void set_locked(bool _b) { change_bit(LOCKED, _b); }


  /// is selected ?
  bool selected() const  { return is_bit_set(SELECTED); }
  /// set selected
  void set_selected(bool _b) { change_bit(SELECTED, _b); }


  /// is hidden ?
  bool hidden() const  { return is_bit_set(HIDDEN); }
  /// set hidden
  void set_hidden(bool _b) { change_bit(HIDDEN, _b); }


  /// is feature ?
  bool feature() const  { return is_bit_set(FEATURE); }
  /// set feature
  void set_feature(bool _b) { change_bit(FEATURE, _b); }


  /// is tagged ?
  bool tagged() const  { return is_bit_set(TAGGED); }
  /// set tagged
  void set_tagged(bool _b) { change_bit(TAGGED, _b); }


  /// is tagged2 ? This is just one more tag info.
  bool tagged2() const  { return is_bit_set(TAGGED2); }
  /// set tagged
  void set_tagged2(bool _b) { change_bit(TAGGED2, _b); }
  
  
  /// is fixed non-manifold ?
  bool fixed_nonmanifold() const  { return is_bit_set(FIXEDNONMANIFOLD); }
  /// set fixed non-manifold
  void set_fixed_nonmanifold(bool _b) { change_bit(FIXEDNONMANIFOLD, _b); }


  /// return whole status
  unsigned int bits() const { return status_; }
  /// set whole status at once
  void set_bits(unsigned int _bits) { status_ = _bits; }


  /// is a certain bit set ?
  bool is_bit_set(unsigned int _s) const { return (status_ & _s) > 0; }
  /// set a certain bit
  void set_bit(unsigned int _s) { status_ |= _s; }
  /// unset a certain bit
  void unset_bit(unsigned int _s) { status_ &= ~_s; }
  /// set or unset a certain bit
  void change_bit(unsigned int _s, bool _b) {  
    if (_b) status_ |= _s; else status_ &= ~_s; }


private: 

  value_type status_;
};


//=============================================================================
} // namespace Attributes
} // namespace OpenMesh
//=============================================================================
#endif // OPENMESH_ATTRIBUTE_STATUS_HH defined
//=============================================================================
