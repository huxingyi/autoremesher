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


#ifndef H_HEXDOM_ALGO_MIXED_CONSTRAINED_SOLVER_H
#define H_HEXDOM_ALGO_MIXED_CONSTRAINED_SOLVER_H


#include <exploragram/basic/common.h>
#include <exploragram/hexdom/basic.h>
#include <exploragram/hexdom/id_map.h>
#include <geogram/NL/nl.h>
#include <algorithm>



 //***************************************************
 // HOW TO USE:
 //***************************************************
 // MatrixMixedConstrainedSolver intsolver(nb_vars);
 // // tell that variable var_id is a multiple of m
 // intsolver.set_multiplicity(var_id, m);
 // // tell 4 times the linear constraints
 // FOR(pass, 4) {
 //  // construct each constraint
 //	intsolver.begin_constraint();
 //	intsolver.add_constraint_coeff(var_id, coeff);
 //	....
 //	intsolver.end_constraint();
 //  // tell the solver to start the next pass
 //  intsolver.end_pass(pass);
 //}
 // // solve in two pass
 // FOR(iter, 2) {
 //	if (iter == 0)	intsolver.start_full_real_iter();
 //	else			intsolver.start_mixed_iter();
 //	 intsolver.begin_energy();
 //	 intsolver.add_energy_coeff(var_id, coeff);
 //	 ....
 //	 intsolver.add_energy_rhs(rhs);
 //	 intsolver.end_energy();
 //	}
 //	intsolver.end_iter();
 //}
 // // get the result
 //double var_value = intsolver.value(var_id);


namespace GEO {

	struct Coeff {
		Coeff(index_t p_index, double p_a) : index(p_index), a(p_a) {
		}

		Coeff() : index(index_t(-1)), a(-1.0) {
		}

		index_t index;
		double a;
	};

	inline std::ostream& operator<<(
	    std::ostream& os, const GEO::vector<Coeff>& obj
	) {
		// write obj to stream
		FOR(i, obj.size())
			os <<(i?" + ":"")<< obj[i].a << ".X_"<<obj[i].index;
		return os;
	}

	//inline double nint(double x) {
	//	// return floor(x + .5);
	//	double r1 = floor(x);
	//	double r2 = floor(x + 1.0);
	//	return (x - r1) < (r2 - x) ? r1 : r2;
	//}

	inline bool coeffCmp(const Coeff& A, const Coeff& B) {
		return (A.index < B.index);
	}

	inline void order_AND_make_unique(vector<Coeff>& c) {
		std::sort(c.begin(), c.end(), coeffCmp);
		double sum = 0;
		index_t ires = 0;
		FOR(i, c.size()) {
			sum += c[i].a;
			if (i + 1 != c.size() && c[i].index == c[i + 1].index) {
				continue;
			}
			if (std::fabs(sum)>1e-10) {
				c[ires] = Coeff(c[i].index, sum);
				ires++;
			}
			sum = 0;
		}
		c.resize(ires);
	}

	/**
	 * \brief Adds a coefficient in a sparse vector.
	 * \details If the coefficient already exists, then \p val
	 *  is added to the existing coefficient, else a new coefficient
	 *  is created and inserted in such a way that the vector remains
	 *  sorted.
	 * \param[in,out] L a reference to the sparse vector,
	 *  represented by a vector<Coeff>.
	 * \param[in] index the index of the coefficient to add.
	 * \param[in] val the value of the coefficient to add.
	 */
	inline void add_to_sorted_sparse_vector(
		vector<Coeff>& L, index_t index, double val
	) {
		// [BL] what follows is an optimized version of:
		//   L.push_back(Coeff(index,val));
		//   order_AND_make_unique(L);
		for (index_t jj = 0; jj < L.size(); ++jj) {
		        if (L[jj].index == index) {
				L[jj].a += val;
				return;
			}
			if (L[jj].index > index) {
			        L.insert(L.begin() + long(jj), Coeff(index, val));
				return;
			}
		}
		L.push_back(Coeff(index, val));
	}

	struct NaiveSparseMatrix {

		void init_zero(index_t p_nb_lines) {
			M.resize(p_nb_lines);
			// [BL] the following line was missing (resize does
			// not reset the existing items in a vector).
			for (index_t i = 0; i < p_nb_lines; ++i) {
				M[i].resize(0);
			}
		}

		void init_identity(index_t size) {
			init_zero(size);
			FOR(i, size) {
				M[i].push_back(Coeff(i, 1));
			}
		}

		index_t nb_coeffs_in_line(index_t l) const {
			return M[l].size();
		}

		const Coeff& get(index_t line, index_t i_th_non_null_coeff) const {
			return M[line][i_th_non_null_coeff];
		}

		void add(index_t line, index_t col, double coeff) {
			add_to_sorted_sparse_vector(M[line], col, coeff);
			// [BL] the code above replaces:
			// L.push_back(Coeff(col,coeff));
			// order_AND_make_unique(L);
		}

		index_t nb_lines() const {
			return index_t(M.size());
		}

		vector<vector<Coeff> > M;
	};



	// a RemoveColMatrix B is a matrix that zeros a column of M by M <-- MB
	struct RemoveColMatrix {

		RemoveColMatrix(vector<Coeff> C, index_t ind_to_remove = 0) {
			geo_assert(ind_to_remove < C.size());
			/* if (ind_to_remove >= 0) [BL] TODO: Check with Nico !!!*/
			std::swap(C.back(), C[ind_to_remove]);
			zero_col = C.back().index;
			double multi = -1. / C.back().a;
			C.pop_back();
			FOR(c, C.size()) {
				line.push_back(Coeff(C[c].index, C[c].a*multi));
			}
		}

		index_t nb_coeffs_in_line(index_t l) const {
			if (zero_col == l) {
				return line.size();
			}
			return 1;
		}

		Coeff get(index_t l, int i_th_non_null_coeff) {
			if (zero_col == l) {
				return line[i_th_non_null_coeff];
			}
			return Coeff(l, 1.);
		}

		// [BL] can we remove this function ?
		/*
		  index_t nb_lines() const {
		  geo_assert_not_reached;
		  return NOT_AN_ID;
		  }
		*/

		index_t zero_col;
		vector<Coeff > line;
	};


	// [BL] passed result by reference instead of making it returned.
	inline void mult(const vector<Coeff>& c, NaiveSparseMatrix& B, vector<Coeff>& result) {
		result.resize(0);
		FOR(co_c, c.size()) {
			index_t lB = c[co_c].index;
			FOR(co_b, B.nb_coeffs_in_line(lB)) {
				result.push_back(Coeff(B.get(lB, co_b).index, B.get(lB, co_b).a * c[co_c].a));
			}
		}
		order_AND_make_unique(result);
	}

	// MatrixM is filled in 3 passes
	// pass 1: creates M0 that sets some variables to zero
	// pass 2: creates M1 that creates an indirection
	// pass 3: creates M2 that manage all remaining problems

	struct MatrixM {

		MatrixM(index_t size) {
			multiplicity.resize(size, 0);
			DEBUG_init_multiplicity.resize(size, 0);
			M0.resize(size);
			FOR(i, size) {
				M0[i] = i;
			}
			pass = 0;
		}

		void finalize_M0() {
			geo_assert(M1.size() == 0);
			geo_assert(M2.nb_lines() == 0);
			index_t M0_nb_col = 0;
			FOR(i, M0.size()) {
				if (M0[i] != NOT_AN_ID) {
					multiplicity[M0_nb_col] = multiplicity[i];
					M0[i] = M0_nb_col;
					M0_nb_col++;
				}
			}
			GEO::Logger::out("HexDom")  << "M0 final size is " << M0.size() << " x " << M0_nb_col <<  std::endl;
			multiplicity.resize(M0_nb_col);
			M1.resize(M0_nb_col);
			FOR(i, M0_nb_col) {
				M1[i] = Coeff(i, 1);
			}
		}

		void finalize_M1() {
			geo_assert(M2.nb_lines() == 0);

			// find M1^n (n-> infty)
			FOR(i, M1.size()) {
				while (M1[i].index != M1[M1[i].index].index) {
					M1[i].a *= M1[M1[i].index].a;
					M1[i].index = M1[M1[i].index].index;
				}
			}

			// compress
			index_t M1_nb_col = 0;
			vector<index_t> to_grp(M1.size(), NOT_AN_ID);
			FOR(i, M1.size()) if (M1[i].index == i) {
				to_grp[i] = M1_nb_col;
				M1_nb_col++;
			}
			vector<int> new_multiplicity(M1_nb_col, 0);
			FOR(i, M1.size()) {
				index_t g = to_grp[M1[i].index];
				new_multiplicity[g] = std::max(new_multiplicity[g], multiplicity[M1[i].index]);
				M1[i].index = g;
			}
			multiplicity = new_multiplicity;
			GEO::Logger::out("HexDom")  << "M1 final size is " << M1.size() << " x " << M1_nb_col <<  std::endl;
			M2.init_identity(M1_nb_col);
			M2t.init_identity(M1_nb_col);
			
		}

		void finalize_M2() {
			// WARNING ! 
			// we do not compress here (there will be unused variables) 
			// some linear solver may not appreciate it...
			index_t M2_nb_col = 0;
			M2_nb_col = M2.M.size();
			kernel_size = M2_nb_col;
			multiplicity.resize(M2_nb_col);
			GEO::Logger::out("HexDom")  << "M2 final size is " << M2.nb_lines() << " x " << M2_nb_col <<  std::endl;
		}

		void end_pass(index_t p_pass) {
			geo_assert(pass == p_pass);
			if (pass == 0) {
				finalize_M0();
			}
			else if (pass == 1) {
				finalize_M1();
			}
			else if (pass == 2) {
				finalize_M2();
			} else {
				check_multiplicity();
			}
			pass++;
		}


		vector<Coeff> mult_by_M0(vector<Coeff>& c) {
			vector<Coeff> cM0;
			FOR(i, c.size()) if (M0[c[i].index] != NOT_AN_ID) cM0.push_back(Coeff(M0[c[i].index], c[i].a));
			return cM0;
		}

		vector<Coeff> mult_by_M1(vector<Coeff>& cM0) {
			vector<Coeff> cM0M1;
			FOR(i, cM0.size()) cM0M1.push_back(Coeff(M1[cM0[i].index].index, cM0[i].a*M1[cM0[i].index].a));
			order_AND_make_unique(cM0M1);
			return cM0M1;
		}

		vector<Coeff> mult_by_M2(vector<Coeff>& cM0M1) {
			vector<Coeff> cM0M1M2;
			mult(cM0M1, M2, cM0M1M2);
			// order_AND_make_unique(cM0M1M2); [BL]: already done in mult()
			return cM0M1M2;
		}

		void inplace_mult_M2_by_B(RemoveColMatrix& B) {
			vector<Coeff> diffline = B.line;
			add_to_sorted_sparse_vector(diffline, B.zero_col, -1.0);
			// [BL] replaced with "insert_in_sorted_vector"
			// diffline.push_back(Coeff(B.zero_col, -1.));
			// order_AND_make_unique(diffline);

			vector<Coeff> M2col = M2t.M[B.zero_col];
			FOR(nc, diffline.size()) {
				FOR(nl, M2col.size()) {
					index_t c = diffline[nc].index;
					index_t l = M2col[nl].index;
					double coeff = diffline[nc].a*M2col[nl].a;
					M2t.add(c, l, coeff);
					M2.add(l, c, coeff);
				}
			}
		}

		bool add_constraint(vector<Coeff>& c) {

			if (pass == 0) {
				if (c.size() == 1) {
					geo_assert(c[0].a != 0);
					M0[c[0].index] = NOT_AN_ID;
				}
				return true;
			}

			vector<Coeff> cM0 = mult_by_M0(c);

			vector<Coeff> cM0M1 = mult_by_M1(cM0);
			if (cM0M1.size() == 0) return  true;

			if (pass == 1) {
				if (cM0M1.size() == 2) {
					FOR(i, 2) if ((std::abs(cM0M1[i].a) != 1)) return false;
					RemoveColMatrix B(cM0M1);
					Coeff co[2] = { Coeff(B.zero_col, 1.0),B.line[0] };
					// find root
					FOR(i, 2) {
						while (co[i].index != M1[co[i].index].index) {
							co[i].a = co[i].a*M1[co[i].index].a;
							co[i].index = M1[co[i].index].index;
						}
					}
					// link
					if (multiplicity[co[0].index] < multiplicity[co[1].index])
						M1[co[0].index] = Coeff(co[1].index, co[1].a*co[0].a);
					else M1[co[1].index] = Coeff(co[0].index, co[1].a*co[0].a);

				}
				return  true;
			}

			vector<Coeff> cM0M1M2 = mult_by_M2(cM0M1);
			if (cM0M1M2.size() == 0) {
				return  true;
			}
			
			if (pass == 2) {
				index_t ind_to_remove = 0;
				FOR(i, cM0M1M2.size()) {
					if (cM0M1M2[i].a!=0 && 
						double(multiplicity[cM0M1M2[i].index])* std::abs(cM0M1M2[i].a) <
						double(multiplicity[cM0M1M2[ind_to_remove].index]) * std::abs(cM0M1M2[ind_to_remove].a)) {
						ind_to_remove = i;
					}
				}
				RemoveColMatrix B(cM0M1M2, ind_to_remove);
				inplace_mult_M2_by_B(B);
			}
			// debug
			geo_assert(pass != 3 || cM0M1M2.empty());// "Kernel must be complete ";
			return true;
			
		}

		void check_multiplicity() {
			std::cerr << " check_that multiplicity are respected\n";
			FOR(x, M0.size()) if (debug_var_multiplicity(x) < DEBUG_init_multiplicity[x]) {
				plop(x);
				plop(debug_var_multiplicity(x));
				plop(DEBUG_init_multiplicity[x]);
				show_line(x);
				geo_assert_not_reached;
			}
		}
		void show_line(index_t x) {
			vector<Coeff> res = get_line(x);
			plop(DEBUG_init_multiplicity[x]);
			FOR(i, res.size()) {
				std::cerr<< res[i].a<<" . "<< res[i].index<< " ( mul= " << multiplicity[res[i].index] <<" )\n";
			}
		}

		vector<Coeff> get_line(index_t l) {
			vector<Coeff> c(1, Coeff(l, 1.));
			vector<Coeff> cM0 = mult_by_M0(c);
			if (M1.empty()) return cM0;
			vector<Coeff> cM0M1 = mult_by_M1(cM0);
			if (M2.M.empty()) return cM0M1;
			return mult_by_M2(cM0M1);
		}

		double debug_var_multiplicity(index_t x) {
			double min_mult = 10000;
			vector<Coeff> res = get_line(x);
			FOR(i, res.size()) {
				min_mult = std::min(
					min_mult,
					double(multiplicity[res[i].index]) * std::abs(res[i].a)
				);
			}
			return min_mult;
		}

		vector<index_t> M0;			// indirection map that removes null variables (set when growing a sphere)
		vector<Coeff> M1;			// indirection map + coefficient that produces groups of variables (for boundary & cuts)
		NaiveSparseMatrix M2;		// removes extra DOF when joining boundaries & cuts
		NaiveSparseMatrix M2t;		// transposed of M2: speed up computations
		vector<int> multiplicity;	// the multiplicity constraint for each variable (solver variable, not user variable)
		index_t kernel_size;	  // number of final DOF
		index_t pass;		  // iteration in filling the constraint matrix
		vector<int> DEBUG_init_multiplicity;	// the multiplicity constraint for each variable (user variable)
	};



	struct EXPLORAGRAM_API MatrixMixedConstrainedSolver {

		MatrixMixedConstrainedSolver(index_t p_nb_vars) :
			M(p_nb_vars),
			size_(p_nb_vars) {
			snap_size = 0;
		}

		/**
			 * \brief Gets the number of variables.
		 * \return the number of variables.
		 */
		index_t size() const {
			return size_;
		}

		// set multiple (including integer) constraints
		void set_multiplicity(index_t id, int period) {
			geo_debug_assert(id < size());
			M.multiplicity[id] = period;
			M.DEBUG_init_multiplicity[id] = period;
		}

		// constraints are included by increasing nuber of variables (from 1 to 3). The last pass (==3) is just for checking.
		void end_pass(index_t p_pass) {
			M.end_pass(p_pass);
		}

		// add a new constraint
		void begin_constraint() {
			new_constraint.clear();
		}

		void add_constraint_coeff(index_t id, double coeff) {
			geo_debug_assert(id < size());
			if (coeff != 0.0) {
				new_constraint.push_back(Coeff(id, coeff));
			}
		}

		bool  end_constraint() {
			return M.add_constraint(new_constraint);
		}
		vector<Coeff> new_constraint;

		// start  / end solving iterations
		void start_new_iter() {
			std::cerr << "New LS iteration \n";
			bool first_iter = V.empty();
			if (first_iter) {
				fixed.resize(M.kernel_size, false);
				V.resize(M.kernel_size);
			}
			nlNewContext();
			nlSolverParameteri(NL_LEAST_SQUARES, NL_TRUE);
			nlSolverParameteri(NL_NB_VARIABLES, NLint(M.kernel_size));
			nlBegin(NL_SYSTEM);

			if (!first_iter) {
				static const double auto_snap_threshold = 1.;// .005;// 25;
				double snap_threshold = 1e20;
				int nb_fixed_var = 0;
				bool snap_size_has_changed = false;
				do {
					std::cerr << "Try enforce mutiplicity equality with snap size  " << snap_size << "\n";
					FOR(pass, 2) {
						//FOR(i, M.kernel_size) if (M.multiplicity[i]>0)std::cerr << i << "  " << M.multiplicity[i] << "\n";
						FOR(i, M.kernel_size) {
							nlSetVariable(i, V[i]);

							if (M.multiplicity[i] < snap_size) continue;
							if (M.multiplicity[i] ==0 ) continue;
							if (M.M2t.nb_coeffs_in_line(i) == 0) continue;
							if (pass == 0 && fixed[i]) continue;
							double val = V[i];
							double snapped_val = double(M.multiplicity[i]) * nint(V[i] / double(M.multiplicity[i]));
							double dist = std::abs(snapped_val - val) / double(M.multiplicity[i]);
							if (pass == 0) {
								if (dist < snap_threshold
									&& snap_threshold>auto_snap_threshold) {
									snap_threshold = std::max(dist + .001, auto_snap_threshold);
								}
								continue;
							}

							//if (!fixed[i]) {
							//	std::cerr << std::setprecision(2);
							//	std::cerr << snap_threshold << "            " << dist << "  " << val << "(" << M.multiplicity[i] << ")\n";
							//	if (dist < snap_threshold)
							//		std::cerr << snap_threshold << " < " << dist << " < " << 1.-	snap_threshold << "\n";
							//}
							
							if (dist < snap_threshold && M.multiplicity[i] != 1000) {
								if (!fixed[i])  nb_fixed_var++;
								fixed[i] = true;
								GEO::Logger::out("HexDom")  << " i " << i << "  snapped_val" << snapped_val << " (V[i] " << V[i] <<  std::endl;
								nlSetVariable(i, snapped_val);
								nlLockVariable(i);
							}
						}
					}

					snap_size_has_changed = false;
					if (nb_fixed_var == 0) {
						if (snap_size >= 1) {
							snap_size_has_changed = true;
							snap_size /= 2;
						}
					}
					plop(nb_fixed_var);
				} while (snap_size_has_changed);
			}
			nlBegin(NL_MATRIX);
		}

		bool converged() {
			
			if (fixed.empty()) return false;
			FOR(i, M.kernel_size)
				if (M.multiplicity[i] > 0
					&& !fixed[i]
					&& M.M2t.nb_coeffs_in_line(i) > 0) return false;
			return true;
		}

		void end_iter() {
			nlEnd(NL_MATRIX);
			nlEnd(NL_SYSTEM);
			nlEnable(NL_VERBOSE);
			nlSolve();
			FOR(i, M.kernel_size) {
				V[i] = nlGetVariable(i);
			}
			nlDeleteContext(nlGetCurrent());
		}

		// set up energy
		void begin_energy() { nlBegin(NL_ROW); }
		void add_energy_coeff(index_t id, double coeff) {
			geo_debug_assert(id < size());
			if (coeff == 0.0) {
				return;
			}
			vector<Coeff> line = M.get_line(id);
			FOR(m, line.size()) {
				nlCoefficient(line[m].index, line[m].a*coeff);
			}
		}
		void add_energy_rhs(double rhs) { nlRightHandSide(rhs); }

		void end_energy() { nlEnd(NL_ROW); }

		// access to the result ;)
		double value(index_t i) {
			geo_debug_assert(i < size());
			double res = 0;
			vector<Coeff> line = M.get_line(i);
			FOR(m, line.size()) {
				res += V[line[m].index] * line[m].a;
			}
			return res;
		}

		double show_var(index_t i) {
			geo_debug_assert(i < size());
			double res = 0;
			vector<Coeff> line = M.get_line(i);
			std::cerr << " var " << i << " = ";
			FOR(m, line.size()) {
				std::cerr << line[m].a << " X " << V[line[m].index] << " + ";
				res += V[line[m].index] * line[m].a;
			}
			GEO::Logger::out("HexDom")  <<  std::endl;
			return res;
		}


		bool solved;
		MatrixM M;					// Matrix + multiplicity of variables
		index_t size_;
		std::vector<double> V;		// inner variables
		std::vector<bool> fixed;	// is it fixed already ?
		int snap_size;
	};

}


#endif


