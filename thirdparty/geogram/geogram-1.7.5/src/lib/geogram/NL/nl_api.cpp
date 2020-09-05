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

#include "nl_private.h"
#include "nl_matrix.h"
#include "nl_context.h"
#include "nl_iterative_solvers.h"
#include "nl_preconditioners.h"
#include "nl_superlu.h"
#include "nl_cholmod.h"
#include "nl_arpack.h"
#include "nl_mkl.h"
#include "nl_cuda.h"

/*****************************************************************************/

static NLSparseMatrix* nlGetCurrentSparseMatrix() {
    NLSparseMatrix* result = NULL;
    switch(nlCurrentContext->matrix_mode) {
	case NL_STIFFNESS_MATRIX: {
	    nl_assert(nlCurrentContext->M != NULL);	    
	    nl_assert(nlCurrentContext->M->type == NL_MATRIX_SPARSE_DYNAMIC);
	    result = (NLSparseMatrix*)(nlCurrentContext->M);
	} break;
	case NL_MASS_MATRIX: {
	    nl_assert(nlCurrentContext->B != NULL);
	    nl_assert(nlCurrentContext->B->type == NL_MATRIX_SPARSE_DYNAMIC);
	    result = (NLSparseMatrix*)(nlCurrentContext->B);
	} break;
	default:
	    nl_assert_not_reached;
    }
    return result;
}

/*****************************************************************************/

static NLCRSMatrix* nlGetCurrentCRSMatrix() {
    NLCRSMatrix* result = NULL;
    switch(nlCurrentContext->matrix_mode) {
	case NL_STIFFNESS_MATRIX: {
	    nl_assert(nlCurrentContext->M != NULL);	    
	    nl_assert(nlCurrentContext->M->type == NL_MATRIX_CRS);
	    result = (NLCRSMatrix*)(nlCurrentContext->M);
	} break;
	case NL_MASS_MATRIX: {
	    nl_assert(nlCurrentContext->B != NULL);
	    nl_assert(nlCurrentContext->B->type == NL_MATRIX_CRS);
	    result = (NLCRSMatrix*)(nlCurrentContext->B);
	} break;
	default:
	    nl_assert_not_reached;
    }
    return result;
}

/*****************************************************************************/

NLboolean nlInitExtension(const char* extension) {
    if(!strcmp(extension, "SUPERLU")) {
        return nlInitExtension_SUPERLU();
    } else if(!strcmp(extension, "CHOLMOD")) {
        return nlInitExtension_CHOLMOD();
    } else if(!strcmp(extension, "ARPACK")) {
	/* 
	 * SUPERLU is needed by OpenNL's ARPACK driver
	 * (factorizes the matrix for the shift-invert spectral
	 *  transform).
	 */
	return nlInitExtension_SUPERLU() && nlInitExtension_ARPACK();
    } else if(!strcmp(extension, "MKL")) {
	return nlInitExtension_MKL();
    } else if(!strcmp(extension, "CUDA")) {
	return nlInitExtension_CUDA();
    }
    return NL_FALSE;
}

NLboolean nlExtensionIsInitialized(const char* extension) {
    if(!strcmp(extension, "SUPERLU")) {
        return nlExtensionIsInitialized_SUPERLU();
    } else if(!strcmp(extension, "CHOLMOD")) {
        return nlExtensionIsInitialized_CHOLMOD();
    } else if(!strcmp(extension, "ARPACK")) {
	/* 
	 * SUPERLU is needed by OpenNL's ARPACK driver
	 * (factorizes the matrix for the shift-invert spectral
	 *  transform).
	 */
	return nlExtensionIsInitialized_SUPERLU() && nlExtensionIsInitialized_ARPACK();
    } else if(!strcmp(extension, "MKL")) {
	return nlExtensionIsInitialized_MKL();
    } else if(!strcmp(extension, "CUDA")) {
	return nlExtensionIsInitialized_CUDA();
    }
    return NL_FALSE;
}

void nlInitialize(int argc, char** argv) {
    int i=0;
    char* ptr=NULL;
    char extension[255];
    /* Find all the arguments with the form:
     * nl:<extension>=true|false
     * and try to activate the corresponding extensions.
     */
    for(i=1; i<argc; ++i) {
	ptr = strstr(argv[i],"=true");
	if(!strncmp(argv[i], "nl:", 3) &&
	   (strlen(argv[i]) > 3) &&
	   (ptr != NULL)) {
	    strncpy(extension, argv[i]+3, (size_t)(ptr-argv[i]-3));
	    extension[(size_t)(ptr-argv[i]-3)] = '\0';
	    if(nlInitExtension(extension)) {
		nl_fprintf(stdout,"OpenNL %s: initialized\n", extension);
	    } else {
		nl_fprintf(stderr,"OpenNL %s: could not initialize\n", extension);		
	    }
	}
    }
}

/*****************************************************************************/
/* Get/Set parameters */

void nlSolverParameterd(NLenum pname, NLdouble param) {
    nlCheckState(NL_STATE_INITIAL);
    switch(pname) {
    case NL_THRESHOLD: {
        nl_assert(param >= 0);
        nlCurrentContext->threshold = (NLdouble)param;
        nlCurrentContext->threshold_defined = NL_TRUE;
    } break;
    case NL_OMEGA: {
        nl_range_assert(param,1.0,2.0);
        nlCurrentContext->omega = (NLdouble)param;
    } break;
    default: {
        nlError("nlSolverParameterd","Invalid parameter");
        nl_assert_not_reached;
    }
    }
}

void nlSolverParameteri(NLenum pname, NLint param) {
    nlCheckState(NL_STATE_INITIAL);
    switch(pname) {
    case NL_SOLVER: {
        nlCurrentContext->solver = (NLenum)param;
    } break;
    case NL_NB_VARIABLES: {
        nl_assert(param > 0);
        nlCurrentContext->nb_variables = (NLuint)param;
    } break;
    case NL_NB_SYSTEMS: {
	nl_assert(param > 0);
	nlCurrentContext->nb_systems = (NLuint)param;
    } break;
    case NL_LEAST_SQUARES: {
        nlCurrentContext->least_squares = (NLboolean)param;
    } break;
    case NL_MAX_ITERATIONS: {
        nl_assert(param > 0);
        nlCurrentContext->max_iterations = (NLuint)param;
        nlCurrentContext->max_iterations_defined = NL_TRUE;
    } break;
    case NL_SYMMETRIC: {
        nlCurrentContext->symmetric = (NLboolean)param;        
    } break;
    case NL_INNER_ITERATIONS: {
        nl_assert(param > 0);
        nlCurrentContext->inner_iterations = (NLuint)param;
    } break;
    case NL_PRECONDITIONER: {
        nlCurrentContext->preconditioner = (NLuint)param;
        nlCurrentContext->preconditioner_defined = NL_TRUE;
    } break;
    default: {
        nlError("nlSolverParameteri","Invalid parameter");
        nl_assert_not_reached;
    }
    }
}

void nlGetBooleanv(NLenum pname, NLboolean* params) {
    switch(pname) {
    case NL_LEAST_SQUARES: {
        *params = nlCurrentContext->least_squares;
    } break;
    case NL_SYMMETRIC: {
        *params = nlCurrentContext->symmetric;
    } break;
    default: {
        nlError("nlGetBooleanv","Invalid parameter");
        nl_assert_not_reached;
    } 
    }
}

void nlGetDoublev(NLenum pname, NLdouble* params) {
    switch(pname) {
    case NL_THRESHOLD: {
        *params = nlCurrentContext->threshold;
    } break;
    case NL_OMEGA: {
        *params = nlCurrentContext->omega;
    } break;
    case NL_ERROR: {
        *params = nlCurrentContext->error;
    } break;
    case NL_ELAPSED_TIME: {
        *params = nlCurrentContext->elapsed_time;        
    } break;
    case NL_GFLOPS: {
        if(nlCurrentContext->elapsed_time == 0) {
            *params = 0.0;
        } else {
            *params = (NLdouble)(nlCurrentContext->flops) /
                (nlCurrentContext->elapsed_time * 1e9);
        }
    } break;
    default: {
        nlError("nlGetDoublev","Invalid parameter");
        nl_assert_not_reached;
    } 
    }
}

void nlGetIntegerv(NLenum pname, NLint* params) {
    switch(pname) {
    case NL_SOLVER: {
        *params = (NLint)(nlCurrentContext->solver);
    } break;
    case NL_NB_VARIABLES: {
        *params = (NLint)(nlCurrentContext->nb_variables);
    } break;
    case NL_NB_SYSTEMS: {
	*params = (NLint)(nlCurrentContext->nb_systems);
    } break;
    case NL_LEAST_SQUARES: {
        *params = (NLint)(nlCurrentContext->least_squares);
    } break;
    case NL_MAX_ITERATIONS: {
        *params = (NLint)(nlCurrentContext->max_iterations);
    } break;
    case NL_SYMMETRIC: {
        *params = (NLint)(nlCurrentContext->symmetric);
    } break;
    case NL_USED_ITERATIONS: {
        *params = (NLint)(nlCurrentContext->used_iterations);
    } break;
    case NL_PRECONDITIONER: {
        *params = (NLint)(nlCurrentContext->preconditioner);        
    } break;
    case NL_NNZ: {
        *params = (NLint)(nlMatrixNNZ(nlCurrentContext->M));
    } break;
    default: {
        nlError("nlGetIntegerv","Invalid parameter");
        nl_assert_not_reached;
    } 
    }
}


void nlGetIntegervL(NLenum pname, NLlong* params) {
    switch(pname) {
    case NL_SOLVER: {
        *params = (NLlong)(nlCurrentContext->solver);
    } break;
    case NL_NB_VARIABLES: {
        *params = (NLlong)(nlCurrentContext->nb_variables);
    } break;
    case NL_NB_SYSTEMS: {
	*params = (NLlong)(nlCurrentContext->nb_systems);
    } break;
    case NL_LEAST_SQUARES: {
        *params = (NLlong)(nlCurrentContext->least_squares);
    } break;
    case NL_MAX_ITERATIONS: {
        *params = (NLlong)(nlCurrentContext->max_iterations);
    } break;
    case NL_SYMMETRIC: {
        *params = (NLlong)(nlCurrentContext->symmetric);
    } break;
    case NL_USED_ITERATIONS: {
        *params = (NLlong)(nlCurrentContext->used_iterations);
    } break;
    case NL_PRECONDITIONER: {
        *params = (NLlong)(nlCurrentContext->preconditioner);        
    } break;
    case NL_NNZ: {
        *params = (NLlong)(nlMatrixNNZ(nlCurrentContext->M));
    } break;
    default: {
        nlError("nlGetIntegervL","Invalid parameter");
        nl_assert_not_reached;
    } 
    }
}

/******************************************************************************/
/* Enable / Disable */

void nlEnable(NLenum pname) {
    switch(pname) {
	case NL_NORMALIZE_ROWS: {
	    nl_assert(nlCurrentContext->state != NL_STATE_ROW);
	    nlCurrentContext->normalize_rows = NL_TRUE;
	} break;
	case NL_VERBOSE: {
	    nlCurrentContext->verbose = NL_TRUE;
	} break;
	case NL_VARIABLES_BUFFER: {
	    nlCurrentContext->user_variable_buffers = NL_TRUE;
	} break;
	case NL_NO_VARIABLES_INDIRECTION: {
	    nlCurrentContext->no_variables_indirection = NL_TRUE;	    
	} break;
    default: {
        nlError("nlEnable","Invalid parameter");        
        nl_assert_not_reached;
    }
    }
}

void nlDisable(NLenum pname) {
    switch(pname) {
	case NL_NORMALIZE_ROWS: {
	    nl_assert(nlCurrentContext->state != NL_STATE_ROW);
	    nlCurrentContext->normalize_rows = NL_FALSE;
	} break;
	case NL_VERBOSE: {
	    nlCurrentContext->verbose = NL_FALSE;
	} break;
	case NL_VARIABLES_BUFFER: {
	    nlCurrentContext->user_variable_buffers = NL_FALSE;
	} break;
	case NL_NO_VARIABLES_INDIRECTION: {
	    nlCurrentContext->no_variables_indirection = NL_FALSE;	    
	} break;
	default: {
	    nlError("nlDisable","Invalid parameter");                
	    nl_assert_not_reached;
	}
    }
}

NLboolean nlIsEnabled(NLenum pname) {
    NLboolean result = NL_FALSE;
    switch(pname) {
	case NL_NORMALIZE_ROWS: {
	    result = nlCurrentContext->normalize_rows;
	} break;
	case NL_VERBOSE: {
	    result = nlCurrentContext->verbose;
	} break;
	case NL_VARIABLES_BUFFER: {
	    result = nlCurrentContext->user_variable_buffers;
	} break;
	case NL_NO_VARIABLES_INDIRECTION: {
	    result = nlCurrentContext->no_variables_indirection;	    
	} break;
	default: {
	    nlError("nlIsEnables","Invalid parameter");
	    nl_assert_not_reached;
	}
    }
    return result;
}

/******************************************************************************/
/* NL functions */

void  nlSetFunction(NLenum pname, NLfunc param) {
    switch(pname) {
    case NL_FUNC_SOLVER:
        nlCurrentContext->solver_func = (NLSolverFunc)(param);
        nlCurrentContext->solver = NL_SOLVER_USER;	
        break;
    case NL_FUNC_MATRIX:
	nlDeleteMatrix(nlCurrentContext->M);
	nlCurrentContext->M = nlMatrixNewFromFunction(
	    nlCurrentContext->n, nlCurrentContext->n,
	    (NLMatrixFunc)param
	);
        break;
    case NL_FUNC_PRECONDITIONER:
	nlDeleteMatrix(nlCurrentContext->P);
	nlCurrentContext->P = nlMatrixNewFromFunction(
	    nlCurrentContext->n, nlCurrentContext->n,
	    (NLMatrixFunc)param
	);
        nlCurrentContext->preconditioner = NL_PRECOND_USER;
        break;
    case NL_FUNC_PROGRESS:
        nlCurrentContext->progress_func = (NLProgressFunc)(param);
        break;
    default:
        nlError("nlSetFunction","Invalid parameter");        
        nl_assert_not_reached;
    }
}

void nlGetFunction(NLenum pname, NLfunc* param) {
    switch(pname) {
    case NL_FUNC_SOLVER:
        *param = (NLfunc)(nlCurrentContext->solver_func);
        break;
    case NL_FUNC_MATRIX:
        *param = (NLfunc)(nlMatrixGetFunction(nlCurrentContext->M));
        break;
    case NL_FUNC_PRECONDITIONER:
        *param = (NLfunc)(nlMatrixGetFunction(nlCurrentContext->P));
        break;
    default:
        nlError("nlGetFunction","Invalid parameter");                
        nl_assert_not_reached;
    }
}

/******************************************************************************/
/* Get/Set Lock/Unlock variables */

void nlSetVariable(NLuint index, NLdouble value) {
    nlCheckState(NL_STATE_SYSTEM);
    nl_debug_range_assert(index, 0, nlCurrentContext->nb_variables - 1);
    NL_BUFFER_ITEM(nlCurrentContext->variable_buffer[0],index) = value;
}

void nlMultiSetVariable(NLuint index, NLuint system, NLdouble value) {
    nlCheckState(NL_STATE_SYSTEM);
    nl_debug_range_assert(index, 0, nlCurrentContext->nb_variables-1);
    nl_debug_range_assert(system, 0, nlCurrentContext->nb_systems-1);    
    NL_BUFFER_ITEM(nlCurrentContext->variable_buffer[system],index) = value;
}

NLdouble nlGetVariable(NLuint index) {
    nl_assert(nlCurrentContext->state != NL_STATE_INITIAL);
    nl_debug_range_assert(index, 0, nlCurrentContext->nb_variables - 1);
    return NL_BUFFER_ITEM(nlCurrentContext->variable_buffer[0],index);
}

NLdouble nlMultiGetVariable(NLuint index, NLuint system) {
    nl_assert(nlCurrentContext->state != NL_STATE_INITIAL);
    nl_debug_range_assert(index, 0, nlCurrentContext->nb_variables-1);
    nl_debug_range_assert(system, 0, nlCurrentContext->nb_systems-1);
    return NL_BUFFER_ITEM(nlCurrentContext->variable_buffer[system],index);    
}


void nlLockVariable(NLuint index) {
    nlCheckState(NL_STATE_SYSTEM);
    nl_debug_range_assert(index, 0, nlCurrentContext->nb_variables - 1);
    nlCurrentContext->variable_is_locked[index] = NL_TRUE;
}

void nlUnlockVariable(NLuint index) {
    nlCheckState(NL_STATE_SYSTEM);
    nl_debug_range_assert(index, 0, nlCurrentContext->nb_variables - 1);
    nlCurrentContext->variable_is_locked[index] = NL_FALSE;
}

NLboolean nlVariableIsLocked(NLuint index) {
    nl_assert(nlCurrentContext->state != NL_STATE_INITIAL);
    nl_debug_range_assert(index, 0, nlCurrentContext->nb_variables - 1);
    return nlCurrentContext->variable_is_locked[index];
}

/******************************************************************************/
/* System construction */

static void nlVariablesToVector() {
    NLuint n=nlCurrentContext->n;
    NLuint k,i,index;
    NLdouble value;
    
    nl_assert(nlCurrentContext->x != NULL);
    for(k=0; k<nlCurrentContext->nb_systems; ++k) {
	for(i=0; i<nlCurrentContext->nb_variables; ++i) {
	    if(!nlCurrentContext->variable_is_locked[i]) {
		index = nlCurrentContext->variable_index[i];
		nl_assert(index < nlCurrentContext->n);		
		value = NL_BUFFER_ITEM(nlCurrentContext->variable_buffer[k],i);
		nlCurrentContext->x[index+k*n] = value;
	    }
	}
    }
}

static void nlVectorToVariables() {
    NLuint n=nlCurrentContext->n;
    NLuint k,i,index;
    NLdouble value;

    nl_assert(nlCurrentContext->x != NULL);
    for(k=0; k<nlCurrentContext->nb_systems; ++k) {
	for(i=0; i<nlCurrentContext->nb_variables; ++i) {
	    if(!nlCurrentContext->variable_is_locked[i]) {
		index = nlCurrentContext->variable_index[i];
		nl_assert(index < nlCurrentContext->n);
		value = nlCurrentContext->x[index+k*n];
		NL_BUFFER_ITEM(nlCurrentContext->variable_buffer[k],i) = value;
	    }
	}
    }
}


static void nlBeginSystem() {
    NLuint k;
    
    nlTransition(NL_STATE_INITIAL, NL_STATE_SYSTEM);
    nl_assert(nlCurrentContext->nb_variables > 0);

    nlCurrentContext->variable_buffer = NL_NEW_ARRAY(
	NLBufferBinding, nlCurrentContext->nb_systems
    );
    
    if(nlCurrentContext->user_variable_buffers) {
	nlCurrentContext->variable_value = NULL;
    } else {
	nlCurrentContext->variable_value = NL_NEW_ARRAY(
	    NLdouble,
	    nlCurrentContext->nb_variables * nlCurrentContext->nb_systems
	);
	for(k=0; k<nlCurrentContext->nb_systems; ++k) {
	    nlCurrentContext->variable_buffer[k].base_address =
		nlCurrentContext->variable_value +
		k * nlCurrentContext->nb_variables;
	    nlCurrentContext->variable_buffer[k].stride = sizeof(NLdouble);
	}
    }

    if(!nlCurrentContext->no_variables_indirection) {
	nlCurrentContext->variable_is_locked = NL_NEW_ARRAY(
	    NLboolean, nlCurrentContext->nb_variables
	);
	nlCurrentContext->variable_index = NL_NEW_ARRAY(
	    NLuint, nlCurrentContext->nb_variables
	);
    }
    nlCurrentContext->has_matrix_pattern = NL_FALSE;
}

static void nlEndSystem() {
    nlTransition(NL_STATE_MATRIX_CONSTRUCTED, NL_STATE_SYSTEM_CONSTRUCTED);    
}

static void nlInitializeMSystem() {
    NLuint i;
    NLuint n = nlCurrentContext->nb_variables;

    if(!nlCurrentContext->no_variables_indirection) {

        /* Stores only 1 value per system (rhs of current row). */
	nlCurrentContext->right_hand_side = NL_NEW_ARRAY(
	    double, nlCurrentContext->nb_systems
	);

	n = 0;
	for(i=0; i<nlCurrentContext->nb_variables; i++) {
	    if(!nlCurrentContext->variable_is_locked[i]) {
		nlCurrentContext->variable_index[i] = n;
		n++;
	    } else {
		nlCurrentContext->variable_index[i] = (NLuint)~0;
	    }
	}
	
	nlCurrentContext->x = NL_NEW_ARRAY(
	    NLdouble, n*nlCurrentContext->nb_systems
	);

	nlCurrentContext->n = n;	
	nlVariablesToVector();

	nlRowColumnConstruct(&nlCurrentContext->af);
	nlRowColumnConstruct(&nlCurrentContext->al);
    }

    nlCurrentContext->b = NL_NEW_ARRAY(
	NLdouble, n*nlCurrentContext->nb_systems
    );
    
    nlCurrentContext->n = n;
    nlCurrentContext->current_row = 0;

    /*
     * If the user trusts OpenNL and has left solver as NL_SOLVER_DEFAULT,
     * then we setup reasonable parameters for him.
     */
    if(nlCurrentContext->solver == NL_SOLVER_DEFAULT) {
        if(nlCurrentContext->least_squares || nlCurrentContext->symmetric) {
            nlCurrentContext->solver = NL_CG;
            if(!nlCurrentContext->preconditioner_defined) {
                nlCurrentContext->preconditioner = NL_PRECOND_JACOBI;
            }
        } else {
            nlCurrentContext->solver = NL_BICGSTAB;
        }
        if(!nlCurrentContext->max_iterations_defined) {
            nlCurrentContext->max_iterations = n*5;
        }
        if(!nlCurrentContext->threshold_defined) {
            nlCurrentContext->threshold = 1e-6;
        }
    }

    /* a least squares problem results in a symmetric matrix */
    if(nlCurrentContext->least_squares) {
        nlCurrentContext->symmetric = NL_TRUE;
    }
}

static void nlInitializeMCRSMatrixPattern() {
    NLuint n = nlCurrentContext->n;
    nlCurrentContext->M = (NLMatrix)(NL_NEW(NLCRSMatrix));
    if(nlCurrentContext->symmetric) {
	nlCRSMatrixConstructPatternSymmetric(
	    (NLCRSMatrix*)(nlCurrentContext->M), n
	);
    } else {
	nlCRSMatrixConstructPattern(
	    (NLCRSMatrix*)(nlCurrentContext->M), n, n
	);
    }
    nlCurrentContext->has_matrix_pattern = NL_TRUE;
}

static void nlInitializeMSparseMatrix() {
    NLuint n = nlCurrentContext->n;
    NLenum storage = NL_MATRIX_STORE_ROWS;

    /* SSOR preconditioner requires rows and columns */
    if(nlCurrentContext->preconditioner == NL_PRECOND_SSOR) {
        storage = (storage | NL_MATRIX_STORE_COLUMNS);
    }

    if(
	nlCurrentContext->symmetric &&
        nlCurrentContext->preconditioner == NL_PRECOND_SSOR 
    ) {
	/* 
	 * For now, only used with SSOR preconditioner, because
	 * for other modes it is either unsupported (SUPERLU) or
	 * causes performance loss (non-parallel sparse SpMV)
	 */
        storage = (storage | NL_MATRIX_STORE_SYMMETRIC);
    }

    nlCurrentContext->M = (NLMatrix)(NL_NEW(NLSparseMatrix));
    nlSparseMatrixConstruct(
	     (NLSparseMatrix*)(nlCurrentContext->M), n, n, storage
    );
}

static void nlEndMatrix() {
#ifdef NL_DEBUG    
    NLuint i;
    NLuint_big jj;
    NLCRSMatrix* M = NULL;
#endif    
    nlTransition(NL_STATE_MATRIX, NL_STATE_MATRIX_CONSTRUCTED);    

    if(!nlCurrentContext->no_variables_indirection) {
	nlRowColumnClear(&nlCurrentContext->af);
	nlRowColumnClear(&nlCurrentContext->al);
    }
    
    if(!nlCurrentContext->least_squares) {
        nl_assert(
            nlCurrentContext->ij_coefficient_called || (
                nlCurrentContext->current_row == 
                nlCurrentContext->n
            )
        );
    }

#ifdef NL_DEBUG
    if(nlCurrentContext->has_matrix_pattern) {
	M = nlGetCurrentCRSMatrix();
	for(i=0; i<M->m; ++i) {
	    for(jj=M->rowptr[i]; jj<M->rowptr[i+1]; ++jj) {
		/*
		 * Test that all coefficients were created.
		 * This assertion test will fail whenever 
		 * a length has fewer coefficient than specified
		 * with nlSetRowLength()
		 */
		nl_assert(M->colind[jj] != (NLuint)(-1));
	    }
	}
    }
#endif    
}

static void nlBeginRow() {
    nlTransition(NL_STATE_MATRIX, NL_STATE_ROW);
    nlRowColumnZero(&nlCurrentContext->af);
    nlRowColumnZero(&nlCurrentContext->al);
}

static void nlScaleRow(NLdouble s) {
    NLRowColumn*    af = &nlCurrentContext->af;
    NLRowColumn*    al = &nlCurrentContext->al;
    NLuint nf            = af->size;
    NLuint nl            = al->size;
    NLuint i,k;
    for(i=0; i<nf; i++) {
        af->coeff[i].value *= s;
    }
    for(i=0; i<nl; i++) {
        al->coeff[i].value *= s;
    }
    for(k=0; k<nlCurrentContext->nb_systems; ++k) {
	nlCurrentContext->right_hand_side[k] *= s;
    }
}

static void nlNormalizeRow(NLdouble weight) {
    NLRowColumn*    af = &nlCurrentContext->af;
    NLRowColumn*    al = &nlCurrentContext->al;
    NLuint nf            = af->size;
    NLuint nl            = al->size;
    NLuint i;
    NLdouble norm = 0.0;
    for(i=0; i<nf; i++) {
        norm += af->coeff[i].value * af->coeff[i].value;
    }
    for(i=0; i<nl; i++) {
        norm += al->coeff[i].value * al->coeff[i].value;
    }
    norm = sqrt(norm);
    nlScaleRow(weight / norm);
}

static void nlEndRow() {
    NLRowColumn*    af = &nlCurrentContext->af;
    NLRowColumn*    al = &nlCurrentContext->al;
    NLSparseMatrix* M  = nlGetCurrentSparseMatrix();
    NLdouble* b        = nlCurrentContext->b;
    NLuint nf          = af->size;
    NLuint nl          = al->size;
    NLuint n           = nlCurrentContext->n;
    NLuint current_row = nlCurrentContext->current_row;
    NLuint i,j,jj;
    NLdouble S;
    NLuint k;
    nlTransition(NL_STATE_ROW, NL_STATE_MATRIX);

    if(nlCurrentContext->normalize_rows) {
        nlNormalizeRow(nlCurrentContext->row_scaling);
    } else if(nlCurrentContext->row_scaling != 1.0) {
        nlScaleRow(nlCurrentContext->row_scaling);
    }
    /*
     * if least_squares : we want to solve
     * A'A x = A'b
     */

    if(nlCurrentContext->least_squares) {
        for(i=0; i<nf; i++) {
            for(j=0; j<nf; j++) {
                nlSparseMatrixAdd(
                    M, af->coeff[i].index, af->coeff[j].index,
                    af->coeff[i].value * af->coeff[j].value
                );
            }
        }
	for(k=0; k<nlCurrentContext->nb_systems; ++k) {
	    S = -nlCurrentContext->right_hand_side[k];
	    for(jj=0; jj<nl; ++jj) {
		j = al->coeff[jj].index;
		S += al->coeff[jj].value *
		    NL_BUFFER_ITEM(nlCurrentContext->variable_buffer[k],j);
	    }
	    for(jj=0; jj<nf; jj++) {
		b[ k*n+af->coeff[jj].index ] -= af->coeff[jj].value * S;
	    }
	}
    } else {
        for(jj=0; jj<nf; ++jj) {
            nlSparseMatrixAdd(
                M, current_row, af->coeff[jj].index, af->coeff[jj].value
            );
        }
	for(k=0; k<nlCurrentContext->nb_systems; ++k) {
	    b[k*n+current_row] = nlCurrentContext->right_hand_side[k];
	    for(jj=0; jj<nl; ++jj) {
		j = al->coeff[jj].index;
		b[k*n+current_row] -= al->coeff[jj].value *
		    NL_BUFFER_ITEM(nlCurrentContext->variable_buffer[k],j);
	    }
	}
    }
    nlCurrentContext->current_row++;
    for(k=0; k<nlCurrentContext->nb_systems; ++k) {
	nlCurrentContext->right_hand_side[k] = 0.0;
    }
    nlCurrentContext->row_scaling = 1.0;
}

void nlCoefficient(NLuint index, NLdouble value) {
    nlCheckState(NL_STATE_ROW);
    nl_debug_range_assert(index, 0, nlCurrentContext->nb_variables - 1);
    if(nlCurrentContext->variable_is_locked[index]) {
	/* 
	 * Note: in al, indices are NLvariable indices, 
	 * within [0..nb_variables-1]
	 */
        nlRowColumnAppend(&(nlCurrentContext->al), index, value);
    } else {
	/*
	 * Note: in af, indices are system indices, 
	 * within [0..n-1]
	 */
        nlRowColumnAppend(
	    &(nlCurrentContext->af),
	    nlCurrentContext->variable_index[index], value
	);
    }
}

void nlAddIJCoefficient(NLuint i, NLuint j, NLdouble value) {
#ifdef NL_DEBUG
    NLuint kk;
    if(nlCurrentContext->variable_is_locked != NULL) {    
	for(kk=0; kk<nlCurrentContext->nb_variables; ++kk) {
	    nl_debug_assert(!nlCurrentContext->variable_is_locked[kk]);
	}
    }
#endif    
    nlCheckState(NL_STATE_MATRIX);
    nl_debug_range_assert(i, 0, nlCurrentContext->nb_variables - 1);
    nl_debug_range_assert(j, 0, nlCurrentContext->nb_variables - 1);
    if(nlCurrentContext->has_matrix_pattern) {
	nlCRSMatrixAdd(
	    nlGetCurrentCRSMatrix(), i, j, value
        );
    } else {
	nlSparseMatrixAdd(
	    nlGetCurrentSparseMatrix(), i, j, value
        );
    }
    nlCurrentContext->ij_coefficient_called = NL_TRUE;
}

void nlAddIRightHandSide(NLuint i, NLdouble value) {
#ifdef NL_DEBUG
    NLuint kk;
    if(nlCurrentContext->variable_is_locked != NULL) {        
	for(kk=0; kk<nlCurrentContext->nb_variables; ++kk) {
	    nl_debug_assert(!nlCurrentContext->variable_is_locked[kk]);
	}
    }
#endif
    nlCheckState(NL_STATE_MATRIX);
    nl_debug_range_assert(i, 0, nlCurrentContext->nb_variables - 1);
    nlCurrentContext->b[i] += value;
    nlCurrentContext->ij_coefficient_called = NL_TRUE;
}

void nlMultiAddIRightHandSide(NLuint i, NLuint k, NLdouble value) {
    NLuint n = nlCurrentContext->n;
#ifdef NL_DEBUG
    NLuint kk;
    if(nlCurrentContext->variable_is_locked != NULL) {        
	for(kk=0; kk<nlCurrentContext->nb_variables; ++kk) {
	    nl_debug_assert(!nlCurrentContext->variable_is_locked[kk]);
	}
    }
#endif
    nlCheckState(NL_STATE_MATRIX);
    nl_debug_range_assert(i, 0, nlCurrentContext->nb_variables - 1);
    nl_debug_range_assert(k, 0, nlCurrentContext->nb_systems - 1);
    nlCurrentContext->b[i + k*n] += value;
    nlCurrentContext->ij_coefficient_called = NL_TRUE;
}

void nlRightHandSide(NLdouble value) {
    nlCurrentContext->right_hand_side[0] = value;
}

void nlMultiRightHandSide(NLuint k, NLdouble value) {
    nl_debug_range_assert(k, 0, nlCurrentContext->nb_systems - 1);
    nlCurrentContext->right_hand_side[k] = value;
}

void nlRowScaling(NLdouble value) {
    nlCheckState(NL_STATE_MATRIX);
    nlCurrentContext->row_scaling = value;
}

void nlBegin(NLenum prim) {
    switch(prim) {
    case NL_SYSTEM: {
        nlBeginSystem();
    } break;
    case NL_MATRIX_PATTERN: {
	nlTransition(NL_STATE_SYSTEM, NL_STATE_MATRIX_PATTERN);
	nlInitializeMSystem();
	nlInitializeMCRSMatrixPattern();
    } break;
    case NL_MATRIX: {
	nlTransition(NL_STATE_SYSTEM, NL_STATE_MATRIX);
	if(
	    nlCurrentContext->matrix_mode == NL_STIFFNESS_MATRIX &&
	    nlCurrentContext->M == NULL
	) {
	    nlInitializeMSystem();
	    nlInitializeMSparseMatrix();
	}
    } break;
    case NL_ROW: {
        nlBeginRow();
    } break;
    default: {
        nl_assert_not_reached;
    }
    }
}

void nlEnd(NLenum prim) {
    switch(prim) {
    case NL_SYSTEM: {
        nlEndSystem();
    } break;
    case NL_MATRIX: {
        nlEndMatrix();
    } break;
    case NL_MATRIX_PATTERN: {
	nlTransition(NL_STATE_MATRIX_PATTERN, NL_STATE_SYSTEM);
	nlCRSMatrixPatternCompile(nlGetCurrentCRSMatrix());
    } break;
    case NL_ROW: {
        nlEndRow();
    } break;
    default: {
        nl_assert_not_reached;
    }
    }
}

/******************************************************************************/
/* nlSolve() driver routine */

NLboolean nlSolve() {
    NLboolean result;
    nlCheckState(NL_STATE_SYSTEM_CONSTRUCTED);
    nlCurrentContext->start_time = nlCurrentTime();
    nlCurrentContext->elapsed_time = 0.0;
    nlCurrentContext->flops = 0;    
    result = nlCurrentContext->solver_func();
    if(!nlCurrentContext->no_variables_indirection) {
	nlVectorToVariables();
    }
    nlCurrentContext->elapsed_time = nlCurrentTime() - nlCurrentContext->start_time;
    nlTransition(NL_STATE_SYSTEM_CONSTRUCTED, NL_STATE_SOLVED);
    return result;
}

void nlUpdateRightHandSide(NLdouble* values) {
    /*
     * If we are in the solved state, get back to the
     * constructed state.
     */
    nl_assert(nlCurrentContext->nb_systems == 1);
    if(nlCurrentContext->state == NL_STATE_SOLVED) {
        nlTransition(NL_STATE_SOLVED, NL_STATE_SYSTEM_CONSTRUCTED);
    }
    nlCheckState(NL_STATE_SYSTEM_CONSTRUCTED);
    memcpy(nlCurrentContext->x, values, nlCurrentContext->n * sizeof(double));
}

/******************************************************************************/
/* Buffers management */

void nlBindBuffer(
    NLenum buffer, NLuint k, void* addr, NLuint stride
) {
    nlCheckState(NL_STATE_SYSTEM);    
    nl_assert(nlIsEnabled(buffer));
    nl_assert(buffer == NL_VARIABLES_BUFFER);
    nl_assert(k<nlCurrentContext->nb_systems);
    if(stride == 0) {
	stride = sizeof(NLdouble);
    }
    nlCurrentContext->variable_buffer[k].base_address = addr;
    nlCurrentContext->variable_buffer[k].stride = stride;
}

/******************************************************************************/
/* Eigen solver */

void nlMatrixMode(NLenum matrix) {
    NLuint n = 0;
    NLuint i;
    nl_assert(
	nlCurrentContext->state == NL_STATE_SYSTEM ||
	nlCurrentContext->state == NL_STATE_MATRIX_CONSTRUCTED
    );
    nlCurrentContext->state = NL_STATE_SYSTEM;
    nlCurrentContext->matrix_mode = matrix;
    nlCurrentContext->current_row = 0;
    nlCurrentContext->ij_coefficient_called = NL_FALSE;
    switch(matrix) {
	case NL_STIFFNESS_MATRIX: {
	    /* Stiffness matrix is already constructed. */
	} break ;
	case NL_MASS_MATRIX: {
	    if(nlCurrentContext->B == NULL) {
		for(i=0; i<nlCurrentContext->nb_variables; ++i) {
		    if(!nlCurrentContext->variable_is_locked[i]) {
			++n;
		    }
		}
		nlCurrentContext->B = (NLMatrix)(NL_NEW(NLSparseMatrix));
		nlSparseMatrixConstruct(
		    (NLSparseMatrix*)(nlCurrentContext->B),
		    n, n, NL_MATRIX_STORE_ROWS
		);
	    }
	} break ;
	default:
	    nl_assert_not_reached;
    }
}


void nlEigenSolverParameterd(
    NLenum pname, NLdouble val
) {
    switch(pname) {
	case NL_EIGEN_SHIFT: {
	    nlCurrentContext->eigen_shift =  val;
	} break;
	case NL_EIGEN_THRESHOLD: {
	    nlSolverParameterd(pname, val);
	} break;
	default:
	    nl_assert_not_reached;
    }
}

void nlEigenSolverParameteri(
    NLenum pname, NLint val
) {
    switch(pname) {
	case NL_EIGEN_SOLVER: {
	    nlCurrentContext->eigen_solver = (NLenum)val;
	} break;
	case NL_SYMMETRIC:
	case NL_NB_VARIABLES:	    
	case NL_NB_EIGENS:
	case NL_EIGEN_MAX_ITERATIONS: {
	    nlSolverParameteri(pname, val);
	} break;
	default:
	    nl_assert_not_reached;
    }
}

void nlEigenSolve() {
    if(nlCurrentContext->eigen_value == NULL) {
	nlCurrentContext->eigen_value = NL_NEW_ARRAY(
	    NLdouble,nlCurrentContext->nb_systems
	);
    }
    
    nlMatrixCompress(&nlCurrentContext->M);
    if(nlCurrentContext->B != NULL) {
	nlMatrixCompress(&nlCurrentContext->B);
    }
    
    switch(nlCurrentContext->eigen_solver) {
	case NL_ARPACK_EXT:
	    nlEigenSolve_ARPACK();
	    break;
	default:
	    nl_assert_not_reached;
    }
}

double nlGetEigenValue(NLuint i) {
    nl_debug_assert(i < nlCurrentContext->nb_variables);
    return nlCurrentContext->eigen_value[i];
}

void nlSetRowLength(NLuint i, NLuint m) {
    NLCRSMatrix* M = nlGetCurrentCRSMatrix();
    nl_assert(M != NULL);
    nlCRSMatrixPatternSetRowLength(M, i, m);
}
