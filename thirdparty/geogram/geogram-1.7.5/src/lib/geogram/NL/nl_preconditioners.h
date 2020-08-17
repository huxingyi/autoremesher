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
 *
 */

#ifndef OPENNL_PRECONDITIONERS_H
#define OPENNL_PRECONDITIONERS_H

#include "nl_private.h"
#include "nl_matrix.h"

/**
 * \file geogram/NL/nl_preconditioners.h
 * \brief Internal OpenNL functions that implement preconditioners.
 */

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* preconditioners */

/**
 * \brief Creates a new Jacobi preconditioner
 * \param[in] M the matrix, needs to be of type NL_MATRIX_SPARSE_DYNAMIC
 * \details The inverse of the diagonal is stored in the preconditioner. 
 *  No reference to the input data is kept.
 * \return the Jacobi preconditioner
 */
NLAPI NLMatrix NLAPIENTRY nlNewJacobiPreconditioner(NLMatrix M);

/**
 * \brief Creates a new SSOR preconditioner
 * \param[in] M the matrix, needs to be of type NL_MATRIX_SPARSE_DYNAMIC 
 *  and needs to have both rows and columns storage, and symmetric storage.
 * \param[in] omega the relaxation parameter, within range [1.0,2.0].
 * \details A reference to the input matrix is kept and used in the 
 *  computations.
 * \return the SSOR preconditioner.
 */
NLAPI NLMatrix NLAPIENTRY nlNewSSORPreconditioner(NLMatrix M, double omega);

#ifdef __cplusplus
}
#endif
    
#endif
