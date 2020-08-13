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
//  Helper Functions for binary reading / writing
//
//=============================================================================


//== INCLUDES =================================================================


#include <OpenMesh/Core/System/config.h>
// -------------------- STL
#include <algorithm>
// -------------------- OpenMesh
#include <OpenMesh/Core/IO/BinaryHelper.hh>


//== NAMESPACES ===============================================================


namespace OpenMesh {
namespace IO {

#ifndef DOXY_IGNORE_THIS

//== IMPLEMENTATION ===========================================================

//-----------------------------------------------------------------------------

short int read_short(FILE* _in, bool _swap)
{
  union u1 { short int s; unsigned char c[2]; }  sc;
  fread((char*)sc.c, 1, 2, _in);
  if (_swap) std::swap(sc.c[0], sc.c[1]);
  return sc.s;
}


//-----------------------------------------------------------------------------


int read_int(FILE* _in, bool _swap)
{
  union u2 { int i; unsigned char c[4]; } ic;
  fread((char*)ic.c, 1, 4, _in);
  if (_swap) {
    std::swap(ic.c[0], ic.c[3]);
    std::swap(ic.c[1], ic.c[2]);
  }
  return ic.i;
}


//-----------------------------------------------------------------------------


float read_float(FILE* _in, bool _swap)
{
  union u3 { float f; unsigned char c[4]; } fc;
  fread((char*)fc.c, 1, 4, _in);
  if (_swap) {
    std::swap(fc.c[0], fc.c[3]);
    std::swap(fc.c[1], fc.c[2]);
  }
  return fc.f;
}


//-----------------------------------------------------------------------------


double read_double(FILE* _in, bool _swap)
{
  union u4 { double d; unsigned char c[8]; } dc;
  fread((char*)dc.c, 1, 8, _in);
  if (_swap) {
    std::swap(dc.c[0], dc.c[7]);
    std::swap(dc.c[1], dc.c[6]);
    std::swap(dc.c[2], dc.c[5]);
    std::swap(dc.c[3], dc.c[4]);
  }
  return dc.d;
}

//-----------------------------------------------------------------------------

short int read_short(std::istream& _in, bool _swap)
{
  union u1 { short int s; unsigned char c[2]; }  sc;
  _in.read((char*)sc.c, 2);
  if (_swap) std::swap(sc.c[0], sc.c[1]);
  return sc.s;
}


//-----------------------------------------------------------------------------


int read_int(std::istream& _in, bool _swap)
{
  union u2 { int i; unsigned char c[4]; } ic;
  _in.read((char*)ic.c, 4);
  if (_swap) {
    std::swap(ic.c[0], ic.c[3]);
    std::swap(ic.c[1], ic.c[2]);
  }
  return ic.i;
}


//-----------------------------------------------------------------------------


float read_float(std::istream& _in, bool _swap)
{
  union u3 { float f; unsigned char c[4]; } fc;
  _in.read((char*)fc.c, 4);
  if (_swap) {
    std::swap(fc.c[0], fc.c[3]);
    std::swap(fc.c[1], fc.c[2]);
  }
  return fc.f;
}


//-----------------------------------------------------------------------------


double read_double(std::istream& _in, bool _swap)
{
  union u4 { double d; unsigned char c[8]; } dc;
  _in.read((char*)dc.c, 8);
  if (_swap) {
    std::swap(dc.c[0], dc.c[7]);
    std::swap(dc.c[1], dc.c[6]);
    std::swap(dc.c[2], dc.c[5]);
    std::swap(dc.c[3], dc.c[4]);
  }
  return dc.d;
}


//-----------------------------------------------------------------------------


void write_short(short int _i, FILE* _out, bool _swap)
{
  union u1 { short int s; unsigned char c[2]; } sc;
  sc.s = _i;
  if (_swap) std::swap(sc.c[0], sc.c[1]);
  fwrite((char*)sc.c, 1, 2, _out);
}


//-----------------------------------------------------------------------------


void write_int(int _i, FILE* _out, bool _swap)
{
  union u2 { int i; unsigned char c[4]; } ic;
  ic.i = _i;
  if (_swap) {
    std::swap(ic.c[0], ic.c[3]);
    std::swap(ic.c[1], ic.c[2]);
  }
  fwrite((char*)ic.c, 1, 4, _out);
}


//-----------------------------------------------------------------------------


void write_float(float _f, FILE* _out, bool _swap)
{
  union u3 { float f; unsigned char c[4]; } fc;
  fc.f = _f;
  if (_swap) {
    std::swap(fc.c[0], fc.c[3]);
    std::swap(fc.c[1], fc.c[2]);
  }
  fwrite((char*)fc.c, 1, 4, _out);
}


//-----------------------------------------------------------------------------


void write_double(double _d, FILE* _out, bool _swap)
{
  union u4 { double d; unsigned char c[8]; } dc;
  dc.d = _d;
  if (_swap) {
    std::swap(dc.c[0], dc.c[7]);
    std::swap(dc.c[1], dc.c[6]);
    std::swap(dc.c[2], dc.c[5]);
    std::swap(dc.c[3], dc.c[4]);
  }
  fwrite((char*)dc.c, 1, 8, _out);
}


//-----------------------------------------------------------------------------


void write_short(short int _i, std::ostream& _out, bool _swap)
{
  union u1 { short int s; unsigned char c[2]; } sc;
  sc.s = _i;
  if (_swap) std::swap(sc.c[0], sc.c[1]);
  _out.write((char*)sc.c, 2);
}


//-----------------------------------------------------------------------------


void write_int(int _i, std::ostream& _out, bool _swap)
{
  union u2 { int i; unsigned char c[4]; } ic;
  ic.i = _i;
  if (_swap) {
    std::swap(ic.c[0], ic.c[3]);
    std::swap(ic.c[1], ic.c[2]);
  }
  _out.write((char*)ic.c, 4);
}


//-----------------------------------------------------------------------------


void write_float(float _f, std::ostream& _out, bool _swap)
{
  union u3 { float f; unsigned char c[4]; } fc;
  fc.f = _f;
  if (_swap) {
    std::swap(fc.c[0], fc.c[3]);
    std::swap(fc.c[1], fc.c[2]);
  }
  _out.write((char*)fc.c, 4);
}


//-----------------------------------------------------------------------------


void write_double(double _d, std::ostream& _out, bool _swap)
{
  union u4 { double d; unsigned char c[8]; } dc;
  dc.d = _d;
  if (_swap) {
    std::swap(dc.c[0], dc.c[7]);
    std::swap(dc.c[1], dc.c[6]);
    std::swap(dc.c[2], dc.c[5]);
    std::swap(dc.c[3], dc.c[4]);
  }
  _out.write((char*)dc.c, 8);
}


#endif

//=============================================================================
} // namespace IO
} // namespace OpenMesh
//=============================================================================
