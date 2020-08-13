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

/** \file JacobiLaplaceSmootherT.cc
    
 */

//=============================================================================
//
//  CLASS JacobiLaplaceSmootherT - IMPLEMENTATION
//
//=============================================================================

#define OPENMESH_JACOBI_LAPLACE_SMOOTHERT_C

//== INCLUDES =================================================================

#include <OpenMesh/Tools/Smoother/JacobiLaplaceSmootherT.hh>


//== NAMESPACES ===============================================================


namespace OpenMesh {
namespace Smoother {


//== IMPLEMENTATION ========================================================== 


template <class Mesh>
void
JacobiLaplaceSmootherT<Mesh>::
smooth(unsigned int _n)
{
  if (Base::continuity() > Base::C0)
  {
    Base::mesh_.add_property(umbrellas_);
    if (Base::continuity() > Base::C1)
      Base::mesh_.add_property(squared_umbrellas_);
  }

  LaplaceSmootherT<Mesh>::smooth(_n);

  if (Base::continuity() > Base::C0)
  {
    Base::mesh_.remove_property(umbrellas_);
    if (Base::continuity() > Base::C1)
      Base::mesh_.remove_property(squared_umbrellas_);
  }
}


//-----------------------------------------------------------------------------


template <class Mesh>
void
JacobiLaplaceSmootherT<Mesh>::
compute_new_positions_C0()
{
  typename Mesh::VertexIter  							v_it, v_end(Base::mesh_.vertices_end());
  typename Mesh::ConstVertexOHalfedgeIter voh_it;
  typename Mesh::Normal      							u, p, zero(0,0,0);
  typename Mesh::Scalar      							w;

  for (v_it=Base::mesh_.vertices_begin(); v_it!=v_end; ++v_it)
  {
    if (this->is_active(*v_it))
    {
      // compute umbrella
      u = zero;
      for (voh_it = Base::mesh_.cvoh_iter(*v_it); voh_it.is_valid(); ++voh_it) {
        w = this->weight(Base::mesh_.edge_handle(*voh_it));
        u += vector_cast<typename Mesh::Normal>(Base::mesh_.point(Base::mesh_.to_vertex_handle(*voh_it))) * w;
      }
      u *= this->weight(*v_it);
      u -= vector_cast<typename Mesh::Normal>(Base::mesh_.point(*v_it));

      // damping
      u *= 0.5;
    
      // store new position
      p  = vector_cast<typename Mesh::Normal>(Base::mesh_.point(*v_it));
      p += u;
      this->set_new_position(*v_it, p);
    }
  }
}


//-----------------------------------------------------------------------------


template <class Mesh>
void
JacobiLaplaceSmootherT<Mesh>::
compute_new_positions_C1()
{
  typename Mesh::VertexIter  							v_it, v_end(Base::mesh_.vertices_end());
  typename Mesh::ConstVertexOHalfedgeIter voh_it;
  typename Mesh::Normal      							u, uu, p, zero(0,0,0);
  typename Mesh::Scalar      							w, diag;


  // 1st pass: compute umbrellas
  for (v_it=Base::mesh_.vertices_begin(); v_it!=v_end; ++v_it)
  {
    u = zero;
    for (voh_it = Base::mesh_.cvoh_iter(*v_it); voh_it.is_valid(); ++voh_it) {
      w  = this->weight(Base::mesh_.edge_handle(*voh_it));
      u -= vector_cast<typename Mesh::Normal>(Base::mesh_.point(Base::mesh_.to_vertex_handle(*voh_it)))*w;
    }
    u *= this->weight(*v_it);
    u += vector_cast<typename Mesh::Normal>(Base::mesh_.point(*v_it));

    Base::mesh_.property(umbrellas_, *v_it) = u;
  }


  // 2nd pass: compute updates
  for (v_it=Base::mesh_.vertices_begin(); v_it!=v_end; ++v_it)
  {
    if (this->is_active(*v_it))
    {
      uu   = zero;
      diag = 0.0;   
      for (voh_it = Base::mesh_.cvoh_iter(*v_it); voh_it.is_valid(); ++voh_it) {
        w  = this->weight(Base::mesh_.edge_handle(*voh_it));
        uu   -= Base::mesh_.property(umbrellas_, Base::mesh_.to_vertex_handle(*voh_it));
        diag += (w * this->weight(Base::mesh_.to_vertex_handle(*voh_it)) + 1.0) * w;
      }
      uu   *= this->weight(*v_it);
      diag *= this->weight(*v_it);
      uu   += Base::mesh_.property(umbrellas_, *v_it);
      if (diag) uu *= 1.0/diag;

      // damping
      uu *= 0.25;
    
      // store new position
      p  = vector_cast<typename Mesh::Normal>(Base::mesh_.point(*v_it));
      p -= uu;
      this->set_new_position(*v_it, p);
    }
  }
}


//=============================================================================
} // namespace Smoother
} // namespace OpenMesh
//=============================================================================
