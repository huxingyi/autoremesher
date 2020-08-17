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

#ifndef GEOGRAM_MESH_MESH_COMPARE
#define GEOGRAM_MESH_MESH_COMPARE

#include <geogram/basic/common.h>

/**
 * \file geogram/mesh/mesh_compare.h
 * \brief Functions for comparing two meshes for exact equality
 */

namespace GEO {

    class Mesh;

    /**
     * \brief Flags used to compare two meshes
     * \details These flags can be bit-or'ed together to tell
     * mesh_compare() which properties of the meshes to compare.
     * MeshCompareFlags provides 2 reasonable default flags:
     * - #MESH_COMPARE_SURFACE_PROPS to compare surfaces
     * - #MESH_COMPARE_VOLUME_PROPS to compare volumes
     * \see mesh_compare()
     */
    enum MeshCompareFlags {
        /** The mesh comparison is OK */
        MESH_COMPARE_OK = 0,
        /** Compares mesh dimensions */
        MESH_COMPARE_DIMENSIONS = 1,
        /** Compares mesh number of vertices */
        MESH_COMPARE_NB_VERTICES = 2,
        /** Compares mesh number of facets */
        MESH_COMPARE_NB_FACETS = 4,
        /** Compares mesh number of facets borders */
        MESH_COMPARE_NB_FACET_BORDERS = 8,
        /** Compares mesh areas (see Geom::mesh_area()) */
        MESH_COMPARE_AREAS = 16,
        /** Compares mesh topology (see meshes_have_same_topology()) */
        MESH_COMPARE_TOPOLOGY = 32,
        /** Compares mesh number of tets */
        MESH_COMPARE_NB_TETS = 64,
        /** Compares mesh number of tets borders */
        MESH_COMPARE_NB_TET_BORDERS = 128,

        /** Reasonable flags used to compare surfaces */
        MESH_COMPARE_SURFACE_PROPS =
            MESH_COMPARE_DIMENSIONS |
            MESH_COMPARE_NB_VERTICES |
            MESH_COMPARE_NB_FACETS |
            MESH_COMPARE_NB_FACET_BORDERS |
            MESH_COMPARE_AREAS |
            MESH_COMPARE_TOPOLOGY,

        /** Reasonable flags used to compare volumes */
        MESH_COMPARE_VOLUME_PROPS =
            MESH_COMPARE_SURFACE_PROPS |
            MESH_COMPARE_NB_TETS |
            MESH_COMPARE_NB_TET_BORDERS,

        /** Compare all properties */
        MESH_COMPARE_ALL = MESH_COMPARE_VOLUME_PROPS
    };

    /**
     * \brief Compares two meshes.
     *  Compares the two meshes \p M1 and \p M2 according to the comparison
     *  flags \p flags (see #MeshCompareFlags). The function returns a
     *  comparison status similar to the comparison \p flags:
     *  - if a comparison failed for a test f in in \p flags, the same
     *  flag f is set in the status
     *  - otherwise the flag f is cleared in the status.
     *  A status of zero indicates that all comparisons succeeded.
     * \param[in] M1 the first input mesh
     * \param[in] M2 the second input mesh
     * \param[in] flags specifies which properties of the meshes should be
     *  compared. By default it is set to #MESH_COMPARE_SURFACE_PROPS
     *  information for the two meshes
     * \param[in] tolerance relative tolerance used to compare floating point
     * values (such as the mash areas)
     * \param[in] verbose enables/disables the display of mesh information
     *  for the two meshes, as well mesh comparison error messages.
     * \retval #MESH_COMPARE_OK if meshes \p M1 and \p M2 are identical
     *  according to the comparison criteria
     * \retval the comparison status otherwise.
     * \see MeshCompareFlags
     * \see Geom::mesh_area()
     * \see meshes_have_same_topology()
     */
    MeshCompareFlags GEOGRAM_API mesh_compare(
        const Mesh& M1, const Mesh& M2,
        MeshCompareFlags flags = MESH_COMPARE_SURFACE_PROPS,
        double tolerance = 0.0,
        bool verbose = true
    );
}

#endif

