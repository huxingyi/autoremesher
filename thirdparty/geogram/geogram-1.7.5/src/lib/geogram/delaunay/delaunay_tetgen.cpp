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

#include <geogram/delaunay/delaunay_tetgen.h>
#include <geogram/mesh/mesh.h>
#include <geogram/basic/logger.h>
#include <geogram/basic/command_line.h>
#include <geogram/bibliography/bibliography.h>

#ifdef GEOGRAM_WITH_TETGEN

namespace GEO {

    DelaunayTetgen::DelaunayTetgen(coord_index_t dimension) :
        Delaunay(3)
    {
        if(dimension != 3) {
            throw InvalidDimension(dimension, "DelaunayTetgen", "3");
        }

	geo_cite("DBLP:journals/toms/Si15");
	
        tetgen_in_.firstnumber = 0;
        tetgen_out_.firstnumber = 0;
    }

    DelaunayTetgen::~DelaunayTetgen() {
        // We do not want that the destructor of tetgen_in_
        // deallocates the points, since they are managed
        // by a vector<>.
        tetgen_in_.pointlist = nullptr;
        tetgen_in_.numberofpoints = 0;
    }

    index_t DelaunayTetgen::region(index_t t) const {
	geo_debug_assert(keep_regions_);
	geo_debug_assert(t < nb_cells());
	return index_t(tetgen_out_.tetrahedronattributelist[t]);
    }

    bool DelaunayTetgen::supports_constraints() const {
        return true;
    }

    void DelaunayTetgen::set_vertices(
        index_t nb_vertices, const double* vertices
    ) {
        if(constraints_ != nullptr) {
            set_vertices_constrained(nb_vertices, vertices);
        } else {
            set_vertices_unconstrained(nb_vertices, vertices);
        }
    }

    void DelaunayTetgen::set_vertices_unconstrained(
        index_t nb_vertices, const double* vertices
    ) {
        // Q: quiet
        // n: output tet neighbors
        // V: verbose
        if(CmdLine::get_arg_bool("dbg:tetgen")) {
            tetgen_args_.parse_commandline(const_cast<char*>("Vn"));
        } else {
	    tetgen_args_.parse_commandline(const_cast<char*>("Qn"));            
        }

        Delaunay::set_vertices(nb_vertices, vertices);
        tetgen_out_.deinitialize();
        tetgen_in_.initialize();
        tetgen_in_.numberofpoints = int(nb_vertices);
        tetgen_in_.pointlist = const_cast<double*>(vertices);
        try {
            GEO_3rdParty::tetrahedralize(
                &tetgen_args_, &tetgen_in_, &tetgen_out_
            );
        }
        catch(int error_code) {
            Logger::err("DelaunayTetgen")
                << "Encountered a problem..." << std::endl;
            throw Delaunay::InvalidInput(error_code);
        }
        set_arrays(
            index_t(tetgen_out_.numberoftetrahedra),
            tetgen_out_.tetrahedronlist, 
            tetgen_out_.neighborlist
        );
    }


    void DelaunayTetgen::set_vertices_constrained(
        index_t nb_vertices, const double* vertices
    ) {
        index_t nb_borders = 0;
        for(index_t c=0; c<constraints_->facet_corners.nb(); ++c) {
            if(constraints_->facet_corners.adjacent_facet(c) == NO_FACET) {
                ++nb_borders;
            }
        }

        if(nb_borders != 0) {
            Logger::warn("DelaunayTetgen") 
                << "Constraints have " << nb_borders
                << " edge(s) on the border" 
                << std::endl;
        }


        tetgen_out_.deinitialize();

        // Q: quiet
        // p: input data is surfacic
        // n: output tet neighbors
        // q: desired quality
        // O0: do not optimize mesh
        // V: verbose
        // YY: prohibit steiner points on boundaries
        // (first Y for exterior boundary, second Y for the
        // other ones).
        // AA: generate region tags for each shell.

        std::string cmdline;
        if(refine_) {
            if(CmdLine::get_arg_bool("dbg:tetgen")) {
                cmdline = "Vpnq" + String::to_string(quality_) + "YYAA";
            } else {
                cmdline = "Qpnq" + String::to_string(quality_) + "YYAA";
            }
        } else {
            if(CmdLine::get_arg_bool("dbg:tetgen")) {
                cmdline = "VpnO0YYAA";
            } else {
                cmdline = "QpnO0YYAA";
            }
        }
        tetgen_args_.parse_commandline(const_cast<char*>(cmdline.c_str()));
        
        tetgen_in_.deinitialize();
        tetgen_in_.initialize();
        tetgen_in_.firstnumber = 0 ;
        
        //
        // Copy vertices
        //

        tetgen_in_.numberofpoints = int(
            constraints_->vertices.nb()+nb_vertices
        );
        tetgen_in_.pointlist = new double[3*tetgen_in_.numberofpoints];
        if(constraints_->vertices.nb() != 0) {
            Memory::copy(
                tetgen_in_.pointlist, constraints_->vertices.point_ptr(0), 
                constraints_->vertices.nb()*3*sizeof(double)
            );
        }
        if(nb_vertices != 0) {
            Memory::copy(
                &tetgen_in_.pointlist[3*constraints_->vertices.nb()],
                vertices, nb_vertices*3*sizeof(double)
            );
        }

        // Edges constraints
        // (no need to copy, we make tetgen_in_
        //  point to the edges of the input
        //  constraints mesh)
        
        if(constraints_->edges.nb() != 0) {
            tetgen_in_.numberofedges = int(
                constraints_->edges.nb()
            );
            tetgen_in_.edgelist = const_cast<int*>(
                (const int*)constraints_->edges.vertex_index_ptr(0)
            );
        }
        
        // Copy facet constraints
        //
        // All the polygons are allocated in one go, in a contiguous array.
        
        GEO_3rdParty::tetgenio::polygon* polygons = 
            new GEO_3rdParty::tetgenio::polygon[constraints_->facets.nb()];
        tetgen_in_.numberoffacets = int(constraints_->facets.nb()) ;
        tetgen_in_.facetlist =
            new GEO_3rdParty::tetgenio::facet[tetgen_in_.numberoffacets];
        for(index_t f=0; f<constraints_->facets.nb(); ++f) {
            GEO_3rdParty::tetgenio::facet& F = tetgen_in_.facetlist[f];
            GEO_3rdParty::tetgenio::init(&F);
            F.numberofpolygons = 1;
            F.polygonlist = &polygons[f];
            GEO_3rdParty::tetgenio::polygon& P = F.polygonlist[0];
            GEO_3rdParty::tetgenio::init(&P) ;
            P.numberofvertices = int(constraints_->facets.nb_vertices(f));
            P.vertexlist = reinterpret_cast<int*>(
                const_cast<Mesh*>(constraints_)->facet_corners.vertex_index_ptr(
                    constraints_->facets.corners_begin(f)
                )
            );
            F.numberofholes = 0 ;
            F.holelist = nullptr ;
        }

        bool there_was_an_error = false;
        int error_code = 0;
        
        try {
            GEO_3rdParty::tetrahedralize(
                &tetgen_args_, &tetgen_in_, &tetgen_out_
            );
        } catch(...) {
            there_was_an_error = true;
            Logger::err("DelaunayTetgen")
                << "Encountered a problem, relaunching in diagnose mode..."
                << std::endl;
            cmdline += "d";
            tetgen_args_.parse_commandline(const_cast<char*>(cmdline.c_str()));
            try {
                GEO_3rdParty::tetrahedralize(
                    &tetgen_args_, &tetgen_in_, &tetgen_out_
                );
            } catch(...) {
            }
        }

        // Deallocate the datastructures used by tetgen,
        // and disconnect them from tetgen,
        // so that tetgen does not try to deallocate them.

        // Pointlist was allocated in local array
        tetgen_in_.numberofpoints = 0;
        delete[] tetgen_in_.pointlist;
        tetgen_in_.pointlist = nullptr;

        // Edges were shared with constraint mesh
        // (no need to deallocate)
        tetgen_in_.numberofedges = 0;
        tetgen_in_.edgelist = nullptr;

        // Facets structures were allocated in local
        // array, and vertices indices were shared
        // with constraint mesh
        delete[] tetgen_in_.facetlist; 
        tetgen_in_.facetlist = nullptr;
        tetgen_in_.numberoffacets = 0;
        delete[] polygons;

        if(there_was_an_error) {
            InvalidInput error_report(error_code);
            for(auto f : tetgen_in_.isectfaces) {
                // Note: tetgen reports facets with
                //  1-based indexing ([1...nf]) !!
                error_report.invalid_facets.push_back(index_t(f)-1);
            }
            Logger::err("DelaunayTetgen")
                << "Found "
                << error_report.invalid_facets.size()
                << " facets with intersections." << std::endl;
            throw(error_report);
        }

	index_t nb_tets = index_t(tetgen_out_.numberoftetrahedra);
	
	if(!keep_regions_) {
	    // Determine which regions are incident to
	    // the 'exterior' (neighbor = -1 or tet is adjacent to
	    // a tet in region 0).
	    // The region Id of tet t is determined by:
	    //  tetgen_out_.tetrahedronattributelist[t]

	    std::set<double> good_regions;
	    for(
		index_t t = 0; 
		t < index_t(tetgen_out_.numberoftetrahedra); ++t
	    ) {
		for(index_t f=0; f<4; ++f) {
		    signed_index_t n = (tetgen_out_.neighborlist[t*4+f]);
		    if(
		       n == -1 ||
		       tetgen_out_.tetrahedronattributelist[n] == 0.0
		    ) {
			good_regions.insert(
			    tetgen_out_.tetrahedronattributelist[t]
			);
			break;
		    }
		}
	    }

	    // Remove the tets that are not in good_region.
	    vector<index_t> old2new(
		index_t(tetgen_out_.numberoftetrahedra),index_t(-1)
	    );
	    nb_tets = 0;
	    for(
		index_t t = 0; 
		t < index_t(tetgen_out_.numberoftetrahedra); ++t
	    ) {        
		if(
		    good_regions.find(
			tetgen_out_.tetrahedronattributelist[t]
		    ) != good_regions.end()
		    ) {
		    if(t != nb_tets) {
			Memory::copy(
			    &tetgen_out_.tetrahedronlist[nb_tets * 4],
			    &tetgen_out_.tetrahedronlist[t * 4],
			    4 * sizeof(signed_index_t)
			);
			Memory::copy(
			    &tetgen_out_.neighborlist[nb_tets * 4],
			    &tetgen_out_.neighborlist[t * 4],
			    4 * sizeof(signed_index_t)
			);
		    }
		    old2new[t] = nb_tets;
		    ++nb_tets;
		}
	    }
	    for(index_t i = 0; i < 4 * nb_tets; ++i) {
		signed_index_t t = tetgen_out_.neighborlist[i];
		if(t != -1) {
		    t = signed_index_t(old2new[t]);
		}
		tetgen_out_.neighborlist[i] = t;
	    }
	}

        // Link tetgen's output to Delaunay class data structures.

        Delaunay::set_vertices(
            index_t(tetgen_out_.numberofpoints), tetgen_out_.pointlist
        );

        set_arrays(
            nb_tets,
            tetgen_out_.tetrahedronlist, 
            tetgen_out_.neighborlist
        );
    }


}

#else

// Declare a dummy variable so that
// MSVC does not complain that it 
// generated an empty object file.
int dummy_delaunay_tetgen_compiled = 1;

#endif

