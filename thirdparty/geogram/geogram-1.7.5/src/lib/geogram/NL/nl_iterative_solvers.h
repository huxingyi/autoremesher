/*
 *  Copyright (c) 2004-2010, Bruno Levy
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 *  * Neither the name of the ALICE Project-Team nor the names of its
 *  contributors may be used to endorse or promote products derived from this
 *  software without specific prior written permission.
 * 
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *  If you modify this software, you should include a notice giving the
 *  name of the person performing the modification, the date of modification,
 *  and the reason for such modification.
 *
 *  Contact: Bruno Levy
 *
 *     levy@loria.fr
 *
 *     ALICE Project
 *     LORIA, INRIA Lorraine, 
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX 
 *     FRANCE
 */


#ifndef OPENNL_ITERATIVE_SOLVERS_H
#define OPENNL_ITERATIVE_SOLVERS_H

#include "nl_private.h"
#include "nl_matrix.h"
#include "nl_blas.h"

/**
 * \file geogram/NL/nl_iterative_solvers.h
 * \brief Internal OpenNL functions that implement iterative solvers.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Solves a linear system using an iterative solver
 * \details The implementation of the solvers is inspired by 
 * the lsolver library, by Christian Badura, available from:
 * http://www.mathematik.uni-freiburg.de/IAM/Research/projectskr/lin_solver/
 *
 * About the Conjugate Gradient, details can be found in:
 *  Ashby, Manteuffel, Saylor
 *     A taxonomy for conjugate gradient methods
 *     SIAM J Numer Anal 27, 1542-1568 (1990)
 *
 * \param[in] blas opaque handle to BLAS library, can be either host blas or
 *  CUDA blas. If it is CUDA blas, then matrices should be CUDA matrices.
 * \param[in] M the matrix of the system
 * \param[in] P a preconditionner or NULL if not using a preconditioner
 * \param[in] b the right-hand side of the system
 * \param[out] x the solution of the system
 * \param[in] solver one of NL_CG, NL_BICGSTAB, NL_GMRES
 * \param[in] eps convergence bound, iterations are stopped as soon as
 *   \f$ \| Mx - b \| / \| b \| <  \mbox{eps}\f$
 * \param[in] max_iter maximum number of iterations
 * \param[in] inner_iter number of inner iterations, used by GMRES only
 */
NLAPI NLuint NLAPIENTRY nlSolveSystemIterative(
    NLBlas_t blas,
    NLMatrix M, NLMatrix P, NLdouble* b, NLdouble* x,
    NLenum solver,
    double eps, NLuint max_iter, NLuint inner_iter
);

#ifdef __cplusplus
}
#endif
    
#endif

