/* Automatically generated code, do not edit */
/* Generated from source file: side1.pck */

inline int side1_3d_filter( const double* p0, const double* p1, const double* q0) {
    double p0_0_p1_0 = (p0[0] - p1[0]);
    double p0_1_p1_1 = (p0[1] - p1[1]);
    double p0_2_p1_2 = (p0[2] - p1[2]);
    double r;
    r = (1 * (((p0_0_p1_0 * p0_0_p1_0) + (p0_1_p1_1 * p0_1_p1_1)) + (p0_2_p1_2 * p0_2_p1_2)));
    double p1_0_p0_0 = (p1[0] - p0[0]);
    double q0_0_p0_0 = (q0[0] - p0[0]);
    double p1_1_p0_1 = (p1[1] - p0[1]);
    double q0_1_p0_1 = (q0[1] - p0[1]);
    double p1_2_p0_2 = (p1[2] - p0[2]);
    double q0_2_p0_2 = (q0[2] - p0[2]);
    r = (r - (2 * (((p1_0_p0_0 * q0_0_p0_0) + (p1_1_p0_1 * q0_1_p0_1)) + (p1_2_p0_2 * q0_2_p0_2))));
    int int_tmp_result;
    double eps;
    double max1 = fabs(p0_0_p1_0);
    if( (max1 < fabs(p0_1_p1_1)) )
    {
        max1 = fabs(p0_1_p1_1);
    } 
    if( (max1 < fabs(p0_2_p1_2)) )
    {
        max1 = fabs(p0_2_p1_2);
    } 
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
    double max2 = fabs(p0_0_p1_0);
    if( (max2 < fabs(p0_1_p1_1)) )
    {
        max2 = fabs(p0_1_p1_1);
    } 
    if( (max2 < fabs(p0_2_p1_2)) )
    {
        max2 = fabs(p0_2_p1_2);
    } 
    if( (max2 < fabs(q0_0_p0_0)) )
    {
        max2 = fabs(q0_0_p0_0);
    } 
    if( (max2 < fabs(q0_1_p0_1)) )
    {
        max2 = fabs(q0_1_p0_1);
    } 
    if( (max2 < fabs(q0_2_p0_2)) )
    {
        max2 = fabs(q0_2_p0_2);
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
    if( (lower_bound_1 < 2.23755023300058943229e-147) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 5.59936185544450928309e+101) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (4.44425370757048798480e-15 * (max1 * max2));
        if( (r > eps) )
        {
            int_tmp_result = 1;
        } 
        else 
        {
            if( (r < -eps) )
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


inline int side1_4d_filter( const double* p0, const double* p1, const double* q0) {
    double p0_0_p1_0 = (p0[0] - p1[0]);
    double p0_1_p1_1 = (p0[1] - p1[1]);
    double p0_2_p1_2 = (p0[2] - p1[2]);
    double p0_3_p1_3 = (p0[3] - p1[3]);
    double r;
    r = (1 * ((((p0_0_p1_0 * p0_0_p1_0) + (p0_1_p1_1 * p0_1_p1_1)) + (p0_2_p1_2 * p0_2_p1_2)) + (p0_3_p1_3 * p0_3_p1_3)));
    double p1_0_p0_0 = (p1[0] - p0[0]);
    double q0_0_p0_0 = (q0[0] - p0[0]);
    double p1_1_p0_1 = (p1[1] - p0[1]);
    double q0_1_p0_1 = (q0[1] - p0[1]);
    double p1_2_p0_2 = (p1[2] - p0[2]);
    double q0_2_p0_2 = (q0[2] - p0[2]);
    double p1_3_p0_3 = (p1[3] - p0[3]);
    double q0_3_p0_3 = (q0[3] - p0[3]);
    r = (r - (2 * ((((p1_0_p0_0 * q0_0_p0_0) + (p1_1_p0_1 * q0_1_p0_1)) + (p1_2_p0_2 * q0_2_p0_2)) + (p1_3_p0_3 * q0_3_p0_3))));
    int int_tmp_result;
    double eps;
    double max1 = fabs(p1_0_p0_0);
    if( (max1 < fabs(p0_0_p1_0)) )
    {
        max1 = fabs(p0_0_p1_0);
    } 
    if( (max1 < fabs(p0_1_p1_1)) )
    {
        max1 = fabs(p0_1_p1_1);
    } 
    if( (max1 < fabs(p0_2_p1_2)) )
    {
        max1 = fabs(p0_2_p1_2);
    } 
    if( (max1 < fabs(p0_3_p1_3)) )
    {
        max1 = fabs(p0_3_p1_3);
    } 
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
    double max2 = fabs(p0_0_p1_0);
    if( (max2 < fabs(p0_1_p1_1)) )
    {
        max2 = fabs(p0_1_p1_1);
    } 
    if( (max2 < fabs(p0_2_p1_2)) )
    {
        max2 = fabs(p0_2_p1_2);
    } 
    if( (max2 < fabs(p0_3_p1_3)) )
    {
        max2 = fabs(p0_3_p1_3);
    } 
    if( (max2 < fabs(q0_0_p0_0)) )
    {
        max2 = fabs(q0_0_p0_0);
    } 
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
    double lower_bound_1;
    double upper_bound_1;
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
    if( (lower_bound_1 < 1.85816790703293534018e-147) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 5.59936185544450928309e+101) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (6.44428177279185717888e-15 * (max1 * max2));
        if( (r > eps) )
        {
            int_tmp_result = 1;
        } 
        else 
        {
            if( (r < -eps) )
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


inline int side1_6d_filter( const double* p0, const double* p1, const double* q0) {
    double p0_0_p1_0 = (p0[0] - p1[0]);
    double p0_1_p1_1 = (p0[1] - p1[1]);
    double p0_2_p1_2 = (p0[2] - p1[2]);
    double p0_3_p1_3 = (p0[3] - p1[3]);
    double p0_4_p1_4 = (p0[4] - p1[4]);
    double p0_5_p1_5 = (p0[5] - p1[5]);
    double r;
    r = (1 * ((((((p0_0_p1_0 * p0_0_p1_0) + (p0_1_p1_1 * p0_1_p1_1)) + (p0_2_p1_2 * p0_2_p1_2)) + (p0_3_p1_3 * p0_3_p1_3)) + (p0_4_p1_4 * p0_4_p1_4)) + (p0_5_p1_5 * p0_5_p1_5)));
    double p1_0_p0_0 = (p1[0] - p0[0]);
    double q0_0_p0_0 = (q0[0] - p0[0]);
    double p1_1_p0_1 = (p1[1] - p0[1]);
    double q0_1_p0_1 = (q0[1] - p0[1]);
    double p1_2_p0_2 = (p1[2] - p0[2]);
    double q0_2_p0_2 = (q0[2] - p0[2]);
    double p1_3_p0_3 = (p1[3] - p0[3]);
    double q0_3_p0_3 = (q0[3] - p0[3]);
    double p1_4_p0_4 = (p1[4] - p0[4]);
    double q0_4_p0_4 = (q0[4] - p0[4]);
    double p1_5_p0_5 = (p1[5] - p0[5]);
    double q0_5_p0_5 = (q0[5] - p0[5]);
    r = (r - (2 * ((((((p1_0_p0_0 * q0_0_p0_0) + (p1_1_p0_1 * q0_1_p0_1)) + (p1_2_p0_2 * q0_2_p0_2)) + (p1_3_p0_3 * q0_3_p0_3)) + (p1_4_p0_4 * q0_4_p0_4)) + (p1_5_p0_5 * q0_5_p0_5))));
    int int_tmp_result;
    double eps;
    double max1 = fabs(p0_0_p1_0);
    if( (max1 < fabs(p0_1_p1_1)) )
    {
        max1 = fabs(p0_1_p1_1);
    } 
    if( (max1 < fabs(p0_2_p1_2)) )
    {
        max1 = fabs(p0_2_p1_2);
    } 
    if( (max1 < fabs(p0_3_p1_3)) )
    {
        max1 = fabs(p0_3_p1_3);
    } 
    if( (max1 < fabs(p0_4_p1_4)) )
    {
        max1 = fabs(p0_4_p1_4);
    } 
    if( (max1 < fabs(p0_5_p1_5)) )
    {
        max1 = fabs(p0_5_p1_5);
    } 
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
    double max2 = fabs(p0_0_p1_0);
    if( (max2 < fabs(p0_1_p1_1)) )
    {
        max2 = fabs(p0_1_p1_1);
    } 
    if( (max2 < fabs(p0_2_p1_2)) )
    {
        max2 = fabs(p0_2_p1_2);
    } 
    if( (max2 < fabs(p0_3_p1_3)) )
    {
        max2 = fabs(p0_3_p1_3);
    } 
    if( (max2 < fabs(p0_4_p1_4)) )
    {
        max2 = fabs(p0_4_p1_4);
    } 
    if( (max2 < fabs(p0_5_p1_5)) )
    {
        max2 = fabs(p0_5_p1_5);
    } 
    if( (max2 < fabs(q0_0_p0_0)) )
    {
        max2 = fabs(q0_0_p0_0);
    } 
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
    if( (lower_bound_1 < 1.41511993781011659868e-147) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 5.59936185544450928309e+101) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (1.11111223981318615596e-14 * (max1 * max2));
        if( (r > eps) )
        {
            int_tmp_result = 1;
        } 
        else 
        {
            if( (r < -eps) )
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


inline int side1_7d_filter( const double* p0, const double* p1, const double* q0) {
    double p0_0_p1_0 = (p0[0] - p1[0]);
    double p0_1_p1_1 = (p0[1] - p1[1]);
    double p0_2_p1_2 = (p0[2] - p1[2]);
    double p0_3_p1_3 = (p0[3] - p1[3]);
    double p0_4_p1_4 = (p0[4] - p1[4]);
    double p0_5_p1_5 = (p0[5] - p1[5]);
    double p0_6_p1_6 = (p0[6] - p1[6]);
    double r;
    r = (1 * (((((((p0_0_p1_0 * p0_0_p1_0) + (p0_1_p1_1 * p0_1_p1_1)) + (p0_2_p1_2 * p0_2_p1_2)) + (p0_3_p1_3 * p0_3_p1_3)) + (p0_4_p1_4 * p0_4_p1_4)) + (p0_5_p1_5 * p0_5_p1_5)) + (p0_6_p1_6 * p0_6_p1_6)));
    double p1_0_p0_0 = (p1[0] - p0[0]);
    double q0_0_p0_0 = (q0[0] - p0[0]);
    double p1_1_p0_1 = (p1[1] - p0[1]);
    double q0_1_p0_1 = (q0[1] - p0[1]);
    double p1_2_p0_2 = (p1[2] - p0[2]);
    double q0_2_p0_2 = (q0[2] - p0[2]);
    double p1_3_p0_3 = (p1[3] - p0[3]);
    double q0_3_p0_3 = (q0[3] - p0[3]);
    double p1_4_p0_4 = (p1[4] - p0[4]);
    double q0_4_p0_4 = (q0[4] - p0[4]);
    double p1_5_p0_5 = (p1[5] - p0[5]);
    double q0_5_p0_5 = (q0[5] - p0[5]);
    double p1_6_p0_6 = (p1[6] - p0[6]);
    double q0_6_p0_6 = (q0[6] - p0[6]);
    r = (r - (2 * (((((((p1_0_p0_0 * q0_0_p0_0) + (p1_1_p0_1 * q0_1_p0_1)) + (p1_2_p0_2 * q0_2_p0_2)) + (p1_3_p0_3 * q0_3_p0_3)) + (p1_4_p0_4 * q0_4_p0_4)) + (p1_5_p0_5 * q0_5_p0_5)) + (p1_6_p0_6 * q0_6_p0_6))));
    int int_tmp_result;
    double eps;
    double max1 = fabs(p0_0_p1_0);
    if( (max1 < fabs(p0_1_p1_1)) )
    {
        max1 = fabs(p0_1_p1_1);
    } 
    if( (max1 < fabs(p0_2_p1_2)) )
    {
        max1 = fabs(p0_2_p1_2);
    } 
    if( (max1 < fabs(p0_3_p1_3)) )
    {
        max1 = fabs(p0_3_p1_3);
    } 
    if( (max1 < fabs(p0_4_p1_4)) )
    {
        max1 = fabs(p0_4_p1_4);
    } 
    if( (max1 < fabs(p0_5_p1_5)) )
    {
        max1 = fabs(p0_5_p1_5);
    } 
    if( (max1 < fabs(p0_6_p1_6)) )
    {
        max1 = fabs(p0_6_p1_6);
    } 
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
    double max2 = fabs(p0_0_p1_0);
    if( (max2 < fabs(p0_1_p1_1)) )
    {
        max2 = fabs(p0_1_p1_1);
    } 
    if( (max2 < fabs(p0_2_p1_2)) )
    {
        max2 = fabs(p0_2_p1_2);
    } 
    if( (max2 < fabs(p0_3_p1_3)) )
    {
        max2 = fabs(p0_3_p1_3);
    } 
    if( (max2 < fabs(p0_4_p1_4)) )
    {
        max2 = fabs(p0_4_p1_4);
    } 
    if( (max2 < fabs(p0_5_p1_5)) )
    {
        max2 = fabs(p0_5_p1_5);
    } 
    if( (max2 < fabs(p0_6_p1_6)) )
    {
        max2 = fabs(p0_6_p1_6);
    } 
    if( (max2 < fabs(q0_0_p0_0)) )
    {
        max2 = fabs(q0_0_p0_0);
    } 
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
    if( (lower_bound_1 < 1.27080861580266953580e-147) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 5.59936185544450928309e+101) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (1.37779349582504943796e-14 * (max1 * max2));
        if( (r > eps) )
        {
            int_tmp_result = 1;
        } 
        else 
        {
            if( (r < -eps) )
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


inline int side1_8d_filter( const double* p0, const double* p1, const double* q0) {
    double p0_0_p1_0 = (p0[0] - p1[0]);
    double p0_1_p1_1 = (p0[1] - p1[1]);
    double p0_2_p1_2 = (p0[2] - p1[2]);
    double p0_3_p1_3 = (p0[3] - p1[3]);
    double p0_4_p1_4 = (p0[4] - p1[4]);
    double p0_5_p1_5 = (p0[5] - p1[5]);
    double p0_6_p1_6 = (p0[6] - p1[6]);
    double p0_7_p1_7 = (p0[7] - p1[7]);
    double r;
    r = (1 * ((((((((p0_0_p1_0 * p0_0_p1_0) + (p0_1_p1_1 * p0_1_p1_1)) + (p0_2_p1_2 * p0_2_p1_2)) + (p0_3_p1_3 * p0_3_p1_3)) + (p0_4_p1_4 * p0_4_p1_4)) + (p0_5_p1_5 * p0_5_p1_5)) + (p0_6_p1_6 * p0_6_p1_6)) + (p0_7_p1_7 * p0_7_p1_7)));
    double p1_0_p0_0 = (p1[0] - p0[0]);
    double q0_0_p0_0 = (q0[0] - p0[0]);
    double p1_1_p0_1 = (p1[1] - p0[1]);
    double q0_1_p0_1 = (q0[1] - p0[1]);
    double p1_2_p0_2 = (p1[2] - p0[2]);
    double q0_2_p0_2 = (q0[2] - p0[2]);
    double p1_3_p0_3 = (p1[3] - p0[3]);
    double q0_3_p0_3 = (q0[3] - p0[3]);
    double p1_4_p0_4 = (p1[4] - p0[4]);
    double q0_4_p0_4 = (q0[4] - p0[4]);
    double p1_5_p0_5 = (p1[5] - p0[5]);
    double q0_5_p0_5 = (q0[5] - p0[5]);
    double p1_6_p0_6 = (p1[6] - p0[6]);
    double q0_6_p0_6 = (q0[6] - p0[6]);
    double p1_7_p0_7 = (p1[7] - p0[7]);
    double q0_7_p0_7 = (q0[7] - p0[7]);
    r = (r - (2 * ((((((((p1_0_p0_0 * q0_0_p0_0) + (p1_1_p0_1 * q0_1_p0_1)) + (p1_2_p0_2 * q0_2_p0_2)) + (p1_3_p0_3 * q0_3_p0_3)) + (p1_4_p0_4 * q0_4_p0_4)) + (p1_5_p0_5 * q0_5_p0_5)) + (p1_6_p0_6 * q0_6_p0_6)) + (p1_7_p0_7 * q0_7_p0_7))));
    int int_tmp_result;
    double eps;
    double max1 = fabs(p0_1_p1_1);
    if( (max1 < fabs(p0_2_p1_2)) )
    {
        max1 = fabs(p0_2_p1_2);
    } 
    if( (max1 < fabs(p1_0_p0_0)) )
    {
        max1 = fabs(p1_0_p0_0);
    } 
    if( (max1 < fabs(p0_0_p1_0)) )
    {
        max1 = fabs(p0_0_p1_0);
    } 
    if( (max1 < fabs(p0_3_p1_3)) )
    {
        max1 = fabs(p0_3_p1_3);
    } 
    if( (max1 < fabs(p0_4_p1_4)) )
    {
        max1 = fabs(p0_4_p1_4);
    } 
    if( (max1 < fabs(p0_5_p1_5)) )
    {
        max1 = fabs(p0_5_p1_5);
    } 
    if( (max1 < fabs(p0_6_p1_6)) )
    {
        max1 = fabs(p0_6_p1_6);
    } 
    if( (max1 < fabs(p0_7_p1_7)) )
    {
        max1 = fabs(p0_7_p1_7);
    } 
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
    if( (max1 < fabs(p1_7_p0_7)) )
    {
        max1 = fabs(p1_7_p0_7);
    } 
    double max2 = fabs(p0_1_p1_1);
    if( (max2 < fabs(p0_2_p1_2)) )
    {
        max2 = fabs(p0_2_p1_2);
    } 
    if( (max2 < fabs(p0_0_p1_0)) )
    {
        max2 = fabs(p0_0_p1_0);
    } 
    if( (max2 < fabs(p0_3_p1_3)) )
    {
        max2 = fabs(p0_3_p1_3);
    } 
    if( (max2 < fabs(p0_4_p1_4)) )
    {
        max2 = fabs(p0_4_p1_4);
    } 
    if( (max2 < fabs(p0_5_p1_5)) )
    {
        max2 = fabs(p0_5_p1_5);
    } 
    if( (max2 < fabs(p0_6_p1_6)) )
    {
        max2 = fabs(p0_6_p1_6);
    } 
    if( (max2 < fabs(p0_7_p1_7)) )
    {
        max2 = fabs(p0_7_p1_7);
    } 
    if( (max2 < fabs(q0_0_p0_0)) )
    {
        max2 = fabs(q0_0_p0_0);
    } 
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
    if( (lower_bound_1 < 1.15542931091530087067e-147) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 5.59936185544450928309e+101) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (1.66670090166682227006e-14 * (max1 * max2));
        if( (r > eps) )
        {
            int_tmp_result = 1;
        } 
        else 
        {
            if( (r < -eps) )
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

