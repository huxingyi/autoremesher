#include "kernel.pckh"

#ifdef DIM3

Sign predicate(dot)(
    point(p0), point(p1), point(p2) DIM
) {
  scalar a11 = p1_0 - p0_0 ;
  scalar a12 = p1_1 - p0_1 ;
  scalar a13 = p1_2 - p0_2 ;

  scalar a21 = p2_0 - p0_0 ;
  scalar a22 = p2_1 - p0_1 ;
  scalar a23 = p2_2 - p0_2 ;

  scalar Delta = (a11*a21 + a12*a22 + a13*a23);

  return sign(Delta);
}

#endif
