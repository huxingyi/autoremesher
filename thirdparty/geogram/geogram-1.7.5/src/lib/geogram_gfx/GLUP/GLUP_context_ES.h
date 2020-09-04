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
        virtual ~Context_ES2();
        
        /**
         * \copydoc Context::profile_name()
         */
        virtual const char* profile_name() const;

        /**
         * \copydoc Context::setup()
         */
        virtual void setup();

        /**
         * \copydoc Context::primitive_supports_array_mode()
         */
        virtual bool primitive_supports_array_mode(GLUPprimitive prim) const;


        /**
         * \copydoc Context::get_primitive_pseudo_file()
         */
        virtual void get_primitive_pseudo_file(
            std::vector<GLSL::Source>& sources
        );
        
        /**
         * \copydoc Context::get_vertex_shader_preamble_pseudo_file()
         */
        virtual void get_vertex_shader_preamble_pseudo_file(
            std::vector<GLSL::Source>& sources
        );

        /**
         * \copydoc Context::get_fragment_shader_preamble_pseudo_file()
         */
        virtual void get_fragment_shader_preamble_pseudo_file(
            std::vector<GLSL::Source>& sources
        );

        /**
         * \copydoc Context::get_toggles_pseudo_file()
         */
        virtual void get_toggles_pseudo_file(
            std::vector<GLSL::Source>& sources            
        );

    protected:
        
        /**
         * \copydoc Context::prepare_to_draw()
         */
        virtual void prepare_to_draw(GLUPprimitive primitive);

        /**
         * \copydoc Context::done_draw()
         */
        virtual void done_draw(GLUPprimitive primitive);
        
        /**
         * \copydoc Context::get_state_variable_address()
         */
        Memory::pointer get_state_variable_address(const char* name);

        /**
         * \copydoc Context::do_update_uniform_buffer()
         */
        virtual void do_update_uniform_buffer();

        /**
         * \copydoc Context::copy_uniform_state_to_current_program()
         */
        virtual void copy_uniform_state_to_current_program();

        /**
         * \copydoc Context::update_base_picking_id()
         */
        virtual void update_base_picking_id(GLint new_value);
        
        /**
         * \copydoc Context::setup_GLUP_POINTS()
         */
        virtual void setup_GLUP_POINTS();

        /**
         * \copydoc Context::setup_GLUP_LINES()
         */
        virtual void setup_GLUP_LINES();


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
        virtual void setup_GLUP_TRIANGLES();

        /**
         * \copydoc Context::setup_GLUP_QUADS()
         */
        virtual void setup_GLUP_QUADS();

        /**
         * \copydoc Context::setup_GLUP_TETRAHEDRA()
         */
        virtual void setup_GLUP_TETRAHEDRA();

        /**
         * \copydoc Context::setup_GLUP_PRISMS()
         */
        virtual void setup_GLUP_PRISMS();

        /**
         * \copydoc Context::setup_GLUP_HEXAHEDRA()
         */
        virtual void setup_GLUP_HEXAHEDRA();

        /**
         * \copydoc Context::setup_GLUP_PYRAMIDS()
         */
        virtual void setup_GLUP_PYRAMIDS();

        /**
         * \copydoc Context::setup_GLUP_CONNECTORS()
         */
        virtual void setup_GLUP_CONNECTORS();

        /**
         * \copydoc Context::setup_GLUP_CONNECTORS()
         */
        virtual void setup_GLUP_SPHERES();
	
        /**
         * \copydoc Context::flush_immediate_buffers()
         */
        virtual void flush_immediate_buffers();


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

