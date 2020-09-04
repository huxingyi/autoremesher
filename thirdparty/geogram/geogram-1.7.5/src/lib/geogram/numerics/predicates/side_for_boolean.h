/* Automatically generated code, do not edit */
/* Generated from source file: side_for_boolean.pck */

inline int side_for_boolean_3d_filter( const double* p1, const double* p2, const double* p3, const double* p4, const double* p5, const double* p6, const double* p7, const double* p8, const double* p9, const double* pa, const double* pb, const double* pc) {
    double a00;
    double a01;
    double a02;
    double a03;
    double a10;
    double a11;
    double a12;
    double a13;
    double a20;
    double a21;
    double a22;
    double a23;
    double a30;
    double a31;
    double a32;
    double a33;
    double p2_1_p1_1 = (p2[1] - p1[1]);
    double p3_2_p1_2 = (p3[2] - p1[2]);
    double p2_2_p1_2 = (p2[2] - p1[2]);
    double p3_1_p1_1 = (p3[1] - p1[1]);
    a00 = ((p2_1_p1_1 * p3_2_p1_2) - (p2_2_p1_2 * p3_1_p1_1));
    double p3_0_p1_0 = (p3[0] - p1[0]);
    double p2_0_p1_0 = (p2[0] - p1[0]);
    a01 = ((p2_2_p1_2 * p3_0_p1_0) - (p2_0_p1_0 * p3_2_p1_2));
    a02 = ((p2_0_p1_0 * p3_1_p1_1) - (p2_1_p1_1 * p3_0_p1_0));
    a03 = -(((a00 * p1[0]) + (a01 * p1[1])) + (a02 * p1[2]));
    double p5_1_p4_1 = (p5[1] - p4[1]);
    double p6_2_p4_2 = (p6[2] - p4[2]);
    double p5_2_p4_2 = (p5[2] - p4[2]);
    double p6_1_p4_1 = (p6[1] - p4[1]);
    a10 = ((p5_1_p4_1 * p6_2_p4_2) - (p5_2_p4_2 * p6_1_p4_1));
    double p6_0_p4_0 = (p6[0] - p4[0]);
    double p5_0_p4_0 = (p5[0] - p4[0]);
    a11 = ((p5_2_p4_2 * p6_0_p4_0) - (p5_0_p4_0 * p6_2_p4_2));
    a12 = ((p5_0_p4_0 * p6_1_p4_1) - (p5_1_p4_1 * p6_0_p4_0));
    a13 = -(((a10 * p4[0]) + (a11 * p4[1])) + (a12 * p4[2]));
    double p8_1_p7_1 = (p8[1] - p7[1]);
    double p9_2_p7_2 = (p9[2] - p7[2]);
    double p8_2_p7_2 = (p8[2] - p7[2]);
    double p9_1_p7_1 = (p9[1] - p7[1]);
    a20 = ((p8_1_p7_1 * p9_2_p7_2) - (p8_2_p7_2 * p9_1_p7_1));
    double p9_0_p7_0 = (p9[0] - p7[0]);
    double p8_0_p7_0 = (p8[0] - p7[0]);
    a21 = ((p8_2_p7_2 * p9_0_p7_0) - (p8_0_p7_0 * p9_2_p7_2));
    a22 = ((p8_0_p7_0 * p9_1_p7_1) - (p8_1_p7_1 * p9_0_p7_0));
    a23 = -(((a20 * p7[0]) + (a21 * p7[1])) + (a22 * p7[2]));
    double pb_1_pa_1 = (pb[1] - pa[1]);
    double pc_2_pa_2 = (pc[2] - pa[2]);
    double pb_2_pa_2 = (pb[2] - pa[2]);
    double pc_1_pa_1 = (pc[1] - pa[1]);
    a30 = ((pb_1_pa_1 * pc_2_pa_2) - (pb_2_pa_2 * pc_1_pa_1));
    double pc_0_pa_0 = (pc[0] - pa[0]);
    double pb_0_pa_0 = (pb[0] - pa[0]);
    a31 = ((pb_2_pa_2 * pc_0_pa_0) - (pb_0_pa_0 * pc_2_pa_2));
    a32 = ((pb_0_pa_0 * pc_1_pa_1) - (pb_1_pa_1 * pc_0_pa_0));
    a33 = -(((a30 * pa[0]) + (a31 * pa[1])) + (a32 * pa[2]));
    double m12;
    m12 = ((a10 * a01) - (a00 * a11));
    double m13;
    m13 = ((a20 * a01) - (a00 * a21));
    double m14;
    m14 = ((a30 * a01) - (a00 * a31));
    double m23;
    m23 = ((a20 * a11) - (a10 * a21));
    double m24;
    m24 = ((a30 * a11) - (a10 * a31));
    double m34;
    m34 = ((a30 * a21) - (a20 * a31));
    double m123;
    m123 = (((m23 * a02) - (m13 * a12)) + (m12 * a22));
    double m124;
    m124 = (((m24 * a02) - (m14 * a12)) + (m12 * a32));
    double m134;
    m134 = (((m34 * a02) - (m14 * a22)) + (m13 * a32));
    double m234;
    m234 = (((m34 * a12) - (m24 * a22)) + (m23 * a32));
    double Delta;
    Delta = ((((m234 * a03) - (m134 * a13)) + (m124 * a23)) - (m123 * a33));
    int int_tmp_result;
    double eps;
    double max1;
    double max7 = fabs(p2_1_p1_1);
    if( (max7 < fabs(p2_0_p1_0)) )
    {
        max7 = fabs(p2_0_p1_0);
    } 
    if( (max7 < fabs(p8_1_p7_1)) )
    {
        max7 = fabs(p8_1_p7_1);
    } 
    if( (max7 < fabs(p5_0_p4_0)) )
    {
        max7 = fabs(p5_0_p4_0);
    } 
    if( (max7 < fabs(p5_1_p4_1)) )
    {
        max7 = fabs(p5_1_p4_1);
    } 
    if( (max7 < fabs(p8_0_p7_0)) )
    {
        max7 = fabs(p8_0_p7_0);
    } 
    if( (max7 < fabs(pb_1_pa_1)) )
    {
        max7 = fabs(pb_1_pa_1);
    } 
    if( (max7 < fabs(pb_0_pa_0)) )
    {
        max7 = fabs(pb_0_pa_0);
    } 
    max1 = max7;
    double max2 = fabs(p5_2_p4_2);
    if( (max2 < fabs(p2_1_p1_1)) )
    {
        max2 = fabs(p2_1_p1_1);
    } 
    if( (max2 < fabs(p8_1_p7_1)) )
    {
        max2 = fabs(p8_1_p7_1);
    } 
    if( (max2 < fabs(p5_1_p4_1)) )
    {
        max2 = fabs(p5_1_p4_1);
    } 
    if( (max2 < fabs(p2_2_p1_2)) )
    {
        max2 = fabs(p2_2_p1_2);
    } 
    if( (max2 < fabs(p8_2_p7_2)) )
    {
        max2 = fabs(p8_2_p7_2);
    } 
    if( (max2 < fabs(pb_1_pa_1)) )
    {
        max2 = fabs(pb_1_pa_1);
    } 
    if( (max2 < fabs(pb_2_pa_2)) )
    {
        max2 = fabs(pb_2_pa_2);
    } 
    if( (max1 < max2) )
    {
        max1 = max2;
    } 
    double max3 = fabs(p5_2_p4_2);
    if( (max3 < fabs(p2_0_p1_0)) )
    {
        max3 = fabs(p2_0_p1_0);
    } 
    if( (max3 < fabs(p5_0_p4_0)) )
    {
        max3 = fabs(p5_0_p4_0);
    } 
    if( (max3 < fabs(p2_2_p1_2)) )
    {
        max3 = fabs(p2_2_p1_2);
    } 
    if( (max3 < fabs(p8_2_p7_2)) )
    {
        max3 = fabs(p8_2_p7_2);
    } 
    if( (max3 < fabs(p8_0_p7_0)) )
    {
        max3 = fabs(p8_0_p7_0);
    } 
    if( (max3 < fabs(pb_2_pa_2)) )
    {
        max3 = fabs(pb_2_pa_2);
    } 
    if( (max3 < fabs(pb_0_pa_0)) )
    {
        max3 = fabs(pb_0_pa_0);
    } 
    if( (max1 < max3) )
    {
        max1 = max3;
    } 
    double max4;
    double max6 = fabs(p3_0_p1_0);
    if( (max6 < fabs(p6_2_p4_2)) )
    {
        max6 = fabs(p6_2_p4_2);
    } 
    if( (max6 < fabs(p3_2_p1_2)) )
    {
        max6 = fabs(p3_2_p1_2);
    } 
    if( (max6 < fabs(p9_0_p7_0)) )
    {
        max6 = fabs(p9_0_p7_0);
    } 
    if( (max6 < fabs(p6_0_p4_0)) )
    {
        max6 = fabs(p6_0_p4_0);
    } 
    if( (max6 < fabs(p9_2_p7_2)) )
    {
        max6 = fabs(p9_2_p7_2);
    } 
    if( (max6 < fabs(pc_2_pa_2)) )
    {
        max6 = fabs(pc_2_pa_2);
    } 
    if( (max6 < fabs(pc_0_pa_0)) )
    {
        max6 = fabs(pc_0_pa_0);
    } 
    max4 = max6;
    double max5 = fabs(p3_0_p1_0);
    if( (max5 < fabs(p9_1_p7_1)) )
    {
        max5 = fabs(p9_1_p7_1);
    } 
    if( (max5 < fabs(p3_1_p1_1)) )
    {
        max5 = fabs(p3_1_p1_1);
    } 
    if( (max5 < fabs(p9_0_p7_0)) )
    {
        max5 = fabs(p9_0_p7_0);
    } 
    if( (max5 < fabs(p6_0_p4_0)) )
    {
        max5 = fabs(p6_0_p4_0);
    } 
    if( (max5 < fabs(p6_1_p4_1)) )
    {
        max5 = fabs(p6_1_p4_1);
    } 
    if( (max5 < fabs(pc_1_pa_1)) )
    {
        max5 = fabs(pc_1_pa_1);
    } 
    if( (max5 < fabs(pc_0_pa_0)) )
    {
        max5 = fabs(pc_0_pa_0);
    } 
    if( (max4 < max5) )
    {
        max4 = max5;
    } 
    double max8 = fabs(p9_1_p7_1);
    if( (max8 < fabs(p6_2_p4_2)) )
    {
        max8 = fabs(p6_2_p4_2);
    } 
    if( (max8 < fabs(p3_2_p1_2)) )
    {
        max8 = fabs(p3_2_p1_2);
    } 
    if( (max8 < fabs(p3_1_p1_1)) )
    {
        max8 = fabs(p3_1_p1_1);
    } 
    if( (max8 < fabs(p9_2_p7_2)) )
    {
        max8 = fabs(p9_2_p7_2);
    } 
    if( (max8 < fabs(p6_1_p4_1)) )
    {
        max8 = fabs(p6_1_p4_1);
    } 
    if( (max8 < fabs(pc_2_pa_2)) )
    {
        max8 = fabs(pc_2_pa_2);
    } 
    if( (max8 < fabs(pc_1_pa_1)) )
    {
        max8 = fabs(pc_1_pa_1);
    } 
    if( (max4 < max8) )
    {
        max4 = max8;
    } 
    double lower_bound_1;
    double upper_bound_1;
    lower_bound_1 = max6;
    upper_bound_1 = max6;
    if( (max5 < lower_bound_1) )
    {
        lower_bound_1 = max5;
    } 
    else 
    {
        if( (max5 > upper_bound_1) )
        {
            upper_bound_1 = max5;
        } 
    } 
    if( (max8 < lower_bound_1) )
    {
        lower_bound_1 = max8;
    } 
    else 
    {
        if( (max8 > upper_bound_1) )
        {
            upper_bound_1 = max8;
        } 
    } 
    if( (max7 < lower_bound_1) )
    {
        lower_bound_1 = max7;
    } 
    if( (max1 < lower_bound_1) )
    {
        lower_bound_1 = max1;
    } 
    else 
    {
        if( (max1 > upper_bound_1) )
        {
            upper_bound_1 = max1;
        } 
    } 
    if( (max2 < lower_bound_1) )
    {
        lower_bound_1 = max2;
    } 
    if( (max3 < lower_bound_1) )
    {
        lower_bound_1 = max3;
    } 
    if( (max4 < lower_bound_1) )
    {
        lower_bound_1 = max4;
    } 
    else 
    {
        if( (max4 > upper_bound_1) )
        {
            upper_bound_1 = max4;
        } 
    } 
    if( (lower_bound_1 < 9.48293983771330104494e-38) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 1.29807421463370633067e+33) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (3.40251829549579613644e-12 * (((((((max2 * max8) * max3) * max6) * max7) * max5) * max1) * max4));
        if( (Delta > eps) )
        {
            int_tmp_result = 1;
        } 
        else 
        {
            if( (Delta < -eps) )
            {
                int_tmp_result = -1;
            } 
            else 
            {
                return FPG_UNCERTAIN_VALUE;
            } 
        } 
    } 
    return int_tmp_result;
} 

Sign side_for_boolean_3d_exact(
    const double* p1, const double* p2, const double* p3,
    const double* p4, const double* p5, const double* p6,
    const double* p7, const double* p8, const double* p9,
    const double* pa, const double* pb, const double* pc 
) {
   expansion_nt p1_0(p1[0]);
   expansion_nt p1_1(p1[1]);
   expansion_nt p1_2(p1[2]);   
   expansion_nt p2_0(p2[0]);
   expansion_nt p2_1(p2[1]);
   expansion_nt p2_2(p2[2]);
   expansion_nt p3_0(p3[0]);
   expansion_nt p3_1(p3[1]);
   expansion_nt p3_2(p3[2]);   
   expansion_nt p4_0(p4[0]);
   expansion_nt p4_1(p4[1]);
   expansion_nt p4_2(p4[2]);   
   expansion_nt p5_0(p5[0]);
   expansion_nt p5_1(p5[1]);
   expansion_nt p5_2(p5[2]);   
   expansion_nt p6_0(p6[0]);
   expansion_nt p6_1(p6[1]);
   expansion_nt p6_2(p6[2]);   
   expansion_nt p7_0(p7[0]);
   expansion_nt p7_1(p7[1]);
   expansion_nt p7_2(p7[2]);   
   expansion_nt p8_0(p8[0]);
   expansion_nt p8_1(p8[1]);
   expansion_nt p8_2(p8[2]);   
   expansion_nt p9_0(p9[0]);
   expansion_nt p9_1(p9[1]);
   expansion_nt p9_2(p9[2]);   
   expansion_nt pa_0(pa[0]);
   expansion_nt pa_1(pa[1]);
   expansion_nt pa_2(pa[2]);   
   expansion_nt pb_0(pb[0]);
   expansion_nt pb_1(pb[1]);
   expansion_nt pb_2(pb[2]);   
   expansion_nt pc_0(pc[0]);
   expansion_nt pc_1(pc[1]);
   expansion_nt pc_2(pc[2]);   
   

   expansion_nt a00 = (p2_1 - p1_1) * (p3_2 - p1_2) - (p2_2 - p1_2) * (p3_1 - p1_1);
   expansion_nt a01 = (p2_2 - p1_2) * (p3_0 - p1_0) - (p2_0 - p1_0) * (p3_2 - p1_2);
   expansion_nt a02 = (p2_0 - p1_0) * (p3_1 - p1_1) - (p2_1 - p1_1) * (p3_0 - p1_0);
   expansion_nt a03 = -(a00*p1_0 + a01*p1_1 + a02*p1_2);
   expansion_nt a10 = (p5_1 - p4_1) * (p6_2 - p4_2) - (p5_2 - p4_2) * (p6_1 - p4_1);
   expansion_nt a11 = (p5_2 - p4_2) * (p6_0 - p4_0) - (p5_0 - p4_0) * (p6_2 - p4_2);
   expansion_nt a12 = (p5_0 - p4_0) * (p6_1 - p4_1) - (p5_1 - p4_1) * (p6_0 - p4_0);
   expansion_nt a13 = -(a10*p4_0 + a11*p4_1 + a12*p4_2);
   expansion_nt a20 = (p8_1 - p7_1) * (p9_2 - p7_2) - (p8_2 - p7_2) * (p9_1 - p7_1);
   expansion_nt a21 = (p8_2 - p7_2) * (p9_0 - p7_0) - (p8_0 - p7_0) * (p9_2 - p7_2);
   expansion_nt a22 = (p8_0 - p7_0) * (p9_1 - p7_1) - (p8_1 - p7_1) * (p9_0 - p7_0);
   expansion_nt a23 = -(a20*p7_0 + a21*p7_1 + a22*p7_2);
   expansion_nt a30 = (pb_1 - pa_1) * (pc_2 - pa_2) - (pb_2 - pa_2) * (pc_1 - pa_1);
   expansion_nt a31 = (pb_2 - pa_2) * (pc_0 - pa_0) - (pb_0 - pa_0) * (pc_2 - pa_2);
   expansion_nt a32 = (pb_0 - pa_0) * (pc_1 - pa_1) - (pb_1 - pa_1) * (pc_0 - pa_0);
   expansion_nt a33 = -(a30*pa_0 + a31*pa_1 + a32*pa_2);

   expansion_nt m12 = a10*a01 - a00*a11;
   expansion_nt m13 = a20*a01 - a00*a21;
   expansion_nt m14 = a30*a01 - a00*a31;
   expansion_nt m23 = a20*a11 - a10*a21;
   expansion_nt m24 = a30*a11 - a10*a31;
   expansion_nt m34 = a30*a21 - a20*a31;

   expansion_nt m123 = m23*a02 - m13*a12 + m12*a22;
   expansion_nt m124 = m24*a02 - m14*a12 + m12*a32;
   expansion_nt m134 = m34*a02 - m14*a22 + m13*a32;
   expansion_nt m234 = m34*a12 - m24*a22 + m23*a32;

   expansion_nt Delta = (m234*a03 - m134*a13 + m124*a23 - m123*a33);
   return (Delta).sign();
}
