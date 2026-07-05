/* Automatically generated code, do not edit */
/* Generated from source file: det4d.pck */

inline int det_4d_filter( const double* p0, const double* p1, const double* p2, const double* p3) {
    double m12;
    m12 = ((p1[0] * p0[1]) - (p0[0] * p1[1]));
    double m13;
    m13 = ((p2[0] * p0[1]) - (p0[0] * p2[1]));
    double m14;
    m14 = ((p3[0] * p0[1]) - (p0[0] * p3[1]));
    double m23;
    m23 = ((p2[0] * p1[1]) - (p1[0] * p2[1]));
    double m24;
    m24 = ((p3[0] * p1[1]) - (p1[0] * p3[1]));
    double m34;
    m34 = ((p3[0] * p2[1]) - (p2[0] * p3[1]));
    double m123;
    m123 = (((m23 * p0[2]) - (m13 * p1[2])) + (m12 * p2[2]));
    double m124;
    m124 = (((m24 * p0[2]) - (m14 * p1[2])) + (m12 * p3[2]));
    double m134;
    m134 = (((m34 * p0[2]) - (m14 * p2[2])) + (m13 * p3[2]));
    double m234;
    m234 = (((m34 * p1[2]) - (m24 * p2[2])) + (m23 * p3[2]));
    double Delta;
    Delta = ((((m234 * p0[3]) - (m134 * p1[3])) + (m124 * p2[3])) - (m123 * p3[3]));
    int int_tmp_result;
    double eps;
    double max1 = fabs(p0[0]);
    if( (max1 < fabs(p1[0])) )
    {
        max1 = fabs(p1[0]);
    } 
    if( (max1 < fabs(p2[0])) )
    {
        max1 = fabs(p2[0]);
    } 
    if( (max1 < fabs(p3[0])) )
    {
        max1 = fabs(p3[0]);
    } 
    double max2 = fabs(p0[1]);
    if( (max2 < fabs(p1[1])) )
    {
        max2 = fabs(p1[1]);
    } 
    if( (max2 < fabs(p2[1])) )
    {
        max2 = fabs(p2[1]);
    } 
    if( (max2 < fabs(p3[1])) )
    {
        max2 = fabs(p3[1]);
    } 
    double max3 = fabs(p0[2]);
    if( (max3 < fabs(p1[2])) )
    {
        max3 = fabs(p1[2]);
    } 
    if( (max3 < fabs(p2[2])) )
    {
        max3 = fabs(p2[2]);
    } 
    if( (max3 < fabs(p3[2])) )
    {
        max3 = fabs(p3[2]);
    } 
    double max4 = fabs(p0[3]);
    if( (max4 < fabs(p1[3])) )
    {
        max4 = fabs(p1[3]);
    } 
    if( (max4 < fabs(p2[3])) )
    {
        max4 = fabs(p2[3]);
    } 
    if( (max4 < fabs(p3[3])) )
    {
        max4 = fabs(p3[3]);
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
    if( (lower_bound_1 < 3.20402459074399025456e-74) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 1.44740111546645196071e+76) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (2.11135406605316806158e-14 * (((max1 * max2) * max3) * max4));
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

