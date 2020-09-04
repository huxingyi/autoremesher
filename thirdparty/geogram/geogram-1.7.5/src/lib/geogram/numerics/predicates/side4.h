/* Automatically generated code, do not edit */
/* Generated from source file: side4.pck */

inline int side4_3d_filter( const double* p0, const double* p1, const double* p2, const double* p3, const double* p4) {
    double a11;
    a11 = (p1[0] - p0[0]);
    double a12;
    a12 = (p1[1] - p0[1]);
    double a13;
    a13 = (p1[2] - p0[2]);
    double p1_0_p0_0 = (p1[0] - p0[0]);
    double p1_1_p0_1 = (p1[1] - p0[1]);
    double p1_2_p0_2 = (p1[2] - p0[2]);
    double a14;
    a14 = -(((p1_0_p0_0 * p1_0_p0_0) + (p1_1_p0_1 * p1_1_p0_1)) + (p1_2_p0_2 * p1_2_p0_2));
    double a21;
    a21 = (p2[0] - p0[0]);
    double a22;
    a22 = (p2[1] - p0[1]);
    double a23;
    a23 = (p2[2] - p0[2]);
    double p2_0_p0_0 = (p2[0] - p0[0]);
    double p2_1_p0_1 = (p2[1] - p0[1]);
    double p2_2_p0_2 = (p2[2] - p0[2]);
    double a24;
    a24 = -(((p2_0_p0_0 * p2_0_p0_0) + (p2_1_p0_1 * p2_1_p0_1)) + (p2_2_p0_2 * p2_2_p0_2));
    double a31;
    a31 = (p3[0] - p0[0]);
    double a32;
    a32 = (p3[1] - p0[1]);
    double a33;
    a33 = (p3[2] - p0[2]);
    double p3_0_p0_0 = (p3[0] - p0[0]);
    double p3_1_p0_1 = (p3[1] - p0[1]);
    double p3_2_p0_2 = (p3[2] - p0[2]);
    double a34;
    a34 = -(((p3_0_p0_0 * p3_0_p0_0) + (p3_1_p0_1 * p3_1_p0_1)) + (p3_2_p0_2 * p3_2_p0_2));
    double a41;
    a41 = (p4[0] - p0[0]);
    double a42;
    a42 = (p4[1] - p0[1]);
    double a43;
    a43 = (p4[2] - p0[2]);
    double p4_0_p0_0 = (p4[0] - p0[0]);
    double p4_1_p0_1 = (p4[1] - p0[1]);
    double p4_2_p0_2 = (p4[2] - p0[2]);
    double a44;
    a44 = -(((p4_0_p0_0 * p4_0_p0_0) + (p4_1_p0_1 * p4_1_p0_1)) + (p4_2_p0_2 * p4_2_p0_2));
    double Delta1;
    Delta1 = (((a21 * ((a32 * a43) - (a33 * a42))) - (a31 * ((a22 * a43) - (a23 * a42)))) + (a41 * ((a22 * a33) - (a23 * a32))));
    double Delta2;
    Delta2 = (((a11 * ((a32 * a43) - (a33 * a42))) - (a31 * ((a12 * a43) - (a13 * a42)))) + (a41 * ((a12 * a33) - (a13 * a32))));
    double Delta3;
    Delta3 = (((a11 * ((a22 * a43) - (a23 * a42))) - (a21 * ((a12 * a43) - (a13 * a42)))) + (a41 * ((a12 * a23) - (a13 * a22))));
    double Delta4;
    Delta4 = (((a11 * ((a22 * a33) - (a23 * a32))) - (a21 * ((a12 * a33) - (a13 * a32)))) + (a31 * ((a12 * a23) - (a13 * a22))));
    double r;
    r = ((((Delta1 * a14) - (Delta2 * a24)) + (Delta3 * a34)) - (Delta4 * a44));
    double eps;
    double max1 = fabs(a11);
    if( (max1 < fabs(a21)) )
    {
        max1 = fabs(a21);
    } 
    if( (max1 < fabs(a31)) )
    {
        max1 = fabs(a31);
    } 
    double max2 = fabs(a12);
    if( (max2 < fabs(a13)) )
    {
        max2 = fabs(a13);
    } 
    if( (max2 < fabs(a22)) )
    {
        max2 = fabs(a22);
    } 
    if( (max2 < fabs(a23)) )
    {
        max2 = fabs(a23);
    } 
    double max3 = fabs(a22);
    if( (max3 < fabs(a23)) )
    {
        max3 = fabs(a23);
    } 
    if( (max3 < fabs(a32)) )
    {
        max3 = fabs(a32);
    } 
    if( (max3 < fabs(a33)) )
    {
        max3 = fabs(a33);
    } 
    double lower_bound_1;
    double upper_bound_1;
    int Delta4_sign;
    int int_tmp_result;
    lower_bound_1 = max1;
    upper_bound_1 = max1;
    if( (max3 < lower_bound_1) )
    {
        lower_bound_1 = max3;
    } 
    else 
    {
        if( (max3 > upper_bound_1) )
        {
            upper_bound_1 = max3;
        } 
    } 
    if( (max2 < lower_bound_1) )
    {
        lower_bound_1 = max2;
    } 
    else 
    {
        if( (max2 > upper_bound_1) )
        {
            upper_bound_1 = max2;
        } 
    } 
    if( (lower_bound_1 < 1.63288018496748314939e-98) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 3.21387608851797948065e+60) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (5.11071278299732992696e-15 * ((max2 * max3) * max1));
        if( (Delta4 > eps) )
        {
            int_tmp_result = 1;
        } 
        else 
        {
            if( (Delta4 < -eps) )
            {
                int_tmp_result = -1;
            } 
            else 
            {
                return FPG_UNCERTAIN_VALUE;
            } 
        } 
    } 
    Delta4_sign = int_tmp_result;
    int int_tmp_result_FFWKCAA;
    double max4 = max1;
    if( (max4 < fabs(a41)) )
    {
        max4 = fabs(a41);
    } 
    double max5 = max3;
    if( (max5 < max2) )
    {
        max5 = max2;
    } 
    double max6 = max3;
    if( (max6 < fabs(a42)) )
    {
        max6 = fabs(a42);
    } 
    if( (max6 < fabs(a43)) )
    {
        max6 = fabs(a43);
    } 
    double max7 = fabs(p1_0_p0_0);
    if( (max7 < fabs(p1_1_p0_1)) )
    {
        max7 = fabs(p1_1_p0_1);
    } 
    if( (max7 < fabs(p1_2_p0_2)) )
    {
        max7 = fabs(p1_2_p0_2);
    } 
    if( (max7 < fabs(p2_0_p0_0)) )
    {
        max7 = fabs(p2_0_p0_0);
    } 
    if( (max7 < fabs(p2_2_p0_2)) )
    {
        max7 = fabs(p2_2_p0_2);
    } 
    if( (max7 < fabs(p2_1_p0_1)) )
    {
        max7 = fabs(p2_1_p0_1);
    } 
    if( (max7 < fabs(p3_0_p0_0)) )
    {
        max7 = fabs(p3_0_p0_0);
    } 
    if( (max7 < fabs(p3_1_p0_1)) )
    {
        max7 = fabs(p3_1_p0_1);
    } 
    if( (max7 < fabs(p3_2_p0_2)) )
    {
        max7 = fabs(p3_2_p0_2);
    } 
    if( (max7 < fabs(p4_0_p0_0)) )
    {
        max7 = fabs(p4_0_p0_0);
    } 
    if( (max7 < fabs(p4_1_p0_1)) )
    {
        max7 = fabs(p4_1_p0_1);
    } 
    if( (max7 < fabs(p4_2_p0_2)) )
    {
        max7 = fabs(p4_2_p0_2);
    } 
    lower_bound_1 = max7;
    upper_bound_1 = max7;
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
    if( (max6 < lower_bound_1) )
    {
        lower_bound_1 = max6;
    } 
    else 
    {
        if( (max6 > upper_bound_1) )
        {
            upper_bound_1 = max6;
        } 
    } 
    if( (lower_bound_1 < 1.12285198342304832993e-59) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 3.21387608851797948065e+60) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (1.24661365310273025710e-13 * ((((max5 * max6) * max4) * max7) * max7));
        if( (r > eps) )
        {
            int_tmp_result_FFWKCAA = 1;
        } 
        else 
        {
            if( (r < -eps) )
            {
                int_tmp_result_FFWKCAA = -1;
            } 
            else 
            {
                return FPG_UNCERTAIN_VALUE;
            } 
        } 
    } 
    return (Delta4_sign * int_tmp_result_FFWKCAA);
} 


inline int side4_4d_filter( const double* p0, const double* p1, const double* p2, const double* p3, const double* p4, const double* q0, const double* q1, const double* q2, const double* q3) {
    double p1_0_p0_0 = (p1[0] - p0[0]);
    double p1_1_p0_1 = (p1[1] - p0[1]);
    double p1_2_p0_2 = (p1[2] - p0[2]);
    double p1_3_p0_3 = (p1[3] - p0[3]);
    double l1;
    l1 = (1 * ((((p1_0_p0_0 * p1_0_p0_0) + (p1_1_p0_1 * p1_1_p0_1)) + (p1_2_p0_2 * p1_2_p0_2)) + (p1_3_p0_3 * p1_3_p0_3)));
    double p2_0_p0_0 = (p2[0] - p0[0]);
    double p2_1_p0_1 = (p2[1] - p0[1]);
    double p2_2_p0_2 = (p2[2] - p0[2]);
    double p2_3_p0_3 = (p2[3] - p0[3]);
    double l2;
    l2 = (1 * ((((p2_0_p0_0 * p2_0_p0_0) + (p2_1_p0_1 * p2_1_p0_1)) + (p2_2_p0_2 * p2_2_p0_2)) + (p2_3_p0_3 * p2_3_p0_3)));
    double p3_0_p0_0 = (p3[0] - p0[0]);
    double p3_1_p0_1 = (p3[1] - p0[1]);
    double p3_2_p0_2 = (p3[2] - p0[2]);
    double p3_3_p0_3 = (p3[3] - p0[3]);
    double l3;
    l3 = (1 * ((((p3_0_p0_0 * p3_0_p0_0) + (p3_1_p0_1 * p3_1_p0_1)) + (p3_2_p0_2 * p3_2_p0_2)) + (p3_3_p0_3 * p3_3_p0_3)));
    double p4_0_p0_0 = (p4[0] - p0[0]);
    double p4_1_p0_1 = (p4[1] - p0[1]);
    double p4_2_p0_2 = (p4[2] - p0[2]);
    double p4_3_p0_3 = (p4[3] - p0[3]);
    double l4;
    l4 = (1 * ((((p4_0_p0_0 * p4_0_p0_0) + (p4_1_p0_1 * p4_1_p0_1)) + (p4_2_p0_2 * p4_2_p0_2)) + (p4_3_p0_3 * p4_3_p0_3)));
    double q0_0_p0_0 = (q0[0] - p0[0]);
    double q0_1_p0_1 = (q0[1] - p0[1]);
    double q0_2_p0_2 = (q0[2] - p0[2]);
    double q0_3_p0_3 = (q0[3] - p0[3]);
    double a10;
    a10 = (2 * ((((p1_0_p0_0 * q0_0_p0_0) + (p1_1_p0_1 * q0_1_p0_1)) + (p1_2_p0_2 * q0_2_p0_2)) + (p1_3_p0_3 * q0_3_p0_3)));
    double q1_0_p0_0 = (q1[0] - p0[0]);
    double q1_1_p0_1 = (q1[1] - p0[1]);
    double q1_2_p0_2 = (q1[2] - p0[2]);
    double q1_3_p0_3 = (q1[3] - p0[3]);
    double a11;
    a11 = (2 * ((((p1_0_p0_0 * q1_0_p0_0) + (p1_1_p0_1 * q1_1_p0_1)) + (p1_2_p0_2 * q1_2_p0_2)) + (p1_3_p0_3 * q1_3_p0_3)));
    double q2_0_p0_0 = (q2[0] - p0[0]);
    double q2_1_p0_1 = (q2[1] - p0[1]);
    double q2_2_p0_2 = (q2[2] - p0[2]);
    double q2_3_p0_3 = (q2[3] - p0[3]);
    double a12;
    a12 = (2 * ((((p1_0_p0_0 * q2_0_p0_0) + (p1_1_p0_1 * q2_1_p0_1)) + (p1_2_p0_2 * q2_2_p0_2)) + (p1_3_p0_3 * q2_3_p0_3)));
    double q3_0_p0_0 = (q3[0] - p0[0]);
    double q3_1_p0_1 = (q3[1] - p0[1]);
    double q3_2_p0_2 = (q3[2] - p0[2]);
    double q3_3_p0_3 = (q3[3] - p0[3]);
    double a13;
    a13 = (2 * ((((p1_0_p0_0 * q3_0_p0_0) + (p1_1_p0_1 * q3_1_p0_1)) + (p1_2_p0_2 * q3_2_p0_2)) + (p1_3_p0_3 * q3_3_p0_3)));
    double a20;
    a20 = (2 * ((((p2_0_p0_0 * q0_0_p0_0) + (p2_1_p0_1 * q0_1_p0_1)) + (p2_2_p0_2 * q0_2_p0_2)) + (p2_3_p0_3 * q0_3_p0_3)));
    double a21;
    a21 = (2 * ((((p2_0_p0_0 * q1_0_p0_0) + (p2_1_p0_1 * q1_1_p0_1)) + (p2_2_p0_2 * q1_2_p0_2)) + (p2_3_p0_3 * q1_3_p0_3)));
    double a22;
    a22 = (2 * ((((p2_0_p0_0 * q2_0_p0_0) + (p2_1_p0_1 * q2_1_p0_1)) + (p2_2_p0_2 * q2_2_p0_2)) + (p2_3_p0_3 * q2_3_p0_3)));
    double a23;
    a23 = (2 * ((((p2_0_p0_0 * q3_0_p0_0) + (p2_1_p0_1 * q3_1_p0_1)) + (p2_2_p0_2 * q3_2_p0_2)) + (p2_3_p0_3 * q3_3_p0_3)));
    double a30;
    a30 = (2 * ((((p3_0_p0_0 * q0_0_p0_0) + (p3_1_p0_1 * q0_1_p0_1)) + (p3_2_p0_2 * q0_2_p0_2)) + (p3_3_p0_3 * q0_3_p0_3)));
    double a31;
    a31 = (2 * ((((p3_0_p0_0 * q1_0_p0_0) + (p3_1_p0_1 * q1_1_p0_1)) + (p3_2_p0_2 * q1_2_p0_2)) + (p3_3_p0_3 * q1_3_p0_3)));
    double a32;
    a32 = (2 * ((((p3_0_p0_0 * q2_0_p0_0) + (p3_1_p0_1 * q2_1_p0_1)) + (p3_2_p0_2 * q2_2_p0_2)) + (p3_3_p0_3 * q2_3_p0_3)));
    double a33;
    a33 = (2 * ((((p3_0_p0_0 * q3_0_p0_0) + (p3_1_p0_1 * q3_1_p0_1)) + (p3_2_p0_2 * q3_2_p0_2)) + (p3_3_p0_3 * q3_3_p0_3)));
    double a40;
    a40 = (2 * ((((p4_0_p0_0 * q0_0_p0_0) + (p4_1_p0_1 * q0_1_p0_1)) + (p4_2_p0_2 * q0_2_p0_2)) + (p4_3_p0_3 * q0_3_p0_3)));
    double a41;
    a41 = (2 * ((((p4_0_p0_0 * q1_0_p0_0) + (p4_1_p0_1 * q1_1_p0_1)) + (p4_2_p0_2 * q1_2_p0_2)) + (p4_3_p0_3 * q1_3_p0_3)));
    double a42;
    a42 = (2 * ((((p4_0_p0_0 * q2_0_p0_0) + (p4_1_p0_1 * q2_1_p0_1)) + (p4_2_p0_2 * q2_2_p0_2)) + (p4_3_p0_3 * q2_3_p0_3)));
    double a43;
    a43 = (2 * ((((p4_0_p0_0 * q3_0_p0_0) + (p4_1_p0_1 * q3_1_p0_1)) + (p4_2_p0_2 * q3_2_p0_2)) + (p4_3_p0_3 * q3_3_p0_3)));
    double b00;
    b00 = (((a11 * ((a22 * a33) - (a23 * a32))) - (a21 * ((a12 * a33) - (a13 * a32)))) + (a31 * ((a12 * a23) - (a13 * a22))));
    double b01;
    b01 = -((((a22 * a33) - (a23 * a32)) + ((a23 * a31) - (a21 * a33))) + ((a21 * a32) - (a22 * a31)));
    double b02;
    b02 = ((((a12 * a33) - (a13 * a32)) + ((a13 * a31) - (a11 * a33))) + ((a11 * a32) - (a12 * a31)));
    double b03;
    b03 = -((((a12 * a23) - (a13 * a22)) + ((a13 * a21) - (a11 * a23))) + ((a11 * a22) - (a12 * a21)));
    double b10;
    b10 = -(((a10 * ((a22 * a33) - (a23 * a32))) - (a20 * ((a12 * a33) - (a13 * a32)))) + (a30 * ((a12 * a23) - (a13 * a22))));
    double b11;
    b11 = ((((a22 * a33) - (a23 * a32)) + ((a23 * a30) - (a20 * a33))) + ((a20 * a32) - (a22 * a30)));
    double b12;
    b12 = -((((a12 * a33) - (a13 * a32)) + ((a13 * a30) - (a10 * a33))) + ((a10 * a32) - (a12 * a30)));
    double b13;
    b13 = ((((a12 * a23) - (a13 * a22)) + ((a13 * a20) - (a10 * a23))) + ((a10 * a22) - (a12 * a20)));
    double b20;
    b20 = (((a10 * ((a21 * a33) - (a23 * a31))) - (a20 * ((a11 * a33) - (a13 * a31)))) + (a30 * ((a11 * a23) - (a13 * a21))));
    double b21;
    b21 = -((((a21 * a33) - (a23 * a31)) + ((a23 * a30) - (a20 * a33))) + ((a20 * a31) - (a21 * a30)));
    double b22;
    b22 = ((((a11 * a33) - (a13 * a31)) + ((a13 * a30) - (a10 * a33))) + ((a10 * a31) - (a11 * a30)));
    double b23;
    b23 = -((((a11 * a23) - (a13 * a21)) + ((a13 * a20) - (a10 * a23))) + ((a10 * a21) - (a11 * a20)));
    double b30;
    b30 = -(((a10 * ((a21 * a32) - (a22 * a31))) - (a20 * ((a11 * a32) - (a12 * a31)))) + (a30 * ((a11 * a22) - (a12 * a21))));
    double b31;
    b31 = ((((a21 * a32) - (a22 * a31)) + ((a22 * a30) - (a20 * a32))) + ((a20 * a31) - (a21 * a30)));
    double b32;
    b32 = -((((a11 * a32) - (a12 * a31)) + ((a12 * a30) - (a10 * a32))) + ((a10 * a31) - (a11 * a30)));
    double b33;
    b33 = ((((a11 * a22) - (a12 * a21)) + ((a12 * a20) - (a10 * a22))) + ((a10 * a21) - (a11 * a20)));
    double Delta;
    Delta = (((b00 + b10) + b20) + b30);
    double DeltaLambda0;
    DeltaLambda0 = ((((b01 * l1) + (b02 * l2)) + (b03 * l3)) + b00);
    double DeltaLambda1;
    DeltaLambda1 = ((((b11 * l1) + (b12 * l2)) + (b13 * l3)) + b10);
    double DeltaLambda2;
    DeltaLambda2 = ((((b21 * l1) + (b22 * l2)) + (b23 * l3)) + b20);
    double DeltaLambda3;
    DeltaLambda3 = ((((b31 * l1) + (b32 * l2)) + (b33 * l3)) + b30);
    double r;
    r = ((Delta * l4) - ((((a40 * DeltaLambda0) + (a41 * DeltaLambda1)) + (a42 * DeltaLambda2)) + (a43 * DeltaLambda3)));
    double eps;
    double max1 = fabs(p1_3_p0_3);
    if( (max1 < fabs(p1_0_p0_0)) )
    {
        max1 = fabs(p1_0_p0_0);
    } 
    if( (max1 < fabs(p1_1_p0_1)) )
    {
        max1 = fabs(p1_1_p0_1);
    } 
    if( (max1 < fabs(p1_2_p0_2)) )
    {
        max1 = fabs(p1_2_p0_2);
    } 
    double max2 = fabs(p2_2_p0_2);
    if( (max2 < fabs(p2_1_p0_1)) )
    {
        max2 = fabs(p2_1_p0_1);
    } 
    if( (max2 < fabs(p2_3_p0_3)) )
    {
        max2 = fabs(p2_3_p0_3);
    } 
    if( (max2 < fabs(p2_0_p0_0)) )
    {
        max2 = fabs(p2_0_p0_0);
    } 
    double max3 = fabs(p3_0_p0_0);
    if( (max3 < fabs(p3_1_p0_1)) )
    {
        max3 = fabs(p3_1_p0_1);
    } 
    if( (max3 < fabs(p3_3_p0_3)) )
    {
        max3 = fabs(p3_3_p0_3);
    } 
    if( (max3 < fabs(p3_2_p0_2)) )
    {
        max3 = fabs(p3_2_p0_2);
    } 
    double max4 = fabs(q0_3_p0_3);
    if( (max4 < fabs(q0_0_p0_0)) )
    {
        max4 = fabs(q0_0_p0_0);
    } 
    if( (max4 < fabs(q0_1_p0_1)) )
    {
        max4 = fabs(q0_1_p0_1);
    } 
    if( (max4 < fabs(q0_2_p0_2)) )
    {
        max4 = fabs(q0_2_p0_2);
    } 
    if( (max4 < fabs(q1_0_p0_0)) )
    {
        max4 = fabs(q1_0_p0_0);
    } 
    if( (max4 < fabs(q1_1_p0_1)) )
    {
        max4 = fabs(q1_1_p0_1);
    } 
    if( (max4 < fabs(q1_2_p0_2)) )
    {
        max4 = fabs(q1_2_p0_2);
    } 
    if( (max4 < fabs(q1_3_p0_3)) )
    {
        max4 = fabs(q1_3_p0_3);
    } 
    double max5 = fabs(q1_0_p0_0);
    if( (max5 < fabs(q1_1_p0_1)) )
    {
        max5 = fabs(q1_1_p0_1);
    } 
    if( (max5 < fabs(q1_2_p0_2)) )
    {
        max5 = fabs(q1_2_p0_2);
    } 
    if( (max5 < fabs(q1_3_p0_3)) )
    {
        max5 = fabs(q1_3_p0_3);
    } 
    if( (max5 < fabs(q2_0_p0_0)) )
    {
        max5 = fabs(q2_0_p0_0);
    } 
    if( (max5 < fabs(q2_1_p0_1)) )
    {
        max5 = fabs(q2_1_p0_1);
    } 
    if( (max5 < fabs(q2_2_p0_2)) )
    {
        max5 = fabs(q2_2_p0_2);
    } 
    if( (max5 < fabs(q2_3_p0_3)) )
    {
        max5 = fabs(q2_3_p0_3);
    } 
    double max6 = fabs(q2_0_p0_0);
    if( (max6 < fabs(q2_1_p0_1)) )
    {
        max6 = fabs(q2_1_p0_1);
    } 
    if( (max6 < fabs(q2_2_p0_2)) )
    {
        max6 = fabs(q2_2_p0_2);
    } 
    if( (max6 < fabs(q2_3_p0_3)) )
    {
        max6 = fabs(q2_3_p0_3);
    } 
    if( (max6 < fabs(q3_0_p0_0)) )
    {
        max6 = fabs(q3_0_p0_0);
    } 
    if( (max6 < fabs(q3_1_p0_1)) )
    {
        max6 = fabs(q3_1_p0_1);
    } 
    if( (max6 < fabs(q3_2_p0_2)) )
    {
        max6 = fabs(q3_2_p0_2);
    } 
    if( (max6 < fabs(q3_3_p0_3)) )
    {
        max6 = fabs(q3_3_p0_3);
    } 
    double lower_bound_1;
    double upper_bound_1;
    int Delta_sign;
    int int_tmp_result;
    lower_bound_1 = max1;
    upper_bound_1 = max1;
    if( (max2 < lower_bound_1) )
    {
        lower_bound_1 = max2;
    } 
    else 
    {
        if( (max2 > upper_bound_1) )
        {
            upper_bound_1 = max2;
        } 
    } 
    if( (max3 < lower_bound_1) )
    {
        lower_bound_1 = max3;
    } 
    else 
    {
        if( (max3 > upper_bound_1) )
        {
            upper_bound_1 = max3;
        } 
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
    if( (max6 < lower_bound_1) )
    {
        lower_bound_1 = max6;
    } 
    else 
    {
        if( (max6 > upper_bound_1) )
        {
            upper_bound_1 = max6;
        } 
    } 
    if( (lower_bound_1 < 4.14607644401726239868e-50) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 4.83570327845851562508e+24) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (4.38046888801178809320e-12 * (((((max1 * max4) * max2) * max5) * max3) * max6));
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
    Delta_sign = int_tmp_result;
    int int_tmp_result_FFWKCAA;
    double max7 = max1;
    if( (max7 < max2) )
    {
        max7 = max2;
    } 
    if( (max7 < max3) )
    {
        max7 = max3;
    } 
    if( (max7 < max6) )
    {
        max7 = max6;
    } 
    double max8 = max1;
    if( (max8 < fabs(p4_1_p0_1)) )
    {
        max8 = fabs(p4_1_p0_1);
    } 
    if( (max8 < fabs(p4_2_p0_2)) )
    {
        max8 = fabs(p4_2_p0_2);
    } 
    if( (max8 < fabs(p4_0_p0_0)) )
    {
        max8 = fabs(p4_0_p0_0);
    } 
    if( (max8 < fabs(p4_3_p0_3)) )
    {
        max8 = fabs(p4_3_p0_3);
    } 
    if( (max7 < max8) )
    {
        max7 = max8;
    } 
    double max9 = max1;
    if( (max9 < max5) )
    {
        max9 = max5;
    } 
    if( (max9 < max8) )
    {
        max9 = max8;
    } 
    double max10;
    double max11 = max4;
    if( (max11 < max5) )
    {
        max11 = max5;
    } 
    max10 = max11;
    if( (max10 < max1) )
    {
        max10 = max1;
    } 
    if( (max10 < max4) )
    {
        max10 = max4;
    } 
    if( (max10 < max5) )
    {
        max10 = max5;
    } 
    if( (max10 < max6) )
    {
        max10 = max6;
    } 
    lower_bound_1 = max10;
    upper_bound_1 = max10;
    if( (max11 < lower_bound_1) )
    {
        lower_bound_1 = max11;
    } 
    else 
    {
        if( (max11 > upper_bound_1) )
        {
            upper_bound_1 = max11;
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
    if( (max7 < lower_bound_1) )
    {
        lower_bound_1 = max7;
    } 
    else 
    {
        if( (max7 > upper_bound_1) )
        {
            upper_bound_1 = max7;
        } 
    } 
    if( (max8 < lower_bound_1) )
    {
        lower_bound_1 = max8;
    } 
    if( (max9 < lower_bound_1) )
    {
        lower_bound_1 = max9;
    } 
    else 
    {
        if( (max9 > upper_bound_1) )
        {
            upper_bound_1 = max9;
        } 
    } 
    if( (lower_bound_1 < 6.06263132863556750071e-38) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 4.83570327845851562508e+24) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (1.21914442286604163181e-10 * (((((((max8 * max11) * max2) * max10) * max3) * max10) * max9) * max7));
        if( (r > eps) )
        {
            int_tmp_result_FFWKCAA = 1;
        } 
        else 
        {
            if( (r < -eps) )
            {
                int_tmp_result_FFWKCAA = -1;
            } 
            else 
            {
                return FPG_UNCERTAIN_VALUE;
            } 
        } 
    } 
    return (Delta_sign * int_tmp_result_FFWKCAA);
} 


inline int side4_6d_filter( const double* p0, const double* p1, const double* p2, const double* p3, const double* p4, const double* q0, const double* q1, const double* q2, const double* q3) {
    double p1_0_p0_0 = (p1[0] - p0[0]);
    double p1_1_p0_1 = (p1[1] - p0[1]);
    double p1_2_p0_2 = (p1[2] - p0[2]);
    double p1_3_p0_3 = (p1[3] - p0[3]);
    double p1_4_p0_4 = (p1[4] - p0[4]);
    double p1_5_p0_5 = (p1[5] - p0[5]);
    double l1;
    l1 = (1 * ((((((p1_0_p0_0 * p1_0_p0_0) + (p1_1_p0_1 * p1_1_p0_1)) + (p1_2_p0_2 * p1_2_p0_2)) + (p1_3_p0_3 * p1_3_p0_3)) + (p1_4_p0_4 * p1_4_p0_4)) + (p1_5_p0_5 * p1_5_p0_5)));
    double p2_0_p0_0 = (p2[0] - p0[0]);
    double p2_1_p0_1 = (p2[1] - p0[1]);
    double p2_2_p0_2 = (p2[2] - p0[2]);
    double p2_3_p0_3 = (p2[3] - p0[3]);
    double p2_4_p0_4 = (p2[4] - p0[4]);
    double p2_5_p0_5 = (p2[5] - p0[5]);
    double l2;
    l2 = (1 * ((((((p2_0_p0_0 * p2_0_p0_0) + (p2_1_p0_1 * p2_1_p0_1)) + (p2_2_p0_2 * p2_2_p0_2)) + (p2_3_p0_3 * p2_3_p0_3)) + (p2_4_p0_4 * p2_4_p0_4)) + (p2_5_p0_5 * p2_5_p0_5)));
    double p3_0_p0_0 = (p3[0] - p0[0]);
    double p3_1_p0_1 = (p3[1] - p0[1]);
    double p3_2_p0_2 = (p3[2] - p0[2]);
    double p3_3_p0_3 = (p3[3] - p0[3]);
    double p3_4_p0_4 = (p3[4] - p0[4]);
    double p3_5_p0_5 = (p3[5] - p0[5]);
    double l3;
    l3 = (1 * ((((((p3_0_p0_0 * p3_0_p0_0) + (p3_1_p0_1 * p3_1_p0_1)) + (p3_2_p0_2 * p3_2_p0_2)) + (p3_3_p0_3 * p3_3_p0_3)) + (p3_4_p0_4 * p3_4_p0_4)) + (p3_5_p0_5 * p3_5_p0_5)));
    double p4_0_p0_0 = (p4[0] - p0[0]);
    double p4_1_p0_1 = (p4[1] - p0[1]);
    double p4_2_p0_2 = (p4[2] - p0[2]);
    double p4_3_p0_3 = (p4[3] - p0[3]);
    double p4_4_p0_4 = (p4[4] - p0[4]);
    double p4_5_p0_5 = (p4[5] - p0[5]);
    double l4;
    l4 = (1 * ((((((p4_0_p0_0 * p4_0_p0_0) + (p4_1_p0_1 * p4_1_p0_1)) + (p4_2_p0_2 * p4_2_p0_2)) + (p4_3_p0_3 * p4_3_p0_3)) + (p4_4_p0_4 * p4_4_p0_4)) + (p4_5_p0_5 * p4_5_p0_5)));
    double q0_0_p0_0 = (q0[0] - p0[0]);
    double q0_1_p0_1 = (q0[1] - p0[1]);
    double q0_2_p0_2 = (q0[2] - p0[2]);
    double q0_3_p0_3 = (q0[3] - p0[3]);
    double q0_4_p0_4 = (q0[4] - p0[4]);
    double q0_5_p0_5 = (q0[5] - p0[5]);
    double a10;
    a10 = (2 * ((((((p1_0_p0_0 * q0_0_p0_0) + (p1_1_p0_1 * q0_1_p0_1)) + (p1_2_p0_2 * q0_2_p0_2)) + (p1_3_p0_3 * q0_3_p0_3)) + (p1_4_p0_4 * q0_4_p0_4)) + (p1_5_p0_5 * q0_5_p0_5)));
    double q1_0_p0_0 = (q1[0] - p0[0]);
    double q1_1_p0_1 = (q1[1] - p0[1]);
    double q1_2_p0_2 = (q1[2] - p0[2]);
    double q1_3_p0_3 = (q1[3] - p0[3]);
    double q1_4_p0_4 = (q1[4] - p0[4]);
    double q1_5_p0_5 = (q1[5] - p0[5]);
    double a11;
    a11 = (2 * ((((((p1_0_p0_0 * q1_0_p0_0) + (p1_1_p0_1 * q1_1_p0_1)) + (p1_2_p0_2 * q1_2_p0_2)) + (p1_3_p0_3 * q1_3_p0_3)) + (p1_4_p0_4 * q1_4_p0_4)) + (p1_5_p0_5 * q1_5_p0_5)));
    double q2_0_p0_0 = (q2[0] - p0[0]);
    double q2_1_p0_1 = (q2[1] - p0[1]);
    double q2_2_p0_2 = (q2[2] - p0[2]);
    double q2_3_p0_3 = (q2[3] - p0[3]);
    double q2_4_p0_4 = (q2[4] - p0[4]);
    double q2_5_p0_5 = (q2[5] - p0[5]);
    double a12;
    a12 = (2 * ((((((p1_0_p0_0 * q2_0_p0_0) + (p1_1_p0_1 * q2_1_p0_1)) + (p1_2_p0_2 * q2_2_p0_2)) + (p1_3_p0_3 * q2_3_p0_3)) + (p1_4_p0_4 * q2_4_p0_4)) + (p1_5_p0_5 * q2_5_p0_5)));
    double q3_0_p0_0 = (q3[0] - p0[0]);
    double q3_1_p0_1 = (q3[1] - p0[1]);
    double q3_2_p0_2 = (q3[2] - p0[2]);
    double q3_3_p0_3 = (q3[3] - p0[3]);
    double q3_4_p0_4 = (q3[4] - p0[4]);
    double q3_5_p0_5 = (q3[5] - p0[5]);
    double a13;
    a13 = (2 * ((((((p1_0_p0_0 * q3_0_p0_0) + (p1_1_p0_1 * q3_1_p0_1)) + (p1_2_p0_2 * q3_2_p0_2)) + (p1_3_p0_3 * q3_3_p0_3)) + (p1_4_p0_4 * q3_4_p0_4)) + (p1_5_p0_5 * q3_5_p0_5)));
    double a20;
    a20 = (2 * ((((((p2_0_p0_0 * q0_0_p0_0) + (p2_1_p0_1 * q0_1_p0_1)) + (p2_2_p0_2 * q0_2_p0_2)) + (p2_3_p0_3 * q0_3_p0_3)) + (p2_4_p0_4 * q0_4_p0_4)) + (p2_5_p0_5 * q0_5_p0_5)));
    double a21;
    a21 = (2 * ((((((p2_0_p0_0 * q1_0_p0_0) + (p2_1_p0_1 * q1_1_p0_1)) + (p2_2_p0_2 * q1_2_p0_2)) + (p2_3_p0_3 * q1_3_p0_3)) + (p2_4_p0_4 * q1_4_p0_4)) + (p2_5_p0_5 * q1_5_p0_5)));
    double a22;
    a22 = (2 * ((((((p2_0_p0_0 * q2_0_p0_0) + (p2_1_p0_1 * q2_1_p0_1)) + (p2_2_p0_2 * q2_2_p0_2)) + (p2_3_p0_3 * q2_3_p0_3)) + (p2_4_p0_4 * q2_4_p0_4)) + (p2_5_p0_5 * q2_5_p0_5)));
    double a23;
    a23 = (2 * ((((((p2_0_p0_0 * q3_0_p0_0) + (p2_1_p0_1 * q3_1_p0_1)) + (p2_2_p0_2 * q3_2_p0_2)) + (p2_3_p0_3 * q3_3_p0_3)) + (p2_4_p0_4 * q3_4_p0_4)) + (p2_5_p0_5 * q3_5_p0_5)));
    double a30;
    a30 = (2 * ((((((p3_0_p0_0 * q0_0_p0_0) + (p3_1_p0_1 * q0_1_p0_1)) + (p3_2_p0_2 * q0_2_p0_2)) + (p3_3_p0_3 * q0_3_p0_3)) + (p3_4_p0_4 * q0_4_p0_4)) + (p3_5_p0_5 * q0_5_p0_5)));
    double a31;
    a31 = (2 * ((((((p3_0_p0_0 * q1_0_p0_0) + (p3_1_p0_1 * q1_1_p0_1)) + (p3_2_p0_2 * q1_2_p0_2)) + (p3_3_p0_3 * q1_3_p0_3)) + (p3_4_p0_4 * q1_4_p0_4)) + (p3_5_p0_5 * q1_5_p0_5)));
    double a32;
    a32 = (2 * ((((((p3_0_p0_0 * q2_0_p0_0) + (p3_1_p0_1 * q2_1_p0_1)) + (p3_2_p0_2 * q2_2_p0_2)) + (p3_3_p0_3 * q2_3_p0_3)) + (p3_4_p0_4 * q2_4_p0_4)) + (p3_5_p0_5 * q2_5_p0_5)));
    double a33;
    a33 = (2 * ((((((p3_0_p0_0 * q3_0_p0_0) + (p3_1_p0_1 * q3_1_p0_1)) + (p3_2_p0_2 * q3_2_p0_2)) + (p3_3_p0_3 * q3_3_p0_3)) + (p3_4_p0_4 * q3_4_p0_4)) + (p3_5_p0_5 * q3_5_p0_5)));
    double a40;
    a40 = (2 * ((((((p4_0_p0_0 * q0_0_p0_0) + (p4_1_p0_1 * q0_1_p0_1)) + (p4_2_p0_2 * q0_2_p0_2)) + (p4_3_p0_3 * q0_3_p0_3)) + (p4_4_p0_4 * q0_4_p0_4)) + (p4_5_p0_5 * q0_5_p0_5)));
    double a41;
    a41 = (2 * ((((((p4_0_p0_0 * q1_0_p0_0) + (p4_1_p0_1 * q1_1_p0_1)) + (p4_2_p0_2 * q1_2_p0_2)) + (p4_3_p0_3 * q1_3_p0_3)) + (p4_4_p0_4 * q1_4_p0_4)) + (p4_5_p0_5 * q1_5_p0_5)));
    double a42;
    a42 = (2 * ((((((p4_0_p0_0 * q2_0_p0_0) + (p4_1_p0_1 * q2_1_p0_1)) + (p4_2_p0_2 * q2_2_p0_2)) + (p4_3_p0_3 * q2_3_p0_3)) + (p4_4_p0_4 * q2_4_p0_4)) + (p4_5_p0_5 * q2_5_p0_5)));
    double a43;
    a43 = (2 * ((((((p4_0_p0_0 * q3_0_p0_0) + (p4_1_p0_1 * q3_1_p0_1)) + (p4_2_p0_2 * q3_2_p0_2)) + (p4_3_p0_3 * q3_3_p0_3)) + (p4_4_p0_4 * q3_4_p0_4)) + (p4_5_p0_5 * q3_5_p0_5)));
    double b00;
    b00 = (((a11 * ((a22 * a33) - (a23 * a32))) - (a21 * ((a12 * a33) - (a13 * a32)))) + (a31 * ((a12 * a23) - (a13 * a22))));
    double b01;
    b01 = -((((a22 * a33) - (a23 * a32)) + ((a23 * a31) - (a21 * a33))) + ((a21 * a32) - (a22 * a31)));
    double b02;
    b02 = ((((a12 * a33) - (a13 * a32)) + ((a13 * a31) - (a11 * a33))) + ((a11 * a32) - (a12 * a31)));
    double b03;
    b03 = -((((a12 * a23) - (a13 * a22)) + ((a13 * a21) - (a11 * a23))) + ((a11 * a22) - (a12 * a21)));
    double b10;
    b10 = -(((a10 * ((a22 * a33) - (a23 * a32))) - (a20 * ((a12 * a33) - (a13 * a32)))) + (a30 * ((a12 * a23) - (a13 * a22))));
    double b11;
    b11 = ((((a22 * a33) - (a23 * a32)) + ((a23 * a30) - (a20 * a33))) + ((a20 * a32) - (a22 * a30)));
    double b12;
    b12 = -((((a12 * a33) - (a13 * a32)) + ((a13 * a30) - (a10 * a33))) + ((a10 * a32) - (a12 * a30)));
    double b13;
    b13 = ((((a12 * a23) - (a13 * a22)) + ((a13 * a20) - (a10 * a23))) + ((a10 * a22) - (a12 * a20)));
    double b20;
    b20 = (((a10 * ((a21 * a33) - (a23 * a31))) - (a20 * ((a11 * a33) - (a13 * a31)))) + (a30 * ((a11 * a23) - (a13 * a21))));
    double b21;
    b21 = -((((a21 * a33) - (a23 * a31)) + ((a23 * a30) - (a20 * a33))) + ((a20 * a31) - (a21 * a30)));
    double b22;
    b22 = ((((a11 * a33) - (a13 * a31)) + ((a13 * a30) - (a10 * a33))) + ((a10 * a31) - (a11 * a30)));
    double b23;
    b23 = -((((a11 * a23) - (a13 * a21)) + ((a13 * a20) - (a10 * a23))) + ((a10 * a21) - (a11 * a20)));
    double b30;
    b30 = -(((a10 * ((a21 * a32) - (a22 * a31))) - (a20 * ((a11 * a32) - (a12 * a31)))) + (a30 * ((a11 * a22) - (a12 * a21))));
    double b31;
    b31 = ((((a21 * a32) - (a22 * a31)) + ((a22 * a30) - (a20 * a32))) + ((a20 * a31) - (a21 * a30)));
    double b32;
    b32 = -((((a11 * a32) - (a12 * a31)) + ((a12 * a30) - (a10 * a32))) + ((a10 * a31) - (a11 * a30)));
    double b33;
    b33 = ((((a11 * a22) - (a12 * a21)) + ((a12 * a20) - (a10 * a22))) + ((a10 * a21) - (a11 * a20)));
    double Delta;
    Delta = (((b00 + b10) + b20) + b30);
    double DeltaLambda0;
    DeltaLambda0 = ((((b01 * l1) + (b02 * l2)) + (b03 * l3)) + b00);
    double DeltaLambda1;
    DeltaLambda1 = ((((b11 * l1) + (b12 * l2)) + (b13 * l3)) + b10);
    double DeltaLambda2;
    DeltaLambda2 = ((((b21 * l1) + (b22 * l2)) + (b23 * l3)) + b20);
    double DeltaLambda3;
    DeltaLambda3 = ((((b31 * l1) + (b32 * l2)) + (b33 * l3)) + b30);
    double r;
    r = ((Delta * l4) - ((((a40 * DeltaLambda0) + (a41 * DeltaLambda1)) + (a42 * DeltaLambda2)) + (a43 * DeltaLambda3)));
    double eps;
    double max1 = fabs(p3_2_p0_2);
    if( (max1 < fabs(p3_0_p0_0)) )
    {
        max1 = fabs(p3_0_p0_0);
    } 
    if( (max1 < fabs(p3_3_p0_3)) )
    {
        max1 = fabs(p3_3_p0_3);
    } 
    if( (max1 < fabs(p3_4_p0_4)) )
    {
        max1 = fabs(p3_4_p0_4);
    } 
    if( (max1 < fabs(p3_1_p0_1)) )
    {
        max1 = fabs(p3_1_p0_1);
    } 
    if( (max1 < fabs(p3_5_p0_5)) )
    {
        max1 = fabs(p3_5_p0_5);
    } 
    double max2 = fabs(p2_1_p0_1);
    if( (max2 < fabs(p2_4_p0_4)) )
    {
        max2 = fabs(p2_4_p0_4);
    } 
    if( (max2 < fabs(p2_2_p0_2)) )
    {
        max2 = fabs(p2_2_p0_2);
    } 
    if( (max2 < fabs(p2_0_p0_0)) )
    {
        max2 = fabs(p2_0_p0_0);
    } 
    if( (max2 < fabs(p2_3_p0_3)) )
    {
        max2 = fabs(p2_3_p0_3);
    } 
    if( (max2 < fabs(p2_5_p0_5)) )
    {
        max2 = fabs(p2_5_p0_5);
    } 
    double max3 = fabs(p1_0_p0_0);
    if( (max3 < fabs(p1_1_p0_1)) )
    {
        max3 = fabs(p1_1_p0_1);
    } 
    if( (max3 < fabs(p1_2_p0_2)) )
    {
        max3 = fabs(p1_2_p0_2);
    } 
    if( (max3 < fabs(p1_3_p0_3)) )
    {
        max3 = fabs(p1_3_p0_3);
    } 
    if( (max3 < fabs(p1_4_p0_4)) )
    {
        max3 = fabs(p1_4_p0_4);
    } 
    if( (max3 < fabs(p1_5_p0_5)) )
    {
        max3 = fabs(p1_5_p0_5);
    } 
    double max4 = fabs(q0_0_p0_0);
    if( (max4 < fabs(q0_1_p0_1)) )
    {
        max4 = fabs(q0_1_p0_1);
    } 
    if( (max4 < fabs(q0_2_p0_2)) )
    {
        max4 = fabs(q0_2_p0_2);
    } 
    if( (max4 < fabs(q0_3_p0_3)) )
    {
        max4 = fabs(q0_3_p0_3);
    } 
    if( (max4 < fabs(q0_4_p0_4)) )
    {
        max4 = fabs(q0_4_p0_4);
    } 
    if( (max4 < fabs(q0_5_p0_5)) )
    {
        max4 = fabs(q0_5_p0_5);
    } 
    if( (max4 < fabs(q1_0_p0_0)) )
    {
        max4 = fabs(q1_0_p0_0);
    } 
    if( (max4 < fabs(q1_1_p0_1)) )
    {
        max4 = fabs(q1_1_p0_1);
    } 
    if( (max4 < fabs(q1_2_p0_2)) )
    {
        max4 = fabs(q1_2_p0_2);
    } 
    if( (max4 < fabs(q1_3_p0_3)) )
    {
        max4 = fabs(q1_3_p0_3);
    } 
    if( (max4 < fabs(q1_4_p0_4)) )
    {
        max4 = fabs(q1_4_p0_4);
    } 
    if( (max4 < fabs(q1_5_p0_5)) )
    {
        max4 = fabs(q1_5_p0_5);
    } 
    double max5 = fabs(q1_0_p0_0);
    if( (max5 < fabs(q1_1_p0_1)) )
    {
        max5 = fabs(q1_1_p0_1);
    } 
    if( (max5 < fabs(q1_2_p0_2)) )
    {
        max5 = fabs(q1_2_p0_2);
    } 
    if( (max5 < fabs(q1_3_p0_3)) )
    {
        max5 = fabs(q1_3_p0_3);
    } 
    if( (max5 < fabs(q1_4_p0_4)) )
    {
        max5 = fabs(q1_4_p0_4);
    } 
    if( (max5 < fabs(q1_5_p0_5)) )
    {
        max5 = fabs(q1_5_p0_5);
    } 
    if( (max5 < fabs(q2_0_p0_0)) )
    {
        max5 = fabs(q2_0_p0_0);
    } 
    if( (max5 < fabs(q2_1_p0_1)) )
    {
        max5 = fabs(q2_1_p0_1);
    } 
    if( (max5 < fabs(q2_2_p0_2)) )
    {
        max5 = fabs(q2_2_p0_2);
    } 
    if( (max5 < fabs(q2_3_p0_3)) )
    {
        max5 = fabs(q2_3_p0_3);
    } 
    if( (max5 < fabs(q2_4_p0_4)) )
    {
        max5 = fabs(q2_4_p0_4);
    } 
    if( (max5 < fabs(q2_5_p0_5)) )
    {
        max5 = fabs(q2_5_p0_5);
    } 
    double max6 = fabs(q2_0_p0_0);
    if( (max6 < fabs(q2_1_p0_1)) )
    {
        max6 = fabs(q2_1_p0_1);
    } 
    if( (max6 < fabs(q2_2_p0_2)) )
    {
        max6 = fabs(q2_2_p0_2);
    } 
    if( (max6 < fabs(q2_3_p0_3)) )
    {
        max6 = fabs(q2_3_p0_3);
    } 
    if( (max6 < fabs(q2_4_p0_4)) )
    {
        max6 = fabs(q2_4_p0_4);
    } 
    if( (max6 < fabs(q2_5_p0_5)) )
    {
        max6 = fabs(q2_5_p0_5);
    } 
    if( (max6 < fabs(q3_0_p0_0)) )
    {
        max6 = fabs(q3_0_p0_0);
    } 
    if( (max6 < fabs(q3_1_p0_1)) )
    {
        max6 = fabs(q3_1_p0_1);
    } 
    if( (max6 < fabs(q3_2_p0_2)) )
    {
        max6 = fabs(q3_2_p0_2);
    } 
    if( (max6 < fabs(q3_3_p0_3)) )
    {
        max6 = fabs(q3_3_p0_3);
    } 
    if( (max6 < fabs(q3_4_p0_4)) )
    {
        max6 = fabs(q3_4_p0_4);
    } 
    if( (max6 < fabs(q3_5_p0_5)) )
    {
        max6 = fabs(q3_5_p0_5);
    } 
    double lower_bound_1;
    double upper_bound_1;
    int Delta_sign;
    int int_tmp_result;
    lower_bound_1 = max4;
    upper_bound_1 = max4;
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
    else 
    {
        if( (max2 > upper_bound_1) )
        {
            upper_bound_1 = max2;
        } 
    } 
    if( (max3 < lower_bound_1) )
    {
        lower_bound_1 = max3;
    } 
    else 
    {
        if( (max3 > upper_bound_1) )
        {
            upper_bound_1 = max3;
        } 
    } 
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
    if( (max6 < lower_bound_1) )
    {
        lower_bound_1 = max6;
    } 
    else 
    {
        if( (max6 > upper_bound_1) )
        {
            upper_bound_1 = max6;
        } 
    } 
    if( (lower_bound_1 < 3.31864264949884013629e-50) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 4.83570327845851562508e+24) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (1.66564133587113165316e-11 * (((((max3 * max4) * max2) * max5) * max1) * max6));
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
    Delta_sign = int_tmp_result;
    int int_tmp_result_FFWKCAA;
    double max7 = max1;
    if( (max7 < max2) )
    {
        max7 = max2;
    } 
    if( (max7 < max3) )
    {
        max7 = max3;
    } 
    double max8 = max3;
    if( (max8 < fabs(p4_5_p0_5)) )
    {
        max8 = fabs(p4_5_p0_5);
    } 
    if( (max8 < fabs(p4_0_p0_0)) )
    {
        max8 = fabs(p4_0_p0_0);
    } 
    if( (max8 < fabs(p4_1_p0_1)) )
    {
        max8 = fabs(p4_1_p0_1);
    } 
    if( (max8 < fabs(p4_2_p0_2)) )
    {
        max8 = fabs(p4_2_p0_2);
    } 
    if( (max8 < fabs(p4_3_p0_3)) )
    {
        max8 = fabs(p4_3_p0_3);
    } 
    if( (max8 < fabs(p4_4_p0_4)) )
    {
        max8 = fabs(p4_4_p0_4);
    } 
    if( (max7 < max8) )
    {
        max7 = max8;
    } 
    if( (max7 < max6) )
    {
        max7 = max6;
    } 
    double max9 = max3;
    if( (max9 < max8) )
    {
        max9 = max8;
    } 
    if( (max9 < max5) )
    {
        max9 = max5;
    } 
    double max10 = max4;
    if( (max10 < max3) )
    {
        max10 = max3;
    } 
    if( (max10 < max5) )
    {
        max10 = max5;
    } 
    double max11 = max4;
    if( (max11 < max5) )
    {
        max11 = max5;
    } 
    if( (max10 < max11) )
    {
        max10 = max11;
    } 
    if( (max10 < max6) )
    {
        max10 = max6;
    } 
    lower_bound_1 = max1;
    upper_bound_1 = max1;
    if( (max2 < lower_bound_1) )
    {
        lower_bound_1 = max2;
    } 
    if( (max7 < lower_bound_1) )
    {
        lower_bound_1 = max7;
    } 
    else 
    {
        if( (max7 > upper_bound_1) )
        {
            upper_bound_1 = max7;
        } 
    } 
    if( (max8 < lower_bound_1) )
    {
        lower_bound_1 = max8;
    } 
    if( (max9 < lower_bound_1) )
    {
        lower_bound_1 = max9;
    } 
    else 
    {
        if( (max9 > upper_bound_1) )
        {
            upper_bound_1 = max9;
        } 
    } 
    if( (max10 < lower_bound_1) )
    {
        lower_bound_1 = max10;
    } 
    else 
    {
        if( (max10 > upper_bound_1) )
        {
            upper_bound_1 = max10;
        } 
    } 
    if( (max11 < lower_bound_1) )
    {
        lower_bound_1 = max11;
    } 
    else 
    {
        if( (max11 > upper_bound_1) )
        {
            upper_bound_1 = max11;
        } 
    } 
    if( (lower_bound_1 < 4.87975611107819181771e-38) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 4.83570327845851562508e+24) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (6.92085098542795335117e-10 * (((((((max8 * max11) * max2) * max10) * max1) * max10) * max9) * max7));
        if( (r > eps) )
        {
            int_tmp_result_FFWKCAA = 1;
        } 
        else 
        {
            if( (r < -eps) )
            {
                int_tmp_result_FFWKCAA = -1;
            } 
            else 
            {
                return FPG_UNCERTAIN_VALUE;
            } 
        } 
    } 
    return (Delta_sign * int_tmp_result_FFWKCAA);
} 


inline int side4_7d_filter( const double* p0, const double* p1, const double* p2, const double* p3, const double* p4, const double* q0, const double* q1, const double* q2, const double* q3) {
    double p1_0_p0_0 = (p1[0] - p0[0]);
    double p1_1_p0_1 = (p1[1] - p0[1]);
    double p1_2_p0_2 = (p1[2] - p0[2]);
    double p1_3_p0_3 = (p1[3] - p0[3]);
    double p1_4_p0_4 = (p1[4] - p0[4]);
    double p1_5_p0_5 = (p1[5] - p0[5]);
    double p1_6_p0_6 = (p1[6] - p0[6]);
    double l1;
    l1 = (1 * (((((((p1_0_p0_0 * p1_0_p0_0) + (p1_1_p0_1 * p1_1_p0_1)) + (p1_2_p0_2 * p1_2_p0_2)) + (p1_3_p0_3 * p1_3_p0_3)) + (p1_4_p0_4 * p1_4_p0_4)) + (p1_5_p0_5 * p1_5_p0_5)) + (p1_6_p0_6 * p1_6_p0_6)));
    double p2_0_p0_0 = (p2[0] - p0[0]);
    double p2_1_p0_1 = (p2[1] - p0[1]);
    double p2_2_p0_2 = (p2[2] - p0[2]);
    double p2_3_p0_3 = (p2[3] - p0[3]);
    double p2_4_p0_4 = (p2[4] - p0[4]);
    double p2_5_p0_5 = (p2[5] - p0[5]);
    double p2_6_p0_6 = (p2[6] - p0[6]);
    double l2;
    l2 = (1 * (((((((p2_0_p0_0 * p2_0_p0_0) + (p2_1_p0_1 * p2_1_p0_1)) + (p2_2_p0_2 * p2_2_p0_2)) + (p2_3_p0_3 * p2_3_p0_3)) + (p2_4_p0_4 * p2_4_p0_4)) + (p2_5_p0_5 * p2_5_p0_5)) + (p2_6_p0_6 * p2_6_p0_6)));
    double p3_0_p0_0 = (p3[0] - p0[0]);
    double p3_1_p0_1 = (p3[1] - p0[1]);
    double p3_2_p0_2 = (p3[2] - p0[2]);
    double p3_3_p0_3 = (p3[3] - p0[3]);
    double p3_4_p0_4 = (p3[4] - p0[4]);
    double p3_5_p0_5 = (p3[5] - p0[5]);
    double p3_6_p0_6 = (p3[6] - p0[6]);
    double l3;
    l3 = (1 * (((((((p3_0_p0_0 * p3_0_p0_0) + (p3_1_p0_1 * p3_1_p0_1)) + (p3_2_p0_2 * p3_2_p0_2)) + (p3_3_p0_3 * p3_3_p0_3)) + (p3_4_p0_4 * p3_4_p0_4)) + (p3_5_p0_5 * p3_5_p0_5)) + (p3_6_p0_6 * p3_6_p0_6)));
    double p4_0_p0_0 = (p4[0] - p0[0]);
    double p4_1_p0_1 = (p4[1] - p0[1]);
    double p4_2_p0_2 = (p4[2] - p0[2]);
    double p4_3_p0_3 = (p4[3] - p0[3]);
    double p4_4_p0_4 = (p4[4] - p0[4]);
    double p4_5_p0_5 = (p4[5] - p0[5]);
    double p4_6_p0_6 = (p4[6] - p0[6]);
    double l4;
    l4 = (1 * (((((((p4_0_p0_0 * p4_0_p0_0) + (p4_1_p0_1 * p4_1_p0_1)) + (p4_2_p0_2 * p4_2_p0_2)) + (p4_3_p0_3 * p4_3_p0_3)) + (p4_4_p0_4 * p4_4_p0_4)) + (p4_5_p0_5 * p4_5_p0_5)) + (p4_6_p0_6 * p4_6_p0_6)));
    double q0_0_p0_0 = (q0[0] - p0[0]);
    double q0_1_p0_1 = (q0[1] - p0[1]);
    double q0_2_p0_2 = (q0[2] - p0[2]);
    double q0_3_p0_3 = (q0[3] - p0[3]);
    double q0_4_p0_4 = (q0[4] - p0[4]);
    double q0_5_p0_5 = (q0[5] - p0[5]);
    double q0_6_p0_6 = (q0[6] - p0[6]);
    double a10;
    a10 = (2 * (((((((p1_0_p0_0 * q0_0_p0_0) + (p1_1_p0_1 * q0_1_p0_1)) + (p1_2_p0_2 * q0_2_p0_2)) + (p1_3_p0_3 * q0_3_p0_3)) + (p1_4_p0_4 * q0_4_p0_4)) + (p1_5_p0_5 * q0_5_p0_5)) + (p1_6_p0_6 * q0_6_p0_6)));
    double q1_0_p0_0 = (q1[0] - p0[0]);
    double q1_1_p0_1 = (q1[1] - p0[1]);
    double q1_2_p0_2 = (q1[2] - p0[2]);
    double q1_3_p0_3 = (q1[3] - p0[3]);
    double q1_4_p0_4 = (q1[4] - p0[4]);
    double q1_5_p0_5 = (q1[5] - p0[5]);
    double q1_6_p0_6 = (q1[6] - p0[6]);
    double a11;
    a11 = (2 * (((((((p1_0_p0_0 * q1_0_p0_0) + (p1_1_p0_1 * q1_1_p0_1)) + (p1_2_p0_2 * q1_2_p0_2)) + (p1_3_p0_3 * q1_3_p0_3)) + (p1_4_p0_4 * q1_4_p0_4)) + (p1_5_p0_5 * q1_5_p0_5)) + (p1_6_p0_6 * q1_6_p0_6)));
    double q2_0_p0_0 = (q2[0] - p0[0]);
    double q2_1_p0_1 = (q2[1] - p0[1]);
    double q2_2_p0_2 = (q2[2] - p0[2]);
    double q2_3_p0_3 = (q2[3] - p0[3]);
    double q2_4_p0_4 = (q2[4] - p0[4]);
    double q2_5_p0_5 = (q2[5] - p0[5]);
    double q2_6_p0_6 = (q2[6] - p0[6]);
    double a12;
    a12 = (2 * (((((((p1_0_p0_0 * q2_0_p0_0) + (p1_1_p0_1 * q2_1_p0_1)) + (p1_2_p0_2 * q2_2_p0_2)) + (p1_3_p0_3 * q2_3_p0_3)) + (p1_4_p0_4 * q2_4_p0_4)) + (p1_5_p0_5 * q2_5_p0_5)) + (p1_6_p0_6 * q2_6_p0_6)));
    double q3_0_p0_0 = (q3[0] - p0[0]);
    double q3_1_p0_1 = (q3[1] - p0[1]);
    double q3_2_p0_2 = (q3[2] - p0[2]);
    double q3_3_p0_3 = (q3[3] - p0[3]);
    double q3_4_p0_4 = (q3[4] - p0[4]);
    double q3_5_p0_5 = (q3[5] - p0[5]);
    double q3_6_p0_6 = (q3[6] - p0[6]);
    double a13;
    a13 = (2 * (((((((p1_0_p0_0 * q3_0_p0_0) + (p1_1_p0_1 * q3_1_p0_1)) + (p1_2_p0_2 * q3_2_p0_2)) + (p1_3_p0_3 * q3_3_p0_3)) + (p1_4_p0_4 * q3_4_p0_4)) + (p1_5_p0_5 * q3_5_p0_5)) + (p1_6_p0_6 * q3_6_p0_6)));
    double a20;
    a20 = (2 * (((((((p2_0_p0_0 * q0_0_p0_0) + (p2_1_p0_1 * q0_1_p0_1)) + (p2_2_p0_2 * q0_2_p0_2)) + (p2_3_p0_3 * q0_3_p0_3)) + (p2_4_p0_4 * q0_4_p0_4)) + (p2_5_p0_5 * q0_5_p0_5)) + (p2_6_p0_6 * q0_6_p0_6)));
    double a21;
    a21 = (2 * (((((((p2_0_p0_0 * q1_0_p0_0) + (p2_1_p0_1 * q1_1_p0_1)) + (p2_2_p0_2 * q1_2_p0_2)) + (p2_3_p0_3 * q1_3_p0_3)) + (p2_4_p0_4 * q1_4_p0_4)) + (p2_5_p0_5 * q1_5_p0_5)) + (p2_6_p0_6 * q1_6_p0_6)));
    double a22;
    a22 = (2 * (((((((p2_0_p0_0 * q2_0_p0_0) + (p2_1_p0_1 * q2_1_p0_1)) + (p2_2_p0_2 * q2_2_p0_2)) + (p2_3_p0_3 * q2_3_p0_3)) + (p2_4_p0_4 * q2_4_p0_4)) + (p2_5_p0_5 * q2_5_p0_5)) + (p2_6_p0_6 * q2_6_p0_6)));
    double a23;
    a23 = (2 * (((((((p2_0_p0_0 * q3_0_p0_0) + (p2_1_p0_1 * q3_1_p0_1)) + (p2_2_p0_2 * q3_2_p0_2)) + (p2_3_p0_3 * q3_3_p0_3)) + (p2_4_p0_4 * q3_4_p0_4)) + (p2_5_p0_5 * q3_5_p0_5)) + (p2_6_p0_6 * q3_6_p0_6)));
    double a30;
    a30 = (2 * (((((((p3_0_p0_0 * q0_0_p0_0) + (p3_1_p0_1 * q0_1_p0_1)) + (p3_2_p0_2 * q0_2_p0_2)) + (p3_3_p0_3 * q0_3_p0_3)) + (p3_4_p0_4 * q0_4_p0_4)) + (p3_5_p0_5 * q0_5_p0_5)) + (p3_6_p0_6 * q0_6_p0_6)));
    double a31;
    a31 = (2 * (((((((p3_0_p0_0 * q1_0_p0_0) + (p3_1_p0_1 * q1_1_p0_1)) + (p3_2_p0_2 * q1_2_p0_2)) + (p3_3_p0_3 * q1_3_p0_3)) + (p3_4_p0_4 * q1_4_p0_4)) + (p3_5_p0_5 * q1_5_p0_5)) + (p3_6_p0_6 * q1_6_p0_6)));
    double a32;
    a32 = (2 * (((((((p3_0_p0_0 * q2_0_p0_0) + (p3_1_p0_1 * q2_1_p0_1)) + (p3_2_p0_2 * q2_2_p0_2)) + (p3_3_p0_3 * q2_3_p0_3)) + (p3_4_p0_4 * q2_4_p0_4)) + (p3_5_p0_5 * q2_5_p0_5)) + (p3_6_p0_6 * q2_6_p0_6)));
    double a33;
    a33 = (2 * (((((((p3_0_p0_0 * q3_0_p0_0) + (p3_1_p0_1 * q3_1_p0_1)) + (p3_2_p0_2 * q3_2_p0_2)) + (p3_3_p0_3 * q3_3_p0_3)) + (p3_4_p0_4 * q3_4_p0_4)) + (p3_5_p0_5 * q3_5_p0_5)) + (p3_6_p0_6 * q3_6_p0_6)));
    double a40;
    a40 = (2 * (((((((p4_0_p0_0 * q0_0_p0_0) + (p4_1_p0_1 * q0_1_p0_1)) + (p4_2_p0_2 * q0_2_p0_2)) + (p4_3_p0_3 * q0_3_p0_3)) + (p4_4_p0_4 * q0_4_p0_4)) + (p4_5_p0_5 * q0_5_p0_5)) + (p4_6_p0_6 * q0_6_p0_6)));
    double a41;
    a41 = (2 * (((((((p4_0_p0_0 * q1_0_p0_0) + (p4_1_p0_1 * q1_1_p0_1)) + (p4_2_p0_2 * q1_2_p0_2)) + (p4_3_p0_3 * q1_3_p0_3)) + (p4_4_p0_4 * q1_4_p0_4)) + (p4_5_p0_5 * q1_5_p0_5)) + (p4_6_p0_6 * q1_6_p0_6)));
    double a42;
    a42 = (2 * (((((((p4_0_p0_0 * q2_0_p0_0) + (p4_1_p0_1 * q2_1_p0_1)) + (p4_2_p0_2 * q2_2_p0_2)) + (p4_3_p0_3 * q2_3_p0_3)) + (p4_4_p0_4 * q2_4_p0_4)) + (p4_5_p0_5 * q2_5_p0_5)) + (p4_6_p0_6 * q2_6_p0_6)));
    double a43;
    a43 = (2 * (((((((p4_0_p0_0 * q3_0_p0_0) + (p4_1_p0_1 * q3_1_p0_1)) + (p4_2_p0_2 * q3_2_p0_2)) + (p4_3_p0_3 * q3_3_p0_3)) + (p4_4_p0_4 * q3_4_p0_4)) + (p4_5_p0_5 * q3_5_p0_5)) + (p4_6_p0_6 * q3_6_p0_6)));
    double b00;
    b00 = (((a11 * ((a22 * a33) - (a23 * a32))) - (a21 * ((a12 * a33) - (a13 * a32)))) + (a31 * ((a12 * a23) - (a13 * a22))));
    double b01;
    b01 = -((((a22 * a33) - (a23 * a32)) + ((a23 * a31) - (a21 * a33))) + ((a21 * a32) - (a22 * a31)));
    double b02;
    b02 = ((((a12 * a33) - (a13 * a32)) + ((a13 * a31) - (a11 * a33))) + ((a11 * a32) - (a12 * a31)));
    double b03;
    b03 = -((((a12 * a23) - (a13 * a22)) + ((a13 * a21) - (a11 * a23))) + ((a11 * a22) - (a12 * a21)));
    double b10;
    b10 = -(((a10 * ((a22 * a33) - (a23 * a32))) - (a20 * ((a12 * a33) - (a13 * a32)))) + (a30 * ((a12 * a23) - (a13 * a22))));
    double b11;
    b11 = ((((a22 * a33) - (a23 * a32)) + ((a23 * a30) - (a20 * a33))) + ((a20 * a32) - (a22 * a30)));
    double b12;
    b12 = -((((a12 * a33) - (a13 * a32)) + ((a13 * a30) - (a10 * a33))) + ((a10 * a32) - (a12 * a30)));
    double b13;
    b13 = ((((a12 * a23) - (a13 * a22)) + ((a13 * a20) - (a10 * a23))) + ((a10 * a22) - (a12 * a20)));
    double b20;
    b20 = (((a10 * ((a21 * a33) - (a23 * a31))) - (a20 * ((a11 * a33) - (a13 * a31)))) + (a30 * ((a11 * a23) - (a13 * a21))));
    double b21;
    b21 = -((((a21 * a33) - (a23 * a31)) + ((a23 * a30) - (a20 * a33))) + ((a20 * a31) - (a21 * a30)));
    double b22;
    b22 = ((((a11 * a33) - (a13 * a31)) + ((a13 * a30) - (a10 * a33))) + ((a10 * a31) - (a11 * a30)));
    double b23;
    b23 = -((((a11 * a23) - (a13 * a21)) + ((a13 * a20) - (a10 * a23))) + ((a10 * a21) - (a11 * a20)));
    double b30;
    b30 = -(((a10 * ((a21 * a32) - (a22 * a31))) - (a20 * ((a11 * a32) - (a12 * a31)))) + (a30 * ((a11 * a22) - (a12 * a21))));
    double b31;
    b31 = ((((a21 * a32) - (a22 * a31)) + ((a22 * a30) - (a20 * a32))) + ((a20 * a31) - (a21 * a30)));
    double b32;
    b32 = -((((a11 * a32) - (a12 * a31)) + ((a12 * a30) - (a10 * a32))) + ((a10 * a31) - (a11 * a30)));
    double b33;
    b33 = ((((a11 * a22) - (a12 * a21)) + ((a12 * a20) - (a10 * a22))) + ((a10 * a21) - (a11 * a20)));
    double Delta;
    Delta = (((b00 + b10) + b20) + b30);
    double DeltaLambda0;
    DeltaLambda0 = ((((b01 * l1) + (b02 * l2)) + (b03 * l3)) + b00);
    double DeltaLambda1;
    DeltaLambda1 = ((((b11 * l1) + (b12 * l2)) + (b13 * l3)) + b10);
    double DeltaLambda2;
    DeltaLambda2 = ((((b21 * l1) + (b22 * l2)) + (b23 * l3)) + b20);
    double DeltaLambda3;
    DeltaLambda3 = ((((b31 * l1) + (b32 * l2)) + (b33 * l3)) + b30);
    double r;
    r = ((Delta * l4) - ((((a40 * DeltaLambda0) + (a41 * DeltaLambda1)) + (a42 * DeltaLambda2)) + (a43 * DeltaLambda3)));
    double eps;
    double max1 = fabs(p1_0_p0_0);
    if( (max1 < fabs(p1_1_p0_1)) )
    {
        max1 = fabs(p1_1_p0_1);
    } 
    if( (max1 < fabs(p1_2_p0_2)) )
    {
        max1 = fabs(p1_2_p0_2);
    } 
    if( (max1 < fabs(p1_3_p0_3)) )
    {
        max1 = fabs(p1_3_p0_3);
    } 
    if( (max1 < fabs(p1_4_p0_4)) )
    {
        max1 = fabs(p1_4_p0_4);
    } 
    if( (max1 < fabs(p1_5_p0_5)) )
    {
        max1 = fabs(p1_5_p0_5);
    } 
    if( (max1 < fabs(p1_6_p0_6)) )
    {
        max1 = fabs(p1_6_p0_6);
    } 
    double max2 = fabs(p3_0_p0_0);
    if( (max2 < fabs(p3_4_p0_4)) )
    {
        max2 = fabs(p3_4_p0_4);
    } 
    if( (max2 < fabs(p3_2_p0_2)) )
    {
        max2 = fabs(p3_2_p0_2);
    } 
    if( (max2 < fabs(p3_1_p0_1)) )
    {
        max2 = fabs(p3_1_p0_1);
    } 
    if( (max2 < fabs(p3_3_p0_3)) )
    {
        max2 = fabs(p3_3_p0_3);
    } 
    if( (max2 < fabs(p3_5_p0_5)) )
    {
        max2 = fabs(p3_5_p0_5);
    } 
    if( (max2 < fabs(p3_6_p0_6)) )
    {
        max2 = fabs(p3_6_p0_6);
    } 
    double max3 = fabs(p2_5_p0_5);
    if( (max3 < fabs(p2_2_p0_2)) )
    {
        max3 = fabs(p2_2_p0_2);
    } 
    if( (max3 < fabs(p2_3_p0_3)) )
    {
        max3 = fabs(p2_3_p0_3);
    } 
    if( (max3 < fabs(p2_0_p0_0)) )
    {
        max3 = fabs(p2_0_p0_0);
    } 
    if( (max3 < fabs(p2_1_p0_1)) )
    {
        max3 = fabs(p2_1_p0_1);
    } 
    if( (max3 < fabs(p2_6_p0_6)) )
    {
        max3 = fabs(p2_6_p0_6);
    } 
    if( (max3 < fabs(p2_4_p0_4)) )
    {
        max3 = fabs(p2_4_p0_4);
    } 
    double max4 = fabs(q0_0_p0_0);
    if( (max4 < fabs(q0_1_p0_1)) )
    {
        max4 = fabs(q0_1_p0_1);
    } 
    if( (max4 < fabs(q0_2_p0_2)) )
    {
        max4 = fabs(q0_2_p0_2);
    } 
    if( (max4 < fabs(q0_3_p0_3)) )
    {
        max4 = fabs(q0_3_p0_3);
    } 
    if( (max4 < fabs(q0_4_p0_4)) )
    {
        max4 = fabs(q0_4_p0_4);
    } 
    if( (max4 < fabs(q0_5_p0_5)) )
    {
        max4 = fabs(q0_5_p0_5);
    } 
    if( (max4 < fabs(q0_6_p0_6)) )
    {
        max4 = fabs(q0_6_p0_6);
    } 
    if( (max4 < fabs(q1_0_p0_0)) )
    {
        max4 = fabs(q1_0_p0_0);
    } 
    if( (max4 < fabs(q1_1_p0_1)) )
    {
        max4 = fabs(q1_1_p0_1);
    } 
    if( (max4 < fabs(q1_2_p0_2)) )
    {
        max4 = fabs(q1_2_p0_2);
    } 
    if( (max4 < fabs(q1_3_p0_3)) )
    {
        max4 = fabs(q1_3_p0_3);
    } 
    if( (max4 < fabs(q1_4_p0_4)) )
    {
        max4 = fabs(q1_4_p0_4);
    } 
    if( (max4 < fabs(q1_5_p0_5)) )
    {
        max4 = fabs(q1_5_p0_5);
    } 
    if( (max4 < fabs(q1_6_p0_6)) )
    {
        max4 = fabs(q1_6_p0_6);
    } 
    double max5 = fabs(q1_0_p0_0);
    if( (max5 < fabs(q1_1_p0_1)) )
    {
        max5 = fabs(q1_1_p0_1);
    } 
    if( (max5 < fabs(q1_2_p0_2)) )
    {
        max5 = fabs(q1_2_p0_2);
    } 
    if( (max5 < fabs(q1_3_p0_3)) )
    {
        max5 = fabs(q1_3_p0_3);
    } 
    if( (max5 < fabs(q1_4_p0_4)) )
    {
        max5 = fabs(q1_4_p0_4);
    } 
    if( (max5 < fabs(q1_5_p0_5)) )
    {
        max5 = fabs(q1_5_p0_5);
    } 
    if( (max5 < fabs(q1_6_p0_6)) )
    {
        max5 = fabs(q1_6_p0_6);
    } 
    if( (max5 < fabs(q2_0_p0_0)) )
    {
        max5 = fabs(q2_0_p0_0);
    } 
    if( (max5 < fabs(q2_1_p0_1)) )
    {
        max5 = fabs(q2_1_p0_1);
    } 
    if( (max5 < fabs(q2_2_p0_2)) )
    {
        max5 = fabs(q2_2_p0_2);
    } 
    if( (max5 < fabs(q2_3_p0_3)) )
    {
        max5 = fabs(q2_3_p0_3);
    } 
    if( (max5 < fabs(q2_4_p0_4)) )
    {
        max5 = fabs(q2_4_p0_4);
    } 
    if( (max5 < fabs(q2_5_p0_5)) )
    {
        max5 = fabs(q2_5_p0_5);
    } 
    if( (max5 < fabs(q2_6_p0_6)) )
    {
        max5 = fabs(q2_6_p0_6);
    } 
    double max6 = fabs(q2_0_p0_0);
    if( (max6 < fabs(q2_1_p0_1)) )
    {
        max6 = fabs(q2_1_p0_1);
    } 
    if( (max6 < fabs(q2_2_p0_2)) )
    {
        max6 = fabs(q2_2_p0_2);
    } 
    if( (max6 < fabs(q2_3_p0_3)) )
    {
        max6 = fabs(q2_3_p0_3);
    } 
    if( (max6 < fabs(q2_4_p0_4)) )
    {
        max6 = fabs(q2_4_p0_4);
    } 
    if( (max6 < fabs(q2_5_p0_5)) )
    {
        max6 = fabs(q2_5_p0_5);
    } 
    if( (max6 < fabs(q2_6_p0_6)) )
    {
        max6 = fabs(q2_6_p0_6);
    } 
    if( (max6 < fabs(q3_0_p0_0)) )
    {
        max6 = fabs(q3_0_p0_0);
    } 
    if( (max6 < fabs(q3_1_p0_1)) )
    {
        max6 = fabs(q3_1_p0_1);
    } 
    if( (max6 < fabs(q3_2_p0_2)) )
    {
        max6 = fabs(q3_2_p0_2);
    } 
    if( (max6 < fabs(q3_3_p0_3)) )
    {
        max6 = fabs(q3_3_p0_3);
    } 
    if( (max6 < fabs(q3_4_p0_4)) )
    {
        max6 = fabs(q3_4_p0_4);
    } 
    if( (max6 < fabs(q3_5_p0_5)) )
    {
        max6 = fabs(q3_5_p0_5);
    } 
    if( (max6 < fabs(q3_6_p0_6)) )
    {
        max6 = fabs(q3_6_p0_6);
    } 
    double lower_bound_1;
    double upper_bound_1;
    int Delta_sign;
    int int_tmp_result;
    lower_bound_1 = max3;
    upper_bound_1 = max3;
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
    else 
    {
        if( (max2 > upper_bound_1) )
        {
            upper_bound_1 = max2;
        } 
    } 
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
    if( (max6 < lower_bound_1) )
    {
        lower_bound_1 = max6;
    } 
    else 
    {
        if( (max6 > upper_bound_1) )
        {
            upper_bound_1 = max6;
        } 
    } 
    if( (lower_bound_1 < 3.04548303565602498901e-50) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 4.83570327845851562508e+24) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (2.78873548804336160566e-11 * (((((max1 * max4) * max3) * max5) * max2) * max6));
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
    Delta_sign = int_tmp_result;
    int int_tmp_result_FFWKCAA;
    double max7 = max3;
    if( (max7 < max1) )
    {
        max7 = max1;
    } 
    if( (max7 < max2) )
    {
        max7 = max2;
    } 
    if( (max7 < max6) )
    {
        max7 = max6;
    } 
    double max8 = max1;
    if( (max8 < fabs(p4_4_p0_4)) )
    {
        max8 = fabs(p4_4_p0_4);
    } 
    if( (max8 < fabs(p4_1_p0_1)) )
    {
        max8 = fabs(p4_1_p0_1);
    } 
    if( (max8 < fabs(p4_5_p0_5)) )
    {
        max8 = fabs(p4_5_p0_5);
    } 
    if( (max8 < fabs(p4_0_p0_0)) )
    {
        max8 = fabs(p4_0_p0_0);
    } 
    if( (max8 < fabs(p4_2_p0_2)) )
    {
        max8 = fabs(p4_2_p0_2);
    } 
    if( (max8 < fabs(p4_3_p0_3)) )
    {
        max8 = fabs(p4_3_p0_3);
    } 
    if( (max8 < fabs(p4_6_p0_6)) )
    {
        max8 = fabs(p4_6_p0_6);
    } 
    if( (max7 < max8) )
    {
        max7 = max8;
    } 
    double max9 = max1;
    if( (max9 < max5) )
    {
        max9 = max5;
    } 
    if( (max9 < max8) )
    {
        max9 = max8;
    } 
    double max10 = max4;
    if( (max10 < max1) )
    {
        max10 = max1;
    } 
    if( (max10 < max5) )
    {
        max10 = max5;
    } 
    if( (max10 < max6) )
    {
        max10 = max6;
    } 
    double max11 = max4;
    if( (max11 < max5) )
    {
        max11 = max5;
    } 
    if( (max10 < max11) )
    {
        max10 = max11;
    } 
    lower_bound_1 = max3;
    upper_bound_1 = max3;
    if( (max2 < lower_bound_1) )
    {
        lower_bound_1 = max2;
    } 
    if( (max7 < lower_bound_1) )
    {
        lower_bound_1 = max7;
    } 
    else 
    {
        if( (max7 > upper_bound_1) )
        {
            upper_bound_1 = max7;
        } 
    } 
    if( (max8 < lower_bound_1) )
    {
        lower_bound_1 = max8;
    } 
    if( (max9 < lower_bound_1) )
    {
        lower_bound_1 = max9;
    } 
    else 
    {
        if( (max9 > upper_bound_1) )
        {
            upper_bound_1 = max9;
        } 
    } 
    if( (max10 < lower_bound_1) )
    {
        lower_bound_1 = max10;
    } 
    else 
    {
        if( (max10 > upper_bound_1) )
        {
            upper_bound_1 = max10;
        } 
    } 
    if( (max11 < lower_bound_1) )
    {
        lower_bound_1 = max11;
    } 
    else 
    {
        if( (max11 > upper_bound_1) )
        {
            upper_bound_1 = max11;
        } 
    } 
    if( (lower_bound_1 < 4.48906690519700369396e-38) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 4.83570327845851562508e+24) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (1.34926049830188433875e-09 * (((((((max8 * max11) * max3) * max10) * max2) * max10) * max9) * max7));
        if( (r > eps) )
        {
            int_tmp_result_FFWKCAA = 1;
        } 
        else 
        {
            if( (r < -eps) )
            {
                int_tmp_result_FFWKCAA = -1;
            } 
            else 
            {
                return FPG_UNCERTAIN_VALUE;
            } 
        } 
    } 
    return (Delta_sign * int_tmp_result_FFWKCAA);
} 


inline int side4_8d_filter( const double* p0, const double* p1, const double* p2, const double* p3, const double* p4, const double* q0, const double* q1, const double* q2, const double* q3) {
    double p1_0_p0_0 = (p1[0] - p0[0]);
    double p1_1_p0_1 = (p1[1] - p0[1]);
    double p1_2_p0_2 = (p1[2] - p0[2]);
    double p1_3_p0_3 = (p1[3] - p0[3]);
    double p1_4_p0_4 = (p1[4] - p0[4]);
    double p1_5_p0_5 = (p1[5] - p0[5]);
    double p1_6_p0_6 = (p1[6] - p0[6]);
    double p1_7_p0_7 = (p1[7] - p0[7]);
    double l1;
    l1 = (1 * ((((((((p1_0_p0_0 * p1_0_p0_0) + (p1_1_p0_1 * p1_1_p0_1)) + (p1_2_p0_2 * p1_2_p0_2)) + (p1_3_p0_3 * p1_3_p0_3)) + (p1_4_p0_4 * p1_4_p0_4)) + (p1_5_p0_5 * p1_5_p0_5)) + (p1_6_p0_6 * p1_6_p0_6)) + (p1_7_p0_7 * p1_7_p0_7)));
    double p2_0_p0_0 = (p2[0] - p0[0]);
    double p2_1_p0_1 = (p2[1] - p0[1]);
    double p2_2_p0_2 = (p2[2] - p0[2]);
    double p2_3_p0_3 = (p2[3] - p0[3]);
    double p2_4_p0_4 = (p2[4] - p0[4]);
    double p2_5_p0_5 = (p2[5] - p0[5]);
    double p2_6_p0_6 = (p2[6] - p0[6]);
    double p2_7_p0_7 = (p2[7] - p0[7]);
    double l2;
    l2 = (1 * ((((((((p2_0_p0_0 * p2_0_p0_0) + (p2_1_p0_1 * p2_1_p0_1)) + (p2_2_p0_2 * p2_2_p0_2)) + (p2_3_p0_3 * p2_3_p0_3)) + (p2_4_p0_4 * p2_4_p0_4)) + (p2_5_p0_5 * p2_5_p0_5)) + (p2_6_p0_6 * p2_6_p0_6)) + (p2_7_p0_7 * p2_7_p0_7)));
    double p3_0_p0_0 = (p3[0] - p0[0]);
    double p3_1_p0_1 = (p3[1] - p0[1]);
    double p3_2_p0_2 = (p3[2] - p0[2]);
    double p3_3_p0_3 = (p3[3] - p0[3]);
    double p3_4_p0_4 = (p3[4] - p0[4]);
    double p3_5_p0_5 = (p3[5] - p0[5]);
    double p3_6_p0_6 = (p3[6] - p0[6]);
    double p3_7_p0_7 = (p3[7] - p0[7]);
    double l3;
    l3 = (1 * ((((((((p3_0_p0_0 * p3_0_p0_0) + (p3_1_p0_1 * p3_1_p0_1)) + (p3_2_p0_2 * p3_2_p0_2)) + (p3_3_p0_3 * p3_3_p0_3)) + (p3_4_p0_4 * p3_4_p0_4)) + (p3_5_p0_5 * p3_5_p0_5)) + (p3_6_p0_6 * p3_6_p0_6)) + (p3_7_p0_7 * p3_7_p0_7)));
    double p4_0_p0_0 = (p4[0] - p0[0]);
    double p4_1_p0_1 = (p4[1] - p0[1]);
    double p4_2_p0_2 = (p4[2] - p0[2]);
    double p4_3_p0_3 = (p4[3] - p0[3]);
    double p4_4_p0_4 = (p4[4] - p0[4]);
    double p4_5_p0_5 = (p4[5] - p0[5]);
    double p4_6_p0_6 = (p4[6] - p0[6]);
    double p4_7_p0_7 = (p4[7] - p0[7]);
    double l4;
    l4 = (1 * ((((((((p4_0_p0_0 * p4_0_p0_0) + (p4_1_p0_1 * p4_1_p0_1)) + (p4_2_p0_2 * p4_2_p0_2)) + (p4_3_p0_3 * p4_3_p0_3)) + (p4_4_p0_4 * p4_4_p0_4)) + (p4_5_p0_5 * p4_5_p0_5)) + (p4_6_p0_6 * p4_6_p0_6)) + (p4_7_p0_7 * p4_7_p0_7)));
    double q0_0_p0_0 = (q0[0] - p0[0]);
    double q0_1_p0_1 = (q0[1] - p0[1]);
    double q0_2_p0_2 = (q0[2] - p0[2]);
    double q0_3_p0_3 = (q0[3] - p0[3]);
    double q0_4_p0_4 = (q0[4] - p0[4]);
    double q0_5_p0_5 = (q0[5] - p0[5]);
    double q0_6_p0_6 = (q0[6] - p0[6]);
    double q0_7_p0_7 = (q0[7] - p0[7]);
    double a10;
    a10 = (2 * ((((((((p1_0_p0_0 * q0_0_p0_0) + (p1_1_p0_1 * q0_1_p0_1)) + (p1_2_p0_2 * q0_2_p0_2)) + (p1_3_p0_3 * q0_3_p0_3)) + (p1_4_p0_4 * q0_4_p0_4)) + (p1_5_p0_5 * q0_5_p0_5)) + (p1_6_p0_6 * q0_6_p0_6)) + (p1_7_p0_7 * q0_7_p0_7)));
    double q1_0_p0_0 = (q1[0] - p0[0]);
    double q1_1_p0_1 = (q1[1] - p0[1]);
    double q1_2_p0_2 = (q1[2] - p0[2]);
    double q1_3_p0_3 = (q1[3] - p0[3]);
    double q1_4_p0_4 = (q1[4] - p0[4]);
    double q1_5_p0_5 = (q1[5] - p0[5]);
    double q1_6_p0_6 = (q1[6] - p0[6]);
    double q1_7_p0_7 = (q1[7] - p0[7]);
    double a11;
    a11 = (2 * ((((((((p1_0_p0_0 * q1_0_p0_0) + (p1_1_p0_1 * q1_1_p0_1)) + (p1_2_p0_2 * q1_2_p0_2)) + (p1_3_p0_3 * q1_3_p0_3)) + (p1_4_p0_4 * q1_4_p0_4)) + (p1_5_p0_5 * q1_5_p0_5)) + (p1_6_p0_6 * q1_6_p0_6)) + (p1_7_p0_7 * q1_7_p0_7)));
    double q2_0_p0_0 = (q2[0] - p0[0]);
    double q2_1_p0_1 = (q2[1] - p0[1]);
    double q2_2_p0_2 = (q2[2] - p0[2]);
    double q2_3_p0_3 = (q2[3] - p0[3]);
    double q2_4_p0_4 = (q2[4] - p0[4]);
    double q2_5_p0_5 = (q2[5] - p0[5]);
    double q2_6_p0_6 = (q2[6] - p0[6]);
    double q2_7_p0_7 = (q2[7] - p0[7]);
    double a12;
    a12 = (2 * ((((((((p1_0_p0_0 * q2_0_p0_0) + (p1_1_p0_1 * q2_1_p0_1)) + (p1_2_p0_2 * q2_2_p0_2)) + (p1_3_p0_3 * q2_3_p0_3)) + (p1_4_p0_4 * q2_4_p0_4)) + (p1_5_p0_5 * q2_5_p0_5)) + (p1_6_p0_6 * q2_6_p0_6)) + (p1_7_p0_7 * q2_7_p0_7)));
    double q3_0_p0_0 = (q3[0] - p0[0]);
    double q3_1_p0_1 = (q3[1] - p0[1]);
    double q3_2_p0_2 = (q3[2] - p0[2]);
    double q3_3_p0_3 = (q3[3] - p0[3]);
    double q3_4_p0_4 = (q3[4] - p0[4]);
    double q3_5_p0_5 = (q3[5] - p0[5]);
    double q3_6_p0_6 = (q3[6] - p0[6]);
    double q3_7_p0_7 = (q3[7] - p0[7]);
    double a13;
    a13 = (2 * ((((((((p1_0_p0_0 * q3_0_p0_0) + (p1_1_p0_1 * q3_1_p0_1)) + (p1_2_p0_2 * q3_2_p0_2)) + (p1_3_p0_3 * q3_3_p0_3)) + (p1_4_p0_4 * q3_4_p0_4)) + (p1_5_p0_5 * q3_5_p0_5)) + (p1_6_p0_6 * q3_6_p0_6)) + (p1_7_p0_7 * q3_7_p0_7)));
    double a20;
    a20 = (2 * ((((((((p2_0_p0_0 * q0_0_p0_0) + (p2_1_p0_1 * q0_1_p0_1)) + (p2_2_p0_2 * q0_2_p0_2)) + (p2_3_p0_3 * q0_3_p0_3)) + (p2_4_p0_4 * q0_4_p0_4)) + (p2_5_p0_5 * q0_5_p0_5)) + (p2_6_p0_6 * q0_6_p0_6)) + (p2_7_p0_7 * q0_7_p0_7)));
    double a21;
    a21 = (2 * ((((((((p2_0_p0_0 * q1_0_p0_0) + (p2_1_p0_1 * q1_1_p0_1)) + (p2_2_p0_2 * q1_2_p0_2)) + (p2_3_p0_3 * q1_3_p0_3)) + (p2_4_p0_4 * q1_4_p0_4)) + (p2_5_p0_5 * q1_5_p0_5)) + (p2_6_p0_6 * q1_6_p0_6)) + (p2_7_p0_7 * q1_7_p0_7)));
    double a22;
    a22 = (2 * ((((((((p2_0_p0_0 * q2_0_p0_0) + (p2_1_p0_1 * q2_1_p0_1)) + (p2_2_p0_2 * q2_2_p0_2)) + (p2_3_p0_3 * q2_3_p0_3)) + (p2_4_p0_4 * q2_4_p0_4)) + (p2_5_p0_5 * q2_5_p0_5)) + (p2_6_p0_6 * q2_6_p0_6)) + (p2_7_p0_7 * q2_7_p0_7)));
    double a23;
    a23 = (2 * ((((((((p2_0_p0_0 * q3_0_p0_0) + (p2_1_p0_1 * q3_1_p0_1)) + (p2_2_p0_2 * q3_2_p0_2)) + (p2_3_p0_3 * q3_3_p0_3)) + (p2_4_p0_4 * q3_4_p0_4)) + (p2_5_p0_5 * q3_5_p0_5)) + (p2_6_p0_6 * q3_6_p0_6)) + (p2_7_p0_7 * q3_7_p0_7)));
    double a30;
    a30 = (2 * ((((((((p3_0_p0_0 * q0_0_p0_0) + (p3_1_p0_1 * q0_1_p0_1)) + (p3_2_p0_2 * q0_2_p0_2)) + (p3_3_p0_3 * q0_3_p0_3)) + (p3_4_p0_4 * q0_4_p0_4)) + (p3_5_p0_5 * q0_5_p0_5)) + (p3_6_p0_6 * q0_6_p0_6)) + (p3_7_p0_7 * q0_7_p0_7)));
    double a31;
    a31 = (2 * ((((((((p3_0_p0_0 * q1_0_p0_0) + (p3_1_p0_1 * q1_1_p0_1)) + (p3_2_p0_2 * q1_2_p0_2)) + (p3_3_p0_3 * q1_3_p0_3)) + (p3_4_p0_4 * q1_4_p0_4)) + (p3_5_p0_5 * q1_5_p0_5)) + (p3_6_p0_6 * q1_6_p0_6)) + (p3_7_p0_7 * q1_7_p0_7)));
    double a32;
    a32 = (2 * ((((((((p3_0_p0_0 * q2_0_p0_0) + (p3_1_p0_1 * q2_1_p0_1)) + (p3_2_p0_2 * q2_2_p0_2)) + (p3_3_p0_3 * q2_3_p0_3)) + (p3_4_p0_4 * q2_4_p0_4)) + (p3_5_p0_5 * q2_5_p0_5)) + (p3_6_p0_6 * q2_6_p0_6)) + (p3_7_p0_7 * q2_7_p0_7)));
    double a33;
    a33 = (2 * ((((((((p3_0_p0_0 * q3_0_p0_0) + (p3_1_p0_1 * q3_1_p0_1)) + (p3_2_p0_2 * q3_2_p0_2)) + (p3_3_p0_3 * q3_3_p0_3)) + (p3_4_p0_4 * q3_4_p0_4)) + (p3_5_p0_5 * q3_5_p0_5)) + (p3_6_p0_6 * q3_6_p0_6)) + (p3_7_p0_7 * q3_7_p0_7)));
    double a40;
    a40 = (2 * ((((((((p4_0_p0_0 * q0_0_p0_0) + (p4_1_p0_1 * q0_1_p0_1)) + (p4_2_p0_2 * q0_2_p0_2)) + (p4_3_p0_3 * q0_3_p0_3)) + (p4_4_p0_4 * q0_4_p0_4)) + (p4_5_p0_5 * q0_5_p0_5)) + (p4_6_p0_6 * q0_6_p0_6)) + (p4_7_p0_7 * q0_7_p0_7)));
    double a41;
    a41 = (2 * ((((((((p4_0_p0_0 * q1_0_p0_0) + (p4_1_p0_1 * q1_1_p0_1)) + (p4_2_p0_2 * q1_2_p0_2)) + (p4_3_p0_3 * q1_3_p0_3)) + (p4_4_p0_4 * q1_4_p0_4)) + (p4_5_p0_5 * q1_5_p0_5)) + (p4_6_p0_6 * q1_6_p0_6)) + (p4_7_p0_7 * q1_7_p0_7)));
    double a42;
    a42 = (2 * ((((((((p4_0_p0_0 * q2_0_p0_0) + (p4_1_p0_1 * q2_1_p0_1)) + (p4_2_p0_2 * q2_2_p0_2)) + (p4_3_p0_3 * q2_3_p0_3)) + (p4_4_p0_4 * q2_4_p0_4)) + (p4_5_p0_5 * q2_5_p0_5)) + (p4_6_p0_6 * q2_6_p0_6)) + (p4_7_p0_7 * q2_7_p0_7)));
    double a43;
    a43 = (2 * ((((((((p4_0_p0_0 * q3_0_p0_0) + (p4_1_p0_1 * q3_1_p0_1)) + (p4_2_p0_2 * q3_2_p0_2)) + (p4_3_p0_3 * q3_3_p0_3)) + (p4_4_p0_4 * q3_4_p0_4)) + (p4_5_p0_5 * q3_5_p0_5)) + (p4_6_p0_6 * q3_6_p0_6)) + (p4_7_p0_7 * q3_7_p0_7)));
    double b00;
    b00 = (((a11 * ((a22 * a33) - (a23 * a32))) - (a21 * ((a12 * a33) - (a13 * a32)))) + (a31 * ((a12 * a23) - (a13 * a22))));
    double b01;
    b01 = -((((a22 * a33) - (a23 * a32)) + ((a23 * a31) - (a21 * a33))) + ((a21 * a32) - (a22 * a31)));
    double b02;
    b02 = ((((a12 * a33) - (a13 * a32)) + ((a13 * a31) - (a11 * a33))) + ((a11 * a32) - (a12 * a31)));
    double b03;
    b03 = -((((a12 * a23) - (a13 * a22)) + ((a13 * a21) - (a11 * a23))) + ((a11 * a22) - (a12 * a21)));
    double b10;
    b10 = -(((a10 * ((a22 * a33) - (a23 * a32))) - (a20 * ((a12 * a33) - (a13 * a32)))) + (a30 * ((a12 * a23) - (a13 * a22))));
    double b11;
    b11 = ((((a22 * a33) - (a23 * a32)) + ((a23 * a30) - (a20 * a33))) + ((a20 * a32) - (a22 * a30)));
    double b12;
    b12 = -((((a12 * a33) - (a13 * a32)) + ((a13 * a30) - (a10 * a33))) + ((a10 * a32) - (a12 * a30)));
    double b13;
    b13 = ((((a12 * a23) - (a13 * a22)) + ((a13 * a20) - (a10 * a23))) + ((a10 * a22) - (a12 * a20)));
    double b20;
    b20 = (((a10 * ((a21 * a33) - (a23 * a31))) - (a20 * ((a11 * a33) - (a13 * a31)))) + (a30 * ((a11 * a23) - (a13 * a21))));
    double b21;
    b21 = -((((a21 * a33) - (a23 * a31)) + ((a23 * a30) - (a20 * a33))) + ((a20 * a31) - (a21 * a30)));
    double b22;
    b22 = ((((a11 * a33) - (a13 * a31)) + ((a13 * a30) - (a10 * a33))) + ((a10 * a31) - (a11 * a30)));
    double b23;
    b23 = -((((a11 * a23) - (a13 * a21)) + ((a13 * a20) - (a10 * a23))) + ((a10 * a21) - (a11 * a20)));
    double b30;
    b30 = -(((a10 * ((a21 * a32) - (a22 * a31))) - (a20 * ((a11 * a32) - (a12 * a31)))) + (a30 * ((a11 * a22) - (a12 * a21))));
    double b31;
    b31 = ((((a21 * a32) - (a22 * a31)) + ((a22 * a30) - (a20 * a32))) + ((a20 * a31) - (a21 * a30)));
    double b32;
    b32 = -((((a11 * a32) - (a12 * a31)) + ((a12 * a30) - (a10 * a32))) + ((a10 * a31) - (a11 * a30)));
    double b33;
    b33 = ((((a11 * a22) - (a12 * a21)) + ((a12 * a20) - (a10 * a22))) + ((a10 * a21) - (a11 * a20)));
    double Delta;
    Delta = (((b00 + b10) + b20) + b30);
    double DeltaLambda0;
    DeltaLambda0 = ((((b01 * l1) + (b02 * l2)) + (b03 * l3)) + b00);
    double DeltaLambda1;
    DeltaLambda1 = ((((b11 * l1) + (b12 * l2)) + (b13 * l3)) + b10);
    double DeltaLambda2;
    DeltaLambda2 = ((((b21 * l1) + (b22 * l2)) + (b23 * l3)) + b20);
    double DeltaLambda3;
    DeltaLambda3 = ((((b31 * l1) + (b32 * l2)) + (b33 * l3)) + b30);
    double r;
    r = ((Delta * l4) - ((((a40 * DeltaLambda0) + (a41 * DeltaLambda1)) + (a42 * DeltaLambda2)) + (a43 * DeltaLambda3)));
    double eps;
    double max1 = fabs(p2_5_p0_5);
    if( (max1 < fabs(p2_3_p0_3)) )
    {
        max1 = fabs(p2_3_p0_3);
    } 
    if( (max1 < fabs(p2_0_p0_0)) )
    {
        max1 = fabs(p2_0_p0_0);
    } 
    if( (max1 < fabs(p2_1_p0_1)) )
    {
        max1 = fabs(p2_1_p0_1);
    } 
    if( (max1 < fabs(p2_6_p0_6)) )
    {
        max1 = fabs(p2_6_p0_6);
    } 
    if( (max1 < fabs(p2_2_p0_2)) )
    {
        max1 = fabs(p2_2_p0_2);
    } 
    if( (max1 < fabs(p2_4_p0_4)) )
    {
        max1 = fabs(p2_4_p0_4);
    } 
    if( (max1 < fabs(p2_7_p0_7)) )
    {
        max1 = fabs(p2_7_p0_7);
    } 
    double max2 = fabs(p1_4_p0_4);
    if( (max2 < fabs(p1_3_p0_3)) )
    {
        max2 = fabs(p1_3_p0_3);
    } 
    if( (max2 < fabs(p1_7_p0_7)) )
    {
        max2 = fabs(p1_7_p0_7);
    } 
    if( (max2 < fabs(p1_0_p0_0)) )
    {
        max2 = fabs(p1_0_p0_0);
    } 
    if( (max2 < fabs(p1_2_p0_2)) )
    {
        max2 = fabs(p1_2_p0_2);
    } 
    if( (max2 < fabs(p1_5_p0_5)) )
    {
        max2 = fabs(p1_5_p0_5);
    } 
    if( (max2 < fabs(p1_1_p0_1)) )
    {
        max2 = fabs(p1_1_p0_1);
    } 
    if( (max2 < fabs(p1_6_p0_6)) )
    {
        max2 = fabs(p1_6_p0_6);
    } 
    double max3 = fabs(p3_3_p0_3);
    if( (max3 < fabs(p3_0_p0_0)) )
    {
        max3 = fabs(p3_0_p0_0);
    } 
    if( (max3 < fabs(p3_1_p0_1)) )
    {
        max3 = fabs(p3_1_p0_1);
    } 
    if( (max3 < fabs(p3_2_p0_2)) )
    {
        max3 = fabs(p3_2_p0_2);
    } 
    if( (max3 < fabs(p3_4_p0_4)) )
    {
        max3 = fabs(p3_4_p0_4);
    } 
    if( (max3 < fabs(p3_5_p0_5)) )
    {
        max3 = fabs(p3_5_p0_5);
    } 
    if( (max3 < fabs(p3_6_p0_6)) )
    {
        max3 = fabs(p3_6_p0_6);
    } 
    if( (max3 < fabs(p3_7_p0_7)) )
    {
        max3 = fabs(p3_7_p0_7);
    } 
    double max4 = fabs(q0_0_p0_0);
    if( (max4 < fabs(q0_1_p0_1)) )
    {
        max4 = fabs(q0_1_p0_1);
    } 
    if( (max4 < fabs(q0_2_p0_2)) )
    {
        max4 = fabs(q0_2_p0_2);
    } 
    if( (max4 < fabs(q0_3_p0_3)) )
    {
        max4 = fabs(q0_3_p0_3);
    } 
    if( (max4 < fabs(q0_4_p0_4)) )
    {
        max4 = fabs(q0_4_p0_4);
    } 
    if( (max4 < fabs(q0_5_p0_5)) )
    {
        max4 = fabs(q0_5_p0_5);
    } 
    if( (max4 < fabs(q0_6_p0_6)) )
    {
        max4 = fabs(q0_6_p0_6);
    } 
    if( (max4 < fabs(q0_7_p0_7)) )
    {
        max4 = fabs(q0_7_p0_7);
    } 
    if( (max4 < fabs(q1_0_p0_0)) )
    {
        max4 = fabs(q1_0_p0_0);
    } 
    if( (max4 < fabs(q1_1_p0_1)) )
    {
        max4 = fabs(q1_1_p0_1);
    } 
    if( (max4 < fabs(q1_2_p0_2)) )
    {
        max4 = fabs(q1_2_p0_2);
    } 
    if( (max4 < fabs(q1_3_p0_3)) )
    {
        max4 = fabs(q1_3_p0_3);
    } 
    if( (max4 < fabs(q1_4_p0_4)) )
    {
        max4 = fabs(q1_4_p0_4);
    } 
    if( (max4 < fabs(q1_5_p0_5)) )
    {
        max4 = fabs(q1_5_p0_5);
    } 
    if( (max4 < fabs(q1_6_p0_6)) )
    {
        max4 = fabs(q1_6_p0_6);
    } 
    if( (max4 < fabs(q1_7_p0_7)) )
    {
        max4 = fabs(q1_7_p0_7);
    } 
    double max5 = fabs(q1_0_p0_0);
    if( (max5 < fabs(q1_1_p0_1)) )
    {
        max5 = fabs(q1_1_p0_1);
    } 
    if( (max5 < fabs(q1_2_p0_2)) )
    {
        max5 = fabs(q1_2_p0_2);
    } 
    if( (max5 < fabs(q1_3_p0_3)) )
    {
        max5 = fabs(q1_3_p0_3);
    } 
    if( (max5 < fabs(q1_4_p0_4)) )
    {
        max5 = fabs(q1_4_p0_4);
    } 
    if( (max5 < fabs(q1_5_p0_5)) )
    {
        max5 = fabs(q1_5_p0_5);
    } 
    if( (max5 < fabs(q1_6_p0_6)) )
    {
        max5 = fabs(q1_6_p0_6);
    } 
    if( (max5 < fabs(q1_7_p0_7)) )
    {
        max5 = fabs(q1_7_p0_7);
    } 
    if( (max5 < fabs(q2_0_p0_0)) )
    {
        max5 = fabs(q2_0_p0_0);
    } 
    if( (max5 < fabs(q2_1_p0_1)) )
    {
        max5 = fabs(q2_1_p0_1);
    } 
    if( (max5 < fabs(q2_2_p0_2)) )
    {
        max5 = fabs(q2_2_p0_2);
    } 
    if( (max5 < fabs(q2_3_p0_3)) )
    {
        max5 = fabs(q2_3_p0_3);
    } 
    if( (max5 < fabs(q2_4_p0_4)) )
    {
        max5 = fabs(q2_4_p0_4);
    } 
    if( (max5 < fabs(q2_5_p0_5)) )
    {
        max5 = fabs(q2_5_p0_5);
    } 
    if( (max5 < fabs(q2_6_p0_6)) )
    {
        max5 = fabs(q2_6_p0_6);
    } 
    if( (max5 < fabs(q2_7_p0_7)) )
    {
        max5 = fabs(q2_7_p0_7);
    } 
    double max6 = fabs(q2_0_p0_0);
    if( (max6 < fabs(q2_1_p0_1)) )
    {
        max6 = fabs(q2_1_p0_1);
    } 
    if( (max6 < fabs(q2_2_p0_2)) )
    {
        max6 = fabs(q2_2_p0_2);
    } 
    if( (max6 < fabs(q2_3_p0_3)) )
    {
        max6 = fabs(q2_3_p0_3);
    } 
    if( (max6 < fabs(q2_4_p0_4)) )
    {
        max6 = fabs(q2_4_p0_4);
    } 
    if( (max6 < fabs(q2_5_p0_5)) )
    {
        max6 = fabs(q2_5_p0_5);
    } 
    if( (max6 < fabs(q2_6_p0_6)) )
    {
        max6 = fabs(q2_6_p0_6);
    } 
    if( (max6 < fabs(q2_7_p0_7)) )
    {
        max6 = fabs(q2_7_p0_7);
    } 
    if( (max6 < fabs(q3_0_p0_0)) )
    {
        max6 = fabs(q3_0_p0_0);
    } 
    if( (max6 < fabs(q3_1_p0_1)) )
    {
        max6 = fabs(q3_1_p0_1);
    } 
    if( (max6 < fabs(q3_2_p0_2)) )
    {
        max6 = fabs(q3_2_p0_2);
    } 
    if( (max6 < fabs(q3_3_p0_3)) )
    {
        max6 = fabs(q3_3_p0_3);
    } 
    if( (max6 < fabs(q3_4_p0_4)) )
    {
        max6 = fabs(q3_4_p0_4);
    } 
    if( (max6 < fabs(q3_5_p0_5)) )
    {
        max6 = fabs(q3_5_p0_5);
    } 
    if( (max6 < fabs(q3_6_p0_6)) )
    {
        max6 = fabs(q3_6_p0_6);
    } 
    if( (max6 < fabs(q3_7_p0_7)) )
    {
        max6 = fabs(q3_7_p0_7);
    } 
    double lower_bound_1;
    double upper_bound_1;
    int Delta_sign;
    int int_tmp_result;
    lower_bound_1 = max4;
    upper_bound_1 = max4;
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
    if( (max3 < lower_bound_1) )
    {
        lower_bound_1 = max3;
    } 
    else 
    {
        if( (max3 > upper_bound_1) )
        {
            upper_bound_1 = max3;
        } 
    } 
    if( (max2 < lower_bound_1) )
    {
        lower_bound_1 = max2;
    } 
    else 
    {
        if( (max2 > upper_bound_1) )
        {
            upper_bound_1 = max2;
        } 
    } 
    if( (max6 < lower_bound_1) )
    {
        lower_bound_1 = max6;
    } 
    else 
    {
        if( (max6 > upper_bound_1) )
        {
            upper_bound_1 = max6;
        } 
    } 
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
    if( (lower_bound_1 < 2.82528483194754087282e-50) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 4.83570327845851562508e+24) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (4.37492894694731040560e-11 * (((((max2 * max4) * max1) * max5) * max3) * max6));
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
    Delta_sign = int_tmp_result;
    int int_tmp_result_FFWKCAA;
    double max7 = max1;
    if( (max7 < max3) )
    {
        max7 = max3;
    } 
    double max8 = max2;
    if( (max8 < fabs(p4_3_p0_3)) )
    {
        max8 = fabs(p4_3_p0_3);
    } 
    if( (max8 < fabs(p4_1_p0_1)) )
    {
        max8 = fabs(p4_1_p0_1);
    } 
    if( (max8 < fabs(p4_4_p0_4)) )
    {
        max8 = fabs(p4_4_p0_4);
    } 
    if( (max8 < fabs(p4_0_p0_0)) )
    {
        max8 = fabs(p4_0_p0_0);
    } 
    if( (max8 < fabs(p4_2_p0_2)) )
    {
        max8 = fabs(p4_2_p0_2);
    } 
    if( (max8 < fabs(p4_5_p0_5)) )
    {
        max8 = fabs(p4_5_p0_5);
    } 
    if( (max8 < fabs(p4_6_p0_6)) )
    {
        max8 = fabs(p4_6_p0_6);
    } 
    if( (max8 < fabs(p4_7_p0_7)) )
    {
        max8 = fabs(p4_7_p0_7);
    } 
    if( (max7 < max8) )
    {
        max7 = max8;
    } 
    if( (max7 < max2) )
    {
        max7 = max2;
    } 
    if( (max7 < max6) )
    {
        max7 = max6;
    } 
    double max9 = max8;
    if( (max9 < max2) )
    {
        max9 = max2;
    } 
    if( (max9 < max5) )
    {
        max9 = max5;
    } 
    double max10 = max4;
    double max11 = max4;
    if( (max11 < max5) )
    {
        max11 = max5;
    } 
    if( (max10 < max11) )
    {
        max10 = max11;
    } 
    if( (max10 < max2) )
    {
        max10 = max2;
    } 
    if( (max10 < max6) )
    {
        max10 = max6;
    } 
    if( (max10 < max5) )
    {
        max10 = max5;
    } 
    lower_bound_1 = max11;
    upper_bound_1 = max11;
    if( (max9 < lower_bound_1) )
    {
        lower_bound_1 = max9;
    } 
    else 
    {
        if( (max9 > upper_bound_1) )
        {
            upper_bound_1 = max9;
        } 
    } 
    if( (max7 < lower_bound_1) )
    {
        lower_bound_1 = max7;
    } 
    else 
    {
        if( (max7 > upper_bound_1) )
        {
            upper_bound_1 = max7;
        } 
    } 
    if( (max1 < lower_bound_1) )
    {
        lower_bound_1 = max1;
    } 
    if( (max3 < lower_bound_1) )
    {
        lower_bound_1 = max3;
    } 
    if( (max8 < lower_bound_1) )
    {
        lower_bound_1 = max8;
    } 
    if( (max10 < lower_bound_1) )
    {
        lower_bound_1 = max10;
    } 
    else 
    {
        if( (max10 > upper_bound_1) )
        {
            upper_bound_1 = max10;
        } 
    } 
    if( (lower_bound_1 < 4.17402518597284772324e-38) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 4.83570327845851562508e+24) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (2.41492645607254025015e-09 * (((((((max8 * max11) * max1) * max10) * max3) * max10) * max9) * max7));
        if( (r > eps) )
        {
            int_tmp_result_FFWKCAA = 1;
        } 
        else 
        {
            if( (r < -eps) )
            {
                int_tmp_result_FFWKCAA = -1;
            } 
            else 
            {
                return FPG_UNCERTAIN_VALUE;
            } 
        } 
    } 
    return (Delta_sign * int_tmp_result_FFWKCAA);
} 

