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

#ifndef GEOGRAM_MESH_MESH_PREPROCESSING
#define GEOGRAM_MESH_MESH_PREPROCESSING

#include <geogram/basic/common.h>
#include <geogram/mesh/mesh_repair.h>
#include <geogram/mesh/mesh_fill_holes.h>
#include <geogram/mesh/mesh_degree3_vertices.h>

/**
 * \file geogram/mesh/mesh_preprocessing.h
 * \brief Functions to pre-process a mesh
 */

namespace GEO {

    class Mesh;

    /**
     * \brief Removes the facets that have an area smaller than
     *  a given threshold.
     * \note This creates holes (borders) in the mesh.
     * \param[in,out] M the mesh to be processed
     * \param[in] min_facet_area facets with an area smaller than 
     *  this threshold are removed
     */
    void GEOGRAM_API remove_small_facets(Mesh& M, double min_facet_area);

    /**
     * \brief Removes the connected components that have an area
     *  smaller than a given threshold.
     * \param[in,out] M the mesh to be processed
     * \param[in] min_component_area the connected components with an
     *  area smaller than this threshold are removed
     * \param[in] min_component_facets the connected components with 
     *  less than min_component_facets facets are removed
     */
    void GEOGRAM_API remove_small_connected_components(
        Mesh& M, 
        double min_component_area, 
        index_t min_component_facets = 0.0
    );

    /**
     * \brief Orients the normals in such a way that each connected
     *  component has a positive signed volume.
     * \param[in,out] M the mesh to be processed
     */
    void GEOGRAM_API orient_normals(Mesh& M);

    /**
     * \brief Inverts all the normals of a mesh.
     * \param[in,out] M the mesh to be processed
     */
    void GEOGRAM_API invert_normals(Mesh& M);

    /**
     * \brief Enlarges a surface by moving the vertices
     *  on the border.
     * \details Shifts the vertices on the border by epsilon along
     * a direction tangent to the surface and normal to
     * the border. This enlarges the surface in such a
     * way that small holes are geometrically closed (but
     * not combinatorially). Subsequent remeshing with
     * CentroidalVoronoiTesselation results in a watertight
     * surface.
     * \param[in,out] M the mesh to be processed
     * \param[in] epsilon the distance along which border vertices
     *  are shifted
     */
    void GEOGRAM_API expand_border(Mesh& M, double epsilon);

    /**
     * \brief Removes the degree 2 vertices in a surface mesh.
     * \details Degree two vertices cause some combinatorial
     *   problems in some algorithms, since they make the same
     *   pair of facets adjacent twice. This function disconnects
     *   the concerned facets.
     */
    void GEOGRAM_API remove_degree2_vertices(Mesh& M);
}

#endif

