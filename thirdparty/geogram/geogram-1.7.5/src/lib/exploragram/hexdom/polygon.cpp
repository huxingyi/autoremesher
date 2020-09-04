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

#include <exploragram/hexdom/polygon.h>
#include <geogram/NL/nl.h>
namespace GEO {

	vec2 Poly2d::barycenter() {
		vec2 bary(0, 0);
		FOR(fv, index_t(pts.size())) {
			bary = bary + (1. / double(pts.size()))*pts[fv];
		}
		return bary;
	}

	static int dump_contour_save_id = 0;
	void Poly2d::dump_contour() {
		index_t nbv = pts.size();
		Mesh export_mesh;
		export_mesh.vertices.create_vertices(nbv);
		FOR(i, nbv) X(&export_mesh)[i] = vec3(pts[i][0], pts[i][1], 0);
		vector<index_t> num;
		FOR(i, nbv) num.push_back(i);
		export_mesh.facets.create_polygon(num);
		char filename[1024];
		sprintf(filename, "C:/DATA/2dcontours/nimp2D%i.obj", dump_contour_save_id++);
		mesh_save(export_mesh, filename);
	}	
	
	void Poly3d::dump_contour() {
		index_t nbv = pts.size();
		Mesh export_mesh;
		export_mesh.vertices.create_vertices(nbv);
		FOR(i, nbv) X(&export_mesh)[i] = vec3(pts[i][0], pts[i][1], pts[i][2] );
		vector<index_t> num;
		FOR(i, nbv) num.push_back(i);
		export_mesh.facets.create_polygon(num);
		char filename[1024];
		sprintf(filename, "C:/DATA/2dcontours/nimp3D%i.obj", dump_contour_save_id++);
		mesh_save(export_mesh, filename);
	}

	// returns 1024. if concave angle is encountered or if proposed triangle contains one of pts
	// otherwise returns max angle of the proposed triangle
	double Poly2d::cost(index_t i, index_t j, index_t k) {
		vec2 C[3] = { pts[i], pts[j], pts[k] };
		double m = 0;
		FOR(v, 3) {
			// note that angle is not the angle inside the triangle, but its complement
			// angle variable has the "direction" information, thus it is negative for concave angles (right turn) and positive for convex angles (left turn)
			double angle = atan2(
				det(C[(v + 1) % 3] - C[(v + 0) % 3], C[(v + 2) % 3] - C[(v + 1) % 3]),
				dot(C[(v + 1) % 3] - C[(v + 0) % 3], C[(v + 2) % 3] - C[(v + 1) % 3])
			);
			if (angle <= 0) return 1024.;
			m = std::max(m, M_PI - angle);
		}

		FOR(other, pts.size()) { // TODO c'est con de faire ça, vaut mieux regarder si le triangle est inversé (ça ne gere pas tout [comme, d'ailleurs, le teste courant!])
			if (other == i || other == j || other == k) continue;
			vec2 P = pts[other];
			bool inside = true;
			FOR(l, 3) {
				inside = inside && (det(normalize(C[(l + 1) % 3] - C[l]), normalize(P - C[l])) > 0);
			}
			if (inside) return 1024.;
		}
		return m;
	}

    // this function has O(n^4) computational cost
	bool Poly2d::try_triangulate_minweight(vector<index_t>& triangles) {
		triangles.clear();
		index_t n = pts.size();
		geo_assert(n >= 3);

		//if (n == 3) {
		//    FOR(v, 3) {
		//        triangles.push_back(v);
		//    }
		//    return true;
		//}

		// we store in this table results of subproblems
		// table[i*n + j] stores the triangulation cost for points from i to j
		// the entry table[0*n + n-1] has the final result.
		std::vector<double> table(n*n, 0.);

		// this table stores triangle indices: for each subproblem (i,j) we have table[i*n + j]==k, i.e. the triangle is (i,k,j)
		std::vector<index_t> tri(n*n, index_t(-1));

		// note that the table is filled in diagonals; elements below main diagonal are not used at all
		for (index_t pbsize = 2; pbsize < n; pbsize++) {
			for (index_t i = 0, j = pbsize; j < n; i++, j++) {
				// recall that we are testing triangle (i,k,j) which splits the problem (i,j) into
				// two smaller subproblems (i,k) and (k,j)
				double minv = 1e20;
				index_t mink = index_t(-1);
				for (index_t k = i + 1; k < j; k++) {
					double val = table[i*n + k] + table[k*n + j] + cost(i, k, j);
					if (minv <= val) continue;
					minv = val;
					mink = k;
				}
                geo_assert(mink!=index_t(-1));
				table[i*n + j] = minv;
				tri[i*n + j] = mink;
			}
		}

//		if (table[n-1] >= 1024.) return false;

		vector<index_t> Q(1, n - 1);
		FOR(t, Q.size()) {
			index_t idx = Q[t];

			index_t i = idx / n;
			index_t k = tri[idx];
			index_t j = idx % n;

			geo_assert(i!=index_t(-1) && k != index_t(-1) && j!=index_t(-1));
			triangles.push_back(i);
			triangles.push_back(k);
			triangles.push_back(j);

			if (k + 2 <= j) Q.push_back(k*n + j);
			if (i + 2 <= k) Q.push_back(i*n + k);
		}

		if (table[n - 1] >= 1024.) {
			plop("may dump_contour for debug...");//dump_contour();
		}
		return table[n-1] < 1024.;
	}


	// find parity of original points
	index_t Poly2d::parity_of_original_points() {
		index_t nbv = pts.size();
		index_t dec = 0;
		double dec_score[2] = { 0, 0 };
		FOR(q, nbv / 2) {
			FOR(d, 2)
			    dec_score[d] = std::max(dec_score[d], std::abs(
					det(pts[(q * 2 + 0 + d) % nbv] - pts[(q * 2 + 1 + d) % nbv],
						pts[(q * 2 + 2 + d) % nbv] - pts[(q * 2 + 1 + d) % nbv])));
		}
		if (dec_score[1] < dec_score[0])dec = 1;
		return dec;
	}


	bool Poly2d::middle_point_quadrangulate(vector<index_t>& quads) {
		index_t nbv = pts.size();
		vec2 G = barycenter();
		index_t dec = parity_of_original_points();
		FOR(q, nbv / 2) {
			quads.push_back(nbv);
			FOR(v, 3) quads.push_back((q * 2 + 1 - dec + v) % nbv);
		}
		pts.push_back(G);
		return true;
	}


	bool Poly2d::quads_are_valid(vector<index_t>& quads) {
		
		// geometric criteria
		FOR(q, quads.size() / 4) {
			FOR(e, 4) {
				vec2 v0 = normalize(pts[quads[4 * q + next_mod(e, 4)]] - pts[quads[4 * q + e]]);
				vec2 v1 = normalize(pts[quads[4 * q + prev_mod(e, 4)]] - pts[quads[4 * q + e]]);
				if (det(v0, v1) < sin(M_PI / 8.)) return false;
			}
		}
		return true;
	}



	struct Contour2D {
		void resize(index_t n) { pos_.resize(n); angu_.resize(n);vid_.resize(n);}
		void compute_angu() {
			//plop("compute_angu() will not sffice to capture sing 5");
			angu_.resize(pos_.size());
			FOR(v, pos_.size()) {
				angu_[v] = 1;
				vec2 d0 = pos(v) - pos(int(v)-1);
				vec2 d1 = pos(v+1) - pos(v);
				double angle = atan2(det(d0, d1), dot(d0, d1));
				if (angle < M_PI / 4.) angu_[v] = 0;
				if (angle < -M_PI / 4.) angu_[v] = -1;
			}
		}

		vec2 normal(int v) {// equals 0 for the singularity
			mat2 R90; R90(0, 0) = 0;	R90(0, 1) = -1; R90(1, 0) = 1;	R90(1, 1) = 0;
			return normalize(R90*(pos(v+1) - pos(v - 1)));
		}

		vec2& pos(int i) { return aupp(i, pos_); }
		int& angu(int i) { return aupp(i, angu_); }
		int& vid(int i) { return aupp(i, vid_); }
	    
		vec2& pos(index_t i) { return aupp(i, pos_); }
		int& angu(index_t i) { return aupp(i, angu_); }
		int& vid(index_t i) { return aupp(i, vid_); }
	    

		void show() {
			GEO::Logger::out("HexDom")  << "\npos.size = " << pos_.size() <<  std::endl; FOR(i, pos_.size()) std::cerr << pos_[i] << "\t";
			GEO::Logger::out("HexDom")  << "\nangu.size = " << angu_.size() <<  std::endl; FOR(i, angu_.size()) std::cerr << angu_[i] << "\t";
			GEO::Logger::out("HexDom")  << "\nvid.size = " << vid_.size() <<  std::endl; FOR(i, vid_.size()) std::cerr << vid_[i] << "\t";
		}
		
		void remove(int i) {
		        i = i%int(pos_.size());
			pos_.erase(pos_.begin() + i);
			angu_.erase(angu_.begin() + i);
			vid_.erase(vid_.begin() + i);

		}
	    

		vector<vec2> pos_;
		vector<int> angu_;
		vector<int> vid_;
	};

	static int export_debug_mesh_id = 0;
	struct QuadrangulateWithOneSingularity {
		QuadrangulateWithOneSingularity(vector<vec2>& p_pts, vector<index_t>& p_quads) 
			:pts(p_pts), quads(p_quads) {
			R90(0, 0) = 0;	R90(0, 1) = -1; R90(1, 0) = 1;	R90(1, 1) = 0;
		}

		// returns the index of the singularity
		int init_contour(vector<int>& angu,int sing_valence) {
		        index_t offset = 0;
			// find the best offset
			double best_dist2 = 1e20;
			contour.resize(pts.size()+1);
			contour.pos(0) = vec2(0, 0);
			FOR(off, pts.size()) {
				vec2 dir(1, 0);
				FOR(v, pts.size()) {
					contour.pos(v + 1) = contour.pos(v) + dir;
					if (aupp(off + v + 1, angu) < 0) dir = -(R90*dir);
					if (aupp(off + v + 1, angu) > 0) dir = R90*dir;
				}
				vec2 diag = contour.pos(-1) ;

				if (sing_valence == 3 && diag.x == -diag.y && diag.length2() < best_dist2) {
					best_dist2 = diag.length2();
					offset = off;
				};
				if (sing_valence == 5 && diag.x == diag.y && diag.length2() < best_dist2) {
					best_dist2 = diag.length2();
					offset = off;
				};
			}
			// decal gridpos w.r.t offset
			vec2 dir(1, 0);
			FOR(v, pts.size()) {
				contour.pos(v + 1) = contour.pos(v) + dir;
				if (aupp(offset + v + 1, angu) < 0) dir = -(R90*dir);
				if (aupp(offset + v + 1, angu) > 0) dir = R90*dir;
			}

			// define mapping contour -> pts 
			FOR(v, pts.size() )  contour.vid(v ) = int(offset + v ) % int(pts.size());
			contour.vid_.back() = contour.vid_.front();
			
			// singularity on border   TODO CHECK angu on singularity !
			if ((contour.pos(-1) - contour.pos(0)).length2() < .1) { contour.remove(int(contour.pos_.size()) - 1); contour.compute_angu(); return 0; }

			// add pts 
			vec2 A = contour.pos(0);
			vec2 B = contour.pos(-1);
			if (sing_valence == 3) for (int i = int(B.x + 1.0); i < int(A.x); i++) {
				contour.pos_.push_back(vec2(i, B.y));	
				contour.vid_.push_back(int(pts.size()));
				pts.push_back(contour.pos_.back());
			}

			if (sing_valence == 5) for (int i = int(B.x - 1); i > int(A.x); i--) {
				contour.pos_.push_back(vec2(double(i), B.y));
				contour.vid_.push_back(int(pts.size()));
				pts.push_back(contour.pos_.back());
			}

			index_t singularity_index = contour.pos_.size();
			contour.pos_.push_back(vec2(A.x, B.y));
			contour.vid_.push_back(int(pts.size()));
			pts.push_back(contour.pos_.back());
			
			for (int j = int(B.y - 1); j > int(A.y); j--) {
			    contour.vid_.push_back(contour.vid(2*singularity_index-contour.pos_.size()));
				contour.pos_.push_back(vec2(A.x, double(j)));
			}
			contour.compute_angu();
			contour.angu(singularity_index) = 2 - sing_valence;
			return int(singularity_index);
		}

		
		void export_debug_mesh() {
			Mesh outm;
			outm.vertices.create_vertices(contour.pos_.size());
			vector<index_t> vid(contour.pos_.size());
			FOR(i, contour.pos_.size()) {
				vid[i] = i;
				X(&outm)[i] = vec3(contour.pos(i)[0], contour.pos(i)[1], 0);
			}
			Attribute<int> angu_attr(outm.vertices.attributes(), "angu");
			FOR(i, contour.pos_.size()) angu_attr[i] = contour.angu(i);

			outm.facets.create_polygon(vid);
			mesh_save(outm, "C:/DATA/2dcontours/contour2D"+ String::to_string(export_debug_mesh_id ++) +".geogram");
		}


		bool try_to_punch() {
			if (contour.pos_.size() < 4) return false;
			FOR(v, contour.pos_.size()) {
				if (contour.angu(v) != 1) continue;
				// cut ear
				if (contour.angu(v + 1) == 1) {
				    FOR(s, 4) quads.push_back(index_t(contour.vid(int(v) - 1 + int(s))));
					contour.remove(int(v));
					contour.remove(int(v));
					contour.angu(int(v) - 1)++;
					contour.angu(int(v)) ++;
					return true;
				}
				// add new point
				vec2 npos = contour.pos(v - 1) + contour.pos(v + 1) - contour.pos(v);
				bool conflict = false;
				FOR(vv, contour.pos_.size()) if (vv != v && (contour.pos(vv) - npos).length2() < .1) conflict = true;
				if (conflict) continue;
				FOR(s,3) quads.push_back(index_t(contour.vid(int(v) - 1+int(s))));
				quads.push_back(index_t(pts.size()));
				contour.vid(v) = int(pts.size());
				pts.push_back(npos);
				contour.pos(v) = npos;
				contour.angu(v-1) ++;
				contour.angu(v) = -1;
				contour.angu(v+1) ++;
				return true;
			}
			return false;
		}


		bool apply(vector<int>& angu,int sing_valence) {
			int singularity_index=-1;
			index_t border_size = pts.size();
			if (sing_valence == 3|| sing_valence == 5) {
				singularity_index = init_contour(angu, sing_valence);
			} else if (sing_valence == 4) {
				contour.resize(pts.size());
				vec2 dir(1, 0);
				contour.pos(0) = vec2(0, 0);
				for (index_t v = 1; v < pts.size();v++) {
					contour.pos(v) = contour.pos(v - 1) + dir;
					if (angu[v]< 0) dir = -(R90*dir);
					if (angu[v]> 0) dir = R90*dir;
				}
				FOR(v, pts.size()) contour.vid(v) = int(v);
				FOR(v, pts.size()) contour.angu(v) = angu[v];
				if ((contour.pos_.back() + dir - contour.pos_.front()).length2() > .1) return false; // check that it is closed
			}
			else {
				return false;
			}

			//plop("valok");
			//plop(sing_valence);
			//plop(singularity_index);
			//plop(border_size);
			//plop(contour.pos_.size());
			vector<vec2> theta_r(border_size);
			vec2 O(.5, .5);
			if (singularity_index!=-1) O= contour.pos(singularity_index);
			FOR(v, border_size) theta_r[v][1] = (contour.pos(v) - O).length();
			theta_r[0][0] = 0;
			FOR(v, border_size - 1) {
				theta_r[v + 1][0] = theta_r[v][0] + atan2(det(contour.pos(v) - O, contour.pos(v + 1) - O), dot(contour.pos(v) - O, contour.pos(v + 1) - O));
			}

			FOR(v, border_size) FOR(vv, border_size) if (vv != v && (theta_r[vv] - theta_r[v]).length2() < .0001) {
				FOR(w, theta_r.size()) plop(theta_r[w]);
				return false;
			}
			

			while (try_to_punch()) {
				//export_debug_mesh();
				//plop(export_debug_mesh_id);
				if (quads.size() > 1000) geo_assert_not_reached;
			}

			nlNewContext();
			nlSolverParameteri(NL_LEAST_SQUARES, NL_TRUE);
			nlSolverParameteri(NL_NB_VARIABLES, NLint(2*pts.size()));
			nlBegin(NL_SYSTEM);
			FOR(v, border_size) FOR(d,2){
				nlSetVariable(v * 2 +d, pts[v][d]);
				nlLockVariable(v * 2 + d);
			}
			nlBegin(NL_MATRIX);
			FOR(q, quads.size() / 4) FOR(e, 4) FOR(d, 2) {
				nlBegin(NL_ROW);
				nlCoefficient(quads[4 * q + e]*2+d, -1.);
				nlCoefficient(quads[4 * q + ((e+1)%4)] * 2 + d, 1.);
				nlEnd(NL_ROW);
			}

			nlEnd(NL_MATRIX);
			nlEnd(NL_SYSTEM);
			nlSolve();
			FOR(v, pts.size()) FOR(d, 2)  pts[v][d]= nlGetVariable(2*v+d);
			nlDeleteContext(nlGetCurrent());
			//export_debug_mesh();
			if (contour.pos_.size() > 2) { pts.resize(index_t(border_size)); quads.clear(); return false; }
			
			if (!Poly2d(pts).quads_are_valid(quads)) {
				//export_debug_mesh();
				FOR(i, contour.pos_.size())  plop(contour.pos(i));
			}
			return Poly2d(pts).quads_are_valid(quads);

		}
		mat2 R90;

		vector<vec2>& pts;
		vector<index_t>& quads;
		Contour2D contour;
	};













	bool Poly2d::try_quad_cover(vector<index_t>& quads) {
		vector<int> angu(pts.size(), 1);
		int sing_valence = 0;
		FOR(v, pts.size()) {
			vec2 d0 = aupp(v, pts) - aupp(v - 1, pts);
			vec2 d1 = aupp(v + 1, pts) - aupp(v, pts);
			double angle = atan2(det(d0, d1), dot(d0, d1));
			if (angle < M_PI / 4.) angu[v] = 0;
			if (angle < -M_PI / 4.) angu[v] = -1;
			sing_valence += angu[v];
		}
		//plop("try_quad_cover");
		//dump_contour();
		
		QuadrangulateWithOneSingularity doit(pts,quads);
		if (doit.apply(angu, sing_valence)) return true;
		
		return false;
	}





	bool Poly2d::try_quadrangulate(vector<index_t>& quads) {
		bool verbose = false;

		index_t nbv = pts.size();
		if (verbose) plop(nbv);
		if (nbv < 4) return false;
		if (nbv == 4) {
			FOR(v, 4) quads.push_back(v);
			if (!quads_are_valid(quads)) { GEO::Logger::out("HexDom")  << "FAIL" <<  std::endl; return false; }
			return true;
		}


		if (nbv % 2 != 0) {
			GEO::Logger::out("HexDom")  << "There is no way to quadrangulate a surface with an odd number of boundary edges" <<  std::endl;
			return false;
		}


		return try_quad_cover(quads);

		/*
		// precompute a few things
		vector<double> angle(nbv);
		vector<double> length(nbv);
		double ave_length = 0;
		FOR(i, nbv) {
			vec2 P[3];
			FOR(p, 3) P[p] = aupp(i + p - 1, pts);
			angle[i] = (180. / M_PI)*atan2(det(P[1] - P[0], P[2] - P[1]), dot(P[1] - P[0], P[2] - P[1]));
			if (verbose)GEO::Logger::out("HexDom")  << "i= " << i << "angle = " << angle[i] <<  std::endl;
			length[i] = (P[1] - P[0]).length() / double(nbv);
			ave_length += length[i];
		}
		plop("gna");


		// define outputs of the search
		index_t start = index_t(-1);
		index_t end = index_t(-1);
		index_t nb_nv_pts = index_t(-1);
		double best_score = 0;



		index_t dec = parity_of_original_points();
		plop("gna");

		FOR(test_start, nbv) {
			plop(test_start);
			index_t test_end;
			index_t test_nb_nv_pts;
			double test_score;

			plop("gna");

			FOR(d, nbv - 5) {
				plop(d);

				test_end = test_start + d + 3;

				vec2 A[3];
				FOR(i, 3) A[i] = aupp(int(test_start + i) - 1, pts);
				vec2 B[3];
				FOR(i, 3) B[i] = aupp(int(test_end + i) - 1, pts);
				vec2 nA1A2 = normalize(A[2] - A[1]);
				vec2 nA1A0 = normalize(A[0] - A[1]);
				vec2 nB1B2 = normalize(B[2] - B[1]);
				vec2 nB1B0 = normalize(B[0] - B[1]);
				vec2 nAB = normalize(B[1] - A[1]);
				vec2 nBA = -nAB;

				double worst_det = 1;
				worst_det = std::min(worst_det, det(nA1A2, nAB));
				worst_det = std::min(worst_det, det(nAB, nA1A0));
				worst_det = std::min(worst_det, det(nB1B2, nBA));
				worst_det = std::min(worst_det, det(nBA, nB1B0));

				test_score = worst_det;


				double AB_relative_length = floor((B[1] - A[1]).length() / ave_length);
				test_nb_nv_pts = index_t(std::max(0, int(AB_relative_length) - 1));

				if (test_nb_nv_pts % 2 != int(d % 2)) {
					if (test_nb_nv_pts == 0) test_nb_nv_pts = 1; else test_nb_nv_pts--;
				}

				if (angle[test_start] < 1) test_score += 1;
				if (angle[test_end] < 1) test_score += 1;
				if (angle[test_start] < -45) test_score += 2;
				if (angle[test_end] < -45) test_score += 2;
				if ((test_start % 2) == 1 - dec) test_score -= 10;
				if ((test_end % 2) == 1 - dec) test_score -= 10;
				test_nb_nv_pts = 1;

				if (best_score < test_score) {
					bool can_cut = true;
					FOR(dd, nbv) {
						index_t ind = test_start + dd;
						if (ind > test_start && ind < test_end)
							can_cut = can_cut && det(nAB, aupp(ind, pts) - A[1]) < 0;
						if (ind > test_end)
							can_cut = can_cut && det(nAB, aupp(ind, pts) - A[1]) > 0;
					}
					if (verbose)
						std::cerr << "can_cut = " << can_cut << "  test_score = " << test_score
						<< "   test_start = " << test_start << "   test_end = " << test_end
						<< "   test_nb_nv_pts = " << test_nb_nv_pts << std::endl;
					if (can_cut) {
						start = test_start;
						end = test_end;
						nb_nv_pts = test_nb_nv_pts;
						best_score = test_score;
					}
				}
			}


		}

		plop("gna");

		if (nbv > 8)
			if (nb_nv_pts != index_t(-1)) {
				if (verbose)GEO::Logger::out("HexDom")  << "remove quad strip from " << start << " with score = " << best_score << " with nbpts" << nb_nv_pts <<  std::endl;

				vector<index_t> global_vid[2]; // gives indices in "pts" from indices in "poly[i]"

				// fill both half with existing points
				//int end = start + nb_nv_pts + 3;
				FOR(d, end - start + 1)                                 global_vid[0].push_back((start + d) % nbv);
				FOR(d, nbv - (end - start) + 1)         global_vid[1].push_back((end + d) % nbv);


				// add new vertices along the cut
				FOR(i, nb_nv_pts)                                       global_vid[0].push_back(nbv + i);
				FOR(i, nb_nv_pts)                                       global_vid[1].push_back(nbv + (nb_nv_pts - 1 - i));
				FOR(i, nb_nv_pts) {
					double c = 1.0 - double(i + 1) / double(nb_nv_pts + 1);
					pts.push_back((1. - c)*pts[start] + c*pts[end% nbv]);
				}

				// solve on two halves
				vector<vec2> poly[2];
				FOR(i, 2) FOR(fv, global_vid[i].size()) poly[i].push_back(pts[global_vid[i][fv]]);

				vector<index_t> poly_quad[2];
				FOR(i, 2) if (!Poly2d(poly[i]).try_quadrangulate(poly_quad[i])) return false;
				// add new pts to global
				FOR(i, 2) for (index_t d = global_vid[i].size(); d < poly[i].size(); d++) {
					global_vid[i].push_back(pts.size());
					pts.push_back(poly[i][d]);
				}
				FOR(i, 2) FOR(qu, poly_quad[i].size()) quads.push_back(global_vid[i][poly_quad[i][qu]]);

				if (!quads_are_valid(quads)) { GEO::Logger::out("HexDom")  << "FAIL remove quad strip" <<  std::endl; return false; }
				return true;
			}


		plop("gna");


		if (verbose) GEO::Logger::out("HexDom")  << "middle_point_quadrangulate(quads)" <<  std::endl;
		middle_point_quadrangulate(quads);
		if (!quads_are_valid(quads)) { GEO::Logger::out("HexDom")  << "FAIL middle_point_quadrangulate" <<  std::endl; return false; }

		return true;
		*/
	}

	/*****************************************************************************************************/

	vec3 Poly3d::barycenter() {
		vec3 bary(0, 0, 0);
		FOR(fv, pts.size()) {
			bary = bary + (1. / double(pts.size()))*pts[fv];
		}
		return bary;
	}

	vec3 Poly3d::normal() {
		vec3 n(0, 0, 0);
		vec3 bary = barycenter();
		FOR(fv, pts.size()) {
			n = n + cross(pts[fv] - bary, pts[next_mod(fv, pts.size())] - bary);
		//	plop(n);
		}
		n = normalize(n);
		return n;
	}


	bool Poly3d::try_triangulate_minweight(vector<index_t>& triangles) {
		index_t nbv = pts.size();
		if (nbv == 3) {
			FOR(v, 3) {
				triangles.push_back(v);
			}
			return true;
		}
		geo_assert(nbv > 3);

		vector<vec2> pts2d;
		Basis3d b(normal());
		FOR(fv, nbv) {
			pts2d.push_back(b.project_xy(pts[fv]));
		}

		return Poly2d(pts2d).try_triangulate_minweight(triangles);
	}

	/**
	 * WARNING: it may introduce new vertices in pts
	 */
	bool Poly3d::try_quadrangulate(vector<index_t>& quads) {
		index_t nbv = pts.size();
		if (nbv < 4) return false;
		vec3 G = barycenter();
		if (normal().length2() < 1e-20) return false;
		Basis3d b(normal());

		vector<vec2> pts2d;
		FOR(fv, nbv) pts2d.push_back(b.project_xy(pts[fv] - G));
		Poly2d p2d(pts2d);
		if (!p2d.try_quadrangulate(quads)) {
			//dump_contour();
			return false;
		}
		for (index_t i = pts.size(); i < p2d.pts.size(); i++)
			pts.push_back(G + b.un_project_xy(p2d.pts[i]));
		return true;
	}

}
