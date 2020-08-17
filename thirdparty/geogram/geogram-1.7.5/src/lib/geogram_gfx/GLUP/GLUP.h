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

#ifndef GEOGRAM_GFX_GLUP_GLUP
#define GEOGRAM_GFX_GLUP_GLUP

#include <geogram_gfx/api/defs.h>
#define GLUP_API GEOGRAM_GFX_API

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * \file geogram_gfx/GLUP/GLUP.h
     * \brief GLUP: GL Useful Primitives
     */

    /**
     * \name GLUP base types
     * @{ 
     */

    typedef float GLUPfloat;
    typedef double GLUPdouble;
    typedef int GLUPint;
    typedef unsigned int GLUPuint;    
    typedef int GLUPsizei;
    typedef unsigned int GLUPenum;
    typedef unsigned long int GLUPuint64;
    typedef void GLUPvoid;
    typedef int GLUPbitfield;
    
    enum {
        GLUP_FALSE=0, GLUP_TRUE = 1
    };

    typedef unsigned char GLUPboolean;
    
    /**
     * @}
     */

    /************************************************/
    
    /**
     * \name GLUP context manipulation
     * @{ 
     */

    /**
     * \brief Gets the GLSL declaration of GLUP uniform state.
     * \return a pointer to GLSL source code that declares 
     *  GLUP uniform state.
     * \details Can be used by client-code shaders that need to
     *  have access to the GLUP uniform state.
     */
    GLUP_API const char* glupUniformStateDeclaration(void);

    /**
     * \brief Compiles a GLSL shader.
     * \param[in] target one of GL_VERTEX_SHADER, GL_FRAGMENT_SHADER,
     *  GL_COMPUTE_SHADER, GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER,
     *  GL_GEOMETRY_SHADER.
     * \return an opaque handle to the compiled shader, or 0 if an error
     *  occured.
     */
    GLUP_API GLUPuint glupCompileShader(GLUPenum target, const char* source);

    /**
     * \brief Compiles a GLSL program.
     * \param[in] source the program source. Shader stages are indicated
     *  by special comments:
     *  //stage GL_VERTEX_SHADER
     *  //stage GL_FRAGMENT_SHADER
     *  //stage GL_COMPUTE_SHADER
     *  //stage GL_TESS_CONTROL_SHADER
     *  //stage GL_TESS_EVALUATION_SHADER
     *  //stage GL_GEOMETRY_SHADER
     * \return an opaque handle to the compiled program, or 0 if an error
     *  occured.
     */
    GLUP_API GLUPuint glupCompileProgram(const char* source);
    
    /**
     * \brief Opaque identifier of a GLUP context.
     */
    typedef void* GLUPcontext;

    /**
     * \brief Creates a new GLUP context.
     * \details OpenGL needs to be already initialized when this function
     *  is called (there needs to be a valid active OpenGL context).
     * \return the newly created GLUPcontext
     */
    GLUPcontext GLUP_API glupCreateContext(void);

    /**
     * \brief Deletes a GLUP context.
     * \param[in] context the GLUPcontext to be deleted
     */
    void GLUP_API glupDeleteContext(GLUPcontext context);

    /**
     * \brief Gets the current GLUP context.
     * \return the current GLUP context
     */
    GLUPcontext GLUP_API glupCurrentContext(void);

    /**
     * \brief The name of the profile implemented by the
     *  current context.
     * \return a const pointer to the string with the name
     *  of the current profile.
     */
    GLUP_API const char* glupCurrentProfileName(void);
    
    /**
     * \brief Makes a GLUP context the current one.
     * \param[in] context the GLUP context to be made current
     */
    void GLUP_API glupMakeCurrent(GLUPcontext context);

    /**
     * \brief Binds the GLUP uniform state to a program.
     * \param[in] program the handle to the GLSL program.
     * \details It is necessary to call this function for
     *  client programs that need to access the GLUP uniform state.
     */
    void GLUP_API glupBindUniformState(GLUPuint program);
    
    /**
     * @}
     */

    /************************************************/
    
    /**
     * \name GLUP enable / disable
     * @{ 
     */

    typedef enum {
        GLUP_LIGHTING           =0,
        GLUP_VERTEX_COLORS      =1,
        GLUP_TEXTURING          =2,
        GLUP_DRAW_MESH          =3,
        GLUP_CLIPPING           =4,
        GLUP_INDIRECT_TEXTURING =5,
	GLUP_VERTEX_NORMALS     =6,	
        GLUP_PICKING            =7,
	GLUP_ALPHA_DISCARD      =8,
	GLUP_NORMAL_MAPPING     =9
    } GLUPtoggle;

    void GLUP_API glupEnable(GLUPtoggle toggle);
    void GLUP_API glupDisable(GLUPtoggle toggle);
    GLUPboolean GLUP_API glupIsEnabled(GLUPtoggle toggle);
    
    /**
     * @}
     */

    /************************************************/

    /**
     * \name GLUP texturing
     * @{ 
     */


    /**
     * \brief The texture units used by GLUP for 1D,2D and 3D
     *  texturing.
     * \details Before binding the texture, call 
     *   glActiveTexture(GL_TEXTURE0 + unit) before binding the texture.
     *   For instance, for 2D texturing, call 
     *   glActiveTexture(GL_TEXTURE0 + GLUP_TEXTURE_2D_UNIT).
     */
    enum {
        GLUP_TEXTURE_1D_UNIT=0,
        GLUP_TEXTURE_2D_UNIT=1,
        GLUP_TEXTURE_3D_UNIT=2
    };


    /**
     * \brief The targets to be used as the first 
     *  argument of glBindTexture() for GLUP 1D, 2D
     *  and 3D textures.
     * \details we do not use 1D textures, because
     *  they are not supported by all OpenGL versions.
     *  Both targets for 1D and 2D textures are GL_TEXTURE_2D.
     */
    enum {
        GLUP_TEXTURE_1D_TARGET=0x0DE1,
        GLUP_TEXTURE_2D_TARGET=0x0DE1,
        GLUP_TEXTURE_3D_TARGET=0x806F
    };

    typedef enum {
        GLUP_TEXTURE_1D=1,
        GLUP_TEXTURE_2D=2,
        GLUP_TEXTURE_3D=3
    } GLUPtextureType;

    void GLUP_API glupTextureType(GLUPtextureType type);
    GLUPtextureType GLUP_API glupGetTextureType(void);
    
    typedef enum {
        GLUP_TEXTURE_REPLACE=0,
        GLUP_TEXTURE_MODULATE=1,
        GLUP_TEXTURE_ADD=2
    } GLUPtextureMode;

    void GLUP_API glupTextureMode(GLUPtextureMode mode);
    GLUPtextureMode GLUP_API glupGetTextureMode(void);
    
    /**
     * @}
     */

    /************************************************/
    
    
    /**
     * \name GLUP drawing state
     * @{ 
     */

    
    typedef enum {
        GLUP_FRONT_COLOR=0,
        GLUP_BACK_COLOR=1,
        GLUP_MESH_COLOR=2,
        GLUP_FRONT_AND_BACK_COLOR=3
    } GLUPcolor;

    void GLUP_API glupSetColor4fv(GLUPcolor color, const GLUPfloat* rgba);
    void GLUP_API glupGetColor4fv(GLUPcolor color, float* rgba);    
    
    void GLUP_API glupSetColor3fv(GLUPcolor color, const GLUPfloat* rgba);
    void GLUP_API glupSetColor4f(
        GLUPcolor color, GLUPfloat r, GLUPfloat g, GLUPfloat b, GLUPfloat a
    );
    void GLUP_API glupSetColor3f(
        GLUPcolor color, GLUPfloat r, GLUPfloat g, GLUPfloat b
    );
    void GLUP_API glupSetColor4dv(GLUPcolor color, const GLUPdouble* rgba);
    void GLUP_API glupSetColor3dv(GLUPcolor color, const GLUPdouble* rgba); 
    void GLUP_API glupSetColor4d(
        GLUPcolor color, GLUPdouble r, GLUPdouble g, GLUPdouble b, GLUPdouble a
    );
    void GLUP_API glupSetColor3d(
        GLUPcolor color, GLUPdouble r, GLUPdouble g, GLUPdouble b
    );

    void GLUP_API glupLightVector3f(GLUPfloat x, GLUPfloat y, GLUPfloat z);
    void GLUP_API glupLightVector3fv(GLUPfloat* xyz);
    void GLUP_API glupGetLightVector3fv(GLUPfloat* xyz);

    void GLUP_API glupSetPointSize(GLUPfloat size);
    GLUPfloat GLUP_API glupGetPointSize(void);
    
    void GLUP_API glupSetMeshWidth(GLUPint width);
    GLUPint GLUP_API glupGetMeshWidth(void);

    void GLUP_API glupSetCellsShrink(GLUPfloat x);
    GLUPfloat GLUP_API glupGetCellsShrink(void);

    void GLUP_API glupSetAlphaThreshold(GLUPfloat x);
    GLUPfloat GLUP_API glupGetAlphaThreshold(void);

    void GLUP_API glupSetSpecular(GLUPfloat x);
    GLUPfloat GLUP_API glupGetSpecular(void);
    
    /**
     * @}
     */
    
    /************************************************/
    
    /**
     * \name GLUP picking
     * @{
     */

    typedef enum {
        GLUP_PICK_PRIMITIVE=1,
        GLUP_PICK_CONSTANT=2
    } GLUPpickingMode;

    void GLUP_API glupPickingMode(GLUPpickingMode mode);
    GLUPpickingMode GLUP_API glupGetPickingMode(void);

    void GLUP_API glupPickingId(GLUPuint64 id);
    GLUPuint64 GLUP_API glupGetPickingId(void);

    void GLUP_API glupBasePickingId(GLUPuint64 id);
    GLUPuint64 GLUP_API glupGetBasePickingId(void);
    
    /**
     * @}
     */

    /************************************************/
    
    /**
     * \name GLUP clipping
     * @{
     */
    
    typedef enum {
        GLUP_CLIP_STANDARD=0,
        GLUP_CLIP_WHOLE_CELLS=1,
        GLUP_CLIP_STRADDLING_CELLS=2,
        GLUP_CLIP_SLICE_CELLS=3
    } GLUPclipMode;

    /**
     * \brief Sets the current clipping mode.
     * \param[in] mode one of GLUP_CLIP_STANDARD,
     *  GLUP_CLIP_WHOLE_CELLS, GLUP_CLIP_STRADDLING_CELLS,
     *  GLUP_CLIP_SLICE_CELLS (not implemented yet).
     */
    void GLUP_API glupClipMode(GLUPclipMode mode);

    /**
     * \brief Gets the clipping mode.
     * \return the current clipping mode
     */
    GLUPclipMode GLUP_API glupGetClipMode(void);

    /**
     * \brief Defines the plane used by GLUP clipping.
     * \details The specified plane equation is pre-multiplied
     *  by the inverse of the current modelview matrix before 
     *  being stored in the state (like in the old fixed-functionality
     *  pipeline of OpenGL).
     * \param[in] eqn a pointer to 4 doubles with the equantion of the 
     *  clipping plane.
     */
    void GLUP_API glupClipPlane(const GLUPdouble* eqn);

    /**
     * \brief Gets the current clipping plane.
     * \param[out] eqn a pointer to 4 doubles where to store the
     *  current clipping plane equation.
     */
    void GLUP_API glupGetClipPlane(GLUPdouble* eqn);
    
    /**
     * @}
     */

    /************************************************/    
    
    /**
     * \name GLUP matrices
     * @{ 
     */

    typedef enum {
        GLUP_MODELVIEW_MATRIX = 0,
        GLUP_PROJECTION_MATRIX = 1,
        GLUP_TEXTURE_MATRIX = 2
    } GLUPmatrix;

    void GLUP_API glupMatrixMode(GLUPmatrix matrix);
    GLUPmatrix GLUP_API glupGetMatrixMode(void);
    void GLUP_API glupPushMatrix(void);
    void GLUP_API glupPopMatrix(void);

    /**
     * \brief Queries a GLUP matrix.
     * \param[in] matrix symbolic name of the matrix, one of
     *   GLUP_PROJECTION_MATRIX, GLUP_MODELVIEW_MATRIX, GLUP_TEXTURE_MATRIX
     * \param[out] ptr a pointer to an array of 16 doubles, where to store
     *   the current matrix on top of the specified stack.
     */
    void GLUP_API glupGetMatrixdv(GLUPmatrix matrix, GLUPdouble* ptr);

    /**
     * \brief Queries a GLUP matrix.
     * \param[in] matrix symbolic name of the matrix, one of
     *   GLUP_PROJECTION_MATRIX, GLUP_MODELVIEW_MATRIX, GLUP_TEXTURE_MATRIX
     * \param[out] ptr a pointer to an array of 16 doubles, where to store
     *   the current matrix on top of the specified stack.
     */
    void GLUP_API glupGetMatrixfv(GLUPmatrix matrix, GLUPfloat* ptr);
    
    void GLUP_API glupLoadIdentity(void);
    void GLUP_API glupLoadMatrixf(const GLUPfloat* M);
    void GLUP_API glupLoadMatrixd(const GLUPdouble* M);    
    void GLUP_API glupMultMatrixf(const GLUPfloat* M);
    void GLUP_API glupMultMatrixd(const GLUPdouble* M);    
    void GLUP_API glupTranslatef(GLUPfloat x, GLUPfloat y, GLUPfloat z);
    void GLUP_API glupTranslated(GLUPdouble x, GLUPdouble y, GLUPdouble z);
    void GLUP_API glupScalef(GLUPfloat sx, GLUPfloat sy, GLUPfloat sz);
    void GLUP_API glupScaled(GLUPdouble sx, GLUPdouble sy, GLUPdouble sz);
    void GLUP_API glupRotatef(
        GLUPfloat angle, GLUPfloat x, GLUPfloat y, GLUPfloat z
    );
    void GLUP_API glupRotated(
        GLUPdouble angle, GLUPdouble x, GLUPdouble y, GLUPdouble z
        );
    void GLUP_API glupOrtho(
        GLUPdouble left, GLUPdouble right,
        GLUPdouble bottom, GLUPdouble top,
        GLUPdouble nearVal, GLUPdouble farVal
    );
    void GLUP_API glupOrtho2D(
        GLUPdouble left, GLUPdouble right, GLUPdouble bottom, GLUPdouble top
    );
    void GLUP_API glupFrustum(
        GLUPdouble left, GLUPdouble right,
        GLUPdouble bottom, GLUPdouble top,
        GLUPdouble nearVal, GLUPdouble farVal
    ); 
    void GLUP_API glupPerspective(
        GLUPdouble fovy, GLUPdouble aspect,
        GLUPdouble zNear, GLUPdouble zFar
    );

    GLUPint GLUP_API glupProject(
        GLUPdouble objx, GLUPdouble objy, GLUPdouble objz,
        const GLUPdouble modelMatrix[16],
        const GLUPdouble projMatrix[16],
        const GLUPint viewport[4],
        GLUPdouble* winx,  GLUPdouble* winy,  GLUPdouble* winz
    );
    
    GLUPboolean GLUP_API glupUnProject(
        GLUPdouble winx, GLUPdouble winy, GLUPdouble winz,
        const GLUPdouble modelMatrix[16],
        const GLUPdouble projMatrix[16],
        const GLUPint viewport[4],
        GLUPdouble *objx, GLUPdouble *objy, GLUPdouble *objz
    );

    GLUPboolean GLUP_API glupInvertMatrixfv(
        GLUPfloat Minvert[16],        
        const GLUPfloat M[16]
    );

    GLUPboolean GLUP_API glupInvertMatrixdv(
        GLUPdouble Minvert[16],        
        const GLUPdouble M[16]
    );
    
    /**
     * @}
     */

    /************************************************/    
    
    /**
     * \name GLUP drawing functions
     * @{ 
     */
    
    /**
     * \brief Symbolic values corresponding to GLUP
     *  primitive types.
     * \see glupBegin(), glupEnd(), glupDrawArrays(), glupDrawElements()
     */
    typedef enum {
        GLUP_POINTS     =0,
        GLUP_LINES      =1,
        GLUP_TRIANGLES  =2,
        GLUP_QUADS      =3,
        GLUP_TETRAHEDRA =4,
        GLUP_HEXAHEDRA  =5,
        GLUP_PRISMS     =6,
        GLUP_PYRAMIDS   =7,
        GLUP_CONNECTORS =8,
	GLUP_SPHERES    =9,
        GLUP_NB_PRIMITIVES = 10
    } GLUPprimitive;

    /**
     * \brief Tests whether a given GLUP primitive supports array mode.
     * \details If array mode is supported, then one can use glupDrawArray()
     *  and glupDrawElements() with the specified primitive.
     * \param[in] prim the primitive to be tested.
     * \retval GLUP_TRUE if array mode is supported with \p prim
     * \retval GLUP_FALSE otherwise
     */
    GLUPboolean GLUP_API glupPrimitiveSupportsArrayMode(GLUPprimitive prim);
    
    void GLUP_API glupDrawArrays(
        GLUPprimitive primitive, GLUPint first, GLUPsizei count
    );
    
    void GLUP_API glupDrawElements(
        GLUPprimitive primitive, GLUPsizei count,
        GLUPenum type, const GLUPvoid* indices
    );

    void GLUP_API glupBegin(GLUPprimitive primitive);
    void GLUP_API glupEnd(void);
    
    void GLUP_API glupVertex2fv(const GLUPfloat* xy);
    void GLUP_API glupVertex3fv(const GLUPfloat* xyz);
    void GLUP_API glupVertex4fv(const GLUPfloat* xyzw);

    void GLUP_API glupVertex2dv(const GLUPdouble* xy);
    void GLUP_API glupVertex3dv(const GLUPdouble* xyz);
    void GLUP_API glupVertex4dv(const GLUPdouble* xyzw);

    void GLUP_API glupVertex2f(GLUPfloat x, GLUPfloat y);        
    void GLUP_API glupVertex3f(GLUPfloat x, GLUPfloat y, GLUPfloat z);    
    void GLUP_API glupVertex4f(
        GLUPfloat x, GLUPfloat y, GLUPfloat z, GLUPfloat w
    );

    void GLUP_API glupVertex2d(GLUPdouble x, GLUPdouble y);        
    void GLUP_API glupVertex3d(GLUPdouble x, GLUPdouble y, GLUPdouble z);    
    void GLUP_API glupVertex4d(
        GLUPdouble x, GLUPdouble y, GLUPdouble z, GLUPdouble w
    );

    void GLUP_API glupColor3fv(const GLUPfloat* rgb);
    void GLUP_API glupColor4fv(const GLUPfloat* rgba);

    void GLUP_API glupColor3dv(const GLUPdouble* rgb);
    void GLUP_API glupColor4dv(const GLUPdouble* rgba);

    void GLUP_API glupColor3f(GLUPfloat r, GLUPfloat g, GLUPfloat b);    
    void GLUP_API glupColor4f(
        GLUPfloat r, GLUPfloat g, GLUPfloat b, GLUPfloat a
    );

    void GLUP_API glupColor3d(GLUPdouble r, GLUPdouble g, GLUPdouble b);    
    void GLUP_API glupColor4d(
        GLUPdouble r, GLUPdouble g, GLUPdouble b, GLUPdouble a
    );

    void GLUP_API glupTexCoord2fv(const GLUPfloat* st);
    void GLUP_API glupTexCoord3fv(const GLUPfloat* stu);
    void GLUP_API glupTexCoord4fv(const GLUPfloat* stuv);

    void GLUP_API glupTexCoord2dv(const GLUPdouble* st);
    void GLUP_API glupTexCoord3dv(const GLUPdouble* stu);
    void GLUP_API glupTexCoord4dv(const GLUPdouble* stuv);

    void GLUP_API glupTexCoord1f(GLUPfloat s);
    void GLUP_API glupTexCoord2f(GLUPfloat s, GLUPfloat t);        
    void GLUP_API glupTexCoord3f(GLUPfloat s, GLUPfloat t, GLUPfloat u);    
    void GLUP_API glupTexCoord4f(
        GLUPfloat s, GLUPfloat t, GLUPfloat u, GLUPfloat v
    );

    void GLUP_API glupTexCoord1d(GLUPdouble s);            
    void GLUP_API glupTexCoord2d(GLUPdouble s, GLUPdouble t);        
    void GLUP_API glupTexCoord3d(GLUPdouble s, GLUPdouble t, GLUPdouble u);    
    void GLUP_API glupTexCoord4d(
        GLUPdouble s, GLUPdouble t, GLUPdouble u, GLUPdouble v
    );


    void GLUP_API glupNormal3fv(GLUPfloat* xyz);
    void GLUP_API glupNormal3f(GLUPfloat x, GLUPfloat y, GLUPfloat z);    

    void GLUP_API glupNormal3dv(GLUPdouble* xyz);
    void GLUP_API glupNormal3d(GLUPdouble x, GLUPdouble y, GLUPdouble z);    
    
    
    /**
     * \brief Specifies a GLSL program to be used for drawing the primitives.
     * \details Can be used with both immediate mode (glupBegin()/glupEnd())
     *  and array mode (glupDrawArrays(), glupDrawElements()). If the specified
     *  program is non-zero, then it is used instead of the default GLUP 
     *  program. To access the GLUP uniform state in the program, one may
     *  append the result of glupUniformStateDeclaration() to the GLSL source
     *  of the program.
     * \param[in] program the id of the GLSL program to be used.
     */
    void GLUP_API glupUseProgram(GLUPuint program);
    
    /**
     * @}
     */

    /************************************************/    
    
    /**
     * \name GLUP Vertex Array Object wrapper or emulation.
     * @{ 
     */

    /**
     * \brief Generate vertex array object names.
     * \details This function is a wrapper around glGenVertexArrays() 
     *  if it is supported, else it emulates it.
     * \param[in] n the number of vertex array object names to generate. 
     * \param[in] arrays an array in which the generated vertex array 
     *   object names are stored. 
     */
    void GLUP_API glupGenVertexArrays(GLUPsizei n, GLUPuint* arrays);

    /**
     * \brief Deletes vertex array objects.
     * \details This function is a wrapper around glDeleteVertexArrays() 
     *  if it is supported, else it emulates it.
     * \param[in] n the number of vertex array objects to be deleted. 
     * \param[in] arrays the address of an array containing the \p n 
     *   names of the objects to be deleted. 
     */
    void GLUP_API glupDeleteVertexArrays(GLUPsizei n, const GLUPuint *arrays);


    /**
     * \brief Binds a vertex array object.
     * \details This function is a wrapper around glBindVertexArray() 
     *  if it is supported, else it emulates it.
     * \param[in] array the name of the vertex array to bind. 
     */
    void GLUP_API glupBindVertexArray(GLUPuint array);

    /**
     * \brief Gets the name of the bound vertex array object.
     * \details This function uses glGet(GL_VERTEX_ARRAY_BINDING) if
     *  vertex array objects are supported, else it emulates it.
     * \return the name of the bound vertex array object or 0
     *  if no vertex array object is bound.
     */
    GLUPuint GLUP_API glupGetVertexArrayBinding(void);

    /**
     * @}
     */

#ifdef __cplusplus
}
#endif

#endif
