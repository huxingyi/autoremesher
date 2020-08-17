/* Automatically generated code, do not edit */
/* Generated from source file: det3d.pck */

inline int det_3d_filter( const double* p0, const double* p1, const double* p2) {
    double Delta;
    Delta = (((p0[0] * ((p1[1] * p2[2]) - (p1[2] * p2[1]))) - (p1[0] * ((p0[1] * p2[2]) - (p0[2] * p2[1])))) + (p2[0] * ((p0[1] * p1[2]) - (p0[2] * p1[1]))));
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
    double max2 = fabs(p0[1]);
    if( (max2 < fabs(p0[2])) )
    {
        max2 = fabs(p0[2]);
    } 
    if( (max2 < fabs(p1[1])) )
    {
        max2 = fabs(p1[1]);
    } 
    if( (max2 < fabs(p1[2])) )
    {
        max2 = fabs(p1[2]);
    } 
    double max3 = fabs(p1[1]);
    if( (max3 < fabs(p1[2])) )
    {
        max3 = fabs(p1[2]);
    } 
    if( (max3 < fabs(p2[1])) )
    {
        max3 = fabs(p2[1]);
    } 
    if( (max3 < fabs(p2[2])) )
    {
        max3 = fabs(p2[2]);
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
    if( (lower_bound_1 < 1.92663387981871579179e-98) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 1.11987237108890185662e+102) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (3.11133555671680765034e-15 * ((max2 * max3) * max1));
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
