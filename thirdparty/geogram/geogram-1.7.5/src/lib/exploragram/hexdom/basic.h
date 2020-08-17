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

#ifndef H_HEXDOM_ALGO_BASIC_H
#define H_HEXDOM_ALGO_BASIC_H

#include <exploragram/basic/common.h>
#include <geogram/basic/geometry.h>
#include <geogram/basic/string.h>
#include <cmath>
#include <algorithm>
#include <assert.h>
#include <time.h>

#ifndef FOR
#define FOR(i,max) for (index_t i = 0; i<index_t(max); i++)
#endif

namespace GEO {

	struct FF_param {
		FF_param();
		bool rigid_border;
	};
	struct HexdomParam {
		static FF_param  FF;
	};

	template<class T> void min_equal(T& A, T B) { if (A > B) A = B; }
	template<class T> void max_equal(T& A, T B) { if (A < B) A = B; }

	inline double nint(double x) { return floor(x + .5); }

    inline index_t next_mod(index_t i, index_t imax) {
	return (i + 1) % imax;
    }

    inline index_t prev_mod(index_t i, index_t imax) {
	return (i + index_t(int(imax) - 1)) % imax;
    }

    template <class T> T clamp(T in, T vmin, T vmax) {
	if (in<vmin) return vmin;
	if (in>vmax) return vmax;
	return in;
    }

    const index_t NOT_AN_ID = index_t(-1);

    struct EXPLORAGRAM_API  IdPair : public std::pair < index_t, index_t > {
        IdPair(index_t a = index_t(-1), index_t b = index_t(-1)) : std::pair < index_t, index_t >(a, b) {
	}
    };

    typedef vecng<3, Numeric::int32> vec3i;

    inline void show(mat3 r) {
        std::cerr << "\n";
        FOR(i, 3) {
            std::cerr << "\n";
            FOR(j, 3) {
                std::cerr << "   " << r(i, j);
            }
        }
    }
    
    inline vec3 col(const mat3& M, index_t j) {
        return vec3(M(0, j), M(1, j), M(2, j));
    }
    
    inline vec3 operator*(const mat3& M, const vec3& v) {
        return vec3(
            M(0, 0)*v[0] + M(0, 1)*v[1] + M(0, 2)*v[2],
            M(1, 0)*v[0] + M(1, 1)*v[1] + M(1, 2)*v[2],
            M(2, 0)*v[0] + M(2, 1)*v[1] + M(2, 2)*v[2]
        );
    }

    inline vec3i operator*(const mat3& M, const vec3i& v) {
        return vec3i(
            int(M(0, 0)*v[0] + M(0, 1)*v[1] + M(0, 2)*v[2]),
            int(M(1, 0)*v[0] + M(1, 1)*v[1] + M(1, 2)*v[2]),
            int(M(2, 0)*v[0] + M(2, 1)*v[1] + M(2, 2)*v[2])
        );
    }

    inline mat3 mat3_from_coeffs(double a00, double a01, double a02, double a10, double a11, double a12, double a20, double a21, double a22) {
        mat3 res;
        res(0, 0) = a00;        res(0, 1) = a01;        res(0, 2) = a02;
        res(1, 0) = a10;        res(1, 1) = a11;        res(1, 2) = a12;
        res(2, 0) = a20;        res(2, 1) = a21;        res(2, 2) = a22;
        return res;
    }
    
    inline mat3 mat3_from_coeffs(double* c) {
        mat3 res;
        FOR(i, 9) res.data()[i] = c[i];
	return res;
    }

    inline double trace(const mat3& m) { return m(0, 0) + m(1, 1) + m(2, 2); }
    
    inline double Frobenius_norm(const mat3& m) {return trace(m.transpose()*m);}// easy to optimmize
    
    inline vec2 operator*(const mat2& M, const vec2& v) {
	return vec2(M(0, 0)*v[0] + M(0, 1)*v[1], M(1, 0)*v[0] + M(1, 1)*v[1]) ;
    }

    
    inline vec3i snap_to_integer(const vec3& f) {
	return vec3i(int(round(f[0])), int(round(f[1])), int(round(f[2])));
    }

    inline bool is_integer(double d) {
	return d == floor(d);
    }


    // a une periode pres    
    template <class T> inline 
        T& aupp(int id, vector<T>& data){
		while (id <0) id += int(data.size());
		while (id >= int(data.size())) id -= int(data.size());
		return data[id];
    }

    // a une periode pres    
    template <class T> inline 
        T& aupp(index_t id, vector<T>& data){
	while (id >= data.size()) id -= data.size();
	return data[id];
    }




struct EXPLORAGRAM_API BBox1 {
	BBox1() { min = 1e20; max = -1e20; }
	double length() { return max - min; }
	bool intersect(const BBox1& b) const { return contains(b.min) || contains(b.max) || b.contains(min) || b.contains(max); }
	bool contains(const double& v) const { return v > min && v < max; }
	bool is_null() const;
	void add(const BBox1& b);
	void add(const double& P) { min_equal(min, P); max_equal(max, P);}
	void dilate(double eps) { min -= eps; max += eps; }
	double bary() const { return (max + min) / 2.; }

	double min;
	double max;
};
}




/**************** debugging and logging ***************************/

#define reach(x){GEO::Logger::out("HexDom") <<"\n=========    mark    ===>    "<<#x<<"    =============\n"<< std::endl; }


#define check1 GEO::Logger::out("HexDom") <<"checkpoint 1 reached"<< std::endl;
#define check2 GEO::Logger::out("HexDom") <<"checkpoint 2 reached"<< std::endl;
#define check3 GEO::Logger::out("HexDom") <<"checkpoint 3 reached"<< std::endl;


std::string EXPLORAGRAM_API plop_file(const char* file, int line);

template <class T> inline std::string plop_val(T val) {
    return "  =>  " + GEO::String::to_string(val);
}

template <> inline std::string plop_val(const char*) {
    return "";
}

inline std::string plop_unquote(const char* str) {
   std::string result(str);
   if(result.length() > 2 && result[0] == '\"' && result[result.length()-1] == '\"') {
      result = result.substr(1, result.length()-2);
   }
   return result;
}

#define plop(x) GEO::Logger::out("HexDom")  << "    ->|plop|<-     " << plop_file(__FILE__, __LINE__) << " : " << plop_unquote(#x) << plop_val(x) <<  std::endl
#define error(x) GEO::Logger::out("HexDom")  << "ERROR " << plop_file(__FILE__, __LINE__) << " : " << plop_unquote(#x) << plop_val(x) <<  std::endl

/***************** OS/multithreading *******************************/

#ifdef GEO_OPENMP
#define get_thread_range(nb_tasks,istart,iend)	\
    index_t istart ;				\
    index_t iend ;				\
    {int thread_id = omp_get_thread_num();	\
	int n_threads = omp_get_num_threads();			        \
	istart = index_t((thread_id*int(nb_tasks)) / n_threads);	\
	iend = index_t(((thread_id + 1)*int(nb_tasks)) / n_threads);	\
	if (thread_id == n_threads - 1) iend = nb_tasks;		\
    }
#else
#define get_thread_range(nb_tasks,istart,iend)	\
    index_t istart=0;	   \
    index_t iend=nb_tasks; 
#endif

#endif
