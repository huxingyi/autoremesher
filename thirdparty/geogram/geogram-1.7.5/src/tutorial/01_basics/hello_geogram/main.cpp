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

// A mesh file converter using Geogram.

#include <geogram/basic/command_line.h>
#include <geogram/basic/command_line_args.h>
#include <geogram/basic/logger.h>
#include <geogram/basic/stopwatch.h>
#include <geogram/mesh/mesh.h>
#include <geogram/mesh/mesh_io.h>

int main(int argc, char** argv) {
    
    // Initialize the Geogram library.
    GEO::initialize();

    // Import standard command line arguments.
    GEO::CmdLine::import_arg_group("standard");

    // Parse command line options and filenames.
    std::vector<std::string> filenames;
    if(!GEO::CmdLine::parse(
           argc, argv, filenames, "in_mesh_file <out_mesh_file>"
    )) {
        return 1;
    }

    // Default output filename is "out.meshb" if unspecified.
    if(filenames.size() == 1) {
        filenames.push_back("out.meshb");
    }

    // Display input and output filenames.
    GEO::Logger::div("Command line");
    GEO::Logger::out("MyApp") << "Input file: " << filenames[0] << std::endl;
    GEO::Logger::out("MyApp") << "Output file: " << filenames[1] << std::endl;

    // Declare a mesh.
    GEO::Mesh M;

    // Load the mesh and display timings.
    GEO::Logger::div("Loading");
    {
        GEO::Stopwatch W("load");
        if(!GEO::mesh_load(filenames[0], M)) {
            return 1;
        }
    }

    // Save the mesh and display timings.    
    GEO::Logger::div("Saving");
    {
        GEO::Stopwatch W("save");        
        if(!GEO::mesh_save(M,filenames[1])) {
            return 1;
        }
    }
    return 0;
}

