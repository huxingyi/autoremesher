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

#include <geogram/mesh/mesh.h>
#include <geogram/mesh/mesh_io.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/mesh_repair.h>
#include <geogram/delaunay/delaunay.h>
#include <geogram/voronoi/RVD.h>
#include <geogram/numerics/predicates.h>
#include <geogram/basic/logger.h>
#include <geogram/basic/command_line.h>
#include <geogram/basic/command_line_args.h>
#include <geogram/basic/file_system.h>
#include <geogram/basic/progress.h>
#include <stdarg.h>


namespace {

    using namespace GEO;

   /**
    * \brief Creates a surfacic mesh from a cube.
    * \param[out] M the resulting mesh
    */
    void initialize_mesh_with_box(Mesh& M) {
        M.clear();
        M.vertices.set_dimension(3);

        const double d = 1.0;
        
        M.vertices.create_vertex(vec3(-d, -d, -d).data());
        M.vertices.create_vertex(vec3(-d, -d, d).data());
        M.vertices.create_vertex(vec3(-d, d, -d).data());
        M.vertices.create_vertex(vec3(-d, d, d).data());
        M.vertices.create_vertex(vec3(d, -d, -d).data());
        M.vertices.create_vertex(vec3(d, -d, d).data());
        M.vertices.create_vertex(vec3(d, d, -d).data());
        M.vertices.create_vertex(vec3(d, d, d).data());
      
        M.facets.create_quad(7,6,2,3);
        M.facets.create_quad(1,3,2,0);
        M.facets.create_quad(5,7,3,1);
        M.facets.create_quad(4,6,7,5);
        M.facets.create_quad(4,5,1,0);
        M.facets.create_quad(6,4,0,2);
        
        M.facets.connect();
    }

    void center_scale_mesh(Mesh& M, vec3 center, double radius) {
        double xyz_min[3];
        double xyz_max[3];
        get_bbox(M, xyz_min, xyz_max);
        double scale = 2.0*radius / Geom::distance(xyz_min, xyz_max, 3);
        vec3 g = 0.5*(vec3(xyz_min) + vec3(xyz_max));
        for(index_t i=0; i<M.vertices.nb(); ++i) {
            M.vertices.point(i) -= g;
            M.vertices.point(i) *= scale;
            M.vertices.point(i) += center;
        }
    }

    /**
     * \brief Shrinks a mesh.
     * \param[in,out] M the mesh to be shrunk
     * \param[in] factor the shrinking factor (1.0 means
     *   no shrinking, 0.5 means average shrinking).
     */
    void shrink_mesh(Mesh& M, double factor) {
        double xyz_min[3];
        double xyz_max[3];
        get_bbox(M, xyz_min, xyz_max);
        vec3 g = 0.5*(vec3(xyz_min) + vec3(xyz_max));
        for(index_t i=0; i<M.vertices.nb(); ++i) {
            M.vertices.point(i) -= g;
            M.vertices.point(i) *= factor;
            M.vertices.point(i) += g;
        }
    }


    /**
     * \brief Tests whether the facets of a mesh are exactly planar.
     * \retval true if all the facets are exactly planar
     * \retval false otherwise
     */
    bool mesh_facets_are_planar(const Mesh& M) {
        for(index_t f=0; f<M.facets.nb(); ++f) {
            for(index_t c=M.facets.corners_begin(f); c+3<M.facets.corners_end(f); ++c) {
                index_t v1 = M.facet_corners.vertex(c);
                index_t v2 = M.facet_corners.vertex(c+1);
                index_t v3 = M.facet_corners.vertex(c+2);
                index_t v4 = M.facet_corners.vertex(c+3);
                if(
                    PCK::orient_3d(
                        M.vertices.point_ptr(v1),
                        M.vertices.point_ptr(v2),
                        M.vertices.point_ptr(v3),
                        M.vertices.point_ptr(v4)
                    ) != ZERO
                ) {
                    return false;
                }
            }
        }
        return true;
    }


    /**
     * \brief Tests whether all the vertices of a mesh are of degree 3.
     * \retval true if all the vertices are of degree 3
     * \retval false otherwise
     */
    bool mesh_vertices_are_degree_3(const Mesh& M) {
        vector<int> degree(M.vertices.nb(),0);
        for(index_t f=0; f<M.facets.nb(); ++f) {
            for(index_t c=M.facets.corners_begin(f); c<M.facets.corners_end(f); ++c) {
                ++degree[M.facet_corners.vertex(c)];
            }
        }
        for(index_t v=0; v<degree.size(); ++v) {
            if(degree[v] != 3) {
                return false;
            }
        }
        return true;
    }
}


int main(int argc, char** argv) {

    GEO::initialize();
    GEO::Logger::instance()->set_quiet(false);
    GEO::CmdLine::import_arg_group("standard");
    GEO::CmdLine::import_arg_group("algo");
    GEO::CmdLine::declare_arg_percent("size", 10.0, "elements size, in bbox diagonal percent");
    GEO::CmdLine::declare_arg("shrink", 0.9, "cells shrink");
    GEO::CmdLine::declare_arg("border_only", false, "output only RVC facets on the border");
    
    std::vector<std::string> filenames;
    if(!GEO::CmdLine::parse(argc, argv, filenames, "points_filename <cell_filename>")) {
        return 1;
    }
    
    if(filenames.size() != 1 && filenames.size() != 2) {
        return 1;
    }

    GEO::Mesh points;
    GEO::MeshIOFlags flags;
    flags.reset_element(GEO::MESH_FACETS);
    flags.reset_element(GEO::MESH_CELLS);    
    GEO::mesh_load(filenames[0], points, flags);
    GEO::mesh_repair(points);

    double diag = GEO::bbox_diagonal(points);
    double size = GEO::CmdLine::get_arg_percent("size",diag);
    double shrink = GEO::CmdLine::get_arg_double("shrink");
    bool border_only = GEO::CmdLine::get_arg_bool("border_only");
    
    //   Since we compute restricted Voronoi cells one cell at a
    // time, the mesh argument of the restricted Voronoi diagram
    // is not used.
    GEO::Mesh dummy_mesh;
    
    //  Create a Delaunay API that encapsulates a Kd-tree
    GEO::Delaunay_var delaunay = Delaunay::create(3,"NN");
    delaunay->set_vertices(points.vertices.nb(), points.vertices.point_ptr(0));
    
    GEO::RestrictedVoronoiDiagram_var RVD = 
        GEO::RestrictedVoronoiDiagram::create(delaunay, &dummy_mesh);
    
    GEO::Mesh cell;
    GEO::Mesh clipped;
    GEO::Attribute<signed_index_t> facet_id;
    if(border_only) {
        facet_id.bind(clipped.facets.attributes(),"id");
    }
    
    if(filenames.size() == 2) {
        mesh_load(filenames[1],cell);
    } else {
        initialize_mesh_with_box(cell);
    }

    if(!mesh_vertices_are_degree_3(cell)) {
        Logger::err("RVC") << "Mesh vertices are not all of degree 3" << std::endl;
        exit(-1);
    }
    
    if(mesh_facets_are_planar(cell)) {
        Logger::out("RVC") << "Mesh facets are planar (good)" << std::endl;
    } else {
        Logger::warn("RVC") << "Mesh facets are not planar" << std::endl;        
    }
    
    std::ofstream out("RVC.obj");
    index_t offset = 1;

    index_t progress_divider =
        (points.vertices.nb() > 10000) ? 100 : 1;

    GEO::ProgressTask task("RVC.obj",points.vertices.nb()/progress_divider);
    //   For each point, create a cube centered on the point
    // and clip it with the Voronoi cell of the point.
    for(GEO::index_t i=0; i<points.vertices.nb(); ++i) {

        if(!(i%progress_divider)) {
            task.progress(i/progress_divider);
        }
        
        center_scale_mesh(cell, points.vertices.point(i), size);
        RVD->compute_RVC(i,cell,clipped,facet_id.is_bound());

        if(shrink != 1.0) {
            shrink_mesh(clipped, shrink);
        }

        //  Append the generated mesh to the output mesh.
        for(index_t j=0; j<clipped.vertices.nb(); ++j) {
            out << "v " << clipped.vertices.point(j) << std::endl;
        }
        for(index_t f=0; f<clipped.facets.nb(); ++f) {
            if(border_only && facet_id[f] >= 0) {
                continue;
            }
            out << "f ";
            for(index_t c=clipped.facets.corners_begin(f); c<clipped.facets.corners_end(f); ++c) {
                out << clipped.facet_corners.vertex(c) + offset << " ";
            }
            out << std::endl;
        }
        offset += clipped.vertices.nb();
    }
    
    return 0;
}
