// This file is part of Directional, a library for directional field processing.
// Copyright (C) 2017 Amir Vaxman <avaxman@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
#ifndef DIRECTIONAL_ROTATION_TO_RAW_H
#define DIRECTIONAL_ROTATION_TO_RAW_H

#include <igl/edge_topology.h>
#include <igl/per_face_normals.h>
#include <directional/rotation_to_representative.h>
#include <directional/representative_to_raw.h>

namespace directional
{
  // Converts the rotation angle representation + global rotation to raw format
  // Inputs::
  //  V:              #V X 3 vertex coordinates.
  //  F:              #F by 3 face vertex indices.
  //  EV:             #E x 2 edges 2 vertices indices.
  //  EF:             #E X 2 edges 2 faces indices.
  //  normals:        #F normals for each face.
  //  rotationAngles: #E angles that encode deviation from parallel transport EF(i,0)->EF(i,1)
  //  N:              The degree of the field.
  //  globalRotation: The angle between the vector on the first face and its basis in radians.
  // Outputs:
  //  raw: #F by 3*N matrix with all N explicit vectors of each directional.
  IGL_INLINE void adjustment_to_raw(const Eigen::MatrixXd& V,
                                    const Eigen::MatrixXi& F,
                                    const Eigen::MatrixXi& EV,
                                    const Eigen::MatrixXi& EF,
                                    const Eigen::MatrixXd& normals,
                                    const Eigen::MatrixXd& rotationAngles,
                                    int N,
                                    double globalRotation,
                                    Eigen::MatrixXd& raw)
  {
    Eigen::MatrixXd representative;
    rotation_to_representative(V, F, EV, EF, rotationAngles, N, globalRotation, representative);
    representative_to_raw(normals, representative, N, raw);
  }
  
  //version with only (V,F)
  IGL_INLINE void rotation_to_raw(const Eigen::MatrixXd& V,
                                  const Eigen::MatrixXi& F,
                                  const Eigen::MatrixXd& rotationAngles,
                                  int N,
                                  double globalRotation,
                                  Eigen::MatrixXd& raw)
  {
    Eigen::MatrixXi EV, x, EF;
    igl::edge_topology(V, F, EV, x, EF);
    Eigen::MatrixXd normals;
    igl::per_face_normals(V, F, normals);
    rotation_to_raw(V, F, EV, EF, norm, rotationAngles, N, globalRotation, raw);
  }
}

#endif
