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

#ifndef OPENNL_MATRIX_H
#define OPENNL_MATRIX_H

/**
 * \file geogram/NL/nl_matrix.h
 * \brief Internal OpenNL functions to manipulate sparse matrices.
 */

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Abstract matrix interface */

struct NLMatrixStruct;
typedef struct NLMatrixStruct* NLMatrix;

/**
 * \brief Function pointer type for matrix destructors.
 */    
typedef void(*NLDestroyMatrixFunc)(NLMatrix M);    

/**
 * \brief Function pointer type for matrix x vector product.
 */
typedef void(*NLMultMatrixVectorFunc)(NLMatrix M, const double* x, double* y);

#define NL_MATRIX_SPARSE_DYNAMIC 0x1001
#define NL_MATRIX_CRS            0x1002
#define NL_MATRIX_SUPERLU_EXT    0x1003    
#define NL_MATRIX_CHOLMOD_EXT    0x1004    
#define NL_MATRIX_FUNCTION       0x1005
#define NL_MATRIX_OTHER          0x1006
    
/**
 * \brief The base class for abstract matrices.
 */
struct NLMatrixStruct {
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
     * \details One of NL_MATRIX_SPARSE_DYNAMIC, 
     *  NL_MATRIX_CRS, NL_MATRIX_SUPERLU_EXT,
     *  NL_CHOLDMOD_MATRIX_EXT, NL_MATRIX_FUNCTION,
     *  NL_MATRIX_OTHER
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
};

/**
 * \brief Deletes a matrix
 * \details If \p M is NULL, then the function does nothing
 * \param[in] M the matrix to be deleted
 */
NLAPI void NLAPIENTRY nlDeleteMatrix(NLMatrix M);

/**
 * \brief Computes a matrix x vector product
 * \param[in] M the matrix
 * \param[in] x the vector to be multiplied by the matrix
 * \param[out] y the result
 */
NLAPI void NLAPIENTRY nlMultMatrixVector(
    NLMatrix M, const double* x, double* y
);
    
/******************************************************************************/
/* Dynamic arrays for sparse row/columns */

/**
 * \brief Represents a coefficient in a sparse matrix
 * \relates NLSparseMatrix
 */
typedef struct  {
    /**
     * \brief index of the coefficient.
     */    
    NLuint index;

    /**
     * \brief value of the coefficient. 
     */    
    NLdouble value; 
} NLCoeff;

/**
 * \brief Represents a row or a column of a sparse matrix
 * \relates NLSparseMatrix
 */
typedef struct {
    /**
     * \brief number of coefficients. 
     */    
    NLuint size;
    
    /** 
     * \brief number of coefficients that can be 
     * stored without reallocating memory.
     */    
    NLuint capacity;

    /**
     * \brief the array of coefficients, with enough
     * space to store capacity coefficients.
     */
    NLCoeff* coeff;  
} NLRowColumn;

/**
 * \brief Constructs a new NLRowColumn
 * \param[in,out] c a pointer to an 
 *  uninitialized NLRowColumn
 * \relates NLRowColumn
 */
NLAPI void NLAPIENTRY nlRowColumnConstruct(NLRowColumn* c);

/**
 * \brief Destroys a NLRowColumn
 * \details Only the memory allocated by the 
 *  NLRowColumn is freed. The NLRowColumn structure
 *  is not freed.
 * \param[in,out] c a pointer to an NLRowColumn
 * \relates NLRowColumn
 */
NLAPI void NLAPIENTRY nlRowColumnDestroy(NLRowColumn* c);

/**
 * \brief Allocates additional storage for
 *  the coefficients of an NLRowColumn
 * \details Operates like the class vector of the C++ standard library, 
 *  by doubling the capacity each time it is needed. This amortizes
 *  the cost of the growing operations as compared to re-allocating
 *  at each element insertion
 * \param[in,out] c a pointer to an NLRowColumn
 * \relates NLRowColumn
 */
NLAPI void NLAPIENTRY nlRowColumnGrow(NLRowColumn* c);

/**
 * \brief Adds a coefficient to an NLRowColumn.    
 * \details Performs the following operation:
 *  \f$ a_i \leftarrow a_i + value \f$. If the NLRowColumn
 *  already has a coefficient with index \p index, then 
 *  the value is added to that coefficient, else a new
 *  coefficient is created. Additional storage is allocated
 *  as need be.
 * \param[in,out] c a pointer to an NLRowColumn
 * \param[in] index index of the coefficient
 * \param[in] value value of the coefficient
 * \relates NLRowColumn
 */
NLAPI void NLAPIENTRY nlRowColumnAdd(
    NLRowColumn* c, NLuint index, NLdouble value
);

/**
 * \brief Appends a coefficient to an NLRowColumn    .
 * \details In contrast with nlRowColumnAdd(), this function does
 *  not tests whether a coefficient with index \p index already exists
 *  in the NLRowColumn. A new coefficient is always created.
 * \param[in,out] c a pointer to an NLRowColumn
 * \param[in] index index of the coefficient
 * \param[in] value value of the coefficient
 * \relates NLRowColumn
 */
NLAPI void NLAPIENTRY nlRowColumnAppend(
    NLRowColumn* c, NLuint index, NLdouble value
);

/**
 * \brief Zeroes an NLRowColumn.
 * \details No memory is deallocated, the capacity remains
 *  the same.
 * \param[in,out] c a pointer to an NLRowColumn
 * \relates NLRowColumn
 */
NLAPI void NLAPIENTRY nlRowColumnZero(NLRowColumn* c);

/**
 * \brief Zeroes an NLRowColumn and deallocates the memory
 *  used by the NLRowColumn.
 * \details On exit, capacity is zeroed
 * \param[in,out] c a pointer to an NLRowColumn
 * \relates NLRowColumn
 */
NLAPI void NLAPIENTRY nlRowColumnClear(NLRowColumn* c);

/**
 * \brief Sorts the coefficients of an NLRowColumn
 *  by increasing index
 * \param[in,out] c a pointer to an NLRowColumn
 * \relates NLRowColumn
 */
NLAPI void NLAPIENTRY nlRowColumnSort(NLRowColumn* c);

/******************************************************************************/
/* Compressed Row Storage */

/**
 * \brief Type for internal indices.
 * \details Matrix dimension is limited to 4G x 4G. Number of non-zero entries
 *  is limited to 2^32 (4G) in standard mode, and to 2^64 in GARGANTUA mode.
 */
#ifdef GARGANTUA
    typedef NLulong NLuint_big;
#else
    typedef NLuint NLuint_big;
#endif
    
    
/**
 * \brief A compact self-contained storage for 
 *  sparse matrices.
 * \details Unlike with NLSparseMatrix, it is not possible
 *  to add new coefficients in an NLCRSMatrix.
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
     * \details One of NL_MATRIX_DYNAMIC, NL_MATRIX_CRS, NL_MATRIX_SUPERLU_EXT,
     *  NL_CHOLDMOD_MATRIX_EXT
     */
    NLenum type;
    
    /**
     * \brief destructor
     */
    NLDestroyMatrixFunc destroy_func;

    /**
     * \brief Matrix x vector product
     */
    NLMultMatrixVectorFunc mult_func;
    
    /**
     * \brief array of coefficient values, 
     * size = NNZ (number of non-zero coefficients)
     */    
    NLdouble* val;    

    /**
     * \brief row pointers, size = m+1 
     */    
    NLuint_big* rowptr;

    /**
     * \brief column indices, size = NNZ 
     */    
    NLuint* colind;

    /**
     * \brief number of slices, used by parallel spMv
     */    
    NLuint nslices;

    /** 
     * \brief slice pointers, size = nslices + 1, 
     * used by parallel spMv 
     */    
    NLuint* sliceptr;

    /**
     * \brief NL_TRUE if symmetric storage is used,
     *  NL_FALSE otherwise.
     */
    NLboolean symmetric_storage;
} NLCRSMatrix;

/**
 * \brief Constructs a new NLCRSMatrix
 * \param[in,out] M pointer to an uninitialized NLCRSMatrix
 * \param[in] m number of rows
 * \param[in] n number of columns
 * \param[in] nnz number of non-zero coefficientsz
 * \param[in] nslices number of slices, used by parallel spMv
 *  (typically, nslices = number of cores)
 * \relates NLCRSMatrix
 */
NLAPI void NLAPIENTRY nlCRSMatrixConstruct(
    NLCRSMatrix* M, NLuint m, NLuint n, NLuint_big nnz, NLuint nslices
);

/**
 * \brief Constructs a new NLCRSMatrix with symmetric storage
 * \param[in,out] M pointer to an uninitialized NLCRSMatrix
 * \param[in] n number of rows and columns
 * \param[in] nnz number of non-zero coefficients
 * \relates NLCRSMatrix
 */
NLAPI void NLAPIENTRY nlCRSMatrixConstructSymmetric(
    NLCRSMatrix* M, NLuint n, NLuint_big nnz
);


/**
 * \brief Constructs a new NLCRSMatrix with only the pattern.
 * \details This function should be used when the number of coefficients
 *  in each row is not known in advance.
 * \param[in,out] M pointer to an uninitialized NLCRSMatrix
 * \param[in] m number of rows
 * \param[in] n number of columns
 * \relates NLCRSMatrix
 */
NLAPI void NLAPIENTRY nlCRSMatrixConstructPattern(
    NLCRSMatrix* M, NLuint m, NLuint n
);

/**
 * \brief Constructs a new NLCRSMatrix with symmetric storage
 * \details This function should be used when the number of coefficients
 *  in each row is not known in advance.
 * \param[in,out] M pointer to an uninitialized NLCRSMatrix
 * \param[in] n number of rows and columns
 * \param[in] nnz number of non-zero coefficients
 * \relates NLCRSMatrix
 */
NLAPI void NLAPIENTRY nlCRSMatrixConstructPatternSymmetric(
    NLCRSMatrix* M, NLuint n
);

/**
 * \brief Specifies the number of non-zero entries in the row of a 
 *  matrix that was constructed by nlCRSMatrixConstructPattern() or
 *  nlCRSMatrixConstructPatternSymmetric().
 */
NLAPI void NLAPIENTRY nlCRSMatrixPatternSetRowLength(
    NLCRSMatrix* M, NLuint i, NLuint n
);


/**
 * \brief Intializes a NLCRSMatrix from the pattern (row
 *  lengths).
 * \param[in] M a pointer to the NLCRSMatrix to be compiled.
 */
NLAPI void NLAPIENTRY nlCRSMatrixPatternCompile(NLCRSMatrix* M);    

/**
 * \brief Adds a coefficient to an NLSparseMatrix
 * \details Performs the following operation:
 *  \$ a_{i,j} \leftarrow a_{i,j} + \mbox{value} \$
 * \param[in,out] M a pointer to an NLSparseMatrix
 * \param[in] i index of the row
 * \param[in] j index of the column
 * \param[in] value the coefficient to be added
 * \relates NLSparseMatrix
 */
NLAPI void NLAPIENTRY nlCRSMatrixAdd(
    NLCRSMatrix* M, NLuint i, NLuint j, NLdouble value
);

    
/**
 * \brief Loads a NLCRSMatrix from a file
 * \param[out] M a pointer to an uninitialized NLCRSMatriix 
 * \param[in] filename the name of the file
 * \retval NL_TRUE on success
 * \retval NL_FALSE on error
 * \relates NLCRSMatrix
 */
NLAPI NLboolean NLAPIENTRY nlCRSMatrixLoad(
    NLCRSMatrix* M, const char* filename
);

/**
 * \brief Saves a NLCRSMatrix into a file
 * \param[in] M a pointer to the NLCRSMatriix 
 * \param[in] filename the name of the file
 * \retval NL_TRUE on success
 * \retval NL_FALSE on error
 * \relates NLCRSMatrix
 */
NLAPI NLboolean NLAPIENTRY nlCRSMatrixSave(
    NLCRSMatrix* M, const char* filename
);

/**
 * \brief Gets the number of non-zero coefficient
 *  in an NLCRSMatrix
 * \param[in] M a pointer to the NLCRSMatrix
 * \return the number of non-zero coefficients in \p M
 * \relates NLCRSMatrix
 */
NLAPI NLuint_big NLAPIENTRY nlCRSMatrixNNZ(NLCRSMatrix* M);
    
/******************************************************************************/
/* SparseMatrix data structure */

/**
 * for NLSparseMatrix storage: indicates that rows are stored.
 * \relates NLSparseMatrix
 */
#define NL_MATRIX_STORE_ROWS          1

/**
 * for NLSparseMatrix storage: indicates that columns are stored.
 * \relates NLSparseMatrix
 */
#define NL_MATRIX_STORE_COLUMNS       2

/**
 * for NLSparseMatrix storage: indicates that symmetric storage
 * is used (only the lower triangular part is stored).
 * \relates NLSparseMatrix
 */
#define NL_MATRIX_STORE_SYMMETRIC     4
    
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
     * \details One of NL_MATRIX_DYNAMIC, NL_MATRIX_CRS, NL_MATRIX_SUPERLU_EXT,
     *  NL_CHOLDMOD_MATRIX_EXT
     */
    NLenum type;
    
    /**
     * \brief destructor
     */
    NLDestroyMatrixFunc destroy_func;

    /**
     * \brief Matrix x vector product
     */
    NLMultMatrixVectorFunc mult_func;

    
    /**
     * \brief number of elements in the diagonal 
     */    
    NLuint diag_size;

    /**
     * \brief Number of elements allocated to store the diagonal
     */
    NLuint diag_capacity;
    
    /**
     * \brief indicates what is stored in this matrix 
     */    
    NLenum storage;

    /**
     * \brief the rows if (storage & NL_MATRIX_STORE_ROWS), size = m,
     * NULL otherwise
     */     
    NLRowColumn* row;

    /** 
     * \brief the columns if (storage & NL_MATRIX_STORE_COLUMNS), size = n,
     * NULL otherwise
     */     
    NLRowColumn* column;

    /**
     * \brief the diagonal elements, size = diag_size 
     */     
    NLdouble*    diag;

    /**
     * \brief Number of row descriptors allocated in the row
     *  array.
     */
    NLuint row_capacity;

    /**
     * \brief Number of column descriptors allocated in the 
     *  column array.
     */
    NLuint column_capacity;
    
} NLSparseMatrix;


/**
 * \brief Constructs a new NLSparseMatrix
 * \param[in] m number of rows
 * \param[in] n number of columns
 * \param[in] storage a bitwise or combination of flags that
 *  indicate what needs to be stored in the matrix.
 * \return a pointer to a dynamically allocated NLSparseMatrix.
 *   It can be later deallocated by nlDeleteMatrix().
 * \relates NLSparseMatrix
 */
NLAPI NLMatrix NLAPIENTRY nlSparseMatrixNew(
    NLuint m, NLuint n, NLenum storage
);

/**
 * \brief Constructs a new NLSparseMatrix
 * \param[in,out] M a pointer to an uninitialized NLSparseMatrix
 * \param[in] m number of rows
 * \param[in] n number of columns
 * \param[in] storage a bitwise or combination of flags that
 *  indicate what needs to be stored in the matrix.
 * \relates NLSparseMatrix
 */
NLAPI void NLAPIENTRY nlSparseMatrixConstruct(
    NLSparseMatrix* M, NLuint m, NLuint n, NLenum storage
);

/**
 * \brief Destroys an NLSparseMatrix
 * \details Only the memory allocated by the NLSparseMatrix
 *  is freed. The NLSparseMatrix structure is not freed.
 * \param[in,out] M a pointer to an NLSparseMatrix
 * \relates NLSparseMatrix
 */
NLAPI void NLAPIENTRY nlSparseMatrixDestroy(NLSparseMatrix* M);

/**
 * \brief Computes a matrix-vector product
 * \param[in] A a pointer to the matrix
 * \param[in] x the vector to be multiplied, size = A->n
 * \param[in] y where to store the result, size = A->m
 * \relates NLSparseMatrix
 */
NLAPI void NLAPIENTRY nlSparseMatrixMult(
    NLSparseMatrix* A, const NLdouble* x, NLdouble* y
);    
    
/**
 * \brief Adds a coefficient to an NLSparseMatrix
 * \details Performs the following operation:
 *  \$ a_{i,j} \leftarrow a_{i,j} + \mbox{value} \$
 * \param[in,out] M a pointer to an NLSparseMatrix
 * \param[in] i index of the row
 * \param[in] j index of the column
 * \param[in] value the coefficient to be added
 * \relates NLSparseMatrix
 */
NLAPI void NLAPIENTRY nlSparseMatrixAdd(
    NLSparseMatrix* M, NLuint i, NLuint j, NLdouble value
);

/**
 * \brief Adds a matrix to another sparse matrix.
 * \details Does M += mul * N. If N has symmetric storage, then
 *  M needs also to have symmetric storage.
 * \param[in,out] M a pointer to an NLSparseMatrix
 * \param[in] mul a multiplicative factor that pre-multiply
 *   all coefficients of \p N
 * \param[in] N a matrix. Needs to be either a NLSparseMatrix or
 *   a NLCRSMatrix.
 */
NLAPI void NLAPIENTRY nlSparseMatrixAddMatrix(
    NLSparseMatrix* M, double mul, const NLMatrix N
);	
    
/**
 * \brief Zeroes an NLSparseMatrix
 * \details The memory is not freed.
 * \param[in,out] M a pointer to the NLSparseMatrix to zero
 * \relates NLSparseMatrix
 */
NLAPI void NLAPIENTRY nlSparseMatrixZero( NLSparseMatrix* M);

/**
 * \brief Clears an NLSparseMatrix
 * \details The memory is freed.
 * \param[in,out] M a pointer to the NLSparseMatrix to zero
 * \relates NLSparseMatrix
 */
NLAPI void NLAPIENTRY nlSparseMatrixClear( NLSparseMatrix* M);

/**
 * \brief Gets the number of non-zero coefficient
 *  in an NLSparseMatrix
 * \param[in] M a pointer to the NLSparseMatrix
 * \return the number of non-zero coefficients in \p M
 * \relates NLSparseMatrix
 */
NLAPI NLuint_big NLAPIENTRY nlSparseMatrixNNZ( NLSparseMatrix* M);

/**
 * \brief Sorts the coefficients in an NLSParseMatrix
 * \param[in,out] M a pointer to the NLSparseMatrix 
 * \relates NLSparseMatrix
 */
NLAPI void NLAPIENTRY nlSparseMatrixSort( NLSparseMatrix* M);

/**
 * \brief Adds a new empty row to a sparse matrix.
 * \param[in,out] M a pointer to the sparse matrix.
 */
NLAPI void NLAPIENTRY nlSparseMatrixAddRow( NLSparseMatrix* M);

/**
 * \brief Adds a new empty column to a sparse matrix.
 * \param[in,out] M a pointer to the sparse matrix.
 */
NLAPI void NLAPIENTRY nlSparseMatrixAddColumn( NLSparseMatrix* M);

/**
 * \brief Adds a row of a sparse matrix to another row.
 *   (row[i1] += s * row[i2]).
 * \param[in,out] M a pointer to a sparse matrix.
 * \param[in] i1 index of the row.
 * \param[in] s scaling factor.
 * \param[in] i2 index of the other row.
 */
NLAPI void NLAPIENTRY nlSparseMatrixMAddRow(
    NLSparseMatrix* M, NLuint i1, double s, NLuint i2
);

/**
 * \brief Scales a row of a sparse matrix.
 *   (row[i] *= s).
 * \param[in,out] M a pointer to a sparse matrix.
 * \param[in] i index of the row.
 * \param[in] s scaling factor.
 * \pre M has row storage and has not column storage.
 */
NLAPI void NLAPIENTRY nlSparseMatrixScaleRow(
    NLSparseMatrix* M, NLuint i, double s
);

/**
 * \brief Zeroes a row of a sparse matrix.
 *   (row[i] = 0).
 * \param[in,out] M a pointer to a sparse matrix.
 * \param[in] i index of the row.
 * \pre M has row storage and has not column storage.
 */
NLAPI void NLAPIENTRY nlSparseMatrixZeroRow(
    NLSparseMatrix* M, NLuint i
);

    
/******************************************************************************/

/**
 * \brief Creates a compressed row storage matrix from a dynamic sparse matrix.
 * \details The matrix \p M should have stored rows. If \p M has 
 *  symmetric storage, then the constructed matrix also has symmetric storage.
 * \param[in] M the dynamic sparse matrix.
 * \return a pointer to the created NLCRSMatrix
 * \relates NLCRSMatrix
 */
NLAPI NLMatrix NLAPIENTRY nlCRSMatrixNewFromSparseMatrix(NLSparseMatrix* M);    

/**
 * \brief Creates a compressed row storage matrix from a dynamic sparse matrix.
 * \details The matrix \p M should have stored rows. It is supposed 
 *  to be symmetric. If it does not have symmetric storage, then its upper 
 *  triangular part is ignored.
 * \param[in] M the dynamic sparse matrix.
 * \return a pointer to the created NLCRSMatrix
 * \relates NLCRSMatrix
 */
NLAPI NLMatrix NLAPIENTRY nlCRSMatrixNewFromSparseMatrixSymmetric(
    NLSparseMatrix* M
);    

    
/**
 * \brief Compresses a dynamic sparse matrix into a CRS matrix.
 * \details If the input matrix is not a dynamic sparse matrix, it is left
 *  unmodified.
 * \param[in,out] M a pointer to the matrix to be compressed
 * \relates NLMatrix
 */
NLAPI void NLAPIENTRY nlMatrixCompress(NLMatrix* M);

/**
 * \brief Gets the number of non-zero entries in a matrix.
 * \details If the matrix is sparse, it gets the number of non-zero entries,
 *  else it returns m*n
 * \return the number of non-zero entries in \p M
 */
NLAPI NLuint_big NLAPIENTRY nlMatrixNNZ(NLMatrix M);

/**
 * \brief Factorizes a matrix.
 * \details The corresponding extension needs to be successfully initialized
 *  before calling this function.
 * \param[in] M the input matrix
 * \param[in] solver a direct solver, i.e., one of:
 *   - NL_SUPERLU_EXT
 *   - NL_PERM_SUPERLU_EXT
 *   - NL_SYMMETRIC_SUPERLU_EXT
 *   - NL_CHOLMOD_EXT
 * \return a factorization of \p M, or NULL if \p M is singular. When calling
 *  nlMultMatrixVector() with the result, it solves a linear system (the result
 *  may be thought of as the inverse of \p M).
 */
NLAPI NLMatrix NLAPIENTRY nlMatrixFactorize(NLMatrix M, NLenum solver);
    
/******************************************************************************/

/**
 * \brief Function pointer type for matrix-vector products.
 */
    typedef void(*NLMatrixFunc)(const double* x, double* y);

/**
 * \brief Creates a matrix implemented by a matrix-vector function.
 * \param[in] m number of rows 
 * \param[in] n number of columns
 * \param[in] func a function that implements the matrix x vector product,
 *  and that takes the right hand side and the left hand side as arguments.
 */	     
NLAPI NLMatrix NLAPIENTRY nlMatrixNewFromFunction(
    NLuint m, NLuint n, NLMatrixFunc func
);	     

/**
 * \brief Gets the function pointer that implements matrix x vector product.
 * \param[in] M the matrix
 * \return the pointer to the matrix x vector product function if M was created
 *   by nlMatrixNewFromFunction(), NULL otherwise
 */
NLAPI NLMatrixFunc NLAPIENTRY nlMatrixGetFunction(NLMatrix M);

/******************************************************************************/

/**
 * \brief Creates a matrix that represents the product of two matrices.
 * \param[in] M an m times k matrix. 
 * \param[in] product_owns_M if set to NL_TRUE, then caller is no longer 
 *  responsible for the memory associated to M, and M will be destroyed 
 *  when the product will be destroyed. If it is set to NL_FALSE, then the
 *  product only keeps a reference to M, and the caller remains responsible
 *  for deallocating M.
 * \param[in] N a k times n matrix. 
 * \param[in] product_owns_N if set to NL_TRUE, then caller is no longer 
 *  responsible for the memory associated to N, and N will be destroyed 
 *  when the product will be destroyed. If it is set to NL_FALSE, then the
 *  product only keeps a reference to N, and the caller remains responsible
 *  for deallocating N.
 * \return an NLMatrix that represents the product between M and N
 */
NLAPI NLMatrix NLAPIENTRY nlMatrixNewFromProduct(
    NLMatrix M, NLboolean product_owns_M,
    NLMatrix N, NLboolean product_owns_N
);

/******************************************************************************/
    
#ifdef __cplusplus
}
#endif

#endif
