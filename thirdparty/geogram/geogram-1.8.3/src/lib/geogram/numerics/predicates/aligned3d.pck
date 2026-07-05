#include "kernel.pckh"

#ifdef DIM3

Sign predicate(aligned)(
    point(p0), point(p1), point(p2) DIM
) {
  scalar a11 = p1_0 - p0_0 ;
  scalar a12 = p1_1 - p0_1 ;
  scalar a13 = p1_2 - p0_2 ;

  scalar a21 = p2_0 - p0_0 ;
  scalar a22 = p2_1 - p0_1 ;
  scalar a23 = p2_2 - p0_2 ;

  scalar delta1 = det2x2(a12,a22,a13,a23) ;
  scalar delta2 = det2x2(a13,a23,a11,a21) ; 
  scalar delta3 = det2x2(a11,a21,a12,a22) ;

  return (sign(delta1) == 0 && sign(delta2) == 0 && sign(delta3) == 0) ? 0 : 1 ;
}

#endif
