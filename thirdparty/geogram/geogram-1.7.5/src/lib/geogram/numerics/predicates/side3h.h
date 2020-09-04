/* Automatically generated code, do not edit */
/* Generated from source file: side3h.pck */

inline int side3h_3d_filter( const double* p0, const double* p1, const double* p2, const double* p3, double h0, double h1, double h2, double h3, const double* q0, const double* q1, const double* q2) {
    double l1;
    l1 = (h1 - h0);
    double l2;
    l2 = (h2 - h0);
    double l3;
    l3 = (h3 - h0);
    double p1_0_p0_0 = (p1[0] - p0[0]);
    double q0_0_p0_0 = (q0[0] - p0[0]);
    double p1_1_p0_1 = (p1[1] - p0[1]);
    double q0_1_p0_1 = (q0[1] - p0[1]);
    double p1_2_p0_2 = (p1[2] - p0[2]);
    double q0_2_p0_2 = (q0[2] - p0[2]);
    double a10;
    a10 = (2 * (((p1_0_p0_0 * q0_0_p0_0) + (p1_1_p0_1 * q0_1_p0_1)) + (p1_2_p0_2 * q0_2_p0_2)));
    double q1_0_p0_0 = (q1[0] - p0[0]);
    double q1_1_p0_1 = (q1[1] - p0[1]);
    double q1_2_p0_2 = (q1[2] - p0[2]);
    double a11;
    a11 = (2 * (((p1_0_p0_0 * q1_0_p0_0) + (p1_1_p0_1 * q1_1_p0_1)) + (p1_2_p0_2 * q1_2_p0_2)));
    double q2_0_p0_0 = (q2[0] - p0[0]);
    double q2_1_p0_1 = (q2[1] - p0[1]);
    double q2_2_p0_2 = (q2[2] - p0[2]);
    double a12;
    a12 = (2 * (((p1_0_p0_0 * q2_0_p0_0) + (p1_1_p0_1 * q2_1_p0_1)) + (p1_2_p0_2 * q2_2_p0_2)));
    double p2_0_p0_0 = (p2[0] - p0[0]);
    double p2_1_p0_1 = (p2[1] - p0[1]);
    double p2_2_p0_2 = (p2[2] - p0[2]);
    double a20;
    a20 = (2 * (((p2_0_p0_0 * q0_0_p0_0) + (p2_1_p0_1 * q0_1_p0_1)) + (p2_2_p0_2 * q0_2_p0_2)));
    double a21;
    a21 = (2 * (((p2_0_p0_0 * q1_0_p0_0) + (p2_1_p0_1 * q1_1_p0_1)) + (p2_2_p0_2 * q1_2_p0_2)));
    double a22;
    a22 = (2 * (((p2_0_p0_0 * q2_0_p0_0) + (p2_1_p0_1 * q2_1_p0_1)) + (p2_2_p0_2 * q2_2_p0_2)));
    double p3_0_p0_0 = (p3[0] - p0[0]);
    double p3_1_p0_1 = (p3[1] - p0[1]);
    double p3_2_p0_2 = (p3[2] - p0[2]);
    double a30;
    a30 = (2 * (((p3_0_p0_0 * q0_0_p0_0) + (p3_1_p0_1 * q0_1_p0_1)) + (p3_2_p0_2 * q0_2_p0_2)));
    double a31;
    a31 = (2 * (((p3_0_p0_0 * q1_0_p0_0) + (p3_1_p0_1 * q1_1_p0_1)) + (p3_2_p0_2 * q1_2_p0_2)));
    double a32;
    a32 = (2 * (((p3_0_p0_0 * q2_0_p0_0) + (p3_1_p0_1 * q2_1_p0_1)) + (p3_2_p0_2 * q2_2_p0_2)));
    double b00;
    b00 = ((a11 * a22) - (a12 * a21));
    double b01;
    b01 = (a21 - a22);
    double b02;
    b02 = (a12 - a11);
    double b10;
    b10 = ((a12 * a20) - (a10 * a22));
    double b11;
    b11 = (a22 - a20);
    double b12;
    b12 = (a10 - a12);
    double b20;
    b20 = ((a10 * a21) - (a11 * a20));
    double b21;
    b21 = (a20 - a21);
    double b22;
    b22 = (a11 - a10);
    double Delta;
    Delta = ((b00 + b10) + b20);
    double DeltaLambda0;
    DeltaLambda0 = (((b01 * l1) + (b02 * l2)) + b00);
    double DeltaLambda1;
    DeltaLambda1 = (((b11 * l1) + (b12 * l2)) + b10);
    double DeltaLambda2;
    DeltaLambda2 = (((b21 * l1) + (b22 * l2)) + b20);
    double r;
    r = ((Delta * l3) - (((a30 * DeltaLambda0) + (a31 * DeltaLambda1)) + (a32 * DeltaLambda2)));
    double eps;
    double max1 = fabs(q2_2_p0_2);
    if( (max1 < fabs(q0_0_p0_0)) )
    {
        max1 = fabs(q0_0_p0_0);
    } 
    if( (max1 < fabs(q0_1_p0_1)) )
    {
        max1 = fabs(q0_1_p0_1);
    } 
    if( (max1 < fabs(q0_2_p0_2)) )
    {
        max1 = fabs(q0_2_p0_2);
    } 
    if( (max1 < fabs(q1_0_p0_0)) )
    {
        max1 = fabs(q1_0_p0_0);
    } 
    if( (max1 < fabs(q1_1_p0_1)) )
    {
        max1 = fabs(q1_1_p0_1);
    } 
    if( (max1 < fabs(q1_2_p0_2)) )
    {
        max1 = fabs(q1_2_p0_2);
    } 
    if( (max1 < fabs(q2_0_p0_0)) )
    {
        max1 = fabs(q2_0_p0_0);
    } 
    if( (max1 < fabs(q2_1_p0_1)) )
    {
        max1 = fabs(q2_1_p0_1);
    } 
    double max2 = fabs(p2_0_p0_0);
    if( (max2 < fabs(p2_1_p0_1)) )
    {
        max2 = fabs(p2_1_p0_1);
    } 
    if( (max2 < fabs(p2_2_p0_2)) )
    {
        max2 = fabs(p2_2_p0_2);
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
    if( (lower_bound_1 < 2.22985945097100191780e-74) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 2.59614842926741294957e+33) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (8.99983341597279045654e-14 * (((max3 * max1) * max2) * max1));
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
    double max4 = max2;
    if( (max4 < fabs(l1)) )
    {
        max4 = fabs(l1);
    } 
    if( (max4 < fabs(l2)) )
    {
        max4 = fabs(l2);
    } 
    double max5 = max2;
    if( (max5 < max3) )
    {
        max5 = max3;
    } 
    if( (max5 < fabs(l3)) )
    {
        max5 = fabs(l3);
    } 
    double max6 = max2;
    if( (max6 < fabs(q2_2_p0_2)) )
    {
        max6 = fabs(q2_2_p0_2);
    } 
    if( (max6 < fabs(q0_0_p0_0)) )
    {
        max6 = fabs(q0_0_p0_0);
    } 
    if( (max6 < fabs(q0_1_p0_1)) )
    {
        max6 = fabs(q0_1_p0_1);
    } 
    if( (max6 < fabs(q0_2_p0_2)) )
    {
        max6 = fabs(q0_2_p0_2);
    } 
    if( (max6 < fabs(q2_0_p0_0)) )
    {
        max6 = fabs(q2_0_p0_0);
    } 
    if( (max6 < fabs(q2_1_p0_1)) )
    {
        max6 = fabs(q2_1_p0_1);
    } 
    double max7 = max3;
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
    int r_sign;
    int int_tmp_result_FFWKCAA;
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
    if( (lower_bound_1 < 5.53478725478149652989e-50) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 2.59614842926741294957e+33) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (7.73996217364502738018e-13 * (((((max7 * max1) * max6) * max1) * max5) * max4));
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
    r_sign = int_tmp_result_FFWKCAA;
    return (Delta_sign * r_sign);
} 

