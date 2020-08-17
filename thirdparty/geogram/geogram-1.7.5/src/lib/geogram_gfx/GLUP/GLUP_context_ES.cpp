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

#include <geogram_gfx/GLUP/GLUP_context_ES.h>
#include <geogram_gfx/basic/GLSL.h>
#include <geogram/basic/string.h>

#ifdef GEO_OS_EMSCRIPTEN
#include <emscripten.h>
#endif

#ifdef GEO_GL_ES2

/*
 * Notes:
 *   J'ai reussi a corriger le bug de picking de primitives WEBGL: ce sont les
 *  int-s encodes dans des float-s qui font un peu n'importe quoi. En ajoutant
 *  0.5 au primitive_id a la sortie du vertex shader, ca regle le probleme.
 *
 *   Il faudra peut-etre quand meme encoder les valeurs de picking de l'etat
 *  sous forme de vec4 plutot qu'en int, parceque je ne suis pas sur que ces
 *  ints puissent stocker plus que 65535 (mais le fait que je voie du bleu 
 *  quand on met plein de primitive tend a montrer que ca marche quand meme...)
 *
 *   Il manque encore le picking en mode "slice cell"
 *
 *   Les triangles n'ont pas besoin d'indices d'elements je pense ...
 *
 * Slice cells:
 * ============
 * Pour implanter le mode "slice cells", je me demande s'il vaut mieux
 *  - avoir un tout petit buffer, et faire un flush par cellule ?
 *  - tout assembler dans des gros buffers, et faire un flush tous les
 *    65K sommets ?
 *  -> essayer le premier (plus simple a programmer), et si \c{c}a rame
 *    faire le deuxi\`eme:
 *       Ca a l'air de marcher pas trop mal.
 *       Rem: le ELEMENT_ARRAY_BUFFER est en entiers 32 bits, certaines archis
 *    (telephones etc...) peuvent avoir besoin d'un ELEMENT_ARRAY_BUFFER 
 *    16 bits (ce qu'on pourrait assez facilement avoir...)
 *
 * texture 3D:
 * ===========
 *   OpenGL ES 2.0 n'a pas de textures 3D, mais on peut les emuler avec des
 * textures 2D:
 * https://www.khronos.org/webgl/wiki/WebGL_and_OpenGL_Differences
 * mais existe peut-etre une extension (a tester)
 *
 * The shaders
 * ===========
 * The shaders can be compiled either in OpenGL ES, GLSL 1.3 or GLSL 1.5.
 *  OpenGL ES mode is used by Emscripten.
 *  GLSL 1.3 mode is used by desktop OpenGL on:
 *     - Linux Intel i915
 *     - Linux NVidia
 *     - Windows
 *  GLSL 1.5 mode is used by destop OpenGL on MacOSX (MacOSX supports 1.5 
 *   but does not support 1.3)
 *
 * TODO: In the shaders, in/out declarations are selected by #ifdef GL_ES,
 *  instead of testing GLSL version, is it normal ?
 */

namespace {
    using namespace GEO;

    /**
     * \brief Creates the source of a GLSL function that
     *   gets mesh texture coordinates.
     * \details This function is necessary because we do not have 
     *   a way of specifying uniform constant arrays in OpenGL ES 2.0
     * \param[in] tex_coords an array of 4*\p nb_tex_coords floats
     * \param[in] nb_tex_coords number of vec4s that define texture coordinates
     * \return a std::string with the source of the shader
     */
    std::string create_mesh_tex_coord_lookup_function(
        GLUPfloat* tex_coords, index_t nb_tex_coords
    ) {
        std::ostringstream output;
        output <<
            "vec4 get_mesh_tex_coord(in int vertex_id) { \n"
            "    int idx = glup_mod(vertex_id," << nb_tex_coords << ");\n";
        for(index_t i=0; i<nb_tex_coords; ++i) {
            output << "    if(idx == " << i << ") {";
            output << " return vec4("
                   << tex_coords[4*i  ] << ","
                   << tex_coords[4*i+1] << ","
                   << tex_coords[4*i+2] << ","
                   << tex_coords[4*i+3] << "); }\n";
        }
        //   Will never get here, but some GLSL compilers will not
        // accept a function that may not return a value.
        output << "   return vec4(0.0, 0.0, 0.0, 0.0);\n";
        output << "}\n";
        return output.str();
    }

    /**
     * \brief Creates the source of a GLSL function that
     *   gets mesh texture coordinates.
     * \details The size of the texture coordinates array is
     *   automatically computed.
     * \param[in] tex an array of texture coordinates
     * \return a pointer to a string with the sources of
     *  the shader.
     */

#define mesh_tex_coord_lookup(tex)                    \
    create_mesh_tex_coord_lookup_function(            \
        tex, index_t(sizeof(tex) / (4*sizeof(float))) \
    )
}

namespace GLUP {

    extern bool vertex_array_emulate;
    
    const char* Context_ES2::profile_name() const {
        return "GLUPES2";
    }

    void Context_ES2::prepare_to_draw(GLUPprimitive primitive) {
        Context::prepare_to_draw(primitive);
#if defined(GEO_GL_150) && defined(GL_POINT_SPRITE)
        if(
            (!use_core_profile_ || use_ES_profile_) &&
            primitive == GLUP_POINTS
        ) {
            glEnable(GL_POINT_SPRITE);
        }
#endif
    }

    void Context_ES2::done_draw(GLUPprimitive primitive) {
        Context::done_draw(primitive);
    }
    
    bool Context_ES2::primitive_supports_array_mode(GLUPprimitive prim) const {
        // Note: points, spheres, lines and triangles without mesh can
	// support array mode, but this will not work with picking,
	// therefore it is disabled.
#ifdef GEO_OS_ANDROID
	return
	    (prim == GLUP_POINTS ||
	     prim == GLUP_SPHERES ||
	     prim == GLUP_LINES ||
	     prim == GLUP_TRIANGLES);
#else	
        geo_argused(prim);
	return false;
#endif	
    }

    /**
     * \brief Computes the number of elements in the buffer for
     *  a given primitive.
     * \param[in] nb_vertices_per_glup_primitive number of vertices
     *  per primitive
     * \param[in] nb_elements_per_glup_primitive number of elements
     *  per primitive
     * \return total number of elements required to draw all the
     *  primitives in the buffer.
     */
    inline index_t nb_elements(
        index_t nb_vertices_per_glup_primitive,
        index_t nb_elements_per_glup_primitive
    ) {
        index_t nb_glup_primitives = 
            IMMEDIATE_BUFFER_SIZE / nb_vertices_per_glup_primitive;
        return (
            nb_glup_primitives * nb_elements_per_glup_primitive
        );
    }

    
    void Context_ES2::setup() {

        bool extension_standard_derivatives = false;
        bool extension_vertex_array_object = false;
        
#ifdef GEO_OS_EMSCRIPTEN

/**
 * \brief Tests whether a WebGL extension is supported.       
 * \details WebGL requires getExtension() to be called for initializing
 *  the extension (if supported, this returns an extension object,
 *  else this returns null).
 * \param[in] ext the name of the extension surrounded by single quotes
 * \retval true if the extension is supported
 * \retval false otherwise
 */
#define WEBGL_EXTENSION_SUPPORTED(ext)                      \
        EM_ASM_INT({return (                                \
             Module.ctx.getExtension(ext) == null ? 0 : 1   \
        );},0) != 0 

        
        extension_standard_derivatives = WEBGL_EXTENSION_SUPPORTED(
            'OES_standard_derivatives'
        );

        extension_vertex_array_object = WEBGL_EXTENSION_SUPPORTED(
            'OES_vertex_array_object'
        );

#else
        //TODO: it seems that standard derivatives and vertex array
        //object extension names are not prefixed with GL_OES_ in
        //OpenGL ES mode (but they are in WebGL, oh my....)
        
        if(use_ES_profile_) {
            extension_standard_derivatives =
                extension_is_supported("GL_OES_standard_derivatives");

	    // ES profile can be artificially set to true for non-NVidia
	    // GPUs, see constructor.
            extension_vertex_array_object =
                extension_is_supported("GL_OES_vertex_array_object") ||
	        extension_is_supported("GL_ARB_vertex_array_object") ;
	    
        } else {
            extension_standard_derivatives = true;
            extension_vertex_array_object = true;
        }
#endif

        //   Switch-on GLUP's emulation for Vertex Array
        // Objects if they are not supported.
        if(!extension_vertex_array_object) {
            GLUP::vertex_array_emulate = true;
        }

        Logger::out("GLUP") 
            << "OES_standard_derivatives: "
            << extension_standard_derivatives
            << std::endl;
	
        Logger::out("GLUP")
            << "vertex_array_object: "
            << extension_vertex_array_object
            << std::endl;

        create_CPU_side_uniform_buffer();


        /************** VAOs and VBOs for whole cells clipping ******/
        
        //   Compute the maximum number of elements required to draw
        // the vertices in a buffer, for all types of volumetric
        // primitives.
        index_t max_nb_elements = 0;
        max_nb_elements = std::max(max_nb_elements, nb_elements(4,12));
        max_nb_elements = std::max(max_nb_elements, nb_elements(8,36));
        max_nb_elements = std::max(max_nb_elements, nb_elements(6,24));
        max_nb_elements = std::max(max_nb_elements, nb_elements(5,18));        
        max_nb_elements = std::max(max_nb_elements, nb_elements(4,12));        
        
        nb_clip_cells_elements_ = max_nb_elements;        
        clip_cells_elements_ = new Numeric::uint16[nb_clip_cells_elements_];

        glupGenVertexArrays(1,&clip_cells_VAO_);
        glupBindVertexArray(clip_cells_VAO_);
        
        bind_immediate_state_buffers_to_VAO();

        glGenBuffers(1, &clip_cells_elements_VBO_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, clip_cells_elements_VBO_);

        update_buffer_object(
            clip_cells_elements_VBO_,
            GL_ELEMENT_ARRAY_BUFFER,
            max_nb_elements * sizeof(Numeric::uint32),
            nullptr // no need to copy the buffer, it will be overwritten after.
        );

        glupBindVertexArray(0);

        // Shaders in GLSL 2.0 do not support gl_VertexID, we need to
        // emulate it.
        create_vertex_id_VBO();
        
        /************** VAOs and VBOs for sliced cells clipping ******/

        max_nb_elements = 6;
        index_t max_nb_vertices = 12;
        
        glupGenVertexArrays(1,&sliced_cells_VAO_);
        glupBindVertexArray(sliced_cells_VAO_);

        glGenBuffers(1, &sliced_cells_elements_VBO_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sliced_cells_elements_VBO_);

        update_buffer_object(
            sliced_cells_elements_VBO_,
            GL_ELEMENT_ARRAY_BUFFER,
            max_nb_elements * sizeof(Numeric::uint16),
            nullptr // no need to copy the buffer, it will be overwritten after.
        );
        
        for(index_t i=0; i<4; ++i) {
            glGenBuffers(1, &sliced_cells_vertex_attrib_VBO_[i]);
            glBindBuffer(GL_ARRAY_BUFFER, sliced_cells_vertex_attrib_VBO_[i]);

            update_buffer_object(
                sliced_cells_vertex_attrib_VBO_[i],
                GL_ARRAY_BUFFER,
                max_nb_vertices * sizeof(Numeric::float32) * 4,
                nullptr // no need to copy the buffer, it will be overwritten.
            );

            glVertexAttribPointer(
                i,
                4,
                GL_FLOAT,
                GL_FALSE,
                0,
                nullptr
            );
        }
        glEnableVertexAttribArray(0);
        glupBindVertexArray(0);
    }

    Context_ES2::Context_ES2() :
        nb_clip_cells_elements_(0),
        clip_cells_elements_(nullptr),        
        clip_cells_elements_VBO_(0),
        clip_cells_VAO_(0),
        sliced_cells_elements_VBO_(0),
        sliced_cells_VAO_(0),
        vertex_id_VBO_bound_(false) {
        for(index_t i=0; i<4; ++i) {
            sliced_cells_vertex_attrib_VBO_[i] = 0;
        }
	GLSL_version_ = GLSL::supported_language_version();
#if defined(GEO_OS_EMSCRIPTEN) || defined(GEO_OS_ANDROID)
        use_ES_profile_ = true;
	Logger::out("GLUP") << "ES2 context, supported GLSL version = " << GLSL_version_
			    << std::endl;
#endif	
    }
    
    Context_ES2::~Context_ES2() {
        glDeleteBuffers(1,&clip_cells_elements_VBO_);
        glupDeleteVertexArrays(1, &clip_cells_VAO_);
        for(index_t i=0; i<4; ++i) {
            glDeleteBuffers(1, &sliced_cells_vertex_attrib_VBO_[i]);
        }
        glupDeleteVertexArrays(1, &sliced_cells_VAO_);
        delete[] clip_cells_elements_;
    }
    
    Memory::pointer Context_ES2::get_state_variable_address(
        const char* name
    ) {
        geo_assert(
            variable_to_offset_.find(name) != variable_to_offset_.end()
        );
        return uniform_buffer_data_ + variable_to_offset_[name];
    }

    void Context_ES2::do_update_uniform_buffer() {
        GEO_CHECK_GL();
        
        if(!uniform_buffer_dirty_) {
            return;
        }

        GEO_CHECK_GL();
        
        if(matrices_dirty_) {
            update_matrices();
        }

        GEO_CHECK_GL();
        
        if(lighting_dirty_) {
            update_lighting();
        }

        GEO_CHECK_GL();
        
        glUseProgram(latest_program_);

        GEO_CHECK_GL();

        if(latest_program_ != 0) {
            copy_uniform_state_to_current_program();
        }
        
        GEO_CHECK_GL();
        
        uniform_buffer_dirty_ = false;
    }

    void Context_ES2::copy_uniform_state_to_current_program() {
        GLint loc;

        // Matrices (in vertex shader).
        {
            loc = glGetUniformLocation(
                latest_program_, "GLUP_VS.modelviewprojection_matrix"
            );
            glUniformMatrix4fv(
                loc, 1, GL_FALSE,
                uniform_state_.modelviewprojection_matrix.get_pointer()
            );
            loc = glGetUniformLocation(
                latest_program_, "GLUP_VS.modelview_matrix"
            );
            glUniformMatrix4fv(
                loc, 1, GL_FALSE,
                uniform_state_.modelview_matrix.get_pointer()
            );
            loc = glGetUniformLocation(
                latest_program_, "GLUP_VS.projection_matrix"
            );
            glUniformMatrix4fv(
                loc, 1, GL_FALSE,
                uniform_state_.projection_matrix.get_pointer()
            );
            loc = glGetUniformLocation(
                latest_program_, "GLUP_VS.texture_matrix"
            );
            glUniformMatrix4fv(
                loc, 1, GL_FALSE,
                uniform_state_.texture_matrix.get_pointer()
            );
            loc = glGetUniformLocation(
                latest_program_, "GLUP_VS.normal_matrix"
            );

            // Normal matrix is stored in state buffer with padding.
            float normal_matrix[9];
            for(index_t i=0; i<3; ++i) {
                for(index_t j=0; j<3; ++j) {
                    normal_matrix[i*3+j] =
                        uniform_state_.normal_matrix.get_pointer()[i*4+j];
                }
            }
            
            glUniformMatrix3fv(loc, 1, GL_FALSE, normal_matrix);


	    loc = glGetUniformLocation(
		latest_program_, "GLUP_VS.inverse_modelview_matrix"		
	    );
	    glUniformMatrix4fv(
		loc, 1, GL_FALSE,
		uniform_state_.inverse_modelview_matrix.get_pointer()
	    );
	    loc = glGetUniformLocation(
		latest_program_, "GLUP_VS.inverse_projection_matrix"		
	    );
	    glUniformMatrix4fv(
		loc, 1, GL_FALSE,
		uniform_state_.inverse_projection_matrix.get_pointer()
	    );
	    loc = glGetUniformLocation(
		latest_program_, "GLUP_VS.inverse_modelviewprojection_matrix"
	    );
	    glUniformMatrix4fv(
		loc, 1, GL_FALSE,
		uniform_state_.inverse_modelviewprojection_matrix.get_pointer()
	    );
	    
            loc = glGetUniformLocation(
                latest_program_, "GLUP_VS.viewport"
            );
	    GLint viewport[4];
	    glGetIntegerv(GL_VIEWPORT, viewport);
	    glUniform4f(
		loc,
		float(viewport[0]),
		float(viewport[1]),
		float(viewport[2]),
		float(viewport[3])		
	    );
        }

	// Matrices (in fragment shader)
	{
            loc = glGetUniformLocation(
                latest_program_, "GLUP.texture_matrix"
            );
            glUniformMatrix4fv(
                loc, 1, GL_FALSE,
                uniform_state_.texture_matrix.get_pointer()
            );
	    loc = glGetUniformLocation(
		latest_program_, "GLUP.modelviewprojection_matrix"		
	    );
	    glUniformMatrix4fv(
		loc, 1, GL_FALSE,
		uniform_state_.modelviewprojection_matrix.get_pointer()
	    );
	    loc = glGetUniformLocation(
		latest_program_, "GLUP.inverse_modelviewprojection_matrix"
	    );
	    glUniformMatrix4fv(
		loc, 1, GL_FALSE,
		uniform_state_.inverse_modelviewprojection_matrix.get_pointer()
	    );

            loc = glGetUniformLocation(
                latest_program_, "GLUP.normal_matrix"
            );
            // Normal matrix is stored in state buffer with padding.
            float normal_matrix[9];
            for(index_t i=0; i<3; ++i) {
                for(index_t j=0; j<3; ++j) {
                    normal_matrix[i*3+j] =
                        uniform_state_.normal_matrix.get_pointer()[i*4+j];
                }
            }
            glUniformMatrix3fv(loc, 1, GL_FALSE, normal_matrix);

            loc = glGetUniformLocation(
                latest_program_, "GLUP.viewport"
            );
	    GLint viewport[4];
	    glGetIntegerv(GL_VIEWPORT, viewport);
	    glUniform4f(
		loc,
		float(viewport[0]),
		float(viewport[1]),
		float(viewport[2]),
		float(viewport[3])		
	    );
	}
	
        // Points (in vertex shader).
        {
            loc = glGetUniformLocation(
                latest_program_, "GLUP_VS.point_size"
            );
            glUniform1f(loc, uniform_state_.point_size.get());
        }
        
        
        // Lighting.
        {
            loc = glGetUniformLocation(
                latest_program_, "GLUP.light_vector"
            );
            glUniform3fv(
                loc, 1, uniform_state_.light_vector.get_pointer()
            );
            loc = glGetUniformLocation(
                latest_program_, "GLUP.light_half_vector"
            );
            glUniform3fv(
                loc, 1, uniform_state_.light_half_vector.get_pointer()
            );
        }

        // All the toggles.
        for(index_t i=0; i<uniform_state_.toggle.size(); ++i) {
            loc = glGetUniformLocation(
                latest_program_,
                ("GLUP." + uniform_state_.toggle[i].name()).c_str()
            );
            glUniform1i(
                loc, GLint(uniform_state_.toggle[i].get())
            );
        }

        // Colors.
        if(!uniform_state_.toggle[GLUP_VERTEX_COLORS].get()) {
            for(index_t i=0; i<uniform_state_.color.size(); ++i) {
                loc = glGetUniformLocation(
                    latest_program_,
                    ("GLUP." + uniform_state_.color[i].name()).c_str()
                    );
                glUniform4fv(
                    loc, 1, uniform_state_.color[i].get_pointer()
                );
            }
        }

        // Mesh.
        if(uniform_state_.toggle[GLUP_DRAW_MESH].get()) {
            loc = glGetUniformLocation(latest_program_, "GLUP.mesh_width");
            glUniform1f(loc, uniform_state_.mesh_width.get());
        }

        // Texturing.
        if(uniform_state_.toggle[GLUP_TEXTURING].get()) {
            loc = glGetUniformLocation(
                latest_program_, "GLUP.texture_mode"
            );
            glUniform1i(loc, uniform_state_.texture_mode.get());
            loc = glGetUniformLocation(
                latest_program_, "GLUP.texture_type"
            );
            glUniform1i(loc, uniform_state_.texture_type.get());
        }

        // Cell shrink
        loc = glGetUniformLocation(latest_program_, "GLUP.cells_shrink");
        glUniform1f(loc, uniform_state_.cells_shrink.get());

        // Picking
        if(uniform_state_.toggle[GLUP_PICKING].get()) {
            loc = glGetUniformLocation(
                latest_program_, "GLUP.picking_mode"
            );
            glUniform1i(loc, uniform_state_.picking_mode.get());
            loc = glGetUniformLocation(
                latest_program_, "GLUP.picking_id"
            );
            glUniform1i(loc, uniform_state_.picking_id.get());
            loc = glGetUniformLocation(
                latest_program_, "GLUP.base_picking_id"
            );
            glUniform1i(loc, uniform_state_.base_picking_id.get());
        }

        // Clipping.
        if(uniform_state_.toggle[GLUP_CLIPPING].get()) {
            loc = glGetUniformLocation(
                latest_program_, "GLUP.clipping_mode"
            );
            glUniform1i(loc, uniform_state_.clipping_mode.get());
            loc = glGetUniformLocation(
                latest_program_, "GLUP.clip_plane"
            );
            glUniform4fv(loc, 1, uniform_state_.clip_plane.get_pointer());
            loc = glGetUniformLocation(
                latest_program_, "GLUP_VS.world_clip_plane"
            );
            glUniform4fv(
                loc, 1, uniform_state_.world_clip_plane.get_pointer()
            );
            loc = glGetUniformLocation(
                latest_program_, "GLUP.world_clip_plane"
            );
            glUniform4fv(
                loc, 1, uniform_state_.world_clip_plane.get_pointer()
            );
        }

        // alpha discard.
        if(uniform_state_.toggle[GLUP_ALPHA_DISCARD].get()) {
            loc = glGetUniformLocation(
                latest_program_, "GLUP.alpha_threshold"
            );
            glUniform1f(loc, uniform_state_.alpha_threshold.get());
	}

	// specular
	{
            loc = glGetUniformLocation(
                latest_program_, "GLUP.specular"
            );
            glUniform1f(loc, uniform_state_.specular.get());
	}
    }

    void Context_ES2::update_base_picking_id(GLint new_value) {
        if(uniform_state_.toggle[GLUP_PICKING].get()) {
            uniform_state_.base_picking_id.set(new_value);
            if(latest_program_ != 0) {
                GEO_CHECK_GL();                                        
                GLint loc = glGetUniformLocation(
                    latest_program_, "GLUP.base_picking_id"
                );
                GEO_CHECK_GL();
                glUseProgram(latest_program_);
                glUniform1i(loc, new_value);
                GEO_CHECK_GL();                
            }
        }
    }


/*****************************************************************************/

    void Context_ES2::setup_GLUP_POINTS() {
        set_primitive_info(
            GLUP_POINTS, GL_POINTS,
            GLSL::compile_program_with_includes_no_link(
                this,
                "//stage GL_VERTEX_SHADER\n"
                "//import <GLUPES/points_vertex_shader.h>\n",
                "//stage GL_FRAGMENT_SHADER\n"
                "//import <GLUPES/points_fragment_shader.h>\n"                
            )
        );
    }

    void Context_ES2::setup_GLUP_LINES() {
        set_primitive_info(
            GLUP_LINES, GL_LINES,
            GLSL::compile_program_with_includes_no_link(
                this,
                "//stage GL_VERTEX_SHADER\n"
                "//import <GLUPES/vertex_shader.h>\n",
                "//stage GL_FRAGMENT_SHADER\n"
                "//import <GLUPES/lines_fragment_shader.h>\n"                
            )
        );
    }

    void Context_ES2::setup_primitive_generic(
        index_t nb_elements_per_primitive,
        index_t* element_indices
    ) {

        set_primitive_info_immediate_index_mode(
            primitive_source_, GL_TRIANGLES,
            GLSL::compile_program_with_includes_no_link(
                this,
                "//stage GL_VERTEX_SHADER\n"
                "//import <GLUPES/vertex_shader.h>\n",
                "//stage GL_FRAGMENT_SHADER\n"
                "//import <GLUPES/fragment_shader.h>\n"                
            ),
            nb_elements_per_primitive, element_indices
        );
    }
    
    void Context_ES2::setup_GLUP_TRIANGLES() {
        static index_t element_indices[3]  = {
            0, 1, 2
        };
        setup_primitive_generic(
            index_t(sizeof(element_indices)/sizeof(index_t)),
            element_indices
        );
    }

    void Context_ES2::setup_GLUP_QUADS() {
        static index_t element_indices[6]  = {
            0, 1, 2,
            0, 2, 3
        };
        setup_primitive_generic(
            index_t(sizeof(element_indices)/sizeof(index_t)),
            element_indices
        );
    }

    void Context_ES2::setup_GLUP_TETRAHEDRA() {
        static index_t element_indices[12] = {
            1,3,2,
            0,2,3,
            3,1,0,
            0,1,2
        };
        setup_primitive_generic(
            index_t(sizeof(element_indices)/sizeof(index_t)),
            element_indices
        );
    }

    void Context_ES2::setup_GLUP_HEXAHEDRA() {
        static index_t element_indices[36] = {
            0,2,6, 0,6,4,
            3,1,5, 3,5,7,
            1,0,4, 1,4,5,
            2,3,7, 2,7,6,
            1,3,2, 1,2,0,
            4,6,7, 4,7,5
        };
        setup_primitive_generic(
            index_t(sizeof(element_indices)/sizeof(index_t)),
            element_indices
        );
    }

    void Context_ES2::setup_GLUP_PRISMS() {
        static index_t element_indices[24] = {
            0,1,2,
            3,5,4,
            0,3,4, 0,4,1,
            0,2,5, 0,5,3,
            1,4,5, 1,5,2
        };
        setup_primitive_generic(
            index_t(sizeof(element_indices)/sizeof(index_t)),
            element_indices
        );
    }

    void Context_ES2::setup_GLUP_PYRAMIDS() {
        static index_t element_indices[18] = {
            0,1,2, 0,2,3,
            0,4,1,
            0,3,4,
            2,4,3,
            2,1,4
        };
        setup_primitive_generic(
            index_t(sizeof(element_indices)/sizeof(index_t)),
            element_indices
        );
    }

    void Context_ES2::setup_GLUP_CONNECTORS() {
        static index_t element_indices[12] = {
            0,1,2, 0,2,3,
            2,1,0,
            3,2,0
        };
        setup_primitive_generic(
            index_t(sizeof(element_indices)/sizeof(index_t)),
            element_indices
        );
    }

    void Context_ES2::setup_GLUP_SPHERES() {
        set_primitive_info(
            GLUP_SPHERES, GL_POINTS,
            GLSL::compile_program_with_includes_no_link(
                this,
                "//stage GL_VERTEX_SHADER\n"
                "//import <GLUPES/spheres_vertex_shader.h>\n",
                "//stage GL_FRAGMENT_SHADER\n"
                "//import <GLUPES/spheres_fragment_shader.h>\n"                
            )
        );
    }
    
    bool Context_ES2::cell_by_cell_clipping() const {
        if(!uniform_state_.toggle[GLUP_CLIPPING].get()) {
            return false;
        }

        if(
            immediate_state_.primitive() != GLUP_TETRAHEDRA &&
            immediate_state_.primitive() != GLUP_HEXAHEDRA &&
            immediate_state_.primitive() != GLUP_PRISMS &&
            immediate_state_.primitive() != GLUP_PYRAMIDS &&
            immediate_state_.primitive() != GLUP_CONNECTORS 
        ) {
            return false;
        }
        
        return (
            uniform_state_.clipping_mode.get() == GLUP_CLIP_WHOLE_CELLS ||
            uniform_state_.clipping_mode.get() == GLUP_CLIP_STRADDLING_CELLS
        );
    }

    bool Context_ES2::sliced_cells_clipping() const {
        if(!uniform_state_.toggle[GLUP_CLIPPING].get()) {
            return false;
        }

        if(
            immediate_state_.primitive() != GLUP_TETRAHEDRA &&
            immediate_state_.primitive() != GLUP_HEXAHEDRA &&
            immediate_state_.primitive() != GLUP_PRISMS &&
            immediate_state_.primitive() != GLUP_PYRAMIDS &&
            immediate_state_.primitive() != GLUP_CONNECTORS 
        ) {
            return false;
        }
        
        return (
            uniform_state_.clipping_mode.get() == GLUP_CLIP_SLICE_CELLS 
        );
    }
    
    void Context_ES2::flush_immediate_buffers_with_cell_by_cell_clipping() {
        index_t cur_vertex = 0;
        index_t cur_element_out = 0;
        index_t nb_vertices_per_cell =
            nb_vertices_per_primitive[immediate_state_.primitive()];
        index_t nb_elements_per_cell = primitive_info_[
            immediate_state_.primitive()].nb_elements_per_primitive;

        // Assemble the list of indices (elements) that correspond
        // to the vertices of the visible cells.
        while(cur_vertex < immediate_state_.nb_vertices()) {
            if(!cell_is_clipped(cur_vertex)) {
                for(index_t lei=0; lei < nb_elements_per_cell; ++lei) {
                    geo_debug_assert(
                        cur_element_out < nb_clip_cells_elements_
                    );
                    clip_cells_elements_[cur_element_out] =
                        Numeric::uint16(
                            cur_vertex + primitive_info_[
                                immediate_state_.primitive()
                            ].primitive_elements[lei]
                        );
                    ++cur_element_out;
                }
            }
            cur_vertex += nb_vertices_per_cell;
        }

        glupBindVertexArray(clip_cells_VAO_);

        // Stream the values in all bound buffers (and attach
        // them to the VAO).
        for(index_t i=0; i<ImmediateState::NB_IMMEDIATE_BUFFERS; ++i) {
            if(immediate_state_.buffer[i].is_enabled()) {
                glEnableVertexAttribArray(i);
                if(immediate_state_.buffer[i].VBO() != 0) {
                    stream_buffer_object(
                        immediate_state_.buffer[i].VBO(),
                        GL_ARRAY_BUFFER,
                        immediate_state_.buffer[i].size_in_bytes(),
                        immediate_state_.buffer[i].data()
                    );
                }
            }
        }

        if(
            vertex_id_VBO_ != 0 && ((
                immediate_state_.primitive() >= GLUP_TRIANGLES &&
                uniform_state_.toggle[GLUP_DRAW_MESH].get()
            ) || uniform_state_.toggle[GLUP_PICKING].get())
        ) {
	    if(!vertex_id_VBO_bound_) {
		glEnableVertexAttribArray(GLUP_VERTEX_ID_ATTRIBUTE);
		glBindBuffer(
		    GL_ARRAY_BUFFER, vertex_id_VBO_
		);
		glVertexAttribPointer(
		    GLUP_VERTEX_ID_ATTRIBUTE,
		    1,                 // 1 component per attribute
		    GL_UNSIGNED_SHORT, // components are 16 bits integers
		    GL_FALSE,          // do not normalize
		    0,                 // stride
		    nullptr            // pointer (relative to bound VBO beginning)
		);
		vertex_id_VBO_bound_ = true;
	    }
        } else {
	    if(vertex_id_VBO_bound_) {
		vertex_id_VBO_bound_ = false;
		glDisableVertexAttribArray(GLUP_VERTEX_ID_ATTRIBUTE);
	    }
        }
        
        // Stream the indices into the elements VBO.
        stream_buffer_object(
            clip_cells_elements_VBO_,
            GL_ELEMENT_ARRAY_BUFFER,
            cur_element_out * sizeof(Numeric::uint16),
            clip_cells_elements_
        );
        
        glDrawElements(
            primitive_info_[immediate_state_.primitive()].GL_primitive,
            GLsizei(cur_element_out),
            GL_UNSIGNED_SHORT,
            nullptr
        );
        
        glupBindVertexArray(0);
        glDisableVertexAttribArray(GLUP_VERTEX_ID_ATTRIBUTE);
        immediate_state_.reset();
    }

    void Context_ES2::flush_immediate_buffers_with_sliced_cells_clipping() {
        
        MarchingCell* marching_cell = nullptr;
        switch(immediate_state_.primitive()) {
        case GLUP_TETRAHEDRA:
            marching_cell = &marching_tet_;
            break;
        case GLUP_HEXAHEDRA:
            marching_cell = &marching_hex_;
            break;
        case GLUP_PRISMS:
            marching_cell = &marching_prism_;
            break;
        case GLUP_PYRAMIDS:
            marching_cell = &marching_pyramid_;
            break;
        case GLUP_CONNECTORS:
            marching_cell = &marching_connector_;
            break;
        case GLUP_POINTS:
        case GLUP_LINES:
        case GLUP_TRIANGLES:
        case GLUP_QUADS:
	case GLUP_SPHERES:
        case GLUP_NB_PRIMITIVES:
            geo_assert_not_reached;
        }

        glupBindVertexArray(sliced_cells_VAO_);

        index_t v0=0;
        while(v0 < immediate_state_.nb_vertices()) {
            index_t config = get_config(v0, marching_cell->nb_vertices());

            //   Compute all the intersection vertices (plus their
            // attributes if enabled).
            for(index_t i=0; i<marching_cell->config_size(config); ++i) {
                index_t e = marching_cell->config_edges(config)[i];
                compute_intersection(
                    v0+marching_cell->edge_vertex(e,0),
                    v0+marching_cell->edge_vertex(e,1),
                    e
                );
            }

            if(marching_cell->config_size(config) != 0) {
                
                stream_buffer_object(
                    sliced_cells_elements_VBO_,
                    GL_ELEMENT_ARRAY_BUFFER,
                    marching_cell->max_config_size() * sizeof(Numeric::uint32),
                    marching_cell->config_edges(config)
                );

                stream_buffer_object(
                    sliced_cells_vertex_attrib_VBO_[0],
                    GL_ARRAY_BUFFER,
                    12 * sizeof(Numeric::float32) * 4,
                    &isect_vertex_attribute_[0][0]
                );

                for(index_t i=1; i<4; ++i) {
                    if(immediate_state_.buffer[i].is_enabled()) {
                        glEnableVertexAttribArray(i);
                        stream_buffer_object(
                            sliced_cells_vertex_attrib_VBO_[i],
                            GL_ARRAY_BUFFER,
                            12 * sizeof(Numeric::float32) * 4,
                            &isect_vertex_attribute_[i][0]
                        );
                    } else {
                        glDisableVertexAttribArray(i);
                    }
                }

                glDrawElements(
                    GL_TRIANGLE_FAN,
                    GLsizei(marching_cell->config_size(config)), 
                    GL_UNSIGNED_INT,
                    nullptr
                );
            }
            
            v0 += marching_cell->nb_vertices();
        }
        
        glupBindVertexArray(0);        
        immediate_state_.reset();
    }
    
    void Context_ES2::flush_immediate_buffers() {
        classify_vertices_in_immediate_buffers();        
        shrink_cells_in_immediate_buffers();
        if(cell_by_cell_clipping()) {
            flush_immediate_buffers_with_cell_by_cell_clipping();            
        } else if(sliced_cells_clipping()) {
            flush_immediate_buffers_with_sliced_cells_clipping();
        } else {
            Context::flush_immediate_buffers();            
        }
    }


    void Context_ES2::get_primitive_pseudo_file(
        std::vector<GLSL::Source>& sources
    ) {
        Context::get_primitive_pseudo_file(sources);
        
        static GLUPfloat tri_tex_coords[12] = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f            
        };

        static GLUPfloat quad_tex_coords[16] = {
            0.0f, 0.0f, 1.0f, 1.0f,
            1.0f, 0.0f, 0.0f, 1.0f,
            1.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 1.0f, 0.0f,            
        };
        
        static GLUPfloat tet_tex_coords[16] = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f            
        };

        static GLUPfloat hex_tex_coords[32] = {
            0.0f, 0.0f, 0.0f, 0.0f,            
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 1.0f, 0.0f,
            1.0f, 0.0f, 0.0f, 0.0f,            
            1.0f, 0.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 0.0f,
        };

        static GLUPfloat prism_tex_coords[24] = {
            1.0f, 0.0f, 0.0f, 0.0f,            
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            1.0f, 0.0f, 0.0f, 1.0f,            
            0.0f, 1.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f, 1.0f
        };

        
        switch(primitive_source_) {
        case GLUP_TRIANGLES:
            sources.push_back(mesh_tex_coord_lookup(tri_tex_coords));
            break;
        case GLUP_QUADS:
            sources.push_back(mesh_tex_coord_lookup(quad_tex_coords));
            break;
        case GLUP_TETRAHEDRA:
            sources.push_back(mesh_tex_coord_lookup(tet_tex_coords));
            break;
        case GLUP_HEXAHEDRA:
            sources.push_back(mesh_tex_coord_lookup(hex_tex_coords));    
            break;
        case GLUP_PRISMS:
            sources.push_back(mesh_tex_coord_lookup(prism_tex_coords));
            break;
            
        case GLUP_POINTS:
        case GLUP_LINES:
        case GLUP_PYRAMIDS:
        case GLUP_CONNECTORS:
	case GLUP_SPHERES:
            sources.push_back(
                "#define GLUP_NO_MESH_TEX_COORDS\n"
                "vec4 get_mesh_tex_coord(in int vertex_id) {\n"
                "   return vec4(0.0, 0.0, 0.0, 0.0);\n"
                "}\n"
            );
            break;

        case GLUP_NB_PRIMITIVES:
            geo_assert_not_reached;
        }
    }

    void Context_ES2::get_vertex_shader_preamble_pseudo_file(
        std::vector<GLSL::Source>& sources
    ) {
        if(use_ES_profile_) {
	    if(GLSL_version_ >= 3.0) {
		sources.push_back(
		    "#version 300 es\n"
		    "#define GLUP_VERTEX_SHADER \n"
		);
	    } else {
		sources.push_back(
		    "#version 100\n"
		    "#define GLUP_VERTEX_SHADER \n"
		);
	    }
        } else {
            sources.push_back(
#ifdef GEO_OS_APPLE		
		"#version 150 core          \n"
#else		
                "#version 130               \n"
#endif		
                "#define GLUP_VERTEX_SHADER \n"
            );
        }
    }
    

    void Context_ES2::get_fragment_shader_preamble_pseudo_file(
        std::vector<GLSL::Source>& sources
    ) {
        if(use_ES_profile_) {
#if defined(GEO_OS_EMSCRIPTEN) || defined(GEO_OS_ANDROID)
	    if(GLSL_version_ >= 3.0) {
		sources.push_back("#version 300 es\n");
	    } else {
		sources.push_back("#version 100\n");		
	    }
#endif	    
	    sources.push_back(
                "#define GLUP_FRAGMENT_SHADER                    \n"
                "#extension GL_OES_standard_derivatives : enable \n"
                "#extension GL_EXT_frag_depth : enable           \n"
		"#extension GL_OES_texture3D : enable            \n"		
		"#ifndef GL_OES_texture3D                        \n"
		"   #define GLUP_NO_TEXTURE_3D                   \n"		
		"#endif                                          \n"
                "precision highp float;                          \n"
                "#ifdef GL_FRAGMENT_PRECISION_HIGH               \n"
                "   precision highp int;                         \n"
                "#endif                                          \n"
            );
        } else {
            sources.push_back(
#ifdef GEO_OS_APPLE		
		"#version 150 core                               \n"
#else		
                "#version 130                                    \n"
#endif		
                "#define GLUP_FRAGMENT_SHADER                    \n"
            );
        }
    }

    void Context_ES2::get_toggles_pseudo_file(
        std::vector<GLSL::Source>& sources        
    ) {
        
        //   In GLUPES2, the state is splitted into vertex shader
        // and fragment shader state. The toggles are in the fragment
        // shader state, therefore the vertex shader can only know the
        // statically defined toggles. For the other ones, we
        // "conservatively" assume that they are on (on the worst case,
        // this means the vertex shader will copy more attributes than
        // needed).
        
        std::string toggle_maybe_enabled_source = 
            "bool glupIsEnabled(in int toggle) {\n";

        for(index_t i=0; i<uniform_state_.toggle.size(); ++i) {
            std::string toggle_var_name = uniform_state_.toggle[i].name();
            std::string toggle_name = toggle_var_name;
            size_t pos = toggle_name.find("_enabled");
            geo_assert(pos != std::string::npos);
            toggle_name = "GLUP_" +
                String::to_uppercase(toggle_name.substr(0,pos));
            
            std::string test =
                "   if(toggle=="+toggle_name + ") {\n";

            toggle_maybe_enabled_source += test;

            if(toggles_source_undetermined_ & (1 << i)) {
                toggle_maybe_enabled_source += "      return true;\n";
            } else {
                if(toggles_source_state_ & (1 << i)) {
                    toggle_maybe_enabled_source +=
                        "      return true;\n";                        
                } else {
                    toggle_maybe_enabled_source +=
                        "      return false;\n";                        
                }
            }
            toggle_maybe_enabled_source += "   }\n";            
        }

        toggle_maybe_enabled_source += "   return false; \n}\n";

        sources.push_back("#ifdef GLUP_VERTEX_SHADER\n");
        sources.push_back(toggle_maybe_enabled_source);
        sources.push_back("#else\n");
        Context::get_toggles_pseudo_file(sources);
        sources.push_back("#endif\n");
    }
}

#endif

