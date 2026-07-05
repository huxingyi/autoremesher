/* Automatically generated code, do not edit */
/* Generated from source file: side3_2dlifted.pck */

inline int side3_2dlifted_2d_filter( const double* p0, const double* p1, const double* p2, const double* p3, double h0, double h1, double h2, double h3) {
    double a11;
    a11 = (p1[0] - p0[0]);
    double a12;
    a12 = (p1[1] - p0[1]);
    double a13;
    a13 = (h0 - h1);
    double a21;
    a21 = (p2[0] - p0[0]);
    double a22;
    a22 = (p2[1] - p0[1]);
    double a23;
    a23 = (h0 - h2);
    double a31;
    a31 = (p3[0] - p0[0]);
    double a32;
    a32 = (p3[1] - p0[1]);
    double a33;
    a33 = (h0 - h3);
    double Delta1;
    Delta1 = ((a21 * a32) - (a22 * a31));
    double Delta2;
    Delta2 = ((a11 * a32) - (a12 * a31));
    double Delta3;
    Delta3 = ((a11 * a22) - (a12 * a21));
    double r;
    r = (((Delta1 * a13) - (Delta2 * a23)) + (Delta3 * a33));
    double eps;
    double max1 = fabs(a11);
    if( (max1 < fabs(a12)) )
    {
        max1 = fabs(a12);
    } 
    double max2 = fabs(a21);
    if( (max2 < fabs(a22)) )
    {
        max2 = fabs(a22);
    } 
    double lower_bound_1;
    double upper_bound_1;
    int Delta3_sign;
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
    if( (lower_bound_1 < 5.00368081960964635413e-147) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 5.59936185544450928309e+101) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (8.88720573725927976811e-16 * (max1 * max2));
        if( (Delta3 > eps) )
        {
            int_tmp_result = 1;
        } 
        else 
        {
            if( (Delta3 < -eps) )
            {
                int_tmp_result = -1;
            } 
            else 
            {
                return FPG_UNCERTAIN_VALUE;
            } 
        } 
    } 
    Delta3_sign = int_tmp_result;
    int int_tmp_result_FFWKCAA;
    double max3 = max1;
    if( (max3 < max2) )
    {
        max3 = max2;
    } 
    double max4 = fabs(a13);
    if( (max4 < fabs(a23)) )
    {
        max4 = fabs(a23);
    } 
    if( (max4 < fabs(a33)) )
    {
        max4 = fabs(a33);
    } 
    double max5 = max2;
    if( (max5 < fabs(a31)) )
    {
        max5 = fabs(a31);
    } 
    if( (max5 < fabs(a32)) )
    {
        max5 = fabs(a32);
    } 
    lower_bound_1 = max3;
    upper_bound_1 = max3;
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
    if( (lower_bound_1 < 1.63288018496748314939e-98) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 5.59936185544450928309e+101) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (5.11071278299732992696e-15 * ((max3 * max5) * max4));
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
    return (Delta3_sign * int_tmp_result_FFWKCAA);
} 
