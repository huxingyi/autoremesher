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

#ifndef GEOGRAM_VORONOI_GENERIC_RVD
#define GEOGRAM_VORONOI_GENERIC_RVD

#include <geogram/basic/common.h>
#include <geogram/basic/numeric.h>
#include <geogram/voronoi/generic_RVD_utils.h>
#include <geogram/voronoi/RVD_callback.h>
#include <geogram/numerics/predicates.h>
#include <geogram/mesh/index.h>
#include <geogram/basic/geometry_nd.h>
#include <geogram/basic/process.h>
#include <geogram/basic/attributes.h>
#include <geogram/basic/argused.h>

#include <deque>
#include <algorithm>
#include <iostream>

/**
 * \file geogram/voronoi/generic_RVD.h
 * \brief Generic implementation of restricted Voronoi diagrams.
 * \note This file contains functions and classes used by the 
 *  internal implementation of GEO::GenericVoronoiDiagram. 
 *  They are not meant to be used directly by client 
 *  code.
 */

namespace GEOGen {

    /**
     * \brief Computes the intersection between a surface (Mesh) and a
     *  Voronoi diagram (dual of a Delaunay).
     * \details The surface may be embedded in nD
     * (the Voronoi diagram is then of dimension n).
     * \note This is an internal implementation class, not meant to
     *  be used directly, use GEO::RestrictedVoronoiDiagram instead.
     */
    template <index_t DIM>
    class RestrictedVoronoiDiagram {

        /** \brief This class type */
        typedef RestrictedVoronoiDiagram<DIM> thisclass;

    public:
        /**
         * \brief Gets the dimension
         */
        static coord_index_t dimension() {
            return DIM;
        }

        /**
         * \brief Used to allocate the generated points.
         */
        typedef GEOGen::PointAllocator PointAllocator;

        /**
         * \brief Internal representation of vertices.
         */
        typedef GEOGen::Vertex Vertex;

        /**
         * \brief Internal representation of polygons.
         */
        typedef GEOGen::Polygon Polygon;

        /**
         * \brief Internal representation of volumetric cells.
         */
        typedef GEOGen::ConvexCell Polyhedron;

        /********************************************************************/

        /**
         * \brief Constructs a new RestrictedVoronoiDiagram.
         * \param[in] delaunay the Delaunay triangulation
         * \param[in] mesh the input mesh
         */
        RestrictedVoronoiDiagram(
            Delaunay* delaunay,
            GEO::Mesh* mesh
        ) :
            mesh_(mesh),
            delaunay_(delaunay),
            intersections_(DIM),
            symbolic_(false),
            check_SR_(true),
            exact_(false)
        {
            delaunay_nn_ = dynamic_cast<GEO::Delaunay_NearestNeighbors*>(
                delaunay_
            );
            dimension_ = DIM;
            facets_begin_ = UNSPECIFIED_RANGE;
            facets_end_ = UNSPECIFIED_RANGE;
            tets_begin_ = UNSPECIFIED_RANGE;
            tets_end_ = UNSPECIFIED_RANGE;
            connected_components_priority_ = false;
            facet_seed_marking_ = nullptr;
            connected_component_changed_ = false;
            current_connected_component_ = 0;
            cur_stamp_ = -1;
            current_facet_ = GEO::max_index_t();
            current_seed_ = GEO::max_index_t();
            current_polygon_ = nullptr;
            current_tet_ = GEO::max_index_t();
            current_polyhedron_ = nullptr;
        }

        /**
         * \brief Sets traveral priority.
         * \details If connected_components_priority is set,
         * then the connected components of the
         * restricted Voronoi cells will be traversed
         * one by one.
         */
        void set_connected_components_priority(bool x) {
            connected_components_priority_ = x;
        }


	/**
	 * \brief Tests whether connected components priority is
	 *  set.
         * \details If connected_components_priority is set,
         *  then the connected components of the
         *  restricted Voronoi cells will be traversed
         *  one by one.
	 * \retval true if connected components priority is used.
	 * \retval false otherwise.
	 */
	bool connected_components_priority() const {
	    return connected_components_priority_;
	}
	
        /**
         * \brief Gets the input mesh.
         */
        const GEO::Mesh* mesh() const {
            return mesh_;
        }

        /**
         * \brief Gets the input mesh.
         */
        GEO::Mesh* mesh() {
            return mesh_;
        }

        /**
         * \brief Gets the Delaunay triangulation.
         */
        const Delaunay* delaunay() const {
            return delaunay_;
        }

        /**
         * \brief Gets the Delaunay triangulation.
         */
        Delaunay* delaunay() {
            return delaunay_;
        }

        /**
         * \brief Sets the Delaunay triangulation.
         */
        void set_delaunay(Delaunay* delaunay) {
            delaunay_ = delaunay;
            delaunay_nn_ = dynamic_cast<GEO::Delaunay_NearestNeighbors*>(
                delaunay_
            );
        }

        /**
         * \brief Sets the input mesh.
         */
        void set_mesh(GEO::Mesh* mesh) {
            mesh_ = mesh;
        }

        /**
         * \brief Sets the facets range.
         * \details Computations can be restricted to a contiguous facet range.
         * \param[in] facets_begin first facet in the range.
         * \param[in] facets_end one position past the last facet in the range.
         */
        void set_facets_range(index_t facets_begin, index_t facets_end) {
            geo_debug_assert(facets_end >= facets_begin);
            facets_begin_ = facets_begin;
            facets_end_ = facets_end;
        }

        /**
         * \brief Sets the tetrahedra range.
         * \details Computations can be restricted to a contiguous
         *  tetrahedra range.
         * \param[in] tets_begin first tetrahedron in the range.
         * \param[in] tets_end one position past the last
         *  tetrahedron in the range.
         */
        void set_tetrahedra_range(index_t tets_begin, index_t tets_end) {
            geo_debug_assert(tets_end >= tets_begin);
            tets_begin_ = tets_begin;
            tets_end_ = tets_end;
        }

        /**
         * \brief Gets the number of facets in the current range.
         * \see set_facets_range()
         */
        index_t nb_facets_in_range() const {
            return facets_end_ - facets_begin_;
        }

        /**
         * \brief Gets the number of tetrahedra in the current range.
         * \see set_tetrahedra_range()
         */
        index_t nb_tetrahedra_in_range() const {
            return tets_end_ - tets_begin_;
        }

        /**
         * \brief Gets the index of the mesh facet currently processed.
         * \details Can be used in surfacic traversals (and not volumetric
         *  traversals).
         */
        index_t current_facet() const {
            return current_facet_;
        }

        /**
         * \brief Gets the index of the Delaunay vertex currently processed.
         * \details Can be used in both surfacic traversals and volumetric
         *  traversals.
         */
        index_t current_seed() const {
            return current_seed_;
        }

        /**
         * \brief Gets the current polygon.
         * \details The current polygon corresponds to the
         *  intersection between the current facet
         *  and the Voronoi cell of the current seed. Can be used
         *  in surfacic traversals (and not volumetric traversals).
         */
        const Polygon& current_polygon() const {
            return *current_polygon_;
        }

        /**
         * \brief Gets the undex of the mesh tetrahedron currently processed.
         * \details Can be used in volumetric traversals (and not in surfacic
         *  traversals).
         */
        index_t current_tet() const {
            return current_tet_;
        }

        /**
         * \brief Gets the current cell.
         * \details The current cell corresponds to the
         *  intersection between the current tetrahedron
         *  and the Voronoi cell of the current seed.
         *  Can be used in volumetric traversals (and not in
         *  surfacic traversals).
         */
        const Polyhedron& current_polyhedron() const {
            return *current_polyhedron_;
        }

        /**
         * \brief Sets symbolic mode.
         * \details If exact mode is active, symbolic mode is enforced.
         * \param[in] x if set, the symbolic representation of the intersections
         *  are computed.
         */
        void set_symbolic(bool x) {
            symbolic_ = x;
            // exact mode requires symbolic mode.
            if(exact_) {
                symbolic_ = true;
            }
        }

        /**
         * \brief Tests whether symbolic mode is active.
         */
        bool symbolic() const {
            return symbolic_;
        }

        /**
         * \brief Specifies whether exact predicates should be used.
         * \details If exact predicates are used, symbolic mode is ensured.
         * \param[in] x if set, exact predicates are used.
         */
        void set_exact_predicates(bool x) {
            exact_ = x;
            // exact mode requires symbolic mode.
            if(exact_) {
                symbolic_ = true;
            }
        }

        /**
         * \brief Tests whether exact predicates are used.
         */
        bool exact_predicates() const {
            return exact_;
        }

        /**
         * \brief Specifies whether radius of security should be enforced.
         */
        void set_check_SR(bool x) {
            check_SR_ = x;
        }

        /**
         * \brief Tests whether radius of security is enforced.
	 * \retval true if radius of security test is used.
	 * \retval false otherwise.
         */
        bool check_SR() const {
            return check_SR_;
        }

	/**
	 * \brief Gets the PointAllocator.
	 * \return a pointer to the PointAllocator, used
	 *  to create the new vertices generated by 
	 *  intersections.
	 */
        PointAllocator* point_allocator() {
	    return &intersections_;
	}
	
    protected:
        /**
         * \name Adapter classes for surfacic computation
         * @{
         */

        /**
         * \brief Adapter class used internally to implement for_each_polygon()
         * \details Overrides constness checks, to allow using temporaries as
         *   argument of for_each_xxx().
         * \tparam ACTION the user action class.
         */
        template <class ACTION>
        class PolygonAction {
        public:
            /**
             * \brief Creates a new PolygonAction around a user ACTION instance.
             * \param[in] do_it the user ACTION instance
             */
            PolygonAction(const ACTION& do_it) :
                do_it_(do_it) {
            }

            /**
             * \brief Callback called for each polygon.
             * \details Routes the callback to the wrapped user action class.
             * \param[in] v index of current Delaunay seed
             * \param[in] f index of current mesh facet
             * \param[in] P intersection between current mesh facet
             *  and the Voronoi cell of \p v
             */
            void operator() (
                index_t v,
                index_t f,
                const Polygon& P
            ) const {
                GEO::geo_argused(f);
                const_cast<ACTION&> ( do_it_)(v, P);
            }

        protected:
            const ACTION& do_it_;
        };

        /**
         * \brief Adapter class used internally to implement
         *  for_each_triangle().
         * \details Overrides constness checks, to allow using temporaries as
         * argument of for_each_xxx().
         * \tparam ACTION the user action class
         */
        template <class ACTION>
        class TriangleAction {
        public:
            /**
             * \brief Creates a new TriangleAction that wraps a
             *  user ACTION instance.
             * \param[in] do_it the user ACTION instance
             */
            TriangleAction(const ACTION& do_it) :
                do_it_(do_it) {
            }

            /**
             * \brief Callback called for each integration simplex.
             * \details Decomposes the polygon \p P into triangles and
             *  calls the callback of the wrapped user action class
             *  for each triangle.
             * \param[in] v index of current Delaunay seed
             * \param[in] f index of current mesh facet
             * \param[in] P intersection between current mesh facet and
             *  the Voronoi cell of \p v
             */
            void operator() (
                index_t v,
                index_t f,
                const Polygon& P
            ) const {
                GEO::geo_argused(f);
                for(index_t i = 1; i + 1 < P.nb_vertices(); i++) {
                    const_cast<ACTION&> (do_it_)(
                        v, P.vertex(0), P.vertex(i), P.vertex(i + 1)
                    );
                }
            }

        protected:
            const ACTION& do_it_;
        };

        /**
         * \brief Adapter class used internally
         *  to implement for_each_halfedge().
         * \details Overrides constness checks, to allow using temporaries as
         *  argument of for_each_xxx().
         * \tparam ACTION the user action class..
         */
        template <class ACTION>
        class HalfedgeAction {
        public:
            /**
             * \brief Creates a new HalfedgeAction that wraps
             *  a user ACTION instance.
             * \param[in] do_it the user ACTION instance
             */
            HalfedgeAction(const ACTION& do_it) :
                do_it_(do_it) {
            }

            /**
             * \brief Callback called for each integration simplex.
             * \details Calls the callback of the wrapped
             *  user action class for each edge that has the INTERSECT flag.
             * \param[in] v index of current Delaunay seed
             * \param[in] f index of current mesh facet
             * \param[in] P intersection between current mesh facet
             *  and the Voronoi cell of \p v
             */
            void operator() (
                index_t v,
                index_t f,
                const Polygon& P
            ) const {
                GEO::geo_argused(v);
                GEO::geo_argused(f);
                for(index_t i = 0; i < P.nb_vertices(); i++) {
                    if(P.vertex(i).check_flag(INTERSECT)) {
                        index_t j = P.next_vertex(i);
                        const_cast<ACTION&> (do_it_)(
                            P.vertex(i), P.vertex(j)
                        );
                    }
                }
            }

        protected:
            const ACTION& do_it_;
        };

        /**
         * \brief Adapter class used internally to implement
         *  for_each_border_halfedge().
         * \details Overrides constness checks, to allow using temporaries as
         * argument of for_each_xxx().
         * \tparam ACTION the user action class..
         */
        template <class ACTION>
        class BorderHalfedgeAction {
        public:
            /**
             * \brief Creates a new BorderHalfedgeAction that wraps
             *  a user ACTION instance.
             * \param[in] do_it the user ACTION instance
             */
            BorderHalfedgeAction(const ACTION& do_it) :
                do_it_(do_it) {
            }

            /**
             * \brief Callback called for each integration simplex.
             * \details Calls the callback of the wrapped
             *  user action class for each edge that is on the
             *  border of the input surface.
             * \param[in] v index of current Delaunay seed
             * \param[in] f index of current mesh facet
             * \param[in] P intersection between current mesh facet and
             *  the Voronoi cell of \p v
             */
            void operator() (
                index_t v,
                index_t f,
                const Polygon& P
            ) const {
                GEO::geo_argused(f);
                for(index_t i = 0; i < P.nb_vertices(); i++) {
                    if(P.vertex(i).check_flag(ORIGINAL)) {
                        if(P.vertex(i).adjacent_facet() == -1) {
                            index_t j = P.next_vertex(i);
                            const_cast<ACTION&> (do_it_)(
                                v, P.vertex(i), P.vertex(j)
                            );
                        }
                    }
                }
            }

        private:
            const ACTION& do_it_;
        };

        /**
         * \brief Adapter class used internally to implement
         *  for_each_primal_triangle()
         * \details Overrides constness checks, to allow using temporaries as
         *  argument of for_each_xxx()
         */
        template <class ACTION>
        class PrimalTriangleAction {
        public:
            /**
             * \brief Creates a new PrimalTriangleAction that wraps
             *  a user ACTION instance.
             * \param[in] do_it the user ACTION instance
             */
            PrimalTriangleAction(const ACTION& do_it) :
                do_it_(do_it) {
            }

            /**
             * \brief Callback called for each primal triangle.
             * \param[in] iv1 index of current Delaunay seed
             * \param[in] f index of current mesh facet
             * \param[in] P intersection between current mesh facet and
             *  the Voronoi cell of \p v
             */
            void operator() (
                index_t iv1,
                index_t f,
                const Polygon& P
            ) const {
                GEO::geo_argused(f);
                for(index_t i = 0; i < P.nb_vertices(); i++) {
                    const Vertex& ve = P.vertex(i);
                    // Primal triangles correspond to vertices of
                    // the RVD that are on two bisectors.
                    if(ve.sym().nb_bisectors() == 2) {
                        index_t iv2 = ve.sym().bisector(0);
                        index_t iv3 = ve.sym().bisector(1);
                        // This test generates triangle (iv1,iv2,iv3)
                        // only once (i.e. if iv1 is the vertex with
                        // the smallest index).
                        if(iv1 < iv2 && iv1 < iv3) {
                            const_cast<ACTION&> (do_it_)(iv1, iv2, iv3);
                        }
                    }
                }
            }

        protected:
            const ACTION& do_it_;
        };

        /**
         * @}
         * \name Adapter classes for volumetric computation
         * @{
         */

        /**
         * \brief Adapter class used internally to implement
         *  for_each_polyhedron()
         * \details Overrides constness checks, to allow using temporaries as
         *  argument of for_each_xxx()
         * \tparam ACTION the user action class
         */
        template <class ACTION>
        class PolyhedronAction {
        public:
            /**
             * \brief Creates a new PolyhedronAction that wraps
             *  a user ACTION instance.
             * \param[in] do_it the user ACTION instance
             */
            PolyhedronAction(const ACTION& do_it) :
                do_it_(do_it) {
            }

            /**
             * \brief Callback called for each polyhedron
             * \details Routes the callback to the wrapped user action class.
             * \param[in] v index of current Delaunay seed
             * \param[in] t index of current mesh tetrahedron
             * \param[in] C intersection between current mesh tetrahedron
             *  and the Voronoi cell of \p v
             */
            void operator() (
                index_t v,
                index_t t,
                const Polyhedron& C
            ) const {
                const_cast<ACTION&> ( do_it_)(v, t, C);
            }

        protected:
            const ACTION& do_it_;
        };

        /**
         * \brief Adapter class used internally to implement
         *  for_each_volumetric_integration_simplex()
         * \details Overrides constness checks, to allow using temporaries as
         *  argument of for_each_xxx().
         * \tparam ACTION the user action class. It needs to implement:
         *  operator()(index_t v, signed_index_t v_adj,
         *    index_t t, signed_index_t t_adj,
         *    const Vertex& v1, const Vertex& v2, const Vertex& v3
         *  )
         *  where the parameters are as follows:
         *    - v is the index of the current Voronoi cell
         *    (or Delaunay vertex)
         *    - v_adj is the index of the Voronoi cell adjacent to t accros
         *    facet (\p v1, \p v2, \p v3) or -1 if it does not exists
         *    adjacent to v or -1 if current face is a tetrahedron facet
         *    - t is the index of the current tetrahedron
         *    - t_adj is the index of the tetrahedron adjacent to t accros
         *    facet (\p v1, \p v2, \p v3) or -1 if it does not exists
         *    - v1,v2 and v3 are the three vertices of the facet on the
         *    border of the restricted Voronoi cell.
         */
        template <class ACTION>
        class VolumetricIntegrationSimplexAction {
        public:
            /**
             * \brief Creates a new VolumetricIntegrationSimplexAction 
             *  that wraps a user ACTION instance.
             * \param[in] do_it the user ACTION instance
             * \param[in] visit_inner_tets if set, all the tetrahedron-cell
             *  intersections are visited, else only tetrahedra on the border
             *  of the restricted Voronoi cell are visited. Since all the
             *  visited triangles are connected to the current Voronoi seed
             *  by a tetrahedron, the computed volume is the same
             *  in both cases.
             * \param[in] coherent_triangles if set, this ensures that
             *  the polygonal facets of the cells are always triangulated
             *  in a coherent manner when seen from two different cells.
             *  For instance, it is required if a tetrahedral mesh is
             *  reconstructed.
             */
            VolumetricIntegrationSimplexAction(
                const ACTION& do_it,
                bool visit_inner_tets = false,
                bool coherent_triangles = false
            ) :
                do_it_(do_it),
                visit_inner_tets_(visit_inner_tets),
                coherent_triangles_(coherent_triangles)
            {
            }

            /**
             * \brief Callback called for each polyhedron
             * \details Routes the callback to the wrapped user action class.
             * \param[in] v index of current Delaunay seed
             * \param[in] t index of current mesh tetrahedron
             * \param[in] C intersection between current mesh tetrahedron
             *  and the Voronoi cell of \p v
             */
            void operator() (
                index_t v,
                index_t t,
                const Polyhedron& C
            ) const {
                for(index_t cv = 0; cv < C.max_v(); ++cv) {
                    signed_index_t ct = C.vertex_triangle(cv);
                    if(ct == -1) {
                        continue;
                    }
                    geo_debug_assert(C.triangle_is_used(index_t(ct)));

                    signed_index_t adjacent = C.vertex_id(cv);
                    signed_index_t v_adj = -1;
                    signed_index_t t_adj = -1;

                    if(adjacent < 0) {
                        // Negative adjacent indices correspond to
                        // tet-tet links (ignored when we want to triangulate
                        // the border of the restricted Voronoi cell while
                        // ignoring internal structures).
                        if(!visit_inner_tets_) {
                            continue;
                        }
                        t_adj = -adjacent - 1;
                    } else if(adjacent > 0) {
                        // Positive adjacent indices correspond to
                        // Voronoi seed - Voronoi seed link
                        v_adj = adjacent - 1;
                    }
                    // and adjacent indicex equal to zero corresponds
                    // to tet on border.

                    Polyhedron::Corner c1(
                        index_t(ct),
                        index_t(C.find_triangle_vertex(index_t(ct), cv))
                    );

                    // If required, ensure that two polygonal facets
                    // seen from two different volumetric cells will
                    // be triangulated coherently.
                    if(coherent_triangles_) {
                        move_to_first_corner_of_facet(C, c1, v);
                    }

                    const Vertex& v1 = C.triangle_dual(c1.t);

                    Polyhedron::Corner c2 = c1;
                    C.move_to_next_around_vertex(c2);
                    geo_debug_assert(c2 != c1);

                    Polyhedron::Corner c3 = c2;
                    C.move_to_next_around_vertex(c3);
                    geo_debug_assert(c3 != c1);
                    do {
                        const Vertex& v2 = C.triangle_dual(c2.t);
                        const Vertex& v3 = C.triangle_dual(c3.t);
                        const_cast<ACTION&> (do_it_)(
                            v, v_adj, t, t_adj, v1, v2, v3
                        );
                        c2 = c3;
                        C.move_to_next_around_vertex(c3);
                    } while(c3 != c1);
                }
            }

            /**
             * \brief Finds the first corner of a facet in a Polyhedron.
             * \details This function is used to ensure that a facet is
             *  triangulated coherently when seen from two different
             *  volumetric cells, by generating a fan of triangles
             *  that radiates from the first corner. The global order
             *  used to find the first
             *  corner is defined by the function symbolic_compare().
             *
             * \param[in] C the Polyhedron
             * \param[in,out] c a corner of the facet, replaced by the
             *  first corner of the facet on exit.
             * \param[in] center_vertex_id index of the current Voronoi seed
             *  (needed to determine the full symbolic information in the
             *  vertices).
             */
            void move_to_first_corner_of_facet(
                const Polyhedron& C, Polyhedron::Corner& c,
                index_t center_vertex_id
            ) const {
                Polyhedron::Corner first = c;
                Polyhedron::Corner cur = c;
                do {
                    if(symbolic_compare(
                            C.triangle_dual(cur.t),
                            C.triangle_dual(c.t),
                            center_vertex_id
                        )) {
                        c = cur;
                    }
                    C.move_to_next_around_vertex(cur);
                } while(cur != first);
            }

            /**
             * \brief Compares the symbolic information of two vertices
             *  in such a way that a global order is defined.
             * \details This function is used to ensure that a facet is
             *  triangulated coherently when seen from two different
             *  volumetric cells (it uniquely determines the "first" vertex).
             * \param[in] p1 first vertex to compare
             * \param[in] p2 second vertex to compare
             * \param[in] center_vertex_id index of the current Voronoi seed
             *  (needed to determine the full symbolic information in
             *   \p p1 and \p p2).
             * \return true if p1 is before p2 in the global order,
             *  false otherwise.
             */
            static bool symbolic_compare(
                const Vertex& p1, const Vertex& p2, index_t center_vertex_id
            ) {
                GEO::signed_quadindex K1(
                    signed_index_t(center_vertex_id), 
                    p1.sym()[0], p1.sym()[1], p1.sym()[2]
                );
                GEO::signed_quadindex K2(
                    signed_index_t(center_vertex_id), 
                    p2.sym()[0], p2.sym()[1], p2.sym()[2]
                );
                return K1 < K2;
            }

        protected:
            const ACTION& do_it_;
            bool visit_inner_tets_;
            bool coherent_triangles_;
        };

        /**
         * \brief Adapter class used internally to implement
         *  for_each_tetrahedron()
         * \details Overrides constness checks, to allow using temporaries as
         *  argument of for_each_xxx()
         * \tparam ACTION the user action class. It needs to implement:
         *  operator()(index_t v, signed_index_t v_adj,
         *    index_t t, index_t t_adj,
         *    const Vertex& v0, const Vertex& v1, 
         *    const Vertex& v2, const Vertex& v3
         *  )
         *  where the parameters are as follows:
         *    - v is the index of the current Voronoi cell
         *    (or Delaunay vertex)
         *    - v_adj is the index of the Voronoi cell adjacent to t accros
         *    facet (\p v1, \p v2, \p v3) or -1 if it does not exists
         *    adjacent to v or -1 if current face is a tetrahedron facet
         *    - t is the index of the current tetrahedron
         *    - t_adj is the index of the tetrahedron adjacent to t accros
         *    facet (\p v1, \p v2, \p v3) or -1 if it does not exists
         *    - v0,v1,v2 and v3 are the four vertices of tetrahedron.
         */
        template <class ACTION>
        class TetrahedronAction {
        public:
            /**
             * \brief Creates a new TetrahedronAction that wraps
             *  a user ACTION instance.
             * \param[in] do_it the user ACTION instance
             */
            TetrahedronAction(
                const ACTION& do_it
            ) :
                do_it_(do_it)
            {
            }

            /**
             * \brief Callback called for each polyhedron
             * \details Routes the callback to the wrapped user action class.
             * \param[in] v index of current Delaunay seed
             * \param[in] t index of current mesh tetrahedron
             * \param[in] C intersection between current mesh tetrahedron
             *  and the Voronoi cell of \p v
             */
            void operator() (
                index_t v,
                index_t t,
                const Polyhedron& C
            ) const {

                // Find a vertex of the current cell,
                // that will be used as the 'origin'
                // vertex
                const Vertex* v0 = nullptr;
                index_t t0;
                for(t0 = 0; t0 < C.max_t(); ++t0) {
                    if(C.triangle_is_used(t0)) {
                        v0 = &C.triangle_dual(t0);
                        break;
                    }
                }

                // If current cell is empty, return
                if(v0 == nullptr) {
                    return;
                }

                for(index_t cv = 0; cv < C.max_v(); ++cv) {
                    signed_index_t ct = C.vertex_triangle(cv);
                    if(ct == -1) {
                        continue;
                    }
                    geo_debug_assert(C.triangle_is_used(index_t(ct)));

                    signed_index_t adjacent = C.vertex_id(cv);
                    signed_index_t v_adj = -1;
                    signed_index_t t_adj = -1;

                    if(adjacent < 0) {
                        // Negative adjacent indices correspond to
                        // tet-tet links
                        t_adj = -adjacent - 1;
                    } else if(adjacent > 0) {
                        // Positive adjacent indices correspond to
                        // Voronoi seed - Voroni seed link
                        v_adj = adjacent - 1;
                    }
                    // and adjacent indicex equal to zero corresponds
                    // to tet on border.

                    Polyhedron::Corner c1(
                        index_t(ct), C.find_triangle_vertex(index_t(ct), cv)
                    );

                    // If the current facet is incident to
                    // the origin vertex, then skip it (else
                    // it would generate flat tetrahedra)
                    if(facet_is_incident_to_vertex(C, c1, t0)) {
                        continue;
                    }

                    const Vertex& v1 = C.triangle_dual(c1.t);

                    Polyhedron::Corner c2 = c1;
                    C.move_to_next_around_vertex(c2);
                    geo_debug_assert(c2 != c1);

                    Polyhedron::Corner c3 = c2;
                    C.move_to_next_around_vertex(c3);
                    geo_debug_assert(c3 != c1);
                    do {
                        const Vertex& v2 = C.triangle_dual(c2.t);
                        const Vertex& v3 = C.triangle_dual(c3.t);
                        const_cast<ACTION&> (do_it_)(
                            v, v_adj, t, t_adj, * v0, v1, v2, v3
                        );
                        c2 = c3;
                        C.move_to_next_around_vertex(c3);
                    } while(c3 != c1);
                }
            }

        protected:
            /**
             * \brief Tests whether a Polyhedron facet is incident
             *  to a vertex.
             * \param[in] C the Polyhedron
             * \param[in] c a corner of the facet
             * \param[in] t the index of the vertex in dual form (in other
             *  words, a triangle index).
             * \return true if the facet incident to corner \p c 
             *  is also incident to the vertex dual to \p t, false otherwise
             */
            bool facet_is_incident_to_vertex(
                const Polyhedron& C, Polyhedron::Corner& c,
                index_t t
            ) const {
                Polyhedron::Corner first = c;
                Polyhedron::Corner cur = c;
                do {
                    if(cur.t == t) {
                        return true;
                    }
                    C.move_to_next_around_vertex(cur);
                } while(cur != first);
                return false;
            }

        protected:
            const ACTION& do_it_;
        };

        /**
         * \brief Adapter class used internally to implement
         *  for_each_primal_tetrahedron()
         * \details Overrides constness checks, to allow using temporaries as
         *  argument of for_each_xxx()
         * \tparam ACTION the user action class
         */
        template <class ACTION>
        class PrimalTetrahedronAction {
        public:
            /**
             * \brief Constructs a new PrimalTetrahedronAction.
             * \param[in] do_it the user ACTION instance.
             */
            PrimalTetrahedronAction(const ACTION& do_it) :
                do_it_(do_it) {
            }

            /**
             * \brief Callback called for each polyhedron
             * \details Routes the callback to the wrapped user action class.
             * \param[in] v index of current Delaunay seed
             * \param[in] t index of current mesh tetrahedron
             * \param[in] C intersection between current mesh tetrahedron
             *  and the Voronoi cell of \p v
             */
            void operator() (
                index_t v,
                index_t t,
                const Polyhedron& C
            ) const {
                GEO::geo_argused(t);
                for(index_t it = 0; it < C.max_t(); ++it) {
                    if(C.triangle_is_used(it)) {
                        const SymbolicVertex& sym = C.triangle_dual(it).sym();
                        if(sym.nb_bisectors() == 3) {
                            index_t v1 = sym.bisector(0);
                            index_t v2 = sym.bisector(1);
                            index_t v3 = sym.bisector(2);
                            // This test ensures that the tet (v,v1,v2,v3)
                            // is generated only once.
                            if(v < v1 && v < v2 && v < v3) {
                                const_cast<ACTION&> (do_it_)(v, v1, v2, v3);
                            }
                        }
                    }
                }
            }

        protected:
            const ACTION& do_it_;
        };

    public:
        /**
         * @}
         * \name Public interface for computation/iteration
         * @{
         */

        /**
         * \brief Iterates on the facets of this RVD.
         * \param[in] action the user action object
         * \tparam ACTION needs to implement:
         *  operator()(index_t v, index_t f, const Polygon& P) const
         *  where v denotes the index of the current Voronoi cell
         *  (or Delaunay vertex), f the index of the current facet
         *  and P the computed intersection between facet f
         *  and the Voronoi cell of v.
         */
        template <class ACTION>
        inline void for_each_polygon(
            const ACTION& action
        ) {
            this->template compute_surfacic<PolygonAction<ACTION> >(
                PolygonAction<ACTION>(action)
            );
        }

        /**
         * \brief Iterates on the facets of this RVD, triangulated on the fly.
         * \param[in] action the user action object
         * \tparam TRIACTION needs to implement:
         *  operator()(index_t c, const TopoPolyVertex& v1, v2, v3) const
         *  where c denotes the index of the current Voronoi cell
         *  (or Delaunay vertex).
         */
        template <class TRIACTION>
        inline void for_each_triangle(
            const TRIACTION& action
        ) {
            this->template compute_surfacic<TriangleAction<TRIACTION> >(
                TriangleAction<TRIACTION>(action)
            );
        }

        /**
         * \brief Iterates on the halfedges on the borders of the
         *  restricted Voronoi cells.
         * \param[in] action the user action object
         * \tparam HEACTION needs to implement:
         *  operator()(index_t c, const TopoPolyVertex& v1, v2) const
         *  where c denotes the index of the current Voronoi cell
         *  (or Delaunay vertex).
         */
        template <class HEACTION>
        inline void for_each_halfedge(
            const HEACTION& action
        ) {
            this->template compute_surfacic<HalfedgeAction<HEACTION> >(
                HalfedgeAction<HEACTION>(action)
            );
        }

        /**
         * \brief Iterates on the halfedges on the borders of the
         *  restricted Voronoi cells that are on the boundary of the input mesh.
         * \param[in] action the user action object
         * \tparam BOACTION needs to implement:
         *  operator()(index_t c, const TopoPolyVertexEdge& v1, v2) const
         *  where c denotes the index of the current Voronoi cell
         *  (or Delaunay vertex).
         */
        template <class BOACTION>
        inline void for_each_border_halfedge(
            const BOACTION& action
        ) {
            this->template compute_surfacic<BorderHalfedgeAction<BOACTION> >(
                BorderHalfedgeAction<BOACTION>(action)
            );
        }

        /**
         * \brief Iterates on the triangles of the Restricted
         *  Delaunay Triangulation.
         * \param[in] action the user action object
         * \tparam PRIMTRIACTION needs to implement:
         *  operator()(index_t i, unsigned j, index_t k) const
         *  where i,j,k denote the three indices of the Delaunay vertices
         *  that define the primal triangle.
         */
        template <class PRIMTRIACTION>
        inline void for_each_primal_triangle(
            const PRIMTRIACTION& action
        ) {
            bool sym_backup = symbolic();
            set_symbolic(true);
            this->template compute_surfacic<
                PrimalTriangleAction<PRIMTRIACTION
                > >(
                    PrimalTriangleAction<PRIMTRIACTION>(action)
                );
            set_symbolic(sym_backup);
        }

        /**
         * \brief Iterates on the polyhedra of this RVD.
         * \param[in] action the user action object
         * \tparam ACTION needs to implement:
         *  operator()(index_t v, index_t t, const Polyhedron& C) const
         *  where v denotes the index of the current Voronoi cell
         *  (or Delaunay vertex), t the index of the current tetrahedron
         *  and C the computed intersection between tetrahedron t
         *  and the Voronoi cell of v.
         */
        template <class ACTION>
        inline void for_each_polyhedron(
            const ACTION& action
        ) {
            this->template compute_volumetric<PolyhedronAction<ACTION> >(
                PolyhedronAction<ACTION>(action)
            );
        }

        /**
         * \brief Iterates on the polyhedra of this RVD decomposed
         *  on the fly into tetrahedra.
         * \details The generated tetrahedra may be geometrically incorrect,
         *  but they are algebraically correct. In other word, their signed
         *  volumes sum as the volume of the restricted Voronoi cell.
         * \param[in] action the user action object
         * \param[in] visit_inner_tets if set, all the tetrahedron-cell
         *  intersections are visited, else only tetrahedra on the border
         *  of the restricted Voronoi cell are visited. Since all the visited
         *  triangles are connected to the current Voronoi seed by a
         *  tetrahedron, the computed volume is the same in both cases.
         * \param[in] coherent_triangles if set, this ensures that the
         *  polygonal facets of the cells are always triangulated in a
         *  coherent manner when seen from two different cells.
         *  For instance, it is required if a tetrahedral mesh is
         *  reconstructed.
         * \tparam ACTION needs to implement:
         *  operator()(index_t v, signed_index_t v_adj,
         *    index_t t, index_t t_adj,
         *    const Vertex& v1, const Vertex& v2, const Vertex& v3
         *  )
         *  where the parameters are as follows:
         *    - v is the index of the current Voronoi cell
         *    (or Delaunay vertex)
         *    - v_adj is the index of the Voronoi cell adjacent to t accros
         *    facet (\p v1, \p v2, \p v3) or -1 if it does not exists
         *    adjacent to v or -1 if current face is a tetrahedron facet
         *    - t is the index of the current tetrahedron
         *    - t_adj is the index of the tetrahedron adjacent to t accros
         *    facet (\p v1, \p v2, \p v3) or -1 if it does not exists
         *    - v1,v2 and v3 are the three vertices of the facet on the
         *    border of the restricted Voronoi cell.
         */
        template <class ACTION>
        inline void for_each_volumetric_integration_simplex(
            const ACTION& action,
            bool visit_inner_tets = false, bool coherent_triangles = false
        ) {
            this->template compute_volumetric<
                VolumetricIntegrationSimplexAction<ACTION> 
             >(
                VolumetricIntegrationSimplexAction<ACTION>(
                    action, visit_inner_tets, coherent_triangles
                )
            );
        }

        /**
         * \brief Iterates on the polyhedra of this RVD decomposed
         *  on the fly into tetrahedra.
         * \details The tetrahedra are generated by connecting one of
         *  the vertices of the cell to the other ones.
         * \param[in] action the user action object
         * \tparam ACTION needs to implement:
         *  operator()(index_t v, signed_index_t v_adj,
         *    index_t t, index_t t_adj,
         *    const Vertex& v0, const Vertex& v1, 
         *    const Vertex& v2, const Vertex& v3
         *  )
         *  where the parameters are as follows:
         *    - v is the index of the current Voronoi cell
         *    (or Delaunay vertex)
         *    - v_adj is the index of the Voronoi cell adjacent to t accros
         *    facet (\p v1, \p v2, \p v3) or -1 if it does not exists
         *    adjacent to v or -1 if current face is a tetrahedron facet
         *    - t is the index of the current tetrahedron
         *    - t_adj is the index of the tetrahedron adjacent to t accros
         *    facet (\p v1, \p v2, \p v3) or -1 if it does not exists
         *    - v0,v1,v2 and v3 are the four vertices of tetrahedron.
         */
        template <class ACTION>
        inline void for_each_tetrahedron(
            const ACTION& action
        ) {
            this->template compute_volumetric<TetrahedronAction<ACTION> >(
                TetrahedronAction<ACTION>(
                    action
                )
            );
        }

        /**
         * \brief Iterates on the primal tetrahedra of this RVD.
         * \details The tetrahedra are not coherently oriented,
         *  and need a subsequent traversal operation to reorient
         *  them. They can be also reoriented geometrically using
         *  the orient3d() predicate.
         * \param[in] action the user action object
         * \tparam ACTION needs to implement:
         *  operator()(index_t v0, index_t v1, index_t v2, index_t v3)
         *  where v0,v1,v2 and v3 are the indices of the four vertices
         *  of tetrahedron.
         */
        template <class ACTION>
        inline void for_each_primal_tetrahedron(
            const ACTION& action
        ) {
            bool sym_backup = symbolic();
            set_symbolic(true);
            this->template compute_volumetric<PrimalTetrahedronAction<ACTION> >(
                PrimalTetrahedronAction<ACTION>(
                    action
                )
            );
            set_symbolic(sym_backup);
        }

    protected:
        /**
         * @}
         * \name Computation
         * @{
         */

        /**
         * \brief Low-level API of Restricted Voronoi Diagram traversal.
         * \details Client code may use for_each_facet(),for_each_triangle() or
         *  for_each_primal_triangle() instead.
         * \tparam ACTION needs to implement:
         *  operator()(index_t v, index_t f, const Polygon& P) const
         *  where v denotes the index of the current Voronoi cell
         *  (or Delaunay vertex), f the index of the current facet
         *  and P the computed intersection between the Voronoi cell of
         *  v and facet f.
         */
        template <class ACTION>
        inline void compute_surfacic(const ACTION& action) {
            if(connected_components_priority_) {
                this->template compute_surfacic_with_cnx_priority<ACTION>(
                    action
                );
            } else {
                this->template compute_surfacic_with_seeds_priority<ACTION>(
                    action
                );
            }
        }

        /**
         * \brief Low-level API of Restricted Voronoi Diagram traversal
         *  with seeds priority in surfacic mode.
         * \details Client code may use for_each_facet(),for_each_triangle() or
         *  for_each_primal_triangle() instead.
         * \tparam ACTION needs to implement:
         *  operator()(index_t v, index_t f, const Polygon& P) const
         *  where v denotes the index of the current Voronoi cell
         *  (or Delaunay vertex), f the index of the current facet
         *  and P the computed intersection between the Voronoi cell of
         *  v and facet f.
         */
        template <class ACTION>
        inline void compute_surfacic_with_seeds_priority(
            const ACTION& action
        ) {
            if(
               facets_begin_ == UNSPECIFIED_RANGE && 
               facets_end_ == UNSPECIFIED_RANGE
            ) {
                facets_begin_ = 0;
                facets_end_ = mesh_->facets.nb();
            }
            current_polygon_ = nullptr;
            GEO::vector<index_t> seed_stamp(
                delaunay_->nb_vertices(), index_t(-1)
            );
            GEO::vector<bool> facet_is_marked(facets_end_-facets_begin_, false);
            init_get_neighbors();

            FacetSeedStack adjacent_facets;
            SeedStack adjacent_seeds;
            Polygon F;
            GEO::Attribute<double> vertex_weight;
            vertex_weight.bind_if_is_defined(
		mesh_->vertices.attributes(), "weight"
	    );

            // The algorithm propagates along both the facet-graph of
            // the surface and the 1-skeleton of the Delaunay triangulation,
            // and computes all the relevant intersections between
            // each Voronoi cell and facet.
            for(index_t f = facets_begin_; f < facets_end_; f++) {
                if(!facet_is_marked[f-facets_begin_]) {
                    // Propagate along the facet-graph.
                    facet_is_marked[f-facets_begin_] = true;
                    adjacent_facets.push(
                        FacetSeed(f, find_seed_near_facet(f))
                    );
                    while(!adjacent_facets.empty()) {
                        current_facet_ = adjacent_facets.top().f;
                        current_seed_ = adjacent_facets.top().seed;
                        adjacent_facets.pop();

                        // Copy the current facet from the Mesh into
                        // RestrictedVoronoiDiagram's Polygon data structure
                        // (gathers all the necessary information)
                        F.initialize_from_mesh_facet(
                            mesh_, current_facet_, symbolic_, vertex_weight
                        );

                        // Propagate along the Delaunay 1-skeleton
                        // This will traverse all the seeds such that their
                        // Voronoi cell has a non-empty intersection with
                        // the current facet.
                        seed_stamp[current_seed_] = current_facet_;
                        adjacent_seeds.push(current_seed_);

                        while(!adjacent_seeds.empty()) {
                            current_seed_ = adjacent_seeds.top();
                            adjacent_seeds.pop();

                            current_polygon_ = intersect_cell_facet(
                                current_seed_, F
                            );

                            action(
                                current_seed_, current_facet_, current_polygon()
                            );

                            // Propagate to adjacent facets and adjacent seeds
                            for(index_t v = 0;
                                v < current_polygon().nb_vertices(); v++
                            ) {
                                const Vertex& ve = current_polygon().vertex(v);
                                signed_index_t neigh_f = ve.adjacent_facet();
                                if(
                                    neigh_f >= signed_index_t(facets_begin_) &&
                                    neigh_f < signed_index_t(facets_end_) &&
                                    neigh_f != signed_index_t(current_facet_)
                                ) {
                                    if(!facet_is_marked[
                                           index_t(neigh_f)-facets_begin_
                                    ]) {
                                        facet_is_marked[
                                            index_t(neigh_f)-facets_begin_
                                        ] = true;
                                        adjacent_facets.push(
                                            FacetSeed(
                                                index_t(neigh_f),
                                                current_seed_
                                            )
                                        );
                                    }
                                }
                                signed_index_t neigh_s = ve.adjacent_seed();
                                if(neigh_s != -1) {
                                    if(
                                        seed_stamp[neigh_s] != current_facet_
                                    ) {
                                        seed_stamp[neigh_s] = current_facet_;
                                        adjacent_seeds.push(index_t(neigh_s));
                                    }
                                }
                            }
                        }
                    }
                }
            }
            current_polygon_ = nullptr;
        }

        /**
         * \brief Low-level API of Restricted Voronoi Diagram traversal .
         * \details Selects seed-priority or tetrahedron-priority modes
         *  according to connected_components_priority mode.
         *  Client code may use for_each_polyhedron() or
         *  for_each_volumetric_integration_simplex() instead of this function.
         * \tparam ACTION needs to implement:
         *  operator()(index_t v, index_t t, const Polyhedron& C) const
         *  where v denotes the index of the current Voronoi cell
         *  (or Delaunay vertex), t the index of the current tetrahedron
         *  and C the computed intersection between the Voronoi cell of
         *  v and tetrahedron t
         */
        template <class ACTION>
        inline void compute_volumetric(
            const ACTION& action
        ) {
	    if(connected_components_priority_) {
		this->template compute_volumetric_with_cnx_priority<ACTION>(
		    action
		);
	    } else {
		this->template compute_volumetric_with_seeds_priority<ACTION>(
		    action
		);
	    }
        }

        /**
         * \brief Low-level API of Restricted Voronoi Diagram traversal
         *  with seeds priority in volumetric mode.
         * \details Client code may use for_each_polyhedron() or
         *  for_each_volumetric_integration_simplex() instead.
         * \tparam ACTION needs to implement:
         *  operator()(index_t v, index_t t, const Polyhedron& C) const
         *  where v denotes the index of the current Voronoi cell
         *  (or Delaunay vertex), t the index of the current tetrahedron
         *  and C the computed intersection between the Voronoi cell of
         *  v and tetrahedron t
         */
        template <class ACTION>
        inline void compute_volumetric_with_seeds_priority(
            const ACTION& action
        ) {
            if(
               tets_begin_ == UNSPECIFIED_RANGE && 
               tets_end_ == UNSPECIFIED_RANGE
            ) {
                tets_begin_ = 0;
                tets_end_ = mesh_->cells.nb();
            }

            geo_assert(tets_begin_ != UNSPECIFIED_RANGE);
            geo_assert(tets_end_ != UNSPECIFIED_RANGE);

            GEO::vector<index_t> seed_stamp(
                delaunay_->nb_vertices(), index_t(-1)
            );
            GEO::vector<bool> tet_is_marked(tets_end_-tets_begin_, false);
            init_get_neighbors();

            TetSeedStack adjacent_tets;
            SeedStack adjacent_seeds;
            Polyhedron C(dimension());
            GEO::Attribute<double> vertex_weight;
            vertex_weight.bind_if_is_defined(
		mesh_->vertices.attributes(), "weight"
	    );
            
            current_polyhedron_ = &C;
            // The algorithm propagates along both the facet-graph of
            // the surface and the 1-skeleton of the Delaunay triangulation,
            // and computes all the relevant intersections between
            // each Voronoi cell and facet.
            for(index_t t = tets_begin_; t < tets_end_; ++t) {
                if(!tet_is_marked[t-tets_begin_]) {
                    // Propagate along the tet-graph.
                    tet_is_marked[t-tets_begin_] = true;
                    adjacent_tets.push(
                        TetSeed(t, find_seed_near_tet(t))
                    );
                    while(!adjacent_tets.empty()) {
                        current_tet_ = adjacent_tets.top().f;
                        current_seed_ = adjacent_tets.top().seed;
                        adjacent_tets.pop();

                        // Note: current cell could be looked up here,
                        // (from current_tet_) if we chose to keep it
                        // and copy it right before clipping (I am
                        // not sure that it is worth it, lookup time
                        // will be probably fast enough)

                        // Propagate along the Delaunay 1-skeleton
                        // This will traverse all the seeds such that their
                        // Voronoi cell has a non-empty intersection with
                        // the current facet.
                        seed_stamp[current_seed_] = current_tet_;
                        adjacent_seeds.push(current_seed_);

                        while(!adjacent_seeds.empty()) {
                            current_seed_ = adjacent_seeds.top();
                            adjacent_seeds.pop();

                            C.initialize_from_mesh_tetrahedron(
                                mesh_, current_tet_, symbolic_, vertex_weight
                            );

                            intersect_cell_cell(
                                current_seed_, C
                            );

                            action(
                                current_seed_, current_tet_,
                                current_polyhedron()
                            );

                            // Propagate to adjacent tets and adjacent seeds
                            // Iterate on the vertices of the cell (remember:
                            // the cell is represented in dual form)
                            for(index_t v = 0;
                                v < current_polyhedron().max_v(); ++v
                            ) {

                                //  Skip clipping planes that are no longer
                                // connected to a cell facet.
                                if(
                                    current_polyhedron().vertex_triangle(v)
                                    == -1
                                ) {
                                    continue;
                                }

                                signed_index_t id =
                                    current_polyhedron().vertex_id(v);
                                if(id > 0) {
                                    // Propagate to adjacent seed
                                    index_t neigh_s = index_t(id - 1);
                                    if(seed_stamp[neigh_s] != current_tet_) {
                                        seed_stamp[neigh_s] = current_tet_;
                                        adjacent_seeds.push(neigh_s);
                                    }
                                } else if(id < 0) {
                                    // id==0 corresponds to facet on boundary
                                    //  (skipped)
                                    // id<0 corresponds to adjacent tet index

                                    // Propagate to adjacent tet
                                    signed_index_t neigh_t = -id - 1;
                                    if(
                                        neigh_t >=
                                        signed_index_t(tets_begin_) &&
                                        neigh_t <  signed_index_t(tets_end_) &&
                                        neigh_t != signed_index_t(current_tet_)
                                    ) {
                                        if(!tet_is_marked[
                                               index_t(neigh_t)-tets_begin_
                                        ]) {
                                            tet_is_marked[
                                                index_t(neigh_t)-tets_begin_
                                            ] = true;
                                            adjacent_tets.push(
                                                TetSeed(
                                                    index_t(neigh_t),
                                                    current_seed_
                                                )
                                            );
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            current_polyhedron_ = nullptr;
        }


        /**
         * \brief Low-level API of Restricted Voronoi Diagram traversal
         *  with connected components priority.
         * \details Client code may use for_each_cell() instead.
         * This version of the algorithm traverses the RVD and ensures that
         * the group of subfacets that belong to the same restricted Voronoi
         * cell will be traversed consecutively. It is used by the algorithm
         * that computes the final surface in CVT (i.e., the dual of the
         * connected components).
         * \note This function is less efficient than
         *  compute_volumetric_with_seeds_priority() but
         *  is required by some traversals that need to be done in that order.
         * \tparam ACTION needs to implement:
         *  operator()(index_t v, index_t t, const Polyhedron& C) const
         *  where v denotes the index of the current Voronoi cell
         *  (or Delaunay vertex), c the index of the current tetrahedron
         *  and C the computed intersection between the Voronoi cell of
         *  v and tetrahedron t.
         */
        template <class ACTION>
        inline void compute_volumetric_with_cnx_priority(
            const ACTION& action
        ) {

            if(
                tets_begin_ == UNSPECIFIED_RANGE && 
                tets_end_ == UNSPECIFIED_RANGE
            ) {
                tets_begin_ = 0;
                tets_end_ = mesh_->cells.nb();
            }

	    geo_assert(tets_begin_ != UNSPECIFIED_RANGE);
	    geo_assert(tets_end_ != UNSPECIFIED_RANGE);
	    
            current_polyhedron_ = nullptr;
            init_get_neighbors();

            std::deque<TetSeed> adjacent_seeds;
            std::stack<index_t> adjacent_tets;

            static const index_t NO_STAMP = index_t(-1);
            GEO::vector<index_t> tet_stamp(
                tets_end_ - tets_begin_, NO_STAMP
            );

            TetSeedMarking visited(
                tets_end_ - tets_begin_, delaunay_->nb_vertices()
            );

	    // Yes, facet_seed_marking_ points to the TetSeedMarking,
	    // (TetSeedMarking is typedef-ed as FacetSeedMarking),
	    // ugly I know... to be revised.
            facet_seed_marking_ = &visited;
            Polyhedron C(dimension());
	    current_polyhedron_ = &C;
	    
            current_connected_component_ = 0;
            // index_t C_index = tets_end_ + 1; // Unused (see comment later)

            GEO::Attribute<double> vertex_weight;
            vertex_weight.bind_if_is_defined(
		mesh_->vertices.attributes(),"weight"
	    );
            
            // The algorithm propagates along both the facet-graph of
            // the surface and the 1-skeleton of the Delaunay triangulation,
            // and computes all the relevant intersections between
            // each Voronoi cell and facet.
            for(index_t t = tets_begin_; t < tets_end_; ++t) {
                if(tet_stamp[t - tets_begin_] == NO_STAMP) {
                    current_tet_ = t;
                    current_seed_ = find_seed_near_tet(t);

                    adjacent_seeds.push_back(
                        TetSeed(current_tet_, current_seed_)
                    );

                    // Propagate along the Delaunay-graph.
                    while(!adjacent_seeds.empty()) {
			// Yes, f, because TetSeed is typedef-ed as FacetSeed
                        current_tet_ = adjacent_seeds.front().f;
                        current_seed_ = adjacent_seeds.front().seed;
                        adjacent_seeds.pop_front();
                        if(
                            tet_stamp[current_tet_ - tets_begin_] ==
                            current_seed_
                        ) {
                            continue;
                        }

                        if(visited.is_marked(current_tet_, current_seed_)) {
                            continue;
                        }

                        connected_component_changed_ = true;
                        adjacent_tets.push(current_tet_);
                        tet_stamp[current_tet_ - tets_begin_] =
                            current_seed_;
                        while(!adjacent_tets.empty()) {
                            current_tet_ = adjacent_tets.top();
                            adjacent_tets.pop();

                            // Copy the current tet from the Mesh into
                            // RestrictedVoronoiDiagram's Polyhedron data structure
                            // (gathers all the necessary information)
			    
			    C.initialize_from_mesh_tetrahedron(
				mesh_, current_tet_, symbolic_, vertex_weight
			    );

			    // Note: difference with compute_surfacic_with_cnx_priority():
			    // Since intersect_cell_cell() overwrites C, we
			    // need to initialize C from the mesh for each visited
			    // (tet,seed) pair (and the test for current_tet_ change
			    // with C_index is not used here). 
			    // C_index = current_tet_;

			    intersect_cell_cell(current_seed_, C);
                            action(
                                current_seed_, current_tet_, current_polyhedron()
                            );
                            connected_component_changed_ = false;

                            bool touches_RVC_border = false;

                            // Propagate to adjacent tets and adjacent seeds
                            for(index_t v = 0;
                                v < current_polyhedron().max_v(); ++v
                            ) {

				//   Skip clipping planes that are no longer
				// connected to a cell facet.
				if(
				    current_polyhedron().vertex_triangle(v)
				    == -1
				) {
				    continue;
				}

				signed_index_t id = current_polyhedron().vertex_id(v);
				if(id < 0) {
				    // id == 0 corresponds to facet on boundary
				    //   (skipped)
				    // id < 0 corresponds to adjacent tet index
				    signed_index_t s_neigh_t = -id-1;
				    if(
				       s_neigh_t >= signed_index_t(tets_begin_) &&
				       s_neigh_t < signed_index_t(tets_end_)
				    ) {
					geo_debug_assert(
					    s_neigh_t !=
					    signed_index_t(current_tet_)
					);
					index_t neigh_t = index_t(s_neigh_t);
					if(
					    tet_stamp[neigh_t - tets_begin_] !=
					    current_seed_
					) {
					    tet_stamp[neigh_t - tets_begin_] =
						current_seed_;
					    adjacent_tets.push(neigh_t);
					}
				    }
				} else if(id > 0) {
				    index_t neigh_s = index_t(id-1);
                                    touches_RVC_border = true;
                                    TetSeed ts(current_tet_, neigh_s);
                                    if(!visited.is_marked(ts)) {
                                        adjacent_seeds.push_back(ts);
                                    }				    
				}

                            }
                            if(touches_RVC_border) {
                                visited.mark(
                                    TetSeed(current_tet_, current_seed_),
                                    current_connected_component_
                                );
                            }
                        }
                        ++current_connected_component_;
                    }
                }
            }
            facet_seed_marking_ = nullptr;
        }

	
    public:
        /**
         * \brief Tests whether a (facet,seed) couple was visited.
         * \param[in] f index of the facet
         * \param[in] s index of the seed
         */
        bool facet_seed_is_visited(index_t f, index_t s) const {
            geo_debug_assert(facet_seed_marking_ != nullptr);
            return facet_seed_marking_->is_marked(FacetSeed(f, s));
        }

        /**
         * \brief Gets the index of the connected component associated
         *  with a (facet,seed).
         * \param[in] f index of the facet
         * \param[in] s index of the seed
         * \return the index of the connected component or -1 if the
         *  (\p f, \p s) couple was not visited already
         */
        signed_index_t get_facet_seed_connected_component(
            index_t f, index_t s
        ) const {
            geo_debug_assert(facet_seed_marking_ != nullptr);
            return facet_seed_marking_->get_connected_component(
                FacetSeed(f, s)
            );
        }

        /**
         * \brief Tests whether the current connected component changed.
         */
        bool connected_component_changed() const {
            return connected_component_changed_;
        }

        /**
         * \brief Gets the index of the current connected component.
         */
        index_t current_connected_component() const {
            return current_connected_component_;
        }

    protected:
        /**
         * \brief Low-level API of Restricted Voronoi Diagram traversal
         *  with connected components priority.
         * \details Client code may use for_each_facet(),for_each_triangle() or
         *  for_each_primal_triangle() instead.
         * This version of the algorithm traverses the RVD and ensures that
         * the group of subfacets that belong to the same restricted Voronoi
         * cell will be traversed consecutively. It is used by the algorithm
         * that computes the final surface in CVT (i.e., the dual of the
         * connected components).
         * \note This function is less efficient than
         *  compute_surfacic_with_seeds_priority() but
         *  is required by some traversals that need to be done in that order.
         * \tparam ACTION needs to implement:
         *  operator()(index_t v, index_t f, const Polygon& P) const
         *  where v denotes the index of the current Voronoi cell
         *  (or Delaunay vertex), f the index of the current facet
         *  and P the computed intersection between the Voronoi cell of
         *  v and facet f.
         */
        template <class ACTION>
        inline void compute_surfacic_with_cnx_priority(
            const ACTION& action
        ) {

            if(
                facets_begin_ == UNSPECIFIED_RANGE && 
                facets_end_ == UNSPECIFIED_RANGE
            ) {
                facets_begin_ = 0;
                facets_end_ = mesh_->facets.nb();
            }

            current_polygon_ = nullptr;
            init_get_neighbors();

            std::deque<FacetSeed> adjacent_seeds;
            std::stack<index_t> adjacent_facets;

            static const index_t NO_STAMP = index_t(-1);
            GEO::vector<index_t> facet_stamp(
                facets_end_ - facets_begin_, NO_STAMP
            );

            FacetSeedMarking visited(
                facets_end_ - facets_begin_, delaunay_->nb_vertices()
            );

            facet_seed_marking_ = &visited;
            Polygon F;
            current_connected_component_ = 0;
            index_t F_index = facets_end_ + 1;

            GEO::Attribute<double> vertex_weight;
            vertex_weight.bind_if_is_defined(
		mesh_->vertices.attributes(),"weight"
	    );
            
            // The algorithm propagates along both the facet-graph of
            // the surface and the 1-skeleton of the Delaunay triangulation,
            // and computes all the relevant intersections between
            // each Voronoi cell and facet.
            for(index_t f = facets_begin_; f < facets_end_; ++f) {
                if(facet_stamp[f - facets_begin_] == NO_STAMP) {
                    current_facet_ = f;
                    current_seed_ = find_seed_near_facet(f);

                    adjacent_seeds.push_back(
                        FacetSeed(current_facet_, current_seed_)
                    );

                    // Propagate along the Delaunay-graph.
                    while(
                        !adjacent_seeds.empty()
                    ) {
                        current_facet_ = adjacent_seeds.front().f;
                        current_seed_ = adjacent_seeds.front().seed;
                        adjacent_seeds.pop_front();
                        if(
                            facet_stamp[current_facet_ - facets_begin_] ==
                            current_seed_
                        ) {
                            continue;
                        }

                        if(visited.is_marked(current_facet_, current_seed_)) {
                            continue;
                        }

                        connected_component_changed_ = true;
                        adjacent_facets.push(current_facet_);
                        facet_stamp[current_facet_ - facets_begin_] =
                            current_seed_;
                        while(!adjacent_facets.empty()) {
                            current_facet_ = adjacent_facets.top();
                            adjacent_facets.pop();

                            // Copy the current facet from the Mesh into
                            // RestrictedVoronoiDiagram's Polygon data structure
                            // (gathers all the necessary information)
                            if(F_index != current_facet_) {
                                F.initialize_from_mesh_facet(
                                    mesh_, current_facet_, symbolic_,
                                    vertex_weight
                                );
                                F_index = current_facet_;
                            }

                            current_polygon_ = intersect_cell_facet(
                                current_seed_, F
                            );
                            action(
                                current_seed_, current_facet_, current_polygon()
                            );
                            connected_component_changed_ = false;

                            bool touches_RVC_border = false;

                            // Propagate to adjacent facets and adjacent seeds
                            for(index_t v = 0;
                                v < current_polygon().nb_vertices(); v++
                            ) {
                                const Vertex& ve = current_polygon().vertex(v);
                                signed_index_t s_neigh_f = ve.adjacent_facet();
                                if(
                                    s_neigh_f >= signed_index_t(facets_begin_)
                                    &&
                                    s_neigh_f < signed_index_t(facets_end_)
                                ) {
                                    geo_debug_assert(
                                        s_neigh_f !=
                                        signed_index_t(current_facet_)
                                    );
                                    index_t neigh_f = index_t(s_neigh_f);
                                    if(
                                        facet_stamp[neigh_f - facets_begin_] !=
                                        current_seed_
                                    ) {
                                        facet_stamp[neigh_f - facets_begin_] =
                                            current_seed_;
                                        adjacent_facets.push(neigh_f);
                                    }
                                }
                                signed_index_t neigh_s = ve.adjacent_seed();
                                if(neigh_s != -1) {
                                    touches_RVC_border = true;
                                    FacetSeed fs(
                                        current_facet_, index_t(neigh_s)
                                    );
                                    if(!visited.is_marked(fs)) {
                                        adjacent_seeds.push_back(fs);
                                    }
                                }
                            }
                            if(touches_RVC_border) {
                                visited.mark(
                                    FacetSeed(current_facet_, current_seed_),
                                    current_connected_component_
                                );
                            }
                        }
                        ++current_connected_component_;
                    }
                }
            }
            facet_seed_marking_ = nullptr;
        }

        /**
         * \brief Finds a seed near a given facet.
         * \param[in] f index of the facet in the mesh
         * \return the index of a Voronoi seed such that there is a
         *   non-empty intersection between the Voronoi cell
         *   of the seed and facet \p f.
         */
        index_t find_seed_near_facet(index_t f) {
            const double* p = mesh_->vertices.point_ptr(
                mesh_->facets.vertex(f,0)
            );
            return find_seed_near_point(p);
        }

        /**
         * \brief Finds a seed near a given tetrahedron.
         * \param[in] t index of the tetrahedron in the mesh
         * \return the index of a Voronoi seed such that there is a
         *   non-empty intersection between the Voronoi cell
         *   of the seed and tetrahedron \p t.
         */
        index_t find_seed_near_tet(index_t t) {
            index_t v = mesh_->cells.tet_vertex(t, 0);
            const double* p = mesh_->vertices.point_ptr(v);
            return find_seed_near_point(p);
        }

        /**
         * \brief Finds a seed near a given point.
         * \param[in] p pointer to the coordinates of the point
         * \return the index of a Voronoi seed such that its
         *   Voronoi cell contains the point \p p.
         */
        index_t find_seed_near_point(const double* p) {
            // In order to be compatible with the symbolic
            // perturbation, if the nearest neighbor is
            // non-unique, we need to return the one of
            // lowest index (because in case of several seeds
            // at equal distance, the one of lowest index
            // is guaranteed to have the facet in its Voronoi
            // cell from the point of view of symbolic
            // perturbation).
            if(exact_ && delaunay_nn_ != nullptr) {
                // TODO: may need more than 10
                index_t neighbors[10];
                double neighbors_sq_dist[10];
                index_t nb = 10;
                if(delaunay_nn_->nb_vertices() < nb) {
                    nb = delaunay_nn_->nb_vertices();
                }
                delaunay_nn_->nn_search()->get_nearest_neighbors(
                    nb, p, neighbors, neighbors_sq_dist
                );
                index_t nearest = neighbors[0];
                double min_d = neighbors_sq_dist[0];
                for(index_t i = 1; i < nb; ++i) {
                    if(neighbors_sq_dist[i] != min_d) {
                        break;
                    }
                    if(neighbors[i] < nearest) {
                        nearest = neighbors[i];
                    }
                }
                return nearest;
            }

            return delaunay_->nearest_vertex(p);
        }

        /**
         * @}
         * \name Clipping for surfacic mode
         * @{
         */

        /**
         * \brief Swaps two pointers between two polygons.
         * \details Used by re-entrant Sutherlang-Hogdman clipping.
         */
        void swap_polygons(Polygon*& ping, Polygon*& pong) {
            if(ping != &P1 && ping != &P2) {
                // First clipping operation, ping points to F
                // (current facet copied)
                ping = &P2;
                pong = &P1;
            } else {
                std::swap(ping, pong);
            }
        }

        /**
         * \brief Computes the intersection between the Voronoi cell
         * of a seed and a facet.
         * \param[in] seed the index of the seed
         * \param[in] F the facet represented as a Polygon
         * \details The result is provided in current_polygon_
         */
        Polygon* intersect_cell_facet(index_t seed, Polygon& F) {
            intersections_.clear();

            // Initialize ping-pong pointers for Sutherland-Hodgman
            // re-entrant clipping and copy current facet into 'ping' buffer.
            Polygon* ping = &F;
            Polygon* pong = &P2;

            // Clip current facet by current Voronoi cell (associated with seed)
            if(delaunay_nn_ != nullptr) {
                clip_by_cell_SR(seed, ping, pong);   // "Security Radius" mode.
            } else {
                clip_by_cell(seed, ping, pong);   // Standard mode.
            }

            return ping; // Yes, 'ping', and not 'pong'
                         // see comments in clip_by_cell()
        }

        /**
         * \brief Computes the intersection between the Voronoi cell of a
         * vertex and the Mesh 'ping'.
         *
         * \details The result is returned in \p ping (Note that
         * \p ping and \p pong are references, and that they are swapped
         * after each bisector clipping, this is why the final result
         * is in \p ping (and not in \p pong).
         * This version uses the Security Radius algorithm.
         *
         * \param[in] i index of the vertex that defines the Voronoi cell
         * \param[in,out] ping the input polygon. On exit, contains the result.
         * \param[out] pong a buffer used to implement reentrant clipping.
         *  Its content is modified by the function.
         */
        void clip_by_cell_SR(index_t i, Polygon*& ping, Polygon*& pong) {
            // 'Security radius' mode.
            // Note: the vertices of the neighborhood are returned in
            // increasing distance to pi. We stop the clippings as soon as the
            // 'security radius' is reached.
            const double* geo_restrict pi = delaunay_->vertex_ptr(i);
            geo_assume_aligned(pi, geo_dim_alignment(DIM));

            index_t jj = 0;
            index_t prev_nb_neighbors = 0;
            neighbors_.resize(0);
            while(neighbors_.size() < delaunay_nn_->nb_vertices() - 1) {

                delaunay_nn_->get_neighbors(i, neighbors_);
                if(neighbors_.size() == 0) {
                    return;
                }
                if(prev_nb_neighbors == neighbors_.size()) {
                    return;
                }

                for(; jj < neighbors_.size(); jj++) {
                    index_t j = neighbors_[jj];
                    double R2 = 0.0;
                    for(index_t k = 0; k < ping->nb_vertices(); k++) {
                        geo_decl_aligned(double dik);
                        const double* geo_restrict pk = ping->vertex(k).point();
                        geo_assume_aligned(pk, geo_dim_alignment(DIM));
                        dik = GEO::Geom::distance2(pi, pk, dimension());
                        R2 = std::max(R2, dik);
                    }
                    geo_decl_aligned(double dij);
                    const double* geo_restrict pj = delaunay_->vertex_ptr(j);
                    geo_assume_aligned(pj, geo_dim_alignment(DIM));
                    dij = GEO::Geom::distance2(pi, pj, dimension());
                    // A little bit more than 4, because when
                    // exact predicates are used, we need to
                    // include tangent bisectors in the computation.
                    if(dij > 4.1 * R2) {
                        return;
                    }
                    clip_by_plane(*ping, *pong, i, j);
                    swap_polygons(ping, pong);
                }

                if(!check_SR_) {
                    return;
                }

                index_t nb_neighbors = neighbors_.size();
                prev_nb_neighbors = nb_neighbors;

                if(nb_neighbors > 8) {
                    nb_neighbors += nb_neighbors / 8;
                } else {
                    nb_neighbors++;
                }

                nb_neighbors = std::min(
                    nb_neighbors,
                    delaunay_nn_->nb_vertices() - 1
                );

                delaunay_nn_->enlarge_neighborhood(i, nb_neighbors);
            }
        }

        /**
         * \brief Computes the intersection between a Voronoi cell
         *  and a polygon.
         *
         * \details The Voronoi cell is determined by vertex \p i and
         * the input polygon is in \p ping. The result is returned
         *  in \p ping (Note that
         * \p ping and \p pong are references, and that they are swapped
         * after each bisector clipping, this is why the final result
         * is in \p ping (and not in \p pong).
         *
         * \param[in] i index of the vertex that defines the Voronoi cell
         * \param[in,out] ping the input polygon. On exit, contains the result.
         * \param[out] pong a buffer used to implement reentrant clipping.
         *  Its content is modified by the function.
         */
        void clip_by_cell(index_t i, Polygon*& ping, Polygon*& pong) {
            get_neighbors(i);
            for(index_t jj = 0; jj < neighbors_.size(); jj++) {
                index_t j = neighbors_[jj];
                clip_by_plane(*ping, *pong, i, j);
                swap_polygons(ping, pong);
            }
        }

        /**
         * \brief Computes the intersection between a polygon and a half-space.
         *
         * \details The input polygon is in \p ping
         * and the half-space is determined by the positive side
         * of the bisector of segment [\p i,\p j] (the side of \p i).
         * The result is stored into the Polygon \p pong.
         *
         * \param[in] i index of the first extremity of the bisector
         * \param[in] j index of the second extremity of the bisector
         * \param[in] ping the input polygon
         * \param[out] pong \p ping clipped by the bisector
         */

        void clip_by_plane(
            Polygon& ping, Polygon& pong,
            index_t i, index_t j
        ) {
            ping.clip_by_plane<DIM>(
                pong, intersections_, mesh_, delaunay_, i, j, exact_, symbolic_
            );
        }

        /**
         * @}
         * \name Clipping for volumetric mode
         * @{
         */
        
    public:
        /**
         * \brief Computes the intersection between a Voronoi cell
         *  and a cell with radius of security or plain mode.
         * \param[in] seed the index of the seed that defines the Voronoi cell
         * \param[in,out] C the cell to be clipped
         */
        void intersect_cell_cell(index_t seed, Polyhedron& C) {
            // Clip current facet by current Voronoi cell (associated with seed)
            if(delaunay_nn_ != nullptr) {
                clip_by_cell_SR(seed, C);   // "Security Radius" mode.
            } else {
                clip_by_cell(seed, C);   // Standard mode.
            }
        }

    protected:
        /**
         * \brief Computes the intersection between a Voronoi cell
         *  and a cell in radius-of-security mode.
         * \param[in] seed the index of the seed that defines the Voronoi cell
         * \param[in,out] C the cell to be clipped
         */
        void clip_by_cell_SR(index_t seed, Polyhedron& C) {

            // 'Security radius' mode.
            // Note: the vertices of the neighborhood are returned in
            // increasing distance to pi. We stop the clippings as soon as the
            // 'security radius' is reached.
            const double* geo_restrict pi = delaunay_->vertex_ptr(seed);
            geo_assume_aligned(pi, geo_dim_alignment(DIM));

            index_t jj = 0;
            index_t prev_nb_neighbors = 0;
            neighbors_.resize(0);

            while(neighbors_.size() < delaunay_nn_->nb_vertices() - 1) {

                delaunay_nn_->get_neighbors(seed, neighbors_);
                if(neighbors_.size() == 0) {
                    return;
                }
                if(prev_nb_neighbors == neighbors_.size()) {
                    return;
                }

                for(; jj < neighbors_.size(); jj++) {
                    index_t j = neighbors_[jj];
                    double R2 = 0.0;
                    for(index_t k = 0; k < C.max_t(); ++k) {
                        if(!C.triangle_is_used(k)) {
                            continue;
                        }
                        geo_decl_aligned(double dik);
                        const double* geo_restrict pk =
                            C.triangle_dual(k).point();
                        geo_assume_aligned(pk, geo_dim_alignment(DIM));
                        dik = GEO::Geom::distance2(pi, pk, dimension());
                        R2 = std::max(R2, dik);
                    }
                    geo_decl_aligned(double dij);
                    const double* geo_restrict pj = delaunay_->vertex_ptr(j);
                    geo_assume_aligned(pj, geo_dim_alignment(DIM));
                    dij = GEO::Geom::distance2(pi, pj, dimension());
                    // A little bit more than 4, because when
                    // exact predicates are used, we need to
                    // include tangent bisectors in the computation.
                    if(dij > 4.1 * R2) {
                        return;
                    }
                    clip_by_plane(C, seed, j);
                }

                if(!check_SR_) {
                    return;
                }

                index_t nb_neighbors = neighbors_.size();
                prev_nb_neighbors = nb_neighbors;

                if(nb_neighbors > 8) {
                    nb_neighbors += nb_neighbors / 8;
                } else {
                    nb_neighbors++;
                }

                nb_neighbors = std::min(
                    nb_neighbors,
                    delaunay_nn_->nb_vertices() - 1
                );
                delaunay_nn_->enlarge_neighborhood(seed, nb_neighbors);
            }
        }

        /**
         * \brief Computes the intersection between a Voronoi cell
         *  and a cell in plain mode.
         * \param[in] seed the index of the seed that defines the Voronoi cell
         * \param[in,out] C the cell to be clipped
         */
        void clip_by_cell(index_t seed, Polyhedron& C) {
            get_neighbors(seed);
            // Check whether cell is empty (may happen with
            // power diagrams)
            if(neighbors_.size() == 0) {
                C.clear();
            }
            for(index_t jj = 0; jj < neighbors_.size(); jj++) {
                index_t j = neighbors_[jj];
                clip_by_plane(C, seed, j);
            }
        }

        /**
         * \brief Computes the intersection between a Voronoi cell
         *  and a half-space determined by a bisector.
         * \param[in,out] C cell to be clipped
         * \param[in] i index of the first extremity of the bisector
         * \param[in] j index of the second extremity of the bisector
         */
        void clip_by_plane(Polyhedron& C, index_t i, index_t j) {
            C.clip_by_plane<DIM>(
                mesh_, delaunay_, i, j, exact_, symbolic_
            );
        }

        /**
         * @}
         * \name Optimized get neighbors
         * @{
         */

        /**
         * \brief Creates the data structure for optimized get_neighbors()
         * function.
         *
         * \details This function is only used when the stored delaunay
         *  triangulation is a traditional one. When the stored delaunay
         *  triangulation is represented by a KdTree, function is not used.
         */
        void init_get_neighbors() {
            // In dimension 3 (and if we do not used the ANN-based algorithm),
            // we can use the faster 'stamp-based' algorithm for finding the
            // neighbors.
            if(delaunay_->dimension() == 3 && delaunay_->nb_cells() != 0) {
                cur_stamp_ = 0;
                stamp_.assign(delaunay_->nb_vertices(), -1);
            }
        }

        /**
         * \brief Caches the neighbors of a Delaunay vertex.
         *
         * \details This function is only used when the stored delaunay
         *  triangulation is a traditional one. When the stored delaunay
         *  triangulation is represented by a KdTree, function is not used.
         */
        void get_neighbors(index_t v) {
            if(stamp_.size() == 0) {
                // Used in ANN mode and with higher dimensions.
                delaunay_->get_neighbors(v, neighbors_);
            } else {
                // Used in 3D mode with standard Delaunay.
                // The following loop replaces
                //    delaunay_->get_neighbors(v,neighbors_) ;
                // (and makes the overall algorithm 10 to 30% more efficient)
                neighbors_.resize(0);
                index_t t = index_t(delaunay_->vertex_cell(v));
                do {
                    index_t lv = delaunay_->index(t, signed_index_t(v));
                    for(index_t lw = 0; lw < delaunay_->cell_size(); lw++) {
                        if(lw != lv) {
                            index_t w = index_t(delaunay_->cell_vertex(t, lw));
                            if(stamp_[w] != cur_stamp_) {
                                stamp_[w] = cur_stamp_;
                                neighbors_.push_back(w);
                            }
                        }
                    }
                    t = index_t(delaunay_->next_around_vertex(t, lv));
                } while(t != index_t(delaunay_->vertex_cell(v)));
                cur_stamp_++;
            }
        }

        /** @} */

    protected:
        GEO::Mesh* mesh_;
        Delaunay* delaunay_;
        GEO::Delaunay_NearestNeighbors* delaunay_nn_;

        PointAllocator intersections_;
        Polygon* current_polygon_;
        Polygon P1, P2;
        GEO::vector<index_t> neighbors_;
        index_t current_facet_;
        index_t current_seed_;
        Polyhedron* current_polyhedron_;
        index_t current_tet_;

        // For optimized get_neighbors().
        signed_index_t cur_stamp_;
        GEO::vector<signed_index_t> stamp_;

        bool symbolic_;
        bool check_SR_;
        bool exact_;

        coord_index_t dimension_;

        static const index_t UNSPECIFIED_RANGE = index_t(-1);

        index_t facets_begin_;
        index_t facets_end_;

        index_t tets_begin_;
        index_t tets_end_;

        bool connected_components_priority_;
        FacetSeedMarking* facet_seed_marking_;
        bool connected_component_changed_;
        index_t current_connected_component_;

    private:
        /**
         * \brief Forbids construction from copy.
         */
        RestrictedVoronoiDiagram(const thisclass& rhs);

        /**
         * \brief Forbids assignment.
         */
        thisclass& operator= (const thisclass& rhs);
    };
}

namespace GEO {

    /**
     * \brief Symbolic representation of a RestrictedVoronoiDiagram vertex.
     */
    typedef GEOGen::SymbolicVertex SymbolicVertex;
}

#endif

