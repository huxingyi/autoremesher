#include "kernel.pckh"

#ifdef DIM3

/*
 * dim=3 is a special case (intrinsic dim = ambient dim),
 * computations are made directly in ambient space, 
 * without using barycentric coordinates in tetrahedron
 * (q0,q1,q2,q3)  (that is ignored).
 */
Sign predicate(side4)(
    point(p0), point(p1), point(p2), point(p3), point(p4) DIM
) {
  scalar a11 = p1_0 - p0_0 ;
  scalar a12 = p1_1 - p0_1 ;
  scalar a13 = p1_2 - p0_2 ;
  scalar a14 = -sq_dist(p1,p0) ;

  scalar a21 = p2_0 - p0_0 ;
  scalar a22 = p2_1 - p0_1 ;
  scalar a23 = p2_2 - p0_2 ;
  scalar a24 = -sq_dist(p2,p0) ;

  scalar a31 = p3_0 - p0_0 ;
  scalar a32 = p3_1 - p0_1 ;
  scalar a33 = p3_2 - p0_2 ;
  scalar a34 = -sq_dist(p3,p0) ;

  scalar a41 = p4_0 - p0_0 ;
  scalar a42 = p4_1 - p0_1 ;
  scalar a43 = p4_2 - p0_2 ;
  scalar a44 = -sq_dist(p4,p0) ;

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
     sos(p2, Sign(Delta4_sign*sign(Delta2)))
     sos(p3, Sign(Delta4_sign*sign(Delta3)))
     sos(p4, NEGATIVE)
  end_sos
}

#else

/*
 * general case (arbitrary dimension), 
 * using barycentric coordinates in tetrahedron
 * (q0,q1,q2,q3).  
 */
Sign predicate(side4)(
    point(p0), point(p1), point(p2), point(p3), point(p4),
    point(q0), point(q1), point(q2), point(q3)  DIM
) {
  /* note: 1* for ensuring mcc's degree compatibility */
  scalar l1 = 1*sq_dist(p1,p0);
  scalar l2 = 1*sq_dist(p2,p0);
  scalar l3 = 1*sq_dist(p3,p0);
  scalar l4 = 1*sq_dist(p4,p0);

  scalar a10 = 2*dot_at(p1,q0,p0);
  scalar a11 = 2*dot_at(p1,q1,p0);
  scalar a12 = 2*dot_at(p1,q2,p0);
  scalar a13 = 2*dot_at(p1,q3,p0);

  scalar a20 = 2*dot_at(p2,q0,p0);
  scalar a21 = 2*dot_at(p2,q1,p0);
  scalar a22 = 2*dot_at(p2,q2,p0);
  scalar a23 = 2*dot_at(p2,q3,p0);

  scalar a30 = 2*dot_at(p3,q0,p0);
  scalar a31 = 2*dot_at(p3,q1,p0);
  scalar a32 = 2*dot_at(p3,q2,p0);
  scalar a33 = 2*dot_at(p3,q3,p0);

  scalar a40 = 2*dot_at(p4,q0,p0);
  scalar a41 = 2*dot_at(p4,q1,p0);
  scalar a42 = 2*dot_at(p4,q2,p0);
  scalar a43 = 2*dot_at(p4,q3,p0);

 /*
  * [ b00 b01 b02 b03 ]           [  1   1   1   1  ]-1
  * [ b10 b11 b12 b13 ]           [ a10 a11 a12 a13 ]
  * [ b20 b21 b22 b23 ] = Delta * [ a20 a21 a22 a23 ]
  * [ b30 b31 b32 b33 ]           [ a30 a31 a32 a33 ]
  */

  scalar b00= det3x3(a11,a12,a13,a21,a22,a23,a31,a32,a33);
  scalar b01=-det_111_2x3(a21,a22,a23,a31,a32,a33);
  scalar b02= det_111_2x3(a11,a12,a13,a31,a32,a33);
  scalar b03=-det_111_2x3(a11,a12,a13,a21,a22,a23);

  scalar b10=-det3x3(a10,a12,a13,a20,a22,a23,a30,a32,a33);
  scalar b11= det_111_2x3(a20,a22,a23,a30,a32,a33);
  scalar b12=-det_111_2x3(a10,a12,a13,a30,a32,a33);
  scalar b13= det_111_2x3(a10,a12,a13,a20,a22,a23);

  scalar b20= det3x3(a10,a11,a13,a20,a21,a23,a30,a31,a33);
  scalar b21=-det_111_2x3(a20,a21,a23,a30,a31,a33);
  scalar b22= det_111_2x3(a10,a11,a13,a30,a31,a33);
  scalar b23=-det_111_2x3(a10,a11,a13,a20,a21,a23);

  scalar b30=-det3x3(a10,a11,a12,a20,a21,a22,a30,a31,a32);
  scalar b31= det_111_2x3(a20,a21,a22,a30,a31,a32);
  scalar b32=-det_111_2x3(a10,a11,a12,a30,a31,a32);
  scalar b33= det_111_2x3(a10,a11,a12,a20,a21,a22);
 
  scalar Delta=b00+b10+b20+b30;

 /* 
  *       [ Lambda0 ]   [ b01 b02 b03 ]   [ l1 ]   [ b00 ]
  *       [ Lambda1 ]   [ b11 b12 b13 ]   [ l2 ]   [ b10 ]
  * Delta [ Lambda2 ] = [ b21 b22 b23 ] * [ l3 ] + [ b20 ]
  *       [ Lambda3 ]   [ b31 b32 b33 ]   [ l4 ]   [ b30 ]
  */

  scalar DeltaLambda0 = b01*l1+b02*l2+b03*l3+b00;  
  scalar DeltaLambda1 = b11*l1+b12*l2+b13*l3+b10;  
  scalar DeltaLambda2 = b21*l1+b22*l2+b23*l3+b20;  
  scalar DeltaLambda3 = b31*l1+b32*l2+b33*l3+b30;  

  scalar r = Delta*l4 - (
            a40*DeltaLambda0+
            a41*DeltaLambda1+
            a42*DeltaLambda2+
            a43*DeltaLambda3  
         );
  
  Sign Delta_sign = sign(Delta);
  generic_predicate_result(Delta_sign*sign(r)) ;
  begin_sos5(p0,p1,p2,p3,p4)
     sos(p0, Sign( Delta_sign*sign(Delta - (
                    (b01+b02+b03)*a30 + 
                    (b11+b12+b13)*a31 + 
                    (b21+b22+b23)*a32 +    
                    (b31+b32+b33)*a33
             )))
     )
     sos(p1, Sign( Delta_sign*sign(a30*b01+a31*b11+a32*b21+a33*b31)))
     sos(p2, Sign( Delta_sign*sign(a30*b02+a31*b12+a32*b22+a33*b32)))
     sos(p3, Sign( Delta_sign*sign(a30*b03+a31*b13+a32*b23+a33*b33)))
     sos(p4, NEGATIVE)
  end_sos
}

#endif
