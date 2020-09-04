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

#ifndef GEOGRAM_MESH_MESH_GEOMETRY
#define GEOGRAM_MESH_MESH_GEOMETRY

#include <geogram/basic/common.h>
#include <geogram/mesh/mesh.h>
#include <geogram/basic/geometry.h>
#include <geogram/basic/geometry_nd.h>

/**
 * \file geogram/mesh/mesh_geometry.h
 * \brief Functions for accessing the geometry in a mesh
 */

namespace GEO {

    namespace Geom {

        /**
         * \brief Gets a mesh vertex by its index.
         * \param[in] M the mesh
         * \param[in] v the index of the vertex
         * \return a const reference to the \p v%th vertex of a mesh
         * \pre M.vertices.dimension() >= 3
         */
        inline const vec3& mesh_vertex(const Mesh& M, index_t v) {
            geo_debug_assert(M.vertices.dimension() >= 3);
            return *(const vec3*) (M.vertices.point_ptr(v));
        }

        /**
         * \brief Gets a mesh vertex by its index.
         * \param[in] M the mesh
         * \param[in] v the index of the vertex
         * \return a const reference to the \p v%th vertex of a mesh
         * \pre M.vertices.dimension() >= 3
         */
        inline const vec3& mesh_vertex_ref(const Mesh& M, index_t v) {
            geo_debug_assert(M.vertices.dimension() >= 3);
            return *(vec3 const *) (M.vertices.point_ptr(v));
        }

        /**
         * \brief Gets a mesh vertex by its index.
         * \param[in] M the mesh
         * \param[in] v the index of the vertex
         * \return a reference to the \p v%th vertex of a mesh
         * \pre M.vertices.dimension() >= 3
         */
        inline vec3& mesh_vertex_ref(Mesh& M, index_t v) {
            geo_debug_assert(M.vertices.dimension() >= 3);
            return *(vec3*) (M.vertices.point_ptr(v));
        }
	
        /**
         * \brief Gets a mesh vertex by an incident corner index.
         * \param[in] M the mesh
         * \param[in] c the index of a corner incident to the vertex
         * \return a reference to the \p v%th vertex of a mesh
         * \pre M.vertices.dimension() >= 3
         */
        inline const vec3& mesh_corner_vertex(const Mesh& M, index_t c) {
            return mesh_vertex(M, M.facet_corners.vertex(c));
        }

        /**
         * \brief Gets a mesh vertex by an incident corner index.
         * \param[in] M the mesh
         * \param[in] c the index of a corner incident to the vertex
         * \return a const reference to the \p v%th vertex of a mesh
         * \pre M.vertices.dimension() >= 3
         */
        inline vec3& mesh_corner_vertex_ref(Mesh& M, index_t c) {
            return mesh_vertex_ref(M, M.facet_corners.vertex(c));
        }

        /**
         * \brief Gets a mesh vertex normal by vertex index.
         * \param[in] M the mesh
         * \param[in] v the index of the vertex
         * \return a const reference to the stored normal of vertex \p v
         * \pre M.vertices.dimension() >= 6
         */
        inline const vec3& mesh_vertex_normal(const Mesh& M, index_t v) {
            geo_debug_assert(M.vertices.dimension() >= 6);
            return *(const vec3*) (M.vertices.point_ptr(v) + 3);
        }

        /**
         * \brief Gets a mesh vertex normal by vertex index.
         * \param[in] M the mesh
         * \param[in] v the index of the vertex
         * \return a reference to the stored normal of vertex \p v
         * \pre M.vertices.dimension() >= 6
         */
        inline vec3& mesh_vertex_normal_ref(Mesh& M, index_t v) {
            geo_debug_assert(M.vertices.dimension() >= 6);
            return *(vec3*) (M.vertices.point_ptr(v) + 3);
        }

        /**
         * \brief Gets a mesh vertex normal by vertex index.
         * \param[in] M the mesh
         * \param[in] v the index of the vertex
         * \return a const reference to the stored normal of vertex \p v
         * \pre M.vertices.dimension() >= 6
         */
        inline const vec3& mesh_vertex_normal_ref(const Mesh& M, index_t v) {
            geo_debug_assert(M.vertices.dimension() >= 6);
            return *(vec3 const *) (M.vertices.point_ptr(v) + 3);
        }

        /**
         * \brief Computes the area of a facet.
         * \param[in] M a const reference to the mesh
         * \param[in] f index of the facet
         * \param[in] dim dimension that will be used to compute the area
         * \return the area of the facet, obtained by considering the
         *  \p dim first coordinates of the vertices only
         */
        inline double mesh_facet_area(const Mesh& M, index_t f, index_t dim=0) {
            geo_debug_assert(dim <= M.vertices.dimension());
            if(dim == 0) {
                dim = M.vertices.dimension();
            }
            double result = 0.0;
            // Check for empty facet, should not happen.
            if(M.facets.corners_end(f) == M.facets.corners_begin(f)) {
                return result;
            }
            const double* p0 = M.vertices.point_ptr(
                M.facet_corners.vertex(M.facets.corners_begin(f))
            );
            for(
                index_t i = M.facets.corners_begin(f) + 1;
                i + 1 < M.facets.corners_end(f); i++
            ) {
                result += GEO::Geom::triangle_area(
                    p0,
                    M.vertices.point_ptr(M.facet_corners.vertex(i)),
                    M.vertices.point_ptr(M.facet_corners.vertex(i + 1)),
                    coord_index_t(dim)
                );
            }
            return result;
        }
        
        /**
         * \brief Computes the normal to a mesh facet.
         * \param[in] M the mesh
         * \param[in] f the facet index in \p M
         * \return the normal to facet \p f
         * \pre dimension >= 3
         * \note the computed vector is not normalized.
         */
        vec3 GEOGRAM_API  mesh_facet_normal(const Mesh& M, index_t f);

        /**
         * \brief Gets the centroid of the vertices of a facet in a mesh.
         * \param[in] M the mesh
         * \param[in] f the index of the facet
         * \return the 3d centroid of facet \p f in \p M
         */
        inline vec3 mesh_facet_center(const Mesh& M, index_t f) {
            vec3 result(0.0, 0.0, 0.0);
            double count = 0.0;
            for(index_t c = M.facets.corners_begin(f);
                c < M.facets.corners_end(f); ++c) {
                result += Geom::mesh_corner_vertex(M, c);
                count += 1.0;
            }
            return (1.0 / count) * result;
        }


        /**
         * \brief Gets the centroid of a tetrahedron in a mesh.
         * \param[in] M the mesh
         * \param[in] t the index of the tetrahedron
         * \return the 3d centroid of tetrahedron \p t in \p M
         */
        inline vec3 mesh_tet_center(const Mesh& M, index_t t) {
            index_t iv1 = M.cells.vertex(t, 0);
            index_t iv2 = M.cells.vertex(t, 1);
            index_t iv3 = M.cells.vertex(t, 2);
            index_t iv4 = M.cells.vertex(t, 3);
            const vec3& v1 = Geom::mesh_vertex(M, iv1);
            const vec3& v2 = Geom::mesh_vertex(M, iv2);
            const vec3& v3 = Geom::mesh_vertex(M, iv3);
            const vec3& v4 = Geom::mesh_vertex(M, iv4);
            return 0.25 * (v1 + v2 + v3 + v4);
        }

        /**
         * \brief Gets a vector by a mesh corner.
         * \param[in] M a const reference to the mesh
         * \param[in] c1 a corner index in \p M
         * \return a vector originating at \p c1 and 
         *  pointing at the next corner around the facet
         *  incident to \p c1
         * \pre M.facets.are_simplices()
         */
        inline vec3 mesh_corner_vector(const Mesh& M, index_t c1) {
            geo_debug_assert(M.facets.are_simplices());
            index_t c2 = M.facets.next_corner_around_facet(c1/3, c1);
            index_t v1 = M.facet_corners.vertex(c1);
            index_t v2 = M.facet_corners.vertex(c2);
            return mesh_vertex(M,v2) - mesh_vertex(M,v1);
        }

        /**
         * \brief Computes the angle between the normal vectors
	 *  of two mesh facets sharing an edge.
         * \param[in] M a const reference to the mesh
         * \param[in] c a corner index in \p M
         * \return the angle between the facet that contains c and
         *  the facet adjacent to c
         * \pre M.facets.are_simplices() && M.corner_adjacent_facet(c) != -1
         */
        double GEOGRAM_API mesh_normal_angle(const Mesh& M, index_t c);

        /**
         * \brief Computes the angle between the normal vectors
	 *  of two mesh facets sharing an edge.
         * \param[in] M a const reference to the mesh
         * \param[in] f1 , f2 two facets of the mesh
         * \return the angle between \p f1 and \p f2
         */
	double GEOGRAM_API mesh_unsigned_normal_angle(
	    const Mesh& M, index_t f1, index_t f2
	);

        /**
         * \brief Computes the total surface area of a mesh in arbitrary
         *  dimension.
         * \param[in] M the mesh
         * \param[in] dim the dimension to be used for the computation
         * \return the area of the mesh \p M computed in dim \p d.
         * \pre dim <= M.vertices.dimension()
         */
        double GEOGRAM_API mesh_area(const Mesh& M, index_t dim);

        /**
         * \brief Computes the total surface area of a mesh.
         * \param[in] M the mesh
         * \return the area of the mesh computed in M.vertices.dimension() dim.
         */
        inline double mesh_area(const Mesh& M) {
            return mesh_area(M, M.vertices.dimension());
        }
    }

    /**
     * \brief Computes the normals to the vertices, and stores
     *  them as additional coordinates.
     * \param[in,out] M the mesh
     */
    void GEOGRAM_API compute_normals(Mesh& M);

    /**
     * \brief Smoothes a mesh.
     * \details Moves each point of mesh \p M to the barycenter of its
     * neighbors. This operation is repeated the specified number of times \p
     * nb_iter.
     * \param[in,out] M the mesh to smooth
     * \param[in] nb_iter number of smoothing iterations
     * \param[in] normals_only if set, only stored normals are smoothed.
     */
    void GEOGRAM_API simple_Laplacian_smooth(
        Mesh& M, index_t nb_iter, bool normals_only
    );

    /**
     * \brief Gets the bounding box of a mesh.
     * \param[in] M The mesh
     * \param[out] xyzmin the lower corner of the bounding box
     * \param[out] xyzmax the upper corner of the bounding box
     */
    void GEOGRAM_API get_bbox(const Mesh& M, double* xyzmin, double* xyzmax);

    /**
     * \brief Computes the length of the bounding box diagonal of a mesh.
     * \param[in] M the mesh
     * \return The length of \p M%'s bounding box diagonal
     */
    double GEOGRAM_API bbox_diagonal(const Mesh& M);

    /**
     * \brief Normalizes and scales the stored vertex normals by a factor.
     * \details If no normal are stored, then they are created and
     *  computed. Normals are stored in coordinates 3,4,5 of the vertices.
     * \param[in,out] M the mesh
     * \param[in] s the factor used to scale the normals
     */
    void GEOGRAM_API set_anisotropy(Mesh& M, double s);

    /**
     * \brief Normalizes the stored vertex normals.
     * \param[in,out] M the mesh
     */
    void GEOGRAM_API unset_anisotropy(Mesh& M);

    /**
     * \brief Computes a sizing field using an estimate of lfs
     *  (local feature size).
     * \details The sizing field is stored in \p M%'s vertices weights.
     * \param[in,out] M the mesh
     * \param[in] gradation the exponent to be applied to the sizing field
     * \param[in] nb_lfs_samples if set to 0, the vertices of \p M are used,
     *  else \p M is resampled (needed if \p M's facets density is
     *  highly irregular).
     */
    void GEOGRAM_API compute_sizing_field(
        Mesh& M, double gradation = 1.0, index_t nb_lfs_samples = 0
    );

    /**
     * \brief Computes vertices weights in such a way that triangle
     *  areas are normalized.
     * \details If this function is used, then
     *  CentroidalVoronoiTesselation generates Voronoi cells of
     *  equal areas.
     * \param[in,out] M the mesh
     */
    void GEOGRAM_API normalize_embedding_area(Mesh& M);

    /**
     * \brief Computes the volume of a cell in a mesh.
     * \param[in] M a const reference to the mesh
     * \param[in] c the index of the cell
     * \return the volume of the cell 
     * \pre c < M.cells.nb()
     */
    double GEOGRAM_API mesh_cell_volume(
        const Mesh& M, index_t c
    );
    
    /**
     * \brief Computes the volume of the cells of a mesh.
     * \param[in] M a const reference to the mesh
     * \return the volume of the cells of the mesh
     */
    double GEOGRAM_API mesh_cells_volume(const Mesh& M);


    /**
     * \brief Computes the normal of a cell facet.
     * \param[in] M a const reference to the mesh
     * \param[in] c the index of the cell
     * \param[in] lf the local index of the facet within cell \p c
     * \return the vector normal to facet \p lf in cell \p c
     * \pre c < M.cells.nb() && lf < M.cells
     * \note the computed vector is not normalized
     */
    vec3 GEOGRAM_API mesh_cell_facet_normal(
        const Mesh& M, index_t c, index_t lf
    );
    
    /**
     * \brief Computes the average edge length in a surface.
     * \param[in] M a const reference to a surface mesh
     * \return the average edge length
     */
    double GEOGRAM_API surface_average_edge_length(
        const Mesh& M
    );
}

#endif

