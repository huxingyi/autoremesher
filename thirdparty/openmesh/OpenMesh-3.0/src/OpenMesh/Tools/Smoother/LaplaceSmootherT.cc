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

/** \file LaplaceSmootherT.cc
    
 */

//=============================================================================
//
//  CLASS LaplaceSmootherT - IMPLEMENTATION
//
//=============================================================================

#define OPENMESH_LAPLACE_SMOOTHERT_C

//== INCLUDES =================================================================

#include <OpenMesh/Tools/Smoother/LaplaceSmootherT.hh>


//== NAMESPACES ===============================================================


namespace OpenMesh {
namespace Smoother {


//== IMPLEMENTATION ========================================================== 


template <class Mesh>
LaplaceSmootherT<Mesh>::
LaplaceSmootherT(Mesh& _mesh)
  : SmootherT<Mesh>(_mesh)
{
  // custom properties
  Base::mesh_.add_property(vertex_weights_);
  Base::mesh_.add_property(edge_weights_);
}


//-----------------------------------------------------------------------------


template <class Mesh>
LaplaceSmootherT<Mesh>::
~LaplaceSmootherT()
{
  // free custom properties
  Base::mesh_.remove_property(vertex_weights_);
  Base::mesh_.remove_property(edge_weights_);
}


//-----------------------------------------------------------------------------


template <class Mesh>
void
LaplaceSmootherT<Mesh>::
initialize(Component _comp, Continuity _cont)
{
  SmootherT<Mesh>::initialize(_comp, _cont);

  // calculate weights
  switch (_comp)
  {
    case Base::Tangential:
      compute_weights(UniformWeighting);
      break;


    case Base::Normal:
      compute_weights(CotWeighting);
      break;
      

    case Base::Tangential_and_Normal:
      compute_weights(UniformWeighting);
      break;
  }
}


//-----------------------------------------------------------------------------


template <class Mesh>
void
LaplaceSmootherT<Mesh>::
compute_weights(LaplaceWeighting _weighting)
{
  typename Mesh::VertexIter        v_it, v_end(Base::mesh_.vertices_end());
  typename Mesh::EdgeIter          e_it, e_end(Base::mesh_.edges_end());
  typename Mesh::HalfedgeHandle    heh0, heh1, heh2;
  typename Mesh::VertexHandle      v0, v1;
  const typename Mesh::Point       *p0, *p1, *p2;
  typename Mesh::Normal            d0, d1;
  typename Mesh::Scalar            weight, lb(-1.0), ub(1.0);



  // init vertex weights
  for (v_it=Base::mesh_.vertices_begin(); v_it!=v_end; ++v_it)
    Base::mesh_.property(vertex_weights_, *v_it) = 0.0;



  switch (_weighting)
  {
    // Uniform weighting
    case UniformWeighting:
    {
      for (e_it=Base::mesh_.edges_begin(); e_it!=e_end; ++e_it)
      {
        heh0   = Base::mesh_.halfedge_handle(*e_it, 0);
        heh1   = Base::mesh_.halfedge_handle(*e_it, 1);
        v0     = Base::mesh_.to_vertex_handle(heh0);
        v1     = Base::mesh_.to_vertex_handle(heh1);

        Base::mesh_.property(edge_weights_, *e_it)  = 1.0;
        Base::mesh_.property(vertex_weights_, v0)  += 1.0;
        Base::mesh_.property(vertex_weights_, v1)  += 1.0;
      }

      break;
    }


    // Cotangent weighting
    case CotWeighting:
    {
      for (e_it=Base::mesh_.edges_begin(); e_it!=e_end; ++e_it)
      {
        weight = 0.0;

        heh0   = Base::mesh_.halfedge_handle(*e_it, 0);
        v0     = Base::mesh_.to_vertex_handle(heh0);
        p0     = &Base::mesh_.point(v0);

        heh1   = Base::mesh_.halfedge_handle(*e_it, 1);
        v1     = Base::mesh_.to_vertex_handle(heh1);
        p1     = &Base::mesh_.point(v1);

        heh2   = Base::mesh_.next_halfedge_handle(heh0);
        p2     = &Base::mesh_.point(Base::mesh_.to_vertex_handle(heh2));
        d0     = (*p0 - *p2); d0.normalize();
        d1     = (*p1 - *p2); d1.normalize();
        weight += 1.0 / tan(acos(std::max(lb, std::min(ub, dot(d0,d1) ))));

        heh2   = Base::mesh_.next_halfedge_handle(heh1);
        p2     = &Base::mesh_.point(Base::mesh_.to_vertex_handle(heh2));
        d0     = (*p0 - *p2); d0.normalize();
        d1     = (*p1 - *p2); d1.normalize();
        weight += 1.0 / tan(acos(std::max(lb, std::min(ub, dot(d0,d1) ))));

        Base::mesh_.property(edge_weights_, *e_it) = weight;
        Base::mesh_.property(vertex_weights_, v0)  += weight;
        Base::mesh_.property(vertex_weights_, v1)  += weight;
      }
      break;
    }
  }

  
  // invert vertex weights:
  // before: sum of edge weights
  // after: one over sum of edge weights
  for (v_it=Base::mesh_.vertices_begin(); v_it!=v_end; ++v_it)
  {
    weight = Base::mesh_.property(vertex_weights_, *v_it);
    if (weight)
      Base::mesh_.property(vertex_weights_, *v_it) = 1.0 / weight;
  }
}



//=============================================================================
} // namespace Smoother
} // namespace OpenMesh
//=============================================================================
