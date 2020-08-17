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

#include <geogram_gfx/gui/simple_mesh_application.h>

#include <geogram/mesh/mesh_repair.h>
#include <geogram/mesh/mesh_fill_holes.h>
#include <geogram/mesh/mesh_degree3_vertices.h>
#include <geogram/mesh/mesh_intersection.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/mesh_preprocessing.h>
#include <geogram/mesh/mesh_remesh.h>
#include <geogram/mesh/mesh_decimate.h>
#include <geogram/mesh/mesh_tetrahedralize.h>
#include <geogram/mesh/mesh_topology.h>
#include <geogram/mesh/mesh_AABB.h>

#include <geogram/delaunay/LFS.h>

#include <geogram/points/co3ne.h>

#include <geogram/basic/command_line.h>
#include <geogram/basic/command_line_args.h>
#include <geogram/basic/file_system.h>

namespace {
    using namespace GEO;

    /**
     * \brief Graphic interface to geometry processing functionalities
     *  of geogram.
     */
    class GeoBoxApplication : public SimpleMeshApplication {
    public:

        /**
         * \brief GeoBoxApplication constructor.
         */
        GeoBoxApplication() : SimpleMeshApplication("GeoBox") {
        }

	void geogram_initialize(int argc, char** argv) override {
	    GEO::initialize();
	    GEO::CmdLine::import_arg_group("co3ne");
            GEO::CmdLine::import_arg_group("pre");
            GEO::CmdLine::import_arg_group("post");
            GEO::CmdLine::import_arg_group("remesh");
            GEO::CmdLine::import_arg_group("opt");
            GEO::CmdLine::import_arg_group("tet");
	    SimpleMeshApplication::geogram_initialize(argc, argv);
	}

        void draw_about() override {
            ImGui::Separator();            
            if(ImGui::BeginMenu(icon_UTF8("info") + " About...")) {
                ImGui::Text(
                    "             GEObox\n"
                    "  The geometry processing toolbox\n"
                    "\n"
                    );
		float sz = float(280.0 * std::min(scaling(), 2.0));		
                ImGui::Image(
                    convert_to_ImTextureID(geogram_logo_texture_),
                    ImVec2(sz, sz)
                );
                ImGui::Text(
                    "\n"
                    "With algorithms from:\n"
                    "\n"
                    "* ERC StG GOODSHAPE (ERC-StG-205693) \n"
                    "* ERC PoC VORPALINE (ERC-PoC-334829) \n"
                    "\n"
                    "  ...as well as new ones.\n"
                    "\n"
                );
                ImGui::Separator();
                ImGui::Text(
#ifdef GEO_OS_EMSCRIPTEN
                    "This version runs in your webbrowser\n"
                    "using Emscripten.\n"
                    "To get a (faster!) native executable\n"
                    "and the sources, see:"
#endif
                    "\n"
                    "       Project's homepage:\n"
                    "  alice.loria.fr/software/geogram/\n"
                    "\n"
                    "       (C)opyright 2006-2016\n"
                    "      The ALICE project, Inria\n"
                );
                ImGui::Text("\n");
                ImGui::Separator();
                ImGui::Text(
                    "%s",
                    (
                        "GEOGRAM version:" +
                        Environment::instance()->get_value("version")
                    ).c_str()
                );
                ImGui::EndMenu();
            }
        }
        
        /**
         * \brief Draws and manages the menus and the commands.
         * \details Overloads Application::draw_application_menus()
         */
        void draw_application_menus() override {
            if(ImGui::BeginMenu("Points")) {
                if(ImGui::MenuItem("smooth point set")) {
                    GEO::Command::set_current(
                        "void smooth(                                         "
                        "   index_t nb_iterations=2 [number of iterations],   "
                        "   index_t nb_neighbors=30 [number of nearest neigh.]"
                        ") [smoothes a pointset]",
                        this, &GeoBoxApplication::smooth_point_set
                    );
                }
                
                if(ImGui::MenuItem("reconstruct surface")) {
                    GEO::Command::set_current(
                "void reconstruct(                                    "
                "   double radius=5.0       [search radius (in % bbox. diag.)],"
                "   index_t nb_smth_iter=2  [number of smoothing iterations],  "
                "   index_t nb_neighbors=30 [number of nearest neighbors]      "
                ") [reconstructs a surface from a pointset]",
                        this, &GeoBoxApplication::reconstruct
                    );
                }
                ImGui::EndMenu();
            }

            
            if(ImGui::BeginMenu("Surface")) {
		ImGui::MenuItem("    Repair", nullptr, false, false);
		
		if(ImGui::MenuItem("repair surface")) {
		    GEO::Command::set_current(
                " void repair(                        "
                "   double epsilon = 1e-6 [point merging tol. (% bbox. diag.)],"
                "   double min_comp_area = 0.03                   "
                "        [for removing small cnx (% total area)], "
                "   double max_hole_area = 1e-3                   "
                "        [for filling holes (% total area)],      "
                "   index_t max_hole_edges = 2000                 "
                "        [max. nb. edges in filled hole],         "
                "   double max_degree3_dist = 0.0                 "
                "        [for removing deg3 vrtx (% bbox. diag.)],"
                "   bool remove_isect = false                     "
                "        [remove intersecting triangles]          "
                " ) [repairs a surfacic mesh]",
                            this, &GeoBoxApplication::repair_surface
                        );
		}

		if(ImGui::MenuItem("merge vertices")) {
		    GEO::Command::set_current(
                "void merge_vertices(                                       "
                "   double epsilon=1e-6                                     "
                "     [tolerance for merging vertices (in % bbox diagonal)],"
                ") [merges the vertices that are within tolerance]          ",
                            this, &GeoBoxApplication::merge_vertices
                    );
		}
		
		ImGui::Separator();
		ImGui::MenuItem("    Remesh", nullptr, false, false);
		
		if(ImGui::MenuItem("remesh smooth")) {
		    GEO::Command::set_current(
                    "void remesh_smooth(                                      "
#ifdef GEO_OS_EMSCRIPTEN                    
                    "  index_t nb_points = 5000  [number of points in remesh],"
#else
                    "  index_t nb_points = 30000 [number of points in remesh],"
#endif                    
                    "  double tri_shape_adapt = 1.0                           "
                    "          [triangles shape adaptation],                  "
                    "  double tri_size_adapt = 0.0                            "
                    "          [triangles size adaptation],                   "
                    "  index_t normal_iter = 3 [nb normal smoothing iter.],   "
                    "  index_t Lloyd_iter = 5 [nb Lloyd iter.],               "
                    "  index_t Newton_iter = 30 [nb Newton iter.],            "
                    "  index_t Newton_m = 7 [nb Newton eval. per step],       "
                    "  index_t LFS_samples = 10000                            "
                    "    [nb samples (used if size adapt != 0)]               "
                    ")",
                             this, &GeoBoxApplication::remesh_smooth  
                    );
		}

		if(ImGui::MenuItem("decimate")) {
		    GEO::Command::set_current(
                    "void decimate(                                            "
                    "   index_t nb_bins = 100  [the higher-the more precise],  "
                    "   bool remove_deg3_vrtx = true [remove degree3 vertices],"
                    "   bool keep_borders = true,                              "
                    "   bool repair = true                                     "
                    ") [quick and dirty mesh decimator (vertex clustering)]",
                             this, &GeoBoxApplication::decimate
                    );
		}
		
		ImGui::Separator();
		ImGui::MenuItem("    Create...", nullptr, false, false);
		
		if(ImGui::MenuItem("create cube")) {
		    GEO::Command::set_current(
			"void create_cube("
			"    double x1=0, double y1=0, double z1=0,"
			"    double x2=1, double y2=1, double z2=1"
			")",
			this, &GeoBoxApplication::create_cube
                    );
		}
		if(ImGui::MenuItem("create icosahedron")) {
		    create_icosahedron();
		}
		ImGui::EndMenu();
	    }
            
            if(ImGui::BeginMenu("Volume")) {
                if(ImGui::MenuItem("tet meshing")) {
                    Command::set_current(
                "void tet_meshing("
                "    bool preprocess=true [preprocesses the surface],        "
                "    bool refine=true     [insert points to improve quality],"
                "    double quality=1.0   [the smaller - the higher quality],"
                "    bool verbose=false   [enable tetgen debug messages]     "
                ") [Fills-in a closed mesh with tets, using tetgen]",
                         this,&GeoBoxApplication::tet_meshing
                    );
                }
                ImGui::EndMenu();
            }
            
            if(ImGui::BeginMenu("Mesh")) {
		
		ImGui::MenuItem("    Stats", nullptr, false, false);
		
		if(ImGui::MenuItem("show mesh stats")) {
		    show_statistics();
		}
		if(ImGui::MenuItem("show mesh topo")) {
		    show_topology();
		}

		ImGui::Separator();
		ImGui::MenuItem("    Edit", nullptr, false, false);		
		    
                if(ImGui::MenuItem("clear")) {
                    Command::set_current(
                        "void clear(bool yes_I_am_sure=false) "
                        "[removes all elements from the mesh]",
                        this, &GeoBoxApplication::clear
                    );
                }
                
                if(ImGui::MenuItem("remove elements")) {
                    Command::set_current(
                "void remove_elements(                                   "
                "    bool vertices=false   [removes everyting],          "
                "    bool edges=false      [removes mesh edges],         "
                "    bool facets=false     [removes the surfacic part],  "
                "    bool cells=false      [removes the volumetric part],"
                "    bool kill_isolated_vx=false [kill isolated vertices]"
                ") [removes mesh elements]",
                        this, &GeoBoxApplication::remove_elements
                    );
                }
                
                if(ImGui::MenuItem("remove isolated vrtx")) {
                    Command::set_current(
                "void remove_isolated_vertices(bool yes_I_am_sure=false) "
                "[removes vertices that are not connected to any element]",
                        this, &GeoBoxApplication::remove_isolated_vertices
                    );
                }

		ImGui::Separator();
		ImGui::MenuItem("    Selection", nullptr, false, false);				
		
		if(ImGui::MenuItem("select all vrtx")) {
		    select_all_vertices();
		}
		if(ImGui::MenuItem("unselect all vrtx")) {
		    unselect_all_vertices();
		}
		if(ImGui::MenuItem("invert vrtx sel")) {
		    invert_vertices_selection();
		}
		if(ImGui::MenuItem(
		       "sel vrtx on surf brdr")
		) {
		    select_vertices_on_surface_border();
		}
		if(ImGui::MenuItem("unsel vrtx on surf brdr")) {
		    unselect_vertices_on_surface_border();
		}
		if(ImGui::MenuItem("delete sel vrtx")) {
		    delete_selected_vertices();
		}
                ImGui::EndMenu();
            }

            if(ImGui::BeginMenu("Attributes")) {
                if(ImGui::MenuItem("compute LFS")) {
                    Command::set_current(
                        "compute_local_feature_size(std::string attribute_name=\"LFS\")",
                        this, &GeoBoxApplication::compute_local_feature_size
                    );
                }
                if(ImGui::MenuItem("compute dist. to brdr")) {
                    Command::set_current(
                        "compute_distance_to_border(std::string attribute_name=\"distance\")",
                        this, &GeoBoxApplication::compute_distance_to_border
                    );
                }
                ImGui::EndMenu();
            }
        }

        void clear(bool are_you_sure = false) {
            if(are_you_sure) {
                mesh()->clear();
                mesh()->vertices.set_single_precision();
                mesh_gfx()->set_mesh(mesh());
            }
        }

	bool load(const std::string& filename) override {
	    bool result = SimpleMeshApplication::load(filename);
	    if(result && FileSystem::extension(filename) == "stl") {
		mesh_.vertices.set_double_precision();	
		mesh_repair(mesh_);
		mesh_.vertices.set_single_precision();			
	    }
	    return result;
	}
	
        void remove_elements(
            bool vertices=false,
            bool edges=false,
            bool facets=false,
            bool cells=false,
            bool kill_isolated_vx=false
        ) {
            if(vertices) {
                mesh()->clear();
            } else {
                if(facets) {
                    mesh()->facets.clear();
                }
                if(edges) {
                    mesh()->edges.clear();
                }
                if(cells) {
                    mesh()->cells.clear();
                }
                if(kill_isolated_vx) {
                    mesh()->vertices.remove_isolated();
                }
            }
            if(mesh()->facets.nb() == 0 && mesh()->cells.nb() == 0) {
                show_vertices();
            }
            mesh_gfx()->set_mesh(mesh());
        }

        void remove_isolated_vertices(bool yes_I_am_sure = false) {
            if(yes_I_am_sure) {
                mesh()->vertices.remove_isolated();
                mesh_gfx()->set_mesh(mesh());
            }
        }

        void show_statistics() {
            show_console();
            mesh()->show_stats("Mesh");
        }

        void show_topology() {
            show_console();
            Logger::out("MeshTopology/surface")
                << "Nb components = "
                << mesh_nb_connected_components(*mesh())
                << std::endl;
            Logger::out("MeshTopology/surface")
                << "Nb borders = "
                << mesh_nb_borders(*mesh())
                << std::endl;
            Logger::out("MeshTopology/surface")
                << "Xi = "
                << mesh_Xi(*mesh())
                << std::endl;
        }

        void smooth_point_set(
            index_t nb_iterations=2, index_t nb_neighbors=30
        ) {
            begin();
            if(nb_iterations != 0) {
                Co3Ne_smooth(*mesh(), nb_neighbors, nb_iterations);
            }
            end();
        }

        void reconstruct(
            double radius=5.0,
            index_t nb_iterations=0, index_t nb_neighbors=30
        ) {
            hide_surface();
            begin();
            double R = bbox_diagonal(*mesh());
            mesh_repair(*mesh(), MESH_REPAIR_COLOCATE, 1e-6*R);
            radius *= 0.01 * R;
            if(nb_iterations != 0) {
                Co3Ne_smooth(*mesh(), nb_neighbors, nb_iterations);
            }
            Co3Ne_reconstruct(*mesh(), radius);
            end();
            hide_vertices();
            show_surface();
        }
        
        void repair_surface(
            double epsilon = 1e-6,
            double min_comp_area = 0.03,
            double max_hole_area = 1e-3,
            index_t max_hole_edges = 2000,
            double max_degree3_dist = 0.0,
            bool remove_isect = false
        ) {
            begin();

            double bbox_diagonal = GEO::bbox_diagonal(*mesh());
            epsilon *= (0.01 * bbox_diagonal);
            double area = Geom::mesh_area(*mesh(),3);
            min_comp_area *= area;
            max_hole_area *= area;

            mesh_repair(*mesh(), MESH_REPAIR_DEFAULT, epsilon);

            if(min_comp_area != 0.0) {
                double nb_f_removed = mesh()->facets.nb();
                remove_small_connected_components(*mesh(), min_comp_area);
                nb_f_removed -= mesh()->facets.nb();
                if(nb_f_removed != 0) {
                    mesh_repair(
                        *mesh(), MESH_REPAIR_DEFAULT, epsilon
                    );
                }
            }

            if(max_hole_area != 0.0 && max_hole_edges != 0) {
                fill_holes(
                    *mesh(), max_hole_area, max_hole_edges
                );
            }

            if(max_degree3_dist > 0.0) {
                max_degree3_dist *= (0.01 * bbox_diagonal);
                remove_degree3_vertices(*mesh(), max_degree3_dist);
            }
        
            if(remove_isect) {
                Logger::out("Mesh") << "Removing intersections" << std::endl;
                mesh_remove_intersections(*mesh());
                Logger::out("Mesh") << "Removed intersections" << std::endl;
            }
            end();
        }

        void merge_vertices(
            double epsilon=1e-6
        ) {
            begin();
            epsilon *= (0.01 * bbox_diagonal(*mesh()));
            mesh_repair(*mesh(), MESH_REPAIR_DEFAULT, epsilon);
            end();
        }


        void remesh_smooth(
            index_t nb_points = 30000,
            double tri_shape_adapt = 1.0,
            double tri_size_adapt = 0.0,
            index_t normal_iter = 3,
            index_t Lloyd_iter = 5,
            index_t Newton_iter = 30,
            index_t Newton_m = 7,
            index_t LFS_samples = 10000
        ) {
            if(mesh()->facets.nb() == 0) {
                Logger::err("Remesh")
                    << "mesh has no facet" << std::endl;
                return;
            }
        
            if(!mesh()->facets.are_simplices()) {
                Logger::err("Remesh")
                    << "mesh need to be simplicial, use repair"
                    << std::endl;
                return;
            }
            
            begin();
            Mesh remesh;

            if(tri_shape_adapt != 0.0) {
                tri_shape_adapt *= 0.02;
                compute_normals(*mesh());
                if(normal_iter != 0) {
                    Logger::out("Nsmooth") << "Smoothing normals, "
                                                << normal_iter
                                                << " iteration(s)" << std::endl;
                    simple_Laplacian_smooth(
                        *mesh(), normal_iter, true
                    );
                }
                set_anisotropy(*mesh(), tri_shape_adapt);
            } else {
                mesh()->vertices.set_dimension(3);
            }

            if(tri_size_adapt != 0.0) {
                compute_sizing_field(
                    *mesh(), tri_size_adapt, LFS_samples
                );
            } else {
                AttributesManager& attributes =
                    mesh()->vertices.attributes();
                if(attributes.is_defined("weight")) {
                    attributes.delete_attribute_store("weight");
                }
            }
        
            GEO::remesh_smooth(
                *mesh(), remesh,
                nb_points, 0,
                Lloyd_iter, Newton_iter, Newton_m
            );

            MeshElementsFlags what = MeshElementsFlags(
                MESH_VERTICES | MESH_EDGES | MESH_FACETS | MESH_CELLS
            );
            mesh()->clear();
            mesh()->copy(remesh, true, what);
            
            end();
        }

        void decimate(
            index_t nb_bins = 100,
            bool remove_deg3_vrtx = true,
            bool keep_borders = true,
            bool repair = true
        ) {
            begin();
            MeshDecimateMode mode = MESH_DECIMATE_DUP_F;
            if(remove_deg3_vrtx) {
                mode = MeshDecimateMode(mode | MESH_DECIMATE_DEG_3);
            }
            if(keep_borders) {
                mode = MeshDecimateMode(mode | MESH_DECIMATE_KEEP_B);
            }
            mesh_decimate_vertex_clustering(*mesh(), nb_bins, mode);
            if(repair) {
                repair_surface();
            }
            end();
        }

        void create_cube(
            double x1=0, double y1=0, double z1=0,
            double x2=1, double y2=1, double z2=1
        ) {
            begin();
            Mesh& M = *mesh();
            if(M.vertices.dimension() < 3) {
                Logger::err("Mesh") << "Dimension smaller than 3"
                                    << std::endl;
                return;
            }

            index_t v0 = M.vertices.create_vertex(vec3(x1,y1,z1).data());
            index_t v1 = M.vertices.create_vertex(vec3(x1,y1,z2).data());
            index_t v2 = M.vertices.create_vertex(vec3(x1,y2,z1).data());
            index_t v3 = M.vertices.create_vertex(vec3(x1,y2,z2).data());
            index_t v4 = M.vertices.create_vertex(vec3(x2,y1,z1).data());
            index_t v5 = M.vertices.create_vertex(vec3(x2,y1,z2).data());
            index_t v6 = M.vertices.create_vertex(vec3(x2,y2,z1).data());
            index_t v7 = M.vertices.create_vertex(vec3(x2,y2,z2).data());
            
            M.facets.create_quad(v3,v7,v6,v2);
            M.facets.create_quad(v0,v1,v3,v2);
            M.facets.create_quad(v1,v5,v7,v3);
            M.facets.create_quad(v5,v4,v6,v7);
            M.facets.create_quad(v0,v4,v5,v1);
            M.facets.create_quad(v2,v6,v4,v0);

            M.facets.connect();
            end();
        }

        void create_icosahedron() {
            begin();
            Mesh& M = *mesh();
            if(M.vertices.dimension() < 3) {
                Logger::err("Mesh") << "Dimension smaller than 3"
                                    << std::endl;
                return;
            }
            
            static double points[] = {
                0,          0.0,       1.175571,
                1.051462,   0.0,       0.5257311,
                0.3249197,  1.0,       0.5257311,
                -0.8506508, 0.618034,  0.5257311,
                -0.8506508, -0.618034, 0.5257311,
                0.3249197,  -1.0,      0.5257311,
                0.8506508,  0.618034,  -0.5257311,
                0.8506508,  -0.618034, -0.5257311,
                -0.3249197,  1.0,      -0.5257311,
                -1.051462,   0.0,      -0.5257311,
                -0.3249197, -1.0,      -0.5257311,
                0.0,         0.0,      -1.175571
            };
        
            static index_t facets[] = {
                0,1,2,
                0,2,3,
                0,3,4,
                0,4,5,
                0,5,1,
                1,5,7,
                1,7,6,
                1,6,2,
                2,6,8,
                2,8,3,
                3,8,9,
                3,9,4,
                4,9,10,
                4,10,5,
                5,10,7,
                6,7,11,
                6,11,8,
                7,10,11,
                8,11,9,
                9,11,10,
            };

            index_t first_v = M.vertices.create_vertices(12);
            for(index_t v=0; v<12; ++v) {
                Geom::mesh_vertex_ref(M,first_v+v) =
                    vec3(points[3*v], points[3*v+1], points[3*v+2]) ;
            }

            for(index_t f=0; f<20; ++f) {
                M.facets.create_triangle(
                    first_v + facets[3*f],
                    first_v + facets[3*f+1],
                    first_v + facets[3*f+2]
                );
            }
            
            M.facets.connect();
            end();
        }

        void tet_meshing(
            bool preprocess=true,
            bool refine=true,
            double quality=1.0,
            bool verbose=false
        ) {
            if(verbose) {
                show_console();
            }
            hide_mesh();
            begin();
            CmdLine::set_arg("dbg:tetgen",verbose);        
            mesh()->cells.clear();
            mesh()->vertices.remove_isolated();
            mesh_tetrahedralize(*mesh(), preprocess, refine, quality);
            if(mesh()->cells.nb() != 0) {
                mesh()->cells.compute_borders();
            }
            end();
            show_volume();
        }

        void select_all_vertices() {
            Attribute<bool> v_selection(
                mesh()->vertices.attributes(), "selection"
            );
            for(index_t v=0; v<mesh()->vertices.nb(); ++v) {
                v_selection[v] = true;
            }
        }

        void unselect_all_vertices() {
            if(mesh()->vertices.attributes().is_defined("selection")) {
                mesh()->vertices.attributes().delete_attribute_store(
                    "selection"
                );
            }
        }

        void invert_vertices_selection() {
            Attribute<bool> v_selection(
                mesh()->vertices.attributes(), "selection"
            );
            for(index_t v=0; v<mesh()->vertices.nb(); ++v) {
                v_selection[v] = !v_selection[v];
            }
        }

        void select_vertices_on_surface_border() {
            Attribute<bool> v_selection(
                mesh()->vertices.attributes(), "selection"
            );
            for(index_t f=0; f<mesh()->facets.nb(); ++f) {
                for(
                    index_t c=mesh()->facets.corners_begin(f);
                    c < mesh()->facets.corners_end(f); ++c
                ) {
                    if(mesh()->facet_corners.adjacent_facet(c) == NO_FACET) {
                        v_selection[mesh()->facet_corners.vertex(c)] = true;
                    }
                }
            }
        }

        void unselect_vertices_on_surface_border() {
            Attribute<bool> v_selection(
                mesh()->vertices.attributes(), "selection"
            );
            for(index_t f=0; f<mesh()->facets.nb(); ++f) {
                for(
                    index_t c=mesh()->facets.corners_begin(f);
                    c < mesh()->facets.corners_end(f); ++c
                ) {
                    if(mesh()->facet_corners.adjacent_facet(c) == NO_FACET) {
                        v_selection[mesh()->facet_corners.vertex(c)] = false;
                    }
                }
            }
        }

        void delete_selected_vertices() {
            if(!mesh()->vertices.attributes().is_defined("selection")) {
                return;
            }

            Attribute<bool> v_selection(
                mesh()->vertices.attributes(), "selection"
            );


            {
                vector<index_t> delete_e(mesh()->edges.nb(),0);
                for(index_t e=0; e<mesh()->edges.nb(); ++e) {
                    if(
                        v_selection[mesh()->edges.vertex(e,0)] ||
                        v_selection[mesh()->edges.vertex(e,0)]
                    ) {
                        delete_e[e] = 1;
                    }
                }
                mesh()->edges.delete_elements(delete_e);
            }

            {
                vector<index_t> delete_f(mesh()->facets.nb(),0);
                for(index_t f=0; f<mesh()->facets.nb(); ++f) {
                    for(index_t lv=0; lv<mesh()->facets.nb_vertices(f); ++lv) {
                        if(v_selection[mesh()->facets.vertex(f,lv)]) {
                            delete_f[f] = 1;
                            break;
                        }
                    }
                }
                mesh()->facets.delete_elements(delete_f);
            }

            {
                vector<index_t> delete_c(mesh()->cells.nb(),0);
                for(index_t c=0; c<mesh()->cells.nb(); ++c) {
                    for(index_t lv=0; lv<mesh()->cells.nb_vertices(c); ++lv) {
                        if(v_selection[mesh()->cells.vertex(c,lv)]) {
                            delete_c[c] = 1;
                            break;
                        }
                    }
                }
                mesh()->cells.delete_elements(delete_c);
            }

            {
                vector<index_t> delete_v(mesh()->vertices.nb(),0);
                for(index_t v=0; v<mesh()->vertices.nb(); ++v) {
                    if(v_selection[v]) {
                        delete_v[v] = 1;
                    }
                }
                mesh()->vertices.delete_elements(delete_v);
            }
        }

        void compute_local_feature_size(std::string attribute_name) {
            begin();
            LocalFeatureSize LFS(
                mesh()->vertices.nb(), mesh()->vertices.point_ptr(0)
            );

            Attribute<double> lfs(
                mesh()->vertices.attributes(), attribute_name
            );

            for(index_t v=0; v<mesh()->vertices.nb(); ++v) {
                lfs[v] = ::sqrt(
                    LFS.squared_lfs(mesh()->vertices.point_ptr(v))
                );
            }
            end();
            set_attribute("vertices." + attribute_name);
            show_attributes();
        }

        void compute_distance_to_border(std::string attribute_name) {
            if(mesh()->cells.nb() == 0) {
                Logger::err("Distance") << "Mesh has no cell"
                                        << std::endl;
                return;
            }
            begin();
            Attribute<double> distance(
                mesh()->vertices.attributes(), attribute_name
            );
            MeshFacetsAABB AABB(*mesh());
            for(index_t v=0; v<mesh()->vertices.nb(); ++v) {
                distance[v] = ::sqrt(
                    AABB.squared_distance(vec3(mesh()->vertices.point_ptr(v)))
                );
            }
            end();
            set_attribute("vertices." + attribute_name);
            show_attributes();
        }
        
    protected:
        
        void hide_mesh() {
            mesh_gfx()->set_mesh(nullptr);            
        }
        
        void begin() {
            mesh()->vertices.set_double_precision();            
        }

        void end() {
            orient_normals(*mesh());            
            mesh()->vertices.set_single_precision();
            mesh_gfx()->set_mesh(mesh());
            if(mesh()->facets.nb() == 0 && mesh()->cells.nb() == 0) {
                show_vertices();
            }
        }
        
    };
}

int main(int argc, char** argv) {
    GeoBoxApplication app;
    app.start(argc, argv);
    return 0;
}
