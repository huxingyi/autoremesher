

#include <exploragram/basic/common.h>
#include <exploragram/hexdom/quadmesher.h>
#include <exploragram/hexdom/geometry.h>
#include <exploragram/hexdom/mesh_utils.h>
#include <geogram/mesh/mesh_io.h>
#include <exploragram/hexdom/extra_connectivity.h>
#include <geogram/NL/nl.h>
#include <geogram/mesh/mesh_geometry.h>

#include <deque>
#include <assert.h>
#include <cmath>
namespace GEO {

	
	template<class T>
	T lower_abs_modulo(T value, T module = 1.) {
		return value - module * std::floor(value / module + .5);
	}
	template<class T>
	T lower_positive_modulo(T value, T module = 1.) {
		return value - module * std::floor(value / module);
	}

	typedef vecng<6, double> vec6;

	void add_edge(Mesh* m, vec3 P0, vec3 P1, double value ) {
		Attribute<double> val(m->edges.attributes(), "val");
		index_t off_v = m->vertices.create_vertices(2);
		X(m)[off_v] = P0;
		X(m)[off_v + 1] = P1;
		val[m->edges.create_edge(off_v, off_v + 1)] = value;
	}

	void add_triangle(Mesh* m, vec3 P0, vec3 P1, vec3 P2, double value ) {
		Attribute<double> val(m->facets.attributes(), "val");
		index_t off_v = m->vertices.create_vertices(3);
		X(m)[off_v] = P0; X(m)[off_v + 1] = P1; X(m)[off_v + 2] = P2;
		val[m->facets.create_triangle(off_v, off_v + 1, off_v + 2)] = value;
	}

        static mat2 angle_to_mat(double p_alpha, double p_beta) {
		double rot_angle = (p_beta + p_alpha )/ 2.;
		double rot_values[4] = { cos(rot_angle), sin(rot_angle), -sin(rot_angle) ,cos(rot_angle)};
		mat2 rot(rot_values);
		mat2 inv_rot = rot.inverse();

		double diag_angle = (p_beta - p_alpha) / 2.;
		double diag_values[4] = { cos(diag_angle), 0,0, sin(diag_angle) };
		mat2 diag(diag_values);
		return inv_rot*diag*rot;
	}


	static vec2 operator*(mat2& M, vec2& v) {
		return vec2(M(0, 0)*v[0] + M(0, 1)*v[1], M(1, 0)*v[0] + M(1, 1)*v[1] );
	}


	struct FF2D {
		FF2D(Mesh* p_m) : fec(p_m){
			m = p_m;
			feature_edge.bind(m->facet_corners.attributes(), "feature_edge");
			alpha.bind(m->facets.attributes(), "alpha");
			beta.bind(m->facets.attributes(), "delta");
			aniso.bind(m->facets.attributes(), "aniso");
		}

		void init_feature_edge() {
			FOR(h, m->facet_corners.nb()) {
				feature_edge[h] = false;
				if (fec.opposite(h) == NOT_AN_ID) {
					feature_edge[h] = true;
					continue;
				}
				vec3 n[2] = {
					Geom::triangle_normal(X(m)[fec.org(fec.prev(h))], X(m)[fec.org(h)], X(m)[fec.dest(h)]),
					Geom::triangle_normal(X(m)[fec.dest(fec.next(fec.opposite(h)))], X(m)[fec.dest(h)], X(m)[fec.org(h)])
				};
				FOR(f, 2) n[f] = normalize(n[f]);
				if (acos(dot(n[0], n[1])) > M_PI / 3.) {
					feature_edge[h] = true;
					feature_edge[fec.opposite(h)] = true;
				}
			}
		}


		void local_basis(index_t h, vec3& x, vec3& y, vec3& z) {
			z = normalize(Geom::mesh_facet_normal(*m, fec.facet(h)));
			x = normalize(X(m)[fec.dest(h)] - X(m)[fec.org(h)]);
			y = normalize(cross(z, x));
		}

		void param_per_triangle(Mesh* debug_mesh) {
			FOR(f, m->facets.nb()) {
				vec3 G = Geom::mesh_facet_center(*m, f);
				vec3 x, y, z;
				local_basis(m->facets.corner(f, 0), x, y, z);
				double scale = std::sqrt(Geom::mesh_facet_area(*m, f));
				FOR(i, 2) {
					double rot = i?beta[f] : alpha[f];
					if (rot> 100) continue;
					vec3 vect = .2*scale*(x*cos(rot) + y*sin(rot));
					add_edge(debug_mesh, G - vect, G + vect, i);
				}
				FOR(i, 32) {
					vec2 ref(cos(double(i)/5.) , sin(double(i) / 5.));
					ref = aniso[f]*ref;
					add_edge(debug_mesh, G , G +0.1*scale*(ref[0]*x+ref[1]*y), i);
				}

			}
		}		

		vec3 edge_geom(index_t h) {
			return X(m)[fec.dest(h)] - X(m)[fec.org(h)];
		}
		double vector_angle(vec3 v0, vec3 v1) { return atan2(cross(v0, v1).length(), dot(v0, v1)); }
		double angle_w_r_t_ref(index_t h) {
			index_t h_ref = m->facets.corner(fec.facet(h),0);
			if (h == h_ref) return 0;
			double angle = vector_angle(edge_geom(h), edge_geom(h_ref));
			if (h == m->facets.corner(fec.facet(h), 2)) return angle;
			if (h == m->facets.corner(fec.facet(h), 1)) return 2.*M_PI - angle;
			return angle; // [BL seems to be missing !!]
		}

		double corner_angle(index_t h) { return vector_angle(X(m)[fec.dest(h)] - X(m)[fec.org(h)], X(m)[fec.org(fec.prev(h))] - X(m)[fec.org(h)]);}

		// if // transport: alpha[fec.facet(fec.opposite(h)) ] = basis_change(h) + alpha[fec.facet(h) ]; 
		double basis_change(index_t h) {
			return M_PI+ angle_w_r_t_ref(h) - angle_w_r_t_ref(fec.opposite(h));
		}



	    void naive_LS_blur_delta(Mesh* debug_mesh) {
		        geo_argused(debug_mesh);
			nlNewContext();
			nlSolverParameteri(NL_LEAST_SQUARES, NL_TRUE);
			nlSolverParameteri(NL_NB_VARIABLES, NLint(3*m->facets.nb()));
			nlBegin(NL_SYSTEM);

			FOR(f, m->facets.nb()) if (beta[f] < 100) {
				Matrix<2, double> M = angle_to_mat(alpha[f], beta[f]);
				nlSetVariable(3 * f, M(0, 0)); nlLockVariable(3 * f);
				nlSetVariable(3 * f+1, M(0,1)); nlLockVariable(3 * f+1);
				nlSetVariable(3 * f+2, M(1, 1)); nlLockVariable(3 * f+2);
			}

			nlBegin(NL_MATRIX);
			FOR(h, 3 * m->facets.nb()) {
				if (feature_edge[h]) continue; geo_assert(fec.opposite(h) != NOT_AN_ID);
				double angle = -basis_change(h);
				double c = cos(angle);
				double s = sin(angle);

				index_t f = fec.facet(h);
				index_t opp = fec.facet(fec.opposite(h));
				index_t a0 = 3 * f;
				index_t b0 = 3 * f + 1;
				index_t c0 = 3 * f + 2;
				index_t a1 = 3 * opp;
				index_t b1 = 3 * opp + 1;
				index_t c1 = 3 * opp + 2;
				nlBegin(NL_ROW);
				nlCoefficient(a0, -1);
				nlCoefficient(a1, c*c);
				nlCoefficient(b1, -2 * s*c);
				nlCoefficient(c1, s*s);
				nlEnd(NL_ROW);
				nlBegin(NL_ROW);
				nlCoefficient(b0, -1);
				nlCoefficient(a1, s*c);
				nlCoefficient(b1, c*c -s*s);
				nlCoefficient(c1, -s*c);
				nlEnd(NL_ROW);

				nlBegin(NL_ROW);
				nlCoefficient(c0, -1);
				nlCoefficient(a1, s*s);
				nlCoefficient(b1, 2.*s*c);
				nlCoefficient(c1, c*c);
				nlEnd(NL_ROW);



				// data fitting term

				// double scale = .1; [BL unused]
				//nlBegin(NL_ROW); nlCoefficient(a0, -scale); nlRightHandSide(scale); nlEnd(NL_ROW);
				//nlBegin(NL_ROW); nlCoefficient(b0, -scale); nlRightHandSide(0); nlEnd(NL_ROW);
				//nlBegin(NL_ROW); nlCoefficient(c0, -scale); nlRightHandSide(scale); nlEnd(NL_ROW);

			}
			nlEnd(NL_MATRIX);
			nlEnd(NL_SYSTEM);
			nlSolve();
			Attribute<double> A(m->facets.attributes(), "a");
			Attribute<double> B(m->facets.attributes(), "b");
			Attribute<double> C(m->facets.attributes(), "c");
			FOR(f, m->facets.nb()) {
				mat2 M;
				M(0, 0) = nlGetVariable(3 * f);
				M(0, 1) = nlGetVariable(3 * f + 1);
				M(1, 0) = nlGetVariable(3 * f + 1);
				M(1, 1) = nlGetVariable(3 * f + 2);
				aniso[f] = M;

			}
		
		}

		void naive_LS_smooth() {
			static const double N = 4.;// N sym dir field... N=4, just change it for debug
			nlNewContext();
			nlSolverParameteri(NL_LEAST_SQUARES, NL_TRUE);
			nlSolverParameteri(NL_NB_VARIABLES, NLint(2 * m->facets.nb()));
			nlBegin(NL_SYSTEM);

			FOR(h, 3 * m->facets.nb()) {
				if (!feature_edge[h]) continue;
				index_t f = fec.facet(h);
				double angle = -angle_w_r_t_ref(h);//alpha[f] + M_PI / 4. + .5*delta[f];
				double avt[2] = { cos(angle),sin(angle) };
				double ap[2];
				mat2 inv = aniso[f].inverse();
				mult(inv, avt,ap);
				angle = atan2(ap[1], ap[0]);
				nlSetVariable(f * 2, cos(N*angle));
				nlLockVariable(f * 2);
				nlSetVariable(f * 2 + 1, sin(N*angle));
				nlLockVariable(f * 2 + 1);
			}

			nlBegin(NL_MATRIX);
			FOR(h, 3 * m->facets.nb()) {
				if (feature_edge[h]) continue; geo_assert(fec.opposite(h) != NOT_AN_ID);

				double angle = basis_change(h);
				angle *= N;
				double rot[2][2] = { { cos(angle),sin(angle) },{ -sin(angle),cos(angle) } };
				FOR(d, 2) {
					nlBegin(NL_ROW);
					nlCoefficient(fec.facet(h) * 2 + d, -1);
					FOR(dd, 2) nlCoefficient(fec.facet(fec.opposite(h)) * 2 + dd, rot[d][dd]);
					nlEnd(NL_ROW);
				}
			}
			nlEnd(NL_MATRIX);
			nlEnd(NL_SYSTEM);
			nlSolve();
			FOR(f, m->facets.nb()) {
				double angle = (1. / N)*atan2(nlGetVariable(f * 2 + 1), nlGetVariable(f * 2));
				double avt[2] = { cos(angle),sin(angle) };
				double ap[2];
				mult(aniso[f], avt, ap);
				angle = atan2(ap[1], ap[0]);

				alpha[f] = (1. / N)*atan2(nlGetVariable(f * 2 + 1), nlGetVariable(f * 2));
			}
		}


		void apply(Mesh* debug_mesh) {
			init_feature_edge();
			FOR(f, m->facets.nb()) alpha[f] = beta[f] = 1000;

			// for each fature edge:
			//  -> compute the #angu geom to the next feature edge around vertex
			//	-> define #angu in map 
			//  -> if #angu in map est impair: 
			//		=> alpha = #angu in map - #angu geom
			//		=> paralell transport first constraint
			// ===> RESULT : fix an angle + possible delta a some triangles

			FOR(h, 3*m->facets.nb()) {
				if (!feature_edge[h]) continue;
				if (beta[fec.facet(h)]<20) continue; // already constrained by previous halfedge
				alpha[fec.facet(h)] = -angle_w_r_t_ref(h);
				
				// find all corners sharing org(h)
				vector<index_t> edges;
				edges.push_back(h);
				while (!feature_edge[fec.prev(edges.back())]) 
					edges.push_back(fec.opposite(fec.prev(edges.back())));
				
				// compute their angles
				double sum = 0;
				FOR(i, edges.size())  sum+= corner_angle(edges[i]);
				double wanted_sum = 0.5*M_PI * floor(sum / (0.5*M_PI) + .5);
				if (wanted_sum < .1) wanted_sum = 0.5*M_PI;
				int nb_angu = int(std::floor(wanted_sum / (0.5*M_PI)));


				// parallel transport
				FOR(i, edges.size()-1) alpha[fec.facet(edges[i+1])] = basis_change(fec.prev(edges[i])) + alpha[fec.facet(edges[i ])];

				if (nb_angu % 2 == 0)
					FOR(i, edges.size() - 1) alpha[fec.facet(edges[i + 1])] += (sum - wanted_sum)*double(i + 1) / double(edges.size() - 1);
				else FOR(i, edges.size())
					beta[fec.facet(edges[i])] = alpha[fec.facet(edges[i])] + M_PI / 2. +(sum - wanted_sum);
				
				//plop(edges.size());
				//plop(wanted_sum);
			}
			naive_LS_blur_delta(debug_mesh);
			
		//naive_LS_smooth();

			//FOR(f, m->facets.nb()) alpha[f] = delta[f] = 1000;
			//FOR(f, m->facets.nb()) FOR(lv, 3) {
			//	index_t h = m->facets.corner(f, lv);
			//	if (feature_edge[h]) alpha[f] = -angle_w_r_t_ref(h);
			//}
		}



		
	

		FacetsExtraConnectivity fec;
		Attribute<bool> feature_edge;	// halfedge boolean
		Attribute<double> alpha;
		Attribute<double> beta;
		Attribute<mat2> aniso;
		Mesh* m;
	};


	void current_test(Mesh* m, Mesh* debug_mesh) {
		FF2D ff2d(m);
		ff2d.apply(debug_mesh);
		ff2d.param_per_triangle(debug_mesh);
		return;

	}
}
