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

#ifndef GEOGRAM_MESH_MESH_SEGMENTATION
#define GEOGRAM_MESH_MESH_SEGMENTATION

#include <geogram/basic/common.h>
#include <geogram/basic/attributes.h>

/**
 * \file geogram/mesh/mesh_segmentation.h
 * \details Functions to split a surface mesh into multiple parts.
 */

namespace GEO {
    class Mesh;

    enum MeshSegmenter {
        
        /** continuous version of variational shape approximation*/
        SEGMENT_GEOMETRIC_VSA_L2,
        
        /** anisotropic continous version of variational shape approximation */
        SEGMENT_GEOMETRIC_VSA_L12,

        /** spectral segmentation with 8 manifold harmonics */        
        SEGMENT_SPECTRAL_8,

        /** spectral segmentation with 20 manifold harmonics */                
        SEGMENT_SPECTRAL_20,

        /** 
         * spectral segmentation with 100 manifold harmonics 
         * (uses some memory, use with caution !)
         */                
        SEGMENT_SPECTRAL_100,

        /** create two segments along shortest inertia axis */
        SEGMENT_INERTIA_AXIS
    };

    /**
     * \brief Computes a segmentation of a mesh.
     * \details The segmentation is stored in the "chart" facet attribute.
     * \param[in,out] mesh the mesh to be segmented. For now, only triangulated
     *  meshes are supported.
     * \param[in] segmenter one of 
     *   SEGMENT_GEOMETRIC_VSA_L2, SEGMENT_GEOMETRIC_VSA_L12, 
     *   SEGMENT_SPECTRAL_8, SEGMENT_SPECTRAL_20, SEGMENT_SPECTRAL_100
     * \param[in] nb_segments desired number of segment
     * \return number of charts
     */
    index_t GEOGRAM_API mesh_segment(
        Mesh& mesh, MeshSegmenter segmenter,
        index_t nb_segments, bool verbose=false
    );
    
}

#endif


