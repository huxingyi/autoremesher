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

#ifndef OPENNL_MKL_H
#define OPENNL_MKL_H

#include "nl_private.h"
#include "nl_matrix.h"

/**
 * \file geogram/NL/nl_mkl.h
 * \brief Internal OpenNL functions that interface MKL.
 */

/**
 * \brief Initializes the MKL extension
 * \details This dynamically loads the MKL
 *  library available in the system (if available) and
 *  retreives the symbols in there.
 * \retval NL_TRUE if MKL could be successfully
 *   dynamically loaded and all functions could be
 *   found in it.
 * \retval NL_FALSE otherwise.
 * \note For now, only implemented under Linux in 
 *  dynamic libraries mode, compiled with Intel C Compiler
 */
NLboolean nlInitExtension_MKL(void);

/**
 * \brief Tests whether the MKL extension is initialized.
 * \retval NL_TRUE if the extension is initialized
 * \retval NL_FALSE otherwise
 */
NLboolean nlExtensionIsInitialized_MKL(void);

/**
 * \brief The function pointer to the sparse-matrix times 
 *  vector function that uses the MKL. If the extension is
 *  not initialized, then the pointer is NULL. Works for
 *  matrix in the CRS format. Both general and symmetric
 *  storage are supported.
 */
extern NLMultMatrixVectorFunc NLMultMatrixVector_MKL;

/**
 * \brief Creates an optimized MKL matrix from a dynamic sparse matrix.
 * \details The matrix \p M should have stored rows. Symmetric storage
 *  is not supported yet.
 * \param[in] M the dynamic sparse matrix.
 * \return a pointer to the created matrix
 */
NLMatrix nlMKLMatrixNewFromSparseMatrix(NLSparseMatrix* M);

/**
 * \brief Creates an optimized MKL matrix from a CRS sparse matrix.
 * \details The matrix \p M should have stored rows. Symmetric storage
 *  is not supported yet.
 * \param[in] M the dynamic sparse matrix.
 * \return a pointer to the created matrix
 */
NLMatrix nlMKLMatrixNewFromCRSMatrix(NLCRSMatrix* M);

#endif
