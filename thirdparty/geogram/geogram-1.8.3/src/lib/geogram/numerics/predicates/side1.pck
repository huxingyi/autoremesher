#include "kernel.pckh"

Sign predicate(side1)(
   point(p0), point(p1), point(q0)  DIM
) {
  /* note: 1* for ensuring mcc's degree compatibility */
  scalar r = 1*sq_dist(p0,p1) ;
  r -= 2*dot_at(p1,q0,p0) ;
  generic_predicate_result(sign(r)) ;
  begin_sos2(p0,p1)
     sos(p0,POSITIVE) 
     sos(p1,NEGATIVE) 
  end_sos
}
