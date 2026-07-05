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

#include <geogram/mesh/mesh_remesh.h>
#include <geogram/mesh/mesh.h>
#include <geogram/mesh/mesh_repair.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/mesh_halfedges.h>
#include <geogram/mesh/mesh_reorder.h>
#include <geogram/mesh/mesh_preprocessing.h>
#include <geogram/mesh/mesh_io.h>
#include <geogram/mesh/mesh_AABB.h>
#include <geogram/voronoi/CVT.h>
#include <geogram/NL/nl.h>
#include <geogram/basic/command_line.h>
#include <geogram/basic/stopwatch.h>
#include <geogram/basic/progress.h>
#include <geogram/bibliography/bibliography.h>

#include <geogram/mesh/mesh_io.h>

/****************************************************************************/

namespace GEO {

    void remesh_smooth(
        Mesh& M_in, Mesh& M_out,
        index_t nb_points,
        coord_index_t dim,
        index_t nb_Lloyd_iter,
        index_t nb_Newton_iter,
        index_t Newton_m,
	bool adjust,
	double adjust_max_edge_distance

    ) {

        geo_cite("DBLP:journals/cgf/YanLLSW09");
        geo_cite("DBLP:conf/imr/LevyB12");
	
        if(dim == 0) {
            dim = coord_index_t(M_in.vertices.dimension());
        }

	geo_argused(dim); 
	
        Stopwatch W("Remesh");

        CentroidalVoronoiTesselation CVT(&M_in);
       
        if(nb_points == 0) {
            nb_points = M_in.vertices.nb();
        }
        CVT.compute_initial_sampling(nb_points, true); // true: for verbose

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

	if(adjust) {
	    mesh_adjust_surface(M_out, M_in, adjust_max_edge_distance);
	}
    }

    /************************************************************************/

    /**
     * \brief Gets the middle segment of a quat
     * \param[in] AABB a reference to a MeshFacetsAABB
     * \param[in] f a facet
     * \param[out] q1 , q2 the extremities of the middle segment
     * \details The quad has vertices p1, p2, p3, p4, on exit
     *  q1 = 1/2(p1+p4) and q2 = 1/2(p2+p3)
     */
    inline void get_quad_middle_segment(
	const MeshFacetsAABB& AABB, index_t f, vec3& q1, vec3& q2
    ) {
	geo_assert(AABB.mesh()->facets.nb_vertices(f) == 4);
	index_t v1 = AABB.mesh()->facets.vertex(f,0);
	index_t v2 = AABB.mesh()->facets.vertex(f,1);
	index_t v3 = AABB.mesh()->facets.vertex(f,2);
	index_t v4 = AABB.mesh()->facets.vertex(f,3);		
	vec3 p1(AABB.mesh()->vertices.point_ptr(v1));
	vec3 p2(AABB.mesh()->vertices.point_ptr(v2));
	vec3 p3(AABB.mesh()->vertices.point_ptr(v3));
	vec3 p4(AABB.mesh()->vertices.point_ptr(v4));
	q1 = 0.5*(p1+p4);
	q2 = 0.5*(p2+p3);
    }
    
    /**
     * \brief Gets the nearest point on a surface along a ray
     * \param[in] AABB the facets of the surface 
     *  as a MeshFacetsAABB
     * \param[in] R1 the ray, both directions are tested to find
     *  the nearest point
     * \param[in] max_dist if the nearest point is further away
     *  than \p max_dist, then the origin of the ray is returned
     * \param[in] ribbon_mode if set, project on middle segment 
     *  of quad
     */
    inline vec3 nearest_along_bidirectional_ray(
	const MeshFacetsAABB& AABB, const Ray& R1,
	double max_dist, bool ribbon_mode=false
    ) {
	vec3 p = R1.origin;
	vec3 result = p;
	Ray R2(R1.origin, -R1.direction);
	MeshFacetsAABB::Intersection I1;
	MeshFacetsAABB::Intersection I2;
	bool has_I1 = AABB.ray_nearest_intersection(R1,I1);
	bool has_I2 = AABB.ray_nearest_intersection(R2,I2);

	if(has_I1 && ribbon_mode) {
	    vec3 q,q1,q2;
	    double l1,l2;
	    get_quad_middle_segment(AABB, I1.f, q1, q2);
	    Geom::point_segment_squared_distance(I1.p, q1, q2, q, l1, l2);
	    I1.p=q;
	}
	    
	if(has_I2 && ribbon_mode) {
	    vec3 q,q1,q2;
	    double l1,l2;
	    get_quad_middle_segment(AABB, I2.f, q1, q2);
	    Geom::point_segment_squared_distance(I2.p, q1, q2, q, l1, l2);
	    I2.p=q;
	}
	
	if(has_I1 && !has_I2) {
	    result = I1.p;
	}
	if(!has_I1 && has_I2) {
	    result = I2.p;
	}
	if(has_I1 && has_I2) {
	    if(Geom::distance2(p,I1.p) < Geom::distance2(p,I2.p)) {
		result = I1.p;
	    } else {
		result = I2.p;
	    }
	}
	if(Geom::distance2(result,p) > max_dist*max_dist) {
	    result = p;
	}
	return result;
    }


    /**
     * \brief Creates a ribbon orthogonal to the border of a
     *  surface mesh.
     * \param[in] M the input surface mesh
     * \param[out] ribbon the generated ribbon
     * \param[in] height the height of the ribbon
     */
    static void create_ribbon_on_border(
	const Mesh& M,
	Mesh& ribbon,
	double height
    ) {
	index_t nb_border_edges=0;
	vector<vec3> Nv(M.vertices.nb(), vec3(0.0, 0.0, 0.0));
        for(index_t f: M.facets) {
            vec3 N = Geom::mesh_facet_normal(M, f);
            for(index_t c1: M.facets.corners(f)) {
                if(M.facet_corners.adjacent_facet(c1) == NO_FACET) {
                    index_t c2 = M.facets.next_corner_around_facet(f, c1);
                    index_t v1 = M.facet_corners.vertex(c1);
                    index_t v2 = M.facet_corners.vertex(c2);
                    Nv[v1] += N;
                    Nv[v2] += N;
		    ++nb_border_edges;
                }
            }
        }

	ribbon.vertices.set_dimension(3);
	ribbon.vertices.create_vertices(4*nb_border_edges);
	ribbon.facets.create_quads(nb_border_edges);
	
	index_t cur_border_e = 0;
        for(index_t f: M.facets) {
            for(index_t c1: M.facets.corners(f)) {
                if(M.facet_corners.adjacent_facet(c1) == NO_FACET) {
                    index_t c2 = M.facets.next_corner_around_facet(f, c1);
                    index_t v1 = M.facet_corners.vertex(c1);
                    index_t v2 = M.facet_corners.vertex(c2);
                    const vec3& p1 = Geom::mesh_vertex(M, v1);
                    const vec3& p2 = Geom::mesh_vertex(M, v2);

		    vec3 U1 = 0.5*height * normalize(Nv[v1]);
		    vec3 U2 = 0.5*height * normalize(Nv[v2]);		    
		    
		    vec3 q1 = p1 + U1;
		    vec3 q2 = p2 + U2;
		    vec3 q3 = p2 - U2;
		    vec3 q4 = p1 - U1;
		    
		    for(index_t c=0; c<3; ++c) {
			ribbon.vertices.point_ptr(4*cur_border_e  )[c] = q1[c];
			ribbon.vertices.point_ptr(4*cur_border_e+1)[c] = q2[c];
			ribbon.vertices.point_ptr(4*cur_border_e+2)[c] = q3[c];
			ribbon.vertices.point_ptr(4*cur_border_e+3)[c] = q4[c];
		    }

		    ribbon.facets.set_vertex(cur_border_e, 0, 4*cur_border_e  );
		    ribbon.facets.set_vertex(cur_border_e, 1, 4*cur_border_e+1);
		    ribbon.facets.set_vertex(cur_border_e, 2, 4*cur_border_e+2);
		    ribbon.facets.set_vertex(cur_border_e, 3, 4*cur_border_e+3);
		    ++cur_border_e;
		}
	    }
	}
    }
    
    /************************************************************************/  

    void GEOGRAM_API mesh_adjust_surface(
	Mesh& surface,
	Mesh& reference,
	double max_edge_distance,
	bool project_borders
    ) {
	// The algorithm:
	// 1) For each surface vertex v (located at Pv) with normal Nv,
	//    we determine a "target point" Qv as the intersection between
	//    the ray R(Pv,Nv) and the reference surface.
	// 2) For each facet f with center point Pf, we determine a "target
	//    point" Qf that corresponds to the intersection between the ray
	//    R(Pf, Nf) and the reference surface, where Nf is the sum of
	//    the directions Nv associated with the vertices of the facet.
	// 3) We optimize for the lambda's in the relations that correspond to
	//    1) and 2), that is, the sum of the squared distances
	//    || P + lambda N - Q ||^2 for each v, for each f
	// 4) For each v, Pv = Pv + lambda_v Nv
	//
	// For surfaces with borders, there is a subtlety: instead of using
	// the normal to the surface, we use for Nv a direction tangent to
	// the surface and normal to the border of the surface, and to find
	// the reference point Qv, we construct a "ribbon", obtained by
	// sweeping a segment normal to the reference surface along the
	// border of the reference surface. The point Qv is obtained by
	//   - compute the intersection I between the ray R(Pv, Nv) and the
	//     ribbon. This intersection is in a quad supported by a segment
	//     [q1,q2] on the border of the reference surface
	//   - Qv is determined as the nearest point to I on [q1,q2]
	//
	// For each border edge e=(i,j), the center point Pe=0.5(Pi+Pj), the
	// direction Ne = 0.5(Ni+Nj), a point Qe is determined (using the same
	// algorithm as in the previous point) and a least-squares term
	// || Pe - Qe ||^2 is added to the quantity to be minimized.
	//
	// An optional final step (brutally) assigns Pv = Qv for each border
	// vertex (but in general it gives a worse result on the facets
	// adjacent to the border, so the option project_borders is deactivated
	// by default).
	
	// Relative importance of border accuracy
	// (weights least squares terms)
	const double border_importance = 2.0;

	// Geometric search uses larger maximum
	// distance for points on the border.
	const double border_distance_factor = 10.0;
	
	MeshFacetsAABB AABB(reference);

        // For each vertex, direction along which the neighbor
	// on the surface is searched (or the neighbor on the
	// ribbon if it is a vertex on the border)
	vector<vec3> Nv(surface.vertices.nb(), vec3(0.0, 0.0, 0.0));

	// average edge length incident to vertex
	vector<double> Lv(surface.vertices.nb(), 0.0);

	// number of edges incident to a vertex
	vector<index_t> Cv(surface.vertices.nb(), 0);

	// Compute directions Nv for inner vertices
	// (use surface normal)
	for(index_t f: surface.facets) {
	    vec3 n = Geom::mesh_facet_normal(surface, f);
	    index_t d = surface.facets.nb_vertices(f);
	    for(index_t lv=0;lv<d;++lv) {
		index_t v1 = surface.facets.vertex(f,lv);
		index_t v2 = surface.facets.vertex(
		    f,(lv==d-1)?0:lv+1
		);
		Nv[v1] += n;
		double l = Geom::distance2(
		    vec3(surface.vertices.point_ptr(v1)),
		    vec3(surface.vertices.point_ptr(v2))	    
		);
		l = ::sqrt(l);
		Lv[v1] += l;
		Lv[v2] += l;
		Cv[v1]++;
		Cv[v2]++;
	    }
	}

	// Normalize average incident edge length
	for(index_t v: surface.vertices) {
	    if(Cv[v] != 0) {
		Lv[v] /= double(Cv[v]);
	    }
	}


	// Compute Nv for vertices on the border: the used vector
	// is normal to the border (tangent to the surface), because
	// we will find nearest neighbor from the "ribbon" mesh.
	index_t nb_v_on_border = 0; 
	vector<bool> v_on_border(surface.vertices.nb(), false);
        for(index_t f: surface.facets) {
            for(index_t c1: surface.facets.corners(f)) {
                if(surface.facet_corners.adjacent_facet(c1) == NO_FACET) {
                    index_t v = surface.facet_corners.vertex(c1);
		    v_on_border[v] = true;
		    ++nb_v_on_border;
		    Nv[v] = vec3(0.0, 0.0, 0.0);
                }
            }
        }

        for(index_t f: surface.facets) {
            vec3 N = Geom::mesh_facet_normal(surface, f);
            for(index_t c1: surface.facets.corners(f)) {
                if(surface.facet_corners.adjacent_facet(c1) == NO_FACET) {
                    index_t c2 = surface.facets.next_corner_around_facet(f, c1);
                    index_t v1 = surface.facet_corners.vertex(c1);
                    index_t v2 = surface.facet_corners.vertex(c2);
                    const vec3& p1 = Geom::mesh_vertex(surface, v1);
                    const vec3& p2 = Geom::mesh_vertex(surface, v2);
                    vec3 Ne = cross(p2 - p1, N);
                    Nv[v1] += Ne;
                    Nv[v2] += Ne;
                }
            }
        }

	bool reference_has_borders = false;
	for(index_t c: reference.facet_corners) {
	    if(reference.facet_corners.adjacent_facet(c) == NO_FACET) {
		reference_has_borders = true;
		break;
	    }
	}

	// Create ribbon and AABB for the ribbon if the reference surface
	// has vertices on the border
	Mesh border_ribbon;
	MeshFacetsAABB border_ribbon_AABB;
	if(nb_v_on_border != 0 && reference_has_borders) {
	    create_ribbon_on_border(
		reference, border_ribbon,
		max_edge_distance*4.0*surface_average_edge_length(surface)
	    );
	    border_ribbon_AABB.initialize(border_ribbon);
	}
	
	// nearest point along Nv
	vector<vec3> Qv(surface.vertices.nb());
	for(index_t v: surface.vertices) {
	    vec3 p(surface.vertices.point_ptr(v));
	    if(v_on_border[v] && reference_has_borders) {
		Qv[v] = nearest_along_bidirectional_ray(
		    border_ribbon_AABB, Ray(p, Nv[v]),
		    border_distance_factor*max_edge_distance*Lv[v],
		    true
		);
	    } else {
		Qv[v] = nearest_along_bidirectional_ray(
		    AABB, Ray(p, Nv[v]),max_edge_distance*Lv[v]
		);
	    }
	}
    
	nlNewContext();
	nlSolverParameteri(NL_LEAST_SQUARES, NL_TRUE);	    
	nlSolverParameteri(
	    NL_NB_VARIABLES, NLint(surface.vertices.nb())
	);


	nlBegin(NL_SYSTEM);
	nlBegin(NL_MATRIX);

	// For each vertex v, least squares constraint that
	// makes the vertex "attracted" by Qv[v]
	for(index_t v: surface.vertices) {
	    if(v_on_border[v]) {
		nlRowScaling(border_importance);
	    }

	    // p + lambda_v * Nv = q ---> lambda_v * Nv = q - v
	    for(index_t c=0; c<3; ++c) {
		nlBegin(NL_ROW);
		nlCoefficient(v,Nv[v][c]);
		nlRightHandSide(Qv[v][c] - surface.vertices.point_ptr(v)[c]);
		nlEnd(NL_ROW);
	    }
	}

	// For each facet f, least squares constraint that
	// makes the center of the facet "attracted" by the
	// nearest point along the averaged directions of
	// all vertices of the facet
	for(index_t f: surface.facets) {
	    index_t d = surface.facets.nb_vertices(f);
	    
	    vec3 Nf(0.0, 0.0, 0.0);
	    vec3 Pf;
	    double Lf=0.0; 
	    
	    for(index_t lv=0; lv<d; ++lv) {
		index_t v= surface.facets.vertex(f,lv);
		Nf += Nv[v];
		Pf += vec3(surface.vertices.point_ptr(v));
		Lf += Lv[v];
	    }
	    Pf = (1.0 / double(d))*Pf;
	    Lf = (1.0 / double(d))*Lf;
	    vec3 Qf = nearest_along_bidirectional_ray(
		AABB, Ray(Pf, Nf), max_edge_distance*Lf
	    );
	    
	    // 1/d(Sum Pv + lambda_v Nv) = Qf
	    // --> Pf + 1/d(Sum lambda_v Nv) = Qf
	    // --> Sum (1/d lambda_v Nv) = Qf - Pf
	    for(index_t c=0; c<3; ++c) {
		nlBegin(NL_ROW);
		for(index_t lv=0; lv<d; ++lv) {
		    index_t v = surface.facets.vertex(f,lv);
		    nlCoefficient(v,Nv[v][c]/double(d));
		}
		nlRightHandSide(Qf[c]-Pf[c]);
		nlEnd(NL_ROW);
	    }
	}

	// For each edge on the border,
	// makes the center of the edge attracted by the
	// nearest point on the ribbon along the averated directions
	// of the two vertices of the edge
	if(nb_v_on_border != 0 && reference_has_borders) {
	    for(index_t f: surface.facets) {
		for(index_t c1: surface.facets.corners(f)) {
		    if(surface.facet_corners.adjacent_facet(c1) == NO_FACET) {
			index_t c2 = surface.facets.next_corner_around_facet(
			    f, c1
			);
			index_t v1 = surface.facet_corners.vertex(c1);
			index_t v2 = surface.facet_corners.vertex(c2);
			const vec3& p1 = Geom::mesh_vertex(surface, v1);
			const vec3& p2 = Geom::mesh_vertex(surface, v2);
			vec3 p = 0.5*(p1+p2);
			vec3 N = 0.5*(Nv[v1] + Nv[v2]);
			vec3 q = nearest_along_bidirectional_ray(
			    border_ribbon_AABB, Ray(p, N),
			    border_distance_factor*max_edge_distance*
			                          0.5*(Lv[v1]+Lv[v2]),
			    true
			);

			// 1/2(p1 + p2 + lambda_1 N1 + lambda_2 N2) = q
			// --> p + 1/2(lambda_1 N1 + lambda_2 N2) = q
			// -->  (1/2 lambda_1 N1 + 1/2 lambda_2 N2) = q - p
			for(index_t c=0; c<3; ++c) {
			    nlRowScaling(0.5*border_importance);
			    nlBegin(NL_ROW);
			    nlCoefficient(v1,0.5*N[c]);
			    nlCoefficient(v2,0.5*N[c]);
			    nlRightHandSide(q[c]-p[c]);
			    nlEnd(NL_ROW);
			}
			
		    }
                }
	    }
	}
	
	nlEnd(NL_MATRIX);
	nlEnd(NL_SYSTEM);
	
	nlSolve();

	// Displace each vertex v along Nv[v] by
	// the solution of the least squares problem
	// at v
	for(index_t v: surface.vertices) {
	    vec3 p(surface.vertices.point_ptr(v));
	    p = p + nlGetVariable(v)*Nv[v];
	    for(index_t c=0; c<3; ++c) {
		surface.vertices.point_ptr(v)[c] = p[c];
	    }
	}

	// (Brutally) project border vertices
	if(project_borders && nb_v_on_border != 0 && reference_has_borders) {
	    for(index_t v: surface.vertices) {
		if(v_on_border[v]) {
		    vec3 p(surface.vertices.point_ptr(v));
		    vec3 q = nearest_along_bidirectional_ray(
			border_ribbon_AABB, Ray(p, Nv[v]),
			border_distance_factor*max_edge_distance*0.5*(Lv[v]),
			true
		    );
		    for(index_t c=0; c<3; ++c) {
			surface.vertices.point_ptr(v)[c] = q[c];
		    }
		}
	    }
	}
	
	nlDeleteContext(nlGetCurrent());
    }
}

