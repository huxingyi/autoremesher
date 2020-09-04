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

#include <geogram/lua/lua_io.h>
#include <geogram/lua/lua_wrap.h>
#include <geogram/basic/file_system.h>
#include <geogram/basic/logger.h>
#include <geogram/bibliography/bibliography.h>
#include <map>
#include <sstream>

namespace {
    using namespace GEO;

    std::map<std::string, const char*> embedded_files;

    /**
     * \brief Calls LUA's "require" function, that
     *  was previously saved in the "GEO" table in
     *  the registry.
     * \param[in] L a pointer to the LUA state.
     * \return the number of return values pushed onto
     *  the stack.
     */
    int call_lua_require(lua_State* L) {
	lua_getfield(L,LUA_REGISTRYINDEX,"GEO");
	lua_getfield(L, -1, "lua_require_func");
	lua_pushvalue(L,-3); 
	lua_call(L,1,1);
	return 1;
    }

    /**
     * \brief Our own implementation of "require".
     * \details First looks-up the file within the
     *  embedded LUA files, then calls LUA's
     *  require function if not found.
     * \param[in] L a pointer to the LUA state.
     * \return the number of return values pushed onto
     *  the stack.
     */
    int require(lua_State* L) {
	if(lua_gettop(L) != 1) {
	    return luaL_error(
		L, "'require()' invalid number of arguments"
	    );
	}
	if(!lua_isstring(L,1)) {
	    return luaL_error(
		L, "'require()' argument is not a string"
	    );
	}
	auto it = embedded_files.find(
	    std::string("lib/") + std::string(lua_tostring(L,1)) + ".lua"
	);
	if(it == embedded_files.end()) {
	    return call_lua_require(L);
	}
	const char* source = it->second;
	if(luaL_dostring(L,source)) {
	    const char* msg = lua_tostring(L,-1);
	    GEO::Logger::err("LUA") << msg << std::endl;
	}
	return 0;
    }

    /************************************************************************/
    
    /**
     * \brief Calls LUA's "tostring" function, that
     *  was previously saved in the "GEO" table in
     *  the registry.
     * \param[in] L a pointer to the LUA state.
     * \return the number of return values pushed onto
     *  the stack.
     */
    int call_lua_tostring(lua_State* L) {
	lua_getfield(L,LUA_REGISTRYINDEX,"GEO");
	lua_getfield(L, -1, "lua_tostring_func");
	lua_pushvalue(L,-3); 
	lua_call(L,1,1);
	return 1;
    }

    inline void append(std::string& s, const char* c) {
	s += (c==nullptr) ? "nil" : c;
    }
    
    int tostring(lua_State* L) {
	if(lua_gettop(L) != 1) {
	    return luaL_error(
		L, "'tostring()' invalid number of arguments"
	    );
	}
	if(lua_type(L,1) == LUA_TTABLE) {
	    std::string result = "{";
	    // Traverse the table (see LUA API doc. example).
	    lua_pushnil(L);  // first key
	    bool first = true;
	    while (lua_next(L, 1) != 0) {
		if(!first) {
		    result += ", ";
		}

		// We need to copy the key before calling
		// tostring(), else this may modify the
		// table in-place, which we do not want to do !
		lua_pushvalue(L,-2);
		
		append(result,lua_tostring(L,-1));
		
		lua_pop(L,1);
		
		result += "=";

		// We need to copy the value before calling
		// tostring(), else this may modify the
		// table in-place, which we do not want to do !
		lua_pushvalue(L,-1);		
		append(result,lua_tostring(L,-1));
		lua_pop(L,1);
		
		lua_pop(L,1);
		first = false;
	    }
	    result += "}";
	    lua_pushstring(L,result.c_str());
	    return 1;
	}
	return call_lua_tostring(L);
    }
    
    /************************************************************************/
    
    /**
     * \brief Our own implementation of "print".
     * \details Redirects all outputs to geogram outputs.
     * \param[in] L a pointer to the LUA state.
     * \return the number of return values pushed onto
     *  the stack.
     */
    int print(lua_State* L) {
	std::ostringstream out;
	int nargs = lua_gettop(L);	
	lua_getglobal(L, "tostring");
	for(int i=1; i<=nargs; ++i) {
	    const char *s;
	    size_t l;
	    lua_pushvalue(L, -1);  // function to be called. 
	    lua_pushvalue(L, i);   // value to print. 
	    lua_call(L, 1, 1);
	    s = lua_tolstring(L, -1, &l);  // get result.
	    if (s == nullptr) {
		return luaL_error(
		    L, "'tostring' must return a string to 'print'"
		);
	    }
	    if (i>1) {
		out << "\t";
	    }
	    out << s;
	    lua_pop(L, 1);  // pop result. 
	}
	Logger::out("LUA") << out.str() << std::endl;
	return 0;
    }

    namespace LUAFileSystemImpl {

	// These three functions needed adaptation, because in the FileSystem
	// API they take the returned vector as a reference argument.
	// The following function return it, so that the generic LUA wrapper
	// mechanism (lua_wrap.h) understands it and sends it back to LUA.
	// Note: recursive mode not implemented yet (it is bugged in
	// FileSystem, to be fixed...)

	
	static std::vector<std::string> get_directory_entries(
	    const std::string& path
	) {
	    std::vector<std::string> result;
	    FileSystem::get_directory_entries(path,result);
	    return result;
	}

	static std::vector<std::string> get_files(const std::string& path) {
	    std::vector<std::string> result;
	    FileSystem::get_files(path,result);
	    return result;
	}

	static std::vector<std::string> get_subdirectories(
	    const std::string& path
	) {
	    std::vector<std::string> result;
	    FileSystem::get_subdirectories(path,result);
	    return result;
	}

	static const char* os_name() {
	    const char* result = "unknown";
#if defined(GEO_OS_LINUX)
	    result = "Linux";
#elif defined(GEO_OS_APPLE)
	    result = "Apple";
#elif defined(GEO_OS_WINDOWS)
	    result = "Windows";
#elif defined(GEO_OS_ANDROID)
	    result = "Android";
#elif defined(GEO_OS_UNIX)
	    result = "Generic Unix";
#endif
	    return result;
	}
	
    }
}


void register_embedded_lua_file(
    const char* filename, const char* data
) {
    embedded_files[std::string(filename)] = data;
}

void list_embedded_lua_files(
    std::vector<std::string>& filenames
) {
    filenames.clear();
    for(auto& it : embedded_files) {
	filenames.push_back(it.first);
    }
}

void get_embedded_lua_file(
    const char* filename, const char** data
) {
    auto it = embedded_files.find(filename);
    *data = (it == embedded_files.end()) ? nullptr : it->second;
}
    
void init_lua_io(lua_State* L) {
    
    geo_cite_with_info(
	"WEB:lua",
	"GEOGRAM has an embedded LUA interpreter "
	"(LUA is worth one thousand times the few kilobytes it uses !)."
    );
    
    lua_newtable(L);

    lua_bindwrapper(L, FileSystem::is_file);
    lua_bindwrapper(L, FileSystem::is_directory);
    lua_bindwrapper(L, FileSystem::create_directory);
    lua_bindwrapper(L, FileSystem::delete_directory);
    lua_bindwrapper(L, FileSystem::delete_file);
    lua_bindwrapper(L, FileSystem::get_current_working_directory);
    lua_bindwrapper(L, FileSystem::set_current_working_directory);
    lua_bindwrapper(L, FileSystem::rename_file);
    lua_bindwrapper(L, FileSystem::get_time_stamp);
    lua_bindwrapper(L, FileSystem::extension);
    lua_bindwrapper(L, FileSystem::base_name);
    lua_bindwrapper(L, FileSystem::dir_name);
    lua_bindwrapper(L, FileSystem::copy_file);
    lua_bindwrapper(L, FileSystem::set_executable_flag);
    lua_bindwrapper(L, FileSystem::touch);
    lua_bindwrapper(L, FileSystem::normalized_path);		
    lua_bindwrapper(L, FileSystem::home_directory);
    lua_bindwrapper(L, FileSystem::documents_directory);	    
    
    lua_bindwrapper(L, LUAFileSystemImpl::get_directory_entries);
    lua_bindwrapper(L, LUAFileSystemImpl::get_files);
    lua_bindwrapper(L, LUAFileSystemImpl::get_subdirectories);		

    lua_bindwrapper(L, LUAFileSystemImpl::os_name);		
    
    lua_setglobal(L, "FileSystem");
    
    //  Save a copy of LUA's built-in "require"
    // and "print" functions in the "GEO" table declared to
    // the registry.
    lua_newtable(L);
    lua_getglobal(L, "require");
    lua_setfield(L, -2, "lua_require_func");
    lua_getglobal(L, "print");
    lua_setfield(L, -2, "lua_print_func");
    lua_getglobal(L, "tostring");
    lua_setfield(L, -2, "lua_tostring_func");    
    lua_setfield(L, LUA_REGISTRYINDEX, "GEO");

    //  Replace require(),print() and tostring() with our own versions.
    lua_bindwrapperglobal(L, require);
    lua_bindwrapperglobal(L, print);
    lua_bindwrapperglobal(L, tostring);    
}

/************************************************************************/

// In lua 5.3, lua_replace, lua_insert and lua_remove are macros, 
// whereas they are functions in lua 5.2.
// I redeclare them as functions here, so that we can load extension
// modules initially meant for use with lua 5.2 (such as "lgi", that
// allows loading libraries with gobject introspection, such as cairo
// and Poppler).

#ifdef lua_replace
#undef lua_replace
extern "C" int GEOGRAM_API lua_replace(lua_State* L, int idx);
extern "C" int GEOGRAM_API lua_replace(lua_State* L, int idx) {
    lua_copy(L, -1, idx);
    lua_pop(L,1);
    return 0;
}
#endif

#ifdef lua_insert
#undef lua_insert
extern "C" int GEOGRAM_API lua_insert(lua_State* L, int idx);
extern "C" int GEOGRAM_API lua_insert(lua_State* L, int idx) {
    lua_rotate(L, idx, 1);
    return 0;
}
#endif

#ifdef lua_remove
#undef lua_remove
extern "C" int GEOGRAM_API lua_remove(lua_State* L, int idx);
extern "C" int GEOGRAM_API lua_remove(lua_State* L, int idx) {
    lua_rotate(L, idx, -1);
    lua_pop(L,1);
    return 0;
}
#endif
