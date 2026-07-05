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

#ifndef H_EXPLORAGRAM_OPTIMAL_TRANSPORT_OPTIMAL_TRANSPORT_3D_H
#define H_EXPLORAGRAM_OPTIMAL_TRANSPORT_OPTIMAL_TRANSPORT_3D_H

#include <exploragram/basic/common.h>
#include <exploragram/optimal_transport/optimal_transport.h>

/**
 * \file exploragram/optimal_transport/optimal_transport_3d.h
 * \brief Solver for semi-discrete optimal transport in 3d (
 *  multilevel and Newton).
 */

namespace GEO {
    
    class RVDPolyhedronCallback;

    /**
     * \brief Computes the centroids of the Laguerre cells that
     *  correspond to optimal transport.
     * \param[in] omega a reference to the mesh that represents the
     *  domain
     * \param[in] nb_points number of points
     * \param[in] points a pointer to the coordinates of the points
     * \param[out] centroids a pointer to the computed centroids of 
     *  the Laguerre cells that correspond to the optimal transport of
     *  the uniform measure to the points
     * \param[in] cb an optional RVD polyhedron callback to be called on the
     *  restricted power diagram once it is computed.
     */
    void EXPLORAGRAM_API compute_Laguerre_centroids_3d(
        Mesh* omega,
        index_t nb_points,
        const double* points,
        double* centroids,
	RVDPolyhedronCallback* cb=nullptr,
	bool verbose=false,
	index_t nb_iter=2000
    );
    
    /**
     * \brief Computes semi-discrete optimal transport maps.
     * \details Computes an optimal transport map between two
     *  distributions in 3D. The first distribution is represented
     *  by a 3D tetrahedral mesh. The second distribution is a sum
     *  of Diracs.
     *  The algorithm is described in the following references:
     *   - 3D algorithm: http://arxiv.org/abs/1409.1279
     *   - Earlier 2D version by Quentin M\'erigot: 
     *    Q. Merigot. A multiscale approach to optimal transport.
     *    Computer Graphics Forum 30 (5) 1583--1592, 2011 (Proc SGP 2011).
     *   - Earlier article on OT and power diagrams: 
     *    F. Aurenhammer, F. Hoffmann, and B. Aronov. Minkowski-type theorems 
     *    and least-squares clustering. Algorithmica, 20:61-76, 1998.
     */
    class EXPLORAGRAM_API OptimalTransportMap3d : public OptimalTransportMap {
    public:
        /**
         * \brief OptimalTransportMap3d constructor.
         * \param[in] mesh the source distribution, represented as a 3d mesh
         * \param[in] delaunay factory name of the Delaunay triangulation, one
	 *  of "PDEL" (parallel), "BPOW" (sequential)
         * \param[in] BRIO true if vertices are already ordered using BRIO
         */
        OptimalTransportMap3d(
            Mesh* mesh,
            const std::string& delaunay = "PDEL",
            bool BRIO = false
        );

	/**
	 * \brief OptimalTransportMap destructor.
	 */
	~OptimalTransportMap3d() override;

	/**
	 * \copydoc OptimalTransportMap::get_RVD()
	 */
	void get_RVD(Mesh& M) override;

	/**
	 * \copydoc OptimalTransportMap::compute_Laguerre_centroids()
	 */
	void compute_Laguerre_centroids(double* centroids) override;

	/**
	 * \brief Gets the total mass of the mesh.
	 * \details Take the weights into account if they are present.
	 * \return the total mass of the mesh.
	 */
	double total_mesh_mass() const;
	
      protected:
	/**
	 * \copydoc OptimalTransportMap::call_callback_on_RVD()
	 */
	void call_callback_on_RVD() override;
    };

    /**********************************************************************/
    
    /**
     * \brief Computes a shape that interpolates the two input tet
     *  meshes.
     * \details The shape is composed of tetrahedra
     * \param [in] CVT the Centroidal Voronoi Tesselation
     *   used to sample the second shape
     * \param [in] OTM the Optimal Transport Map
     * \param [out] morph mesh where to store the morphing shape. It uses
     *   6d coordinates (original location + final location).
     * \param[in] filter_tets if true, remove the tetrahedra that are outside
     *   the source mesh.
     */
    void EXPLORAGRAM_API compute_morph(
        CentroidalVoronoiTesselation& CVT,
        OptimalTransportMap3d& OTM,
        Mesh& morph,
        bool filter_tets=true
    );


    /**
     * \brief Computes the surface that corresponds to discontinuities
     *  in the optimal transport map.
     * \details The surface is determined as the facets of Voronoi cells
     *  that are adjacent in Pow(X)|M1 but not in Vor(X)|M2 
     * \param [in] CVT the Centroidal Voronoi Tesselation
     *   used to sample the second shape M2
     * \param [in] OTM the Optimal Transport Map with the
     *   power diagram that samples the first shape M1
     * \param [out] singular_set where to store the singular surface
     */
    void EXPLORAGRAM_API compute_singular_surface(        
        CentroidalVoronoiTesselation& CVT,
        OptimalTransportMap3d& OTM,
        Mesh& singular_set
    );

}

#endif
