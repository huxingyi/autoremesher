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

#ifndef GEOGRAM_DELAUNAY_LFS
#define GEOGRAM_DELAUNAY_LFS

#include <geogram/basic/common.h>
#include <geogram/delaunay/delaunay.h>

/**
 * \file geogram/delaunay/LFS.h
 * \brief A class for computing local feature size
 */

namespace GEO {

    /**
     * \brief Computes an approximation of lfs (local feature size).
     *
     *  Given a surface S and a point p in R^3, lfs(p) corresponds to
     *  the distance between p and the medial axis of S. Given a
     *  sampling of S, this class computes the distance to the nearest pole
     *  (a good approximation of lfs, see Amenta and Bern's paper).
     */
    class GEOGRAM_API LocalFeatureSize {
    public:
        /**
         * \brief Initializes lfs computation.
         * \param[in] nb_pts number of points
         * \param[in] pts a sampling of the surface, represented by
         *  a contiguous array of doubles.
         */
        LocalFeatureSize(index_t nb_pts, const double* pts) {
            sliver_angle_threshold_ = 0.01;
            init(nb_pts, pts);
        }

        /**
         * \brief Computes the squared local feature size at a query point.
         * \param[in] p the query point
         * \return approximate squared local feature size at point \p p.
         */
        double squared_lfs(const double* p) const {
            index_t v = spatial_search_->nearest_vertex(p);
            const double* q = spatial_search_->vertex_ptr(v);
            return
                geo_sqr(p[0] - q[0]) +
                geo_sqr(p[1] - q[1]) +
                geo_sqr(p[2] - q[2]);
        }

        /**
         * \brief Gets the number of poles.
         * \return the number of poles.
         */
        index_t nb_poles() const {
            return poles_.size()/3;
        }

        /**
         * \brief Gets a reference to a pole.
         * \param[in] i the index of the pole
         * \return a const pointer to the three coordinates of the
         *  \p i th pole
         * \pre i < nb_poles()
         */
        const double* pole(index_t i) const {
            geo_debug_assert(i < nb_poles());
            return &poles_[3*i];
        }
        
    protected:
        /**
         * \brief Constructs the internal representation used to compute
         *  the local feature size.
         * \param[in] nb_pts number of points
         * \param[in] pts pointer to the points coordinates, as a contiguous
         *  array of doubles.
         */
        void init(index_t nb_pts, const double* pts);

    private:
        double sliver_angle_threshold_;
        vector<double> poles_;
        Delaunay_var spatial_search_;
    };
}

#endif

