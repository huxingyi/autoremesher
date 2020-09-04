/**
 * Macros and functions for facilitating interfacing
 * ImGui with WebGL when used with emscripten.
 * This file is not part of the initial ImGui distribution.
 * [Bruno Levy 11/26/2017]
 */

#ifndef H_GLUP_COMPAT_H
#define H_GLUP_COMPAT_H

#include <geogram_gfx/basic/GL.h> 
#include <geogram_gfx/basic/GLSL.h>
#include <geogram_gfx/GLUP/GLUP.h>

#ifndef GL_VERTEX_ARRAY_BINDING
#  define GL_VERTEX_ARRAY_BINDING 0x85B5
#endif

#ifdef glBindVertexArray
#  undef glBindVertexArray
#endif
#define glBindVertexArray glupBindVertexArray

#ifdef glGenVertexArrays
#  undef glGenVertexArrays
#endif
#define glGenVertexArrays glupGenVertexArrays

namespace {
   inline void glup_glGetIntegerv(GLenum name, GLint* value) {
      if(name == GL_VERTEX_ARRAY_BINDING) {
	 *value = glupGetVertexArrayBinding();
      } else {
	 glGetIntegerv(name, value);
      }
   }
}
   
#ifdef glGetIntegerv
#  undef glGetIntegerv
#endif
#define glGetIntegerv glup_glGetIntegerv
   


#endif // Include guard
