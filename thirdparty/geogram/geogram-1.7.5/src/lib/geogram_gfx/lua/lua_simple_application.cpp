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

#include <geogram_gfx/lua/lua_simple_application.h>
#include <geogram_gfx/gui/simple_application.h>
#include <geogram/lua/lua_wrap.h>
#include <geogram/basic/stopwatch.h>

namespace {
    using namespace GEO;

    namespace LUAGLUPVIEWERImpl {
	static double t0 = 0.0;
	
	static int ElapsedTime(lua_State* L) {
	    if(lua_gettop(L) != 0) {
		return luaL_error(
		    L, "'GLUP.ElapsedTime()' invalid number of arguments"
		);
	    }
	    double result = 0.0;
	    result = GEO::SystemStopwatch::now() - t0;
	    lua_pushnumber(L,double(result));    
	    return 1;
	}

	static int ResetViewer(lua_State* L) {
	    if(lua_gettop(L) != 0) {
		return luaL_error(
		    L, "'GLUP.ResetViewer()' invalid number of arguments"
		);
	    }
	    
	    GEO::SimpleApplication* app = GEO::SimpleApplication::instance();
	    if(app != nullptr) {
		app->home();
		app->set_lighting(true);
	    }
	    
	    t0 = GEO::SystemStopwatch::now();
	    return 0;
	}

	static int ArcadeStyle(lua_State* L) {
	    if(lua_gettop(L) != 0) {
		return luaL_error(
		    L, "'GLUP.ArcadeStyle()' invalid number of arguments"
		);
	    }

	    GEO::SimpleApplication* app = GEO::SimpleApplication::instance(); 
	    if(app != nullptr) {
		app->home();
		app->set_lighting(false);
		app->set_background_color(vec4f(0.0, 0.0, 0.0, 1.0));
	    }
	    
	    return 0;    
	}

	static int SetRegionOfInterest(lua_State* L) {
	    if(lua_gettop(L) != 6) {
		return luaL_error(
		    L,
		    "'GLUP.SetRegionOfInterest()' invalid number of arguments"
		);
	    }
	    if(
		!lua_isnumber(L,1) ||
		!lua_isnumber(L,2) ||
		!lua_isnumber(L,3) ||
		!lua_isnumber(L,4) ||	
		!lua_isnumber(L,5) ||
		!lua_isnumber(L,6) 
	    ) {
		return luaL_error(
		    L,
		    "'GLUP.SetRegionOfInterest()' arguments should be numbers"
		);
	    }
	    GEO::SimpleApplication* app = GEO::SimpleApplication::instance();
	    if(app != nullptr) {
		app->set_region_of_interest(
		    lua_tonumber(L,1),
		    lua_tonumber(L,2),
		    lua_tonumber(L,3),
		    lua_tonumber(L,4),
		    lua_tonumber(L,5),
		    lua_tonumber(L,6)	
		);
	    }
	    return 0;
	}

	static int GetRegionOfInterest(lua_State* L) {
	    if(lua_gettop(L) != 0) {
		return luaL_error(
		    L,
		    "'GLUP.GetRegionOfInterest()' invalid number of arguments"
		);
	    }

	    GEO::SimpleApplication* app = GEO::SimpleApplication::instance();
	    if(app != nullptr) {
		double xm,ym,zm,xM,yM,zM;		
		app->get_region_of_interest(
		    xm, ym, zm, xM, yM, zM
		);
		lua_pushnumber(L,xm);
		lua_pushnumber(L,ym);
		lua_pushnumber(L,zm);
		lua_pushnumber(L,xM);
		lua_pushnumber(L,yM);
		lua_pushnumber(L,zM);            
	    }	    
	    return 6;
	}
    }
}
    
void init_lua_simple_application(lua_State* L) {
    lua_getglobal(L,"GLUP");
    geo_assert(!lua_isnil(L,-1)); // Make sure GLUP was registered before.
    GEO::lua_bindwrapper(L,LUAGLUPVIEWERImpl::ElapsedTime);
    GEO::lua_bindwrapper(L,LUAGLUPVIEWERImpl::ResetViewer);
    GEO::lua_bindwrapper(L,LUAGLUPVIEWERImpl::ArcadeStyle);
    GEO::lua_bindwrapper(L,LUAGLUPVIEWERImpl::SetRegionOfInterest);
    GEO::lua_bindwrapper(L,LUAGLUPVIEWERImpl::GetRegionOfInterest);
    lua_pop(L,1);
}

