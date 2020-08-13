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
//  Implements a simple singleton template
//
//=============================================================================


#ifndef __SINGLETON_HH__
#define __SINGLETON_HH__


//=== INCLUDES ================================================================

// OpenMesh
#include <OpenMesh/Core/System/config.h>

// STL
#include <stdexcept>
#include <iostream>


//== NAMESPACES ===============================================================


namespace OpenMesh {


//=== IMPLEMENTATION ==========================================================


/** A simple singleton template.
    Encapsulates an arbitrary class and enforces its uniqueness.
*/

template <typename T>
class SingletonT
{
public:

  /** Singleton access function.
      Use this function to obtain a reference to the instance of the 
      encapsulated class. Note that this instance is unique and created
      on the first call to Instance().
  */

  static T& Instance()
  {
    if (!pInstance__)
    {
      // check if singleton alive
      if (destroyed__)
      {
	OnDeadReference();
      }
      // first time request -> initialize
      else
      {
	Create();
      }
    }
    return *pInstance__;
  }


private:

  // Disable constructors/assignment to enforce uniqueness
  SingletonT();
  SingletonT(const SingletonT&);
  SingletonT& operator=(const SingletonT&);

  // Create a new singleton and store its pointer
  static void Create()
  {
    static T theInstance;
    pInstance__ = &theInstance;
  }
  
  // Will be called if instance is accessed after its lifetime has expired
  static void OnDeadReference()
  {
    throw std::runtime_error("[Singelton error] - Dead reference detected!\n");
  }

  virtual ~SingletonT()
  {
    pInstance__ = 0;
    destroyed__ = true;
  }
  
  static T*     pInstance__;
  static bool   destroyed__;
};



//=============================================================================
} // namespace OpenMesh
//=============================================================================
#if defined(OM_INCLUDE_TEMPLATES) && !defined(OPENMESH_SINGLETON_C)
#  define OPENMESH_SINGLETON_TEMPLATES
#  include "SingletonT.cc"
#endif
//=============================================================================
#endif // __SINGLETON_HH__
//=============================================================================
