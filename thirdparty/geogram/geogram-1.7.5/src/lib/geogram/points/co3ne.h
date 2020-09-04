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

#ifndef GEOGRAM_POINTS_CO3NE
#define GEOGRAM_POINTS_CO3NE

#include <geogram/basic/common.h>
#include <geogram/basic/numeric.h>

/**
 * \file geogram/points/co3ne.h
 * \brief Implementation of the Simple and Scalable Surface 
 *    Reconstruction algorithm (Concurrent Co-Cones).
 */

namespace GEO {

    class Mesh;

    /**
     * \brief Smoothes a point set by projection onto the
     *  nearest neighbors best approximating plane.
     * \param[in,out] M the pointset (facets and tets are ignored if present)
     * \param[in] nb_neighbors number of neighbors used to compute the
     *  best approximating tangent planes
     * \param[in] nb_iterations number of iterations
     */
    void GEOGRAM_API Co3Ne_smooth(
        Mesh& M, index_t nb_neighbors, index_t nb_iterations
    );

    /**
     * \brief Computes the normals to a point set. The normal
     *  is obtained from the nearest neighbors best approximating
     *  plane. Normals are stored in the "normal" vertex attribute.
     * \param[in] M the pointset (facets and tets are ignored if present)
     * \param[in] nb_neighbors number of neighbors used to compute the
     *  best approximating tangent planes
     * \param[in] reorient if true, try to orient the normals by 
     *  propagation over the KNN graph.
     * \retval true if normals where successfully computed.
     * \retval false otherwise (when the user pushes the cancel button).
     */
    bool GEOGRAM_API Co3Ne_compute_normals(
        Mesh& M, index_t nb_neighbors, bool reorient = false
    );

    /**
     * \brief Given a pointset with normals stored in the "normal" vertex
     *  attribute, reconstruct the triangles.
     * \param[in,out] M input pointset and output mesh
     * \param[in] radius maximum distance used to connect neighbors with
     *  triangles
     */
    void GEOGRAM_API Co3Ne_reconstruct(Mesh& M, double radius);

    /**
     * \brief Computes the normals and reconstructs the triangles
     *  of a pointset.
     * \details The normal is obtained from the nearest
     *  neighbors best approximating plane.
     *  The normals are used "on the fly" and not stored, therefore
     *  calling this function is more efficient than calling
     *  Co3Ne_compute_normals() then Co3Ne_reconstruct().
     * \param[in,out] M the input pointset and the output mesh
     * \param[in] nb_neighbors number of neighbors used to compute the
     *  best approximating tangent planes
     * \param[in] nb_iterations number of smoothing iterations
     * \param[in] radius maximum distance used to connect neighbors with
     *  triangles
     */
    void GEOGRAM_API Co3Ne_smooth_and_reconstruct(
        Mesh& M, index_t nb_neighbors, index_t nb_iterations, double radius
    );
}

#endif

