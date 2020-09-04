/*
 *  Copyright (c) 2012-2016, Bruno Levy
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

#include <geogram/parameterization/mesh_segmentation.h>
#include <geogram/points/principal_axes.h>
#include <geogram/mesh/mesh.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/mesh_io.h>
#include <geogram/basic/numeric.h>
#include <geogram/numerics/matrix_util.h>

#include <deque>


namespace {
    using namespace GEO;


    /**
     * \brief Finds the facet of a mesh that is the furthest away
     *  from a given facet.
     * \param[in] chart a reference to a Chart
     * \param[in] f the facet
     * \return the facet of \p chart that is furthest away from \p f.
     */
    index_t furthest_facet(Chart& chart, index_t f) {
	vec3 p = Geom::mesh_facet_center(chart.mesh,f);
	index_t result = f;
	double best_dist2 = 0.0;
	for(index_t ff=0; ff<chart.facets.size(); ++ff) {
	    index_t f2 = chart.facets[ff];
	    if(f2 != f) {
		vec3 q = Geom::mesh_facet_center(chart.mesh,f2);
		double cur_dist2 = distance2(p,q);
		if(cur_dist2 >= best_dist2) {
		    result = f2;
		    best_dist2 = cur_dist2;
		}
	    }
	}
	return result;
    }
    
    void find_furthest_facet_pair_along_principal_axis(
        Chart& chart, index_t& f0, index_t& f1,
        index_t axis
    ) {
        f0 = NO_FACET;
        f1 = NO_FACET;
        double min_z = Numeric::max_float64() ;
        double max_z = Numeric::min_float64() ;

        PrincipalAxes3d axes ;
        axes.begin() ;
	for(index_t ff=0; ff<chart.facets.size(); ++ff) {
	    index_t f = chart.facets[ff];
	    for(index_t lv=0; lv<chart.mesh.facets.nb_vertices(f); ++lv) {
		index_t v = chart.mesh.facets.vertex(f,lv);
		axes.add_point(vec3(chart.mesh.vertices.point_ptr(v)));
	    }
	}
        axes.end() ;
        vec3 center = axes.center() ;

        // If these facets do not exist (for instance, in the case
        //  of a torus), find two facets far away one from each other
        //  along the longest axis.
        // vec3 X = axes.axis(2 - axis) ;
	vec3 X = axes.axis(axis) ;	
        if(f0 == NO_FACET || f1 == NO_FACET || f0 == f1) {
	    for(index_t ff=0; ff<chart.facets.size(); ++ff) {
		index_t f = chart.facets[ff];
		vec3 p = Geom::mesh_facet_center(chart.mesh, f);
                double z = dot(p - center, X) ;
                if(z < min_z) {
                    min_z = z ;
                    f0 = f ;
                }
                if(z > max_z) {
                    max_z = z ;
                    f1 = f ;
                }
            } 
        }
	if(f1 == f0) {
	    f1 = furthest_facet(chart, f0);
	}
    }


    /**
     * \brief Comparison functor for greedy algorithms that compute mesh 
     *  partitions.
     */
    class FacetDistanceCompare {
    public:

	/**
	 * \brief FacetDistanceCompare constructor.
	 * \param[in] dist_in a facet attribute attached to a surface.
	 */
	FacetDistanceCompare(Attribute<double>& dist_in) : distance(dist_in) {
	}

	/**
	 * \brief Compares two facets.
	 * \param[in] f1 , f2 the two facets.
	 * \retval true of the stored distance of \p f1 is smaller than the 
	 *  one for \p f2.
	 * \retval false otherwise.
	 */
	bool operator()(index_t f1, index_t f2) const {
	    return distance[f1] < distance[f2];
	}
	Attribute<double>& distance;
    };
}

namespace GEO {

    void split_chart_along_principal_axis(
	Chart& chart, Chart& new_chart_1, Chart& new_chart_2, index_t axis,
	bool verbose
    ) {

	if(verbose) {
	    Logger::out("Segment")
		<< "Splitting chart " << chart.id << " : size = "
		<< chart.facets.size() << std::endl;
	}
	
	Attribute<index_t> chart_id(chart.mesh.facets.attributes(), "chart");

	index_t f1,f2;
	find_furthest_facet_pair_along_principal_axis(
	    chart, f1, f2, axis
	);
	
        // Sanity check
	for(index_t ff=0; ff<chart.facets.size(); ++ff) {
	    index_t f = chart.facets[ff];
	    geo_assert(chart_id[f] == chart.id);
	}
	
	geo_assert(chart_id[f1] == chart.id);
	geo_assert(chart_id[f2] == chart.id);

        // Clear chart ids
	for(index_t ff=0; ff<chart.facets.size(); ++ff) {
	    index_t f = chart.facets[ff];
	    geo_assert(chart_id[f] == chart.id);
	    chart_id[f] = index_t(-1);
	}

	
	Attribute<double> distance(chart.mesh.facets.attributes(),"distance");
	for(index_t ff=0; ff<chart.facets.size(); ++ff) {
	    distance[chart.facets[ff]] = Numeric::max_float64();
	}
	FacetDistanceCompare facet_cmp(distance);

	// There is maybe a smarter way for having a priority queue with
	// modifiable weights...
	
        std::multiset< index_t, FacetDistanceCompare > queue(
	    facet_cmp
	);
	
        facet_cmp.distance[f1] = 0.0;      
        facet_cmp.distance[f2] = 0.0;

	chart_id[f1] = new_chart_1.id;
	chart_id[f2] = new_chart_2.id;
	
	new_chart_1.facets.clear();
	new_chart_2.facets.clear();
	
        queue.insert(f1);
        queue.insert(f2);

        while (!queue.empty()) {
            index_t top = *(queue.begin());
            queue.erase(queue.begin());
	    for(index_t c: chart.mesh.facets.corners(top)) {
		index_t neigh = chart.mesh.facet_corners.adjacent_facet(c);
		
		if(
		    neigh == index_t(-1) || (
			chart_id[neigh] != index_t(-1) &&
			chart_id[neigh] != new_chart_1.id &&
			chart_id[neigh] != new_chart_2.id
		    )
		) {
		    continue;
		}
		
		double new_value = length(
		    Geom::mesh_facet_center(chart.mesh,top) - 
		    Geom::mesh_facet_center(chart.mesh,neigh)
		) + facet_cmp.distance[top];
		
		if(chart_id[neigh] == index_t(-1)) {
		    facet_cmp.distance[neigh] = new_value;
		    chart_id[neigh] = chart_id[top];
		    queue.insert(neigh);
		} else if(new_value < facet_cmp.distance[neigh]) {
		    queue.erase(neigh);
		    facet_cmp.distance[neigh] = new_value;
		    chart_id[neigh] = chart_id[top];
		    queue.insert(neigh);
		}
	    }
        }

	index_t nb1=0;
	index_t nb2=0;
	for(index_t ff=0; ff<chart.facets.size(); ++ff) {
	    index_t f = chart.facets[ff];
	    if(chart_id[f] == new_chart_1.id) {
		new_chart_1.facets.push_back(f);
		++nb1;
	    } else {
		geo_assert(chart_id[f] == new_chart_2.id);
		new_chart_2.facets.push_back(f);
		++nb2;
	    }
	}

	if(verbose) {
	    Logger::out("Segment")
		<< "new sizes: " << nb1 << " " << nb2 << std::endl;
	}
	
	chart.facets.clear();
    }


    namespace Geom {

	void get_mesh_bbox_2d(
	    const Mesh& mesh, Attribute<double>& tex_coord,
	    double& xmin, double& ymin,
	    double& xmax, double& ymax
	) {
	    xmin = Numeric::max_float64();
	    ymin = Numeric::max_float64();
	    xmax = Numeric::min_float64();
	    ymax = Numeric::min_float64();
	    for(index_t c: mesh.facet_corners) {
		xmin = std::min(xmin, tex_coord[2*c]);
		ymin = std::min(ymin, tex_coord[2*c+1]);
		xmax = std::max(xmax, tex_coord[2*c]);
		ymax = std::max(ymax, tex_coord[2*c+1]);		    
	    }
	}
	
	void get_chart_bbox_2d(
	    const Chart& chart, Attribute<double>& tex_coord,
	    double& xmin, double& ymin,
	    double& xmax, double& ymax
	) {
	    xmin = Numeric::max_float64();
	    ymin = Numeric::max_float64();
	    xmax = Numeric::min_float64();
	    ymax = Numeric::min_float64();
	    for(index_t ff=0; ff<chart.facets.size(); ++ff) {
		index_t f = chart.facets[ff];
		for(index_t c: chart.mesh.facets.corners(f)) {
		    xmin = std::min(xmin, tex_coord[2*c]);
		    ymin = std::min(ymin, tex_coord[2*c+1]);
		    xmax = std::max(xmax, tex_coord[2*c]);
		    ymax = std::max(ymax, tex_coord[2*c+1]);		    
		}
	    }
	}

	double mesh_facet_area_2d(
	    const Mesh& mesh, index_t f, Attribute<double>& tex_coord
	) {
	    double result = 0.0;
	    index_t c1 = mesh.facets.corners_begin(f);
	    vec2 p1(tex_coord[2*c1], tex_coord[2*c1+1]);
	    for(
		index_t c2 = c1+1;
		c2+1 < mesh.facets.corners_end(f); ++c2
	    ) {
		index_t c3 = c2+1;
		vec2 p2(tex_coord[2*c2], tex_coord[2*c2+1]);
		vec2 p3(tex_coord[2*c3], tex_coord[2*c3+1]);
		result += Geom::triangle_area(p1,p2,p3);
	    }
	    return result;
	}
	
	double mesh_area_2d(const Mesh& mesh, Attribute<double>& tex_coord) {
	    double result = 0.0;
	    for(index_t f: mesh.facets) {
		result += mesh_facet_area_2d(mesh, f, tex_coord);
	    }
	    return result;
	}

	
	double chart_area_2d(const Chart& chart, Attribute<double>& tex_coord) {
	    double result = 0.0;
	    for(index_t ff=0; ff<chart.facets.size(); ++ff) {
		index_t f = chart.facets[ff];
		result += mesh_facet_area_2d(chart.mesh, f, tex_coord);
	    }
	    return result;
	}

	double chart_area(const Chart& chart) {
	    double result = 0.0;
	    for(index_t ff=0; ff<chart.facets.size(); ++ff) {
		index_t f = chart.facets[ff];
		result += Geom::mesh_facet_area(chart.mesh, f);
	    }
	    return result;
	}
    }

    
}

