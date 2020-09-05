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

#include "nl_mkl.h"
#include "nl_context.h"

/**
 * \file nl_mkl.c
 * \brief Weak-coupling adapter to call MKL from OpenNL.
 */

typedef unsigned int MKL_INT;

typedef void (*FUNPTR_mkl_cspblas_dcsrgemv)(
    const char *transa, const MKL_INT *m, const double *a,
    const MKL_INT *ia, const MKL_INT *ja, const double *x, double *y
);

typedef void (*FUNPTR_mkl_cspblas_dcsrsymv)(
    const char *transa, const MKL_INT *m, const double *a,
    const MKL_INT *ia, const MKL_INT *ja, const double *x, double *y
);

typedef enum {
    SPARSE_STATUS_SUCCESS           = 0,    
    SPARSE_STATUS_NOT_INITIALIZED   = 1,    
    SPARSE_STATUS_ALLOC_FAILED      = 2,    
    SPARSE_STATUS_INVALID_VALUE     = 3,    
    SPARSE_STATUS_EXECUTION_FAILED  = 4,    
    SPARSE_STATUS_INTERNAL_ERROR    = 5,    
    SPARSE_STATUS_NOT_SUPPORTED     = 6     
} sparse_status_t;

typedef enum {
    SPARSE_INDEX_BASE_ZERO  = 0,           
    SPARSE_INDEX_BASE_ONE   = 1            
} sparse_index_base_t;

typedef enum {
    SPARSE_OPERATION_NON_TRANSPOSE       = 10,
    SPARSE_OPERATION_TRANSPOSE           = 11,
    SPARSE_OPERATION_CONJUGATE_TRANSPOSE = 12
} sparse_operation_t;

typedef enum {
    SPARSE_MATRIX_TYPE_GENERAL            = 20,   
    SPARSE_MATRIX_TYPE_SYMMETRIC          = 21,   
    SPARSE_MATRIX_TYPE_HERMITIAN          = 22,   
    SPARSE_MATRIX_TYPE_TRIANGULAR         = 23,
    SPARSE_MATRIX_TYPE_DIAGONAL           = 24,   
    SPARSE_MATRIX_TYPE_BLOCK_TRIANGULAR   = 25,
    SPARSE_MATRIX_TYPE_BLOCK_DIAGONAL     = 26    
} sparse_matrix_type_t;

typedef enum {
    SPARSE_FILL_MODE_LOWER  = 40,
    SPARSE_FILL_MODE_UPPER  = 41 
} sparse_fill_mode_t;

typedef enum {
    SPARSE_DIAG_NON_UNIT    = 50,
    SPARSE_DIAG_UNIT        = 51 
} sparse_diag_type_t;


struct matrix_descr {
    sparse_matrix_type_t type; 
    sparse_fill_mode_t mode; 
    sparse_diag_type_t diag; 
};

typedef enum  {
    SPARSE_MEMORY_NONE          = 80, 
    SPARSE_MEMORY_AGGRESSIVE    = 81 
} sparse_memory_usage_t;

struct  sparse_matrix;
typedef struct sparse_matrix *sparse_matrix_t;


typedef sparse_status_t (*FUNPTR_mkl_sparse_d_create_csr)(
    sparse_matrix_t* A, sparse_index_base_t indexing, 
    MKL_INT rows, MKL_INT cols, MKL_INT* rows_start,
    MKL_INT* rows_end, MKL_INT* col_indx, double* values
);

typedef sparse_status_t (*FUNPTR_mkl_sparse_d_mv)(
    sparse_operation_t operation, double alpha,
    const sparse_matrix_t A, struct matrix_descr descr,
    const double* x, double beta, double* y
);

typedef sparse_status_t (*FUNPTR_mkl_sparse_destroy)(
    sparse_matrix_t A
);

typedef sparse_status_t (*FUNPTR_mkl_sparse_set_mv_hint)(
    sparse_matrix_t A, sparse_operation_t operation,  
    struct matrix_descr descr, MKL_INT expected_calls
);

typedef sparse_status_t (*FUNPTR_mkl_sparse_set_memory_hint)(
    sparse_matrix_t A, sparse_memory_usage_t  policy
);

typedef sparse_status_t (*FUNPTR_mkl_sparse_optimize)(
    sparse_matrix_t A
);

/**
 * \brief The structure that stores the handle to 
 *  the MKL shared object, the function pointers
 *  and the detected version.
 */
typedef struct {
    NLdll DLL_mkl_intel_lp64;
    NLdll DLL_mkl_intel_thread;
    NLdll DLL_mkl_core;
    NLdll DLL_iomp5;

    FUNPTR_mkl_cspblas_dcsrgemv mkl_cspblas_dcsrgemv;
    FUNPTR_mkl_cspblas_dcsrsymv mkl_cspblas_dcsrsymv;

    FUNPTR_mkl_sparse_d_create_csr mkl_sparse_d_create_csr;
    FUNPTR_mkl_sparse_d_mv mkl_sparse_d_mv;
    FUNPTR_mkl_sparse_destroy mkl_sparse_destroy;
    FUNPTR_mkl_sparse_set_mv_hint mkl_sparse_set_mv_hint;
    FUNPTR_mkl_sparse_set_memory_hint mkl_sparse_set_memory_hint;
    FUNPTR_mkl_sparse_optimize mkl_sparse_optimize;
} MKLContext;

/**
 * \brief Gets the MKL context.
 * \return a pointer to the MKL context
 */
static MKLContext* MKL() {
    static MKLContext context;
    static NLboolean init = NL_FALSE;
    if(!init) {
        init = NL_TRUE;
        memset(&context, 0, sizeof(context));
    }
    return &context;
}

NLboolean nlExtensionIsInitialized_MKL() {
    if(
	MKL()->DLL_iomp5 == NULL ||
	MKL()->DLL_mkl_core == NULL ||
	MKL()->DLL_mkl_intel_thread == NULL ||
	MKL()->DLL_mkl_intel_lp64 == NULL ||
	MKL()->mkl_cspblas_dcsrgemv == NULL ||
	MKL()->mkl_cspblas_dcsrsymv == NULL ||
	MKL()->mkl_sparse_d_create_csr == NULL ||
	MKL()->mkl_sparse_d_mv == NULL ||
	MKL()->mkl_sparse_destroy == NULL ||
	MKL()->mkl_sparse_set_mv_hint == NULL ||
	MKL()->mkl_sparse_set_memory_hint == NULL ||
	MKL()->mkl_sparse_optimize == NULL
    ) {
        return NL_FALSE;
    }
    return NL_TRUE;
}

/**
 * \brief Finds and initializes a function pointer to
 *  one of the functions in MKL.
 * \details Function pointers are stored into the 
 *  MKLContext returned by the function MKL().
 *  If a symbol is not found, returns NL_FALSE from the
 *  calling function.
 */
#define find_mkl_func(name)                                  \
    if(                                                      \
        (                                                    \
            MKL()->name =                                    \
            (FUNPTR_##name)nlFindFunction(                   \
		   MKL()->DLL_mkl_intel_lp64,#name           \
	    )					             \
        ) == NULL                                            \
    ) {                                                      \
        nlError("nlInitExtension_MKL","function not found"); \
        return NL_FALSE;                                     \
    }

static void nlTerminateExtension_MKL(void) {
    if(!nlExtensionIsInitialized_MKL()) {
	return;
    }
    nlCloseDLL(MKL()->DLL_mkl_intel_lp64);
    nlCloseDLL(MKL()->DLL_mkl_intel_thread);
    nlCloseDLL(MKL()->DLL_mkl_core);
    nlCloseDLL(MKL()->DLL_iomp5);
    memset(MKL(), 0, sizeof(MKLContext));
    
}

NLMultMatrixVectorFunc NLMultMatrixVector_MKL = NULL;

static void NLMultMatrixVector_MKL_impl(
    NLMatrix M_in, const double* x, double* y
) {
#ifdef GARGANTUA
    nl_arg_used(M_in);
    nl_arg_used(x);
    nl_arg_used(y);
    nl_assert_not_reached; /* Not implemented yet ! */
#else    
    NLCRSMatrix* M = (NLCRSMatrix*)(M_in);
    nl_debug_assert(M_in->type == NL_MATRIX_CRS);
    if(M->symmetric_storage) {
	MKL()->mkl_cspblas_dcsrsymv(
	    "N", /* No transpose */
	    &M->m,
	    M->val,
	    M->rowptr,
	    M->colind,
	    x,
	    y
	);
    } else {
	MKL()->mkl_cspblas_dcsrgemv(
	    "N", /* No transpose */
	    &M->m,
	    M->val,
	    M->rowptr,
	    M->colind,
	    x,
	    y
	);
    }
#endif    
}


#define INTEL_PREFIX "/opt/intel/"
#define LIB_DIR "lib/intel64/"
#define MKL_PREFIX  INTEL_PREFIX "mkl/" LIB_DIR

NLboolean nlInitExtension_MKL(void) {
    NLenum flags = NL_LINK_LAZY | NL_LINK_GLOBAL;
    if(nlCurrentContext == NULL || !nlCurrentContext->verbose) {
	flags |= NL_LINK_QUIET;
    }
    
    if(MKL()->DLL_mkl_intel_lp64 != NULL) {
        return nlExtensionIsInitialized_MKL();
    }
    
    MKL()->DLL_iomp5 = nlOpenDLL(
	INTEL_PREFIX LIB_DIR "libiomp5.so",
	flags
    );    
    MKL()->DLL_mkl_core = nlOpenDLL(
	MKL_PREFIX "libmkl_core.so",
	flags
    );    
    MKL()->DLL_mkl_intel_thread = nlOpenDLL(
	MKL_PREFIX "libmkl_intel_thread.so",
	flags
    );    
    MKL()->DLL_mkl_intel_lp64 = nlOpenDLL(
	MKL_PREFIX "libmkl_intel_lp64.so",
	flags
    );
    
    if(
	MKL()->DLL_iomp5 == NULL ||
	MKL()->DLL_mkl_core == NULL ||
	MKL()->DLL_mkl_intel_thread == NULL ||
	MKL()->DLL_mkl_intel_lp64 == NULL
    ) {
        return NL_FALSE;
    }

    find_mkl_func(mkl_cspblas_dcsrgemv);
    find_mkl_func(mkl_cspblas_dcsrsymv);
    
    find_mkl_func(mkl_sparse_d_create_csr);
    find_mkl_func(mkl_sparse_d_mv);
    find_mkl_func(mkl_sparse_destroy);
    find_mkl_func(mkl_sparse_set_mv_hint);
    find_mkl_func(mkl_sparse_set_memory_hint);
    find_mkl_func(mkl_sparse_optimize);

    
    if(nlExtensionIsInitialized_MKL()) {
	NLMultMatrixVector_MKL = NLMultMatrixVector_MKL_impl;
    }
    
    atexit(nlTerminateExtension_MKL);
    return NL_TRUE;
}

/*************************************************************************/

typedef struct {
    NLuint m;
    NLuint n;
    NLenum type;
    NLDestroyMatrixFunc destroy_func;
    NLMultMatrixVectorFunc mult_func;
    sparse_matrix_t A;
    NLdouble* val;    
    NLuint* rowptr;
    NLuint* colind;
} NLMKLMatrix;

static void nlMKLMatrixDestroy(NLMKLMatrix* M) {
    MKL()->mkl_sparse_destroy(M->A);
    M->A = NULL;
    NL_DELETE_ARRAY(M->val);
    NL_DELETE_ARRAY(M->rowptr);
    NL_DELETE_ARRAY(M->colind);    
    M->m = 0;
    M->n = 0;
}

static void nlMKLMatrixMult(
    NLMKLMatrix* M, const double* x, double* y
) {
    struct matrix_descr descr;
    descr.type = SPARSE_MATRIX_TYPE_GENERAL;
    MKL()->mkl_sparse_d_mv(
	SPARSE_OPERATION_NON_TRANSPOSE,
	1.0, M->A, descr,
	x, 0.0, y
    );
}

NLMatrix nlMKLMatrixNewFromSparseMatrix(NLSparseMatrix* M) {
    NLuint_big nnz = nlSparseMatrixNNZ(M);
    NLuint i,ij,k; 
    NLMKLMatrix* result = NL_NEW(NLMKLMatrix);
    struct matrix_descr descr;
    descr.type = SPARSE_MATRIX_TYPE_GENERAL;

    nl_assert(M->storage & NL_MATRIX_STORE_ROWS);
    nl_assert((M->storage & NL_MATRIX_STORE_SYMMETRIC) == 0);

    result->m = M->m;
    result->n = M->n;
    result->type = NL_MATRIX_OTHER;
    result->destroy_func = (NLDestroyMatrixFunc)nlMKLMatrixDestroy;
    result->mult_func = (NLMultMatrixVectorFunc)nlMKLMatrixMult;
    result->val = NL_NEW_ARRAY(double, nnz);
    result->rowptr = NL_NEW_ARRAY(NLuint, M->m+1);
    result->colind = NL_NEW_ARRAY(NLuint, nnz);
    
    /* Convert matrix to CRS format */
    k=0;
    for(i=0; i<M->m; ++i) {
        NLRowColumn* Ri = &(M->row[i]);
        result->rowptr[i] = k;
        for(ij=0; ij<Ri->size; ij++) {
            NLCoeff* c = &(Ri->coeff[ij]);
            result->val[k] = c->value;
            result->colind[k] = c->index;
            ++k;
        }
    }
    result->rowptr[M->m] = k;

    MKL()->mkl_sparse_d_create_csr(
	&(result->A), SPARSE_INDEX_BASE_ZERO, M->m, M->n,
	result->rowptr, result->rowptr+1,
	result->colind, result->val
    );

    MKL()->mkl_sparse_set_mv_hint(
	result->A, SPARSE_OPERATION_NON_TRANSPOSE, descr, 1000
    );

    MKL()->mkl_sparse_set_memory_hint(
	result->A, SPARSE_MEMORY_AGGRESSIVE
    );

    MKL()->mkl_sparse_optimize(result->A);
    
    return (NLMatrix)result;
}

NLMatrix nlMKLMatrixNewFromCRSMatrix(NLCRSMatrix* M) {
    NLuint_big nnz = nlCRSMatrixNNZ(M);
    NLMKLMatrix* result = NL_NEW(NLMKLMatrix);
    struct matrix_descr descr;
    descr.type = SPARSE_MATRIX_TYPE_GENERAL;

    nl_assert(!M->symmetric_storage);

    result->m = M->m;
    result->n = M->n;
    result->type = NL_MATRIX_OTHER;
    result->destroy_func = (NLDestroyMatrixFunc)nlMKLMatrixDestroy;
    result->mult_func = (NLMultMatrixVectorFunc)nlMKLMatrixMult;
    result->val = NL_NEW_ARRAY(double, nnz);
    result->rowptr = NL_NEW_ARRAY(NLuint, M->m+1);
    result->colind = NL_NEW_ARRAY(NLuint, nnz);

    memcpy(result->val, M->val, nnz*sizeof(double));
    memcpy(result->rowptr, M->rowptr, (M->m+1)*sizeof(NLuint));
    memcpy(result->colind, M->colind, nnz*sizeof(NLuint));

    MKL()->mkl_sparse_d_create_csr(
	&(result->A), SPARSE_INDEX_BASE_ZERO, M->m, M->n,
	result->rowptr, result->rowptr+1,
	result->colind, result->val
    );

    MKL()->mkl_sparse_set_mv_hint(
	result->A, SPARSE_OPERATION_NON_TRANSPOSE, descr, 1000
    );

    MKL()->mkl_sparse_set_memory_hint(
	result->A, SPARSE_MEMORY_AGGRESSIVE
    );

    MKL()->mkl_sparse_optimize(result->A);
    
    return (NLMatrix)result;
}
