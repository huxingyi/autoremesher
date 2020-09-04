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

#include <geogram/voronoi/CVT.h>
#include <geogram/voronoi/RVD.h>
#include <geogram/mesh/mesh_repair.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/numerics/optimizer.h>
#include <geogram/basic/progress.h>
#include <geogram/basic/argused.h>
#include <geogram/bibliography/bibliography.h>

/****************************************************************************/

namespace GEO {

    CentroidalVoronoiTesselation*
    CentroidalVoronoiTesselation::instance_ = nullptr;

    CentroidalVoronoiTesselation::CentroidalVoronoiTesselation(
        Mesh* mesh, coord_index_t dim, const std::string& delaunay
    ) {
        use_RVC_centroids_ = true;
        show_iterations_ = false;
        constrained_cvt_ = false;
        dimension_ =
            (dim != 0) ? dim : coord_index_t(mesh->vertices.dimension());
        geo_assert(index_t(dimension_) <= mesh->vertices.dimension());
        is_projection_ = true;
        delaunay_ = Delaunay::create(dimension_, delaunay);
        RVD_ = RestrictedVoronoiDiagram::create(delaunay_, mesh);
        mesh_ = mesh;
        geo_assert(instance_ == nullptr);
        instance_ = this;
        progress_ = nullptr;
	geo_cite("Lloyd82leastsquares");
	geo_cite("Du:1999:CVT:340312.340319");
	geo_cite("DBLP:journals/tog/LiuWLSYLY09");
    }

    CentroidalVoronoiTesselation::CentroidalVoronoiTesselation(
        Mesh* mesh, const vector<vec3>& R3_embedding, coord_index_t dim,
        const std::string& delaunay
    ) {
        use_RVC_centroids_ = true;
        show_iterations_ = false;
        constrained_cvt_ = false;
        dimension_ =
            (dim != 0) ? dim : coord_index_t(mesh->vertices.dimension());
        geo_assert(index_t(dimension_) <= mesh->vertices.dimension());
        is_projection_ = (R3_embedding.size() == 0);
        delaunay_ = Delaunay::create(dimension_, delaunay);
        if(is_projection_) {
            RVD_ = RestrictedVoronoiDiagram::create(delaunay_, mesh);
        } else {
            RVD_ = RestrictedVoronoiDiagram::create(
                delaunay_, mesh, R3_embedding
            );
        }
        mesh_ = mesh;
        geo_assert(instance_ == nullptr);
        instance_ = this;
        progress_ = nullptr;
	geo_cite("Lloyd82leastsquares");
	geo_cite("Du:1999:CVT:340312.340319");
	geo_cite("DBLP:journals/tog/LiuWLSYLY09");
    }

    CentroidalVoronoiTesselation::~CentroidalVoronoiTesselation() {
        instance_ = nullptr;
    }

    bool CentroidalVoronoiTesselation::compute_initial_sampling(
        index_t nb_samples
    ) {
        points_.resize(dimension_ * nb_samples);
        return RVD_->compute_initial_sampling(
            points_.data(), nb_samples
        );
    }

    void CentroidalVoronoiTesselation::set_points(
        index_t nb_points, const double* points
    ) {
        points_.resize(dimension_ * nb_points);
        for(index_t i = 0; i < points_.size(); i++) {
            points_[i] = points[i];
        }
    }

    void CentroidalVoronoiTesselation::resize_points(
        index_t nb_points
    ) {
        points_.resize(dimension_ * nb_points);
    }

    void CentroidalVoronoiTesselation::Lloyd_iterations(index_t nb_iter) {
        index_t nb_points = index_t(points_.size() / dimension_);

        vector<double> mg;
        vector<double> m;

        RVD_->set_check_SR(false);

        if(progress_ != nullptr) {
            progress_->reset(nb_iter);
        }

        cur_iter_ = 0;
        nb_iter_ = nb_iter;

        for(index_t i = 0; i < nb_iter; i++) {
            mg.assign(nb_points * dimension_, 0.0);
            m.assign(nb_points, 0.0);
            delaunay_->set_vertices(nb_points, points_.data());
            RVD_->compute_centroids(mg.data(), m.data());
            index_t cur = 0;
            for(index_t j = 0; j < nb_points; j++) {
                if(m[j] > 1e-30 && !point_is_locked(j)) {
                    double s = 1.0 / m[j];
                    for(index_t coord = 0; coord < dimension_; coord++) {
                        points_[cur + coord] = s * mg[cur + coord];
                    }
                }
                cur += dimension_;
            }
            newiteration();
        }

        progress_ = nullptr;
    }

    void CentroidalVoronoiTesselation::compute_surface(
        Mesh* mesh, bool multinerve
    ) {
        index_t nb_points = index_t(points_.size() / dimension_);
        delaunay_->set_vertices(nb_points, points_.data());

        vector<index_t> triangles;
        vector<double> vertices;
        vector<double> vertices_R3;

        RestrictedVoronoiDiagram::RDTMode mode =
            RestrictedVoronoiDiagram::RDTMode(0);

        if(multinerve) {
            mode = RestrictedVoronoiDiagram::RDTMode(
                mode | RestrictedVoronoiDiagram::RDT_MULTINERVE
            );
        }
        if(use_RVC_centroids_) {
            mode = RestrictedVoronoiDiagram::RDTMode(
                mode | RestrictedVoronoiDiagram::RDT_RVC_CENTROIDS
                     | RestrictedVoronoiDiagram::RDT_PREFER_SEEDS
            );
        }

        RVD_->set_check_SR(true);
        RVD_->compute_RDT(
            triangles, vertices, mode, point_is_locked_
        );

        // TODO: projection is not good when the embedding is not one-to-one,
        // for instance with Gauss map. We should use barycentric coordinates
        // instead.
        index_t nb_vertices = index_t(vertices.size() / dimension_);
        vertices_R3.resize(nb_vertices * 3);
        if(is_projection_) {
            double* cur = vertices.data();
            for(index_t v = 0; v < nb_vertices; v++) {
                vertices_R3[3 * v] = cur[0];
                vertices_R3[3 * v + 1] = cur[1];
                vertices_R3[3 * v + 2] = cur[2];
                cur += dimension_;
            }
        } else {
            RVD_->project_points_on_surface(
                nb_vertices, vertices.data(), vertices_R3
            );
        }

        mesh->clear();
        mesh->facets.assign_triangle_mesh(3, vertices_R3, triangles, true);

        if(multinerve) {
            mesh_postprocess_RDT(*mesh);
        } else {
            // The 'repair' phase is needed to reconstruct the
            // facet-facet links, that are not initialized by
            // Mesh::assign_triangle_mesh()
            double radius = bbox_diagonal(*mesh);
            mesh_repair(*mesh, MESH_REPAIR_DEFAULT, 1e-6 * radius);
            // TODO: check: is it really good to have some tolerance here,
            //  not sure, may cause some Moebius configs sometimes.
        }
    }

    void CentroidalVoronoiTesselation::compute_volume(
        Mesh* mesh
    ) {
        geo_assert(volumetric());
        index_t nb_points = index_t(points_.size() / dimension_);
        delaunay_->set_vertices(nb_points, points_.data());

        vector<index_t> tets;
        vector<double> vertices;
        vector<double> vertices_R3;

        RVD_->set_check_SR(true);
        RVD_->compute_RDT(
            tets, vertices
        );

        // TODO: projection is not good when the embedding is not one-to-one,
        // for instance with Gauss map. We should use barycentric coordinates
        // instead.
        index_t nb_vertices = index_t(vertices.size() / dimension_);
        vertices_R3.resize(nb_vertices * 3);
        if(is_projection_) {
            double* cur = vertices.data();
            for(index_t v = 0; v < nb_vertices; v++) {
                vertices_R3[3 * v] = cur[0];
                vertices_R3[3 * v + 1] = cur[1];
                vertices_R3[3 * v + 2] = cur[2];
                cur += dimension_;
            }
        } else {
            // TODO: map vertices from embedding space to 3D space
            // when we are not in projection mode
            geo_assert_not_reached;
        }
        mesh->clear();
        mesh->cells.assign_tet_mesh(3, vertices_R3, tets, true);
    }

    void CentroidalVoronoiTesselation::Newton_iterations(
        index_t nb_iter, index_t m
    ) {
        Optimizer_var optimizer = Optimizer::create("HLBFGS");
	if(optimizer.is_null()) {
	    Logger::warn("CVT") << "This geogram was not compiled with HLBFGS"
				<< " (falling back to Lloyd iterations)"
				<< std::endl;
	    Lloyd_iterations(nb_iter);
	    return;
	}

        index_t n = index_t(points_.size());

        RVD_->set_check_SR(true);

        if(progress_ != nullptr) {
            progress_->reset(nb_iter);
        }

        cur_iter_ = 0;
        nb_iter_ = nb_iter;

        optimizer->set_epsg(0.0);
        optimizer->set_epsf(0.0);
        optimizer->set_epsx(0.0);
        optimizer->set_newiteration_callback(newiteration_CB);
        optimizer->set_funcgrad_callback(funcgrad_CB);
        optimizer->set_N(n);
        optimizer->set_M(m);
        optimizer->set_max_iter(nb_iter);
        optimizer->optimize(points_.data());

        simplex_func_.reset();
        progress_ = nullptr;
    }

    void CentroidalVoronoiTesselation::constrain_points(double* g) const {
        if(point_is_locked_.size() != 0) {
            double* cur_g = g;
            for(index_t i = 0; i < nb_points(); ++i) {
                if(point_is_locked_[i]) {
                    for(index_t c = 0; c < dimension_; c++) {
                        cur_g[c] = 0.0;
                    }
                }
                cur_g += dimension_;
            }
        }
    }

    void CentroidalVoronoiTesselation::funcgrad(
        index_t n, double* x, double& f, double* g
    ) {
        index_t nb_points = n / dimension_;
        delaunay_->set_vertices(nb_points, x);
        Memory::clear(g, n * sizeof(double));
        f = 0.0;
        if(!simplex_func_.is_null()) {
            RVD_->compute_integration_simplex_func_grad(
                f,g,simplex_func_
            );
        } else {
            RVD_->compute_CVT_func_grad(f, g);
        }
        constrain_points(g);
    }

    void CentroidalVoronoiTesselation::newiteration() {
        if(progress_ != nullptr) {
            progress_->next();
        }
        cur_iter_++;
    }

    void CentroidalVoronoiTesselation::funcgrad_CB(
        index_t n, double* x, double& f, double* g
    ) {
        instance_->funcgrad(n, x, f, g);
    }

    void CentroidalVoronoiTesselation::newiteration_CB(
        index_t n, const double* x, double f, const double* g, double gnorm
    ) {
        geo_argused(n);
        geo_argused(x);
        geo_argused(f);
        geo_argused(g);
        geo_argused(gnorm);
        instance_->newiteration();
    }

    void CentroidalVoronoiTesselation::compute_R3_embedding() {
        index_t nb_points = index_t(points_.size() / dimension_);
        points_R3_.resize(nb_points);
        if(is_projection_ && !constrained_cvt_) {
            double* cur = points_.data();
            for(index_t p = 0; p < nb_points; p++) {
                points_R3_[p] = vec3(cur[0], cur[1], cur[2]);
                cur += dimension_;
            }
        } else {
            RVD_->project_points_on_surface(
                nb_points, points_.data(), points_R3_, constrained_cvt_
            );
        }
    }
}

