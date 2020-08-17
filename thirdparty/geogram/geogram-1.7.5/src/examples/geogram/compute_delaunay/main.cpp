/*
 * GEOGRAM example program:
 * compute 3D and 2D Delaunay triangulations.
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
#include <geogram/mesh/mesh_io.h>
#include <geogram/mesh/mesh_reorder.h>
#include <geogram/delaunay/delaunay.h>
#include <algorithm>

namespace {
    using namespace GEO;
    
   /**
    * \brief Loads points from a file.
    * \param[in] points_filename the name of the file with the points.
    *  -If the example was compiled with the Geogram library, then any
    *  mesh file handled by Geogram can be used.
    *  -if the example was compiled with Delaunay_psm (single file), then
    *  the file should be ASCII, with one point per line.
    * \param[in] dimension number of coordinates of the points.
    * \param[out] points the loaded points, in a single vector of coordinates.
    *  In the end, the number of loaded points is points.size()/dimension.
    */
    bool load_points(
	const std::string& points_filename,
	index_t dimension,
	vector<double>& points
	) {
#ifdef GEOGRAM_PSM
	// Simple data input: one point per line, coordinates in ASCII
	LineInput input(points_filename);
	if(!input.OK()) {
	    return false;
	}
	while(!input.eof() && input.get_line()) {
	    input.get_fields();
	    if(input.nb_fields() == dimension) {
		for(index_t c=0; c<dimension; ++c) {
		    points.push_back(input.field_as_double(c));
		}
	    }
	}
#else
	// Using Geogram mesh I/O
	Mesh M;
	MeshIOFlags flags;
	flags.reset_element(MESH_FACETS);
	flags.reset_element(MESH_CELLS);
	if(!mesh_load(points_filename, M, flags)) {
	    return false;
	}
	M.vertices.set_dimension(dimension);
	index_t nb_points = M.vertices.nb();
	points.resize(nb_points * dimension);
	Memory::copy(
	    points.data(),
	    M.vertices.point_ptr(0),
	    M.vertices.nb()*dimension*sizeof(double)
	);
#endif
	return true;
    }

   /**
    * \brief Saves a Delaunay triangulation to a file.
    * \param[in] delaunay a pointer to the Delaunay triangulation.
    * \param[in] filename the name of the file to be saved.
    *  -If the example was compiled with the Geogram library, then any
    *  mesh file handled by Geogram can be used.
    *  if the example was compiled with Delaunay_psm (single file), then
    *  the points and vertices of the triangulation are output in ASCII.
    * \param[in] convex_hull_only if true, then only the triangles on the
    *  convex hull are output.
    */
    void save_Delaunay(
	Delaunay* delaunay, const std::string& filename,
	bool convex_hull_only = false
    ) {
	vector<index_t> tri2v;
	
	if(convex_hull_only) {
	    
	    // The convex hull can be efficiently traversed only if infinite
	    // tetrahedra are kept.
	    geo_assert(delaunay->keeps_infinite());
	    
	    // The convex hull can be retrieved as the finite facets
	    // of the infinite cells (note: it would be also possible to
	    // throw away the infinite cells and get the convex hull as
	    // the facets adjacent to no cell). Here we use the infinite
	    // cells to show an example with them.
	    
	    
	    // This block is just a sanity check
	    {
		for(index_t t=0; t < delaunay->nb_finite_cells(); ++t) {
		    geo_debug_assert(delaunay->cell_is_finite(t));
		}
		
		for(index_t t=delaunay->nb_finite_cells();
		    t < delaunay->nb_cells(); ++t) {
		    geo_debug_assert(delaunay->cell_is_infinite(t));
		}
	    }
	    
	    // This iterates on the infinite cells
	    for(
		index_t t = delaunay->nb_finite_cells();
		t < delaunay->nb_cells(); ++t
	     ) {
		for(index_t lv=0; lv<4; ++lv) {
		    signed_index_t v = delaunay->cell_vertex(t,lv);
		    if(v != -1) {
			tri2v.push_back(index_t(v));
		    }
		}
	    }
	}
	
#ifdef GEOGRAM_PSM
	// Simple data output: output vertices and simplices
	
	Logger::out("Delaunay") << "Saving output to " << filename << std::endl;
	std::ofstream out(filename.c_str());
	
	out << delaunay->nb_vertices() << " vertices" << std::endl;
	for(index_t v=0; v < delaunay->nb_vertices(); ++v) {
	    for(index_t c=0; c < delaunay->dimension(); ++c) {
		out << delaunay->vertex_ptr(v)[c] << " ";
	    }
	    out << std::endl;
	}
	if(convex_hull_only) {
	    out << tri2v.size()/3 << " simplices" << std::endl;
	    for(index_t t=0; t<tri2v.size()/3; ++t) {
		out << tri2v[3*t] << " "
		    << tri2v[3*t+1] << " "
		    << tri2v[3*t+2] << std::endl;
	    }
	} else {
	    out << delaunay->nb_cells() << " simplices" << std::endl;
	    for(index_t t=0; t<delaunay->nb_cells(); ++t) {
		for(index_t lv=0; lv<delaunay->cell_size(); ++lv) {
		    out << delaunay->cell_vertex(t,lv) << " ";
		}
		out << std::endl;
	    }
	}
	
#else
	// Using Geogram mesh I/O: copy Delaunay into a Geogram
	// mesh and save it to disk.
	
	Mesh M_out;
	vector<double> pts(delaunay->nb_vertices() * 3);
	for(index_t v = 0; v < delaunay->nb_vertices(); ++v) {
	    pts[3 * v] = delaunay->vertex_ptr(v)[0];
	    pts[3 * v + 1] = delaunay->vertex_ptr(v)[1];
	    pts[3 * v + 2] =
		(delaunay->dimension() >= 3) ? delaunay->vertex_ptr(v)[2] : 0.0;
	}
	
	if(convex_hull_only) {
	    M_out.facets.assign_triangle_mesh(3, pts, tri2v, true);
	} else if(delaunay->dimension() == 3) {
	    vector<index_t> tet2v(delaunay->nb_cells() * 4);
	    for(index_t t = 0; t < delaunay->nb_cells(); ++t) {
	    tet2v[4 * t] = index_t(delaunay->cell_vertex(t, 0));
	    tet2v[4 * t + 1] = index_t(delaunay->cell_vertex(t, 1));
	    tet2v[4 * t + 2] = index_t(delaunay->cell_vertex(t, 2));
	    tet2v[4 * t + 3] = index_t(delaunay->cell_vertex(t, 3));
	    }
	    M_out.cells.assign_tet_mesh(3, pts, tet2v, true);
	} else if(delaunay->dimension() == 2) {
	    tri2v.resize(delaunay->nb_cells() * 3);
	    for(index_t t = 0; t < delaunay->nb_cells(); ++t) {
		tri2v[3 * t] = index_t(delaunay->cell_vertex(t, 0));
		tri2v[3 * t + 1] = index_t(delaunay->cell_vertex(t, 1));
		tri2v[3 * t + 2] = index_t(delaunay->cell_vertex(t, 2));
	    }
	    M_out.facets.assign_triangle_mesh(3, pts, tri2v, true);
	}
	M_out.show_stats();
	
	Logger::div("Saving the result");
	MeshIOFlags flags;
	flags.set_element(MESH_FACETS);            
	flags.set_element(MESH_CELLS);
	mesh_save(M_out, filename, flags);
#endif    
    }
    
}

int main(int argc, char** argv) {
    using namespace GEO;

    // Needs to be called once.
    GEO::initialize();

    try {

        Stopwatch Wtot("Total time");

        std::vector<std::string> filenames;
 
        CmdLine::import_arg_group("standard");
        CmdLine::import_arg_group("algo");

        CmdLine::declare_arg(
            "convex_hull", false,
            "compute just the convex hull of the points"
        );

        CmdLine::declare_arg(
            "dimension", 3, "3 for 3D, 2 for 2D"
        );

        CmdLine::set_arg("algo:delaunay","default");
        
        if(
            !CmdLine::parse(
                argc, argv, filenames, "pointsfile <outputfile|none>"
            )
        ) {
            return 1;
        }


        std::string points_filename = filenames[0];

        std::string output_filename =
            filenames.size() >= 2 ? filenames[1] : std::string("out.mesh");

        bool output = (output_filename != "none");

        Logger::div("Data I/O");

        Logger::out("I/O") << "Output = " << output_filename << std::endl;

        bool convex_hull_only = CmdLine::get_arg_bool("convex_hull");
        index_t dimension = index_t(CmdLine::get_arg_int("dimension"));

        std::string del = CmdLine::get_arg("algo:delaunay");
        if(del == "default") {
            if(dimension == 3) {
                if(DelaunayFactory::has_creator("PDEL")) {
  		    // PDEL = Parallel 3D Delaunay
                    CmdLine::set_arg("algo:delaunay", "PDEL");
                } else {
		    // BDEL = Sequential 3D Delaunay
                    CmdLine::set_arg("algo:delaunay", "BDEL");
                }
            } else if(dimension == 2) {
	        // BDEL2d = Sequential 2D Delaunay
                CmdLine::set_arg("algo:delaunay", "BDEL2d");                
            }
        }

        Logger::out("Delaunay")
            << "Using " << CmdLine::get_arg("algo:delaunay") << std::endl;

        // Note: To create a parallel Delaunay 3D, one can use directly:
        // Delaunay_var delaunay = Delaunay::create(3,"PDEL") instead
        // of the line below (that uses the command line to select the
        // implementation of Delaunay).
        Delaunay_var delaunay = Delaunay::create(coord_index_t(dimension));

	//   If we want the convex hull, we keep the infinite facets,
	// because the convex hull can be retreived as the finite facets
	// of the infinite cells (note: it would be also possible to
	// throw away the infinite cells and get the convex hull as
	// the facets adjacent to no cell).
	if(convex_hull_only) {
	    delaunay->set_keeps_infinite(true);
	}

	vector<double> points;
	
	if(!load_points(points_filename, dimension, points)) {
	    Logger::err("Delaunay") << "Could not load points" << std::endl;
	    return 1;
	}

	index_t nb_points = points.size() / dimension;
	
	Logger::out("Delaunay")
	    << "Loaded " << nb_points << " points" << std::endl;

	double time = 0.0;
	{
	    Stopwatch Wdel("Delaunay");
	    // Note: this does not transfer ownership of memory, caller
	    // is still responsible of the memory of the points (here the
    	    // vector<double>). No memory is copied, Delaunay just keeps
	    // a pointer.
	    delaunay->set_vertices(nb_points, points.data());
	    time = Wdel.elapsed_time();
	}
	
        Logger::out("Delaunay") << delaunay->nb_cells() << " tetrahedra"
            << std::endl;

	Logger::out("Delaunay") << double(delaunay->nb_cells()) / time
				<< " tetrahedra / second"
				<< std::endl;
	
        if(output) {
	    save_Delaunay(delaunay, output_filename, convex_hull_only);
	}
    }
    catch(const std::exception& e) {
        std::cerr << "Received an exception: " << e.what() << std::endl;
        return 1;
    }

    Logger::out("") << "Everything OK, Returning status 0" << std::endl;
    return 0;
}

