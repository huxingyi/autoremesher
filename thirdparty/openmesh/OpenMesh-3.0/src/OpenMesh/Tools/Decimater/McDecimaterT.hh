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

/** \file McDecimaterT.hh
 */

//=============================================================================
//
//  CLASS McDecimaterT
//
//=============================================================================

#ifndef OPENMESH_MC_DECIMATER_DECIMATERT_HH
#define OPENMESH_MC_DECIMATER_DECIMATERT_HH


//== INCLUDES =================================================================

#include <memory>
#include <OpenMesh/Tools/Decimater/BaseDecimaterT.hh>



//== NAMESPACE ================================================================

namespace OpenMesh  {
namespace Decimater {


//== CLASS DEFINITION =========================================================


/** Multiple choice decimater framework
    \see BaseModT, \ref decimater_docu
*/
template < typename MeshT >
class McDecimaterT : virtual public BaseDecimaterT<MeshT> //virtual especially for the mixed decimater
{
public: //-------------------------------------------------------- public types

  typedef McDecimaterT< MeshT >         Self;
  typedef MeshT                         Mesh;
  typedef CollapseInfoT<MeshT>          CollapseInfo;
  typedef ModBaseT<MeshT>               Module;
  typedef std::vector< Module* >        ModuleList;
  typedef typename ModuleList::iterator ModuleListIterator;

public: //------------------------------------------------------ public methods

  /// Constructor
  McDecimaterT( Mesh& _mesh );

  /// Destructor
  ~McDecimaterT();

public:

  /** Decimate (perform _n_collapses collapses). Return number of
      performed collapses. If _n_collapses is not given reduce as
      much as possible */
  size_t decimate( size_t _n_collapses );

  /// Decimate to target complexity, returns number of collapses
  size_t decimate_to( size_t  _n_vertices )
  {
    return ( (_n_vertices < this->mesh().n_vertices()) ?
	     decimate( this->mesh().n_vertices() - _n_vertices ) : 0 );
  }

  /** Decimate to target complexity (vertices and faces).
   *  Stops when the number of vertices or the number of faces is reached.
   *  Returns number of performed collapses.
   */
  size_t decimate_to_faces( size_t  _n_vertices=0, size_t _n_faces=0 );

  /**
   * Decimate only with constraints, while _factor gives the
   * percentage of the constraints that should be used
   */
  size_t decimate_constraints_only(float _factor);

  size_t samples(){return randomSamples_;}
  void set_samples(const size_t _value){randomSamples_ = _value;}

private: //------------------------------------------------------- private data


  // reference to mesh
  Mesh&      mesh_;

  size_t randomSamples_;

};

//=============================================================================
} // END_NS_DECIMATER
} // END_NS_OPENMESH
//=============================================================================
#if defined(OM_INCLUDE_TEMPLATES) && !defined(OPENMESH_MULTIPLE_CHOICE_DECIMATER_DECIMATERT_CC)
#define OPENMESH_MULTIPLE_CHOICE_DECIMATER_TEMPLATES
#include "McDecimaterT.cc"
#endif
//=============================================================================
#endif // OPENMESH_MC_DECIMATER_DECIMATERT_HH defined
//=============================================================================

