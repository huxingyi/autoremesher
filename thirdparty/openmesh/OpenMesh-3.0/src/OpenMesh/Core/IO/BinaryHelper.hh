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

#ifndef OPENMESH_BINARY_HELPER_HH
#define OPENMESH_BINARY_HELPER_HH


//== INCLUDES =================================================================

#include <OpenMesh/Core/System/config.h>
// -------------------- STL
#if defined( OM_CC_MIPS )
#  include <stdio.h>
#else
#  include <cstdio>
#endif
#include <iostream>
// -------------------- OpenMesh


//== NAMESPACES ===============================================================

namespace OpenMesh {
namespace IO {


//=============================================================================


/** \name Handling binary input/output.
    These functions take care of swapping bytes to get the right Endian.
*/
//@{

//-----------------------------------------------------------------------------


/** Binary read a \c short from \c _is and perform byte swapping if
    \c _swap is true */
short int read_short(FILE* _in, bool _swap=false);

/** Binary read an \c int from \c _is and perform byte swapping if
    \c _swap is true */
int read_int(FILE* _in, bool _swap=false);

/** Binary read a \c float from \c _is and perform byte swapping if
    \c _swap is true */
float read_float(FILE* _in, bool _swap=false);

/** Binary read a \c double from \c _is and perform byte swapping if
    \c _swap is true */
double read_double(FILE* _in, bool _swap=false);

/** Binary read a \c short from \c _is and perform byte swapping if
    \c _swap is true */
short int read_short(std::istream& _in, bool _swap=false);

/** Binary read an \c int from \c _is and perform byte swapping if
    \c _swap is true */
int read_int(std::istream& _in, bool _swap=false);

/** Binary read a \c float from \c _is and perform byte swapping if
    \c _swap is true */
float read_float(std::istream& _in, bool _swap=false);

/** Binary read a \c double from \c _is and perform byte swapping if
    \c _swap is true */
double read_double(std::istream& _in, bool _swap=false);


/** Binary write a \c short to \c _os and perform byte swapping if
    \c _swap is true */
void write_short(short int _i, FILE* _out, bool _swap=false);

/** Binary write an \c int to \c _os and perform byte swapping if
    \c _swap is true */
void write_int(int _i, FILE* _out, bool _swap=false);

/** Binary write a \c float to \c _os and perform byte swapping if
    \c _swap is true */
void write_float(float _f, FILE* _out, bool _swap=false);

/** Binary write a \c double to \c _os and perform byte swapping if
    \c _swap is true */
void write_double(double _d, FILE* _out, bool _swap=false);

/** Binary write a \c short to \c _os and perform byte swapping if
    \c _swap is true */
void write_short(short int _i, std::ostream& _out, bool _swap=false);

/** Binary write an \c int to \c _os and perform byte swapping if
    \c _swap is true */
void write_int(int _i, std::ostream& _out, bool _swap=false);

/** Binary write a \c float to \c _os and perform byte swapping if
    \c _swap is true */
void write_float(float _f, std::ostream& _out, bool _swap=false);

/** Binary write a \c double to \c _os and perform byte swapping if
    \c _swap is true */
void write_double(double _d, std::ostream& _out, bool _swap=false);

//@}


//=============================================================================
} // namespace IO
} // namespace OpenMesh
//=============================================================================
#endif // OPENMESH_MESHREADER_HH defined
//=============================================================================

