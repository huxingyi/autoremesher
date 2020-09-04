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

#include <geogram/basic/command_line_args.h>
#include <geogram/basic/command_line.h>
#include <geogram/basic/process.h>
#include <geogram/basic/logger.h>

#include <set>

namespace {

    using namespace GEO;
    using namespace CmdLine;

    /**
     * \brief Imports the global option group
     */
    void import_arg_group_global() {
        declare_arg(
            "profile", "scan",
            "Vorpaline mode "
	    "(scan, convert, repair, heal, cad, tet, poly, hex, quad)"
        );
        declare_arg(
            "debug", false,
            "Toggles debug mode", ARG_ADVANCED
        );
    }

    /**
     * \brief Imports the pre-processing option group
     */
    void import_arg_group_pre() {
        declare_arg_group("pre", "Preprocessing phase");
        declare_arg(
            "pre", true,
            "Toggles pre-processing phase", ARG_ADVANCED
        );
        declare_arg(
            "pre:Nsmooth_iter", 1,
            "Number of iterations for normals smoothing",
            ARG_ADVANCED
        );
        declare_arg_percent(
            "pre:margin", 0,
            "Expand border (in % of bounding box diagonal)",
            ARG_ADVANCED
        );
        declare_arg(
            "pre:repair", false,
            "Repair input mesh"
        );
        declare_arg_percent(
            "pre:epsilon", 0,
            "Colocate tolerance (in % of bounding box diagonal)",
            ARG_ADVANCED
        );  
        declare_arg_percent(
            "pre:max_hole_area", 0,
            "Fill holes smaller than (in % total area)"
        );
        declare_arg(
            "pre:max_hole_edges", 2000,
            "Fill holes with a smaller nb. of edges"
        );
        declare_arg_percent(
            "pre:min_comp_area", 0,
            "Remove small components (in % total area)"
        );
        declare_arg(
            "pre:vcluster_bins", 0,
            "Number of bins for vertex clustering"
        );
        declare_arg(
            "pre:brutal_kill_borders", 0,
            "Brutally kill facets incident to border (nb iter)"
        );
    }

    /**
     * \brief Imports the remeshing option group
     */
    void import_arg_group_remesh() {
        declare_arg_group("remesh", "Remeshing phase");
        declare_arg(
            "remesh", true,
            "Toggles remeshing phase", ARG_ADVANCED
        );
        declare_arg(
            "remesh:nb_pts", 30000,
            "Number of vertices"
        );
        declare_arg(
            "remesh:anisotropy", 0.0,
            "Anisotropy factor"
        );
        declare_arg(
            "remesh:by_parts", false,
            "Part by part remeshing", ARG_ADVANCED
        );
        declare_arg(
            "remesh:gradation", 0.0,
            "Mesh gradation exponent"
        );
        declare_arg(
            "remesh:lfs_samples", 10000,
            "Number of samples for lfs (gradation)",
            ARG_ADVANCED
        );

#ifdef GEOGRAM_WITH_VORPALINE	
        declare_arg(
            "remesh:sharp_edges", false,
            "Reconstruct sharp edges", ARG_ADVANCED
        );
	
        declare_arg(
            "remesh:Nfactor", 5.0,
            "For sharp_edges", ARG_ADVANCED
        );
#endif
	
        declare_arg(
            "remesh:multi_nerve", true,
            "Insert new vertices to preserve topology",
            ARG_ADVANCED
        );
        declare_arg(
            "remesh:RVC_centroids", true,
            "Use centroids of restricted Voronoi cells", ARG_ADVANCED
        );
        declare_arg(
            "remesh:refine", false,
            "Insert points to lower Hausdorff distance", ARG_ADVANCED
        );
        declare_arg(
            "remesh:max_dist", 0.2,
            "Max. distance to source mesh, relative to avg. edge len",
            ARG_ADVANCED
        );
    }

    /**
     * \brief Declares the algorithm option group
     */
    void import_arg_group_algo() {
        declare_arg_group("algo", "Algorithms", ARG_ADVANCED);
        declare_arg(
            "algo:nn_search", "BNN",
            "Nearest neighbors search (BNN, ...)"
        );
        declare_arg(
            "algo:delaunay", "NN",
            "Delaunay algorithm"
        );
        declare_arg(
            "algo:hole_filling", "loop_split",
            "Hole filling mode (loop_split, Nloop_split, ear_cut)"
        );
        declare_arg(
            "algo:predicates", "fast",
            "Geometric predicates (fast, exact)"
        );
        declare_arg(
            "algo:reconstruct", "Co3Ne",
            "reconstruction algorithm (Co3Ne, Poisson)"
        );
#ifdef GEO_OS_ANDROID
        // NDK's default multithreading seems to be not SMP-compliant
        // (missing memory barriers in synchronization primitives)
        declare_arg(
            "algo:parallel", false,
            "Use parallel standard algorithms"
        );
#else
        declare_arg(
            "algo:parallel", true,
            "Use parallel standard algorithms"
        );
#endif
    }

    /**
     * \brief Imports the post-processing option group
     */
    void import_arg_group_post() {
        declare_arg_group("post", "Postprocessing phase");
        declare_arg(
            "post", true,
            "Toggles post-processing phase", ARG_ADVANCED
        );
        declare_arg(
            "post:repair", false,
            "Repair output mesh"
        );
        declare_arg_percent(
            "post:max_hole_area", 0.0,
            "Fill holes smaller than (in % total area)"
        );
        declare_arg(
            "post:max_hole_edges", 2000,
            "Fill holes with a smaller nb. of edges than"
        );
        declare_arg_percent(
            "post:min_comp_area", 0.0,
            "Remove small components (in % total area)"
        );
        declare_arg_percent(
            "post:max_deg3_dist", 0.1,
            "Degree3 vertices threshold (in % bounding box diagonal)"
        );
        declare_arg(
            "post:isect", false, "Tentatively remove self-intersections"
        );
        declare_arg(
            "post:compute_normals", false, "Compute normals"
        );
    }

    /**
     * \brief Imports the optimizer option group
     */
    void import_arg_group_opt() {
        declare_arg_group("opt", "Optimizer fine tuning", ARG_ADVANCED);
#ifdef GEOGRAM_WITH_HLBFGS
        declare_arg(
            "opt:nb_Lloyd_iter", 5,
            "Number of iterations for Lloyd pre-smoothing"
        );
        declare_arg(
            "opt:nb_Newton_iter", 30,
            "Number of iterations for Newton-CVT"
        );
        declare_arg(
            "opt:nb_LpCVT_iter", 30,
            "Number of iterations for LpCVT"
        );
        declare_arg(
            "opt:Newton_m", 7,
            "Number of evaluations for Hessian approximation"
        );
#else
        declare_arg(
            "opt:nb_Lloyd_iter", 40,
            "Number of iterations for Lloyd pre-smoothing"
        );
        declare_arg(
            "opt:nb_Newton_iter", 0,
            "Number of iterations for Newton-CVT"
        );
        declare_arg(
            "opt:nb_LpCVT_iter", 0,
            "Number of iterations for LpCVT"
        );
        declare_arg(
            "opt:Newton_m", 0,
            "Number of evaluations for Hessian approximation"
        );
#endif	
    }

    /**
     * \brief Imports the system option group
     */
    void import_arg_group_sys() {
        declare_arg_group("sys", "System fine tuning", ARG_ADVANCED);

#ifdef GEO_DEBUG
        declare_arg(
            "sys:assert", "abort",
            "Assertion behavior (abort, throw, breakpoint)"
        );
#else        
        declare_arg(
            "sys:assert", "throw",
            "Assertion behavior (abort, throw, breakpoint)"
        );
#endif
        
        declare_arg(
            "sys:multithread", Process::multithreading_enabled(),
            "Enables multi-threaded computations"
        );
        declare_arg(
            "sys:FPE", Process::FPE_enabled(),
            "Enables floating-point exceptions"
        );
        declare_arg(
            "sys:cancel", Process::cancel_enabled(),
            "Enables interruption of cancelable tasks"
        );
        declare_arg(
            "sys:max_threads", (int) Process::number_of_cores(),
            "Maximum number of concurrent threads"
        );
        declare_arg(
            "sys:use_doubles", false,
            "Uses double precision in output .mesh files"
        );
	declare_arg(
	    "sys:ascii", false,
	    "Use ASCII files whenever supported"
	);	    
        declare_arg(
            "sys:compression_level", 3,
            "Compression level for created .geogram files, in [0..9]"
        );
        declare_arg(
            "sys:lowmem", false,
            "Reduces RAM consumption (but slower)"
        );
        declare_arg(
            "sys:stats", false,
            "Display statistics on exit"
        );
#ifdef GEO_OS_WINDOWS
	declare_arg(
	    "sys:show_win32_console", false,
	    "Display MSDOS window"
	);
#endif	
    }

    /**
     * \brief Imports the NL (Numerical Library) option group
     */
    void import_arg_group_nl() {
	declare_arg_group("nl", "OpenNL (numerical library)", ARG_ADVANCED);
	declare_arg(
	    "nl:MKL", false,
	    "Use Intel Math Kernel Library (if available in the system)"
	);
	declare_arg(
	    "nl:CUDA", false,
	    "Use NVidia CUDA (if available in the system)"
	);
    }
    
    /**
     * \brief Imports the Logger option group
     */
    void import_arg_group_log() {
        declare_arg_group("log", "Logger settings", ARG_ADVANCED);
        declare_arg(
            "log:quiet", false,
            "Turns logging on/off"
        );
        declare_arg(
            "log:pretty", true,
            "Turns console pretty output on/off"
        );
        declare_arg(
            "log:file_name", "",
            "Logs output to the specified file"
        );
        declare_arg(
            "log:features", "*",
            "Semicolon separated list of features selected for log"
        );
        declare_arg(
            "log:features_exclude", "",
            "Semicolon separated list of features excluded from log"
        );
    }

    /**
     * \brief Imports the reconstruction option group
     */
    void import_arg_group_co3ne() {
        declare_arg_group("co3ne", "Reconstruction", ARG_ADVANCED);
        declare_arg(
            "co3ne", false,
            "Use reconstruction", ARG_ADVANCED
        );
        declare_arg(
            "co3ne:nb_neighbors", 30,
            "Number of neighbors used in reconstruction"
        );
        declare_arg(
            "co3ne:Psmooth_iter", 0, "Number of smoothing iterations"
        );
        declare_arg_percent(
            "co3ne:radius", 5,
            "Search radius (in % bounding box diagonal)"
        );
        declare_arg(
            "co3ne:repair", true,
            "Repair output surface"
        );
        declare_arg(
            "co3ne:max_N_angle", 60.0,
            "Filter bad triangles (in degrees)"
        );
        declare_arg_percent(
            "co3ne:max_hole_area", 5,
            "Fill holes smaller than (in % total area)"
        );
        declare_arg(
            "co3ne:max_hole_edges", 500,
            "Fill holes with a smaller nb. of edges"
        );
        declare_arg_percent(
            "co3ne:min_comp_area", 0.01,
            "Remove small components (in % total area)"
        );
        declare_arg(
            "co3ne:min_comp_facets", 10,
            "Remove small components (in facet nb.)"
        );
        declare_arg(
            "co3ne:T12", true,
            "Use also triangles seen from 1 and 2 seeds"
        );
        declare_arg(
           "co3ne:strict", false,
           "enforce combinatorial tests for triangles seen from 3 seeds as well"
        );
        declare_arg(
            "co3ne:use_normals", true,
            "Use existing normal attached to data if available"
        );

        // For now, in co3ne import arg group -> todo: create new import func
        declare_arg_group("poisson", "Reconstruction", ARG_ADVANCED);
        declare_arg(
            "poisson:octree_depth", 8,
            "Octree depth for Poisson reconstruction if used"
        );
    }

    /**
     * \brief Import the statistics option group
     */
    void import_arg_group_stat() {
        declare_arg_group("stat", "Statistics tuning");
        declare_arg_percent(
            "stat:sampling_step", 0.5,
            "For Hausdorff distance"
        );
    }

    /**
     * \brief Imports the polyhedral meshing option group
     */
    void import_arg_group_poly() {
        declare_arg_group("poly", "Polyhedral meshing", ARG_ADVANCED);
        declare_arg(
            "poly", false,
            "Toggles polyhedral meshing"
        );
	declare_arg(
	    "poly:simplify", "tets_voronoi",
	    "one of none (generate all intersections), "
	    "tets (regroup Vornoi cells), "
	    "tets_voronoi (one polygon per Voronoi facet), "
	    "tets_voronoi_boundary (simplify boundary)"
	);
	declare_arg(
	    "poly:normal_angle_threshold", 1e-3,
	    "maximum normal angle deviation (in degrees) for merging boundary facets"
	    " (used if poly:simplify=tets_voronoi_boundary)"
	);
	declare_arg(
	    "poly:cells_shrink", 0.0,
	    "Voronoi cells shrink factor (for visualization purposes), between 0.0 and 1.0"
	);
	declare_arg(
	    "poly:points_file", "",
	    "optional points file name (if left blank, generates and optimizes remesh:nb_pts points)"
	);
	declare_arg(
	    "poly:generate_ids", false,
	    "generate unique ids for vertices and cells (saved in geogram, geogram_ascii and ovm file formats only)"
	);
	declare_arg(
	    "poly:embedding_dim", 0,
	    "force embedding dimension (0 = use input dim.)"
	);
	declare_arg(
	    "poly:tessellate_non_convex_facets", false,
	    "tessellate non-convex facets"
	);
    }    

    /**
     * \brief Imports the hex-dominant meshing option group
     */
    void import_arg_group_hex() {
        declare_arg_group("hex", "Hex-dominant meshing", ARG_ADVANCED);
        declare_arg(
            "hex", false,
            "Toggles hex-dominant meshing"
        );
        declare_arg(
            "hex:save_points", false,
            "Save points to points.meshb"
        );
        declare_arg(
            "hex:save_tets", false,
            "Save tetrahedra (before primitive merging) to tets.meshb"
        );
        declare_arg(
            "hex:save_surface", false,
            "Save surface to surface.meshb"
        );
        declare_arg(
            "hex:save_frames", false,
            "Save frames and surface to frames_surface.eobj"
        );
        declare_arg(
            "hex:prefer_seeds", true,
            "In constrained mode, use seeds whenever possible"
        );
        declare_arg(
            "hex:constrained", true,
            "Use constrained Delaunay triangulation"
        );
        declare_arg(
            "hex:points_file", "",
            "Load points from a file"
        );
        declare_arg(
            "hex:tets_file", "",
            "Load tetrahedra from a file"
        );
        declare_arg(
            "hex:frames_file", "",
            "Load frames from a file"
        );
        declare_arg(
            "hex:prisms", false,
            "generate prisms"
        );
        declare_arg(
            "hex:pyramids", false,
            "generate pyramids"
        );
        declare_arg(
            "hex:algo", "PGP3d",
            "one of (PGP3d, LpCVT)"
        );
        declare_arg(
            "hex:PGP_max_corr_prop", 0.35,
            "maximum correction form (0 to deactivate)"
        );
        declare_arg(
            "hex:PGP_FF_free_topo", 1,
            "number of free topo. frame field opt. iterations"
        );
        declare_arg(
            "hex:PGP_FF_fixed_topo", 1,
            "number of fixed topo. frame field opt. iterations"
        );
        declare_arg(
            "hex:PGP_direct_solver", false,
            "(tentatively) use PGP direct solver"
        );
        declare_arg(
            "hex:border_refine", false,
            "refine border to lower Hausdorff distance"
        );
        declare_arg_percent(
            "hex:border_max_distance", 20,
            "maximum distance to reference (in % of input average edge length)"
        );
        declare_arg(
            "hex:keep_border", false, "keep initial border mesh"
        );
    }

    /**
     * \brief Imports the quad-dominant meshing option group
     */
    void import_arg_group_quad() {
        declare_arg_group("quad", "Quad-dominant meshing", ARG_ADVANCED);
        declare_arg(
            "quad", false,
            "Toggles quad-dominant meshing"
        );
	declare_arg(
	    "quad:relative_edge_length",
	    1.0,
	    "relative edge length"
	);
	declare_arg(
	    "quad:optimize_parity", false,
	    "Optimize quads parity when splitting charts (experimental)"
	);
	declare_arg(
	    "quad:max_scaling_correction", 1.0,
	    "maximum scaling correction factor (use 1.0 to disable)"
	);
    }
    
    /**
     * \brief Imports the tetrahedral meshing option group
     */
    void import_arg_group_tet() {
        declare_arg_group("tet", "Tetrahedral meshing", ARG_ADVANCED);
        declare_arg(
            "tet", false,
            "Toggles tetrahedral meshing"
        );
        declare_arg(
            "tet:refine", true,
            "Generates additional points to improve mesh quality"
        );
        declare_arg(
            "tet:preprocess", true,
            "Pre-processes surface before meshing"
        );
        declare_arg(
            "tet:quality", 2.0,
        "desired element quality (the lower, the better, 2.0 means reasonable)"
        );
    }

    /**
     * \brief Imports the graphics option group
     */
    void import_arg_group_gfx() {
        declare_arg_group("gfx", "OpenGL graphics options", ARG_ADVANCED);
        declare_arg(
            "gfx:GL_profile",
#if defined(GEO_OS_ANDROID)
	    "ES",	    		    
#else		    
	    "core",
#endif	    
            "one of core,ES"
        );
        declare_arg(
            "gfx:GL_version", 0.0,
            "If non-zero, override GL version detection"
        );
        declare_arg(
            "gfx:GL_debug", false,
            "OpenGL debugging context"
        );
        declare_arg(
            "gfx:GLSL_version", 0.0,
            "If non-zero, overrides GLSL version detection"
        );
        declare_arg(
            "gfx:GLUP_profile", "auto",
            "one of auto, GLUP150, GLUP440, GLUPES"
        );
        declare_arg("gfx:full_screen", false, "full screen mode");
        declare_arg(
	    "gfx:no_decoration", false,
	    "no window decoration (full screen mode)"
	);	
	declare_arg(
	    "gfx:transparent", false,
	    "use transparent backgroung (desktop integration)"
	);
        declare_arg(
            "gfx:GLSL_tesselation", true, "use tesselation shaders if available"
        );
	declare_arg("gfx:geometry", "1024x1024", "resolution");
	declare_arg("gfx:keypress", "", "initial key sequence sent to viewer");
    }
    
    /**
     * \brief Imports the biblio option group
     */
    void import_arg_group_biblio() {
        declare_arg_group("biblio", "Bibliography options", ARG_ADVANCED);
	declare_arg("biblio", false, "output bibliography citations");
	declare_arg(
	    "biblio:command_line", false,
	    "dump all command line arguments in biblio. report"
	);
    }

    /**
     * \brief Imports the gui option group.
     */
    void import_arg_group_gui() {
        declare_arg_group("gui", "gui options", ARG_ADVANCED);
	declare_arg("gui:state", "", "gui layout state");
	declare_arg("gui:style", "Dark", "gui style, one of Dark,Light");
	declare_arg("gui:font_size", 18, "font size");
	declare_arg("gui:expert", false, "expert mode for developpers");
#ifdef GEO_OS_ANDROID
	declare_arg("gui:phone_screen", true, "running on a phone (or testing)");	
#else	
	declare_arg("gui:phone_screen", false, "running on a phone (or testing)");
#endif	
    }
    
    /************************************************************************/

    /**
     * \brief Sets the CAD profile
     */
    void set_profile_cad() {
        set_arg("pre:repair", true);
        set_arg_percent("pre:margin", 0.05);
        set_arg("post:repair", true);
        set_arg("remesh:sharp_edges", true);
        set_arg("remesh:RVC_centroids", false);
    }

    /**
     * \brief Sets the scanner profile
     */
    void set_profile_scan() {
        set_arg("pre:Nsmooth_iter", 3);
        set_arg("pre:repair", true);
        set_arg_percent("pre:max_hole_area", 10);
        set_arg("remesh:anisotropy", 1.0);
        set_arg_percent("pre:min_comp_area", 3);
        set_arg_percent("post:min_comp_area", 3);
    }

    /**
     * \brief Sets the conversion profile
     */
    void set_profile_convert() {
        set_arg("pre", false);
        set_arg("post", false);
        set_arg("remesh", false);
    }

    /**
     * \brief Sets the repair profile
     */
    void set_profile_repair() {
        set_arg("pre", true);
        set_arg("pre:repair", true);
        set_arg("post", false);
        set_arg("remesh", false);
    }

    /**
     * \brief Sets the heal profile
     */
    void set_profile_heal() {
        set_arg("remesh", true);
        set_arg("remesh:multi_nerve", false);
        set_arg("post", true);
        set_arg_percent("post:max_hole_area", 10);
        set_arg_percent("post:min_comp_area", 3);
    }

    /**
     * \brief Sets the reconstruction profile
     */
    void set_profile_reconstruct() {
        set_arg("pre", false);
        set_arg("post", false);
        set_arg("remesh", false);
        set_arg("co3ne", true);
    }

    /**
     * \brief Sets the hex-dominant meshing profile
     */
    void set_profile_hex() {
        set_arg("hex", true);
    }

    /**
     * \brief Sets the quad-dominant meshing profile
     */
    void set_profile_quad() {
        set_arg("quad", true);
    }
    
    /**
     * \brief Sets the tetrahedral meshing profile
     */
    void set_profile_tet() {
        set_arg("tet", true);
    }

    /**
     * \brief Sets the polyhedral meshing profile
     */
    void set_profile_poly() {
        set_arg("poly", true);
    }
}

namespace GEO {

    namespace CmdLine {

        bool import_arg_group(
            const std::string& name
        ) {
	    static std::set<std::string> imported;
	    if(imported.find(name) != imported.end()) {
		return true;
	    }
	    imported.insert(name);
	    
            if(name == "standard") {
                import_arg_group_global();
                import_arg_group_sys();
		import_arg_group_nl();		
                import_arg_group_log();
		import_arg_group_biblio();
            } else if(name == "global") {
                import_arg_group_global();
            } else if(name == "nl") {
	        import_arg_group_nl();
	    } else if(name == "sys") {
                import_arg_group_sys();
            } else if(name == "log") {
                import_arg_group_log();
            } else if(name == "pre") {
                import_arg_group_pre();
            } else if(name == "remesh") {
                import_arg_group_remesh();
            } else if(name == "algo") {
                import_arg_group_algo();
            } else if(name == "post") {
                import_arg_group_post();
            } else if(name == "opt") {
                import_arg_group_opt();
            } else if(name == "co3ne") {
                import_arg_group_co3ne();
            } else if(name == "stat") {
                import_arg_group_stat();
            } else if(name == "quad") {
                import_arg_group_quad();
            } else if(name == "hex") {
                import_arg_group_hex();
            } else if(name == "tet") {
                import_arg_group_tet();
            } else if(name == "poly") {
                import_arg_group_poly();
            } else if(name == "gfx") {
                import_arg_group_gfx();
            } else if(name == "gui") {
		import_arg_group_gui();
	    } else {
                Logger::instance()->set_quiet(false);
                Logger::err("CmdLine")
                    << "No such option group: " << name
                    << std::endl;
                return false;
            }
            return true;
        }

        bool set_profile(
            const std::string& name
        ) {
            if(name == "cad") {
                set_profile_cad();
            } else if(name == "scan") {
                set_profile_scan();
            } else if(name == "convert") {
                set_profile_convert();
            } else if(name == "repair") {
                set_profile_repair();
            } else if(name == "heal") {
                set_profile_heal();
            } else if(name == "reconstruct") {
                set_profile_reconstruct();
            } else if(name == "tet") {
                set_profile_tet();
            } else if(name == "quad") {
                set_profile_quad();
            } else if(name == "hex") {
                set_profile_hex();
            } else if(name == "poly") {
                set_profile_poly();
            } else {
                Logger::instance()->set_quiet(false);
                Logger::err("CmdLine")
                    << "No such profile: " << name
                    << std::endl;
                return false;
            }
            return true;
        }
    }
}

