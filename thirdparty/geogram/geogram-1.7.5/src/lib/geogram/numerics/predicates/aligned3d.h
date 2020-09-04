/* Automatically generated code, do not edit */
/* Generated from source file: aligned3d.pck */

inline int aligned_3d_filter( const double* p0, const double* p1, const double* p2) {
    double a11;
    a11 = (p1[0] - p0[0]);
    double a12;
    a12 = (p1[1] - p0[1]);
    double a13;
    a13 = (p1[2] - p0[2]);
    double a21;
    a21 = (p2[0] - p0[0]);
    double a22;
    a22 = (p2[1] - p0[1]);
    double a23;
    a23 = (p2[2] - p0[2]);
    double delta1;
    delta1 = ((a12 * a23) - (a22 * a13));
    double delta2;
    delta2 = ((a13 * a21) - (a23 * a11));
    double delta3;
    delta3 = ((a11 * a22) - (a21 * a12));
    int int_tmp_result;
    double eps;
    int int_tmp_result_FFWKCAA;
    int int_tmp_result_k60Ocge;
    double max1 = fabs(a12);
    if( (max1 < fabs(a22)) )
    {
        max1 = fabs(a22);
    } 
    double max2 = fabs(a13);
    if( (max2 < fabs(a23)) )
    {
        max2 = fabs(a23);
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
    if( (lower_bound_1 < 5.00368081960964635413e-147) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 1.67597599124282407923e+153) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (8.88720573725927976811e-16 * (max1 * max2));
        if( (delta1 > eps) )
        {
            int_tmp_result = 1;
        } 
        else 
        {
            if( (delta1 < -eps) )
            {
                int_tmp_result = -1;
            } 
            else 
            {
                return FPG_UNCERTAIN_VALUE;
            } 
        } 
    } 
    double max3 = fabs(a11);
    if( (max3 < fabs(a21)) )
    {
        max3 = fabs(a21);
    } 
    lower_bound_1 = max3;
    upper_bound_1 = max3;
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
    if( (lower_bound_1 < 5.00368081960964635413e-147) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 1.67597599124282407923e+153) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (8.88720573725927976811e-16 * (max2 * max3));
        if( (delta2 > eps) )
        {
            int_tmp_result_FFWKCAA = 1;
        } 
        else 
        {
            if( (delta2 < -eps) )
            {
                int_tmp_result_FFWKCAA = -1;
            } 
            else 
            {
                return FPG_UNCERTAIN_VALUE;
            } 
        } 
    } 
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
    if( (lower_bound_1 < 5.00368081960964635413e-147) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 1.67597599124282407923e+153) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (8.88720573725927976811e-16 * (max3 * max1));
        if( (delta3 > eps) )
        {
            int_tmp_result_k60Ocge = 1;
        } 
        else 
        {
            if( (delta3 < -eps) )
            {
                int_tmp_result_k60Ocge = -1;
            } 
            else 
            {
                return FPG_UNCERTAIN_VALUE;
            } 
        } 
    } 
    return ((((int_tmp_result == 0) && (int_tmp_result_FFWKCAA == 0)) && (int_tmp_result_k60Ocge == 0)) ? 0 : 1);
} 
