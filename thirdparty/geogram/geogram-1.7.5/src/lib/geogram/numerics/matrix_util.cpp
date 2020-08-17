/*
 *  Copyright (c) 2012-2014, Bruno Levy
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

#include <geogram/numerics/matrix_util.h>
#include <geogram/basic/process.h>

namespace GEO {

    namespace {
        const double EPS = 0.00001;
        index_t MAX_ITER = 100;
    }

    namespace MatrixUtil {

        void semi_definite_symmetric_eigen(
            const double* mat, index_t n, double* eigen_vec, double* eigen_val
        ) {
            // Number of entries in mat

            index_t nn = (n * (n + 1)) / 2;

            // ==== Step 1: Copy mat to a

            // Note: a is allocated on the stack,
            //  it is more multithread friendly.
            double* a = (double*) (alloca(sizeof(double) * nn));
            for(index_t ij = 0; ij < nn; ij++) {
                a[ij] = mat[ij];
            }

            // Ugly Fortran-porting trick: indices for a are between 1 and n
            a--;

            // ==== Step 2 : Init diagonalization matrix as the unit matrix

            // Note: v is allocated on the stack,
            //  it is more multithread friendly.
            double* v = (double*) (alloca(sizeof(double) * n * n));

            index_t ij = 0;
            for(index_t i = 0; i < n; i++) {
                for(index_t j = 0; j < n; j++) {
                    v[ij] = (i == j) ? 1.0 : 0.0;
                    ij++;
                }
            }

            // Ugly Fortran-porting trick: indices for v are between 1 and n
            v--;

            // ==== Step 3 : compute the weight of the non diagonal terms
            ij = 1;
            double a_norm = 0.0;
            for(index_t i = 1; i <= n; i++) {
                for(index_t j = 1; j <= i; j++) {
                    if(i != j) {
                        double a_ij = a[ij];
                        a_norm += a_ij * a_ij;
                    }
                    ij++;
                }
            }

            if(a_norm != 0.0) {

                double a_normEPS = a_norm * EPS;
                double thr = a_norm;
                index_t nb_iter = 0;

                // Step 4 : rotations
                while(thr > a_normEPS && nb_iter < MAX_ITER) {

                    nb_iter++;
                    double thr_nn = thr / double(nn);

                    for(index_t l = 1; l < n; l++) {
                        for(index_t m = l + 1; m <= n; m++) {

                            // compute sinx and cosx

                            index_t lq = (l * l - l) / 2;
                            index_t mq = (m * m - m) / 2;

                            index_t lm = l + mq;
                            double a_lm = a[lm];
                            double a_lm_2 = a_lm * a_lm;

                            if(a_lm_2 < thr_nn) {
                                continue;
                            }

                            index_t ll = l + lq;
                            index_t mm = m + mq;
                            double a_ll = a[ll];
                            double a_mm = a[mm];

                            double delta = a_ll - a_mm;

                            double x;
                            if(delta == 0.0) {
                                x = -M_PI / 4;
                            } else {
                                x = -atan((a_lm + a_lm) / delta) / 2.0;
                            }

                            double sinx = sin(x);
                            double cosx = cos(x);
                            double sinx_2 = sinx * sinx;
                            double cosx_2 = cosx * cosx;
                            double sincos = sinx * cosx;

                            // rotate L and M columns

                            index_t ilv = n * (l - 1);
                            index_t imv = n * (m - 1);

                            for(index_t i = 1; i <= n; i++) {
                                if((i != l) && (i != m)) {
                                    index_t iq = (i * i - i) / 2;
                                    index_t im;

                                    if(i < m) {
                                        im = i + mq;
                                    } else {
                                        im = m + iq;
                                    }
                                    double a_im = a[im];

                                    index_t il;
                                    if(i < l) {
                                        il = i + lq;
                                    } else {
                                        il = l + iq;
                                    }
                                    double a_il = a[il];

                                    a[il] = a_il * cosx - a_im * sinx;
                                    a[im] = a_il * sinx + a_im * cosx;
                                }

                                ilv++;
                                imv++;

                                double v_ilv = v[ilv];
                                double v_imv = v[imv];

                                v[ilv] = cosx * v_ilv - sinx * v_imv;
                                v[imv] = sinx * v_ilv + cosx * v_imv;
                            }

                            x = a_lm * sincos;
                            x += x;

                            a[ll] = a_ll * cosx_2 + a_mm * sinx_2 - x;
                            a[mm] = a_ll * sinx_2 + a_mm * cosx_2 + x;
                            a[lm] = 0.0;

                            thr = fabs(thr - a_lm_2);
                        }
                    }
                }
            }

            // ==== Step 5: index conversion and copy eigen values

            // back from Fortran to C++
            a++;

            for(index_t i = 0; i < n; i++) {
                index_t k = i + (i * (i + 1)) / 2;
                eigen_val[i] = a[k];
            }

            // ==== Step 6: sort the eigen values and eigen vectors

            // Note: index is allocated on the stack,
            //  it is more multithread friendly.
            index_t* index = (index_t*) (alloca(sizeof(index_t) * n));

            for(index_t i = 0; i < n; i++) {
                index[i] = i;
            }

            for(index_t i = 0; i < (n - 1); i++) {
                double x = eigen_val[i];
                index_t k = i;

                for(index_t j = i + 1; j < n; j++) {
                    if(x < eigen_val[j]) {
                        k = j;
                        x = eigen_val[j];
                    }
                }

                eigen_val[k] = eigen_val[i];
                eigen_val[i] = x;

                index_t jj = index[k];
                index[k] = index[i];
                index[i] = jj;
            }

            // ==== Step 7: save the eigen vectors

            v++; // back from Fortran to to C++

            ij = 0;
            for(index_t k = 0; k < n; k++) {
                index_t ik = index[k] * n;
                for(index_t i = 0; i < n; i++) {
                    eigen_vec[ij++] = v[ik++];
                }
            }
        }
    }
}

