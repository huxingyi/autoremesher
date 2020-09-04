/* Automatically generated code, do not edit */
/* Generated from source file: dot_compare_3d.pck */

inline int dot_compare_3d_filter( const double* p0, const double* p1, const double* p2) {
    double d1;
    d1 = (((p0[0] * p1[0]) + (p0[1] * p1[1])) + (p0[2] * p1[2]));
    double d2;
    d2 = (((p0[0] * p2[0]) + (p0[1] * p2[1])) + (p0[2] * p2[2]));
    int int_tmp_result;
    double double_tmp_result;
    double eps;
    double_tmp_result = (d1 - d2);
    double max1 = fabs(p0[0]);
    if( (max1 < fabs(p0[1])) )
    {
        max1 = fabs(p0[1]);
    } 
    if( (max1 < fabs(p0[2])) )
    {
        max1 = fabs(p0[2]);
    } 
    double max2 = fabs(p1[0]);
    if( (max2 < fabs(p1[1])) )
    {
        max2 = fabs(p1[1]);
    } 
    if( (max2 < fabs(p1[2])) )
    {
        max2 = fabs(p1[2]);
    } 
    if( (max2 < fabs(p2[0])) )
    {
        max2 = fabs(p2[0]);
    } 
    if( (max2 < fabs(p2[1])) )
    {
        max2 = fabs(p2[1]);
    } 
    if( (max2 < fabs(p2[2])) )
    {
        max2 = fabs(p2[2]);
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
    if( (lower_bound_1 < 3.01698158319050667656e-147) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 1.67597599124282407923e+153) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (2.44455106181954323552e-15 * (max1 * max2));
        if( (double_tmp_result > eps) )
        {
            int_tmp_result = 1;
        } 
        else 
        {
            if( (double_tmp_result < -eps) )
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

