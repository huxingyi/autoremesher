/*
 *  Copyright (c) 2012-2014, Bruno Levy
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

#ifndef GEOGRAM_GFX_BASIC_COMMON
#define GEOGRAM_GFX_BASIC_COMMON

#include <geogram/basic/common.h>
#include <geogram_gfx/api/defs.h>
#include <geogram_gfx/basic/GL.h>

/**
 * \file geogram_gfx/basic/common.h
 * \brief Common include file, providing basic definitions. Should be
 *  included before anything else by all header files in geogram_gfx.
 */

namespace GEO {

    
    namespace Graphics {

        /**
         * \brief Initialize Geogram graphics subsystem
         * \details This function must be called once, after a valid OpenGL context
         *  is created, at the beginning of a program to initialize the geogram_gfx 
         *  library. It also installs a exit() handler that calls function terminate() 
         *  when the program exists normally.
         * \warning This function cannot be called before a valid OpenGL context is
         *  available. 
         */
        void GEOGRAM_GFX_API initialize();

        /**
         * \brief Cleans up Geogram graphics subsystem
         * \details This function is called automatically when the program exists
         * normally.
         * \warning This function should \b not be called directly.
         * \see initialize()
         */
        void GEOGRAM_GFX_API terminate();
    }
    
}

#endif

