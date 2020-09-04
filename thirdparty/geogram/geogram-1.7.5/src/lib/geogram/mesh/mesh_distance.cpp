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

#include <geogram/mesh/mesh_distance.h>
#include <geogram/mesh/mesh_AABB.h>
#include <geogram/mesh/mesh_repair.h>
#include <geogram/mesh/mesh_sampling.h>
#include <geogram/basic/process.h>
#include <geogram/basic/stopwatch.h>

#include <algorithm>

namespace {

    using namespace GEO;

    /**
     * \brief Computes largest distance between
     *  an interval of an array of points and
     *  a surface stored in a MeshFacetsAABB.
     * \details Used by compute_max_distance() that
     *  runs multiple instances of this class
     *  (one per core).
     */
    class DistanceThread : public Thread {
    public:
        /**
         * \brief Constructs a new DistanceThread
         * \details This DistanceThread will compute the distances
         *  between a batch of points and an axis-aligned bounding box
         *  tree.
         * \param[in] AABB The axis-aligned bounding box tree used to compute
         *  the distance
         * \param[in] from first point index
         * \param[in] to one position past the last point index
         * \param[in] points_ptr pointer to the points
         * \param[in] points_stride number of doubles between two
         *  consecutive points
         */
        DistanceThread(
            const MeshFacetsAABB& AABB,
            index_t from, index_t to,
            const double* points_ptr, index_t points_stride = 3
        ) :
            AABB_(AABB),
            from_(from),
            to_(to),
            max_sq_dist_(0.0),
            points_ptr_(points_ptr),
            points_stride_(points_stride) {
        }

        /**
         * \brief Runs the thread.
         * \details Computes the distances for the batch of points associated
         *  with this thread.
         */
	void run() override {
            for(index_t v = from_; v < to_; v++) {
                // TODO: optimization
                // if we know that the points are spatially
                // sorted, then we can use AABB_.squared_distance_with_hint()
                double sq_dist = AABB_.squared_distance(
                    *reinterpret_cast<const vec3*>(
                        points_ptr_ + v * points_stride_
                    )
                );
                max_sq_dist_ = std::max(
                    max_sq_dist_, sq_dist
                );
            }
        }

        /**
         * \brief Gets the computed max squared distance.
         * \return the maximum squared distance computed so far
         */
        double max_squared_distance() const {
            return max_sq_dist_;
        }

    private:
        const MeshFacetsAABB& AABB_;
        index_t from_;
        index_t to_;
        double max_sq_dist_;
        const double* points_ptr_;
        index_t points_stride_;
    };

    /**
     * \brief Computes largest distance between
     *  an array of points and a mesh stored in
     *  a MeshFacetsAABB.
     * \details Uses a multithread implementation.
     * \param[out] result the maximum squared distance
     * \param[in] AABB the mesh stored in an axis-aligned bounding box
     * \param[in] nb_points number of query points
     * \param[in] points_ptr pointer to the points coordinates
     * \param[in] points_stride number of doubles between two consecutive
     *  points
     */
    void compute_max_distance(
        double& result,
        const MeshFacetsAABB& AABB,
        index_t nb_points,
        const double* points_ptr,
        index_t points_stride = 3
    ) {
        SystemStopwatch W;
        TypedThreadGroup<DistanceThread> threads;
        index_t nb_threads = Process::maximum_concurrent_threads();
        index_t batch_size = nb_points / nb_threads;
        index_t cur = 0;
        index_t remaining = nb_points;
        for(index_t i = 0; i < nb_threads; i++) {
            index_t this_batch_size = batch_size;
            if(i == nb_threads - 1) {
                this_batch_size = remaining;
            }
            threads.push_back(
                new DistanceThread(
                    AABB,
                    cur, cur + this_batch_size,
                    points_ptr, points_stride
                )
            );
            cur += this_batch_size;
            remaining -= this_batch_size;
        }
        geo_assert(remaining == 0);
        Process::run_threads(threads);
        for(index_t t = 0; t < threads.size(); t++) {
            result = std::max(result, threads[t]->max_squared_distance());
        }
        double elapsed = W.elapsed_user_time();
        if(elapsed == 0.0) {
            Logger::out("AABB")
                << "???? Mqueries / s (too fast to be measured)"
                << std::endl;
        } else {
            Logger::out("AABB")
                << double(nb_points) / (1.0e6 * elapsed)
                << " Mqueries / s"
                << std::endl;
        }
    }
}

/****************************************************************************/

namespace GEO {

    double mesh_one_sided_Hausdorff_distance(
        Mesh& m1, Mesh& m2, double sampling_step
    ) {
        double result = 0.0;
        MeshFacetsAABB AABB(m2);

        index_t nb_points = 0;
        
        if(m1.cells.nb() == 0 && m1.edges.nb() == 0) {
            nb_points = m1.vertices.nb();
            compute_max_distance(
                result, AABB, m1.vertices.nb(),
                m1.vertices.point_ptr(0), m1.vertices.dimension()
            );
        } else {
            // If the mesh has cells, then we need to remove the vertices
            // that are not on the surface, else their distance to the
            // other surface will be included in the computation !
            vector<bool> on_surface(m1.vertices.nb(),false);
            for(index_t f: m1.facets) {
                for(index_t c: m1.facets.corners(f)) {
                    on_surface[m1.facet_corners.vertex(c)] = true;
                }
            }
            for(index_t v: m1.vertices) {
                if(on_surface[v]) {
                    ++nb_points;
                }
            }
            vector<double> points;
            points.reserve(nb_points*m1.vertices.dimension());
            for(index_t v: m1.vertices) {
                if(on_surface[v]) {                
                    for(index_t c=0; c<m1.vertices.dimension(); ++c) {
                        points.push_back(m1.vertices.point_ptr(v)[c]);
                    }
                }
            }

            compute_max_distance(
                result, AABB, nb_points, points.data(),
                m1.vertices.dimension()
            );
        }

        index_t nb_samples = index_t(
            Geom::mesh_area(m1, 3) / geo_sqr(sampling_step)
        );

        if(nb_samples > nb_points) {            

            nb_samples -= nb_points;
            Logger::out("Distance") << "Using " << nb_samples
                << " additional samples"
                << std::endl;
            vector<double> samples(nb_samples * 3);
            Attribute<double> weight; // left unbound
            mesh_generate_random_samples_on_surface<3>(
                m1, samples.data(), nb_samples, weight
            );
            compute_max_distance(
                result, AABB, nb_samples, samples.data(), 3
            );
        }

        return ::sqrt(result);
    }

    double mesh_symmetric_Hausdorff_distance(
        Mesh& m1, Mesh& m2, double sampling_step
    ) {
        return std::max(
            mesh_one_sided_Hausdorff_distance(m1, m2, sampling_step),
            mesh_one_sided_Hausdorff_distance(m2, m1, sampling_step)
        );
    }
}

