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

#ifndef GEOGRAM_MESH_MESH_TETRAHEDRALIZE
#define GEOGRAM_MESH_MESH_TETRAHEDRALIZE

#include <geogram/basic/common.h>

/**
 * \file geogram/mesh/mesh_tetrahedralize.h
 * \brief Functions for filling a surface mesh with tetrahedra.
 */

namespace GEO {

    class Mesh;
    
    /**
     * \brief Fills a closed surface mesh with tetrahedra.
     * \details A constrained Delaunay triangulation algorithm
     *  needs to be interfaced (e.g., compiling with tetgen support).
     * \param [in,out] M a reference to a mesh
     * \param [in] preprocess if true, the surface mesh is preprocessed
     *  to fix some defects (small gaps and intersections). If preprocess
     *  is set and borders are detected after preprocessing, then the function
     *  returns false. If preprocess is set to false, then the caller is 
     *  supposed to provide a correct set of input constraints (that may have
     *  dangling borders / internal constraints).
     * \param [in] refine if true, inserts additional vertices to improve
     *  the quality of the mesh elements
     * \param[in] quality typically in [1.0, 2.0], specifies
     *  the desired quality of mesh elements (1.0 means maximum
     *  quality, and generates a higher number of elements).
     * \param[in] keep_regions if set, then all internal regions are kept, and
     *  a region cell attribute is created, else only tetrahedra in the 
     *  outermost region are kept.
     * \retval true if the mesh was successfuly tetrahedralized
     * \retval false otherwise
     * \note needs a constrained Delaunay algorithm to work (geogram needs
     *  to be compiled with mg-tetra or tetgen).
     */
    bool GEOGRAM_API mesh_tetrahedralize(
        Mesh& M, bool preprocess=true, bool refine=false, double quality=2.0,
	bool keep_regions=false
    );
    
}

#endif
