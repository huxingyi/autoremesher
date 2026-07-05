#include "kernel.pckh"

#ifdef DIM2

Sign predicate(orient)(
    point(p0), point(p1), point(p2) DIM
) {
  scalar a11 = p1_0 - p0_0 ;
  scalar a12 = p1_1 - p0_1 ;

  scalar a21 = p2_0 - p0_0 ;
  scalar a22 = p2_1 - p0_1 ;

  scalar Delta = det2x2(
            a11,a12,
            a21,a22
         ) ;

  return sign(Delta);
}

#endif
