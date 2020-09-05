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

#include "nl_preconditioners.h"
#include "nl_blas.h"
#include "nl_matrix.h"
#include "nl_context.h"

/******************************************************************************/

typedef struct {
    /**
     * \brief number of rows 
     */    
    NLuint m;

    /**
     * \brief number of columns 
     */    
    NLuint n;

    /**
     * \brief Matrix type (=NL_MATRIX_OTHER)
     */
    NLenum type;

    /**
     * \brief Destructor
     */
    NLDestroyMatrixFunc destroy_func;

    /**
     * \brief Matrix x vector product
     */
    NLMultMatrixVectorFunc mult_func;
    
    /**
     * \brief The inverse of the diagonal
     */
    NLdouble* diag_inv;
    
} NLJacobiPreconditioner;


static void nlJacobiPreconditionerDestroy(NLJacobiPreconditioner* M) {
    NL_DELETE_ARRAY(M->diag_inv);
}

static void nlJacobiPreconditionerMult(
    NLJacobiPreconditioner* M, const double* x, double* y
) {
    NLuint i;
    for(i=0; i<M->n; ++i) {
	y[i] = x[i] * M->diag_inv[i];
    }
    nlHostBlas()->flops += (NLulong)(M->n);    
}

NLMatrix nlNewJacobiPreconditioner(NLMatrix M) {
    NLSparseMatrix* M_dyn = NULL;
    NLCRSMatrix* M_CRS = NULL;
    NLJacobiPreconditioner* result = NULL;
    NLuint i;
    NLuint_big jj;
    nl_assert(
	M->type == NL_MATRIX_SPARSE_DYNAMIC ||
	M->type == NL_MATRIX_CRS
    );
    nl_assert(M->m == M->n);
    result = NL_NEW(NLJacobiPreconditioner);    
    result->m = M->m;
    result->n = M->n;
    result->type = NL_MATRIX_OTHER;
    result->destroy_func = (NLDestroyMatrixFunc)nlJacobiPreconditionerDestroy;
    result->mult_func = (NLMultMatrixVectorFunc)nlJacobiPreconditionerMult;
    result->diag_inv = NL_NEW_ARRAY(double, M->n);
    if(M->type == NL_MATRIX_SPARSE_DYNAMIC) {
	M_dyn = (NLSparseMatrix*)M;
	for(i=0; i<M_dyn->n; ++i) {
	    result->diag_inv[i] =
		(M_dyn->diag[i] == 0.0) ? 1.0 : 1.0/M_dyn->diag[i];
	}
    } else if(M->type == NL_MATRIX_CRS) {
	M_CRS = (NLCRSMatrix*)M;	
	for(i=0; i<M_CRS->n; ++i) {
	    result->diag_inv[i] = 1.0;
	    for(jj=M_CRS->rowptr[i]; jj<M_CRS->rowptr[i+1]; ++jj) {
		if(M_CRS->colind[jj] == i) {
		    result->diag_inv[i] = 1.0 / M_CRS->val[jj];
		}
	    }
	}
    }
    return (NLMatrix)result;
}

/**************************************************************/


typedef struct {
    /**
     * \brief number of rows 
     */    
    NLuint m;

    /**
     * \brief number of columns 
     */    
    NLuint n;

    /**
     * \brief Matrix type (=NL_MATRIX_OTHER)
     */
    NLenum type;

    /**
     * \brief Destructor
     */
    NLDestroyMatrixFunc destroy_func;

    /**
     * \brief Matrix x vector product
     */
    NLMultMatrixVectorFunc mult_func;

    /**
     * \brief A pointer to the initial matrix
     */
    NLSparseMatrix* M;

    /**
     * \brief The relaxation parameter
     */
    double omega;
    
    /**
     * \brief Workspace for implementing matrix x vector
     *  product.
     */
    NLdouble* work;
    
} NLSSORPreconditioner;


static void nlSSORPreconditionerDestroy(NLSSORPreconditioner* M) {
    NL_DELETE_ARRAY(M->work);
}


/**
 * \brief Multiplies a vector by the inverse of the
 *  lower triangular block of a sparse matrix.
 * \details \$ x \leftarrow \omega \mbox{trilow}(M)^{-1} x \$,
 *   used to implement the SSOR preconditioner.
 * \param[in] x the vector to be multiplied,
 *  size = A->n
 * \param[out] y where to store the result,
 *  size = A->n
 * \param[in] omega all components are multiplied by omega
 */

static void nlSparseMatrixMultLowerInverse(
    NLSparseMatrix* A, const NLdouble* x, NLdouble* y, double omega
) {
    NLuint n       = A->n;
    NLdouble* diag = A->diag;
    NLuint i;
    NLuint ij;
    NLCoeff* c = NULL;
    NLdouble S;

    nl_assert(A->storage & NL_MATRIX_STORE_SYMMETRIC);
    nl_assert(A->storage & NL_MATRIX_STORE_ROWS);

    for(i=0; i<n; i++) {
        NLRowColumn*  Ri = &(A->row[i]);       
        S = 0;
        for(ij=0; ij < Ri->size; ij++) {
            c = &(Ri->coeff[ij]);
            nl_parano_assert(c->index <= i); 
            if(c->index != i) {
                S += c->value * y[c->index]; 
            }
        }
        nlHostBlas()->flops += (NLulong)(2*Ri->size);                    
        y[i] = (x[i] - S) * omega / diag[i];
    }
    nlHostBlas()->flops += (NLulong)(n*3);                
}
/**
 * \brief Multiplies a vector by the inverse of the
 *  upper triangular block of a sparse matrix.
 * \details \$ x \leftarrow \omega \mbox{triup}(M)^{-1} x \$,
 *   used to implement the SSOR preconditioner.
 * \param[in] x the vector to be multiplied,
 *  size = A->n
 * \param[out] y where to store the result,
 *  size = A->n
 * \param[in] omega all components are multiplied by omega
 */
static void nlSparseMatrixMultUpperInverse(
    NLSparseMatrix* A, const NLdouble* x, NLdouble* y, NLdouble omega
) {
    NLuint n       = A->n;
    NLdouble* diag = A->diag;
    NLint i;
    NLuint ij;
    NLCoeff* c = NULL;
    NLdouble S;

    nl_assert(A->storage & NL_MATRIX_STORE_SYMMETRIC);
    nl_assert(A->storage & NL_MATRIX_STORE_COLUMNS);

    for(i=(NLint)(n-1); i>=0; i--) {
        NLRowColumn*  Ci = &(A->column[i]);       
        S = 0;
        for(ij=0; ij < Ci->size; ij++) {
            c = &(Ci->coeff[ij]);
            nl_parano_assert(c->index >= i); 
            if((NLint)(c->index) != i) {
                S += c->value * y[c->index]; 
            }
        }
        nlHostBlas()->flops += (NLulong)(2*Ci->size);                    
        y[i] = (x[i] - S) * omega / diag[i];
    }
    nlHostBlas()->flops += (NLulong)(n*3);                
}


static void nlSSORPreconditionerMult(
    NLSSORPreconditioner* P, const double* x, double* y
) {
    NLdouble* diag = P->M->diag;
    NLuint i;
    nlSparseMatrixMultLowerInverse(
        P->M, x, P->work, P->omega
    );
    for(i=0; i<P->n; i++) {
        P->work[i] *= (diag[i] / P->omega);
    }
    nlHostBlas()->flops += (NLulong)(P->n);
    nlSparseMatrixMultUpperInverse(
        P->M, P->work, y, P->omega
    );
    nlHostBlas()->Dscal(nlHostBlas(),(NLint)P->n, 2.0 - P->omega, y, 1);
}

NLMatrix nlNewSSORPreconditioner(NLMatrix M_in, double omega) {
    NLSparseMatrix* M = NULL;
    NLSSORPreconditioner* result = NULL;
    nl_assert(M_in->type == NL_MATRIX_SPARSE_DYNAMIC);
    nl_assert(M_in->m == M_in->n);
    M = (NLSparseMatrix*)M_in;
    result = NL_NEW(NLSSORPreconditioner);
    result->m = M->m;
    result->n = M->n;
    result->type = NL_MATRIX_OTHER;
    result->destroy_func = (NLDestroyMatrixFunc)nlSSORPreconditionerDestroy;
    result->mult_func = (NLMultMatrixVectorFunc)nlSSORPreconditionerMult;
    result->M = M;
    result->work = NL_NEW_ARRAY(NLdouble, result->n);
    result->omega = omega;
    return (NLMatrix)result;
}


