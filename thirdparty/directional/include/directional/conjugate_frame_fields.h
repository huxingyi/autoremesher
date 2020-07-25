// This file is part of Directional, a library for directional field processing.
//
// Copyright (C) 2014 Olga Diamanti <olga.diam@gmail.com>, 2018 Amir Vaxman
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.


#ifndef DIRECTIONAL_CONJUGATE_FRAME_FIELDS_H
#define DIRECTIONAL_CONJUGATE_FRAME_FIELDS_H

#include <igl/igl_inline.h>
#include "ConjugateFFSolverData.h"
#include <Eigen/Core>
#include <vector>

namespace directional {
  //todo
  // TODO: isConstrained should become a list of indices for consistency with
  //       n_polyvector
  
  IGL_INLINE void conjugate_frame_fields(const Eigen::MatrixXd &V,
                                         const Eigen::MatrixXi &F,
                                         const Eigen::VectorXi &isConstrained,
                                         const Eigen::MatrixXd &initialSolution,
                                         Eigen::MatrixXd &output,
                                         int _maxIter = 50,
                                         const double _lambdaOrtho = .1,
                                         const double _lambdaInit = 10,
                                         const double _lambdaMultFactor = 1.01,
                                         bool _doHardConstraints = true);
  
  IGL_INLINE double conjugate_frame_fields(const ConjugateFFSolverData &csdata,
                                           const Eigen::VectorXi &isConstrained,
                                           const Eigen::MatrixXd &initialSolution,
                                           Eigen::MatrixXd &output,
                                           int _maxIter = 50,
                                           const double _lambdaOrtho = .1,
                                           const double _lambdaInit = 10,
                                           const double _lambdaMultFactor = 1.01,
                                           bool _doHardConstraints = true);
  
};

#include "conjugate_frame_fields.cpp"

#endif
