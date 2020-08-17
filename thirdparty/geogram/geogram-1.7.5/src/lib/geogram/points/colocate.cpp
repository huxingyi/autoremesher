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

#include <geogram/points/colocate.h>
#include <geogram/points/nn_search.h>
#include <geogram/basic/geometry_nd.h>
#include <geogram/basic/process.h>
#include <geogram/basic/command_line.h>
#include <geogram/basic/algorithm.h>

namespace {

    using namespace GEO;

    /**
     * \brief Implements the colocate() algorithm when a tolerance is used.
     * \details Uses multiple threads to speedup computations.
     */
    class Colocate {
    public:
        /**
         * \brief Creates a new Colocate object.
         * \param[in] NN the nearest neighbors search data structure
         * \param[out] old2new where to store the relation between old indices
         *   and colocated vertices
         * \param[in] tolerance maximum distance for colocated vertices
         */
        Colocate(
            NearestNeighborSearch* NN,
            vector<index_t>& old2new,
            double tolerance
        ) :
            NN_(NN),
            old2new_(old2new),
            sq_tolerance_(geo_sqr(tolerance)) {
        }

        /**
         * \brief Returns the number of points.
         */
        index_t nb_points() const {
            return NN_->nb_points();
        }

        /**
         * \brief Finds the nearest neighbors of a given point.
         * \param[in] i index of the query point
         * \param[in] nb maximum number of neighbors
         * \return true when all the neighbors nearer than
         *    tolerance have been found, false otherwise.
         */
        bool find_nearest_neighbors(index_t i, index_t nb) {
            // allocated on the stack, more multithread-friendly
            // and no need to deallocate (and VC++ does not support
            // int neighbors[nb] where nb is a variable)
            index_t* neighbors = (index_t*) alloca(sizeof(index_t) * nb);
            double* dist = (double*) alloca(sizeof(double) * nb);

            NN_->get_nearest_neighbors(
                nb, NN_->point_ptr(i), neighbors, dist
            );

            index_t smallest = i;
            for(index_t jj = 0; jj < nb; jj++) {
                if(dist[jj] > sq_tolerance_) {
                    old2new_[i] = smallest;
                    return true;
                }
                smallest = std::min(smallest, neighbors[jj]);
            }
            old2new_[i] = smallest;
            return false;
        }

        /**
         * \brief Finds all the neighbors nearer than tolerance from 
         * a given point.
         * \details Called in parallel using parallel_for().
         * \param[in] i index of the query point
         */
        void do_it(index_t i) {
            index_t nb = std::min(index_t(6),nb_points());
            while(!find_nearest_neighbors(i, nb) && nb < nb_points()) {
                if(nb == nb_points()) {
                    break;
                }
                nb += nb / 2;
                nb = std::min(nb, nb_points());
            }
        }

    private:
        NearestNeighborSearch* NN_;
        vector<index_t>& old2new_;
        double sq_tolerance_;
    };

    /************************************************************************/

    /**
     * \brief A comparator for sorting points in lexicographic order.
     */
    class ComparePoints {
    public:
        /**
         * \brief Constructs a new ComparePoints
         * \param[in] points pointer to the array of points
         * \param[in] dim number of coordinates
         * \param[in] stride number of doubles between two consecutive points
         *   (= dim if the array is packed).
         */
        ComparePoints(
            const double* points, coord_index_t dim, index_t stride
        ) :
            points_(points),
            dim_(dim),
            stride_(stride) {
        }

        /**
         * \brief Compares two points given their indices.
         * \param[in] i1 index of the first point
         * \param[in] i2 index of the second point
         */
        bool is_before(index_t i1, index_t i2) const {
            const double* p1 = points_ + i1 * stride_;
            const double* p2 = points_ + i2 * stride_;
            for(coord_index_t c = 0; c < dim_; c++) {
                if(p1[c] < p2[c]) {
                    return true;
                }
                if(p1[c] > p2[c]) {
                    return false;
                }
            }
            return false;
        }

        /**
         * \brief Tests whether two points are identical.
         * \param[in] i1 index of the first point
         * \param[in] i2 index of the second point
         */
        bool is_same(index_t i1, index_t i2) const {
            const double* p1 = points_ + i1 * stride_;
            const double* p2 = points_ + i2 * stride_;
            for(coord_index_t c = 0; c < dim_; c++) {
                if(p1[c] != p2[c]) {
                    return false;
                }
            }
            return true;
        }

        /**
         * \brief Compares two points given their indices.
         * \param[in] i1 index of the first point
         * \param[in] i2 index of the second point
         * \return true if point \p i1 is before point \p i2, false
         *   otherwise.
         */
        bool operator() (index_t i1, index_t i2) const {
            return is_before(i1, i2);
        }

    private:
        const double* points_;
        coord_index_t dim_;
        index_t stride_;
    };
}

/****************************************************************************/

namespace GEO {

    namespace Geom {

        index_t colocate(
            const double* points,
            coord_index_t dim,
            index_t nb_points,
            vector<index_t>& old2new,
            double tolerance,
            index_t stride,
            const std::string& nn_algo
        ) {
            if(nb_points == 0) {
                return 0;
            }
            
            if(stride == 0) {
                stride = dim;
            }
            NearestNeighborSearch_var NN = NearestNeighborSearch::create(
                dim, nn_algo
            );
            NN->set_points(nb_points, points, stride);
            old2new.resize(nb_points, index_t(-1));
            Colocate colocate_obj(NN, old2new, tolerance);
	    
            if(CmdLine::get_arg_bool("sys:multithread")) {
                parallel_for(
		    0, nb_points,
		    [&colocate_obj](index_t i){ colocate_obj.do_it(i); },
		    1, true
		);
            } else {
                for(index_t i = 0; i < nb_points; i++) {
                    colocate_obj.do_it(i);
                }
            }
            index_t result = 0;
            for(index_t i = 0; i < old2new.size(); i++) {
                geo_assert(
                    signed_index_t(old2new[i]) >= 0 &&
                    old2new[i] < nb_points
                );
                index_t j = i;
                // colocate clusters of identical vertices onto smallest index
                while(old2new[j] != j) {
                    j = old2new[j];
                }
                old2new[i] = j;
                if(old2new[i] == i) {
                    result++;
                }
            }
            return result;
        }

        index_t colocate_by_lexico_sort(
            const double* points,
            coord_index_t dim,
            index_t nb_points,
            vector<index_t>& old2new,
            index_t stride
        ) {
            if(nb_points == 0) {
                return 0;
            }
            
            ComparePoints compare_points(points, dim, stride);
            vector<index_t> sorted_indices(nb_points);
            for(index_t i = 0; i < nb_points; i++) {
                sorted_indices[i] = i;
            }
            GEO::sort(
                sorted_indices.begin(), sorted_indices.end(), compare_points
            );
            old2new.assign(nb_points, index_t(-1));

            index_t nb_distinct = 0;

            index_t iv1 = 0;
            while(iv1 < nb_points) {
                nb_distinct++;
                old2new[sorted_indices[iv1]] = sorted_indices[iv1];
                index_t iv2 = iv1 + 1;
                while(
                    iv2 < nb_points &&
                    compare_points.is_same(
                        sorted_indices[iv1], sorted_indices[iv2]
                    )
                ) {
                    old2new[sorted_indices[iv2]] = sorted_indices[iv1];
                    iv2++;
                }
                iv1 = iv2;
            }
            return nb_distinct;
        }
    }
}

