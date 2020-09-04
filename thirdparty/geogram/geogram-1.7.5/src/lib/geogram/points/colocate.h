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

#ifndef GEOGRAM_POINTS_COLOCATE
#define GEOGRAM_POINTS_COLOCATE

#include <geogram/basic/common.h>
#include <geogram/basic/geometry.h>
#include <vector>

/**
 * \file geogram/points/colocate.h
 * \brief Functions to merge points with identical or similar locations
 */

namespace GEO {

    namespace Geom {

        /**
         * \brief Finds sets of identical points in a point set.
         * \param[in] points the point array
         * \param[in] dim dimension of the points
         * \param[in] nb_points number of points
         * \param[out] old2new an array of size nb_points.
         * \param[in] tolerance threshold for colocating points.
         * \param[in] stride number of doubles between two consecutive
         *  points (set to dim if unspecified).
         * \param[in] nn_algo factory name for nearest neighbor search.
         * \return the number of unique points
         */
        index_t GEOGRAM_API colocate(
            const double* points,
            coord_index_t dim,
            index_t nb_points,
            vector<index_t>& old2new,
            double tolerance = 0.0,
            index_t stride = 0,
            const std::string& nn_algo = "default"
        );

        /**
         * \brief Finds sets of identical points in a point set.
         * \details This version uses a lexicographic sort. It does not
         *  have a 'tolerance' parameter (only points with exactly
         *  the same coordinates can be colocated).
         * \param[in] points the point array
         * \param[in] dim dimension of the points
         * \param[in] nb_points number of points
         * \param[out] old2new an array of size nb_points.
         * \param[in] stride number of doubles between two consecutive
         *  points (set to dim if unspecified).
         * \return the number of unique points
         */
        index_t GEOGRAM_API colocate_by_lexico_sort(
            const double* points,
            coord_index_t dim,
            index_t nb_points,
            vector<index_t>& old2new,
            index_t stride
        );
    }
}

#endif

