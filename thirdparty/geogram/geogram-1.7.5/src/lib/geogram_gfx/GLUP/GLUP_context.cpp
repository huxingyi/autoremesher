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

#include <geogram_gfx/GLUP/GLUP_context.h>
#include <geogram_gfx/GLUP/shaders/embedded_shaders.h>
#include <geogram_gfx/basic/GLSL.h>

#include <geogram/basic/command_line.h>
#include <geogram/basic/progress.h>
#include <geogram/basic/logger.h>

#include <geogram/bibliography/bibliography.h>

#ifdef GEO_OS_ANDROID
#  pragma GCC diagnostic ignored "-Wpointer-bool-conversion"
#  pragma GCC diagnostic ignored "-Wtautological-pointer-compare"
#endif

// TODO: for points and spheres: early fragment tests, depth greater
// (+ vertex shader that "drags the point to the camera" in such a way
//  that the frag shader "pushes to the back" and we can have early depth
//  cull (this can probably significantly accelerate rendering when there is
//  a large number of big spheres/points).
//      layout(early_fragment_tests) in;
//       layout(depth_greater) out float gl_FragDepth;
// -----------------------------------------------------------------------------

// TODO: in OpenGL, matrix storage is row major (oh my)... Most of my functions
//  that manipulate matrices need to be renamed/rewritten, in the current state
//  my code is *VERY CONFUSING* !!!
// This concerns:
//    -all the functions that create matrices (for now I transpose right after).
//    -glupUnProject()
//------------------------------------------------------------------------------


namespace {
    using namespace GEO;

    
    void vertex_shader_preamble_pseudo_file(
	GLSL::PseudoFileProvider* provider, std::vector<GLSL::Source>& sources
    ) {
	GLUP::Context* ctxt = dynamic_cast<GLUP::Context*>(provider);
	geo_assert(ctxt != nullptr);
	ctxt->get_vertex_shader_preamble_pseudo_file(sources);
	sources.push_back("#define ");
	sources.push_back(ctxt->profile_name());
	sources.push_back("\n");
    }

    void fragment_shader_preamble_pseudo_file(
	GLSL::PseudoFileProvider* provider, std::vector<GLSL::Source>& sources
    ) {
	GLUP::Context* ctxt = dynamic_cast<GLUP::Context*>(provider);
	geo_assert(ctxt != nullptr);
	ctxt->get_fragment_shader_preamble_pseudo_file(sources);
	sources.push_back("#define ");
	sources.push_back(ctxt->profile_name());
	sources.push_back("\n");
    }

    void geometry_shader_preamble_pseudo_file(
	GLSL::PseudoFileProvider* provider, std::vector<GLSL::Source>& sources
    ) {
	GLUP::Context* ctxt = dynamic_cast<GLUP::Context*>(provider);
	geo_assert(ctxt != nullptr);
	ctxt->get_geometry_shader_preamble_pseudo_file(sources);
	sources.push_back("#define ");
	sources.push_back(ctxt->profile_name());
	sources.push_back("\n");
    }

    void marching_cells_pseudo_file(
	GLSL::PseudoFileProvider* provider, std::vector<GLSL::Source>& sources
    ) {
	GLUP::Context* ctxt = dynamic_cast<GLUP::Context*>(provider);
	geo_assert(ctxt != nullptr);
	ctxt->get_marching_cells_pseudo_file(sources);
	sources.push_back("#define ");
	sources.push_back(ctxt->profile_name());
	sources.push_back("\n");
    }

    void tess_control_shader_preamble_pseudo_file(
	GLSL::PseudoFileProvider* provider, std::vector<GLSL::Source>& sources
    ) {
	GLUP::Context* ctxt = dynamic_cast<GLUP::Context*>(provider);
	geo_assert(ctxt != nullptr);
	ctxt->get_tess_control_shader_preamble_pseudo_file(sources);
	sources.push_back("#define ");
	sources.push_back(ctxt->profile_name());
	sources.push_back("\n");
    }

    void tess_evaluation_shader_preamble_pseudo_file(
	GLSL::PseudoFileProvider* provider, std::vector<GLSL::Source>& sources
    ) {
	GLUP::Context* ctxt = dynamic_cast<GLUP::Context*>(provider);
	geo_assert(ctxt != nullptr);
	ctxt->get_tess_evaluation_shader_preamble_pseudo_file(sources);
	sources.push_back("#define ");
	sources.push_back(ctxt->profile_name());
	sources.push_back("\n");
    }

    void toggles_pseudo_file(
	GLSL::PseudoFileProvider* provider, std::vector<GLSL::Source>& sources
    ) {
	GLUP::Context* ctxt = dynamic_cast<GLUP::Context*>(provider);
	geo_assert(ctxt != nullptr);
	ctxt->get_toggles_pseudo_file(sources);
    }

    void primitive_pseudo_file(
	GLSL::PseudoFileProvider* provider, std::vector<GLSL::Source>& sources
    ) {
	GLUP::Context* ctxt = dynamic_cast<GLUP::Context*>(provider);
	geo_assert(ctxt != nullptr);
	ctxt->get_primitive_pseudo_file(sources);
    }
}

namespace GLUP {
    using namespace GEO;
    
    /*************************************************************************/

    void show_matrix(const GLfloat M[16]) {
        for(index_t i=0; i<4; ++i) {
            for(index_t j=0; j<4; ++j) {
                std::cerr << M[4*i+j] << ' ';
            }
            std::cerr << std::endl;
        }
    }

    void show_vector(const GLfloat v[4]) {
        for(index_t i=0; i<4; ++i) {
            std::cerr << v[i] << ' ';
        }
        std::cerr << std::endl;        
    }
    
    // Used to determine buffer's maximum vertex index, that needs
    // to be an integer multiple of the number of vertices
    // per primitive.
    index_t nb_vertices_per_primitive[GLUP_NB_PRIMITIVES] = {
        1, // GLUP_POINTS     =0,
        2, // GLUP_LINES      =1,
        3, // GLUP_TRIANGLES  =2,
        4, // GLUP_QUADS      =3,
        4, // GLUP_TETRAHEDRA =4,
        8, // GLUP_HEXAHEDRA  =5,
        6, // GLUP_PRISMS     =6,
        5, // GLUP_PYRAMIDS   =7,
        4, // GLUP_CONNECTORS =8,
	1  // GLUP_SPHERES    =9
    };

    static index_t nb_vertices_per_GL_primitive(GLenum primitive) {
        index_t result = 0;
        switch(primitive) {
        case GL_POINTS:
            result = 1;
            break;
        case GL_LINES:
            result = 2;
            break;
        case GL_TRIANGLES:
            result = 3;
            break;
#ifdef GEO_GL_150
        case GL_LINES_ADJACENCY:
            result = 4;
            break;
        case GL_TRIANGLES_ADJACENCY:
            result = 6;
            break;
#endif            
        default:
            geo_assert_not_reached;
        }
        return result;
    }

    bool Context::extension_is_supported(const std::string& extension) {
#ifndef GEO_OS_EMSCRIPTEN
        // This is the new way of testing for an extension: first get
        // the number of extensions, then extension names one extension
        // at a time using glGetStringi
        if(use_core_profile_) {
            if(glGetStringi != nullptr) {
                GLuint num_ext;
                glGetIntegerv(GL_NUM_EXTENSIONS, (GLint*)&num_ext);
                if(num_ext != 0) {
                    for(GLuint i=0; i<num_ext; ++i) {
                        const GLubyte* cur_extension =
                            glGetStringi(GL_EXTENSIONS, i);
                        if(!strcmp(
                               extension.c_str(),
                               (const char*)cur_extension)
                        ) {
                            return true;
                        }
                    }
                }
            }
        }
#endif

        // If new way is unsupported or if using an old-style OpenGL
        // context, then we do it the old way.
        
        // It can happen for instance with OpenGL ES 2.0, that does not
        // support glGetStringi...
        
        const char* extensions = (const char*)(glGetString(GL_EXTENSIONS));
        if(extensions == nullptr) {
            return false;
        }
        return (strstr(extensions, extension.c_str()) != nullptr);
    }
    
    static const char* primitive_name[GLUP_NB_PRIMITIVES] = {
        "GLUP_POINTS",
        "GLUP_LINES",
        "GLUP_TRIANGLES",
        "GLUP_QUADS",
        "GLUP_TETRAHEDRA",
        "GLUP_HEXAHEDRA",
        "GLUP_PRISMS",
        "GLUP_PYRAMIDS",
        "GLUP_CONNECTORS",
	"GLUP_SPHERES"
    };

    const char* Context::glup_primitive_name(GLUPprimitive prim) {
        return primitive_name[prim];
    }
    
    static index_t primitive_dimension[GLUP_NB_PRIMITIVES] = {
        0, // GLUP_POINTS     =0,
        1, // GLUP_LINES      =1,
        2, // GLUP_TRIANGLES  =2,
        2, // GLUP_QUADS      =3,
        3, // GLUP_TETRAHEDRA =4,
        3, // GLUP_HEXAHEDRA  =5,
        3, // GLUP_PRISMS     =6,
        3, // GLUP_PYRAMIDS   =7,
        3, // GLUP_CONNECTORS =8,
	0  // GLUP_SPHERES    =9
    };


    /*
    ** Invert 4x4 matrix.
    ** Contributed by David Moore (See Mesa bug #6748)
    */
    template <class T> inline
    GLboolean invert_matrix_generic(T inv[16], const T m[16]) {
        
        inv[0]  = m[5]*m[10]*m[15] - m[5]*m[11]*m[14] - m[9]*m[6]*m[15]
                + m[9]*m[7]*m[14] + m[13]*m[6]*m[11] - m[13]*m[7]*m[10];
        inv[4]  = -m[4]*m[10]*m[15] + m[4]*m[11]*m[14] + m[8]*m[6]*m[15]
                - m[8]*m[7]*m[14] - m[12]*m[6]*m[11] + m[12]*m[7]*m[10];
        inv[8]  = m[4]*m[9]*m[15] - m[4]*m[11]*m[13] - m[8]*m[5]*m[15]
                + m[8]*m[7]*m[13] + m[12]*m[5]*m[11] - m[12]*m[7]*m[9];
        inv[12] = -m[4]*m[9]*m[14] + m[4]*m[10]*m[13] + m[8]*m[5]*m[14]
                - m[8]*m[6]*m[13] - m[12]*m[5]*m[10] + m[12]*m[6]*m[9];
        inv[1]  = -m[1]*m[10]*m[15] + m[1]*m[11]*m[14] + m[9]*m[2]*m[15]
                - m[9]*m[3]*m[14] - m[13]*m[2]*m[11] + m[13]*m[3]*m[10];
        inv[5]  = m[0]*m[10]*m[15] - m[0]*m[11]*m[14] - m[8]*m[2]*m[15]
                + m[8]*m[3]*m[14] + m[12]*m[2]*m[11] - m[12]*m[3]*m[10];
        inv[9]  = -m[0]*m[9]*m[15] + m[0]*m[11]*m[13] + m[8]*m[1]*m[15]
                - m[8]*m[3]*m[13] - m[12]*m[1]*m[11] + m[12]*m[3]*m[9];
        inv[13] = m[0]*m[9]*m[14] - m[0]*m[10]*m[13] - m[8]*m[1]*m[14]
                + m[8]*m[2]*m[13] + m[12]*m[1]*m[10] - m[12]*m[2]*m[9];
        inv[2]  = m[1]*m[6]*m[15] - m[1]*m[7]*m[14] - m[5]*m[2]*m[15]
                + m[5]*m[3]*m[14] + m[13]*m[2]*m[7] - m[13]*m[3]*m[6];
        inv[6]  = -m[0]*m[6]*m[15] + m[0]*m[7]*m[14] + m[4]*m[2]*m[15]
                - m[4]*m[3]*m[14] - m[12]*m[2]*m[7] + m[12]*m[3]*m[6];
        inv[10] = m[0]*m[5]*m[15] - m[0]*m[7]*m[13] - m[4]*m[1]*m[15]
                + m[4]*m[3]*m[13] + m[12]*m[1]*m[7] - m[12]*m[3]*m[5];
        inv[14] = -m[0]*m[5]*m[14] + m[0]*m[6]*m[13] + m[4]*m[1]*m[14]
                - m[4]*m[2]*m[13] - m[12]*m[1]*m[6] + m[12]*m[2]*m[5];
        inv[3]  = -m[1]*m[6]*m[11] + m[1]*m[7]*m[10] + m[5]*m[2]*m[11]
                - m[5]*m[3]*m[10] - m[9]*m[2]*m[7] + m[9]*m[3]*m[6];
        inv[7]  = m[0]*m[6]*m[11] - m[0]*m[7]*m[10] - m[4]*m[2]*m[11]
                + m[4]*m[3]*m[10] + m[8]*m[2]*m[7] - m[8]*m[3]*m[6];
        inv[11] = -m[0]*m[5]*m[11] + m[0]*m[7]*m[9] + m[4]*m[1]*m[11]
                - m[4]*m[3]*m[9] - m[8]*m[1]*m[7] + m[8]*m[3]*m[5];
        inv[15] = m[0]*m[5]*m[10] - m[0]*m[6]*m[9] - m[4]*m[1]*m[10]
                + m[4]*m[2]*m[9] + m[8]*m[1]*m[6] - m[8]*m[2]*m[5];
        
        T det = m[0]*inv[0] + m[1]*inv[4] + m[2]*inv[8] + m[3]*inv[12];
        
        if (det == T(0.0)) {
            return GL_FALSE;
        }
        
        det = T(1.0) / det;
        
        for (index_t i = 0; i < 16; ++i) {
            inv[i] *= det;
        }

        return GL_TRUE;
    }

    GLboolean invert_matrix(GLfloat inv[16], const GLfloat m[16]) {
	return invert_matrix_generic(inv,m);
    }

    GLboolean invert_matrix(GLdouble inv[16], const GLdouble m[16]) {
	return invert_matrix_generic(inv,m);
    }
    
    void mult_matrices(
        GLfloat out[16], const GLfloat m1[16], const GLfloat m2[16]
    ) {
        Memory::clear(out, sizeof(GLfloat)*16);
        for(index_t i=0; i<4; ++i) {
            for(index_t j=0; j<4; ++j) {
                for(index_t k=0; k<4; ++k) {
                    out[i*4+j] += m1[i*4+k]*m2[k*4+j];
                }
            }
        }
    }

    void mult_matrices(
        GLdouble out[16], const GLdouble m1[16], const GLdouble m2[16]
    ) {
        Memory::clear(out, sizeof(GLdouble)*16);
        for(index_t i=0; i<4; ++i) {
            for(index_t j=0; j<4; ++j) {
                for(index_t k=0; k<4; ++k) {
                    out[i*4+j] += m1[i*4+k]*m2[k*4+j];
                }
            }
        }
    }
    
    void mult_matrix_vector(
        GLfloat out[4], const GLfloat m[16], const GLfloat v[4]
    ) {
        Memory::clear(out, sizeof(GLfloat)*4);
        for(index_t i=0; i<4; ++i) {
            for(index_t j=0; j<4; ++j) {
                out[i] += v[j] * m[4*i+j];
            }
        }
    }

    void mult_transpose_matrix_vector(
        GLfloat out[4], const GLfloat m[16], const GLfloat v[4]
    ) {
        Memory::clear(out, sizeof(GLfloat)*4);
        for(index_t i=0; i<4; ++i) {
            for(index_t j=0; j<4; ++j) {
                out[i] += v[j] * m[4*j+i];
            }
        }
    }

    void mult_transpose_matrix_vector(
        GLdouble out[4], const GLdouble m[16], const GLdouble v[4]
    ) {
        Memory::clear(out, sizeof(GLdouble)*4);
        for(index_t i=0; i<4; ++i) {
            for(index_t j=0; j<4; ++j) {
                out[i] += v[j] * m[4*j+i];
            }
        }
    }
    
    void mult_matrix_vector(
        GLdouble out[4], const GLdouble m[16], const GLdouble v[4]
    ) {
        Memory::clear(out, sizeof(GLdouble)*4);
        for(index_t i=0; i<4; ++i) {
            for(index_t j=0; j<4; ++j) {
                out[i] += v[j] * m[4*i+j];
            }
        }
    }

    void transpose_matrix(GLUPfloat M[16]) {
        for(int i=0; i<4; ++i) {
            for(int j=0; j<4; ++j) {
                if(i < j) {
                    GLUPfloat tmp = M[4*i+j];
                    M[4*i+j] = M[4*j+i];
                    M[4*j+i] = tmp;
                }
            }
        }
    }

    void transpose_matrix(GLUPdouble M[16]) {
        for(int i=0; i<4; ++i) {
            for(int j=0; j<4; ++j) {
                if(i < j) {
                    GLUPdouble tmp = M[4*i+j];
                    M[4*i+j] = M[4*j+i];
                    M[4*j+i] = tmp;
                }
            }
        }
    }
    
    void load_identity_matrix(GLfloat out[16]) {
        for(index_t i=0; i<4; ++i) {
            for(index_t j=0; j<4; ++j) {
                out[i*4+j] = (i == j) ? 1.0f : 0.0f;
            }
        }
    }
    
    /***********************************************************/
    
    void StateVariableBase::initialize(
        Context* context, const char* name
    ) {
        context_ = context;
        name_ = name;
        address_ = context_->get_state_variable_address(name);
    }

    void StateVariableBase::flag_uniform_buffer_as_dirty() {
        context_->flag_uniform_buffer_as_dirty();
    }

    /***********************************************************/

    const char* Context::uniform_state_declaration() {
        return GLSL::get_GLSL_include_file("GLUPGLSL/state.h");
    }

    
    Context::Context() :
        marching_tet_(GLUP_TETRAHEDRA),
        marching_hex_(GLUP_HEXAHEDRA),
        marching_prism_(GLUP_PRISMS),
        marching_pyramid_(GLUP_PYRAMIDS),
        marching_connector_(GLUP_CONNECTORS) {

	geo_cite("DBLP:conf/isvc/ToledoLP07");
	
        matrices_dirty_=true;        
        default_program_ = 0;
        uniform_buffer_=0;
        uniform_binding_point_=0;
        uniform_buffer_size_=0;
        uniform_buffer_data_=nullptr;
        uniform_buffer_dirty_=true;
	lighting_dirty_=true;
	
        matrix_mode_ = GLUP_MODELVIEW_MATRIX;
        matrices_dirty_ = true;

        precompile_shaders_ =
            CmdLine::get_arg_bool("gfx:GLUP_precompile_shaders");

        use_core_profile_ =
            (CmdLine::get_arg("gfx:GL_profile") != "compatibility");

        use_ES_profile_ =
            (CmdLine::get_arg("gfx:GL_profile") == "ES");            
        
        user_program_ = 0;
        world_clip_plane_ = nullptr;
        latest_program_ = 0;
        toggles_config_ = 0;

        vertex_id_VBO_ = 0;

        toggles_source_state_ = 0;
        toggles_source_undetermined_ = 0;

        initialize();
    }
    
    Context::~Context() {
        if(default_program_ != 0) {
            glDeleteProgram(default_program_);
        }
        glDeleteBuffers(1, &uniform_buffer_);
        delete[] uniform_buffer_data_;
        if(vertex_id_VBO_ != 0) {
            glDeleteBuffers(1, &vertex_id_VBO_);
            vertex_id_VBO_ = 0;
        }
    }

    bool Context::primitive_supports_array_mode(GLUPprimitive prim) const {
        return
            primitive_info_[prim].implemented &&
            primitive_info_[prim].VAO == 0;
    }

#ifdef GEO_GL_150
    /**
     * \brief Creates a GLSL vertex program that uses all the variables
     *  of the uniform state.
     * \details Some OpenGL drivers have a compiler that optimizes-out
     *  variables that are not used in the shader. This function creates
     *  a dummy shader that uses all the variables, so that their offsets
     *  can be reliably queried using GLSL program introspection
     *  with glGetUniformIndices().
     */
    static std::string create_vertex_program_that_uses_all_UBO_variables() {
        std::ostringstream output;

        output << "in vec3 position;\n";
        output << "void main() {\n";
        output << "   mat4 M = GLUP.modelviewprojection_matrix;\n";
        
        // Parse uniform state declaration in order to use all variables.
        
        std::istringstream input(Context::uniform_state_declaration());
        std::string line;
        while(std::getline(input,line)) {
            std::vector<std::string> words;
            GEO::String::split_string(line, ' ', words);
            if(
                (words.size() == 2 && words[1][words[1].length()-1] == ';') ||
                (words.size() == 3 && words[2] == ";")
            ) {
                std::string vartype = words[0];
                std::string varname = words[1];
                if(varname[varname.length()-1] == ';') {
                    varname = varname.substr(0, varname.length()-1);
                }
                if(vartype == "bool") {
                    output << "   M[0].x += float(GLUP." << varname << ");\n";
                } else if(vartype == "int") {
                    output << "   M[0].x += float(GLUP." << varname << ");\n";
                } else if(vartype == "float") {
                    output << "   M[0].x += GLUP." << varname << ";\n";
                } else if(vartype == "vec3") {
                    output << "   M[0].xyz += GLUP." << varname << ";\n";
                } else if(vartype == "vec4") {
                    output << "   M[0] += GLUP." << varname << ";\n";
                } else if(vartype == "mat3") {
                    output << "   M[0].xyz += GLUP." << varname << "[0];\n";
                } else if(vartype == "mat4") {
                    output << "   M += GLUP." << varname << ";\n";  
                }
            }
        }

        output << "   gl_Position = M*vec4(position,1.0);\n";
        output << "}\n";
        
        return output.str();
    }
#endif
    
    void Context::setup() {
        
        uniform_buffer_dirty_=true;
        matrices_dirty_=true;        
        uniform_binding_point_=1;

#ifdef GEO_GL_150
        
        // A minimalistic GLSL program that uses the GLUP context.
        // It is there just to use GLSL introspection API to lookup
        // the offsets of GLUP context state variables.
        // Note: it needs to use all the variables of the uniform state,
        // else, depending on the OpenGL driver / library,
        // some variables may be optimized-out and glGetUniformIndices()
        // returns GL_INVALID_INDEX (stupid !), this is why there is
        // this (weird) function
        //  create_vertex_program_that_uses_all_UBO_variables()        

        static const char* shader_source_header_ =
#ifdef GEO_OS_ANDROID
            "#version 300 es\n"
	    "precision highp float;\n"
            "precision lowp sampler3D;\n" ; 	    	
#else	    
            "#version 150 core\n" ; 
#endif
	
        static const char* fragment_shader_source_ =
            "out vec4 colorOut;                      \n"
            "void main() {                           \n"
            "   colorOut = vec4(1.0, 1.0, 1.0, 1.0); \n" 
            "}                                       \n";

        GLuint GLUP_vertex_shader = GLSL::compile_shader(
            GL_VERTEX_SHADER,
            shader_source_header_,
            Context::uniform_state_declaration(),
            create_vertex_program_that_uses_all_UBO_variables().c_str(),
            nullptr
        );

        GLuint GLUP_fragment_shader = GLSL::compile_shader(
            GL_FRAGMENT_SHADER,
            shader_source_header_,
            fragment_shader_source_,
            nullptr
        );

        default_program_ = GLSL::create_program_from_shaders(
            GLUP_vertex_shader,
            GLUP_fragment_shader,
            nullptr
        );

        // Get UBO size

        GLuint UBO_index =
            glGetUniformBlockIndex(default_program_, "GLUPStateBlock");
        
        glUniformBlockBinding(
            default_program_, UBO_index, uniform_binding_point_
        );

        glGetActiveUniformBlockiv(
            default_program_, UBO_index,
            GL_UNIFORM_BLOCK_DATA_SIZE,
            &uniform_buffer_size_
        );

        // Create UBO

        uniform_buffer_data_ = new Memory::byte[uniform_buffer_size_];
        Memory::clear(uniform_buffer_data_, size_t(uniform_buffer_size_));
        glGenBuffers(1, &uniform_buffer_);
        glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer_);
        glBufferData(
            GL_UNIFORM_BUFFER,
            uniform_buffer_size_,
            uniform_buffer_data_,
            GL_DYNAMIC_DRAW
        );
                
        glBindBufferBase(
            GL_UNIFORM_BUFFER,
            uniform_binding_point_,
            uniform_buffer_
        );
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

#endif
        
        setup_state_variables();
        setup_immediate_buffers();
        
        //  Indicate that all toggle states should be
        // read from the state (default mode, superseded later).
        setup_shaders_source_for_toggles(0,~0);
        setup_primitives();
    }

    void Context::setup_state_variables() {
        uniform_state_.toggle.push_back(
            StateVariable<GLboolean>(this,"lighting_enabled",GL_TRUE)
        );
        uniform_state_.toggle.push_back(
            StateVariable<GLboolean>(this,"vertex_colors_enabled",GL_FALSE)
        );
        uniform_state_.toggle.push_back(
            StateVariable<GLboolean>(this,"texturing_enabled",GL_FALSE)
        );
        uniform_state_.toggle.push_back(
            StateVariable<GLboolean>(this,"draw_mesh_enabled",GL_FALSE)
        );
        uniform_state_.toggle.push_back(
            StateVariable<GLboolean>(this,"clipping_enabled",GL_FALSE)
        );
        uniform_state_.toggle.push_back(
            StateVariable<GLboolean>(this,"indirect_texturing_enabled",GL_FALSE)
        );
        uniform_state_.toggle.push_back(
            StateVariable<GLboolean>(this,"vertex_normals_enabled",GL_FALSE)
        );
        uniform_state_.toggle.push_back(
            StateVariable<GLboolean>(this,"picking_enabled",GL_FALSE)
        );
        uniform_state_.toggle.push_back(
            StateVariable<GLboolean>(this,"alpha_discard_enabled",GL_FALSE)
        );
        uniform_state_.toggle.push_back(
            StateVariable<GLboolean>(this,"normal_mapping_enabled",GL_FALSE)
        );
	
        uniform_state_.color.push_back(
            VectorStateVariable(this, "front_color", 4)
        );
        uniform_state_.color.push_back(
            VectorStateVariable(this, "back_color", 4)
        );
        uniform_state_.color.push_back(
            VectorStateVariable(this, "mesh_color", 4)
        );
        
        uniform_state_.light_vector.initialize(this, "light_vector", 3);
        uniform_state_.light_half_vector.initialize(
            this, "light_half_vector", 3
        );
        
        uniform_state_.mesh_width.initialize(this, "mesh_width", 1.0f);
        uniform_state_.cells_shrink.initialize(this, "cells_shrink", 0.0f);

        uniform_state_.picking_mode.initialize(
            this, "picking_mode", GLUP_PICK_PRIMITIVE
        );
        uniform_state_.picking_id.initialize(this, "picking_id", 0);
        uniform_state_.base_picking_id.initialize(this, "base_picking_id", 0);

        uniform_state_.clipping_mode.initialize(
            this, "clipping_mode", GLUP_CLIP_WHOLE_CELLS
        );
        uniform_state_.clip_plane.initialize(this, "clip_plane", 4);
        uniform_state_.world_clip_plane.initialize(
            this, "world_clip_plane", 4
        );
        uniform_state_.clip_clip_plane.initialize(
            this, "clip_clip_plane", 4
        );        

        uniform_state_.texture_mode.initialize(
            this, "texture_mode", GLUP_TEXTURE_MODULATE
        );

        uniform_state_.texture_type.initialize(
            this, "texture_type", GLUP_TEXTURE_2D
        );

	uniform_state_.alpha_threshold.initialize(
	    this, "alpha_threshold", 0.5f
	);

	uniform_state_.specular.initialize(
	    this, "specular", 1.0f
	);
	
        uniform_state_.modelview_matrix.initialize(this, "modelview_matrix");
        uniform_state_.modelviewprojection_matrix.initialize(
            this, "modelviewprojection_matrix"
        );
        uniform_state_.projection_matrix.initialize(this, "projection_matrix");
        uniform_state_.normal_matrix.initialize(this, "normal_matrix");
        uniform_state_.texture_matrix.initialize(this, "texture_matrix");

        uniform_state_.inverse_modelviewprojection_matrix.initialize(
	    this, "inverse_modelviewprojection_matrix"
	);
        uniform_state_.inverse_modelview_matrix.initialize(
	    this, "inverse_modelview_matrix"
	);
        uniform_state_.inverse_projection_matrix.initialize(
	    this, "inverse_projection_matrix"
	);	
	uniform_state_.viewport.initialize(this, "viewport", 4);

        uniform_state_.point_size.initialize(this, "point_size", 1.0f);
        
        matrix_mode_ = GLUP_MODELVIEW_MATRIX;
        update_matrices();
    }

    void Context::setup_immediate_buffers() {

        glupGenVertexArrays(1,&immediate_state_.VAO());
        glupBindVertexArray(immediate_state_.VAO());
        
        for(index_t i=0; i<ImmediateState::NB_IMMEDIATE_BUFFERS; ++i) {
            update_buffer_object(
                immediate_state_.buffer[i].VBO(),
                GL_ARRAY_BUFFER,
                immediate_state_.buffer[i].size_in_bytes(),
                nullptr // no need to copy the buffer, it will be overwritten after.
            );
        }
        
        bind_immediate_state_buffers_to_VAO();
        glupBindVertexArray(0);
    }

    void Context::stream_immediate_buffers() {
	if(immediate_state_.nb_vertices() == IMMEDIATE_BUFFER_SIZE) {
	    for(index_t i=0; i<ImmediateState::NB_IMMEDIATE_BUFFERS; ++i) {
		if(
		    immediate_state_.buffer[i].is_enabled() &&
		    immediate_state_.buffer[i].VBO() != 0
		) {
		    stream_buffer_object(
			immediate_state_.buffer[i].VBO(),
			GL_ARRAY_BUFFER,
			immediate_state_.buffer[i].size_in_bytes(),
			immediate_state_.buffer[i].data()
		    );
		}
	    }
	} else {
	    for(index_t i=0; i<ImmediateState::NB_IMMEDIATE_BUFFERS; ++i) {
		if(
		    immediate_state_.buffer[i].is_enabled() &&
		    immediate_state_.buffer[i].VBO() != 0
		) {
		    size_t bytes =
			immediate_state_.nb_vertices() *
			immediate_state_.buffer[i].dimension() *
			sizeof(GLfloat);
		    glBindBuffer(
			GL_ARRAY_BUFFER, immediate_state_.buffer[i].VBO()
		    );
		    glBufferSubData(
			GL_ARRAY_BUFFER,
			0,
			GLsizeiptr(bytes),
			immediate_state_.buffer[i].data()
		    );
		}
	    }
	}
        glBindBuffer(GL_ARRAY_BUFFER,0);        
    }

    
    Memory::pointer Context::get_state_variable_address(const char* name_in) {
        std::string name = std::string("GLUPStateBlock.") + name_in;
        GLint offset = GLSL::get_uniform_variable_offset(
            default_program_, name.c_str()
        );
        return uniform_buffer_data_ + offset;
    }
    
    void Context::bind_uniform_state(GLuint program) {
#ifndef GEO_GL_150
        geo_argused(program);
#else        
        GLuint UBO_index = glGetUniformBlockIndex(
            program, "GLUPStateBlock"
        );
        if(UBO_index != GL_INVALID_INDEX) {
            glUniformBlockBinding(
                program, UBO_index, uniform_binding_point_
            );
        }
#endif        
    }
    
    void Context::begin(GLUPprimitive primitive) {
        update_toggles_config();
        create_program_if_needed(primitive);
        if(!primitive_info_[primitive].implemented) {
            Logger::warn("GLUP")
                << "glupBegin(): "
                << primitive_name[primitive]
                << " not implemented in this profile" << std::endl;
        }

        GEO_CHECK_GL();    
        update_uniform_buffer();
        GEO_CHECK_GL();
        
        //   If the primitive has a special VAO to be used for immediate
        // mode, then bind it.
        if(primitive_info_[primitive].VAO != 0) {
            GEO_CHECK_GL();            
            glupBindVertexArray(
                primitive_info_[primitive].VAO
            );            
        } else {
            GEO_CHECK_GL();                        
            // Else use the regular VAO used by all immediate-mode primitives
            // (if there is one).
            if(immediate_state_.VAO() != 0) {
                glupBindVertexArray(immediate_state_.VAO());
            }
        }

        GEO_CHECK_GL();                    

        if(uniform_state_.toggle[GLUP_VERTEX_COLORS].get()) {
            immediate_state_.buffer[GLUP_COLOR_ATTRIBUTE].enable();
        } else {
            immediate_state_.buffer[GLUP_COLOR_ATTRIBUTE].disable();
        }

        GEO_CHECK_GL();
        
        if(uniform_state_.toggle[GLUP_TEXTURING].get()) {
            immediate_state_.buffer[GLUP_TEX_COORD_ATTRIBUTE].enable();
        } else {
            immediate_state_.buffer[GLUP_TEX_COORD_ATTRIBUTE].disable();
        }

        GEO_CHECK_GL();        

        if(
	    uniform_state_.toggle[GLUP_LIGHTING].get() &&
	    uniform_state_.toggle[GLUP_VERTEX_NORMALS].get()
	) {
            immediate_state_.buffer[GLUP_NORMAL_ATTRIBUTE].enable();
        } else {
            immediate_state_.buffer[GLUP_NORMAL_ATTRIBUTE].disable();
        }

        GEO_CHECK_GL();        	
	
        immediate_state_.begin(primitive);

        GEO_CHECK_GL();        
        
        if(primitive_info_[primitive].vertex_gather_mode) {
            index_t n = nb_vertices_per_primitive[primitive];
            GLenum GL_primitive = primitive_info_[primitive].GL_primitive;
            n /= nb_vertices_per_GL_primitive(GL_primitive);

            for(index_t i=0; i<ImmediateState::NB_IMMEDIATE_BUFFERS; ++i) {
                if(immediate_state_.buffer[i].is_enabled()) {                
                    for(index_t j=0; j<n; ++j) {
                        glEnableVertexAttribArray(i*n+j);
                    }
                }
            }
        } else {
            for(index_t i=0; i<ImmediateState::NB_IMMEDIATE_BUFFERS; ++i) {
                if(immediate_state_.buffer[i].is_enabled()) {
                    glEnableVertexAttribArray(i);
                    
                    // If not using VBO, specify vertex attrib pointer
                    // using old style API. 
                    
                    if(immediate_state_.buffer[i].VBO() == 0) {                
                        glVertexAttribPointer(
                            i,
                            GLint(immediate_state_.buffer[i].dimension()),
                            GL_FLOAT, GL_FALSE, 0,
                            immediate_state_.buffer[i].data()
                        );
                    }
                }
            }
        }

        GEO_CHECK_GL();
        
        prepare_to_draw(primitive);

        GEO_CHECK_GL();
        
        if(user_program_ != 0) {
            use_program(user_program_);
        } else {
            use_program(primitive_info_[primitive].program(toggles_config_));
        }

        GEO_CHECK_GL();        
    }

    void Context::end() {
        GEO_CHECK_GL();         
        flush_immediate_buffers();
        GEO_CHECK_GL();         
        use_program(0);
        GEO_CHECK_GL();                 

        if(primitive_info_[immediate_state_.primitive()].vertex_gather_mode) {
            index_t n = nb_vertices_per_primitive[immediate_state_.primitive()];
            GLenum GL_primitive =
                primitive_info_[immediate_state_.primitive()].GL_primitive;
            n /= nb_vertices_per_GL_primitive(GL_primitive);
            for(index_t i=0; i<ImmediateState::NB_IMMEDIATE_BUFFERS; ++i) {
                if(immediate_state_.buffer[i].is_enabled()) {
		    for(index_t j=0; j<n; ++j) {
			GEO_CHECK_GL();
			glDisableVertexAttribArray(i*n+j);
			GEO_CHECK_GL();                                     
		    }
		}
            }
        } else {
            for(index_t i=0; i<ImmediateState::NB_IMMEDIATE_BUFFERS; ++i) {
                if(immediate_state_.buffer[i].is_enabled()) {		
		    GEO_CHECK_GL();                
		    glDisableVertexAttribArray(i);
		    GEO_CHECK_GL();
		}
            }
        }

        if(
            uniform_state_.toggle[GLUP_PICKING].get() &&
            uniform_state_.picking_mode.get() == GLUP_PICK_PRIMITIVE
        ) {
            GEO_CHECK_GL();                            
            update_base_picking_id(0);
            GEO_CHECK_GL();                                        
        }

        if(
            primitive_info_[immediate_state_.primitive()].VAO != 0 ||
            immediate_state_.VAO() != 0
        ) {
            GEO_CHECK_GL();                            
            glupBindVertexArray(0);
            GEO_CHECK_GL();                                        
        }
        GEO_CHECK_GL();         
        done_draw(immediate_state_.primitive());
        GEO_CHECK_GL();
    }

    void Context::draw_arrays(
        GLUPprimitive primitive, GLUPint first, GLUPsizei count
    ) {
        update_toggles_config();
        create_program_if_needed(primitive);
        if(!primitive_supports_array_mode(primitive)) {
            Logger::warn("GLUP")
                << profile_name()
                << "::glupDrawArrays(): "
                << primitive_name[primitive]
                << " does not support array mode." << std::endl;
            return;
        }
        prepare_to_draw(primitive);
        update_uniform_buffer();
        if(user_program_ != 0) {
            use_program(user_program_);
        } else {
            use_program(primitive_info_[primitive].program(toggles_config_));
        }
        glDrawArrays(primitive_info_[primitive].GL_primitive, first, count);
        GEO_CHECK_GL();         
        use_program(0);
        GEO_CHECK_GL();         
        done_draw(primitive);
        GEO_CHECK_GL();         
    }

    void Context::draw_elements(
        GLUPprimitive primitive, GLUPsizei count,
        GLUPenum type, const GLUPvoid* indices
    ) {
        GEO_CHECK_GL();    
        update_toggles_config();
        GEO_CHECK_GL();            
        create_program_if_needed(primitive);
        GEO_CHECK_GL();            
        if(!primitive_supports_array_mode(primitive)) {
            Logger::warn("GLUP")
                << profile_name()
                << "::glupDrawElements(): "
                << primitive_name[primitive]
                << " does not support array mode." << std::endl;
            return;
        }
        GEO_CHECK_GL();            
        prepare_to_draw(primitive);
        GEO_CHECK_GL();            
        update_uniform_buffer();
        GEO_CHECK_GL();            
        if(user_program_ != 0) {
            use_program(user_program_);
        } else {
            use_program(primitive_info_[primitive].program(toggles_config_));
        }
        GEO_CHECK_GL();                    
        glDrawElements(
            primitive_info_[primitive].GL_primitive, count, type, indices
        );
        GEO_CHECK_GL(); 
        use_program(0);
        GEO_CHECK_GL();         
        done_draw(primitive);
        GEO_CHECK_GL();         
    }

    void Context::prepare_to_draw(GLUPprimitive primitive) {
#ifndef GEO_GL_440
        geo_argused(primitive);
#else

        if(primitive == GLUP_POINTS || primitive == GLUP_SPHERES) {
            glEnable(GL_PROGRAM_POINT_SIZE);
        } else if(primitive == GLUP_LINES && !use_core_profile_) {
            // Note: glLineWidth is deprecated in OpenGL core profile,
            // for now we still use it if we got a compatibility profile,
            // but for core profile, we are for now unable to display
            // thick lines (TODO: develop a shader for that, painful...)
            glLineWidth(uniform_state_.mesh_width.get());
        }
        
        if(primitive_info_[primitive].GL_primitive == GL_PATCHES) {
            // We generate an isoline for each patch, with the
            // minimum tesselation level. This generates two
            // vertices (we discard one of them in the geometry
            // shader).
            static float levels[4] = {1.0, 1.0, 0.0, 0.0};
            glPatchParameterfv(GL_PATCH_DEFAULT_OUTER_LEVEL, levels);
                
            // Specify number of vertices for GL_PATCH.
            glPatchParameteri(
                GL_PATCH_VERTICES, GLint(nb_vertices_per_primitive[primitive])
            );
        }
#endif
    }

    void Context::done_draw(GLUPprimitive primitive) {
        geo_argused(primitive);
    }
    
    void Context::update_matrices() {
        
        if(!matrices_dirty_) {
            return;
        }

        GLfloat* modelview = matrix_stack_[GLUP_MODELVIEW_MATRIX].top();
        GLfloat* projection = matrix_stack_[GLUP_PROJECTION_MATRIX].top();
	GLfloat* modelviewproject =
	    uniform_state_.modelviewprojection_matrix.get_pointer();
	GLfloat* modelviewproject_invert =
	    uniform_state_.inverse_modelviewprojection_matrix.get_pointer();
	GLfloat* modelview_invert =
	    uniform_state_.inverse_modelview_matrix.get_pointer();
	GLfloat* projection_invert =
	    uniform_state_.inverse_projection_matrix.get_pointer();
	
        copy_vector(
            uniform_state_.modelview_matrix.get_pointer(), modelview, 16
        );
        mult_matrices(modelviewproject, modelview, projection);

        GLboolean OK = invert_matrix(projection_invert, projection);
        if(!OK) {
            Logger::warn("GLUP") << "Singular Projection matrix"
                                 << std::endl;
            show_matrix(projection);
        }
	
        OK = invert_matrix(modelview_invert, modelview);
        if(!OK) {
            Logger::warn("GLUP") << "Singular ModelView matrix"
                                 << std::endl;
            show_matrix(modelview);
        }

	OK = invert_matrix(modelviewproject_invert, modelviewproject);
        if(!OK) {
            Logger::warn("GLUP") << "Singular ModelViewProjection matrix"
                                 << std::endl;
            show_matrix(modelviewproject);
        }
	
        GLfloat* normal_matrix = uniform_state_.normal_matrix.get_pointer();
        //   Copy the upper leftmost 3x3 part of the transpose of
        // modelview_invert_matrix to normal_matrix
        for(index_t i=0; i<3; ++i) {
            for(index_t j=0; j<3; ++j) {
                // Yes, it is i*4
                //   (with a '4' though it is a mat3 with a '3'),
                // mat3 rows are padded-aligned in UBOs !
                // TODO: query padding in UBO using introspection
                // (is it possible ? does not seem to work, so
                //  is padding with 4 universal ??)
                normal_matrix[i*4+j] = modelview_invert[j*4+i];
            }
        }

        copy_vector(
            uniform_state_.texture_matrix.get_pointer(),
            matrix_stack_[GLUP_TEXTURE_MATRIX].top(),
            16
        );

        copy_vector(
            uniform_state_.projection_matrix.get_pointer(),
            matrix_stack_[GLUP_PROJECTION_MATRIX].top(),
            16
        );
        
        mult_matrix_vector(
            uniform_state_.world_clip_plane.get_pointer(),
            uniform_state_.modelview_matrix.get_pointer(),
            uniform_state_.clip_plane.get_pointer()            
        );

	OK = invert_matrix(
	    projection_invert,
	    uniform_state_.projection_matrix.get_pointer()
	);
	if(!OK) {
	    Logger::warn("GLUP") << "Singular projection matrix"
				 << std::endl;
	    show_matrix(uniform_state_.projection_matrix.get_pointer());
	}
	mult_matrix_vector(
	    uniform_state_.clip_clip_plane.get_pointer(),
	    projection_invert,
	    uniform_state_.clip_plane.get_pointer()
	);

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	for(index_t i=0; i<4; ++i) {
	    uniform_state_.viewport.get_pointer()[i] = GLfloat(viewport[i]);
	}
	
        matrices_dirty_ = false;
    }

    void Context::update_lighting() {
        if(!lighting_dirty_) {
            return;
        }

        GLfloat* light_vector =
            uniform_state_.light_vector.get_pointer();
        
        GLfloat* light_half_vector =
            uniform_state_.light_half_vector.get_pointer();

        // Normalize light vector
        
        normalize_vector(light_vector);

        // Compute half vector
        
        copy_vector(light_half_vector, light_vector, 3);
        light_half_vector[2] += 1.0f;
        normalize_vector(light_half_vector);        
        
        lighting_dirty_ = false;
    }

    void Context::update_base_picking_id(GLint new_value) {
#ifndef GEO_GL_150
        geo_argused(new_value);
#else        
        Memory::pointer address = uniform_state_.base_picking_id.address();
        index_t offset = index_t(address - uniform_buffer_data_);
        uniform_state_.base_picking_id.set(new_value);
        glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer_);
        glBufferSubData(
            GL_UNIFORM_BUFFER,
            offset,
            sizeof(int),
            address
        );
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
#endif        
    }
    
    void Context::do_update_uniform_buffer() {
        if(!uniform_buffer_dirty_) {
            return;
        }
        if(matrices_dirty_) {
            update_matrices();
        }
        if(lighting_dirty_) {
            update_lighting();
        }
#ifdef GEO_GL_150
	#ifdef GL_CLIP_DISTANCE0
        if(!use_ES_profile_) {
            if(uniform_state_.toggle[GLUP_CLIPPING].get()) {
                glEnable(GL_CLIP_DISTANCE0);
            } else {
                glDisable(GL_CLIP_DISTANCE0);
            }
        }
	#endif
        if(uniform_buffer_ != 0) {
            glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer_);
            glBufferSubData(
                GL_UNIFORM_BUFFER,
                0,
                uniform_buffer_size_,
                uniform_buffer_data_
            );
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }
#endif        
        uniform_buffer_dirty_ = false;
    }

    void Context::flush_immediate_buffers() {

        
        if(immediate_state_.nb_vertices() == 0) {
            return;
        }

        // Sends the data from the buffers to OpenGL if VBO are used.
        stream_immediate_buffers();

        bool vertex_id_attrib_enabled = false;

        if(vertex_id_VBO_ != 0) {
            if(
                uniform_state_.toggle[GLUP_PICKING].get() || (               
                    primitive_dimension[immediate_state_.primitive()] >= 2 && 
                    uniform_state_.toggle[GLUP_DRAW_MESH].get()
                )
            ) {
                glEnableVertexAttribArray(GLUP_VERTEX_ID_ATTRIBUTE);
                vertex_id_attrib_enabled = true;
            }
        }
        
        GLsizei nb_vertices = GLsizei(immediate_state_.nb_vertices());

        if(primitive_info_[immediate_state_.primitive()].vertex_gather_mode) {
            nb_vertices /= GLsizei(
                nb_vertices_per_primitive[immediate_state_.primitive()]
            );
            nb_vertices *= GLsizei(
                nb_vertices_per_GL_primitive(
                    primitive_info_[immediate_state_.primitive()].GL_primitive
                )
            );
        }

        GEO_CHECK_GL();
        if(primitive_info_[immediate_state_.primitive()].elements_VBO != 0) {
            index_t nb_primitives = index_t(nb_vertices) /
                nb_vertices_per_primitive[immediate_state_.primitive()];
            index_t nb_elements = nb_primitives *
                primitive_info_[immediate_state_.primitive()].
                nb_elements_per_primitive ;

            glDrawElements(
                primitive_info_[immediate_state_.primitive()].GL_primitive,
                GLsizei(nb_elements),
                GL_UNSIGNED_SHORT,
                nullptr
            );
        } else {
            glDrawArrays(
                primitive_info_[immediate_state_.primitive()].GL_primitive,
                0,
                nb_vertices
            );
        }
        GEO_CHECK_GL();        

        // Picking mode uses GLSL primitive_id variable, and add
        // GLUP state base_picking_id to it. This code updates
        // GLUP state base_picking_id and adds the number of drawn
        // primitives to it, so that the next batch will start with
        // the correct base_picking_id
        if(
            uniform_state_.toggle[GLUP_PICKING].get() &&
            uniform_state_.picking_mode.get() == GLUP_PICK_PRIMITIVE
        ) {
            update_base_picking_id(
                uniform_state_.base_picking_id.get() +
                GLint(immediate_state_.nb_primitives())
            );
        }
        immediate_state_.reset();

        if(vertex_id_attrib_enabled) {
            glDisableVertexAttribArray(GLUP_VERTEX_ID_ATTRIBUTE);
        }
    }

    /***********************************************************************/

    void Context::set_primitive_info(
        GLUPprimitive glup_primitive, GLenum gl_primitive, GLuint program,
        bool bind_attrib_loc
    ) {
        primitive_info_[glup_primitive].implemented = true;
        primitive_info_[glup_primitive].GL_primitive = gl_primitive;
        primitive_info_[glup_primitive].shader_map[toggles_config_] = program;

        if(!bind_attrib_loc) {
            return;
        }

        GEO_CHECK_GL();    	
        glBindAttribLocation(program, GLUP_VERTEX_ATTRIBUTE, "vertex_in");
        glBindAttribLocation(program, GLUP_COLOR_ATTRIBUTE, "color_in");
        glBindAttribLocation(program, GLUP_TEX_COORD_ATTRIBUTE, "tex_coord_in");
        glBindAttribLocation(program, GLUP_NORMAL_ATTRIBUTE, "normal_in");	
        if(vertex_id_VBO_ != 0) {
            glBindAttribLocation(
		program, GLUP_VERTEX_ID_ATTRIBUTE, "vertex_id_in"
	    );            
        }
        GEO_CHECK_GL();    	
        GLSL::link_program(program);
        GEO_CHECK_GL();    	
        bind_uniform_state(program);            
        GEO_CHECK_GL();    	

        //  Bind default texture units. We use different texture
        // units because there is a mode where both a 1D and another
        // texture is bound ("indirect texturing").
	
        GLSL::set_program_uniform_by_name(
            program, "texture2Dsampler", GLint(GLUP_TEXTURE_2D_UNIT)
        );        
        GLSL::set_program_uniform_by_name(
            program, "texture1Dsampler", GLint(GLUP_TEXTURE_1D_UNIT)
        );
        GLSL::set_program_uniform_by_name(
            program, "texture3Dsampler", GLint(GLUP_TEXTURE_3D_UNIT)
        );
	
        GEO_CHECK_GL();    	
    }

    void Context::set_primitive_info_vertex_gather_mode(
        GLUPprimitive glup_primitive, GLenum GL_primitive, GLuint program
    ) {
        set_primitive_info(glup_primitive, GL_primitive, program, false);
        index_t n = nb_vertices_per_primitive[glup_primitive];
        n /= nb_vertices_per_GL_primitive(GL_primitive);
        
        // Attribute location are bound here, programmatically,
        // since their index depends on the number of vertices,
        // and GLSL does not like to have that in the declaration
        // (when saying layout(binding = nb_vertices), the GLSL
        // compiler does not "see" that nb_vertices is a constant).
	
        GEO_CHECK_GL();    	
        glBindAttribLocation(program, 0, "vertex_in");
        glBindAttribLocation(program, n, "color_in");
        glBindAttribLocation(program, 2*n, "tex_coord_in");
        glBindAttribLocation(program, 3*n, "normal_in");
	
        GEO_CHECK_GL();    	
        GLSL::link_program(program);

        GEO_CHECK_GL();    		
        bind_uniform_state(program);            

        //  Bind default texture units. We use different texture
        // units because there is a mode where both a 1D and another
        // texture is bound ("indirect texturing").

        GEO_CHECK_GL();    			
        GLSL::set_program_uniform_by_name(
            program, "texture2Dsampler", GLint(GLUP_TEXTURE_2D_UNIT)
        );        
        GLSL::set_program_uniform_by_name(
            program, "texture1Dsampler", GLint(GLUP_TEXTURE_1D_UNIT)
        );
        GLSL::set_program_uniform_by_name(
            program, "texture3Dsampler", GLint(GLUP_TEXTURE_3D_UNIT)
        );
        
        primitive_info_[glup_primitive].vertex_gather_mode = true;
    
        //   We need a special VAO: memory layout is different since
        // we use a single vertex with an array of n attributes...
        //   Note: since the function can be called several times (one
        // per toggles configuration), make sure the VAO does not already
        // exists.
        GEO_CHECK_GL();    			
        if(primitive_info_[glup_primitive].VAO == 0) {
	    GEO_CHECK_GL();    				    
            glupGenVertexArrays(
                1, &(primitive_info_[glup_primitive].VAO)
            );
	    GEO_CHECK_GL();    				    
            glupBindVertexArray(
                primitive_info_[glup_primitive].VAO
            );
	    GEO_CHECK_GL();    			
            for(index_t i=0; i<ImmediateState::NB_IMMEDIATE_BUFFERS; ++i) {
                glBindBuffer(GL_ARRAY_BUFFER,immediate_state_.buffer[i].VBO());
		GEO_CHECK_GL();    					
                for(index_t j=0; j<n; ++j) {
                    glVertexAttribPointer(
                        i*n+j,
                        4,
                        GL_FLOAT,
                        GL_FALSE,
                        GLsizei(sizeof(GL_FLOAT)*4*n),        // stride
                        (const GLvoid*)(sizeof(GL_FLOAT)*4*j) // pointer   
                    );
		    GEO_CHECK_GL();		    
                }
            }

            glBindBuffer(GL_ARRAY_BUFFER,0);        
            glupBindVertexArray(0);
        }
    }

    void Context::set_primitive_info_immediate_index_mode(
        GLUPprimitive glup_primitive, GLenum GL_primitive, GLuint program,
        index_t nb_elements_per_glup_primitive,
        index_t* element_indices 
    ) {
        
        set_primitive_info(glup_primitive, GL_primitive, program);

        if(primitive_info_[glup_primitive].elements_VBO == 0) {

            index_t nb_glup_primitives =
                IMMEDIATE_BUFFER_SIZE /
                nb_vertices_per_primitive[glup_primitive];

            index_t nb_elements =
                nb_glup_primitives * nb_elements_per_glup_primitive;
            
            GLuint elements_VBO = 0;
            glGenBuffers(1, &elements_VBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elements_VBO);

            Numeric::uint16* indices = new Numeric::uint16[nb_elements];
            index_t cur_element=0;
            index_t cur_vertex_offset=0;
            for(
                index_t glup_prim=0; glup_prim<nb_glup_primitives; ++glup_prim
            ) {
                for(index_t le=0; le<nb_elements_per_glup_primitive; ++le) {
                    indices[cur_element] = Numeric::uint16(
                        cur_vertex_offset + element_indices[le]
                    );
                    ++cur_element;
                }
                cur_vertex_offset += nb_vertices_per_primitive[glup_primitive];
            }

            glBufferData(
                GL_ELEMENT_ARRAY_BUFFER,
                GLsizeiptr(sizeof(Numeric::uint16) * nb_elements),
                indices, GL_STATIC_DRAW
            );
            
            delete[] indices;

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            
            primitive_info_[glup_primitive].elements_VBO = elements_VBO;
            primitive_info_[glup_primitive].nb_elements_per_primitive =
                nb_elements_per_glup_primitive;
            primitive_info_[glup_primitive].primitive_elements =
                element_indices;
        }
        
        if(primitive_info_[glup_primitive].VAO == 0) {
            glupGenVertexArrays(1,&primitive_info_[glup_primitive].VAO);
            glupBindVertexArray(primitive_info_[glup_primitive].VAO);

            bind_immediate_state_buffers_to_VAO();

            glBindBuffer(
                GL_ELEMENT_ARRAY_BUFFER,
                primitive_info_[glup_primitive].elements_VBO
            );

            if(vertex_id_VBO_ != 0) {
                glBindBuffer(GL_ARRAY_BUFFER, vertex_id_VBO_);
                glVertexAttribPointer(
                    GLUP_VERTEX_ID_ATTRIBUTE,
                    1,                 // 1 component per attribute
                    GL_UNSIGNED_SHORT, // components are bytes
                    GL_FALSE,          // do not normalize
                    0,                 // stride
                    nullptr            // pointer (relative to bound VBO beginning)
                );
            }
            
            glupBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER,0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
        }
    }
    
    void Context::setup_primitives() {
        primitive_info_.resize(GLUP_NB_PRIMITIVES);
    }

    void Context::setup_shaders_source_for_toggles(
        GLUPbitfield toggles_state,
        GLUPbitfield toggles_undetermined
    ) {
        toggles_source_state_ = toggles_state;
        toggles_source_undetermined_ = toggles_undetermined;
    }

    void Context::setup_shaders_source_for_primitive(
        GLUPprimitive primitive
    ) {
        primitive_source_ = primitive;
    }

    
    std::string Context::primitive_declaration(GLUPprimitive prim) const {
        std::string result =
            std::string("const int glup_primitive = ") +
            primitive_name[prim] + ";\n"                 +
            "const int glup_primitive_dimension = "   +
            String::to_string(primitive_dimension[prim]) +
            ";\n" +
            "const int glup_primitive_nb_vertices = " +
            String::to_string(nb_vertices_per_primitive[prim]) +
            ";\n" +
	    "#define GLUP_PRIMITIVE_DIMENSION " +
	    String::to_string(primitive_dimension[prim]) +
            "\n"	    
            ;
        
        return result;
    }
    
    void Context::update_toggles_config() {
        if(uniform_state_.toggle[GLUP_PICKING].get()) {
            toggles_config_ = (1u << GLUP_PICKING);
        } else {
            toggles_config_ = 0;
            for(index_t i=0; i<uniform_state_.toggle.size(); ++i) {
                if(uniform_state_.toggle[i].get()) {
                    toggles_config_ |= ((PrimitiveInfo::ShaderKey)(1) << i);
                }
            }
        }
    }

    void Context::create_program_if_needed(GLUPprimitive primitive) {
        primitive_source_ = primitive;
        if(
	    !primitive_info_[primitive].program_is_initialized(toggles_config_)
	) {
            setup_shaders_source_for_primitive(primitive);
            setup_shaders_source_for_toggles_config(toggles_config_);
            switch(primitive) {
            case GLUP_POINTS:
                setup_GLUP_POINTS();
                break;
            case GLUP_LINES:
                setup_GLUP_LINES();
                break;
            case GLUP_TRIANGLES:
                setup_GLUP_TRIANGLES();
                break;
            case GLUP_QUADS:
                setup_GLUP_QUADS();
                break;
            case GLUP_TETRAHEDRA:
                setup_GLUP_TETRAHEDRA();
                break;
            case GLUP_HEXAHEDRA:
                setup_GLUP_HEXAHEDRA();
                break;
            case GLUP_PRISMS:
                setup_GLUP_PRISMS();
                break;
            case GLUP_PYRAMIDS:
                setup_GLUP_PYRAMIDS();
                break;
            case GLUP_CONNECTORS:
                setup_GLUP_CONNECTORS();
                break;
            case GLUP_SPHERES:
                setup_GLUP_SPHERES();
                break;
            case GLUP_NB_PRIMITIVES:
                geo_assert_not_reached;
            }
        }
    }
    
    void Context::setup_GLUP_POINTS() {
    }

    void Context::setup_GLUP_LINES() {
    }

    void Context::setup_GLUP_TRIANGLES() {
    }

    void Context::setup_GLUP_QUADS() {
    }

    void Context::setup_GLUP_TETRAHEDRA() {
    }

    void Context::setup_GLUP_HEXAHEDRA() {
    }

    void Context::setup_GLUP_PRISMS() {
    }

    void Context::setup_GLUP_PYRAMIDS() {
    }

    void Context::setup_GLUP_CONNECTORS() {
    }

    void Context::setup_GLUP_SPHERES() {
    }
    
    void Context::shrink_cells_in_immediate_buffers() {
        if(
            uniform_state_.cells_shrink.get() == 0.0f   ||
            immediate_state_.primitive() == GLUP_POINTS ||
            immediate_state_.primitive() == GLUP_LINES  ||
            (uniform_state_.clipping_mode.get() == GLUP_CLIP_SLICE_CELLS &&
             uniform_state_.toggle[GLUP_CLIPPING].get())
        ) {
            return;
        }
        
        GLfloat s = uniform_state_.cells_shrink.get();
        GLfloat g[3];
        index_t nb_v = nb_vertices_per_primitive[immediate_state_.primitive()];
        index_t v=0;
        while(v < immediate_state_.nb_vertices()) {
            g[0] = 0.0f;
            g[1] = 0.0f;
            g[2] = 0.0f;
            for(index_t lv=0; lv<nb_v; ++lv) {
                GLfloat* p = immediate_state_.buffer[0].element_ptr(v+lv);
                g[0] += p[0];
                g[1] += p[1];
                g[2] += p[2];
            }
            g[0] /= GLfloat(nb_v);
            g[1] /= GLfloat(nb_v);
            g[2] /= GLfloat(nb_v);
            for(index_t lv=0; lv<nb_v; ++lv) {
                GLfloat* p = immediate_state_.buffer[0].element_ptr(v+lv);
                p[0] = s*g[0] + (1.0f - s)*p[0];
                p[1] = s*g[1] + (1.0f - s)*p[1];
                p[2] = s*g[2] + (1.0f - s)*p[2];                    
            }
            v += nb_v;
        }
    }
    
    void Context::create_CPU_side_uniform_buffer() {
        uniform_buffer_dirty_=true;
        matrices_dirty_=true;        
        uniform_binding_point_=1;

        std::map<std::string, GLsizei> type_to_size;
        type_to_size["bool"] = GLsizei(sizeof(int));
        type_to_size["vec4"] = GLsizei(4*sizeof(GLfloat));
        type_to_size["vec3"] = GLsizei(3*sizeof(GLfloat));
        type_to_size["mat4"] = GLsizei(4*4*sizeof(GLfloat));
        type_to_size["mat3"] = GLsizei(4*3*sizeof(GLfloat)); // yes, 4, there is padding
        type_to_size["float"] = GLsizei(sizeof(GLfloat));
        type_to_size["int"] = GLsizei(sizeof(GLint));

        // Parse uniform state declaration in order to "emulate" it...
        uniform_buffer_size_ = 0;
        std::istringstream input(uniform_state_declaration());
        std::string line;
        while(std::getline(input,line)) {
            std::vector<std::string> words;
            GEO::String::split_string(line, ' ', words);
            if(
                (words.size() == 2 && words[1][words[1].length()-1] == ';') ||
                (words.size() == 3 && words[2] == ";")
            ) {
                std::string vartype = words[0];
                std::string varname = words[1];
                if(varname[varname.length()-1] == ';') {
                    varname = varname.substr(0, varname.length()-1);
                }
                if(type_to_size.find(vartype) != type_to_size.end()) {
                    variable_to_offset_[varname] = uniform_buffer_size_;
                    uniform_buffer_size_ += type_to_size[vartype];
                }
            }
        }
        
        uniform_buffer_data_ = new Memory::byte[uniform_buffer_size_];
        Memory::clear(uniform_buffer_data_, size_t(uniform_buffer_size_));

        setup_state_variables();
        setup_immediate_buffers();
        setup_primitives();

        world_clip_plane_ = uniform_state_.world_clip_plane.get_pointer();
    }

    void Context::bind_immediate_state_buffers_to_VAO() {
        for(index_t i=0; i<ImmediateState::NB_IMMEDIATE_BUFFERS; ++i) {
            glBindBuffer(
                GL_ARRAY_BUFFER,
                immediate_state_.buffer[i].VBO()
            );
            glVertexAttribPointer(
                i,
                GLint(immediate_state_.buffer[i].dimension()),
                GL_FLOAT,
                GL_FALSE,
                0,      // stride
                nullptr // pointer (relative to bound VBO beginning)
            );
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void Context::classify_vertices_in_immediate_buffers() {
        if(!uniform_state_.toggle[GLUP_CLIPPING].get()) {
            return;
        }
        if(uniform_state_.clipping_mode.get() == GLUP_CLIP_STANDARD) {
            return;
        }
        if(
            immediate_state_.primitive() == GLUP_POINTS ||
            immediate_state_.primitive() == GLUP_LINES  ||
            immediate_state_.primitive() == GLUP_TRIANGLES ||
            immediate_state_.primitive() == GLUP_QUADS
        ) {
            return;
        }

        for(index_t v=0; v<immediate_state_.nb_vertices(); ++v) {
            float* p = immediate_state_.buffer[0].element_ptr(v);
            float s = 0.0;
            for(index_t i=0; i<4; ++i) {
                s += world_clip_plane_[i]*p[i];
            }
            v_is_visible_[v] = (s >= 0);
        }
    }

    
    bool Context::cell_is_clipped(index_t first_v) {
        if(!uniform_state_.toggle[GLUP_CLIPPING].get()) {
            return false;
        }
        if(uniform_state_.clipping_mode.get() == GLUP_CLIP_STANDARD) {
            return false;
        }
        index_t nb_visible=0;
        index_t nb_in_cell =
            nb_vertices_per_primitive[immediate_state_.primitive()];
        for(index_t lv=0; lv<nb_in_cell; ++lv) {
            nb_visible += (v_is_visible_[first_v + lv]);
        }
        switch(uniform_state_.clipping_mode.get()) {
        case GLUP_CLIP_STRADDLING_CELLS:
            return (nb_visible == 0 || nb_visible == nb_in_cell);
        case GLUP_CLIP_WHOLE_CELLS:
            return (nb_visible == 0);
        case GLUP_CLIP_SLICE_CELLS:
            return false;
        }
        return false;
    }

    void Context::copy_uniform_state_to_current_program() {
    }

    void Context::create_vertex_id_VBO() {

        glGenBuffers(1, &vertex_id_VBO_);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_id_VBO_);
        GLushort* indices = new GLushort[65536];
        for(index_t i=0; i<65536; ++i) {
            indices[i] = GLushort(i);
        }
        glBufferData(
            GL_ARRAY_BUFFER,
            GLsizeiptr(sizeof(GLushort)*65536),
            indices, GL_STATIC_DRAW
        );
        delete[] indices;
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Bind the index VBO to the common VAO used by the immediate state.
        if(immediate_state_.VAO() != 0) {
            glupBindVertexArray(immediate_state_.VAO());
            glBindBuffer(GL_ARRAY_BUFFER, vertex_id_VBO_);
            glVertexAttribPointer(
                GLUP_VERTEX_ID_ATTRIBUTE,
                1,                 // 1 component per attribute
                GL_UNSIGNED_SHORT, // components are 16 bits integers
                GL_FALSE,          // do not normalize
                0,                 // stride
                nullptr            // pointer (relative to bound VBO beginning)
            );
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glupBindVertexArray(0);
        }
        
    }

    void Context::get_vertex_shader_preamble_pseudo_file(
        std::vector<GLSL::Source>& sources
    ) {
        geo_argused(sources);
    }
    
    void Context::get_fragment_shader_preamble_pseudo_file(
        std::vector<GLSL::Source>& sources
    ) {
        geo_argused(sources);        
    }

    void Context::get_geometry_shader_preamble_pseudo_file(
        std::vector<GLSL::Source>& sources
    ) {
        geo_argused(sources);        
    }

    void Context::get_tess_control_shader_preamble_pseudo_file(
        std::vector<GLSL::Source>& sources
    ) {
        geo_argused(sources);        
    }

    void Context::get_tess_evaluation_shader_preamble_pseudo_file(
        std::vector<GLSL::Source>& sources
    ) {
        geo_argused(sources);        
    }
    
    void Context::get_toggles_pseudo_file(
        std::vector<GLSL::Source>& sources        
    ) {
        std::string toggle_enabled_source =
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
            toggle_enabled_source += test;
            if(toggles_source_undetermined_ & (1 << i)) {
                toggle_enabled_source +=
                    "      return GLUP."+toggle_var_name+";\n";
            } else {
                if(toggles_source_state_ & (1 << i)) {
                    toggle_enabled_source +=
                        "      return true;\n";
                } else {
                    toggle_enabled_source +=
                        "      return false;\n";
                }
            }
            toggle_enabled_source += "   }\n";
        }
        toggle_enabled_source += "   return false; \n}\n";
        sources.push_back(toggle_enabled_source);
    }

    void Context::get_primitive_pseudo_file(
        std::vector<GLSL::Source>& sources        
    ) {
        sources.push_back(primitive_declaration(primitive_source_));
    }

    void Context::get_marching_cells_pseudo_file(
        std::vector<GLSL::Source>& sources        
    ) {
        const MarchingCell& marching_cell = get_marching_cell();
        sources.push_back(
            marching_cell.GLSL_uniform_state_declaration()
        );
        sources.push_back(
            marching_cell.GLSL_compute_intersections()
        );
    }
    
    const MarchingCell& Context::get_marching_cell() const {
        const MarchingCell* result = nullptr;
        switch(primitive_source_) {
        case GLUP_TETRAHEDRA:
            result = &marching_tet_;
            break;
        case GLUP_HEXAHEDRA:
            result = &marching_hex_;
            break;            
        case GLUP_PRISMS:
            result = &marching_prism_;
            break;            
        case GLUP_PYRAMIDS:
            result = &marching_pyramid_;
            break;            
        case GLUP_CONNECTORS:
            result = &marching_connector_;
            break;            
        case GLUP_POINTS:
        case GLUP_LINES:
        case GLUP_TRIANGLES:
        case GLUP_QUADS:
	case GLUP_SPHERES:
        case GLUP_NB_PRIMITIVES:
            geo_assert_not_reached;
        }
        return *result;
    }

    void Context::initialize() {
        static bool initialized = false;
        if(initialized) {
            return;
        }

        GLSL::register_GLSL_include_file(
            "GLUP/current_profile/vertex_shader_preamble.h",
	    vertex_shader_preamble_pseudo_file
        );
            
        GLSL::register_GLSL_include_file(
            "GLUP/current_profile/fragment_shader_preamble.h",
	    fragment_shader_preamble_pseudo_file
        );
        
        GLSL::register_GLSL_include_file(
            "GLUP/current_profile/geometry_shader_preamble.h",
	    geometry_shader_preamble_pseudo_file
        );

        GLSL::register_GLSL_include_file(
            "GLUP/current_profile/marching_cells.h",
	    marching_cells_pseudo_file
        );
            
        GLSL::register_GLSL_include_file(
            "GLUP/current_profile/tess_control_shader_preamble.h",
	    tess_control_shader_preamble_pseudo_file
        );

        GLSL::register_GLSL_include_file(
            "GLUP/current_profile/tess_evaluation_shader_preamble.h",
	    tess_evaluation_shader_preamble_pseudo_file
        );

        GLSL::register_GLSL_include_file(
            "GLUP/current_profile/toggles.h",
	    toggles_pseudo_file
        );

        GLSL::register_GLSL_include_file(
            "GLUP/current_profile/primitive.h",
	    primitive_pseudo_file
        );
            
        GLUP::register_embedded_shaders_GLUP();
        
        initialized = true;
    }
    
}

