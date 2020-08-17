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

#ifndef GEOGRAM_VORONOI_GENERIC_RVD_POLYGON
#define GEOGRAM_VORONOI_GENERIC_RVD_POLYGON

#include <geogram/basic/common.h>
#include <geogram/voronoi/generic_RVD_vertex.h>
#include <geogram/basic/attributes.h>

/**
 * \file geogram/voronoi/generic_RVD_polygon.h
 * \brief Internal representation of polygons for GenericVoronoiDiagram.
 * \note This file contains functions and classes used by the internal 
 *  implementation of GEO::GenericVoronoiDiagram. 
 *  They are not meant to be used directly by client code.
 */

namespace GEOGen {

    /**
     * \brief Internal representation of polygons for GenericVoronoiDiagram.
     * \details Stores both geometrical and symbolic representations.
     * \note This is an internal implementation class used by
     *  GEO::RestrictedVoronoiDiagram. It is not meant to be 
     *  used directly by client code.
     */
    class Polygon {
    public:

        /**
         * \brief Gets the number of vertices.
         */
        index_t nb_vertices() const {
            return index_t(vertex_.size());
        }

        /**
         * \brief Gets a vertex by index.
         * \param[in] i index of the Vertex in this Polygon
         * \return a const reference to the Vertex at index \p i
         * \pre \p i < nb_vertices()
         */
        const Vertex& vertex(index_t i) const {
            geo_debug_assert(i < nb_vertices());
            return vertex_[i];
        }

        /**
         * \brief Gets a vertex by index.
         * \param[in] i index of the Vertex in this Polygon
         * \return a reference to the Vertex at index \p i
         * \pre \p i < nb_vertices()
         */
        Vertex& vertex(index_t i) {
            geo_debug_assert(i < nb_vertices());
            return vertex_[i];
        }

        /**
         * \brief Gets the index of the successor of a Vertex.
         * \param[in] i index of the Vertex in this Polygon
         * \return the index of the successor of Vertex \p i
         * \pre \p i < nb_vertices()
         */
        index_t next_vertex(index_t i) const {
            geo_debug_assert(i < nb_vertices());
            return
                (i == nb_vertices() - 1) ? 0 : (i + 1)
            ;
        }

        /**
         * \brief Gets the index of the predecessor of a Vertex.
         * \param[in] i index of the Vertex in this Polygon
         * \return the index of the predecessor of Vertex \p i
         * \pre \p ii < nb_vertices()
         */
        index_t prev_vertex(index_t i) const {
            geo_debug_assert(i < nb_vertices());
            return (i == 0) ? (nb_vertices() - 1) : (i - 1);
        }

        /**
         * \brief Adds a Vertex to this Polygon.
         * \param[in] v the vertex to be added. It is copied.
         * \return the address of the stored vertex.
         */
        Vertex* add_vertex(const Vertex& v) {
            vertex_.push_back(v);
            return &*(vertex_.rbegin());
        }

        /**
         * \brief Clears this Polygon.
         */
        void clear() {
            vertex_.resize(0);
        }

        /**
         * \brief Resizes this Polygon.
         * \param[in] sz new size
         */
        void resize(index_t sz) {
            vertex_.resize(sz);
        }

        /**
         * \brief Assigns a mesh facet to this Polygon.
         * \details The facet from the initial mesh is converted into
         *  the internal geometric/symbolic representation.
         * \param[in] mesh the mesh from which the facet is copied
         * \param[in] f the index of the facet in \p mesh
         * \param[in] symbolic if true, symbolic information is copied
         * \param[in] vertex_weight a reference to a vertex attribute
         *  that stores weights. If not bound, then 1.0 is used for
         *  the weights.
         */
        void initialize_from_mesh_facet(
            const Mesh* mesh, index_t f, bool symbolic,
            const GEO::Attribute<double>& vertex_weight
        );

        /**
         * \brief Clips a polygon with a plane.
         * \details Computes the intersection between this Polygon
         * and the half-space determined by the positive side
         * of the bisector of segment [i,j] (on the same side as vertex i).
         *
         * \param[out] target where to store the intersection
         * \param[out] target_intersections
         *    where to allocate the generated vertices
         * \param[in] mesh the input mesh, used by the symbolic information
         * \param[in] delaunay the Delaunay triangulation
         * \param[in] i index of one extremity of bisector in \p delaunay
         * \param[in] j index of the other extremity of the bisector
         *    in \p delaunay
         * \param[in] exact if true, exact predicates are used.
         *   Implies symbolic.
         * \param[in] symbolic if true, symbolic representation
         *   of vertices is computed
         */
        template <index_t DIM>
        void clip_by_plane(
            Polygon& target, PointAllocator& target_intersections,
            const Mesh* mesh, const Delaunay* delaunay,
            index_t i, index_t j,
            bool exact, bool symbolic
        ) {
            if(exact) {
                clip_by_plane_exact<DIM>(
                    target, target_intersections, mesh, delaunay, i, j
                );
            } else {
                clip_by_plane_fast<DIM>(
                    target, target_intersections, delaunay, i, j, symbolic
                );
            }
        }

	/**
	 * \brief Overwrites this Polygon with the contents of another
	 *  polygon.
	 * \param[in] rhs a const reference to the polygon to be copied.
	 */
	void copy(const Polygon& rhs) {
	    vertex_ = rhs.vertex_;
	}

	/**
	 * \brief Swaps the contents of this Polygon and another polygon.
	 * \param[in,out] rhs a reference to the Polygon to be swapped with
	 *  this one.
	 */
	void swap(Polygon& rhs) {
	    vertex_.swap(rhs.vertex_);
	}
	
    protected:
        /**
         * \brief Clips a Polygon with a plane (fast inexact version).
         * \details Computes the intersection between this Polygon
         * and the half-space determined by the positive side
         * of the bisector of segment [i,j] (the side of i).
         * This version uses a "fused" predicates-constructions
         * strategy (and reuses the computations from the predicates
         * to accelerate the constructions).
         *
         * \param[out] target where to store the intersection
         * \param[out] target_intersections
         *   where to allocate the generated vertices
         * \param[in] delaunay the Delaunay triangulation
         * \param[in] i index of one extremity of bisector in \p delaunay
         * \param[in] j index of the other extremity
         *   of the bisector in \p delaunay
         * \param[in] symbolic if true, symbolic representation
         *  of vertices is computed
         *
         * \internal
         * \note Profiling revealed that this routine is where
         * the system spends the largest amount of time
         * (no big surprise...).
         */
        template <index_t DIM>
        void clip_by_plane_fast(
            Polygon& target, PointAllocator& target_intersections,
            const Delaunay* delaunay, index_t i, index_t j,
            bool symbolic
        ) const {
            target.clear();
            if(nb_vertices() == 0) {
                return;
            }

            const double* geo_restrict pi = delaunay->vertex_ptr(i);
            geo_assume_aligned(pi, geo_dim_alignment(DIM));
            const double* geo_restrict pj = delaunay->vertex_ptr(j);
            geo_assume_aligned(pj, geo_dim_alignment(DIM));

            // Compute d = n . m, where n is the
            // normal vector of the bisector [pi,pj]
            // and m the middle point of the bisector.
            geo_decl_aligned(double d);
            d = 0;
            for(coord_index_t c = 0; c < DIM; ++c) {
                d += (pi[c] + pj[c]) * (pi[c] - pj[c]);
            }

            // The predecessor of the first vertex is the last vertex
            index_t prev_k = nb_vertices() - 1;
            const Vertex* prev_vk = &(vertex(prev_k));
            const double* geo_restrict prev_pk = prev_vk->point();
            geo_assume_aligned(prev_pk, geo_dim_alignment(DIM));

            // We compute:
            //    prev_l = prev_vk . n
            geo_decl_aligned(double prev_l);
            prev_l = 0.0;
            for(coord_index_t c = 0; c < DIM; ++c) {
                prev_l += prev_pk[c] * (pi[c] - pj[c]);
            }

            // We compute:
            //    side1(pi,pj,q) = sign(2*q.n - n.m) = sign(2*l - d)
            GEO::Sign prev_status = GEO::geo_sgn(2.0 * prev_l - d);

            for(index_t k = 0; k < nb_vertices(); k++) {
                const Vertex* vk = &(vertex(k));
                const double* pk = vk->point();

                // We compute: l = vk . n
                geo_decl_aligned(double l);
                l = 0.0;
                for(coord_index_t c = 0; c < DIM; ++c) {
                    l += pk[c] * (pi[c] - pj[c]);
                }

                // We compute:
                //   side1(pi,pj,q) = sign(2*q.n - n.m) = sign(2*l - d)
                GEO::Sign status = GEO::geo_sgn(2.0 * l - d);

                // If status of edge extremities differ,
                // then there is an intersection.
                if(status != prev_status && (prev_status != 0)) {
                    Vertex I;
                    double* Ipoint = target_intersections.new_item();
                    I.set_point(Ipoint);
                    if(symbolic) {
                        if(
                            !I.sym().intersect_symbolic(
                                prev_vk->sym(), vk->sym(), j
                            )
                        ) {
                            // We encountered a problem. As a workaround,
                            // we copy prev_vk into the result.
                            I = *prev_vk;
                        }
                    }

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
                    // Compute intersection I by weighting
                    // the edge extremities with the barycentric
                    // coordinates lambda1 and lambda2
                    for(coord_index_t c = 0; c < DIM; ++c) {
                        Ipoint[c] =
                            lambda1 * prev_pk[c] +
                            lambda2 * pk[c];
                    }
                    I.set_weight(
                        lambda1 * prev_vk->weight() + lambda2 * vk->weight()
                    );
                    if(status > 0) {
                        I.copy_edge_from(*prev_vk);
                        I.set_adjacent_seed(signed_index_t(j));
                    } else {
                        I.set_flag(INTERSECT);
                        I.set_adjacent_seed(vk->adjacent_seed());
                    }
                    target.add_vertex(I);
                }
                if(status > 0) {
                    target.add_vertex(*vk);
                }
                prev_vk = vk;
                prev_pk = pk;
                prev_status = status;
                prev_k = k;
                prev_l = l;
            }
        }

        /**
         * \brief Clips a Polygon with a plane (exact version).
         * \details Computes the intersection between this Polygon
         * and the half-space determined by the positive side
         * of the bisector of segment [i,j] (the side of i).
         * This version uses symbolically perturbed exact predicates.
         *
         * \param[out] target where to store the intersection
         * \param[out] target_intersections
         *  where to allocate the generated vertices
         * \param[in] mesh the input mesh (used by exact predicates)
         * \param[in] delaunay the Delaunay triangulation
         * \param[in] i index of one extremity of bisector in \p delaunay
         * \param[in] j index of the other extremity of
         *  the bisector in \p delaunay
         */
        template <index_t DIM>
        void clip_by_plane_exact(
            Polygon& target, PointAllocator& target_intersections,
            const Mesh* mesh, const Delaunay* delaunay,
            index_t i, index_t j
        ) {
            target.clear();
            if(nb_vertices() == 0) {
                return;
            }

            const double* pi = delaunay->vertex_ptr(i);
            const double* pj = delaunay->vertex_ptr(j);

            // The predecessor of the first vertex is the last vertex
            index_t prev_k = nb_vertices() - 1;
            const Vertex* prev_vk = &(vertex(prev_k));
            Sign prev_status = side_exact(
                mesh, delaunay, *prev_vk, pi, pj, DIM
            );

            for(index_t k = 0; k < nb_vertices(); ++k) {
                const Vertex* vk = &(vertex(k));
                Sign status = side_exact(mesh, delaunay, *vk, pi, pj, DIM);

                // If status of edge extremities differ,
                // there is an intersection.
                if(status != prev_status && (prev_status != 0)) {

                    Vertex I;
                    if(
                        !I.sym().intersect_symbolic(
                            prev_vk->sym(), vk->sym(), j
                        )
                    ) {
                        // We encountered a problem. As a workaround,
                        // we copy prev_vk into the result.
                        I = *prev_vk;
                        // geo_assert_not_reached ;
                        // not supposed to happen in exact mode
                    }
                    I.intersect_geom<DIM>(
                        target_intersections, *prev_vk, *vk, pi, pj
                    );
                    if(status > 0) {
                        I.copy_edge_from(*prev_vk);
                        I.set_adjacent_seed(signed_index_t(j));
                    } else {
                        I.set_flag(INTERSECT);
                        I.set_adjacent_seed(vk->adjacent_seed());
                    }
                    target.add_vertex(I);
                }
                if(status > 0) {
                    target.add_vertex(*vk);
                }
                prev_vk = vk;
                prev_status = status;
                prev_k = k;
            }
        }

        /**
         * \brief Returns the position of a point
         * relative to a bisector (exact version).
         * \details Position of q relative to the bisector Pi(i,j).
         *  The symbolic representation of q is used. Symbolic
         *  perturbation is applied to degenerate configurations,
         *  therefore ZERO is never returned.
         * \param[in] mesh the input mesh
         * \param[in] delaunay the Delaunay triangulation
         * \param[in] q query point
         * \param[in] pi one extremity of the bisector
         * \param[in] pj the other extremity of the bisector
         * \param[in] dim dimension of the points
         * \return POSITIVE if q is on pi's side, NEGATIVE otherwise
         *  (ZERO is never encountered thanks to globally coherent
         *  symbolic perturbations).
         */
        static Sign side_exact(
            const Mesh* mesh, const Delaunay* delaunay,
            const Vertex& q, const double* pi, const double* pj,
            coord_index_t dim
        );

    private:
        GEO::vector<Vertex> vertex_;
    };
}

#endif

