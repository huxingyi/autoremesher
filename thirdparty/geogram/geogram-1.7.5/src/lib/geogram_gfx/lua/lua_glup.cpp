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

#include <geogram_gfx/lua/lua_glup.h>
#include <geogram_gfx/GLUP/GLUP.h>
#include <geogram/basic/geometry.h>
#include <geogram/lua/lua_wrap.h>

extern "C" {
#include <geogram/third_party/lua/lauxlib.h>
#include <geogram/third_party/lua/lualib.h>
}

#include <map>
#include <string>


namespace {
    using namespace GEO;
    
    namespace LUAGLUPImpl {

	/**
	 * \brief Gets a string that represents the name of a GLUPmatrix.
	 * \param[in] L a pointer to the LUA state
	 * \param[in] m one of GLUP_MODELVIEW_MATRIX, GLUP_PROJECTION_MATRIX,
	 *  GLUP_TEXTURE_MATRIX
	 * \return one of "modelview", "projection", "texture"
	 */
	static const char* luaglup_matrixname(
	    lua_State* L, GLUPmatrix m
	) {
	    geo_argused(L);
	    const char* result = nullptr;
	    switch(m) {
		case GLUP_MODELVIEW_MATRIX:
		    result = "modelview";
		    break;
		case GLUP_PROJECTION_MATRIX:
		    result = "projection";
		    break;
		case GLUP_TEXTURE_MATRIX:
		    result = "texture";
		    break;
	    }
	    return result;
	}

	/**
	 * \brief Tentatively calls glupPushMatrix(). If a stack overflow 
	 *  is detected, the function does nothing and returns false.
	 * \details The function keeps track of the current matrix depth
	 *  using variables stored in the LUA registry.
	 * \param[in] L a pointer to the LUA state
	 * \retval true if glupPushMatrix() could be successfully called.
	 * \retval false otherwise
	 */
	static bool luaglup_trypushmatrix(lua_State* L) {
	    const index_t luaglup_max_matrix_depth = 13;
	    GLUPmatrix m = glupGetMatrixMode();	    
	    const char* matrix_name = luaglup_matrixname(L,m);
	    lua_getfield(L,LUA_REGISTRYINDEX,"GLUP");
	    lua_getfield(L, -1, matrix_name);
	    index_t depth = index_t(lua_tointeger(L, -1));
	    if(depth >= luaglup_max_matrix_depth) {
		lua_pop(L,2);
		return false;
	    }
	    lua_pop(L,1);
	    lua_pushinteger(L,lua_Integer(depth+1));
	    lua_setfield(L, -2, matrix_name);
	    lua_pop(L,1);
	    glupPushMatrix();
	    return true;
	}

	/**
	 * \brief Tentatively calls glupPopMatrix(). If a stack underflow
	 *  is detected, the function does nothing and returns false.
	 * \details The function keeps track of the current matrix depth
	 *  using variables stored in the LUA registry.
	 * \param[in] L a pointer to the LUA state
	 * \retval true if glupPopMatrix() could be successfully called.
	 * \retval false otherwise
	 */
	static bool luaglup_trypopmatrix(lua_State* L) {
	    GLUPmatrix m = glupGetMatrixMode();	    	    
	    const char* matrix_name = luaglup_matrixname(L,m);
	    lua_getfield(L,LUA_REGISTRYINDEX,"GLUP");
	    lua_getfield(L, -1, matrix_name);
	    index_t depth = index_t(lua_tointeger(L,-1));
	    if(depth == 0) {
		lua_pop(L,2);
		return false;
	    }
	    lua_pop(L,1);
	    lua_pushinteger(L,lua_Integer(depth-1));
	    lua_setfield(L, -2, matrix_name);
	    lua_pop(L,1);
	    glupPopMatrix();
	    return true;
	}

	/**
	 * \brief Pops matrices from the specified GLUP matrix stack
	 *  until there is no more matrix on the stack.
	 * \details The function keeps track of the current matrix depth
	 *  using variables stored in the LUA registry.
	 * \param[in] L a pointer to the LUA state
	 * \param[in] m one of GLUP_MODELVIEW_MATRIX, GLUP_PROJECTION_MATRIX,
	 *  GLUP_TEXTURE_MATRIX
	 */
	static void luaglup_adjustmatrixstack(lua_State* L, GLUPmatrix m) {
	    const char* matrix_name = luaglup_matrixname(L,m);
	    lua_getfield(L,LUA_REGISTRYINDEX,"GLUP");
	    lua_getfield(L, -1, matrix_name);
	    index_t depth = index_t(lua_tointeger(L,-1));
	    lua_pop(L,1);
	    if(depth != 0) {
		GLUPmatrix mode_save = glupGetMatrixMode();
		glupMatrixMode(m);
		while(depth != 0) {
		    glupPopMatrix();
		    --depth;
		}
		lua_pushinteger(L,lua_Integer(depth));
		lua_setfield(L,-2,matrix_name);
		glupMatrixMode(mode_save);
	    }
	    lua_pop(L,1);
	}

	/**
	 * \brief Tests whether the current GLUP state is
	 *  between a glupBegin()/glupEnd() pair.
	 * \details The function keeps track of the current
	 *  state using a variable stored in the LUA registry.
	 * \param[in] L a pointer to the LUA state.
	 * \retval true if glupBegin() was called (and glupEnd()
	 *  was not called yet).
	 * \retval false otherwise.
	 */
	static bool luaglup_primitiveactive(lua_State* L) {
	    lua_getfield(L,LUA_REGISTRYINDEX,"GLUP");
	    lua_getfield(L, -1, "primitive_active");
	    bool result = (lua_toboolean(L,-1) != 0);
	    lua_pop(L,2);
	    return result;
	}

	/**
	 * \brief Specifies to LUA that glupBegin() was called or
	 *  that glupEnd() was called.
	 * \details The function keeps track of the current
	 *  state using a variable stored in the LUA registry.
	 * \param[in] L a pointer to the LUA state.
	 * \param[in] x true if glupBegin() was called (set the primitive_active
	 *  flag), false if glupEnd() was called (reset the primitive_active
	 *  flag).
	 */
	static void luaglup_setprimitiveactive(lua_State* L, bool x) {
	    lua_getfield(L,LUA_REGISTRYINDEX,"GLUP");
	    lua_pushboolean(L, x ? 1 : 0);
	    lua_setfield(L, -2, "primitive_active");
	    lua_pop(L,1);
	}

	
#define DECLARE_GLUP_CONSTANT(C)     \
	lua_pushliteral(L,#C);	     \
	lua_pushinteger(L,GLUP_##C); \
	lua_settable(L,1)

	// Idem: this one could be stored in the registry, using
	// a LUA table. 
	static std::map<std::string, GEO::vec4> lua_glup_colormap;

	static void DECLARE_GLUP_COLOR(
	    const char* name, double r, double g, double b, double a=1.0
	) {
	    lua_glup_colormap[name] = GEO::vec4(r,g,b,a);
	}

	inline bool get_vec4(lua_State* L, double* xyzw, int pos=1) {
	    int nargs = lua_gettop(L);
	    xyzw[0] = 0.0;
	    xyzw[1] = 0.0;
	    xyzw[2] = 0.0;
	    xyzw[3] = 1.0;
	    
	    if(nargs == pos && lua_isstring(L,pos)) {
		const char* name = lua_tostring(L,pos);
		auto it = lua_glup_colormap.find(name);
		if(it == lua_glup_colormap.end()) {
		    return false;
		}
		xyzw[0] = it->second.x;
		xyzw[1] = it->second.y;
		xyzw[2] = it->second.z;
		xyzw[3] = it->second.w;	
	    } else {
		if(nargs > pos+3 || nargs < pos) {
		    return false;
		}
		if(nargs == pos+3) {
		    if(!lua_isnumber(L,pos+3)) {
			return false;
		    }
		    xyzw[3] = lua_tonumber(L,pos+3);
		}
		if(nargs >= pos+2) {
		    if(!lua_isnumber(L,pos+2)) {
			return false;
		    }
		    xyzw[2] = lua_tonumber(L,pos+2);
		}
		if(nargs >= pos+1) {
		    if(!lua_isnumber(L,pos+1)) {
			return false;
		    }
		    xyzw[1] = lua_tonumber(L,pos+1);
		}
		if(nargs >= pos) {
		    if(!lua_isnumber(L,pos)) {
			return false;
		    }
		    xyzw[0] = lua_tonumber(L,pos);
		}
	    }
	    return true;
	}

	static int SetColor(lua_State* L) {
	    if(lua_gettop(L) < 2) {
		return luaL_error(
		    L, "'GLUP.SetColor()' invalid number of arguments"
		);
	    }
	    if(!lua_isinteger(L,1)) {
		return luaL_error(
		    L, "'GLUP.SetColor()' argument should be an integer"
		);
	    }
	    lua_Integer color = lua_tointeger(L,1);
	    double rgba[4];
	    if(!get_vec4(L,rgba,2)) {
		return luaL_error(
		    L, "'GLUP.SetColor()' invalid arguments"
		);
	    }
	    glupSetColor4dv(GLUPcolor(color),rgba);
	    return 0;
	}

	static int GetColor(lua_State* L) {
	    if(lua_gettop(L) != 1) {
		return luaL_error(
		    L, "'GLUP.GetColor()' invalid number of arguments"
		);
	    }
	    if(!lua_isinteger(L,1)) {
		return luaL_error(
		    L, "'GLUP.GetColor()' argument should be an integer"
		);
	    }
	    lua_Integer color = lua_tointeger(L,1);
	    float rgba[4];
	    glupGetColor4fv(GLUPcolor(color),rgba);
	    lua_pushnumber(L,lua_Number(rgba[0]));
	    lua_pushnumber(L,lua_Number(rgba[1]));
	    lua_pushnumber(L,lua_Number(rgba[2]));
	    lua_pushnumber(L,lua_Number(rgba[3]));    
	    return 4;
	}

	static int ClipPlane(lua_State* L) {
	    if(lua_gettop(L) != 4) {
		return luaL_error(
		    L, "'GLUP.ClipPlane()' invalid number of arguments"
		);
	    }
	    if(
		!lua_isnumber(L,1) ||
		!lua_isnumber(L,2) ||
		!lua_isnumber(L,3) ||
		!lua_isnumber(L,4) 	
		) {
		return luaL_error(
		    L, "'GLUP.ClipPlane()' invalid arguments"
		);
	    }
	    double eqn[4];
	    eqn[0] = lua_tonumber(L,1);
	    eqn[1] = lua_tonumber(L,2);
	    eqn[2] = lua_tonumber(L,3);
	    eqn[3] = lua_tonumber(L,4);
	    glupClipPlane(eqn);
	    return 0;
	}

	static int GetClipPlane(lua_State* L) {
	    if(lua_gettop(L) != 0) {
		return luaL_error(
		    L, "'GLUP.GetClipPlane()' invalid number of arguments"
		);
	    }
	    double eqn[4];
	    glupGetClipPlane(eqn);
	    lua_pushnumber(L,eqn[0]);
	    lua_pushnumber(L,eqn[1]);
	    lua_pushnumber(L,eqn[2]);
	    lua_pushnumber(L,eqn[3]);    
	    return 4;
	}

	static int PushMatrix(lua_State* L) {
	    if(lua_gettop(L) != 0) {
		return luaL_error(
		    L, "'GLUP.PushMatrix()' invalid number of arguments"
		);
	    }
	    if(!luaglup_trypushmatrix(L)) {
		GLUPmatrix m = glupGetMatrixMode();		
		const char* matrix_name = luaglup_matrixname(L,m);
		return luaL_error(
		    L, (std::string("'GLUP.PushMatrix()' ") +
			matrix_name + ":stack overflow").c_str()
		);
	    }
	    return 0;
	}

	static int PopMatrix(lua_State* L) {
	    if(lua_gettop(L) != 0) {
		return luaL_error(
		    L, "'GLUP.PopMatrix()' invalid number of arguments"
		);
	    }
	    if(!luaglup_trypopmatrix(L)) {
		GLUPmatrix m = glupGetMatrixMode();	    
		const char* matrix_name = luaglup_matrixname(L,m);
		return luaL_error(
		    L, (std::string("'GLUP.PopMatrix()' ") +
			matrix_name + ":stack underflow").c_str()
		);
	    }
	    return 0;
	}

	static int GetMatrix(lua_State* L) {
	    if(lua_gettop(L) != 1) {
		return luaL_error(
		    L, "'GLUP.GetMatrix()' invalid number of arguments"
		);
	    }
	    if(!lua_isinteger(L,1)) {
		return luaL_error(
		    L, "'GLUP.GetMatrix()' invalid argument"
		);
	    }
	    double M[16];
	    glupGetMatrixdv(GLUPmatrix(lua_tointeger(L,1)),M);
	    for(int i=0; i<16; ++i) {
		lua_pushnumber(L,M[i]);
	    }
	    return 16;
	}

	
	static int LoadMatrix(lua_State* L) {
	    if(lua_gettop(L) != 16) {
		return luaL_error(
		    L, "'GLUP.LoadMatrix()' invalid number of arguments"
		);
	    }
	    double M[16];
	    for(int i=0; i<16; ++i) {
		if(!lua_isnumber(L,i+1)) {
		    return luaL_error(
			L, "'GLUP.LoadMatrix()' invalid argument"
		    );
		}
		M[i] = lua_tonumber(L,i+1);
	    }
	    glupLoadMatrixd(M);
	    return 0;
	}

	static int MultMatrix(lua_State* L) {
	    if(lua_gettop(L) != 16) {
		return luaL_error(
		    L, "'GLUP.MultMatrix()' invalid number of arguments"
		);
	    }
	    double M[16];
	    for(int i=0; i<16; ++i) {
		if(!lua_isnumber(L,i+1)) {
		    return luaL_error(
			L, "'GLUP.MultMatrix()' invalid argument"
		    );
		}
		M[i] = lua_tonumber(L,i+1);
	    }
	    glupMultMatrixd(M);
	    return 0;
	}

	static int Begin(lua_State* L) {
	    if(lua_gettop(L) != 1) {
		return luaL_error(
		    L, "'GLUP.Begin()' invalid number of arguments"
		);
	    }
	    if(!lua_isinteger(L,1)) {
		return luaL_error(
		    L, "'GLUP.Begin()' argument should be an integer"
		);
	    }
	    if(luaglup_primitiveactive(L)) {
		return luaL_error(
		    L, "'GLUP.Begin()' called twice without GLUP.End()"
		);
	    }
	    luaglup_setprimitiveactive(L,true);
	    lua_Integer prim = lua_tointeger(L,1);
	    glupBegin(GLUPprimitive(prim));
	    return 0;
	}
	
	static int End(lua_State* L) {
	    if(lua_gettop(L) != 0) {
		return luaL_error(
		    L, "'GLUP.End()' invalid number of arguments"
		);
	    }
	    if(!luaglup_primitiveactive(L)) {
		return luaL_error(
		    L, "'GLUP.End()' called without GLUP.Begin()"
		);
	    }
	    luaglup_setprimitiveactive(L,false);
	    glupEnd();
	    return 0;
	}

	static int Vertex(lua_State* L) {
	    double xyzw[4];
	    if(!get_vec4(L,xyzw)) {
		return luaL_error(
		    L, "'GLUP.Vertex()' invalid arguments"
		);
	    }
	    glupVertex4dv(xyzw);
	    return 0;
	}

	static int Color(lua_State* L) {
	    double rgba[4];
	    if(!get_vec4(L,rgba)) {
		return luaL_error(
		    L, "'GLUP.Color()' invalid arguments"
		);
	    }
	    glupColor4dv(rgba);
	    return 0;
	}

	static int TexCoord(lua_State* L) {
	    double xyzw[4];
	    if(!get_vec4(L,xyzw)) {
		return luaL_error(
		    L, "'GLUP.TexCoord()' invalid arguments"
		);
	    }
	    glupTexCoord4dv(xyzw);
	    return 0;
	}

	static int Normal(lua_State* L) {
	    double xyzw[4];
	    if(!get_vec4(L,xyzw)) {
		return luaL_error(
		    L, "'GLUP.Normal()' invalid arguments"
		);
	    }
	    glupNormal3dv(xyzw);
	    return 0;
	}
    }
}

/**
 * \brief Directly registers an automatically generated
 *  LUA wrapper for a GLUP function.
 * \param[in] F the name of the function without the
 *  "glup" prefix
 */
#define DECLARE_GLUP_FUNC(F) \
    GEO::lua_bindwrapperwithname(L,glup##F,#F)    

/**
 * \brief Registers an automatically generated LUA wrapper 
 *  for a GLUP function and change its name declared to LUA.
 * \param[in] F the C++ name of the function (with prefix and
 *  namespace)
 * \param[in] N a string with the name under which the function
 *  will be registered to LUA.
 */
#define DECLARE_GLUP_FUNC_WITH_NAME(F,N) \
    GEO::lua_bindwrapperwithname(L,F,N)    

/**
 * \brief Registers a specifically written function or LUA adapter
 *  to LUA. The generated name will be the name of the adapter with
 *  namespaces removed.
 * \param[in] F the C++ name of the function (with namespace).
 */
#define DECLARE_GLUP_ADAPTER(F) \
    GEO::lua_bindwrapper(L,F)

//   All used enum types need to be declared,
// this creates some specializations of the
// templates that communicate with the LUA
// stack.
//  TODO: select the right functions using std::is_enum<>
namespace GEO {
    LUA_DECLAREENUMTYPE(GLUPtoggle);
    LUA_DECLAREENUMTYPE(GLUPtextureType);
    LUA_DECLAREENUMTYPE(GLUPtextureMode);
    LUA_DECLAREENUMTYPE(GLUPpickingMode);
    LUA_DECLAREENUMTYPE(GLUPclipMode);
    LUA_DECLAREENUMTYPE(GLUPmatrix);                        
}

void init_lua_glup(lua_State* L) {
    using namespace LUAGLUPImpl;
    using namespace GEO;

    //  Create the table used to store the matrix stack depths
    // and begin/end status (both are used to recover from
    // client-code errors).
    lua_newtable(L);
    lua_setfield(L,LUA_REGISTRYINDEX,"GLUP");
    
    lua_newtable(L);

    // Enable/Disable constants
    DECLARE_GLUP_CONSTANT(LIGHTING);
    DECLARE_GLUP_CONSTANT(VERTEX_COLORS);
    DECLARE_GLUP_CONSTANT(DRAW_MESH);
    DECLARE_GLUP_CONSTANT(CLIPPING);
    DECLARE_GLUP_CONSTANT(INDIRECT_TEXTURING);
    DECLARE_GLUP_CONSTANT(VERTEX_NORMALS);
    DECLARE_GLUP_CONSTANT(PICKING);
    DECLARE_GLUP_CONSTANT(ALPHA_DISCARD);

    // Colors
    DECLARE_GLUP_CONSTANT(FRONT_COLOR);
    DECLARE_GLUP_CONSTANT(BACK_COLOR);
    DECLARE_GLUP_CONSTANT(MESH_COLOR);
    DECLARE_GLUP_CONSTANT(FRONT_AND_BACK_COLOR);

    // Picking
    DECLARE_GLUP_CONSTANT(PICK_PRIMITIVE);
    DECLARE_GLUP_CONSTANT(PICK_CONSTANT);

    // Clipping
    DECLARE_GLUP_CONSTANT(CLIP_STANDARD);
    DECLARE_GLUP_CONSTANT(CLIP_WHOLE_CELLS);
    DECLARE_GLUP_CONSTANT(CLIP_STRADDLING_CELLS);
    DECLARE_GLUP_CONSTANT(CLIP_SLICE_CELLS);  

    // Matrices
    DECLARE_GLUP_CONSTANT(MODELVIEW_MATRIX);
    DECLARE_GLUP_CONSTANT(PROJECTION_MATRIX);
    DECLARE_GLUP_CONSTANT(TEXTURE_MATRIX);  

    // Primitives
    DECLARE_GLUP_CONSTANT(POINTS);
    DECLARE_GLUP_CONSTANT(LINES);
    DECLARE_GLUP_CONSTANT(TRIANGLES);
    DECLARE_GLUP_CONSTANT(QUADS);
    DECLARE_GLUP_CONSTANT(TETRAHEDRA);
    DECLARE_GLUP_CONSTANT(HEXAHEDRA);
    DECLARE_GLUP_CONSTANT(PRISMS);
    DECLARE_GLUP_CONSTANT(PYRAMIDS);
    DECLARE_GLUP_CONSTANT(CONNECTORS);
    DECLARE_GLUP_CONSTANT(SPHERES);  

    // GLUP Functions
    // There are three cases:
    // 1) GLUP function is directly "wrappable"
    //    -> DECLARE_GLUP_FUNC
    // 2) GLUP function is "wrappable" but needs to be renamed
    //    -> DECLARE_GLUP_FUNC_WITH_NAME
    // 3) GLUP function needs a specific wrapper, written in
    //    the LUAGLUPImpl namespace
    //    -> DECLARE_GLUP_ADAPTER
    
    DECLARE_GLUP_FUNC(Enable);
    DECLARE_GLUP_FUNC(Disable);
    DECLARE_GLUP_FUNC(IsEnabled);
    DECLARE_GLUP_ADAPTER(SetColor);
    DECLARE_GLUP_ADAPTER(GetColor);
    DECLARE_GLUP_FUNC_WITH_NAME(glupLightVector3f,"LightVector");
    DECLARE_GLUP_FUNC(SetPointSize);
    DECLARE_GLUP_FUNC(GetPointSize);
    DECLARE_GLUP_FUNC(SetMeshWidth);
    DECLARE_GLUP_FUNC(GetMeshWidth);
    DECLARE_GLUP_FUNC(SetCellsShrink);
    DECLARE_GLUP_FUNC(GetCellsShrink);
    DECLARE_GLUP_FUNC(SetAlphaThreshold);
    DECLARE_GLUP_FUNC(GetAlphaThreshold);
    DECLARE_GLUP_FUNC(PickingMode);
    DECLARE_GLUP_FUNC(GetPickingMode);
    DECLARE_GLUP_FUNC(PickingId);
    DECLARE_GLUP_FUNC(GetPickingId);
    DECLARE_GLUP_FUNC(BasePickingId);
    DECLARE_GLUP_FUNC(GetBasePickingId);
    DECLARE_GLUP_FUNC(ClipMode);
    DECLARE_GLUP_FUNC(GetClipMode);
    DECLARE_GLUP_ADAPTER(ClipPlane);
    DECLARE_GLUP_ADAPTER(GetClipPlane);
    DECLARE_GLUP_FUNC(MatrixMode);
    DECLARE_GLUP_FUNC(GetMatrixMode);
    DECLARE_GLUP_ADAPTER(PushMatrix);
    DECLARE_GLUP_ADAPTER(PopMatrix);
    DECLARE_GLUP_ADAPTER(GetMatrix);
    DECLARE_GLUP_FUNC(LoadIdentity);
    DECLARE_GLUP_ADAPTER(LoadMatrix);
    DECLARE_GLUP_ADAPTER(MultMatrix);
    DECLARE_GLUP_FUNC_WITH_NAME(glupTranslated,"Translate");
    DECLARE_GLUP_FUNC_WITH_NAME(glupScaled,"Scale");
    DECLARE_GLUP_FUNC_WITH_NAME(glupRotated,"Rotate");      
    DECLARE_GLUP_ADAPTER(Begin);
    DECLARE_GLUP_ADAPTER(End);
    DECLARE_GLUP_ADAPTER(Vertex);
    DECLARE_GLUP_ADAPTER(Color);
    DECLARE_GLUP_ADAPTER(TexCoord);
    DECLARE_GLUP_ADAPTER(Normal);

    DECLARE_GLUP_COLOR("black", 0.0, 0.0, 0.0);
    DECLARE_GLUP_COLOR("white", 1.0, 1.0, 1.0);
    DECLARE_GLUP_COLOR("gray",  0.5, 0.5, 0.5);
    DECLARE_GLUP_COLOR("red", 1.0, 0.0, 0.0);
    DECLARE_GLUP_COLOR("green", 0.0, 1.0, 0.0);
    DECLARE_GLUP_COLOR("blue", 0.0, 0.0, 1.0);
    DECLARE_GLUP_COLOR("yellow", 1.0, 1.0, 0.0);
    DECLARE_GLUP_COLOR("pink", 1.0, 0.75, 0.793);            
    DECLARE_GLUP_COLOR("brown",0.6445,0.164,0.164);

    DECLARE_GLUP_FUNC(SetSpecular);
    DECLARE_GLUP_FUNC(GetSpecular);    
    
    lua_setglobal(L, "GLUP");

    
}

void adjust_lua_glup_state(lua_State* L) {
    using namespace LUAGLUPImpl;

    if(glupCurrentContext() == nullptr) {
	return;
    }
    
    if(luaglup_primitiveactive(L)) {
	glupEnd();
	luaglup_setprimitiveactive(L,false);
    }

    luaglup_adjustmatrixstack(L,GLUP_MODELVIEW_MATRIX);
    luaglup_adjustmatrixstack(L,GLUP_PROJECTION_MATRIX);
    luaglup_adjustmatrixstack(L,GLUP_TEXTURE_MATRIX);    
}

