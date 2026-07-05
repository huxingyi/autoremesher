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
#include <geogram/mesh/mesh_baking.h>
#include <geogram/mesh/mesh_manifold_harmonics.h>
#include <geogram/mesh/mesh_io.h>

#include <geogram/parameterization/mesh_atlas_maker.h>

#include <geogram/image/image.h>
#include <geogram/image/image_library.h>
#include <geogram/image/morpho_math.h>

#include <geogram/delaunay/LFS.h>

#include <geogram/points/co3ne.h>
#include <geogram/points/kd_tree.h>

#include <geogram/third_party/PoissonRecon/poisson_geogram.h>

#include <geogram/basic/command_line.h>
#include <geogram/basic/command_line_args.h>
#include <geogram/basic/file_system.h>
#include <geogram/basic/line_stream.h>

#include <stack>

/**********************************************************************/

namespace {
    using namespace GEO;

    /**
     * \brief A class to read raw scanner input from
     *    http://graphics.stanford.edu/data/3Dscanrep/.
     */
    class StanfordScannerReader : public MeshIOHandler {
    public:

	/**
	 * \brief Reads a .conf file from the Standord scanning repository.
	 * \details Inherits MeshIOHandler, declared to Mesh IO system
	 *  in GeoBoxApplication::geogram_initialize();
	 */
	bool load(
	    const std::string& filename, Mesh& mesh,
	    const MeshIOFlags& ioflags = MeshIOFlags()
	) override {
	    geo_argused(ioflags);
	    
	    mesh.clear();
	    
	    LineInput in(filename);
	    if(!in.OK()) {
		Logger::err("geobox")
		    << "Could not open file " << filename << std::endl;
		return false;
	    }

	    Attribute<int> chart(mesh.vertices.attributes(),"chart");

	    int current_chart = 0;
	    while(!in.eof() && in.get_line()) {
		in.get_fields();
		if(in.nb_fields() == 0) { continue ; }
		if(in.field_matches(0,"bmesh")) {
		    std::string part_filename = in.field(1);
		    part_filename =
			FileSystem::dir_name(filename) + "/" + part_filename;
		    
		    // Translation vector
		    double Tx = in.field_as_double(2);
		    double Ty = in.field_as_double(3);
		    double Tz = in.field_as_double(4);
		    
		    /// Quaternion
		    double Qx = in.field_as_double(5);
		    double Qy = in.field_as_double(6);
		    double Qz = in.field_as_double(7);
		    double Qw = in.field_as_double(8);
		    
		    setup_transform_from_translation_and_quaternion(
			Tx,Ty,Tz,Qx,Qy,Qz,Qw
		    );

		    Mesh part;
		    if(!mesh_load(part_filename,part)) {
			return false;
		    }

		    index_t v_offset = mesh.vertices.nb();
		    mesh.vertices.create_vertices(part.vertices.nb());
		    
		    for(index_t v: part.vertices) {
			// Transform the vertex in place
			transform(part.vertices.point_ptr(v));
			
			// Copy the vertex to our mesh.
			if(mesh.vertices.single_precision()) {
			    for(index_t c=0; c<3; ++c) {
				mesh.vertices.single_precision_point_ptr(
				    v + v_offset
				)[c] = float(part.vertices.point_ptr(v)[c]);
			    }
			} else {
			    for(index_t c=0; c<3; ++c) {
				mesh.vertices.point_ptr(v + v_offset)[c] =
				    double(part.vertices.point_ptr(v)[c]);
			    }
			}

			if(chart.is_bound()) {
			    chart[v + v_offset] = current_chart;
			}
		    }
		    ++current_chart;
		}
	    }
	    return true;
	}

	bool save(
            const Mesh& mesh, const std::string& filename,
            const MeshIOFlags& ioflags
        ) override {
	    geo_argused(mesh);
	    geo_argused(filename);
	    geo_argused(ioflags);
	    return false;
	}
	
    protected:
	void setup_transform_from_translation_and_quaternion(
	    double Tx, double Ty, double Tz,
	    double Qx, double Qy, double Qz, double Qw
	) {
	    /* for unit q, just set s = 2 or set xs = Qx + Qx, etc. */
	    
	    double s = 2.0 / (Qx*Qx + Qy*Qy + Qz*Qz + Qw*Qw);
	    
	    double xs = Qx * s;
	    double ys = Qy * s;
	    double zs = Qz * s;
	    
	    double wx = Qw * xs;
	    double wy = Qw * ys;
	    double wz = Qw * zs;
	    
	    double xx = Qx * xs;
	    double xy = Qx * ys;
	    double xz = Qx * zs;
	    
	    double yy = Qy * ys;
	    double yz = Qy * zs;
	    double zz = Qz * zs;
	    
	    M[0][0] = 1.0 - (yy + zz);
	    M[0][1] = xy - wz;
	    M[0][2] = xz + wy;
	    M[0][3] = 0.0;
	    
	    M[1][0] = xy + wz;
	    M[1][1] = 1 - (xx + zz);
	    M[1][2] = yz - wx;
	    M[1][3] = 0.0;
	    
	    M[2][0] = xz - wy;
	    M[2][1] = yz + wx;
	    M[2][2] = 1 - (xx + yy);
	    M[2][3] = 0.0;
	    
	    M[3][0] = Tx;
	    M[3][1] = Ty;
	    M[3][2] = Tz;
	    M[3][3] = 1.0;
	}
	
	void transform(double* xyz) {
	    double xyzw[4] ;
	    for(unsigned int c=0; c<4; c++) {
		xyzw[c] = M[3][c] ;
	    }
	    for(unsigned int j=0; j<4; j++) {
		for(unsigned int i=0; i<3; i++) {
		    xyzw[j] += M[i][j] * xyz[i] ;
		}
	    }
	    for(unsigned int c=0; c<3; c++) {
		xyz[c] = xyzw[c] / xyzw[3] ;
	    }
	}
	
    private:
	double M[4][4] ;
    };
}


/**********************************************************************/

namespace {
    using namespace GEO;

    /**
     * \brief Graphic interface to geometry processing functionalities
     *  of geogram.
     */
    class GeoBoxApplication : public SimpleMeshApplication {
    public:

	enum TextureMode {
	    NO_TEXTURE=0,
	    UV_GRID=1,
	    RGB_TEXTURE=2,
	    NORMAL_MAP=3
	};
	
        /**
         * \brief GeoBoxApplication constructor.
         */
        GeoBoxApplication() : SimpleMeshApplication("GeoBox") {
	    texture_ = 0;
	    checker_texture_ = 0;
	    texture_mode_ = NO_TEXTURE;
        }

	void geogram_initialize(int argc, char** argv) override {
	    GEO::initialize();
	    geo_register_MeshIOHandler_creator(StanfordScannerReader,"conf");
	    GEO::CmdLine::import_arg_group("co3ne");
            GEO::CmdLine::import_arg_group("pre");
            GEO::CmdLine::import_arg_group("post");
            GEO::CmdLine::import_arg_group("remesh");
            GEO::CmdLine::import_arg_group("opt");
            GEO::CmdLine::import_arg_group("tet");
	    SimpleMeshApplication::geogram_initialize(argc, argv);
	}

	void show_attributes() override {
	    SimpleMeshApplication::show_attributes();
	    texture_mode_ = NO_TEXTURE;
	}

	bool save(const std::string& filename) override {

	    bool result = true;
	    
	    MeshIOFlags flags;
	    
	    if(!texture_image_.is_null()) {
		std::string tex_filename = "";
		if(texture_mode_ == RGB_TEXTURE) {
		    tex_filename =
			FileSystem::base_name(filename) + "_texture.png";
		} else if(texture_mode_ == NORMAL_MAP) {
		    tex_filename =
			FileSystem::base_name(filename) + "_normals.png";
		}
		if(tex_filename != "") {
		    // When saving in ".obj" file format, this will
		    // declare a material with the right texture.
		    flags.set_texture_filename(tex_filename);
		    tex_filename =
			FileSystem::dir_name(filename) + "/" + tex_filename;
		    Logger::out("geobox") << "Saving texture to "
					  << tex_filename
					  << std::endl;
		    ImageLibrary::instance()->save_image(
			tex_filename, texture_image_
		    );
		}
	    }

	    if(CmdLine::get_arg_bool("attributes")) {
		flags.set_attribute(MESH_FACET_REGION);
		flags.set_attribute(MESH_CELL_REGION);            
	    }
	    
	    if(FileSystem::extension(filename) == "geogram") {
		begin();
	    }
	    
	    if(mesh_save(mesh_, filename, flags)) {
		current_file_ = filename;
	    } else {
		result = false;
	    }
	    
	    if(FileSystem::extension(filename) == "geogram") {	    
		end();
	    }
	    
	    return result;
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
		if(phone_screen_) {
		    sz /= 4.0f;
		}
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

		if(ImGui::MenuItem("filter outliers")) {
                    GEO::Command::set_current(
			"void filter_outliers( "
			"  index_t N=70 [number of neighbors],"
			"  double R=0.01 [distance threshold]"
			") [removes outliers from pointset]",
			this, &GeoBoxApplication::filter_outliers
		    );
		}
		
                if(ImGui::MenuItem("reconstruct Co3Ne")) {
                    GEO::Command::set_current(
                "void reconstruct_Co3Ne(                                       "
                "   double radius=5.0       [search radius (in % bbox. diag.)],"
                "   index_t nb_smth_iter=2  [number of smoothing iterations],  "
                "   index_t nb_neighbors=30 [number of nearest neighbors]      "
                ") [reconstructs a surface from a pointset]",
                        this, &GeoBoxApplication::reconstruct_Co3Ne
                    );
                }

                if(ImGui::MenuItem("reconstruct Poisson")) {
                    GEO::Command::set_current(
                "void reconstruct_Poisson(                                     "
                "   index_t depth=8 [octree depth]         "
                ") [reconstructs a surface from a pointset]",
                        this, &GeoBoxApplication::reconstruct_Poisson
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

		if(ImGui::MenuItem("keep largest component")) {
		    GEO::Command::set_current(
			"void keep_largest_component( "
			"    bool are_you_sure=true   "
			") [keeps only the largest connected component]",
			this, &GeoBoxApplication::keep_largest_component
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
		ImGui::MenuItem("    Texture mapping...", nullptr,false,false);
		
                if(ImGui::MenuItem("make texture atlas")) {
		    Command::set_current(
 	    "void make_texture_atlas("
  	    "   bool use_ABF=false [use angle-based flattening],"
	    "   bool use_XATLAS=true [use XATLAS packer]"
	    ") [generates UV coordinates]",
	                this, &GeoBoxApplication::make_texture_atlas
		    );
		}

                if(ImGui::MenuItem("remesh and bake normals")) {
		    Command::set_current(
 	    "void remesh_with_normal_map("
  	    "   index_t nb_vertices=5000 [desired number of vertices],"
	    "   double anisotropy=0.2 [curvature adaptation],"
	    "   bool hires=false [use highres normal map]"
	    ") [remesh and generate normal-mapped mesh]",
	                this, &GeoBoxApplication::remesh_with_normal_map
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
                        "compute_local_feature_size(   "
			"    std::string attribute_name=\"LFS\""
		        ")",
                        this, &GeoBoxApplication::compute_local_feature_size
                    );
                }
                if(ImGui::MenuItem("compute dist. to brdr")) {
                    Command::set_current(
                        "compute_distance_to_border( "
			"    std::string attribute_name=\"distance\""
		        ")",
                        this, &GeoBoxApplication::compute_distance_to_border
                    );
                }
                if(ImGui::MenuItem("compute ambient occl.")) {
                    Command::set_current(
                        "compute_ambient_occlusion( "
			"    std::string attribute_name=\"AO\","
			"    index_t nb_rays_per_vertex=400,"
			"    index_t nb_smoothing_iter=2"
		        ") [per-vertex ambient occlusion]",
                        this, &GeoBoxApplication::compute_ambient_occlusion
                    );
                }

                if(ImGui::MenuItem("compute manifold harmonics")) {
                    Command::set_current(
                        "compute_manifold_harmonics( "
			"    std::string attribute_name=\"MH\","
			"    index_t nb_harmonics=30"
		        ")",
                        this, &GeoBoxApplication::compute_manifold_harmonics
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
	    texture_filename_ = "";
	    bool result = SimpleMeshApplication::load(filename);
	    if(result && FileSystem::extension(filename) == "stl") {
		mesh_.vertices.set_double_precision();	
		mesh_repair(mesh_);
		mesh_.vertices.set_single_precision();			
	    }

	    std::string tex_file_name =
		FileSystem::dir_name(filename) + "/" +
		FileSystem::base_name(filename) + "_texture.png";

	    if(!FileSystem::is_file(tex_file_name)) {
		tex_file_name =
		    FileSystem::dir_name(filename) + "/" +
		    FileSystem::base_name(filename) + "_normals.png";
	    }

	    if(FileSystem::is_file(tex_file_name)) {
		texture_filename_ = tex_file_name;
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

	/**
	 * \brief Removes the outliers from a pointset.
	 * \details Outliers are detected as points that have their
	 *   N-th nearest neighbor further away than a given distance 
	 *   threshold.
	 * \param[in] N the number of nearest neighbors
	 * \param[in] R if N-th neighbor is further away than R
	 *   then point will be discarded
	 */ 
	void filter_outliers(index_t N=70, double R=0.01) {

	    begin();
	    
	    // Remove duplicated vertices
	    mesh_repair(mesh_, GEO::MESH_REPAIR_COLOCATE, 0.0);

	    // Compute nearest neighbors using a KdTree.
	    NearestNeighborSearch_var NN = new BalancedKdTree(3); // 3 is for 3D
	    NN->set_points(mesh_.vertices.nb(), mesh_.vertices.point_ptr(0));
	    // point_ptr(0) is a pointer to the first point
	    // (it is also a pointer to the whole points array since
	    // points are contiguous in a Mesh).
	
	    // Now let's remove the points that have their furthest
	    // nearest neighbor further away than R.

	    // A vector of integers. remove_point[v]=1 if v should be removed.
	    vector<index_t> remove_point(mesh_.vertices.nb(), 0);
       
	    double R2 = R*R; // squared threshold
	                     // (KD-tree returns squared distances)
       
	    // Process all the points in parallel. 
	    // The point sequence [0..mesh_.vertices.nb()-1] is split
	    // into intervals [from,to[ processed by the lambda
	    // function below. There is one interval per processor core,
	    // all processed in parallel.
	    parallel_for_slice(
		0,mesh_.vertices.nb(),
		[this,N,&NN,R2,&remove_point](index_t from, index_t to) {
		    vector<index_t> neigh(N);
		    vector<double> neigh_sq_dist(N);
		    for(index_t v=from; v<to; ++v) {
			NN->get_nearest_neighbors(
			    N, mesh_.vertices.point_ptr(v),
			    neigh.data(), neigh_sq_dist.data()
			);
			remove_point[v] = (neigh_sq_dist[N-1] > R2);
		    }
		}
	    );
       
	    // Now remove the points that should be removed.
	    mesh_.vertices.delete_elements(remove_point);

	    end();
	}

	/**
	 * \brief Keeps the largest connected components of a mesh,
	 *   and deletes all the other ones.
	 * \param[in] are_you_sure confirmation
	 */ 
	void keep_largest_component(bool are_you_sure=true) {

	    if(!are_you_sure) {
		return;
	    }

	    begin();
	    
	    // component[f] will correspond to the component id of facet f
	    vector<index_t> component(mesh_.facets.nb(),index_t(-1));
	    index_t nb_comp=0;

	    // Iterates on all the facets of M
	    // (equivalent to for(index_t f = 0; f < M.facets.nb(); ++f))
	    for(index_t f: mesh_.facets) {
		if(component[f] == index_t(-1)) {
		    // recursive traversal of the connected component
		    // incident to facet f (if it was not already traversed)
		    component[f] = nb_comp;
		    std::stack<index_t> S;
		    S.push(f);
		    while(!S.empty()) {
			index_t top_f = S.top();
			S.pop();
			// Push the neighbors of facet top_f onto the stack if
			// they were not already visited
			for(
			    index_t le=0;
			    le<mesh_.facets.nb_vertices(top_f); ++le
			) {
			    index_t adj_f = mesh_.facets.adjacent(top_f,le);
			    if(adj_f != index_t(-1) &&
			       component[adj_f] == index_t(-1)) {
				component[adj_f] = nb_comp;
				S.push(adj_f);
			    }
			}
		    }
		    ++nb_comp;
		}
	    }
	
	    // Now compute the number of facets in each connected component
	    vector<index_t> comp_size(nb_comp,0);
	    for(index_t f: mesh_.facets) {
		++comp_size[component[f]];
	    }

	    // Determine the id of the largest component
	    index_t largest_comp = 0;
	    index_t largest_comp_size = 0;
	    for(index_t comp=0; comp<nb_comp; ++comp) {
		if(comp_size[comp] >= largest_comp_size) {
		    largest_comp_size = comp_size[comp];
		    largest_comp = comp;
		}
	    }

	    // Remove all the facets that are not in the largest component
	    // component[] is now used as follows:
	    //   component[f] = 0 if f should be kept
	    //   component[f] = 1 if f should be deleted
	    // See GEO::MeshElements::delete_elements() documentation.
	    for(index_t f: mesh_.facets) {
		component[f] = (component[f] != largest_comp) ? 1 : 0;
	    }
	    mesh_.facets.delete_elements(component);

	    end();
	}

	
	/**
	 * \brief Reconstructs the triangles from a poinset in the current
	 *  mesh, using the Concurrent Co-Cones algorithm.
	 * \param[in] radius search radius for neighborhoods
	 * \param[in] nb_iterations number of smoothing iterations
	 * \param[in] nb_neighbors number of neighbors in graph
	 */
        void reconstruct_Co3Ne(
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


        /**
	 * \brief Reconstructs the triangles from a poinset in the current
	 *  mesh, using the Poisson Reconstruction method.
	 * \param[in] depth octree depth
	 * \details Reference: http://hhoppe.com/poissonrecon.pdf
	 *  Based on Misha Kahzdan's PoissonRecon code
	 *    (see geogram/thirdparty/PoissonRecon) with custom adaptations.
	 */ 
	void reconstruct_Poisson(index_t depth=8) {
            hide_surface();
	    begin();
	    mesh_.facets.clear();
	   
	    // Poisson reconstruction needs oriented normals. Use
	    // Concurrent Co-Cones (Co3Ne) to compute them.
	    Mesh points;
	    points.copy(mesh_,false,MESH_VERTICES);
	    Co3Ne_compute_normals(points, 30, true);	    

	    mesh_.clear();
	    PoissonReconstruction poisson;
	    poisson.set_depth(depth);
	    poisson.reconstruct(&points, &mesh_);

	    hide_vertices();
	    show_surface();
	    lighting_ = true;
	    end();
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

	void make_texture_atlas(
	    bool use_ABF=false,  
	    bool use_XATLAS=true
	) {
	    if(mesh_.facets.nb() == 0) {
		Logger::err("Param") << "Mesh has no facet"
				     << std::endl;
		return;
	    }

	    begin();

	    mesh_make_atlas(
		mesh_,
		45.0 * M_PI / 180.0,
		use_ABF    ? PARAM_ABF   : PARAM_LSCM,
		use_XATLAS ? PACK_XATLAS : PACK_TETRIS,
		false // set to true to enable verbose messages
	    );

	    end();
	    
	    texture_mode_ = UV_GRID;
	}

	void remesh_with_normal_map(
	    index_t nb_vertices=7000,
	    double anisotropy=0.3,
	    bool hires=false
	) {
	    if(mesh_.facets.nb() == 0) {
		return;
	    }

	    // Note: all geometric algorithms operate in double precision.
	    mesh_.vertices.set_double_precision();

	    // Step 1: remesh
	    // Create low-resolution mesh (M)
	    if(anisotropy != 0.0) {
		compute_normals(mesh_);
		simple_Laplacian_smooth(mesh_, 3, true);
		set_anisotropy(mesh_, double(anisotropy) * 0.02);
	    }

	    Mesh M;	
	    GEO::remesh_smooth(
		mesh_, M, nb_vertices,
		0, // dimension (0 = default)
		5, // nb Lloyd iter.
		10 // nb Newton iter.
	    );
	    mesh_.vertices.set_dimension(3);

	    // Make sure mesh and remesh have same normal
	    // orientations. (this computes the signed volume
	    // of a mesh and inverts the order of all the
	    // vertices around the all the facets of the mesh
	    // if it is negative).
	    orient_normals(mesh_);	    
	    orient_normals(M);

	    // Step 2: compute UVs in low-resolution mesh (M)
	    mesh_make_atlas(
		M,
		45.0 * M_PI / 180.0,
		PARAM_LSCM,
		PACK_XATLAS,
		false // verbose
	    );

	    const index_t size = hires ? 2048 : 1024;
	    
	    // Step 3: Generate geometry image from parameterized
	    // lowres mesh. A geometry image is an UV atlas where
	    // the colors encode the (x,y,z) coordinates of the
	    // surface. See http://hhoppecom/gim.pdf for an explanation
	    // of the idea.
	    // The geometry image is used to transfer the normals
	    // from the high resolution mesh.
	    Image_var geometry_image = new Image(
		Image::RGB, Image::FLOAT64, size, size
	    );
	    bake_mesh_geometry(&M,geometry_image);

	    // Step 4: generate the normal map.
	    // It is done using the geometry image, and finding for each
	    // point of the geometry image the nearest point on the base
	    // mesh, and copying the normal from the found nearest point.
	    texture_image_ = new Image(Image::RGBA, Image::BYTE, size, size);
	    bake_mesh_facet_normals_indirect(
		geometry_image, texture_image_, &mesh_
	    );

	    // Step 5: dilate the normal map a bit.
	    // This creates a "gutter" of pixels around the charts of the
	    // normal map. This ensures texture interpolation does not blend
	    // the normals with the background of the texture.
	    MorphoMath mm(texture_image_);
	    mm.dilate(10);

	    // Now overwrite the highres mesh with the lowres one.
	    mesh_.copy(M);

	    // Back to single precision for faster display.	    
	    mesh_.vertices.set_single_precision();
	    
	    // Initialize or update graphic representation
	    //   (OpenGL arrays etc...)
	    mesh_gfx_.set_mesh(&mesh_);

	    // Create the normal map texture from the GEO::Image that
	    // contains the normal map.
	    update_texture_from_image();
	    texture_mode_ = NORMAL_MAP;

	    show_mesh_ = true;
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

	void compute_ambient_occlusion(
	    std::string attribute_name = "AO",
	    index_t nb_rays_per_vertex = 400,
	    index_t nb_smoothing_iter = 2
	) {
	    begin();
	    
	    Attribute<double> AO(mesh_.vertices.attributes(), attribute_name);
	    MeshFacetsAABB AABB(mesh_);
	    
	    parallel_for(
		0, mesh_.vertices.nb(),
		[this,&AABB,&AO,nb_rays_per_vertex](index_t v) {
		    double ao = 0.0;
		    vec3 p(mesh_.vertices.point_ptr(v));
		    for(index_t i=0; i<nb_rays_per_vertex; ++i) {

			// https://math.stackexchange.com/questions/1585975/
			//   how-to-generate-random-points-on-a-sphere	
			double u1 = Numeric::random_float64();
			double u2 = Numeric::random_float64();
			
			double theta = 2.0 * M_PI * u2;
			double phi = acos(2.0 * u1 - 1.0) - M_PI / 2.0;
			
			vec3 d(
			    cos(theta)*cos(phi),
			    sin(theta)*cos(phi),
			    sin(phi)
			);
		    
			if(!AABB.ray_intersection(
			       Ray(p + 1e-3*d, d)
			   )) {
			    ao += 1.0;
			}
		    }
		    ao /= double(nb_rays_per_vertex);
		    AO[v] = ao;
		}
	    );

	    if(nb_smoothing_iter != 0) {
		vector<double> next_val;
		vector<index_t> degree;
		for(index_t i=0; i<nb_smoothing_iter; ++i) {
		    next_val.assign(mesh_.vertices.nb(),0.0);
		    degree.assign(mesh_.vertices.nb(),1);
		    for(index_t v: mesh_.vertices) {
			next_val[v] = AO[v];
		    }
		    for(index_t f: mesh_.facets) {
			index_t d = mesh_.facets.nb_vertices(f);
			for(index_t lv=0; lv < d; ++lv) {
			    index_t v1 = mesh_.facets.vertex(f,lv);
			    index_t v2 = mesh_.facets.vertex(f,(lv + 1) % d);
			    degree[v1]++;
			    degree[v2]++;
			    next_val[v1] += AO[v2];
			    next_val[v2] += AO[v1];
			}
		    }
		    for(index_t v: mesh_.vertices) {
			AO[v] = next_val[v] / double(degree[v]);
		    }
		}
	    }
	    
	    end();
            set_attribute("vertices." + attribute_name);
	    current_colormap_texture_ = colormaps_[1].texture; // graylevel
	    lighting_ = false;
	    show_mesh_ = false;
            show_attributes();
	}

	void compute_manifold_harmonics(
	    std::string attribute_name = "MH",
	    index_t nb_harmonics = 30
	) {
	    begin();
	    mesh_compute_manifold_harmonics(
		mesh_, nb_harmonics, FEM_P1_LUMPED, attribute_name
	    );
	    end();
            set_attribute(
		"vertices." +
		attribute_name +
		"[" + String::to_string(nb_harmonics-1) + "]"
	    );
	    current_colormap_texture_ = colormaps_[5].texture;
	    lighting_ = false;
	    show_mesh_ = false;
	    show_attributes();
	}

	
    protected:
        
        void hide_mesh() {
            mesh_gfx()->set_mesh(nullptr);            
        }

	/**
	 * \brief To be called at the beginning of each command.
	 * \details Switches mesh storage from single precision
	 *  to double precision. Single precision is used for
	 *  faster display, and double precision is required
	 *  for all geometry processing functions.
	 */
        void begin() {
            mesh()->vertices.set_double_precision();            
        }

	/**
	 * \brief To be called at the end of each command.
	 * \details Switches mesh storage from double precision
	 *  to single precision. Single precision is used for
	 *  faster display, and double precision is required
	 *  for all geometry processing functions.
	 */
        void end() {
            orient_normals(*mesh());
	   
	    // update bounding box
            double xyzmin[3];
            double xyzmax[3];
            get_bbox(mesh_, xyzmin, xyzmax, false);
            set_region_of_interest(
                xyzmin[0], xyzmin[1], xyzmin[2],
                xyzmax[0], xyzmax[1], xyzmax[2]
            );
	   
            mesh()->vertices.set_single_precision();
            mesh_gfx()->set_mesh(mesh());
            if(mesh()->facets.nb() == 0 && mesh()->cells.nb() == 0) {
                show_vertices();
            }
        }

	/**
	 * \brief Tests whether current object has UVs
	 * \return true if the current object has UV coordinates, 
	 *  false otherwise.
	 */
	bool has_UVs() {
	    return mesh_.facet_corners.attributes().is_defined(
		"tex_coord"
	    );
	}
	
        /**
	 * \brief Draws the object properties box and handles the associated
	 *   GUI components.
	 */ 
	void draw_object_properties() override {
	    SimpleMeshApplication::draw_object_properties();
	    if(has_UVs()) {
		ImGui::Combo(
		    "tex", (int*)&texture_mode_,                
		    "off\0UV grid\0image\0N map\0\0"
		);
	    }
	    
	}	
	
	/**
	 * \brief Called at startup, to initialize OpenGL objects.
	 * \details Creates a default checkerboard texture.
	 */ 
	void GL_initialize() override {
	    SimpleMeshApplication::GL_initialize();
	    if(checker_texture_ == 0) {
		static unsigned char image_data[2*2*4] = {
		    255, 255, 255, 255,   127, 127, 127, 255,
		    127, 127, 127, 255,   255, 255, 255, 255
		};
		glGenTextures(1, &checker_texture_);
		glBindTexture(GL_TEXTURE_2D, checker_texture_);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(
		    GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST
		);
		glTexParameteri(
		    GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST
		);
		glTexImage2D(
		    GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0,
		    GL_RGBA, GL_UNSIGNED_BYTE,
		    image_data
		);
		glBindTexture(GL_TEXTURE_2D, 0);
	    }
	}

	/**
	 * \brief Deletes all OpenGL objects.
	 * \details Called on exit.
	 */ 
	void GL_terminate() override {
	    if(texture_ != 0) {
		glDeleteTextures(1, &texture_);
		texture_ = 0;
	    }
	    if(checker_texture_ != 0) {
		glDeleteTextures(1, &checker_texture_);
		checker_texture_ = 0;
	    }
	    SimpleMeshApplication::GL_terminate();
	}

	bool load_texture(const std::string& filename) {
	    texture_image_.reset();
	    texture_mode_ = NO_TEXTURE;

	    // If graphics are not ready yet, do not load texture.
	    if(glupCurrentContext() == nullptr) {
		return false;
	    }
	    
	    if(!FileSystem::is_file(filename)) {
		return false;
	    }
	    
	    // Load the image file. This creates a GEO::Image object.
	    texture_image_ = ImageLibrary::instance()->load_image(
		filename
	    );
	    
	    if(texture_image_.is_null()) {
		return false;
	    }

	    Logger::out("geobox") << "Using texture: " << filename
				  << std::endl;

	    update_texture_from_image();
	    texture_mode_ = RGB_TEXTURE;
	    if(String::string_ends_with(filename,"_normals.png")) {
		texture_mode_ = NORMAL_MAP;
	    }
	    
	    return true;
	}

	void update_texture_from_image() {
	    // Create OpenGL texture and configure it if not already
	    // present.
	    if(texture_ == 0) {
		glGenTextures(1, &texture_);
		glBindTexture(GL_TEXTURE_2D, texture_);
		glTexParameteri(
		    GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR
		);
		glTexParameteri(
		    GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST
		);
		glTexParameteri(
                    GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE
                );
		glTexParameteri(
                    GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE
                );
		glBindTexture(GL_TEXTURE_2D, 0);		
	    }
	    
	    // Transfer GEO::Image image data to the OpenGL texture.
	    // image_->base_mem() gets a pointer to the raw
	    //  image data, that can be sent directly to OpenGL.
	    glBindTexture(GL_TEXTURE_2D, texture_);
	    glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RGBA,
		GLsizei(texture_image_->width()),
		GLsizei(texture_image_->height()), 0,
		GL_RGBA, GL_UNSIGNED_BYTE,
		texture_image_->base_mem()
	    );
	}
	
	/**
	 * \brief Draws the surface.
	 * \details Initializes texture mapping
	 *  before calling the draw_surface() function
	 *  of the base class (SimpleMeshApplication).
	 */
	void draw_surface() override {

	    if(texture_filename_ != "") {
		load_texture(texture_filename_);
		texture_filename_ = "";
	    }
	    
	    if(has_UVs() && texture_mode_ != NO_TEXTURE) {
		bool use_uv_grid =
		    (texture_mode_ == UV_GRID) || (texture_ == 0);
		if(!use_uv_grid && texture_mode_ == NORMAL_MAP) {
		    glupEnable(GLUP_NORMAL_MAPPING);
		}
		mesh_gfx_.set_texturing(
		    GEO::MESH_FACET_CORNERS,
		    "tex_coord",
		    use_uv_grid ? checker_texture_ : texture_,
		    2,                   // dimension
		    use_uv_grid ? 20 : 1 // repeat
		);
	    } 
	    SimpleMeshApplication::draw_surface();
	    glupDisable(GLUP_NORMAL_MAPPING);
	}
	
    protected:
	std::string texture_filename_;
	Image_var texture_image_;
	GLuint texture_;
	GLuint checker_texture_;
	TextureMode texture_mode_;
    };
}

int main(int argc, char** argv) {
    GeoBoxApplication app;
    app.start(argc, argv);
    return 0;
}
