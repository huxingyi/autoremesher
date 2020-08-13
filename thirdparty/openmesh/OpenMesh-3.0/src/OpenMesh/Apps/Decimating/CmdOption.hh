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

#ifndef CMDOPTION
#define CMDOPTION

template <typename T>
class CmdOption
{
public:

  typedef T value_type;

  CmdOption(const T& _val) : val_(_val), valid_(true), enabled_(false) { }
  CmdOption() : val_(T()),valid_(false), enabled_(false) { }

  // has been set and has a value
  bool is_valid(void) const   { return valid_;   }
  bool has_value(void) const  { return is_valid(); }

  // has been set and may have an value (check with is_valid())
  bool is_enabled() const { return enabled_; }

  void enable() { enabled_ = true; }

  CmdOption& operator = ( const T& _val ) 
  { 
    val_ = _val;
    valid_=true; 
    enable();
    return *this; 
  }

  operator T  () const { return val_; }
  //  operator const T& () const { return val_; }

  operator T* () const  { return is_valid() ? &val_ : NULL; }

private:

  T    val_;
  bool valid_;
  bool enabled_;

private: // non-copyable
  
  CmdOption(const CmdOption&);
  CmdOption& operator = ( const CmdOption& );

};

template < typename T >
std::ostream& operator << ( std::ostream& _os, CmdOption<T>& _opt )
{
  _os << (T&)_opt;
  return _os;
}

#endif
