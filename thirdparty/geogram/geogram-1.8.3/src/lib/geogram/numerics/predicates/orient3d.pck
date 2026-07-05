#include "kernel.pckh"

#ifdef DIM3

Sign predicate(orient)(
    point(p0), point(p1), point(p2), point(p3) DIM
) {
  scalar a11 = p1_0 - p0_0 ;
  scalar a12 = p1_1 - p0_1 ;
  scalar a13 = p1_2 - p0_2 ;

  scalar a21 = p2_0 - p0_0 ;
  scalar a22 = p2_1 - p0_1 ;
  scalar a23 = p2_2 - p0_2 ;

  scalar a31 = p3_0 - p0_0 ;
  scalar a32 = p3_1 - p0_1 ;
  scalar a33 = p3_2 - p0_2 ;

  scalar Delta = det3x3(
            a11,a12,a13,
            a21,a22,a23,
            a31,a32,a33
         ) ;

  return sign(Delta);
}

#endif
