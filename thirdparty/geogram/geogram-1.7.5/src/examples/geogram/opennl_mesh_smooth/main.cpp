/*
 * GEOGRAM example program:
 * least-squares mesh smoothing (Mallet's algorithm)
 * implemented with OpenNL
 */

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

#include <geogram/basic/common.h>
#include <geogram/basic/logger.h>
#include <geogram/basic/command_line.h>
#include <geogram/basic/command_line_args.h>
#include <geogram/basic/stopwatch.h>
#include <geogram/basic/file_system.h>
#include <geogram/mesh/mesh.h>
#include <geogram/mesh/mesh_subdivision.h>
#include <geogram/mesh/mesh_io.h>
#include <geogram/NL/nl.h>

namespace {
    using namespace GEO;

    
    void mesh_smooth(Mesh* M, NLenum solver = NL_SOLVER_DEFAULT) {

	// Chain corners around vertices
	vector<index_t> v2c(M->vertices.nb(), index_t(-1));
	vector<index_t> next_c_around_v(M->facet_corners.nb(), index_t(-1));
	vector<index_t> c2f(M->facet_corners.nb(), index_t(-1));
	for(index_t f=0; f<M->facets.nb(); ++f) {
	    for(index_t c=M->facets.corners_begin(f);
		c<M->facets.corners_end(f); ++c
	    ) {
		index_t v = M->facet_corners.vertex(c);
		next_c_around_v[c] = v2c[v];
		v2c[v] = c;
		c2f[c] = f;
	    }
	}

	nlNewContext();

	if(solver == NL_SUPERLU_EXT || solver == NL_PERM_SUPERLU_EXT) {
	    if(nlInitExtension("SUPERLU")) {
		nlSolverParameteri(NL_SOLVER, NLint(solver));		
	    } else {
		Logger::err("NL") << "Could not init SUPERLU extension";
	    }
	} else if(solver == NL_CHOLMOD_EXT) {
	    if(nlInitExtension("CHOLMOD")) {
		nlSolverParameteri(NL_SOLVER, NLint(solver));		
	    } else {
		Logger::err("NL") << "Could not init CHOLMOD extension";
	    }
	}

	nlSolverParameteri(NL_LEAST_SQUARES, NL_TRUE);
	nlSolverParameteri(NL_NB_VARIABLES, NLint(M->vertices.nb()));
	nlSolverParameteri(NL_NB_SYSTEMS, NLint(M->vertices.dimension()));
	nlEnable(NL_NORMALIZE_ROWS);
	nlEnable(NL_VARIABLES_BUFFER);
	
	Attribute<bool> v_is_locked(M->vertices.attributes(), "selection");

	nlBegin(NL_SYSTEM);
	
	for(index_t coord=0; coord<M->vertices.dimension(); ++coord) {
	    // Bind directly the variables buffer to the coordinates in
	    // the mesh, to avoid copying data.
	    nlBindBuffer(
		NL_VARIABLES_BUFFER, coord,
		M->vertices.point_ptr(0) + coord,
		NLuint(sizeof(double)*M->vertices.dimension())
	    );
	}
	
	for(index_t v=0; v<M->vertices.nb(); ++v) {
	    if(v_is_locked[v]) {
		nlLockVariable(v);
	    }
	}
	
	nlBegin(NL_MATRIX);
	for(index_t v=0; v<M->vertices.nb(); ++v) {
	    nlBegin(NL_ROW);
	    index_t count = 0;
	    for(
		index_t c = v2c[v];
		c != index_t(-1); c = next_c_around_v[c]
	    ) {
		index_t f = c2f[c];
		index_t c2 = M->facets.next_corner_around_facet(f,c);
		index_t w = M->facet_corners.vertex(c2);
		nlCoefficient(w, 1.0);
		++count;
	    }
	    nlCoefficient(v, -double(count));
	    nlEnd(NL_ROW);
	}
	nlEnd(NL_MATRIX);
	nlEnd(NL_SYSTEM);

	Logger::div("Solve");	
	nlSolve();

	nlDeleteContext(nlGetCurrent());
    }
}

int main(int argc, char** argv) {
    using namespace GEO;

    GEO::initialize();

    try {
        Stopwatch Wtot("Total time");

        std::vector<std::string> filenames;

        CmdLine::import_arg_group("standard");
        CmdLine::import_arg_group("algo");
	CmdLine::declare_arg(
	    "solver", "NL_SOLVER_DEFAULT", "solver"
	);
	CmdLine::declare_arg(
	    "nb_subdivide", 2, "number of times mesh is subdivided"
	);
	
        if(
            !CmdLine::parse(
                argc, argv, filenames, "inmesh <outmesh>"
            )
        ) {
            return 1;
        }

	
	if(filenames.size() != 2) {
	    Logger::out("Smooth") << "Generating output to out.geogram"
				  << std::endl;
	    filenames.push_back("out.geogram");
	}
	
        Logger::div("Data I/O");

        Mesh M;

	MeshIOFlags flags;
	flags.reset_element(MESH_CELLS);
	flags.set_attributes(MESH_ALL_ATTRIBUTES);
	if(!mesh_load(filenames[0], M, flags)) {
	    return 1;
	}

	{
	    Attribute<bool> is_locked(M.vertices.attributes(), "selection");
	    for(index_t v=0; v<M.vertices.nb(); ++v) {
		is_locked[v] = true;
	    }
	    for(index_t i=0; i<CmdLine::get_arg_uint("nb_subdivide"); ++i) {
		mesh_split_triangles(M);
	    }
	}
	
	NLenum solver = NL_SOLVER_DEFAULT;
	std::string solver_string = CmdLine::get_arg("solver");
	if(solver_string == "NL_CG") {
	    solver = NL_CG;
	} else if(solver_string == "NL_SUPERLU_EXT") {
	    solver = NL_SUPERLU_EXT;
	} else if(solver_string == "NL_PERM_SUPERLU_EXT") {
	    solver = NL_PERM_SUPERLU_EXT;
	} else if(solver_string == "NL_SYMMETRIC_SUPERLU_EXT") {
	    solver = NL_SYMMETRIC_SUPERLU_EXT;
	} else if(solver_string == "NL_CHOLMOD_EXT") {
	    solver = NL_CHOLMOD_EXT;
	}
	
	mesh_smooth(&M, solver);

	if(!mesh_save(M, filenames[1], flags)) {
	    return 1;
	}

    }
    catch(const std::exception& e) {
        std::cerr << "Received an exception: " << e.what() << std::endl;
        return 1;
    }

    Logger::out("") << "Everything OK, Returning status 0" << std::endl;
    return 0;
}

