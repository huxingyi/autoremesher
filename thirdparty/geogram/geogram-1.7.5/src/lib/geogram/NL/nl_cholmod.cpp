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

#include "nl_cholmod.h"
#include "nl_context.h"
#include "nl_mkl.h"

/**
 * \file nl_cholmod.c
 * \brief Weak-coupling adapter to call CHOLMOD from OpenNL.
 */

#ifdef NL_OS_UNIX
#  ifdef NL_OS_APPLE
#      define CHOLMOD_LIB_NAME "libcholmod.dylib"
#  else
#      define CHOLMOD_LIB_NAME "libcholmod.so"
#  endif
#else
#  define CHOLMOD_LIB_NAME "libcholmod.xxx"
#endif

/******************************************************************/
/*            Excerpt from cholmod_core.h                         */
/******************************************************************/

/* A dense matrix in column-oriented form.  It has no itype since it contains
 * no integers.  Entry in row i and column j is located in x [i+j*d].
 */
typedef struct cholmod_dense_struct {
    size_t nrow ;       /* the matrix is nrow-by-ncol */
    size_t ncol ;
    size_t nzmax ;      /* maximum number of entries in the matrix */
    size_t d ;          /* leading dimension (d >= nrow must hold) */
    void *x ;           /* size nzmax or 2*nzmax, if present */
    void *z ;           /* size nzmax, if present */
    int xtype ;         /* pattern, real, complex, or zomplex */
    int dtype ;         /* x and z double or float */
} cholmod_dense ;

/* A sparse matrix stored in compressed-column form. */

typedef struct cholmod_sparse_struct
{
    size_t nrow ;       /* the matrix is nrow-by-ncol */
    size_t ncol ;
    size_t nzmax ;      /* maximum number of entries in the matrix */

    /* pointers to int or SuiteSparse_long: */
    void *p ;           /* p [0..ncol], the column pointers */
    void *i ;           /* i [0..nzmax-1], the row indices */

    /* for unpacked matrices only: */
    void *nz ;          /* nz [0..ncol-1], the # of nonzeros in each col.  In
                         * packed form, the nonzero pattern of column j is in
        * A->i [A->p [j] ... A->p [j+1]-1].  In unpacked form, column j is in
        * A->i [A->p [j] ... A->p [j]+A->nz[j]-1] instead.  In both cases, the
        * numerical values (if present) are in the corresponding locations in
        * the array x (or z if A->xtype is CHOLMOD_ZOMPLEX). */

    /* pointers to double or float: */
    void *x ;           /* size nzmax or 2*nzmax, if present */
    void *z ;           /* size nzmax, if present */

    int stype ;         /* Describes what parts of the matrix are considered:
                         *
        * 0:  matrix is "unsymmetric": use both upper and lower triangular parts
        *     (the matrix may actually be symmetric in pattern and value, but
        *     both parts are explicitly stored and used).  May be square or
        *     rectangular.
        * >0: matrix is square and symmetric, use upper triangular part.
        *     Entries in the lower triangular part are ignored.
        * <0: matrix is square and symmetric, use lower triangular part.
        *     Entries in the upper triangular part are ignored.
        *
        * Note that stype>0 and stype<0 are different for cholmod_sparse and
        * cholmod_triplet.  See the cholmod_triplet data structure for more
        * details.
        */

    int itype ;         /* CHOLMOD_INT:     p, i, and nz are int.
                         * CHOLMOD_INTLONG: p is SuiteSparse_long,
                         *                  i and nz are int.
                         * CHOLMOD_LONG:    p, i, and nz are SuiteSparse_long */

    int xtype ;         /* pattern, real, complex, or zomplex */
    int dtype ;         /* x and z are double or float */
    int sorted ;        /* TRUE if columns are sorted, FALSE otherwise */
    int packed ;        /* TRUE if packed (nz ignored), FALSE if unpacked
                         * (nz is required) */

} cholmod_sparse ;



typedef void* cholmod_common_ptr;
typedef cholmod_dense* cholmod_dense_ptr;
typedef cholmod_sparse* cholmod_sparse_ptr;
typedef void* cholmod_factor_ptr;

/** \brief defines the kind of numerical values used. */
typedef enum cholmod_xtype_enum {
    CHOLMOD_PATTERN =0,
    CHOLMOD_REAL    =1,
    CHOLMOD_COMPLEX =2,
    CHOLMOD_ZOMPLEX =3
} cholmod_xtype;

/** \brief defines the linear system to be solved. */
typedef enum cholmod_solve_type_enum {
   CHOLMOD_A    =0,   
   CHOLMOD_LDLt =1,   
   CHOLMOD_LD   =2,   
   CHOLMOD_DLt  =3,   
   CHOLMOD_L    =4,   
   CHOLMOD_Lt   =5,   
   CHOLMOD_D    =6,   
   CHOLMOD_P    =7,   
   CHOLMOD_Pt   =8   
} cholmod_solve_type;
    
/**
 * \brief 0 if all the coefficients are stored, -1 for lower triangular, 
 *   +1 for upper triangular.
 */
typedef int cholmod_stype;

typedef void (*FUNPTR_cholmod_start)(cholmod_common_ptr);

typedef cholmod_sparse_ptr (*FUNPTR_cholmod_allocate_sparse)(
    size_t m, size_t n, size_t nnz, int sorted,
    int packed, int stype, int xtype, cholmod_common_ptr
);

typedef cholmod_dense_ptr (*FUNPTR_cholmod_allocate_dense)(
    size_t m, size_t n, size_t d, int xtype, cholmod_common_ptr
);

typedef cholmod_factor_ptr (*FUNPTR_cholmod_analyze)(
    cholmod_sparse_ptr A, cholmod_common_ptr
);

typedef int (*FUNPTR_cholmod_factorize)(
    cholmod_sparse_ptr A, cholmod_factor_ptr L, cholmod_common_ptr
);

typedef cholmod_dense_ptr (*FUNPTR_cholmod_solve)(
    int solve_type, cholmod_factor_ptr, cholmod_dense_ptr, cholmod_common_ptr
);

typedef void (*FUNPTR_cholmod_free_factor)(
    cholmod_factor_ptr*, cholmod_common_ptr
);

typedef void (*FUNPTR_cholmod_free_dense)(
    cholmod_dense_ptr*, cholmod_common_ptr
);

typedef void (*FUNPTR_cholmod_free_sparse)(
    cholmod_sparse_ptr*, cholmod_common_ptr
);

typedef void (*FUNPTR_cholmod_finish)(cholmod_common_ptr);

/**
 * \brief The structure that stores the handle to 
 *  the CHOLMOD shared object, the function pointers
 *  and the detected version.
 */
typedef struct {
    /**
     * \brief Space for CHOLMOD's common variables.
     * \details On my version, sizeof(cholmod_common_struct)
     *  returns 2664. I give it (much) extra space in case
     *  future versions of CHOLMOD use larger space.
     */
    char cholmod_common[16384];

    FUNPTR_cholmod_start cholmod_start;
    FUNPTR_cholmod_allocate_sparse cholmod_allocate_sparse;
    FUNPTR_cholmod_allocate_dense cholmod_allocate_dense;
    FUNPTR_cholmod_analyze cholmod_analyze;
    FUNPTR_cholmod_factorize cholmod_factorize;
    FUNPTR_cholmod_solve cholmod_solve;
    FUNPTR_cholmod_free_factor cholmod_free_factor;
    FUNPTR_cholmod_free_sparse cholmod_free_sparse;        
    FUNPTR_cholmod_free_dense cholmod_free_dense;
    FUNPTR_cholmod_finish cholmod_finish;
    
    NLdll DLL_handle;
} CHOLMODContext;

/**
 * \brief Gets the CHOLMOD context.
 * \return a pointer to the CHOLMOD context
 */
static CHOLMODContext* CHOLMOD() {
    static CHOLMODContext context;
    static NLboolean init = NL_FALSE;
    if(!init) {
        init = NL_TRUE;
        memset(&context, 0, sizeof(context));
    }
    return &context;
}

NLboolean nlExtensionIsInitialized_CHOLMOD() {
    return
        CHOLMOD()->DLL_handle != NULL &&
        CHOLMOD()->cholmod_start != NULL &&
        CHOLMOD()->cholmod_allocate_sparse != NULL &&
        CHOLMOD()->cholmod_allocate_dense != NULL &&
        CHOLMOD()->cholmod_analyze != NULL &&
        CHOLMOD()->cholmod_factorize != NULL &&
        CHOLMOD()->cholmod_solve != NULL &&
        CHOLMOD()->cholmod_free_factor != NULL &&
        CHOLMOD()->cholmod_free_sparse != NULL &&
        CHOLMOD()->cholmod_free_dense != NULL &&
        CHOLMOD()->cholmod_finish != NULL ;
}

/**
 * \brief Finds and initializes a function pointer to
 *  one of the functions in CHOLMOD.
 * \details Function pointers are stored into the 
 *  CHOLMODContext returned by the function CHOLMOD().
 *  If a symbol is not found, returns NL_FALSE from the
 *  calling function.
 */
#define find_cholmod_func(name)                                        \
    if(                                                                \
        (                                                              \
            CHOLMOD()->name =                                          \
            (FUNPTR_##name)nlFindFunction(CHOLMOD()->DLL_handle,#name) \
        ) == NULL                                                      \
    ) {                                                                \
        nlError("nlInitExtension_CHOLMOD","function not found");       \
        return NL_FALSE;                                               \
    }


static void nlTerminateExtension_CHOLMOD(void) {
    if(CHOLMOD()->DLL_handle != NULL) {
        CHOLMOD()->cholmod_finish(&CHOLMOD()->cholmod_common);
        nlCloseDLL(CHOLMOD()->DLL_handle);
        CHOLMOD()->DLL_handle = NULL;
	memset(CHOLMOD(), 0, sizeof(CHOLMODContext));
    }
}

NLboolean nlInitExtension_CHOLMOD(void) {
    NLenum flags = NL_LINK_NOW | NL_LINK_USE_FALLBACK;
    if(nlCurrentContext == NULL || !nlCurrentContext->verbose) {
	flags |= NL_LINK_QUIET;
    }
    
    if(CHOLMOD()->DLL_handle != NULL) {
        return nlExtensionIsInitialized_CHOLMOD();
    }

    /*
     *   MKL has a built-in CHOLMOD that conflicts with
     * the CHOLMOD used by OpenNL (to be fixed). For now
     * we simply output a warning message and deactivate the
     * CHOLMOD extension if the MKL extension was initialized
     * before.
     */
    if(NLMultMatrixVector_MKL != NULL) {
	nl_fprintf(
	    stderr,
	    "CHOLMOD extension incompatible with MKL (deactivating)"
	);
	return NL_FALSE;
    }

    
    CHOLMOD()->DLL_handle = nlOpenDLL(CHOLMOD_LIB_NAME,flags);
    if(CHOLMOD()->DLL_handle == NULL) {
        return NL_FALSE;
    }

    find_cholmod_func(cholmod_start);
    find_cholmod_func(cholmod_allocate_sparse);
    find_cholmod_func(cholmod_allocate_dense);
    find_cholmod_func(cholmod_analyze);
    find_cholmod_func(cholmod_factorize);
    find_cholmod_func(cholmod_solve);
    find_cholmod_func(cholmod_free_factor);
    find_cholmod_func(cholmod_free_sparse);
    find_cholmod_func(cholmod_free_dense);
    find_cholmod_func(cholmod_finish);

    CHOLMOD()->cholmod_start(&CHOLMOD()->cholmod_common);

    atexit(nlTerminateExtension_CHOLMOD);
    return NL_TRUE;
}

/*************************************************************************/

/**
 * \brief The class for matrices factorized with Cholmod.
 */
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
     * \brief Matrix type
     * \details Set to NL_MATRIX_OTHER
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
     * \brief The Cholesky factor computer by Cholmod
     */
    cholmod_factor_ptr L;
    
} NLCholmodFactorizedMatrix;

static void nlCholmodFactorizedMatrixDestroy(NLCholmodFactorizedMatrix* M) {
    if(nlExtensionIsInitialized_CHOLMOD()) {
	CHOLMOD()->cholmod_free_factor(&M->L, &CHOLMOD()->cholmod_common);
    }
}

static void nlCholmodFactorizedMatrixMult(
    NLCholmodFactorizedMatrix* M, const double* x, double* y
) {
    /* 
     * TODO: see whether CHOLDMOD can use user-allocated vectors
     * (and avoid copy)
     */
    cholmod_dense_ptr X=CHOLMOD()->cholmod_allocate_dense(
        M->n, 1, M->n, CHOLMOD_REAL, &CHOLMOD()->cholmod_common
    );
    cholmod_dense_ptr Y=NULL;

    memcpy(X->x, x, M->n*sizeof(double));    
    Y = CHOLMOD()->cholmod_solve(
	CHOLMOD_A, M->L, X, &CHOLMOD()->cholmod_common
    );
    memcpy(y, Y->x, M->n*sizeof(double));    
    
    CHOLMOD()->cholmod_free_dense(&X, &CHOLMOD()->cholmod_common);
    CHOLMOD()->cholmod_free_dense(&Y, &CHOLMOD()->cholmod_common);
}

NLMatrix nlMatrixFactorize_CHOLMOD(
    NLMatrix M, NLenum solver
) {
    NLCholmodFactorizedMatrix* LLt = NULL;
    NLCRSMatrix* CRS = NULL;
    cholmod_sparse_ptr cM= NULL;
    NLuint nnz, cur, i, j;
    NLuint_big jj;
    int* rowptr = NULL;
    int* colind = NULL;
    double* val = NULL;
    NLuint n = M->n;

    nl_assert(solver == NL_CHOLMOD_EXT);
    nl_assert(M->m == M->n);
    
    if(M->type == NL_MATRIX_CRS) {
        CRS = (NLCRSMatrix*)M;
    } else if(M->type == NL_MATRIX_SPARSE_DYNAMIC) {
	/* 
	 * Note: since we convert once again into symmetric storage,
	 * we could also directly read the NLSparseMatrix there instead
	 * of copying once more...
	 */
        CRS = (NLCRSMatrix*)nlCRSMatrixNewFromSparseMatrix((NLSparseMatrix*)M);
    }

    LLt = NL_NEW(NLCholmodFactorizedMatrix);
    LLt->m = M->m;
    LLt->n = M->n;
    LLt->type = NL_MATRIX_OTHER;
    LLt->destroy_func = (NLDestroyMatrixFunc)(nlCholmodFactorizedMatrixDestroy);
    LLt->mult_func = (NLMultMatrixVectorFunc)(nlCholmodFactorizedMatrixMult);

    /*
     * Compute required nnz, if matrix is not already with symmetric storage,
     * ignore entries in the upper triangular part.
     */
    
    nnz=0;
    for(i=0; i<n; ++i) {
	for(jj=CRS->rowptr[i]; jj<CRS->rowptr[i+1]; ++jj) {
	    j=CRS->colind[jj];
	    if(j <= i) {
		++nnz;
	    }
	}
    }

    /*
     * Copy CRS matrix into CHOLDMOD matrix (and ignore upper trianglar part)
     */
    
    cM = CHOLMOD()->cholmod_allocate_sparse(
        n, n, nnz,    /* Dimensions and number of non-zeros */
        NL_FALSE,     /* Sorted = false */
        NL_TRUE,      /* Packed = true  */
        1,            /* stype (-1 = lower triangular, 1 = upper triangular) */
        CHOLMOD_REAL, /* Entries are real numbers */
        &CHOLMOD()->cholmod_common
    );

    rowptr = (int*)cM->p;
    colind = (int*)cM->i;
    val = (double*)cM->x;
    cur = 0;
    for(i=0; i<n; ++i) {
        rowptr[i] = (int)cur;
	for(jj=CRS->rowptr[i]; jj<CRS->rowptr[i+1]; ++jj) {
            j = CRS->colind[jj];
            if(j <= i) {
		val[cur] = CRS->val[jj];
		colind[cur] = (int)j;
		++cur;
            }
        }
    }
    rowptr[n] = (int)cur;
    nl_assert(cur==nnz);

    LLt->L = CHOLMOD()->cholmod_analyze(cM, &CHOLMOD()->cholmod_common);
    if(!CHOLMOD()->cholmod_factorize(cM, LLt->L, &CHOLMOD()->cholmod_common)) {
        CHOLMOD()->cholmod_free_factor(&LLt->L, &CHOLMOD()->cholmod_common);
	NL_DELETE(LLt);
    }
    
    CHOLMOD()->cholmod_free_sparse(&cM, &CHOLMOD()->cholmod_common);
    
    if((NLMatrix)CRS != M) {
        nlDeleteMatrix((NLMatrix)CRS);
    }

    return (NLMatrix)(LLt);
}

