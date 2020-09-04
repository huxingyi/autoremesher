#include "kernel.pckh"

#ifdef DIM3

Sign predicate(side4h)(
     point(p0), point(p1), point(p2), point(p3), point(p4), 
     scalar h0, scalar h1, scalar h2, scalar h3, scalar h4 
DIM ) {

  scalar a11 = p1_0 - p0_0 ;
  scalar a12 = p1_1 - p0_1 ;
  scalar a13 = p1_2 - p0_2 ;
  scalar a14 = h0 - h1;

  scalar a21 = p2_0 - p0_0 ;
  scalar a22 = p2_1 - p0_1 ;
  scalar a23 = p2_2 - p0_2 ;
  scalar a24 = h0 - h2;

  scalar a31 = p3_0 - p0_0 ;
  scalar a32 = p3_1 - p0_1 ;
  scalar a33 = p3_2 - p0_2 ;
  scalar a34 = h0 - h3;

  scalar a41 = p4_0 - p0_0 ;
  scalar a42 = p4_1 - p0_1 ;
  scalar a43 = p4_2 - p0_2 ;
  scalar a44 = h0 - h4;

  /*
   * Note: we could probably reuse some of the 2x2 co-factors 
   * (but for now I'd rather keep this form that is easier to
   *  read ... and to debug if need be !)
   */

  scalar Delta1 = det3x3(
            a21,a22,a23,
            a31,a32,a33,
            a41,a42,a43
        ) ;
  
  scalar Delta2 = det3x3(
            a11,a12,a13,
            a31,a32,a33,
            a41,a42,a43
        ) ;
 
  scalar Delta3 = det3x3(
            a11,a12,a13,
            a21,a22,a23,
            a41,a42,a43
        ) ;
 
  scalar Delta4 = det3x3(
            a11,a12,a13,
            a21,a22,a23,
            a31,a32,a33
        ) ;
 
  scalar r = Delta1*a14-Delta2*a24+Delta3*a34-Delta4*a44 ;      
  Sign Delta4_sign = sign(Delta4);
  generic_predicate_result(Delta4_sign*sign(r)) ;
  begin_sos5(p0,p1,p2,p3,p4)
     sos(p0, Sign(Delta4_sign*sign((Delta2-Delta1)+(Delta4-Delta3))))
     sos(p1, Sign(Delta4_sign*sign(Delta1)))
     sos(p2, Sign(-Delta4_sign*sign(Delta2)))
     sos(p3, Sign(Delta4_sign*sign(Delta3)))
     sos(p4, NEGATIVE)
  end_sos

}

#endif
