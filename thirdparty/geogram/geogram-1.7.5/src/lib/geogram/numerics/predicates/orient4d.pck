#include "kernel.pckh"

#ifdef DIM3

Sign predicate(orienth)(
    point(p0), point(p1), point(p2), point(p3), point(p4),
    scalar h0, scalar h1, scalar h2, scalar h3, scalar h4
    DIM
) {

  scalar a00 = p1_0 - p0_0 ;
  scalar a01 = p1_1 - p0_1 ;
  scalar a02 = p1_2 - p0_2 ;
  scalar a03 = h1 - h0 ;

  scalar a10 = p2_0 - p0_0 ;
  scalar a11 = p2_1 - p0_1 ;
  scalar a12 = p2_2 - p0_2 ;
  scalar a13 = h2 - h0 ;

  scalar a20 = p3_0 - p0_0 ;
  scalar a21 = p3_1 - p0_1 ;
  scalar a22 = p3_2 - p0_2 ;
  scalar a23 = h3 - h0 ;

  scalar a30 = p4_0 - p0_0 ;
  scalar a31 = p4_1 - p0_1 ;
  scalar a32 = p4_2 - p0_2 ;
  scalar a33 = h4 - h0 ;

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
