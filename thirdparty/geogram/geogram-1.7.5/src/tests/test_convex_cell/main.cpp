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
#include <geogram/basic/line_stream.h>
#include <geogram/basic/stopwatch.h>
#include <geogram/basic/geometry.h>
#include <geogram/voronoi/generic_RVD_cell.h>
#include <geogram/mesh/mesh_halfedges.h>
#include <geogram/mesh/mesh_io.h>
#include <geogram/mesh/mesh_topology.h>
#include <geogram/mesh/mesh_geometry.h>

namespace {

    using namespace GEO;
    using GEOGen::ConvexCell;

    /**
     * \brief Creates a mesh from a cube.
     * \details Creates a mesh that represents a cube centered on the
     * origin, with a edge length of 2.
     * \param[out] M the resulting mesh
     */
    void initialize_mesh_with_box(Mesh& M) {
        M.clear();;;
        M.vertices.set_dimension(3);

        M.vertices.create_vertex(vec3(-1, -1, -1).data());
        M.vertices.create_vertex(vec3(-1, -1, 1).data());
        M.vertices.create_vertex(vec3(-1, 1, -1).data());
        M.vertices.create_vertex(vec3(-1, 1, 1).data());
        M.vertices.create_vertex(vec3(1, -1, -1).data());
        M.vertices.create_vertex(vec3(1, -1, 1).data());
        M.vertices.create_vertex(vec3(1, 1, -1).data());
        M.vertices.create_vertex(vec3(1, 1, 1).data());

        M.facets.create_quad(7,6,2,3);
        M.facets.create_quad(1,3,2,0);
        M.facets.create_quad(5,7,3,1);
        M.facets.create_quad(4,6,7,5);
        M.facets.create_quad(4,5,1,0);
        M.facets.create_quad(6,4,0,2);

        M.facets.connect();
    }

    /**
     * \brief Saves a Delaunay triangulation to a file
     * \details Saves the vertices of the Delaunay triangulation \p delaunay
     * to file \p filename;
     * \param[in] delaunay a Delaunay triangulation
     * \param[in] filename path to the file to save
     * \retval true if the file could be successfully saved
     * \retval false otherwise
     */
    bool save_points(const Delaunay* delaunay, const std::string& filename) {
        std::ofstream out(filename.c_str());
        if(!out) {
            Logger::out("I/O")
                << "Could not save points to file " << filename << "..."
                << std::endl;
            return false;
        }

        Logger::out("I/O")
            << "Saving points to file " << filename << "..."
            << std::endl;

        out << delaunay->nb_vertices() << std::endl;
        for(index_t i = 0; i < delaunay->nb_vertices(); ++i) {
            for(coord_index_t c = 0; c < delaunay->dimension(); ++c) {
                out << delaunay->vertex_ptr(i)[c] << ' ';
            }
            out << std::endl;
        }
        return true;
    }

    /**
     * \brief Loads a XYZ point file
     * \details Load points from file \p filename in XYZ format and stores the
     * points coordinates to output vector \p points.
     * \param[out] points output vector of points coordinates
     * \param[in] filename path to the file to load
     * \retval true if the file could be successfully loaded
     * \retval false otherwise
     */
    bool load_points(
        vector<double>& points,
        const std::string& filename
    ) {
        try {
            LineInput in(filename);
            if(!in.OK()) {
                return false;
            }
            index_t nb_points = 0;
            index_t cur = 0;
            while(!in.eof() && in.get_line()) {
                in.get_fields();
                switch(in.nb_fields()) {
                    case 1:
                    {
                        nb_points = in.field_as_uint(0);
                        points.resize(3 * nb_points);
                    } break;
                    case 3:
                    {
                        if(cur >= nb_points) {
                            Logger::err("I/O")
                                << "too many points in .xyz file"
                                << std::endl;
                            return false;
                        }
                        points[3 * cur] = in.field_as_double(0);
                        points[3 * cur + 1] = in.field_as_double(1);
                        points[3 * cur + 2] = in.field_as_double(2);
                        ++cur;
                    } break;
                    default:
                    {
                        Logger::err("I/O")
                            << "invalid number of fields in .xyz file"
                            << std::endl;
                        return false;
                    }
                }
            }
        }
        catch(const std::exception& ex) {
            Logger::err("I/O") << ex.what() << std::endl;
            return false;
        }
        return true;
    }

    /**
     * \brief Creates a random point-set on the surface of a sphere
     * \details Generates \p nb_vertices vertices randomly distributed on the
     * surface of a sphere centered on [0.5, 0.5, 0.5] with radius 1. The
     * generated vertices are stored to output vector \p vertices.
     * \param[out] vertices output vector of vertices coordinates
     * \param[in] nb_vertices number of vertices to generate
     */
    void init_sphere(vector<double>& vertices, index_t nb_vertices) {
        vertices.resize((nb_vertices + 1) * 3);
        vertices[0] = 0.0;
        vertices[1] = 0.0;
        vertices[2] = 0.0;
        for(index_t i = 1; i < nb_vertices + 1; ++i) {
            vec3 v(
                Numeric::random_float64(),
                Numeric::random_float64(),
                Numeric::random_float64()
            );
            v = normalize(v - vec3(0.5, 0.5, 0.5));
            vertices[3 * i] = v[0];
            vertices[3 * i + 1] = v[1];
            vertices[3 * i + 2] = v[2];
        }
    }

    /**
     * \brief Creates a random point-set on the surface of a cone
     * \details Generates \p nb_vertices vertices randomly distributed on the
     * surface of a cone. The generated vertices are stored to output vector
     * \p vertices.
     * \param[out] vertices output vector of vertices coordinates
     * \param[in] nb_vertices number of vertices to generate
     * \param[in] use_random_vertices if true, random angular sampling is used
     */
     void init_cone(
        vector<double>& vertices, index_t nb_vertices, bool use_random_vertices=true
     ) {
        vertices.resize((nb_vertices + 1) * 3);
        vertices[0] = 0.0;
        vertices[1] = 0.0;
        vertices[2] = 0.0;
        for(index_t i = 1; i < nb_vertices + 1; ++i) {
            vec3 N;
            if(use_random_vertices) {
                N = vec3(
                    Numeric::random_float64(), Numeric::random_float64(), 0.0
                );
                N = 2.0 * N - vec3(1.0, 1.0, 0.0);
            } else {
                double s = ::sin(double(i) * 2.0 * M_PI / double(nb_vertices - 1));
                double c = ::cos(double(i) * 2.0 * M_PI / double(nb_vertices - 1));
                N = vec3(s, c, 0.0);
            }
            N = normalize(N);
            N = N - vec3(0.0, 0.0, 1.0);
            vertices[3 * i] = N[0];
            vertices[3 * i + 1] = N[1];
            vertices[3 * i + 2] = N[2];
        }
    }
}

int main(int argc, char** argv) {

    using namespace GEO;
    using GEOGen::ConvexCell;

    GEO::initialize();
    int result = 0;

    try {

        Stopwatch W("Total time");

        std::vector<std::string> filenames;
        std::string output_filename = "out.eobj";
        std::string points_filename;

        CmdLine::import_arg_group("standard");
        CmdLine::import_arg_group("algo");

        CmdLine::declare_arg("nb_clip", 10, "number of clipping planes");
        CmdLine::declare_arg(
            "nb_clip_times", 1, "number of times clipping is done"
        );
        CmdLine::declare_arg(
            "shape", "sphere", "one of sphere,cone,file"
        );
        CmdLine::declare_arg(
            "save_points", false, "save generated points into points.xyz"
        );
        CmdLine::declare_arg(
            "integer", false, "integer coordinates only"
        );
        CmdLine::declare_arg(
            "lrs", false, "generate output for lrs (lrs.ine file)"
        );

	CmdLine::declare_arg(
	    "integer_coord_mul", 1e6, "multiplicative factor before integer conversion"
	);

	CmdLine::declare_arg(
	    "integer_Ncoord_mul", 1e6, "multiplicative factor before normal vector integer conversion"
	);
	
        if(
            !CmdLine::parse(
                argc, argv, filenames, "<pointsfile> <outputfile>"
            )
        ) {
            return 1;
        }

        Delaunay_var delaunay = Delaunay::create(3);
        index_t nb_vertices = CmdLine::get_arg_uint("nb_clip");
        index_t nb_times = CmdLine::get_arg_uint("nb_clip_times");
        std::string shape = CmdLine::get_arg("shape");
        bool exact = (CmdLine::get_arg("algo:predicates") == "exact");

        if(exact) {
            Logger::out("Predicates")
                << "Using exact predicates" << std::endl;
        }

        vector<double> vertices;
        if(shape == "sphere") {
            Logger::out("Program")
                << "Using random sphere shape" << std::endl;
            init_sphere(vertices, nb_vertices);
        } else if(shape == "cone") {
            Logger::out("Program")
                << "Using random cone shape" << std::endl;
            init_cone(vertices, nb_vertices);
        } else if(shape == "file") {
            Logger::out("Program")
                << "Taking shape from file" << std::endl;

            if(filenames.size() == 0) {
                Logger::err("Program")
                    << "Missing input shape file argument" << std::endl;
                return 1;
            }

            points_filename = filenames[0];
            filenames.erase(filenames.begin());

            Logger::out("I/O")
                << "Loading shape from file " << points_filename
                << std::endl;

            if(!load_points(vertices, points_filename)) {
                Logger::err("I/O")
                    << "Could not load file: " << points_filename
                    << std::endl;
                return 1;
            }
            nb_vertices = (vertices.size() / 3) - 1;
        } else {
            Logger::err("Program")
                << shape << ": invalid shape" << std::endl;
            return 1;
        }

        if(filenames.size() >= 1) {
            output_filename = filenames[0];
        }

        if(filenames.size() > 1) {
            Logger::warn("Program")
                << "Extraneous files ignored" << std::endl;
        }

        delaunay->set_vertices(nb_vertices + 1, &(vertices[0]));

        if(CmdLine::get_arg_bool("save_points")) {
            if(!save_points(delaunay, "points.xyz")) {
                return 1;
            }
        }

        CmdLine::set_arg("nb_clip", delaunay->nb_vertices() - 1);

        Mesh M;
        initialize_mesh_with_box(M);
        ConvexCell C(3);
        C.initialize_from_surface_mesh(&M, true);

        for(index_t k = 0; k < nb_times; ++k) {
            for(index_t i = 1; i < nb_vertices; ++i) {
                C.clip_by_plane<3>(&M, delaunay, 0, i, exact, exact);
            }
        }

        Mesh C_mesh;
        C.convert_to_mesh(&C_mesh);

	
	double coord_scale = CmdLine::get_arg_double("integer_coord_mul");
	double N_scale = CmdLine::get_arg_double("integer_Ncoord_mul");

	bool integer_mode = CmdLine::get_arg_bool("integer");
	
	if(integer_mode) {
	    double xyz_min[3];
	    double xyz_max[3];
	    get_bbox(C_mesh, xyz_min, xyz_max);
	    double R = xyz_max[0]-xyz_min[0];
	    R = std::max(R, xyz_max[1]-xyz_min[1]);
	    R = std::max(R, xyz_max[2]-xyz_min[2]);	    
	    FOR(v,C_mesh.vertices.nb()) {
		double* p = C_mesh.vertices.point_ptr(v);
		FOR(c,3) {
		    p[c] = (p[c] - xyz_min[c]) * coord_scale / R;
		}
	    }
	} else {
	    coord_scale = 1.0;
	    N_scale = 1.0;
	}

	if(CmdLine::get_arg_bool("lrs")) {
	    Logger::out("ConvexCell") << "Generating lrs.ine" << std::endl;
	    std::ofstream out("lrs.ine");
	    out << "cell" << std::endl;
	    out << "*convex cell output converted to LRS format" << std::endl;
	    out << "H-representation" << std::endl;
	    out << "begin" << std::endl;
	    out << C_mesh.facets.nb() << " " << 4 << " " << "rational" << std::endl;
	    FOR(f,C_mesh.facets.nb()) {
		index_t n = C_mesh.facets.nb_vertices(f);
		vec3 N(0.0, 0.0, 0.0);
		vec3 g(0.0, 0.0, 0.0);
		FOR(lv,n) {
		    index_t v1 = C_mesh.facets.vertex(f,lv);
		    index_t v2 = C_mesh.facets.vertex(f,(lv+1)%n);
		    index_t v3 = C_mesh.facets.vertex(f,(lv+2)%n);
		    vec3 p1(C_mesh.vertices.point_ptr(v1));
		    vec3 p2(C_mesh.vertices.point_ptr(v2));
		    vec3 p3(C_mesh.vertices.point_ptr(v3));
		    N += cross(p1-p2,p3-p2);
		    g += p1;
		}
		g = (1.0 / double(n)) * g;
		N = normalize(N);
		N = N_scale * N;
		double d = -dot(N,g);
		if(integer_mode) {
		    out << long(d) << " " << long(N.x) << " " << long(N.y) << " " << long(N.z) << std::endl;
		} else {
		    out << d << " " << N.x << " " << N.y << " " << N.z << std::endl;		    
		}
	    }
	    out << "end" << std::endl;	    
	}
	
        Logger::out("I/O")
            << "Saving mesh to file " << output_filename
            << std::endl;

        if(!mesh_save(C_mesh, output_filename)) {
            return 1;
        }

        if(!meshes_have_same_topology(M, C_mesh, true)) {
            return 1;
        }
    }
    catch(const std::exception& e) {
        std::cerr << "Received an exception: " << e.what() << std::endl;
        return 1;
    }

    return result;
}

