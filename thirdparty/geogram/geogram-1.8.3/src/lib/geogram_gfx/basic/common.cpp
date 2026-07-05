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

#include <geogram_gfx/basic/common.h>
#include <geogram_gfx/basic/GLSL.h>
#include <geogram_gfx/GLUP/GLUP_context.h>
#include <geogram/basic/logger.h>
#include <cstdlib>

#ifdef GEO_OS_EMSCRIPTEN
#include <GLFW/glfw3.h>

extern "C" {
    using namespace GEO;

    /*****************/

    // Functions used by Dear Imgui but not implemented by Emscripten's GLFW
    // (declare dummy stubs)
    
    struct GLFWgamepadstate;
    
    int glfwGetError(const char** description);
    int glfwGetGamepadState(int jid, GLFWgamepadstate* state);
    void glfwGetMonitorContentScale (
        GLFWmonitor *monitor, float *xscale, float *yscale
    );
    void glfwGetMonitorWorkarea (
        GLFWmonitor *monitor, int* xpos, int* ypos, int* width, int* height
    );
    void glfwSetWindowAttrib(GLFWwindow* window, int attrib, int value);
    void glfwSetWindowOpacity(GLFWwindow *window, float opacity);

    /*****************/
    
    int glfwGetError(const char** description) {
        geo_argused(description);
        return 0; // GLFW_NO_ERROR
    }

    int glfwGetGamepadState(int jid, GLFWgamepadstate* state) {
        geo_argused(jid);
        geo_argused(state);
        return GLFW_FALSE;
    }

    void glfwGetMonitorContentScale (
        GLFWmonitor *monitor, float *xscale, float *yscale
    ) {
        geo_argused(monitor);
        if(xscale != nullptr) {
            *xscale = 1.0f;
        }
        if(yscale != nullptr) {
            *yscale = 1.0f;
        }
    }

    void glfwGetMonitorWorkarea (
        GLFWmonitor *monitor, int* xpos, int* ypos, int* width, int* height
    ) {
        geo_argused(monitor);
        if(xpos != nullptr) {
            *xpos = 0;
        }
        if(ypos != nullptr) {
            *ypos = 0;
        }
        if(width != nullptr) {
            *width = 1024;
        }
        if(height != nullptr) {
            *height = 1024;
        }
    }

    void glfwSetWindowAttrib(GLFWwindow* window, int attrib, int value) {
        geo_argused(window);
        geo_argused(attrib);
        geo_argused(value);
    }

    void glfwSetWindowOpacity(GLFWwindow *window, float opacity) {
        geo_argused(window);
        geo_argused(opacity);
    }
}
#endif


namespace GEO {
    
    namespace Graphics {

        void initialize() {
#if !defined(GEO_OS_EMSCRIPTEN) && !defined(GEO_OS_ANDROID)
            if(!gladLoadGL()) {
                Logger::err("GLAD") << "Could not load OpenGL"
                                   << std::endl;
            }
#endif
            GL::initialize();
            GLSL::initialize();
        }

        void terminate() {
            GLSL::terminate();
            GL::terminate();
        }
        
    }
    
}
