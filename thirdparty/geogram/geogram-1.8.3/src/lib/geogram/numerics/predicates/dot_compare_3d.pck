#include "kernel.pckh"

#ifdef DIM3

Sign predicate(dot_compare)(
    point(p0), point(p1), point(p2) DIM
) {
  scalar d1 = p0_0*p1_0 + p0_1*p1_1 + p0_2 * p1_2;
  scalar d2 = p0_0*p2_0 + p0_1*p2_1 + p0_2 * p2_2;  
  return sign(d1-d2);
}

#endif
