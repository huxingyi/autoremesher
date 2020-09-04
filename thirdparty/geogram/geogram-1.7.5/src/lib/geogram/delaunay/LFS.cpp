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

#include <geogram/delaunay/LFS.h>
#include <geogram/basic/geometry.h>
#include <geogram/basic/logger.h>
#include <geogram/basic/command_line.h>

namespace {

    using namespace GEO;

    const index_t pid_size = 6;
    int pid[pid_size][4] = {
        {0, 1, 2, 3},
        {0, 2, 1, 3},
        {0, 3, 1, 2},
        {1, 2, 0, 3},
        {1, 3, 2, 0},
        {2, 3, 0, 1}
    };

    /**
     * \brief Computes the circumcenter and squared radius of a tetrahedron.
     * \param[in] p first vertex of the tetrahedron
     * \param[in] q second vertex of the tetrahedron
     * \param[in] r third vertex of the tetrahedron
     * \param[in] s fourth vertex of the tetrahedron
     * \param[out] t_circumcenter computed circumcenter of the tetrahedron
     * \param[out] squared_radius computed squared radius of the tetrahedron
     * \param[out] dihedral_angles if non-nullptr, 
     *   computed dihedral angles of the tetrahedron
     * \return true if successful, false otherwise 
     *  (for instance, if the tetrahedron is flat).
     */
    bool tetra_circumcenter_squaredradius(
        const vec3& p, const vec3& q,
        const vec3& r, const vec3& s,
        vec3& t_circumcenter, double& squared_radius,
        double* dihedral_angles = nullptr
    ) {

        vec3 qp = q - p;
        vec3 rp = r - p;
        vec3 sp = s - p;

        double tet_vol_x_12 = 2.0 * dot(qp, cross(rp, sp));

        // it is not a safe check, one should avoid
        // to input a degenerated tetra.
        if(tet_vol_x_12 == 0.0) {
            return false;
        }

        vec3 d_t_circumcenter =
            length2(qp) * cross(rp, sp) +
            length2(rp) * cross(sp, qp) +
            length2(sp) * cross(qp, rp);

        squared_radius = length2(
            d_t_circumcenter) / (tet_vol_x_12 * tet_vol_x_12
            );
        d_t_circumcenter = (1.0 / tet_vol_x_12) * d_t_circumcenter;
        t_circumcenter = p + d_t_circumcenter;

        if(dihedral_angles) {
            // compute Dihedral angle directly
            // the following computation is not optimized.
            vec3 point[4];
            point[0] = p;
            point[1] = q;
            point[2] = r;
            point[3] = s;
            for(index_t i = 0; i < pid_size; i++) {
                vec3 b1 = point[pid[i][0]] - point[pid[i][2]];
                vec3 b2 = point[pid[i][1]] - point[pid[i][0]];
                vec3 b3 = point[pid[i][3]] - point[pid[i][1]];
                vec3 b2b3 = cross(b2, b3);
                dihedral_angles[i] = ::fabs(
                    ::atan2(
                        length(b2) * dot(b1, b2b3),
                        dot(cross(b1, b2), b2b3)
                    )
                );
            }
        }
        return true;
    }

    /**
     * \brief Gets a Delaunay vertex by global index.
     * \param[in] delaunay the Delaunay triangulation
     * \param[in] v the index of the vertex
     * \return a const reference to the vertex, as a vec3
     */
    inline const vec3& delaunay_vertex(Delaunay* delaunay, index_t v) {
        return *(const vec3*) delaunay->vertex_ptr(v);
    }

    /**
     * \brief Gets a Delaunay vertex by tetrahedron index 
     *  and local vertex index.
     * \param[in] delaunay the Delaunay triangulation
     * \param[in] c the index of the tetrahedron
     * \param[in] lv the local index of the vertex (0,1,2 or 3) 
     *  in tetrahedron \p c.
     * \return a const reference to the vertex, as a vec3.
     */
    inline const vec3& delaunay_tet_vertex(
        Delaunay* delaunay, index_t c, index_t lv
    ) {
        return delaunay_vertex(delaunay, index_t(delaunay->cell_vertex(c, lv)));
    }

    index_t tet_facet_vertex[4][3] = {
        {1, 2, 3},
        {0, 3, 2},
        {3, 0, 1},
        {2, 1, 0}
    };

    /**
     * \brief Computes the normal to a tetrahedron facet.
     * \param[in] delaunay the Delaunay triangulation
     * \param[in] t the index of the tetrahedron
     * \param[in] f the local index (0,1,2 or 3) of 
     *  the facet in the tetrahedron \p t
     * \return the normal to the facet \p f or tetrahedron \p t
     */
    vec3 delaunay_facet_normal(
        Delaunay* delaunay, index_t t, index_t f
    ) {
        geo_debug_assert(f < 4);
        index_t v1 = index_t(delaunay->cell_vertex(t, tet_facet_vertex[f][0]));
        index_t v2 = index_t(delaunay->cell_vertex(t, tet_facet_vertex[f][1]));
        index_t v3 = index_t(delaunay->cell_vertex(t, tet_facet_vertex[f][2]));
        const vec3& p1 = delaunay_vertex(delaunay, v1);
        const vec3& p2 = delaunay_vertex(delaunay, v2);
        const vec3& p3 = delaunay_vertex(delaunay, v3);
        return cross(p2 - p1, p3 - p1);
    }
}

/****************************************************************************/

namespace GEO {

    void LocalFeatureSize::init(index_t nb_pts, const double* pts) {
        // Note: I need PDEL here instead of ANN/BNN since I need
        //  to access the cells (and ANN/BNN only give me the
        //  neighbors)

        Logger::out("LFS") << "Delaunay" << std::endl;
        Delaunay_var delaunay = Delaunay::create(3, "PDEL");
        delaunay->set_vertices(nb_pts, pts);
        Logger::out("LFS") << "Done Delaunay" << std::endl;
        
        vector<vec3> circumcenter(delaunay->nb_cells());
        vector<bool> voronoi_cell_is_infinite(
            delaunay->nb_vertices(), false
        );
        vector<bool> is_sliver(delaunay->nb_cells());

        static const index_t NO_POLE = index_t(-1);
        // Index of incident tet whose circumcenter is the pole
        vector<index_t> positive_pole(delaunay->nb_vertices(),NO_POLE);
        vector<index_t> negative_pole(delaunay->nb_vertices(),NO_POLE);

        vector<vec3> avg_infinite_dir(
            delaunay->nb_vertices(), vec3(0, 0, 0)
        );
        vector<double> dist(delaunay->nb_vertices(), 0.0);

        Logger::out("LFS") << "(1) Circumcenters and slivers" << std::endl; 
        // Step 1: compute circumcenters and check for slivers
        const double sliver_quality = sliver_angle_threshold_ / 180.0 * M_PI;
        for(index_t t = 0; t < delaunay->nb_cells(); t++) {
            const vec3& p = delaunay_tet_vertex(delaunay, t, 0);
            const vec3& q = delaunay_tet_vertex(delaunay, t, 1);
            const vec3& r = delaunay_tet_vertex(delaunay, t, 2);
            const vec3& s = delaunay_tet_vertex(delaunay, t, 3);

            double dihedral_angle[6];
            double tet_radius;
            bool ok = tetra_circumcenter_squaredradius(
                p, q, r, s,
                circumcenter[t], tet_radius, dihedral_angle
            );

            for(index_t a = 0; a < 6; a++) {
                if(!ok) {
                    break;
                }
                ok = ok &&
                    dihedral_angle[a] >= sliver_quality &&
                    dihedral_angle[a] <= M_PI - sliver_quality;
            }
            is_sliver[t] = !ok;
        }

        Logger::out("LFS") << "(2) Positive poles" << std::endl;
        // Step 2: compute positive poles
        for(index_t t = 0; t < delaunay->nb_cells(); t++) {
            if(is_sliver[t]) {
                continue;
            }
            signed_index_t f_inf = -1;
            for(index_t f = 0; f < 4; f++) {
                if(delaunay->cell_adjacent(t, f) == -1) {
                    f_inf = signed_index_t(f);
                    break;
                }
            }
            if(f_inf == -1) {
                // tet t does not have facet on border
                const vec3& c = circumcenter[t];
                for(index_t lv = 0; lv < 4; lv++) {
                    index_t iv = index_t(delaunay->cell_vertex(t, lv));
                    const vec3& v = delaunay_vertex(delaunay, iv);
                    double d = length2(c - v);
                    if(d > dist[iv]) {
                        dist[iv] = d;
                        positive_pole[iv] = t;
                    }
                }
            } else {
                // tet t has facet f_inf on border
                // positive pole = average direction of infinite Voronoi edges
                index_t v = index_t(delaunay->cell_vertex(t, index_t(f_inf)));
                voronoi_cell_is_infinite[v] = true;
                avg_infinite_dir[v] +=
                    delaunay_facet_normal(delaunay, t, index_t(f_inf));
            }
        }

        Logger::out("LFS") << "(3) Negative poles" << std::endl;        
        // Step 3: compute negative poles
        std::fill(dist.begin(), dist.end(), 0.0);
        for(index_t t = 0; t < delaunay->nb_cells(); t++) {
            if(is_sliver[t]) {
                continue;
            }
            bool t_is_infinite = false;
            for(index_t f = 0; f < 4; f++) {
                if(delaunay->cell_adjacent(t, f) == -1) {
                    t_is_infinite = true;
                    break;
                }
            }
            if(t_is_infinite) {
                continue;
            }
            const vec3& c = circumcenter[t];
            for(index_t lv = 0; lv < 4; lv++) {
                index_t iv = index_t(delaunay->cell_vertex(t, lv));
                const vec3& v = delaunay_vertex(delaunay, iv);
                vec3 N;
                if(voronoi_cell_is_infinite[iv]) {
                    N = -avg_infinite_dir[iv];
                } else {
                    N = v - circumcenter[positive_pole[iv]];
                }
                double d = dot(c - v, N);
                if(d > dist[iv]) {
                    dist[iv] = d;
                    negative_pole[iv] = t;
                }
            }
        }

        Logger::out("LFS") << "(4) Kd-tree" << std::endl;
        // Step 4: create search structure
        vector<bool> is_pole(delaunay->nb_cells(), false);
        for(index_t iv = 0; iv < delaunay->nb_vertices(); iv++) {
            if(negative_pole[iv] != NO_POLE) {
                is_pole[index_t(negative_pole[iv])] = true;
            }
            if(!voronoi_cell_is_infinite[iv]) {
                if(positive_pole[iv] != NO_POLE) {
                    is_pole[index_t(positive_pole[iv])] = true;
                }
            }
        }

        index_t nb_poles = 0;
        for(index_t t = 0; t < delaunay->nb_cells(); t++) {
            if(is_pole[t]) {
                nb_poles++;
            }
        }

        poles_.reserve(nb_poles * 3);
        for(index_t t = 0; t < delaunay->nb_cells(); t++) {
            if(is_pole[t]) {
                poles_.push_back(circumcenter[t].x);
                poles_.push_back(circumcenter[t].y);
                poles_.push_back(circumcenter[t].z);
            }
        }

        spatial_search_ = Delaunay::create(3, "NN");
        spatial_search_->set_vertices(poles_.size() / 3, poles_.data());

        Logger::out("LFS") << "Done init." << std::endl;        
    }
}

