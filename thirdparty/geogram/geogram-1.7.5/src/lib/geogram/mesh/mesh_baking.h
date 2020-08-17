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

#ifndef H_GEO_MESH_ALGO_MESH_BAKING_H
#define H_GEO_MESH_ALGO_MESH_BAKING_H

#include <geogram/basic/common.h>
#include <geogram/basic/attributes.h>

/**
 * \file geogram/mesh/mesh_baking.h
 * \brief Functions to bake attributes from meshes to
 *  images.
 */

namespace GEO {

    class Mesh;
    class Image;

    /**
     * \brief Bakes the facet normals of a parameterized mesh
     *  into an image.
     * \details The facet normals are baked over the previous content
     *  of the image (the image is not cleared).
     * \param[in] mesh a pointer to the input esh, with a
     *   texture coordinates stored in the "tex_coord" facet 
     *   corner attribute.
     * \param[out] target the target image.
     */
    void GEOGRAM_API bake_mesh_facet_normals(Mesh* mesh, Image* target);

    /**
     * \brief Bakes the facet normals of a parameterized mesh
     *  into an image.
     * \details The facet normals are baked over the previous content
     *  of the image (the image is not cleared). Normals are linearly
     *  interpolated in mesh facets.
     * \param[in] mesh a pointer to the input esh, with a
     *   texture coordinates stored in the "tex_coord" facet 
     *   corner attribute.
     * \param[out] target the target image.
     */
    void GEOGRAM_API bake_mesh_vertex_normals(Mesh* mesh, Image* target);
    
    /**
     * \brief Bakes an attribute of a parameterized mesh
     * \details The attribute is baked over the previous content
     *  of the image (the image is not cleared).
     * \param[in] mesh a pointer to the input mesh, with a
     *   texture coordinates stored in the "tex_coord" facet 
     *   corner attribute.
     * \param[out] target the target image.
     * \param[in] attribute the attribute, can be a vertex, facet corner
     *  or facet attribute. Vertex and facet corner attributes are 
     *  linearly interpolated. If it is a vector attribute, its available
     *  components are copied to the r,g,b,a components of the interpolated 
     *  color.
     * \param[in] bias optional value to be added to the attribute values 
     *  before storing them into the color components.
     * \param[in] scale optional value that scales the attribute values 
     *  after \p bias is added and before storing them into the color 
     *  components.
     */
    void GEOGRAM_API bake_mesh_attribute(
	Mesh* mesh, Image* target, Attribute<double>& attribute,
	double bias=0.0, double scale=1.0
    );

    /**
     * \brief Bakes mesh geometry into an image (creates a geometry image).
     * \param[in] mesh a pointer to the input mesh, with a
     *   texture coordinates stored in the "tex_coord" facet 
     *   corner attribute.
     * \param[out] target a pointer to the geometry image. It should use
     *   Image::FLOAT64 storage for the color components.
     * \param[in] clear if set, then all the pixel components of the target 
     *   geometry image are set to Numeric:max_float64() before rasterizing 
     *   the facets. 
     */
    void GEOGRAM_API bake_mesh_geometry(
	Mesh* mesh, Image* target, bool clear = true
    );
    
    /**
     * \brief Bakes facet normals from a (in general) high resolution
     *  mesh to a (in general) parameterized lower resolution mesh
     *  represented as a geometry image. 
     * \details The facet normals are baked over the previous content
     *  of the image (the image is not cleared). The implementation uses
     *  a MeshFacetsAABB that changes the order of the elements of \p highres.
     *  Whenever a pixel of \p geometry has both its r,g,b components set
     *  to Numeric::max_float64(), the corresponding pixel in \p target is left
     *  unchanged.
     * \param[in] geometry the geometry image
     * \param[out] target the target image
     * \param[in] highres the high-resolution mesh
     * \pre \p geometry and \p target have the same size, and \p geometry uses
     *  FLOAT64 component encoding.
     */
    void GEOGRAM_API bake_mesh_facet_normals_indirect(
	Image* geometry, Image* target, Mesh* highres
    );


    /**
     * \brief Bakes a vertex attribute from a (in general) 
     *  high resolution point set to a (in general) parameterized 
     *  lower resolution mesh represented as a geometry image. 
     * \details The attribute is baked over the previous content
     *  of the image (the image is not cleared). 
     *  Whenever a pixel of \p geometry has both its r,g,b components set
     *  to Numeric::max_float64(), the corresponding pixel in \p target is left
     *  unchanged.
     * \param[in] geometry the geometry image
     * \param[out] target the target image
     * \param[in] highres the high-resolution pointset. 
     * \param[in] attribute the vertex attribute of \p highres to be baked.
     * \pre geometry and target have the same size, and geometry uses
     *  FLOAT64 component encoding.
     * \param[in] bias optional value to be added to the attribute values 
     *  before storing them into the color components.
     * \param[in] scale optional value that scales the attribute values 
     *  after \p bias is added and before storing them into the color 
     *  components.
     */
    void GEOGRAM_API bake_mesh_points_attribute_indirect(
	Image* geometry, Image* target,
	Mesh* highres, Attribute<double>& attribute,
	double bias=0.0, double scale=1.0
    );
}

#endif
