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

#ifndef OPENNL_CONTEXT_H
#define OPENNL_CONTEXT_H

#include "nl_private.h"
#include "nl_matrix.h"

/**
 * \file geogram/NL/nl_context.h
 * \brief Internal OpenNL functions to manipulate contexts.
 */

/******************************************************************************/
/* NLContext data structure */


/**
 * \brief The callback type for solver routines.
 * \details Used by nlSetFunction(NL_FUNC_SOLVER,f)
 */
typedef NLboolean(*NLSolverFunc)(void);

/**
 * \brief The callback type for displaying progress.
 * \details Used by nlSetFunction(NL_FUNC_PROGRESS,f)
 */
typedef void(*NLProgressFunc)(
    NLuint cur_iter, NLuint max_iter, double cur_err, double max_err
);

#define NL_STATE_INITIAL                0
#define NL_STATE_SYSTEM                 1
#define NL_STATE_MATRIX                 2
#define NL_STATE_ROW                    3
#define NL_STATE_MATRIX_CONSTRUCTED     4
#define NL_STATE_SYSTEM_CONSTRUCTED     5
#define NL_STATE_SOLVED                 6
#define NL_STATE_MATRIX_PATTERN         7

/**
 * \brief Stores the information relevant to
 *  access variables stored in buffers.
 */
typedef struct {
    /**
     * \brief Address of the first element in the buffer.
     */
    void* base_address;
    /**
     * \brief Number of bytes between the addresses of two 
     *   consecutive elements in the buffer.
     */
    NLuint stride;
} NLBufferBinding;

/**
 * \brief Access to a double value in a buffer
 * \param[in] B a NLBufferBinding
 * \param[in] i index
 * \return the \p i th value in \p B
 */
#define NL_BUFFER_ITEM(B,i) \
    *(double*)((void*)((char*)((B).base_address)+((i)*(B).stride)))


typedef struct {
    /**
     * \brief State of the finite-state automaton.
     * \details Used to check that OpenNL functions
     *  were called in the correct order.
     */
    NLenum           state;

    /**
     * \brief NL_TRUE if variables are allocated by the user,
     *  NL_FALSE if they are managed by OpenNL.
     */
    NLboolean        user_variable_buffers;
    
    /**
     * \brief Buffer bindings for the variables, dimension = nb_systems.
     */
    NLBufferBinding* variable_buffer;
    
    /**
     * \brief Values of the variables, dimension = nb_systems * nb_variables.
     */
    NLdouble*        variable_value;

    /**
     * \brief Locked flags of the variables, dimension = nb_variables.
     */
    NLboolean*       variable_is_locked;

    /**
     * \brief Index of the variable in the actual linear system,
     *  dimension = nb_variables.
     */
    NLuint*          variable_index;
    
    /**
     * \brief The number of not locked variables.
     */
    NLuint           n;


    /**
     * \brief The current matrix mode.
     * \details One of NL_STIFFNESS_MATRIX, NL_MASS_MATRIX.
     */
    NLenum           matrix_mode;

    /**
     * \brief The matrix of the system.
     */
    NLMatrix         M;

    /**
     * \brief The preconditioner.
     */
    NLMatrix         P;

    /**
     * \brief The right-hand side matrix.
     * \details Used by the eigen solver for generalized
     *  eigenproblems.
     */
    NLMatrix         B;
    
    /**
     * \brief The coefficients that correspond to the
     *  free variables in the row being built.
     * \details Indices correspond to system indices,
     *  within [0..n-1].
     */
    NLRowColumn      af;

    /**
     * \brief The coefficients that correspond to the
     *  locked variables in the row being built.
     * \details Indices correspond to variables indices,
     *  within [0..nb_variables-1].
     */
    NLRowColumn      al;

    /**
     * \brief The vector of free variables, solution of
     *  the system.
     */
    NLdouble*        x;

    /**
     * \brief The vector of right hand sides, of size nb_systems * n.
     */
    NLdouble*        b;

    /**
     * \brief The right hand sides of the row being 
     *  built. An array of nb_systems doubles.
     * \details Specified by nlRightHandSide() and nlMultiRightHandSide()
     */
    NLdouble*        right_hand_side;

    /**
     * \brief The scaling coefficient for the row being 
     *  build.
     * \details Specified by nlSetRowParameter(NL_ROW_SCALING, rhs)
     */
    NLdouble         row_scaling;

    /**
     * \brief The used solver, as a symbolic constant.
     */
    NLenum           solver;

    /**
     * \brief The used preconditioner, as a symbolic constant.
     */
    NLenum           preconditioner;

    /**
     * \brief True if preconditioner was defined by client.
     */
    NLboolean        preconditioner_defined;
    
    /**
     * \brief The number of variables.
     */
    NLuint           nb_variables;

    /**
     * \brief The number of linear systems to solve.
     */
    NLuint           nb_systems;

    /**
     * \brief True if NLIJCoefficient() was called
     */
    NLboolean        ij_coefficient_called;
    
    /**
     * \brief The index of the current row
     */
    NLuint           current_row;

    /**
     * \brief Indicates whether a least squares system
     *  is constructed.
     */
    NLboolean        least_squares;

    /**
     * \brief Indicates whether the matrix is symmetric.
     */
    NLboolean        symmetric;

    /**
     * \brief Maximum number of iterations.
     */
    NLuint           max_iterations;


    /**
     * \brief True if max_iterations was defined by client.
     */
    NLboolean        max_iterations_defined;
    
    /**
     * \brief Maximum number of inner iterations.
     * \details used by GMRES.
     */
    NLuint           inner_iterations;

    /**
     * \brief Convergence threshold.
     * \details Iterations are stopped whenever
     *  \f$ \| A x - b \| / \| b \| < \mbox{threshold} \f$
     */
    NLdouble         threshold;

    /**
     * \brief True if threshold was defined by client.
     */
    NLboolean        threshold_defined;
    
    /**
     * \brief Relaxation parameter for the SSOR 
     *  preconditioner.
     */
    NLdouble         omega;

    /**
     * \brief If true, all the rows are normalized.
     */
    NLboolean        normalize_rows;
    
    /**
     * \brief used number of iterations during latest solve.
     */
    NLuint           used_iterations;

    /**
     * \brief error obtained after latest solve.
     */
    NLdouble         error;


    /**
     * \brief start time marking the beginning of
     *  latest solve.
     */
    NLdouble         start_time;
    
    /**
     * \brief elapsed time for latest solve.
     */
    NLdouble         elapsed_time;

    /**
     * \brief the function pointer for the solver.
     */
    NLSolverFunc     solver_func;

    /**
     * \brief the function pointer for logging progress.
     */
    NLProgressFunc   progress_func;

    /**
     * \brief if true, some logging information is 
     *  displayed during solve.
     */
    NLboolean        verbose;

    /**
     * \brief Total number of floating point operations
     *  used during latest solve.
     */
    NLulong          flops;

    /**
     * \brief The eigen solver. Should be NL_ARPACK_EXT.
     */
    NLenum           eigen_solver;

    /**
     * \brief The shift parameter of the spectral shift-invert
     *   transform.
     */
    NLdouble         eigen_shift;

    /**
     * \brief NL_TRUE if spectral shift-invert transform is used,
     *   NL_FALSE otherwise;
     */
    NLboolean        eigen_shift_invert;

    /**
     * \brief the array of eigen values. Dimension = nb_systems. 
     */
    NLdouble*        eigen_value;

    /**
     * \brief temporary array for eigen values, used for sorting.
     * \details It is put in the context so that in a multithreading
     *  context, with a thread local storage context, it will continue
     *  to work.
     */
    NLdouble*        temp_eigen_value;

    /**
     * \brief true if nlBegin(NL_MATRIX_PATTERN) was called.
     */
    NLboolean        has_matrix_pattern;

    /**
     * \brief if no_variables_indirection is set, then there is no locked
     *  variable, and the linear system directly uses the variables vector
     *  and right-hand-side.
     */
    NLboolean        no_variables_indirection;
    
} NLContextStruct;

/**
 * \brief Pointer to the current context.
 */
extern thread_local NLContextStruct* nlCurrentContext;

/**
 * \brief Makes sure that the finite state automaton is
 *  in the expected state.
 * \details If expected state and current state differ,
 *  then the program is aborted with an error message.
 * \param[in] state the expected state.
 */
void nlCheckState(NLenum state);

/**
 * \brief Implements a transition of the finite state automaton.
 * \details If the current state does not match \p state, then
 *  the program is aborted with an error message. The current 
 *  state is replaced by \p to_state. 
 * \param[in] from_state the expected current state
 * \param[in] to_state the new state
 */
void nlTransition(NLenum from_state, NLenum to_state);

/**
 * \brief Implements the default solver
 * \details Calls the right solver according to 
 *  nlCurrentContext->solver.
 * \retval NL_TRUE if solve was successful
 * \retval NL_FALSE otherwise
 */
NLboolean nlDefaultSolver(void);

#endif
