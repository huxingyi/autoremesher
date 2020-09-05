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

#include "nl_matrix.h"
#include "nl_superlu.h"
#include "nl_cholmod.h"
#include "nl_mkl.h"
#include "nl_context.h"
#include "nl_blas.h"

/*
 Some warnings about const cast in callback for
 qsort() function.
 */

#ifdef __clang__
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif


/************************************************************************/

void nlDeleteMatrix(NLMatrix M) {
    if(M == NULL) {
        return;
    }
    M->destroy_func(M);
    NL_DELETE(M);
}

void nlMultMatrixVector(
    NLMatrix M, const double* x, double* y
) {
    M->mult_func(M,x,y);
}

/************************************************************************/

void nlRowColumnConstruct(NLRowColumn* c) {
    c->size     = 0;
    c->capacity = 0;
    c->coeff    = NULL;
}

void nlRowColumnDestroy(NLRowColumn* c) {
    NL_DELETE_ARRAY(c->coeff);
    c->size = 0;
    c->capacity = 0;
}

void nlRowColumnGrow(NLRowColumn* c) {
    if(c->capacity != 0) {
        c->capacity = 2 * c->capacity;
        c->coeff = NL_RENEW_ARRAY(NLCoeff, c->coeff, c->capacity);
    } else {
        c->capacity = 4;
        c->coeff = NL_NEW_ARRAY(NLCoeff, c->capacity);
    }
}

void nlRowColumnAdd(NLRowColumn* c, NLuint index, NLdouble value) {
    NLuint i;
    for(i=0; i<c->size; i++) {
        if(c->coeff[i].index == index) {
            c->coeff[i].value += value;
            return;
        }
    }
    if(c->size == c->capacity) {
        nlRowColumnGrow(c);
    }
    c->coeff[c->size].index = index;
    c->coeff[c->size].value = value;
    c->size++;
}

/* Does not check whether the index already exists */
void nlRowColumnAppend(NLRowColumn* c, NLuint index, NLdouble value) {
    if(c->size == c->capacity) {
        nlRowColumnGrow(c);
    }
    c->coeff[c->size].index = index;
    c->coeff[c->size].value = value;
    c->size++;
}

void nlRowColumnZero(NLRowColumn* c) {
    c->size = 0;
}

void nlRowColumnClear(NLRowColumn* c) {
    c->size     = 0;
    c->capacity = 0;
    NL_DELETE_ARRAY(c->coeff);
}

static int nlCoeffCompare(const void* p1, const void* p2) {
    return (((NLCoeff*)(p2))->index < ((NLCoeff*)(p1))->index);
}

void nlRowColumnSort(NLRowColumn* c) {
    qsort(c->coeff, c->size, sizeof(NLCoeff), nlCoeffCompare);
}

/******************************************************************************/
/* CRSMatrix data structure */

/**
 * \brief Destroys a NLCRSMatrix
 * \details Only the memory allocated by the NLCRSMatrix is freed,
 *  The NLCRSMatrix structure is not freed.
 * \param[in,out] M pointer to an NLCRSMatrix
 * \relates NLCRSMatrix
 */
static void nlCRSMatrixDestroy(NLCRSMatrix* M) {
    NL_DELETE_ARRAY(M->val);
    NL_DELETE_ARRAY(M->rowptr);
    NL_DELETE_ARRAY(M->colind);
    NL_DELETE_ARRAY(M->sliceptr);
    M->m = 0;
    M->n = 0;
    M->nslices = 0;
}


NLboolean nlCRSMatrixSave(NLCRSMatrix* M, const char* filename) {
#ifdef GARGANTUA
    nl_arg_used(M);
    nl_arg_used(filename);
    nl_assert_not_reached; /* not implemented yet ! */
    return NL_FALSE;
#else    
    NLuint nnz = M->rowptr[M->m];
    FILE* f = fopen(filename, "rb");
    if(f == NULL) {
        nlError("nlCRSMatrixSave", "Could not open file");
        return NL_FALSE;
    }

    fwrite(&M->m, sizeof(NLuint), 1, f);
    fwrite(&M->n, sizeof(NLuint), 1, f);
    fwrite(&nnz, sizeof(NLuint), 1, f);

    fwrite(M->rowptr, sizeof(NLuint), M->m+1, f);
    fwrite(M->colind, sizeof(NLuint), nnz, f);
    fwrite(M->val, sizeof(double), nnz, f);
    
    return NL_TRUE;
#endif    
}

NLboolean nlCRSMatrixLoad(NLCRSMatrix* M, const char* filename) {
#ifdef GARGANTUA
    nl_arg_used(M);
    nl_arg_used(filename);
    nl_assert_not_reached; /* not implemented yet ! */
    return NL_FALSE;    
#else    
    NLuint nnz = 0;
    FILE* f = fopen(filename, "rb");
    NLboolean truncated = NL_FALSE;
    
    if(f == NULL) {
        nlError("nlCRSMatrixLoad", "Could not open file");
        return NL_FALSE;
    }
    
    truncated = truncated || (
        fread(&M->m, sizeof(NLuint), 1, f) != 1 ||
        fread(&M->n, sizeof(NLuint), 1, f) != 1 ||
        fread(&nnz, sizeof(NLuint), 1, f) != 1
    );

    if(truncated) {
        M->rowptr = NULL;
        M->colind = NULL;
        M->val = NULL;
    } else {
        M->rowptr = NL_NEW_ARRAY(NLuint, M->m+1);
        M->colind = NL_NEW_ARRAY(NLuint, nnz);
        M->val = NL_NEW_ARRAY(double, nnz);
        truncated = truncated || (
            fread(M->rowptr, sizeof(NLuint), M->m+1, f) != M->m+1 ||
            fread(M->colind, sizeof(NLuint), nnz, f) != nnz ||
            fread(M->val, sizeof(double), nnz, f) != nnz
        );
    }

    if(truncated) {
        nlError("nlCRSMatrixSave", "File appears to be truncated");
        NL_DELETE_ARRAY(M->rowptr);
        NL_DELETE_ARRAY(M->colind);
        NL_DELETE_ARRAY(M->val);
        return NL_FALSE;
    } else {
        M->nslices = 1;    
        M->sliceptr = NL_NEW_ARRAY(NLuint, M->nslices+1);
        M->sliceptr[0] = 0;
        M->sliceptr[1] = M->m;
    }

    fclose(f);
    return NL_TRUE;
#endif    
}

NLuint_big nlCRSMatrixNNZ(NLCRSMatrix* M) {
    return M->rowptr[M->m];
}

static void nlCRSMatrixMultSlice(
    NLCRSMatrix* M, const double* x, double* y, NLuint Ibegin, NLuint Iend
) {
    NLuint i;
    NLuint_big j;
    for(i=Ibegin; i<Iend; ++i) {
        double sum=0.0;
        for(j=M->rowptr[i]; j<M->rowptr[i+1]; ++j) {
            sum += M->val[j] * x[M->colind[j]];
        }
        y[i] = sum; 
    }
}

/**
 * \brief Computes a matrix-vector product
 * \param[in] M a pointer to the matrix
 * \param[in] x the vector to be multiplied, size = A->n
 * \param[in] y where to store the result, size = A->m
 * \relates NLSparseMatrix
 */
static void nlCRSMatrixMult(
    NLCRSMatrix* M, const double* x, double* y
) {
    int slice;
    int nslices = (int)(M->nslices);
    NLuint i,j;
    NLuint_big jj;
    NLdouble a;
    
    if(M->symmetric_storage) {
        for(i=0; i<M->m; ++i) {
            y[i] = 0.0;
        }
        for(i=0; i<M->m; ++i) {
            for(jj=M->rowptr[i]; jj<M->rowptr[i+1]; ++jj) {
                a = M->val[jj];
                j = M->colind[jj];
                y[i] += a * x[j];
                if(j != i) {
                    y[j] += a * x[i];
                }
            }
        }
    } else {
    
#if defined(_OPENMP)
#pragma omp parallel for private(slice)
#endif
    
	for(slice=0; slice<nslices; ++slice) {
	    nlCRSMatrixMultSlice(
		M,x,y,M->sliceptr[slice],M->sliceptr[slice+1]
	    );
	}
    }

    nlHostBlas()->flops += (NLulong)(2*nlCRSMatrixNNZ(M));
}

void nlCRSMatrixConstruct(
    NLCRSMatrix* M, NLuint m, NLuint n, NLuint_big nnz, NLuint nslices
) {
    M->m = m;
    M->n = n;
    M->type = NL_MATRIX_CRS;
    M->destroy_func = (NLDestroyMatrixFunc)nlCRSMatrixDestroy;
    if(NLMultMatrixVector_MKL != NULL) {
	M->mult_func = (NLMultMatrixVectorFunc)NLMultMatrixVector_MKL;
    } else {
	M->mult_func = (NLMultMatrixVectorFunc)nlCRSMatrixMult;
    }
    M->nslices = nslices;
    M->val = NL_NEW_ARRAY(double, nnz);
    M->rowptr = NL_NEW_ARRAY(NLuint_big, m+1);
    M->colind = NL_NEW_ARRAY(NLuint, nnz);
    M->sliceptr = NL_NEW_ARRAY(NLuint, nslices+1);
    M->symmetric_storage = NL_FALSE;
}

void nlCRSMatrixConstructSymmetric(
    NLCRSMatrix* M, NLuint n, NLuint_big nnz
) {
    M->m = n;
    M->n = n;
    M->type = NL_MATRIX_CRS;
    M->destroy_func = (NLDestroyMatrixFunc)nlCRSMatrixDestroy;
    M->mult_func = (NLMultMatrixVectorFunc)nlCRSMatrixMult;
    M->nslices = 0;
    M->val = NL_NEW_ARRAY(double, nnz);
    M->rowptr = NL_NEW_ARRAY(NLuint_big, n+1);
    M->colind = NL_NEW_ARRAY(NLuint, nnz);
    M->sliceptr = NULL;
    M->symmetric_storage = NL_TRUE;
}


void nlCRSMatrixConstructPattern(
    NLCRSMatrix* M, NLuint m, NLuint n
) {
    M->m = m;
    M->n = n;
    M->type = NL_MATRIX_CRS;
    M->destroy_func = (NLDestroyMatrixFunc)nlCRSMatrixDestroy;
    if(NLMultMatrixVector_MKL != NULL) {
	M->mult_func = (NLMultMatrixVectorFunc)NLMultMatrixVector_MKL;
    } else {
	M->mult_func = (NLMultMatrixVectorFunc)nlCRSMatrixMult;
    }
    M->nslices = 0;
    M->val = NULL;
    M->rowptr = NL_NEW_ARRAY(NLuint_big, m+1);
    M->colind = NULL;
    M->sliceptr = NULL;
    M->symmetric_storage = NL_FALSE;
}

void nlCRSMatrixConstructPatternSymmetric(
    NLCRSMatrix* M, NLuint n
) {
    M->m = n;
    M->n = n;
    M->type = NL_MATRIX_CRS;
    M->destroy_func = (NLDestroyMatrixFunc)nlCRSMatrixDestroy;
    M->mult_func = (NLMultMatrixVectorFunc)nlCRSMatrixMult;
    M->nslices = 0;
    M->val = NULL;
    M->rowptr = NL_NEW_ARRAY(NLuint_big, n+1);
    M->colind = NULL;
    M->sliceptr = NULL;
    M->symmetric_storage = NL_TRUE;
}

void nlCRSMatrixPatternSetRowLength(
    NLCRSMatrix* M, NLuint i, NLuint n
) {
    nl_assert(i < M->m);
    nl_assert(n <= M->n);
    /* Test that matrix is in 'pattern' state */
    nl_assert(M->colind == NULL);
    nl_assert(M->val == NULL);
    /* Store row length in rowptr */
    M->rowptr[i+1] = (NLuint_big)(n); 
}

void nlCRSMatrixComputeSlices(NLCRSMatrix* CRS);

void nlCRSMatrixComputeSlices(NLCRSMatrix* CRS) {
    NLuint_big slice_size = CRS->rowptr[CRS->m] / (NLuint_big)(CRS->nslices);
    NLuint slice, cur_row;
    NLuint_big cur_bound, cur_NNZ;
    /* Create "slices" to be used by parallel sparse matrix vector product */
    if(CRS->sliceptr != NULL) {
	cur_bound = slice_size;
	cur_NNZ = 0;
	cur_row = 0;
	CRS->sliceptr[0]=0;
	for(slice=1; slice<CRS->nslices; ++slice) {
	    while(cur_NNZ < cur_bound && cur_row < CRS->m) {
		++cur_row;
		cur_NNZ += CRS->rowptr[cur_row+1] - CRS->rowptr[cur_row];
	    }
	    CRS->sliceptr[slice] = cur_row;
	    cur_bound += slice_size;
	}
	CRS->sliceptr[CRS->nslices]=CRS->m;
    }
}

void nlCRSMatrixPatternCompile(NLCRSMatrix* M) {
    NLuint nslices = 8; /* TODO get number of cores */
    NLuint i;
    NLuint_big nnz;
    NLuint k;
    /* Test that matrix is in 'pattern' state */
    nl_assert(M->colind == NULL);
    nl_assert(M->val == NULL);
    for(i=0; i<M->m; ++i) {
	M->rowptr[i+1] += M->rowptr[i];
    }
    nnz = M->rowptr[M->m];
    M->val = NL_NEW_ARRAY(double, nnz);
    M->colind = NL_NEW_ARRAY(NLuint, nnz);
    for(k=0; k<nnz; ++k) {
	M->colind[k] = (NLuint)(-1);
    }
    M->sliceptr = NL_NEW_ARRAY(NLuint, nslices+1);
    M->nslices  = nslices;
    nlCRSMatrixComputeSlices(M);
}

void nlCRSMatrixAdd(
    NLCRSMatrix* M, NLuint i, NLuint j, NLdouble value
) {
    NLuint_big jj;    
    /* Test that matrix is in 'compiled' state */
    nl_assert(M->colind != NULL);
    nl_assert(M->val != NULL);
    nl_assert(i < M->m);
    nl_assert(j < M->n);
    if(M->symmetric_storage && j > i) {
	return;
    }
    for(jj=M->rowptr[i]; jj<M->rowptr[i+1]; ++jj) {
	if(M->colind[jj] == j) {
	    M->val[jj] += value;
	    return;
	} else if(M->colind[jj] == (NLuint)(-1)) {
	    M->colind[jj] = j;
	    M->val[jj] += value;
	    return;
	}
    }
    /* If this line is reached, it means that too many coefficients
     * were added to row j, i.e. a number of coefficients larger than
     * the row length previously declared with nlCRSMatrixPatternSetRowLength()
     */
    nl_assert_not_reached;
}

/******************************************************************************/
/* SparseMatrix data structure */


static void nlSparseMatrixDestroyRowColumns(NLSparseMatrix* M) {
    NLuint i;
    if(M->storage & NL_MATRIX_STORE_ROWS) {
        for(i=0; i<M->m; i++) {
            nlRowColumnDestroy(&(M->row[i]));
        }
        NL_DELETE_ARRAY(M->row);
    }
    M->storage = (NLenum)((int)(M->storage) & ~NL_MATRIX_STORE_ROWS);
    
    if(M->storage & NL_MATRIX_STORE_COLUMNS) {
        for(i=0; i<M->n; i++) {
            nlRowColumnDestroy(&(M->column[i]));
        }
        NL_DELETE_ARRAY(M->column);
    }
    M->storage = (NLenum)((int)(M->storage) & ~NL_MATRIX_STORE_COLUMNS);    
}

void nlSparseMatrixDestroy(NLSparseMatrix* M) {
    nl_assert(M->type == NL_MATRIX_SPARSE_DYNAMIC);
    nlSparseMatrixDestroyRowColumns(M);
    NL_DELETE_ARRAY(M->diag);
#ifdef NL_PARANOID
    NL_CLEAR(NLSparseMatrix,M);
#endif
}

void nlSparseMatrixAdd(NLSparseMatrix* M, NLuint i, NLuint j, NLdouble value) {
    nl_parano_range_assert(i, 0, M->m - 1);
    nl_parano_range_assert(j, 0, M->n - 1);
    if((M->storage & NL_MATRIX_STORE_SYMMETRIC) && (j > i)) {
        return;
    }
    if(i == j) {
        M->diag[i] += value;
    }
    if(M->storage & NL_MATRIX_STORE_ROWS) {
        nlRowColumnAdd(&(M->row[i]), j, value);
    }
    if(M->storage & NL_MATRIX_STORE_COLUMNS) {
        nlRowColumnAdd(&(M->column[j]), i, value);
    }
}

static void nlSparseMatrixAddSparseMatrix(
    NLSparseMatrix* M, double mul, const NLSparseMatrix* N    
) {
    NLuint i,j,ii,jj;
    nl_assert(M->m == N->m);
    nl_assert(M->n == N->n);
    if(N->storage & NL_MATRIX_STORE_SYMMETRIC) {
	nl_assert(M->storage & NL_MATRIX_STORE_SYMMETRIC);
    }
    if(N->storage & NL_MATRIX_STORE_ROWS) {
	for(i=0; i<N->m; ++i) {
	    for(jj=0; jj<N->row[i].size; ++jj) {
		nlSparseMatrixAdd(
		    M,
		    i, N->row[i].coeff[jj].index,
		    mul*N->row[i].coeff[jj].value
		);
	    }
	}
    } else {
	nl_assert(N->storage & NL_MATRIX_STORE_COLUMNS);	
	for(j=0; j<N->n; ++j) {
	    for(ii=0; ii<N->column[j].size; ++ii) {
		nlSparseMatrixAdd(
		    M,
		    N->column[j].coeff[ii].index, j,
		    mul*N->column[j].coeff[ii].value
		);
	    }
	}
    }
}

static void nlSparseMatrixAddCRSMatrix(
    NLSparseMatrix* M, double mul, const NLCRSMatrix* N    
) {
    NLuint i;
    NLuint_big jj;
    nl_assert(M->m == N->m);
    nl_assert(M->n == N->n);
    for(i=0; i<M->m; ++i) {
	for(jj=N->rowptr[i]; jj<N->rowptr[i+1]; ++jj) {
	    nlSparseMatrixAdd(
		M,
		i,
		N->colind[jj],
		mul*N->val[jj]
	    );
	}
    }
}

void nlSparseMatrixAddMatrix(
    NLSparseMatrix* M, double mul, const NLMatrix N
) {
    nl_assert(M->m == N->m);
    nl_assert(M->n == N->n);
    if(N->type == NL_MATRIX_SPARSE_DYNAMIC) {
	nlSparseMatrixAddSparseMatrix(M, mul, (const NLSparseMatrix*)N);
    } else if(N->type == NL_MATRIX_CRS) {
	nlSparseMatrixAddCRSMatrix(M, mul, (const NLCRSMatrix*)N);	
    } else {
	nl_assert_not_reached;
    }
}
    


void nlSparseMatrixZero( NLSparseMatrix* M) {
    NLuint i;
    if(M->storage & NL_MATRIX_STORE_ROWS) {
        for(i=0; i<M->m; i++) {
            nlRowColumnZero(&(M->row[i]));
        }
    }
    if(M->storage & NL_MATRIX_STORE_COLUMNS) {
        for(i=0; i<M->n; i++) {
            nlRowColumnZero(&(M->column[i]));
        }
    }
    NL_CLEAR_ARRAY(NLdouble, M->diag, M->diag_size);
}

void nlSparseMatrixClear( NLSparseMatrix* M) {
    NLuint i;
    if(M->storage & NL_MATRIX_STORE_ROWS) {
        for(i=0; i<M->m; i++) {
            nlRowColumnClear(&(M->row[i]));
        }
    }
    if(M->storage & NL_MATRIX_STORE_COLUMNS) {
        for(i=0; i<M->n; i++) {
            nlRowColumnClear(&(M->column[i]));
        }
    }
    NL_CLEAR_ARRAY(NLdouble, M->diag, M->diag_size);
}

/* Returns the number of non-zero coefficients */
NLuint_big nlSparseMatrixNNZ( NLSparseMatrix* M) {
    NLuint_big nnz = 0;
    NLuint i;
    if(M->storage & NL_MATRIX_STORE_ROWS) {
        for(i = 0; i<M->m; i++) {
            nnz += (NLuint_big)(M->row[i].size);
        }
    } else if (M->storage & NL_MATRIX_STORE_COLUMNS) {
        for(i = 0; i<M->n; i++) {
            nnz += (NLuint_big)(M->column[i].size);
        }
    } else {
        nl_assert_not_reached;
    }
    return nnz;
}

void nlSparseMatrixSort( NLSparseMatrix* M) {
    NLuint i;
    if(M->storage & NL_MATRIX_STORE_ROWS) {
        for(i = 0; i<M->m; i++) {
            nlRowColumnSort(&(M->row[i]));                
        }
    } 
    if (M->storage & NL_MATRIX_STORE_COLUMNS) {
        for(i = 0; i<M->n; i++) {
            nlRowColumnSort(&(M->column[i]));
        }
    } 
}

void nlSparseMatrixMAddRow(
    NLSparseMatrix* M, NLuint i1, double s, NLuint i2
) {
    NLuint jj;
    NLRowColumn* Ri2 = &(M->row[i2]);
    NLCoeff* c = NULL;

    nl_debug_assert(i1 < M->m);
    nl_debug_assert(i2 < M->m);
    
    for(jj=0; jj<Ri2->size; ++jj) {
	c = &(Ri2->coeff[jj]);
	nlSparseMatrixAdd(M, i1, c->index, s*c->value);
    }
}

void nlSparseMatrixScaleRow(
    NLSparseMatrix* M, NLuint i, double s
) {
    NLuint jj;
    NLRowColumn* Ri = &(M->row[i]);
    NLCoeff* c = NULL;

    nl_assert(M->storage & NL_MATRIX_STORE_ROWS);
    nl_assert(!(M->storage & NL_MATRIX_STORE_COLUMNS));    
    nl_debug_assert(i < M->m);
    
    for(jj=0; jj<Ri->size; ++jj) {
	c = &(Ri->coeff[jj]);
	c->value *= s;
    }
    if(i < M->diag_size) {
	M->diag[i] *= s;
    }
}

void nlSparseMatrixZeroRow(
    NLSparseMatrix* M, NLuint i
) {
    NLRowColumn* Ri = &(M->row[i]);

    nl_debug_assert(i < M->m);
    
    Ri->size = 0;
    if(i < M->diag_size) {
	M->diag[i] = 0.0;
    }
}


/*****************************************************************************/
/* SparseMatrix x Vector routines, internal helper routines */

static void nlSparseMatrix_mult_rows_symmetric(
    NLSparseMatrix* A,
    const NLdouble* x,
    NLdouble* y
) {
    NLuint m = A->m;
    NLuint i,ij;
    NLCoeff* c = NULL;
    for(i=0; i<m; i++) {
        NLRowColumn* Ri = &(A->row[i]);
        y[i] = 0;
        for(ij=0; ij<Ri->size; ++ij) {
            c = &(Ri->coeff[ij]);
            y[i] += c->value * x[c->index];
            if(i != c->index) {
                y[c->index] += c->value * x[i];
            }
        }
    }
}

static void nlSparseMatrix_mult_rows(
        NLSparseMatrix* A,
        const NLdouble* x,
        NLdouble* y
) {
    /* 
     * Note: OpenMP does not like unsigned ints
     * (causes some floating point exceptions),
     * therefore I use here signed ints for all
     * indices.
     */
    
    int m = (int)(A->m);
    int i,ij;
    NLCoeff* c = NULL;
    NLRowColumn* Ri = NULL;

#if defined(_OPENMP)    
#pragma omp parallel for private(i,ij,c,Ri)
#endif
    
    for(i=0; i<m; i++) {
        Ri = &(A->row[i]);       
        y[i] = 0;
        for(ij=0; ij<(int)(Ri->size); ij++) {
            c = &(Ri->coeff[ij]);
            y[i] += c->value * x[c->index];
        }
    }
}

static void nlSparseMatrix_mult_cols_symmetric(
        NLSparseMatrix* A,
        const NLdouble* x,
        NLdouble* y
) {
    NLuint n = A->n;
    NLuint j,ii;
    NLCoeff* c = NULL;
    for(j=0; j<n; j++) {
        NLRowColumn* Cj = &(A->column[j]);       
        y[j] = 0;
        for(ii=0; ii<Cj->size; ii++) {
            c = &(Cj->coeff[ii]);
            y[c->index] += c->value * x[j];
            if(j != c->index) {
                y[j] += c->value * x[c->index];
            }
        }
    }
}

static void nlSparseMatrix_mult_cols(
        NLSparseMatrix* A,
        const NLdouble* x,
        NLdouble* y
) {
    NLuint n = A->n;
    NLuint j,ii; 
    NLCoeff* c = NULL;
    NL_CLEAR_ARRAY(NLdouble, y, A->m);
    for(j=0; j<n; j++) {
        NLRowColumn* Cj = &(A->column[j]);
        for(ii=0; ii<Cj->size; ii++) {
            c = &(Cj->coeff[ii]);
            y[c->index] += c->value * x[j];
        }
    }
}

void nlSparseMatrixMult(
    NLSparseMatrix* A, const NLdouble* x, NLdouble* y
) {
    nl_assert(A->type == NL_MATRIX_SPARSE_DYNAMIC);
    if(A->storage & NL_MATRIX_STORE_ROWS) {
        if(A->storage & NL_MATRIX_STORE_SYMMETRIC) {
            nlSparseMatrix_mult_rows_symmetric(A, x, y);
        } else {
            nlSparseMatrix_mult_rows(A, x, y);
        }
    } else {
        if(A->storage & NL_MATRIX_STORE_SYMMETRIC) {
            nlSparseMatrix_mult_cols_symmetric(A, x, y);
        } else {
            nlSparseMatrix_mult_cols(A, x, y);
        }
    }
    nlHostBlas()->flops += (NLulong)(2*nlSparseMatrixNNZ(A));
}

NLMatrix nlSparseMatrixNew(
    NLuint m, NLuint n, NLenum storage
) {
    NLSparseMatrix* result = NL_NEW(NLSparseMatrix);
    nlSparseMatrixConstruct(result, m, n, storage);
    return (NLMatrix)result;
}

void nlSparseMatrixConstruct(
    NLSparseMatrix* M, NLuint m, NLuint n, NLenum storage
) {
    NLuint i;
    M->m = m;
    M->n = n;
    M->type = NL_MATRIX_SPARSE_DYNAMIC;
    M->destroy_func = (NLDestroyMatrixFunc)nlSparseMatrixDestroy;
    M->mult_func = (NLMultMatrixVectorFunc)nlSparseMatrixMult;
    M->storage = storage;
    if(storage & NL_MATRIX_STORE_ROWS) {
        M->row = NL_NEW_ARRAY(NLRowColumn, m);
	M->row_capacity = m;
        for(i=0; i<n; i++) {
            nlRowColumnConstruct(&(M->row[i]));
        }
    } else {
        M->row = NULL;
	M->row_capacity = 0;
    }

    if(storage & NL_MATRIX_STORE_COLUMNS) {
        M->column = NL_NEW_ARRAY(NLRowColumn, n);
	M->column_capacity = n;
        for(i=0; i<n; i++) {
            nlRowColumnConstruct(&(M->column[i]));
        }
    } else {
        M->column = NULL;
	M->column_capacity = 0;
    }

    M->diag_size = MIN(m,n);
    M->diag_capacity = M->diag_size;
    M->diag = NL_NEW_ARRAY(NLdouble, M->diag_size);
}

/**
 * \brief Adjusts the size of the diagonal of 
 *  an NLSparseMatrix after the number of rows or c
 *  olumns have changed.
 * \param[in,out] M a pointer to the sparse matrix.
 */
static void adjust_diag(NLSparseMatrix* M) {
    NLuint new_diag_size = MIN(M->m, M->n);
    NLuint i;
    if(new_diag_size > M->diag_size) {
	if(new_diag_size > M->diag_capacity) {
	    M->diag_capacity *= 2;
	    if(M->diag_capacity == 0) {
		M->diag_capacity = 16;
	    }
	    M->diag = NL_RENEW_ARRAY(double, M->diag, M->diag_capacity);
	    for(i=M->diag_size; i<new_diag_size; ++i) {
		M->diag[i] = 0.0;
	    }
	}
	M->diag_size= new_diag_size;
    }
}

void nlSparseMatrixAddRow( NLSparseMatrix* M) {
    ++M->m;
    if(M->storage & NL_MATRIX_STORE_ROWS) {
	if(M->m > M->row_capacity) {
	    M->row_capacity *= 2;
	    if(M->row_capacity == 0) {
		M->row_capacity = 16;
	    }
	    M->row = NL_RENEW_ARRAY(
		NLRowColumn, M->row, M->row_capacity
	    );
	}
	nlRowColumnConstruct(&(M->row[M->m-1]));
    }
    adjust_diag(M);
}

void nlSparseMatrixAddColumn( NLSparseMatrix* M) {
    ++M->n;
    if(M->storage & NL_MATRIX_STORE_COLUMNS) {
	if(M->n > M->column_capacity) {
	    M->column_capacity *= 2;
	    if(M->column_capacity == 0) {
		M->column_capacity = 16;
	    }
	    M->column = NL_RENEW_ARRAY(
		NLRowColumn, M->column, M->column_capacity
	    );
	}
	nlRowColumnConstruct(&(M->column[M->n-1]));
    }
    adjust_diag(M);
}


/*****************************************************************/

NLMatrix nlCRSMatrixNewFromSparseMatrix(NLSparseMatrix* M) {
    NLuint_big nnz = nlSparseMatrixNNZ(M);
    NLuint nslices = 8; /* TODO: get number of cores */
    NLuint i,ij,k; 
    NLCRSMatrix* CRS = NL_NEW(NLCRSMatrix);

    nl_assert(M->storage & NL_MATRIX_STORE_ROWS);

    if(M->storage & NL_MATRIX_STORE_SYMMETRIC) {
        nl_assert(M->m == M->n);
        nlCRSMatrixConstructSymmetric(CRS, M->n, nnz);        
    } else {
        nlCRSMatrixConstruct(CRS, M->m, M->n, nnz, nslices);
    }
    
    nlSparseMatrixSort(M);
    /* Convert matrix to CRS format */
    k=0;
    for(i=0; i<M->m; ++i) {
        NLRowColumn* Ri = &(M->row[i]);
        CRS->rowptr[i] = k;
        for(ij=0; ij<Ri->size; ij++) {
            NLCoeff* c = &(Ri->coeff[ij]);
            CRS->val[k] = c->value;
            CRS->colind[k] = c->index;
            ++k;
        }
    }
    CRS->rowptr[M->m] = k;
    nlCRSMatrixComputeSlices(CRS);
    return (NLMatrix)CRS;
}

NLMatrix nlCRSMatrixNewFromSparseMatrixSymmetric(NLSparseMatrix* M) {
    NLuint_big nnz;
    NLuint i,j,jj,k;
    NLCRSMatrix* CRS = NL_NEW(NLCRSMatrix);
    
    nl_assert(M->storage & NL_MATRIX_STORE_ROWS);
    nl_assert(M->m == M->n);

    nlSparseMatrixSort(M);
    
    if(M->storage & NL_MATRIX_STORE_SYMMETRIC) {
        nnz = nlSparseMatrixNNZ(M);
    } else {
        nnz = 0;
        for(i=0; i<M->n; ++i) {
            NLRowColumn* Ri = &M->row[i];
            for(jj=0; jj<Ri->size; ++jj) {
                j = Ri->coeff[jj].index;
                if(j <= i) {
                    ++nnz;
                }
            }
        }
    }

    nlCRSMatrixConstructSymmetric(CRS, M->n, nnz);        

    k=0;
    for(i=0; i<M->m; ++i) {
        NLRowColumn* Ri = &(M->row[i]);
        CRS->rowptr[i] = k;
        for(jj=0; jj<Ri->size; ++jj) {
            j = Ri->coeff[jj].index;
            if((M->storage & NL_MATRIX_STORE_SYMMETRIC)) {
                nl_debug_assert(j <= i);
            }
            if(j <= i) {
                CRS->val[k] = Ri->coeff[jj].value;
                CRS->colind[k] = j;
                ++k;
            }
        }
    }
    CRS->rowptr[M->m] = k;

    return (NLMatrix)CRS;
}


void nlMatrixCompress(NLMatrix* M) {
    NLMatrix result = NULL;
    
    if(
	(*M)->type == NL_MATRIX_CRS &&
	nlExtensionIsInitialized_MKL()
    ) {
	result = nlMKLMatrixNewFromCRSMatrix((NLCRSMatrix*)*M);
	nlDeleteMatrix(*M);
	*M = result;
	return;
    }
    
    if((*M)->type != NL_MATRIX_SPARSE_DYNAMIC) {
        return;
    }
    
    if(nlExtensionIsInitialized_MKL()) {
	result = nlMKLMatrixNewFromSparseMatrix((NLSparseMatrix*)*M);
    } else {
	result = nlCRSMatrixNewFromSparseMatrix((NLSparseMatrix*)*M);
    }
    nlDeleteMatrix(*M);
    *M = result;
}

NLuint_big nlMatrixNNZ(NLMatrix M) {
    if(M->type == NL_MATRIX_SPARSE_DYNAMIC) {
	return nlSparseMatrixNNZ((NLSparseMatrix*)M);
    } else if(M->type == NL_MATRIX_CRS) {
	return nlCRSMatrixNNZ((NLCRSMatrix*)M);	
    }
    return (NLuint_big)(M->m) * (NLuint_big)(M->n);
}

NLMatrix nlMatrixFactorize(NLMatrix M, NLenum solver) {
    NLMatrix result = NULL;
    switch(solver) {
	case NL_SUPERLU_EXT:
	case NL_PERM_SUPERLU_EXT:      
	case NL_SYMMETRIC_SUPERLU_EXT:
	    result = nlMatrixFactorize_SUPERLU(M,solver);
	    break;
	case NL_CHOLMOD_EXT:
	    result = nlMatrixFactorize_CHOLMOD(M,solver);	    
	    break;
	default:
	    nlError("nlMatrixFactorize","unknown solver");
    }
    return result;
}

/*****************************************************************/

/**
 * \brief A matrix class implemented by a function.
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
     * \details One of NL_MATRIX_SPARSE_DYNAMIC, 
     *  NL_MATRIX_CRS, NL_MATRIX_SUPERLU_EXT,
     *  NL_MATRIX_CHOLDMOD_EXT, NL_MATRIX_FUNCTION,
     *  NL_MATRIX_OTHER
     */
    NLenum type;

    /**
     * \brief Destructor
     */
    NLDestroyMatrixFunc destroy_func;

    /**
     * \brief Matrix x vector product (abstract matrix API,
     *  takes matrix, rhs and lhs)
     */
    NLMultMatrixVectorFunc mult_func;

    /**
     * \brief Matrix x vector product (user API, only takes
     *  rhs and lhs).
     */
    NLMatrixFunc matrix_func;
} NLFunctionMatrix;

static void nlFunctionMatrixDestroy(NLFunctionMatrix* M) {
    (void)M; /* to avoid 'unused parameter' warning */
    /* 
     * Nothing special to do, 
     * there is no dynamic allocated mem.
     */
}

static void nlFunctionMatrixMult(
    NLFunctionMatrix* M, const NLdouble* x, NLdouble* y
) {
    M->matrix_func(x,y);
}

NLMatrix nlMatrixNewFromFunction(NLuint m, NLuint n, NLMatrixFunc func) {
    NLFunctionMatrix* result = NL_NEW(NLFunctionMatrix);
    result->m = m;
    result->n = n;
    result->type = NL_MATRIX_FUNCTION;
    result->destroy_func = (NLDestroyMatrixFunc)nlFunctionMatrixDestroy;
    result->mult_func = (NLMultMatrixVectorFunc)nlFunctionMatrixMult;
    result->matrix_func = func;
    return (NLMatrix)result;
}

NLMatrixFunc nlMatrixGetFunction(NLMatrix M) {
    if(M == NULL) {
	return NULL;
    }
    if(M->type != NL_MATRIX_FUNCTION) {
	return NULL;
    }
    return ((NLFunctionMatrix*)M)->matrix_func;
}

/******************************************************************************/

/**
 * \brief A matrix class that implements the product between two matrices.
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
     * \brief matrix type, NL_MATRIX_OTHER
     */
    NLenum type;

    /**
     * \brief Destructor
     */
    NLDestroyMatrixFunc destroy_func;

    /**
     * \brief Matrix x vector product (abstract matrix API,
     *  takes matrix, rhs and lhs)
     */
    NLMultMatrixVectorFunc mult_func;

    /**
     * \brief Matrix x vector product (user API, only takes
     *  rhs and lhs).
     */
    NLMatrixFunc matrix_func;

    /**
     * \brief First matrix of the product.
     */
    NLMatrix M;

    /**
     * \brief NL_TRUE if memory ownership was transferred,
     *  NL_FALSE otherwise.
     */
    NLboolean owns_M;
    
    /**
     * \brief Second matrix of the product.
     */
    NLMatrix N;

    /**
     * \brief NL_TRUE if memory ownership was transferred,
     *  NL_FALSE otherwise.
     */
    NLboolean owns_N;
    
    /**
     * \brief A temporary vector of dimension N->m (= M->n)
     */
    NLdouble* work;
} NLMatrixProduct;


static void nlMatrixProductDestroy(NLMatrixProduct* P) {
    NL_DELETE_ARRAY(P->work);
    if(P->owns_M) {
	nlDeleteMatrix(P->M); P->M = NULL;
    }
    if(P->owns_N) {
	nlDeleteMatrix(P->N); P->N = NULL;
    }
}

static void nlMatrixProductMult(
    NLMatrixProduct* P, const NLdouble* x, NLdouble* y
) {
    nlMultMatrixVector(P->N, x, P->work);
    nlMultMatrixVector(P->M, P->work, y);
}

NLMatrix nlMatrixNewFromProduct(
    NLMatrix M, NLboolean owns_M, NLMatrix N, NLboolean owns_N
) {
    NLMatrixProduct* result = NL_NEW(NLMatrixProduct);
    nl_assert(M->n == N->m);
    result->m = M->m;
    result->n = N->n;
    result->type = NL_MATRIX_OTHER;
    result->work = NL_NEW_ARRAY(NLdouble,N->m);
    result->destroy_func = (NLDestroyMatrixFunc)nlMatrixProductDestroy;
    result->mult_func = (NLMultMatrixVectorFunc)nlMatrixProductMult;
    result->M = M;
    result->owns_M = owns_M;
    result->N = N;
    result->owns_N = owns_N;
    return (NLMatrix)result;
}

/******************************************************************************/
