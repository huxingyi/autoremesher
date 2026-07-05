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

#include <geogram/parameterization/mesh_atlas_maker.h>
#include <geogram/parameterization/mesh_LSCM.h>
#include <geogram/parameterization/mesh_ABF.h>
#include <geogram/parameterization/mesh_segmentation.h>
#include <geogram/parameterization/mesh_param_validator.h>
#include <geogram/parameterization/mesh_param_packer.h>
#include <geogram/mesh/mesh.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/mesh_halfedges.h>
#include <geogram/mesh/mesh_topology.h>
#include <geogram/mesh/mesh_io.h>
#include <geogram/points/principal_axes.h>
#include <geogram/basic/progress.h>
#include <deque>
#include <stack>

namespace {
    using namespace GEO;

    /**
     * \brief Computes a mesh parameterization by projection onto the
     *  least squares average plane.
     * \param[in] M the chart to be parameterized.
     */
    void mesh_parameterize_by_projection(Mesh& M) {
	vec3 N;
	vec3 center;

	if(M.facets.nb() == 1) {
	    index_t f = 0;
	    center = Geom::mesh_facet_center(M, f);
	    N = Geom::mesh_facet_normal(M,f);
	} else {
	    PrincipalAxes3d LSN;
	    LSN.begin();
	    for(index_t f : M.facets) {
		for(index_t c: M.facets.corners(f)) {
		    index_t v = M.facet_corners.vertex(c);
		    LSN.add_point(vec3(M.vertices.point_ptr(v)));		
		}
	    }
	    LSN.end();
	    center = LSN.center();
	    N = LSN.normal();
	}
	
	vec3 U = normalize(Geom::perpendicular(N));
	vec3 V = normalize(cross(N,U));

        Attribute<double> tex_coord;
        tex_coord.bind_if_is_defined(M.vertices.attributes(), "tex_coord");
        if(!tex_coord.is_bound()) {
            tex_coord.create_vector_attribute(
                M.vertices.attributes(), "tex_coord", 2
            );
        }
        geo_assert(tex_coord.dimension() == 2);        
        for(index_t v: M.vertices) {
            vec3 p(M.vertices.point_ptr(v));
            p -= center;
            double pu = dot(p,U);
            double pv = dot(p,V);
            tex_coord[2*v]   = pu;
            tex_coord[2*v+1] = pv;
        }
    }

    void measure_chart(
        Mesh& chart,
        signed_index_t& Xi,
        index_t& nb_borders,
        double& volume, double& surface_area, double& holes_area
    ) {
        vec3 origin(0.0, 0.0, 0.0);
        
        volume       = 0.0;
        surface_area = 0.0;
        holes_area   = 0.0;
        nb_borders   = 0;
        
        vector<bool> visited(chart.facet_corners.nb(), false);
        MeshHalfedges MH(chart);
        vector<vec3> P;
        for(index_t f: chart.facets) {
            for(index_t c: chart.facets.corners(f)) {
                if(
                    chart.facet_corners.adjacent_facet(c) == index_t(-1) &&
                    !visited[c]
                ) {
                    ++nb_borders;
                    P.resize(0);
                    MeshHalfedges::Halfedge H(f,c);
                    do {
                        visited[H.corner] = true;
                        index_t v = chart.facet_corners.vertex(H.corner);
                        P.push_back(vec3(chart.vertices.point_ptr(v)));
                        MH.move_to_next_around_border(H);
                    } while(!visited[H.corner]);
                    vec3 G(0.0,0.0,0.0);
                    for(vec3 p : P) {
                        G += p;
                    }
                    G = (1.0/double(P.size())) * G;
                    for(index_t i=0; i<P.size(); ++i) {
                        index_t j = (i+1)%P.size();
                        holes_area += Geom::triangle_area(P[j],P[i],G);
                        volume += Geom::tetra_signed_volume(
                            origin, P[j], P[i], G
                        );
                    }
                }
            }
        }
        for(index_t f: chart.facets) {
            geo_assert(chart.facets.nb_vertices(f) == 3);
            index_t v1 = chart.facets.vertex(f,0);
            index_t v2 = chart.facets.vertex(f,1);
            index_t v3 = chart.facets.vertex(f,2);            
            volume += Geom::tetra_signed_volume(
                origin,
                vec3(chart.vertices.point_ptr(v1)),
                vec3(chart.vertices.point_ptr(v2)),
                vec3(chart.vertices.point_ptr(v3))
            );                 
            
            surface_area += Geom::triangle_area(
                vec3(chart.vertices.point_ptr(v1)),
                vec3(chart.vertices.point_ptr(v2)),
                vec3(chart.vertices.point_ptr(v3))                 
            );
        }

        volume = ::fabs(volume);
        Xi = mesh_Xi(chart);
    }

    enum ChartType {
        CHART_TYPE_MONSTROID  = 0,
        CHART_TYPE_DISKOID    = 1,
        CHART_TYPE_CYLINDROID = 2,
        CHART_TYPE_SOCKOID    = 3
    };

// Uncomment to save first segments with a name
// that indicates how it was classified    
// #define DEBUG_CHART_CLASSIFICATION
    
#ifdef DEBUG_CHART_CLASSIFICATION
    const char* chart_type_as_string(ChartType c) {
        static const char* names[] = {
            "monstroid",
            "diskoid",
            "cylindroid",
            "sockoid"
        };
        return names[c];
    }
#endif
    
    ChartType chart_type(Mesh& chart) {
        signed_index_t Xi;
        index_t nb_borders;
        double volume;
        double surface_area;
        double holes_area;
        
        measure_chart(chart, Xi, nb_borders, volume, surface_area, holes_area);

        double hs_ratio = holes_area / surface_area;
        
        if(nb_borders == 1 && Xi == 1) {
            return (hs_ratio < 1.0/2.0) ?
                CHART_TYPE_SOCKOID :
                CHART_TYPE_DISKOID ;
        }

        if(nb_borders == 2 && Xi == 0 && hs_ratio < 1.0
        ) {
            return (hs_ratio < 1.0) ?
                CHART_TYPE_CYLINDROID :
                CHART_TYPE_DISKOID;
        }

        return CHART_TYPE_MONSTROID;
    }
    
    /**
     * \brief Computes a texture atlas.
     */
    class AtlasMaker {
    public:
	
	AtlasMaker(Mesh& mesh) :
	    mesh_(mesh),
	    hard_angles_threshold_(0.0) {
	    tex_coord_.bind_if_is_defined(
		mesh_.facet_corners.attributes(), "tex_coord"
	    );
	    if(!tex_coord_.is_bound()) {
		tex_coord_.create_vector_attribute(
		    mesh_.facet_corners.attributes(), "tex_coord", 2
		);
	    }
            geo_assert(tex_coord_.dimension() == 2);
	    chart_parameterizer_ = PARAM_ABF;
	    verbose_ = false;
#ifdef GEO_OS_ANDROID
            max_chart_size_ = 3000;
#else            
            max_chart_size_ = 30000; 
#endif            
	}

	~AtlasMaker() {
	}

	void set_verbose(bool x) {
	    verbose_ = x;
	    validator_.set_verbose(x);
	}
	
	void set_hard_angles_threshold(double x) {
	    hard_angles_threshold_ = x;
	}

	void set_chart_parameterizer(ChartParameterizer param) {
	    chart_parameterizer_ = param;
	}
	
	void make_atlas() {

	    ProgressTask progress("Atlas",100);
	    progress.progress(0);

            // Total number of facets in triangulated mesh
	    index_t total_f = 0;
            for(index_t f: mesh_.facets) {
                total_f += (mesh_.facets.nb_vertices(f)-2);
            }

            // Current number of parameterized facets
	    index_t param_f = 0;

            std::stack<Mesh*> S;
            
            {
                get_initial_segmentation();
                vector<Mesh*> charts;
                get_charts(mesh_, charts);

                for(index_t i=0; i<charts.size(); ++i) {
#ifdef DEBUG_CHART_CLASSIFICATION
                    mesh_save(
                        *(charts[i]),
                        "chart_" + String::to_string(i) + "_" +
                        chart_type_as_string(chart_type(*charts[i]))  +
                        ".geogram"
                    );
#endif
                    S.push(charts[i]);
                }
            }

            try {
                while(!S.empty()) {
                    Mesh* M = S.top();
                    S.pop();
                    
                    if(verbose_) {
                        Logger::out("MAM") << "Processing chart, size="
                                           << M->facets.nb() << std::endl;
                    }
                    
                    if(
                        precheck_chart(*M)     &&
                        parameterize_chart(*M) &&
                        postcheck_chart(*M)
                    ) {
                        
                        if(verbose_) {
                            Logger::out("MAM") << "=== CHART OK" << std::endl;
                        }
                        
                        commit_chart(*M);
                        param_f += M->facets.nb();
                        progress.progress(param_f * 100 / total_f);
                        
                    } else {
                        
                        if(verbose_) {
                            Logger::out("MAM")
                                << "=== CHART NOT OK (splitting)" << std::endl;
                        }

                        index_t nb_segments =
                            M->facets.nb() / max_chart_size_ + 1;

                        MeshSegmenter segmenter = SEGMENT_GEOMETRIC_VSA_L2;
                        

                        ChartType type = chart_type(*M);
                        switch(type) {
                        case CHART_TYPE_MONSTROID:
                            nb_segments = std::max(nb_segments, index_t(7));
                            break;
                        case CHART_TYPE_DISKOID:
                            nb_segments = std::max(nb_segments, index_t(4));
                            break;
                        case CHART_TYPE_SOCKOID:
                            nb_segments = 2;
                            segmenter = SEGMENT_INERTIA_AXIS;
                            break;
                        case CHART_TYPE_CYLINDROID:
                            nb_segments = 2;
                            segmenter = SEGMENT_INERTIA_AXIS;
                            break;
                        }

                        nb_segments = std::max(nb_segments, index_t(6));
                        geo_assert(M->facets.nb() > 1);                        
                        if(
                            M->facets.nb() <= nb_segments ||
                            mesh_segment(*M, segmenter, nb_segments) < 2
                        ) {
                            Attribute<index_t> chart(
                                M->facets.attributes(),"chart"
                            );
                            for(index_t f: M->facets) {
                                chart[f] = f;
                            }
                        }                         
                        vector<Mesh*> charts;
                        get_charts(*M, charts);
                        for(Mesh* C: charts) {
                            S.push(C);
                        }
                    }
                    delete M;
                }
            } catch(...) {
		Logger::out("Atlas") << "Job canceled" << std::endl;
	    }
	}
	
    protected:

        /*
         * \brief Segments mesh along sharp creases and optional
         *  initial segmentation in the "chart" facet attribute.
         * \details Sharp creases are edges for which the dihedral
         *  angle is larger than hard_angles_threshold_ or that are
         *  adjacent to two facets with different "chart" attribute
         *  (if specified).
         */

	void get_initial_segmentation() {
            Attribute<index_t> initial_chart;
            if(Attribute<index_t>::is_defined(
                   mesh_.facets.attributes(), "chart"
            )) {
                initial_chart.bind(
                    mesh_.facets.attributes(), "initial_chart"
                );
            }
            Attribute<index_t> chart(mesh_.facets.attributes(), "chart");
	    for(index_t f: mesh_.facets) {
                if(initial_chart.is_bound()) {
                    initial_chart[f] = chart[f];
                }
		chart[f] = index_t(-1);
	    }
            
	    index_t cur_chart = 0;
	    for(index_t f: mesh_.facets) {
		std::stack<index_t> S;
		if(chart[f] == index_t(-1)) {
		    chart[f] = cur_chart;
		    S.push(f);
		    do {
			index_t cur_f = S.top();
			S.pop();
			for(index_t c: mesh_.facets.corners(cur_f)) {
			    index_t f2=mesh_.facet_corners.adjacent_facet(c);

                            if(f2 == NO_FACET) {
                                continue;
                            }
                            
                            bool is_on_chart_border = (
                                Geom::mesh_unsigned_normal_angle(
                                    mesh_,cur_f,f2) > hard_angles_threshold_
                            );
                            if(initial_chart.is_bound()) {
                                is_on_chart_border = is_on_chart_border ||
                                    (initial_chart[cur_f] !=
                                     initial_chart[f2]);
                            }
                            if(chart[f2] != cur_chart && !is_on_chart_border) {
                                chart[f2] = cur_chart;
                                S.push(f2);
			    }
                        }
		    } while(!S.empty());
		    ++cur_chart;
		}
	    }

            if(cur_chart == 1 && mesh_.facets.nb() > max_chart_size_) {
                index_t nb_charts = mesh_.facets.nb() / max_chart_size_ + 1;
                nb_charts = std::max(nb_charts, index_t(4));
                mesh_segment(mesh_, SEGMENT_GEOMETRIC_VSA_L2, nb_charts);
            }
        }

        /**
         * \brief Extracts one mesh per chart in input mesh
         * \details Charts are determined from the "chart" facet attribute
         * \param[in] M the mesh
         * \param[in,out] charts the extracted charts. Caller
         *  has the responsibility of deallocating each individual
         *  mesh.
         */
        void get_charts(Mesh& M, vector<Mesh*>& charts) {
            Attribute<index_t> chart(M.facets.attributes(), "chart");
            Attribute<index_t> vertex_id(M.vertices.attributes(), "id");
            
            // If M is a chart that was obtained by a previous call to
            // get_charts(), M_corner is already bound (else it is unbound):
            // For each facet corner of M, keeps the facet corner id in mesh_
            // (will be used to transfer texture coordinates after chart
            //  parameterization).
            Attribute<index_t> M_corner;
            M_corner.bind_if_is_defined(
                M.facet_corners.attributes(),"corner_id"
            );

            vector<index_t> facets;
            vector<bool> f_is_visited(M.facets.nb(),false);
            std::stack<index_t> S;
            
            for(index_t f0: M.facets) {
                if(!f_is_visited[f0]) {

                    charts.push_back(new Mesh);
                    Mesh& C = *(charts[charts.size()-1]);
                    C.vertices.set_dimension(3);
                    // For each facet corner of C, keeps the facet
                    // corner id in M (will be used to transfer texture
                    // coordinates after chart parameterization).
                    Attribute<index_t> C_corner(
                        C.facet_corners.attributes(), "corner_id"
                    );
                    
                    // Step 1: get chart facets
                    facets.resize(0);
                    facets.push_back(f0);
                    f_is_visited[f0] = true;
                    S.push(f0);

                    while(!S.empty()) {
                        index_t f = S.top();
                        S.pop();
                        for(index_t e=0; e<M.facets.nb_vertices(f); ++e) {
                            index_t g = M.facets.adjacent(f,e);
                            if(
                                g != index_t(-1) && !f_is_visited[g] &&
                                chart[g] == chart[f0]
                            ) {
                                facets.push_back(g);
                                f_is_visited[g] = true;
                                S.push(g);
                            }
                        }
                    }

                    // step 2: reset vertex ids
                    for(index_t f: facets) {
                        for(index_t c: M.facets.corners(f)) {
                            index_t v = M.facet_corners.vertex(c);
                            vertex_id[v] = index_t(-1);
                        }
                    }

                    // step 3: copy vertices
                    index_t nb_vertices = 0;
                    for(index_t f: facets) {
                        for(index_t c: M.facets.corners(f)) {
                            index_t v = M.facet_corners.vertex(c);
                            if(vertex_id[v] == index_t(-1)) {
                                C.vertices.create_vertex(
                                    M.vertices.point_ptr(v)
                                );
                                vertex_id[v] = nb_vertices; // M to C
                                ++nb_vertices;
                            }
                        }
                    }

                    // step 4: copy (and triangulate) facets
                    for(index_t f: facets) {
                        index_t c1 = M.facets.corners_begin(f);
                        index_t v1 = vertex_id[M.facet_corners.vertex(c1)];
                        for(
                            index_t c2 = c1+1;
                            c2+1 < M.facets.corners_end(f); ++c2
                        ) {
                            index_t c3=c2+1;
                            index_t v2=vertex_id[M.facet_corners.vertex(c2)];
                            index_t v3=vertex_id[M.facet_corners.vertex(c3)];
                            geo_assert(v1 < nb_vertices);
                            geo_assert(v2 < nb_vertices);
                            geo_assert(v3 < nb_vertices);		    
                            index_t t = C.facets.create_triangle(v1,v2,v3);
                            if(&M == &mesh_) {
                                C_corner[C.facets.corner(t,0)] = c1;
                                C_corner[C.facets.corner(t,1)] = c2;
                                C_corner[C.facets.corner(t,2)] = c3;
                            } else {
                                C_corner[C.facets.corner(t,0)] = M_corner[c1];
                                C_corner[C.facets.corner(t,1)] = M_corner[c2];
                                C_corner[C.facets.corner(t,2)] = M_corner[c3];
                            }
                        }
                    }
                    C.facets.connect();
                }
            }
        }
        
        /**
         * \brief Tests done on chart before parameterization
         * \details A chart cannot be parameterized if it has no
         *   border. It will not be parameterized if it has too
         *   many facets
         * \param[in] chart the chart to be tested
         * \retval true if the chart is going to be parameterized
         * \retval false otherwise
         */
	bool precheck_chart(const Mesh& chart) {
            bool has_borders = false;
            for(index_t c: chart.facet_corners) {
                if(chart.facet_corners.adjacent_facet(c) == index_t(-1)) {
                    has_borders = true;
                    break;
                }
            }
            if(!has_borders) {
                if(verbose_) {
                    Logger::out("MAM") << "precheck !OK (chart has no border)"
                                       << std::endl;
                }
                return false;
            }
	    if(chart.facets.nb() > max_chart_size_) {
                if(verbose_) {
                    Logger::out("MAM") << "precheck !OK (chart too large)"
                                       << std::endl;
                }
                return false;
            }
            if(verbose_) {
                Logger::out("MAM") << "precheck OK" << std::endl;
            }
	    return true;
	}

        /**
         * \brief Compute U,V coordinates in chart
         * \details U,V coordinates are stored in the "tex_coord" vertex
         *  attribute
         * \param[in,out] chart the chart to be parameterized
         */
	bool parameterize_chart(Mesh& chart) {
            if(chart.facets.nb() == 1) {
                mesh_parameterize_by_projection(chart);
                return true;
            }
            switch(chart_parameterizer_) {
            case PARAM_PROJECTION:
                mesh_parameterize_by_projection(chart);
                break;
            case PARAM_LSCM:
                mesh_compute_LSCM(chart, "tex_coord", false, "", verbose_);
                break;
            case PARAM_SPECTRAL_LSCM:
                mesh_compute_LSCM(chart, "tex_coord", true, "", verbose_);
                break;
            case PARAM_ABF:
                mesh_compute_ABF_plus_plus(chart, "tex_coord", verbose_);
                break;
            }
            return true;
        }

	bool postcheck_chart(Mesh& chart) {
	    bool OK = validator_.chart_is_valid(chart);

	    // If a small chart has a problem, then try
	    // simply to project it.
	    if(!OK && chart.facets.nb() <= 10) {
                    mesh_parameterize_by_projection(chart);
                    // Some single-facet charts may fail to be validated if
                    // they are too skinny (filling ratio will be too bad),
                    // so we force accept if there is a single facet.
                    OK = (chart.facets.nb() == 1)  ||
                        validator_.chart_is_valid(chart);
                }
	    return OK;
	}
        
        /**
         * \brief Copies texture coordinates of a chart to the main mesh.
         * \details Texture coordinates are taken from the "tex_coord" vertex
         *  attribute of \p chart and written into the "tex_coord" facet corner
         *  attribute of mesh_, using the "corner_id" vertex attribute of
         *  \p chart that points towards the facet corners in mesh_.
         */
        void commit_chart(Mesh& chart) {
            Attribute<index_t> C_corner(
                chart.facet_corners.attributes(),"corner_id"
            );
            Attribute<double> C_tex_coord(
                chart.vertices.attributes(),"tex_coord"
            );
            Attribute<double> M_tex_coord(
                mesh_.facet_corners.attributes(), "tex_coord"
            );
            for(index_t c: chart.facet_corners) {
                index_t v = chart.facet_corners.vertex(c);                
                index_t mesh_c = C_corner[c];
                M_tex_coord[2*mesh_c  ] = C_tex_coord[2*v  ];
                M_tex_coord[2*mesh_c+1] = C_tex_coord[2*v+1];
            }
        }
        
    private:
	Mesh& mesh_;
	ChartParameterizer chart_parameterizer_;
	ParamValidator validator_;
	double hard_angles_threshold_; // in radians
	Attribute<double> tex_coord_;
        index_t max_chart_size_;
	Mesh chart_as_mesh_;
	bool verbose_;
    };

    /**
     * \brief Tests whether two facets are on the same chart
     * \details Two facets are on the same chart if their texture
     *  coordinates match along the edge they are adjacent to
     * \param[in] mesh a reference to the mesh
     * \param[in] f1 a facet of the mesh
     * \param[in] e1 an edge of \p f1
     * \param[in] tex_coord a reference to the facet corners tex coords
     */
    bool is_same_chart(
	const Mesh& mesh,
	index_t f1, index_t e1, const Attribute<double>& tex_coord
    ) {
	index_t c11 = mesh.facets.corners_begin(f1)+e1;
	index_t c12 = mesh.facets.next_corner_around_facet(f1,c11); 	
	index_t f2  = mesh.facets.adjacent(f1,e1);
	geo_assert(f2 != index_t(-1));
	index_t e2  = mesh.facets.find_adjacent(f2,f1);
	geo_assert(e2 != index_t(-1));
	index_t c21 = mesh.facets.corners_begin(f2)+e2;
	index_t c22 = mesh.facets.next_corner_around_facet(f2,c21); 		

	return
	    (tex_coord[2*c11  ] == tex_coord[2*c22  ]) &&
	    (tex_coord[2*c11+1] == tex_coord[2*c22+1]) &&
	    (tex_coord[2*c12  ] == tex_coord[2*c21  ]) &&
	    (tex_coord[2*c12+1] == tex_coord[2*c21+1]) ;
	    
    }
    
}

namespace GEO {

    void mesh_make_atlas(
	Mesh& mesh, double hard_angles_threshold, // in degrees
	ChartParameterizer param,
	ChartPacker pack,
	bool verbose 
    ) {
	AtlasMaker atlas(mesh);
	atlas.set_hard_angles_threshold(
	   hard_angles_threshold * M_PI / 180.0
	);
	atlas.set_chart_parameterizer(param);
	atlas.set_verbose(verbose);
	atlas.make_atlas();
        mesh_get_charts(mesh);
        switch(pack) {
        case PACK_NONE:
            break;
        case PACK_TETRIS:
            pack_atlas_using_tetris_packer(mesh);
            break;
        case PACK_XATLAS:
            pack_atlas_only_normalize_charts(mesh);
            pack_atlas_using_xatlas(mesh);
            break;
        }
    }

    index_t mesh_get_charts(Mesh& mesh) {
	Attribute<index_t> chart(mesh.facets.attributes(),"chart");
	Attribute<double> tex_coord;
	tex_coord.bind_if_is_defined(
	    mesh.facet_corners.attributes(), "tex_coord"
	);
	if(!tex_coord.is_bound()) {
	    Logger::err("Chart") << "mesh does not have facet corner tex coords"
				 << std::endl;
	    return 0;
	}
	if(tex_coord.dimension() != 2) {
	    Logger::err("Chart") << "facet corner tex coords not of dimension 2"
				 << std::endl;
	    return 0;
	}
	chart.fill(index_t(-1));
	std::stack<index_t> S;
	index_t current_chart = 0;
	for(index_t f : mesh.facets) {
	    if(chart[f] == index_t(-1)) {
		chart[f] = current_chart;
		S.push(f);
		while(!S.empty()) {
		    index_t f1 = S.top();
		    S.pop();
		    for(index_t e=0; e<mesh.facets.nb_vertices(f1); ++e) {
			index_t f2 = mesh.facets.adjacent(f1,e);
			if(
			    f2 != index_t(-1) &&
			    chart[f2] == index_t(-1) &&
			    is_same_chart(mesh,f1,e,tex_coord)
			) {
			    chart[f2] = current_chart;
			    S.push(f2);
			}
		    }
		}
		current_chart++;		
	    }
	}
        return current_chart;
    }
    
}
