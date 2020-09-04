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

#ifndef GEOGRAM_BASIC_COMMAND_LINE_ARGS
#define GEOGRAM_BASIC_COMMAND_LINE_ARGS

#include <geogram/basic/common.h>

/**
 * \file geogram/basic/command_line_args.h
 * \brief Definitions of standard command line arguments
 */

namespace GEO {

    namespace CmdLine {

        /**
         * \brief Imports a group of command line arguments.
         * \details
         * The behavior of the the Vorpaline library is controlled by several
         * properties organized in the following groups:
         *
         * - global - global options (profile, debug).
         * - sys - Process settings (FPE, multithreading, ...).
         * - log - Logger settings.
         * - standard - standard settings (global + sys + log).
         * - pre - configuration of the pre-processing phase.
         * - remesh - configuration of the remeshing phase.
         * - post - configuration of the post-processing phase.
         * - algo - fine-tuning of the algorithms.
         * - opt - fine-tuning of the optimizer.
         * - co3ne - configuration of the reconstruction phase.
         * - stat - Statistics settings
         *
         * To allow setting the properties from the command line,
         * users must declare command line arguments and groups for the
         * Vorpaline properties that they want to control, plus their own
         * specific command line options. The function import_arg_group()
         * relieves users of this tiresome and repetitive task by declaring a
         * whole group of command line arguments.
         *
         * The following example illustrates how to use import_arg_group() in
         * a main program, to declare command line options for the "standard"
         * and "remesh" Vorpaline property groups:
         *
         * \code
         * int main(int argc, char* argv[]) {
         *      GEO::initialize();
         *      import_arg_group("standard");
         *      import_arg_group("remesh");
         *      declare_arg("useroption1", "default value", "This controls...");
         *      declare_arg("useroption2", "default value", "This controls...");
         *      ...
         *      return 0;
         * }
         * \endcode
         *
         * \param[in] name the name of the group to import.
         * \retval \c true if the group name has been
         * successfully importe (the name is valid).
         * \retval \c false otherwise.
         */
        bool GEOGRAM_API import_arg_group(
            const std::string& name
        );

        /**
         * \brief Sets the current application profile.
         * \details A profile is a predefined set of property values that
         * affect the configuration of the Vorpaline library in order to
         * perform very specific tasks. Only a single profile can be active at
         * a time, but nothing prevents from tweaking indivual properties to
         * fine tune the profile values.
         *
         * Vorpaline defines the following profiles:
         * - cad - configures Vorpaline to process files issued from CAD
         *   software.
         * - scan - configures Vorpaline to process files issued from 3D
         *   scanners.
         * - convert - performs file conversion only. No remeshing.
         * - repair - repares small problems in input data, no remeshing.
         * - heal - repares bigger problems in the input data, using
         *   remeshing.
         * - reconstruct - reconstructs a mesh from scratch using the \b
         *   points of the the input data (input triangles are ignored).
         *
         * \param[in] name the name of the profile, as defined above.
         * \retval \c true if the profile name is valid and has been
         * successfully set.
         * \retval \c false otherwise.
         */
        bool GEOGRAM_API set_profile(
            const std::string& name
        );
    }
}

#endif

