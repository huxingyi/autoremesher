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

/**
 * \file geogram/NL/nl_blas.h
 * \brief Abstract interface to the basic linear algebra routines 
 *  used by OpenNL internally.
 */

#ifndef OPENNL_BLAS_H
#define OPENNL_BLAS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief The opaque structure used by the BLAS abstraction layer.
 */
struct NLBlas;

/**
 * \brief A handle to a BLAS abstraction layer.
 */
typedef struct NLBlas* NLBlas_t;

/**
 * \brief Specifies whether matrix should be transposed.
 * \details Used by dtpsv() and dgemv()
 */
typedef enum {
    NoTranspose=0, Transpose=1, ConjugateTranspose=2
} MatrixTranspose ;

/**
 * \brief Specifies which triangular part of a matrix should be used.
 * \details Used by dtpsv() 
 */
typedef enum {
    UpperTriangle=0, LowerTriangle=1
} MatrixTriangle ;

/**
 * \brief Specifies which triangular part of a matrix should be used.
 * \details Used by dtpsv() 
 */
typedef enum {
    UnitTriangular=0, NotUnitTriangular=1
} MatrixUnitTriangular ;

/**
 * \brief Specifies on which type of memory a function should be applied.
 * \details NL_HOST_MEMORY refers to CPU RAM and NL_DEVICE_MEMORY to GPU
 *  RAM.
 */
typedef enum {
    NL_HOST_MEMORY, NL_DEVICE_MEMORY
} NLmemoryType;

/**
 * \brief Allocates memory in host or in device.
 * \param[in] blas a handle to the BLAS abstraction layer
 * \param[in] type whether RAM should be allocated on the CPU
 *  (NL_HOST_MEMORY) or the GPU (NL_DEVICE_MEMORY)
 * \param[in] size the number of bytes of the memory bloc to be
 *  allocated.
 * \return a pointer to the allocated memory.
 */
typedef void* (*FUNPTR_malloc)(
    NLBlas_t blas, NLmemoryType type, size_t size
);

/**
 * \brief Frees memory from host or from device.
 * \param[in] blas a handle to the BLAS abstraction layer
 * \param[in] type whether RAM should be allocated on the CPU
 *  (NL_HOST_MEMORY) or the GPU (NL_DEVICE_MEMORY)
 * \param[in] size the number of bytes of the memory bloc to be
 *  allocated.
 * \param[in] ptr a pointer to the memory to be freed.
 */
typedef void (*FUNPTR_free)(
    NLBlas_t blas, NLmemoryType type, size_t size, void* ptr
);

/**
 * \brief Copies a bloc of memory.
 * \param[in] blas a handle to the BLAS abstraction layer.
 * \param[in] to , to_type a pointer to the destination and
 *  indication of whether the destination is on CPU RAM 
 *  (NL_HOST_MEMORY) or GPU RAM (NL_DEVICE_MEMORY).
 * \param[in] from , frm_type a pointer to the source and
 *  indication of whether the source is on CPU RAM 
 *  (NL_HOST_MEMORY) or GPU RAM (NL_DEVICE_MEMORY).
 * \param[in] size number of bytes to be copied.
 */
typedef void (*FUNPTR_memcpy)(
    NLBlas_t blas,
    void* to, NLmemoryType to_type,
    void* from, NLmemoryType from_type,
    size_t size
);

/**
 * \brief Copies a vector.
 * \details In formula: \f$ y \leftarrow x \f$
 * \param[in] blas a handle to the BLAS abstraction layer
 * \param[in] n dimension of the vector
 * \param[in] x source vector
 * \param[in] incx straddle between two consecutive coefficients
 *  of the source vector
 * \param[out] y destination vector
 * \param[in] incy straddle between two consecutive coefficients
 *  of the destination vector
 */
typedef void (*FUNPTR_dcopy)(
    NLBlas_t blas, int n, const double *x, int incx, double *y, int incy
);

/**
 * \brief Scales a vector.
 * \details In formula: \f$ x \leftarrow a x \f$
 * \param[in] blas a handle to the BLAS abstraction layer
 * \param[in] n dimension of the vector
 * \param[in] a scaling coefficient
 * \param[in,out] x vector to be scaled
 * \param[in] incx straddle between two consecutive coefficients
 */
typedef void (*FUNPTR_dscal)(
    NLBlas_t blas, int n, double a, double *x, int incx
);


/**
 * \brief Computes the dot product between two vectors
 * \param[in] blas a handle to the BLAS abstraction layer
 * \param[in] n dimension of the vectors
 * \param[in] x first vector
 * \param[in] incx straddle between two consecutive coefficients
 *  of the first vector
 * \param[in] y second vector
 * \param[in] incy straddle between two consecutive coefficients
 *  of the second vector
 * \return the dot product between \p x and \p y
 */
typedef double (*FUNPTR_ddot)(
    NLBlas_t blas, int n, const double *x, int incx, const double *y, int incy
);

/**
 * \brief Computes the norm of a vector
 * \param[in] blas a handle to the BLAS abstraction layer
 * \param[in] n dimension of the vector
 * \param[in] x the vector
 * \param[in] incx straddle between two consecutive coefficients 
 *  of the vector
 * \return the norm of \p x
 */
typedef double (*FUNPTR_dnrm2)(NLBlas_t blas, int n, const double *x, int incx);

/**
 * \brief Computes a linear combination of two vectors
 * \details In formula: \f$ y \leftarrow a x + y \f$
 * \param[in] blas a handle to the BLAS abstraction layer
 * \param[in] n dimension of the vectors
 * \param[in] a scaling coefficient
 * \param[in] x source vector to be scaled and added
 * \param[in] incx straddle between two consecutive coefficients
 *  of the source vector
 * \param[in,out] y the vector that \p a \p x should be added to
 * \param[in] incy straddle between two consecutive coefficients
 *  of the destination vector
 */
typedef void (*FUNPTR_daxpy)(
    NLBlas_t blas, int n,
    double a, const double *x, int incx, double *y, int incy
);


/**
 * \brief Computes a matrix-vector product
 * \details performs one of the matrix-vector operations   
 * \f$ y = \alpha A x + \beta y \f$   or 
 * \f$ y = \alpha A^t x + \beta y \f$,
 * where \f$ \alpha \f$ and \f$ \beta \f$ are scalars, 
 *  x and y are vectors and A is an m by n matrix.   
 * \param[in] blas a handle to the BLAS abstraction layer
 * \param[in] trans one of (NoTranspose, Transpose, ConjugateTranspose), 
 *  specifies the operation to be performed as follows:
 *  - NoTranspose: \f$ y = \alpha A x + \beta y \f$
 *  - Transpose: \f$ y = \alpha A^t x + \beta y \f$
 *  - ConjugateTranspose: \f$ y = \alpha A^t x + \beta y \f$
 * \param[in] m number of rows of the matrix A
 * \param[in] n number of columns of the matrix A
 * \param[in] alpha the scalar \f$ \alpha \f$
 * \param[in] A an array of dimension (ldA,n). On entry, 
 *  the leading m by n part of the array A must contain 
 *  the array of coefficients.
 * \param[in] ldA specifies the first dimension of A as declared 
 *  by the caller. ldA must be at least max(1,m).  
 * \param[in] x array of dimension at least   
 *  ( 1 + ( n - 1 )*abs( incx ) ) when trans = NoTranspose
 *  and at least  ( 1 + ( m - 1 )*abs( incx ) ) otherwise.   
 *  Before entry, the incremented array X must contain the   
 *  vector x.   
 * \param[in] incx the increment for the elements of x
 * \param[in] beta the scalar \f$ \beta \f$
 * \param[in,out] y an array of dimension
 *  ( 1 + ( m - 1 )*abs( incy ) ) when trans = NoTranspose
 *  and at least ( 1 + ( n - 1 )*abs( incy ) ) otherwise.   
 *  Before entry with beta non-zero, the incremented array y
 *  must contain the vector y. On exit, y is overwritten by the 
 *  updated vector y.   
 * \param[in] incy the increment for the elements of y
 */
typedef void (*FUNPTR_dgemv)( 
    NLBlas_t blas, MatrixTranspose trans, int m, int n, double alpha,
    const double *A, int ldA, const double *x, int incx,
    double beta, double *y, int incy 
);


/**
 * \brief Solves a linear system
 * \details Solves one of the systems of equations 
 *   \f$ A x = b \f$   or   \f$ A^t x = b \f$
 *  where b and x are n element vectors and A is an n by n unit, or 
 *  non-unit, upper or lower triangular matrix, supplied in packed form.
 *  No test for singularity or near-singularity is included in this 
 *  routine. Such tests must be performed before calling this routine. 
 * \param[in] blas a handle to the BLAS abstraction layer
 * \param[in] uplo one of (UpperTriangle, LowerTriangle), specifies
 *  whether A is an upper or lower triangular matrix as follows:
 *  - UpperTriangle: A is an upper triangular matrix
 *  - LowerTriangle: A is a lower triangular matrix
 * \param[in] trans one of (NoTranspose, Transpose, ConjugateTranspose), 
 *  specifies the equations to be solved as follows:
 *  - NoTranspose: \f$ A x = b \f$
 *  - Transpose: \f$ A^t x = b \f$
 *  - ConjugateTranspose: \f$ A^t x = b \f$
 * \param[in] diag one of (UnitTriangular, NotUnitTriangular), 
 *  specifies whether or not A is unit triangular as follows:
 *  - UnitTriangular: A is assumed to be unit triangular
 *  - NotUnitTriangular: A is not assumed to be unit triangular
 * \param[in] n the order of the matrix
 * \param[in] AP an array of dimension at least ( ( n*( n + 1))/2).
 * - if uplo = UpperTriangular, the array AP  must 
 *  contain the upper triangular matrix packed sequentially, 
 *  column by column, so that AP[0] contains a(1,1), 
 *  AP[1] and AP[2] contain a(1,2) and a(2,2) respectively, and so on. 
 * - if uplo = LowerTriangular, the array AP must 
 *  contain the lower triangular matrix packed sequentially, 
 *  column by column, so that AP[0] contains a(1,1 ), 
 *  AP[1] and AP[2] contain a(2,1) and a(3,1) respectively, and so on. 
 * Note that when diag = UnitTriangular, the diagonal elements of 
 *  A are not referenced, but are assumed to be unity. 
 * \param[in,out] x array of dimension at least (1 + (n-1)*abs( incx )).
 *  Before entry, the incremented array x must contain the n 
 *  element right-hand side vector b. On exit, x is overwritten 
 *  with the solution vector x. 
 * \param[in] incx specifies the increment for the elements of x. 
 *  Must not be zero. 
 */
typedef void (*FUNPTR_dtpsv)(
    NLBlas_t blas, MatrixTriangle uplo, MatrixTranspose trans,
    MatrixUnitTriangular diag, int n, const double *AP,
    double *x, int incx 
);

struct NLBlas {
    FUNPTR_malloc Malloc;
    FUNPTR_free Free;
    FUNPTR_memcpy Memcpy;

    FUNPTR_dcopy Dcopy;
    FUNPTR_dscal Dscal;
    FUNPTR_ddot  Ddot;
    FUNPTR_dnrm2 Dnrm2;
    FUNPTR_daxpy Daxpy;
    FUNPTR_dgemv Dgemv;
    FUNPTR_dtpsv Dtpsv;

    NLboolean has_unified_memory;
    double start_time;
    NLulong flops;
    NLulong used_ram[2];
    NLulong max_used_ram[2];
    
    /* 
     * Used for stats of the linear solver
     * (a bit ugly, should not be here, but
     * more convenient for now...)
     */
    double sq_rnorm; 
    double sq_bnorm;
};

/**
 * \brief Tests whether BLAS device uses the same address
 *  space as the CPU. 
 * \param[in] blas a pointer to the BLAS abstraction layer.
 * \return NL_TRUE if device and CPU use the same address space.
 * \retval NL_FALSE otherwise.
 */
NLAPI NLboolean NLAPIENTRY nlBlasHasUnifiedMemory(NLBlas_t blas);

/**
 * \brief Restes the flops and memory used statistics.
 * \param[in] blas a pointer to the BLAS abstraction layer.
 */
NLAPI void NLAPIENTRY nlBlasResetStats(NLBlas_t blas);

/**
 * \brief Gets the number of floating point operations per
 *  seconds computed since creation of the BLAS abstaction
 *  layer or since the latest call to nlBlasResetStats().
 * \return the number of GFlops.
 */
NLAPI double NLAPIENTRY nlBlasGFlops(NLBlas_t blas);

/**
 * \brief Gets the currently used amount of memory 
 *  computed since creation of the BLAS abstaction
 *  layer or since the latest call to nlBlasResetStats().
 * \param[in] blas a pointer to the BLAS abstraction layer.
 * \param[in] type one of NL_HOST_MEMORY, NL_DEVICE_MEMORY.
 * \return the amount of used memory, in bytes.
 */
NLAPI NLulong NLAPIENTRY nlBlasUsedRam(NLBlas_t blas, NLmemoryType type);

/**
 * \brief Gets the high mark of used amount of memory
 *  computed since creation of the BLAS abstaction
 *  layer or since the latest call to nlBlasResetStats().
 * \param[in] blas a pointer to the BLAS abstraction layer.
 * \param[in] type one of NL_HOST_MEMORY, NL_DEVICE_MEMORY.
 * \return the high mark of the used memory, in bytes.
 */
NLAPI NLulong NLAPIENTRY nlBlasMaxUsedRam(NLBlas_t blas, NLmemoryType type);

/**
 * \brief Gets a pointer to the BLAS abstraction layer for
 *  BLAS operation on the host CPU.
 * \return a pointer to the BLAS abstraction layer.
 */
NLAPI NLBlas_t NLAPIENTRY nlHostBlas(void);

/**
 * \brief Allocates a vector of doubles;
 * \param[in] blas a pointer to the BLAS abstraction layer.
 * \param[in] memtype one of NL_HOST_MEMORY, NL_DEVICE_MEMORY.
 * \param[in] dim number of components in the vector.
 */
#define NL_NEW_VECTOR(blas, memtype, dim) \
    (double*)blas->Malloc(blas,memtype,(size_t)(dim)*sizeof(double))

/**
 * \brief Deletes a vector of doubles;
 * \param[in] blas a pointer to the BLAS abstraction layer.
 * \param[in] memtype one of NL_HOST_MEMORY, NL_DEVICE_MEMORY.
 * \param[in] dim number of components in the vector.
 * \param[in] ptr a pointer to the vector to be deleted.
 */
#define NL_DELETE_VECTOR(blas, memtype, dim, ptr) \
    blas->Free(blas,memtype,(size_t)(dim)*sizeof(double),ptr)

/******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif
