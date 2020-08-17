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

#ifndef GEOGRAM_MESH_MESH_SAMPLING
#define GEOGRAM_MESH_MESH_SAMPLING

#include <geogram/basic/common.h>
#include <geogram/mesh/mesh.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/basic/geometry_nd.h>
#include <geogram/basic/logger.h>
#include <algorithm>

/**
 * \file geogram/mesh/mesh_sampling.h
 * \brief Functions to generate random samples
 *  on surfacic and in volumetric meshes
 */

namespace GEO {

    /**
     * \brief Computes the mass of a mesh facet.
     * \details The function can optionally take into account the vertex
     *  weights.
     * \param[in] mesh the surface mesh
     * \param[in] f a facet index in \p mesh
     * \param[in] vertex_weight a reference to a vertex attribute. If
     *  it is bound, it is used to weight the vertices.
     * \return the mass of facet \p f in \p mesh
     */
    template <index_t DIM>
    inline double mesh_facet_mass(
        const Mesh& mesh,
        index_t f,
        Attribute<double>& vertex_weight
    ) {
        geo_debug_assert(mesh.facets.are_simplices());
        geo_debug_assert(mesh.vertices.dimension() >= DIM);
        typedef vecng<DIM, double> Point;
        index_t v1 = mesh.facets.vertex(f,0);
        index_t v2 = mesh.facets.vertex(f,1);
        index_t v3 = mesh.facets.vertex(f,2);        
        
        if(vertex_weight.is_bound()) {
            return Geom::triangle_mass(
                *reinterpret_cast<const Point*>(mesh.vertices.point_ptr(v1)),
                *reinterpret_cast<const Point*>(mesh.vertices.point_ptr(v2)),
                *reinterpret_cast<const Point*>(mesh.vertices.point_ptr(v3)),
                vertex_weight[v1],
                vertex_weight[v2],
                vertex_weight[v3]
            );
        }
        return Geom::triangle_area(
            *reinterpret_cast<const Point*>(mesh.vertices.point_ptr(v1)),
            *reinterpret_cast<const Point*>(mesh.vertices.point_ptr(v2)),
            *reinterpret_cast<const Point*>(mesh.vertices.point_ptr(v3))
        );
    }

    /**
     * \brief Generates a set of random samples over a surfacic mesh.
     * \param[in] mesh the mesh
     * \param[out] p pointer to an array of generated samples, of size
     *   \p nb_points times DIM. To be allocated by the caller.
     * \param[in] nb_points number of points to generate
     * \param[in] weight a reference to a vertex attribute. If bound, it
     *  is taken into account.
     * \param[in] facets_begin_in if specified, first index of the facet
     *  sequence in which points should be generated. If left unspecified (-1),
     *  points are generated over all the facets of the mesh.
     * \param[in] facets_end_in if specified, one position past the last
     *  index of the facet sequence in which points should be generated.
     *  If left unspecified (-1), points are generated over all the facets
     *  of the mesh.
     * \tparam DIM dimension of the points, specified as a template argument
     *  for efficiency reasons
     * \return true if everything went OK, false otherwise. Whenever all the
     *  points land in the same facet, the function returns false to notify
     *  a potential numerical problem.
     */
    template <index_t DIM>
    inline bool mesh_generate_random_samples_on_surface(
        const Mesh& mesh,
        double* p,
        index_t nb_points,
        Attribute<double>& weight,
        signed_index_t facets_begin_in = -1,
        signed_index_t facets_end_in = -1
    ) {
        geo_assert(mesh.facets.are_simplices());
        geo_assert(mesh.vertices.dimension() >= DIM);
        geo_assert(mesh.facets.nb() > 0);

        index_t facets_begin = 0;
        index_t facets_end = mesh.facets.nb();
        if(facets_begin_in != -1) {
            facets_begin = index_t(facets_begin_in);
        }
        if(facets_end_in != -1) {
            facets_end = index_t(facets_end_in);
        }

        typedef vecng<DIM, double> Point;

        // To ensure reproducibility accros successive
        // runs, reset the random number generator.
        Numeric::random_reset();

        vector<double> s(nb_points);
        for(index_t i = 0; i < nb_points; i++) {
            s[i] = Numeric::random_float64();
        }
        std::sort(s.begin(), s.end());

        double Atot = 0.0;
        for(index_t t = facets_begin; t < facets_end; ++t) {
            double At = mesh_facet_mass<DIM>(mesh, t, weight);
            Atot += At;
        }

        signed_index_t first_t = -1;
        signed_index_t last_t = 0;

        index_t cur_t = facets_begin;
        double cur_s =
            mesh_facet_mass<DIM>(mesh, facets_begin, weight) / Atot;
        for(index_t i = 0; i < nb_points; i++) {
            geo_debug_assert(i < s.size());
            while(s[i] > cur_s && cur_t < facets_end - 1) {
                cur_t++;
                geo_debug_assert(cur_t < facets_end);
                cur_s += mesh_facet_mass<DIM>(mesh, cur_t, weight) / Atot;
            }
            if(first_t == -1) {
                first_t = signed_index_t(cur_t);
            }
            last_t = std::max(last_t, signed_index_t(cur_t));

            // TODO: take weights into account
            //  with a new random_point_in_triangle_weighted()
            //  function.
            index_t v1 = mesh.facets.vertex(cur_t,0);
            index_t v2 = mesh.facets.vertex(cur_t,1);
            index_t v3 = mesh.facets.vertex(cur_t,2);            
            Point cur_p = Geom::random_point_in_triangle(
                *reinterpret_cast<const Point*>(mesh.vertices.point_ptr(v1)),
                *reinterpret_cast<const Point*>(mesh.vertices.point_ptr(v2)),
                *reinterpret_cast<const Point*>(mesh.vertices.point_ptr(v3))
            );
            for(coord_index_t coord = 0; coord < DIM; coord++) {
                p[i * DIM + coord] = cur_p[coord];
            }
        }
        if(mesh.facets.nb() > 1 && last_t == first_t) {
            Logger::warn("Sampler")
                << "Did put all the points in the same triangle"
                << std::endl;
            return false;
        }
        return true;
    }

    /************************************************************************/

    /**
     * \brief Computes the mass of a mesh tetrahedron.
     * \details The function can optionally take into account the vertex
     *  weights.
     * \param[in] mesh the surface mesh
     * \param[in] t a tetrahedron index in \p mesh
     * \return the mass of tetrahedron \p t in \p mesh
     */
    template <index_t DIM>
    inline double mesh_tetra_mass(
        const Mesh& mesh,
        index_t t
    ) {
        geo_debug_assert(mesh.vertices.dimension() >= DIM);
        typedef vecng<DIM, double> Point;

        index_t v0 = mesh.cells.vertex(t, 0);
        index_t v1 = mesh.cells.vertex(t, 1);
        index_t v2 = mesh.cells.vertex(t, 2);
        index_t v3 = mesh.cells.vertex(t, 3);

        double result = Geom::tetra_volume(
            *reinterpret_cast<const Point*>(mesh.vertices.point_ptr(v0)),
            *reinterpret_cast<const Point*>(mesh.vertices.point_ptr(v1)),
            *reinterpret_cast<const Point*>(mesh.vertices.point_ptr(v2)),
            *reinterpret_cast<const Point*>(mesh.vertices.point_ptr(v3))
        );

        return result;
    }

    /**
     * \brief Computes the mass of a mesh tetrahedron.
     * \details The function can optionally take into account the vertex
     *  weights.
     * \param[in] mesh the surface mesh
     * \param[in] t a tetrahedron index in \p mesh
     * \param[in] weight a reference to a vertex weight attribute. If it
     *  is bound, it is taken into account in mass computation
     * \return the mass of tetrahedron \p t in \p mesh
     */
    template <index_t DIM>
    inline double mesh_tetra_mass(
        const Mesh& mesh,
        index_t t,
        const Attribute<double>& weight 
    ) {
        double result = mesh_tetra_mass<DIM>(mesh, t);

        if(weight.is_bound()) {
            index_t v0 = mesh.cells.vertex(t, 0);
            index_t v1 = mesh.cells.vertex(t, 1);
            index_t v2 = mesh.cells.vertex(t, 2);
            index_t v3 = mesh.cells.vertex(t, 3);
            result *= (
                weight[v0] + weight[v1] +
                weight[v2] + weight[v3]
            ) / 4.0;
            // TODO: check whether this is the correct formula
        }

        return result;
    }
    
    /**
     * \brief Generates a set of random samples in a volumetric mesh.
     * \param[in] mesh the mesh
     * \param[out] p pointer to an array of generated samples, of size
     *   \p nb_points times DIM. To be allocated by the caller.
     * \param[in] nb_points number of points to generate
     * \param[in] vertex_weight if bound, vertex weights are taken into account
     * \param[in] tets_begin_in if specified, first index of the tetrahedron
     *  sequence in which points should be generated. If left unspecified (-1),
     *  points are generated over all the tetrahedra of the mesh.
     * \param[in] tets_end_in if specified, one position past the last
     *  index of the tetrahedron sequence in which points should be generated.
     *  If left unspecified (-1), points are generated over all the tetrahedra
     *  of the mesh.
     * \tparam DIM dimension of the points, specified as a template argument
     *  for efficiency reasons
     * \return true if everything went OK, false otherwise. Whenever all the
     *  points land in the same tetrahedron, the function returns false
     *  to notify potential numerical problem.
     */
    template <index_t DIM>
    inline bool mesh_generate_random_samples_in_volume(
        const Mesh& mesh,
        double* p,
        index_t nb_points,
        Attribute<double>& vertex_weight,
        signed_index_t tets_begin_in = -1,
        signed_index_t tets_end_in = -1
    ) {
        geo_assert(mesh.vertices.dimension() >= DIM);
        geo_assert(mesh.cells.nb() > 0);

        index_t tets_begin = 0;
        index_t tets_end = mesh.cells.nb();
        if(tets_begin_in != -1) {
            tets_begin = index_t(tets_begin_in);
        }
        if(tets_end_in != -1) {
            tets_end = index_t(tets_end_in);
        }

        typedef vecng<DIM, double> Point;

        // To ensure reproducibility accros successive
        // runs, reset the random number generator.
        Numeric::random_reset();

        vector<double> s(nb_points);
        for(index_t i = 0; i < nb_points; i++) {
            s[i] = Numeric::random_float64();
        }
        std::sort(s.begin(), s.end());

        double Vtot = 0.0;
        for(index_t t = tets_begin; t < tets_end; ++t) {
            double Vt = mesh_tetra_mass<DIM>(mesh, t, vertex_weight);
            Vtot += Vt;
        }

        signed_index_t first_t = -1;
        signed_index_t last_t = 0;

        index_t cur_t = tets_begin;
        double cur_s =
            mesh_tetra_mass<DIM>(mesh, tets_begin, vertex_weight) / Vtot;
        for(index_t i = 0; i < nb_points; i++) {
            geo_debug_assert(i < s.size());
            while(s[i] > cur_s && cur_t < tets_end - 1) {
                cur_t++;
                geo_debug_assert(cur_t < tets_end);
                cur_s += mesh_tetra_mass<DIM>(
                    mesh, cur_t, vertex_weight
                ) / Vtot;
            }
            if(first_t == -1) {
                first_t = signed_index_t(cur_t);
            }
            last_t = std::max(last_t, signed_index_t(cur_t));

            index_t v0 = mesh.cells.vertex(cur_t, 0);
            index_t v1 = mesh.cells.vertex(cur_t, 1);
            index_t v2 = mesh.cells.vertex(cur_t, 2);
            index_t v3 = mesh.cells.vertex(cur_t, 3);

            // TODO: take weights into account
            //  with a new random_point_in_tetra_weighted()
            //  function.
            Point cur_p = Geom::random_point_in_tetra(
                *reinterpret_cast<const Point*>(mesh.vertices.point_ptr(v0)),
                *reinterpret_cast<const Point*>(mesh.vertices.point_ptr(v1)),
                *reinterpret_cast<const Point*>(mesh.vertices.point_ptr(v2)),
                *reinterpret_cast<const Point*>(mesh.vertices.point_ptr(v3))
            );
            for(coord_index_t coord = 0; coord < DIM; coord++) {
                p[i * DIM + coord] = cur_p[coord];
            }
        }
        if(mesh.cells.nb() > 1 && last_t == first_t) {
            Logger::warn("Sampler")
                << "Did put all the points in the same triangle"
                << std::endl;
            return false;
        }
        return true;
    }
}

#endif

