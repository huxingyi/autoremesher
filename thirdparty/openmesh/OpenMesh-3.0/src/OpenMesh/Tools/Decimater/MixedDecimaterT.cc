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
#define OPENMESH_MIXED_DECIMATER_DECIMATERT_CC

//== INCLUDES =================================================================

#include <OpenMesh/Tools/Decimater/MixedDecimaterT.hh>

#include <vector>
#if defined(OM_CC_MIPS)
#  include <float.h>
#else
#  include <cfloat>
#endif

//== NAMESPACE ===============================================================
namespace OpenMesh {
namespace Decimater {

//== IMPLEMENTATION ==========================================================

template<class Mesh>
MixedDecimaterT<Mesh>::MixedDecimaterT(Mesh& _mesh) :
  BaseDecimaterT<Mesh>(_mesh),McDecimaterT<Mesh>(_mesh), DecimaterT<Mesh>(_mesh) {

}

//-----------------------------------------------------------------------------

template<class Mesh>
MixedDecimaterT<Mesh>::~MixedDecimaterT() {

}

//-----------------------------------------------------------------------------
template<class Mesh>
size_t MixedDecimaterT<Mesh>::decimate(const size_t _n_collapses, const float _mc_factor) {

  if (_mc_factor > 1.0)
    return 0;

  size_t n_collapses_mc = static_cast<size_t>(_mc_factor*_n_collapses);
  size_t n_collapses_inc = static_cast<size_t>(_n_collapses - n_collapses_mc);

  size_t r_collapses = 0;
  if (_mc_factor > 0.0)
    r_collapses = McDecimaterT<Mesh>::decimate(n_collapses_mc);
  if (_mc_factor < 1.0)
    r_collapses += DecimaterT<Mesh>::decimate(n_collapses_inc);

  return r_collapses;

}

template<class Mesh>
size_t MixedDecimaterT<Mesh>::decimate_to_faces(const size_t  _n_vertices,const size_t _n_faces, const float _mc_factor ){

  if (_mc_factor > 1.0)
    return 0;

  std::size_t r_collapses = 0;
  if (_mc_factor > 0.0)
  {
    bool constraintsOnly = (_n_vertices == 0) && (_n_faces == 1);
    if (!constraintsOnly) {
      size_t mesh_faces = this->mesh().n_faces();
      size_t mesh_vertices = this->mesh().n_vertices();
      //reduce the mesh only for _mc_factor
      size_t n_vertices_mc = static_cast<size_t>(mesh_vertices - _mc_factor * (mesh_vertices - _n_vertices));
      size_t n_faces_mc = static_cast<size_t>(mesh_faces - _mc_factor * (mesh_faces - _n_faces));

      r_collapses = McDecimaterT<Mesh>::decimate_to_faces(n_vertices_mc, n_faces_mc);
    } else {

      const size_t samples = this->samples();

      // MinimalSample count for the McDecimater
      const size_t min = 2;

      // Maximal number of samples for the McDecimater
      const size_t max = samples;

      // Number of incremental steps
      const size_t steps = 7;

      for ( size_t i = 0; i < steps; ++i ) {

        // Compute number of samples to be used
        size_t samples = int (double( min) + double(i)/(double(steps)-1.0) * (max-2) ) ;

        // We won't allow 1 here, as this is the last step in the incremental part
        float decimaterLevel = (float(i + 1)) * _mc_factor / (float(steps) );

        this->set_samples(samples);
        r_collapses += McDecimaterT<Mesh>::decimate_constraints_only(decimaterLevel);
      }
    }
  }

  //Update the mesh::n_vertices function, otherwise the next Decimater function will delete too much
  this->mesh().garbage_collection();

  //reduce the rest of the mesh
  if (_mc_factor < 1.0) {
    r_collapses += DecimaterT<Mesh>::decimate_to_faces(_n_vertices,_n_faces);
  }


  return r_collapses;
}

//=============================================================================
}// END_NS_MC_DECIMATER
} // END_NS_OPENMESH
//=============================================================================
