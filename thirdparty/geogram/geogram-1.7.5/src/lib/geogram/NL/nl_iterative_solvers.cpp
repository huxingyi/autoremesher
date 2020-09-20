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
 
/*
 *  Copyright (c) 2020 Jeremy HU <jeremy-at-dust3d dot org>. All rights reserved. 
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 *
 *  This file modified for customized progress reporting in Dust3D/AutoRemesher project
 *  geogram_report_progress_round and geogram_report_progress_callback are introduced
 *
 */

#include "nl_iterative_solvers.h"
#include "nl_blas.h"
#include "nl_matrix.h"
#include "nl_context.h"
#include <geogram_report_progress.h>

/************************************************************************/
/* Solvers */

/*
 * The implementation of the solvers is inspired by 
 * the lsolver library, by Christian Badura, available from:
 * http://www.mathematik.uni-freiburg.de
 * /IAM/Research/projectskr/lin_solver/
 *
 * About the Conjugate Gradient, details can be found in:
 *  Ashby, Manteuffel, Saylor
 *     A taxononmy for conjugate gradient methods
 *     SIAM J Numer Anal 27, 1542-1568 (1990)
 *
 *  This version is completely abstract, the same code can be used for 
 * CPU/GPU, dense matrix / sparse matrix etc...
 *  Abstraction is realized through:
 *   - Abstract blas interface (NLBlas_t), that can implement BLAS 
 *     operations on the CPU or on the GPU.
 *   - Abstract matrix interface (NLMatrix), that can implement different
 *     versions of matrix x vector product (CPU/GPU, sparse/dense ...)
 */

/************************************************************************/

static NLuint nlSolveSystem_CG(
    NLBlas_t blas,
    NLMatrix M, NLdouble* b, NLdouble* x,
    double eps, NLuint max_iter
) {
    NLint N = (NLint)M->m;

    NLdouble *g = NL_NEW_VECTOR(blas, NL_DEVICE_MEMORY, N);
    NLdouble *r = NL_NEW_VECTOR(blas, NL_DEVICE_MEMORY, N); 
    NLdouble *p = NL_NEW_VECTOR(blas, NL_DEVICE_MEMORY, N);
    NLuint its=0;
    NLdouble t, tau, sig, rho, gam;
    NLdouble b_square=blas->Ddot(blas,N,b,1,b,1);
    NLdouble err=eps*eps*b_square;
    NLdouble curr_err;

    nlMultMatrixVector(M,x,g);
    blas->Daxpy(blas,N,-1.,b,1,g,1);
    blas->Dscal(blas,N,-1.,g,1);
    blas->Dcopy(blas,N,g,1,r,1);
    curr_err = blas->Ddot(blas,N,g,1,g,1);
    ++geogram_report_progress_round;
    while ( curr_err >err && its < max_iter) {
	if(nlCurrentContext != NULL) {
	    if(nlCurrentContext->progress_func != NULL) {
		nlCurrentContext->progress_func(its, max_iter, curr_err, err);
	    }
        geogram_report_progress_callback(geogram_report_progress_tag, (float)its / max_iter);
	    if(nlCurrentContext->verbose && !(its % 100)) {
		nl_printf ( "%4d : %.10e -- %.10e\n", its, curr_err, err );
	    }
	}
	nlMultMatrixVector(M,r,p);
        rho=blas->Ddot(blas,N,p,1,p,1);
        sig=blas->Ddot(blas,N,r,1,p,1);
        tau=blas->Ddot(blas,N,g,1,r,1);
        t=tau/sig;
        blas->Daxpy(blas,N,t,r,1,x,1);
        blas->Daxpy(blas,N,-t,p,1,g,1);
        gam=(t*t*rho-tau)/tau;
        blas->Dscal(blas,N,gam,r,1);
        blas->Daxpy(blas,N,1.,g,1,r,1);
        ++its;
        curr_err = blas->Ddot(blas,N,g,1,g,1);
    }
    NL_DELETE_VECTOR(blas, NL_DEVICE_MEMORY, N, g);
    NL_DELETE_VECTOR(blas, NL_DEVICE_MEMORY, N, r);
    NL_DELETE_VECTOR(blas, NL_DEVICE_MEMORY, N, p);
    blas->sq_bnorm = b_square;
    blas->sq_rnorm = curr_err;
    return its;
}

static NLuint nlSolveSystem_PRE_CG(
    NLBlas_t blas,
    NLMatrix M, NLMatrix P, NLdouble* b, NLdouble* x,
    double eps, NLuint max_iter
) {
    NLint     N        = (NLint)M->n;
    NLdouble* r = NL_NEW_VECTOR(blas, NL_DEVICE_MEMORY, N);
    NLdouble* d = NL_NEW_VECTOR(blas, NL_DEVICE_MEMORY, N);
    NLdouble* h = NL_NEW_VECTOR(blas, NL_DEVICE_MEMORY, N);
    NLdouble *Ad = h;
    NLuint its=0;
    NLdouble rh, alpha, beta;
    NLdouble b_square = blas->Ddot(blas,N,b,1,b,1);
    NLdouble err=eps*eps*b_square;
    NLdouble curr_err;

    nlMultMatrixVector(M,x,r);
    blas->Daxpy(blas,N,-1.,b,1,r,1);
    nlMultMatrixVector(P,r,d);
    blas->Dcopy(blas,N,d,1,h,1);
    rh=blas->Ddot(blas,N,r,1,h,1);
    curr_err = blas->Ddot(blas,N,r,1,r,1);
    ++geogram_report_progress_round;
    while ( curr_err >err && its < max_iter) {
	if(nlCurrentContext != NULL) {
	    if(nlCurrentContext->progress_func != NULL) {
		nlCurrentContext->progress_func(its, max_iter, curr_err, err);
	    }
        geogram_report_progress_callback(geogram_report_progress_tag, (float)its / max_iter);
	    if( nlCurrentContext->verbose && !(its % 100)) {
		nl_printf ( "%4d : %.10e -- %.10e\n", its, curr_err, err );
	    }
	}
	nlMultMatrixVector(M,d,Ad);
        alpha=rh/blas->Ddot(blas,N,d,1,Ad,1);
        blas->Daxpy(blas,N,-alpha,d,1,x,1);
        blas->Daxpy(blas,N,-alpha,Ad,1,r,1);
	nlMultMatrixVector(P,r,h);
        beta=1./rh;
	rh=blas->Ddot(blas,N,r,1,h,1);
	beta*=rh;
        blas->Dscal(blas,N,beta,d,1);
        blas->Daxpy(blas,N,1.,h,1,d,1);
        ++its;
        curr_err = blas->Ddot(blas,N,r,1,r,1);
    }
    NL_DELETE_VECTOR(blas, NL_DEVICE_MEMORY, N, r);
    NL_DELETE_VECTOR(blas, NL_DEVICE_MEMORY, N, d);
    NL_DELETE_VECTOR(blas, NL_DEVICE_MEMORY, N, h);
    blas->sq_bnorm = b_square;
    blas->sq_rnorm = curr_err;
    return its;
}

static NLuint nlSolveSystem_BICGSTAB(
    NLBlas_t blas,
    NLMatrix M, NLdouble* b, NLdouble* x,
    double eps, NLuint max_iter
) {
    NLint     N   = (NLint)M->n;
    NLdouble *rT  = NL_NEW_VECTOR(blas, NL_DEVICE_MEMORY, N); 
    NLdouble *d   = NL_NEW_VECTOR(blas, NL_DEVICE_MEMORY, N); 
    NLdouble *h   = NL_NEW_VECTOR(blas, NL_DEVICE_MEMORY, N); 
    NLdouble *u   = NL_NEW_VECTOR(blas, NL_DEVICE_MEMORY, N); 
    NLdouble *Ad  = NL_NEW_VECTOR(blas, NL_DEVICE_MEMORY, N); 
    NLdouble *t   = NL_NEW_VECTOR(blas, NL_DEVICE_MEMORY, N); 
    NLdouble *s   = h;
    NLdouble rTh, rTAd, rTr, alpha, beta, omega, st, tt;
    NLuint its=0;
    NLdouble b_square = blas->Ddot(blas,N,b,1,b,1);
    NLdouble err=eps*eps*b_square;
    NLdouble *r = NL_NEW_VECTOR(blas, NL_DEVICE_MEMORY, N);
    nlMultMatrixVector(M,x,r);
    blas->Daxpy(blas,N,-1.,b,1,r,1);
    blas->Dcopy(blas,N,r,1,d,1);
    blas->Dcopy(blas,N,d,1,h,1);
    blas->Dcopy(blas,N,h,1,rT,1);
    nl_assert( blas->Ddot(blas,N,rT,1,rT,1)>1e-40 );
    rTh=blas->Ddot(blas,N,rT,1,h,1);
    rTr=blas->Ddot(blas,N,r,1,r,1);
    ++geogram_report_progress_round;
    while ( rTr>err && its < max_iter) {
	if(nlCurrentContext != NULL) {
	    if(nlCurrentContext->progress_func != NULL) {
		nlCurrentContext->progress_func(its, max_iter, rTr, err);
	    }
        geogram_report_progress_callback(geogram_report_progress_tag, (float)its / max_iter);
	    if( (nlCurrentContext->verbose) && !(its % 100)) {
		nl_printf ( "%4d : %.10e -- %.10e\n", its, rTr, err );
	    }
	}
	nlMultMatrixVector(M,d,Ad);
        rTAd=blas->Ddot(blas,N,rT,1,Ad,1);
        nl_assert( fabs(rTAd)>1e-40 );
        alpha=rTh/rTAd;
        blas->Daxpy(blas,N,-alpha,Ad,1,r,1);
        blas->Dcopy(blas,N,h,1,s,1);
        blas->Daxpy(blas,N,-alpha,Ad,1,s,1);
	nlMultMatrixVector(M,s,t);
        blas->Daxpy(blas,N,1.,t,1,u,1);
        blas->Dscal(blas,N,alpha,u,1);
        st=blas->Ddot(blas,N,s,1,t,1);
        tt=blas->Ddot(blas,N,t,1,t,1);
        if ( fabs(st)<1e-40 || fabs(tt)<1e-40 ) {
            omega = 0.;
        } else {
            omega = st/tt;
        }
        blas->Daxpy(blas,N,-omega,t,1,r,1);
        blas->Daxpy(blas,N,-alpha,d,1,x,1);
        blas->Daxpy(blas,N,-omega,s,1,x,1);
        blas->Dcopy(blas,N,s,1,h,1);
        blas->Daxpy(blas,N,-omega,t,1,h,1);
        beta=(alpha/omega)/rTh;
	rTh=blas->Ddot(blas,N,rT,1,h,1);
	beta*=rTh;
        blas->Dscal(blas,N,beta,d,1);
        blas->Daxpy(blas,N,1.,h,1,d,1);
        blas->Daxpy(blas,N,-beta*omega,Ad,1,d,1);
        rTr=blas->Ddot(blas,N,r,1,r,1);
        ++its;
    }
    NL_DELETE_VECTOR(blas, NL_DEVICE_MEMORY, N, r);
    NL_DELETE_VECTOR(blas, NL_DEVICE_MEMORY, N, rT);
    NL_DELETE_VECTOR(blas, NL_DEVICE_MEMORY, N, d);
    NL_DELETE_VECTOR(blas, NL_DEVICE_MEMORY, N, h);
    NL_DELETE_VECTOR(blas, NL_DEVICE_MEMORY, N, u);
    NL_DELETE_VECTOR(blas, NL_DEVICE_MEMORY, N, Ad);
    NL_DELETE_VECTOR(blas, NL_DEVICE_MEMORY, N, t);
    blas->sq_bnorm = b_square;
    blas->sq_rnorm = rTr;
    return its;
}

static NLuint nlSolveSystem_PRE_BICGSTAB(
    NLBlas_t blas,
    NLMatrix M, NLMatrix P, NLdouble* b, NLdouble* x,
    double eps, NLuint max_iter
) {
    NLint     N   = (NLint)M->n;
    NLdouble *rT  = NL_NEW_VECTOR(blas, NL_DEVICE_MEMORY, N);
    NLdouble *d   = NL_NEW_VECTOR(blas, NL_DEVICE_MEMORY, N);
    NLdouble *h   = NL_NEW_VECTOR(blas, NL_DEVICE_MEMORY, N);
    NLdouble *u   = NL_NEW_VECTOR(blas, NL_DEVICE_MEMORY, N);
    NLdouble *Sd  = NL_NEW_VECTOR(blas, NL_DEVICE_MEMORY, N);
    NLdouble *t   = NL_NEW_VECTOR(blas, NL_DEVICE_MEMORY, N);
    NLdouble *aux = NL_NEW_VECTOR(blas, NL_DEVICE_MEMORY, N);
    NLdouble *s   = h;
    NLdouble rTh, rTSd, rTr, alpha, beta, omega, st, tt;
    NLuint its=0;
    NLdouble b_square = blas->Ddot(blas,N,b,1,b,1);
    NLdouble err  = eps*eps*b_square;
    NLdouble *r   = NL_NEW_VECTOR(blas, NL_DEVICE_MEMORY, N);

    nlMultMatrixVector(M,x,r);
    blas->Daxpy(blas,N,-1.,b,1,r,1);
    nlMultMatrixVector(P,r,d);
    blas->Dcopy(blas,N,d,1,h,1);
    blas->Dcopy(blas,N,h,1,rT,1);
    nl_assert( blas->Ddot(blas,N,rT,1,rT,1)>1e-40 );
    rTh=blas->Ddot(blas,N,rT,1,h,1);
    rTr=blas->Ddot(blas,N,r,1,r,1);
    ++geogram_report_progress_round;
    while ( rTr>err && its < max_iter) {
	if(nlCurrentContext != NULL) {	
	    if(nlCurrentContext->progress_func != NULL) {
		nlCurrentContext->progress_func(its, max_iter, rTr, err);
	    }
        geogram_report_progress_callback(geogram_report_progress_tag, (float)its / max_iter);
	    if( (nlCurrentContext->verbose) && !(its % 100)) {
		nl_printf ( "%4d : %.10e -- %.10e\n", its, rTr, err );
	    }
	}
	nlMultMatrixVector(M,d,aux);
	nlMultMatrixVector(P,aux,Sd);
        rTSd=blas->Ddot(blas,N,rT,1,Sd,1);
        nl_assert( fabs(rTSd)>1e-40 );
        alpha=rTh/rTSd;
        blas->Daxpy(blas,N,-alpha,aux,1,r,1);
        blas->Dcopy(blas,N,h,1,s,1);
        blas->Daxpy(blas,N,-alpha,Sd,1,s,1);
	nlMultMatrixVector(M,s,aux);
	nlMultMatrixVector(P,aux,t);
        blas->Daxpy(blas,N,1.,t,1,u,1);
        blas->Dscal(blas,N,alpha,u,1);
        st=blas->Ddot(blas,N,s,1,t,1);
        tt=blas->Ddot(blas,N,t,1,t,1);
        if ( fabs(st)<1e-40 || fabs(tt)<1e-40 ) {
            omega = 0.;
        } else {
            omega = st/tt;
        }
        blas->Daxpy(blas,N,-omega,aux,1,r,1);
        blas->Daxpy(blas,N,-alpha,d,1,x,1);
        blas->Daxpy(blas,N,-omega,s,1,x,1);
        blas->Dcopy(blas,N,s,1,h,1);
        blas->Daxpy(blas,N,-omega,t,1,h,1);
        beta=(alpha/omega)/rTh;
	rTh=blas->Ddot(blas,N,rT,1,h,1);
	beta*=rTh;
        blas->Dscal(blas,N,beta,d,1);
        blas->Daxpy(blas,N,1.,h,1,d,1);
        blas->Daxpy(blas,N,-beta*omega,Sd,1,d,1);
        rTr=blas->Ddot(blas,N,r,1,r,1);
        ++its;
    }
    NL_DELETE_VECTOR(blas, NL_DEVICE_MEMORY, N, r);
    NL_DELETE_VECTOR(blas, NL_DEVICE_MEMORY, N, rT);
    NL_DELETE_VECTOR(blas, NL_DEVICE_MEMORY, N, d);
    NL_DELETE_VECTOR(blas, NL_DEVICE_MEMORY, N, h);
    NL_DELETE_VECTOR(blas, NL_DEVICE_MEMORY, N, u);
    NL_DELETE_VECTOR(blas, NL_DEVICE_MEMORY, N, Sd);
    NL_DELETE_VECTOR(blas, NL_DEVICE_MEMORY, N, t);
    NL_DELETE_VECTOR(blas, NL_DEVICE_MEMORY, N, aux);
    blas->sq_bnorm = b_square;
    blas->sq_rnorm = rTr;
    return its;
}

/* 
 * Note: this one cannot be executed on device (GPU)
 * because it directly manipulates the vectors.
 */
static NLuint nlSolveSystem_GMRES(
    NLBlas_t blas,
    NLMatrix M, NLdouble* b, NLdouble* x,
    double eps, NLuint max_iter, NLuint inner_iter
) {
    NLint    n    = (NLint)M->n;
    NLint    m    = (NLint)inner_iter;
    typedef NLdouble *NLdoubleP;     
    NLdouble *V   = NL_NEW_ARRAY(NLdouble, n*(m+1)   );
    NLdouble *U   = NL_NEW_ARRAY(NLdouble, m*(m+1)/2 );
    NLdouble *r   = NL_NEW_ARRAY(NLdouble, n         );
    NLdouble *y   = NL_NEW_ARRAY(NLdouble, m+1       );
    NLdouble *c   = NL_NEW_ARRAY(NLdouble, m         );
    NLdouble *s   = NL_NEW_ARRAY(NLdouble, m         );
    NLdouble **v  = NL_NEW_ARRAY(NLdoubleP, m+1      );
    NLint i, j, io, uij, u0j; 
    NLint its = -1;
    NLdouble beta, h, rd, dd, nrm2b;

    /* 
     * The way it is written, this routine will not
     * work on the GPU since it directly modifies the
     * vectors.
     */
    nl_assert(nlBlasHasUnifiedMemory(blas));
    
    for ( i=0; i<=m; ++i ){
        v[i]=V+i*n;
    }
    
    nrm2b=blas->Dnrm2(blas,n,b,1);
    io=0;

    do  { /* outer loop */
        ++io;
	nlMultMatrixVector(M,x,r);
        blas->Daxpy(blas,n,-1.,b,1,r,1);
        beta=blas->Dnrm2(blas,n,r,1);
        blas->Dcopy(blas,n,r,1,v[0],1);
        blas->Dscal(blas,n,1./beta,v[0],1);

        y[0]=beta;
        j=0;
        uij=0;
        do { /* inner loop: j=0,...,m-1 */
            u0j=uij;
	    nlMultMatrixVector(M,v[j],v[j+1]);
            blas->Dgemv(
                blas,Transpose,n,j+1,1.,V,n,v[j+1],1,0.,U+u0j,1
            );
            blas->Dgemv(
                blas,NoTranspose,n,j+1,-1.,V,n,U+u0j,1,1.,v[j+1],1
            );
            h=blas->Dnrm2(blas,n,v[j+1],1);
            blas->Dscal(blas,n,1./h,v[j+1],1);
            for (i=0; i<j; ++i ) { /* rotiere neue Spalte */
                double tmp = c[i]*U[uij]-s[i]*U[uij+1];
                U[uij+1]   = s[i]*U[uij]+c[i]*U[uij+1];
                U[uij]     = tmp;
                ++uij;
            }
            { /* berechne neue Rotation */
                rd     = U[uij];
                dd     = sqrt(rd*rd+h*h);
                c[j]   = rd/dd;
                s[j]   = -h/dd;
                U[uij] = dd;
                ++uij;
            }
            { /* rotiere rechte Seite y (vorher: y[j+1]=0) */
                y[j+1] = s[j]*y[j];
                y[j]   = c[j]*y[j];
            }
            ++j;
        } while ( 
            j<m && fabs(y[j])>=eps*nrm2b 
        );
        { /* minimiere bzgl Y */
            blas->Dtpsv(
		blas,
                UpperTriangle,
                NoTranspose,
                NotUnitTriangular,
                j,U,y,1
            );
            /* correct X */
            blas->Dgemv(blas,NoTranspose,n,j,-1.,V,n,y,1,1.,x,1);
        }
    } while ( fabs(y[j])>=eps*nrm2b && (m*(io-1)+j) < (NLint)max_iter);
    
    /* Count the inner iterations */
    its = m*(io-1)+j;
    blas->sq_bnorm = nrm2b*nrm2b;
    blas->sq_rnorm = y[j]*y[j];
    NL_DELETE_VECTOR(blas, NL_DEVICE_MEMORY, n, V);
    NL_DELETE_VECTOR(blas, NL_DEVICE_MEMORY, n, U);
    NL_DELETE_VECTOR(blas, NL_DEVICE_MEMORY, n, r);
    NL_DELETE_VECTOR(blas, NL_DEVICE_MEMORY, n, y);
    NL_DELETE_VECTOR(blas, NL_DEVICE_MEMORY, n, c);
    NL_DELETE_VECTOR(blas, NL_DEVICE_MEMORY, n, s);
    NL_DELETE_VECTOR(blas, NL_DEVICE_MEMORY, n, v);
    return (NLuint)its;
}

/************************************************************************/
/* Main driver routine */

NLuint nlSolveSystemIterative(
    NLBlas_t blas,
    NLMatrix M, NLMatrix P, NLdouble* b_in, NLdouble* x_in,
    NLenum solver,
    double eps, NLuint max_iter, NLuint inner_iter
) {
    NLuint N = M->n;
    NLuint result=0;
    NLdouble rnorm=0.0;
    NLdouble bnorm=0.0; 
    double* b = b_in;
    double* x = x_in;
    nl_assert(M->m == M->n);

    if(!nlBlasHasUnifiedMemory(blas)) {
	b = NL_NEW_VECTOR(blas, NL_DEVICE_MEMORY, (int)M->n);
	blas->Memcpy(
	    blas,
	    b, NL_DEVICE_MEMORY,
	    b_in, NL_HOST_MEMORY, (size_t)N*sizeof(double)
	);
	x = NL_NEW_VECTOR(blas, NL_DEVICE_MEMORY, (int)M->n);
	blas->Memcpy(
	    blas,
	    x, NL_DEVICE_MEMORY,
	    x_in, NL_HOST_MEMORY, (size_t)N*sizeof(double)
	);	
    }

    switch(solver) {
	case NL_CG:
	    if(P == NULL) {
		result = nlSolveSystem_CG(blas,M,b,x,eps,max_iter);
	    } else {
		result = nlSolveSystem_PRE_CG(blas,M,P,b,x,eps,max_iter);
	    }
	    break;
	case NL_BICGSTAB:
	    if(P == NULL) {
		result = nlSolveSystem_BICGSTAB(blas,M,b,x,eps,max_iter);
	    } else {
		result = nlSolveSystem_PRE_BICGSTAB(blas,M,P,b,x,eps,max_iter);
	    }
	    break;
	case NL_GMRES:
	    result = nlSolveSystem_GMRES(blas,M,b,x,eps,max_iter,inner_iter);
	    break;
	default:
	    nl_assert_not_reached;
    }


    /* Get residual norm and rhs norm from BLAS context */
    if(nlCurrentContext != NULL) {
	bnorm = sqrt(blas->sq_bnorm);
	rnorm = sqrt(blas->sq_rnorm);
	if(bnorm == 0.0) {
	    nlCurrentContext->error = rnorm;
	    if(nlCurrentContext->verbose) {
		nl_printf(
		    "in OpenNL : ||Ax-b|| = %e\n",nlCurrentContext->error
		);
	    }
	} else {
	    nlCurrentContext->error = rnorm/bnorm;
	    if(nlCurrentContext->verbose) {
		nl_printf("in OpenNL : ||Ax-b||/||b|| = %e\n",
		       nlCurrentContext->error
		);
	    }
	}
	nlCurrentContext->used_iterations = result;
    }


    if(!nlBlasHasUnifiedMemory(blas)) {
	blas->Memcpy(
	    blas,
	    x_in, NL_HOST_MEMORY, x, NL_DEVICE_MEMORY, (size_t)N*sizeof(double)
	);	
	NL_DELETE_VECTOR(blas, NL_DEVICE_MEMORY, (int)M->n, x);
	NL_DELETE_VECTOR(blas, NL_DEVICE_MEMORY, (int)M->n, b);
    }
    
    return result;
}

/************************************************************************/
