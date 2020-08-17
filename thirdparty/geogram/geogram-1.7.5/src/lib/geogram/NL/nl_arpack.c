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

#include "nl_arpack.h"
#include "nl_context.h"

/**
 * \file nl_arpack.c
 * \brief Weak-coupling adapter to call ARPACK from OpenNL.
 */

#ifdef NL_OS_UNIX
#  ifdef NL_OS_APPLE
#      define ARPACK_LIB_NAME "libarpack.dylib"
#  else
#      define ARPACK_LIB_NAME "libarpack.so"
#  endif
#else
#  define ARPACK_LIB_NAME "libarpack.dll"
#endif


typedef int ARint;
typedef int ARlogical;


/* double precision symmetric routines */

typedef void (*FUNPTR_dsaupd)(
    ARint *ido, char *bmat, ARint *n, char *which,
    ARint *nev, double *tol, double *resid,
    ARint *ncv, double *V, ARint *ldv,
    ARint *iparam, ARint *ipntr, double *workd,
    double *workl, ARint *lworkl, ARint *info
);

typedef void (*FUNPTR_dseupd)(
    ARlogical *rvec, char *HowMny, ARlogical *select,
    double *d, double *Z, ARint *ldz,
    double *sigma, char *bmat, ARint *n,
    char *which, ARint *nev, double *tol,
    double *resid, ARint *ncv, double *V,
    ARint *ldv, ARint *iparam, ARint *ipntr,
    double *workd, double *workl,
    ARint *lworkl, ARint *info
);

/* double precision nonsymmetric routines */
    
typedef void (*FUNPTR_dnaupd)(
    ARint *ido, char *bmat, ARint *n, char *which,
    ARint *nev, double *tol, double *resid,
    ARint *ncv, double *V, ARint *ldv,
    ARint *iparam, ARint *ipntr, double *workd,
    double *workl, ARint *lworkl, ARint *info
);

typedef void (*FUNPTR_dneupd)(
    ARlogical *rvec, char *HowMny, ARlogical *select,
    double *dr, double *di, double *Z,
    ARint *ldz, double *sigmar,
    double *sigmai, double *workev,
    char *bmat, ARint *n, char *which,
    ARint *nev, double *tol, double *resid,
    ARint *ncv, double *V, ARint *ldv,
    ARint *iparam, ARint *ipntr,
    double *workd, double *workl,
    ARint *lworkl, ARint *info
);



/**
 * \brief The structure that stores the handle to 
 *  the ARPACK shared object and the function pointers.
 */
typedef struct {
    FUNPTR_dsaupd dsaupd;
    FUNPTR_dseupd dseupd;
    FUNPTR_dnaupd dnaupd;
    FUNPTR_dneupd dneupd;
    NLdll DLL_handle;
} ARPACKContext;


/**
 * \brief Gets the ARPACK context.
 * \return a pointer to the ARPACK context
 */
static ARPACKContext* ARPACK() {
    static ARPACKContext context;
    static NLboolean init = NL_FALSE;
    if(!init) {
        init = NL_TRUE;
        memset(&context, 0, sizeof(context));
    }
    return &context;
}

NLboolean nlExtensionIsInitialized_ARPACK() {
    return
        ARPACK()->DLL_handle != NULL &&
        ARPACK()->dsaupd != NULL &&
        ARPACK()->dseupd != NULL &&   
        ARPACK()->dnaupd != NULL &&
        ARPACK()->dneupd != NULL;
}

static void nlTerminateExtension_ARPACK(void) {
    if(ARPACK()->DLL_handle != NULL) {
        nlCloseDLL(ARPACK()->DLL_handle);
        ARPACK()->DLL_handle = NULL;
    }
}


/**
 * \brief Appends an underscore to the input string
 * \param[in] str the input string
 * \return the input string with an appended underscore,
 *   created in a static buffer.
 */
static char* u(const char* str) {
    static char buff[1000];
    sprintf(buff, "%s_", str);
    return buff;
}

/**
 * \brief Finds and initializes a function pointer to
 *  one of the functions in ARPACK.
 * \details Function pointers are stored into the 
 *  ARPACKContext returned by the function ARPACK().
 *  If a symbol is not found, returns NL_FALSE from the
 *  calling function.
 */
#define find_arpack_func(name)                                             \
    if(                                                                    \
        (                                                                  \
            ARPACK()->name =                                               \
            (FUNPTR_##name)nlFindFunction(ARPACK()->DLL_handle,u(#name))   \
        ) == NULL                                                          \
    ) {                                                                    \
        nlError("nlInitExtension_ARPACK","function not found");            \
        nlError("nlInitExtension_ARPACK",u(#name));	   		   \
        return NL_FALSE;                                                   \
    }

NLboolean nlInitExtension_ARPACK(void) {
    NLenum flags = NL_LINK_NOW | NL_LINK_USE_FALLBACK;
    if(nlCurrentContext == NULL || !nlCurrentContext->verbose) {
	flags |= NL_LINK_QUIET;
    }
    
    if(ARPACK()->DLL_handle != NULL) {
        return nlExtensionIsInitialized_ARPACK();
    }
    
    ARPACK()->DLL_handle = nlOpenDLL(ARPACK_LIB_NAME, flags);
    if(ARPACK()->DLL_handle == NULL) {
        return NL_FALSE;
    }

    find_arpack_func(dsaupd);
    find_arpack_func(dseupd);
    find_arpack_func(dnaupd);
    find_arpack_func(dneupd);

    atexit(nlTerminateExtension_ARPACK);
    return NL_TRUE;
}

/*****************************************************************************/

/**
 * \brief Creates the OPerator used by ARPACK
 * \param[in] symmetric NL_TRUE if matrix is symmetric and there is no
 *  right-hand side matrix. NL_FALSE otherwise.
 */
static NLMatrix create_OP(NLboolean symmetric) {
    NLuint n = nlCurrentContext->M->n;
    NLuint i;
    NLMatrix result = NULL;
    
	
    if(nlCurrentContext->eigen_shift != 0.0) {
	/*
	 * A = M
	 */
	NLSparseMatrix* A = NL_NEW(NLSparseMatrix);
	nlSparseMatrixConstruct(A, n, n, NL_MATRIX_STORE_ROWS);
	nlSparseMatrixAddMatrix(A, 1.0, nlCurrentContext->M);
	if(nlCurrentContext->B == NULL) {
	    /*
	     * A = A - shift * Id
	     */
	    for(i=0; i<n; ++i) {
		nlSparseMatrixAdd(A, i, i, -nlCurrentContext->eigen_shift);
	    }
	} else {
	    /*
	     * A = A - shift * B
	     */
	    nlSparseMatrixAddMatrix(
		A, -nlCurrentContext->eigen_shift, nlCurrentContext->B
	    );
	}

	/* 
	 * OP = A^{-1} 
	 */
	if(nlCurrentContext->verbose) {
	    nl_printf("Factorizing matrix...\n");
	}
	result = nlMatrixFactorize(
	    (NLMatrix)A,
	    symmetric ? NL_SYMMETRIC_SUPERLU_EXT : NL_PERM_SUPERLU_EXT
	);
	if(nlCurrentContext->verbose) {
	    if(result == NULL) {
		nl_printf("Could not factorize matrix\n");
	    } else {
		nl_printf("Matrix factorized\n");
	    }
	}
	nlDeleteMatrix((NLMatrix)A);
    } else {
	/* 
	 * OP = M^{-1} 
	 */
	if(nlCurrentContext->verbose) {
	    nl_printf("Factorizing matrix...\n");
	}
	result = nlMatrixFactorize(
	    nlCurrentContext->M,
	    symmetric ? NL_SYMMETRIC_SUPERLU_EXT : NL_PERM_SUPERLU_EXT
	);
	if(nlCurrentContext->verbose) {
	    if(result == NULL) {
		nl_printf("Could not factorize matrix\n");		
	    } else {
		nl_printf("Matrix factorized\n");
	    }
	}
    }

    if(result == NULL) {
	return NULL;
    }
    
    if(nlCurrentContext->B != NULL) {
	/* 
	 * OP = OP * B
	 */	
	result = nlMatrixNewFromProduct(
	    result, NL_TRUE, /* mem. ownership transferred */
	    nlCurrentContext->B, NL_FALSE  /* mem. ownership kept by context */
	);
    }

    return result;
}

static int eigencompare(const void* pi, const void* pj) {
    NLuint i = *(const NLuint*)pi;
    NLuint j = *(const NLuint*)pj;
    double vali = fabs(nlCurrentContext->temp_eigen_value[i]);
    double valj = fabs(nlCurrentContext->temp_eigen_value[j]);
    if(vali == valj) {
	return 0;
    }
    return vali < valj ? -1 : 1;
}

void nlEigenSolve_ARPACK(void) {
    NLboolean symmetric =
	nlCurrentContext->symmetric && (nlCurrentContext->B == NULL); 
    int n = (int)nlCurrentContext->M->n; /* Dimension of the matrix */
    int nev = /* Number of eigenvectors requested */
	(int)nlCurrentContext->nb_systems;
    NLMatrix OP = create_OP(symmetric);
    int ncv = (int)(nev * 2.5); /* Length of Arnoldi factorization */
                 /* Rule of thumb in ARPACK documentation: ncv > 2 * nev */
    int* iparam = NULL;
    int* ipntr  = NULL;
    NLdouble* resid = NULL;
    NLdouble* workev = NULL;
    NLdouble* workd = NULL;
    NLdouble* workl = NULL;
    NLdouble* v = NULL;
    NLdouble* d = NULL;
    ARlogical* select = NULL;
    ARlogical rvec = 1;
    double sigmar = 0.0;
    double sigmai = 0.0;
    int ierr;
    int i,k,kk;
    int ldv = (int)n;
    char* bmat = (char*)"I";   /*Standard problem */
    char* which = (char*)"LM"; /*Largest eigenvalues, but we invert->smallest */
    char* howmny = (char*)"A"; /*which eigens should be computed: all */
    double tol = nlCurrentContext->threshold;
    int ido = 0;  /* reverse communication variable (which operation ?) */
    int info = 1; /* start with initial value of resid */
    int lworkl;   /* size of work array */
    NLboolean converged = NL_FALSE;
    NLdouble value;
    int index;
    int* sorted; /* indirection array for sorting eigenpairs */

    if(OP == NULL) {
	nlError("nlEigenSolve_ARPACK","Could not factorize matrix");
	return;
    }
    
    if(ncv > n) {
	ncv = n;
    }

    if(nev > n) {
	nev = n;
    }

    if(nev + 2 > ncv) {
	nev = ncv  - 2;
    }

    
    if(symmetric) {
	lworkl = ncv * (ncv + 8) ;
    } else {
	lworkl = 3*ncv*ncv + 6*ncv ; 
    }
    iparam = NL_NEW_ARRAY(int, 11);
    ipntr  = NL_NEW_ARRAY(int, 14);

    iparam[1-1] = 1; /* ARPACK chooses the shifts */
    iparam[3-1] = (int)nlCurrentContext->max_iterations;
    iparam[7-1] = 1; /* Normal mode (we do not use 
         shift-invert (3) since we do our own shift-invert */

    workev = NL_NEW_ARRAY(NLdouble, 3*ncv);
    workd = NL_NEW_ARRAY(NLdouble, 3*n);

    resid = NL_NEW_ARRAY(NLdouble, n);
    for(i=0; i<n; ++i) {
	resid[i] = 1.0; /* (double)i / (double)n; */
    }
    v = NL_NEW_ARRAY(NLdouble, ldv*ncv);
    if(symmetric) {
	d = NL_NEW_ARRAY(NLdouble, 2*ncv);
    } else {
	d = NL_NEW_ARRAY(NLdouble, 3*ncv);	
    }
    workl = NL_NEW_ARRAY(NLdouble, lworkl);

    /********** Main ARPACK loop ***********/

    if(nlCurrentContext->verbose) {
	if(symmetric) {
	    nl_printf("calling dsaupd()\n");	    
	} else {
	    nl_printf("calling dnaupd()\n");
	}
    }
    while(!converged) {
	/*
	if(nlCurrentContext->verbose) {
	    fprintf(stderr, ".");
	    fflush(stderr);
	}
	*/
	if(symmetric) {
	    ARPACK()->dsaupd(
		&ido, bmat, &n, which, &nev, &tol, resid, &ncv,
		v, &ldv, iparam, ipntr, workd, workl, &lworkl, &info
	    );
	} else {
	    ARPACK()->dnaupd(
		&ido, bmat, &n, which, &nev, &tol, resid, &ncv,
		v, &ldv, iparam, ipntr, workd, workl, &lworkl, &info
	    );
	}
	if(ido == 1) {
	    nlMultMatrixVector(
             OP,
	     workd+ipntr[1-1]-1, /*The "-1"'s are for FORTRAN-to-C conversion */
	     workd+ipntr[2-1]-1  /*to keep the same indices as in ARPACK doc  */
	    );
	} else {
	    converged = NL_TRUE;
	}
    }

    /********** ARPACK post-processing *****/

    if(info < 0) {
	if(symmetric) {
	    nl_fprintf(stderr, "\nError with dsaupd(): %d\n", info);	    
	} else {
	    nl_fprintf(stderr, "\nError with dnaupd(): %d\n", info);
	}
    } else {
	if(nlCurrentContext->verbose) {
	    fprintf(stderr, "\nconverged\n");
	}
	
	select = NL_NEW_ARRAY(ARlogical, ncv);
	for(i=0; i<ncv; ++i) {
	    select[i] = 1;
	}
	
	if(nlCurrentContext->verbose) {
	    if(symmetric) {
		nl_printf("calling dseupd()\n");		
	    } else {
		nl_printf("calling dneupd()\n");
	    }
	}
	
        if(symmetric) {
            ARPACK()->dseupd(
                &rvec, howmny, select, d, v, 
                &ldv, &sigmar, bmat, &n, which, &nev, 
                &tol, resid, &ncv, v, &ldv, 
                iparam, ipntr, workd,
		workl, &lworkl, &ierr 
	    );
        } else {
	    ARPACK()->dneupd(
		&rvec, howmny, select, d, d+ncv,
                v, &ldv, 
                &sigmar, &sigmai, workev, bmat, &n,
		which, &nev, &tol, 
                resid, &ncv, v, &ldv, iparam, 
		ipntr, workd, workl, &lworkl, &ierr 
            ) ;
	}	


	if(nlCurrentContext->verbose) {
	    if(ierr != 0) {		
		if(symmetric) {
		    nl_fprintf(stderr, "Error with dseupd(): %d\n", ierr);
		} else {
		    nl_fprintf(stderr, "Error with dneupd(): %d\n", ierr);
		}
	    } else {
		if(symmetric) {
		    nl_printf("dseupd() OK, nconv= %d\n", iparam[3-1]);
		} else {
		    nl_printf("dneupd() OK, nconv= %d\n", iparam[3-1]);
		}
	    }
	}
	
	NL_DELETE_ARRAY(select);
    }

    /********** Apply spectral transform ***/

    for(i=0; i<nev; ++i) {
	d[i] = (fabs(d[i]) < 1e-30) ? 1e30 : 1.0 / d[i] ;
	d[i] += nlCurrentContext->eigen_shift ;
    }            

    /********** Sort eigenpairs ************/
    
    /* Make it visible to the eigen_compare function */
    nlCurrentContext->temp_eigen_value = d;
    sorted = NL_NEW_ARRAY(int, nev);
    for(i=0; i<nev; ++i) {
	sorted[i] = i;
    }
    qsort(sorted, (size_t)nev, sizeof(NLuint), eigencompare);
    nlCurrentContext->temp_eigen_value = NULL;
    
    /********** Copy to NL context *********/

    for(k=0; k<nev; ++k) {
	kk = sorted[k];
	nlCurrentContext->eigen_value[k] = d[kk];
	for(i=0; i<(int)nlCurrentContext->nb_variables; ++i) {
	    if(!nlCurrentContext->variable_is_locked[i]) {
		index = (int)nlCurrentContext->variable_index[i];
		nl_assert(index < n);
		value = v[kk*n+index];
		NL_BUFFER_ITEM(
		    nlCurrentContext->variable_buffer[k],(NLuint)i
		) = value;
	    }
	}
    }
    
    /********** Cleanup ********************/

    NL_DELETE_ARRAY(sorted);
    NL_DELETE_ARRAY(workl);
    NL_DELETE_ARRAY(d);
    NL_DELETE_ARRAY(v);
    NL_DELETE_ARRAY(resid);
    NL_DELETE_ARRAY(workd);
    NL_DELETE_ARRAY(workev);
    nlDeleteMatrix(OP);
    NL_DELETE_ARRAY(iparam);
    NL_DELETE_ARRAY(ipntr);
}

/*****************************************************************************/

