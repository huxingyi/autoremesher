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

#include <geogram/mesh/mesh_repair.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/index.h>
#include <geogram/mesh/mesh_halfedges.h>
#include <geogram/mesh/mesh_io.h>
#include <geogram/mesh/mesh_preprocessing.h>
#include <geogram/points/colocate.h>
#include <geogram/basic/geometry_nd.h>
#include <geogram/basic/stopwatch.h>
#include <geogram/basic/command_line.h>
#include <geogram/basic/argused.h>
#include <geogram/basic/algorithm.h>
#include <stack>
#include <queue>

namespace {

    using namespace GEO;

    /**
     * \brief Merges the vertices of a mesh that are at the same
     *  geometric location
     * \param[in] M the mesh
     * \param[in] colocate_epsilon tolerance for merging vertices
     */
    void repair_colocate_vertices(Mesh& M, double colocate_epsilon) {
        vector<index_t> old2new;

	if(M.vertices.nb() == 0) {
	    return;
	}
	
        index_t nb_new_vertices = 0;
        if(colocate_epsilon == 0.0) {
            nb_new_vertices = Geom::colocate_by_lexico_sort(
                M.vertices.point_ptr(0), 3, M.vertices.nb(),
                old2new, M.vertices.dimension()
            );
        } else {
            nb_new_vertices = Geom::colocate(
                M.vertices.point_ptr(0), 3, M.vertices.nb(),
                old2new, colocate_epsilon, M.vertices.dimension()
            );
        }

        if(nb_new_vertices == M.vertices.nb()) {
            return;
        }

        Logger::out("Validate") << "Removed "
            << M.vertices.nb() - nb_new_vertices
            << " duplicated vertices" << std::endl;


        // Replace vertex indices for edges
        for(index_t e: M.edges) {
            M.edges.set_vertex(e, 0, old2new[M.edges.vertex(e,0)]);
            M.edges.set_vertex(e, 1, old2new[M.edges.vertex(e,1)]);            
        }

        // Replace vertex indices for facets
        for(index_t c: M.facet_corners) {
            M.facet_corners.set_vertex(c, old2new[M.facet_corners.vertex(c)]);
        }

        // Replace vertex indices for cells
        for(index_t ce: M.cells) {
            for(index_t c: M.cells.corners(ce)) {
                M.cell_corners.set_vertex(c, old2new[M.cell_corners.vertex(c)]);
            }
        } 
        
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
    }

    /**
     * \brief Tests whether a facet is degenerate.
     * \param[in] M the mesh that the facet belongs to
     * \param[in] f the index of the facet in \p M
     * \return true if facet \p f has duplicated vertices,
     *  false otherwise
     */
    bool facet_is_degenerate(const Mesh& M, index_t f) {
        index_t nb_vertices = M.facets.nb_vertices(f);
        if(nb_vertices != 3) {
            index_t* vertices = (index_t*)alloca(nb_vertices*sizeof(index_t));
            for(index_t lv=0; lv<nb_vertices; ++lv) {
                vertices[lv] = M.facets.vertex(f,lv);
            }
            std::sort(vertices, vertices + nb_vertices);
            return std::unique(
                vertices, vertices + nb_vertices
            ) != vertices + nb_vertices;
        } 
        index_t c1 = M.facets.corners_begin(f);
        index_t c2 = c1 + 1;
        index_t c3 = c2 + 1;
        index_t v1 = M.facet_corners.vertex(c1);
        index_t v2 = M.facet_corners.vertex(c2);
        index_t v3 = M.facet_corners.vertex(c3);
        return v1 == v2 || v2 == v3 || v3 == v1;
    }

    /**
     * \brief Generates a unique ordering of the vertices of
     *  a facet.
     * \details Shifts and inverts the order of f's vertices in
     * such a way that f's first vertex has the smallest index
     * and it's predecessor->successor have increasing vertex
     * indices. This ensures that the same facet has a unique
     * representation (used to detect duplicated facets).
     * \param[in] M the mesh that the facet belongs to
     * \param[in] f the index of the facet in \p M
     */
    void normalize_facet_vertices_order(Mesh& M, index_t f) {
        index_t d = M.facets.nb_vertices(f);
        index_t c_min = M.facets.corners_begin(f);
        for(index_t c = M.facets.corners_begin(f) + 1;
            c < M.facets.corners_end(f); ++c
        ) {
            if(M.facet_corners.vertex(c) < M.facet_corners.vertex(c_min)) {
                c_min = c;
            }
        }
        index_t c_prev = M.facets.prev_corner_around_facet(f, c_min);
        index_t c_next = M.facets.next_corner_around_facet(f, c_min);
        bool direct = (
            M.facet_corners.vertex(c_next) >= M.facet_corners.vertex(c_prev)
        );
        index_t* f_vertex = (index_t*) alloca(sizeof(signed_index_t) * d);
        {
            index_t c = c_min;
            for(index_t i = 0; i < d; i++) {
                f_vertex[i] = M.facet_corners.vertex(c);
                c = direct ? M.facets.next_corner_around_facet(f, c)
                    : M.facets.prev_corner_around_facet(f, c);
            }
        }
        for(index_t i = 0; i < d; i++) {
            index_t c = M.facets.corners_begin(f);
            M.facet_corners.set_vertex(c + i, f_vertex[i]);
        }
    }

    /**
     * \brief Comparator class for sorting facets.
     */
    class CompareFacets {
    public:
        /**
         * \brief Constructs a new CompareFacets.
         * \param[in] M the mesh
         */
        explicit CompareFacets(const Mesh& M) :
            mesh_(M) {
        }

        /**
         * \brief Tests the lexicographic order of two facets by their indices.
         * \param[in] f1 index of the first facet
         * \param[in] f2 index of the second facet
         * \return true if facet \p f1 is before facet \p f2 according to
         *  the lexicographic order of its vertices, false otherwise.
         */
        bool is_before(index_t f1, index_t f2) const {
            index_t c1 = mesh_.facets.corners_begin(f1);
            index_t c2 = mesh_.facets.corners_begin(f2);
            while(
                c1 != mesh_.facets.corners_end(f1) &&
                c2 != mesh_.facets.corners_end(f2)
            ) {
                index_t v1 = mesh_.facet_corners.vertex(c1);
                index_t v2 = mesh_.facet_corners.vertex(c2);
                if(v1 > v2) {
                    return false;
                }
                if(v1 < v2) {
                    return true;
                }
                c1++;
                c2++;
            }
            return (
                c1 == mesh_.facets.corners_end(f1) &&
                c2 != mesh_.facets.corners_end(f2)
            ) ;
        }

        /**
         * \brief Tests whether two facets are identical.
         * \param[in] f1 index of the first facet
         * \param[in] f2 index of the second facet
         * \return true if facets \p f1 and \p f2 have the same
         *  vertices, false otherwise
         */
        bool is_same(index_t f1, index_t f2) const {
            if(mesh_.facets.nb_vertices(f1) != mesh_.facets.nb_vertices(f2)) {
                return false;
            }
            index_t c1 = mesh_.facets.corners_begin(f1);
            index_t c2 = mesh_.facets.corners_begin(f2);
            while(c1 != mesh_.facets.corners_end(f1)) {
                geo_debug_assert(c2 != mesh_.facets.corners_end(f2));
                index_t v1 = mesh_.facet_corners.vertex(c1);
                index_t v2 = mesh_.facet_corners.vertex(c2);
                if(v1 != v2) {
                    return false;
                }
                c1++;
                c2++;
            }
            return true;
        }

        /**
         * \brief Tests the lexicographic order of two facets by their indices.
         * \param[in] f1 index of the first facet
         * \param[in] f2 index of the second facet
         * \return true if facet \p f1 is before facet \p f2 according to
         *  the lexicographic order of its vertices, false otherwise.
         */
        bool operator() (index_t f1, index_t f2) const {
            return is_before(f1, f2);
        }

    private:
        const Mesh& mesh_;
    };


    /**
     * \brief Finds the non-duplicated vertices of a facet
     * \param[in] M a const reference to a mesh
     * \param[in] f a facet index in \p M
     * \param[out] new_polygon on exit, where to append the 
     *              non-duplicated vertices of facet \p f
     *              and a terminal index_t(-1)
     * \retval true if the facet has three non-duplicated 
     *               vertices and more
     * \retval false otherwise
     */
    bool find_facet_non_duplicated_vertices(
        const Mesh& M, index_t f, vector<index_t>& new_polygon
    ) {
        index_t first_corner = index_t(-1);
        
        // Find the first vertex that is different from
        // its predecessor around the facet.
        for(index_t c1: M.facets.corners(f)) {
            index_t c2 = M.facets.next_corner_around_facet(f,c1);
            if(M.facet_corners.vertex(c1) != M.facet_corners.vertex(c2)) {
                first_corner = c2;
                break;
            }
        }

        // All the vertices may be identical (if the facet
        // is completely degenerate).
        if(first_corner == index_t(-1)) {
            return false;
        }

        index_t c = first_corner;
        index_t cur_v = index_t(-1);
        index_t nb = 0;
        
        do {
            index_t v = M.facet_corners.vertex(c);
            if(v != cur_v) {
                new_polygon.push_back(v);
                cur_v = v;
                ++nb;
            }
            c = M.facets.next_corner_around_facet(f,c);
        } while(c != first_corner);
        new_polygon.push_back(index_t(-1));

        //   If there were only 2 non-duplicated vertices, then
        // the facet is degenerate and is "rolled back" (we do
        // not want to generate facets with two vertices only).
        if(nb == 2) {
            new_polygon.resize(new_polygon.size()-3);
            return false;
        }

        return true;
    }
    
    
    /**
     * \brief Detects degenerate facets in a mesh.
     * \param[in] M the mesh
     * \param[in] check_duplicates if true, duplicated facets are
     *  detected and all but one instance of each is marked as to be
     *  removed.
     * \param[out] remove_f indicates for each facet whether it should be
     *  removed. If remove_f[f] != 0 if f should be removed, else f 
     *  should be kept. If remove_f.size() == 0, then there is
     *  no facet to remove, else remove_f.size() == M.facets.nb(). 
     * \param[out] old_polygons if non zero, on exit contains the
     *  indices of the polygonal facets that had duplicated vertices
     * \param[out] new_polygons if non zero, on exit contains the
     *  polygonal facets to be created to replace the input polygonal
     *  facets that have duplicated vertices. Each individual facet
     *  is terminated by index_t(-1)
     */
    void detect_bad_facets(
        Mesh& M, bool check_duplicates, vector<index_t>& remove_f,
        vector<index_t>* old_polygons = nullptr,
        vector<index_t>* new_polygons = nullptr
    ) {
        index_t nb_duplicates = 0;
        index_t nb_degenerate = 0;
        if(check_duplicates) {
            // Reorder vertices around each facet to make
            // it easier to compare two facets.
            for(index_t f: M.facets) {
                normalize_facet_vertices_order(M, f);
            }
            // Indirect-sort the facets in lexicographic
            // order. 
            vector<index_t> f_sort(M.facets.nb());
            for(index_t f: M.facets) {
                f_sort[f] = f;
            }
            CompareFacets compare_facets(M);
            GEO::sort(f_sort.begin(), f_sort.end(), compare_facets);
            // Now f_sort[0] ... fsort[nb_facets-1] contains the indices
            // of the sorted facets. This ensures that the indices of the
            // facets with the same vertices (i.e. duplicated facets)
            // appear at contiguous sequences in fsort.

            // Traverse in fsort the sequences of duplicate facets. 
            // The algorithm detects the sequence of indices 
            // f_sort[if1] ... f_sort[if2-1] that contain facets 
            // with the same indices.
            index_t if1 = 0;
            while(if1 < M.facets.nb()) {
                index_t if2 = if1 + 1;
                while(
                    if2 < M.facets.nb() &&
                    compare_facets.is_same(f_sort[if1], f_sort[if2])
                ) {
                    nb_duplicates++;
                    // Tag all facets in f_sort[if1+1] ... f_sort[if2-1] as
                    // 'to be removed' (because they all have the same vertices
                    // as f_sort[if1]).
                    if(remove_f.size() == 0) {
                        remove_f.resize(M.facets.nb(), 0);
                    }
                    remove_f[f_sort[if2]] = 1;
                    if2++;
                }
                if1 = if2;
            }
        }

        // Now, we tag the degenerate facets as 'to be removed'. A
        // facet is degenerate if it is incident to the same vertex several
        // times.
        for(index_t f: M.facets) {
            if(
                (remove_f.size() == 0 || remove_f[f] == 0) &&
                facet_is_degenerate(M, f)
            ) {
                nb_degenerate++;
                if(remove_f.size() == 0) {
                    remove_f.resize(M.facets.nb(), 0);
                }
                remove_f[f] = 1;

                // If we found a degenerate polygonal facet and
                // we want to regenerate a valid one:
                if(
                    old_polygons != nullptr &&
                    new_polygons != nullptr &&
                    M.facets.nb_vertices(f) > 3
                ) {
                    if(find_facet_non_duplicated_vertices(
                           M,f,*new_polygons
                    )) {
                        old_polygons->push_back(f);
                    }
                }
            }
        }
        if(nb_duplicates != 0 || nb_degenerate != 0) {
            Logger::out("Validate")
                << "Detected " << nb_duplicates << " duplicate and "
                << nb_degenerate << " degenerate facets"
                << std::endl;
        }
    }

    /**
     * \brief Detects and removes the degenerate facets in a mesh.
     * \param[in] M the mesh
     * \param[in] check_duplicates if true, duplicated facets are
     *  removed.
     */
    void repair_remove_bad_facets(Mesh& M, bool check_duplicates) {
        vector<index_t> remove_f;
        vector<index_t> old_polygons;
        vector<index_t> new_polygons;
        detect_bad_facets(
            M, check_duplicates, remove_f, &old_polygons, &new_polygons
        );
        index_t current_old_polygon=0;
        if(remove_f.size() != 0) {
            //   Create the new facets that correspond to input polygonal
            // facets that had duplicated vertices.
            //   This needs to be done before deleting the bad facets,
            // else some vertices will become isolated and will be
            // discarded.
            index_t b=0;
            index_t e=0;
            while(b < new_polygons.size()) {
                while(new_polygons[e] != index_t(-1)) {
                    ++e;
                }
                index_t new_f = M.facets.create_polygon(e-b);
                M.facets.attributes().copy_item(
                    new_f, old_polygons[current_old_polygon]
                );
                ++current_old_polygon;
                // We created a new facet that we want to keep !!
                remove_f.push_back(0); 
                for(index_t lv=0; lv<e-b; ++lv) {
                    M.facets.set_vertex(new_f,lv,new_polygons[b+lv]);
                }
                ++e;
                b=e;
            }
            M.facets.delete_elements(remove_f);            
        }
        
    }

    /************************************************************************/

    /**
     * \brief Connects the facets in a mesh.
     * \details Reconstructs the corners.adjacent_facet links.
     *  Note that the Moebius law is not respected by this
     *  function (adjacent facets may have incoherent orientations).
     *  This function outputs a mesh with possibly not coherently 
     *  oriented triangles. In other words, for two
     *  corners c1, c2, if we have:
     *   - v1 = facet_corners.vertex(c1)
     *   - v2 = facet_corners.vertex(
     *            c1,facets.next_corner_around_facet(c2f(c1),c1)
     *          )
     *   - w1 = facet_corners.vertex(c2)
     *   - w2 = facet_corners.vertex(
     *            c2,facets.next_corner_around_facet(c2f(c2),c2)
     *          )
     *  then c1 and c2 are adjacent if we have:
     *   - v1=w2 and v2=w1 (as usual) or:
     *   - v1=v2 and w1=w2 ('inverted' configuration)
     *  The output of this function can be then post-processed by 
     *  repair_reorient_facets_anti_moebius() to recover coherent
     *  orientations.
     * \param[in] M the mesh to repair
     */
    void repair_connect_facets(
        Mesh& M
    ) {
        const index_t NON_MANIFOLD=index_t(-2);

        // Reset all facet-facet adjacencies.
        for(index_t c: M.facet_corners) {
            M.facet_corners.set_adjacent_facet(c,NO_FACET);
        }

        // For each vertex v, v2c[v] gives the index of a 
        // corner incident to vertex v.
        vector<index_t> v2c(M.vertices.nb(),NO_CORNER);

        // For each corner c, next_c_around_v[c] is the 
        // linked list of all the corners incident to 
        // vertex v.
        vector<index_t> next_c_around_v(M.facet_corners.nb(),NO_CORNER);

        // For each corner c, c2f[c] is the index of
        // the facet incident to c (or use c/3 if
        // M is triangulated).
        vector<index_t> c2f;
        if(!M.facets.are_simplices()) {
            c2f.assign(M.facet_corners.nb(), NO_FACET);
        }

        // Compute v2c and next_c_around_v
        for(index_t c: M.facet_corners) {
            index_t v = M.facet_corners.vertex(c);
            next_c_around_v[c] = v2c[v];
            v2c[v] = c;
        }

        // Compute f2c (only if M is not triangulated, 
        // because if M is triangulated, we have f2c(c) = c/3).
        if(!M.facets.are_simplices()) {
            for(index_t f: M.facets) {
                for(index_t c: M.facets.corners(f)) {
                    c2f[c]=f;
                }
            }
        }

        for(index_t f1: M.facets) {
            for(index_t c1: M.facets.corners(f1)) {
                if(M.facet_corners.adjacent_facet(c1) == NO_FACET) {
                    index_t adj_corner = NO_CORNER;
                    index_t v1=M.facet_corners.vertex(c1);
                    index_t v2=M.facet_corners.vertex(
                                   M.facets.next_corner_around_facet(f1,c1)
                               );

                    index_t c2 = v2c[v1];

                    // Lookup candidate adjacent edges from incident
                    // edges list.
                    while(c2 != NO_CORNER) {
                        if(c2 != c1) {
                            index_t f2 =
                                M.facets.are_simplices() ? c2/3 : c2f[c2];
                            index_t c3 =
                                M.facets.prev_corner_around_facet(f2,c2);
                            index_t v3 = M.facet_corners.vertex(c3);
                            // Check with standard orientation.
                            if(v3 == v2) {
                                if(adj_corner == NO_CORNER) {
                                    adj_corner = c3;
                                } else {
                                    adj_corner = NON_MANIFOLD;
                                }
                            } else {
                                // Check with the other ("wrong") orientation
                                c3 = M.facets.next_corner_around_facet(f2,c2);
                                v3 = M.facet_corners.vertex(c3);
                                if(v3 == v2) {
                                    if(adj_corner == NO_CORNER) {
                                        adj_corner = c2;
                                    } else {
                                        adj_corner = NON_MANIFOLD;
                                    }
                                }
                            }
                        }
                        c2 = next_c_around_v[c2];
                    }
                    if(
                        adj_corner != NO_CORNER && 
                        adj_corner != NON_MANIFOLD
                    ) {
                        M.facet_corners.set_adjacent_facet(adj_corner,f1);
                        index_t f2 = M.facets.are_simplices() ? 
                                     adj_corner/3 : 
                                     c2f[adj_corner] ;
                        M.facet_corners.set_adjacent_facet(c1,f2);
                    } 
                }
            }
        }
    }

    /************************************************************************/

    /**
     * \brief Tests the relative orientation of two adjacent facets
     * \param[in] M the mesh
     * \param[in] f1 index of the first facet
     * \param[in] c11 index of a corner in facet \p f1
     * \param[in] f2 index of the second facet
     * \return 1 if \p f1 and \p f2 have compatible orientations, -1 if
     *  they have incompatible orientations, 0 if they are not adjacent
     */
    inline signed_index_t repair_relative_orientation(
        Mesh& M, index_t f1, index_t c11, index_t f2
    ) {
        index_t c12 = M.facets.next_corner_around_facet(f1, c11);
        index_t v11 = M.facet_corners.vertex(c11);
        index_t v12 = M.facet_corners.vertex(c12);
        for(index_t c21: M.facets.corners(f2)) {
            index_t c22 = M.facets.next_corner_around_facet(f2, c21);
            index_t v21 = M.facet_corners.vertex(c21);
            index_t v22 = M.facet_corners.vertex(c22);
            if(v11 == v21 && v12 == v22) {
                return -1;
            }
            if(v11 == v22 && v12 == v21) {
                return 1;
            }
        }
        return 0;
    }

    /**
     * \brief Removes an adjacency connections between two facets in a mesh
     * \param[in] M the mesh
     * \param[in] f1 index of the first facet
     * \param[in] f2 index of the second facet
     */
    void repair_dissociate(
        Mesh& M, index_t f1, index_t f2
    ) {
        for(index_t c: M.facets.corners(f1)) {
            if(M.facet_corners.adjacent_facet(c) == f2) {
                M.facet_corners.set_adjacent_facet(c, NO_FACET);
            }
        }
        for(index_t c: M.facets.corners(f2)) {
            if(M.facet_corners.adjacent_facet(c) == f1) {
                M.facet_corners.set_adjacent_facet(c, NO_FACET);
            }
        }
    }

    /**
     * \brief Greedily propagates facet reorientation in a mesh
     * \details Whenever a Moebius loop is encountered, the involved
     *  facets are disconnected from their neighbors.
     * \param[in] M the mesh
     * \param[in] f index of the current facet
     * \param[in,out] visited a vector used to mark facets that were
     *  already traversed
     * \param[out] moebius_count number of Moebius loops encountered
     * \param[out] moebius_facets a pointer to a vector. On exit,
     *  *moebius_facets[f] has a non-zero value if facet f is
     *  incident to an edge that could not be consistently oriented.
     *  If nullptr, then this information is not returned.
     */
    void repair_propagate_orientation(
        Mesh& M, index_t f, const std::vector<bool>& visited,
        index_t& moebius_count,
        vector<index_t>* moebius_facets = nullptr
    ) {
        index_t nb_plus = 0;
        index_t nb_minus = 0;
        for(index_t c: M.facets.corners(f)) {
            index_t f2 = M.facet_corners.adjacent_facet(c);
            if(f2 != NO_FACET && visited[index_t(f2)]) {
                signed_index_t ori = 
                    repair_relative_orientation(M, f, c, f2);
                switch(ori) {
                    case 1:
                        nb_plus++;
                        break;
                    case -1:
                        nb_minus++;
                        break;
                    case 0:
                        geo_assert_not_reached;
                }
            }
        }
        if(nb_plus != 0 && nb_minus != 0) {
            moebius_count++;
            if(moebius_facets != nullptr) {
                moebius_facets->resize(M.facets.nb(), 0);
                (*moebius_facets)[f] = 1;
                for(index_t c: M.facets.corners(f)) {
                    index_t f2 = M.facet_corners.adjacent_facet(c);
                    if(f2 != NO_FACET) {
                        (*moebius_facets)[f2] = 1;
                    }
                }
            }
            if(nb_plus > nb_minus) {
                nb_minus = 0;
                for(index_t c: M.facets.corners(f)) {
                    index_t f2 = M.facet_corners.adjacent_facet(c);
                    if(
                        f2 != NO_FACET && visited[f2] &&
                        repair_relative_orientation(M, f, c, f2) < 0
                    ) {
                        repair_dissociate(M, f, f2);
                    }
                }
            } else {
                nb_plus = 0;
                for(index_t c: M.facets.corners(f)) {
                    index_t f2 = M.facet_corners.adjacent_facet(c);
                    if(
                        f2 != NO_FACET && visited[index_t(f2)] &&
                        repair_relative_orientation(M, f, c, f2) > 0
                    ) {
                        repair_dissociate(M, f, f2);
                    }
                }
            }
        }
	geo_argused(nb_plus);
        if(nb_minus != 0) {
            M.facets.flip(f);
        }
    }

    /**
     * \brief Tests whether a facet of a mesh is on the border.
     * \param[in] M the mesh
     * \param[in] f index of the facet
     * \return true if \p f is on the border of \p M, false otherwise
     */
    bool facet_is_on_border(Mesh& M, index_t f) {
        for(index_t c: M.facets.corners(f)) {
            if(M.facet_corners.adjacent_facet(c) == NO_FACET) {
                return true;
            }
        }
        return false;
    }

    /**
     * \brief Used to represent graph distance to border.
     *  Since values are clamped to a small number (typically 5),
     *  this fits in a single byte.
     */
    typedef Numeric::uint8 facet_distance_t;

    /**
     * \brief Computes for each facet its facet-graph distance to
     *  the border of the mesh, clamped to max_iter.
     * \param[in] M the mesh
     * \param[out] D for each facet, its graph distance to the border
     * \param[in] max_iter maximumm number of iterations (determines the
     *  largest possible computed graph distance).
     */
    void compute_border_distance(
        Mesh& M, vector<facet_distance_t>& D, index_t max_iter
    ) {
        geo_assert(max_iter < 256);
        D.assign(M.facets.nb(), facet_distance_t(max_iter));
        for(index_t f: M.facets) {
            if(facet_is_on_border(M, f)) {
                D[f] = facet_distance_t(0);
            }
        }
        for(signed_index_t i = 1; i < signed_index_t(max_iter); i++) {
            for(index_t f: M.facets) {
                if(D[f] == signed_index_t(max_iter)) {
                    for(index_t c: M.facets.corners(f)) {
                        index_t g = M.facet_corners.adjacent_facet(c);
                        if(g != NO_FACET && D[g] == facet_distance_t(i - 1)) {
                            D[f] = facet_distance_t(i);
                            break;
                        }
                    }
                }
            }
        }
    }

    /**
     * \brief A priority queue specialized to
     *  the specific case where priorities can
     *  take a small number of distinct values.
     * \details
     *  It is implemented as an array of stacks.
     */
    class SimplePriorityQueue {
    public:
        /**
         * \param[in] D priorities
         * \param[in] max_distance max value in D
         */
        SimplePriorityQueue(
            const vector<facet_distance_t>& D,
            facet_distance_t max_distance
        ) :
            stacks_(index_t(max_distance + 1)),
            D_(D) {
        }

        /**
         * \brief Pushes a facet onto the priority queue
         * \param[in] f index of the facet to push
         */
        void push(index_t f) {
            geo_debug_assert(D_[f] < stacks_.size());
            stacks_[D_[f]].push(f);
        }

        /**
         * \brief Pops a facet from the priority queue
         * \return the index of the popped facet
         */
        index_t pop() {
            for(
                signed_index_t i = signed_index_t(stacks_.size()) - 1;
                i >= 0; i--
            ) {
                if(!stacks_[i].empty()) {
                    index_t result = stacks_[i].top();
                    stacks_[i].pop();
                    return result;
                }
            }
            geo_assert_not_reached;
        }

        /**
         * \brief Tests whether this SimplePriorityQueue is empty
         * \return true if this SimplePriorityQueue is empty, false
         *  otherwise
         */
        bool empty() {
            for(index_t i = 0; i < stacks_.size(); i++) {
                if(!stacks_[i].empty()) {
                    return false;
                }
            }
            return true;
        }

    private:
        vector<std::stack<index_t> > stacks_;
        const vector<facet_distance_t>& D_;
    };

    /**
     * \brief Reorients the facets with a heuristic that reduces
     *  the impact of Moebius loops.
     * \param[in,out] M the mesh to repair
     * \param[out] moebius_facets a pointer to a vector. On exit,
     *  *moebius_facets[f] has a non-zero value if facet f is
     *  incident to an edge that could not be consistently oriented.
     *  If nullptr, then this information is not returned.
     */
    void repair_reorient_facets_anti_moebius(
        Mesh& M, vector<index_t>* moebius_facets=nullptr
    ) {
        const int max_iter = 5;
        vector<facet_distance_t> D;
        std::vector<bool> visited(M.facets.nb(), false);
        compute_border_distance(M, D, max_iter);
        SimplePriorityQueue Q(D, max_iter);

        index_t moebius_count = 0;
        index_t nb_visited = 0;
        for(signed_index_t i = max_iter; i >= 0; i--) {
            for(index_t f: M.facets) {
                if(!visited[f] && D[f] == i) {
                    Q.push(f);
                    visited[f] = true;
                    nb_visited++;
                    while(!Q.empty()) {
                        index_t f1 = Q.pop();
                        for(index_t c: M.facets.corners(f1)) {
                            index_t f2 = M.facet_corners.adjacent_facet(c);
                            if(f2 != NO_FACET && !visited[f2]) {
                                visited[f2] = true;
                                nb_visited++;
                                repair_propagate_orientation(
                                    M, f2, visited, 
                                    moebius_count, moebius_facets
                                );
                                Q.push(f2);
                            }
                        }
                    }
                }
                if(nb_visited == M.facets.nb()) {
                    break;
                }
            }
        }
        if(moebius_count != 0) {
            Logger::out("Validate")
                << "Encountered " << moebius_count
                << " ambiguous facet orientation (Moebius)"
                << std::endl;
        }
    }

    /************************************************************************/

    /**
     * \brief Finds the corner by facet and vertex index
     * \param[in] M the mesh
     * \param[in] f the facet index
     * \param[in] v the vertex index
     * \return the index of the corner that corresponds to \p f and \p v
     * \pre such a corner does not exist in \p M
     */
    inline index_t find_corner(
        const Mesh& M, index_t f, index_t v
    ) {
        for(index_t c: M.facets.corners(f)) {
            if(M.facet_corners.vertex(c) == v) {
                return c;
            }
        }
        geo_assert_not_reached;
    }

    /**
     * \brief Splits the non-manifold vertices
     * \param[in] M the mesh to repair
     */
    void repair_split_non_manifold_vertices(Mesh& M) {
        std::vector<bool> c_is_visited(M.facet_corners.nb(), false);
        std::vector<bool> v_is_used(M.vertices.nb(), false);
        // new vertices are stored separately to avoid
        // too large vector growth that would occur if
        // pushed back to M.vertices_.
        vector<double> new_vertices;
        index_t nb_vertices = M.vertices.nb();
        for(index_t f: M.facets) {
            for(index_t c: M.facets.corners(f)) {
                if(!c_is_visited[c]) {
                    index_t cur_f = f;
                    index_t cur_c = c;
                    index_t old_v = M.facet_corners.vertex(c);
                    index_t new_v = old_v;
                    if(v_is_used[old_v]) {
                        new_v = nb_vertices;
                        nb_vertices++;
                        for(
                            index_t coord = 0; coord < M.vertices.dimension();
                            coord++
                        ) {
                            new_vertices.push_back(
                                M.vertices.point_ptr(old_v)[coord]
                            );
                        }
                    } else {
                        v_is_used[old_v] = true;
                    }

                    index_t count = 0;
                    for(;;) {
                        c_is_visited[cur_c] = true;
                        // cannot use corners.set_vertex
                        // since vertices are not created yet
                        // (would generate an assertion fail).
                        M.facet_corners.set_vertex_no_check(cur_c,new_v);
                        cur_f = M.facet_corners.adjacent_facet(cur_c);
                        if(cur_f == NO_FACET || cur_f == f) {
                            break;
                        }
                        cur_c = find_corner(M, index_t(cur_f), old_v);
                        count++;
                        geo_assert(count < 10000);
                    }

                    if(cur_f == NO_FACET) {
                        cur_f = f;
                        cur_c = c;
                        count = 0;
                        for(;;) {
                            cur_c = M.facets.prev_corner_around_facet(
                                index_t(cur_f), cur_c
                            );
                            cur_f = M.facet_corners.adjacent_facet(cur_c);
                            if(cur_f == NO_FACET) {
                                break;
                            }
                            cur_c = find_corner(M, index_t(cur_f), old_v);
                            c_is_visited[cur_c] = true;
                            // cannot use corners.set_vertex
                            // since size is not updated yet
                            // (would generate an assertion fail).
                            M.facet_corners.set_vertex_no_check(cur_c,new_v);
                            count++;
                            geo_assert(count < 10000);
                        } 
                    }
                }
            }
        }
        if(new_vertices.size() != 0) {
            Logger::out("Validate")
                << "Detected non-manifold vertices" << std::endl;
            Logger::out("Validate") << "   (fixed by generating "
                << nb_vertices - M.vertices.nb()
                << " new vertices)"
                << std::endl;

            index_t first_v = M.vertices.create_vertices(
                new_vertices.size() / M.vertices.dimension()
            );
            
            for(index_t i=0; i<new_vertices.size(); ++i) {
                M.vertices.point_ptr(first_v)[i] = new_vertices[i];
            }
            
        }
    }
}

/****************************************************************************/

namespace GEO {

    void mesh_repair(
        Mesh& M, MeshRepairMode mode, double colocate_epsilon
    ) {
        index_t nb_vertices_in = M.vertices.nb();
        index_t nb_facets_in = M.facets.nb();
        
        if(mode & MESH_REPAIR_COLOCATE) {
            repair_colocate_vertices(M, colocate_epsilon);
        }
        if(mode & MESH_REPAIR_TRIANGULATE) {
            M.facets.triangulate();
        }
        repair_remove_bad_facets(
            M, (mode & MESH_REPAIR_DUP_F) != 0
        );

        repair_connect_facets(M);
        repair_reorient_facets_anti_moebius(M);
        repair_split_non_manifold_vertices(M);

        if(
            (mode & MESH_REPAIR_RECONSTRUCT) != 0
        ) {
            double Marea = Geom::mesh_area(M,3);
            remove_small_connected_components(
                M, 
                CmdLine::get_arg_percent("co3ne:min_comp_area",Marea),
                CmdLine::get_arg_uint("co3ne:min_comp_facets")
            );
            fill_holes(
                M,
                CmdLine::get_arg_percent("co3ne:max_hole_area",Marea),
                CmdLine::get_arg_uint("co3ne:max_hole_edges")
            );
            // We do that one more time, to remove the small 
            // connected components
            // yielded by the detected non-manifold edges.
            remove_small_connected_components(
                M, 
                CmdLine::get_arg_percent("co3ne:min_comp_area",Marea),
                CmdLine::get_arg_uint("co3ne:min_comp_facets")
            );

            // We need to do that one more time after removing the
            // small component, to ensure that everything is correct.
            repair_connect_facets(M);
            repair_reorient_facets_anti_moebius(M);
            repair_split_non_manifold_vertices(M);

        }
	
        if((mode & MESH_REPAIR_QUIET) == 0) {
	    if(
		M.vertices.nb() != nb_vertices_in ||
		M.facets.nb() != nb_facets_in
	    ) {
		M.show_stats("Validate");
	    }
        }
    }

    void mesh_postprocess_RDT(
        Mesh& M
    ) {
        vector<index_t> f_is_bad(M.facets.nb(), 0);
        vector<signed_index_t> v_nb_incident(M.vertices.nb(), 0);
        detect_bad_facets(M, true, f_is_bad);
        bool changed = false;
        do {
            changed = false;
            v_nb_incident.assign(M.vertices.nb(), 0);
            for(index_t f: M.facets) {
                if(f_is_bad[f] == 0) {
                    for(index_t c: M.facets.corners(f)) {
                        ++v_nb_incident[M.facet_corners.vertex(c)];
                    }
                }
            }
            for(index_t f: M.facets) {
                if(f_is_bad[f] == 0) {
                    for(index_t c: M.facets.corners(f)) {
                        if(v_nb_incident[M.facet_corners.vertex(c)] == 1) {
                            f_is_bad[f] = 1;
                            changed = true;
                            break;
                        }
                    }
                }
            }
        } while(changed);
        M.facets.delete_elements(f_is_bad);

        repair_connect_facets(M);
        repair_reorient_facets_anti_moebius(M);
        repair_split_non_manifold_vertices(M);

        M.show_stats("Validate");
    }
    
    void mesh_reorient(Mesh& M, vector<index_t>* moebius_facets) {
        repair_reorient_facets_anti_moebius(M, moebius_facets);
    }

    void mesh_detect_colocated_vertices(
        const Mesh& M, vector<index_t>& v_colocated_index,
        double colocate_epsilon
    ) {
        Geom::colocate(
            M.vertices.point_ptr(0),
            coord_index_t(M.vertices.dimension()),
            M.vertices.nb(),
            v_colocated_index,
            colocate_epsilon
        );
    }

    void mesh_detect_isolated_vertices(
        const Mesh& M, vector<index_t>& v_is_isolated
    ) {
        v_is_isolated.assign(M.vertices.nb(),1);
        for(index_t e: M.edges) {
            v_is_isolated[M.edges.vertex(e,0)] = 0;
            v_is_isolated[M.edges.vertex(e,1)] = 0;            
        }
        for(index_t f: M.facets) {
            for(index_t lv=0; lv<M.facets.nb_vertices(f); ++lv) {
                v_is_isolated[M.facets.vertex(f,lv)] = 0;
            }
        }
        for(index_t c: M.cells) {
            for(index_t lv=0; lv<M.cells.nb_vertices(c); ++lv) {
                v_is_isolated[M.cells.vertex(c,lv)] = 0;
            }
        }
    }

    void mesh_detect_degenerate_facets(
        const Mesh& M, vector<index_t>& f_is_degenerate
    ) {
        f_is_degenerate.resize(M.facets.nb());
        for(index_t f: M.facets) {
            f_is_degenerate[f] = facet_is_degenerate(M,f);
        }
    }
}

