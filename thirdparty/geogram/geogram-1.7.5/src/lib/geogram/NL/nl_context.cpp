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

#include "nl_context.h"
#include "nl_iterative_solvers.h"
#include "nl_preconditioners.h"
#include "nl_superlu.h"
#include "nl_cholmod.h"
#include "nl_matrix.h"
#include "nl_mkl.h"
#include "nl_cuda.h"

thread_local NLContextStruct* nlCurrentContext = NULL;

NLContext nlNewContext() {
    NLContextStruct* result     = NL_NEW(NLContextStruct);
    result->state               = NL_STATE_INITIAL;
    result->solver              = NL_SOLVER_DEFAULT;
    result->max_iterations      = 100;
    result->threshold           = 1e-6;
    result->omega               = 1.5;
    result->row_scaling         = 1.0;
    result->inner_iterations    = 5;
    result->solver_func         = nlDefaultSolver;
    result->progress_func       = NULL;
    result->verbose             = NL_FALSE;
    result->nb_systems          = 1;
    result->matrix_mode         = NL_STIFFNESS_MATRIX;
    nlMakeCurrent(result);
    return result;
}

void nlDeleteContext(NLContext context_in) {
    NLContextStruct* context = (NLContextStruct*)(context_in);
    if(nlCurrentContext == context) {
        nlCurrentContext = NULL;
    }

    nlDeleteMatrix(context->M);
    context->M = NULL;

    nlDeleteMatrix(context->P);
    context->P = NULL;

    nlDeleteMatrix(context->B);
    context->B = NULL;
    
    nlRowColumnDestroy(&context->af);
    nlRowColumnDestroy(&context->al);

    NL_DELETE_ARRAY(context->variable_value);
    NL_DELETE_ARRAY(context->variable_buffer);
    NL_DELETE_ARRAY(context->variable_is_locked);
    NL_DELETE_ARRAY(context->variable_index);
    
    NL_DELETE_ARRAY(context->x);
    NL_DELETE_ARRAY(context->b);
    NL_DELETE_ARRAY(context->right_hand_side);

    NL_DELETE_ARRAY(context->eigen_value);
    
#ifdef NL_PARANOID
    NL_CLEAR(NLContextStruct, context);
#endif
    NL_DELETE(context);
}

void nlMakeCurrent(NLContext context) {
    nlCurrentContext = (NLContextStruct*)(context);
}

NLContext nlGetCurrent() {
    return nlCurrentContext;
}

/************************************************************************/
/* Finite state automaton   */

void nlCheckState(NLenum state) {
    nl_assert(nlCurrentContext->state == state);
}

void nlTransition(NLenum from_state, NLenum to_state) {
    nlCheckState(from_state);
    nlCurrentContext->state = to_state;
}

/************************************************************************/
/* Preconditioner setup and default solver */

static void nlSetupPreconditioner() {
    /* Check compatibility between solver and preconditioner */
    if(
        nlCurrentContext->solver == NL_BICGSTAB && 
        nlCurrentContext->preconditioner == NL_PRECOND_SSOR
    ) {
        nlWarning(
            "nlSolve", 
            "cannot use SSOR preconditioner with non-symmetric matrix, "
	    "switching to Jacobi"
        );
        nlCurrentContext->preconditioner = NL_PRECOND_JACOBI;        
    }
    if(
        nlCurrentContext->solver == NL_GMRES && 
        nlCurrentContext->preconditioner != NL_PRECOND_NONE
    ) {
        nlWarning("nlSolve", "Preconditioner not implemented yet for GMRES");
        nlCurrentContext->preconditioner = NL_PRECOND_NONE;        
    }
    if(
        nlCurrentContext->solver == NL_SUPERLU_EXT && 
        nlCurrentContext->preconditioner != NL_PRECOND_NONE
    ) {
        nlWarning("nlSolve", "Preconditioner not implemented yet for SUPERLU");
        nlCurrentContext->preconditioner = NL_PRECOND_NONE;        
    }
    if(
        nlCurrentContext->solver == NL_CHOLMOD_EXT && 
        nlCurrentContext->preconditioner != NL_PRECOND_NONE
    ) {
        nlWarning("nlSolve", "Preconditioner not implemented yet for CHOLMOD");
        nlCurrentContext->preconditioner = NL_PRECOND_NONE;        
    }
    if(
        nlCurrentContext->solver == NL_PERM_SUPERLU_EXT && 
        nlCurrentContext->preconditioner != NL_PRECOND_NONE
    ) {
        nlWarning(
	    "nlSolve", "Preconditioner not implemented yet for PERMSUPERLU"
	);
        nlCurrentContext->preconditioner = NL_PRECOND_NONE;        
    }
    if(
        nlCurrentContext->solver == NL_SYMMETRIC_SUPERLU_EXT && 
        nlCurrentContext->preconditioner != NL_PRECOND_NONE
    ) {
        nlWarning(
	    "nlSolve", "Preconditioner not implemented yet for PERMSUPERLU"
	);
        nlCurrentContext->preconditioner = NL_PRECOND_NONE;        
    }

    nlDeleteMatrix(nlCurrentContext->P);
    nlCurrentContext->P = NULL;
    
    switch(nlCurrentContext->preconditioner) {
    case NL_PRECOND_NONE:
        break;
    case NL_PRECOND_JACOBI:
	nlCurrentContext->P = nlNewJacobiPreconditioner(nlCurrentContext->M);
        break;
    case NL_PRECOND_SSOR:
	nlCurrentContext->P = nlNewSSORPreconditioner(
	    nlCurrentContext->M,nlCurrentContext->omega
	);	
        break;
    case NL_PRECOND_USER:
        break;
    default:
        nl_assert_not_reached;
    }

    if(nlCurrentContext->preconditioner != NL_PRECOND_SSOR) {
        if(getenv("NL_LOW_MEM") == NULL) {
            nlMatrixCompress(&nlCurrentContext->M);
        }
    }
}

static NLboolean nlSolveDirect() {
    NLdouble* b = nlCurrentContext->b;
    NLdouble* x = nlCurrentContext->x;
    NLuint n = nlCurrentContext->n;
    NLuint k;
    
    NLMatrix F = nlMatrixFactorize(
	nlCurrentContext->M, nlCurrentContext->solver
    );
    if(F == NULL) {
	return NL_FALSE;
    }
    for(k=0; k<nlCurrentContext->nb_systems; ++k) {
	if(nlCurrentContext->no_variables_indirection) {
	    x = (double*)nlCurrentContext->variable_buffer[k].base_address;
	    nl_assert(nlCurrentContext->variable_buffer[k].stride == sizeof(double));
	}
	nlMultMatrixVector(F, b, x);
	b += n;
	x += n;
    }
    nlDeleteMatrix(F);
    return NL_TRUE;
}

static NLboolean nlSolveIterative() {
    NLboolean use_CUDA = NL_FALSE;
    NLdouble* b = nlCurrentContext->b;
    NLdouble* x = nlCurrentContext->x;
    NLuint n = nlCurrentContext->n;
    NLuint k;
    NLBlas_t blas = nlHostBlas();
    NLMatrix M = nlCurrentContext->M;
    NLMatrix P = nlCurrentContext->P;

    /*
     * For CUDA: it is implemented for
     *   all iterative solvers except GMRES
     *   Jacobi preconditioner
     */
    if(nlExtensionIsInitialized_CUDA() &&
       (nlCurrentContext->solver != NL_GMRES) && 
       (nlCurrentContext->preconditioner == NL_PRECOND_NONE ||
	nlCurrentContext->preconditioner == NL_PRECOND_JACOBI)
    ) {
	if(nlCurrentContext->verbose) { 
	    nl_printf("Using CUDA\n");
	} 
	use_CUDA = NL_TRUE;
	blas = nlCUDABlas();
	if(nlCurrentContext->preconditioner == NL_PRECOND_JACOBI) {
	    P = nlCUDAJacobiPreconditionerNewFromCRSMatrix(M);
	}
	M = nlCUDAMatrixNewFromCRSMatrix(M);
    }

    /* 
     * We do not count CUDA transfers and CUDA matrix construction
     * when estimating GFlops
     */
    nlCurrentContext->start_time = nlCurrentTime();     
    nlBlasResetStats(blas);
    
    for(k=0; k<nlCurrentContext->nb_systems; ++k) {

	if(nlCurrentContext->no_variables_indirection) {
	    x = (double*)nlCurrentContext->variable_buffer[k].base_address;
	    nl_assert(nlCurrentContext->variable_buffer[k].stride == sizeof(double));
	}
	
	nlSolveSystemIterative(
	    blas,
	    M,
	    P,
	    b,
	    x,
	    nlCurrentContext->solver,
	    nlCurrentContext->threshold,
	    nlCurrentContext->max_iterations,
	    nlCurrentContext->inner_iterations
	);

	b += n;	
	x += n;
    }

    nlCurrentContext->flops += blas->flops;
    
    if(use_CUDA) {
	nlDeleteMatrix(M);
	nlDeleteMatrix(P);
    }
    
    return NL_TRUE;
}

NLboolean nlDefaultSolver() {
    NLboolean result = NL_TRUE;
    nlSetupPreconditioner();
    switch(nlCurrentContext->solver) {
	case NL_CG:
	case NL_BICGSTAB:
	case NL_GMRES: {
	    result = nlSolveIterative();
	} break;

	case NL_SUPERLU_EXT: 
	case NL_PERM_SUPERLU_EXT: 
	case NL_SYMMETRIC_SUPERLU_EXT: 
	case NL_CHOLMOD_EXT: {
	    result = nlSolveDirect();
	} break;
	default:
	    nl_assert_not_reached;
    }
    return result;
}
