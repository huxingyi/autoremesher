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

#ifndef GEOGRAM_MESH_MESH_ATLAS_MAKER
#define GEOGRAM_MESH_MESH_ATLAS_MAKER

#include <geogram/basic/common.h>
#include <geogram/basic/numeric.h>

/**
 * \file geogram/mesh/mesh_atlas_maker.h
 */

namespace GEO {

    class Mesh;

    /**
     * \brief Parameterizer used to flatten
     *  individual charts.
     */ 
    enum ChartParameterizer {
	PARAM_PROJECTION, PARAM_LSCM,
	PARAM_SPECTRAL_LSCM, PARAM_ABF
    };

    /**
     * \brief Packer used to organize the charts
     *  in texture space.
     */ 
    enum ChartPacker {
	PACK_NONE, PACK_TETRIS, PACK_XATLAS
    };

#ifndef GOMGEN   
    /**
     * \brief Generates u,v coordinates for a given mesh.
     * \details The generated u,v coordinates are stored 
     *  in an attribute attached to the facet corners.
     *  The mesh is first decomposed into a set of charts,
     *  then each chart is flatenned using \p param. After
     *  parameterization, if distortion is too high, the 
     *  chart is further decomposed into smaller charts.
     *  Finally, all the charts are packed in texture space
     *  using \p pack. If the mesh has a facet attribute 
     *  named "chart", then it is used to initialize the
     *  segmentation.
     * \param [in,out] mesh the mesh to be parameterized. For now, 
     *  only triangulated meshes are supported.
     * \param[in] hard_angle_threshold edges for which the
     *  dihedral angle is larger than this threshold are
     *  considered as chart boundaries (in degrees)
     * \param[in] param one of:
     *  - PARAM_PROJECTION: projection on least-squares fitted
     *    plane
     *  - PARAM_LSCM: Least Squares Conformal Maps with 
     *    two fixed points
     *  - PARAM_SPECTRAL_LSCM: spectral Least Squares 
     *    Conformal Maps (less distorted than PARAM_LSCM)
     *  - PARAM_ABF: Angle-Based Flattening++ (best quality)
     * \param[in] pack one of:
     *  - PACK_NONE: no texture packing
     *  - PACK_TETRIS: using built-in "Tetris" packing, as 
     *    in the original LSCM article
     *  - PACK_XATLAS: use the XAtlas library to do the packing
     */
#endif   
    void GEOGRAM_API mesh_make_atlas(
	Mesh& mesh,
	double hard_angles_threshold = 45.0,
	ChartParameterizer param = PARAM_ABF,
	ChartPacker pack = PACK_XATLAS,
	bool verbose = false
    );

#ifndef GOMGEN
    /**
     * \brief Gets the charts attribute from a parameterized mesh.
     * \details The charts are stored in a facet attribute named "chart"
     *  of type index_t
     * \param[in,out] mesh a parameterized mesh
     * \return the number of charts
     */
#endif    
    index_t GEOGRAM_API mesh_get_charts(Mesh& mesh);
}

#endif

