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

/** \file MixedDecimaterT.cc
 */

//=============================================================================
//
//  CLASS MixedDecimaterT - IMPLEMENTATION
//
//=============================================================================

#ifndef OPENMESH_MIXED_DECIMATER_DECIMATERT_HH
#define OPENMESH_MIXED_DECIMATER_DECIMATERT_HH


//== INCLUDES =================================================================

#include <memory>
#include <OpenMesh/Tools/Decimater/McDecimaterT.hh>
#include <OpenMesh/Tools/Decimater/DecimaterT.hh>



//== NAMESPACE ================================================================

namespace OpenMesh  {
namespace Decimater {


//== CLASS DEFINITION =========================================================


/** Mixed decimater framework
    \see BaseModT, \ref decimater_docu
*/
template < typename MeshT >
class MixedDecimaterT : public McDecimaterT<MeshT>, public DecimaterT<MeshT>
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
  MixedDecimaterT( Mesh& _mesh );

  /// Destructor
  ~MixedDecimaterT();

public:

  /** Decimate (perform _n_collapses collapses). Return number of
      performed collapses. If _n_collapses is not given reduce as
      much as possible */
  size_t decimate( const size_t _n_collapses, const float _mc_factor );

  /// Decimate to target complexity, returns number of collapses
  size_t decimate_to( size_t  _n_vertices, const float _mc_factor )
  {
    return ( (_n_vertices < this->mesh().n_vertices()) ?
       decimate( this->mesh().n_vertices() - _n_vertices, _mc_factor ) : 0 );
  }

  /** Decimate to target complexity (vertices and faces).
   *  Stops when the number of vertices or the number of faces is reached.
   *  Returns number of performed collapses.
   */
  size_t decimate_to_faces( const size_t  _n_vertices=0, const size_t _n_faces=0 , const float _mc_factor = 0.8);

private: //------------------------------------------------------- private data

};

//=============================================================================
} // END_NS_DECIMATER
} // END_NS_OPENMESH
//=============================================================================
#if defined(OM_INCLUDE_TEMPLATES) && !defined(OPENMESH_MIXED_DECIMATER_DECIMATERT_CC)
#define OPENMESH_MIXED_DECIMATER_TEMPLATES
#include "MixedDecimaterT.cc"
#endif
//=============================================================================
#endif // OPENMESH_MIXED_DECIMATER_DECIMATERT_HH
//=============================================================================
