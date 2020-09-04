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

#include <geogram/mesh/mesh_remesh.h>
#include <geogram/mesh/mesh.h>
#include <geogram/mesh/mesh_repair.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/mesh_halfedges.h>
#include <geogram/mesh/mesh_reorder.h>
#include <geogram/mesh/mesh_preprocessing.h>
#include <geogram/mesh/mesh_io.h>
#include <geogram/voronoi/CVT.h>
#include <geogram/basic/command_line.h>
#include <geogram/basic/stopwatch.h>
#include <geogram/basic/progress.h>
#include <geogram/bibliography/bibliography.h>

/****************************************************************************/

namespace GEO {

    void remesh_smooth(
        Mesh& M_in, Mesh& M_out,
        index_t nb_points,
        coord_index_t dim,
        index_t nb_Lloyd_iter,
        index_t nb_Newton_iter,
        index_t Newton_m
    ) {

        geo_cite("DBLP:journals/cgf/YanLLSW09");
        geo_cite("DBLP:conf/imr/LevyB12");
	
        if(dim == 0) {
            dim = coord_index_t(M_in.vertices.dimension());
        }

	geo_argused(dim); // See TODO later.
	
        Stopwatch W("Remesh");

        CentroidalVoronoiTesselation CVT(&M_in);
       
        /*
         * TODO: reactivate projection, debug
         vector<vec3> R3_embedding(M_in.vertices.nb());
         for(index_t i=0; i<M_in.vertices.nb(); ++i) {
             R3_embedding[i] = vec3(M_in.vertices.point_ptr(i));
         }
         CentroidalVoronoiTesselation CVT(&M_in, R3_embedding, dim);
        */
       
        if(nb_points == 0) {
            nb_points = M_in.vertices.nb();
        }
        CVT.compute_initial_sampling(nb_points);

        try {
            ProgressTask progress("Lloyd", 100);
            CVT.set_progress_logger(&progress);
            CVT.Lloyd_iterations(nb_Lloyd_iter);
        }
        catch(const TaskCanceled&) {
            // TODO_CANCEL
        }

        if(nb_Newton_iter != 0) {
            try {
                ProgressTask progress("Newton", 100);
                CVT.set_progress_logger(&progress);
                CVT.Newton_iterations(nb_Newton_iter, Newton_m);
            }
            catch(const TaskCanceled&) {
                // TODO_CANCEL
            }
        }

        if(M_in.vertices.dimension() == 6 &&
           CmdLine::get_arg_bool("dbg:save_6d")
        ) {
            Logger::out("Remesh")
                << "Saving source mesh into mesh6.obj6" << std::endl;
            mesh_save(M_in, "mesh6.obj6");
            Logger::out("Remesh")
                << "Saving sampling into points6.txt" << std::endl;
            std::ofstream out("points6.txt");
            out << CVT.delaunay()->nb_vertices() << std::endl;
            for(index_t i = 0; i < CVT.delaunay()->nb_vertices(); i++) {
                for(coord_index_t c = 0; c < 6; c++) {
                    out << CVT.delaunay()->vertex_ptr(i)[c] << " ";
                }
                out << std::endl;
            }
        }

        // Delete auxiliary storage used for each threads (it uses a lot of RAM,
        //   we need this RAM to create the surface now...)
        CVT.RVD()->delete_threads();

        CVT.set_use_RVC_centroids(
            CmdLine::get_arg_bool("remesh:RVC_centroids")
        );
        bool multi_nerve = CmdLine::get_arg_bool("remesh:multi_nerve");

        Logger::out("Remesh") << "Computing RVD..." << std::endl;

        CVT.compute_surface(&M_out, multi_nerve);
        if(CmdLine::get_arg_bool("dbg:save_ANN_histo")) {
            Logger::out("ANN")
                << "Saving histogram to ANN_histo.dat" << std::endl;
            std::ofstream out("ANN_histo.dat");
            CVT.delaunay()->save_histogram(out);
        }
    }

    /************************************************************************/
}

