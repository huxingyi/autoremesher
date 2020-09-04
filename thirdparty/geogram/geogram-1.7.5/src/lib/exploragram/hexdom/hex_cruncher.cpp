/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000-2015 INRIA - Project ALICE
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  If you modify this software, you should include a notice giving the
 *  name of the person performing the modification, the date of modification,
 *  and the reason for such modification.
 *
 *  Contact for Graphite: Bruno Levy - Bruno.Levy@inria.fr
 *  Contact for this Plugin: Nicolas Ray - nicolas.ray@inria.fr
 *
 *     Project ALICE
 *     LORIA, INRIA Lorraine,
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX
 *     FRANCE
 *
 *  Note that the GNU General Public License does not permit incorporating
 *  the Software into proprietary programs.
 *
 * As an exception to the GPL, Graphite can be linked with the following
 * (non-GPL) libraries:
 *     Qt, tetgen, SuperLU, WildMagic and CGAL
 */

#include <exploragram/hexdom/hex_cruncher.h>
#include <exploragram/hexdom/intersect_tools.h>
#include <exploragram/hexdom/polygon.h>
#include <exploragram/hexdom/mesh_inspector.h>
#include <geogram/basic/geometry.h>
#include <geogram/points/colocate.h>
#include <geogram/mesh/triangle_intersection.h>
#include <geogram/mesh/mesh_repair.h> //used in bourrin subdivide hex
#define FPG_UNCERTAIN_VALUE 0
#include <geogram/numerics/predicates/orient3d.h>

#ifdef GEO_COMPILER_MSVC
#include <intrin.h>
void nico_assert(bool b) {
	if (!b) __debugbreak();
	geo_assert(b);
}
#else
#define nico_assert(b) geo_assert(b)
#endif

namespace GEO {


	// Extra connectivity dedicaed to surfaces with triangles and quads only.
	// Halfedges are indiced by 4* facet + local_id => requires padding for triangles
	struct QuadExtraConnectivity {
		void init(Mesh* p_m) {
			m = p_m;
			FOR(f, m->facets.nb()) {
				nico_assert(m->facets.nb_vertices(f) == 4 || m->facets.nb_vertices(f) == 3);// check that surface is quadragulated
			
			}
			opp_h.clear();
			opp_h.resize(4 * m->facets.nb(), NOT_AN_ID); // NOT facet_corners !!!
			create_non_manifold_facet_adjacence(m);
			FOR(f, m->facets.nb()) FOR(lc, m->facets.nb_vertices(f)) {
				index_t opp_f = m->facets.adjacent(f, lc);
				if (opp_f == NOT_AN_ID) { plop("bad adjacency detected "); continue; }
			
				index_t opp_lc = NOT_AN_ID;
				FOR(opp_lc_it, m->facets.nb_vertices(opp_f)) 
					if (f == m->facets.adjacent(opp_f, opp_lc_it)
						&& m->facets.vertex(f, (lc + 1) % m->facets.nb_vertices(f)) == m->facets.vertex(opp_f, opp_lc_it)
						) opp_lc = opp_lc_it;
				if (opp_lc == NOT_AN_ID) plop("not a symetric opposite !");
				set_opp(4 * f + lc, 4 * opp_f + opp_lc);
				if (vertex(4 * f + lc) != vertex(next(opp(4 * f + lc)))) plop("wrong opposites");
				if (vertex(next(4 * f + lc)) != vertex(opp(4 * f + lc))) plop("wrong opposites");
			}

		}


		void check_integrity() {
			FOR(h, 4 * m->facets.nb()) if (valid(h)) {
				FOR(d, 5) nico_assert(valid(next(h, d)));
				nico_assert(valid(opp(h)));
				nico_assert(valid(next_around_vertex(h)));
				nico_assert(valid(next_around_vertex(next_around_vertex(h))));
			}
		}

		void debug_export_adjacence() {
			FOR(f, m->facets.nb()) FOR(lc, m->facets.nb_vertices(f)) {
				if (opp(4 * f + lc) != NOT_AN_ID)
					m->facets.set_adjacent(f, lc, face(opp(4 * f + lc)));
				else m->facets.set_adjacent(f, lc, NOT_AN_ID);
			}
		}

		bool valid(index_t h) { return h < 4 * m->facets.nb() && (h%4)<m->facets.nb_vertices(h/4); }

		index_t fsize(index_t e){ nico_assert(valid(e)); return m->facets.nb_vertices(face(e)); }
		void set_opp(index_t i, index_t j) { nico_assert(valid(i) && valid(j));  opp_h[i] = j; opp_h[j] = i; }
		index_t face(index_t e) { nico_assert(valid(e)); return e / 4; }
		index_t local_id(index_t e) { nico_assert(valid(e)); return e % 4; }
		index_t next(index_t e, index_t nb = 1) { nico_assert(valid(e)); return 4 * face(e) + ((e%4 + nb) % fsize(e)); }
		index_t opp(index_t e) { nico_assert(valid(e)); return opp_h[e]; }
		index_t vertex(index_t e) { nico_assert(valid(e)); return m->facets.vertex(face(e), local_id(e)); }
		index_t corner(index_t e) { nico_assert(valid(e)); return m->facets.corner(face(e), local_id(e)); }
		index_t next_around_vertex(index_t e) { nico_assert(valid(e)); return opp(next(e, fsize(e)-1)); }

		void set_vertex(index_t e, index_t v) { nico_assert(valid(e) && v < m->vertices.nb()); m->facets.set_vertex(face(e), local_id(e), v); }


		bool has_valid_one_ring(index_t e) {
			index_t cir = e;
			int count = 0;
			do {
				if (cir == NOT_AN_ID)   return false;
				if (count++ == 1000) return false;
				cir = next_around_vertex(cir);
			} while (cir != e);
			return true;
		}
		int valence(index_t e) {
			nico_assert(has_valid_one_ring(e));
			index_t cir = e;
			int count = 0;
			do {
				count++;
				cir = next_around_vertex(cir);
			} while (cir != e);
			return count;
		}

		bool is_closed() {
			FOR(h, opp_h.size()) if (opp_h[h] == NOT_AN_ID) {
				Attribute<double> deb(m->vertices.attributes(), "debug");
				deb[vertex(h)] = 10;
				deb[vertex(next(h))] = 10;
				return false;
			}
			FOR(v, m->vertices.nb()) if (!has_valid_one_ring(v)) {
				Attribute<double> deb(m->vertices.attributes(), "debug");
				deb[v] = 10;
				return false;
			}
			return true;
		}


		Mesh* m;
		vector<index_t> opp_h;
	};



	struct CutSingularity {
		Mesh* m;                // ;)
		vector<bool> visited;   // prevents iterating more than once on the same cut
		vec3 N;                 // normal to the current cut
		QuadExtraConnectivity qem;
		vector<index_t> border;
		vector<vec3> pts;
		vector<index_t> quads;

		CutSingularity(Mesh* p_m) {
			m = p_m;
			qem.init(m);
			visited.resize(4 * m->facets.nb(), false);
		}

		bool create_edge_loop(index_t  h, vector<index_t>& test_border) {
			double sigma_angle = 0;
			index_t cir = h;
			do {
				visited[cir] = true;
				if (qem.fsize(cir) == 3) return false;
				vec3 Nup = facet_normal(m, qem.face(cir));
				if (qem.opp(cir) == NOT_AN_ID) { test_border.clear(); break; }
				vec3 Ndown = facet_normal(m, qem.face(qem.opp(cir)));
				if (dot(N, Nup) > .5 || dot(N, Ndown) < -.5) return false; 
				


				vec3 cir_dir = X(m)[qem.vertex(qem.next(cir))] - X(m)[qem.vertex(cir)];
				cir_dir = normalize(cir_dir);

				test_border.push_back(cir);
				index_t next = NOT_AN_ID;
				index_t in_cir = qem.next(cir);


				//double best_dot = -1e20;
				double best_angle = -M_PI;
				do {
					if (qem.fsize(in_cir)!=4 || qem.opp(in_cir) == NOT_AN_ID) {
						next = NOT_AN_ID;
						return false;
					}

					vec3 in_cir_dir = X(m)[qem.vertex(qem.next(in_cir))] - X(m)[qem.vertex(in_cir)];
					in_cir_dir = normalize(in_cir_dir);

					if (fabs(dot(in_cir_dir, N)) < sin(M_PI / 8.)	    // stay in the cut plane (orthogonal to N)
						&& in_cir != qem.opp(cir)				        // do not go back
						) {
						double newangle = atan2(dot(N, cross(cir_dir, in_cir_dir)), dot(in_cir_dir, cir_dir));
						if (best_angle < newangle) {
							best_angle = newangle;
							next = in_cir;
						}
					}
					in_cir = qem.next(qem.opp(in_cir));
				} while (in_cir != qem.next(cir));
				sigma_angle += best_angle;

				if (next == NOT_AN_ID || test_border.size() > 30) {
					return false;
				}
				cir = next;
			} while (cir != h);
			if (sigma_angle < 0) return false;
			return true;
		}

		bool cut_separates_2_shorts_closed_quads_strips(index_t h) {
			// check that it separates two smalls and close quads strips
			index_t quad_strip_start[2] = { qem.next(h), qem.next(qem.opp(h)) };
			FOR(q, 2) {
				index_t it = quad_strip_start[q];
				int i = 0;
				for (;;) {
					if (qem.fsize(it) == 3) return false;
					if (i == 30 || qem.opp(it) == NOT_AN_ID)
						return false;
					it = qem.next(qem.opp(it), 2);
					if (it == quad_strip_start[q]) break;
					i++;
				}
			}
			return true;
		}


		bool can_easily_discard_current_edge_loop(vector<index_t>& test_border) {
			if (test_border.size() < 3) return true;
			if (!border.empty() && test_border.size() >= border.size()) return true;
			if (test_border.size() == 4
				&& (qem.face(qem.opp(test_border[0])) == qem.face(qem.opp(test_border[2]))
					|| qem.face(test_border[0]) == qem.face(test_border[2])
					)
				) return true;
			if (test_border.size() % 2 != 0) return true;
			return false;
		}



		bool apply() {
			vec3 best_N;
			double best_cut_area = 1e20;

			// double ave_edge_length = get_facet_average_edge_size(m);  // BL: unused.
			//0;
			//FOR(f, m->facets.nb()) FOR(v, 4) ave_edge_length += (X(m)[m->facets.vertex(f, v)] - X(m)[m->facets.vertex(f, (v + 1) % 4)]).length();
			//ave_edge_length /= 4.*double(m->facets.nb());
			

			// STEP 1: determine a valid cut along halfedges "border", its quadrangulation "quads", with vertices "pts[i]" (starting with vertices of "border")


			int nb_border_tried = 0;
			int nb_border_success = 0;
			Attribute<int> fail_c(m->facet_corners.attributes(), "fail_c");
			FOR(h, 4 * m->facets.nb()) {
				if (!qem.valid(h)) continue;
				fail_c[m->facets.corner(h / 4, h % 4)] = 0;
				fail_c[m->facets.corner(h/4,h%4)] = 10;

				if (visited[h]) continue;
				fail_c[m->facets.corner(h / 4, h % 4)] = 20;

				// index_t f = qem.face(h); // [BL: unused]
				if (qem.fsize(h)!=4) continue;
				fail_c[m->facets.corner(h / 4, h % 4)] = 30;

				if (qem.opp(h) == NOT_AN_ID) continue;
				fail_c[m->facets.corner(h / 4, h % 4)] = 40;

				if (!cut_separates_2_shorts_closed_quads_strips(h)) continue;
				fail_c[m->facets.corner(h / 4, h % 4)] = 50;

				N = X(m)[qem.vertex(qem.next(h, 3))] - X(m)[qem.vertex(h)];
				N = normalize(N);
				vector<index_t> test_border;
				
			
				// STEP 1.1 create the edge loop starting from h
				if (!create_edge_loop(h, test_border))  continue;
				
				fail_c[m->facets.corner(h / 4, h % 4)] = 60;

			
				
			
				// STEP 1.2 check if the edge loop starting from h is valid and better than previous loop
				if (can_easily_discard_current_edge_loop(test_border)) continue;
				fail_c[m->facets.corner(h / 4, h % 4)] = 70;

				vector<vec3> test_pts;
				vector<index_t> test_quads;
				FOR(e, test_border.size()) test_pts.push_back(X(m)[qem.vertex(test_border[e])]);


				double cut_area = 0;
				FOR(e, test_pts.size()) cut_area -= dot(cross(test_pts[e], test_pts[(e + 1) % test_pts.size()]), N);
				if (best_cut_area < cut_area)continue;

			

				Poly3d p3(test_pts);
				nb_border_tried++;



				if (!p3.try_quadrangulate(test_quads)) {
					continue;
				} else {
					nb_border_success++;
					bool cut_will_intersect = false;

					FacetIntersect finter(m);
					FOR(q, test_quads.size() / 4) {
						vector<vec3> quad;
						FOR(lc, 4) quad.push_back(test_pts[test_quads[4 * q + lc]]);
						cut_will_intersect = cut_will_intersect || finter.get_intersections(quad).size()>0;
					}

					if (cut_will_intersect) {
						continue;
					}
				}

				// STEP 1.3 validate the new loop
				{
					border.swap(test_border);
					pts.swap(test_pts);
					test_quads.swap(quads);
					best_cut_area = cut_area;
					best_N = N;
				}
			}

			plop(nb_border_tried);
			plop(nb_border_success);

			if (border.empty()) return false;

			vector<index_t> upper_v;
			vector<index_t> lower_v;
			{
				index_t off_v = m->vertices.create_vertices(border.size());
				FOR(e, border.size()) {
					upper_v.push_back(qem.vertex(border[e]));
					pts.push_back(X(m)[upper_v[e]]);
					lower_v.push_back(off_v + e);
					X(m)[off_v + e] = pts[e];
				};
			}
			vector<vector<index_t> > opp_fan(border.size());
			FOR(e, border.size()) {
				index_t cir = qem.opp(border[e]);
				do {
					opp_fan[e].push_back(cir);
					nico_assert(qem.fsize(cir) == 4);
					cir = qem.opp(qem.next(cir, 3));
				} while (cir != border[next_mod(e, border.size())]);
			}	
			FOR(e, border.size())
				FOR(v, opp_fan[e].size())
					qem.set_vertex(opp_fan[e][v], lower_v[next_mod(e, border.size())]);
		
			if (pts.size() > border.size()) {
				index_t off_v = m->vertices.create_vertices(2 * (pts.size() - border.size()));
				FOR(i, pts.size() - border.size()) {
					FOR(d,2) X(m)[off_v + 2 * i + d] = pts[border.size() + i];

					upper_v.push_back(off_v + 2 * i);
					lower_v.push_back(off_v + 2 * i + 1);
				}
			}
			FOR(q, quads.size() / 4) {
				m->facets.create_quad(
					upper_v[quads[4 * q + 0]],
					upper_v[quads[4 * q + 3]],
					upper_v[quads[4 * q + 2]],
					upper_v[quads[4 * q + 1]]
				) ;
				m->facets.create_quad(
					lower_v[quads[4 * q + 0]],
					lower_v[quads[4 * q + 1]],
					lower_v[quads[4 * q + 2]],
					lower_v[quads[4 * q + 3]]
				) ;
			}
			// debug output

			Attribute<int> date(m->edges.attributes(), "date");
			if (!border.empty()) {
				index_t off_e = m->edges.create_edges(border.size());
				FOR(e, border.size()) {
					date[off_e + e] = int(off_e);
					FOR(extr, 2)
						m->edges.set_vertex(off_e + e, extr, qem.vertex(border[(e + extr) % border.size()]));
				}
			}
			return true;

		}
	};




	inline double cos_corner(vec3 B, vec3 A, vec3 C) {
		return dot(normalize(B - A), normalize(C - A));
	}

	struct VertexPuncher {
		Mesh* m;
		Mesh* newhex;
		QuadExtraConnectivity qem;
		DynamicHBoxes hb;                              //      -> a static BBox tree
		FacetIntersect finter;
		double ave_edge_length;
		Attribute<bool> dead_face;

		int nb_punchs;
		index_t punch_v;
		index_t nv_punch_v;
		index_t H[3][4];
		index_t oppH[3][4];
		vec3 old_vertex_position;
		vec3 new_vertex_position;
		vector<int> v2nb_facets;           //      -> facets that have moved
		index_t via_facet[3];


		Attribute<double> failt; ///DEBUG
		Attribute<bool> isquad;


		VertexPuncher(Mesh* p_m, Mesh* p_newhex): finter(p_m) {
			m = p_m;
			newhex = p_newhex;
			dead_face.bind(m->facets.attributes(), "dead_face");
		}



		void unglue_duplicates() {
			FOR(lf, 3) {
				index_t f = via_facet[lf];
				if (f == NOT_AN_ID) continue;
				index_t cir_h[4];
				index_t cir_opp[4];
				index_t opp_f = NOT_AN_ID;
				FOR(lh, 4) {
					index_t h = 4 * f + lh;
					index_t h_opp = qem.opp(h);
					if (qem.vertex(qem.next(h_opp, 2)) == qem.vertex(qem.next(h, 3))
						&& qem.vertex(qem.next(h_opp, 3)) == qem.vertex(qem.next(h, 2))) {
						FOR(i, 4) {
							cir_h[i] = 4 * f + (lh + i) % 4;
							cir_opp[i] = 4 * qem.face(h_opp) + (qem.local_id(h_opp) + 4 - i) % 4;
						}
						opp_f = qem.face(h_opp);
						continue;
					}
				}
				if (opp_f == NOT_AN_ID) continue;

				dead_face[f] = true;
				dead_face[opp_f] = true;
				FOR(i, 4) {
					//std::cerr << qem.vertex(cir_h[i]) << "  " << qem.vertex(qem.next(cir_opp[i])) << "  \n";
					if (qem.opp(cir_h[i]) == qem.opp(cir_opp[i])) continue;
					qem.set_opp(qem.opp(cir_opp[i]), qem.opp(cir_h[i]));
					qem.set_opp(cir_h[i], cir_opp[i]);
				};
			}
		}




		BBox facet_bbox(index_t f) {
			BBox res;
			FOR(fv, m->facets.nb_vertices(f)) res.add(X(m)[m->facets.vertex(f, fv)]);
			return res;
		}

		void init() {
			isquad.bind(m->facets.attributes(), "isquad");

			qem.init(m);
			nb_punchs = 0;
			v2nb_facets.clear();
			//moved_facets.clear();

			v2nb_facets.resize(m->vertices.nb(), 0);
			FOR(f, m->facets.nb()) FOR(v, m->facets.nb_vertices(f)) v2nb_facets[m->facets.vertex(f, v)]++;



			FOR(f, m->facets.nb())dead_face[f] = false;

			// mesh resolution
			ave_edge_length = 0;
			int nb_samples = 0;
			FOR(h, 4 * m->facets.nb()) if (qem.valid(h)) {
				ave_edge_length += (X(m)[qem.vertex(h)] - X(m)[qem.vertex(qem.next(h))]).length();
				nb_samples++;
			}
			ave_edge_length /= double(nb_samples);

			// structures to find facets
			vector<BBox> inboxes(m->facets.nb());
			FOR(f, m->facets.nb()) inboxes[f] = facet_bbox(f);//FOR(fv, m->facets.nb_vertices(f)) inboxes[f].add(X(m)[m->facets.vertex(f, fv)]);
			hb.init(inboxes);


		}



		bool init_one_ring(index_t h) {
			// check there is no triangles involved
			index_t cir = h;
			FOR(i, 3) {
				if (qem.fsize(cir) != 4) return false;
				cir = qem.next_around_vertex(cir);
			}
			// init H
			FOR(f, 3) {
				FOR(v, 4) {
					H[f][v] = qem.next(h, v);
					if (H[f][v] == NOT_AN_ID)   return false;// TO REMOVE if m is closed                    
				}
				//if (!isquad[qem.face(H[f][0])])   return false;
				h = qem.next_around_vertex(h);
			}

			// check valence 3
			if (qem.next_around_vertex(H[2][0]) != H[0][0]) return false;

			// init the opposites
			FOR(f, 3) FOR(e, 4) {
				oppH[f][e] = qem.opp(H[f][e]);
				if (oppH[f][e] == NOT_AN_ID)       return false;// TO REMOVE if m is closed

			}
			return true;
		}
		// create new hex
		void create_new_hex() {
			index_t off_v = newhex->vertices.create_vertices(8);
			Attribute<double> init(newhex->vertices.attributes(), "init");
			init[off_v] = -100; FOR(i, 7)init[off_v + i + 1] = nb_punchs;
			X(newhex)[off_v + 0] = old_vertex_position;
			X(newhex)[off_v + 1] = X(m)[qem.vertex(H[0][1])];
			X(newhex)[off_v + 2] = X(m)[qem.vertex(H[0][3])];
			X(newhex)[off_v + 3] = X(m)[qem.vertex(H[0][2])];
			X(newhex)[off_v + 4] = X(m)[qem.vertex(H[2][1])];
			X(newhex)[off_v + 5] = X(m)[qem.vertex(H[2][2])];
			X(newhex)[off_v + 6] = X(m)[qem.vertex(H[1][2])];
			X(newhex)[off_v + 7] = X(m)[nv_punch_v];
			newhex->cells.create_hex(off_v + 0, off_v + 1, off_v + 2, off_v + 3, off_v + 4, off_v + 5, off_v + 6, off_v + 7);
		}

		bool new_hex_geometry_is_crappy() {
			
		
			FOR(front, 2) {// front=0 for actual faces, front =1 for new faces 
				FOR(fid, 3) {
					vector<vec3> v(4);
					if (front == 0) {
						v[0] = old_vertex_position;
						v[1] = X(m)[qem.vertex(H[fid][1])];
						v[2] = X(m)[qem.vertex(H[fid][2])];
						v[3] = X(m)[qem.vertex(H[fid][3])];
					}
					else {
						v[0] = X(m)[nv_punch_v];
						v[1] = X(m)[qem.vertex(H[next_mod(fid, 3)][2])];
						v[2] = X(m)[qem.vertex(H[next_mod(fid, 3)][1])];
						v[3] = X(m)[qem.vertex(H[fid][2])];
					}
					vec3 n = Poly3d(v).normal();
					FOR(lv, 4) if (dot(n, cross(normalize(v[(lv + 2) % 4] - v[(lv + 1) % 4]), normalize(v[(lv) % 4] - v[(lv + 1) % 4]))) < .1)
						return true;

				}
			}
			return false;

		}

		//topo_punch
		void topo_punch() {
			FOR(f, 3) qem.set_vertex(H[f][0], nv_punch_v);
			FOR(f, 3) qem.set_vertex(H[f][1], qem.vertex(oppH[(f + 1) % 3][1]));
			FOR(f, 3) qem.set_vertex(H[f][2], qem.vertex(oppH[(f + 1) % 3][2]));
			FOR(f, 3) qem.set_vertex(H[f][3], qem.vertex(oppH[(f + 2) % 3][1]));
			FOR(f, 3) qem.set_opp(H[f][1], oppH[(f + 1) % 3][2]);
			FOR(f, 3) qem.set_opp(H[f][2], oppH[(f + 2) % 3][1]);
		}
		bool topo_can_punch() {
			vector<index_t> neigh;
			// check if a vertex is duplicated
			FOR(ring, 3) FOR(lv, 2) neigh.push_back(H[ring][lv + 1]);
			FOR(n0, 6) if (qem.opp(neigh[n0]) == NOT_AN_ID) return false;
			FOR(n0, 6)for (index_t n1 = 0; n1 < n0; n1++)
				if (qem.vertex(neigh[n0]) == qem.vertex(neigh[n1])) return false;;


			// check is two boundary edges are opposite
			FOR(n0, 6)for (index_t n1 = 0; n1 < n0; n1++)
				if (qem.opp(neigh[n0]) == neigh[n1]) return false;
			return true;
		}


		void produce_diamon(vector<vec3>& P, vec3 A, vec3 B, vec3 C, vec3 D, double h) {
			P.reserve(6);
			P.resize(4);
			P[0] = A; P[1] = B; P[2] = C; P[3] = D;
			vec3 G = Poly3d(P).barycenter();
			vec3 n = Poly3d(P).normal();
			P.push_back(G + h*n);
			P.push_back(G - h*n);
		}


		bool punch_will_produce_intersection() {
			// check for geometric intersections
			index_t Qv[3][4];
			FOR(f, 3) Qv[f][0] =nv_punch_v;
			FOR(f, 3) Qv[f][1] = qem.vertex(oppH[(f + 1) % 3][1]);
			FOR(f, 3) Qv[f][2] = qem.vertex(oppH[(f + 1) % 3][2]);
			FOR(f, 3) Qv[f][3] = qem.vertex(oppH[(f + 2) % 3][1]);


			FOR(f0, 3)FOR(f1, 3) {
				if (f0 >= f1) continue;
				vector<vec3> Q0(4), Q1(4);
				FOR(i, 4) Q0[i]= X(m)[Qv[f0][i]];
				FOR(i, 4) Q1[i] = X(m)[Qv[f1][i]];
				if (polyintersect(Q0, Q1)) return true;
			}
			FOR(fid, 3) {
				vector<vec3> Q;
				FOR(i, 4) Q.push_back(X(m)[Qv[fid][i]]);
				if (finter.get_intersections(Q).size() > 0) return true;
				continue;
			}
			return false;
		}


		bool apply(int& nbmaxpunch) {
			if (m->facets.nb() == 0) return false;
			init();
			bool finished = false;
			failt.bind(m->vertices.attributes(), "failt");
			FOR(v, m->vertices.nb()) failt[v] = 0;


	
			while (!finished) {
				
				bool found_vertex_to_punch = false;
				FOR(seed, 4 * m->facets.nb()) {
					if (!qem.valid(seed)) continue;
					if (qem.next_around_vertex(seed) < seed || qem.next_around_vertex(qem.next_around_vertex(seed)) < seed) continue;
					punch_v = qem.vertex(seed);
					if (!init_one_ring(seed)) { failt[punch_v] = std::max(failt[punch_v], 10.); continue; }
					if (!topo_can_punch()) { failt[punch_v] = std::max(failt[punch_v], 20.); continue; }
					old_vertex_position = X(m)[punch_v];
					nv_punch_v = punch_v;

					if (tet_vol(X(m)[punch_v],
						X(m)[qem.vertex(H[0][1])],
						X(m)[qem.vertex(H[1][1])],
						X(m)[qem.vertex(H[2][1])]
						) > 0) continue;

					// do we already have 4+ faces of the hex ?
					bool bad_config = false;
					index_t punch_cand[3] = { NOT_AN_ID, NOT_AN_ID, NOT_AN_ID };
					FOR(i, 3) via_facet[i] = NOT_AN_ID;
					index_t punch_cand_ref = NOT_AN_ID;
					FOR(i, 3) if (qem.valence(oppH[i][2]) == 3) {
						punch_cand[i] = qem.vertex(qem.next(oppH[i][2], 2));
						punch_cand_ref = punch_cand[i];
						via_facet[i] = qem.face(oppH[i][2]);
					}
					FOR(i, 3) {
						if (punch_cand[i] != NOT_AN_ID && punch_cand[i] != punch_cand_ref)
							bad_config = true;
						if (qem.valence(H[i][2]) == 3)
							if ((via_facet[i] == NOT_AN_ID) != (via_facet[(i + 2) % 3] == NOT_AN_ID))
								bad_config = true;
						if (via_facet[i] != NOT_AN_ID && m->facets.nb_vertices(via_facet[i])==3)
							bad_config = true;
					}
					if (bad_config) { failt[punch_v] = std::max(failt[punch_v], 20.); continue; }

					if (punch_cand_ref != NOT_AN_ID) {
						nv_punch_v = punch_cand_ref;
						FOR(i, 3) if (via_facet[i] != NOT_AN_ID) dead_face[via_facet[i]] = true;             // the face will be cancelled by its opposite
					}



					if (nv_punch_v == punch_v) {
						//if (search_for_existing_vertex_to_punch) continue;
						// check geometry of existing faces
						bool have_bad_angle = false;
						FOR(ring, 3) FOR(lv, 4)
							have_bad_angle = have_bad_angle || std::abs(cos_corner(
								X(m)[qem.vertex(H[ring][lv])],
								X(m)[qem.vertex(H[ring][(lv + 1) % 4])],
								X(m)[qem.vertex(H[ring][(lv + 2) % 4])]
							)) > .8;
						if (have_bad_angle) { failt[punch_v] = std::max(failt[punch_v], 30.); continue; }

						//-------------------------
						{
							vec3 cubebary(0, 0, 0);
							FOR(i, 3) FOR(e, 2) cubebary = cubebary + X(m)[qem.vertex(H[i][1 + e])];
							cubebary = (1. / 6.) *cubebary;
							vec3 decal = cubebary - old_vertex_position;
							vec3 n[3];
							FOR(i, 3) n[i] = facet_normal(m, qem.face(H[i][0]));
							if (dot(decal, n[0] + n[1] + n[2]) > 0) {
							    failt[punch_v] = std::max(failt[punch_v], 40.);
								continue;
							}
							new_vertex_position = 2. * cubebary - old_vertex_position;
						}
						// new way to compute new position
						vec3 n[3];
						mat3 mat;
						index_t tri[3][3];// 3 triplet of vertices that miss the last point
						FOR(f, 3) {
							index_t next_f = next_mod(f, 3);
							tri[f][0] = qem.vertex(H[f][2]);
							tri[f][1] = qem.vertex(H[next_f][1]);
							tri[f][2] = qem.vertex(H[next_f][2]);
						}

						FOR(f, 3) {
							have_bad_angle = have_bad_angle
								|| std::abs(cos_corner(X(m)[tri[f][0]], X(m)[tri[f][1]], X(m)[tri[f][2]])) > cos(M_PI / 8.);
						}
						if (have_bad_angle) { failt[punch_v] = std::max(failt[punch_v], 50.); continue; }

						FOR(f, 3) {
							n[f] = normalize(cross(X(m)[tri[f][1]] - X(m)[tri[f][0]], X(m)[tri[f][2]] - X(m)[tri[f][0]]));
							FOR(j, 3) mat(f, j) = n[f][j];
						}
						mat3 inv = mat.inverse();
						vec3 b;
						FOR(i, 3) b[i] = dot(n[i], X(m)[tri[i][0]]);
						mult(inv, b.data(), new_vertex_position.data());
						X(m)[nv_punch_v] = new_vertex_position;
					}

					// check that punch vertex is convex
					if (new_hex_geometry_is_crappy()) {
						X(m)[punch_v] = old_vertex_position;
						failt[punch_v] = std::max(failt[punch_v], 60.); 
						 continue; 
					}


				
					if (punch_will_produce_intersection()) {
						X(m)[punch_v] = old_vertex_position;
						continue;
					}

					// split vertex if needed (non manifold)
					if (punch_v == nv_punch_v && v2nb_facets[punch_v] != 3) {
						index_t nvv = m->vertices.create_vertex();
						v2nb_facets[punch_v] -= 3;
						v2nb_facets.push_back(3);
						X(m)[punch_v] = old_vertex_position;
						
						X(m)[nvv] = new_vertex_position;
						punch_v = nvv;
						nv_punch_v = nvv;
					}
				

					FOR(f, 3) { v2nb_facets[qem.vertex(H[f][1])]--; v2nb_facets[qem.vertex(H[f][2])]++; }
					plop("create_new_hex()");
					create_new_hex();
					topo_punch();
					FOR(lf, 3) hb.update_bbox(qem.face(H[lf][0]), facet_bbox(qem.face(H[lf][0])));
					FOR(lf, 3) finter.hb.update_bbox(qem.face(H[lf][0]), facet_bbox(qem.face(H[lf][0])));
					found_vertex_to_punch = true;

					unglue_duplicates();

					nb_punchs++;
					if (!(nb_punchs % 100)) plop(nb_punchs);
					if (-1 == --nbmaxpunch) {
						plop(nbmaxpunch);
						goto cleanup;// return true;
					}// debug only... to be removed
				}
				plop("done");
				finished = !found_vertex_to_punch;
				
			}

		cleanup:

			qem.debug_export_adjacence();
			vector<index_t> to_kill(m->facets.nb());
			FOR(f, m->facets.nb()) to_kill[f]= (m->facets.nb_vertices(f) == 4) ;
			FOR(f, m->facets.nb()) {
				index_t opp = qem.face(qem.opp(4 * f));
				if (m->facets.nb_vertices(f) != 4) continue;
				if (m->facets.nb_vertices(opp) != 4) { to_kill[f] = false; continue; }
				for (index_t h = 4 * f; h < 4 * (f + 1); h++)
					if (qem.face(qem.opp(h)) != opp) {
						to_kill[f] = false;
						to_kill[opp] = false;
					}
			}
			m->facets.delete_elements(to_kill);
			//check_no_intersecting_faces(m, true);
			plop(nb_punchs);
			return nb_punchs > 0;
		}


	};






	static void remove_scabs(Mesh* m, Mesh* newhex) {
		GEO::Logger::out("HexDom")  << "try to remove_scabs" <<  std::endl;

		Attribute<int> ft(m->facets.attributes(), "ft");            // facet type 
		vector<index_t> to_kill(m->facets.nb(), false);             // ;( faces may be hurt and even killed in this fonction
		vector<index_t> local_id(m->vertices.nb(), NOT_AN_ID);  // ids in the array of new vertices (projected onto the boundary)


		QuadExtraConnectivity qem;
		qem.init(m);
		FOR(f, m->facets.nb()) ft[f] = 0;
		FOR(h, 4 * m->facets.nb()) {

			if (!qem.valid(h)) continue;
			if (to_kill[qem.face(h)]) continue;
			if (qem.opp(h) == NOT_AN_ID) continue;
			if (ft[qem.face(qem.opp(h))] != 0) continue;

			vector<index_t> contour;
			bool fail = false;
			{index_t cir = h;
			do {
				contour.push_back(cir);
				if (qem.opp(cir) == NOT_AN_ID) { fail = true; break; }
				if (qem.opp(qem.next(cir, 3)) == NOT_AN_ID) { fail = true; break; }
				if (qem.fsize(cir) != 4) { fail = true; break; }
				if (qem.fsize(qem.opp(qem.next(cir, 1))) != 4) { fail = true; break; }
				if (qem.fsize(qem.opp(qem.next(cir, 3))) == 4) { fail = true; break; }
				cir = qem.next(qem.opp(cir), 2);
			} while (cir != h);
			}
			if (fail) continue;




			FOR(c, contour.size()) ft[qem.face(contour[c])] = 1;

			vector<index_t> in_facets;
			vector<index_t> out_facets;
			in_facets.push_back(qem.face(qem.opp(qem.next(h))));
			out_facets.push_back(qem.face(qem.opp(qem.next(h, 3))));

			ft[in_facets[0]] = 2;
			ft[out_facets[0]] = 3;

			vector<index_t> stack;
			stack.push_back(in_facets[0]);
			stack.push_back(out_facets[0]);
			while (!stack.empty() && !fail) {
				index_t f = stack.back();
				stack.pop_back();
				for (index_t cir = 4 * f; cir < 4 * f + m->facets.nb_vertices(f); cir++) {
					// need to have an opposite
					if (qem.opp(cir) == NOT_AN_ID) {
						fail = true;
						break;
					}
					// do not link triangle/quads OR quads with the contour
					index_t oppf = qem.face(qem.opp(cir));
					if (ft[oppf] != 1 && m->facets.nb_vertices(oppf) != m->facets.nb_vertices(f)) {
						fail = true;
						break;
					}




					if (ft[oppf] == 0) {
						// linked by a reasonably flat edge---only for inside until outside have better mesh quality            
						if (dot(facet_normal(m, f), facet_normal(m, oppf)) < .8 && ft[f] == 2) {
							fail = true;
							break;
						}
						ft[oppf] = ft[f];
						if (ft[f] == 2) in_facets.push_back(oppf);
						if (ft[f] == 3) out_facets.push_back(oppf);
						stack.push_back(oppf);
					}
				}
			}
			if (fail) {
				FOR(f, m->facets.nb()) ft[f] = 0;
				continue;
			}
			// check that in_facets and outfacets are topo disks 
			index_t in_facets_nb_neig = 0;
			FOR(i, in_facets.size()) {
				nico_assert(m->facets.nb_vertices(in_facets[i]) == 4);
				for (index_t cir = 4 * in_facets[i]; cir < 4 * in_facets[i] + 4; cir++) {
					if (ft[qem.face(qem.opp(cir))] != 2)
						in_facets_nb_neig++;
				}
			}
			if (in_facets_nb_neig != contour.size()) {
				FOR(f, m->facets.nb()) ft[f] = 0;
				continue;
			}
			index_t out_facets_nb_neig = 0;
			FOR(i, out_facets.size()) {
				nico_assert(m->facets.nb_vertices(out_facets[i]) != 4);
				for (index_t cir = 4 * out_facets[i]; cir < 4 * out_facets[i] + 3; cir++) {
					if (ft[qem.face(qem.opp(cir))] != 3)
						out_facets_nb_neig++;
				}
			}
			if (out_facets_nb_neig != contour.size()) 	{
				FOR(f, m->facets.nb()) ft[f] = 0; 
				continue;
			}


			//generate new vertices
			index_t nbv = 0;
			vector<vec3> packed_v; // for each vertex of infacets, stores the 3-uplet (pos, normal, projected pos)
			FOR(i, in_facets.size()) {
				for (index_t cir = 4 * in_facets[i]; cir < 4 * in_facets[i] + 4; cir++) {
					index_t v = qem.vertex(cir);
					if (local_id[v] == NOT_AN_ID) {
						local_id[v] = nbv;
						nbv++;
						packed_v.push_back(X(m)[v]);
						packed_v.push_back(vec3(0, 0, 0));
						packed_v.push_back(vec3(0, 0, 0));
					}
					// accumulate facet normals into vertex normal
					packed_v[3 * local_id[v] + 1] = packed_v[3 * local_id[v] + 1] + facet_normal(m, in_facets[i]);
				}
			}



			//return ;

			double ave_decal_length = 0;
			int nb_intersections = 0;
			FOR(v, nbv) {
				vec3 O = packed_v[3 * v];
				vec3 O2 = packed_v[3 * v] + normalize(packed_v[3 * v + 1]);
				FOR(i, out_facets.size()) {
					//FOR(tr, 2) 
					{
						//index_t cir = 4 * out_facets[i] + 2 * tr; // each quad is decomposed into 2 triangles 
						vec3 P[3];
						FOR(p, 3)  P[p] = X(m)[m->facets.vertex(out_facets[i], p)];// qem.vertex(qem.next(cir, p))];
						double sign[3];
						FOR(p, 3) sign[p] = tetra_volume_sign(P[p], P[(p + 1) % 3], O, O2);
						if (!same_sign(sign[0], sign[1]) || !same_sign(sign[0], sign[2])) continue;
						double c0 = Geom::tetra_volume(P[0], P[1], P[2], O);
						double c1 = Geom::tetra_volume(P[0], P[1], P[2], O2);
						plop("intersection found");
						packed_v[3 * v + 2] = O + (c0 / (c0 - c1))*(O2 - O);
						ave_decal_length += (packed_v[3 * v + 2] - O).length();
						nb_intersections++;
						break;
					}

					// if (packed_v[3 * v + 2].length2() != 0) break; //[Bruno: never executed, there is the "break" before].
				}
			}
			ave_decal_length /= double(nb_intersections);

			FOR(v, nbv) {
				if (packed_v[3 * v + 2].length2() == 0) {
					plop("panic mode no intersection found");
					packed_v[3 * v + 2] = packed_v[3 * v + 0] - ave_decal_length * normalize(packed_v[3 * v + 1]);
				}
			}



			// vertices on border must match the contour
			FOR(c, contour.size()) packed_v[3 * local_id[qem.vertex(qem.next(contour[c]))] + 2] = X(m)[qem.vertex(contour[c])];


			// generate hexes
			index_t off_c = newhex->cells.create_hexes(in_facets.size());
			index_t off_v = newhex->vertices.create_vertices(2 * nbv);
			FOR(lv, nbv) {
				X(newhex)[off_v + 2 * lv] = packed_v[3 * lv];
				X(newhex)[off_v + 2 * lv + 1] = packed_v[3 * lv + 2];
			}
			FOR(i, in_facets.size()) {
				index_t f = in_facets[i];
				newhex->cells.set_vertex(off_c + i, 0, off_v + 2 * local_id[qem.vertex(4 * f)]);
				newhex->cells.set_vertex(off_c + i, 1, off_v + 2 * local_id[qem.vertex(4 * f + 1)]);
				newhex->cells.set_vertex(off_c + i, 2, off_v + 2 * local_id[qem.vertex(4 * f + 3)]);
				newhex->cells.set_vertex(off_c + i, 3, off_v + 2 * local_id[qem.vertex(4 * f + 2)]);
				newhex->cells.set_vertex(off_c + i, 4, off_v + 1 + 2 * local_id[qem.vertex(4 * f)]);
				newhex->cells.set_vertex(off_c + i, 5, off_v + 1 + 2 * local_id[qem.vertex(4 * f + 1)]);
				newhex->cells.set_vertex(off_c + i, 6, off_v + 1 + 2 * local_id[qem.vertex(4 * f + 3)]);
				newhex->cells.set_vertex(off_c + i, 7, off_v + 1 + 2 * local_id[qem.vertex(4 * f + 2)]);
			}
			plop("gna");
			FOR(c, contour.size())      to_kill[qem.face(contour[c])] = true;
			FOR(i, in_facets.size())    to_kill[in_facets[i]] = true;
			FOR(i, out_facets.size())   to_kill[out_facets[i]] = true;
			FOR(v, m->vertices.nb()) local_id[v] = NOT_AN_ID; //not clearly needed, but brainless
		

		}
		m->facets.delete_elements(to_kill);
	}


    /* [BL unused]
	static void evaluate_edges_valence(Mesh* m) {
		Attribute<int> edge_angle(m->facet_corners.attributes(), "edgeangle");
		FOR(f, m->facets.nb()) FOR(h, 4) edge_angle[m->facets.corner(f, h)] = rand() % 3;

	}
    */
    
	static bool next_crunch(Mesh* m, Mesh* newhex, int nb_max_punch) {
		GEO::Logger::out("HexDom")  << "next_crunch" <<  std::endl;
		//newhex->clear();

		if (m->facets.nb() == 0) return false;
		//static index_t nb_splits = 0;

		static index_t iter = index_t(-1);

		if (iter == index_t(-1)) {
			iter = 0;
		}
		else {
			iter++;
		}

		//if (iter == 0) evaluate_edges_valence(m);


		plop(iter);

		bool did_something = false;
		while (nb_max_punch > 0) {

			plop(nb_max_punch);
			GEO::Logger::out("HexDom")  << "Try to Punch" <<  std::endl;
			VertexPuncher punch(m, newhex);
			if (punch.apply(nb_max_punch)) {
				//check_no_intersecting_faces(m,true);
				did_something = true;
				if (nb_max_punch <= 0) return true;
			}
			else {
				//plop(did_something);
				if (did_something) return true;
				break;
			}
		}



		GEO::Logger::out("HexDom")  << "Try to cut" <<  std::endl;
		static int cutit = 0;
		CutSingularity cut(m);
		if (cut.apply()) {
			GEO::Logger::out("HexDom")  << "------------------------" <<  std::endl;
			GEO::Logger::out("HexDom")  << "CUT success... doing a small laplacian smoothing to separate collocated vertices" <<  std::endl;
			plop(cutit);
			cutit++;
			//return false; // DEBUG TO REMOVE
			//Attribute<vec3> real_geometry(m->vertices.attributes(), "real_geometry");
			//FOR(v, m->vertices.nb()) geo_assert((real_geometry[v] - X(m)[v]).length2()<1e-15);
			//return false;
			//check_no_intersecting_faces(m,true);
			return true;
		}

		Attribute<vec3> real_geometry(m->vertices.attributes(), "real_geometry");
		FOR(v, m->vertices.nb()) real_geometry[v] = X(m)[v];
		
		GEO::Logger::out("HexDom")  << "------------------------" <<  std::endl;
		GEO::Logger::out("HexDom")  << "CUT FAILED" <<  std::endl;
		remove_scabs(m, newhex);
		return false;
	}



	void punch_and_cut(Mesh* m, Mesh* newhex, int nb_iter, int nb_punch_per_iter, bool check_validity) {
		FOR(i, nb_iter) {
			m->edges.clear();
			GEO::Logger::out("HexDom")  << "iteration " << i << " / " << nb_iter << "  with " << nb_punch_per_iter << "  punch per iter" <<  std::endl;
			if (!next_crunch(m, newhex, nb_punch_per_iter))break;
			if (check_validity) {
				m->edges.clear();
			}
		}
	}



	void bourrin_quadrangulate_facets(Mesh* m) {
		if (m->facets.nb() == 0) return;
		Attribute<bool> isquad(m->facets.attributes(), "isquad");
		index_t nb_facets = m->facets.nb();
		FOR(f, nb_facets) {
			index_t nbv = m->facets.nb_vertices(f);
			vec3 bary(0, 0, 0);
			FOR(fv, nbv) bary = bary + (1. / double(nbv))*X(m)[m->facets.vertex(f, fv)];
			index_t nb_v = m->facets.nb_corners(f);
			FOR(lc, nb_v) {
				vec3 v[3] = {
					X(m)[m->facets.vertex(f, prev_mod(lc, nb_v))],
					X(m)[m->facets.vertex(f, lc)],
					X(m)[m->facets.vertex(f, next_mod(lc, nb_v))]
				};
				index_t off_v = m->vertices.create_vertices(4);
				X(m)[off_v] = bary;
				X(m)[off_v + 1] = 0.5*(v[0] + v[1]);
				X(m)[off_v + 2] = v[1];
				X(m)[off_v + 3] = 0.5*(v[1] + v[2]);
				isquad[m->facets.create_quad(off_v + 0, off_v + 1, off_v + 2, off_v + 3)] = (nb_v == 4);
			}
		}
		{
			vector<index_t> to_kill(m->facets.nb(), false);
			FOR(f, nb_facets) to_kill[f] = true;
			m->facets.delete_elements(to_kill);
		}

		// merge vertices
		{
			vector<index_t> to_kill(m->vertices.nb(), 0);
			vector<index_t> old2new(m->vertices.nb());
			Geom::colocate(m->vertices.point_ptr(0), 3, m->vertices.nb(), old2new, 1e-15);
			FOR(f, m->facets.nb()) FOR(fv, 4) m->facets.set_vertex(f, fv, old2new[m->facets.vertex(f, fv)]);
			FOR(v, m->vertices.nb()) if (old2new[v] != v) to_kill[v] = NOT_AN_ID;
			m->vertices.delete_elements(to_kill);
		}


	}

	void bourrin_subdivide_hexes(Mesh* m) {
		if (m->cells.nb() == 0) return;
		index_t nb_cells = m->cells.nb();
		index_t off_v = m->vertices.create_vertices(64 * nb_cells);
		index_t off_c = m->cells.create_hexes(8 * nb_cells);
		FOR(c, nb_cells) FOR(nc, 8)FOR(nv, 8)
			m->cells.set_vertex(off_c + 8 * c + nc, nv, off_v + 64 * c + 8 * nc + nv);
		FOR(c, nb_cells) {
			vec3 pts[8];
			FOR(cv, 8) pts[cv] = X(m)[m->cells.vertex(c, cv)];

			FOR(nci, 2)FOR(ncj, 2)FOR(nck, 2) {				// new cell position in cell c

				FOR(nvi, 2)FOR(nvj, 2)FOR(nvk, 2) {			// new vertex position in new cell
					index_t nc = nci + 2 * ncj + 4 * nck;
					index_t nv = nvi + 2 * nvj + 4 * nvk;
					vec3 coeff[2];
					coeff[0] = vec3(double(nci + nvi) / 2., double(ncj + nvj) / 2., double(nck + nvk) / 2.);
					coeff[1] = vec3(1, 1, 1) - coeff[0];
					vec3 pos = vec3(0, 0, 0);
					FOR(i, 2)FOR(j, 2)FOR(k, 2) {
						pos += coeff[i][0] * coeff[j][1] * coeff[k][2] * pts[i + 2 * j + 4 * k];
					}
					X(m)[off_v + 64 * c + 8 * nc + nv] = pos;
				}

			}
		}


		vector<index_t> to_kill(m->cells.nb(), false);
		FOR(c, nb_cells) to_kill[c] = true;
		m->cells.delete_elements(to_kill);

		mesh_repair(*m, MESH_REPAIR_COLOCATE, 1e-15);

	}
	
	struct Polyline {
		void compute_param() {
			dist_to_org.resize(P.size());
			FOR(i, P.size()) {
				if (i == 0) dist_to_org[0] = 0;
				else dist_to_org[i] = dist_to_org[i - 1] + (P[i - 1] - P[i]).length();
			}
		}
		double length() { 
			geo_assert(!P.empty()); 
			if (dist_to_org.size() != P.size()) compute_param();
			return dist_to_org.back();
		}
		vec3 interpolate(double prop) {
			double d = prop*length();
			FOR(i, P.size()-1) {
				if (dist_to_org[i + 1] - dist_to_org[i] < 1e-8) return P[i];
				double c = (d - dist_to_org[i]) / (dist_to_org[i + 1] - dist_to_org[i]);
				return c*P[i] + (1. - c)*P[i + 1];
			}
			return P.back();
		}
		vector<vec3> P;
		vector<double> dist_to_org;
	};
	




	void quadrangulate_easy_boundary(Mesh* m) {
		plop("quadrangulate_easy_boundary");
		QuadExtraConnectivity qem;
		qem.init(m);
		double ave_edge_length = get_facet_average_edge_size(m);
		

		//mark charts
		index_t nb_charts = 0;
		Attribute<index_t> chart(m->facets.attributes(), "chart");
		FOR(seed, m->facets.nb()) chart[seed] = index_t(-1);
		FOR(seed, m->facets.nb()) {
		    if (chart[seed] != index_t(-1)) continue;
			if (m->facets.nb_vertices(seed)==4) continue;
			chart[seed] = nb_charts;
			vector<index_t> stack;
			stack.push_back(seed);
			while (!stack.empty()) {
				index_t f = stack.back();
				stack.pop_back();
				FOR(e, 3) {
					index_t h = 4 * f + e;
					index_t fopp = qem.face (qem.opp(h));
					if (qem.fsize(qem.opp(h)) == 4) continue;
					if (chart[fopp] != index_t(-1)) continue;
					stack.push_back(fopp);
					chart[fopp] = nb_charts;
				}
			}
			nb_charts++;
		}

		vector<vector<int> > chart_to_subcharts(nb_charts);
		vector<int> subchart_to_charts;

		//mark sub charts
		int nb_subcharts = 0;
		Attribute<int> subchart(m->facets.attributes(), "subchart");
		FOR(seed, m->facets.nb()) subchart[seed] = -1;
		FOR(seed, m->facets.nb()) {
			if (subchart[seed] != -1) continue;
			if (m->facets.nb_vertices(seed) == 4) continue;
			subchart[seed] = nb_subcharts;
			chart_to_subcharts[chart[seed]] .push_back(subchart[seed]);
			subchart_to_charts.push_back(int(chart[seed]));

			vector<index_t> stack;
			stack.push_back(seed);
			while (!stack.empty()) {
				index_t f = stack.back();
				stack.pop_back();
				vec3 n_f = facet_normal(m, f);
				FOR(e, 3) {
					index_t h = 4 * f + e;
					index_t fopp = qem.face(qem.opp(h));
					vec3 n_fopp = facet_normal(m, fopp);
					if (qem.fsize(qem.opp(h)) == 4) continue;
					if (subchart[fopp] != -1) continue;
					if (dot(n_f, n_fopp) < cos(M_PI / 4.)) continue;// hard edge detected
					stack.push_back(fopp);
					subchart[fopp] = nb_subcharts;
				}
			}
			nb_subcharts++;
		}
		// try to quadrangulate each subchart
		vector<vector<vector<index_t> > > quads(nb_charts);
		vector<vector<vector<vec3> > > pts(nb_charts);
		FOR(c, nb_charts) quads[c].resize(chart_to_subcharts[c].size());
		FOR(c, nb_charts) pts[c].resize(chart_to_subcharts[c].size());


		Attribute<int> order(m->facet_corners.attributes(), "order");
		FOR(sub, nb_subcharts) {
			
			vector<index_t> border;
			// gather halfedges in border
			{
				FOR(h, 4 * m->facets.nb()) if (qem.valid(h))
				    if (subchart[qem.face(h)] == int(sub)
					&& subchart[qem.face(qem.opp(h))] != int(sub))
					border.push_back(h);
				if (border.size() < 4) continue;
			}
			// order border halfedges
			{
				// do not start with an hardedge
				FOR(cur, border.size()) if (qem.fsize(qem.opp(border[cur])) == 4) std::swap(border[0], border[cur]);
				if (qem.fsize(qem.opp(border[0])) !=4) continue;
				//link them
				for (index_t cur = 0; cur < border.size() - 1; cur++)
					for (index_t next = cur + 1; next < border.size(); next++)
						if (qem.vertex(qem.next(border[cur])) == qem.vertex(border[next]))
							std::swap(border[cur + 1], border[next]);
			}
			// check that we have a loop
			{
				bool topodisk = true;
				for (index_t cur = 0; cur < border.size() - 1; cur++)
					topodisk = topodisk && (qem.vertex(qem.next(border[cur])) == qem.vertex(border[cur + 1]));
				if (!topodisk) { plop(topodisk); continue; }
			}
			/*DEBUG*/FOR(cur,border.size()) order[qem.corner(border[cur])] = int(cur+10);
			
			// constuct the problem.
			int chartid = subchart_to_charts[sub];
			vector<vec3> l_pts;

			vector<vec3> edge_org;
			vector<bool> hardedge;
			index_t offset = 0;
			index_t N = border.size();
			FOR(cur, N) {
				edge_org.push_back(X(m)[qem.vertex(border[cur])]);
				if (cur == 0) {
					hardedge.push_back(false);
					continue;
				}
				hardedge.push_back(qem.fsize(qem.opp(border[cur])) != 4
					&& subchart[qem.face(qem.opp(border[cur]))] == subchart[qem.face(qem.opp(border[cur - 1]))]);
				if (!hardedge.back()) offset = cur;
			}
			{vector<vec3> tmp(N); FOR(cur, N) tmp[cur] = edge_org[(cur + offset) % N]; edge_org = tmp; }
			{vector<bool> tmp(N); FOR(cur, N) tmp[cur] = hardedge[(cur + offset) % N]; hardedge = tmp; }

			index_t i = 0;
			while (i < N) {
				//plop(i);
				l_pts.push_back(edge_org[i]);
				if (!hardedge[i])
				{
					i++;
				} else {
					Polyline poly;
					while (i < N+1 && hardedge[i%N]) {
						
						poly.P.push_back(edge_org[i%N]);
						i++;
					}
					plop(i);
					plop(poly.length());
					plop(ave_edge_length);
					int nb_seg = int(2 * nint(0.5*poly.length() / ave_edge_length));
					nb_seg = std::max(1, nb_seg);
					FOR(s, nb_seg - 1) plop(poly.interpolate(double(s + 1) / double(nb_seg) - .001));
				
				}
			}
			//plop("border completed");

			vector<index_t> l_quads;
			if (Poly3d(l_pts).try_quadrangulate(l_quads)) {
				int loc_sub = -1;
				FOR(l, chart_to_subcharts[chartid].size())
				    if (chart_to_subcharts[chartid][l] == int(sub))
					loc_sub = int(l);

				FOR(p, l_pts.size()) plop(l_pts[p]);
				FOR(p, l_quads.size()) plop(l_quads[p]);

				pts[chartid][loc_sub] = l_pts;
				quads[chartid][loc_sub] = l_quads;

			}
		}

		// replace charts that have been sucessfully remeshed

		vector<bool> chart_remesh_is_ok(nb_charts, true);
		FOR(chartid, nb_charts)  FOR(lsub, pts[chartid].size())
			chart_remesh_is_ok[chartid] = chart_remesh_is_ok[chartid] && !pts[chartid][lsub].empty();
		plop("gna");

		vector<index_t> to_kill(m->facets.nb(),false);
		FOR(f, m->facets.nb()) if (chart[f]!=NOT_AN_ID) to_kill[f] = chart_remesh_is_ok[chart[f]];
		plop("gna");

		Attribute<int> kill2(m->facets.attributes(), "kill2");
		FOR(f, m->facets.nb()) kill2[f] = int(to_kill[f]);

		FOR(chartid, nb_charts) {
			if (!chart_remesh_is_ok[chartid]) continue;
			FOR(lsub, pts[chartid].size()) {
				index_t off_v = m->vertices.create_vertices(pts[chartid][lsub].size());
				FOR(p, pts[chartid][lsub].size())
					X(m)[off_v+p] = pts[chartid][lsub][p];
				index_t off_f = m->facets.create_quads(quads[chartid][lsub].size() / 4);
				FOR(q, quads[chartid][lsub].size()/4)
					FOR(lv,4) m->facets.set_vertex(off_f + q,lv,off_v+quads[chartid][lsub][4*q+lv]);
			}
		}



		to_kill.resize(m->facets.nb(),false);
		m->facets.delete_elements(to_kill);

		// merge vertices
		vector<index_t> to_kill_v(m->vertices.nb(), 0);
		vector<index_t> old2new(m->vertices.nb());
		Geom::colocate(m->vertices.point_ptr(0), 3, m->vertices.nb(), old2new, 1e-15);


		//FOR(f, m->facets.nb()) FOR(lv, m->facets.nb_vertices(f))
		//	if(old2new[m->facets.vertex(f, lv)]
		//		== old2new[m->facets.vertex(f, (lv + 1) % m->facets.nb_vertices(f))])
		//{	Attribute<int> crush(m->facets.attributes(), "crush");
		//crush[f] = 1;
		//plop(f); plop(m->facets.vertex(f, lv)); return;
		//}

		
		FOR(f, m->facets.nb()) FOR(fv, m->facets.nb_vertices(f)) m->facets.set_vertex(f, fv, old2new[m->facets.vertex(f, fv)]);
		FOR(v, m->vertices.nb()) if (old2new[v] != v) to_kill_v[v] = NOT_AN_ID;
		m->vertices.delete_elements(to_kill_v);

	}
	
	void prepare_crunch(Mesh* m, bool subdivide, bool revert) {
		if (subdivide) bourrin_quadrangulate_facets(m);
		if (revert) FOR(f, m->facets.nb()) {
			index_t save_v = m->facets.vertex(f, 0);
			m->facets.set_vertex(f, 0, m->facets.vertex(f, 2));
			m->facets.set_vertex(f, 2, save_v);
		}
		quadrangulate_easy_boundary(m);
	}
	void hex_crunch(Mesh* m, Mesh* hex) {
	        geo_argused(hex);
		check_no_intersecting_faces(m, false);
		prepare_crunch(m, false, true);
		plop("prepare_crunch done");
		plop(get_intersecting_faces(m).size());
		return;
	}


}
