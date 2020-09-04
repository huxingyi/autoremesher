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

#include <geogram/mesh/mesh_fill_holes.h>
#include <geogram/mesh/mesh.h>
#include <geogram/mesh/mesh_repair.h>
#include <geogram/mesh/mesh_halfedges.h>
#include <geogram/mesh/mesh_io.h>
#include <geogram/mesh/index.h>
#include <geogram/basic/command_line.h>
#include <geogram/basic/logger.h>

#undef geo_debug_assert
#define geo_debug_assert(x) geo_assert(x)

namespace {

    using namespace GEO;

    /**
     * \brief Checks whether a halfedge is
     *  incident to a vertex.
     * \details Checks whether the origin of \p H
     * is adjacent to \p v. In other words,
     * returns true in one of the following
     * configurations (the origin of \p H is
     * denoted by \p x).
     * \code
     *
     *           H
     *    v--->x--->*
     *
     *           H
     *    *--->x--->v
     *
     * \endcode
     * \param[in] MH the mesh, wrapped with halfedge accessors
     * \param[in] H the halfedge
     * \param[in] v the index of the vertex
     */
    bool halfedge_has_neighbor(
        const MeshHalfedges& MH,
        const MeshHalfedges::Halfedge& H,
        index_t v
    ) {

        geo_debug_assert(MH.halfedge_is_valid(H));

        const Mesh& M = MH.mesh();
        index_t f = H.facet;
        index_t c = H.corner;

        {
            index_t cnext = M.facets.next_corner_around_facet(f, c);
            if(M.facet_corners.vertex(cnext) == v) {
                return true;
            }
        }

        {
            index_t cprev = M.facets.prev_corner_around_facet(f, c);
            if(M.facet_corners.vertex(cprev) == v) {
                return true;
            }
        }

        return false;
    }

    /**
     * \brief Checks whether an halfedge exists between
     * the two origins of two halfedges.
     * \param[in] MH the mesh, wrapped with halfedge accessors
     * \param[in] h1 first halfedge
     * \param[in] h2 second halfedge
     * \return true if an halfedge exists between the origins
     *  of \p h1 and \p h2, false otherwise
     */
    bool halfedge_exists_between_vertices(
        const MeshHalfedges& MH,
        const MeshHalfedges::Halfedge& h1,
        const MeshHalfedges::Halfedge& h2
    ) {
        index_t v2 = MH.mesh().facet_corners.vertex(h2.corner);
        MeshHalfedges::Halfedge H = h1;
        do {
            if(halfedge_has_neighbor(MH, H, v2)) {
                return true;
            }
            if(!MH.move_to_prev_around_vertex(H)) {
                break;
            }
        } while(H != h1);
        return false;
    }

    /**
     * \brief Gets the 3d vertex at the origin of a halfedge.
     * \param[in] MH the mesh, wrapped with halfedge accessors
     * \param[in] H the halfedge
     * \return a const reference to the geometry of the 3d vertex at the
     *  origin of \p H
     */
    inline const vec3& halfedge_vertex(
        const MeshHalfedges& MH, const MeshHalfedges::Halfedge& H
    ) {
        return Geom::mesh_vertex(
            MH.mesh(), MH.mesh().facet_corners.vertex(H.corner)
        );
    }

    /**
     * \brief Internal representation of a Hole.
     * \details A Hole is an ordered sequence of Halfedge.
     */
    typedef vector<MeshHalfedges::Halfedge> Hole;

    /**
     * \brief Computes a vector orthogonal to the border of a surface and in the
     *  tangent plane of the surface.
     * \param[in] MH the mesh, wrapped with halfedge accessors
     * \param[in] H the halfedge
     * \return a 3d vector orthogonal to \p H, in the plange of the 
     *  surface triangle incident to \p H and pointing towards the 
     *  exterior of the surface.
     */
    vec3 border_normal(
        const MeshHalfedges& MH,
        const MeshHalfedges::Halfedge& H
    ) {
        const Mesh& M = MH.mesh();
        index_t c = H.corner;
        index_t f = H.facet;
        index_t v1 = M.facet_corners.vertex(c);
        c = M.facets.next_corner_around_facet(f, c);
        index_t v2 = M.facet_corners.vertex(c);
        vec3 E = Geom::mesh_vertex(M, v2) - Geom::mesh_vertex(M, v1);
        vec3 N = Geom::mesh_facet_normal(M, f);
        return cross(E, N);
    }

    /**
     * \brief Splits a hole into two.
     * \details This function is used recursively to triangulate the holes.
     * \param[in] MH the mesh, wrapped with halfedge accessors
     * \param[in] hole the hole to be split
     * \param[out] hole1 one of the computed halves
     * \param[out] hole2 the other half
     * \param[in] use_normals if set, then couples of vertices
     * (v1,v2) that have their border normals that match
     * the new segment [v1,v2], are connected in priority.
     * This improves the result on 788_raw_hand.off (but
     * may cause some triangles overlap)
     * \retval true on success
     * \retval false otherwise (for instance, if no valid edge
     *  could be found to split the hole)
     */
    bool split_hole(
        const MeshHalfedges& MH, const Hole& hole,
        Hole& hole1, Hole& hole2, bool use_normals
    ) {

        // Step 0: compute normals to border
        vector<vec3> N;
        if(use_normals) {
            N.assign(hole.size(), vec3(0.0, 0.0, 0.0));
            for(index_t i = 0; i < hole.size(); i++) {
                index_t j = i + 1;
                if(j == hole.size()) {
                    j = 0;
                }
                vec3 n = border_normal(MH, hole[i]);
                N[i] += n;
                N[j] += n;
            }
            for(index_t i = 0; i < N.size(); i++) {
                N[i] = normalize(N[i]);
            }
        }

        // Step 1: compute total curve length and curvilinear abscissa
        vector<double> s(hole.size());
        double cur_s = 0.0;
        s[0] = cur_s;
        for(index_t i = 1; i < hole.size(); i++) {
            const vec3& p1 = halfedge_vertex(MH, hole[i - 1]);
            const vec3& p2 = halfedge_vertex(MH, hole[i]);
            cur_s += length(p2 - p1);
            s[i] = cur_s;
        }
        const vec3& p1 = halfedge_vertex(MH, hole[hole.size() - 1]);
        const vec3& p2 = halfedge_vertex(MH, hole[0]);
        double total_length = cur_s + length(p2 - p1);

        // Step 2: find best pair to connect
        double best_rij = Numeric::max_float64();
        signed_index_t v1 = -1;
        signed_index_t v2 = -1;
        for(index_t i = 0; i < hole.size(); i++) {
            for(index_t j = i + 2; j < hole.size(); j++) {

                // Do not split using vertices
                // already connected by an edge.
                if(
                    (i == 0 && j == hole.size() - 1) ||
                    halfedge_exists_between_vertices(MH, hole[i], hole[j])
                ) {
                    continue;
                }

                double dsij = std::min(
                    s[j] - s[i], total_length - (s[j] - s[i])
                );
                const vec3& pi = halfedge_vertex(MH, hole[i]);
                const vec3& pj = halfedge_vertex(MH, hole[j]);
                double dxij = length(pj - pi);

                dsij = std::max(dsij, 1e-6);
                dxij = std::max(dxij, 1e-6);
                double rij = dxij / dsij;

                if(use_normals) {
                    const vec3& Pi = halfedge_vertex(MH, hole[i]);
                    const vec3& Pj = halfedge_vertex(MH, hole[j]);
                    vec3 Dij = normalize(Pj - Pi);

                    // between -1 (worse) and 1 (best)
                    double angle_factor =
                        0.5 * (dot(Dij, N[i]) - dot(Dij, N[j]));

                    // between 0 (best) and 1 (worse)
                    angle_factor = 0.5 * (1.0 - angle_factor);

                    rij *= angle_factor;
                }

                if(rij < best_rij) {
                    best_rij = rij;
                    v1 = signed_index_t(i);
                    v2 = signed_index_t(j);
                }
            }
        }

        if(v1 == -1 || v2 == -1) {
            return false;
        }

        // Now I do not think this can happen
        // (to be checked)
        if(v2 < v1) {
	    std::swap(v1, v2);
        }

        // Step 3: copy the two "sub-holes"
        hole1.clear();
        hole2.clear();
        for(signed_index_t i = 0; i < signed_index_t(hole.size()); i++) {
            if(i <= v1 || i >= v2) {
                hole1.push_back(hole[i]);
            }
            if(i >= v1 && i <= v2) {
                hole2.push_back(hole[i]);
            }
        }

        return true;
    }

    /**
     * \brief Triangulates a hole.
     * \param[in] MH the mesh, wrapped with halfedge accessors
     * \param[in] hole the hole, represented by a vector of halfedges
     * \param[out] triangles the generated triangles
     * \param[in] use_normals if set, then couples of vertices
     * (v1,v2) that have their border normals that match
     * the new segment [v1,v2], are connected in priority.
     * This improves the result on 788_raw_hand.off (but
     * may cause some triangles overlap)
     * \param[in] clear if set, \p triangles is cleared
     * \retval true on success
     * \retval false otherwise (for instance, if no valid edge
     *  could be found to split the hole)
     */
    bool triangulate_hole_loop_splitting(
        const MeshHalfedges& MH, const Hole& hole,
        vector<trindex>& triangles, bool use_normals,
        bool clear = true
    ) {
        bool ok = true;
        if(clear) {
            triangles.clear();
        }
        if(hole.size() <= 3) {
            if(hole.size() == 3) {
                trindex T(
                    MH.mesh().facet_corners.vertex(hole[0].corner),
                    MH.mesh().facet_corners.vertex(hole[1].corner),
                    MH.mesh().facet_corners.vertex(hole[2].corner),
                    trindex::KEEP_ORDER
                );
                triangles.push_back(T);
            }
        } else {
            Hole hole1, hole2;
            ok = split_hole(MH, hole, hole1, hole2, use_normals);
            ok = ok && triangulate_hole_loop_splitting(
                MH, hole1, triangles, use_normals, false
            );
            ok = ok && triangulate_hole_loop_splitting(
                MH, hole2, triangles, use_normals, false
            );
        }
        return ok;
    }

    /************************************************************************/

    /**
     * \brief Computes the score obtained when generating a triangle
     * in the ear cutting algorithm triangulate_hole_ear_cutting().
     * \param[in] M the mesh
     * \param[in] T1 first triangle, encoded as a vertex indices triplet
     * \param[in] T2 second triangle, encoded as a vertex indices triplet
     * \return the score obtained when generating an ear from \p T1 to \p T2
     */
    double ear_score(
        const Mesh& M,
        const trindex& T1,
        const trindex& T2
    ) {
        geo_debug_assert(T1.indices[1] == T2.indices[0]);
        const vec3& p10 = Geom::mesh_vertex(M, T1.indices[0]);
        const vec3& p11 = Geom::mesh_vertex(M, T1.indices[1]);
        const vec3& p12 = Geom::mesh_vertex(M, T1.indices[2]);
        const vec3& p20 = Geom::mesh_vertex(M, T2.indices[0]);
        const vec3& p21 = Geom::mesh_vertex(M, T2.indices[1]);
        const vec3& p22 = Geom::mesh_vertex(M, T2.indices[2]);
        vec3 n = normalize(
            Geom::triangle_normal(p10, p11, p12) +
            Geom::triangle_normal(p20, p21, p22)
        );
        vec3 a = normalize(p11 - p10);
        vec3 b = normalize(p21 - p20);
        return -::atan2(dot(n, cross(a, b)), dot(a, b));
    }

    /**
     * \brief Triangulates a hole using the ear cutting algorithm.
     * \param[in] MH the mesh, wrapped with halfedge accessors
     * \param[in] hole_in the hole, represented by a vector of halfedges
     * \param[out] triangles the generated triangles
     * \param[out] clear if set, \p triangles is cleared
     */
    void triangulate_hole_ear_cutting(
        const MeshHalfedges& MH, const Hole& hole_in,
        vector<trindex>& triangles, bool clear = true
    ) {
        if(clear) {
            triangles.clear();
        }
        if(hole_in.size() <= 3) {
            if(hole_in.size() == 3) {
                trindex T(
                    MH.mesh().facet_corners.vertex(hole_in[0].corner),
                    MH.mesh().facet_corners.vertex(hole_in[1].corner),
                    MH.mesh().facet_corners.vertex(hole_in[2].corner),
                    trindex::KEEP_ORDER
                );
                triangles.push_back(T);
            }
        } else {
            const Mesh& M = MH.mesh();

            // Step 1: convert hole into easier-to-manipulate representation.
            vector<trindex> hole;
            hole.reserve(hole_in.size());
            for(index_t i = 0; i < hole_in.size(); i++) {
                const MeshHalfedges::Halfedge& H = hole_in[i];
                geo_debug_assert(H.facet != MeshHalfedges::Halfedge::NO_FACET);
                index_t c = H.corner;
                index_t v1 = M.facet_corners.vertex(c);
                c = M.facets.next_corner_around_facet(H.facet, c);
                index_t v2 = M.facet_corners.vertex(c);
                c = M.facets.next_corner_around_facet(H.facet, c);
                index_t v3 = M.facet_corners.vertex(c);
                hole.push_back(trindex(v1, v2, v3, trindex::KEEP_ORDER));
            }

            // Step 2: ear cutting
            while(hole.size() > 3) {
                signed_index_t best_i1 = -1;
                double best_score = Numeric::min_float64();
                // TODO: take existing edges into account.
                for(index_t i1 = 0; i1 < hole.size(); i1++) {
                    index_t i2 = i1 + 1;
                    if(i2 == hole.size()) {
                        i2 = 0;
                    }
                    double score = ear_score(M, hole[i1], hole[i2]);
                    if(score > best_score) {
                        best_i1 = signed_index_t(i1);
                        best_score = score;
                    }
                }
                geo_assert(best_i1 != -1);
                index_t best_i2 = index_t(best_i1) + 1;
                if(best_i2 == hole.size()) {
                    best_i2 = 0;
                }
                const trindex& T1 = hole[best_i1];
                const trindex& T2 = hole[best_i2];
                geo_debug_assert(T1.indices[1] == T2.indices[0]);
                trindex T(
                    T1.indices[0], T2.indices[1], T1.indices[1],
                    trindex::KEEP_ORDER
                );
                hole[best_i1] = T;
                hole.erase(hole.begin() + std::ptrdiff_t(best_i2));
                triangles.push_back(T);
            }

            // Step 3: last triangle
            geo_assert(hole.size() == 3);
            trindex T(
                hole[0].indices[0],
                hole[1].indices[0],
                hole[2].indices[0],
                trindex::KEEP_ORDER
            );
            triangles.push_back(T);
        }
    }

    /************************************************************************/

    /**
     * \brief Computes the area of a hole, i.e. the area of the generated
     *  triangles that will fill the hole.
     * \param[in] M the mesh
     * \param[in] triangles the triangles that will fill the hole
     * \return the summed ares of the triangles in \p triangles
     */
    double hole_area(
        const Mesh& M, const vector<trindex>& triangles
    ) {
        double result = 0.0;
        for(index_t t = 0; t < triangles.size(); t++) {
            index_t i = triangles[t].indices[0];
            index_t j = triangles[t].indices[1];
            index_t k = triangles[t].indices[2];
            const vec3& p1 = Geom::mesh_vertex(M, i);
            const vec3& p2 = Geom::mesh_vertex(M, j);
            const vec3& p3 = Geom::mesh_vertex(M, k);
            result += Geom::triangle_area(p1, p2, p3);
        }
        return result;
    }

    /**
     * \brief Strategy used to fill the holes.
     */
    enum HoleFilling {
        LOOP_SPLIT,  /**< Splits loops by generating small segments */
        NLOOP_SPLIT, /**< Takes normals into account */
        EAR_CUT      /**< Uses the "ear cutting" strategy */
    };

    /************************************************************************/

    /**
     * \brief Removes all the facets of a mesh that are 
     *  on a bridge.
     * \details A facet is said to be on a bridge if it is
     *  incident to a border and if when turning around the
     *  border it is encountered more than once.
     */
    void remove_bridges(Mesh& M) {
        MeshHalfedges MH(M);
        vector<bool> corner_is_visited(M.facet_corners.nb(),false);
        vector<index_t> f_status(M.facets.nb(),0);
        index_t f_stamp=1;
        const index_t BRIDGE = index_t(-1);
        
        for(index_t f: M.facets) {
            for(index_t c: M.facets.corners(f)) {
                if(
                    M.facet_corners.adjacent_facet(c) == NO_FACET &&
                    !corner_is_visited[c]
                ) {
                    MeshHalfedges::Halfedge first(f, c);
                    MeshHalfedges::Halfedge H(f, c);
                    do {
                        corner_is_visited[H.corner] = true;
                        MH.move_to_next_around_facet(H);
                        while(MH.move_to_next_around_vertex(H)) {
                            if(f_status[H.facet] == f_stamp) {
                                f_status[H.facet] = BRIDGE;
                            } else if(
                                f_status[H.facet] != BRIDGE && 
                                f_status[H.facet] != f_stamp) {
                                f_status[H.facet] = f_stamp;
                            }
                        }
                    } while(H != first);
                    ++f_stamp;
                }
            }
        }
        index_t nb_bridges = 0;
        for(index_t f: M.facets) {
            if(f_status[f] == BRIDGE) {
                ++nb_bridges;
            } else {
                f_status[f] = 0;
            }
        }
        if(nb_bridges != 0) {
            M.facets.delete_elements(f_status);
            Logger::out("Bridges") 
                << "Removed " << nb_bridges << " bridge(s)"
                << std::endl;
        } 
    }
    
}

/****************************************************************************/

namespace GEO {

    void fill_holes(
	Mesh& M, double max_area, index_t max_edges, bool repair
    ) {

	// mesh_save(M, "before_fill_holes.geogram");
	
        if(max_area == 0.0 || max_edges == 0) {
            return;
        }

        remove_bridges(M);

        MeshHalfedges MH(M);

        vector<Hole> holes;

        index_t nb_filled_holes     = 0;
        index_t nb_skipped_by_edges = 0;
        index_t nb_skipped_by_area  = 0;
        index_t nb_could_not_fill   = 0;

        // Step 1: detect holes
        {
            vector<bool> corner_is_visited(M.facet_corners.nb(), false);
            for(index_t f: M.facets) {
                for(index_t c: M.facets.corners(f)) {
                    if(
                        M.facet_corners.adjacent_facet(c) == NO_FACET &&
                        !corner_is_visited[c]
                    ) {
                        holes.push_back(Hole());
                        MeshHalfedges::Halfedge first(f, c);
                        MeshHalfedges::Halfedge H(f, c);
                        do {
                            holes.rbegin()->push_back(H);
                            corner_is_visited[H.corner] = true;
                            MH.move_to_next_around_border(H);
                        } while(H != first);

                        if(holes.rbegin()->size() > max_edges) {
                            ++nb_skipped_by_edges;
                            holes.pop_back();
                        }
                    }
                }
            }
        }

        if(holes.size() == 0) {
            return;
        }

        Logger::out("FillHoles") << "Found " << holes.size()
            << " holes" << std::endl;

        HoleFilling algo = LOOP_SPLIT;
        std::string algo_name = CmdLine::get_arg("algo:hole_filling");
        if(algo_name == "loop_split") {
            algo = LOOP_SPLIT;
        } else if(algo_name == "Nloop_split") {
            algo = NLOOP_SPLIT;
        } else if(algo_name == "ear_cut") {
            algo = EAR_CUT;
        } else {
            Logger::warn("FillHoles")
                << algo_name << ": no such hole filling method"
                << std::endl;
            Logger::warn("FillHoles")
                << "falling back to \'loop_split\'"
                << std::endl;
        }

        
        for(index_t i = 0; i < holes.size(); i++) {
	    vector<trindex> triangles;
	    bool ok = true;
	    switch(algo) {
		case LOOP_SPLIT:
		    ok = triangulate_hole_loop_splitting(
			MH, holes[i], triangles, false
		    );
		    break;
		case NLOOP_SPLIT:
		    ok = triangulate_hole_loop_splitting(
			MH, holes[i], triangles, true
		    );
		    break;
		case EAR_CUT:
		    triangulate_hole_ear_cutting(MH, holes[i], triangles);
		    break;
	    }

	    if(ok) {
		if(hole_area(M, triangles) < max_area) {
		    for(index_t j = 0; j < triangles.size(); j++) {
			M.facets.create_triangle(
			    triangles[j].indices[2],
			    triangles[j].indices[1],
			    triangles[j].indices[0]
			);
		    }
		    ++nb_filled_holes;
		} else {
		    ++nb_skipped_by_area;
		}
	    } else {
		++nb_could_not_fill;
	    }
	    
        }

        if(nb_skipped_by_area != 0) {
            Logger::out("FillHoles")
                << "Skipped " << nb_skipped_by_area 
                << " holes (area too large)" << std::endl;
        }

        if(nb_skipped_by_edges != 0) {
            Logger::out("FillHoles")
                << "Skipped " << nb_skipped_by_edges 
                << " holes (too many edges)" << std::endl;
        }

        if(nb_could_not_fill != 0) {
            Logger::out("FillHoles")
                << "Skipped " << nb_could_not_fill
                << " holes (could not fill)" << std::endl;
        }

        if(nb_filled_holes != 0 && repair) {
            // Needed because we may generate zero-length edges
            // and zero-area facets that need to be eliminated.
            // Note: this also reconstructs the connections between the facets.
	    MeshRepairMode mode = MESH_REPAIR_DEFAULT;
            mesh_repair(M, mode); 
        }
    }

    /**
     * \brief Tessellates a hole.
     * \param[in] MH the MeshHalfHeddes the hole belongs to.
     * \param[in] H the hole.
     * \param[in] max_nb_vertices maximum number of vertices in
     *  the new facets to create.
     * \param[in] copy_facet_attrib an optional facet index. If 
     *  specified, all the attributes of this facet will be copied
     *  to the created facets.
     */
    static void tessellate_hole(
	MeshHalfedges& MH, Hole& H, index_t max_nb_vertices,
	index_t copy_facet_attrib = index_t(-1)
    ) {
	Mesh& M = MH.mesh();
	if(H.size() <= max_nb_vertices) {
	    index_t f = M.facets.create_polygon(H.size());
	    FOR(i,H.size()) {
		index_t v = M.facet_corners.vertex(H[i].corner);
		M.facets.set_vertex(f,i,v);
	    }
	    if(copy_facet_attrib != index_t(-1)) {
		M.facets.attributes().copy_item(f, copy_facet_attrib);
	    }
	} else {
	    Hole H1,H2;
	    split_hole(MH,H,H1,H2,false);
	    tessellate_hole(MH,H1,max_nb_vertices,copy_facet_attrib);
	    tessellate_hole(MH,H2,max_nb_vertices,copy_facet_attrib);
	}
    }

    void tessellate_facets(
	Mesh& M, index_t max_nb_vertices
    ) {
        MeshHalfedges MH(M);
	vector<index_t> delete_f(M.facets.nb(),0);
	for(index_t f: M.facets) {
	    if(M.facets.nb_vertices(f) > max_nb_vertices) {
		delete_f[f] = 1;
		Hole h;
		for(index_t c: M.facets.corners(f)) {
		    h.push_back(MeshHalfedges::Halfedge(f,c));
		}
		tessellate_hole(MH, h, max_nb_vertices, f);
	    }
	}
	delete_f.resize(M.facets.nb());
	M.facets.delete_elements(delete_f);
	M.facets.connect();
	if(max_nb_vertices == 3) {
	    M.facets.is_simplicial();
	}
    }
    
}

