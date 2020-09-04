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

#ifndef GEOGRAM_MESH_MESH_INTERSECTION
#define GEOGRAM_MESH_MESH_INTERSECTION

/**
 * \file mesh_intersection.h
 */

#include <geogram/basic/common.h>
#include <geogram/basic/numeric.h>

namespace GEO {

    class Mesh;

    /**
     * \brief Computes the union of two surface meshes.
     * \details A and B need to be two closed surface
     *  mesh without intersections.
     * \note This is work in progress, the function is
     *  not robust yet. 
     * \param[in] A , B the two operands.
     * \param[out] result the computed mesh.
     */
    void GEOGRAM_API mesh_union(Mesh& result, Mesh& A, Mesh& B);

    /**
     * \brief Computes the intersection of two surface meshes.
     * \details A and B need to be two closed surface
     *  mesh without intersections.
     * \note This is work in progress, the function is
     *  not robust yet. 
     * \param[in] A , B the two operands.
     * \param[out] result the computed mesh.
     */
    void GEOGRAM_API mesh_intersection(Mesh& result, Mesh& A, Mesh& B);

    /**
     * \brief Computes the difference of two surface meshes.
     * \details A and B need to be two closed surface
     *  mesh without intersections.
     * \note This is work in progress, the function is
     *  not robust yet. 
     * \param[in] A , B the two operands.
     * \param[out] result the computed mesh.
     */
    void GEOGRAM_API mesh_difference(Mesh& result, Mesh& A, Mesh& B);
    
    /**
     * \brief Attempts to make a surface mesh conformal by
     *  removing intersecting facets and re-triangulating the holes.
     */
    void GEOGRAM_API mesh_remove_intersections(Mesh& M, index_t max_iter = 3);
}

#endif

