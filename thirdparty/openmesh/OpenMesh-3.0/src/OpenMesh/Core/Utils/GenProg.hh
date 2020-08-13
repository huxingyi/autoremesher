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
//  Utils for generic/generative programming
//
//=============================================================================

#ifndef OPENMESH_GENPROG_HH
#define OPENMESH_GENPROG_HH


//== INCLUDES =================================================================

#include <OpenMesh/Core/System/config.h>


//== NAMESPACES ===============================================================

namespace OpenMesh {

namespace GenProg  {
#ifndef DOXY_IGNORE_THIS

//== IMPLEMENTATION ===========================================================


/// This type maps \c true or \c false to different types.
template <bool b> struct Bool2Type { enum { my_bool = b }; };

/// This class generates different types from different \c int 's.
template <int i>  struct Int2Type  { enum { my_int = i }; };

/// Handy typedef for Bool2Type<true> classes
typedef Bool2Type<true> True;

/// Handy typedef for Bool2Type<false> classes
typedef Bool2Type<false> False;

//-----------------------------------------------------------------------------
/// compile time assertions 
template <bool Expr> struct AssertCompile;
template <> struct AssertCompile<true> {};



//--- Template "if" w/ partial specialization ---------------------------------
#if OM_PARTIAL_SPECIALIZATION


template <bool condition, class Then, class Else>
struct IF { typedef Then Result; };

/** Template \c IF w/ partial specialization
\code
typedef IF<bool, Then, Else>::Result  ResultType;
\endcode    
*/
template <class Then, class Else>
struct IF<false, Then, Else> { typedef Else Result; };





//--- Template "if" w/o partial specialization --------------------------------
#else


struct SelectThen 
{
  template <class Then, class Else> struct Select {
    typedef Then Result;
  };
};

struct SelectElse
{
  template <class Then, class Else> struct Select {
    typedef Else Result;
  };
};

template <bool condition> struct ChooseSelector {
  typedef SelectThen Result;
};

template <> struct ChooseSelector<false> {
  typedef SelectElse Result;
};


/** Template \c IF w/o partial specialization. Use it like
\code
typedef IF<bool, Then, Else>::Result  ResultType;
\endcode    
*/

template <bool condition, class Then, class Else>
class IF 
{ 
  typedef typename ChooseSelector<condition>::Result  Selector;
public:
  typedef typename Selector::template Select<Then, Else>::Result  Result;
};

#endif

//=============================================================================
#endif
} // namespace GenProg
} // namespace OpenMesh

#define assert_compile(EXPR)                GenProg::AssertCompile<(EXPR)>();

//=============================================================================
#endif // OPENMESH_GENPROG_HH defined
//=============================================================================
