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

#ifndef H_EXPLORAGRAM_OPTIMAL_TRANSPORT_OPTIMAL_TRANSPORT_2D_H
#define H_EXPLORAGRAM_OPTIMAL_TRANSPORT_OPTIMAL_TRANSPORT_2D_H

#include <exploragram/basic/common.h>
#include <exploragram/optimal_transport/optimal_transport.h>
#include <geogram/voronoi/generic_RVD_polygon.h>

/**
 * \file exploragram/optimal_transport/optimal_transport_2d.h
 * \brief Solver for semi-discrete optimal transport in 2d.
 */


namespace GEO {

    /**
     * \brief Computes the centroids of the Laguerre cells that
     *  correspond to optimal transport in 2D.
     * \param[in] omega a reference to the mesh that represents the
     *  domain
     * \param[in] nb_points number of points
     * \param[in] points a pointer to the coordinates of the points
     * \param[out] centroids a pointer to the computed centroids of 
     *  the Laguerre cells that correspond to the optimal transport of
     *  the uniform measure to the points
     * \param[out] RVD if non-nullptr, a mesh with the 
     *  restricted Voronoi diagram.
     * \param[in] nb_air_particles number of air particles.
     * \param[in] air_particles a pointer to the array of doubles with the
     *  coordinates of the air particles.
     * \param[in] stride number of doubles between two consecutive air
     *  particles in the array, or 0 if tightly packed.
     * \param[in] air_fraction the fraction of the total mass occupied by air.
     * \param[in] weights_in an optional array of nb_points 
     *  doubles corresponding to the initial value of the weight vector.
     * \param[out] weights_out the computed value of the weights.
     * \param[in] nb_iter maximum number of Newton iterations.
     */
    void EXPLORAGRAM_API compute_Laguerre_centroids_2d(
        Mesh* omega,
        index_t nb_points,
        const double* points,
        double* centroids,
	Mesh* RVD=nullptr,
	bool verbose=false,
	index_t nb_air_particles = 0,
	const double* air_particles = nullptr,
	index_t air_particles_stride = 0,	
	double air_fraction = 0.0,
	const double* weights_in = nullptr,
	double* weights_out = nullptr,	
	index_t nb_iter = 1000
    );


    /**
     * \brief Computes semi-discrete optimal transport maps.
     * \details Computes an optimal transport map between two
     *  distributions in 2D. The first distribution is represented
     *  by a 2D triangulated mesh. The second distribution is a sum
     *  of Diracs with 2D coordinates.
     *  The algorithm is described in the following references:
     *   - 3D algorithm: http://arxiv.org/abs/1409.1279
     *   - Earlier 2D version by Quentin M\'erigot: 
     *    Q. Merigot. A multiscale approach to optimal transport.
     *    Computer Graphics Forum 30 (5) 1583--1592, 2011 (Proc SGP 2011).
     *   - Earlier article on OT and power diagrams: 
     *    F. Aurenhammer, F. Hoffmann, and B. Aronov. Minkowski-type theorems 
     *    and least-squares clustering. Algorithmica, 20:61-76, 1998.
     */
    class EXPLORAGRAM_API OptimalTransportMap2d : public OptimalTransportMap {
    public:
        /**
         * \brief OptimalTransportMap2d constructor.
         * \param[in] mesh the source distribution, represented as a 2d mesh.
	 *  It can be also a 3D mesh with the Z coordinate set to 0.
         * \param[in] delaunay factory name of the Delaunay triangulation.
         * \param[in] BRIO true if vertices are already ordered using BRIO
         */
        OptimalTransportMap2d(
            Mesh* mesh,
            const std::string& delaunay = "BPOW2d",
	    bool BRIO=false
        );

	/**
	 * \brief OptimalTransportMap destructor.
	 */
	~OptimalTransportMap2d() override;

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

      public:
	/**
	 * \brief Used by clipping operations.
	 */
	GEOGen::Polygon work_;
	/**
	 * \brief Used by clipping operations.
	 */
	GEOGen::Polygon clipped_;
    };

    /*********************************************************************/

}

#endif
