#include "kernel.pckh"

#if defined(DIM3) || defined(DIMN_EXACT)

#define get_plane(A, B, C, D, P1, P2, P3) \
   A = (P2##_1 - P1##_1) * (P3##_2 - P1##_2) - (P2##_2 - P1##_2) * (P3##_1 - P1##_1); \
   B = (P2##_2 - P1##_2) * (P3##_0 - P1##_0) - (P2##_0 - P1##_0) * (P3##_2 - P1##_2); \
   C = (P2##_0 - P1##_0) * (P3##_1 - P1##_1) - (P2##_1 - P1##_1) * (P3##_0 - P1##_0); \
   D = -(A*P1##_0 + B*P1##_1 + C*P1##_2)

Sign predicate(side_for_boolean)(
    point(p1), point(p2), point(p3),
    point(p4), point(p5), point(p6),
    point(p7), point(p8), point(p9),
    point(pa), point(pb), point(pc) DIM
) {

   scalar a00, a01, a02, a03;
   scalar a10, a11, a12, a13;
   scalar a20, a21, a22, a23;
   scalar a30, a31, a32, a33;

   get_plane(a00, a01, a02, a03, p1, p2, p3);
   get_plane(a10, a11, a12, a13, p4, p5, p6);
   get_plane(a20, a21, a22, a23, p7, p8, p9);
   get_plane(a30, a31, a32, a33, pa, pb, pc);   

   scalar m12 = a10*a01 - a00*a11;
   scalar m13 = a20*a01 - a00*a21;
   scalar m14 = a30*a01 - a00*a31;
   scalar m23 = a20*a11 - a10*a21;
   scalar m24 = a30*a11 - a10*a31;
   scalar m34 = a30*a21 - a20*a31;

   scalar m123 = m23*a02 - m13*a12 + m12*a22;
   scalar m124 = m24*a02 - m14*a12 + m12*a32;
   scalar m134 = m34*a02 - m14*a22 + m13*a32;
   scalar m234 = m34*a12 - m24*a22 + m23*a32;
        
   scalar Delta = (m234*a03 - m134*a13 + m124*a23 - m123*a33);
   return sign(Delta);
}

#endif
