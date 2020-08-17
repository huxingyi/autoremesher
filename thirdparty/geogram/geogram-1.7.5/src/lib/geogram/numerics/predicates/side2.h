/* Automatically generated code, do not edit */
/* Generated from source file: side2.pck */

inline int side2_3d_filter( const double* p0, const double* p1, const double* p2, const double* q0, const double* q1) {
    double p1_0_p0_0 = (p1[0] - p0[0]);
    double p1_1_p0_1 = (p1[1] - p0[1]);
    double p1_2_p0_2 = (p1[2] - p0[2]);
    double l1;
    l1 = (1 * (((p1_0_p0_0 * p1_0_p0_0) + (p1_1_p0_1 * p1_1_p0_1)) + (p1_2_p0_2 * p1_2_p0_2)));
    double p2_0_p0_0 = (p2[0] - p0[0]);
    double p2_1_p0_1 = (p2[1] - p0[1]);
    double p2_2_p0_2 = (p2[2] - p0[2]);
    double l2;
    l2 = (1 * (((p2_0_p0_0 * p2_0_p0_0) + (p2_1_p0_1 * p2_1_p0_1)) + (p2_2_p0_2 * p2_2_p0_2)));
    double q0_0_p0_0 = (q0[0] - p0[0]);
    double q0_1_p0_1 = (q0[1] - p0[1]);
    double q0_2_p0_2 = (q0[2] - p0[2]);
    double a10;
    a10 = (2 * (((p1_0_p0_0 * q0_0_p0_0) + (p1_1_p0_1 * q0_1_p0_1)) + (p1_2_p0_2 * q0_2_p0_2)));
    double q1_0_p0_0 = (q1[0] - p0[0]);
    double q1_1_p0_1 = (q1[1] - p0[1]);
    double q1_2_p0_2 = (q1[2] - p0[2]);
    double a11;
    a11 = (2 * (((p1_0_p0_0 * q1_0_p0_0) + (p1_1_p0_1 * q1_1_p0_1)) + (p1_2_p0_2 * q1_2_p0_2)));
    double a20;
    a20 = (2 * (((p2_0_p0_0 * q0_0_p0_0) + (p2_1_p0_1 * q0_1_p0_1)) + (p2_2_p0_2 * q0_2_p0_2)));
    double a21;
    a21 = (2 * (((p2_0_p0_0 * q1_0_p0_0) + (p2_1_p0_1 * q1_1_p0_1)) + (p2_2_p0_2 * q1_2_p0_2)));
    double Delta;
    Delta = (a11 - a10);
    double DeltaLambda0;
    DeltaLambda0 = (a11 - l1);
    double DeltaLambda1;
    DeltaLambda1 = (l1 - a10);
    double r;
    r = (((Delta * l2) - (a20 * DeltaLambda0)) - (a21 * DeltaLambda1));
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
    double max2 = fabs(q0_0_p0_0);
    if( (max2 < fabs(q0_1_p0_1)) )
    {
        max2 = fabs(q0_1_p0_1);
    } 
    if( (max2 < fabs(q0_2_p0_2)) )
    {
        max2 = fabs(q0_2_p0_2);
    } 
    if( (max2 < fabs(q1_0_p0_0)) )
    {
        max2 = fabs(q1_0_p0_0);
    } 
    if( (max2 < fabs(q1_1_p0_1)) )
    {
        max2 = fabs(q1_1_p0_1);
    } 
    if( (max2 < fabs(q1_2_p0_2)) )
    {
        max2 = fabs(q1_2_p0_2);
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
    if( (lower_bound_1 < 2.23755023300058943229e-147) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 3.74144419156711063983e+50) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (4.44425370757048798480e-15 * (max1 * max2));
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
    double max3 = max1;
    if( (max3 < max2) )
    {
        max3 = max2;
    } 
    double max4 = max2;
    if( (max4 < fabs(p2_0_p0_0)) )
    {
        max4 = fabs(p2_0_p0_0);
    } 
    if( (max4 < fabs(p2_1_p0_1)) )
    {
        max4 = fabs(p2_1_p0_1);
    } 
    if( (max4 < fabs(p2_2_p0_2)) )
    {
        max4 = fabs(p2_2_p0_2);
    } 
    if( (max3 < max4) )
    {
        max3 = max4;
    } 
    int r_sign;
    int int_tmp_result_FFWKCAA;
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
    if( (max4 < lower_bound_1) )
    {
        lower_bound_1 = max4;
    } 
    if( (lower_bound_1 < 2.22985945097100191780e-74) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 3.74144419156711063983e+50) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (8.99983341597279045654e-14 * (((max1 * max4) * max4) * max3));
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


inline int side2_4d_filter( const double* p0, const double* p1, const double* p2, const double* q0, const double* q1) {
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
    double a20;
    a20 = (2 * ((((p2_0_p0_0 * q0_0_p0_0) + (p2_1_p0_1 * q0_1_p0_1)) + (p2_2_p0_2 * q0_2_p0_2)) + (p2_3_p0_3 * q0_3_p0_3)));
    double a21;
    a21 = (2 * ((((p2_0_p0_0 * q1_0_p0_0) + (p2_1_p0_1 * q1_1_p0_1)) + (p2_2_p0_2 * q1_2_p0_2)) + (p2_3_p0_3 * q1_3_p0_3)));
    double Delta;
    Delta = (a11 - a10);
    double DeltaLambda0;
    DeltaLambda0 = (a11 - l1);
    double DeltaLambda1;
    DeltaLambda1 = (l1 - a10);
    double r;
    r = (((Delta * l2) - (a20 * DeltaLambda0)) - (a21 * DeltaLambda1));
    double eps;
    double max1 = fabs(p1_2_p0_2);
    if( (max1 < fabs(p1_0_p0_0)) )
    {
        max1 = fabs(p1_0_p0_0);
    } 
    if( (max1 < fabs(p1_1_p0_1)) )
    {
        max1 = fabs(p1_1_p0_1);
    } 
    if( (max1 < fabs(p1_3_p0_3)) )
    {
        max1 = fabs(p1_3_p0_3);
    } 
    double max2 = fabs(q0_0_p0_0);
    if( (max2 < fabs(q0_1_p0_1)) )
    {
        max2 = fabs(q0_1_p0_1);
    } 
    if( (max2 < fabs(q0_2_p0_2)) )
    {
        max2 = fabs(q0_2_p0_2);
    } 
    if( (max2 < fabs(q0_3_p0_3)) )
    {
        max2 = fabs(q0_3_p0_3);
    } 
    if( (max2 < fabs(q1_0_p0_0)) )
    {
        max2 = fabs(q1_0_p0_0);
    } 
    if( (max2 < fabs(q1_1_p0_1)) )
    {
        max2 = fabs(q1_1_p0_1);
    } 
    if( (max2 < fabs(q1_2_p0_2)) )
    {
        max2 = fabs(q1_2_p0_2);
    } 
    if( (max2 < fabs(q1_3_p0_3)) )
    {
        max2 = fabs(q1_3_p0_3);
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
    if( (lower_bound_1 < 1.85816790703293534018e-147) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 1.87072209578355531992e+50) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (6.44428177279185717888e-15 * (max1 * max2));
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
    double max3 = max1;
    if( (max3 < max2) )
    {
        max3 = max2;
    } 
    double max4 = max2;
    if( (max4 < fabs(p2_0_p0_0)) )
    {
        max4 = fabs(p2_0_p0_0);
    } 
    if( (max4 < fabs(p2_2_p0_2)) )
    {
        max4 = fabs(p2_2_p0_2);
    } 
    if( (max4 < fabs(p2_1_p0_1)) )
    {
        max4 = fabs(p2_1_p0_1);
    } 
    if( (max4 < fabs(p2_3_p0_3)) )
    {
        max4 = fabs(p2_3_p0_3);
    } 
    if( (max3 < max4) )
    {
        max3 = max4;
    } 
    int r_sign;
    int int_tmp_result_FFWKCAA;
    lower_bound_1 = max3;
    upper_bound_1 = max3;
    if( (max1 < lower_bound_1) )
    {
        lower_bound_1 = max1;
    } 
    if( (max4 < lower_bound_1) )
    {
        lower_bound_1 = max4;
    } 
    if( (lower_bound_1 < 1.89528395402941802921e-74) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 1.87072209578355531992e+50) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (1.72443682410932010423e-13 * (((max1 * max4) * max4) * max3));
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


inline int side2_6d_filter( const double* p0, const double* p1, const double* p2, const double* q0, const double* q1) {
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
    double a20;
    a20 = (2 * ((((((p2_0_p0_0 * q0_0_p0_0) + (p2_1_p0_1 * q0_1_p0_1)) + (p2_2_p0_2 * q0_2_p0_2)) + (p2_3_p0_3 * q0_3_p0_3)) + (p2_4_p0_4 * q0_4_p0_4)) + (p2_5_p0_5 * q0_5_p0_5)));
    double a21;
    a21 = (2 * ((((((p2_0_p0_0 * q1_0_p0_0) + (p2_1_p0_1 * q1_1_p0_1)) + (p2_2_p0_2 * q1_2_p0_2)) + (p2_3_p0_3 * q1_3_p0_3)) + (p2_4_p0_4 * q1_4_p0_4)) + (p2_5_p0_5 * q1_5_p0_5)));
    double Delta;
    Delta = (a11 - a10);
    double DeltaLambda0;
    DeltaLambda0 = (a11 - l1);
    double DeltaLambda1;
    DeltaLambda1 = (l1 - a10);
    double r;
    r = (((Delta * l2) - (a20 * DeltaLambda0)) - (a21 * DeltaLambda1));
    double eps;
    double max1 = fabs(p1_1_p0_1);
    if( (max1 < fabs(p1_2_p0_2)) )
    {
        max1 = fabs(p1_2_p0_2);
    } 
    if( (max1 < fabs(p1_3_p0_3)) )
    {
        max1 = fabs(p1_3_p0_3);
    } 
    if( (max1 < fabs(p1_0_p0_0)) )
    {
        max1 = fabs(p1_0_p0_0);
    } 
    if( (max1 < fabs(p1_4_p0_4)) )
    {
        max1 = fabs(p1_4_p0_4);
    } 
    if( (max1 < fabs(p1_5_p0_5)) )
    {
        max1 = fabs(p1_5_p0_5);
    } 
    double max2 = fabs(q0_0_p0_0);
    if( (max2 < fabs(q0_1_p0_1)) )
    {
        max2 = fabs(q0_1_p0_1);
    } 
    if( (max2 < fabs(q0_2_p0_2)) )
    {
        max2 = fabs(q0_2_p0_2);
    } 
    if( (max2 < fabs(q0_3_p0_3)) )
    {
        max2 = fabs(q0_3_p0_3);
    } 
    if( (max2 < fabs(q0_4_p0_4)) )
    {
        max2 = fabs(q0_4_p0_4);
    } 
    if( (max2 < fabs(q0_5_p0_5)) )
    {
        max2 = fabs(q0_5_p0_5);
    } 
    if( (max2 < fabs(q1_0_p0_0)) )
    {
        max2 = fabs(q1_0_p0_0);
    } 
    if( (max2 < fabs(q1_1_p0_1)) )
    {
        max2 = fabs(q1_1_p0_1);
    } 
    if( (max2 < fabs(q1_2_p0_2)) )
    {
        max2 = fabs(q1_2_p0_2);
    } 
    if( (max2 < fabs(q1_3_p0_3)) )
    {
        max2 = fabs(q1_3_p0_3);
    } 
    if( (max2 < fabs(q1_4_p0_4)) )
    {
        max2 = fabs(q1_4_p0_4);
    } 
    if( (max2 < fabs(q1_5_p0_5)) )
    {
        max2 = fabs(q1_5_p0_5);
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
    if( (lower_bound_1 < 1.41511993781011659868e-147) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 1.87072209578355531992e+50) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (1.11111223981318615596e-14 * (max1 * max2));
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
    double max3 = max1;
    if( (max3 < max2) )
    {
        max3 = max2;
    } 
    double max4 = max2;
    if( (max4 < fabs(p2_2_p0_2)) )
    {
        max4 = fabs(p2_2_p0_2);
    } 
    if( (max4 < fabs(p2_4_p0_4)) )
    {
        max4 = fabs(p2_4_p0_4);
    } 
    if( (max4 < fabs(p2_3_p0_3)) )
    {
        max4 = fabs(p2_3_p0_3);
    } 
    if( (max4 < fabs(p2_0_p0_0)) )
    {
        max4 = fabs(p2_0_p0_0);
    } 
    if( (max4 < fabs(p2_1_p0_1)) )
    {
        max4 = fabs(p2_1_p0_1);
    } 
    if( (max4 < fabs(p2_5_p0_5)) )
    {
        max4 = fabs(p2_5_p0_5);
    } 
    if( (max3 < max4) )
    {
        max3 = max4;
    } 
    int r_sign;
    int int_tmp_result_FFWKCAA;
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
    if( (max4 < lower_bound_1) )
    {
        lower_bound_1 = max4;
    } 
    if( (lower_bound_1 < 1.49958502193059513986e-74) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 1.87072209578355531992e+50) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (4.40007476026584016994e-13 * (((max1 * max4) * max4) * max3));
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


inline int side2_7d_filter( const double* p0, const double* p1, const double* p2, const double* q0, const double* q1) {
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
    double a20;
    a20 = (2 * (((((((p2_0_p0_0 * q0_0_p0_0) + (p2_1_p0_1 * q0_1_p0_1)) + (p2_2_p0_2 * q0_2_p0_2)) + (p2_3_p0_3 * q0_3_p0_3)) + (p2_4_p0_4 * q0_4_p0_4)) + (p2_5_p0_5 * q0_5_p0_5)) + (p2_6_p0_6 * q0_6_p0_6)));
    double a21;
    a21 = (2 * (((((((p2_0_p0_0 * q1_0_p0_0) + (p2_1_p0_1 * q1_1_p0_1)) + (p2_2_p0_2 * q1_2_p0_2)) + (p2_3_p0_3 * q1_3_p0_3)) + (p2_4_p0_4 * q1_4_p0_4)) + (p2_5_p0_5 * q1_5_p0_5)) + (p2_6_p0_6 * q1_6_p0_6)));
    double Delta;
    Delta = (a11 - a10);
    double DeltaLambda0;
    DeltaLambda0 = (a11 - l1);
    double DeltaLambda1;
    DeltaLambda1 = (l1 - a10);
    double r;
    r = (((Delta * l2) - (a20 * DeltaLambda0)) - (a21 * DeltaLambda1));
    double eps;
    double max1 = fabs(p1_2_p0_2);
    if( (max1 < fabs(p1_1_p0_1)) )
    {
        max1 = fabs(p1_1_p0_1);
    } 
    if( (max1 < fabs(p1_0_p0_0)) )
    {
        max1 = fabs(p1_0_p0_0);
    } 
    if( (max1 < fabs(p1_4_p0_4)) )
    {
        max1 = fabs(p1_4_p0_4);
    } 
    if( (max1 < fabs(p1_3_p0_3)) )
    {
        max1 = fabs(p1_3_p0_3);
    } 
    if( (max1 < fabs(p1_5_p0_5)) )
    {
        max1 = fabs(p1_5_p0_5);
    } 
    if( (max1 < fabs(p1_6_p0_6)) )
    {
        max1 = fabs(p1_6_p0_6);
    } 
    double max2 = fabs(q0_0_p0_0);
    if( (max2 < fabs(q0_1_p0_1)) )
    {
        max2 = fabs(q0_1_p0_1);
    } 
    if( (max2 < fabs(q0_2_p0_2)) )
    {
        max2 = fabs(q0_2_p0_2);
    } 
    if( (max2 < fabs(q0_3_p0_3)) )
    {
        max2 = fabs(q0_3_p0_3);
    } 
    if( (max2 < fabs(q0_4_p0_4)) )
    {
        max2 = fabs(q0_4_p0_4);
    } 
    if( (max2 < fabs(q0_5_p0_5)) )
    {
        max2 = fabs(q0_5_p0_5);
    } 
    if( (max2 < fabs(q0_6_p0_6)) )
    {
        max2 = fabs(q0_6_p0_6);
    } 
    if( (max2 < fabs(q1_0_p0_0)) )
    {
        max2 = fabs(q1_0_p0_0);
    } 
    if( (max2 < fabs(q1_1_p0_1)) )
    {
        max2 = fabs(q1_1_p0_1);
    } 
    if( (max2 < fabs(q1_2_p0_2)) )
    {
        max2 = fabs(q1_2_p0_2);
    } 
    if( (max2 < fabs(q1_3_p0_3)) )
    {
        max2 = fabs(q1_3_p0_3);
    } 
    if( (max2 < fabs(q1_4_p0_4)) )
    {
        max2 = fabs(q1_4_p0_4);
    } 
    if( (max2 < fabs(q1_5_p0_5)) )
    {
        max2 = fabs(q1_5_p0_5);
    } 
    if( (max2 < fabs(q1_6_p0_6)) )
    {
        max2 = fabs(q1_6_p0_6);
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
    if( (lower_bound_1 < 1.27080861580266953580e-147) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 1.87072209578355531992e+50) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (1.37779349582504943796e-14 * (max1 * max2));
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
    double max3 = max1;
    if( (max3 < max2) )
    {
        max3 = max2;
    } 
    double max4 = max2;
    if( (max4 < fabs(p2_0_p0_0)) )
    {
        max4 = fabs(p2_0_p0_0);
    } 
    if( (max4 < fabs(p2_1_p0_1)) )
    {
        max4 = fabs(p2_1_p0_1);
    } 
    if( (max4 < fabs(p2_2_p0_2)) )
    {
        max4 = fabs(p2_2_p0_2);
    } 
    if( (max4 < fabs(p2_3_p0_3)) )
    {
        max4 = fabs(p2_3_p0_3);
    } 
    if( (max4 < fabs(p2_4_p0_4)) )
    {
        max4 = fabs(p2_4_p0_4);
    } 
    if( (max4 < fabs(p2_5_p0_5)) )
    {
        max4 = fabs(p2_5_p0_5);
    } 
    if( (max4 < fabs(p2_6_p0_6)) )
    {
        max4 = fabs(p2_6_p0_6);
    } 
    if( (max3 < max4) )
    {
        max3 = max4;
    } 
    int r_sign;
    int int_tmp_result_FFWKCAA;
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
    if( (max4 < lower_bound_1) )
    {
        lower_bound_1 = max4;
    } 
    if( (lower_bound_1 < 1.36918881183883509035e-74) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 1.87072209578355531992e+50) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (6.33127335329798996022e-13 * (((max1 * max4) * max4) * max3));
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


inline int side2_8d_filter( const double* p0, const double* p1, const double* p2, const double* q0, const double* q1) {
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
    double a20;
    a20 = (2 * ((((((((p2_0_p0_0 * q0_0_p0_0) + (p2_1_p0_1 * q0_1_p0_1)) + (p2_2_p0_2 * q0_2_p0_2)) + (p2_3_p0_3 * q0_3_p0_3)) + (p2_4_p0_4 * q0_4_p0_4)) + (p2_5_p0_5 * q0_5_p0_5)) + (p2_6_p0_6 * q0_6_p0_6)) + (p2_7_p0_7 * q0_7_p0_7)));
    double a21;
    a21 = (2 * ((((((((p2_0_p0_0 * q1_0_p0_0) + (p2_1_p0_1 * q1_1_p0_1)) + (p2_2_p0_2 * q1_2_p0_2)) + (p2_3_p0_3 * q1_3_p0_3)) + (p2_4_p0_4 * q1_4_p0_4)) + (p2_5_p0_5 * q1_5_p0_5)) + (p2_6_p0_6 * q1_6_p0_6)) + (p2_7_p0_7 * q1_7_p0_7)));
    double Delta;
    Delta = (a11 - a10);
    double DeltaLambda0;
    DeltaLambda0 = (a11 - l1);
    double DeltaLambda1;
    DeltaLambda1 = (l1 - a10);
    double r;
    r = (((Delta * l2) - (a20 * DeltaLambda0)) - (a21 * DeltaLambda1));
    double eps;
    double max1 = fabs(p1_4_p0_4);
    if( (max1 < fabs(p1_3_p0_3)) )
    {
        max1 = fabs(p1_3_p0_3);
    } 
    if( (max1 < fabs(p1_7_p0_7)) )
    {
        max1 = fabs(p1_7_p0_7);
    } 
    if( (max1 < fabs(p1_0_p0_0)) )
    {
        max1 = fabs(p1_0_p0_0);
    } 
    if( (max1 < fabs(p1_6_p0_6)) )
    {
        max1 = fabs(p1_6_p0_6);
    } 
    if( (max1 < fabs(p1_2_p0_2)) )
    {
        max1 = fabs(p1_2_p0_2);
    } 
    if( (max1 < fabs(p1_1_p0_1)) )
    {
        max1 = fabs(p1_1_p0_1);
    } 
    if( (max1 < fabs(p1_5_p0_5)) )
    {
        max1 = fabs(p1_5_p0_5);
    } 
    double max2 = fabs(q0_0_p0_0);
    if( (max2 < fabs(q0_1_p0_1)) )
    {
        max2 = fabs(q0_1_p0_1);
    } 
    if( (max2 < fabs(q0_2_p0_2)) )
    {
        max2 = fabs(q0_2_p0_2);
    } 
    if( (max2 < fabs(q0_3_p0_3)) )
    {
        max2 = fabs(q0_3_p0_3);
    } 
    if( (max2 < fabs(q0_4_p0_4)) )
    {
        max2 = fabs(q0_4_p0_4);
    } 
    if( (max2 < fabs(q0_5_p0_5)) )
    {
        max2 = fabs(q0_5_p0_5);
    } 
    if( (max2 < fabs(q0_6_p0_6)) )
    {
        max2 = fabs(q0_6_p0_6);
    } 
    if( (max2 < fabs(q0_7_p0_7)) )
    {
        max2 = fabs(q0_7_p0_7);
    } 
    if( (max2 < fabs(q1_0_p0_0)) )
    {
        max2 = fabs(q1_0_p0_0);
    } 
    if( (max2 < fabs(q1_1_p0_1)) )
    {
        max2 = fabs(q1_1_p0_1);
    } 
    if( (max2 < fabs(q1_2_p0_2)) )
    {
        max2 = fabs(q1_2_p0_2);
    } 
    if( (max2 < fabs(q1_3_p0_3)) )
    {
        max2 = fabs(q1_3_p0_3);
    } 
    if( (max2 < fabs(q1_4_p0_4)) )
    {
        max2 = fabs(q1_4_p0_4);
    } 
    if( (max2 < fabs(q1_5_p0_5)) )
    {
        max2 = fabs(q1_5_p0_5);
    } 
    if( (max2 < fabs(q1_6_p0_6)) )
    {
        max2 = fabs(q1_6_p0_6);
    } 
    if( (max2 < fabs(q1_7_p0_7)) )
    {
        max2 = fabs(q1_7_p0_7);
    } 
    double lower_bound_1;
    double upper_bound_1;
    int Delta_sign;
    int int_tmp_result;
    lower_bound_1 = max2;
    upper_bound_1 = max2;
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
    if( (lower_bound_1 < 1.15542931091530087067e-147) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 1.87072209578355531992e+50) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (1.66670090166682227006e-14 * (max1 * max2));
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
    double max3 = max2;
    if( (max3 < max1) )
    {
        max3 = max1;
    } 
    double max4 = max2;
    if( (max4 < fabs(p2_4_p0_4)) )
    {
        max4 = fabs(p2_4_p0_4);
    } 
    if( (max4 < fabs(p2_2_p0_2)) )
    {
        max4 = fabs(p2_2_p0_2);
    } 
    if( (max4 < fabs(p2_0_p0_0)) )
    {
        max4 = fabs(p2_0_p0_0);
    } 
    if( (max4 < fabs(p2_1_p0_1)) )
    {
        max4 = fabs(p2_1_p0_1);
    } 
    if( (max4 < fabs(p2_3_p0_3)) )
    {
        max4 = fabs(p2_3_p0_3);
    } 
    if( (max4 < fabs(p2_5_p0_5)) )
    {
        max4 = fabs(p2_5_p0_5);
    } 
    if( (max4 < fabs(p2_6_p0_6)) )
    {
        max4 = fabs(p2_6_p0_6);
    } 
    if( (max4 < fabs(p2_7_p0_7)) )
    {
        max4 = fabs(p2_7_p0_7);
    } 
    if( (max3 < max4) )
    {
        max3 = max4;
    } 
    int r_sign;
    int int_tmp_result_FFWKCAA;
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
    if( (max4 < lower_bound_1) )
    {
        lower_bound_1 = max4;
    } 
    if( (lower_bound_1 < 1.26419510663115923609e-74) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 1.87072209578355531992e+50) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (8.71140112255785451890e-13 * (((max1 * max4) * max4) * max3));
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

