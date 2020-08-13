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

/** \file DecimaterT.hh
 */

//=============================================================================
//
//  CLASS DecimaterT
//
//=============================================================================

#ifndef OPENMESH_DECIMATER_DECIMATERT_HH
#define OPENMESH_DECIMATER_DECIMATERT_HH


//== INCLUDES =================================================================

#include <memory>

#include <OpenMesh/Core/Utils/Property.hh>
#include <OpenMesh/Tools/Utils/HeapT.hh>
#include <OpenMesh/Tools/Decimater/BaseDecimaterT.hh>



//== NAMESPACE ================================================================

namespace OpenMesh  {
namespace Decimater {


//== CLASS DEFINITION =========================================================


/** Decimater framework.
    \see BaseModT, \ref decimater_docu
*/
template < typename MeshT >
class DecimaterT : virtual public BaseDecimaterT<MeshT> //virtual especially for the mixed decimater
{
public: //-------------------------------------------------------- public types

  typedef DecimaterT< MeshT >           Self;
  typedef MeshT                         Mesh;
  typedef CollapseInfoT<MeshT>          CollapseInfo;
  typedef ModBaseT<MeshT>               Module;
  typedef std::vector< Module* >        ModuleList;
  typedef typename ModuleList::iterator ModuleListIterator;

public: //------------------------------------------------------ public methods

  /// Constructor
  DecimaterT( Mesh& _mesh );

  /// Destructor
  ~DecimaterT();

public:

  /** Decimate (perform _n_collapses collapses). Return number of
      performed collapses. If _n_collapses is not given reduce as
      much as possible */
  size_t decimate( size_t _n_collapses = 0 );

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

public:

  typedef typename Mesh::VertexHandle    VertexHandle;
  typedef typename Mesh::HalfedgeHandle  HalfedgeHandle;

  /// Heap interface
  class HeapInterface
  {
  public:

    HeapInterface(Mesh&               _mesh,
      VPropHandleT<float> _prio,
      VPropHandleT<int>   _pos)
      : mesh_(_mesh), prio_(_prio), pos_(_pos)
    { }

    inline bool
    less( VertexHandle _vh0, VertexHandle _vh1 )
    { return mesh_.property(prio_, _vh0) < mesh_.property(prio_, _vh1); }

    inline bool
    greater( VertexHandle _vh0, VertexHandle _vh1 )
    { return mesh_.property(prio_, _vh0) > mesh_.property(prio_, _vh1); }

    inline int
    get_heap_position(VertexHandle _vh)
    { return mesh_.property(pos_, _vh); }

    inline void
    set_heap_position(VertexHandle _vh, int _pos)
    { mesh_.property(pos_, _vh) = _pos; }


  private:
    Mesh&                mesh_;
    VPropHandleT<float>  prio_;
    VPropHandleT<int>    pos_;
  };

  typedef Utils::HeapT<VertexHandle, HeapInterface>  DeciHeap;


private: //---------------------------------------------------- private methods

  /// Insert vertex in heap
  void heap_vertex(VertexHandle _vh);

private: //------------------------------------------------------- private data


  // reference to mesh
  Mesh&      mesh_;

  // heap
  std::auto_ptr<DeciHeap> heap_;

  // vertex properties
  VPropHandleT<HalfedgeHandle>  collapse_target_;
  VPropHandleT<float>           priority_;
  VPropHandleT<int>             heap_position_;

};

//=============================================================================
} // END_NS_DECIMATER
} // END_NS_OPENMESH
//=============================================================================
#if defined(OM_INCLUDE_TEMPLATES) && !defined(OPENMESH_DECIMATER_DECIMATERT_CC)
#define OPENMESH_DECIMATER_TEMPLATES
#include "DecimaterT.cc"
#endif
//=============================================================================
#endif // OPENMESH_DECIMATER_DECIMATERT_HH defined
//=============================================================================

