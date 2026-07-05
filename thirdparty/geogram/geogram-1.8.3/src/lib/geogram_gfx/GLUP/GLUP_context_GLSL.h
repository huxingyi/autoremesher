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

#ifndef GEOGRAM_GFX_GLUP_GLUP_CONTEXT_GLSL
#define GEOGRAM_GFX_GLUP_GLUP_CONTEXT_GLSL

#include <geogram_gfx/basic/common.h>
#include <geogram_gfx/GLUP/GLUP_context.h>

/**
 * \file geogram_gfx/GLUP/GLUP_context_GLSL.h
 * \brief Internal implementation of GLUP using modern OpenGL and GLSL shaders.
 */

#ifdef GEO_GL_150

namespace GLUP {
    using namespace GEO;

    /*********************************************************************/

    /**
     * \brief Implementation of GLUP using modern OpenGL with GLSL 1.50
     *  shaders. 
     * \details All the primitives are implemented with good performance.
     *  Hexahedra and prisms do not support array mode (glupDrawArrays(),
     *  glupDrawElements()). This is because there is no standard OpenGL
     *  primitive with 8 or 5 vertices (except the configurable GL_PATCH
     *  that requires GLSL 4.40).
     */
    class Context_GLSL150 : public Context {
    public:

        /**
         * \brief Context_GLSL150 constructor.
         */
	Context_GLSL150();
	
        /**
         * \copydoc Context::profile_name()
         */
        const char* profile_name() const override;

        /**
         * \copydoc Context::setup()
         */
        void setup() override;
        
    protected:
        /**
         * \copydoc Context::setup_GLUP_POINTS()
         */
        void setup_GLUP_POINTS() override;

        /**
         * \copydoc Context::setup_GLUP_LINES()
         */
        void setup_GLUP_LINES() override;

        /**
         * \copydoc Context::setup_GLUP_TRIANGLES()
         */
        void setup_GLUP_TRIANGLES() override;

        /**
         * \copydoc Context::setup_GLUP_QUADS()
         */
        void setup_GLUP_QUADS() override;

        /**
         * \copydoc Context::setup_GLUP_TETRAHEDRA()
         */
        void setup_GLUP_TETRAHEDRA() override;

        /**
         * \copydoc Context::setup_GLUP_PRISMS()
         */
        void setup_GLUP_PRISMS() override;

        /**
         * \copydoc Context::setup_GLUP_HEXAHEDRA()
         */
        void setup_GLUP_HEXAHEDRA() override;

        /**
         * \copydoc Context::setup_GLUP_PYRAMIDS()
         */
        void setup_GLUP_PYRAMIDS() override;

        /**
         * \copydoc Context::setup_GLUP_CONNECTORS()
         */
        void setup_GLUP_CONNECTORS() override;

        /**
         * \copydoc Context::setup_GLUP_SPHERES()
         */
        void setup_GLUP_SPHERES() override;
	
        /**
         * \copydoc Context::get_vertex_shader_preamble_pseudo_file()
         */
        void get_vertex_shader_preamble_pseudo_file(
            std::vector<GLSL::Source>& sources
        ) override;

        /**
         * \copydoc Context::get_fragment_shader_preamble_pseudo_file()
         */
        void get_fragment_shader_preamble_pseudo_file(
            std::vector<GLSL::Source>& sources
        ) override;

        /**
         * \copydoc Context::get_geometry_shader_preamble_pseudo_file()
         */
        void get_geometry_shader_preamble_pseudo_file(
            std::vector<GLSL::Source>& sources
        ) override;

        /**
         * \copydoc Context::get_primitive_pseudo_file()
         */
        void get_primitive_pseudo_file(
            std::vector<GLSL::Source>& sources            
        ) override;

        /**
         * \brief Deduces from the current primitive_source_ the
         *  input and output layout that should be used by the
         *  geometry shader.
         */
        virtual void get_geometry_shader_layout(
            std::vector<GLSL::Source>& sources                        
        );
    };

    /*********************************************************************/

    /**
     * \brief Implementation of GLUP using modern OpenGL with GLSL 4.40
     *  shaders. 
     * \details This mostly reuses the GLSL 1.50 implementation, except
     *  for hexahedra and prisms, where it uses a tessellation shader to
     *  fetch the vertices. This is because GL_PATCH has a configurable
     *  number of vertices.
     */
    class Context_GLSL440 : public Context_GLSL150 {
    public:

        /**
         * \brief Context_GLSL440 constructor.
         */
        Context_GLSL440();
        
        /**
         * \copydoc Context::profile_name()
         */
        const char* profile_name() const override;
        
    protected:
        /**
         * \copydoc Context::setup_GLUP_HEXAHEDRA()
         */
        void setup_GLUP_HEXAHEDRA() override;

        /**
         * \copydoc Context::setup_GLUP_PYRAMIDS()
         */
        void setup_GLUP_PYRAMIDS() override;

        /**
         * \copydoc Context::get_vertex_shader_preamble_pseudo_file()
         */
        void get_vertex_shader_preamble_pseudo_file(
            std::vector<GLSL::Source>& sources
        ) override;

        /**
         * \copydoc Context::get_fragment_shader_preamble_pseudo_file()
         */
        void get_fragment_shader_preamble_pseudo_file(
            std::vector<GLSL::Source>& sources
        ) override;

        /**
         * \copydoc Context::get_geometry_shader_preamble_pseudo_file()
         */
        void get_geometry_shader_preamble_pseudo_file(
            std::vector<GLSL::Source>& sources
        ) override;

        /**
         * \copydoc Context::get_tess_evaluation_shader_preamble_pseudo_file()
         */
        void get_tess_evaluation_shader_preamble_pseudo_file(
            std::vector<GLSL::Source>& sources
        ) override;
        
        /**
         * \copydoc Context::get_primitive_pseudo_file()
         */
        void get_primitive_pseudo_file(
            std::vector<GLSL::Source>& sources            
        ) override;

        /**
         * \copydoc Context_GLSL150::get_geometry_shader_layout()
         */
        void get_geometry_shader_layout(
            std::vector<GLSL::Source>& sources                        
        ) override;
        
        bool use_tessellation_;
    };

    /*********************************************************************/
}

#endif

#endif

