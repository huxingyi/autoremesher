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

#include <exploragram/hexdom/FF.h>
#include <exploragram/hexdom/frame.h>
#include <exploragram/hexdom/basic.h>
#include <exploragram/hexdom/extra_connectivity.h>
#include <geogram/NL/nl.h>
#include <geogram/numerics/optimizer.h>

#ifdef GEO_OPENMP
#include <omp.h>
#endif
#include <queue>

namespace GEO {

    FFopt::FFopt(Mesh* p_m) {
        m = p_m;
        compute_tet_edge_graph(m,v2e, true); // here  need a bidirectionl edge graph to speed up the LBFGS part
        Attribute<vec3> lockB(m->vertices.attributes(), "lockB");
        num_l_v = m->vertices.nb();
        num_ln_v = m->vertices.nb();
        FOR(inv_v, m->vertices.nb()) {
	    index_t v = m->vertices.nb()-1 - inv_v;
	    if (lockB[v][0] <.5) num_l_v = v;
	    if (lockB[v][2] <.5) num_ln_v = v;
        }
	if (num_ln_v == 0) num_ln_v = m->vertices.nb();
    }

    FFopt::~FFopt() {
        m->edges.clear();
    }




    void FFopt::FF_init(bool generate_sh) {
        Attribute<mat3> B(m->vertices.attributes(), "B");
        Attribute<vec3> lockB(m->vertices.attributes(), "lockB");
        Attribute<SphericalHarmonicL4> sh;
        if (generate_sh) sh.bind(m->vertices.attributes(), "sh");

        double smooth_coeff = 1.;
        double normal_coeff = 100.;

		plop(num_l_v);
		plop(num_ln_v);
		plop("construct system");
        nlNewContext();
        nlSolverParameteri(NL_LEAST_SQUARES, NL_TRUE);
        nlSolverParameteri(NL_NB_VARIABLES, NLint(2 * (num_ln_v - num_l_v) + 9 * m->vertices.nb()));
        nlBegin(NL_SYSTEM);

        // lock frames
        FOR(v, num_l_v) {
            SphericalHarmonicL4 sh48;
            sh48[4] = std::sqrt(7. / 12.);
            sh48[8] = std::sqrt(5. / 12.);
            sh48.euler_rot(mat3_to_euler(normalize_columns(B[v])));
            FOR(i, 9) {
                nlSetVariable(v * 9 + i, sh48[i]);
                nlLockVariable(v * 9 + i);
            }
        }
        nlBegin(NL_MATRIX);
        // smoothing equations
        FOR(e, m->edges.nb()) {
            FOR(i, 9) {
                nlRowScaling(smooth_coeff);
                nlBegin(NL_ROW);
                nlCoefficient(m->edges.vertex(e, 0) * 9 + i, -1.);
                nlCoefficient(m->edges.vertex(e, 1) * 9 + i, 1.);
                nlEnd(NL_ROW);
            }
        }
        // boundary condition enforced by barrier equations
        for (index_t v = num_l_v; v < num_ln_v; v++) {
            SphericalHarmonicL4 sh0, sh4, sh8;
            sh4[4] = std::sqrt(7. / 12.);
            sh0[0] = std::sqrt(5. / 12.);
            sh8[8] = std::sqrt(5. / 12.);
            vec3 xyz = mat3_to_euler(normalize_columns(B[v]));
            sh4.euler_rot(xyz);
            sh0.euler_rot(xyz);
            sh8.euler_rot(xyz);
            FOR(i, 9) {
                nlRowScaling(normal_coeff);
                nlBegin(NL_ROW);
                nlCoefficient(v * 9 + i, 1.);
				nlCoefficient(m->vertices.nb() * 9 + v - num_l_v, sh0[i]);
				nlCoefficient(m->vertices.nb() * 9 + (num_ln_v - num_l_v) + v - num_l_v, sh8[i]);
				nlRightHandSide(sh4[i]);
                nlEnd(NL_ROW);
            }
        }



        nlEnd(NL_MATRIX);
        nlEnd(NL_SYSTEM);
        nlSolve();

        plop("project SH");
        // convert spherical harmonic coefficients to a rotation
#ifdef GEO_OPENMP	
#pragma omp parallel
#endif	
        {
            get_thread_range(m->vertices.nb(), start, end);
            for (index_t v = start; v < end; v++) {
                SphericalHarmonicL4 fv;
                FOR(i, 9) fv[i] = nlGetVariable(v * 9 + i);
                if (generate_sh) sh[v] = fv;
                if (v >= num_l_v) {
                    vec3 oldz = col(B[v], 2);
					if (v > start && v > num_l_v) {
                        vec3  prev = mat3_to_euler(normalize_columns(B[v - 1]));
						B[v] = fv.project_mat3(1e-3, 1e-5, &prev);
                    } else 
                        B[v] = fv.project_mat3(1e-3, 1e-5, nullptr);
                    if (v <= num_ln_v) {
                        AxisPermutation ap;
                        ap.make_col2_equal_to_z(B[v], normalize(oldz));
                        B[v] = Frame(B[v]).apply_permutation(ap);
						FOR(d, 3) B[v](d, 2) = oldz[d];// restore size as well
					}
                }
            }
        }
        nlDeleteContext(nlGetCurrent());

    }


    // place older with constant size
    void FFopt::compute_Bid_norm() {
        Attribute<mat3> B(m->vertices.attributes(), "B");
        double scale = col(B[0], 2).length();
        FOR(v,m->vertices.nb()) {
            FOR(a, 3) {
                vec3 co = scale * normalize(col(B[v], a));
                FOR(d, 3) B[v](d, a) = co[d];// restore size as well
            }
        }
    }
}


namespace {
    
    using namespace GEO;
    
    namespace FF_LBFGS {
        FFopt* ffopt_ptr;
        index_t Num_ln_v;
        index_t Num_l_v;
        double lastf;
        double NRJ_threshold = 1e-5;
        int nb_iters;
        GEO::Optimizer *solver;
    }

    void new_iteration_cb(index_t N, const double* x, double f, const double* g, double gnorm) {
        FF_LBFGS::nb_iters++;
        double stop_crit = std::abs(FF_LBFGS::lastf - f) / std::abs(f);
        FF_LBFGS::lastf = f;
        std::cerr << ".";
        if (stop_crit < FF_LBFGS::NRJ_threshold) {
            GEO::Logger::out("HexDom")  << "  LBFGS iter " << N << " f " << f << " gnorm " << gnorm << " trash " << x[0] * g[0] <<  std::endl;
            GEO::Logger::out("HexDom")  << "stop_crit < NRJ_threshold " <<  std::endl; throw 1;
        }
    }

    void compute_gradient_cb2(unsigned int N, double* x, double& f, double* g) {
        mat3 mEx = mat3_from_coeffs( 0, 0, 0, 0, 0, -1, 0, 1, 0 );
        mat3 mEy = mat3_from_coeffs(0, 0, 1, 0, 0, 0, -1, 0, 0 );
        mat3 mEz = mat3_from_coeffs(0, -1, 0, 1, 0, 0, 0, 0, 0 );
        
        Attribute<mat3> B(FF_LBFGS::ffopt_ptr->m->vertices.attributes(), "B");
        index_t nverts = FF_LBFGS::ffopt_ptr->m->vertices.nb();
        geo_assert(N == 3 * (nverts - FF_LBFGS::Num_ln_v) + FF_LBFGS::Num_ln_v);


		Attribute<bool> border_vertex(FF_LBFGS::ffopt_ptr->m->vertices.attributes(), "border_vertex");
		FOR(v, FF_LBFGS::ffopt_ptr->m->vertices.nb()) border_vertex[v] = false;
		FOR(c, FF_LBFGS::ffopt_ptr->m->cells.nb())  FOR(cf, 4) if (FF_LBFGS::ffopt_ptr->m->cells.adjacent(c, cf) == NOT_AN_ID)
			FOR(cfv, 3) border_vertex[FF_LBFGS::ffopt_ptr->m->cells.facet_vertex(c, cf, cfv)] = true;

#ifdef GEO_OPENMP
        int max_threads = omp_get_max_threads();
#else
        int max_threads = 1;	
#endif	
        double *f_chunks = new double[max_threads](); // f_chunks is initialized to be zero

#ifdef GEO_OPENMP	
#pragma omp parallel
#endif	
        {
#ifdef GEO_OPENMP	    
            int thread_id = omp_get_thread_num();
#else
	    int thread_id = 0;
#endif	    
            get_thread_range(nverts, istart, iend);

           
            mat3 mJR[3], mR, mSinv, mPst, mJPst[3];
            for (index_t v1 = istart; v1 < iend; v1++) {
                if (v1 >= FF_LBFGS::Num_ln_v) {
                    index_t idx = FF_LBFGS::Num_ln_v + (v1 - FF_LBFGS::Num_ln_v) * 3;
                    FOR(i,3) g[idx + i] = 0.;
                    mR = euler_to_mat3(vec3(x[idx], x[idx+1], x[idx+2]));
                    mat3 mRx = rotx(x[idx]);
                    mat3 mRy = roty(x[idx+1]);
                    mat3 mRz = rotz(x[idx+2]);

                    mJR[0] = mR*mEx;
                    mJR[2] = mEz*mR;
                    mJR[1] = mRz*mRy*mEy*mRx;
                }
                else {
                    g[v1] = 0.;
                    mR = normalize_columns(B[v1]) * rotz(x[v1]);
                    mJR[0] = mR* mEz; // init JR[0] = R * Ez; JR[1] and JR[2] are not initialized
                }


                FOR(iv2, int(FF_LBFGS::ffopt_ptr->nb_neigs(v1))) {
                    index_t v2 = FF_LBFGS::ffopt_ptr->neig(v1, iv2);
                    if (v2 >= FF_LBFGS::Num_ln_v) { // init S = Rz Ry Rx
                        index_t idx = FF_LBFGS::Num_ln_v + (v2 - FF_LBFGS::Num_ln_v) * 3;
                        mSinv = euler_to_mat3(vec3(x[idx], x[idx + 1], x[idx + 2]));
                    } else  // init S = constraint * Rz
                        mSinv = normalize_columns(B[v2]) * rotz(x[v2]);
                    
       
                    mSinv = mSinv.transpose();
                    mPst = mSinv* mR; // Pst = S^{-1} * R

					double scale = 1.;
					if (HexdomParam::FF.rigid_border) {
						if (border_vertex[v1])scale += 100.;
						if (border_vertex[v2])scale += 100.;
					}
					if (v1 > v2) FOR(i, 3)
                        f_chunks[thread_id] += scale *(10. / 3.*(pow(mPst(0,i) * mPst(1,i), 2) + pow(mPst(0,i) * mPst(2,i), 2) + pow(mPst(1,i )* mPst(2,i), 2)));
                    
                    if (v1 >= FF_LBFGS::Num_ln_v) {
                        index_t idx = FF_LBFGS::Num_ln_v + (v1 - FF_LBFGS::Num_ln_v) * 3;
                        FOR(d,3) {                         
                            mJPst[d] = mSinv* mJR[d]; // JPst[d] = S^{-1} * JR[d]
                            FOR(i,3)FOR(j,3)
                                g[idx + d] += scale *(20. / 3.*mPst(i,j) * (pow(mPst(i, (j + 1)%3), 2) + pow(mPst(i , (j + 2) % 3), 2))*mJPst[d](i , j));
                        }
                    } else if (v1 >= FF_LBFGS::Num_l_v) {
                        mJPst[0] = mSinv* mJR[0]; // JPst[0] = S^{-1} * JR[0] ; JPst[1] and JPst[2] are not initialized
                        FOR(i, 3)FOR(j, 3)
                            g[v1] += scale *(20. / 3.*mPst(i ,j) * (pow(mPst(i ,(j + 1) % 3), 2) + pow(mPst(i , (j + 2) % 3), 2))*mJPst[0](i , j));
                    }
                } // v2
            } // v1
        } // omp parallel
        f = 0.;
        for (int i = max_threads; i--; f += f_chunks[i]);
        delete[] f_chunks;
    }
}

namespace GEO {

    void FFopt::FF_smooth() {
		
        Attribute<mat3> B(m->vertices.attributes(), "B");

		Attribute<SphericalHarmonicL4> sh(m->vertices.attributes(), "sh");

        // init global variables that must be visible in callbacks
        FF_LBFGS::ffopt_ptr = this;
        FF_LBFGS::Num_ln_v = num_ln_v;
        FF_LBFGS::Num_l_v = num_l_v;
        FF_LBFGS::lastf = 1e20;
        FF_LBFGS::NRJ_threshold = 1e-5;
        FF_LBFGS::nb_iters = 0;

        // create LBFGS solver and unknown vector
        index_t nverts = m->vertices.nb();

        // unknown vetor is packed as follows:
        // FF_LBFGS::Num_ln_v coordinates: 1 rotation angle around the constrained axis
        // nverts - FF_LBFGS::Num_ln_v coordinates: 3 euler angles
        //
        // WARNING: note that locked frames (v<num_l_v) are associated to a useless variables
        double *x = new double[nverts * 3 - FF_LBFGS::Num_ln_v * 2];
        GEO::Optimizer *solver = GEO::Optimizer::create();
        FF_LBFGS::solver = solver;
        solver->set_N((nverts - FF_LBFGS::Num_ln_v) * 3 + FF_LBFGS::Num_ln_v);
        solver->set_M(3);
        solver->set_epsf(1e-5);
        solver->set_epsx(1e-5);
        solver->set_epsg(1e-5);
        solver->set_funcgrad_callback(compute_gradient_cb2);
        solver->set_newiteration_callback(new_iteration_cb);

        // init variables
        for (index_t i = FF_LBFGS::Num_ln_v; i < nverts; i++) {
            index_t idx = (i - FF_LBFGS::Num_ln_v) * 3 + FF_LBFGS::Num_ln_v;
            vec3 xyz = mat3_to_euler(normalize_columns(B[i]));
            FOR(d, 3) x[idx+d] = xyz[d];
        }
        FOR(i, FF_LBFGS::Num_ln_v) x[i] = 0.;



        // solve until we run out of time
        solver->set_max_iter(1000000);
        try { solver->optimize(x); }
        catch (...) {

        }

        // apply a euler rotation to rot... 
        for (index_t i = nverts; i--;) {
            if (i >= FF_LBFGS::Num_ln_v) {
                index_t idx = (i - FF_LBFGS::Num_ln_v) * 3 + FF_LBFGS::Num_ln_v;
                B[i] = euler_to_mat3(vec3 (x[idx], x[idx + 1], x[idx + 2]));
            }
            else  B[i]  = B[i] * rotz(x[i]);

			FOR(d, 9)sh[i][d] = 0;
			sh[i][4] = std::sqrt(7. / 12.);
			sh[i][8] = std::sqrt(5. / 12.);
			sh[i].euler_rot(mat3_to_euler(normalize_columns(B[i])));
	}
        delete[] x;
    }



    //   ___             _      ___            _   _       _
    //  | _ )_ _ _  _ __| |_   |_  )  ___ _ __| |_(_)_ __ (_)______
    //  | _ \ '_| || (_-< ' \   / /  / _ \ '_ \  _| | '  \| |_ / -_)
    //  |___/_|  \_,_/__/_||_| /___| \___/ .__/\__|_|_|_|_|_/__\___|
    //                                   |_|



    void FFopt::brush_frame() {
        plop("brushing");
        Attribute<vec3> lockU(m->vertices.attributes(), "lockU");// how many dimensions are locked
        Attribute<mat3> B(m->vertices.attributes(), "B");
        vector<bool> seen(m->vertices.nb(), false);
        FOR(seed, m->vertices.nb()) {   // multiple components?
            if (seen[seed]) continue;
            seen[seed] = true;
            std::deque<index_t> Q;
            Q.push_back(seed);
            while (Q.size()) {  // start a breadth-first brushing
                index_t cur = Q.front();
                Q.pop_front();
                FOR(lv, nb_neigs(cur)) {
                    index_t v = neig(cur, lv);
                    seen[v] = true;
                    AxisPermutation M=Rij(m,B,v,cur);
                    if (M.mid != 0) {
                        lockU[v] = M.inverse().get_mat()* lockU[v];
                        B[v] = B[v]* M.get_mat();
                        FOR(d, 3) if (std::abs(lockU[v][d]) < .1) lockU[v][d] = 0;
                    }
					Q.push_back(v);
                }
            }
        }
    }

}
