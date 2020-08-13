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
//  CLASS StripifierT
//
//=============================================================================


#ifndef OPENMESH_STRIPIFIERT_HH
#define OPENMESH_STRIPIFIERT_HH


//== INCLUDES =================================================================

#include <vector>
#include <OpenMesh/Core/Utils/Property.hh>


//== FORWARDDECLARATIONS ======================================================


//== NAMESPACES ===============================================================

namespace OpenMesh {


//== CLASS DEFINITION =========================================================




/** \class StripifierT StripifierT.hh <OpenMesh/Tools/Utils/StripifierT.hh>
    This class decomposes a triangle mesh into several triangle strips.
*/

template <class Mesh>
class StripifierT
{
public:

  typedef unsigned int                      Index;
  typedef std::vector<Index>                Strip;
  typedef typename Strip::const_iterator    IndexIterator;
  typedef std::vector<Strip>                Strips;
  typedef typename Strips::const_iterator   StripsIterator;


  /// Default constructor
  StripifierT(Mesh& _mesh);

  /// Destructor
  ~StripifierT();

  /// Compute triangle strips, returns number of strips
  size_t stripify();

  /// delete all strips
  void clear() { Strips().swap(strips_); }

  /// returns number of strips
  size_t n_strips() const { return strips_.size(); }

  /// are strips computed?
  bool is_valid() const { return !strips_.empty(); }

  /// Access strips
  StripsIterator begin() const { return strips_.begin(); }
  /// Access strips
  StripsIterator end()   const { return strips_.end(); }


private:

  typedef std::vector<typename Mesh::FaceHandle>  FaceHandles;


  /// this method does the main work
  void build_strips();

  /// build a strip from a given halfedge (in both directions)
  void build_strip(typename Mesh::HalfedgeHandle _start_hh,
		   Strip& _strip,
		   FaceHandles& _faces);

  FPropHandleT<bool>::reference  processed(typename Mesh::FaceHandle _fh) {
    return mesh_.property(processed_, _fh);
  }
  FPropHandleT<bool>::reference  used(typename Mesh::FaceHandle _fh) {
    return mesh_.property(used_, _fh);
  }



private:

  Mesh&                mesh_;
  Strips               strips_;
  FPropHandleT<bool>   processed_, used_;
};


//=============================================================================
} // namespace OpenMesh
//=============================================================================
#if defined(OM_INCLUDE_TEMPLATES) && !defined(OPENMESH_STRIPIFIERT_C)
#define OPENMESH_STRIPIFIERT_TEMPLATES
#include "StripifierT.cc"
#endif
//=============================================================================
#endif // OPENMESH_STRIPIFIERT_HH defined
//=============================================================================
