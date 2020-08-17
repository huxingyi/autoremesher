
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
 * As an exception to the GPL, Graphite can be linked with 
 *     the following (non-GPL) libraries:
 *     Qt, SuperLU, WildMagic and CGAL
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
     * \param[out] RVD if non-nullptr, a mesh with the restricted Voronoi diagram.
     * \param[in] nb_air_particles number of air particles.
     * \param[in] air_particles a pointer to the array of doubles with the
     *  coordinates of the air particles.
     * \param[in] stride number of doubles between two consecutive air
     *  particles in the array, or 0 if tightly packed.
     * \param[in] air_fraction the fraction of the total mass occupied by air.
     * \param[in] weights_in an optional array of nb_points doubles corresponding to
     *  the initial value of the weight vector.
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
	virtual ~OptimalTransportMap2d();

	/**
	 * \copydoc OptimalTransportMap::get_RVD()
	 */
        virtual void get_RVD(Mesh& M);

	/**
	 * \copydoc OptimalTransportMap::compute_Laguerre_centroids()
	 */
        virtual void compute_Laguerre_centroids(double* centroids);

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
 	virtual void call_callback_on_RVD();

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
