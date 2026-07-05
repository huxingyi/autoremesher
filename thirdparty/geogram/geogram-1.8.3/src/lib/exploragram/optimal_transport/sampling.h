/*
 *  Copyright (c) 2000-2022 Inria
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
 *  Contact: Bruno Levy
 *
 *     https://www.inria.fr/fr/bruno-levy
 *
 *     Inria,
 *     Domaine de Voluceau,
 *     78150 Le Chesnay - Rocquencourt
 *     FRANCE
 *
 */

#ifndef H_EXPLORAGRAM_OPTIMAL_TRANSPORT_SAMPLING_H
#define H_EXPLORAGRAM_OPTIMAL_TRANSPORT_SAMPLING_H

#include <exploragram/basic/common.h>

/**
 * \file exploragram/optimal_transport/sampling.h
 * \brief Functions to sample volumes, to be used with
 *  semi-discrete optimal transport.
 */

namespace GEO {
    class Mesh;
    class CentroidalVoronoiTesselation;

    /**
     * \brief Translates a mesh in such a way that its center matches
     *  the center of another mesh.
     * \param[in] M1 the reference volumetric mesh
     * \param[in,out] M2 the volumetric mesh that will be recentered
     */
    void EXPLORAGRAM_API recenter_mesh(const Mesh& M1, Mesh& M2);

    /**
     * \brief Computes the volume of a tetrahedral mesh.
     * \param[in] M a const reference to the mesh
     * \return the volume of the tetrahedra of M
     */
    double EXPLORAGRAM_API mesh_tets_volume(const Mesh& M);

    /**
     * \brief Rescales a mesh in such a way that its total volume
     *  matches the volume of a reference mesh.
     * \param[in] M1 the reference volumetric mesh
     * \param[in,out] M2 the volumetric mesh that will be rescaled
     */
    void EXPLORAGRAM_API rescale_mesh(const Mesh& M1, Mesh& M2);

    /**
     * \brief Creates a "weight" attribute with varying values.
     * \param[in,out] M a reference to the volumetric mesh that should be 
     *  decorated with densities
     * \param[in] mass1 minimum value of the density
     * \param[out] mass2 maximum value of the density
     * \param[in] func_str specification of the function to be used,
     *  in the form "(+|-)?func(^pow)?", where func is one of 
     *  X,Y,Z,R,sin,dist
     * \param[in] distance_reference if func is "dist" and if non-nullptr,
     *  distance is computed relative to \p distance_reference, else it
     *  is computed relative to \p M.
     * \TODO the same thing is refered here as "mass", "density" and "weight",
     *  this is a total mess.
     */
    void EXPLORAGRAM_API set_density(
        Mesh& M, double mass1, double mass2,
        const std::string& func_str,
        Mesh* distance_reference = nullptr
    );


    /**
     * \brief Computes a point sampling of a surfacic or volumetric
     *  mesh.
     * \details If \p CVT is in volumetric mode and the mesh has cells, 
     *  then the sampling is in the volume, else the sampling is on the
     *  surface (facets) of the mesh.
     * \see CentroidalVoronoiTesselation::set_volumetric()
     * \param[in,out] CVT a CentroidalVoronoiTesselation plugged 
     *  on the volumetric mesh to be sampled
     * \param[in] nb_points number of points to be created
     * \param[in] project_on_border if true, points near the
     *  border are projected onto the boundary of \p M. Needs
     *  VORPALINE to be supported.
     * \param[in] BRIO if true, use Biased Random Insertion
     *  Order [Amenta et.al]
     * \param[in] multilevel if true, use multilevel sampling 
     *  (note: BRIO implies multilevel)
     * \param[in] ratio ratio between the sizes of two sucessive
     *  levels
     * \param[in] levels if specified, the indices that indicate the
     *  beginning of each level will be copied to this vector.
     */
    void EXPLORAGRAM_API sample(
        CentroidalVoronoiTesselation& CVT,
        index_t nb_points, bool project_on_border,
        bool BRIO=true, bool multilevel=true,
        double ratio=0.125,
        vector<index_t>* levels=nullptr        
    );
}

#endif
