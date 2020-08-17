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

#ifndef __NN_SEARCH_ANN__
#include <geogram/points/nn_search.h>
#include "third_party/ANN/ANN.h"

namespace GEO {

    /**
     * \brief Implementation of NearestNeighborSearch using the ANN library.
     * \details Used for testing the implementation of KdTree in Geogram.
     */
    class NearestNeighborSearch_ANN : public NearestNeighborSearch {
    public:
        /**
         * \brief Constructs a new NearestNeighborSearch_ANN.
         * \param[in] dim dimension of the points
         */
        NearestNeighborSearch_ANN(
            coord_index_t dim
        );
        
        virtual void set_points(index_t nb_points, const double* points);
        
        virtual bool stride_supported() const ;
        
        virtual void set_points(
            index_t nb_points, const double* points, index_t stride
        );
        
        virtual void get_nearest_neighbors(
            index_t nb_neighbors,
            const double* query_point,
            index_t* neighbors,
            double* neighbors_sq_dist
        ) const;

    protected:
        /**
         * \brief NearestNeighborSearch_ANN destructor
         */
        virtual ~NearestNeighborSearch_ANN();

    private:
#ifndef ANN_CONTIGUOUS_POINT_ARRAY
        std::vector<ANNcoord*> ann_points_;
#endif
        ANNkd_tree * ann_tree_;
    };
}

#endif

