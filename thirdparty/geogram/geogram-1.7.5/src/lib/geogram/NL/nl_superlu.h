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

#ifndef OPENNL_SUPERLU_H
#define OPENNL_SUPERLU_H

#include "nl_private.h"
#include "nl_matrix.h"

/**
 * \file geogram/NL/nl_superlu.h
 * \brief Internal OpenNL functions that interfaces SuperLU.
 */

/**
 * \brief Factorizes a matrix using SuperLU.
 * \details  The SUPERLU extension needs to be initialized, 
 *   by first calling nlInitExtension("SUPERLU").
 * \param[in] M the input sparse matrix. Should be a
 *   either an NLSparseMatrix or an NLCRSMatrix.
 * \return a factorization P of \p M. Subsequent calls
 *   to nlMultMatrixVector(P,x,y) solves M y = x (P
 *   may be thought-of as M^-1)
 * \param[in] solver one of:
 *   - NL_SUPERLU_EXT if no pre-ordering should be used
 *   - NL_PERM_SUPERLU_EXT pre-ordering for general matrices
 *   - NL_SYMMETRIC_SUPERLU_EXT pre-ordering for symmetric matrices
 */  
NLAPI NLMatrix NLAPIENTRY nlMatrixFactorize_SUPERLU(
    NLMatrix M, NLenum solver
);

/**
 * \brief Initializes the SUPERLU extension
 * \details This dynamically loads the SuperLU 
 *  library available in the system (if available) and
 *  retrieves the symbols in there. It supports SuperLU 5.x.
 * \retval NL_TRUE if SUPERLU could be successfully
 *   dynamically loaded and all functions could be
 *   found in it.
 * \retval NL_FALSE otherwise.
 */
NLboolean nlInitExtension_SUPERLU(void);

/**
 * \brief Tests whether the SUPERLU extension is initialized.
 * \retval NL_TRUE if the extension is initialized
 * \retval NL_FALSE otherwise
 */
NLboolean nlExtensionIsInitialized_SUPERLU(void);

#endif
