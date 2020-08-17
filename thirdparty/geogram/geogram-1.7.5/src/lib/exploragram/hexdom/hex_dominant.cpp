#include <exploragram/hexdom/hex_dominant.h>
#include <exploragram/hexdom/PGP.h>
#include <exploragram/hexdom/basic.h>
#include <exploragram/hexdom/extra_connectivity.h>
#include <geogram/numerics/matrix_util.h>
#include <geogram/basic/permutation.h>
#include <algorithm>
#include <geogram/mesh/triangle_intersection.h>
#include <geogram/mesh/mesh_tetrahedralize.h>
#include <geogram/delaunay/delaunay.h>

#include <exploragram/hexdom/hex_cruncher.h>


#include <geogram/points/nn_search.h>
#include <geogram/points/colocate.h>
#include <queue>

#include <exploragram/hexdom/mesh_inspector.h>
#include <exploragram/hexdom/intersect_tools.h>
#include <exploragram/hexdom/polygon.h>
#include <exploragram/hexdom/frame.h>
#define FPG_UNCERTAIN_VALUE 0
#include <geogram/numerics/predicates/orient3d.h>
namespace GEO {

	static void fill_quad_tri_surface(Mesh* m, bool with_pyramids) {
		if (m->vertices.nb() == 0)  return;
		m->edges.clear();

		if (with_pyramids) {

			vector<index_t> pyrindex;
			// Remark: spliting quads in 2 may be better on boundary... but it is not clear for constrained boundary... 
			vector<index_t> to_kill(m->facets.nb(), 0);

			index_t init_nb_facets = m->facets.nb();
			FOR(f, init_nb_facets) {
				if (m->facets.nb_vertices(f) != 4) continue;

				index_t nvv = m->vertices.create_vertex();
				vec3 n = facet_normal(m, f);
				double d = 0;
				FOR(e, 4) d += .25*(X(m)[m->facets.vertex(f, e)] - X(m)[m->facets.vertex(f, (e + 1) % 4)]).length();
				X(m)[nvv] = facet_bary(m, f) +.2*d*n;
				to_kill[f] = 1;

				index_t off_f = m->facets.create_triangles(4);
				FOR(e, 4) {
					to_kill.push_back(0);
					m->facets.set_vertex(off_f + e, 0, m->facets.vertex(f, e));
					m->facets.set_vertex(off_f + e, 1, m->facets.vertex(f, (e + 1) % 4));
					m->facets.set_vertex(off_f + e, 2, nvv);
				}
				FOR(e, 4) pyrindex.push_back(m->facets.vertex(f, e));
				pyrindex.push_back(nvv);
			}
			m->facets.delete_elements(to_kill, false);

			m->facets.triangulate();
			create_non_manifold_facet_adjacence(m);
			try {
				mesh_tetrahedralize(*m, false, true, 1.);
				index_t off_c = m->cells.create_pyramids(pyrindex.size() / 5);
				FOR(p, pyrindex.size() / 5) FOR(lv, 5)
					m->cells.set_vertex(off_c + p, lv, pyrindex[5 * p + lv]);
			}
			catch (const Delaunay::InvalidInput& error_report) {
				FOR(i, error_report.invalid_facets.size())
					plop(error_report.invalid_facets[i]);
			}

		}
		else {// !with_pyramids
			m->facets.triangulate();
			create_non_manifold_facet_adjacence(m);
			try {
				mesh_tetrahedralize(*m, false, true, 1.);
			}
			catch (const Delaunay::InvalidInput& error_report) {
				FOR(i, error_report.invalid_facets.size())
					plop(error_report.invalid_facets[i]);
			}

		}
	}

	void fill_cavity_with_tetgen(Mesh* input, Mesh* tri, bool with_pyramid) {
		tri->copy(*input, false);
		fill_quad_tri_surface(tri, with_pyramid);
	}


	void add_hexes_to_tetmesh(Mesh* hex, Mesh* tet_mesh) {
		if (hex->cells.nb() == 0) return;


		index_t off_v = tet_mesh->vertices.create_vertices(hex->vertices.nb());
		FOR(v, hex->vertices.nb()) X(tet_mesh)[off_v + v] = X(hex)[v];
		index_t off_c = tet_mesh->cells.create_hexes(hex->cells.nb());
		FOR(c, hex->cells.nb())FOR(cv, 8) {
			index_t vid = hex->cells.vertex(c, cv)+off_v;
			tet_mesh->cells.set_vertex(off_c + c, cv, vid);
		}
		// merge vertices
		double eps = (1e-3)*get_cell_average_edge_size(tet_mesh);
		{
			vector<index_t> to_kill(tet_mesh->vertices.nb(), 0);
			vector<index_t> old2new(tet_mesh->vertices.nb());
			Geom::colocate(tet_mesh->vertices.point_ptr(0), 3, tet_mesh->vertices.nb(), old2new, eps);
			FOR(c, tet_mesh->cells.nb()) FOR(cv, tet_mesh->cells.nb_vertices(c))
				tet_mesh->cells.set_vertex(c, cv, old2new[tet_mesh->cells.vertex(c, cv)]);
			FOR(v, tet_mesh->vertices.nb()) if (old2new[v] != v) to_kill[v] = NOT_AN_ID;
			tet_mesh->vertices.delete_elements(to_kill);
		}


	}

       /*
	*     _____                _             ____                  _       _       
	*    / ____|              (_)           |  _ \                | |     (_)      
	*   | |     __ _ _ __ _ __ _  ___ _ __  | |_) | __ _ _   _  __| | ___  _ _ __  
	*   | |    / _` | '__| '__| |/ _ \ '__| |  _ < / _` | | | |/ _` |/ _ \| | '_ \  
	*   | |___| (_| | |  | |  | |  __/ |    | |_) | (_| | |_| | (_| | (_) | | | | |
	*    \_____\__,_|_|  |_|  |_|\___|_|    |____/ \__,_|\__,_|\__,_|\___/|_|_| |_|
	*/                                                                             



        static bool in_volume(Mesh* surface, vec3 request) {
	    int accum = 0;
	    vec2 R(request[0], request[1]);
	    FOR(f, surface->facets.nb()) {
	        FOR(fan, surface->facets.nb_vertices(f) - 2) {
	            index_t v[3] = {
	                surface->facets.vertex(f,0),
	                surface->facets.vertex(f,1 + fan),
	                surface->facets.vertex(f,2 + fan)
	            };
	            vec2 P[3];
	            FOR(vid, 3) P[vid] = vec2(X(surface)[v[vid]][0], X(surface)[v[vid]][1]) - R;
	            bool in_triangle = true;
	            double tr_orient = det(P[1] - P[0], P[2] - P[0]);
	            if (tr_orient > 0)tr_orient = 1; else tr_orient = -1;
	            FOR(vid, 3) in_triangle = in_triangle && ((det(P[vid], P[(vid + 1) % 3]) > 0) == (tr_orient >0)); ;
	            if (!in_triangle) continue;

	            int sign = orient_3d_filter(X(surface)[v[0]].data(), X(surface)[v[1]].data(), X(surface)[v[2]].data(), request.data());
	            if (sign == 0) return false; // I don't want to deal with degenerate case
	            accum += sign;
	        }
	    }
	    return accum != 0;

	}

	struct GrowPt {
	    GrowPt(vec3 p_pos, mat3 p_r) { pos = p_pos; r = p_r; }
	    vec3 pos;
		mat3 r;
	};

	void Baudoin_mesher(Mesh* m) {
		geo_argused(m);
		//return;
		// init rot
		Attribute<mat3> B(m->vertices.attributes(), "B");
		{
		    vector<vector<vec3> >  dir(m->vertices.nb());
			FOR(f, m->facets.nb()) {
				if (m->facets.nb_vertices(f) != 4) continue;
				FOR(e, 4)
					dir[m->facets.vertex(f, e)].push_back(normalize(X(m)[m->facets.vertex(f, e)] - X(m)[m->facets.vertex(f, (e + 1) % 4)]));
			}
		    FOR(f, m->facets.nb()) {
		        if (m->facets.nb_vertices(f)!=4) continue;
		        FOR(e, 4) dir[m->facets.vertex(f, e)].clear();
		    }
		    FOR(v, m->vertices.nb())
		        if (dir[v].empty()) B[v].load_identity();
		        else B[v] = Frame::representative_frame(dir[v]);
		}

		// compute ave QUAD edge length
		double ave = 0;
		double nb = 0;
		FOR(f, m->facets.nb()) {
		    if (m->facets.nb_vertices(f) != 4) continue;
		    FOR(e, 4) {
		        ave += (X(m)[m->facets.vertex(f, e)] - X(m)[m->facets.vertex(f, (e + 1) % 4)]).length();
		        nb += 1.;
		    }

		}
		ave /= nb;



		vector<vec3> nvvertices;

		vector<GrowPt> g;
		index_t cur = 0;
		// add useless boundary vertices (to prevent intersections)
		FOR(v, m->vertices.nb()) if (B[v].is_identity()) g.push_back(GrowPt(X(m)[v], B[v]));
		cur = g.size();
		FOR(v, m->vertices.nb()) if (!B[v].is_identity()) g.push_back(GrowPt(X(m)[v], B[v]));


		// original paper
		int nb_max_pts = 3000;
		while (cur < g.size() && nb_max_pts-->0) {
		    FOR(d, 3) FOR(s, 2) {
		        vec3 cand = ave * col(g[cur].r,d);
		        if (s > 0) cand *= -1;
		        cand = cand + g[cur].pos;
		        bool fail = false;
		        FOR(i, g.size())
		            if ((g[i].pos - cand).length2() < pow(.5*ave, 2.))
		                fail = true;
		        if (!fail && in_volume(m, cand)) {
		            g.push_back(GrowPt(cand, g[cur].r));
		            nvvertices.push_back(cand);
		        }
		    }
		    cur++;
		}

		index_t off_v = m->vertices.create_vertices(nvvertices.size());
		FOR(i, nvvertices.size())  X(m)[off_v + i] = nvvertices[i];

	}

	
        static void stuff_with_tets_and_pyramids(Mesh* m) {
		if (m->vertices.nb() == 0 || m->facets.nb()==0)  return;
		m->edges.clear();
		check_no_intersecting_faces(m);
		double ave_edge_length = get_facet_average_edge_size(m);

		vector<index_t> pyrindex;
		vector<vec3> pyr_Z;
		vector<index_t> pyr_top_index;

		{// split quads into 4 triangles... and remember them to produce pyramids
			// Remark: spliting quads in 2 may be better on boundary... but it is not clear for constrained boundary... 
			vector<index_t> to_kill(m->facets.nb(), 0);


			index_t init_nb_facets = m->facets.nb();
			FOR(f, init_nb_facets) {
				if (m->facets.nb_vertices(f) != 4) {
					geo_assert(m->facets.nb_vertices(f)==3);
					continue;
				}
				index_t nvv = m->vertices.create_vertex();
				pyr_Z.push_back(facet_normal(m, f));

				double d = 0;
				FOR(e, 4) d += .25*(X(m)[m->facets.vertex(f, e)] - X(m)[m->facets.vertex(f, (e + 1) % 4)]).length();
				X(m)[nvv] = facet_bary(m, f);// +.2*d*n;
				to_kill[f] = 1;

				index_t off_f = m->facets.create_triangles(4);
				FOR(e, 4) {
					to_kill.push_back(0);
					m->facets.set_vertex(off_f + e, 0, m->facets.vertex(f, e));
					m->facets.set_vertex(off_f + e, 1, m->facets.vertex(f, (e + 1) % 4));
					m->facets.set_vertex(off_f + e, 2, nvv);
				}
				FOR(e, 4) pyrindex.push_back(m->facets.vertex(f, e));
				pyrindex.push_back(nvv);
				pyr_top_index.push_back(nvv);
			}
			m->facets.delete_elements(to_kill, false);
		}

		{// mode the tip of pyramid in the normal direction to produce better shape (not flat)
			check_no_intersecting_faces(m);
			vector<double> max_pyr_top_coeff(pyr_top_index.size(), .5);
			bool done = false;
			while (!done){
					Mesh copy;
					copy.copy(*m);
					create_non_manifold_facet_adjacence(&copy);

					FOR(v, pyr_top_index.size())
						X(&copy)[pyr_top_index[v]] = X(m)[pyr_top_index[v]]
						- max_pyr_top_coeff[v]*ave_edge_length * pyr_Z[v];

					vector<index_t> intersections = get_intersecting_faces(&copy);
					done = (intersections.size() == 0);
					vector<bool> vertexpb(copy.vertices.nb(), false);
					FOR(i, intersections.size())  FOR(lv, 3) vertexpb[copy.facets.vertex(intersections[i], lv)] = true;

					FOR(v, pyr_top_index.size()) if (vertexpb[pyr_top_index[v]]) {
						if (max_pyr_top_coeff[v] > .02)max_pyr_top_coeff[v] /= 2.;
						else max_pyr_top_coeff[v] = 0;
					}
			}

			FOR(v, pyr_top_index.size()) X(m)[pyr_top_index[v]]
			    -= std::max(0.0, .7*max_pyr_top_coeff[v])*ave_edge_length * pyr_Z[v];

			check_no_intersecting_faces(m);
		}
		// keep vertices geometry (indices are broken by the tetrahedrisation)
		vector<vec3> pyrpos(pyrindex.size());
		FOR(nv, pyrindex.size()) pyrpos[nv] = X(m)[pyrindex[nv]];

		// tetrahedrize inside
		try {
			FOR(f, m->facets.nb()) geo_assert(m->facets.nb_vertices(f)==3);
			
			mesh_save(*m, "C:/DATA/debug/pretriangulate.geogram");
			m->facets.triangulate();
			mesh_save(*m, "C:/DATA/debug/posttrinagulate.geogram");
			create_non_manifold_facet_adjacence(m);
			mesh_tetrahedralize(*m, false, true, 1.);
		}
		catch (const Delaunay::InvalidInput& error_report) {
			FOR(i, error_report.invalid_facets.size())
				plop(error_report.invalid_facets[i]);
			Attribute<int> intersection(m->facets.attributes(), "intersection");
			FOR(f, m->facets.nb()) intersection[f] = 0;
			FOR(i, error_report.invalid_facets.size())
				intersection[error_report.invalid_facets[i]] = 1;
			mesh_save(*m, "C:/DATA/debug/intersectingsurface2.geogram");
			geo_assert_not_reached;
		}
		
		// restore indices from geometry
		NearestNeighborSearch_var NN = NearestNeighborSearch::create(3);
		NN->set_points(m->vertices.nb(), m->vertices.point_ptr(0), 3);
		FOR(nv, pyrindex.size()) pyrindex[nv] = NN->get_nearest_neighbor(pyrpos[nv].data());


		// produce pyramids
		index_t off_c = m->cells.create_pyramids(pyrindex.size() / 5);
		FOR(p, pyrindex.size() / 5) FOR(lv, 5)
			m->cells.set_vertex(off_c + p, lv, pyrindex[5 * p + lv]);

	}



	void hex_dominant(Mesh* cavity, Mesh* hexahedrons, Mesh* result) {
		plop("HexDominant with vertex_puncher and pyramids");
		hex_crunch(cavity, hexahedrons);

		//return;
		Mesh tets;
		tets.copy(*cavity);
		stuff_with_tets_and_pyramids(&tets);
		result->copy(tets);
		result->facets.clear();
		


		index_t off_c = result->cells.create_hexes(hexahedrons->cells.nb());
		index_t off_v = result->vertices.create_vertices(hexahedrons->vertices.nb());
		FOR(v, hexahedrons->vertices.nb()) X(result)[off_v + v] = X(hexahedrons)[v];
		FOR(c, hexahedrons->cells.nb())FOR(cv, 8) {
			result->cells.set_vertex(off_c + c, cv, off_v + hexahedrons->cells.vertex(c, cv));
		}


		// merge vertices
		double eps = (1e-3)*get_cell_average_edge_size(result);
		{
			vector<index_t> to_kill(result->vertices.nb(), 0);
			vector<index_t> old2new(result->vertices.nb());
			Geom::colocate(result->vertices.point_ptr(0), 3, result->vertices.nb(), old2new, eps);
			FOR(c, result->cells.nb()) FOR(cv, result->cells.nb_vertices(c))
				result->cells.set_vertex(c, cv, old2new[result->cells.vertex(c, cv)]);
			FOR(v, result->vertices.nb()) if (old2new[v] != v) to_kill[v] = NOT_AN_ID;
			result->vertices.delete_elements(to_kill);
		}

		result->cells.connect();
		result->facets.clear();
	//	result->cells.compute_borders();
		return;
	}

}
