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

#include <geogram/parameterization/mesh_segmentation.h>
#include <geogram/mesh/mesh.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/mesh_io.h>
#include <geogram/mesh/mesh_manifold_harmonics.h>
#include <geogram/voronoi/CVT.h>
#include <geogram/voronoi/RVD.h>
#include <geogram/voronoi/RVD_callback.h>
#include <geogram/voronoi/generic_RVD_polygon.h>
#include <geogram/points/principal_axes.h>
#include <geogram/numerics/matrix_util.h>
#include <geogram/basic/numeric.h>

#include <deque>
#include <stack>

/**************************************************************************
 ****  SMOOTH PARTITION                                                ****
 **************************************************************************/

namespace {
    using namespace GEO;


    /**
     * \brief Iterator that traverses all facets incident to 
     * a given internal vertex.
     * \param[in] M a reference to the mesh
     * \param[in] f a facet incident to the vertex
     * \param[in] lv the local index of the vertex in \p f
     * \param[in] CB the function or lambda to be called for 
     *  all facets incident to the vertex
     */
    inline void for_each_facet_around_internal_vertex(
	const Mesh& M, index_t f, index_t lv,
	std::function<void(index_t, index_t)> CB
    ) {
	index_t v = M.facets.vertex(f,lv);
	index_t cur_f = f;
	index_t cur_lv = lv;
	index_t count = 0;
	do {
	    CB(cur_f, cur_lv);
	    cur_f = M.facets.adjacent(cur_f,cur_lv);
	    geo_assert(cur_f != index_t(-1));
	    cur_lv = M.facets.find_vertex(cur_f, v);
	    geo_assert(cur_lv != index_t(-1));
	    ++count;
	    geo_assert(count < 10000); // sanity check (are we looping forever?)
	} while(cur_f != f);
    }

    /**
     * \brief Utility class for mesh partition smoothing
     * \details Determines whether chart indices in the facets incident to
     *  a given vertex could be changed to reduce chart border length
     */
    class SmoothVertex {
    public:

	SmoothVertex() {
	}

        /**
         * \brief SmoothVertex constructor
         * \param[in] M a reference to the mesh
         * \param[in] chart a reference to the partition facet attribute 
         * \param[in] f a facet incident to the vertex
         * \param[in] lv the local index of the vertex in \p f
         */
	SmoothVertex(
	    const Mesh& M,
	    Attribute<index_t>& chart,
	    index_t f,
	    index_t lv
	) {
	    f_ = f;
	    lv_ = lv;
	    v_ = M.facets.vertex(f_,lv_);
	    is_valid_ = true;
	    chart_id_ = index_t(-1);

	    // Get chart1 and chart2 Ids
	    index_t chart1 = index_t(-1);
	    index_t chart2 = index_t(-1);
	    index_t prev_chart = index_t(-1);
	    for_each_facet_around_internal_vertex(
		M,f,lv,
		[&](index_t cur_f, index_t cur_lv) {
		    geo_argused(cur_lv);
		    if(chart1 == index_t(-1)) {
			chart1 = chart[cur_f];
		    } else if(chart[cur_f] != chart1 && chart2 == index_t(-1)) {
			chart2 = chart[cur_f];
		    }
		    prev_chart = chart[cur_f];
		}
	    );

	    // Test that vertex is incident to at most
	    // two charts and that chart id changes at most
	    // twice when turning around the vertex
	    index_t nb_change=0;
	    index_t nb_chart1=0;
	    index_t nb_chart2=0;
	    for_each_facet_around_internal_vertex(
		M,f,lv,
		[&](index_t cur_f, index_t cur_lv) {
		    geo_argused(cur_lv);
		    if(chart[cur_f] != prev_chart) {
			++nb_change;
		    }
		    prev_chart = chart[cur_f];
		    if(chart[cur_f] == chart1) {
			++nb_chart1;
		    } else if(chart[cur_f] == chart2) {
			++nb_chart2;
		    } else {
			is_valid_ = false;
		    }
		}
	    );
	    is_valid_ = is_valid_ &&
		           (chart1 != index_t(-1)) &&
		           (chart2 != index_t(-1)) ;
	    is_valid_ = is_valid_ && (nb_change <= 2);
	    if(!is_valid_) {
		return;
	    }
	    chart_id_ = (nb_chart1 > nb_chart2) ? chart1 : chart2;

	    // Compute delta len
	    delta_len_ = 0.0;
	    for_each_facet_around_internal_vertex(
		M,f,lv,
		[&](index_t cur_f, index_t cur_lv) {
		    index_t N = M.facets.nb_vertices(cur_f);
		    index_t prev_lv = (cur_lv == 0)   ? (N-1) : cur_lv - 1;
		    index_t next_lv = (cur_lv == N-1) ?  0    : cur_lv + 1;
		    index_t prev_v = M.facets.vertex(cur_f, prev_lv);
		    index_t v      = M.facets.vertex(cur_f, cur_lv);
		    index_t next_v = M.facets.vertex(cur_f, next_lv);
		    vec3 prev_p(M.vertices.point_ptr(prev_v));
		    vec3 p(M.vertices.point_ptr(v));
		    vec3 next_p(M.vertices.point_ptr(next_v));
		    if(chart[cur_f] !=
		       chart[M.facets.adjacent(cur_f, cur_lv)]) {
			delta_len_ += Geom::distance(p, next_p);
		    }
		    if(chart[cur_f] != chart_id_) {
			delta_len_ -= Geom::distance(prev_p, p);
		    }
		}
	    );
            is_valid_ = is_valid_ && delta_len_ > 0;
	}

        /**
         * \brief used to sort a vector of SmoothVertex and
         *  smooth them in order of priority.
         */
        bool operator<(const SmoothVertex& rhs) const {
            return (delta_len_ < rhs.delta_len_) ;
        }

        /**
         * \brief Changes chart ids in the facets incident to
         *  this SmoothVertex.
         * \param[in] M a reference to the mesh
         * \param[in,out] chart a reference to the segmentation facet attribute
         * \param[in,out] v_is_locked a vector of booleans that forbids 
         *  smoothing the neighbors of a vertex that was already smoothed
         */
        bool apply(
	    Mesh& M,
            Attribute<index_t>& chart, 
	    std::vector<bool>& v_is_locked
        ) {
	    if(v_is_locked[M.facets.vertex(f_, lv_)]) {
		return false;
	    }
	    for_each_facet_around_internal_vertex(
		M,f_,lv_,
		[&](index_t cur_f, index_t cur_lv) {
		    chart[cur_f] = chart_id_;
		    index_t N = M.facets.nb_vertices(cur_f);
		    index_t next_lv = (cur_lv == N-1) ? 0 : cur_lv + 1;
		    v_is_locked[M.facets.vertex(cur_f,next_lv)] = true;
		}
	    );
	    return true;
	}

        /**
         * \brief Tests whether this SmoothVertex can be applied.
         * \details A SmoothVertex cannot be applied if it is incident
         *  to more than two charts or if one of its neighbors are incident
         *  to more than two charts, or if chart id changes more than twice
         *  when turning around it.
         * \retval true if it can be applied, false otherwise
         */
        bool is_valid() const {
	    return is_valid_ ;
	}
	
    public:
	index_t f_;
	index_t lv_;
	index_t v_;
	index_t chart_id_;
	double delta_len_;
	bool is_valid_;
    };

    /*****************************************************/

    /**
     * \brief Smoothes a mesh segmentation by changing chart ids in order
     *  to reduce total chart border length
     * \param[in,out] M a reference to a mesh
     * \param[in] nb_iter number of iterations of partition smoothing
     */
    void mesh_smooth_segmentation(Mesh& M, index_t nb_iter=10) {

	// For each vertex, store one facet incident to that vertex
	vector<index_t> v_to_f(M.vertices.nb(), index_t(-1));
	for(index_t c: M.facet_corners) {
	    v_to_f[M.facet_corners.vertex(c)] =
		M.facet_corners.adjacent_facet(c) ;
	}

	vector<bool> v_on_border(M.vertices.nb(), false);
	for(index_t c: M.facet_corners) {
	    index_t v = M.facet_corners.vertex(c);
	    if(M.facet_corners.adjacent_facet(c) == index_t(-1)) {
		v_on_border[v] = true;
	    }
	}
	
	// Remove vertices on border and vertices adjacent to a vertex
	// on border
	for(index_t f: M.facets) {
	    for(index_t c1: M.facets.corners(f)) {
		index_t v1 = M.facet_corners.vertex(c1);
		index_t c2 = M.facets.next_corner_around_facet(f,c1);
		index_t v2 = M.facet_corners.vertex(c2);		
		if(
		    M.facet_corners.adjacent_facet(c1) == index_t(-1) ||
                    v_on_border[v2]
		) {
		    v_to_f[v1] = index_t(-1);
		}
	    }
	}
	
	Attribute<index_t> chart(M.facets.attributes(),"chart");
	vector<bool> v_is_locked; 
	vector<SmoothVertex> smooth_vertices;
	
	for(index_t i=0; i<nb_iter; ++i) {
	    smooth_vertices.resize(0);
	    v_is_locked.assign(M.vertices.nb(),false);	    
	    for(index_t v: M.vertices) {
                // skip vertices on border
                //  and vertices adjacent to vertices on border
                //  and isolated vertices
		if(v_to_f[v] == index_t(-1)) {
		    continue;
		}
		SmoothVertex sv(
		    M, chart, v_to_f[v], M.facets.find_vertex(v_to_f[v],v)
		);
		if(sv.is_valid()) {
		    smooth_vertices.push_back(sv);
		}
	    }
            std::sort(smooth_vertices.begin(), smooth_vertices.end()) ;
	    bool changed = false;
	    index_t nb = 0;
	    for(SmoothVertex& sv: smooth_vertices) {
		if(sv.apply(M, chart, v_is_locked)) {
		    ++nb;
		    changed = true;
		}
	    }
	    if(!changed) {
		break;
	    }
	}
    }


    /**
     * \brief Makes sure that each chart of the segmentation is 
     *  connected.
     * \details Segmentation is stored in the "chart" facet attribute.
     *  Generates a new chart id for each connected component of 
     *  the input charts.
     * \return number of charts
     */
    index_t mesh_postprocess_segmentation(Mesh& M, bool verbose=false) {
        Attribute<index_t> chart;
        chart.bind_if_is_defined(M.facets.attributes(),"chart");
        geo_assert(chart.is_bound());

        // Mark facets as non-visited by negating chart id
        for(index_t f: M.facets) {
            signed_index_t id = -signed_index_t(chart[f])-1;
            chart[f] = index_t(id);
        }

        std::stack<index_t> S;
        index_t cur_chart = 0;
        for(index_t f: M.facets) {
            index_t f_chart = chart[f];
            if(signed_index_t(f_chart) < 0) {
                chart[f] = cur_chart;
                S.push(f);
                while(!S.empty()) {
                    index_t cur_f = S.top();
                    S.pop();
                    for(index_t e=0; e<M.facets.nb_vertices(cur_f); ++e) {
                        index_t neigh_f = M.facets.adjacent(cur_f,e);
                        if(
                            neigh_f != index_t(-1) &&
                            chart[neigh_f] == f_chart
                        ) {
                            chart[neigh_f] = cur_chart;
                            S.push(neigh_f);
                        }
                    }
                }
                ++cur_chart;
            }
        }
        if(verbose) {
            Logger::out("Segmentation") << cur_chart << " charts" << std::endl;
        }
        return cur_chart;
    }
}

/***************************************************************************
 ***** MESH_SEGMENT CVT                                                *****
 ***************************************************************************/

namespace {
    using namespace GEO;

    /**
     * \brief Helper class for mesh_segment()
     * \details In a restricted Voronoi diagram, finds for each facet
     *  of the mesh the id of the Voronoi cell that has the largest
     *  intersection with the mesh.
     */
    class PartitionCB : public RVDPolygonCallback {
    public:
	PartitionCB(const Mesh* mesh) : mesh_(mesh) {
	}

	void begin() override {
	    facet_seed_.assign(mesh_->facets.nb(), index_t(-1));
	    facet_RVD_area_.assign(mesh_->facets.nb(), 0.0);
	}

	void end() override {
	    Attribute<index_t> chart(mesh_->facets.attributes(), "chart");
	    for(index_t f:mesh_->facets) {
		chart[f] = facet_seed_[f];
	    }
	}
	
	void operator() (
	    index_t v,
	    index_t t,
	    const GEOGen::Polygon& C
	) const override {
	    double A = area(C);
	    if(facet_seed_[t] == index_t(-1) || A > facet_RVD_area_[t]) {
		facet_seed_[t] = v;
		facet_RVD_area_[t] = A;
	    }
	}
    
        double area(const GEOGen::Polygon& C) const {
	    double result = 0.0;
	    vec3 p0(C.vertex(0).point());
	    for(index_t i=1; i<C.nb_vertices()-1; ++i) {
		vec3 pi(C.vertex(i).point());
		vec3 pj(C.vertex(i+1).point());
		result += Geom::triangle_area(p0,pi,pj);
	    }
	    return result;
	}
    
    private:
	const Mesh* mesh_;
	mutable vector<index_t> facet_seed_;
	mutable vector<double> facet_RVD_area_;
    };
}

/***************************************************************************
 ***** MESH_SEGMENT PPAL AXIS                                          *****
 ***************************************************************************/

namespace {
    using namespace GEO;

    /**
     * \brief Splits a chart along one of its principal axis
     * \details Greedily grow two charts from the two facets that
     *  are furthest away along the specified axis
     * \param[in] M a reference to the Mesh
     * \param[in] axis one of 0,1,2
     * \retval true if chart boundary touches a border
     * \retval false otherwise
     */
    bool split_chart_along_principal_axis(Mesh & M, index_t axis) {
        Attribute<index_t> chart(M.facets.attributes(), "chart");

        PrincipalAxes3d axes ;
        axes.begin() ;
	for(index_t f: M.facets) {
	    for(index_t lv=0; lv<M.facets.nb_vertices(f); ++lv) {
		index_t v = M.facets.vertex(f,lv);
		axes.add_point(vec3(M.vertices.point_ptr(v)));
	    }
	}
        axes.end() ;
        vec3 center = axes.center() ;
	vec3 X = axes.axis(axis) ;

        vector<double> X_coord(M.facets.nb());
        
        for(index_t f: M.facets) {
            X_coord[f] = dot(X,(Geom::mesh_facet_center(M,f) - center));
        }

        vector<double> axis_coord = X_coord;
        std::sort(axis_coord.begin(), axis_coord.end());
        double X_cutoff = axis_coord[axis_coord.size()/2];
        
        for(index_t f: M.facets) {
            chart[f] = (X_coord[f] > X_cutoff);;
        }

        // Test whether chart boundary touches mesh border
        // (which is what we want if we split a cylindroid
        // or a sockoid). 
        for(index_t f: M.facets) {
            index_t N = M.facets.nb_vertices(f);
            for(index_t e1=0; e1<N; ++e1) {
                index_t e2 = (e1+1)%N;

                index_t adj1 = M.facets.adjacent(f,e1);
                index_t adj2 = M.facets.adjacent(f,e2);

                if(
                    adj1 == index_t(-1) &&
                    adj2 != index_t(-1) &&
                    chart[adj2] != chart[f]
                ) {
                    return true;
                }

                if(
                    adj2 == index_t(-1) &&
                    adj1 != index_t(-1) &&
                    chart[adj1] != chart[f]
                ) {
                    return true;
                }
                
            }
            
        }
        
        return false;
    }
}

namespace GEO {
    
    index_t mesh_segment(
        Mesh& M, MeshSegmenter segmenter, index_t nb_segments, bool verbose
    ) {
        geo_assert(M.facets.are_simplices());
        
        double anisotropy =
            (segmenter == SEGMENT_GEOMETRIC_VSA_L12) ? 2.0 : 0.0;
	index_t dimension = 0;
	index_t nb_manifold_harmonics=0; 

	switch(segmenter) {
        case SEGMENT_GEOMETRIC_VSA_L2:                 break;
        case SEGMENT_GEOMETRIC_VSA_L12:                break;
        case SEGMENT_INERTIA_AXIS:                     break;
	case SEGMENT_SPECTRAL_8:        dimension=8;   break;
	case SEGMENT_SPECTRAL_20:       dimension=20;  break;
	case SEGMENT_SPECTRAL_100:      dimension=100; break;
	}

	Attribute<double> geom_bkp;

        if(segmenter == SEGMENT_INERTIA_AXIS) {
            // Pick the axis such that the segmentation obtained
            // by splitting along it has a chart boundary that
            // touches the mesh boundary.
            for(index_t axis=0; axis<3; ++axis) {
                if(split_chart_along_principal_axis(M, 2-axis)) {
                    break;
                }
            }
            mesh_smooth_segmentation(M);
            return mesh_postprocess_segmentation(M,verbose);
        }
        
	if(dimension != 0) {
	    nb_manifold_harmonics = dimension+20;
	    geom_bkp.create_vector_attribute(
		M.vertices.attributes(), "bkp", 3
	    );
	    for(index_t v: M.vertices) {
		geom_bkp[3*v]   = M.vertices.point_ptr(v)[0];
		geom_bkp[3*v+1] = M.vertices.point_ptr(v)[1];
		geom_bkp[3*v+2] = M.vertices.point_ptr(v)[2];
	    }
	    mesh_compute_manifold_harmonics(
		M, nb_manifold_harmonics,
		FEM_P1_LUMPED, "eigen", 0.0, true
	    );
	    Attribute<double> eigen(
		M.vertices.attributes(), "eigen"
	    );
	    M.vertices.set_dimension(dimension);
	    for(index_t v: M.vertices) {
		for(index_t mh=0; mh<dimension; ++mh) {
		    M.vertices.point_ptr(v)[mh] =
			eigen[nb_manifold_harmonics*v + mh + 1];
		}
	    }
            eigen.destroy();
	} else if(anisotropy != 0.0) {
	    compute_normals(M);
	    // smooth normals --------------.
            //                              v
	    simple_Laplacian_smooth(M, 3, true);
	    set_anisotropy(M,anisotropy*0.02);
	}
	
	CentroidalVoronoiTesselation CVT(&M);
	CVT.compute_initial_sampling(nb_segments);
        if(verbose) {
            Logger::out("RVD") << "Optimizing CVT" << std::endl;
        }
	CVT.Lloyd_iterations(30);
	CVT.Newton_iterations(10);
	PartitionCB CB(&M);
	CVT.RVD()->for_each_polygon(CB);

	if(nb_manifold_harmonics != 0) {
	    M.vertices.set_dimension(3);
	    for(index_t v: M.vertices) {
		M.vertices.point_ptr(v)[0] = geom_bkp[3*v];
		M.vertices.point_ptr(v)[1] = geom_bkp[3*v+1];
		M.vertices.point_ptr(v)[2] = geom_bkp[3*v+2];
	    }
	    geom_bkp.destroy();
	} else if(anisotropy != 0.0) {
	    M.vertices.set_dimension(3);
	}

        mesh_smooth_segmentation(M);
        return mesh_postprocess_segmentation(M,verbose);        
    }
}

/***************************************************************************/

