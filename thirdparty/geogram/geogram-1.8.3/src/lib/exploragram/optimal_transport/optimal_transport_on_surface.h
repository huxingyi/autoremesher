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

#ifndef H_EXPLORAGRAM_OPTIMAL_TRANSPORT_OPTIMAL_TRANSPORT_ON_SURFACE_H
#define H_EXPLORAGRAM_OPTIMAL_TRANSPORT_OPTIMAL_TRANSPORT_ON_SURFACE_H

#include <exploragram/basic/common.h>
#include <exploragram/optimal_transport/optimal_transport.h>

/**
 * \file exploragram/optimal_transport/optimal_transport_on_surface.h
 * \brief Solver for semi-discrete optimal transport between pointset and
 *  3D surface.
 */


namespace GEO {

    /**
     * \brief Computes the centroids of the Laguerre cells that
     *  correspond to optimal transport over a surface embedded in 3D.
     * \param[in] omega a reference to the mesh that represents the
     *  domain
     * \param[in] nb_points number of points
     * \param[in] points a pointer to the coordinates of the points
     * \param[out] centroids a pointer to the computed centroids of 
     *  the Laguerre cells that correspond to the optimal transport of
     *  the uniform measure to the points
     */
    void EXPLORAGRAM_API compute_Laguerre_centroids_on_surface(
        Mesh* omega,
        index_t nb_points,
        const double* points,
        double* centroids,
	Mesh* RVD=nullptr,
	bool verbose=false
    );

    /*********************************************************************/

    /**
     * \brief Computes semi-discrete optimal transport maps.
     * \details Computes an optimal transport map between two
     *  distributions in 3D. The first distribution is represented
     *  by a 3D surfacic triangulated mesh. The second distribution is a sum
     *  of Diracs with 3D coordinates.
     *  The algorithm is described in the following references:
     *   - 3D algorithm: http://arxiv.org/abs/1409.1279
     *   - Earlier 2D version by Quentin M\'erigot: 
     *    Q. Merigot. A multiscale approach to optimal transport.
     *    Computer Graphics Forum 30 (5) 1583--1592, 2011 (Proc SGP 2011).
     *   - Earlier article on OT and power diagrams: 
     *    F. Aurenhammer, F. Hoffmann, and B. Aronov. Minkowski-type theorems 
     *    and least-squares clustering. Algorithmica, 20:61-76, 1998.
     */
    class EXPLORAGRAM_API OptimalTransportMapOnSurface :
	public OptimalTransportMap {
    public:
        /**
         * \brief OptimalTransportOnSurface constructor.
         * \param[in] mesh the source distribution, represented as a 3d mesh
         * \param[in] delaunay factory name of the Delaunay triangulation.
         * \param[in] BRIO true if vertices are already ordered using BRIO
         */
        OptimalTransportMapOnSurface(
            Mesh* mesh,
            const std::string& delaunay = "BPOW",
	    bool BRIO=false
        );

	/**
	 * \brief OptimalTransportMap destructor.
	 */
	~OptimalTransportMapOnSurface() override;

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

    /*********************************************************************/    
}

#endif
