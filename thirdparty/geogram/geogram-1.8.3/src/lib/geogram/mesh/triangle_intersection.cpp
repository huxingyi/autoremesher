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

#include <geogram/mesh/triangle_intersection.h>
#include <geogram/numerics/predicates.h>
#include <geogram/basic/string.h>
#include <geogram/basic/geometry.h>
#include <geogram/basic/argused.h>
#include <geogram/basic/logger.h>
#include <geogram/basic/algorithm.h>


#ifdef GEO_COMPILER_CLANG
#pragma GCC diagnostic ignored "-Wswitch-enum"
#endif

// Uncomment to activate debug messages
// #define TT_DEBUG

namespace {

    using namespace GEO;
    
    /**
     * \brief Internal implementation class for 
     *  triangles_intersections()
     * \details Keeps the coordinates of the 2x3
     *  vertices of the triangles and a reference
     *  to the result symbolic information
     */
    class TriangleTriangleIntersection {
    public:

        enum {CACHE_UNINITIALIZED = -2};
        
        TriangleTriangleIntersection(
            const vec3& p0, const vec3& p1, const vec3& p2,
            const vec3& q0, const vec3& q1, const vec3& q2,
            vector<TriangleIsect>* result = nullptr
        ) : result_(result) {
            p_[0] = p0;
            p_[1] = p1;
            p_[2] = p2;
            p_[3] = q0;
            p_[4] = q1;
            p_[5] = q2;
            for(index_t i=0; i<64; ++i) {
                o3d_cache_[i] = CACHE_UNINITIALIZED;
            }
            has_non_degenerate_intersection_ = false;
        }

        void compute() {
#ifdef TT_DEBUG
            Logger::out("TT") << "Call compute()" << std::endl;
#endif            
            
            if(result_ != nullptr) {
                result_->resize(0);
            }

            // Test for degenerate triangles
            if(
                triangle_dim(T1_RGN_P0, T1_RGN_P1, T1_RGN_P2) != 2 ||
                triangle_dim(T2_RGN_P0, T2_RGN_P1, T2_RGN_P2) != 2
            ) {
                /*
                Logger::warn("PCK") << "Tri tri intersect: degenerate triangle "
                                    << "(not supported)"
                                    << std::endl;
                */
                return;
            }

            // If T1 is strictly on one side of the supporting
            // plane of T2, then we are sure there is no intersection
            // and we can stop there.
            {
                TriangleRegion p1,p2,p3;
                TriangleRegion q1,q2,q3;
                get_triangle_vertices(T1_RGN_T, p1,p2,p3);
                get_triangle_vertices(T2_RGN_T, q1,q2,q3);
                Sign o1 = orient3d(q1,q2,q3,p1);
                Sign o2 = orient3d(q1,q2,q3,p2);
                Sign o3 = orient3d(q1,q2,q3,p3);
                if(
                    int(o1)*int(o2) == 1 &&
                    int(o2)*int(o3) == 1 &&
                    int(o3)*int(o1) == 1
                ) {
                    return;
                }
            }


            intersect_edge_triangle(T1_RGN_E0, T2_RGN_T);
            if(finished()) { return; }
            intersect_edge_triangle(T1_RGN_E1, T2_RGN_T);
            if(finished()) { return; }            
            intersect_edge_triangle(T1_RGN_E2, T2_RGN_T);
            if(finished()) { return; }            

            intersect_edge_triangle(T2_RGN_E0, T1_RGN_T);
            if(finished()) { return; }            
            intersect_edge_triangle(T2_RGN_E1, T1_RGN_T);
            if(finished()) { return; }            
            intersect_edge_triangle(T2_RGN_E2, T1_RGN_T);
            if(finished()) { return; }            
            
            // The same intersection can appear several times,
            // remove the duplicates
            if(result_ != nullptr) {
                sort_unique(*result_);
#ifdef TT_DEBUG
                std::string message;
                for(TriangleIsect I: *result_) {
                    message += (" " + String::to_string(I));
                }
                Logger::out("II") << "result: " << message << std::endl;
#endif                
            }
        }

        /**
         * \brief Tests if there as a non-degenerate intersection
         * \retval true if an intersection different from two colocated
         *  vertices was found.
         * \retval false otherwise.
         */
        bool has_non_degenerate_intersection() const {
            return has_non_degenerate_intersection_;
        }
        
    protected:

        /**
         * \brief Tests whether computation is finished
         * \details If we just want to know whether there is an intersection
         *  then we can stop sooner.
         */
        bool finished() const {
#ifdef TT_DEBUG            
            return false;
#endif            
            return (result_ == nullptr && has_non_degenerate_intersection_);
        }
        
        void intersect_edge_triangle(TriangleRegion E, TriangleRegion T) {

#ifdef TT_DEBUG
            Logger::out("TT") << std::endl;
            Logger::out("TT") << "   ET " << std::make_pair(E,T) << std::endl;
#endif            
            
            geo_debug_assert(region_dim(E) == 1);            
            geo_debug_assert(region_dim(T) == 2);

            TriangleRegion R1 = E;
            TriangleRegion R2 = T;
            
            TriangleRegion p1,p2,p3;
            TriangleRegion e1,e2,e3;
            TriangleRegion q1,q2;
            
            get_triangle_vertices(T,p1,p2,p3);
            get_triangle_edges(T,e1,e2,e3);            
            get_edge_vertices(E,q1,q2);

            Sign o1 = orient3d(p1,p2,p3,q1);
            Sign o2 = orient3d(p1,p2,p3,q2);

            // If both extremities of the segment on same side of triangle
            // plane, then we are sure there is no intersection and we can
            // stop there.
            if(int(o1) * int(o2) == 1) {
                return;
            }
            
            if(o1 == 0 && o2 == 0) {
#ifdef TT_DEBUG
            Logger::out("TT") << "      ET coplanar" << std::endl;
#endif            
                // Special case: triangle and segment are co-planar
                index_t nax = normal_axis(p1,p2,p3);

                // Test whether the extremities of the segment
                // are in the triangle
                {
                    Sign a1 = orient2d(q1,p1,p2,nax);
                    Sign a2 = orient2d(q1,p2,p3,nax);
                    Sign a3 = orient2d(q1,p3,p1,nax);                

                    Sign b1 = orient2d(q2,p1,p2,nax);
                    Sign b2 = orient2d(q2,p2,p3,nax);
                    Sign b3 = orient2d(q2,p3,p1,nax);                

                    if(
                        int(a1)*int(a2) > 0 &&
                        int(a2)*int(a3) > 0 &&
                        int(a3)*int(a1) > 0 
                    ) {
                        add_intersection(q1,T);
                        if(finished()) { return; }
                    }

                    if(
                        int(b1)*int(b2) > 0 &&
                        int(b2)*int(b3) > 0 &&
                        int(b3)*int(b1) > 0 
                    ) {
                        add_intersection(q2,T);
                        if(finished()) { return; }                        
                    }
                }
                
                intersect_edge_edge_2d(E,e1,nax);
                if(finished()) { return; }
                intersect_edge_edge_2d(E,e2,nax);
                if(finished()) { return; }                
                intersect_edge_edge_2d(E,e3,nax);
                if(finished()) { return; }                
                
            } else {

                
                // Update symbolic information of segment
                // if one of the segment vertices is on
                // the triangle's supporting plane.
                
                if(o1 == ZERO) {
                    R1 = q1;
                } else if(o2 == ZERO) {
                    R1 = q2;
                }
                
                Sign oo1 = orient3d(p2,p3,q1,q2);
                Sign oo2 = orient3d(p3,p1,q1,q2);

                if(int(oo1)*int(oo2) == -1) {
                    return;
                }
                
                Sign oo3 = orient3d(p1,p2,q1,q2);
                
                // Update symbolic information of triangle
                // if intersection is
                // on a vertex or on an edge
                int nb_zeros = (oo1 == ZERO) + (oo2 == ZERO) + (oo3 == ZERO);
                geo_debug_assert(nb_zeros != 3);
                if(nb_zeros == 1) {
                    if(oo1 == ZERO) {
                        R2 = e1;
                    } else if(oo2 == ZERO) {
                        R2 = e2;                        
                    } else {
                        R2 = e3;                        
                    }
                } else if(nb_zeros == 2) {
                    if(oo1 != ZERO) {
                        R2 = p1;
                    } else if(oo2 != ZERO) {
                        R2 = p2;
                    } else {
                        R2 = p3;
                    }
                }

#ifdef TT_DEBUG
                Logger::out("TT") << o1 << " " << o2
                                  << "     "
                                  << oo1 << " " << oo2 << " " << oo3
                                  << std::endl;
#endif                
                // Intersection is outside triangle if oo1, oo2 and oo3
                // do not have the same sign (or zero)
                bool outside =
                    (int(oo1) * int(oo2) == -1) ||
                    (int(oo2) * int(oo3) == -1) ||
                    (int(oo3) * int(oo1) == -1) ;

                if(!outside) {
                    add_intersection(R1,R2);
                }
            }
        }
        

        void intersect_edge_edge_2d(
            TriangleRegion E1, TriangleRegion E2, index_t nax
        ) {
#ifdef TT_DEBUG
            Logger::out("TT") << "   EE 2d " << std::make_pair(E1,E2)
                              << " axis: " << nax << std::endl;
#endif            
            geo_debug_assert(region_dim(E1) == 1);
            geo_debug_assert(region_dim(E2) == 1);

            TriangleRegion R1 = E1;
            TriangleRegion R2 = E2;
            
            TriangleRegion p1,p2;
            get_edge_vertices(E1,p1,p2);
            
            TriangleRegion q1,q2;
            get_edge_vertices(E2,q1,q2);

            Sign a1 = orient2d(q1,q2,p1,nax);
            Sign a2 = orient2d(q1,q2,p2,nax);

            if(a1 == ZERO && a2 == ZERO) {
                // Special case: 1D
                // (the 2x2 edge extremities are aligned)
                intersect_edge_edge_1d(E1,E2);
            } else {
                // Update symbolic information if one of E1's vertices is
                // on the supporting line of E2
                if(a1 == ZERO) {
                    R1 = p1;
                } else if(a2 == ZERO) {
                    R1 = p2;
                }
                
                Sign b1 = orient2d(p1,p2,q1,nax);
                Sign b2 = orient2d(p1,p2,q2,nax);
                
                // Update symbolic information if one of E2's vertices is
                // on the supporting line of E1
                if(b1 == ZERO) {
                    R2 = q1;
                } else if(b2 == ZERO) {
                    R2 = q2;
                }
                
                if( int(a1)*int(a2) != 1 && int(b1)*int(b2) != 1) {
                    add_intersection(R1,R2);
                }
            }
        }

        void intersect_edge_edge_1d(
            TriangleRegion E1, TriangleRegion E2
        ) {
#ifdef TT_DEBUG
            Logger::out("TT") << "   EE 1d " << std::make_pair(E1,E2)
                              << std::endl;
#endif            
            geo_debug_assert(region_dim(E1) == 1);
            geo_debug_assert(region_dim(E2) == 1);

            TriangleRegion p1,p2;
            get_edge_vertices(E1,p1,p2);
            
            TriangleRegion q1,q2;
            get_edge_vertices(E2,q1,q2);
            
            Sign d1 = dot3d(p1,q1,q2);
            Sign d2 = dot3d(p2,q1,q2);                
            Sign d3 = dot3d(q1,p1,p2);
            Sign d4 = dot3d(q2,p1,p2);                
            
            // Test for identical vertices
            // (small optimization, each time there is an identical pair
            //  of points, the two dot3d predicates they participiate to
            //  should return ZERO, so we can filter)
            
            if(d1 == ZERO && d3 == ZERO && points_are_identical(p1,q1)) {
                add_intersection(p1,q1);
            }
                
            if(d2 == ZERO && d3 == ZERO && points_are_identical(p2,q1)) {
                add_intersection(p2,q1);
            }
            
            if(d1 == ZERO && d4 == ZERO && points_are_identical(p1,q2)) {
                add_intersection(p1,q2);
            }
            
            if(d2 == ZERO && d4 == ZERO && points_are_identical(p2,q2)) {
                add_intersection(p2,q2);
            }
            
            // Test for point in segment
                
            if(d1 == NEGATIVE) {
                add_intersection(p1,E2);
            }

            if(d2 == NEGATIVE) {
                add_intersection(p2,E2);
            }
                
            if(d3 == NEGATIVE) {
                add_intersection(E1,q1);
            }

            if(d4 == NEGATIVE) {
                add_intersection(E1,q2);
            }
        }
        
        void add_intersection(TriangleRegion R1, TriangleRegion R2) {
#ifdef TT_DEBUG
            Logger::out("TT") << "      ==>I: " << std::make_pair(R1,R2)
                              << std::endl;
#endif            
            if(region_dim(R1) >= 1 || region_dim(R2) >= 1) {
                has_non_degenerate_intersection_ = true;
            }
            if(is_in_T1(R1)) {
                geo_debug_assert(!is_in_T1(R2));
                if(result_ != nullptr) {
                    result_->push_back(std::make_pair(R1,R2));
                }
            } else {
                geo_debug_assert(is_in_T1(R2));
                if(result_ != nullptr) {
                    result_->push_back(std::make_pair(R2,R1));
                }
            }
        }

        Sign orient3d(
            TriangleRegion i, TriangleRegion j,
            TriangleRegion k, TriangleRegion l
        ) const {

            geo_debug_assert(region_dim(i) == 0);
            geo_debug_assert(region_dim(j) == 0);
            geo_debug_assert(region_dim(k) == 0);
            geo_debug_assert(region_dim(l) == 0);

            // Index for the cache (1 bit set for
            // each vertex)
            
            index_t o3d_idx =
                (1u << index_t(i)) |
                (1u << index_t(j)) |
                (1u << index_t(k)) |
                (1u << index_t(l)) ;                

            geo_debug_assert(o3d_idx < 64);

            // Result of the predicate should be flipped if the
            // order of the arguments is permutted by an odd permutation
            
            bool flip = odd_order(index_t(i),index_t(j),index_t(k),index_t(l));

            // If cache not initialized, set cache value
            
            if(o3d_cache_[o3d_idx] == CACHE_UNINITIALIZED) {
                int o = flip ? -int(PCK::orient_3d(p_[i],p_[j],p_[k],p_[l])) :
                                int(PCK::orient_3d(p_[i],p_[j],p_[k],p_[l])) ;
                o3d_cache_[o3d_idx] = Numeric::int8(o);
            }

            // Get result from the cache
            
            Sign result =  flip ? Sign(-o3d_cache_[o3d_idx])
                                : Sign( o3d_cache_[o3d_idx]);

            // Sanity check: did our cache return the same result as
            // directly calling the predicate ?
            geo_debug_assert(
                result == PCK::orient_3d(p_[i], p_[j], p_[k], p_[l])
            );
            
            return result;
        }

        /**
         * \brief Tests the parity of the permutation of a list of
         *  four distinct indices with respect to the canonical order.
         */  
        static bool odd_order(index_t i, index_t j, index_t k, index_t l) {
            // Implementation: sort the elements (bubble sort is OK for
            // such a small number), and invert parity each time
            // two elements are swapped.
            index_t tab[4] = { i, j, k, l };
            const int N = 4;
            bool result = false;
            for (int I = 0; I < N - 1; ++I) {
                for (int J = 0; J < N - I - 1; ++J) {
                    if (tab[J] > tab[J + 1]) {
                        std::swap(tab[J], tab[J + 1]);
                        result = !result;
                    }
                }
            }
            return result;
        }

        Sign orient2d(
            TriangleRegion i, TriangleRegion j, TriangleRegion k,
            index_t normal_axis
        ) const {
            // Note: no cache for orient2d (tested, did not bring
            // any performance gain).
            geo_debug_assert(region_dim(i) == 0);
            geo_debug_assert(region_dim(j) == 0);
            geo_debug_assert(region_dim(k) == 0);
            double pi[2];
            double pj[2];
            double pk[2];
            for(coord_index_t c = 0; c < 2; c++) {
                pi[c] = p_[i][index_t((normal_axis + 1 + c) % 3)];
                pj[c] = p_[j][index_t((normal_axis + 1 + c) % 3)];
                pk[c] = p_[k][index_t((normal_axis + 1 + c) % 3)];
            }
            return PCK::orient_2d(pi,pj,pk);
        }

        /**
         * \brief Computes the dot product between two vectors supported
         *  by three points.
         * \param[in] i , j , k the three points
         * \return sign(dot(pj-pi,pk-pi))
         */
        Sign dot3d(
            TriangleRegion i, TriangleRegion j, TriangleRegion k
        ) const {
            geo_debug_assert(region_dim(i) == 0);
            geo_debug_assert(region_dim(j) == 0);
            geo_debug_assert(region_dim(k) == 0);
            return PCK::dot_3d(p_[i].data(), p_[j].data(), p_[k].data());
        }
        
        
        /**
         * \brief Computes the coordinate along which a triangle can be
         *  projected without introducting degeneracies.
         * \param[in] v1 , v2 , v3 the three vertices of the triangle
         * \return the coordinate to be used for 2d computations (0,1 or 2)
         */
        coord_index_t normal_axis(
            TriangleRegion v1, TriangleRegion v2, TriangleRegion v3
        ) {
            geo_debug_assert(region_dim(v1) == 0);
            geo_debug_assert(region_dim(v2) == 0);
            geo_debug_assert(region_dim(v3) == 0);            
            
            const vec3& p1 = p_[v1];
            const vec3& p2 = p_[v2];
            const vec3& p3 = p_[v3];

            return Geom::triangle_normal_axis(p1,p2,p3);
        }

        
        bool points_are_identical(TriangleRegion i, TriangleRegion j) {
            geo_debug_assert(region_dim(i) == 0);
            geo_debug_assert(region_dim(j) == 0);
            const vec3& p1 = p_[i];
            const vec3& p2 = p_[j];            
            return (p1[0] == p2[0]) &&
                   (p1[1] == p2[1]) &&
                   (p1[2] == p2[2]) ;
        }

        /**
         * \brief Detects degenerate triangles
         * \retval 0 if all the vertices of the triangle are the same point
         * \retval 1 if the vertices of the triangle are co-linear
         * \retval 2 otherwise
         */
        index_t triangle_dim(
            TriangleRegion i, TriangleRegion j, TriangleRegion k
        ) {
            geo_debug_assert(region_dim(i) == 0);
            geo_debug_assert(region_dim(j) == 0);
            geo_debug_assert(region_dim(k) == 0);

            const vec3& p1 = p_[i];
            const vec3& p2 = p_[j];
            const vec3& p3 = p_[k];            
            
            if(!PCK::aligned_3d(p1.data(), p2.data(), p3.data())) {
                return 2;
            }
            if(points_are_identical(i,j) && points_are_identical(j,k)) {
                return 0;
            }
            return 1;
        }


    private:
        vec3 p_[6];
        vector<TriangleIsect>* result_;
        bool has_non_degenerate_intersection_;
        mutable Numeric::int8 o3d_cache_[64];
    };
    
}

/****************************************************************************/

namespace GEO {

    std::string region_to_string(TriangleRegion rgn) {
	const char* strs[T_RGN_NB] = {
            "T1.P0",
            "T1.P1",
            "T1.P2",

            "T2.P0",
            "T2.P1",
            "T2.P2",
        
            "T1.E0",
            "T1.E1",
            "T1.E2",

            "T2.E0",
            "T2.E1",
            "T2.E2",
        
            "T1.T", 
            "T2.T"
	};
	geo_assert(int(rgn) < int(T_RGN_NB));
	return strs[int(rgn)];
    }

    // This version returns the symbolic information.
    bool triangles_intersections(
        const vec3& p0, const vec3& p1, const vec3& p2,
        const vec3& q0, const vec3& q1, const vec3& q2,
        vector<TriangleIsect>& result
    ) {
	result.resize(0);
        TriangleTriangleIntersection I(
            p0, p1, p2,
            q0, q1, q2,
            &result
        );
        I.compute();
        return I.has_non_degenerate_intersection();
    }

    // This version is just a predicate (returns true if
    // there is a non-degenerate intersection, false
    // otherwise).
    bool triangles_intersections(
        const vec3& p0, const vec3& p1, const vec3& p2,
        const vec3& q0, const vec3& q1, const vec3& q2
    ) {
        TriangleTriangleIntersection I(
            p0, p1, p2,
            q0, q1, q2
        );
        I.compute();
        return I.has_non_degenerate_intersection();
    }

    coord_index_t region_dim(TriangleRegion r) {
        geo_debug_assert(index_t(r) < T_RGN_NB);
        static coord_index_t trgl_rgn_dim[T_RGN_NB] = {
            0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2
        };
        return trgl_rgn_dim[index_t(r)];
    }

    TriangleRegion swap_T1_T2(TriangleRegion R) {
        TriangleRegion result=T_RGN_NB;
        switch(R) {
        case T1_RGN_P0:
            result = T2_RGN_P0;
            break;
        case T1_RGN_P1:
            result = T2_RGN_P1;
            break;
        case T1_RGN_P2:
            result = T2_RGN_P2;
            break;
        case T2_RGN_P0:
            result = T1_RGN_P0;
            break;
        case T2_RGN_P1:
            result = T1_RGN_P1;
            break;
        case T2_RGN_P2:
            result = T1_RGN_P2;
            break;
        case T1_RGN_E0:
            result = T2_RGN_E0;
            break;
        case T1_RGN_E1:
            result = T2_RGN_E1;
            break;
        case T1_RGN_E2:
            result = T2_RGN_E2;
            break;
        case T2_RGN_E0:
            result = T1_RGN_E0;
            break;
        case T2_RGN_E1:
            result = T1_RGN_E1;
            break;
        case T2_RGN_E2:
            result = T1_RGN_E2;
            break;
        case T1_RGN_T:
            result = T2_RGN_T;
            break;
        case T2_RGN_T:
            result = T1_RGN_T;
            break;
        case T_RGN_NB:
            geo_assert_not_reached;
        }
        return result;
    }

    void get_triangle_vertices(
        TriangleRegion T,
        TriangleRegion& p0, TriangleRegion& p1, TriangleRegion& p2
    ) {
        geo_debug_assert(region_dim(T) == 2);
        switch(T) {
        case T1_RGN_T: {
            p0 = T1_RGN_P0; p1 = T1_RGN_P1; p2 = T1_RGN_P2; 
        } break;
        case T2_RGN_T: {
            p0 = T2_RGN_P0; p1 = T2_RGN_P1; p2 = T2_RGN_P2; 
        } break;
        default:
            geo_assert_not_reached;
        }
    }

    void get_triangle_edges(
        TriangleRegion T,
        TriangleRegion& e0, TriangleRegion& e1, TriangleRegion& e2
    ) {
        geo_debug_assert(region_dim(T) == 2);
        switch(T) {
        case T1_RGN_T: {
            e0 = T1_RGN_E0; e1 = T1_RGN_E1; e2 = T1_RGN_E2; 
        } break;
        case T2_RGN_T: {
            e0 = T2_RGN_E0; e1 = T2_RGN_E1; e2 = T2_RGN_E2; 
        } break;
        default:
            geo_assert_not_reached;
        }
    }
        
    void get_edge_vertices(
        TriangleRegion E, TriangleRegion& q0, TriangleRegion& q1
    ) {
        geo_debug_assert(region_dim(E) == 1);
        switch(E) {
        case T1_RGN_E0: {
            q0 = T1_RGN_P1;
            q1 = T1_RGN_P2;
        } break;
        case T1_RGN_E1: {
            q0 = T1_RGN_P2;
            q1 = T1_RGN_P0;
        } break;
        case T1_RGN_E2: {
            q0 = T1_RGN_P0;
            q1 = T1_RGN_P1;
        } break;
        case T2_RGN_E0: {
            q0 = T2_RGN_P1;
            q1 = T2_RGN_P2;
        } break;
        case T2_RGN_E1: {
            q0 = T2_RGN_P2;
            q1 = T2_RGN_P0;
        } break;
        case T2_RGN_E2: {
            q0 = T2_RGN_P0;
            q1 = T2_RGN_P1;
        } break;
        default:
            geo_assert_not_reached;
        };
    }

    TriangleRegion GEOGRAM_API regions_convex_hull(
        TriangleRegion R1, TriangleRegion R2
    ) {
        geo_debug_assert(is_in_T1(R1) == is_in_T1(R2));
        if(R1 == R2) {
            return R1;
        }

        TriangleRegion R = is_in_T1(R1) ? T1_RGN_T : T2_RGN_T;

        if(region_dim(R1) == 1 && region_dim(R2) == 0) {
            TriangleRegion v1,v2;
            get_edge_vertices(R1,v1,v2);
            if(R2 == v1 || R2 == v2) {
                R = R1;
            }
        } else if(region_dim(R2) == 1 && region_dim(R1) == 0) {
            TriangleRegion v1,v2;
            get_edge_vertices(R2,v1,v2);
            if(R1 == v1 || R1 == v2) {
                R = R2;
            }
        } else if(region_dim(R1) == 0 && region_dim(R2) == 0) {
            for(TriangleRegion E: {
                    T1_RGN_E0, T1_RGN_E1, T1_RGN_E2,
                    T2_RGN_E0, T2_RGN_E1, T2_RGN_E2
                }
            ) {
                TriangleRegion v1,v2;
                get_edge_vertices(E,v1,v2);
                if((R1 == v1 && R2 == v2) || (R1 == v2 && R2 == v1)) {
                    R = E;
                    break;
                }
            }
        }

        return R;
    }
    
}

