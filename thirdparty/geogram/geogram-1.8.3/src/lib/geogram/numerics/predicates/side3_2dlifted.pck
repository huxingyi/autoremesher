#include "kernel.pckh"

#ifdef DIM2

Sign predicate(side3_2dlifted)(
     point(p0), point(p1), point(p2), point(p3),
     scalar h0, scalar h1, scalar h2, scalar h3
DIM ) {

  scalar a11 = p1_0 - p0_0 ;
  scalar a12 = p1_1 - p0_1 ;
  scalar a13 = h0 - h1;

  scalar a21 = p2_0 - p0_0 ;
  scalar a22 = p2_1 - p0_1 ;
  scalar a23 = h0 - h2;

  scalar a31 = p3_0 - p0_0 ;
  scalar a32 = p3_1 - p0_1 ;
  scalar a33 = h0 - h3;


  scalar Delta1 = det2x2(
            a21,a22,
            a31,a32
        ) ;
  
  scalar Delta2 = det2x2(
            a11,a12,
            a31,a32
        ) ;
 
  scalar Delta3 = det2x2(
            a11,a12,
            a21,a22
        ) ;
 
  scalar r = Delta1*a13-Delta2*a23+Delta3*a33 ;
  Sign Delta3_sign = sign(Delta3);
  generic_predicate_result(Delta3_sign*sign(r)) ;
  begin_sos4(p0,p1,p2,p3)
     sos(p0, Sign(Delta3_sign*sign((Delta2-Delta1)+Delta3)))
     sos(p1, Sign(Delta3_sign*sign(Delta1)))
     sos(p2, Sign(Delta3_sign*sign(Delta2)))
     sos(p3, NEGATIVE)
  end_sos

}

#endif
