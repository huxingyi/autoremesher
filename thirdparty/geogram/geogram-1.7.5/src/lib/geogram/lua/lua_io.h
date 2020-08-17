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

#ifndef GEOGRAM_LUA_LUA_FILESYSTEM
#define GEOGRAM_LUA_LUA_FILESYSTEM

/**
 * \file geogram/lua/lua_io.h
 * \brief LUA bindings for geogram IO and filesystem functions
 */

#include <geogram/api/defs.h>

#ifdef __cplusplus
#include <vector>
#include <string>
#endif

#ifdef __cplusplus
extern "C" {
#endif
    
#include <geogram/third_party/lua/lua.h>

/**
 * \brief Initializes LUA filesystem operations.
 * \param[in] L a pointer to the LUA state.
 */
void GEOGRAM_API init_lua_io(lua_State* L);


/**
 * \brief Registers an "embedded file", i.e. a 
 *  static string with some LUA sources in it.
 * \param[in] filename the name of the "file" to
 *  be used to retreive the data.
 * \param[in] data the static string with the sources
 *  associated with the file.
 */
void GEOGRAM_API register_embedded_lua_file(
    const char* filename, const char* data
);

/**
 * \brief Gets an "embedded file" by its filename.
 * \param[in] filename the name that was previously
 *  used to register the file, using register_embedded_file()
 * \param[out] data a pointer to the static string with the
 *  "file" data, or nullptr if no file was registered with that
 *  name.
 */
void GEOGRAM_API get_embedded_lua_file(
    const char* filename, const char** data
);

#ifdef __cplusplus    
}

/**
 * \brief Lists the "embedded file" filenames.
 * \param[out] filenames a vector with all the filenames
 *  that were previously registered using register_embedded_lua_file()
 */
void GEOGRAM_API list_embedded_lua_files(
    std::vector<std::string>& filenames
);


#endif

#endif

