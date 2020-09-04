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

#ifndef GEOGRAM_DELAUNAY_DELAUNAY_NN
#define GEOGRAM_DELAUNAY_DELAUNAY_NN

#include <geogram/basic/common.h>
#include <geogram/delaunay/delaunay.h>
#include <geogram/points/nn_search.h>
#include <geogram/basic/process.h>

/**
 * \file geogram/delaunay/delaunay_nn.h
 * \brief Implementation of Delaunay using nearest neighbors
 */

namespace GEO {

    /**
     * \brief Delaunay interface for NearestNeighbors search.
     * \details
     * This does not fully implement a Delaunay triangulation,
     * cell-related queries are not implemented. It only
     * implements neighborhood queries, which are the only ones
     * needed by RestrictedVoronoiDiagram with radius of security.
     */
    class GEOGRAM_API Delaunay_NearestNeighbors : public Delaunay {
    public:
        /**
         * \brief Creates a new Delaunay_NearestNeighbors.
         * \param[in] dimension the dimension of the points
         */
        Delaunay_NearestNeighbors(coord_index_t dimension);

        /**
         * \brief Stores nb neighbors with vertex i.
         * \details By default, Delaunay::default_nb_neighbors()
         *  are stored for each vertex. This function changes
         *  the number of stored neighbors for a given vertex.
         * \param[in] i index of the vertex which neighborhood should
         *  be enlarged
         * \param[in] nb new number of vertices in vertex \p i%'s neighborhood.
         */
        virtual void enlarge_neighborhood(index_t i, index_t nb);

        virtual void set_vertices(
            index_t nb_vertices, const double* vertices
        );

        virtual index_t nearest_vertex(const double* p) const;

        /**
         * \brief Gets the NearestNeighborSearch used internally.
         * \return a pointer to the NearestNeighborSearch.
         */
        NearestNeighborSearch* nn_search() {
            return NN_;
        }

    public:
        /**
         * \brief Used internally for parallel
         *  computation of the neighborhoods
         *  in Delaunay.
         */
        virtual void store_neighbors_CB(index_t i);

    protected:
        /**
         * \brief Delaunay_NearestNeighbors destructor
         */
        virtual ~Delaunay_NearestNeighbors();

        /**
         * \brief Internal implementation for get_neighbors (with vector).
         * \param[in] v index of the Delaunay vertex
         * \param[in,out] neighbors the computed neighbors of vertex \p v.
         *    Its size is used to determine the number of queried neighbors.
         */
        virtual void get_neighbors_internal(
            index_t v, vector<index_t>& neighbors
        ) const;

        /**
         * \brief Internal implementation for get_neighbors (with pointers).
         * \param[in] v index of the Delaunay vertex
         * \param[in] nb_neighbors required number of neighbors
         * \param[out] neighbors the computed neighbors of vertex \p v,
         *  allocated and managed by caller
         * \return the obtained number of neighbors (can be
         * smaller than nb_neighbors if duplicate points
         * are encountered)
         */
        virtual index_t get_neighbors_internal(
            index_t v, index_t nb_neighbors, index_t* neighbors
        ) const;

    private:
        NearestNeighborSearch_var NN_;
    };
}

#endif

