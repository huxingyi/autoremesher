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

#ifndef GEOGRAM_VORONOI_CVT
#define GEOGRAM_VORONOI_CVT

#include <geogram/basic/common.h>
#include <geogram/voronoi/RVD.h>
#include <geogram/voronoi/integration_simplex.h>
#include <geogram/mesh/mesh.h>
#include <geogram/delaunay/delaunay.h>


/**
 * \file geogram/voronoi/CVT.h
 * \brief Main class for computing centroidal Voronoi tesselations.
 */

namespace GEO {

    class RestrictedVoronoiDiagram;
    class ProgressTask;

    /**
     * \brief CentroidalVoronoiTesselation is the main component
     *  of the remeshing algorithm.
     *
     * \details It evenly distributes points over a surface embedded in Rn,
     *  where n can be of arbitrary dimension. The geometrical
     *  computations are done by RestrictedVoronoiDiagram, and
     *  the numerical optimization by Optimizer.
     */
    class GEOGRAM_API CentroidalVoronoiTesselation {

        /** \brief This class type */
        typedef CentroidalVoronoiTesselation thisclass;

    public:
        /**
         * \brief Constructs a new CentroidalVoronoiTesselation.
         * \details This constructor should be used when the
         *  first three coordinates of the mesh are x,y,z.
         * \param[in] mesh a pointer to the input mesh
         * \param[in] dimension If set, uses only the dimension first
         *  coordinates in mesh, else dimension is determined
         *  by mesh->dimension().
         * \param[in] delaunay factory name of the implementation of
         *  Delaunay triangulation. Default uses ANN and radius
         *  of security.
         */
        CentroidalVoronoiTesselation(
            Mesh* mesh,
            coord_index_t dimension = 0,
            const std::string& delaunay = "default"
        );

        /**
         * \brief Constructs a new CentroidalVoronoiTesselation.
         * \details This constructor should be used when the coordinates of
         *  the mesh are not related with R3.
         * \param[in] mesh a pointer to the input mesh
         * \param[in] R3_embedding (dimension = mesh->nb_vertices()):
         *  coordinates of the mesh vertices in R3. Ignored
         *  if size is zero.
         * \param[in] dimension If set, uses only the dimension first
         *  coordinates in mesh, else dimension is determined
         *  by mesh->dimension().
         * \param[in] delaunay factory name of the implementation of
         *  Delaunay triangulation. delaunay="default" uses
         *  ANN and radius of security.
         */
        CentroidalVoronoiTesselation(
            Mesh* mesh,
            const vector<vec3>& R3_embedding, coord_index_t dimension = 0,
            const std::string& delaunay = "default"
        );

        /**
         * \brief Destructor
         */
        virtual ~CentroidalVoronoiTesselation();

        /**
         * \brief Computes a random initial sampling of the surface in nD.
         *
         * \details This initial sampling (of low quality/regularity) needs to
         * be further optimized (using Lloyd_iterations() and
         *  Newton_iterations()).
         *
         * \param[in] nb_samples number of points to generate in the sampling
         */
        bool compute_initial_sampling(index_t nb_samples);

        /**
         * \brief Initializes the points with a user-specified vector.
         *
         * \param[in] nb_points number of points in \p points
         * \param[in] points (size = dimension()*nb_points):
         *  user-defined initialization. It is copied into
         *  an internal vector
         */
        void set_points(index_t nb_points, const double* points);

        /**
         * \brief Changes the number of points.
         * \param[in] nb_points new number of points
         * \details Resizes the internal vector used to store the points
         */
        void resize_points(index_t nb_points);

        /**
         * \brief Relaxes the points with Lloyd's algorithm.
         * \details It is in general less efficient than Newton, but more
         *  resistant to heterogeneous point distribution. Therefore a
         *  small number of Lloyd iterations may be used right after
         *  a call to compute_initial_sampling() to regularize
         *  the point set before calling Newton_iterations().
         * \param[in] nb_iter number of iterations
         */
        virtual void Lloyd_iterations(index_t nb_iter);

        /**
         * \brief Relaxes the points with Newton-Lloyd's algorithm.
         * \param[in] nb_iter number of iterations
         * \param[in] m number of evaluations used for Hessian approximation
         */
        virtual void Newton_iterations(
            index_t nb_iter, index_t m = 7
        );

        /**
         * \brief Computes the surfacic mesh (using the current points).
         * \param[out] mesh the computed surface
         * \param[in] multinerve If set, does topology control (uses
         *  the dual of the connected components of the RVD).
         */
        void compute_surface(Mesh* mesh, bool multinerve = true);

        /**
         * \brief Computes the volumetric mesh (using the current points).
         * \param[out] mesh the computed volumetric mesh
         * \pre volumetric()
         */
        void compute_volume(Mesh* mesh);

        /**
         * \brief Specifies whether a progress bar should be used.
         * \param[in] x If set, shows iterations using a "progress bar".
         */
        void set_show_iterations(bool x) {
            show_iterations_ = x;
        }

        /**
         * \brief Specifies whether centroids of Voronoi cells should be used.
         * \param[in] x If set (default = true), compute_surface() replaces
         *  the vertices with the centroids of the
         *  connected components of the restricted Voronoi cells.
         */
        void set_use_RVC_centroids(bool x) {
            use_RVC_centroids_ = x;
        }

        /**
         * \brief Specifies whether constrained mode should be used.
         * \param[in] x If set (default = false), compute_surface() projects
         * the vertices onto the input surface.
         */
        void set_constrained_cvt(bool x) {
            constrained_cvt_ = x;
        }

        /**
         * Returns the input mesh.
         */
        Mesh* mesh() {
            return mesh_;
        }

        /**
         * Returns the Delaunay triangulation.
         */
        Delaunay* delaunay() {
            return delaunay_;
        }

        /**
         * Returns the RestrictedVoronoiDiagram.
         */
        RestrictedVoronoiDiagram* RVD() {
            return RVD_;
        }

        /**
         * \brief Restricts computation to a part of the input mesh.
         * \details The part of the input mesh should be specified as
         *    a contiguous range of facet indices.
         * \param[in] facets_begin first facet in the range
         * \param[in] facets_end one past last facet in the range
         */
        void set_facets_range(index_t facets_begin, index_t facets_end) {
            RVD_->set_facets_range(facets_begin, facets_end);
        }

        /**
         * \brief Makes this CentroidalVoronoiTesselation the current one.
         * \details The Optimizer uses global variables, therefore there can
         *  be only one CentroidalVoronoiTesselation simultaneously active.
         *  This function can be used to change the currently active
         *  CentroidalVoronoiTesselation.
         * \note Most users will not need to use this function.
         * \pre There is no current CentroidalVoronoiTesselation.
         */
        void make_current() {
            geo_assert(instance_ == nullptr);
            instance_ = this;
        }

        /**
         * \brief Resets the current CentroidalVoronoiTesselation to nullptr.
         * \details The Optimizer uses global variables, therefore there can
         *  be only one CentroidalVoronoiTesselation simultaneously active.
         *  This function can be used to change the currently active
         *  CentroidalVoronoiTesselation.
         * \note Most users will not need to use this function.
         * \pre This CentroidalVoronoiTesselation is the current one.
         */
        void done_current() {
            geo_assert(instance_ == this);
            instance_ = nullptr;
        }

    public:
        /**
         * \brief Callback for the numerical solver.
         * \details Evaluates the objective function and its gradient.
         * \param[in] n number of variables
         * \param[in] x current value of the variables
         * \param[out] f current value of the objective function
         * \param[out] g gradient of the objective function
         */
        static void funcgrad_CB(
            index_t n, double* x, double& f, double* g
        );

        /**
         * \brief Callback for the numerical solver.
         * \details Updates the progress bar.
         * \param[in] n number of variables
         * \param[in] x current value of the variables
         * \param[in] f current value of the objective function
         * \param[in] g gradient of the objective function
         * \param[in] gnorm norm of the gradient of the objective function
         */
        static void newiteration_CB(
            index_t n, const double* x, double f, const double* g, double gnorm
        );

        /**
         * \brief Sets a client for the progress bars.
         * \param[in] progress the ProgressTask.
         */
        void set_progress_logger(ProgressTask* progress) {
            progress_ = progress;
        }

        /**
         * \brief Gets the dimension of the points.
         * \details Can be smaller than the dimension of the mesh.
         */
        coord_index_t dimension() const {
            return dimension_;
        }

        /**
         * \brief Gets the number of points to be optimized.
         */
        index_t nb_points() const {
            return index_t(points_.size() / dimension_);
        }

        /**
         * \brief Gets the representation of a point in R3.
         * \param[in] p index of the point
         * \return a const reference to the 3d version of the point
         * \pre p < nb_points()
         */
        const vec3& R3_embedding(index_t p) const {
            return RVD_->R3_embedding(p);
        }

        /**
         * \brief Returns the representation of a point in embedding space.
         * \param[in] p index of the point
         * \return a pointer to the coordinates of the point
         * \pre p < nb_points()
         */
        double* embedding(index_t p) {
            geo_debug_assert(p < nb_points());
            return &(points_[0]) + dimension_ * p;
        }

        /**
         * \brief Tests whether volumetric mode is used.
         */
        bool volumetric() const {
            return RVD_->volumetric();
        }

        /**
         * \brief Sets volumetric mode.
         * \param[in] x if true, volumetric mode is used, otherwise
         *  surfacic mode is used.
         */
        void set_volumetric(bool x) {
            RVD_->set_volumetric(x);
        }

        /**
         * \brief Tests whether a point is locked.
         * \details A locked point is constrained to stay at the same position
         *    during the optimization.
         * \param[in] i index of the point
         * \pre i < nb_points()
         */
        bool point_is_locked(index_t i) const {
            geo_debug_assert(
                point_is_locked_.size() == 0 || i < point_is_locked_.size()
            );
            return point_is_locked_.size() != 0 && point_is_locked_[i];
        }

        /**
         * \brief Locks a point.
         * \details A locked point is constrained to stay at the same position
         *    during the optimization.
         * \param[in] i index of the point
         * \pre i < nb_points()
         */
        void lock_point(index_t i) {
            geo_debug_assert(i < nb_points());
            if(point_is_locked_.size() != nb_points()) {
                point_is_locked_.resize(nb_points(), false);
            }
            point_is_locked_[i] = true;
        }

        /**
         * \brief Unlocks a point.
         * \details A locked point is constrained to stay at the same position
         *    during the optimization.
         * \param[in] i index of the point
         * \pre i < nb_points()
         */
        void unlock_point(index_t i) {
            geo_debug_assert(i < nb_points());
            if(
                point_is_locked_.size() != nb_points()
            ) {
                point_is_locked_.resize(nb_points(), false);
            }
            point_is_locked_[i] = false;
        }

        /**
         * \brief Unlocks all the points.
         * \details A locked point is constrained to stay at the same position
         *    during the optimization.
         */
        void unlock_all_points() {
            point_is_locked_.clear();
        }

    protected:
        /**
         * \brief Callback for the numerical solver.
         * \details Updates the progress bar.
         */
        virtual void newiteration();

        /**
         * \brief Computes the objective function and its gradient.
         * \param[in] n number of variables
         * \param[in] x current value of the variables
         * \param[out] f current value of the objective function
         * \param[out] g gradient of the objective function
         */
        virtual void funcgrad(index_t n, double* x, double& f, double* g);

        /**
         * \brief Constrains the locked points.
         * \details Zeroes the gradient relative to the components
         *  of locked points.
         * \param[in,out] g gradient of the objective function
         */
        void constrain_points(double* g) const;

        /**
         * \brief Computes the 3d representation of the Nd points.
         * \details It projects the points onto the Nd surface, then recovers
         *  the 3d coordinates by barycentric interpolation.
         */
        void compute_R3_embedding();

        static CentroidalVoronoiTesselation* instance_;
        bool show_iterations_;
        coord_index_t dimension_;
        Delaunay_var delaunay_;
        RestrictedVoronoiDiagram_var RVD_;
        Mesh* mesh_;

        vector<double> points_;
        vector<vec3> points_R3_;
        vector<bool> point_is_locked_;

        ProgressTask* progress_;
        index_t cur_iter_;
        index_t nb_iter_;

        bool is_projection_;   /**< the Nd -> 3d transform is a projection */
        bool constrained_cvt_;
        bool use_RVC_centroids_;

        IntegrationSimplex_var simplex_func_;
          /**< \brief Integration simplex used by custom codes, e.g. LpCVT */

    private:
        /** \brief Forbids construction by copy. */
        CentroidalVoronoiTesselation(const thisclass& rhs);

        /** \brief Forbids assignment. */
        thisclass& operator= (const thisclass& rhs);
    };
}

#endif

