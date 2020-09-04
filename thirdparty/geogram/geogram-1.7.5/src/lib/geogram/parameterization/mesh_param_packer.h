/*
 *  Copyright (c) 2012-2016, Bruno Levy
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

#ifndef GEOGRAM_MESH_MESH_PARAM_PACKER
#define GEOGRAM_MESH_MESH_PARAM_PACKER

#include <geogram/basic/common.h>
#include <geogram/parameterization/mesh_segmentation.h>
#include <geogram/basic/geometry.h>
#include <geogram/basic/attributes.h>

/**
 * \file geogram/mesh/mesh_param_packer.h
 */

namespace GEO {


    /**
     * \brief Packs an atlas using the xatlas library.
     * \details The mesh needs to have a parameterization
     *  stored in the tex_coord facet_corner attribute.
     * \param[in,out] mesh a reference to the mesh
     */
    void GEOGRAM_API pack_atlas_using_xatlas(Mesh& mesh);

    /****************************************************************/

    /**
     * \brief Packs a set of surfaces in parameter (texture) space.
     * \details This organizes the charts of a parameterization in
     *  a way that tries to minimize unused texture space.
     *  The algorithm, by Nicolas Ray, is described in the following
     *  reference: 
     *  - least squares mode: Least Squares Conformal Maps, 
     *    Levy, Petitjean, Ray, Maillot, ACM SIGGRAPH, 2002
     */
    class GEOGRAM_API Packer {
    public:

	/**
	 * \brief Packer constructor.
	 */
        Packer() ;

	/**
	 * \brief Packs a texture atlas.
	 * \param[in,out] mesh a surface mesh.
	 * \details The mesh is supposed to have texture coordinates
	 *  stored in a 2d vector attribute attached to the facet
	 *  corners and named "tex_coord".
	 */
        void pack_surface(Mesh& mesh, bool normalize_only);

	/**
	 * \brief Gets the size of the target texture image in
	 *  pixels.
	 * \return the size of the target texture image.
	 */
        index_t image_size_in_pixels() const {
	    return image_size_in_pixels_;
	}

	/**
	 * \brief Sets the size of the target texture image in
	 *  pixels.
	 * \param[in] size the size of the target texture image.
	 */
        void set_image_size_in_pixels(index_t size) {
            image_size_in_pixels_ = size;
        }

	/**
	 * \brief Gets the size of the margin (or "gutter") around the charts.
	 * \details This may be required to avoid undesirable blends due to
	 *  mip-mapping.
	 * \return the number of empty pixels to be preserved around each chart.
	 */
        index_t margin_width_in_pixels() const {
	    return margin_width_in_pixels_;
	}

	/**
	 * \brief Sets the size of the margin (or "gutter") around the charts.
	 * \details This may be required to avoid undesirable blends due to
	 *  mip-mapping.
	 * \param[in] width the number of empty pixels to be preserved 
	 *  around each chart.
	 */
        void set_margin_width_in_pixels(index_t width) {
            margin_width_in_pixels_ = width;
        } 

      protected:
	/**
	 * \brief Packs a set of charts.
	 * \param[in,out] charts a const reference to a vector with the 
	 *  charts to be packed.
	 * \param[in] normalize_only if set, just normalize texture coordinates
	 *  and do not pack the charts.
	 * \details All the charts are supposed to be attached to the same mesh.
	 *  Texture coordinates are stored in a 2d vector attribute attached to
	 *  the facet corners of the mesh and called "tex_coord".
	 */
        void pack_charts(vector<Chart>& charts, bool normalize_only = false);

	/**
	 * \brief Normalizes the parameterization of a chart.
	 * \param[in,out] chart a reference to the chart to be normalized.
	 * \details This rescales texture coordinates in such a way that the
	 *  chart has the same area in 3D and in texture space. 
	 *  This also applies a rotation to the texture coordinates such that 
	 *  the area of the bounding rectangle is minimized. 
	 *  Texture coordinates are stored in a 2d vector attribute attached 
	 *  to the facet corners of the mesh and called "tex_coord".
	 */
        void normalize_chart(Chart& chart);

      private:
        double total_area_3d_ ;
        index_t image_size_in_pixels_ ;
        index_t margin_width_in_pixels_ ;
	Attribute<double> tex_coord_;
	Attribute<index_t> chart_attr_;
	
      private:
	/**
	 * \brief Forbids copy.
	 */
        Packer(const Packer& rhs) ;

	/**
	 * \brief Forbids copy.
	 */
        Packer& operator=(const Packer& rhs) ;
    } ;
  
    /****************************************************************/

}
#endif

