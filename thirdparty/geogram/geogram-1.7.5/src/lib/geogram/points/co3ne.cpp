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

#include <geogram/points/co3ne.h>
#include <geogram/points/nn_search.h>
#include <geogram/points/principal_axes.h>
#include <geogram/mesh/mesh.h>
#include <geogram/mesh/mesh_io.h>
#include <geogram/mesh/index.h>
#include <geogram/mesh/mesh_repair.h>
#include <geogram/mesh/mesh_topology.h>
#include <geogram/mesh/mesh_reorder.h>
#include <geogram/basic/geometry.h>
#include <geogram/basic/process.h>
#include <geogram/basic/assert.h>
#include <geogram/basic/progress.h>
#include <geogram/basic/command_line.h>
#include <geogram/basic/algorithm.h>
#include <geogram/basic/stopwatch.h>
#include <stack>
#include <queue>

namespace {
    using namespace GEO;

    /**
     * \brief number of elements in sine/cosine table.
     */
    static const index_t sincos_nb = 10;

    /**
     * \brief sine/cosine table.
     * \details We keep a small table of sines and cosines for 
     *  speeding up things a little bit.
     *  Table entries are as follows:
     *  - sincos_table[i][0] = sin(2*M_PI*i/(sincos_nb-1))
     *  - sincos_table[i][1] = cos(2*M_PI*i/(sincos_nb-1))
     */
    static double sincos_table[10][2] = {  
        {0,1},
        {0.642788,0.766044},
        {0.984808,0.173648},
        {0.866025,-0.5},
        {0.34202,-0.939693},
        {-0.34202,-0.939693},
        {-0.866025,-0.5},
        {-0.984808,0.173648},
        {-0.642788,0.766044},
        {-2.44929e-16,1}
    };

    /**
     * \brief Used by the algorithm that reorients normals.
     */
    struct OrientNormal {
	/**
	 * \brief OrientNormal constructor.
	 * \param[in] v_in the index of a point
	 * \param[in] dot_in the dot product between the (unit)
	 *  normal vector at \p v_in and the normal vector at 
	 *  the point that initiated propagation to \p v_in.
	 */
	OrientNormal(
	    index_t v_in, double dot_in
	) : v(v_in), dot(dot_in) {
	}

	/**
	 * \brief Compares two OrientNormal objects
	 * \retval true if \p rhs should be processed before this
	 *  OrientObject.
	 * \retval false otherwise.
	 */
	bool operator<(const OrientNormal& rhs) const {
	    return (::fabs(dot) < ::fabs(rhs.dot));
	}
	index_t v;
	double dot;
    };

    
    /************************************************************/

    /**
     * \brief Extracts a manifold surface from the set of 
     *  triangles reconstructed by Co3Ne.
     */
    class Co3NeManifoldExtraction {
    public:
        static const index_t NO_CORNER = index_t(-1);
        static const index_t NO_FACET  = index_t(-1);
        static const index_t NO_CNX    = index_t(-1);

        /**
         * \brief Initializes a new Co3NeManifoldExtraction with
         *  a list of triangles.
         * \param[in,out] target the target mesh. It needs to be already 
         *  initialized with the vertices.
         * \param[in,out] good_triangles the good triangles reconstructed
         *  by Co3Ne. They are 'stealed' by the mesh (on exit, good_triangles
         *  is empty). If some non-manifold edges are detected, then all
         *  the triangles incident to any manifold edge are ignored.
         */
        Co3NeManifoldExtraction(
            Mesh& target, 
            vector<index_t>& good_triangles
        ) : M_(target) {
            strict_ = CmdLine::get_arg_bool("co3ne:strict");
            if(strict_) {
                vector<index_t> first_triangle;
                for(index_t i=0; i<3; ++i) {
                    first_triangle.push_back(*good_triangles.rbegin());
                    good_triangles.pop_back();
                }

                M_.facets.assign_triangle_mesh(first_triangle, true);
                
                init_and_remove_non_manifold_edges();
                init_connected_components();
                add_triangles(good_triangles);
            } else {

                M_.facets.assign_triangle_mesh(good_triangles, true);
                
                init_and_remove_non_manifold_edges();
                init_connected_components();
            }
        }

        /**
         * \brief Tentatively adds triangle from the specified list.
         * \details Some geometric and topological properties are
         *  verified by connect_and_validate_triangle() before accepting
         *  the triangle.
         * \see connect_and_validate_triangle()
         */
        void add_triangles(const vector<index_t>& not_so_good_triangles) {
            bool pretty = CmdLine::get_arg_bool("log:pretty");
            
            index_t nb_triangles = not_so_good_triangles.size()/3;
            Logger::out("Co3ne") << "Tentatively add " 
                                 << nb_triangles << " triangles" << std::endl;
            vector<bool> t_is_classified(nb_triangles,false);
            bool changed = true;
            index_t max_iter = strict_ ? 5000 : 50;
            index_t iter = 0;
            bool first = true;
            while(changed && iter < max_iter) {
                if(first) {
                    CmdLine::ui_clear_line();
                } else {
                    first = false;
                }
                if(pretty) {
                    CmdLine::ui_message(
                        "o-[Manifold Rec] Iteration:" + String::to_string(iter)
                    );
                } else {
                    Logger::out("Manifold Rec")
                        << "Iteration:" << iter << std::endl;
                }
                changed = false;
                ++iter;
                for(index_t t=0; t<nb_triangles; ++t) {
                    if(!t_is_classified[t]) {
                        index_t i = not_so_good_triangles[3*t];
                        index_t j = not_so_good_triangles[3*t+1];
                        index_t k = not_so_good_triangles[3*t+2];
                        index_t new_t = add_triangle(i,j,k);
                        bool classified = false;
                        if(connect_and_validate_triangle(new_t, classified)) {
                            changed = true;
                        } else {
                            rollback_triangle();
                        }
                        if(classified) {
                            t_is_classified[t] = true;
                        }
                    }
                }
            }
            if(pretty) {
                CmdLine::ui_clear_line();            
                CmdLine::ui_message(
                    "o-[Manifold Rec] Iteration:" +
                    String::to_string(iter) + "\n"
                );
            } else {
                Logger::out("Manifold Rec")
                    << "Iteration:" << iter << std::endl;
            }
        }

    protected:

        /**
         * \brief Initializes the combinatorial data
         *  structures and deletes all facets incident
         *  to a non-manifold edge.
         */
        void init_and_remove_non_manifold_edges() {
            next_c_around_v_.assign(
                M_.facet_corners.nb(), index_t(NO_CORNER)
            );
            v2c_.assign(
                M_.vertices.nb(),index_t(NO_CORNER)
            );
            for(index_t t=0; t<M_.facets.nb(); ++t) {
                insert(t);
            }

            index_t nb_non_manifold = 0;
            vector<index_t> remove_t;
            for(index_t t=0; t<M_.facets.nb(); ++t) {
                if(!connect(t)) {
                    remove_t.resize(M_.facets.nb(),0);
                    remove_t[t] = 1;
                    ++nb_non_manifold;
                }
            }

            mesh_reorient(M_, &remove_t);

            if(remove_t.size() == 0) {
                Logger::out("Co3Ne") 
                    << "All edges are manifold and well oriented"
                    << std::endl;
            } else {
                index_t nb_remove_t = 0;
                for(index_t t=0; t<M_.facets.nb(); ++t) {
                    if(remove_t[t] != 0) {
                        ++nb_remove_t;
                    }
                }
                index_t nb_moebius = nb_remove_t - nb_non_manifold;
                Logger::out("Co3Ne") 
                    << "Removing " << nb_remove_t 
                    << " triangles ("
                    << nb_non_manifold << " non_manifold, "
                    << nb_moebius
                    << " moebius)"
                    << std::endl;
                M_.facets.delete_elements(remove_t,false);
            }

            // We need to re-compute next_c_around_v_ and v2c_
            // since all the indices changed in the mesh
            // (even if remove_t is empty, because mesh_reorient() may
            // have changed triangles orientation).
            next_c_around_v_.assign(M_.facet_corners.nb(), index_t(NO_CORNER));
            v2c_.assign(M_.vertices.nb(),index_t(NO_CORNER));
            for(index_t t=0; t<M_.facets.nb(); ++t) {
                insert(t);
            }
        }

        /**
         * \brief Tentatively connects a newly added triangle
         *  to the current mesh under construction. Accepted 
         *  triangles satisfy the following criteria:
         *  - each new triangle should be either incident to at least
         *    two edges of existing triangles, or to one existing triangle
         *    and one isolated point.
         *  - the normals to the new triangle and its neighbor should 
         *    not point to opposite directions.
         *  - inserting the new triangle should not generate 'by-excess'
         *    non-manifold vertices. A 'by-excess' non-manifold vertex
         *    has a closed loop of triangles in its neighbors plus
         *    additional triangles.
         *  - the orientation of the surface should be coherent (no Moebius
         *    strip).
         * \param[in] t index of the triangle
         * \param[out] classified true if the status of the triangle 
         *  (accepted/rejected) could be completely determined,
         *  false if its status may still change during subsequent iterations
         * \retval true if all combinatorial and geometric tests succeeded
         * \retval false otherwise
         */
        bool connect_and_validate_triangle(index_t t, bool& classified) {
            index_t adj_c[3];
            classified = false;

            //   Combinatorial test (I): tests whether the three 
            // candidate edges are manifold.
            if(!get_adjacent_corners(t,adj_c)) {
                classified = true;
                return false ;
            }

            //   Geometric test: tests whether the angles formed with
            // the candidate neighbors do not indicate degenerate sharp
            // creases.
            for(index_t i=0; i<3; ++i) {
                if(adj_c[i] != NO_CORNER) {
                    index_t t2 = c2f(adj_c[i]);
                    if(!triangles_normals_agree(t,t2)) {
                        classified = true;
                        return false;
                    }
                }
            }

            int nb_neighbors = 
                (adj_c[0] != NO_CORNER) + 
                (adj_c[1] != NO_CORNER) + 
                (adj_c[2] != NO_CORNER) ;

            // Combinatorial test (II)
            switch(nb_neighbors) {
                // If the candidate triangle is adjacent to no other
                // triangle, reject it
               case 0: {
                  return false ;
               } 
               // If the candidate triangle is adjacent to a single
               // triangle, reject it if the vertex opposite to 
               // the common edge is not isolated.
               case 1: {
                   // If not in strict mode, we reject the triangle.
                   // Experimentally, it improves the result.
                   if(!strict_) {
                       return false;
                   }
                   index_t other_vertex=index_t(-1);
                   for(index_t i=0; i<3; ++i) {
                       if(adj_c[i] != NO_CORNER) {
                           other_vertex = 
                               M_.facet_corners.vertex(
                                   M_.facets.corners_begin(t) + ((i+2)%3)
                               );
                       }
                   }
                   geo_debug_assert(other_vertex != index_t(-1));
                   // Test whether other_vertex is isolated, reject
                   // the triangle if other_vertex is NOT isolated.
                   index_t nb_incident_T = nb_incident_triangles(other_vertex);
                   geo_assert(nb_incident_T != 0); // There is at least THIS T.
                   if(nb_incident_T > 1) {
                       return false; 
                   }
               }
            }

            connect_adjacent_corners(t,adj_c);

            // Combinatorial test (III): test non-manifold vertices
            for(
                index_t c=M_.facets.corners_begin(t);
                c<M_.facets.corners_end(t); ++c
            ) {
                index_t v = M_.facet_corners.vertex(c);
                bool moebius=false;
                if(vertex_is_non_manifold_by_excess(v,moebius)) {
                    classified = true;
                    return false;
                }
                // It should not occur since we remove all Moebius configs
                // from the T3s and forbid Moebius configs when inserting
                // the T12s. However, some transient moebius configurations
                // due to triangle t may appear (since the Moebius test is
                // right after the non-manifold test).
                if(moebius) {
                    Logger::warn("Co3Ne") 
                        << "Encountered Moebius configuration" << std::endl;
                    classified = true;
                    return false;
                }
            }

            // Combinatorial test (IV): orientability
            if(!enforce_orientation_from_triangle(t)) {
                return false;
            }

            classified = true;
            return true;
        }


        /**
         * \brief Tentatively enforces mesh orientation starting from a
         *  given triangle.
         * \details The triangle \p t is rejected if it is incident to 
         *  the same connected component with two different orientations.
         * \param[in] t index of the triangle to start mesh orientation from
         * \retval true if the mesh could be coherently oriented
         * \retval false otherwise
         */
        bool enforce_orientation_from_triangle(index_t t) {

            // Index of adjacent triangle 
            // (or NO_FACET if no neighbor)
            index_t adj[3]; 

            // Index of adjacent connected component 
            // (or NO_CNX if no neighbor)
            index_t adj_cnx[3]; 

            //   Orientation of adjacent triangle relative to 
            // triangle t (or 0 if no neighbor)
            signed_index_t adj_ori[3];

            for(index_t i=0; i<3; ++i) {
                index_t c = M_.facets.corners_begin(t)+i;
                adj[i] = index_t(M_.facet_corners.adjacent_facet(c));
            }

            
            for(index_t i=0; i<3; ++i) {
                if(adj[i] == NO_FACET) {
                    adj_ori[i] = 0;
                    adj_cnx[i] = NO_CNX;
                } else {
                    adj_ori[i] = 
                        (triangles_have_same_orientation(t,adj[i])) ? 1 : -1;
                    adj_cnx[i] = cnx_[adj[i]];
                }
            }

            //  If in the neighborhood the same connected component appears
            // with two opposite orientations, then connecting the triangle
            // would create a Moebius strip (the triangle is rejected)
            for(index_t i=0; i<3; ++i) {
                if(adj[i] != NO_FACET) {
                    for(index_t j=i+1; j<3; ++j) {
                        if(
                            adj_cnx[j] == adj_cnx[i] && 
                            adj_ori[j] != adj_ori[i]
                        ) {
                            return false;
                        }
                    }
                }
            }

            //  The triangle is accepted, 
            // now reorient all the connected components and the
            // triangle coherently.

            // Find the largest component incident to t
            index_t largest_neigh_comp = NO_CNX;
            for(index_t i=0; i<3; ++i) {
                if(
                    adj_cnx[i] != NO_CNX && (
                        largest_neigh_comp == NO_CNX ||
                        cnx_size_[adj_cnx[i]] >
                        cnx_size_[adj_cnx[largest_neigh_comp]]
                    )
                ) {
                    
                    largest_neigh_comp = i;
                }
            }
            geo_assert(largest_neigh_comp != NO_CNX);
    
            // Orient t like the largest incident component
            index_t comp = adj_cnx[largest_neigh_comp];

            cnx_.resize(std::max(t+1, cnx_.size()));
            cnx_[t] = comp;
            ++cnx_size_[comp];
            if(adj_ori[largest_neigh_comp] == -1) {
                flip_triangle(t);
                for(index_t i=0; i<3; ++i) {
                    adj_ori[i] = -adj_ori[i];
                }
            }

            // Merge (and reorient if need be) all the other incident 
            // components
            for(index_t i=0; i<3; ++i) {
                if(
                    i != largest_neigh_comp && 
                    adj[i] != NO_FACET && cnx_[adj[i]] != comp
                ) {
                    merge_connected_component(
                        adj[i], comp, (adj_ori[i] == -1)
                    );
                }
            }

            return true;
        }


        /**
         * \brief Adds a new triangle to the surface and to the
         *  combinatorial data structure.
         * \param[in] i first index of the triangle
         * \param[in] j second index of the triangle
         * \param[in] k third index of the triangle
         */
        index_t add_triangle(index_t i, index_t j, index_t k) {
            index_t result = M_.facets.create_triangle(i,j,k);
            next_c_around_v_.push_back(index_t(NO_CORNER));
            next_c_around_v_.push_back(index_t(NO_CORNER));
            next_c_around_v_.push_back(index_t(NO_CORNER));            
            insert(result);
            return result;
        }

        /**
         * \brief Removes the latest triangle from both
         *  the mesh and the combinatorial data structure.
         */
        void rollback_triangle() {
            index_t t = M_.facets.nb()-1;
            remove(t);
            M_.facets.pop();
        }


        /**
         * \brief Inverts the orientation of a triangle.
         * \param[in] t the index of the triangle to be flipped.
         */
        void flip_triangle(index_t t) {

            // Remove t from the additional combinatorial data structure
            // (it is both simpler and more efficient to do that 
            //  than updating it).
            remove(
                t,
                false // disconnect is set to false because 
                      // we will re-insert t right after.
            ); 

            index_t c1 = M_.facets.corners_begin(t);
            index_t c2 = c1+1;
            index_t c3 = c2+1;
            index_t v1 = M_.facet_corners.vertex(c1);
            index_t f1 = M_.facet_corners.adjacent_facet(c1);
            index_t f2 = M_.facet_corners.adjacent_facet(c2);
            index_t v3 = M_.facet_corners.vertex(c3);

            M_.facet_corners.set_vertex(c1,v3);
            M_.facet_corners.set_adjacent_facet(c1,f2);
            M_.facet_corners.set_adjacent_facet(c2,f1);
            M_.facet_corners.set_vertex(c3,v1);

            // Re-insert t into the additional combinatorial data structure.
            insert(t); 
        }

        /**
         * \brief Inserts a triangle of the mesh into the data structures
         *  used for topology checks.
         * \param[in] t index of the triangles to be inserted
         * \pre \p t is a valid triangle index in the mesh
         */
        void insert(index_t t) {
            for(
                index_t c=M_.facets.corners_begin(t);
                c<M_.facets.corners_end(t); ++c
            ) {
                index_t v = M_.facet_corners.vertex(c);
                if(v2c_[v] == NO_CORNER) {
                    v2c_[v] = c;
                    next_c_around_v_[c] = c;
                } else {
                    next_c_around_v_[c] = next_c_around_v_[v2c_[v]];
                    next_c_around_v_[v2c_[v]] = c;
                }
            }
        }

        /**
         * \brief Removes a triangle of the mesh from the data structures
         *  used for topology/combinatorial checks.
         * \param[in] t index of the triangles to be removed
         * \param[in] disconnect if true, connections from the neighbors
         *  to t are set to -1 (facet_corners.adjacent_facet).
         * \pre \p t is a valid triangle index in the mesh
         */
        void remove(index_t t, bool disconnect=true) {
            if(disconnect) {
                for(
                    index_t c=M_.facets.corners_begin(t);
                    c<M_.facets.corners_end(t); ++c
                ) {
                    
                    // Disconnect facet-facet link that point to t
                    index_t t2 = M_.facet_corners.adjacent_facet(c);
                    if(t2 != NO_FACET) {
                        for(
                            index_t c2=M_.facets.corners_begin(index_t(t2)); 
                            c2<M_.facets.corners_end(index_t(t2)); 
                            ++c2
                        ) {
                            if(
                                M_.facet_corners.adjacent_facet(c2) == t
                            ) {
                                M_.facet_corners.set_adjacent_facet(
                                    c2,NO_FACET
                                );
                            }
                        }
                    }
                }
            }
             

            for(
                index_t c=M_.facets.corners_begin(t);
                c<M_.facets.corners_end(t); ++c
            ) {
                // Remove t from combinatorial data structures
                index_t v = M_.facet_corners.vertex(c);
                if(next_c_around_v_[c] == c) {
                    v2c_[v] = NO_CORNER;
                } else {
                    index_t c_pred = next_c_around_v_[c];
                    while(next_c_around_v_[c_pred] != c) {
                        c_pred = next_c_around_v_[c_pred];
                    }
                    next_c_around_v_[c_pred] = next_c_around_v_[c];
                    v2c_[v] = c_pred;
                }
            }
        }

        /**
         * \brief Gets the number of triangles incident
         *  to a vertex.
         * \param[in] v index of the vertex
         * \return the number of triangles incident to \p v
         */
        index_t nb_incident_triangles(index_t v) const {
            index_t result = 0;
            index_t c = v2c_[v];
            do {
                ++result;
                c = next_c_around_v_[c];
            } while(c != v2c_[v]);
            return result;
        }
        
        /**
         * \brief Tests whether a given vertex is non-manifold
         *  by excess.
         * \details A vertex is non-manifold by-excess if its
         *  set of incident triangles contains a closed loop
         *  of triangles and additional triangles.
         * \param[in] v index of the vertex to be tested
         * \retval true if \p v is non-manifold by excess
         * \retval false otherwise 
         */
        bool vertex_is_non_manifold_by_excess(index_t v, bool& moebius) {
            index_t nb_v_neighbors = nb_incident_triangles(v);
            index_t c = v2c_[v];
            do {
                index_t loop_size=0;
                index_t c_cur = c ;
                do {
                    ++loop_size;
                    if(c_cur == NO_CORNER) {
                        break;
                    }
                    if(loop_size > 100) {
                        // Probably Moebious strip or something...
                        moebius = true;
                        break;
                    }
                    c_cur = next_around_vertex_unoriented(v,c_cur);
                } while(c_cur != c);

                if(c_cur == c && loop_size < nb_v_neighbors) {
                    return true;
                }
                c = next_c_around_v_[c]; 
            } while(c != v2c_[v]);

            return false;
        }

        /**
         * \brief Gets the next corner around a vertex from a given
         *  corner.
         * \details This function works even for a mesh that has triangles
         *  that are not coherently oriented. In other words, for two
         *  corners c1, c2, if we have:
         *   - v1 = facet_corners.vertex(c1)
         *   - v2 = facet_corners.vertex(
         *       c1,facets.next_corner_around_facet(c2f(c1),c1)
         *   )
         *   - w1 = facet_corners.vertex(c2)
         *   - w2 = facet_corners.vertex(
         *         c2,facets.next_corner_around_facet(c2f(c2),c2)
         *   )
         *  then we can have:
         *   - v1=w2 and v2=w1 (as usual) or:
         *   - v1=v2 and w1=w2 ('inverted' configuration)
         * \param[in] v the vertex
         * \param[in] c1 a corner incident to \p v or pointing to \p v
         * \return another corner incident to the \p v
         */
        index_t next_around_vertex_unoriented(
            index_t v, index_t c1
        ) const {
            index_t f1 = c2f(c1);
            index_t v1 = M_.facet_corners.vertex(c1);
            index_t v2 = M_.facet_corners.vertex(
                M_.facets.next_corner_around_facet(f1,c1)
            );

            geo_debug_assert(v1 == v || v2 == v);

            index_t f2 = M_.facet_corners.adjacent_facet(c1);
            if(f2 != NO_FACET) {
                for(
                    index_t c2 = M_.facets.corners_begin(f2); 
                    c2 < M_.facets.corners_end(f2); 
                    ++c2
                ) {
                    index_t w1 = M_.facet_corners.vertex(c2);
                    index_t w2 = M_.facet_corners.vertex(
                        M_.facets.next_corner_around_facet(f2,c2)
                    ); 
                    if(
                        (v1 == w1 && v2 == w2) ||
                        (v1 == w2 && v2 == w1)
                    ) {
                        if(w2 == v) {
                            return M_.facets.next_corner_around_facet(f2,c2);
                        } else {
                            geo_debug_assert(w1 == v);
                            return M_.facets.prev_corner_around_facet(f2,c2);
                        }
                    }
                }
            }
            return NO_CORNER;
        }

        /**
         * \brief Gets the three corners adjacent to a triangle.
         * \details This function works even for a mesh that has triangles
         *  that are not coherently oriented. In other words, for two
         *  corners c1, c2, if we have:
         *   - v1 = facet_corners.vertex(c1)
         *   - v2 = facet_corners.vertex(
         *       c1,facets.next_corner_around_facet(c2f(c1),c1))
         *   - w1 = facet_corners.vertex(c2)
         *   - w2 = facet_corners.vertex(
         *       c2,facets.next_corner_around_facet(c2f(c2),c2))
         *  then c1 and c2 are adjacent if we have:
         *   - v1=w2 and v2=w1 (as usual) or:
         *   - v1=v2 and w1=w2 ('inverted' configuration)
         * \param[in] t1 index of the triangle
         * \param[out] adj_c index of the adjacent corners 
         *  (array of 3 integers). Each entry contains a valid corner index
         *  or NO_CORNER if the corresponding edge is on the border.
         * \retval true if the three edges are manifold
         * \retval false otherwise (and then \p adj_c contains undefined
         *  values).
         */
        bool get_adjacent_corners(index_t t1, index_t* adj_c) {
            for(
                index_t c1 = M_.facets.corners_begin(t1);
                c1<M_.facets.corners_end(t1); ++c1
            ) {
                index_t v2 = M_.facet_corners.vertex(
                    M_.facets.next_corner_around_facet(t1,c1)
                );

                *adj_c = NO_CORNER;

                // Traverse the circular incident edge list
                index_t c2=next_c_around_v_[c1];
                while(c2 != c1) {
                    index_t t2 = c2f(c2);
                    index_t c3 = M_.facets.prev_corner_around_facet(t2,c2);
                    index_t v3 = M_.facet_corners.vertex(c3);
                    if(v3 == v2) {
                        // Found an adjacent edge
                        if(*adj_c == NO_CORNER) {
                            *adj_c = c3;
                            geo_debug_assert(c3 != c1);
                        } else {
                            // If there was already an adjacent edge,
                            // then this is a non-manifold configuration
                            return false;
                        }
                    }

                    // Check with the other (wrong) orientation
                    c3 = M_.facets.next_corner_around_facet(t2,c2);
                    v3 = M_.facet_corners.vertex(c3);
                    if(v3 == v2) {
                        // Found an adjacent edge
                        if(*adj_c == NO_CORNER) {
                            *adj_c = c2;
                            geo_debug_assert(c2 != c1);
                        } else {
                            // If there was already an adjacent edge,
                            // then this is a non-manifold configuration
                            return false;
                        }
                    }
                    c2 = next_c_around_v_[c2];
                } 
                ++adj_c;
            }
            return true;
        }

        /**
         * \brief Tentatively connect a triangle of the mesh with its
         *   neighbors.
         * \details This function is independent of triangles orientations,
         *  see get_adjacent_corners().
         * \param[in] t index of the triangle to be connected
         * \param[in] adj_c an array of three integers that indicate
         *  for each corner of the triangle the index of the adjacent
         *  corner or NO_CORNER if the corner is on the border.
         */
        void connect_adjacent_corners(index_t t, index_t* adj_c) {
            for(index_t i=0; i<3; ++i) {
                if(adj_c[i] != NO_CORNER) {
                    index_t c = M_.facets.corners_begin(t)+i;
                    M_.facet_corners.set_adjacent_facet(c, c2f(adj_c[i]));
                    M_.facet_corners.set_adjacent_facet(adj_c[i], t);
                }
            }
        }

        /**
         * \brief Tentatively connect a triangle of the mesh with its
         *   neighbors.
         * \details This function is independent of triangles orientations,
         *  see get_adjacent_corners().
         * \param[in] t index of the triangle to be connected
         * \retval false if the connection would have created non-manifold
         *  edges
         * \retval true otherwise
         */
        bool connect(index_t t) {
            index_t adj_c[3];
            if(!get_adjacent_corners(t,adj_c)) {
                return false;
            }
            connect_adjacent_corners(t, adj_c);
            return true;
        }

        /**
         * \brief Gets a facet index by corner index.
         * \details for a triangulated mesh, indexing is 
         *  implicit, and we do not need to store a c2f array.
         * \param[in] c corner index
         * \return the index of the facet incident to c
         */
        index_t c2f(index_t c) const {
            geo_debug_assert(c != NO_CORNER);
            geo_debug_assert(c < M_.facet_corners.nb());
            return c/3;
        }


        /**
         * \brief Tests whether two triangles have the 
         *  same orientation.
         * \param[in] t1 first triangle
         * \param[in] t2 second triangle
         * \retval true if \p t1 and \p t2 have the same
         *  orientation
         * \retval false otherwise
         * \pre \p t1 and \p t2 share an edge
         */
        bool triangles_have_same_orientation(
            index_t t1,
            index_t t2
        ) {
            index_t c1 = M_.facets.corners_begin(t1);
            index_t i1 = M_.facet_corners.vertex(c1);
            index_t j1 = M_.facet_corners.vertex(c1+1);
            index_t k1 = M_.facet_corners.vertex(c1+2);
            
            index_t c2 = M_.facets.corners_begin(t2);
            index_t i2 = M_.facet_corners.vertex(c2);
            index_t j2 = M_.facet_corners.vertex(c2+1);
            index_t k2 = M_.facet_corners.vertex(c2+2);

            if(
                (i1==i2 && j1==j2) ||
                (i1==k2 && j1==i2) ||
                (i1==j2 && j1==k2) ||
                (k1==k2 && i1==i2) ||
                (k1==j2 && i1==k2) ||
                (k1==i2 && i1==j2) ||
                (j1==j2 && k1==k2) ||
                (j1==i2 && k1==j2) ||
                (j1==k2 && k1==i2) 
            ) {
                return false;
            }

            return true;
        }


        /**
         * \brief Tests whether the normals of two triangles that
         *  share an edge 'agree', i.e. whether they do not form
         *  a too sharp angle.
         * \param[in] t1 index of the first triangle
         * \param[in] t2 index of the second triangle
         * \retval true if the normals of both triangles do not 
         *  point in opposite directions
         * \retval false otherwise
         * \pre the two triangles are incident to the same edge 
         *  (they have two vertices in common)
         */
        bool triangles_normals_agree(
            index_t t1,
            index_t t2
        ) const {
            const vec3* points = 
                reinterpret_cast<const vec3*>(M_.vertices.point_ptr(0));

            index_t c1 = M_.facets.corners_begin(t1);
            index_t i1 = M_.facet_corners.vertex(c1);
            index_t j1 = M_.facet_corners.vertex(c1+1);
            index_t k1 = M_.facet_corners.vertex(c1+2);
            
            index_t c2 = M_.facets.corners_begin(t2);
            index_t i2 = M_.facet_corners.vertex(c2);
            index_t j2 = M_.facet_corners.vertex(c2+1);
            index_t k2 = M_.facet_corners.vertex(c2+2);

            vec3 n1 = normalize(
                cross(
                    points[j1] - points[i1],
                    points[k1] - points[i1]
                    )
                );

            vec3 n2 = normalize(
                cross(
                    points[j2] - points[i2],
                    points[k2] - points[i2]
                    )
                );
            
            double d = dot(n1,n2);
            // Test for combinatorial orientation,
            // if t1 and t2 have opposite orientation,
            // then we flip one of the normals (i.e., 
            // we simply change the sign of the dot product).
            if(
                (i1==i2 && j1==j2) ||
                (i1==k2 && j1==i2) ||
                (i1==j2 && j1==k2) ||
                (k1==k2 && i1==i2) ||
                (k1==j2 && i1==k2) ||
                (k1==i2 && i1==j2) ||
                (j1==j2 && k1==k2) ||
                (j1==i2 && k1==j2) ||
                (j1==k2 && k1==i2) 
            ) {
                d = -d;
            }
            return (d > -0.8);
        }

        /**
         * \brief Merges two connected components.
         * \details The connected component incident to \p t
         *  is replaced with \p comp2.
         * \param [in] t index of a triangle incident
         *  to the first connected component
         * \param [in] comp2 index of the second connected
         *  component
         * \param [in] flip if true, flip the triangles
         * \pre At least one of the triangles adjacent to 
         *  \p t (directly or not) is incident to 
         *  component \p comp2
         */
        void merge_connected_component(
            index_t t,
            index_t comp2,
            bool flip
        ) {
            geo_assert(comp2 != cnx_[t]);

            std::stack<index_t> S;
            index_t comp1 = cnx_[t];


            cnx_[t] = comp2;
            --cnx_size_[comp1];
            ++cnx_size_[comp2];
            if(flip) {
                flip_triangle(t);
            }
            S.push(t);
            while(!S.empty()) {
                index_t t1 = S.top(); 
                S.pop();
                for(
                    index_t c = M_.facets.corners_begin(t1);
                    c < M_.facets.corners_end(t1); ++c
                ) {
                    index_t t2 = M_.facet_corners.adjacent_facet(c);
                    if(t2 != NO_FACET && cnx_[t2] == comp1) {
                        cnx_[t2] = comp2;
                        --cnx_size_[comp1];
                        ++cnx_size_[comp2];
                        if(flip) {
                            flip_triangle(t2);
                        }
                        S.push(t2);
                    }
                }
            }
            geo_assert(cnx_size_[comp1] == 0);
        }

        /**
         * \brief Initializes the date structures
         *  that represent the connected components.
         * \details This function computes cnx_ and
         *  cnx_size_. The array cnx_[f] gives for each
         *  facet f the index of the connected component
         *  that contains f, and the array cnx_size_[comp]
         *  gives for each connected component comp the
         *  number of facets in comp.
         */
        void init_connected_components() {
            cnx_.assign(M_.facets.nb(), index_t(NO_CNX));
            cnx_size_.clear();
            for(index_t t=0; t<M_.facets.nb(); ++t) {
                if(cnx_[t] == NO_CNX) {
                    index_t cnx_id  = cnx_size_.size();
                    index_t nb = 0;
                    std::stack<index_t> S;
                    S.push(t);
                    cnx_[t] = cnx_id;
                    ++nb;
                    while(!S.empty()) {
                        index_t t2 = S.top(); 
                        S.pop();
                        for(
                            index_t c=M_.facets.corners_begin(t2); 
                            c<M_.facets.corners_end(t2); ++c
                        ) {
                            index_t t3 = M_.facet_corners.adjacent_facet(c);
                            if(t3 != NO_FACET && cnx_[t3] != cnx_id) {
                                geo_assert(cnx_[t3] == NO_CNX);
                                cnx_[t3] = cnx_id;
                                ++nb;
                                S.push(t3);
                            }
                        }
                    }
                    cnx_size_.push_back(nb);
                }
            }
            Logger::out("Co3Ne") 
                << "Found " << cnx_size_.size() << " connected components" 
                << std::endl;
        }

    private:
        Mesh& M_;

        /**
         * \brief For each corner, next_c_around_v_[c]
         * chains the circular list of corners 
         * incident to the same corner as c.
         */
        vector<index_t> next_c_around_v_;

        /**
         * \brief For each vertex v, v2c_[v] contains a
         *  corner incident to v, or NO_VERTEX if v is
         *  isolated.
         */
        vector<index_t> v2c_;


        /**
         * \brief For each triangle t, cnx_[t] contains
         *  the index of the connected component of the 
         *  mesh incident to t.
         */
        vector<index_t> cnx_;

        /**
         * \brief For each connected component C, 
         *  cnx_size_[C] contains the number of 
         *  facets in C.
         */
        vector<index_t> cnx_size_;

        /**
         * \brief In strict mode, each inserted triangle
         *  is checked for non-manifold configuration.
         *  In non-strict mode, only T2 and T1 triangles are
         *  tested (those seen from only 2 or only 1 Voronoi
         *  cell), T3 triangles are inserted without test.
         */
        bool strict_;
    }; 

    /************************************************************/

    /**
     * \brief Comparator class for sorting facets.
     */
    class CompareTriangles {
    public:
        /**
         * \brief Constructs a new CompareFacets.
         * \param[in] triangles a const reference to a vector 
         *  of indices triplets
         */
        explicit CompareTriangles(const vector<index_t>& triangles) :
            triangles_(triangles) {
        }

        /**
         * \brief Tests the lexicographic order of two facets by their indices.
         * \param[in] f1 index of the first facet
         * \param[in] f2 index of the second facet
         * \return true if facet \p f1 is before facet \p f2 according to
         *  the lexicographic order of its vertices, false otherwise.
         */
        bool is_before(index_t f1, index_t f2) const {
            for(index_t c=0; c<3; c++) {
                index_t v1 = triangles_[3*f1+c];
                index_t v2 = triangles_[3*f2+c];
                if(v1 > v2) {
                    return false;
                }
                if(v1 < v2) {
                    return true;
                }
            }
            return false;
        }

        /**
         * \brief Tests whether two facets are identical.
         * \param[in] f1 index of the first facet
         * \param[in] f2 index of the second facet
         * \return true if facets \p f1 and \p f2 have the same
         *  vertices, false otherwise
         */
        bool is_same(index_t f1, index_t f2) const {
            for(index_t c=0; c<3; c++) {
                index_t v1 = triangles_[3*f1+c];
                index_t v2 = triangles_[3*f2+c];
                if(v1 != v2) {
                    return false;
                }
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
        const vector<index_t>& triangles_;
    };


    /**
     * \brief Splits the raw list of triangles reconstructed 
     *  by the Co3Ne algorithm into two lists, good triangles
     *  and "not so good" triangles.
     * \details The triangles that appear 3 times (seen from 3 
     *  different Voronoi cells) are the good ones, else they
     *  are the "not so good" ones.
     * \param[in,out] triangles the input list of triangles. It
     *   is modified by the algorithm (it is reordered).
     * \param[out] good_triangles the good triangles
     * \param[out] not_so_good_triangles the not-so-good triangles
     */
    static void co3ne_split_triangles_list(
        vector<index_t>& triangles,
        vector<index_t>& good_triangles,
        vector<index_t>& not_so_good_triangles
    ) {
        index_t nb_triangles = triangles.size()/3;
        
        // Step 1: normalize vertices order
        for(index_t i=0; i<triangles.size(); i+=3) {
            index_t* ptr = &triangles[i];
            std::sort(ptr, ptr+3);
        }

        // Step 2: sort the triangles in lexicographic order
        vector<index_t> t_sort(nb_triangles);
        for(index_t t=0; t<nb_triangles; ++t) {
            t_sort[t] = t ;
        }
        CompareTriangles compare_triangles(triangles);
        GEO::sort(t_sort.begin(), t_sort.end(), compare_triangles);
        

        // Step 3: select the triangles that appear exactly 3 times
        index_t if1 = 0;
        while(if1 < nb_triangles) {
            index_t if2 = if1 + 1;
            while(
                if2 < nb_triangles &&
                compare_triangles.is_same(t_sort[if1], t_sort[if2])
            ) {
                if2++;
            }

            index_t t = t_sort[if1];
            if(if2 - if1 == 3) {
                good_triangles.push_back(triangles[3*t]);
                good_triangles.push_back(triangles[3*t+1]);
                good_triangles.push_back(triangles[3*t+2]);
            } else if(if2 - if1 <= 2) {
                not_so_good_triangles.push_back(triangles[3*t]);
                not_so_good_triangles.push_back(triangles[3*t+1]);
                not_so_good_triangles.push_back(triangles[3*t+2]);
            }
            if1 = if2;
        }
    }

    /************************************************************/

    /**
     * \brief Used internally by the reconstruction algorithm.
     *  Co3NeRestrictedVoronoiDiagram computes the restricted
     *  Voronoi diagram of a set of disks.
     *
     * \details Given a point set with normals and a radius, this class
     * computes the intersection between the Voronoi diagram of
     * the points and the disks centered on the points and
     * orthogonal to the normals.
     */
    class Co3NeRestrictedVoronoiDiagram {
    public:
        /**
         * \brief Stores a 3D point and the combinatorial information
         *  (index of the adjacent seed). The combinatorial information
         *  is used to reconstruct the triangles at the end of the
         *  algorithm.
         */
        class Vertex {
        public:
            /**
             * \brief Constructs a new uninitialized Vertex.
             */
            Vertex() {
            }

            /**
             * \brief Constructs a Vertex from a 3d point.
             */
            Vertex(const vec3& v) :
                point_(v),
                adjacent_seed_(-1) {
            }

            /**
             * \brief Gets the 3d point associated with this vertex.
             * \return a const reference to the 3d point
             */
            const vec3& point() const {
                return point_;
            }

            /**
             * \brief Gets the 3d point associated with this vertex.
             * \return a const reference to the 3d point
             */
            vec3& point() {
                return point_;
            }

            /**
             * \brief Gets the index of the adjacent seed associated with
             *  this vertex.
             * \details Each vertex stores combinatorial information, i.e.
             *  the index of the adjacent Voronoi seed accros the edge
             *  starting from this vertex
             * \return the index of the adjacent Voronoi seed
             */
            signed_index_t adjacent_seed() const {
                return adjacent_seed_;
            }

            /**
             * \brief Sets the index of the adjacent seed associated with
             *  this vertex.
             * \details Each vertex stores combinatorial information, i.e.
             *  the index of the adjacent Voronoi seed accros the edge
             *  starting from this vertex
             * \param[in] x the index of the adjacent Voronoi seed
             */
            void set_adjacent_seed(signed_index_t x) {
                adjacent_seed_ = x;
            }

        private:
            vec3 point_;
            signed_index_t adjacent_seed_;
        };

        /**
         * \brief Internal representation of the polygons, that represent
         *  the intersection between the disks and the Voronoi cells.
         */
        class Polygon {
        public:
            /**
             * \brief Creates a new uninitialized polygon with a given
             *  number of vertices.
             * \param[in] size number of vertices
             */
            Polygon(index_t size) :
                vertices_(size) {
            }

            /**
             * \brief Gets the number of vertices.
             * \return the number of vertices of this Polygon
             */
            index_t nb_vertices() const {
                return vertices_.size();
            }

            /**
             * \brief Adds a new vertex to this Polygon.
             * \param[in] v the vertex to be added.
             */
            void add_vertex(const Vertex& v) {
                vertices_.push_back(v);
            }

            /**
             * \brief Gets a Vertex by its index.
             * \param[in] i the index of the Vertex
             * \return a reference to the Vertex
             */
            Vertex& vertex(index_t i) {
                return vertices_[i];
            }

            /**
             * \brief Gets a Vertex by its index.
             * \param[in] i the index of the Vertex
             * \return a const reference to the Vertex
             */
            const Vertex& vertex(index_t i) const {
                return vertices_[i];
            }

            /**
             * \brief Gets the index of the next vertex around
             *  the polygon.
             * \param[in] i index of the vertex
             * \return index of the next vertex (successor of \p i)
             *  around the Polygon.
             */
            index_t next_vertex(index_t i) const {
                return (i == nb_vertices() - 1) ? 0 : i + 1;
            }

            /**
             * \brief Removes all the vertices.
             */
            void clear() {
                vertices_.resize(0);
            }

            /**
             * \brief Swaps the vertices of this Polygon with
             *  the vertices of another polygon.
             * \param[in] P the other polygon
             */
            void swap(Polygon& P) {
                vertices_.swap(P.vertices_);
            }

        private:
            vector<Vertex> vertices_;
        };

        /**
         * \brief Constructs a new uninitialized Co3NeRestrictedVoronoiDiagram.
         */
        Co3NeRestrictedVoronoiDiagram() :
            nb_points_(0),
            p_(nullptr),
            p_stride_(0),
            n_(nullptr),
            n_stride_(0),
            radius_(0.0),
            NN_(NearestNeighborSearch::create(3)),
            sqROS_(0.0),
            nb_neighbors_(0)
        {
        }

        /**
         * \brief Co3NeRestrictedVoronoiDiagram destructor.
         */
        ~Co3NeRestrictedVoronoiDiagram() {
            clear();
        }

	
        /**
         * \brief Sets or resets exact mode for nearest neighbor search
         * (default is exact).
         * \details Nearest neighbor search can be exact or approximate.
         * Note that approximate mode cannot be used for the
         * final reconstruction phase (that needs exact combinatorics),
         * but it may speedup the smoothing phase.
         * \param[in] x if set, nearest neighbors search are exact, else they
         *  are approximate
         */
        void set_exact(bool x) {
            NN_->set_exact(x);
        }

        /**
         * \brief Clears this Co3NeRestrictedVoronoiDiagram.
         */
        void clear() {
            NN_.reset();
            nb_points_ = 0;
            p_ = nullptr;
            p_stride_ = 0;
            n_ = nullptr;
            n_stride_ = 0;
            nb_neighbors_ = 0;
        }

        /**
         * \brief Initializes this Co3NeRestrictedVoronoiDiagram from a
         *  pointset stored in a mesh.
         * \details If the mesh \p M has normals, then they are used.
         * \param[in] M the pointset
         */
        void init(Mesh& M) {
            geo_assert(M.vertices.dimension() >= 3);
            geo_assert(M.vertices.dimension() == 3 || NN_->stride_supported());
            double* normals_pointer = nullptr;
            {
                Attribute<double> normal;
                normal.bind_if_is_defined(M.vertices.attributes(), "normal");
                if(normal.is_bound() && normal.dimension() == 3) {
                    normals_pointer = &normal[0];
                }
            }

            if(normals_pointer == nullptr) {
                init(
                    M.vertices.nb(),
                    M.vertices.point_ptr(0), M.vertices.dimension(),
                    nullptr, 0
                );
            } else {
                init(
                    M.vertices.nb(),
                    M.vertices.point_ptr(0), M.vertices.dimension(),
                    normals_pointer, 3
                );
            }
        }

        /**
         * \brief Initializes this Co3NeRestrictedVoronoiDiagram from an
         *  array of points and an array of normals.
         * \param[in] nb_points_in number of points
         * \param[in] p pointer to the coordinates of the points
         * \param[in] p_stride number of doubles between two consecutive points
         * \param[in] n pointer to the coordinates of the normals
         * \param[in] n_stride number of doubles between two consecutive normals
         */
        void init(
            index_t nb_points_in,
            double* p, index_t p_stride,
            double* n, index_t n_stride
        ) {
            nb_points_ = nb_points_in;
            p_ = p;
            p_stride_ = p_stride;
            n_ = n;
            n_stride_ = n_stride;
            NN_->set_points(nb_points(), p_, p_stride_);
        }

        /**
         * \brief Reconstructs the nearest neighbors search data
         * structure.
         * \details This function needs to be called whenever
         * the point set changes.
         */
        void update() {
            init(nb_points_, p_, p_stride_, n_, n_stride_);
        }

        /**
         * \brief Sets the radius of the circles used to determine
         *  points adjacencies.
         * \param[in] r the radius of the circles
         */
        void set_circles_radius(double r) {
            radius_ = r;
            sqROS_ = 4.0 * radius_ * radius_;  // squared radius of security
            // when a neighbor is further away than ROS, then it cannot
            // clip a circle of radius r
        }

        /**
         * \brief Gets the number of points.
         * \return the number of points
         */
        index_t nb_points() const {
            return nb_points_;
        }

        /**
         * \brief Gets a point by its index.
         * \param[in] i index of the point
         * \return a const reference to the point
         */
        const vec3& point(index_t i) const {
            geo_debug_assert(i < nb_points());
            return *(vec3*) (p_ + i * p_stride_);
            // Yes I know, this is a bit ugly...
        }

        /**
         * \brief Gets a normal by point index.
         * \param[in] i index of the point
         * \return a const reference to the normal
         *  associated with the point
         */
        const vec3& normal(index_t i) const {
            geo_debug_assert(n_ != nullptr);
            geo_debug_assert(i < nb_points());
            return *(vec3*) (n_ + i * n_stride_);
            // Yes I know, this is a bit ugly...
        }

        /**
         * \brief Sets the normal associated wigth a point.
         * \param[in] i the index of the point
         * \param[in] N the normal
         */
        void set_normal(index_t i, const vec3& N) const {
            geo_debug_assert(n_ != nullptr);
            geo_debug_assert(i < nb_points());
            double* n = n_ + i * n_stride_;
            n[0] = N.x;
            n[1] = N.y;
            n[2] = N.z;
        }

        /**
         * \brief Computes the intersection between a polygon
         *  and the halfspace determined by the bisector
         *  of two points.
         * \param[in,out] Ping polygon to be clipped
         * \param[in,out] Pong a temporary work variable provided
         *  by the caller
         * \param[in] pi first extremity of the bisector
         * \param[in] pj second extremity of the bisector
         * \param[in] j index of the second extremity of the
         *  bisector (used to store the combinatorial information).
         */
        static void clip_polygon_by_bisector(
            Polygon& Ping, Polygon& Pong,
            const vec3& pi, const vec3& pj, index_t j
        ) {
            if(Ping.nb_vertices() == 0) {
                return;
            }
            Pong.clear();

            vec3 n(
                pi.x - pj.x,
                pi.y - pj.y,
                pi.z - pj.z
            );

            // Compute d = n . m, where n is the
            // normal vector of the bisector [pi,pj]
            // and m twice the middle point of the bisector.
            double d =
                n.x * (pi.x + pj.x) +
                n.y * (pi.y + pj.y) +
                n.z * (pi.z + pj.z);

            // The predecessor of the first vertex is the last vertex
            index_t prev_k = Ping.nb_vertices() - 1;
            const Vertex* prev_vk = &(Ping.vertex(prev_k));

            // We compute:
            //    prev_l = prev_vk . n
            double prev_l = dot(prev_vk->point(), n);

            // We compute:
            //    side1(pi,pj,q) = sign(2*q.n - n.m) = sign(2*l - d)
            Sign prev_status = geo_sgn(2.0 * prev_l - d);

            for(index_t k = 0; k < Ping.nb_vertices(); k++) {
                const Vertex* vk = &(Ping.vertex(k));

                // We compute: l = vk . n
                double l = dot(vk->point(), n);

                // We compute:
                //   side1(pi,pj,q) = sign(2*q.n - n.m) = sign(2*l - d)
                Sign status = geo_sgn(2.0 * l - d);

                // If status of edge extremities differ,
                // then there is an intersection.
                if(status != prev_status && (prev_status != 0)) {

                    // Compute lambda1 and lambda2, the
                    // barycentric coordinates of the intersection I
                    // in the segment [prev_vk vk]
                    // Note that d and l (used for the predicates)
                    // are reused here.
                    double denom = 2.0 * (prev_l - l);
                    double lambda1, lambda2;

                    // Shit happens ! [Forrest Gump]
                    if(::fabs(denom) < 1e-20) {
                        lambda1 = 0.5;
                        lambda2 = 0.5;
                    } else {
                        lambda1 = (d - 2.0 * l) / denom;
                        // Note: lambda2 is also given
                        // by (2.0*l2-d)/denom
                        // (but 1.0 - lambda1 is a bit
                        //  faster to compute...)
                        lambda2 = 1.0 - lambda1;
                    }
                    Vertex V;
                    V.point().x =
                        lambda1 * prev_vk->point().x + lambda2 * vk->point().x;
                    V.point().y =
                        lambda1 * prev_vk->point().y + lambda2 * vk->point().y;
                    V.point().z =
                        lambda1 * prev_vk->point().z + lambda2 * vk->point().z;
                    if(status > 0) {
                        V.set_adjacent_seed(prev_vk->adjacent_seed());
                    } else {
                        V.set_adjacent_seed(signed_index_t(j));
                    }
                    Pong.add_vertex(V);
                }
                if(status > 0) {
                    Pong.add_vertex(*vk);
                }
                prev_vk = vk;
                prev_status = status;
                prev_k = k;
                prev_l = l;
            }
            Ping.swap(Pong);
        }

        /**
         * \brief Computes the squared maximum distance between a point
         *  and the vertices of a polygon.
         * \param[in] p the point
         * \param[in] P the polygon
         * \return the maximum squared distance between \p p and the vertices
         *  of \p P
         */
        static double squared_radius(const vec3& p, const Polygon& P) {
            double result = 0.0;
            for(index_t i = 0; i < P.nb_vertices(); i++) {
                result = std::max(result, distance2(p, P.vertex(i).point()));
            }
            return result;
        }

        /**
         * \brief Computes a polygon that approximates a disk centered
         *  at a point and orthogonal to its normal vector.
         * \param[in] i index of the point
         * \param[out] P an approximation of the circle centered
         *  at point \p i with normal vector \p N. The radius is
         *  defined by set_circles_radius().
         * \param[in] N normal vector
         */
        void get_circle(index_t i, Polygon& P, const vec3& N) const {
            P.clear();
            const vec3& pi = point(i);
            vec3 U = Geom::perpendicular(N);
            U = normalize(U);
            vec3 V = cross(N, U);
            V = normalize(V);
            // We use a table for sine and cosine for speeding up things
            // a little bit (especially on some cell phones / handheld devices
            // that do not have a FPU).
/*
            const index_t nb = 10;
            for(index_t k=0; k<nb; ++k) {
                double alpha = 2.0 * M_PI * double(k) / double(nb - 1);
                double s = sin(alpha);
                double c = cos(alpha);
                vec3 p = pi + c * radius_ * U + s * radius_ * V;
                P.add_vertex(p);
            }
*/

            for(index_t k = 0; k < sincos_nb; ++k) {
                double s = sincos_table[k][0];
                double c = sincos_table[k][1];                
                vec3 p = pi + c * radius_ * U + s * radius_ * V;
                P.add_vertex(p);
            }

        }

        /**
         * \brief Nearest neighbor search
         * \param[in] i index of the query point
         * \param[out] neigh array of nb signed_index_t
         * \param[out] sq_dist array of nb doubles
         * \param[in] nb number of neighbors to be searched
         */
        void get_neighbors(
            index_t i,
            index_t* neigh,
            double* sq_dist,
            index_t nb
        ) const {
            return NN_->get_nearest_neighbors(
                nb, i, neigh, sq_dist
            );
        }

        /**
         * \brief Nearest neighbor search
         * \param[in] i index of the query point
         * \param[out] neigh vector of signed_index_t
         * \param[out] sq_dist array of nb doubles
         * \param[in] nb number of neighbors to be searched
         */
        void get_neighbors(
            index_t i,
            vector<index_t>& neigh,
            vector<double>& sq_dist,
            index_t nb
        ) const {
            neigh.resize(nb);
            sq_dist.resize(nb);
            get_neighbors(i, neigh.data(), sq_dist.data(), nb);
        }

        /**
         * \brief Computes a Restricted Voronoi Cell (RVC), i.e.
         *  the intersection between a disk and the Voronoi cell
         *  of a point.
         * \details The temporary work variables provided by the caller
         *  make it possible to reuse memory accros multiple calls to this
         *  function and thus avoid multiple dynamic memory allocations.
         * \param[in] i index of the point that determines the Voronoi cell.
         * \param[out] P result
         * \param[in] Q work temporary variable provided by caller
         * \param[in] neighbor work temporary variable provided by caller
         * \param[in] squared_dist work temporary variable provided by caller
         */
        void get_RVC(
            index_t i, Polygon& P,
            Polygon& Q,
            vector<index_t>& neighbor,
            vector<double>& squared_dist
        ) const {
            neighbor.resize(0);
            squared_dist.resize(0);
            get_RVC(i, normal(i), P, Q, neighbor, squared_dist);
        }

        /**
         * \brief Computes a Restricted Voronoi Cell (RVC), i.e.
         *  the intersection between a disk and the Voronoi cell
         *  of a point.
         * \details The temporary work variables provided by the caller
         *  make it possible to reuse memory accros multiple calls to this
         *  function and thus avoid multiple dynamic memory allocations.
         * \param[in] i index of the point that determines the Voronoi cell.
         * \param[in] N normal vector at point \p i
         * \param[out] P result
         * \param[in] Q work temporary variable, provided by caller
         * \param[in] neighbor initial neighbor indices
         *  if size is not zero, contains (previously computed)
         *  neighbor indices.
         * \param[in] squared_dist initial neighbor squared distances
         *  if size is not zero, contains (previously computed)
         *  neighbor squared distances.
         */
        void get_RVC(
            index_t i, const vec3& N, Polygon& P,
            Polygon& Q,
            vector<index_t>& neighbor,
            vector<double>& squared_dist
        ) const {
            get_circle(i, P, N);

            index_t nb_neigh = std::min(index_t(nb_points() - 1), index_t(20));
            index_t jj = 0;

            // just in case, limit to 1000 neighbors.
            index_t max_neigh = std::min(index_t(1000), nb_points() - 1);

            while(nb_neigh < max_neigh) {
                if(P.nb_vertices() < 3) {
                    return;
                }
                if(neighbor.size() < nb_neigh) {
                    get_neighbors(i, neighbor, squared_dist, nb_neigh);
                }
                while(jj < nb_neigh && squared_dist[jj] < 1e-30) {
                    jj++;
                }
                while(jj < nb_neigh) {
                    if(squared_dist[jj] > sqROS_) {
                        return;
                    }
                    index_t j = neighbor[jj];
                    double Rk = squared_radius(point(i), P);
                    if(squared_dist[jj] > 4.0 * Rk) {
                        return;
                    }
                    clip_polygon_by_bisector(P, Q, point(i), point(j), j);
                    jj++;
                }
                if(nb_neigh > 3) {
                    nb_neigh += nb_neigh / 3;
                } else {
                    nb_neigh++;
                }
                nb_neigh = std::min(nb_neigh, nb_points()-1);
            }
        }

        /**
         * \brief Gets the number of neighbors, used for nearest neighbors
         *  queries.
         * \return the number of neighbors
         */
        index_t nb_neighbors() const {
            return std::min(nb_neighbors_,nb_points()-1);
        }

        /**
         * \brief Sets the number of neighbors, used for nearest neighbors
         *  queries.
         * \param[in] x the number of neighbors
         */
        void set_nb_neighbors(index_t x) {
            nb_neighbors_ = x;
        }

    private:
        friend class Co3Ne;

        index_t nb_points_;
        double* p_;
        index_t p_stride_;
        double* n_;
        index_t n_stride_;
        double radius_;

        NearestNeighborSearch_var NN_;

        double sqROS_;
        index_t nb_neighbors_;
    };

    /************************************************************************/

    class Co3Ne;

    /**
     * \brief Determines what a thread does in
     *  the multithreaded implementation of the Co3Ne reconstruction algorithm.
     */
    enum Co3NeMode {
        CO3NE_NONE,    /**< uninitialized */
        CO3NE_NORMALS, /**< estimate normals in pointset */
        CO3NE_SMOOTH,  /**< smooth the pointset */
        CO3NE_RECONSTRUCT, /**< reconstruct the triangles */
        CO3NE_NORMALS_AND_RECONSTRUCT
        /**< combined normal estimation and reconstruction */
    };

    /**
     * \brief Internal implementation class for Co3Ne.
     *  Encapsulates the work of one thread.
     */
    class Co3NeThread : public Thread {
    public:
        /**
         * \brief Creates a new Co3NeThread
         * \param[in] master the Co3Ne this thread depends on
         * \param[in] from index of the first point to process
         * \param[in] to one position past the index of the last point
         */
        Co3NeThread(
            Co3Ne* master,
            index_t from, index_t to
        ) :
            master_(master),
            from_(from),
            to_(to) {
            mode_ = CO3NE_NONE;
        }

        /**
         * \brief Sets the mode of this thread
         * \param[in] m the mode, that determines whether normal computation,
         *  smoothing or reconstruction is performed
         */
        void set_mode(Co3NeMode m) {
            mode_ = m;
        }

        /**
         * \brief Does the actual computation of this thread.
         * \details The actual computation is determined by set_mode().
         */
	void run() override {
            switch(mode_) {
                case CO3NE_NORMALS:
                    run_normals();
                    break;
                case CO3NE_SMOOTH:
                    run_smooth();
                    break;
                case CO3NE_RECONSTRUCT:
                    run_reconstruct();
                    break;
                case CO3NE_NORMALS_AND_RECONSTRUCT:
                    run_normals_and_reconstruct();
                    break;
                case CO3NE_NONE:
                    break;
            }
        }

        /**
         * \brief Gets the reconstructed triangles.
         * \return a reference to a vector of indices
         */
        vector<index_t>& triangles() {
            return triangles_;
        }


        /**
         * \brief Gets the number of reconstructed triangles.
         * \return the number of reconstructed triangles
         */
        index_t nb_triangles() const {
            return triangles_.size()/3;
        }

    protected:
        /**
         * \brief Estimates the normals in the pointset.
         */
        void run_normals();

        /**
         * \brief Smoothes the pointset.
         */
        void run_smooth();

        /**
         * \brief Reconstructs the triangles.
         */
        void run_reconstruct();

        /**
         * \brief Estimates the normals and reconstructs the triangles.
         */
        void run_normals_and_reconstruct();

    private:
        Co3Ne* master_;
        index_t from_;
        index_t to_;
        Co3NeMode mode_;
        PrincipalAxes3d least_squares_normal_;
        vector<index_t> triangles_;
    };

    /************************************************************************/

    /**
     * \brief Reconstructs a mesh from a set of point with
     * the Co3Ne algorithm (concurrent co-cones).
     * This class also implements point set smoothing and point
     * set normal estimation.
     */
    class Co3Ne {
    public:
        /**
         * \brief Constructs a new Co3Ne.
         * \param[in] M the pointset
         */
        Co3Ne(Mesh& M) :
            mesh_(M) {
            // TODO: interlace threads (more cache friendly)
            RVD_.init(mesh_);
            index_t nb = Process::maximum_concurrent_threads();
            thread_.clear();
            index_t batch_size = RVD_.nb_points() / nb;
            index_t cur = 0;
            index_t remaining = RVD_.nb_points();
            for(index_t i = 0; i < nb; i++) {
                index_t this_batch_size = batch_size;
                if(i == nb - 1) {
                    this_batch_size = remaining;
                }
                thread_.push_back(
                    new Co3NeThread(
                        this, cur, cur + this_batch_size
                    )
                );
                cur += this_batch_size;
                remaining -= this_batch_size;
            }
            geo_assert(remaining == 0);

            // TODO: pass it as an argument and let Vorpaline's main.cpp
            // communicate with CmdLine.
            double alpha = CmdLine::get_arg_double("co3ne:max_N_angle");
            alpha = alpha * M_PI / 180.0;
            set_max_angle(alpha);
        }

	/**
	 * \brief Runs the threads.
	 */
	void run_threads() {
	    Process::run_threads(thread_);
	}
	
        /**
         * \brief Estimates the normals of the point set.
         * \details They are stored in the "normal" vertex attribute.
         * \param[in] nb_neighbors number of neighbors to be
         *  used for normal estimation
         */
        void compute_normals(index_t nb_neighbors) {
            Attribute<double> normals;
            normals.bind_if_is_defined(mesh_.vertices.attributes(), "normal");
            if(!normals.is_bound()) {
                normals.create_vector_attribute(
                    mesh_.vertices.attributes(), "normal", 3
                );
            }
            RVD_.init(mesh_);
            RVD_.set_nb_neighbors(nb_neighbors);
            for(index_t t = 0; t < thread_.size(); t++) {
                thread_[t]->set_mode(CO3NE_NORMALS);
            }
            run_threads();
        }

	static inline double cos_angle(
	    Attribute<double>& normal, index_t v1, index_t v2
	) {
	    vec3 V1(normal[3*v1],normal[3*v1+1],normal[3*v1+2]);
	    vec3 V2(normal[3*v2],normal[3*v2+1],normal[3*v2+2]);
	    return Geom::cos_angle(V1,V2);
	}

	static inline void flip(Attribute<double>& normal, index_t v) {
	    normal[3*v]   = -normal[3*v];
	    normal[3*v+1] = -normal[3*v+1];
	    normal[3*v+2] = -normal[3*v+2];	    
	}
	
	/**
	 * \brief Tentatively enforces a coherent orientation of normals
	 *  using a breadth-first traveral of the K-nearest-neighbor graph.
	 * \retval true if normals where computed
	 * \retval false otherwise (when the user pushes the cancel button).
	 */
	bool reorient_normals() {
	    Attribute<double> normal;
            normal.bind_if_is_defined(mesh_.vertices.attributes(), "normal");
	    geo_assert(normal.is_bound());

	    //  To resist noisy inputs, propagation is prioritized to the points
	    // that have smallest normal deviations.
	    
	    std::priority_queue<OrientNormal> S;
	    vector<index_t> neighbors(RVD_.nb_neighbors()); 
	    vector<double> dist(RVD_.nb_neighbors());

	    index_t nb=0;
	    ProgressTask progress("Reorient");

	    try {
		std::vector<bool> visited(mesh_.vertices.nb(), false);
		for(index_t v=0; v<mesh_.vertices.nb(); ++v) {
		    if(!visited[v]) {
			S.push(OrientNormal(v,0.0));
			visited[v] = true;
			while(!S.empty()) {
			    OrientNormal top = S.top();
			    ++nb;
			    progress.progress(nb*100/mesh_.vertices.nb());
			    S.pop();
			    if(top.dot < 0.0) {
				flip(normal,top.v);
			    }
			    RVD_.get_neighbors(
				top.v,
				neighbors.data(),dist.data(),RVD_.nb_neighbors()
			    );
			    for(index_t i=0; i<RVD_.nb_neighbors(); ++i) {
				index_t neigh = neighbors[i];
				if(!visited[neigh]) {
				    visited[neigh] = true;
				    double dot =
					cos_angle(normal, top.v, neigh);
				    S.push(OrientNormal(neigh,dot));
				}
			    }
			}
		    }
		}
	    } catch(const TaskCanceled&) {
		return false;
	    }
	    return true;
	} 
	
        /**
         * \brief Smoothes a point set by projection
         * onto the nearest neighbors best
         * approximating planes.
         * \param[in] nb_neighbors number of neighbors to be
         *  used for best approximating plane estimation
         */
        void smooth(index_t nb_neighbors) {
            new_vertices_.resize(mesh_.vertices.nb() * 3);
            RVD_.set_nb_neighbors(nb_neighbors);
            for(index_t t = 0; t < thread_.size(); t++) {
                thread_[t]->set_mode(CO3NE_SMOOTH);
            }
            run_threads();
            /*
              // TODO: once 'steal-arg' mode works for vertices,
              // we can use this one.
            if(RVD_.p_stride_ == 3) {
                MeshMutator::vertices(mesh_).swap(new_vertices_);
            } else */ {
                index_t idx = 0;
                for(index_t i = 0; i < mesh_.vertices.nb(); i++) {
                    double* p = mesh_.vertices.point_ptr(i);
                    for(coord_index_t c = 0; c < 3; c++) {
                        p[c] = new_vertices_[idx];
                        idx++;
                    }
                }
            }
        }

        /**
         * \brief This function needs to be called after the
         * last iteration of smoothing.
         * \details Deallocates the temporary
         *  variables used for smoothing.
         */
        void end_smooth() {
            new_vertices_.clear();
        }

        /**
         * \brief Reconstructs a mesh from a point set.
         * \details If the mesh has a "normal" vertex attribute,
         *  then the existing normals are used, else normals are estimated.
         * \param[in] r maximum distance used to determine
         *  points adjacencies.
         */
        void reconstruct(double r) {
            bool has_normals = false;
            {
                Attribute<double> normal;
                normal.bind_if_is_defined(mesh_.vertices.attributes(),"normal");
                has_normals = (
                    normal.is_bound() && normal.dimension() == 3
                );
            }

	    ProgressTask progress("reconstruct",100);

            if(has_normals) {
                Stopwatch W("Co3Ne recons");
                RVD_.set_circles_radius(r);
                for(index_t t = 0; t < thread_.size(); t++) {
                    thread_[t]->set_mode(CO3NE_RECONSTRUCT);
                    thread_[t]->triangles().clear();
                }
		progress.progress(1);
                run_threads();
		progress.progress(50);
            } else {
                Stopwatch W("Co3Ne recons");                
                Logger::out("Co3Ne")
                    << "using combined \'normals and reconstruct\'"
                    << std::endl;
                RVD_.set_nb_neighbors(
                    CmdLine::get_arg_uint("co3ne:nb_neighbors")
                );
                RVD_.set_circles_radius(r);
                for(index_t t = 0; t < thread_.size(); t++) {
                    thread_[t]->set_mode(CO3NE_NORMALS_AND_RECONSTRUCT);
                    thread_[t]->triangles().clear();
                }
		progress.progress(1);		
                run_threads();
		progress.progress(50);		
            }

            {
                Stopwatch W("Co3Ne manif.");
                RVD_.clear();  // reclaim memory used by ANN

                index_t nb_triangles = 0;
                for(index_t t = 0; t < thread_.size(); t++) {
                    nb_triangles += thread_[t]->nb_triangles();
                }

                Logger::out("Co3Ne") << "Raw triangles: "
                                     << nb_triangles
                                     << std::endl;

                vector<index_t> raw_triangles; 
                raw_triangles.reserve(nb_triangles * 3);
                for(index_t th = 0; th < thread_.size(); th++) {
                    vector<index_t>& triangles = thread_[th]->triangles();
                    raw_triangles.insert(
                        raw_triangles.end(), 
                        triangles.begin(), triangles.end()
                        );
                    thread_[th]->triangles().clear();
                }

                if(CmdLine::get_arg_bool("dbg:co3ne")) {
                    Logger::out("Co3Ne") << ">> co3ne_raw.geogram"
                                         << std::endl;
                    Mesh M;
                    M.vertices.assign_points(
                        mesh_.vertices.point_ptr(0),
                        mesh_.vertices.dimension(),
                        mesh_.vertices.nb()
                    );
                    M.facets.assign_triangle_mesh(raw_triangles, false);
                    M.vertices.set_dimension(3);
                    mesh_save(M, "co3ne_raw.geogram");
                }
                
                vector<index_t> good_triangles;
                vector<index_t> not_so_good_triangles;
                co3ne_split_triangles_list(
                    raw_triangles, good_triangles, not_so_good_triangles
                ); 


                if(CmdLine::get_arg_bool("dbg:co3ne")) {
                    Logger::out("Co3Ne") << ">> co3ne_T3.geogram"
                                         << std::endl;
                    Mesh M;
                    M.vertices.assign_points(
                        mesh_.vertices.point_ptr(0),
                        mesh_.vertices.dimension(),
                        mesh_.vertices.nb()
                    );
                    M.facets.assign_triangle_mesh(good_triangles, false);
                    M.vertices.set_dimension(3);
                    mesh_save(M, "co3ne_T3.geogram");
                }

                if(CmdLine::get_arg_bool("dbg:co3ne")) {
                    Logger::out("Co3Ne") << ">> co3ne_T12.geogram"
                                         << std::endl;
                    Mesh M;
                    M.vertices.assign_points(
                        mesh_.vertices.point_ptr(0),
                        mesh_.vertices.dimension(),
                        mesh_.vertices.nb()
                    );
                    M.facets.assign_triangle_mesh(not_so_good_triangles, false);
                    M.vertices.set_dimension(3);
                    mesh_save(M, "co3ne_T12.geogram");
                }

		progress.progress(53);		
                
                Co3NeManifoldExtraction manifold_extraction(
                    mesh_, good_triangles
                );

		progress.progress(55);				

                if(CmdLine::get_arg_bool("co3ne:T12")) {
                    manifold_extraction.add_triangles(not_so_good_triangles);
                }

		progress.progress(57);

                mesh_reorient(mesh_);

		progress.progress(60);		

                if(CmdLine::get_arg_bool("dbg:co3ne")) {
                    Logger::out("Co3Ne") << ">> co3ne_manif.geogram"
                                         << std::endl;
                    mesh_save(mesh_, "co3ne_manif.geogram");
                }                
            }

            if(CmdLine::get_arg_bool("co3ne:repair")) {
                Stopwatch W("Co3Ne post.");
                mesh_repair(mesh_,
                    MeshRepairMode(
                        MESH_REPAIR_DEFAULT | MESH_REPAIR_RECONSTRUCT
                    )
                );
                if(CmdLine::get_arg_bool("dbg:co3ne")) {
                    Logger::out("Co3Ne") << ">> co3ne_post.geogram"
                                         << std::endl;
                    mesh_save(mesh_, "co3ne_post.geogram");
                }                
            }

	    progress.progress(100);		
	    
            Logger::out("Topology") 
                << "nb components=" << mesh_nb_connected_components(mesh_)
                << " nb borders=" <<  mesh_nb_borders(mesh_)
                << std::endl;

        }

        /**
         * \brief Gets the Co3NeRestrictedVoronoiDiagram associated
         *  with this Co3Ne.
         * \return a reference to the Co3NeRestrictedVoronoiDiagram
         */
        Co3NeRestrictedVoronoiDiagram& RVD() {
            return RVD_;
        }

        /**
         * \brief Sets a point
         * \param[in] i the index of the point
         * \param[in] P the new geometric location at the point
         */
        void set_point(index_t i, const vec3& P) {
            geo_debug_assert(new_vertices_.size() > 3 * i + 2);
            new_vertices_[3 * i] = P.x;
            new_vertices_[3 * i + 1] = P.y;
            new_vertices_[3 * i + 2] = P.z;
        }

        /**
         * \brief Sets a normal vector
         * \param[in] i the index of the point
         * \param[in] N the new normal vector associated with the point
         */
        void set_normal(index_t i, const vec3& N) {
            RVD_.set_normal(i, N);
        }

        /**
         * \brief Sets the maximum angle for determining admissible triangles.
         * \details Admissible triangles have a deviation between their normals
         *  and the normals estimated in the pointset smaller than a given
         *  threshold \p alpha.
         * \param[in] alpha the maximum normal angle deviation
         */
        void set_max_angle(double alpha) {
            min_cos_angle_ = ::cos(alpha);
        }

        Mesh& mesh() {
            return mesh_;
        }
        
    private:
        Mesh& mesh_;
        vector<double> new_vertices_;
        Co3NeRestrictedVoronoiDiagram RVD_;
        TypedThreadGroup<Co3NeThread> thread_;
        double min_cos_angle_;
    };

    /************************************************************************/

    void Co3NeThread::run_normals() {
        Co3NeRestrictedVoronoiDiagram& RVD = master_->RVD();
        index_t nb_neigh = RVD.nb_neighbors();
        vector<index_t> neigh(nb_neigh);
        vector<double> sq_dist(nb_neigh);

        for(index_t i = from_; i < to_; i++) {
            RVD.get_neighbors(
                i, neigh, sq_dist, nb_neigh
            );
            least_squares_normal_.begin();
            for(index_t jj = 0; jj < neigh.size(); jj++) {
                least_squares_normal_.add_point(RVD.point(neigh[jj]));
            }
            least_squares_normal_.end();
            master_->set_normal(i, least_squares_normal_.normal());
        }
    }

    void Co3NeThread::run_smooth() {
        Co3NeRestrictedVoronoiDiagram& RVD = master_->RVD();
        index_t nb_neigh = RVD.nb_neighbors();
        vector<index_t> neigh(nb_neigh);
        vector<double> sq_dist(nb_neigh);

        for(index_t i = from_; i < to_; i++) {
            RVD.get_neighbors(
                i, neigh, sq_dist, nb_neigh
            );
            least_squares_normal_.begin();
            for(index_t jj = 0; jj < neigh.size(); jj++) {
                least_squares_normal_.add_point(RVD.point(neigh[jj]));
            }
            least_squares_normal_.end();
            vec3 N = normalize(least_squares_normal_.normal());
            vec3 g = least_squares_normal_.center();
            vec3 d = RVD.point(i) - g;
            d -= dot(d, N) * N;
            master_->set_point(i, g + d);
        }
    }

    void Co3NeThread::run_reconstruct() {
        Co3NeRestrictedVoronoiDiagram& RVD = master_->RVD();
        vector<index_t> neigh(100);
        vector<double> sq_dist(100);
        Co3NeRestrictedVoronoiDiagram::Polygon P(100);
        Co3NeRestrictedVoronoiDiagram::Polygon Q(100);

        for(index_t i = from_; i < to_; i++) {
            RVD.get_RVC(i, P, Q, neigh, sq_dist);
            for(index_t v1 = 0; v1 < P.nb_vertices(); v1++) {
                index_t v2 = P.next_vertex(v1);
                signed_index_t j = P.vertex(v1).adjacent_seed();
                signed_index_t k = P.vertex(v2).adjacent_seed();
                if(
                    j >= 0 && k >= 0 && j != k
                ) {
                    triangles_.push_back(i);
                    triangles_.push_back(index_t(j));
                    triangles_.push_back(index_t(k));
                }
            }
        }
    }

    void Co3NeThread::run_normals_and_reconstruct() {

        Attribute<double> normal;
        if(CmdLine::get_arg_bool("co3ne:use_normals")) {
            Process::enter_critical_section();
            normal.bind_if_is_defined(
                master_->mesh().vertices.attributes(), "normal"
            );
            Process::leave_critical_section();
        }
        
        std::ofstream RVD_file; 
        bool debug_RVD = false;
        if(
            CmdLine::get_arg_bool("dbg:co3neRVD")
        ) {
            if(CmdLine::get_arg_bool("sys:multithread")) {
                Logger::warn("Co3Ne") 
                    << "dbg:Co3NeRVD cannot work in multithread mode"
                    << std::endl;
                Logger::warn("Co3Ne") 
                    << "use sys:multithread=false"
                    << std::endl;
            } else {
                Logger::out("Co3Ne") << "Saving RVD in co3neRVD.obj" 
                                     << std::endl;
                RVD_file.open("co3neRVD.obj");
                debug_RVD=true;
            }
        }
        index_t cur_v = 0;
        index_t tcount = 0;

        Co3NeRestrictedVoronoiDiagram& RVD = master_->RVD();
        index_t nb_neigh = RVD.nb_neighbors();
        vector<index_t> neigh(100);
        vector<double> sq_dist(100);
        Co3NeRestrictedVoronoiDiagram::Polygon P(100);
        Co3NeRestrictedVoronoiDiagram::Polygon Q(100);

        for(index_t i = from_; i < to_; i++) {

            vec3 N;
            if(normal.is_bound()) {
                RVD.get_neighbors(
                    i, neigh, sq_dist, nb_neigh
                );
                N = vec3(normal[3*i], normal[3*i+1], normal[3*i+2]);
            } else {
                RVD.get_neighbors(
                    i, neigh, sq_dist, nb_neigh
                );
                least_squares_normal_.begin();
                for(index_t jj = 0; jj < neigh.size(); jj++) {
                    least_squares_normal_.add_point(RVD.point(neigh[jj]));
                }
                least_squares_normal_.end();
                N = least_squares_normal_.normal();
            }
            
            RVD.get_RVC(i, N, P, Q, neigh, sq_dist);
            if(debug_RVD) {
                for(index_t v = 0; v < P.nb_vertices(); ++v) {
                    RVD_file << "v "
                        << P.vertex(v).point().x
                        << " "
                        << P.vertex(v).point().y
                        << " "
                        << P.vertex(v).point().z
                        << std::endl;
                }
                RVD_file << "f ";
                for(index_t v = 0; v < P.nb_vertices(); ++v) {
                    ++cur_v;
                    RVD_file << cur_v << " ";
                }
                RVD_file << std::endl;
                RVD_file << "#" << i << " ";
                for(index_t v1 = 0; v1 < P.nb_vertices(); ++v1) {
                    RVD_file << P.vertex(v1).adjacent_seed() << " ";
                }
                RVD_file << std::endl;
            }
            for(index_t v1 = 0; v1 < P.nb_vertices(); v1++) {
                index_t v2 = P.next_vertex(v1);
                signed_index_t j = P.vertex(v1).adjacent_seed();
                signed_index_t k = P.vertex(v2).adjacent_seed();
                if(
                    j >= 0 && k >= 0 && j != k
                ) {
                    ++tcount;
                    triangles_.push_back(i);
                    triangles_.push_back(index_t(j));
                    triangles_.push_back(index_t(k));
                }
            }
        }

        if(normal.is_bound()) {
            Process::enter_critical_section();
            normal.unbind();
            Process::leave_critical_section();
        }
    }
}

/****************************************************************************/

namespace GEO {

    void Co3Ne_smooth(Mesh& M, index_t nb_neighbors, index_t nb_iterations) {
        Co3Ne co3ne(M);
        try {
            ProgressTask progress("Smoothing", nb_iterations);
            for(index_t i = 0; i < nb_iterations; i++) {
                co3ne.smooth(nb_neighbors);
                if(i != nb_iterations - 1) {
                    co3ne.RVD().update();
                }
                progress.next();
            }
            co3ne.end_smooth();
        }
        catch(const TaskCanceled&) {
        }
    }

    bool Co3Ne_compute_normals(Mesh& M, index_t nb_neighbors, bool reorient) {
        {
            Attribute<double> normal;
            normal.bind_if_is_defined(M.vertices.attributes(), "normal");
            if(!normal.is_bound()) {
               normal.create_vector_attribute(
                  M.vertices.attributes(), "normal", 3
               );
            }
        }
        Co3Ne co3ne(M);
	Logger::out("Co3Ne") << "Computing normals" << std::endl; 
        co3ne.compute_normals(nb_neighbors);
	if(reorient) {
	    Logger::out("Co3Ne") << "Orienting normals" << std::endl; 
	    if(!co3ne.reorient_normals()) {
		return false;
	    }
	}
	return true;
    }

    void Co3Ne_reconstruct(Mesh& M, double radius) {
        Co3Ne co3ne(M);
	co3ne.reconstruct(radius);
    }

    void Co3Ne_smooth_and_reconstruct(
        Mesh& M, index_t nb_neighbors, index_t nb_iterations, double radius
    ) {
        Stopwatch W("Co3Ne total");
        
        if(CmdLine::get_arg_bool("co3ne:use_normals")) {
            Attribute<double> normal;
            normal.bind_if_is_defined(M.vertices.attributes(), "normal");
            if(normal.is_bound() && normal.dimension() == 3) {
                Logger::out("Co3Ne") << "Using existing normal attribute"
                                     << std::endl;
            } else {
                Logger::out("Co3Ne") << "No \'normal\' vertex attribute found"
                                      << std::endl;
                Logger::out("Co3Ne") << "(estimating normals)"
                                      << std::endl;
            }
        }

        
        Co3Ne co3ne(M);
        if(nb_iterations != 0) {
            try {
                co3ne.RVD().set_exact(false);
                ProgressTask progress("Co3Ne smooth", nb_iterations);
                for(index_t i = 0; i < nb_iterations; i++) {
                    co3ne.smooth(nb_neighbors);
                    co3ne.RVD().update();
                    progress.next();
                }
                co3ne.end_smooth();
            }
            catch(const TaskCanceled&) {
                // TODO_CANCEL
            }
        }
        Logger::out("Co3Ne") << "Reconstruction..." << std::endl;
        co3ne.RVD().set_exact(true);
        co3ne.reconstruct(radius);
    }
}

