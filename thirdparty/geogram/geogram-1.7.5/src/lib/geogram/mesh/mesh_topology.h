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

#ifndef GEOGRAM_MESH_MESH_TOPOLOGY
#define GEOGRAM_MESH_MESH_TOPOLOGY

#include <geogram/basic/common.h>
#include <geogram/basic/numeric.h>

/**
 * \file geogram/mesh/mesh_topology.h
 * \brief Functions to query and compare mesh topology
 */

namespace GEO {

    class Mesh;
    template <class T>
    class vector;

    /**
     * \brief Computes the connected components of a Mesh.
     * \param[in] M the input mesh
     * \param[out] component component[f] contains the index of the
     * connected component that facet f belongs to.
     * \return the number of connceted components
     * \post component.size() == M.nb_facets()
     */
    index_t GEOGRAM_API get_connected_components(
        const Mesh& M, vector<index_t>& component
    );

    /**
     * \brief Computes the number of connected components of a Mesh.
     */
    index_t GEOGRAM_API mesh_nb_connected_components(const Mesh& M);

    /**
     * \brief Computes the Euler-Poincare characteristic of a surfacic
     *  Mesh.
     * \param[in] M the input mesh
     * \return Xi = V - E + F, where V = number of vertices, E = number of
     *  edges and F = number of faces.
     */
    signed_index_t GEOGRAM_API mesh_Xi(const Mesh& M);

    /**
     * \brief Computes the number of borders of a Mesh.
     * \param[in] M the input mesh
     * \return the number of borders, or -1 if the border is
     *  non-manifold (i.e. has "butterfly" vertices).
     */
    signed_index_t GEOGRAM_API mesh_nb_borders(const Mesh& M);

    /**
     * \brief Compares the topological invariants of two meshes.
     * \details
     *  The topological invariants are: the number of connected
     *  components (get_connected_components()), the Euler-Poincare
     *  characteristic (computed by mesh_Xi()) and the number of
     *  borders (computed by mesh_number_of_borders()). These are
     *  displayed if \p verbose is set to true.
     * \param[in] M1 the first input mesh
     * \param[in] M2 the second input mesh
     * \param[in] verbose enables/disables the display of topological
     * information for the two meshes
     * \retval true if meshes \p M1 and \p M2 have the same topology,
     * \retval false otherwise.
     */
    bool GEOGRAM_API meshes_have_same_topology(
        const Mesh& M1, const Mesh& M2, bool verbose = false
    );
}

#endif

