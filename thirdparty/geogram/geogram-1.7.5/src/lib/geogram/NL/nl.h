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

#ifndef OPENNL_H
#define OPENNL_H

#include "nl_linkage.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NL_VERSION_4_0 1

#define NLAPI

/* 
 * Deactivate warnings about documentation
 * We do that, because CLANG's doxygen parser does not know
 * some doxygen commands that we use (retval, copydoc) and
 * generates many warnings for them...
 */

#if defined(__clang__)
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wdocumentation"        
#pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#endif
    
/**
 * \file geogram/NL/nl.h
 * \brief The public API of the OpenNL linear solver library. 
 * Click the "More..." link below for simple example programs.
 * \details
 * The purpose of the example programs shown below is to demonstrate OpenNL 
 * programs that are as simple as possible. 
 * Note that for such small linear systems, the sparse iterative solvers in 
 * OpenNL will work, but they are completely inappropriate, one would normally 
 * solve the system directly.
 *
 * Example 1 (a simple linear system)
 * ==================================
 * Solve \f$ \left[ \begin{array}{ll} 1 & 2 \\ 3 & 4 \end{array} \right] 
 * \left[ \begin{array}{l} x \\ y \end{array} \right]
 * = \left[ \begin{array}{l} 5 \\ 6 \end{array} \right] \f$
 * \code
 * double x,y; // Solution of the system
 *
 * // Create and initialize OpenNL context
 * nlNewContext();
 * nlSolverParameteri(NL_NB_VARIABLES, 2);
 *
 * // Build system
 * nlBegin(NL_SYSTEM);
 * nlBegin(NL_MATRIX);
 * nlBegin(NL_ROW);
 * nlCoefficient(0, 1.0);
 * nlCoefficient(1, 2.0);
 * nlRightHandSide(5.0);
 * nlEnd(NL_ROW);
 * nlBegin(NL_ROW);
 * nlCoefficient(0, 3.0);
 * nlCoefficient(1, 4.0);
 * nlRightHandSide(6.0);
 * nlEnd(NL_ROW);
 * nlEnd(NL_MATRIX);
 * nlEnd(NL_SYSTEM);
 *
 * // Solve and get solution
 * nlSolve();
 * x = nlGetVariable(0);
 * y = nlGetVariable(1);
 *
 * // Cleanup
 * nlDeleteContext(nlGetCurrent());
 * \endcode
 *
 * Example 2 (least squares regression)
 * ====================================
 * Reads \f$ (X,Y) \f$ coordinates from a file and finds the
 * parameters \f$ a,b \f$ of the straight line \f$ y = ax + b \f$
 * that best fits the data points.
 * \code
 * FILE* input = fopen("datapoints.dat", "r");
 * double X,Y; // current datapoint
 *
 * // Create and initialize OpenNL context
 * nlNewContext();
 * nlSolverParameteri(NL_LEAST_SQUARES, NL_TRUE);
 * nlSolverParameteri(NL_NB_VARIABLES, 2);
 *
 * // Build system
 * nlBegin(NL_SYSTEM);
 * nlBegin(NL_MATRIX);
 * while(!feof(input)) {
 *    fread(input, "%f %f", &X, &Y);
 *    nlBegin(NL_ROW);
 *    nlCoefficient(0, X);
 *    nlCoefficient(1, 1.0);
 *    nlRightHandSide(Y);
 *    nlEnd(NL_ROW);
 * }
 * nlEnd(NL_MATRIX);
 * nlEnd(NL_SYSTEM);
 *
 * // Solve and get solution
 * nlSolve();
 * a = nlGetVariable(0);
 * b = nlGetVariable(1);
 *
 * // Cleanup
 * fclose(input);
 * nlDeleteContext(nlGetCurrent());
 * \endcode
 *
 * Example 3 (least squares regression with locked variable)
 * =========================================================
 * As in the previous example, reads \f$ (X,Y) \f$ coordinates from a 
 * file and finds the parameters \f$ a,b \f$ of the straight line \f$ y = ax + b \f$
 * that best fits the data points, but this time subject to the constraint \f$ a = 1 \f$.
 * \code
 * FILE* input = fopen("datapoints.dat", "r");
 * double X,Y; // current datapoint
 *
 * // Create and initialize OpenNL context
 * nlNewContext();
 * nlSolverParameteri(NL_LEAST_SQUARES, NL_TRUE);
 * nlSolverParameteri(NL_NB_VARIABLES, 2);
 *
 * // Build system
 * nlBegin(NL_SYSTEM);
 * nlLockVariable(0);
 * nlSetVariable(0, 1.0);
 * nlBegin(NL_MATRIX);
 * while(!feof(input)) {
 *    fread(input, "%f %f", &X, &Y);
 *    nlBegin(NL_ROW);
 *    nlCoefficient(0, X);
 *    nlCoefficient(1, 1.0);
 *    nlRightHandSide(Y);
 *    nlEnd(NL_ROW);
 * }
 * nlEnd(NL_MATRIX);
 * nlEnd(NL_SYSTEM);
 *
 * // Solve and get solution
 * nlSolve();
 * a = nlGetVariable(0);
 * b = nlGetVariable(1);
 *
 * // Cleanup
 * fclose(input);
 * nlDeleteContext(nlGetCurrent());
 * \endcode
 *
 * Example 4 (fine tuning)
 * =======================
 *   Before calling nlBegin(NL_SYSTEM), if need be, it is possible to 
 * fine-tune additional parameters. See the following examples:
 *
 * \code
 * nlSolverParameteri(NL_SOLVER, NL_GMRES);
 * nlSolverParameteri(NL_MAX_ITERATIONS, 1000);
 * nlSolverParameterd(NL_THRESHOLD, 1e-10);
 * \endcode
 *
 */
    
/**
 * \name DataTypes and constants
 * @{
 */

/**
 * \brief A symbolic constant. 
 */
typedef unsigned int    NLenum;

/**
 * \brief A truth value (NL_TRUE or NL_FALSE).
 * \see NL_TRUE, NL_FALSE
 */
typedef unsigned char   NLboolean;

/**
 * \brief A set of symbolic constants that can be
 *  combined with the bitwise or operator
 */
typedef unsigned int    NLbitfield;

/**
 * \brief Return type of functions that do not return
 *  a value.
 */
typedef void            NLvoid;

/**
 * \brief A 1-byte signed integer.
 */
typedef signed char     NLbyte;

/**
 * \brief A 2-bytes signed integer.
 */
typedef short           NLshort;

/**
 * \brief A 4-bytes signed integer.
 */
typedef int             NLint; 

/**
 * \brief A 1-byte unsigned integer.
 */
typedef unsigned char   NLubyte;

/**
 * \brief A 2-bytes unsigned integer.
 */
typedef unsigned short  NLushort;

/**
 * \brief A 4-bytes unsigned integer.
 */
typedef unsigned int    NLuint;  

/**
 * \brief A 8-bytes signed integer.
 */
typedef long            NLlong;   
    
/**
 * \brief A 8-bytes unsigned integer.
 */
typedef unsigned long   NLulong;   

/**
 * \brief Size of an object, 4-bytes signed integer.
 */
typedef int             NLsizei;

/**
 * \brief A single-precision floating-point number.
 */
typedef float           NLfloat;

/**
 * \brief A double-precision floating-point number.
 */
typedef double          NLdouble;

/**
 * \brief A function pointer.
 * \see nlSetFunction(), nlGetFunction(), 
 *  NL_FUNC_SOLVER, NL_FUNC_MATRIX,
 *  NL_FUNC_PRECONDITIONER,
 *  NL_FUNC_PROGRESS      
 */
typedef void(*NLfunc)(void);

/**
 * \brief An OpenNL context.
 * \details OpenNL contexts should be considered as
 *  opaque ids.
 * \see nlNewContext(), nlDeleteContext(), nlMakeCurrent(), nlGetCurrent()
 */
typedef void* NLContext; 

/**
 * \brief Constant for false \ref NLboolean%s.
 * \see NLboolean
 */
#define NL_FALSE   0x0
/**
 * \brief Constant for true \ref NLboolean%s.
 * \see NLboolean
 */
#define NL_TRUE    0x1

/**
 * @}
 * \name Solver parameters
 * @{
 */

/**
 * \brief Symbolic constant for nlSolverParameteri() to specify
 *  the used solver.
 * \details Used as follows, before any call to nlBegin():
 * \code
 *   nlSolverParameteri(NL_SOLVER, solver);
 * \endcode
 * where solver is one of (\ref NL_CG, \ref NL_BICGSTAB, \ref NL_GMRES) 
 * or one of the extended solvers if supported 
 * (NL_xxx_SUPERLU_EXT, NL_CNC_xxx) or \ref NL_SOLVER_DEFAULT to
 * let OpenNL decide and use a reasonable solver.
 */
#define NL_SOLVER           0x100

/**
 * \brief Symbolic constant for nlSolverParameteri() to specify
 *  the number of variables.
 * \details Used as follows, before any call to nlBegin():
 * \code
 *   nlSolverParameteri(NL_NB_VARIABLES, nb);
 * \endcode
 * where nb is the number of variables.
 */
#define NL_NB_VARIABLES     0x101

/**
 * \brief Symbolic constant for nlSolverParameteri() to specify
 *  whether least squares mode is used.
 * \details Used as follows, before any call to nlBegin():
 * \code
 *   nlSolverParameteri(NL_LEAST_SQUARES, NL_TRUE);
 * \endcode
 * or
 * \code
 *   nlSolverParameteri(NL_LEAST_SQUARES, NL_FALSE);
 * \endcode
 */
#define NL_LEAST_SQUARES    0x102

/**
 * \brief Symbolic constant for nlSolverParameteri() to specify
 *  the maximum number of iterations before the iterative solver 
 *  is stopped.
 * \details Usage:
 * \code
 *   nlSolverParameteri(NL_MAX_ITERATIONS, nb);
 * \endcode
 *  If the solver is left unspecified or is set to \ref NL_SOLVER_DEFAULT,
 * then a reasonable maximum number of iterations is used (5 times the 
 * number of free variables).
 */
#define NL_MAX_ITERATIONS   0x103

/**
 * \brief Symbolic constant for nlSolverParameterd() to specify
 *  the maximum threshold \f$ \| Ax - b \| / \| b \| \f$ before 
 *  the iterative solver is stopped.
 * \details Usage:
 * \code
 *   nlSolverParameterd(NL_THRESHOLD, epsilon);
 * \endcode
 *  If the solver is left unspecified or is set to 
 *  \ref NL_SOLVER_DEFAULT, then a reasonable value is used (1e-6).
 */
#define NL_THRESHOLD        0x104

/**
 * \brief Symbolic constant for nlSolverParameterd() to specify
 *  the relaxation parameter \f$ \omega \in (0,\ldots 2) \f$ used
 *  by the SSOR preconditioner.
 * \details Usage:
 * \code
 *   nlSolverParameterd(NL_OMEGA, omega)
 * \endcode
 * \see NL_PRECOND_SSOR
 */
#define NL_OMEGA            0x105

/**
 * \brief Symbolic constant for nlSolverParameteri() to specify
 *  whether the constructed matrix is symmetric.
 * \details If the constructed matrix is symmetric, pre-notifying
 *  OpenNL before constructing it may improve performance, by
 *  allowing more efficient data structures and solvers to be used.
 *  Usage:
 * \code
 *   nlSolverParameteri(NL_SYMMETRIC, NL_TRUE);
 * \endcode
 * or
 * \code
 *   nlSolverParameteri(NL_SYMMETRIC, NL_FALSE);
 * \endcode
 * \warning Behavior if undefined if setting \ref NL_SYMMETRIC 
 *  and constructing a non-symmetric matrix afterwards.
 */
#define NL_SYMMETRIC        0x106

/**
 * \brief Symbolic constant for nlGetIntegerv() to obtain the
 *  used number of iteration.
 * \details The solver exits whenever the maximum number of
 *  iterations (\ref NL_MAX_ITERATIONS) is reached or whenever
 *  \f$ \| Ax - b \| / \| b \| \f$ gets smaller than \ref NL_THRESHOLD.
 *  Usage:
 * \code
 *   NLint used_iter;
 *   nlGetIntegerv(NL_USED_ITERATIONS, &used_iter);
 * \endcode
 */
#define NL_USED_ITERATIONS  0x107

/**
 * \brief Symbolic constant for nlGetDoublev() to obtain the
 *  error after nlSolve() is called.
 * \details Gets \f$ \| Ax - b \| / \| b \| \f$.
 *  Usage:
 * \code
 *   NLdouble error;
 *   nlGetDoublev(NL_ERROR, &error);
 * \endcode
 */
#define NL_ERROR            0x108

/**
 * \brief Symbolic constant for nlSolverParameteri() to specify
 *  the number of iterations / number of stored vector in the
 *  inner \ref NL_GMRES loop.
 * \details Only relevant if used solver is \ref NL_GMRES. 
 *  Usage:
 * \code
 *   nlSolverParameteri(NL_INNER_ITERATIONS, nn);
 * \endcode
 */
#define NL_INNER_ITERATIONS 0x109

/**
 * \brief Symbolic constant for nlGetDoublev() to obtain the
 *  time taken by the latest invocation of nlSolve(), in seconds.
 * \details
 *  Usage:
 * \code
 *   NLdouble seconds;
 *   nlGetDoublev(NL_ELAPSED_TIME, &seconds);
 * \endcode
 */
#define NL_ELAPSED_TIME     0x10a

/**
 * \brief Symbolic constant for nlSolverParameteri() to specify
 *  the used preconditioner.
 * \details Should be one of 
 *  (\ref NL_PRECOND_NONE, \ref NL_PRECOND_JACOBI, 
     \ref NL_PRECOND_SSOR, \ref NL_PRECOND_USER).
 *  If NL_PRECOND_USER is used, then the user-defined preconditioner is 
 *  specified using nlSetFunction(). Usage:
 * \code
 *   nlSolverParameteri(NL_PRECONDITIONER, NL_JACOBI);
 * \endcode
 */
#define NL_PRECONDITIONER   0x10b

/**
 * \brief Symbolic constant for nlGetDoublev() to obtain the
 *  average GFlop/Seconds performance counter during the latest
 *  invocation of nlSolve().
 * \details
 *  Usage:
 * \code
 *   NLdouble gflops;
 *   nlGetDoublev(NL_GFLOPS, &gflops);
 * \endcode
 */
#define NL_GFLOPS           0x10c

/**
 * \brief Symbolic constant for nlGetIntegerv() to obtain the
 *  number of non-zero coefficient in the latest linear
 *  system used by nlSolve().
 * \details
 *  Usage:
 * \code
 *   NLint nnz;
 *   nlGetIngerv(NL_NNZ, &nnz);
 * \endcode
 */
#define NL_NNZ              0x10d    


/**
 * \brief Symbolic constant for nlSolverParameteri()/nlGetIntegerv() to
 *  define or query the number of linear systems to be solved.
 */    
#define NL_NB_SYSTEMS       0x10e
    
/**
 * @}
 * \name Solvers
 * @{
 */

/**
 * \brief Symbolic constant for nlSolverParameteri()
 *  to specify that OpenNL should automatically select a reasonable
 *  solver and preconditioner. 
 * \details It is the default operating mode of
 *  OpenNL. It can also be reset as follows:
 * \code
 *  nlSolverParameteri(NL_SOLVER,NL_SOLVER_DEFAULT);
 * \endcode
 * It also lets OpenNL choose (hopefully reasonable) values for maximum number of iterations
 * (\ref NL_MAX_ITERATIONS) = 5 times the number of free variables and 
 * threshold (\ref NL_THRESHOLD) = 1e-6. It uses the Jacobi-preconditioned
 * conjugate gradient solver (\ref NL_CG and \ref NL_PRECOND_JACOBI) if the 
 * matrix is known to be symmetric, and \ref NL_BICGSTAB otherwise.
 */
#define NL_SOLVER_DEFAULT        0x000

/**
 * \brief Symbolic constant for nlSolverParameteri()
 *  to select the Conjugate Gradient solver.
 * \details Usage:
 * \code
 *   nlSolverParameteri(NL_SOLVER, NL_CG);
 * \endcode
 * \warning The Conjugate Gradient solver requires the matrix to be 
 *  symmetric (note that it is always the case in least-squares mode,
 *  but NOT in regular mode).
 */
#define NL_CG                    0x200

/**
 * \brief Symbolic constant for nlSolverParameteri()
 *  to select the Conjugate Gradient solver.
 * \details Usage:
 * \code
 *   nlSolverParameteri(NL_SOLVER, NL_BICGSTAB);
 * \endcode
 * \ref BICGSTAB works for both non-symmetric and symmetric matrices.
 * If the matrix is known to be symmetric, then \ref CG will be more
 * efficient.
 */
#define NL_BICGSTAB              0x201

/**
 * \brief Symbolic constant for nlSolverParameteri()
 *  to select the GMRES solver.
 * \details Usage:
 * \code
 *   nlSolverParameteri(NL_SOLVER, NL_GMRES);
 * \endcode
 * GMRES works for both non-symmetric and symmetric matrices.
 * If the matrix is known to be symmetric, then \ref NL_CG will be more
 * efficient. GMRES has an additional internal number of iterations
 * parameters that can be set as follows:
 * \code
 *   nlSolverParameteri(NL_INNER_ITERATIONS,nn);
 * \endcode
 */
#define NL_GMRES                 0x202

/**
 * @}
 * \name Preconditioners
 * @{
 */

/**
 * \brief Symbolic constant for nlSolverParameteri()
 *  to use no preconditioner.
 * \details Usage:
 * \code
 *   nlSolverParameteri(NL_PRECONDITIONER, NL_PRECOND_NONE);
 * \endcode
 */    
#define NL_PRECOND_NONE       0x000

/**
 * \brief Symbolic constant for nlSolverParameteri()
 *  to use the Jacobi preconditioner.
 * \details The Jacobi preconditioner corresponds to the
 *  inverse of the diagonal elements. Clearly, it requires
 *  that the matrix has no zero element on the diagonal.
 * Usage:
 * \code
 *   nlSolverParameteri(NL_PRECONDITIONER, NL_PRECOND_JACOBI);
 * \endcode
 */    
#define NL_PRECOND_JACOBI     0x300

/**
 * \brief Symbolic constant for nlSolverParameteri()
 *  to use the SSOR (Successive Over Relaxation) preconditioner.
 * \details
 * Usage:
 * \code
 *   nlSolverParameteri(NL_PRECONDITIONER, NL_PRECOND_SSOR);
 * \endcode
 * The SSOR preconditioner can significantly reduce the number
 * of used iterations, but each iteration takes much longer.
 * It has an additional Omega parameter:
 * \see NL_OMEGA
 */    
#define NL_PRECOND_SSOR       0x301

/**
 * \brief Symbolic constant for nlSolverParameteri()
 *  to use a user-defined preconditioner.
 * \details
 * Usage:
 * \code
 *   nlSolverParameteri(NL_PRECONDITIONER, NL_PRECOND_USER);
 * \endcode
 * The user-defined preconditoner is specified as a function
 * pointer, \see nlSetFunction(), NL_FUNC_PRECONDITIONER, NL_PRECONDITIONER
 */    
#define NL_PRECOND_USER       0x303

/**
 * @}
 * \name Enable / Disable
 * @{
 */

/**
 * \brief Symbolic constant for nlEnable() / nlDisable()
 *  to enable or disable rows normalization. 
 * \details When row normalization is enabled, each time
 *   a row is completed with nlEnd(NL_ROW), the 
 *   coefficients of the current row and its right hand
 *   side are divided by the length \f$ L \f$ of the current row
 *   (and multiplied by the \ref NL_ROW_SCALING \f$ s \f$ if one
 *    was specified):
 * \f[
 *   \begin{array}{lcl}
 *      L & \leftarrow & \sqrt{\sum_{j} a_{i,j}^2} \\
 *      a_{i,j} & \leftarrow & a_{i,j} \times s / L \\
 *      b_i     & \leftarrow & b_i     \times s / L 
 *  \end{array}
 * \f]
 *   Usage:
 * \code
 *  nlEnable(NL_NORMALIZE_ROWS);
 * \endcode
 * or
 * \code
 *  nlDisable(NL_NORMALIZE_ROWS);
 * \endcode
 * \see nlEnable(), nlDisable(), nlIsEnabled(), NL_ROW_SCALING
 */
#define NL_NORMALIZE_ROWS  0x400

/**
 * \brief Symbolic constant for nlEnable() / nlDisable()
 *  to enable or disable messages.
 * \details Usage:
 * \code
 *   nlEnable(NL_VERBOSE)
 * \endcode
 * or
 * \code
 *   nlDisable(NL_VERBOSE)
 * \endcode
 * \see nlEnable(), nlDisable(), nlIsEnabled()
 */
#define NL_VERBOSE         0x401


/**
 * \brief Symbolic constant for nlEnable() / nlDisable()
 *  to enable or disable variables indirection.
 * \details Usage:
 * \code
 *   nlEnable(NL_NO_VARIABLES_INDIRECTION)
 * \endcode
 * or
 * \code
 *   nlDisable(NL_NO_VARIABLES_INDIRECTION)
 * \endcode
 * \see nlEnable(), nlDisable(), nlIsEnabled()
 */
#define NL_NO_VARIABLES_INDIRECTION 0x402
    
/**
 * @}
 * \name Context management
 * @{
 */

/**
 * \brief Creates a new OpenNL context
 * \details Must be called before any other OpenNL function.
 *  On exit, the newly created context is the current OpenNL
 *  context. Several OpenNL context may coexist. All OpenNL calls are
 *  forwarded to the current OpenNL context. Use nlMakeCurrent() 
 *  to switch between multiple OpenNL contexts.
 *  Any created context needs to be destroyed before the end
 *  of the program using nlDeleteContext().
 * \return a handle to the newly created context.
 */
    NLAPI NLContext NLAPIENTRY nlNewContext(void);

/**
 * \brief Destroys an existing OpenNL context
 * \details This deallocates all the memory used by the context.
 * \param[in,out] context the context to be destroyed
 */
    NLAPI void NLAPIENTRY nlDeleteContext(NLContext context);

/**
 * \brief Sets the current OpenNL context.
 * \details If several OpenNL contexts need to be used simultaneously,
 *  this function can be used to redirect all OpenNL calls to a specific
 *  context.
 */
    NLAPI void NLAPIENTRY nlMakeCurrent(NLContext context);

/**
 * \brief Gets the current context
 * \return a handle to the current OpenNL context
 */
    NLAPI NLContext NLAPIENTRY nlGetCurrent(void);

/**
 * \brief Initializes an OpenNL extension
 * \details OpenNL may be compiled with several extensions, that provide 
 *  alternative solvers, such as SuperLU (sparse direct solver) and CNC 
 *  (iterative solver on the GPU). This function tests whether an extension 
 *  is supported, and initializes what needs to be initialized in the extention.
 * \retval NL_TRUE if the extension is supported and could be successfully 
 *   initialized
 * \retval NL_FALSE otherwise
 */
    NLAPI NLboolean NLAPIENTRY nlInitExtension(const char* extension);

/**
  * \brief Tests whether an OpenNL extension is initialized.
  * \retval NL_TRUE if the extension is initialized.
  * \retval NL_FALSE otherwise
  */
    NLAPI NLboolean NLAPIENTRY nlExtensionIsInitialized(const char* extension);

/**
 * \brief Initializes OpenNL using command line arguments.
 * \details Command line arguments of the form nl:<extension>=true|false are parsed
 *  and taken into account. Calling this function is not mandatory.
 */
    NLAPI void NLAPIENTRY nlInitialize(int argc, char** argv);
    
/**
 * @}
 * \name State Get/Set
 * @{
 */

/**
 * \brief Specifies a floating-point solver parameter
 * \details This function should be called in the initial state of OpenNL,
 *  before any nlBegin() / nlEnd() call. 
 * \param[in] pname the symbolic name of the parameter, 
 *  one of (\ref NL_THRESHOLD, \ref NL_OMEGA).
 * \param[in] param the double-precision floating-point value of the parameter.
 *  \arg \p If pname = \ref NL_THRESHOLD, then \p param is the maximum value of 
 *   \f$ \| Ax - b \| / \| b \| \f$ before iterations are stopped;
 *  \arg \p if pname = \ref NL_OMEGA and the specified preconditioner 
 *   is \ref NL_SSOR, then \p param is the relaxation parameter, 
 *   in (0.0 .. 2.0) excluded, for the SSOR preconditioner. 
 */
    NLAPI void NLAPIENTRY nlSolverParameterd(NLenum pname, NLdouble param);

/**
 * \brief Specifies an integer solver parameter
 * \details This function should be called in the initial state of OpenNL,
 *  before any nlBegin() / nlEnd() call.
 * \param[in] pname the symbolic name of the parameter, one of 
 *  (\ref NL_SOLVER, \ref NL_NB_VARIABLES, \ref NL_LEAST_SQUARES, 
 *   \ref NL_MAX_ITERATIONS, \ref NL_SYMMETRIC, \ref NL_INNER_ITERATIONS, 
 *   \ref NL_PRECONDITIONER). 
 * \param[in] param the integer value of the parameter.
 *   \arg If \p pname = \ref NL_SOLVER then \p param is the symbolic 
 *    constant that specifies the solver, i.e. one of (NL_CG, NL_BICGSTAB, NL_GMRES) 
 *    or one of the extended solvers if supported (NL_xxx_SUPERLU_EXT, NL_CNC_xxx);
 *   \arg if \p pname = \ref NL_NB_VARIABLES then \p param specifies the number of variables;
 *   \arg if \p pname = \ref NL_LEAST_SQUARES then \p param is a boolean value 
 *    (NL_TRUE or NL_FALSE) that specifies whether least squares mode should be 
 *    used (solves \f$ A^t A x = A^t b \f$ with a possibly non-square matrix \f$ A \f$);
 *   \arg if \p pname = \ref NL_MAX_ITERATIONS then \p param is the maximum number of iterations;
 *   \arg if \p pname = \ref NL_SYMMETRIC then \p param is a boolean value (NL_TRUE or NL_FALSE) that
 *    specifies whether the constructed matrix is symmetric. This is a hint for OpenNL 
 *    that allows using more efficient data structures / solvers. Behavior is undefined
 *    if you lied and specify then a non-symmetric matrix !
 *   \arg if \p pname = \ref NL_INNER_ITERATIONS and the solver is NL_GMRES, 
 *    then \p param is the number of inner-loop iterations / 
 *    number of intermediate vectors used by GMRES;
 *   \arg if \p pname = \ref NL_PRECONDITIONER then \p param is the 
 *    symbolic constant that specifies the preconditioner, i.e. one of 
 *    (\ref NL_PRECOND_NONE, \ref NL_PRECOND_JACOBI, \ref NL_PRECOND_SSOR).
 *
 *  \see NL_SOLVER, NL_NB_VARIABLES, NL_LEAST_SQUARES, NL_MAX_ITERATIONS, 
 *    NL_SYMMETRIC, NL_INNER_ITERATIONS, NL_PRECONDITIONER
 */ 
    NLAPI void NLAPIENTRY nlSolverParameteri(NLenum pname, NLint param);

/**
 * \brief Gets the value of a boolean parameter
 * \param[in] pname the symbolic name of the parameter
 * \param[out] params a pointer to the obtained parameter value
 */
    NLAPI void NLAPIENTRY nlGetBooleanv(NLenum pname, NLboolean* params);

/**
 * \brief Gets the value of a double-precision floating-point parameter
 * \param[in] pname the symbolic name of the parameter
 * \param[out] params a pointer to the obtained parameter value
 */
    NLAPI void NLAPIENTRY nlGetDoublev(NLenum pname, NLdouble* params);

/**
 * \brief Gets the value of an integer parameter
 * \param[in] pname the symbolic name of the parameter
 * \param[out] params a pointer to the obtained parameter value
 */
    NLAPI void NLAPIENTRY nlGetIntegerv(NLenum pname, NLint* params);

/**
 * \brief Gets the value of a 64 bits integer parameter
 * \param[in] pname the symbolic name of the parameter
 * \param[out] params a pointer to the obtained parameter value
 */
    NLAPI void NLAPIENTRY nlGetIntegervL(NLenum pname, NLlong* params);

    
/**
 * \brief Sets a boolean parameter to NL_TRUE
 * \param[in] pname the symbolic name of the parameter
 */
    NLAPI void NLAPIENTRY nlEnable(NLenum pname);

/**
 * \brief Sets a boolean parameter to NL_FALSE
 * \param[in] pname the symbolic name of the parameter
 */
    NLAPI void NLAPIENTRY nlDisable(NLenum pname);

/**
 * \brief Tests a boolean parameter 
 * \param[in] pname the symbolic name of the parameter
 * \return the value of the boolean parameter
 */
    NLAPI NLboolean nlIsEnabled(NLenum pname);

/**
 * @}
 * \name Function pointers
 * @{
 */

/**
 * \brief Symbolic constant for nlSetFunction(), used
 *  to replace OpenNL solver with a user-defined routine.
 * \details
 * \note For advanced users only, requires to dig into 
 *  OpenNL internal structures.
 * \code
 * #include <nl_context.h>
 * NLboolean my_solver() {
 *    NLContextStruct* context = (NLContextStruct*)nlGetCurrent();
 *    ...
 *    ...
 * }
 * nlSetFunction(NL_FUNC_SOLVER, (nlFunc)my_solver);
 * \endcode
 */
#define NL_FUNC_SOLVER         0x600

/**
 * \brief  Symbolic constant for nlSetFunction(), used
 *  to replace OpenNL matrix-vector product with a 
 *  user-defined routine.
 * \details
 * \code
 * void my_matrix_func(const double* x, double* y) {
 *   ...
 * }
 * nlSetFunction(NL_FUNC_MATRIX, (nlFunc)my_matrix_func);
 * \endcode
 */
#define NL_FUNC_MATRIX         0x601

/**
 * \brief  Symbolic constant for nlSetFunction(), used
 *  to replace OpenNL preconditioner with a user-defined routine.
 * \details
 * \code
 * void my_precond_func(const double* x, double* y) {
 *   ...
 * }
 * nlSolverParameteri(NL_PRECONDITIONER, NL_PRECOND_USER);
 * nlSetFunction(NL_FUNC_PRECONDITIONER, (nlFunc)my_precond_func);
 * \endcode
 * \see nlSetFunction(), NL_PRECONDITIONER
 */
#define NL_FUNC_PRECONDITIONER 0x602

/**
 * \brief  Symbolic constant for nlSetFunction(), used
 *  to display a progress bar during solve, using a 
 *  user-defined callback.
 * \details The user-defined progress callback is called 
 *  after each iteration, and can be used to update a
 *  progress bar.
 * \code
 * void my_progress_callback(
 *    NLuint cur_iter,  // Current iteration
 *    NLuint max_iter,  // Maximum number of iterations
 *    double cur_err,   // Current (squared, un-normalized) error
 *    double max_err    // Maximum (squared, un-normalized) error
 * ) {
 *   ...
 * }
 * nlSetFunction(NL_FUNC_PROGRESS, (nlFunc)my_progress_callback);
 * \endcode
 */
#define NL_FUNC_PROGRESS       0x603

/**
 * \brief Sets a function pointer
 * \param[in] pname symbolic name of the function, one of (\ref NL_FUNC_MATRIX,
 *  \ref NL_FUNC_PRECONDITIONER, \ref NL_FUNC_PROGRESS)
 * \param[in] param the function pointer
 * \see nlGetFunction(), NL_FUNC_MATRIX, NL_FUNC_PRECONDITIONER, 
 *  NL_FUNC_PROGRESS
 */
    NLAPI void NLAPIENTRY nlSetFunction(NLenum pname, NLfunc param);

/**
 * \brief Gets a function pointer
 * \param[in] pname symbolic name of the function 
 * \param[out] param the function pointer
 * \see nlSetFunction(), NL_FUNC_MATRIX, NL_FUNC_PRECONDITIONER, 
 *  NL_FUNC_PROGRESS
 */
    NLAPI void NLAPIENTRY nlGetFunction(NLenum pname, NLfunc* param);

/**
 * @}
 * \name Variables
 * @{
 */

/**
 * \brief Sets the value of a variable
 * \param[in] i index of the variable, between 0 and 
 *  nlGetInteger(NL_NB_VARIABLES)-1
 * \param[in] value value of the variable
 * \see nlGetVariable(), nlLockVariable(), nlUnlockVariable(), 
 *  nlVariableIsLocked()
 */
    NLAPI void NLAPIENTRY nlSetVariable(NLuint i, NLdouble value);


/**
 * \brief Sets the value of a variable when there are several systems
 *  to solve
 * \param[in] i index of the variable, between 0 and 
 *  nlGetInteger(NL_NB_VARIABLES)-1
 * \param[in] k index of the system, between 0 and
 *  nlGetInteger(NL_NB_SYSTEMS)-1
 * \param[in] value value of the variable
 * \see nlMultiGetVariable()
 */
    NLAPI void NLAPIENTRY nlMultiSetVariable(
	NLuint i, NLuint k, NLdouble value
    );
    
/**
 * \brief Gets the value of a variable
 * \param[in] i index of the variable, between 0 and 
 *  nlGetInteger(NL_NB_VARIABLES)-1
 * \return the value of the variable
 * \see nlSetVariable(), nlLockVariable(), nlUnlockVariable(), 
 * nlVariableIsLocked()
 */
    NLAPI NLdouble NLAPIENTRY nlGetVariable(NLuint i);

/**
 * \brief Gets the value of a variable when there are several systems
 *  to solve
 * \param[in] i index of the variable, between 0 and 
 *  nlGetInteger(NL_NB_VARIABLES)-1
 * \param[in] k index of the system, between 0 and
 *  nlGetInteger(NL_NB_SYSTEMS)-1
 * \return value value of the variable
 * \see nlMultiSetVariable()
 */
    NLAPI NLdouble NLAPIENTRY nlMultiGetVariable(NLuint i, NLuint k);
    
/**
 * \brief Locks a variable
 * \details Locked variables are no-longer computed by OpenNL, their initial
 *  value, specified by nlSetVariable(), is used as follows:
 *  - in standard mode, locked variables are moved to the right hand side
 *  - in least squares mode, locked variables are removed from the degrees
 *   of freedom and combined into the right hand side
 * \param[in] index index of the variable, between 0 and 
 *  nlGetInteger(NL_NB_VARIABLES)-1
 * \see nlGetVariable(), nlSetVariable(), nlUnlockVariable(), 
 * nlVariableIsLocked()
 */
    NLAPI void NLAPIENTRY nlLockVariable(NLuint index);

/**
 * \brief Unlocks a variable
 * \details Locked variables are no-longer computed by OpenNL, their initial
 *  value, specified by nlSetVariable(), is used as follows:
 *  - in standard mode, locked variables are moved to the right hand side
 *  - in least squares mode, locked variables are removed from the degrees 
 *   of freedom and combined into the right hand side
 * \param[in] index index of the variable, between 0 and 
 *  nlGetInteger(NL_NB_VARIABLES)-1
 * \see nlGetVariable(), nlSetVariable(), nlLockVariable(), nlVariableIsLocked()
 */
    NLAPI void NLAPIENTRY nlUnlockVariable(NLuint index);

/**
 * \brief Tests whether a variable is locked
 * \details Locked variables are no-longer computed by OpenNL, their initial
 *  value, specified by nlSetVariable(), is used as follows:
 *  - in standard mode, locked variables are moved to the right hand side
 *  - in least squares mode, locked variables are removed from the degrees 
 *   of freedom and combined into the right hand side
 * \param[in] index index of the variable, between 0 and 
 *  nlGetInteger(NL_NB_VARIABLES)-1
 * \see nlGetVariable(), nlSetVariable(), nlLockVariable(), nlUnlockVariable()
 */
    NLAPI NLboolean NLAPIENTRY nlVariableIsLocked(NLuint index);

/**
 * @}
 * \name Begin/End
 * @{
 */

/**
 * \brief Symbolic constant for nlBegin() / nlEnd(), to
 *  be used to start creating / finalizing a linear system.
 * \details 
 * \see nlBegin(), nlEnd()
 */
#define NL_SYSTEM  0x0

/**
 * \brief Symbolic constant for nlBegin() / nlEnd(), to
 *  be used to start creating / finalizing a matrix.
 * \details 
 * \see nlBegin(), nlEnd()
 */
#define NL_MATRIX  0x1

/**
 * \brief Symbolic constant for nlBegin() / nlEnd(), to
 *  be used to start creating / finalizing a row.
 * \details 
 * \see nlBegin(), nlEnd()
 */
#define NL_ROW     0x2

/**
 * \brief Symbolic constant for nlBegin() / nlEnd(), to
 *  be used to start creating / finalizing a row.
 * \details 
 * \see nlBegin(), nlEnd()
 */
#define NL_MATRIX_PATTERN  0x3

/**
 * \brief Begins a new primitive
 * \details nlBegin() / nlEnd() calls should be properly nested, as
 *  follows (otherwise an assertion failure is triggered):
 * \code
 *  nlBegin(NL_SYSTEM)
 *    nlBegin(NL_MATRIX)
 *        nlBegin(NL_ROW)
 *        nlEnd(NL_ROW)
 *   ...
 *        nlBegin(NL_ROW)
 *        nlEnd(NL_ROW)
 *    nlEnd(NL_MATRIX)
 *  nlEnd(NL_SYSTEM)
 * \endcode
 * \param[in] primitive one of NL_SYSTEM, NL_MATRIX, NL_ROW
 */
    NLAPI void NLAPIENTRY nlBegin(NLenum primitive);

/**
 * \brief Begins a new primitive
 * \details nlBegin()/nlEnd() calls should be properly nested, as
 *  follows (otherwise an assertion failure is triggered):
 * \code
 *  nlBegin(NL_SYSTEM)
 *    nlBegin(NL_MATRIX)
 *        nlBegin(NL_ROW)
 *        nlEnd(NL_ROW)
 *   ...
 *        nlBegin(NL_ROW)
 *        nlEnd(NL_ROW)
 *    nlEnd(NL_MATRIX)
 *  nlEnd(NL_SYSTEM)
 * \endcode
 * \param[in] primitive one of NL_SYSTEM, NL_MATRIX, NL_ROW
 */
    NLAPI void NLAPIENTRY nlEnd(NLenum primitive);


/**
 * \brief Specifies the length of a row of the matrix.
 * \details Should be called between nlBegin(NL_MATRIX_PATTERN) and
 *  nlEnd(NL_MATRIX_PATTERN).
 * \param[in] i the index of the row.
 * \param[in] n the length of the row.
 */
    NLAPI void NLAPIENTRY nlSetRowLength(NLuint i, NLuint n);
    
/**
 * \brief Appends a coefficient to the current row.
 * \details This function should be called between a
 *  nlBegin(NL_ROW) / nlEnd(NL_ROW) pair (else an assertion failure
 *  is triggered). The coefficient is either accumumated into the matrix or 
 *  into the right-hand side according to the locked/unlocked status
 *  of the variable.
 * \param[in] i index of the variable this coefficient is related with
 * \param[in] value value of the coefficient
 * \see nlBegin(), nlEnd(), NL_ROW, NL_RIGHT_HAND_SIDE, NL_ROW_SCALING,
 *  NL_NORMALIZE_ROWS
 */
    NLAPI void NLAPIENTRY nlCoefficient(NLuint i, NLdouble value);



/**
 * \brief Adds a coefficient to the current matrix.
 * \details This function should be called between a
 *   nlBegin(NL_MATRIX) / nlEnd(NL_MATRIX) pair (else an assertion failure
 *   is triggered). This function should not be called in least squares mode.
 *   There should not be any locked variable when using this function.
 * \param[in] i , j indices
 * \param[in] value value to be added to the coefficient
 */    
    NLAPI void NLAPIENTRY nlAddIJCoefficient(
        NLuint i, NLuint j, NLdouble value
    );


/**
 * \brief Adds a coefficient to a component of the right hand side 
 *  of the equation.
 * \details This function should be called between a
 *   nlBegin(NL_MATRIX) / nlEnd(NL_MATRIX) pair (else an assertion failure
 *   is triggered). This function should not be called in least squares mode.
 *   There should not be any locked variable when using this function.
 * \param[in] i index of the component
 * \param[in] value value to be added to the component
 */    
    NLAPI void NLAPIENTRY nlAddIRightHandSide(NLuint i, NLdouble value);

/**
 * \brief Adds a coefficient to a component of the right hand side 
 *  of the equation.
 * \details This function should be called between a
 *   nlBegin(NL_MATRIX) / nlEnd(NL_MATRIX) pair (else an assertion failure
 *   is triggered). This function should not be called in least squares mode.
 *   There should not be any locked variable when using this function.
 * \param[in] i index of the component
 * \param[in] k index of the system
 * \param[in] value value to be added to the component
 */    
    NLAPI void NLAPIENTRY nlMultiAddIRightHandSide(
	NLuint i, NLuint k, NLdouble value
    );
    
/**
 * \brief Sets the right-hand side of the current row.
 * \details 
 *  - In regular mode, this corresponds to 
 *  the right-hand side \f$ b_i \f$ of equation:
 *   \f[
 *     \sum_j a_{i,j} x_j = b_i
 *   \f]
 *  - In least-squares mode (\ref NL_LEAST_SQUARES), this correspond to the
 *  term \f$ b_i \f$ in:
 *   \f[
 *     \left( \sum_j a_{i,j} x_j - b_i \right)^2
 *   \f]
 * Usage:
 * \code
 *   nlRowParameterd(NL_RIGHT_HAND_SIDE, bi);
 *   nlBegin(NL_ROW);
 *   ...
 *   nlEnd(NL_ROW);
 * \endcode
 * \pre This function should be called between a
 *  nlBegin(NL_ROW) / nlEnd(NL_ROW) pair, and at most once
 *  (else an assertion failure is triggered). 
 * \note The right hand side 
 *  is reset to zero after completion of nlEnd(NL_ROW).
 * \param[in] value value to be accumulated into the right hand side.
 */
    NLAPI void NLAPIENTRY nlRightHandSide(NLdouble value);


/**
 * \brief Sets the right-hand side of the current row when there are
 *  several systems to be solved.
 * \param[in] k index of the system, in 0..nlGetInt(NL_NB_SYSTEMS)-1
 * \param[in] value the coefficient
 * \see nlRightHandSide()
 */
    NLAPI void NLAPIENTRY nlMultiRightHandSide(NLuint k, NLdouble value);
    
/**
 * \brief Sets the row scaling for the next row.
 * \details 
 *  - if \ref NL_NORMALIZE_ROWS is not enabled, then
 *  the coefficients are multiplied by the specified row scaling
 *  coefficient \f$ s \f$ as follows when the row is completed:
 * \f[
 *   \begin{array}{lcl}
 *      a_{i,j} & \leftarrow & a_{i,j} \times s \\
 *      b_i     & \leftarrow & b_i     \times s  
 *  \end{array}
 * \f]
 *  - if \ref NL_NORMALIZE_ROWS is enabled, then
 *  the coefficients are divided by the row length \f$ L \f$
 *  and multiplied by the specified row scaling
 *  coefficient \f$ s \f$ as follows when the row is completed:
 * \f[
 *   \begin{array}{lcl}
 *      L & \leftarrow & \sqrt{\sum_{j} a_{i,j}^2} \\
 *      a_{i,j} & \leftarrow & a_{i,j} \times s / L\\
 *      b_i     & \leftarrow & b_i     \times s / L 
 *  \end{array}
 * \f]
 * \param[in] value the row scaling.
 *  \note The row scaling is used by the next row, and reset to 1.0 after
 *  completion of nlEnd(NL_ROW).
 * \pre This function should be called after nlBegin(NL_MATRIX) 
 *  and before nlBegin(NL_ROW).
 */
    NLAPI void NLAPIENTRY nlRowScaling(NLdouble value);
    
/**
 * @}
 * \name Solve
 * @{
 */

/**
 * \brief Solves the linear system in the current context.
 * \details This function should be called after nlEnd(NL_SYSTEM),
 *  else an assertion failure is triggered. Once the function is
 *  called, client code may get the value of the computed variables
 *  using nlGetVariable(). 
 */
    NLAPI NLboolean NLAPIENTRY nlSolve(void);


/**
 * \brief Updates the right hand side of the constructed system in
 *  one call.
 * \param[in] values a pointer to an array of N doubles, where N 
 *  corresponds to the number of not locked variables.
 * \details If the current state is solved, it resets the current
 *  state to constructed. This function cannot be called if 
 *  NL_NB_SYSTEMS is different from 1.
 */
    NLAPI void NLAPIENTRY nlUpdateRightHandSide(NLdouble* values);

/**
 * @}
 * \name Buffers
 * @{
 */

/**
 * \brief The symbolic constant for variables buffer.
 * \details Used as an argument of nlEnable() / nlDisable() / nlIsEnabled() and
 *   as an argument of nlBindBuffer.
 */
#define NL_VARIABLES_BUFFER 0x1000

/**
 * \brief Specifies a buffer binding to directly map user data to variables 
 *  instead of using nlGetVariable() / nlSetVariable()
 * \details NL_VARIABLES_BUFFER needs to be previouslyu nlEnabled() else 
 *  this function has no effect.
 * \param[in] buffer the type of the buffer, NL_VARIABLES_BUFFER is the 
 *   only supported value
 * \param[in] k the index of the buffer. If type = NL_VARIABLES_BUFFER, this
 *   corresponds to the index of the linear system (0 if there is a single
 *   linear system).
 * \param[in] addr the address of the array to be bound.
 * \param[in] stride number of bytes between two consecutive elements.
 */
    NLAPI void NLAPIENTRY nlBindBuffer(
	NLenum buffer, NLuint k, void* addr, NLuint stride
    );

    
/**
 * @}
 * \name EigenSolver
 * @{
 */    

/**
 * \brief Constant for nlMatrixMode()
 */
#define NL_STIFFNESS_MATRIX 0x3001

/**
 * \brief Constant for nlMatrixMode()
 */
#define NL_MASS_MATRIX 0x3002

/**
 * \brief Specifies to which matrix the subsequent calls
 *  to nlBegin(), nlEnd(), nlCoefficient(), nlAddIJCoefficient()
 *  will be applied.
 * \param[in] matrix one of NL_STIFFNESS_MATRIX (default), NL_MASS_MATRIX
 * \details Calling nlMatrixMode() resets the current row to 0.
 */
NLAPI void NLAPIENTRY nlMatrixMode(NLenum matrix);
    
/**
 * \brief Symbolic constant for nlEigenSolverParameteri(),
 *   number of eigenpairs to compute.
 */
#define NL_NB_EIGENS       NL_NB_SYSTEMS

/**
 * \brief Symbolic constant for nlEigenSolverParameteri(),
 *   maximum number of iterations.
 */
#define NL_EIGEN_MAX_ITERATIONS NL_MAX_ITERATIONS
    
/**
 * \brief Symbolic constant for nlEigenSolverParameterd(),
 *   convergence threshold.
 */
#define NL_EIGEN_THRESHOLD NL_THRESHOLD

/**
 * \brief Symbolic constant for nlEigenSolverParameterd(),
 *   name of the eigen solver to be used.
 */
#define NL_EIGEN_SOLVER 0x2000
    
/**
 * \brief Symbolic constant for nlEigenSolverParameterd(),
 *   shift parameter for the shift-invert transform
 */
#define NL_EIGEN_SHIFT 0x2001

/**
 * \brief Symbolic constant for nlEnable() / nlDisable() / nlIsEnabled(),
 *  shift-invert spectral transform.
 */
#define NL_EIGEN_SHIFT_INVERT 0x2002
    
/**
 * \brief Sets a floating-point parameter of the eigen solver.
 * \param pname symbolic name of the parameter,
 *   one of NL_EIGEN_SHIFT, NL_EIGEN_THRESHOLD.
 */
    NLAPI void NLAPIENTRY nlEigenSolverParameterd(
	NLenum pname, NLdouble val
    );

/**
 * \brief Sets an integer parameter of the eigen solver.
 * \param pname symbolic name of the parameter,
 *   one of NL_EIGEN_SOLVER, NL_NB_EIGENS, NL_NB_VARIABLES, 
 *   NL_EIGEN_MAX_ITERATIONS, NL_SYMMETRIC.
 */
    NLAPI void NLAPIENTRY nlEigenSolverParameteri(
	NLenum pname, NLint val
    );

/**
 * \brief Calls the eigen solver.
 */
    NLAPI void NLAPIENTRY nlEigenSolve(void);


/**
 * \brief Gets an eigenvalue.
 * \param i index of the eigenvalue.
 * \return the \p i th eigenvalue.
 */
    NLAPI double NLAPIENTRY nlGetEigenValue(NLuint i);

/**
 * @}
 * \name Logging and messages
 * @{
 */    

    /**
     * \brief Function pointer type for user printf function.
     */
    typedef int (*NLprintfFunc)(const char* format, ...);

    /**
     * \brief Function pointer type for user fprintf function.
     */
    typedef int (*NLfprintfFunc)(FILE* out, const char* format, ...);
    
    /**
     * \brief Specifies user functions for printing messages.
     */
    NLAPI void NLAPIENTRY nlPrintfFuncs(NLprintfFunc f1, NLfprintfFunc f2);
    
    
/**
 * @}
 */    

#ifdef __cplusplus
}
#endif

#include "nl_ext.h"
#include "nl_64.h"

/*************************************************************************/
    
#endif
