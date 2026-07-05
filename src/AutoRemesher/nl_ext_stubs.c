/* Stub implementations for optional OpenNL extensions.
 * These are compiled as C and linked when the actual external
 * dependencies (SUPERLU, CHOLMOD, MKL, CUDA) are not available.
 * Every return value is NL_FALSE or NULL so the solver degrades gracefully.
 */
#include <geogram/nl/nl.h>
#include <geogram/nl/nl_private.h>
#include <geogram/nl/nl_matrix.h>

/* SuperLU stubs */
NLboolean nlInitExtension_SUPERLU(void) { return NL_FALSE; }
NLboolean nlExtensionIsInitialized_SUPERLU(void) { return NL_FALSE; }
NLMatrix nlMatrixFactorize_SUPERLU(NLMatrix M, NLenum solver) { (void)M; (void)solver; return NULL; }

/* CHOLMOD stubs */
NLboolean nlInitExtension_CHOLMOD(void) { return NL_FALSE; }
NLboolean nlExtensionIsInitialized_CHOLMOD(void) { return NL_FALSE; }
NLMatrix nlMatrixFactorize_CHOLMOD(NLMatrix M, NLenum solver) { (void)M; (void)solver; return NULL; }

/* MKL stubs */
NLboolean nlInitExtension_MKL(void) { return NL_FALSE; }
NLboolean nlExtensionIsInitialized_MKL(void) { return NL_FALSE; }
NLMultMatrixVectorFunc NLMultMatrixVector_MKL = NULL;
NLMatrix nlMKLMatrixNewFromCRSMatrix(NLCRSMatrix *M) { (void)M; return NULL; }
NLMatrix nlMKLMatrixNewFromSparseMatrix(NLSparseMatrix *M) { (void)M; return NULL; }

/* CUDA stubs */
NLboolean nlInitExtension_CUDA(void) { return NL_FALSE; }
NLboolean nlExtensionIsInitialized_CUDA(void) { return NL_FALSE; }
void *nlCUDABlas(void) { return NULL; }
NLMatrix nlCUDAJacobiPreconditionerNewFromCRSMatrix(NLMatrix M) { (void)M; return NULL; }
NLMatrix nlCUDAMatrixNewFromCRSMatrix(NLMatrix M) { (void)M; return NULL; }