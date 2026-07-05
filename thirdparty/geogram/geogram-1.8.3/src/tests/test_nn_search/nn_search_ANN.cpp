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

#include "nn_search_ANN.h"

namespace GEO {

    NearestNeighborSearch_ANN::NearestNeighborSearch_ANN(
        coord_index_t dim
    ) :
        NearestNeighborSearch(dim),
        ann_tree_(nullptr) {
    }

    void NearestNeighborSearch_ANN::set_points(index_t nb_points, const double* points) {
        set_points(nb_points, points, dimension());
    }

    bool NearestNeighborSearch_ANN::stride_supported() const {
        return true;
    }

    void NearestNeighborSearch_ANN::set_points(
        index_t nb_points, const double* points, index_t stride
    ) {
        nb_points_ = nb_points;
        points_ = points;
        stride_ = stride;
        
        // Patched ANN so that we no longer need
        // to generate an array of pointers to
        // the points, See ANN.h
#ifdef ANN_CONTIGUOUS_POINT_ARRAY
        delete ann_tree_;
        ann_tree_ = new ANNkd_tree(
            ANNpointArray(points_, stride_),
            int(nb_points), 
            int(dimension())
        );
#else
        delete ann_tree_;
        ann_tree_ = nullptr;
        ann_points_.resize(nb_points);
        for(index_t i = 0; i < nb_points; i++) {
            ann_points_[i] = const_cast<double*>(points) + stride_ * i;
        }
        ann_tree_ = new ANNkd_tree(
            &ann_points_[0], int(nb_points), int(dimension())
        );
#endif
    }

    void NearestNeighborSearch_ANN::get_nearest_neighbors(
        index_t nb_neighbors,
        const double* query_point,
        index_t* neighbors,
        double* neighbors_sq_dist
    ) const {
        ann_tree_->annkSearch(
            const_cast<double*>(query_point),
            int(nb_neighbors), (ANNidxArray) neighbors, neighbors_sq_dist,
            (exact_ ? 0.0 : 0.1)
        );
    }

    NearestNeighborSearch_ANN::~NearestNeighborSearch_ANN() {
        delete ann_tree_;
        ann_tree_ = nullptr;
    }

}


