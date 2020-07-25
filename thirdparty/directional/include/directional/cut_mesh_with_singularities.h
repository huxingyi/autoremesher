// This file is part of Directional, a library for directional field processing.
//
// Copyright (C) 2015 Olga Diamanti <olga.diam@gmail.com>, 2018 Amir vaxman
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.

#ifndef DIRECTIONAL_CUT_MESH_WITH_SINGULARITIES
#define DIRECTIONAL_CUT_MESH_WITH_SINGULARITIES

#include <igl/igl_inline.h>

#include <Eigen/Core>
#include <vector>

namespace directional {
  // Given a mesh and the singularities of a polyvector field, cut the mesh
  // to disk topology in such a way that the singularities lie at the boundary of
  // the disk, as described in the paper "Mixed Integer Quadrangulation" by
  // Bommes et al. 2009.
  // Inputs:
  //   V                #V by 3 list of the vertex positions
  //   F                #F by 3 list of the faces (must be triangles)
  //   VF               #V list of lists of incident faces (adjacency list), e.g.
  //                    as returned by igl::vertex_triangle_adjacency
  //   VV               #V list of lists of incident vertices (adjacency list), e.g.
  //                    as returned by igl::adjacency_list
  //   TT               #F by 3 triangle to triangle adjacent matrix (e.g. computed
  //                    via igl:triangle_triangle_adjacency)
  //   TTi              #F by 3 adjacent matrix, the element i,j is the id of edge of the
  //                    triangle TT(i,j) that is adjacent with triangle i (e.g. computed
  //                    via igl:triangle_triangle_adjacency)
  //   singularities    #S by 1 list of the indices of the singular vertices
  // Outputs:
  //   cuts             #F by 3 list of boolean flags, indicating the edges that need to be cut
  //                    (has 1 at the face edges that are to be cut, 0 otherwise)
  //
  IGL_INLINE void cut_mesh_with_singularities(const Eigen::MatrixXd &V,
                                              const Eigen::MatrixXi &F,
                                              const std::vector<std::vector<int> >& VF,
                                              const std::vector<std::vector<int> >& VV,
                                              const Eigen::MatrixXi& TT,
                                              const Eigen::MatrixXi& TTi,
                                              const Eigen::VectorXi &singularities,
                                              Eigen::MatrixXi &cuts);
  
  
  //Wrapper of the above with only vertices and faces as mesh input
  IGL_INLINE void cut_mesh_with_singularities(const Eigen::MatrixXd &V,
                                              const Eigen::MatrixXi &F,
                                              const Eigen::VectorXi &singularities,
                                              Eigen::MatrixXi &cuts);
  
};


#ifndef IGL_STATIC_LIBRARY
#include "cut_mesh_with_singularities.cpp"
#endif


#endif
