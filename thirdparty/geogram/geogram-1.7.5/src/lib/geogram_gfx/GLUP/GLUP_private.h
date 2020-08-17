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

#ifndef GEOGRAM_GFX_GLUP_GLUP_PRIVATE
#define GEOGRAM_GFX_GLUP_GLUP_PRIVATE

#include <geogram_gfx/GLUP/GLUP.h>
#include <geogram_gfx/GLUP/GLUP_context.h>

/**
 * \file geogram_gfx/GLUP/GLUP_private.h
 * \brief Slightly faster GLUP functions, not part of the API (do not use in
 *  client code).
 */


namespace GLUP {
    using namespace GEO;
    extern GLUP_API Context* current_context_;
}

inline void glupPrivateBegin(GLUPprimitive primitive) {
    GLUP::current_context_->begin(primitive);
}

inline void glupPrivateEnd() {
    GLUP::current_context_->end();
}

inline void glupPrivateVertex2fv(const GLUPfloat* xy) {
    GLUP::current_context_->immediate_vertex(xy[0], xy[1]);
}

inline void glupPrivateVertex3fv(const GLUPfloat* xyz) {
    GLUP::current_context_->immediate_vertex(xyz[0], xyz[1], xyz[2]);    
}

inline void glupPrivateVertex4fv(const GLUPfloat* xyzw) {
    GLUP::current_context_->immediate_vertex(
        xyzw[0], xyzw[1], xyzw[2], xyzw[3]
    );        
}

inline void glupPrivateVertex2dv(const GLUPdouble* xy) {
    GLUP::current_context_->immediate_vertex(
        GLfloat(xy[0]),
        GLfloat(xy[1])
    );
}

inline void glupPrivateVertex3dv(const GLUPdouble* xyz) {
    GLUP::current_context_->immediate_vertex(
        GLfloat(xyz[0]),
        GLfloat(xyz[1]),
        GLfloat(xyz[2])        
    );
}

inline void glupPrivateVertex4dv(const GLUPdouble* xyzw) {
    GLUP::current_context_->immediate_vertex(
        GLfloat(xyzw[0]),
        GLfloat(xyzw[1]),
        GLfloat(xyzw[2]),
        GLfloat(xyzw[3])                
    );
}

inline void glupPrivateVertex2f(GLUPfloat x, GLUPfloat y) {
    GLUP::current_context_->immediate_vertex(x,y);    
}        

inline void glupPrivateVertex3f(GLUPfloat x, GLUPfloat y, GLUPfloat z) {
    GLUP::current_context_->immediate_vertex(x,y,z);        
}    

inline void glupPrivateVertex4f(
    GLUPfloat x, GLUPfloat y, GLUPfloat z, GLUPfloat w
) {
    GLUP::current_context_->immediate_vertex(x,y,z,w);            
}

inline void glupPrivateVertex2d(GLUPdouble x, GLUPdouble y) {
    GLUP::current_context_->immediate_vertex(
        GLfloat(x),
        GLfloat(y)
    );
}        

inline void glupPrivateVertex3d(GLUPdouble x, GLUPdouble y, GLUPdouble z) {
    GLUP::current_context_->immediate_vertex(
        GLfloat(x),
        GLfloat(y),
        GLfloat(z)
    );
}    

inline void glupPrivateVertex4d(
    GLUPdouble x, GLUPdouble y, GLUPdouble z, GLUPdouble w
) {
    GLUP::current_context_->immediate_vertex(
        GLfloat(x),
        GLfloat(y),
        GLfloat(z),
        GLfloat(w)                
    );
}

inline void glupPrivateColor3fv(const GLUPfloat* rgb) {
    GLUP::current_context_->immediate_color(rgb[0], rgb[1], rgb[2]);
}

inline void glupPrivateColor4fv(const GLUPfloat* rgba) {
    GLUP::current_context_->immediate_color(rgba[0], rgba[1], rgba[2], rgba[3]);
}

inline void glupPrivateColor3dv(const GLUPdouble* rgb) {
    GLUP::current_context_->immediate_color(
        GLfloat(rgb[0]),
        GLfloat(rgb[1]),
        GLfloat(rgb[2])
    );    
}

inline void glupPrivateColor4dv(const GLUPdouble* rgba) {
    GLUP::current_context_->immediate_color(
        GLfloat(rgba[0]),
        GLfloat(rgba[1]),
        GLfloat(rgba[2]),
        GLfloat(rgba[3])        
    );    
}

inline void glupPrivateColor3f(GLUPfloat r, GLUPfloat g, GLUPfloat b) {
    GLUP::current_context_->immediate_color(r, g, b);    
}    

inline void glupPrivateColor4f(
    GLUPfloat r, GLUPfloat g, GLUPfloat b, GLUPfloat a
) {
    GLUP::current_context_->immediate_color(r, g, b, a);        
}

inline void glupPrivateColor3d(GLUPdouble r, GLUPdouble g, GLUPdouble b) {
    GLUP::current_context_->immediate_color(
        GLfloat(r),
        GLfloat(g),
        GLfloat(b)
    );    
}    

inline void glupPrivateColor4d(
    GLUPdouble r, GLUPdouble g, GLUPdouble b, GLUPdouble a
) {
    GLUP::current_context_->immediate_color(
        GLfloat(r),
        GLfloat(g),
        GLfloat(b),
        GLfloat(a)
    );    
}

inline void glupPrivateTexCoord2fv(const GLUPfloat* st) {
    GLUP::current_context_->immediate_tex_coord(st[0], st[1]);    
}

inline void glupPrivateTexCoord3fv(const GLUPfloat* stu) {
    GLUP::current_context_->immediate_tex_coord(stu[0], stu[1], stu[2]);        
}

inline void glupPrivateTexCoord4fv(const GLUPfloat* stuv) {
    GLUP::current_context_->immediate_tex_coord(
        stuv[0], stuv[1], stuv[2], stuv[3]
    );            
}

inline void glupPrivateTexCoord2dv(const GLUPdouble* st) {
    GLUP::current_context_->immediate_tex_coord(
        GLfloat(st[0]),
        GLfloat(st[1])
    );            
}

inline void glupPrivateTexCoord3dv(const GLUPdouble* stu) {
    GLUP::current_context_->immediate_tex_coord(
        GLfloat(stu[0]),
        GLfloat(stu[1]),
        GLfloat(stu[2])
    );            
}

inline void glupPrivateTexCoord4dv(const GLUPdouble* stuv) {
    GLUP::current_context_->immediate_tex_coord(
        GLfloat(stuv[0]),
        GLfloat(stuv[1]),
        GLfloat(stuv[2]),
        GLfloat(stuv[3])
    );            
}

inline void glupPrivateTexCoord1f(GLUPfloat s) {
    GLUP::current_context_->immediate_tex_coord(s);        
}

inline void glupPrivateTexCoord2f(GLUPfloat s, GLUPfloat t) {
    GLUP::current_context_->immediate_tex_coord(s,t);        
}        

inline void glupPrivateTexCoord3f(GLUPfloat s, GLUPfloat t, GLUPfloat u) {
    GLUP::current_context_->immediate_tex_coord(s,t,u);        
}    

inline void glupPrivateTexCoord4f(
    GLUPfloat s, GLUPfloat t, GLUPfloat u, GLUPfloat v
) {
    GLUP::current_context_->immediate_tex_coord(s,t,u,v);    
}

inline void glupPrivateTexCoord1d(GLUPdouble s) {
    GLUP::current_context_->immediate_tex_coord(
        GLfloat(s)
    );        
}            

inline void glupPrivateTexCoord2d(GLUPdouble s, GLUPdouble t) {
    GLUP::current_context_->immediate_tex_coord(
        GLfloat(s),
        GLfloat(t)
    );        
}        

inline void glupPrivateTexCoord3d(GLUPdouble s, GLUPdouble t, GLUPdouble u) {
    GLUP::current_context_->immediate_tex_coord(
        GLfloat(s),
        GLfloat(t),
        GLfloat(u)
    );        
}    

inline void glupPrivateTexCoord4d(
    GLUPdouble s, GLUPdouble t, GLUPdouble u, GLUPdouble v
) {
    GLUP::current_context_->immediate_tex_coord(
        GLfloat(s),
        GLfloat(t),
        GLfloat(u),
        GLfloat(v)
    );        
}


inline void glupPrivateNormal3fv(GLUPfloat* xyz) {
    GLUP::current_context_->immediate_normal(
        xyz[0],xyz[1],xyz[2]
    );
}

inline void glupPrivateNormal3f(GLUPfloat x, GLUPfloat y, GLUPfloat z) {
    GLUP::current_context_->immediate_normal(
        x,y,z
    );
}    

inline void glupPrivateNormal3dv(GLUPdouble* xyz) {
    GLUP::current_context_->immediate_normal(
        GLfloat(xyz[0]),
	GLfloat(xyz[1]),
	GLfloat(xyz[2])
    );
}

inline void glupPrivateNormal3d(GLUPdouble x, GLUPdouble y, GLUPdouble z) {
    GLUP::current_context_->immediate_normal(
        GLfloat(x),
        GLfloat(y),
        GLfloat(z)
    );
}    

#endif
