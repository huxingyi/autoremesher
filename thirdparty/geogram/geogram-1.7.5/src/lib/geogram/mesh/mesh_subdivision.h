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

#ifndef GEOGRAM_MESH_MESH_SUBDIVISION
#define GEOGRAM_MESH_MESH_SUBDIVISION

/**
 * \file geogram/mesh/mesh_subdivision.h
 */

#include <geogram/basic/common.h>
#include <geogram/basic/numeric.h>
#include <geogram/basic/memory.h>

namespace GEO {

    class Mesh;

    /**
     * \brief A set of callbacks that specify how vertices attributes
     *  should be interpolated by subdivision functions.
     * \details Default implementation interpolates vertex geometry.
     */
    class GEOGRAM_API MeshSplitCallbacks {
      public:
	/**
	 * \brief MeshSplitCallbacks constructor.
	 * \param[in] mesh a pointer to the target mesh.
	 */
	MeshSplitCallbacks(Mesh* mesh);

	/**
	 * \brief MeshSplitCallbacks destructor.
	 */
	virtual ~MeshSplitCallbacks();

	/**
	 * \brief Creates a new vertex.
	 * \return the index of the newly created vertex.
	 */
	virtual index_t create_vertex();

	/**
	 * \brief Scales a vertex (v *= s).
	 * \param[in] v the vertex.
	 * \param[in] s the scaling coefficient.
	 */
	virtual void scale_vertex(index_t v, double s);

	/**
	 * \brief Zeroes all attributes of a vertex.
	 * \param[in] v the vertex.
	 */
	virtual void zero_vertex(index_t v);
	
	/**
	 * \brief Adds a scaled vertex to another one (v1 += s*v2).
	 * \param[in] v1 the vertex.
	 * \param[in] s scaling coefficient.
	 * \param[in] v2 the vertex to be added to \p v1.
	 */
	virtual void madd_vertex(
	    index_t v1, double s, index_t v2
	);
	
      protected:
	Mesh* mesh_;
    };
    
    /**
     * \brief Splits each triangle of a surface mesh into four.
     * \param[in,out] M a reference to a surface mesh
     * \param[in] facets_begin (optional) index of the first facet to be split
     * \param[in] facets_end (optional) one position past the index of the
     *   last facet to be split or index_t(-1) if unspecified
     * \param[in] cb an optional pointer to a MeshSplitCallbacks, indicating
     *   how vertices attributes should be interpolated.
     * \pre M.facets.are_simplices() == true
     */
    void GEOGRAM_API mesh_split_triangles(
	Mesh& M, index_t facets_begin = 0, index_t facets_end = index_t(-1),
	MeshSplitCallbacks* cb = nullptr
    );

    /**
     * \brief Splits each facet of a surface mesh into quads.
     * \param[in,out] M a reference to a surface mesh
     * \param[in] facets_begin (optional) index of the first facet to be split
     * \param[in] facets_end (optional) one position past the index of the
     *   last facet to be split or index_t(-1) if unspecified.
     * \param[in] cb an optional pointer to a MeshSplitCallbacks, indicating
     *   how vertices attributes should be interpolated.
     */
    void GEOGRAM_API mesh_split_quads(
	Mesh& M, index_t facets_begin = 0, index_t facets_end = index_t(-1),
	MeshSplitCallbacks* cb = nullptr	
    );

    /**
     * \brief Splits a mesh using Catmull-Clark subdivision rule.
     * \details If the surface mesh has a boundary, then its geometry is left
     *  unchanged.
     * \param[in,out] M a reference to a surface mesh.
     * \param[in] cb an optional pointer to a MeshSplitCallbacks, indicating
     *   how vertices attributes should be interpolated.
     */
    void GEOGRAM_API mesh_split_catmull_clark(
	Mesh& M, MeshSplitCallbacks* cb = nullptr
    );
}

#endif

