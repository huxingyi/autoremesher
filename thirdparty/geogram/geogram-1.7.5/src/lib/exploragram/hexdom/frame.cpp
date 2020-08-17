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

#include <exploragram/hexdom/frame.h>

namespace GEO {

    mat3 normalize_columns(const mat3& B) {
        mat3 res;
        vec3 n;
        FOR(j, 3) n[j] = col(B, j).length(); //= std::sqrt(pow(B(0, j), 2) + pow(B(1, j), 2) + pow(B(2, j), 2));
        FOR(j, 3) geo_assert(n[j] > 1e-20);
        FOR(i, 3)FOR(j, 3) res(i, j) = B(i, j) / n[j];
        return res;
    }

    mat3 invert_columns_norm(const mat3& B) {
        mat3 res;
        vec3 n;
        FOR(j, 3) n[j] = col(B, j).length2();// pow(B(0, j), 2) + pow(B(1, j), 2) + pow(B(2, j), 2);
        FOR(j, 3) geo_assert(n[j] > 1e-20);
        FOR(i, 3)FOR(j, 3) res(i, j) = B(i, j) / n[j];
        return res;
    }

    /**************************************************************************************************/

    mat3 rotx(double angle) {
        double c = cos(angle);
        double s = sin(angle);
        mat3 res = mat3_from_coeffs(
            1,0,0,
            0,c,-s,
            0,s,c
        );
        return res;
    }
    
    mat3 roty(double angle) {
        double c = cos(angle);
        double s = sin(angle);
        mat3 res = mat3_from_coeffs(
            c, 0, s,
            0, 1, 0,
            -s, 0, c
        );
        return res;
    }
    
    mat3 rotz(double angle) {
        double c = cos(angle);
        double s = sin(angle);
        mat3 res = mat3_from_coeffs(
            c, -s, 0,
            s, c, 0,
            0, 0, 1
        );
        return res;
    }
    
    // non optimized version is  "return rotz(xyz[2]) *roty(xyz[1]) *rotx(xyz[0]);"
    mat3 euler_to_mat3(vec3 xyz) {
        double ca = cos(xyz[0]), sa = sin(xyz[0]);
        double cb = cos(xyz[1]), sb = sin(xyz[1]);
        double cg = cos(xyz[2]), sg = sin(xyz[2]);
        mat3 res = mat3_from_coeffs(
            cb*cg,      cg*sa*sb - ca*sg,   ca*cg*sb + sa*sg,
            cb*sg,      sa*sb*sg + ca*cg,   ca*sb*sg - cg*sa,
            -sb,        cb*sa,              ca*cb
        );
        return res;
    }

    vec3 mat3_to_euler(const mat3& r) {//http://www.staff.city.ac.uk/~sbbh653/publications/euler.pdf
        vec3 res;
        if (std::abs(std::abs(r(2, 0)) - 1) > 1e-5) {
            res[1] = -asin(r(2, 0));
            res[0] = atan2(r(2, 1), r(2, 2));
            res[2] = atan2(r(1, 0), r(0, 0));
        } else {
            res[2] = 0;
            if (std::abs(r(2, 0) + 1) < 1e-5) {
                res[1] = M_PI / 2.;
                res[0] = atan2(r(0, 1), r(0, 2));
            } else {
                res[1] = -M_PI / 2.;
                res[0] = atan2(-r(0, 1), -r(0, 2));
            }
        }
        return res;
    }


    /***********************************************************************************************************************************/

    //void generate_AxisPermutations_init_code() {
    //    mat3 r90z = mat3_from_coeffs(0, -1, 0, 1, 0, 0, 0, 0, 1);
    //    FOR(inv, 2) FOR(perm, 3) FOR(rotz, 4) {
    //        mat3 m;
    //        m.load_identity();
    //        if (inv == 1) m = mat3_from_coeffs(0, 0, -1, 0, -1, 0, -1, 0, 0);

    //        FOR(p, perm) FOR(d, 3) {
    //            double tmp = m(d, 0);
    //            m(d, 0) = m(d, 1);
    //            m(d, 1) = m(d, 2);
    //            m(d, 2) = tmp;
    //        }
    //        FOR(r, rotz) m = r90z*m;
    //        FOR(i, 3)FOR(j, 3) m(i, j) = std::floor(m(i, j) + 0.5);
    //        AxisPermutations[12 * inv + 4 * perm + rotz] = m;
    //    }
    //    FOR(i, 24)FOR(j, 24) if ((AxisPermutations[i] * AxisPermutations[j]).is_identity()) AxisPermutations_inv[i] = j;


    //    std::cerr << "static mat3 AxisPermutations[24] = { ";
    //    FOR(i, 24) {
    //        double* ptr = AxisPermutations[i].data();
    //        std::cerr << "mat3_from_coeffs(";
    //        std::cerr << ptr[0];
    //        FOR(d, 8) std::cerr << "," << ptr[d + 1];
    //        if (i<23)std::cerr << ")," << ((i % 4) ? "" : "\n");
    //        else std::cerr << ")};\n";
    //    }

    //    std::cerr << "static int AxisPermutations_inv[24] = { ";
    //    FOR(i, 24) {
    //        if (i<23)std::cerr << AxisPermutations_inv[i] << ",";
    //        else std::cerr << AxisPermutations_inv[i] << "};\n";
    //    }
    //};
    
    static mat3 AxisPermutations[24] = {
	mat3_from_coeffs(1,0,0,0,1,0,0,0,1),
        mat3_from_coeffs(0,-1,0,1,0,0,0,0,1),mat3_from_coeffs(-1,0,0,0,-1,0,0,0,1),mat3_from_coeffs(0,1,0,-1,0,0,0,0,1),mat3_from_coeffs(0,0,1,1,0,0,0,1,0),
        mat3_from_coeffs(-1,0,0,0,0,1,0,1,0),mat3_from_coeffs(0,0,-1,-1,0,0,0,1,0),mat3_from_coeffs(1,0,0,0,0,-1,0,1,0),mat3_from_coeffs(0,1,0,0,0,1,1,0,0),
        mat3_from_coeffs(0,0,-1,0,1,0,1,0,0),mat3_from_coeffs(0,-1,0,0,0,-1,1,0,0),mat3_from_coeffs(0,0,1,0,-1,0,1,0,0),mat3_from_coeffs(0,0,-1,0,-1,0,-1,0,0),
        mat3_from_coeffs(0,1,0,0,0,-1,-1,0,0),mat3_from_coeffs(0,0,1,0,1,0,-1,0,0),mat3_from_coeffs(0,-1,0,0,0,1,-1,0,0),mat3_from_coeffs(0,-1,0,-1,0,0,0,0,-1),
        mat3_from_coeffs(1,0,0,0,-1,0,0,0,-1),mat3_from_coeffs(0,1,0,1,0,0,0,0,-1),mat3_from_coeffs(-1,0,0,0,1,0,0,0,-1),mat3_from_coeffs(-1,0,0,0,0,-1,0,-1,0),
        mat3_from_coeffs(0,0,1,-1,0,0,0,-1,0),mat3_from_coeffs(1,0,0,0,0,1,0,-1,0),mat3_from_coeffs(0,0,-1,1,0,0,0,-1,0)
    };

    static index_t AxisPermutations_inv[24] = { 0,3,2,1,8,5,15,22,4,14,21,11,12,23,9,6,16,17,18,19,20,10,7,13 };

    void AxisPermutation::aligns_B_wrt_ref(mat3 ref, mat3 B) {
            ref= normalize_columns(ref);
            B = normalize_columns(B);
            index_t best_i = 0;
            double best_score = 1e20;
            FOR(i, 24) {
                double score = Frobenius_norm(B*AxisPermutations[i]-ref);
                if (score < best_score) {
                    best_i = i;
                    best_score = score;
                }
            }
            mid = best_i;
        }

     void AxisPermutation::make_col2_equal_to_z(mat3 B, vec3 z) {
            B = normalize_columns(B);
            index_t best_i = 0;
            double best_score = 1e20;
            FOR(i, 24) {
                double score = (col(B*AxisPermutations[i],2) - z).length2();
                if (score < best_score) {
                    best_i = i;
                    best_score = score;
                }
            }
            mid = best_i;
        }

    const mat3& AxisPermutation::get_mat() const {
	return AxisPermutations[mid];
    }

    AxisPermutation AxisPermutation::inverse() {
	return AxisPermutation(AxisPermutations_inv[mid]);
    }

    /***********************************************************************************************************************************/
    
    void Frame::make_z_equal_to(vec3 z) {
	z = normalize(z);
	vec3 x(1, 0, 0);
	vec3 y = cross(z, x);
	if (y.length2() < .1) {
	    x = vec3(0, 1, 0);
	    y = cross(z, x);
	}   
	y = normalize(y);
	x = cross(y, z);
	FOR(d, 3) r(d, 0) = x[d];
	FOR(d, 3) r(d, 1) = y[d];
	FOR(d, 3) r(d, 2) = z[d];
    }

    mat3 Frame::average_frame(vector<mat3>& data) {
	SphericalHarmonicL4 sum;
	FOR(i, data.size()) {
	    SphericalHarmonicL4 nv;
	    nv[4] = std::sqrt(7. / 12.);
	    nv[8] = std::sqrt(5. / 12.);
	    nv.euler_rot(mat3_to_euler(data[i]));
	    sum = sum + nv;
	}
	sum = sum*(1. / sum.norm());
	return sum.project_mat3();
    }

    mat3 Frame::representative_frame(vector<vec3>& bunch_of_vectors, vector<double>& w) {
	SphericalHarmonicL4 sum;
	FOR(i,bunch_of_vectors.size()) {
	    vec3 n = normalize(bunch_of_vectors[i]);
	    mat3 r;
	    Frame(r).make_z_equal_to(n);
	    SphericalHarmonicL4 nv;
	    nv[4] = std::sqrt(7. / 12.);
	    nv.euler_rot(mat3_to_euler(r));
	    nv = nv * w[i];
	    sum = sum + nv;
	}
	sum = sum*(1. / sum.norm());
	return sum.project_mat3();
    }
    
    mat3 Frame::representative_frame(vector<vec3>& bunch_of_vectors) {
	vector<double> w(bunch_of_vectors.size(), 1);
	return  representative_frame(bunch_of_vectors, w);
    }

    /***********************************************************************************************************************************/
    
    AxisPermutation Rij(Mesh* m, Attribute<mat3>& B, index_t i, index_t j) {
        if (i > j)  return Rij(m, B, j, i).inverse(); // façon obscure de rendre le bazar symmetrique ?
        AxisPermutation permut;
        permut.aligns_B_wrt_ref(B[i], B[j]);
        return permut;
    }

	bool triangle_is_frame_singular(Mesh* m, Attribute<mat3>& B, index_t c, index_t cf) {
		mat3 ac_rot;
		ac_rot.load_identity();
		FOR(e, 3) ac_rot = Rij(m, B, m->cells.facet_vertex(c, cf, e), m->cells.facet_vertex(c, cf, next_mod(e, 3))).get_mat()*ac_rot;
		return !is_identity_auvp(ac_rot);
	}

	bool triangle_is_frame_singular___give_stable_direction(Mesh* m, int& stable_dir_index, Attribute<mat3>& B, index_t c, index_t cf, index_t cfv) {
		mat3 ac_rot,id3D;
		stable_dir_index = -1;
		ac_rot.load_identity();
		FOR(e, 3) ac_rot = Rij(m, B, m->cells.facet_vertex(c, cf, (e+ cfv)%3), m->cells.facet_vertex(c, cf, (e + cfv+1) % 3)).get_mat()*ac_rot;
		if (is_identity_auvp(ac_rot)) return false;
		FOR(ax, 3){
			double sum = 0;
			FOR(d, 3) sum += std::abs(ac_rot(d, ax) - id3D(d, ax));
			if (sum<1e-10) stable_dir_index = int(ax);
		}
		return true;
	}
	bool triangle_is_frame_singular___give_stable_direction(Mesh* m, vec3& stable_dir_geom, Attribute<mat3>& B, index_t c, index_t cf, index_t cfv) {
		int stable_dir_index;
		if (!triangle_is_frame_singular___give_stable_direction(m, stable_dir_index, B, c, cf,cfv)) return false; 
		stable_dir_geom = vec3(0, 0, 0);
		if (stable_dir_index == -1) return true;
		FOR(d, 3) stable_dir_geom[d] = B[m->cells.facet_vertex(c, cf, cfv)](d, index_t(stable_dir_index));
		stable_dir_geom = normalize(stable_dir_geom);
		return true;
	}
    
}

