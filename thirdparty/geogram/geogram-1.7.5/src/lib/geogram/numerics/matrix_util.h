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

#ifndef GEOGRAM_NUMERICS_MATRIX_UTIL
#define GEOGRAM_NUMERICS_MATRIX_UTIL

#include <geogram/basic/common.h>
#include <geogram/basic/numeric.h>

/**
 * \file geogram/numerics/matrix_util.h
 * \brief Some utilities for matrix manipulation
 */

namespace GEO {

    /**
     * \brief Utilities for manipulating matrices
     */
    namespace MatrixUtil {

        /**
         * \brief Computes the eigen values and eigen vectors
         * of a semi definite symmetric matrix
         *
         * \param[in] matrix is stored in column symmetric storage:
         *  - matrix = { m11, m12, m22, m13, m23, m33, m14, m24, m34, m44 ... }
         *  - size = n(n+1)/2
         *
         * \param[in] n the dimension of the matrix
         *
         * \param[out] eigen_vectors = { v1, v2, v3, ..., vn }, where:
         *  - vk = vk0, vk1, ..., vkn
         *  - size = n^2, must be allocated by caller
         *
         * \param[out] eigen_values are in decreasing order
         *  - size = n, must be allocated by caller
         */
        void GEOGRAM_API semi_definite_symmetric_eigen(
            const double* matrix, index_t n,
            double* eigen_vectors, double* eigen_values
        );
    }
}

#endif

