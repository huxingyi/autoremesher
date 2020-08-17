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

#include <geogram/mesh/mesh_decimate.h>
#include <geogram/mesh/mesh_repair.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/mesh_degree3_vertices.h>
#include <geogram/points/colocate.h>
#include <geogram/basic/stopwatch.h>
#include <geogram/basic/algorithm.h>

namespace GEO {

    void mesh_decimate_vertex_clustering(
        Mesh& M, index_t nb_bins, MeshDecimateMode mode,
        geo_index_t* vertices_flags
    ) {
        Stopwatch W("Decimate");
        double xyz_min[3];
        double xyz_max[3];
        get_bbox(M, xyz_min, xyz_max);
        double R = ::sqrt(
            geo_sqr(xyz_max[0] - xyz_min[0]) +
            geo_sqr(xyz_max[1] - xyz_min[1]) +
            geo_sqr(xyz_max[2] - xyz_min[2])
        );
        double h = R / double(nb_bins);

        std::vector<bool> is_required;
        if(mode & MESH_DECIMATE_KEEP_B) {
            is_required.assign(M.vertices.nb(), false);
            for(index_t f : M.facets) {
                for(index_t c : M.facets.corners(f)) {
                    if(M.facet_corners.adjacent_facet(c) == NO_FACET) {
                        is_required[M.facet_corners.vertex(c)] = true;
                    }
                }
            }
        }

        if(vertices_flags != nullptr) {
            for(index_t v: M.vertices) {
                if(vertices_flags[v] != 0) {
                    is_required[v] = true;
                }
            }
        }

        vector<double> new_points(M.vertices.nb() * 3);
        for(index_t v: M.vertices) {
            if(is_required.size() != 0 && is_required[v]) {
                double* p = M.vertices.point_ptr(v);
                for(coord_index_t c = 0; c < 3; ++c) {
                    new_points[M.vertices.dimension() * v + c] = p[c];
                }
            } else {
                double* p = M.vertices.point_ptr(v);
                for(coord_index_t c = 0; c < 3; ++c) {
                    double d = p[c] - xyz_min[c];
                    d = double(index_t(d / h)) * h;
                    new_points[M.vertices.dimension() * v + c] = xyz_min[c] + d;
                }
            }
        }

        vector<index_t> old2new;
        index_t nb_new_vertices = Geom::colocate_by_lexico_sort(
            new_points.data(), 3, M.vertices.nb(), old2new, 3
        );

        if(nb_new_vertices == M.vertices.nb()) {
            Logger::warn("Decimate") << "Did not remove any vertex"
                << std::endl;
            return;
        }

        Logger::out("Decimate") << "Removed "
            << M.vertices.nb() - nb_new_vertices
            << " vertices" << std::endl;

        for(index_t c: M.facet_corners) {
            M.facet_corners.set_vertex(c, old2new[M.facet_corners.vertex(c)]);
        }

        // Determine new vertices positions
        new_points.assign(M.vertices.dimension() * M.vertices.nb(), 0.0);
        vector<index_t> new_points_count(M.vertices.nb(), 0);

        for(index_t v: M.vertices) {
            index_t w = old2new[v];
            for(coord_index_t c = 0; c < M.vertices.dimension(); ++c) {
                new_points[w * M.vertices.dimension() + c] +=
                    M.vertices.point_ptr(v)[c];
            }
            new_points_count[w]++;
        }

        for(index_t w: M.vertices) {
            double s = double(new_points_count[w]);
            if(s != 0.0) {
                s = 1.0 / s;
            }
            for(coord_index_t c = 0; c < M.vertices.dimension(); ++c) {
                new_points[w * M.vertices.dimension() + c] *= s;
            }
        }

        M.vertices.assign_points(new_points, M.vertices.dimension(), true);

        // Now old2new is "recycled" for marking vertices that
        // need to be removed.
        for(index_t i = 0; i < old2new.size(); i++) {
            if(old2new[i] == i) {
                old2new[i] = 0;
            } else {
                old2new[i] = 1;
            }
        }

        M.vertices.delete_elements(old2new);
        if(mode & MESH_DECIMATE_DUP_F) {
            mesh_repair(M, MESH_REPAIR_DUP_F);
        } else {
            // Only remove facets with duplicated vertices.
            mesh_repair(M, MeshRepairMode(0));
        }

        if(mode & MESH_DECIMATE_DEG_3) {
            double max_dist = 0.001 * R;
            while(remove_degree3_vertices(M, max_dist) != 0) {}
        }
    }
}

