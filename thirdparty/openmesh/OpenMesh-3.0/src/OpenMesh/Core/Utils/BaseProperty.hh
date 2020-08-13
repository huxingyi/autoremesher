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

#ifndef OPENMESH_BASEPROPERTY_HH
#define OPENMESH_BASEPROPERTY_HH

#include <string>
#include <OpenMesh/Core/IO/StoreRestore.hh>
#include <OpenMesh/Core/System/omstream.hh>

namespace OpenMesh {

//== CLASS DEFINITION =========================================================

/** \class BaseProperty Property.hh <OpenMesh/Core/Utils/PropertyT.hh>

    Abstract class defining the basic interface of a dynamic property.
**/

class OPENMESHDLLEXPORT BaseProperty
{
public:

  /// Indicates an error when a size is returned by a member.
  static const size_t UnknownSize = size_t(-1);

public:

  /// \brief Default constructor.
  ///
  /// In %OpenMesh all mesh data is stored in so-called properties.
  /// We distinuish between standard properties, which can be defined at
  /// compile time using the Attributes in the traits definition and
  /// at runtime using the request property functions defined in one of
  /// the kernels.
  ///
  /// If the property should be stored along with the default properties
  /// in the OM-format one must name the property and enable the persistant
  /// flag with set_persistent().
  ///
  /// \param _name Optional textual name for the property.
  ///
  BaseProperty(const std::string& _name = "<unknown>")
  : name_(_name), persistent_(false)
  {}

  /// \brief Copy constructor
  BaseProperty(const BaseProperty & _rhs)
      : name_( _rhs.name_ ), persistent_( _rhs.persistent_ ) {}

  /// Destructor.
  virtual ~BaseProperty() {}

public: // synchronized array interface

  /// Reserve memory for n elements.
  virtual void reserve(size_t _n) = 0;

  /// Resize storage to hold n elements.
  virtual void resize(size_t _n) = 0;

  /// Clear all elements and free memory.
  virtual void clear() = 0;

  /// Extend the number of elements by one.
  virtual void push_back() = 0;

  /// Let two elements swap their storage place.
  virtual void swap(size_t _i0, size_t _i1) = 0;

  /// Copy one element to another
  virtual void copy(size_t _io, size_t _i1) = 0;
  
  /// Return a deep copy of self.
  virtual BaseProperty* clone () const = 0;

public: // named property interface

  /// Return the name of the property
  const std::string& name() const { return name_; }

  virtual void stats(std::ostream& _ostr) const;

public: // I/O support

  /// Returns true if the persistent flag is enabled else false.
  bool persistent(void) const { return persistent_; }

  /// Enable or disable persistency. Self must be a named property to enable
  /// persistency.
  virtual void set_persistent( bool _yn ) = 0;

  /// Number of elements in property
  virtual size_t       n_elements() const = 0;

  /// Size of one element in bytes or UnknownSize if not known.
  virtual size_t       element_size() const = 0;

  /// Return size of property in bytes
  virtual size_t       size_of() const
  {
    return size_of( n_elements() );
  }

  /// Estimated size of property if it has _n_elem elements.
  /// The member returns UnknownSize if the size cannot be estimated.
  virtual size_t       size_of(size_t _n_elem) const
  {
    return (element_size()!=UnknownSize)
      ? (_n_elem*element_size())
      : UnknownSize;
  }

  /// Store self as one binary block
  virtual size_t store( std::ostream& _ostr, bool _swap ) const = 0;

  /** Restore self from a binary block. Uses reserve() to set the
      size of self before restoring.
  **/
  virtual size_t restore( std::istream& _istr, bool _swap ) = 0;

protected:

  // To be used in a derived class, when overloading set_persistent()
  template < typename T >
  void check_and_set_persistent( bool _yn )
  {
    if ( _yn && !IO::is_streamable<T>() )
      omerr() << "Warning! Type of property value is not binary storable!\n";
    persistent_ = IO::is_streamable<T>() && _yn;
  }

private:

  std::string name_;
  bool        persistent_;
};

}//namespace OpenMesh

#endif //OPENMESH_BASEPROPERTY_HH


