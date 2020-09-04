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

#include <geogram_gfx/GLUP/GLUP_marching_cells.h>
#include <geogram_gfx/basic/GLSL.h>
#include <geogram/basic/string.h>

#ifdef GEO_GL_150
namespace {
    using namespace GLUP;
    /**
     * \brief Sets an entry in an array with specified stride.
     * \param[in] array the address of the first element of the array
     * \param[in] stride number of bytes between two consecutive elements
     *  of the array
     * \param[in] i index of the element to be set
     * \param[in] value value of the element ot be set
     */
    template <class T> inline void set_array_item(
	void* array, size_t stride, index_t i, T value
    ) {
	*reinterpret_cast<T*>(Memory::pointer(array) + (i * stride)) = value;
    }
}
#endif


namespace GLUP {
    using namespace GEO;

    /*******************************************************************/
    
    MarchingCell::MarchingCell(GLUPprimitive prim) {
        UBO_ = 0;
        elements_VBO_ = 0;
        desc_ = nullptr;
        switch(prim) {
        case GLUP_TETRAHEDRA:
            desc_ =
                &MeshCellsStore::cell_type_to_cell_descriptor(MESH_TET);
            uniform_binding_point_ = 2;
            break;
        case GLUP_HEXAHEDRA:
            desc_ =
                &MeshCellsStore::cell_type_to_cell_descriptor(MESH_HEX);
            uniform_binding_point_ = 3;            
            break;
        case GLUP_PRISMS:
            desc_ =
                &MeshCellsStore::cell_type_to_cell_descriptor(MESH_PRISM);
            uniform_binding_point_ = 4;                        
            break;
        case GLUP_PYRAMIDS:
            desc_ =
                &MeshCellsStore::cell_type_to_cell_descriptor(MESH_PYRAMID);
            uniform_binding_point_ = 5;                        
            break;
        case GLUP_CONNECTORS:
            desc_ =
                &MeshCellsStore::cell_type_to_cell_descriptor(MESH_CONNECTOR);
            uniform_binding_point_ = 6;                        
            break;
        case GLUP_POINTS:
        case GLUP_LINES:
        case GLUP_TRIANGLES:
        case GLUP_QUADS:
        case GLUP_SPHERES:	    
        case GLUP_NB_PRIMITIVES:
            geo_assert_not_reached;
        }
            
        index_t nb_v = desc_->nb_vertices;
        
        for(index_t i=0; i<64; ++i) {
            vv_to_e_[i] = index_t(-1);
        }
            
        for(index_t e=0; e<desc_->nb_edges; ++e) {
            index_t v1 = desc_->edge_vertex[e][0];
            index_t v2 = desc_->edge_vertex[e][1];
            vv_to_e_[v1*nb_v+v2] = e;
            vv_to_e_[v2*nb_v+v1] = e;                
        }
            
        nb_vertices_ = desc_->nb_vertices;
        nb_configs_ = 1u << nb_vertices_;
        nb_edges_ = desc_->nb_edges;
        edge_ = new index_t[nb_edges_*2];
        config_ = new index_t[nb_configs_*nb_edges_];
        config_size_ = new index_t[nb_configs_];
        
        for(index_t e=0; e<nb_edges_; ++e) {
            edge_[2*e] = desc_->edge_vertex[e][0];
            edge_[2*e+1] = desc_->edge_vertex[e][1];                
        }

        max_config_size_=0;
        for(index_t config=0; config<nb_configs_; ++config) {
            compute_config(config);
            // It only happens for connectors. 
            if(config_size_[config] < 3) {
                config_size_[config] = 0;
            }
            max_config_size_=std::max(max_config_size_,config_size(config));
        }

        GLSL_uniform_state_declaration_ = std::string() +
        "  const int cell_nb_vertices = "        +
            String::to_string(nb_vertices())     + ";\n"
        "  const int cell_nb_edges = "           +
            String::to_string(nb_edges())        + ";\n"
        "  const int cell_nb_configs = "         +
            String::to_string(nb_configs())      + ";\n"
        "  const int cell_max_config_size = "    +
            String::to_string(max_config_size()) + ";\n" +
        "  layout(shared)                                          \n" 
        "  uniform MarchingCellStateBlock {                        \n" 
        "     int config_size[cell_nb_configs];                    \n" 
        "     int config[cell_nb_configs*cell_max_config_size];    \n"  
        "  } MarchingCell;                                         \n"
        "  int config_size(in int i) {                             \n"
        "    return MarchingCell.config_size[i];                   \n"
        "  }                                                       \n"
        "  int config_edge(in int i, in int j) {                   \n"
        "    return MarchingCell.config[i*cell_max_config_size+j]; \n"
        "  }                                                       \n"
        ;

        GLSL_compute_intersections_ = std::string() +
        "  vec4 isect_point_clip_space[cell_nb_edges];        \n"
        "  vec4 isect_color[cell_nb_edges];                   \n"
        "  vec4 isect_tex_coord[cell_nb_edges];               \n"
        "  void compute_intersection(in int i, in int v1, in int v2) { \n"
        "      vec4 p1 = vertex_clip_space_in(v1);            \n"
        "      vec4 p2 = vertex_clip_space_in(v2);            \n"
        "      float t = -dot(p1, GLUP.clip_clip_plane);      \n"
        "      float d = dot(p2-p1, GLUP.clip_clip_plane);    \n"
        "      if(abs(d) < 1e-6) { t = 0.5; } else { t /= d; }\n"
        "      isect_point_clip_space[i] = mix(p1,p2,t);      \n"
        "      if(glupIsEnabled(GLUP_VERTEX_COLORS)) {        \n"
        "         isect_color[i] = mix(                       \n"
        "             color_in(v1), color_in(v2), t           \n"
        "         );                                          \n"    
        "      }                                              \n"
        "      if(glupIsEnabled(GLUP_TEXTURING)) {            \n"
        "         isect_tex_coord[i] = mix(                   \n"
        "             tex_coord_in(v1), tex_coord_in(v2), t   \n"
        "         );                                          \n"    
        "      }                                              \n"    
        "  }                                                  \n"    
        "  void compute_intersections() {                     \n"
        ;

            
        for(index_t e=0; e<nb_edges(); ++e) {
            GLSL_compute_intersections_ +=
                "   compute_intersection(" +
                String::to_string(e) + "," +
                String::to_string(edge_vertex(e,0)) + "," +
                String::to_string(edge_vertex(e,1)) + ");\n" ;
        }
        
        GLSL_compute_intersections_  += 
        "  }                                       \n"
        ;
    }

    MarchingCell::~MarchingCell() {
        delete[] edge_;
        delete[] config_;
        delete[] config_size_;
        if(UBO_ != 0) {
            glDeleteBuffers(1, &UBO_);
            UBO_ = 0;
        }
        if(elements_VBO_ != 0) {
            glDeleteBuffers(1, &elements_VBO_);
            elements_VBO_ = 0;
        }
    }

    void MarchingCell::compute_config(index_t config) {
        config_size_[config] = 0;
        index_t* config_out = config_ + config * nb_edges_;
        if(config_is_ambiguous(config)) {
            return;
        }
        index_t first_f = index_t(-1);
        index_t f = index_t(-1);
        index_t lv = index_t(-1);
        if(!get_first_edge(first_f,lv,config)) {
            return;
        }
        f = first_f;
        do {
            geo_debug_assert(config_size_[config] < nb_edges_);
            config_out[config_size_[config]] = edge(f,lv);
            ++config_size_[config];
            do {
                move_to_next(f,lv);
            } while(!edge_is_intersected(f,lv,config));
            move_to_opposite(f,lv);
        } while(f != first_f);
    }
        
    void MarchingCell::move_to_opposite(index_t& f, index_t& lv) {
        index_t v = destination_vertex(f, lv);
        index_t e = edge(f, lv);
        geo_debug_assert(
            desc_->edge_adjacent_facet[e][0] == f ||
            desc_->edge_adjacent_facet[e][1] == f
        );
        if(desc_->edge_adjacent_facet[e][0] == f) {
            f = desc_->edge_adjacent_facet[e][1];
        } else {
            f = desc_->edge_adjacent_facet[e][0];
        }
        for(lv = 0; lv < desc_->nb_vertices_in_facet[f]; ++lv) {
            if(desc_->facet_vertex[f][lv] == v) {
                return;
            }
        }
        geo_assert_not_reached;
    }

    bool MarchingCell::config_is_ambiguous(index_t config) {
        for(index_t f=0; f<desc_->nb_facets; ++f) {
            index_t nb_isect_in_f = 0;
            for(index_t lv=0; lv<desc_->nb_vertices_in_facet[f]; ++lv) {
                if(edge_is_intersected(f, lv, config)) {
                    ++nb_isect_in_f;
                }
            }
            if(nb_isect_in_f > 2) {
                return true;
            }
        }
        return false;
    }

    bool MarchingCell::get_first_edge(index_t& f, index_t& lv, index_t config) {
        for(f=0; f<desc_->nb_facets; ++f) {
            for(lv=0; lv<desc_->nb_vertices_in_facet[f]; ++lv) {
                if(edge_is_intersected(f, lv, config)) {
                    return true;
                }
            }
        }
        return false;
    }



    
    GLuint MarchingCell::create_UBO() {

#ifdef GEO_GL_150
        
        // Create a program that uses the UBO

#if defined(GEO_OS_ANDROID)
        static const char* shader_source_header_ =
            "#version 300 es\n"
	    "precision highp float;\n";
#elif defined(GEO_OS_APPLE)
        static const char* shader_source_header_ =
            "#version 150\n";
#else
        static const char* shader_source_header_ =
            "#version 150 core\n";
#endif
	
        // This program is stupid, it is only meant to make sure
        // that all variables in the UBO are used (else some
        // GLSL compilers optimize-it out and we can no-longer
        // query variable offsets from it)
        static const char* vertex_shader_source_ =
            "in vec3 position;                                \n"
            "void main() {                                    \n"
            "  gl_Position.x = float(MarchingCell.config_size[0]); \n"
            "  gl_Position.y = float(MarchingCell.config[0]); \n"
            "  gl_Position.z = float(MarchingCell.config[1]); \n"
            "  gl_Position.w = float(MarchingCell.config[1]); \n"
            "}                                                \n"
            ;
        
        static const char* fragment_shader_source_ =
            "out vec4 colorOut;                      \n"
            "void main() {                           \n"
            "   colorOut = vec4(1.0, 1.0, 1.0, 1.0); \n" 
            "}                                       \n";

        GLuint vertex_shader = GLSL::compile_shader(
            GL_VERTEX_SHADER,
            shader_source_header_,
            GLSL_uniform_state_declaration(),
            vertex_shader_source_,
            nullptr
        );

        GLuint fragment_shader = GLSL::compile_shader(
            GL_FRAGMENT_SHADER,
            shader_source_header_,
            fragment_shader_source_,
            nullptr
        );

        GLuint program = GLSL::create_program_from_shaders(
            vertex_shader,
            fragment_shader,
            nullptr
        );

        // Get UBO size and offsets

        GLuint UBO_index =
            glGetUniformBlockIndex(program, "MarchingCellStateBlock");
        
        if(UBO_index == GL_INVALID_INDEX) {
            Logger::err("GLUP")
                << "MarchingCellsStateBlock"
                << ":did not find uniform state variable"
                << std::endl;
            throw GLSL::GLSLCompileError();
        }
        
        glUniformBlockBinding(
            program, UBO_index, uniform_binding_point_
        );

        GLint uniform_buffer_size;
        
        glGetActiveUniformBlockiv(
            program, UBO_index,
            GL_UNIFORM_BLOCK_DATA_SIZE,
            &uniform_buffer_size
        );

	
        // Create UBO
        
        Memory::byte* UBO_data = new Memory::byte[uniform_buffer_size];
        Memory::clear(UBO_data, size_t(uniform_buffer_size));

        glGenBuffers(1, &UBO_);
        glBindBuffer(GL_UNIFORM_BUFFER, UBO_);

        glBindBufferBase(
            GL_UNIFORM_BUFFER,
            uniform_binding_point_,
            UBO_
        );
        
        
        // Get variable offsets and array strides
        
        GLint config_size_offset = GLSL::get_uniform_variable_offset(
            program, "MarchingCellStateBlock.config_size[0]"
	);

        GLint config_offset = GLSL::get_uniform_variable_offset(
            program, "MarchingCellStateBlock.config[0]"
        );

	// Note: array strides may differ from one OpenGL vendor to another,
	// for instance, for an array of ints,
	//   with NVidia, stride = 4
	//   with Intel,  stride = 16
	// (by quiering, the following code works on both).
	
	size_t config_size_stride = GLSL::get_uniform_variable_array_stride(
	    program, "MarchingCellStateBlock.config_size[0]"
	);

	size_t config_stride = GLSL::get_uniform_variable_array_stride(
	    program, "MarchingCellStateBlock.config[0]"
	);

	void* config_size_ptr = (UBO_data + config_size_offset);
	void* config_ptr = (UBO_data + config_offset);

        for(index_t i=0; i<nb_configs(); ++i) {
	    set_array_item(config_size_ptr, config_size_stride, i, config_size(i));
            for(index_t j=0; j<config_size(i); ++j) {
		set_array_item(config_ptr, config_stride, i*max_config_size()+j, config_edges(i)[j]);
            }
        }

        glBufferData(
            GL_UNIFORM_BUFFER,
            uniform_buffer_size,
            UBO_data,
            GL_STATIC_DRAW
        );

        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        // Delete temporary UBO data
        delete[] UBO_data;
        
        // Delete program and shaders
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        glDeleteProgram(program);


#endif                
        return UBO_;
    }

    GLuint MarchingCell::create_elements_VBO() {
        glGenBuffers(1, &elements_VBO_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elements_VBO_);
        index_t nb_elements = nb_configs() * max_config_size();
        Numeric::uint8* indices = new Numeric::uint8[nb_elements];
        for(index_t i=0; i<nb_configs(); ++i) {
            for(index_t j=0; j<config_size(i); ++j) {
                indices[i*max_config_size()+j] =
                    Numeric::uint8(config_edges(i)[j]);
            }
        }
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            GLsizeiptr(sizeof(Numeric::uint8) * nb_elements),
            indices, GL_STATIC_DRAW
        );
        delete[] indices;
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);        
        return elements_VBO_;
    }
    
    void MarchingCell::bind_uniform_state(GLuint program) {
#ifndef GEO_GL_150
        geo_argused(program);
#else
        GLuint UBO_index = glGetUniformBlockIndex(
            program, "MarchingCellStateBlock"
        );
        if(UBO_index != GL_INVALID_INDEX) {
            glUniformBlockBinding(
                program, UBO_index, uniform_binding_point_
            );
        } else {
            Logger::warn("GLUP")
                << "MarchingCellStateBlock not found" << std::endl;
        }
#endif    
    }
    
}
