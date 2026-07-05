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

#ifndef GEOGRAM_GFX_GLUP_GLUP_CONTEXT_ES
#define GEOGRAM_GFX_GLUP_GLUP_CONTEXT_ES

#include <geogram_gfx/basic/common.h>
#include <geogram_gfx/GLUP/GLUP_context.h>

/**
 * \file geogram_gfx/GLUP/GLUP_context_ES.h
 * \brief Internal implementation of GLUP using OpenGL ES or
 *  GLSL 1.5 with no geometry shader and no uniform buffer.
 */

#ifdef GEO_GL_ES2

namespace GLUP {
    using namespace GEO;

    /*********************************************************************/

    /**
     * \brief Implementation of GLUP using OpenGL ES 2.0.
     * \details Can be also used with OpenGL 3.30. This is the default
     *  GLUP profile used on MacOS/X.
     * \note the following functionalities are not implemented (yet) in
     *   this profile:
     *  - picking is not implemented
     *  - indirect texture mode is not implemented (but anyway, there is
     *    no texture3D in ES)
     */
    class Context_ES2 : public Context {
    public:

        /**
         * \brief Context_ES2 constructor.
         */
        Context_ES2();
        
        /**
         * \brief Context_ES2 destructor.
         */
	~Context_ES2() override;
        
        /**
         * \copydoc Context::profile_name()
         */
        const char* profile_name() const override;

        /**
         * \copydoc Context::setup()
         */
        void setup() override;

        /**
         * \copydoc Context::primitive_supports_array_mode()
         */
	bool primitive_supports_array_mode(GLUPprimitive prim) const override;


        /**
         * \copydoc Context::get_primitive_pseudo_file()
         */
	void get_primitive_pseudo_file(
            std::vector<GLSL::Source>& sources
        ) override;
        
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
         * \copydoc Context::get_toggles_pseudo_file()
         */
	void get_toggles_pseudo_file(
            std::vector<GLSL::Source>& sources            
        ) override;

    protected:
        
        /**
         * \copydoc Context::prepare_to_draw()
         */
	void prepare_to_draw(GLUPprimitive primitive) override;

        /**
         * \copydoc Context::done_draw()
         */
	void done_draw(GLUPprimitive primitive) override;
        
        /**
         * \copydoc Context::get_state_variable_address()
         */
        Memory::pointer get_state_variable_address(const char* name) override;

        /**
         * \copydoc Context::do_update_uniform_buffer()
         */
	void do_update_uniform_buffer() override;

        /**
         * \copydoc Context::copy_uniform_state_to_current_program()
         */
	void copy_uniform_state_to_current_program() override;

        /**
         * \copydoc Context::update_base_picking_id()
         */
	void update_base_picking_id(GLint new_value) override;
        
        /**
         * \copydoc Context::setup_GLUP_POINTS()
         */
	void setup_GLUP_POINTS() override;

        /**
         * \copydoc Context::setup_GLUP_LINES()
         */
	void setup_GLUP_LINES() override;


        /**
         * \brief The generic primitive setup fonction used by all surfacic
         *  and volumetric primitives in this profile.
         * \details Current GLUP primitive type is deduced from 
         *  current value of Context::primitive_source_, set by 
         *  Context::setup_shader_source_for_primitive().
         * \param[in] nb_elements_per_glup_primitive the number of element
         *  indices for each glup primitive. For instance, when drawing
         *  GLUP tetrahedra using OpenGL triangles, there are 4*3 = 12
         *  elements per primitive.
         * \param[in] element_indices a pointer to an array of 
         *  nb_elements_per_glup_primitive integers that encode the
         *  indexing of one element. This array is replicated and shifted
         *  to generate the element index buffer. 
         */
        void setup_primitive_generic(
            index_t nb_elements_per_glup_primitive,
            index_t* element_indices
        );
        
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
         * \copydoc Context::setup_GLUP_CONNECTORS()
         */
	void setup_GLUP_SPHERES() override;
	
        /**
         * \copydoc Context::flush_immediate_buffers()
         */
	void flush_immediate_buffers() override;


        /**
         * \brief Tests whether by-cell clipping is used.
         * \details By-cell clipping is used if a volumetric primitive
         *  is being drawn, if clipping is enabled and if current clipping
         *  mode is GLUP_CLIP_WHOLE_CELLS or GLUP_CLIP_STRADDLING_CELLS. In
         *  this case, software-assisted clipping is used to generate an
         *  element buffer that points to the cells that are visible.
         */
        bool cell_by_cell_clipping() const;

        /**
         * \brief Tests whether sliced-cells clipping is used.
         * \details By-cell clipping is used if a volumetric primitive
         *  is being drawn, if clipping is enabled and if current clipping
         *  mode is GLUP_CLIP_SLICED_CELLS. In
         *  this case, software-assisted clipping is used to generate an
         *  element buffer with the computed intersections.
         */
        bool sliced_cells_clipping() const;
        
        /**
         * \brief Special implementation of flush_immediate_buffers()
         *  that performs cell-by-cell clipping on the CPU side, and
         *  that generates an element buffer for drawing the selected
         *  cells.
         */
        void flush_immediate_buffers_with_cell_by_cell_clipping();

        /**
         * \brief Special implementation of flush_immediate_buffers()
         *  that performes cell slicing on the GPU side and that
         *  uses an element buffer for drawing the selected cells.
         */
        void flush_immediate_buffers_with_sliced_cells_clipping();

        
    private:
        index_t nb_clip_cells_elements_;        
        Numeric::uint16* clip_cells_elements_;
        
        GLuint clip_cells_elements_VBO_;
        GLuint clip_cells_VAO_;

        GLuint sliced_cells_elements_VBO_;
        GLuint sliced_cells_vertex_attrib_VBO_[4];
        GLuint sliced_cells_VAO_;

	double GLSL_version_;

	bool vertex_id_VBO_bound_;
    };

    /*********************************************************************/
    
}

#endif

#endif

