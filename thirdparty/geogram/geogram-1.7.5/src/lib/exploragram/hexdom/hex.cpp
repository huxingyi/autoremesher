#include <exploragram/hexdom/hex.h>
#include <exploragram/hexdom/PGP.h>
#include <exploragram/hexdom/basic.h>
#include <exploragram/hexdom/extra_connectivity.h>
#include <geogram/numerics/matrix_util.h>
#include <geogram/basic/permutation.h>
#include <algorithm>
#include <geogram/mesh/triangle_intersection.h>
#include <geogram/mesh/mesh_tetrahedralize.h>
#include <geogram/delaunay/delaunay.h>

#include <geogram/points/nn_search.h>
#include <geogram/points/colocate.h>
#include <queue>

#include <exploragram/hexdom/mesh_inspector.h>
#include <exploragram/hexdom/intersect_tools.h>
#include <exploragram/hexdom/polygon.h>
#include <exploragram/hexdom/time_log.h>
namespace GEO {



	static void snap_vertices_to_ref_vertices(Mesh* m, Mesh* ref, double eps = 1e-10) {
		if (ref->vertices.nb() == 0) return;
		NearestNeighborSearch_var NN = NearestNeighborSearch::create(3, "default");
		NN->set_points(ref->vertices.nb(), (double*)ref->vertices.point_ptr(0));

		FOR(v, m->vertices.nb()) {
			index_t v_ref = NN->get_nearest_neighbor((double*)&(X(m)[v]));
			if ((X(m)[v] - X(ref)[v_ref]).length() < eps)
				X(m)[v] = X(ref)[v_ref];
		}
	}






	void kill_intersecting_hexes(Mesh* hex) {
		hex->cells.connect();
		index_t nb_intersecting_hex = 0;
		vector<index_t> to_kill(hex->cells.nb(), false);
		
		// alternativeway of finding intersections
		
		Mesh quadset;
		Attribute<index_t> cell_id(quadset.facets.attributes(), "cellid");
		quadset.vertices.create_vertices(hex->vertices.nb());
		FOR(v, hex->vertices.nb()) X(&quadset)[v] = X(hex)[v];
		quadset.facets.create_quads(6 * hex->cells.nb());
		FOR(f, 6 * hex->cells.nb()) cell_id[f] = f / 6;
		FOR(c, hex->cells.nb()) FOR(lf, 6) FOR(lv, 4)  quadset.facets.set_vertex(6 * c+lf, lv, hex->cells.facet_vertex(c,lf,lv));
		
		// kill adjacency faces
		vector<index_t> to_kill_f(quadset.facets.nb(), false);
		FOR(c, hex->cells.nb()) FOR(lf, 6) if (hex->cells.adjacent(c, lf) != NOT_AN_ID) to_kill_f[6 * c + lf] = true;
		quadset.facets.delete_elements(to_kill_f);

		//plop("gre");


		//mesh_save(quadset, "C:\\DATA\\quadeset.meshb");
		vector<index_t> intersections = get_intersecting_faces(&quadset);
		plop(intersections.size());
		nb_intersecting_hex = intersections.size();
		FOR(i, intersections.size()) to_kill[cell_id[intersections[i] ]] = true;
		FOR(i, intersections.size()) plop(intersections[i]);



		//vector<BBox> inboxes(6 * hex->cells.nb());
		//FOR(f, inboxes.size())FOR(cfv, 4)
		//	inboxes[f].add(X(hex)[hex->cells.facet_vertex(f / 6, f % 6, cfv)]);
		//HBoxes hb(inboxes);

		//FOR(c, hex->cells.nb()) FOR(cf, 6) {
		//	if (hex->cells.adjacent(c, cf) != NOT_AN_ID) continue; // just check on boundary
		//	BBox b;
		//	vector<vec3> Q(4);
		//	FOR(cfv, 4) {
		//		Q[cfv] = X(hex)[hex->cells.facet_vertex(c, cf, cfv)];
		//		b.add(Q[cfv]);
		//	}
		//	vector<index_t> primitives;
		//	hb.intersect(b, primitives);

		//	FOR(i, primitives.size()) {
		//		index_t other_c = primitives[i] / 6;
		//		if (other_c == c) continue;
		//		if (to_kill[other_c]) continue;
		//		index_t other_cf = primitives[i] % 6;
		//		if (hex->cells.adjacent(other_c, other_cf) != NOT_AN_ID) continue; // just check on boundary
		//		vector<vec3> P(4);
		//		FOR(other_cfv, 4) P[other_cfv] = X(hex)[hex->cells.facet_vertex(other_c, other_cf, other_cfv)];

		//		// check if opposite
		//		bool is_opp = false;
		//		FOR(v, 4) {
		//			index_t it = 0;
		//			while (it < 4 && (Q[it] - P[(v + it) % 4]).length2() == 0) it++;
		//			is_opp = is_opp || (it == 4);
		//		}

		//		if (is_opp) continue;

		//		// check intersection
		//		vector<TriangleIsect> result;
		//		FOR(c0, 2)FOR(c1, 2) {
		//			if (triangles_intersections(
		//				P[quad_rand_split[c0][0]], P[quad_rand_split[c0][1]], P[quad_rand_split[c0][2]],
		//				Q[quad_rand_split[c1][0]], Q[quad_rand_split[c1][1]], Q[quad_rand_split[c1][2]],
		//				result
		//			)) {
		//				plop("auto intersection found");
		//				to_kill[c] = true;
		//				nb_intersecting_hex++;
		//			}
		//		}
		//	}
		//}
		logt.add_value("nb_intersecting_hex", nb_intersecting_hex);
		plop(nb_intersecting_hex);
		hex->cells.delete_elements(to_kill);
		if (nb_intersecting_hex > 0) kill_intersecting_hexes(hex);
	}


	void hex_set_2_hex_mesh(Mesh* hex, Mesh* quadtri) {
		logt.add_value("gna", 3);
		if (hex->cells.nb() == 0) return;

		// merge vertices
		double eps = (1e-3)*get_cell_average_edge_size(hex);
		{
			vector<index_t> to_kill(hex->vertices.nb(), 0);
			vector<index_t> old2new(hex->vertices.nb());
			Geom::colocate(hex->vertices.point_ptr(0), 3, hex->vertices.nb(), old2new, eps);
			FOR(c, hex->cells.nb()) FOR(cv, 8) hex->cells.set_vertex(c, cv, old2new[hex->cells.vertex(c, cv)]);
			FOR(v, hex->vertices.nb()) if (old2new[v] != v) to_kill[v] = NOT_AN_ID;
			hex->vertices.delete_elements(to_kill);
		}
		snap_vertices_to_ref_vertices(hex, quadtri, eps);

		// check that there is no duplicated hex
		{
			int nb_duplicated_hex = 0;
			vector<vec3> sumVpos(hex->cells.nb(), vec3(0, 0, 0));
			vector<index_t> to_kill(hex->cells.nb(), 0);
			FOR(c, hex->cells.nb()) FOR(lv, 8) sumVpos[c] = sumVpos[c] + hex->vertices.point(hex->cells.vertex(c, lv));
			vector<index_t> bary_old2new(hex->cells.nb());
			Geom::colocate((double*)(sumVpos.data()), 3, hex->cells.nb(), bary_old2new, eps);
			FOR(c, hex->cells.nb()) {
				//geo_assert(bary_old2new[c] == c);
				if (bary_old2new[c] != c) {
					to_kill[c] = NOT_AN_ID;
					nb_duplicated_hex++;
				}
			}
			logt.add_value("nb_duplicated_hex", nb_duplicated_hex);
			plop(nb_duplicated_hex);
			hex->cells.delete_elements(to_kill);
			plop(hex->cells.nb());

		}


		// remove bad shaped hex
		{
			int nb_bad_shaped_hex = 0;
			vector<index_t> to_kill(hex->cells.nb(), false);
			FOR(c, hex->cells.nb()) {
				FOR(cf, 6)FOR(cfv, 4) {
					vec3 C = X(hex)[hex->cells.facet_vertex(c, cf, prev_mod(cfv, 4))];
					vec3 A = X(hex)[hex->cells.facet_vertex(c, cf, cfv)];
					vec3 B = X(hex)[hex->cells.facet_vertex(c, cf, next_mod(cfv, 4))];
					if (std::abs(dot(normalize(B - A), normalize(C - A))) > .6) {
						to_kill[c] = true;
						nb_bad_shaped_hex++;
					}
				}
			}

			logt.add_value("nb_bad_shaped_hex", nb_bad_shaped_hex);
			plop(nb_bad_shaped_hex);
			hex->cells.delete_elements(to_kill);
			plop(hex->cells.nb());

		}

		// remove hexes that are linked by 3 vertices
		{
			index_t nb_hex_linked_by_3_vertices = 0;
			vector<index_t> to_kill(hex->cells.nb(), false);
			vector<vector<index_t> > v2hexface(hex->vertices.nb());
			FOR(c, hex->cells.nb()) FOR(cf, 6) FOR(cfv, 4) v2hexface[hex->cells.facet_vertex(c, cf, cfv)].push_back(6 * c + cf);
			FOR(v, hex->vertices.nb()) FOR(h0, v2hexface[v].size())FOR(h1, h0) {
				index_t hexf0 = v2hexface[v][h0];
				index_t hexf1 = v2hexface[v][h1];
				if (to_kill[hexf0 / 6] || to_kill[hexf1 / 6]) continue;
				index_t f[2][4];
				FOR(lc, 4) f[0][lc] = hex->cells.facet_vertex(hexf0 / 6, hexf0 % 6, lc);
				FOR(lc, 4) f[1][lc] = hex->cells.facet_vertex(hexf1 / 6, hexf1 % 6, lc);
				index_t i0 = index_t(-1), i1 = index_t(-1);
				FOR(lc, 4) if (f[0][lc] == v) i0 = lc;
				FOR(lc, 4) if (f[1][lc] == v) i1 = lc;
				geo_assert(i0 != index_t(-1) && i1 != index_t(-1));
				int nb_shared = 0;
				FOR(lc, 4) if (f[0][(i0 + lc) % 4] == f[1][(i1 + 4 - lc) % 4])nb_shared++;
				if (nb_shared == 3) {
					to_kill[hexf0 / 6] = true;
					nb_hex_linked_by_3_vertices++;
				}
			}
			logt.add_value("nb_hex_linked_by_3_vertices", nb_hex_linked_by_3_vertices);
			plop(nb_hex_linked_by_3_vertices);
			hex->cells.delete_elements(to_kill);
			plop(hex->cells.nb());

		}
		kill_intersecting_hexes(hex);

		// remove hex that are incompatible with quadtri
		int sum_nb_hex_incompatible_with_quadtri = 0;
		if (quadtri != nullptr) {

			int nb_hex_incompatible_with_quadtri = 0;

			// remove all hex that touch border
			vector<index_t> to_kill(hex->cells.nb(), true);

			vector<BBox> inboxes(quadtri->facets.nb());
			FOR(f, quadtri->facets.nb())
				FOR(fv, quadtri->facets.nb_vertices(f))
				inboxes[f].add(X(quadtri)[quadtri->facets.vertex(f, fv)]);
			HBoxes hb(inboxes);


			bool may_have_intersections = true;
			while (may_have_intersections) {
				nb_hex_incompatible_with_quadtri = 0;
				may_have_intersections = false;
				// check for intersection
				hex->cells.connect();
				FOR(c, hex->cells.nb()) {
					to_kill[c] = false;
					FOR(cf, 6) {
						if (hex->cells.adjacent(c, cf) != NOT_AN_ID) continue;
						// Q contains the face of the hex PLUS two extra vertices to make a diamon shape
						vector<vec3> Q;
						Q.reserve(6);
						FOR(cfv, 4) Q.push_back(X(hex)[hex->cells.facet_vertex(c, cf, cfv)]);
						vec3 G = Poly3d(Q).barycenter();
						vec3 n = Poly3d(Q).normal();
						double decal = 0;
						FOR(cfv, 4) decal += .2*.25*(Q[cfv] - Q[next_mod(cfv, 4)]).length(); // .2*ave edge length 
						Q.push_back(G + decal*n);
						Q.push_back(G - decal*n);

						BBox b;
						FOR(cfv, 6) b.add(Q[cfv]);

						vector<index_t> primitives;
						hb.intersect(b, primitives);

						bool have_intersection = false;
						bool have_tri_quad_intersect = false;
						FOR(i, primitives.size()) {
							bool is_quatri_face = false;
							index_t f = primitives[i];
							vector<vec3> P;
							FOR(fv, quadtri->facets.nb_vertices(f))
								P.push_back(X(quadtri)[quadtri->facets.vertex(f, fv)]);
							geo_assert(P.size() < 5);

							// check "is_quatri_face"
							if (P.size() == 4) {
								int nb_match = 0;
								FOR(v, 4) {
									FOR(it, 4) if ((Q[it] - P[(v + it) % 4]).length2() == 0)nb_match++;
									//index_t it = 0;
									//while (it < 4 && (Q[it] - P[(v + it) % 4]).length2() == 0) it++;
									//is_quatri_face = is_quatri_face || (it == 4);
								}
								if (nb_match == 4)	is_quatri_face = true;
								if (nb_match == 3)  have_tri_quad_intersect = true;
							}
							else {
								FOR(v, 3) {
									index_t it = 0;
									while (it < 3 && (Q[it] - P[(v + it) % 3]).length2() == 0) it++;
									have_tri_quad_intersect = have_tri_quad_intersect || (it == 3);
								}
							}


							// check for non degenerated intersections
							if (!is_quatri_face) {
								vector<TriangleIsect> result;
								FOR(diam, 12) {
									if (P.size() == 3)
										have_intersection = have_intersection || triangles_intersections(
											P[0], P[1], P[2],
											Q[diamon_split[diam][0]], Q[diamon_split[diam][1]], Q[diamon_split[diam][2]], result
										);
									else {
										FOR(qu, 4)
											have_intersection = have_intersection || triangles_intersections(
												P[quad_split[qu][0]], P[quad_split[qu][1]], P[quad_split[qu][2]],
												Q[diamon_split[diam][0]], Q[diamon_split[diam][1]], Q[diamon_split[diam][2]], result
											);
									}

								}
							}

						}

						if (have_intersection) to_kill[c] = true;
						if (have_tri_quad_intersect) to_kill[c] = true;
						if (have_intersection || have_tri_quad_intersect)
							nb_hex_incompatible_with_quadtri++;
					}
				}

				sum_nb_hex_incompatible_with_quadtri += nb_hex_incompatible_with_quadtri;

				plop(nb_hex_incompatible_with_quadtri);
				may_have_intersections = (nb_hex_incompatible_with_quadtri > 0);
				hex->cells.delete_elements(to_kill);
				plop(hex->cells.nb());
			}
			logt.add_value("nb_hex_incompatible_with_quadtri", sum_nb_hex_incompatible_with_quadtri);
		}

		hex->cells.connect();
		hex->cells.compute_borders();

	}





}
