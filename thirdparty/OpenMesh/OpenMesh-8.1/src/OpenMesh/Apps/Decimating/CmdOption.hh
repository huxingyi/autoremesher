/* ========================================================================= *
 *                                                                           *
 *                               OpenMesh                                    *
 *           Copyright (c) 2001-2015, RWTH-Aachen University                 *
 *           Department of Computer Graphics and Multimedia                  *
 *                          All rights reserved.                             *
 *                            www.openmesh.org                               *
 *                                                                           *
 *---------------------------------------------------------------------------*
 * This file is part of OpenMesh.                                            *
 *---------------------------------------------------------------------------*
 *                                                                           *
 * Redistribution and use in source and binary forms, with or without        *
 * modification, are permitted provided that the following conditions        *
 * are met:                                                                  *
 *                                                                           *
 * 1. Redistributions of source code must retain the above copyright notice, *
 *    this list of conditions and the following disclaimer.                  *
 *                                                                           *
 * 2. Redistributions in binary form must reproduce the above copyright      *
 *    notice, this list of conditions and the following disclaimer in the    *
 *    documentation and/or other materials provided with the distribution.   *
 *                                                                           *
 * 3. Neither the name of the copyright holder nor the names of its          *
 *    contributors may be used to endorse or promote products derived from   *
 *    this software without specific prior written permission.               *
 *                                                                           *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       *
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED *
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A           *
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER *
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,  *
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,       *
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR        *
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF    *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING      *
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS        *
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.              *
 *                                                                           *
 * ========================================================================= */



#ifndef CMDOPTION
#define CMDOPTION

template <typename T>
class CmdOption
{
public:

  typedef T value_type;

  explicit CmdOption(const T& _val) : val_(_val), valid_(true), enabled_(false) { }
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

  operator T* () const  { return is_valid() ? &val_ : nullptr; }

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
