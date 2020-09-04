/**
 * Interfacing Geogram's Shewchuk-type expansions
 * with CGAL.
 * 
 * Developped during the CGAL developper meeting
 * by Andreas Fabri and Bruno Levy
 * Nancy September 2015.
 */

#ifndef __CGAL_EXPANSION_NT__
#define __CGAL_EXPANSION_NT__

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Coercion_traits.h>
#include <CGAL/enum.h>
#include <CGAL/Interval_nt.h>
#include <CGAL/predicates/kernel_ftC3.h>
#include <iostream>
#include <string>
#include <MultiPrecision_psm.h>


/*********************************************/
/* Some optimized helpers functions for CGAL */

namespace GEO {

    /**
     * \brief Low-level implementation of the in_sphere predicate.
     */
    inline Sign expansion_nt_side_of_oriented_sphere(
        const GEO::expansion_nt& px,
        const GEO::expansion_nt& py,
        const GEO::expansion_nt& pz,
        const GEO::expansion_nt& qx,
        const GEO::expansion_nt& qy,
        const GEO::expansion_nt& qz,
        const GEO::expansion_nt& rx,
        const GEO::expansion_nt& ry,
        const GEO::expansion_nt& rz,
        const GEO::expansion_nt& sx,
        const GEO::expansion_nt& sy,
        const GEO::expansion_nt& sz,
        const GEO::expansion_nt& tx,
        const GEO::expansion_nt& ty,
        const GEO::expansion_nt& tz       
    ) {
        const expansion& ptx = expansion_diff(px.rep(),tx.rep());
        const expansion& pty = expansion_diff(py.rep(),ty.rep());
        const expansion& ptz = expansion_diff(pz.rep(),tz.rep());        
        const expansion& pt2 = expansion_length2(ptx,pty,ptz);
        
        const expansion& qtx = expansion_diff(qx.rep(),tx.rep());
        const expansion& qty = expansion_diff(qy.rep(),ty.rep());
        const expansion& qtz = expansion_diff(qz.rep(),tz.rep());        
        const expansion& qt2 = expansion_length2(qtx,qty,qtz);
        
        const expansion& rtx = expansion_diff(rx.rep(),tx.rep());
        const expansion& rty = expansion_diff(ry.rep(),ty.rep());
        const expansion& rtz = expansion_diff(rz.rep(),tz.rep());        
        const expansion& rt2 = expansion_length2(rtx,rty,rtz);
        
        const expansion& stx = expansion_diff(sx.rep(),tx.rep());
        const expansion& sty = expansion_diff(sy.rep(),ty.rep());
        const expansion& stz = expansion_diff(sz.rep(),tz.rep());        
        const expansion& st2 = expansion_length2(stx,sty,stz);
        
        return sign_of_expansion_determinant(
            ptx,pty,ptz,pt2,
            rtx,rty,rtz,rt2,
            qtx,qty,qtz,qt2,
            stx,sty,stz,st2
        );
        // Note that the determinant above is det(P,R,Q,S)
        // (Q and R are swapped)!
    }

    /**
     * \brief Low-level implementation of the in_sphere predicate.
     */
    inline Sign expansion_nt_orientation(
        const expansion_nt& px,
        const expansion_nt& py,
        const expansion_nt& pz,
        const expansion_nt& qx,
        const expansion_nt& qy,
        const expansion_nt& qz,
        const expansion_nt& rx,
        const expansion_nt& ry,
        const expansion_nt& rz,
        const expansion_nt& sx,
        const expansion_nt& sy,
        const expansion_nt& sz
    ) {
        const expansion& qpx = expansion_diff(qx.rep(),px.rep());
        const expansion& qpy = expansion_diff(qy.rep(),py.rep());
        const expansion& qpz = expansion_diff(qz.rep(),pz.rep());
        
        const expansion& rpx = expansion_diff(rx.rep(),px.rep());
        const expansion& rpy = expansion_diff(ry.rep(),py.rep());
        const expansion& rpz = expansion_diff(rz.rep(),pz.rep());        

        const expansion& spx = expansion_diff(sx.rep(),px.rep());
        const expansion& spy = expansion_diff(sy.rep(),py.rep());
        const expansion& spz = expansion_diff(sz.rep(),pz.rep());        

        return sign_of_expansion_determinant(
            qpx,rpx,spx,
            qpy,rpy,spy,
            qpz,rpz,spz
        );
    }
}


/***************************************************/
/* CGAL number type and algebraic traits interface */

namespace CGAL {

    /**
     * \brief Algebraic_structure_traits specialization for expansion_nt
     * \note the following functions are not available:
     *   - Gcd (required by UniqueFactorizationDomain)
     *   - Integral_division (required by IntegralDomain)
     *   - Sqrt (required by FieldWithSqrt)
     *   - Is_square (required by IntegralDomainWithoutDivision)
     */
    template <> struct Algebraic_structure_traits< GEO::expansion_nt >
        : public Algebraic_structure_traits_base<
                GEO::expansion_nt, Integral_domain_without_division_tag
          > {

        /**
         * \brief Indicates that all operations with expansion_nt are exact.
         */
        typedef Tag_true  Is_exact;

        /**
         * \brief Indicates that the performance of the operations 
         *   with expansion_nt are not sensitive to the condition
         *   number of the involved matrices.
         */
        typedef Tag_false Is_numerical_sensitive;

        /**
         * \brief Is_zero function, 
         *  as required by Algebraic_structure_traits concept
         */
        struct Is_zero : public std::unary_function< Type, bool > {
            /**
             * \brief Tests whether an expansion_nt is zero
             * \param x the expansion_nt to be tested
             * \retval true if \p x is equal to zero
             * \retval false otherwise
             */
            bool operator()( const Type& x) const {
                return GEO::expansion_nt_is_zero(x);
            }
        };

        /**
         * \brief Is_one function, 
         *  as required by Algebraic_structure_traits concept
         */
        struct Is_one : public std::unary_function< Type, bool > {
            /**
             * \brief Tests whether an expansion_nt is equal to one
             * \param x the expansion_nt to be tested
             * \retval true if \p x is equal to one
             * \retval false otherwise
             */
            bool operator()( const Type& x ) const {
                return GEO::expansion_nt_is_one(x);
            }
        };

        /**
         * \brief Square function, 
         *  as required by Algebraic_structure_traits concept
         */
        struct Square : public std::unary_function< Type, Type > {
            /**
             * \brief Computes the square of an expansion_nt
             * \param x the expansion_nt to be squared
             * \return An expansion_nt equal to \p x * \p x
             */
            Type operator()( const Type& x ) const {
                return GEO::expansion_nt_square(x);
            }
        };
    };

    /**
     * \brief Real_embeddable_traits specialization for expansion_nt
     * \note To_inverval() is not implemented. We can probably implement
     *  it by taking the leading component of the expansion and generating
     *  the other end of the inverval by changing its least significant bit.
     */
    template <> struct Real_embeddable_traits< GEO::expansion_nt >
        : public INTERN_RET::Real_embeddable_traits_base<
        GEO::expansion_nt,CGAL::Tag_true
    > {

        /**
         * \brief Sgn function, 
         *  as required by Real_embeddable_traits
         */
        struct Sgn : public std::unary_function< Type, ::CGAL::Sign > {
            /**
             * \brief Computes the sign of an expansion_nt
             * \param x the expansion_nt to be tested
             * \retval CGAL::POSITIVE if \p x is positive
             * \retval CGAL::ZERO if \p x is zero
             * \retval CGAL::NEGATIVE if \p x is negative
             */
            ::CGAL::Sign operator()( const Type& x ) const {
                int s = int(x.sign());
                return CGAL::Sign(s);
            }
        };

        /**
         * \brief To_double function, 
         *  as required by Real_embeddable_traits
         */
        struct To_double : public std::unary_function< Type, double > {
            /**
             * \brief Converts an expansion_nt into a double
             * \param x the expansion_nt to be converted
             * \return a double that approximates the value of \p x
             */
            double operator()( const Type& x ) const {
                return x.estimate();
            }
        };

        /**
         * \brief Compare function, 
         *  as required by Real_embeddable_traits
         */
        struct Compare : public std::binary_function<
            Type,Type,Comparison_result
        > {
            /**
             * \brief Compares two expansion_nt
             * \param [in] x,y the expansion_nt to be compared
             * \retval CGAL::LARGER if \p x is greater than \p y
             * \retval CGAL::EQUAL if \p x = \p y
             * \retval CGAL::SMALLER if \p x is smaller than \p y
             */
            Comparison_result operator() ( const Type& x, const Type& y) const {
                int s = int(GEO::expansion_nt_compare(x,y));
                return CGAL::sign(s);
            }
        };
    };

    CGAL_DEFINE_COERCION_TRAITS_FOR_SELF(GEO::expansion_nt)
    CGAL_DEFINE_COERCION_TRAITS_FROM_TO(short  ,GEO::expansion_nt)
    CGAL_DEFINE_COERCION_TRAITS_FROM_TO(int    ,GEO::expansion_nt)
    CGAL_DEFINE_COERCION_TRAITS_FROM_TO(long   ,GEO::expansion_nt)
    CGAL_DEFINE_COERCION_TRAITS_FROM_TO(float  ,GEO::expansion_nt)
    CGAL_DEFINE_COERCION_TRAITS_FROM_TO(double ,GEO::expansion_nt)

    /**
     * \brief Optimized specialization of CGAL::determinant()
     *  for expansion_nt.
     * \details expansion_nt_determinant() uses the low-level 
     *  expansion API, and is more efficient than
     *  instancing CGAL::determinant with expansion_nt.
     */
    template<> inline GEO::expansion_nt determinant(
        const GEO::expansion_nt& a11,
        const GEO::expansion_nt& a12,
        const GEO::expansion_nt& a21,
        const GEO::expansion_nt& a22
    ) {
        return GEO::expansion_nt_determinant(
            a11,a12,
            a21,a22
        );
    }
    
    /**
     * \brief Optimized specialization of CGAL::determinant()
     *  for expansion_nt.
     * \details expansion_nt_determinant() uses the low-level 
     *  expansion API, and is more efficient than
     *  instancing CGAL::determinant with expansion_nt.
     */
    template<> inline GEO::expansion_nt determinant(
        const GEO::expansion_nt& a11,
        const GEO::expansion_nt& a12,
        const GEO::expansion_nt& a13,
        const GEO::expansion_nt& a21,
        const GEO::expansion_nt& a22,
        const GEO::expansion_nt& a23,
        const GEO::expansion_nt& a31,
        const GEO::expansion_nt& a32,
        const GEO::expansion_nt& a33
    ) {
        return GEO::expansion_nt_determinant(
            a11,a12,a13,
            a21,a22,a23,
            a31,a32,a33
        );
    }
    
    /**
     * \brief Optimized specialization of CGAL::determinant()
     *  for expansion_nt.
     * \details expansion_nt_determinant() uses the low-level 
     *  expansion API, and is more efficient than
     *  instancing CGAL::determinant with expansion_nt.
     */
    template<> inline GEO::expansion_nt determinant(
        const GEO::expansion_nt& a11,
        const GEO::expansion_nt& a12,
        const GEO::expansion_nt& a13,
        const GEO::expansion_nt& a14,
        const GEO::expansion_nt& a21,
        const GEO::expansion_nt& a22,
        const GEO::expansion_nt& a23,
        const GEO::expansion_nt& a24,
        const GEO::expansion_nt& a31,
        const GEO::expansion_nt& a32,
        const GEO::expansion_nt& a33,
        const GEO::expansion_nt& a34,
        const GEO::expansion_nt& a41,
        const GEO::expansion_nt& a42,
        const GEO::expansion_nt& a43,
        const GEO::expansion_nt& a44
    ) {
        return GEO::expansion_nt_determinant(
            a11,a12,a13,a14,
            a21,a22,a23,a24,
            a31,a32,a33,a34,
            a41,a42,a43,a44            
        );
    }


    
   /*
    * \brief Specialization of "in_sphere()" 
    *  (called "side_of_oriented_sphere") predicate for expansion_nt.
    * \details it makes CGAL faster than if using the 
    *  generic implementation.
    */
    template<> inline CGAL::Same_uncertainty_nt<
        CGAL::Oriented_side, GEO::expansion_nt
    >::type side_of_oriented_sphereC3(
        const GEO::expansion_nt& px,
        const GEO::expansion_nt& py,
        const GEO::expansion_nt& pz,
        const GEO::expansion_nt& qx,
        const GEO::expansion_nt& qy,
        const GEO::expansion_nt& qz,
        const GEO::expansion_nt& rx,
        const GEO::expansion_nt& ry,
        const GEO::expansion_nt& rz,
        const GEO::expansion_nt& sx,
        const GEO::expansion_nt& sy,
        const GEO::expansion_nt& sz,
        const GEO::expansion_nt& tx,
        const GEO::expansion_nt& ty,
        const GEO::expansion_nt& tz       
    ) {
        int sgn = GEO::expansion_nt_side_of_oriented_sphere(
            px,py,pz, qx,qy,qz, rx,ry,rz, sx,sy,sz, tx,ty,tz
        );
        return CGAL::sign(sgn);
    }

   /*
    * \brief Specialization of orientation predicate for expansion_nt.
    * \details it makes CGAL faster than if using 
    *  the generic implementation.
    */
    template<> inline CGAL::Same_uncertainty_nt<
        CGAL::Oriented_side, GEO::expansion_nt
    >::type
    orientationC3(
        const GEO::expansion_nt& px,
        const GEO::expansion_nt& py,
        const GEO::expansion_nt& pz,
        const GEO::expansion_nt& qx,
        const GEO::expansion_nt& qy,
        const GEO::expansion_nt& qz,
        const GEO::expansion_nt& rx,
        const GEO::expansion_nt& ry,
        const GEO::expansion_nt& rz,
        const GEO::expansion_nt& sx,
        const GEO::expansion_nt& sy,
        const GEO::expansion_nt& sz
    ) {
        int sgn = GEO::expansion_nt_orientation(
            px,py,pz, qx,qy,qz, rx,ry,rz, sx,sy,sz
        );
        return CGAL::sign(sgn);
    }
}


#endif
