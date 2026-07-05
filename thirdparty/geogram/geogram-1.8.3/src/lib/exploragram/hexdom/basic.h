/*
 *  Copyright (c) 2000-2022 Inria
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 *  * Neither the name of the ALICE Project-Team nor the names of its
 *  contributors may be used to endorse or promote products derived from this
 *  software without specific prior written permission.
 * 
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *  Contact: Bruno Levy
 *
 *     https://www.inria.fr/fr/bruno-levy
 *
 *     Inria,
 *     Domaine de Voluceau,
 *     78150 Le Chesnay - Rocquencourt
 *     FRANCE
 *
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
