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

#ifndef GEOGRAM_MESH_MESH_DISTANCE
#define GEOGRAM_MESH_MESH_DISTANCE

#include <geogram/basic/common.h>

/**
 * \file mesh_distance.h
 * \brief Distance computations between meshes
 */

namespace GEO {

    class Mesh;

    /**
     * \brief Computes an approximation of the
     *  single sided Hausdorff distance dH(m1->m2) between
     *  two surfacic meshes.
     *
     *  The single sided Hausdorff distance dH(m1,m2)
     *  is defined as:
     *  \f$dH(m1->m2) = Max(min(d(p,q) | q \in m2) | p \in m1)\f$
     *
     *  The mesh \p m1 is sampled as discrete locations and
     *  the max of the distances between these samples
     *  and \p m2 is returned. 
     *
     * \remark Only the facets of meshes \p m1 and
     *  \p m2 are used (line segments and volumetric cells are ignored).
     *  Note that the order of the mesh facets are changed.
     *
     * \param[in] m1 , m2 two surfacic meshes whose distance is computed
     * \param[in] sampling_dist average distance between
     *  two samples (the smaller, the more accurate).
     *  At least all the vertices of m1 are sampled,
     *  and additional random samples on the surface
     *  are added until sampling density is reached,
     *  i.e. nb_samples >= area(m1) / sampling_dist^2
     */
    double GEOGRAM_API mesh_one_sided_Hausdorff_distance(
        Mesh& m1, Mesh& m2, double sampling_dist
    );

    /**
     * \brief Computes an approximation of the
     *  symmetric Hausdorff distance dH(m1<->m2)
     *  between two surfacic meshes.
     *
     * The symmetric Hausdorff distance dH(m1<->m2)
     * is defined as:
     * dH(m1<->m2) = Max(dH(m1->m2),dH(m2->m1))
     *
     * \remark Only the facets of meshes \p m1 and
     *  \p m2 are used (line segments and volumetric cells are ignored).
     *  Note that the order of the mesh facets are changed.
     *
     * \param[in] m1 , m2 two surfacic meshes whose distance is computed
     * \param[in] sampling_dist average distance between
     *  two samples (the smaller, the more accurate).
     *  At least all the vertices of m1 (resp. m2)
     *  are sampled, and additional random samples
     *  on the surface are added until sampling
     *  density is reached, i.e.
     *  nb_samples_m1 >= area(m1) / sampling_dist^2
     *  (resp. m2).
     *
     * \see mesh_one_sided_Hausdorff_distance()
     */
    double GEOGRAM_API mesh_symmetric_Hausdorff_distance(
        Mesh& m1, Mesh& m2, double sampling_dist
    );
}

#endif

