#include <exploragram/hexdom/hex_candidates.h> 
#include <exploragram/hexdom/mesh_utils.h> 
#include <geogram/mesh/mesh.h>
#include <geogram/points/colocate.h>
#include <exploragram/hexdom/geometry.h>
namespace GEO {


	static vec3 change_tet_basis(vec3 in, vec3  P[4], vec3 P_img[4]) { // TODO sortir ça + trglgrad dans un fichier apart
		Matrix<12, double> M;
		M.load_zero();
		double RHS[12];
		double X[12];

		index_t cur_row = 0; // in fact cur_row can be replaced by v*3+dimx
		FOR(v, 4)  FOR(dimx, 3) {
			FOR(dimu, 3)
				M(4 * dimx + dimu, cur_row) = P[v][dimu];
			M(4 * dimx + 3, cur_row) = 1.0;
			RHS[cur_row] = P_img[v][dimx];
			cur_row++;

		}
		geo_assert(cur_row == 12);

		Matrix<12, double> inv = M.inverse();

		FOR(i, 12)            X[i] = 0;
		FOR(i, 12) FOR(j, 12) X[i] += inv(j, i)*RHS[j];

		vec3 res(0, 0, 0);
		FOR(dimx, 3)  FOR(dimu, 3)
			res[dimx] += in[dimu] * X[4 * dimx + dimu];

		FOR(dimx, 3) res[dimx] += X[4 * dimx + 3];
		return res;
	}

	static bool in_tet(vec3 test, vec3 P[4], double eps) {
		int find[4][3] = { { 1, 3, 2 },{ 3, 0, 2 },{ 0, 3, 1 },{ 0, 1, 2 } };
		if (std::abs(dot(P[3] - P[0], cross(P[2] - P[0], P[1] - P[0]))) < 1e-15) return false;// check for flat tet
		FOR(f, 4) {
			vec3 vA = P[find[f][0]];
			vec3 vB = P[find[f][1]];
			vec3 vC = P[find[f][2]];
			vec3 n = cross(vB - vA, vC - vA);
			n = normalize(n);
			if (dot(test - vA, n) > eps) return false;
		}
		return true;
	}

	static void get_grid_vertices(Mesh* m, Attribute<vec3>& UC, index_t t, std::vector<vec3>& psetX, std::vector<vec3>& psetU, bool dual) {
		// compute a bbox in the parametric domain
		int bbox[2][3] = { { 1000000, 1000000, 1000000 },{ -1000000, -1000000, -1000000 } };
		FOR(i, 4) FOR(dim, 3) {
			bbox[0][dim] = std::min(bbox[0][dim], int(std::floor(UC[m->cells.corner(t, i)][dim])) - 1);
			bbox[1][dim] = std::max(bbox[1][dim], int(std::ceil(UC[m->cells.corner(t, i)][dim])) + 1);
		}

		vec3 lX[4], lU[4];
		FOR(i, 4) {
			lX[i] = m->vertices.point(m->cells.vertex(t, i));
			lU[i] = UC[m->cells.corner(t, i)];
		}

		// raster the bbox and project to geometric space points included in the parametric space tet
		for (int i0 = bbox[0][0]; i0 <= bbox[1][0]; i0++) {
			for (int i1 = bbox[0][1]; i1 <= bbox[1][1]; i1++) {
				for (int i2 = bbox[0][2]; i2 <= bbox[1][2]; i2++) {
					vec3 testpt(i0, i1, i2);
					if (dual) testpt += vec3(.5, .5, .5);
					if (in_tet(testpt, lU, .001)) {
						psetX.push_back(change_tet_basis(testpt, lU, lX));
						psetU.push_back(testpt);
					}
				}
			}
		}
	}




	void export_points(Mesh* m, Mesh* pts) {
		Attribute<bool> has_param(m->cell_facets.attributes(), "has_param");
		Attribute<vec3> UC(m->cell_corners.attributes(), "U");

		FOR(c, m->cells.nb()) {
			if (c % 100 == 0) GEO::Logger::out("HexDom")  << " EXPORT point set: tet" << c << " / " << m->cells.nb() <<  std::endl;
			if (!has_param[m->cells.facet(c, 0)] || !has_param[m->cells.facet(c, 1)] || !has_param[m->cells.facet(c, 2)] || !has_param[m->cells.facet(c, 3)]) continue;

			std::vector<vec3> seedU;
			std::vector<vec3> seedX;
			get_grid_vertices(m, UC, c, seedX, seedU, false);
			index_t off = pts->vertices.create_vertices(index_t(seedX.size()));
			FOR(lv, seedX.size()) X(pts)[off + lv] = seedX[lv];
		}

		// remove colocated vertices
		double eps = (1e-2)*get_cell_average_edge_size(m);
		vector<index_t> to_kill(pts->vertices.nb(), 0);
		vector<index_t> old2new(pts->vertices.nb());
		Geom::colocate(pts->vertices.point_ptr(0), 3, pts->vertices.nb(), old2new, eps);
		FOR(v, pts->vertices.nb()) if (old2new[v] != v) to_kill[v] = NOT_AN_ID;
		plop(pts->vertices.nb());
		pts->vertices.delete_elements(to_kill);
		plop(pts->vertices.nb());

	}
	inline bool intersect_unit_box(vec3 p_boxcenter, vec3 tri[3]) {
		float boxcenter[3] = { float(p_boxcenter[0]), float(p_boxcenter[1]), float(p_boxcenter[2]) };
		float boxhalfsize[3] = { .5, .5, .5 };
		float triverts[3][3] = {
			{ float(tri[0][0]), float(tri[0][1]), float(tri[0][2]) },
			{ float(tri[1][0]), float(tri[1][1]), float(tri[1][2]) },
			{ float(tri[2][0]), float(tri[2][1]), float(tri[2][2]) }
		};
		return (triBoxOverlap(boxcenter, boxhalfsize, triverts) != 0);
	}

	inline void init_centered_unit_cube_face_bary(vec3 *cube_face_bary) {
		vec3 cubeU[8];
		FOR(k, 2)FOR(j, 2)FOR(i, 2)
			cubeU[4 * i + 2 * j + k] = vec3(i, j, k) - vec3(.5, .5, .5);
		CellDescriptor hexdescr = MeshCellsStore::cell_type_to_cell_descriptor(MESH_HEX);
		FOR(lf, 6) {
			cube_face_bary[lf] = vec3(0, 0, 0);
			for (int lv = 0; lv < 4; lv++) cube_face_bary[lf] += .5 * cubeU[hexdescr.facet_vertex[lf][lv]];
		}
	}
    
        static bool cell_has_param(Mesh* m, Attribute<bool>& has_param, index_t c) {
		FOR(f, 4) if (!has_param[m->cells.facet(c, f)]) return false;
		return true;
	}

	static bool param_is_degenerated(Mesh* m, Attribute<vec3>& UC, index_t tet) {
		vec3 P[4];
		FOR(i, 4)  P[i] = UC[m->cells.corner(tet, i)];
		return (dot(P[3] - P[0], cross(P[2] - P[0], P[1] - P[0])) == 0);
	}


	static void make_neig_tet_compatible(Mesh* m, Attribute<vec3>& UC, index_t ref_tet, index_t cf) {
		{// preconditions
			Attribute<bool> has_param(m->cell_facets.attributes(), "has_param");
			geo_assert(cell_has_param(m, has_param, ref_tet));
			geo_assert(m->cells.adjacent(ref_tet, cf) != NOT_AN_ID);
			geo_assert(cell_has_param(m, has_param, m->cells.adjacent(ref_tet, cf)));
		}

		index_t opp_tet = m->cells.adjacent(ref_tet, cf);
		index_t ABC[3];
		FOR(cfv, 3) ABC[cfv] = m->cells.facet_vertex(ref_tet, cf, cfv);

		index_t ref_c[3];
		index_t opp_c[3];

		index_t Dc = NOT_AN_ID;


		FOR(opp_corner, 4) {
			index_t v = m->cells.vertex(opp_tet, opp_corner);
			bool match_found = false;
			FOR(i, 3) if (ABC[i] == v) {
				opp_c[i] = m->cells.corner(opp_tet, opp_corner);
				match_found = true;
			}
			if (!match_found) {
				Dc = m->cells.corner(opp_tet, opp_corner);
			}
		}
		geo_assert(Dc != NOT_AN_ID);
		FOR(i, 3) ref_c[i] = cell_facet_corner_id(m, ref_tet, cf, i);

		vec3 ref_xyz[3];
		ref_xyz[0] = normalize(UC[ref_c[1]] - UC[ref_c[0]]);
		ref_xyz[1] = normalize(UC[ref_c[2]] - UC[ref_c[0]]);
		ref_xyz[2] = normalize(cross(ref_xyz[0], ref_xyz[1]));
		ref_xyz[1] = normalize(cross(ref_xyz[2], ref_xyz[0]));

		vec3 opp_xyz[3];
		opp_xyz[0] = normalize(UC[opp_c[1]] - UC[opp_c[0]]);
		opp_xyz[1] = normalize(UC[opp_c[2]] - UC[opp_c[0]]);
		opp_xyz[2] = normalize(cross(opp_xyz[0], opp_xyz[1]));
		opp_xyz[1] = normalize(cross(opp_xyz[2], opp_xyz[0]));

		vec3 local;
		FOR(d, 3)local[d] = dot(opp_xyz[d], UC[Dc] - UC[opp_c[0]]);

		vec3 save = UC[Dc];
		UC[Dc] = UC[ref_c[0]];
		FOR(d, 3) UC[Dc] = UC[Dc] + local[d] * ref_xyz[d];

		FOR(v, 3) UC[opp_c[v]] = UC[ref_c[v]];


		//assumes that two coordinates are never (non integer and closer than 1e-8)
		FOR(d, 3)  FOR(ds, 3) {
			if (std::abs((UC[Dc][d] - round(UC[Dc][d])) - (save[ds] - round(save[ds]))) < 1e-8) {
				UC[Dc][d] = round(UC[Dc][d]) + save[ds] - round(save[ds]);
			}
			if (std::abs((UC[Dc][d] - round(UC[Dc][d])) + (save[ds] - round(save[ds]))) < 1e-8) {
				UC[Dc][d] = round(UC[Dc][d]) - save[ds] + round(save[ds]);
			}
		}

		//FOR(d, 3)  if (std::abs(UC[Dc][d] - round(UC[Dc][d])) <.05) UC[Dc][d] = round(UC[Dc][d]);

	}

	void export_hexes(Mesh* m, Mesh* hex) {
		Attribute<bool> has_param(m->cell_facets.attributes(), "has_param");
		Attribute<vec3> UC(m->cell_corners.attributes(), "U");

		vec3 cube_face_bary[6];
		init_centered_unit_cube_face_bary(cube_face_bary);



		FOR(c, m->cells.nb()) {

			if (c % 1000 == 0) GEO::Logger::out("HexDom")  << " EXPORT HEXES  tet = " << c << " / " << m->cells.nb() <<  std::endl;
			if (!cell_has_param(m, has_param, c)) continue;
			if (param_is_degenerated(m, UC, c)) continue;
			std::vector<vec3> seedU; // contains centers of all cubes inside tet c
			{// init seedU
				std::vector<vec3> trash;
				get_grid_vertices(m, UC, c, trash, seedU, true);
			}
			FOR(sid, seedU.size()) {
				vec3 ptsU[8]; // vertices of the cube centered in seedU[sid]
				vec3 ptsX[8];
				bool ptsdone[8] = {};
				bool have_boundary_face[6] = {};
				FOR(k, 2) FOR(j, 2) FOR(i, 2)
					ptsU[4 * i + 2 * j + k] = seedU[sid] - vec3(.5, .5, .5) + vec3(i, j, k);


				std::vector<index_t> tet_stack;
				tet_stack.push_back(c);
				std::vector<index_t> tet_done;
				bool has_sing_tet = false;

				while (!tet_stack.empty()) {
					index_t curt = tet_stack.back(); tet_stack.pop_back();

					// check if tet is singular
					if (!cell_has_param(m, has_param, curt)) { has_sing_tet = true; break; }


					// create local geometry
					vec3 lX[4], lU[4];
					FOR(i, 4) {
						lX[i] = m->vertices.point(m->cells.vertex(curt, i));
						lU[i] = UC[m->cells.corner(curt, i)];
					}

					// find cubes corners located inside the current tet
					FOR(i, 8) if (in_tet(ptsU[i], lU, 1e-5)) {
						ptsX[i] = change_tet_basis(ptsU[i], lU, lX);
						ptsdone[i] = true;
					}

					FOR(lf, 4) {
						index_t corners[3] = {
							cell_facet_corner_id(m, curt, lf, 0),
							cell_facet_corner_id(m, curt, lf, 1),
							cell_facet_corner_id(m, curt, lf, 2)
						};
						//index_t corners[3];
						//FOR(v, 3) FOR(corn, 4) if (verts[v] == m->cell_corners.vertex(m->cells.corner(curt, corn)))
						//	corners[v] = m->cells.corner(curt, corn);


						vec3 tri[3] = { UC[corners[0]], UC[corners[1]], UC[corners[2]] };

						index_t oppt = m->cells.adjacent(curt, lf);
						if (oppt == NO_CELL) {
							// if the facet matches a unit cube facet in U coordinates, mark this unit cube facet
							vec3 face_normal = normalize(cross(tri[1] - tri[0], tri[2] - tri[0]));
							FOR(cubef, 6)
								if ((face_normal - cube_face_bary[cubef]).length2() < 1e-10
									&&  round(.5 + dot(cube_face_bary[cubef], tri[0] - seedU[sid])) == 1) {
									have_boundary_face[cubef] = true;
									// need to check that it is not a concave hardedge
									FOR(cubef_in, 6) {
										if (cubef == cubef_in) continue;

										bool all_are_outside = true;
										FOR(d, 3) {
											all_are_outside = all_are_outside &&
												(.5 + dot(cube_face_bary[cubef_in], tri[d] - seedU[sid]) > 1 - 1e-5);
										}
										have_boundary_face[cubef] = have_boundary_face[cubef] && !all_are_outside;
									}
								}
							continue;
						}

						bool alreadydone = false;
						FOR(p, tet_done.size()) alreadydone = alreadydone || (tet_done[p] == oppt);
						if (alreadydone) continue;

						if (intersect_unit_box(seedU[sid], tri)) {
							if (cell_has_param(m, has_param, oppt) && !param_is_degenerated(m, UC, oppt)) {
								make_neig_tet_compatible(m, UC, curt, lf);
								tet_stack.push_back(oppt);
							}
							else {
								has_sing_tet = true;
								break;
							}
						}
					}
					tet_done.push_back(curt);
				}
				bool all_vertices_found = true;
				FOR(i, 8) all_vertices_found = all_vertices_found && ptsdone[i];
				if (all_vertices_found && !has_sing_tet) {

					index_t off = hex->vertices.create_vertices(8);
					hex->cells.create_hex(off, off + 1, off + 2, off + 3, off + 4, off + 5, off + 6, off + 7);
					FOR(i, 8)      hex->vertices.point(off + i) = ptsX[i];
				}
			}
		}
	}

}
