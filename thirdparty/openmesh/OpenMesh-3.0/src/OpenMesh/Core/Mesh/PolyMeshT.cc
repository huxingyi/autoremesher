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
//  CLASS PolyMeshT - IMPLEMENTATION
//
//=============================================================================


#define OPENMESH_POLYMESH_C


//== INCLUDES =================================================================

#include <OpenMesh/Core/Mesh/PolyMeshT.hh>
#include <OpenMesh/Core/Geometry/LoopSchemeMaskT.hh>
#include <OpenMesh/Core/Utils/vector_cast.hh>
#include <OpenMesh/Core/System/omstream.hh>
#include <vector>


//== NAMESPACES ===============================================================


namespace OpenMesh {

//== IMPLEMENTATION ==========================================================

template <class Kernel>
uint PolyMeshT<Kernel>::find_feature_edges(Scalar _angle_tresh)
{
  assert(Kernel::has_edge_status());//this function needs edge status property
  uint n_feature_edges = 0;
  for (EdgeIter e_it = Kernel::edges_begin(); e_it != Kernel::edges_end(); ++e_it)
  {
    if (fabs(calc_dihedral_angle(e_it)) > _angle_tresh)
    {//note: could be optimized by comparing cos(dih_angle) vs. cos(_angle_tresh)
      status(e_it).set_feature(true);
      n_feature_edges++;
    }
    else
    {
      status(e_it).set_feature(false);
    }
  }
  return n_feature_edges;
}

//-----------------------------------------------------------------------------

template <class Kernel>
typename PolyMeshT<Kernel>::Normal
PolyMeshT<Kernel>::
calc_face_normal(FaceHandle _fh) const
{
  assert(this->halfedge_handle(_fh).is_valid());
  ConstFaceVertexIter fv_it(this->cfv_iter(_fh));
  
  Point p0 = this->point(*fv_it);
  Point p0i = p0; //save point of vertex 0
  ++fv_it;
  Point p1 = this->point(*fv_it);
  Point p1i = p1; //save point of vertex 1
  ++fv_it;
  Point p2;
  
  //calculate area-weighted average normal of polygon's ears
  Normal n(0,0,0);
  for(; fv_it.is_valid(); ++fv_it)
  {
    p2 = this->point(*fv_it);
    n += vector_cast<Normal>(calc_face_normal(p0, p1, p2)); 
    p0 = p1;
    p1 = p2;
  }
  
  //two additional steps since we started at vertex 2, not 0
  n += vector_cast<Normal>(calc_face_normal(p0i, p0, p1)); 
  n += vector_cast<Normal>(calc_face_normal(p1i, p0i, p1));

  typename vector_traits<Normal>::value_type norm = n.length();
  
  // The expression ((n *= (1.0/norm)),n) is used because the OpenSG
  // vector class does not return self after component-wise
  // self-multiplication with a scalar!!!
  return (norm != typename vector_traits<Normal>::value_type(0)) ? ((n *= (typename vector_traits<Normal>::value_type(1)/norm)),n) : Normal(0,0,0);
}

//-----------------------------------------------------------------------------


template <class Kernel>
typename PolyMeshT<Kernel>::Normal
PolyMeshT<Kernel>::
calc_face_normal(const Point& _p0,
     const Point& _p1,
     const Point& _p2) const
{
#if 1
  // The OpenSG <Vector>::operator -= () does not support the type Point
  // as rhs. Therefore use vector_cast at this point!!!
  // Note! OpenSG distinguishes between Normal and Point!!!
  Normal p1p0(vector_cast<Normal>(_p0));  p1p0 -= vector_cast<Normal>(_p1);
  Normal p1p2(vector_cast<Normal>(_p2));  p1p2 -= vector_cast<Normal>(_p1);

  Normal n    = cross(p1p2, p1p0);
  typename vector_traits<Normal>::value_type norm = n.length();

  // The expression ((n *= (1.0/norm)),n) is used because the OpenSG
  // vector class does not return self after component-wise
  // self-multiplication with a scalar!!!
  return (norm != typename vector_traits<Normal>::value_type(0)) ? ((n *= (typename vector_traits<Normal>::value_type(1)/norm)),n) : Normal(0,0,0);
#else
  Point p1p0 = _p0;  p1p0 -= _p1;
  Point p1p2 = _p2;  p1p2 -= _p1;

  Normal n = vector_cast<Normal>(cross(p1p2, p1p0));
  typename vector_traits<Normal>::value_type norm = n.length();

  return (norm != 0.0) ? n *= (1.0/norm) : Normal(0,0,0);
#endif
}

//-----------------------------------------------------------------------------

template <class Kernel>
typename PolyMeshT<Kernel>::Point
PolyMeshT<Kernel>::
calc_face_centroid(FaceHandle _fh) const
{
  Point _pt;
  _pt.vectorize(0);
  Scalar valence = 0.0;
  for (ConstFaceVertexIter cfv_it = this->cfv_iter(_fh); cfv_it.is_valid(); ++cfv_it, valence += 1.0)
  {
    _pt += this->point(*cfv_it);
  }
  _pt /= valence;
  return _pt;
}
//-----------------------------------------------------------------------------


template <class Kernel>
void
PolyMeshT<Kernel>::
update_normals()
{
  // Face normals are required to compute the vertex and the halfedge normals
  if (Kernel::has_face_normals() ) {     
    update_face_normals();

    if (Kernel::has_vertex_normals() ) update_vertex_normals();
    if (Kernel::has_halfedge_normals()) update_halfedge_normals();
  }
}


//-----------------------------------------------------------------------------


template <class Kernel>
void
PolyMeshT<Kernel>::
update_face_normals()
{
  FaceIter f_it(Kernel::faces_begin()), f_end(Kernel::faces_end());

  for (; f_it != f_end; ++f_it)
    this->set_normal(*f_it, calc_face_normal(*f_it));
}


//-----------------------------------------------------------------------------


template <class Kernel>
void
PolyMeshT<Kernel>::
update_halfedge_normals(const double _feature_angle)
{
  HalfedgeIter h_it(Kernel::halfedges_begin()), h_end(Kernel::halfedges_end());

  for (; h_it != h_end; ++h_it)
    this->set_normal(*h_it, calc_halfedge_normal(*h_it, _feature_angle));
}


//-----------------------------------------------------------------------------


template <class Kernel>
typename PolyMeshT<Kernel>::Normal
PolyMeshT<Kernel>::
calc_halfedge_normal(HalfedgeHandle _heh, const double _feature_angle) const
{
  if(Kernel::is_boundary(_heh))
    return Normal(0,0,0);
  else
  {
    std::vector<FaceHandle> fhs; fhs.reserve(10);

    HalfedgeHandle heh = _heh;

    // collect CW face-handles
    do
    {
      fhs.push_back(Kernel::face_handle(heh));

      heh = Kernel::next_halfedge_handle(heh);
      heh = Kernel::opposite_halfedge_handle(heh);
    }
    while(heh != _heh && !Kernel::is_boundary(heh) && !is_estimated_feature_edge(heh, _feature_angle));

    // collect CCW face-handles
    if(heh != _heh && !is_estimated_feature_edge(_heh, _feature_angle))
    {
      heh = Kernel::opposite_halfedge_handle(_heh);

      if ( !Kernel::is_boundary(heh) ) {
        do
        {

          fhs.push_back(Kernel::face_handle(heh));

          heh = Kernel::prev_halfedge_handle(heh);
          heh = Kernel::opposite_halfedge_handle(heh);
        }
        while(!Kernel::is_boundary(heh) && !is_estimated_feature_edge(heh, _feature_angle));
      }
    }

    Normal n(0,0,0);
    for(unsigned int i=0; i<fhs.size(); ++i)
      n += Kernel::normal(fhs[i]);

    return n.normalize();
  }
}


//-----------------------------------------------------------------------------


template <class Kernel>
bool
PolyMeshT<Kernel>::
is_estimated_feature_edge(HalfedgeHandle _heh, const double _feature_angle) const
{
  EdgeHandle eh = Kernel::edge_handle(_heh);

  if(Kernel::has_edge_status())
  {
    if(Kernel::status(eh).feature())
      return true;
  }

  if(Kernel::is_boundary(eh))
    return false;

  // compute angle between faces
  FaceHandle fh0 = Kernel::face_handle(_heh);
  FaceHandle fh1 = Kernel::face_handle(Kernel::opposite_halfedge_handle(_heh));

  Normal fn0 = Kernel::normal(fh0);
  Normal fn1 = Kernel::normal(fh1);

  // dihedral angle above angle threshold
  return ( dot(fn0,fn1) < cos(_feature_angle) );
}


//-----------------------------------------------------------------------------


template <class Kernel>
typename PolyMeshT<Kernel>::Normal
PolyMeshT<Kernel>::
calc_vertex_normal(VertexHandle _vh) const
{
  Normal n;
  calc_vertex_normal_fast(_vh,n);

  Scalar norm = n.length();
  if (norm != 0.0) n *= (Scalar(1.0)/norm);

  return n;
}

//-----------------------------------------------------------------------------
template <class Kernel>
void PolyMeshT<Kernel>::
calc_vertex_normal_fast(VertexHandle _vh, Normal& _n) const
{
  _n.vectorize(0.0);
  for (ConstVertexFaceIter vf_it=this->cvf_iter(_vh); vf_it.is_valid(); ++vf_it)
    _n += this->normal(*vf_it);
}

//-----------------------------------------------------------------------------
template <class Kernel>
void PolyMeshT<Kernel>::
calc_vertex_normal_correct(VertexHandle _vh, Normal& _n) const
{
  _n.vectorize(0.0);
  ConstVertexIHalfedgeIter cvih_it = cvih_iter(_vh);
  if (!cvih_it)
  {//don't crash on isolated vertices
    return;
  }
  Normal in_he_vec;
  calc_edge_vector(cvih_it, in_he_vec);
  for ( ; cvih_it; ++cvih_it)
  {//calculates the sector normal defined by cvih_it and adds it to _n
    if (is_boundary(cvih_it))
    {
      continue;
    }
    HalfedgeHandle out_heh(next_halfedge_handle(cvih_it));
    Normal out_he_vec;
    calc_edge_vector(out_heh, out_he_vec);
    _n += cross(in_he_vec, out_he_vec);//sector area is taken into account
    in_he_vec = out_he_vec;
    in_he_vec *= -1;//change the orientation
  }
}

//-----------------------------------------------------------------------------
template <class Kernel>
void PolyMeshT<Kernel>::
calc_vertex_normal_loop(VertexHandle _vh, Normal& _n) const
{
  static const LoopSchemeMaskDouble& loop_scheme_mask__ =
                  LoopSchemeMaskDoubleSingleton::Instance();

  Normal t_v(0.0,0.0,0.0), t_w(0.0,0.0,0.0);
  unsigned int vh_val = valence(_vh);
  unsigned int i = 0;
  for (ConstVertexOHalfedgeIter cvoh_it = cvoh_iter(_vh); cvoh_it; ++cvoh_it, ++i)
  {
    VertexHandle r1_v(to_vertex_handle(cvoh_it));
    t_v += (typename vector_traits<Point>::value_type)(loop_scheme_mask__.tang0_weight(vh_val, i))*this->point(r1_v);
    t_w += (typename vector_traits<Point>::value_type)(loop_scheme_mask__.tang1_weight(vh_val, i))*this->point(r1_v);
  }
  _n = cross(t_w, t_v);//hack: should be cross(t_v, t_w), but then the normals are reversed?
}

//-----------------------------------------------------------------------------


template <class Kernel>
void
PolyMeshT<Kernel>::
update_vertex_normals()
{
  VertexIter  v_it(Kernel::vertices_begin()), v_end(Kernel::vertices_end());

  for (; v_it!=v_end; ++v_it)
    this->set_normal(*v_it, calc_vertex_normal(*v_it));
}

//=============================================================================
} // namespace OpenMesh
//=============================================================================
