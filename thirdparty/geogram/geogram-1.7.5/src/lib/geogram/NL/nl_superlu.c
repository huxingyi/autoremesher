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

#include "nl_superlu.h"
#include "nl_context.h"

/**
 * \file nl_superlu.c
 * \brief Weak-coupling adapter to call SuperLU from OpenNL, 
 *  works with SuperLU version >= 5.x.
 */

#ifdef NL_OS_UNIX
#  ifdef NL_OS_APPLE
#      define SUPERLU_LIB_NAME "libsuperlu_5.dylib"
#  else
#      define SUPERLU_LIB_NAME "libsuperlu.so"
#  endif
#else
#  define SUPERLU_LIB_NAME "libsuperlu.xxx"
#endif


/**********************************/
/** Exerpt from SuperLU includes **/
/** <slu_cdefs.h>                **/
/** <supermatrix.h>              **/
/**                              **/
/**********************************/

typedef enum {
    SLU_NC,    /* column-wise, no supernode */
    SLU_NCP,   /* column-wise, column-permuted, no supernode 
                  (The consecutive columns of nonzeros, after permutation,
                   may not be stored  contiguously.) */
    SLU_NR,    /* row-wize, no supernode */
    SLU_SC,    /* column-wise, supernode */
    SLU_SCP,   /* supernode, column-wise, permuted */    
    SLU_SR,    /* row-wise, supernode */
    SLU_DN,     /* Fortran style column-wise storage for dense matrix */
    SLU_NR_loc  /* distributed compressed row format  */ 
} Stype_t;

typedef enum {
    SLU_S,     /* single */
    SLU_D,     /* double */
    SLU_C,     /* single complex */
    SLU_Z      /* double complex */
} Dtype_t;


typedef enum {
    SLU_GE,    /* general */
    SLU_TRLU,  /* lower triangular, unit diagonal */
    SLU_TRUU,  /* upper triangular, unit diagonal */
    SLU_TRL,   /* lower triangular */
    SLU_TRU,   /* upper triangular */
    SLU_SYL,   /* symmetric, store lower half */
    SLU_SYU,   /* symmetric, store upper half */
    SLU_HEL,   /* Hermitian, store lower half */
    SLU_HEU    /* Hermitian, store upper half */
} Mtype_t;

typedef int int_t;

typedef struct {
    int_t  nnz;	    /* number of nonzeros in the matrix */
    void *nzval;    /* pointer to array of nonzero values, packed by raw */
    int_t  *colind; /* pointer to array of columns indices of the nonzeros */
    int_t  *rowptr; /* pointer to array of beginning of rows in nzval[] 
		       and colind[]  */
                    /* Note:
		       Zero-based indexing is used;
		       rowptr[] has nrow+1 entries, the last one pointing
		       beyond the last row, so that rowptr[nrow] = nnz. */
} NRformat;

typedef struct {
        Stype_t Stype; /* Storage type: interprets the storage structure 
                          pointed to by *Store. */
        Dtype_t Dtype; /* Data type. */
        Mtype_t Mtype; /* Matrix type: describes the mathematical property of 
                          the matrix. */
        int_t  nrow;   /* number of rows */
        int_t  ncol;   /* number of columns */
        void *Store;   /* pointer to the actual storage of the matrix */
} SuperMatrix;

/* Stype == SLU_DN */
typedef struct {
    int_t lda;    /* leading dimension */
    void *nzval;  /* array of size lda*ncol to represent a dense matrix */
} DNformat;


typedef enum {NO, YES}                                          yes_no_t;
typedef enum {DOFACT, SamePattern, SamePattern_SameRowPerm, FACTORED} fact_t;
typedef enum {NOROWPERM, LargeDiag, MY_PERMR}                   rowperm_t;
typedef enum {NATURAL, MMD_ATA, MMD_AT_PLUS_A, COLAMD,
              METIS_AT_PLUS_A, PARMETIS, ZOLTAN, MY_PERMC}      colperm_t;
typedef enum {NOTRANS, TRANS, CONJ}                             trans_t;
typedef enum {NOEQUIL, ROW, COL, BOTH}                          DiagScale_t;
typedef enum {NOREFINE, SLU_SINGLE=1, SLU_DOUBLE, SLU_EXTRA}    IterRefine_t;
typedef enum {LUSUP, UCOL, LSUB, USUB, LLVL, ULVL}              MemType;
typedef enum {HEAD, TAIL}                                       stack_end_t;
typedef enum {SYSTEM, USER}                                     LU_space_t;
typedef enum {ONE_NORM, TWO_NORM, INF_NORM}                     norm_t;
typedef enum {SILU, SMILU_1, SMILU_2, SMILU_3}                  milu_t;

typedef struct {
    fact_t        Fact;
    yes_no_t      Equil;
    colperm_t     ColPerm;
    trans_t       Trans;
    IterRefine_t  IterRefine;
    double        DiagPivotThresh;
    yes_no_t      SymmetricMode;
    yes_no_t      PivotGrowth;
    yes_no_t      ConditionNumber;
    rowperm_t     RowPerm;
    int           ILU_DropRule;
    double        ILU_DropTol;    /* threshold for dropping */
    double        ILU_FillFactor; /* gamma in the secondary dropping */
    norm_t        ILU_Norm;       /* infinity-norm, 1-norm, or 2-norm */
    double        ILU_FillTol;    /* threshold for zero pivot perturbation */
    milu_t        ILU_MILU;
    double        ILU_MILU_Dim;   /* Dimension of PDE (if available) */
    yes_no_t      ParSymbFact;
    yes_no_t      ReplaceTinyPivot; /* used in SuperLU_DIST */
    yes_no_t      SolveInitialized;
    yes_no_t      RefineInitialized;
    yes_no_t      PrintStat;
    int           nnzL, nnzU;      /* used to store nnzs for now       */
    int           num_lookaheads;  /* num of levels in look-ahead      */
    yes_no_t      lookahead_etree; /* use etree computed from the
                                      serial symbolic factorization */
    yes_no_t      SymPattern;      /* symmetric factorization          */
} superlu_options_t;

typedef void* superlu_options_ptr;

typedef float    flops_t;
typedef unsigned char Logical;

typedef struct {
    int     *panel_histo;    /* histogram of panel size distribution */
    double  *utime;          /* running time at various phases */
    flops_t *ops;            /* operation count at various phases */
    int     TinyPivots;      /* number of tiny pivots */
    int     RefineSteps;     /* number of iterative refinement steps */
    int     expansions;      /* number of memory expansions (SuperLU4) */
} SuperLUStat_t;

/*! \brief Headers for 4 types of dynamatically managed memory */
typedef struct e_node {
    int size;      /* length of the memory that has been used */
    void *mem;     /* pointer to the new malloc'd store */
} ExpHeader;

typedef struct {
    int  size;
    int  used;
    int  top1;  /* grow upward, relative to &array[0] */
    int  top2;  /* grow downward */
    void *array;
} LU_stack_t;

typedef struct {
    int     *xsup;    /* supernode and column mapping */
    int     *supno;   
    int     *lsub;    /* compressed L subscripts */
    int	    *xlsub;
    void    *lusup;   /* L supernodes */
    int     *xlusup;
    void    *ucol;    /* U columns */
    int     *usub;
    int	    *xusub;
    int     nzlmax;   /* current max size of lsub */
    int     nzumax;   /*    "    "    "      ucol */
    int     nzlumax;  /*    "    "    "     lusup */
    int     n;        /* number of columns in the matrix */
    LU_space_t MemModel; /* 0 - system malloc'd; 1 - user provided */
    int     num_expansions;
    ExpHeader *expanders; /* Array of pointers to 4 types of memory */
    LU_stack_t stack;     /* use user supplied memory */
} GlobalLU_t;


/*****************************************/
/** End of exerpt from SuperLU includes **/
/**                                     **/
/*****************************************/

/*****************************************/
/** Functions pointers to allow dynamic **/
/** linking of superlu libs.            **/
/**                                     **/
/*****************************************/

typedef void (*FUNPTR_set_default_options)(superlu_options_ptr options);
typedef void (*FUNPTR_ilu_set_default_options)(superlu_options_ptr options);
typedef void (*FUNPTR_StatInit)(SuperLUStat_t *);
typedef void (*FUNPTR_StatFree)(SuperLUStat_t *);

typedef void (*FUNPTR_dCreate_CompCol_Matrix)(
    SuperMatrix *, int, int, int, const double *,
    const int *, const int *, Stype_t, Dtype_t, Mtype_t);

typedef void (*FUNPTR_dCreate_Dense_Matrix)(
    SuperMatrix *, int, int, const double *, int,
    Stype_t, Dtype_t, Mtype_t);

typedef void (*FUNPTR_Destroy_SuperNode_Matrix)(SuperMatrix *);
typedef void (*FUNPTR_Destroy_CompCol_Matrix)(SuperMatrix *);
typedef void (*FUNPTR_Destroy_CompCol_Permuted)(SuperMatrix *);
typedef void (*FUNPTR_Destroy_SuperMatrix_Store)(SuperMatrix *);

typedef void (*FUNPTR_dgssv)(
    superlu_options_ptr, SuperMatrix *, int *, int *, SuperMatrix *,
    SuperMatrix *, SuperMatrix *, SuperLUStat_t *, int *
);

typedef void (*FUNPTR_dgstrs)(
    trans_t, SuperMatrix *, SuperMatrix *, int *, int *,
    SuperMatrix *, SuperLUStat_t*, int *    
);

typedef void (*FUNPTR_get_perm_c)(int, SuperMatrix *, int *);
typedef void (*FUNPTR_sp_preorder)(
   superlu_options_t *, SuperMatrix*, int*, int*, SuperMatrix*
);
typedef int (*FUNPTR_sp_ienv)(int);
typedef int (*FUNPTR_input_error)(const char *, int *);

typedef void (*FUNPTR_dgstrf) (superlu_options_t *options, SuperMatrix *A,
        int relax, int panel_size, int *etree, void *work, int lwork,
        int *perm_c, int *perm_r, SuperMatrix *L, SuperMatrix *U,
    	GlobalLU_t *Glu, /* persistent to facilitate multiple factorizations */
        SuperLUStat_t *stat, int *info
);


/**
 * \brief The structure that stores the handle to 
 *  the SuperLU shared object and the function pointers.
 */
typedef struct {
    FUNPTR_set_default_options set_default_options;
    FUNPTR_ilu_set_default_options ilu_set_default_options;    
    FUNPTR_StatInit StatInit;
    FUNPTR_StatFree StatFree;
    FUNPTR_dCreate_CompCol_Matrix dCreate_CompCol_Matrix;
    FUNPTR_dCreate_Dense_Matrix dCreate_Dense_Matrix;
    FUNPTR_Destroy_SuperNode_Matrix Destroy_SuperNode_Matrix;
    FUNPTR_Destroy_CompCol_Matrix Destroy_CompCol_Matrix;
    FUNPTR_Destroy_CompCol_Permuted Destroy_CompCol_Permuted;    
    FUNPTR_Destroy_SuperMatrix_Store Destroy_SuperMatrix_Store;
    FUNPTR_dgssv dgssv;
    FUNPTR_dgstrs dgstrs;
    FUNPTR_get_perm_c get_perm_c;
    FUNPTR_sp_preorder sp_preorder;
    FUNPTR_sp_ienv sp_ienv;
    FUNPTR_dgstrf dgstrf;
    FUNPTR_input_error input_error;
    
    NLdll DLL_handle;
} SuperLUContext;

/**
 * \brief Gets the SuperLU context.
 * \return a pointer to the SuperLU context
 */
static SuperLUContext* SuperLU() {
    static SuperLUContext context;
    static NLboolean init = NL_FALSE;
    if(!init) {
        init = NL_TRUE;
        memset(&context, 0, sizeof(context));
    }
    return &context;
}

NLboolean nlExtensionIsInitialized_SUPERLU() {
    return
        SuperLU()->DLL_handle != NULL &&
        SuperLU()->set_default_options != NULL &&
        SuperLU()->ilu_set_default_options != NULL &&   
        SuperLU()->StatInit != NULL &&
        SuperLU()->StatFree != NULL &&
        SuperLU()->dCreate_CompCol_Matrix != NULL &&
        SuperLU()->dCreate_Dense_Matrix != NULL &&
        SuperLU()->Destroy_SuperNode_Matrix != NULL &&
        SuperLU()->Destroy_CompCol_Matrix != NULL &&
        SuperLU()->Destroy_CompCol_Permuted != NULL &&	
        SuperLU()->Destroy_SuperMatrix_Store != NULL &&
        SuperLU()->dgssv != NULL &&
        SuperLU()->dgstrs != NULL &&
	SuperLU()->get_perm_c != NULL &&
	SuperLU()->sp_preorder != NULL &&
	SuperLU()->sp_ienv != NULL &&
	SuperLU()->dgstrf != NULL &&
	SuperLU()->input_error != NULL;
}

static void nlTerminateExtension_SUPERLU(void) {
    if(SuperLU()->DLL_handle != NULL) {
        nlCloseDLL(SuperLU()->DLL_handle);
        SuperLU()->DLL_handle = NULL;
	memset(SuperLU(), 0, sizeof(SuperLUContext));
    }
}


/**
 * \brief Finds and initializes a function pointer to
 *  one of the functions in SuperLU.
 * \details Function pointers are stored into the 
 *  SuperLUContext returned by the function SuperLU().
 *  If a symbol is not found, returns NL_FALSE from the
 *  calling function.
 */
#define find_superlu_func(name)                                   \
    if(                                                           \
        (                                                         \
            SuperLU()->name =                                     \
            (FUNPTR_##name)nlFindFunction(SuperLU()->DLL_handle,#name) \
        ) == NULL                                                 \
    ) {                                                           \
        nlError("nlInitExtension_SUPERLU","function not found");  \
        nlError("nlInitExtension_SUPERLU",#name);                 \
        return NL_FALSE;                                          \
    }


NLboolean nlInitExtension_SUPERLU(void) {
    NLenum flags = NL_LINK_NOW | NL_LINK_USE_FALLBACK;
    if(nlCurrentContext == NULL || !nlCurrentContext->verbose) {
	flags |= NL_LINK_QUIET;
    }
    
    if(SuperLU()->DLL_handle != NULL) {
        return nlExtensionIsInitialized_SUPERLU();
    }

    SuperLU()->DLL_handle = nlOpenDLL(SUPERLU_LIB_NAME, flags);
    if(SuperLU()->DLL_handle == NULL) {
        return NL_FALSE;
    }
    
    find_superlu_func(set_default_options);
    find_superlu_func(ilu_set_default_options);    
    find_superlu_func(StatInit);
    find_superlu_func(StatFree);
    find_superlu_func(dCreate_CompCol_Matrix);
    find_superlu_func(dCreate_Dense_Matrix);
    find_superlu_func(Destroy_SuperNode_Matrix);
    find_superlu_func(Destroy_CompCol_Matrix);
    find_superlu_func(Destroy_CompCol_Permuted);        
    find_superlu_func(Destroy_SuperMatrix_Store);
    find_superlu_func(dgssv);
    find_superlu_func(dgstrs);
    find_superlu_func(get_perm_c);    
    find_superlu_func(sp_preorder);
    find_superlu_func(sp_ienv);
    find_superlu_func(dgstrf);
    find_superlu_func(input_error);

    atexit(nlTerminateExtension_SUPERLU);
    return NL_TRUE;
}

/************************************************************************/

/**
 * \brief The class for matrices factorized with SuperLU.
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
     * \details set to NL_MATRIX_OTHER
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
     * \brief Lower triangular factor.
     */
    SuperMatrix L;

    /**
     * \brief Upper triangular factor.
     */
    SuperMatrix U;

    /**
     * \brief Rows permutation.
     */
    int* perm_r;

    /**
     * \brief Columns permutation.
     */
    int* perm_c;

    /**
     * \brief Indicates whether the matrix or its transpose
     *  was factorized.
     */
    trans_t trans;
    
} NLSuperLUFactorizedMatrix;


static void nlSuperLUFactorizedMatrixDestroy(NLSuperLUFactorizedMatrix* M) {
    if(nlExtensionIsInitialized_SUPERLU()) {
	SuperLU()->Destroy_SuperNode_Matrix(&M->L);
	SuperLU()->Destroy_CompCol_Matrix(&M->U);
    }
    NL_DELETE_ARRAY(M->perm_r);
    NL_DELETE_ARRAY(M->perm_c);
}

static void nlSuperLUFactorizedMatrixMult(
    NLSuperLUFactorizedMatrix* M, const double* x, double* y
) {
    SuperMatrix B;
    SuperLUStat_t stat;
    int info = 0;
    NLuint i;

    /* Create vector */
    SuperLU()->dCreate_Dense_Matrix(
        &B, (int)(M->n), 1, y, (int)(M->n), 
        SLU_DN, /* Fortran-type column-wise storage */
        SLU_D,  /* doubles */
        SLU_GE  /* general */
    );

    /* copy rhs onto y (superLU matrix-vector product expects it here */
    for(i = 0; i < M->n; i++){
        y[i] = x[i];
    }

    /* Call SuperLU triangular solve */
    SuperLU()->StatInit(&stat) ;

    SuperLU()->dgstrs(
       M->trans, &M->L, &M->U, M->perm_c, M->perm_r, &B, &stat, &info
    );

    SuperLU()->StatFree(&stat) ;
    
    /*  Only the "store" structure needs to be 
     *  deallocated (the array has been allocated
     * by client code).
     */
    SuperLU()->Destroy_SuperMatrix_Store(&B) ;
}

/*
 * Copied from SUPERLU/dgssv.c, removed call to linear solve.
 */
static void dgssv_factorize_only(
      superlu_options_t *options, SuperMatrix *A, int *perm_c, int *perm_r,
      SuperMatrix *L, SuperMatrix *U,
      SuperLUStat_t *stat, int *info, trans_t *trans
) {
    SuperMatrix *AA = NULL;
        /* A in SLU_NC format used by the factorization routine.*/
    SuperMatrix AC; /* Matrix postmultiplied by Pc */
    int      lwork = 0, *etree, i;
    GlobalLU_t Glu; /* Not needed on return. */
    
    /* Set default values for some parameters */
    int      panel_size;     /* panel size */
    int      relax;          /* no of columns in a relaxed snodes */
    int      permc_spec;

    nl_assert(A->Stype == SLU_NR || A->Stype == SLU_NC);
    
    *trans = NOTRANS;

    if ( options->Fact != DOFACT ) *info = -1;
    else if ( A->nrow != A->ncol || A->nrow < 0 ||
	 (A->Stype != SLU_NC && A->Stype != SLU_NR) ||
	 A->Dtype != SLU_D || A->Mtype != SLU_GE )
	*info = -2;
    if ( *info != 0 ) {
	i = -(*info);
	SuperLU()->input_error("SUPERLU/OpenNL dgssv_factorize_only", &i);
	return;
    }

    /* Convert A to SLU_NC format when necessary. */
    if ( A->Stype == SLU_NR ) {
	NRformat *Astore = (NRformat*)A->Store;
	AA = NL_NEW(SuperMatrix);
	SuperLU()->dCreate_CompCol_Matrix(
	    AA, A->ncol, A->nrow, Astore->nnz, 
	    (double*)Astore->nzval, Astore->colind, Astore->rowptr,
	    SLU_NC, A->Dtype, A->Mtype
	);
	*trans = TRANS;
    } else {
        if ( A->Stype == SLU_NC ) AA = A;
    }

    nl_assert(AA != NULL);
    
    /*
     * Get column permutation vector perm_c[], according to permc_spec:
     *   permc_spec = NATURAL:  natural ordering 
     *   permc_spec = MMD_AT_PLUS_A: minimum degree on structure of A'+A
     *   permc_spec = MMD_ATA:  minimum degree on structure of A'*A
     *   permc_spec = COLAMD:   approximate minimum degree column ordering
     *   permc_spec = MY_PERMC: the ordering already supplied in perm_c[]
     */
    permc_spec = (int)(options->ColPerm);
    if ( permc_spec != MY_PERMC && options->Fact == DOFACT )
	SuperLU()->get_perm_c(permc_spec, AA, perm_c);
    
    etree = NL_NEW_ARRAY(int,A->ncol);
    SuperLU()->sp_preorder(options, AA, perm_c, etree, &AC);
    panel_size = SuperLU()->sp_ienv(1);
    relax = SuperLU()->sp_ienv(2);
    SuperLU()->dgstrf(options, &AC, relax, panel_size, etree,
            NULL, lwork, perm_c, perm_r, L, U, &Glu, stat, info);

    NL_DELETE_ARRAY(etree);
    SuperLU()->Destroy_CompCol_Permuted(&AC);
    if ( A->Stype == SLU_NR ) {
	SuperLU()->Destroy_SuperMatrix_Store(AA);
	NL_DELETE(AA);
    }
}


NLMatrix nlMatrixFactorize_SUPERLU(
    NLMatrix M, NLenum solver
) {
    NLSuperLUFactorizedMatrix* LU = NULL;
    NLCRSMatrix* CRS = NULL;
    SuperMatrix superM;
    NLuint n = M->n;
    superlu_options_t options;
    SuperLUStat_t     stat;
    NLint info = 0;       /* status code  */
    
    nl_assert(M->m == M->n);

    if(M->type == NL_MATRIX_CRS) {
        CRS = (NLCRSMatrix*)M;
    } else if(M->type == NL_MATRIX_SPARSE_DYNAMIC) {
        CRS = (NLCRSMatrix*)nlCRSMatrixNewFromSparseMatrix((NLSparseMatrix*)M);
    }

    nl_assert(!(CRS->symmetric_storage));
    
    LU = NL_NEW(NLSuperLUFactorizedMatrix);
    LU->m = M->m;
    LU->n = M->n;
    LU->type = NL_MATRIX_OTHER;
    LU->destroy_func = (NLDestroyMatrixFunc)(nlSuperLUFactorizedMatrixDestroy);
    LU->mult_func = (NLMultMatrixVectorFunc)(nlSuperLUFactorizedMatrixMult);
    LU->perm_c = NL_NEW_ARRAY(int, n);
    LU->perm_r = NL_NEW_ARRAY(int, n);    

    SuperLU()->dCreate_CompCol_Matrix(
        &superM, (int)n, (int)n, (int)nlCRSMatrixNNZ(CRS),
        CRS->val, (int*)CRS->colind, (int*)CRS->rowptr, 
        SLU_NR,              /* Row_wise, no supernode */
        SLU_D,               /* doubles                */ 
        CRS->symmetric_storage ? SLU_SYL : SLU_GE
    );

    SuperLU()->set_default_options(&options);
    switch(solver) {
    case NL_SUPERLU_EXT: {
        options.ColPerm = NATURAL;
    } break;
    case NL_PERM_SUPERLU_EXT: {
        options.ColPerm = COLAMD;
    } break;
    case NL_SYMMETRIC_SUPERLU_EXT: {
        options.ColPerm = MMD_AT_PLUS_A;
        options.SymmetricMode = YES;
    } break;
    default: 
        nl_assert_not_reached;
    }
    
    SuperLU()->StatInit(&stat);

    dgssv_factorize_only(
	  &options, &superM, LU->perm_c, LU->perm_r,
	  &LU->L, &LU->U, &stat, &info, &LU->trans
    );

    SuperLU()->StatFree(&stat);
    
    /*
     * Only the "store" structure needs to be deallocated 
     * (the arrays have been allocated by us, they are in CRS).
     */
    SuperLU()->Destroy_SuperMatrix_Store(&superM);
    
    if((NLMatrix)CRS != M) {
        nlDeleteMatrix((NLMatrix)CRS);
    }

    if(info != 0) {
	NL_DELETE(LU);
	LU = NULL;
    }
    return (NLMatrix)LU;
}
