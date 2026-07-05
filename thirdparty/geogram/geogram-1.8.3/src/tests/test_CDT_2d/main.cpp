/*
 *  Copyright (c) 2000-2022 Inria
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
 *  Contact: Bruno Levy
 *
 *     https://www.inria.fr/fr/bruno-levy
 *
 *     Inria,
 *     Domaine de Voluceau,
 *     78150 Le Chesnay - Rocquencourt
 *     FRANCE
 *
 */

#include <geogram/mesh/mesh.h>
#include <geogram/mesh/mesh_io.h>
#include <geogram/delaunay/CDT_2d.h>
#include <geogram/basic/logger.h>
#include <geogram/basic/command_line.h>
#include <geogram/basic/command_line_args.h>

int main(int argc, char** argv) {

    GEO::initialize();
    GEO::Logger::instance()->set_quiet(false);
    GEO::CmdLine::import_arg_group("standard");
    GEO::CmdLine::import_arg_group("algo");
    GEO::CmdLine::declare_arg(
        "constrained", true, "compute a constrained triangulation"
    );
    GEO::CmdLine::declare_arg(
        "delaunay", true, "compute a Delaunay triangulation"
    );
    GEO::CmdLine::declare_arg(
        "remove_external_triangles", false,
        "remove triangles adjacent to border"
    );
    GEO::CmdLine::declare_arg("quad", false, "enclosing polygon is a quad");
    
    std::vector<std::string> filenames;
    if(!GEO::CmdLine::parse(argc, argv, filenames, "constraints_filename")) {
        return 1;
    }
    
    if(filenames.size() != 1) {
        return 1;
    }

    GEO::Mesh constraints;
    GEO::mesh_load(filenames[0], constraints);
    constraints.vertices.set_dimension(2);
    
    GEO::CDT2d cdt;
    cdt.set_delaunay(GEO::CmdLine::get_arg_bool("delaunay"));

    GEO::index_t n=0;
    if(GEO::CmdLine::get_arg_bool("quad")) {
        n=4;
        GEO::vec2 p0(constraints.vertices.point_ptr(0));
        GEO::vec2 p1(constraints.vertices.point_ptr(1));
        GEO::vec2 p2(constraints.vertices.point_ptr(2));
        GEO::vec2 p3(constraints.vertices.point_ptr(3));                
        cdt.create_enclosing_quad(p0,p1,p2,p3);
    } else {
        n=3;
        GEO::vec2 p0(constraints.vertices.point_ptr(0));
        GEO::vec2 p1(constraints.vertices.point_ptr(1));
        GEO::vec2 p2(constraints.vertices.point_ptr(2));
        cdt.create_enclosing_triangle(p0,p1,p2);
    }
    
    cdt.insert(constraints.vertices.nb()-n, constraints.vertices.point_ptr(n));
    if(GEO::CmdLine::get_arg_bool("constrained")) {
        for(GEO::index_t e: constraints.edges) {
            cdt.insert_constraint(
                constraints.edges.vertex(e,0), constraints.edges.vertex(e,1)
            );
        }
        // Create the vertices coming from constraint intersections
        for(GEO::index_t v=constraints.vertices.nb(); v<cdt.nv(); ++v) {
            constraints.vertices.create_vertex(cdt.point(v).data());
        }
        
    }
    cdt.check_consistency();
    if(GEO::CmdLine::get_arg_bool("remove_external_triangles")) {
        cdt.remove_external_triangles();
    }
    cdt.check_consistency();
    GEO::Logger::out("CDT") << "CDT OK" << std::endl;
    
    for(GEO::index_t t=0; t<cdt.nT(); ++t) {
        constraints.facets.create_triangle(
            cdt.Tv(t,0), cdt.Tv(t,1), cdt.Tv(t,2)
        );
    }

    constraints.facets.connect();
    
    GEO::mesh_save(constraints,"result.geogram");    
    
    return 0;
}
