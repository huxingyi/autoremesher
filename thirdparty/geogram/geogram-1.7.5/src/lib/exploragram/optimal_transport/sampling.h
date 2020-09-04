
/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000-2009 INRIA - Project ALICE
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  If you modify this software, you should include a notice giving the
 *  name of the person performing the modification, the date of modification,
 *  and the reason for such modification.
 *
 *  Contact: Bruno Levy - levy@loria.fr
 *
 *     Project ALICE
 *     LORIA, INRIA Lorraine, 
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX 
 *     FRANCE
 *
 *  Note that the GNU General Public License does not permit incorporating
 *  the Software into proprietary programs. 
 *
 * As an exception to the GPL, Graphite can be linked with the following (non-GPL) libraries:
 *     Qt, SuperLU, WildMagic and CGAL
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
