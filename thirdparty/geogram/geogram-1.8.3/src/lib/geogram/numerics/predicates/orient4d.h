/* Automatically generated code, do not edit */
/* Generated from source file: orient4d.pck */

inline int orienth_3d_filter( const double* p0, const double* p1, const double* p2, const double* p3, const double* p4, double h0, double h1, double h2, double h3, double h4) {
    double a00;
    a00 = (p1[0] - p0[0]);
    double a01;
    a01 = (p1[1] - p0[1]);
    double a02;
    a02 = (p1[2] - p0[2]);
    double a03;
    a03 = (h1 - h0);
    double a10;
    a10 = (p2[0] - p0[0]);
    double a11;
    a11 = (p2[1] - p0[1]);
    double a12;
    a12 = (p2[2] - p0[2]);
    double a13;
    a13 = (h2 - h0);
    double a20;
    a20 = (p3[0] - p0[0]);
    double a21;
    a21 = (p3[1] - p0[1]);
    double a22;
    a22 = (p3[2] - p0[2]);
    double a23;
    a23 = (h3 - h0);
    double a30;
    a30 = (p4[0] - p0[0]);
    double a31;
    a31 = (p4[1] - p0[1]);
    double a32;
    a32 = (p4[2] - p0[2]);
    double a33;
    a33 = (h4 - h0);
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
    double max1 = fabs(a00);
    if( (max1 < fabs(a10)) )
    {
        max1 = fabs(a10);
    } 
    if( (max1 < fabs(a20)) )
    {
        max1 = fabs(a20);
    } 
    if( (max1 < fabs(a30)) )
    {
        max1 = fabs(a30);
    } 
    double max2 = fabs(a01);
    if( (max2 < fabs(a11)) )
    {
        max2 = fabs(a11);
    } 
    if( (max2 < fabs(a21)) )
    {
        max2 = fabs(a21);
    } 
    if( (max2 < fabs(a31)) )
    {
        max2 = fabs(a31);
    } 
    double max3 = fabs(a02);
    if( (max3 < fabs(a12)) )
    {
        max3 = fabs(a12);
    } 
    if( (max3 < fabs(a22)) )
    {
        max3 = fabs(a22);
    } 
    if( (max3 < fabs(a32)) )
    {
        max3 = fabs(a32);
    } 
    double max4 = fabs(a03);
    if( (max4 < fabs(a13)) )
    {
        max4 = fabs(a13);
    } 
    if( (max4 < fabs(a23)) )
    {
        max4 = fabs(a23);
    } 
    if( (max4 < fabs(a33)) )
    {
        max4 = fabs(a33);
    } 
    double lower_bound_1;
    double upper_bound_1;
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
    if( (lower_bound_1 < 2.89273249588395233567e-74) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 7.23700557733225900010e+75) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (3.17768858673611390687e-14 * (((max1 * max2) * max3) * max4));
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

