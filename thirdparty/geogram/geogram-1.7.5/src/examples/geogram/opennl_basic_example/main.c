/*
 *  Copyright (c) 2004-2014, Bruno Levy
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
 *     Bruno.Levy@inria.fr
 *     http://www.loria.fr/~levy
 *
 *     ALICE Project
 *     LORIA, INRIA Lorraine, 
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX 
 *     FRANCE
 *
 */

#include <geogram/NL/nl.h>
#include <stdio.h>
#include <string.h>

/**
 * \brief Tests OpenNL solve with 
 *  a simple linear system.
 * \details 
 *  Solve \f$ \left[ \begin{array}{ll} 1 & 2 \\ 3 & 4 \end{array} \right] 
 *    \left[ \begin{array}{l} x \\ y \end{array} \right]
 *  = \left[ \begin{array}{l} 5 \\ 6 \end{array} \right] \f$
 */
static void test_simple_linear_solve(NLint solver) {
    NLboolean symmetric = NL_FALSE;
    
    printf("\n");    
    printf("Testing linear solve\n");
    printf("====================\n");


    
    switch(solver) {
    case NL_SOLVER_DEFAULT:
        printf("Using default solver (BiCGStab)\n");
        break;
    case NL_CG:
        printf("Using CG\n");
	symmetric = NL_TRUE;
	break;
    case NL_GMRES:
        printf("Using GMRES\n");        
        break;
    case NL_BICGSTAB:
        printf("Using BiCGSTAB\n");                
        break;
    case NL_PERM_SUPERLU_EXT:
        printf("(with permutation) ");
	/* Fall through */
    case NL_SUPERLU_EXT:
        printf("Using SUPERLU\n");
        if(nlInitExtension("SUPERLU")) {
            printf("...SUPERLU extension successfully initialized\n");
        } else {
            printf("...failed to initialize SUPERLU extension\n");
            printf("Needs Linux/shared librariess/-DGEO_DYNAMIC_LIBS\n");
            return;
        }
        break;
    case NL_CHOLMOD_EXT:
        symmetric = NL_TRUE;
        printf("using CHOLMOD\n");
        if(nlInitExtension("CHOLMOD")) {
            printf("...CHOLMOD extension successfully initialized\n");
        }else {
            printf("...failed to initialize CHOLMOD extension\n");
            printf("Needs Linux/shared librariess/-DGEO_DYNAMIC_LIBS\n");
            return;
        }
        break;
    }

    if(symmetric) {
        printf("Creating linear system:\n");
        printf("  1.0*x0 - 5.0*x1 = 5.0\n");
        printf(" -5.0*x0 + 4.0*x1 = 6.0\n");
    } else {
        printf("Creating linear system:\n");
        printf("  1.0*x0 + 2.0*x1 = 5.0\n");
        printf("  3.0*x0 + 4.0*x1 = 6.0\n");
    }
    
    /* Create and initialize OpenNL context */
    nlNewContext();
    nlSolverParameteri(NL_NB_VARIABLES, 2);
    nlSolverParameteri(NL_SOLVER, solver);
    
    /* Build system */
    nlBegin(NL_SYSTEM);
    nlBegin(NL_MATRIX);
    nlBegin(NL_ROW);
    nlCoefficient(0, 1.0);
    nlCoefficient(1, symmetric ? -5.0 : 2.0);
    nlRightHandSide(5.0);
    nlEnd(NL_ROW);
    nlBegin(NL_ROW);
    nlCoefficient(0, symmetric ? -5.0 : 3.0);
    nlCoefficient(1, 4.0);
    nlRightHandSide(6.0);
    nlEnd(NL_ROW);
    nlEnd(NL_MATRIX);
    nlEnd(NL_SYSTEM);
  
    /* Solve and get solution */
    printf("Solving...\n");
    nlSolve();
    

    printf("Solution:   x0=%f   x1=%f\n", nlGetVariable(0), nlGetVariable(1));

    if(symmetric) {
        printf("Verifying:\n");
        printf(
            "  1.0*x0 - 5.0*x1 = %f\n",
            1.0 * nlGetVariable(0) - 5.0 * nlGetVariable(1)
        );
        printf(
            " -5.0*x0 + 4.0*x1 = %f\n",
            -5.0 * nlGetVariable(0) + 4.0 * nlGetVariable(1)
        );
    } else {
        printf("Verifying:\n");
        printf(
            "  1.0*x0 + 2.0*x1 = %f\n",
            1.0 * nlGetVariable(0) + 2.0 * nlGetVariable(1)
        );
        printf(
            "  3.0*x0 + 4.0*x1 = %f\n",
            3.0 * nlGetVariable(0) + 4.0 * nlGetVariable(1)
        );
    }

    /* Cleanup */
    nlDeleteContext(nlGetCurrent());
}

static void test_least_squares_regression(
    NLboolean origin, NLboolean use_SSOR_precond
) {
    NLint nb_pts = 7, k;
    NLdouble XY[7][2] = {
        {1.0, 3.5},
        {2.0, 3.8},
        {3.0, 5.5},
        {4.0, 5.4},
        {5.0, 6.3},
        {6.0, 8.2},
        {7.0, 9.5},
    };

    printf("\n");
    if(origin) {
        printf("Testing constrained least-squares regression\n");
        printf("============================================\n");
    } else {
        printf("Testing least-squares regression\n");
        printf("================================\n");        
    }

    
    nlNewContext();
    nlSolverParameteri(NL_NB_VARIABLES, 2);
    nlSolverParameteri(NL_LEAST_SQUARES, NL_TRUE);

    if(use_SSOR_precond) {
        printf("Using SSOR preconditioner\n");
        nlSolverParameteri(NL_PRECONDITIONER, NL_PRECOND_SSOR);
    } else {
        printf("Using default preconditioner (Jacobi)\n");        
    }
    
    nlBegin(NL_SYSTEM);
    if(origin) {
        nlLockVariable(1);
        nlSetVariable(1,0.0);
    }
    nlBegin(NL_MATRIX);
    for(k=0; k<nb_pts; ++k) {
        nlBegin(NL_ROW);
        nlCoefficient(0, XY[k][0]);
        nlCoefficient(1, 1.0);
        nlRightHandSide(XY[k][1]);
        nlEnd(NL_ROW);
    }
    nlEnd(NL_MATRIX);
    nlEnd(NL_SYSTEM);

    /* Solve and get solution */
    printf("Solving...\n");
    nlSolve();

    printf("Solution:   a=%f   b=%f\n", nlGetVariable(0), nlGetVariable(1));

    /* Cleanup */
    nlDeleteContext(nlGetCurrent());
}


int main(int argc, char** argv) {

    nlInitialize(argc, argv);
    
    test_simple_linear_solve(NL_SOLVER_DEFAULT);
    test_simple_linear_solve(NL_GMRES);
    test_simple_linear_solve(NL_BICGSTAB);
    test_simple_linear_solve(NL_SUPERLU_EXT);
    test_simple_linear_solve(NL_PERM_SUPERLU_EXT);
    test_simple_linear_solve(NL_CHOLMOD_EXT);
    
    test_least_squares_regression(NL_FALSE, NL_FALSE);
    test_least_squares_regression(NL_FALSE, NL_TRUE);    
    test_least_squares_regression(NL_TRUE, NL_FALSE);
    test_least_squares_regression(NL_TRUE, NL_TRUE);        

    
    return 0;
}
