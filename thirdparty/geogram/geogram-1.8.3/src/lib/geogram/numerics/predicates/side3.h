/* Automatically generated code, do not edit */
/* Generated from source file: side3.pck */

inline int side3_2d_filter( const double* p0, const double* p1, const double* p2, const double* p3, const double* q0, const double* q1, const double* q2) {
    double p1_0_p0_0 = (p1[0] - p0[0]);
    double p1_1_p0_1 = (p1[1] - p0[1]);
    double l1;
    l1 = (1 * ((p1_0_p0_0 * p1_0_p0_0) + (p1_1_p0_1 * p1_1_p0_1)));
    double p2_0_p0_0 = (p2[0] - p0[0]);
    double p2_1_p0_1 = (p2[1] - p0[1]);
    double l2;
    l2 = (1 * ((p2_0_p0_0 * p2_0_p0_0) + (p2_1_p0_1 * p2_1_p0_1)));
    double p3_0_p0_0 = (p3[0] - p0[0]);
    double p3_1_p0_1 = (p3[1] - p0[1]);
    double l3;
    l3 = (1 * ((p3_0_p0_0 * p3_0_p0_0) + (p3_1_p0_1 * p3_1_p0_1)));
    double q0_0_p0_0 = (q0[0] - p0[0]);
    double q0_1_p0_1 = (q0[1] - p0[1]);
    double a10;
    a10 = (2 * ((p1_0_p0_0 * q0_0_p0_0) + (p1_1_p0_1 * q0_1_p0_1)));
    double q1_0_p0_0 = (q1[0] - p0[0]);
    double q1_1_p0_1 = (q1[1] - p0[1]);
    double a11;
    a11 = (2 * ((p1_0_p0_0 * q1_0_p0_0) + (p1_1_p0_1 * q1_1_p0_1)));
    double q2_0_p0_0 = (q2[0] - p0[0]);
    double q2_1_p0_1 = (q2[1] - p0[1]);
    double a12;
    a12 = (2 * ((p1_0_p0_0 * q2_0_p0_0) + (p1_1_p0_1 * q2_1_p0_1)));
    double a20;
    a20 = (2 * ((p2_0_p0_0 * q0_0_p0_0) + (p2_1_p0_1 * q0_1_p0_1)));
    double a21;
    a21 = (2 * ((p2_0_p0_0 * q1_0_p0_0) + (p2_1_p0_1 * q1_1_p0_1)));
    double a22;
    a22 = (2 * ((p2_0_p0_0 * q2_0_p0_0) + (p2_1_p0_1 * q2_1_p0_1)));
    double a30;
    a30 = (2 * ((p3_0_p0_0 * q0_0_p0_0) + (p3_1_p0_1 * q0_1_p0_1)));
    double a31;
    a31 = (2 * ((p3_0_p0_0 * q1_0_p0_0) + (p3_1_p0_1 * q1_1_p0_1)));
    double a32;
    a32 = (2 * ((p3_0_p0_0 * q2_0_p0_0) + (p3_1_p0_1 * q2_1_p0_1)));
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
    double max1 = fabs(p2_0_p0_0);
    if( (max1 < fabs(p2_1_p0_1)) )
    {
        max1 = fabs(p2_1_p0_1);
    } 
    double max2 = fabs(q0_0_p0_0);
    if( (max2 < fabs(q0_1_p0_1)) )
    {
        max2 = fabs(q0_1_p0_1);
    } 
    if( (max2 < fabs(q1_0_p0_0)) )
    {
        max2 = fabs(q1_0_p0_0);
    } 
    if( (max2 < fabs(q1_1_p0_1)) )
    {
        max2 = fabs(q1_1_p0_1);
    } 
    if( (max2 < fabs(q2_0_p0_0)) )
    {
        max2 = fabs(q2_0_p0_0);
    } 
    if( (max2 < fabs(q2_1_p0_1)) )
    {
        max2 = fabs(q2_1_p0_1);
    } 
    double max3 = fabs(p1_0_p0_0);
    if( (max3 < fabs(p1_1_p0_1)) )
    {
        max3 = fabs(p1_1_p0_1);
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
    if( (lower_bound_1 < 2.79532528033945620759e-74) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 2.59614842926741294957e+33) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (3.64430756537603111258e-14 * (((max3 * max2) * max1) * max2));
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
    if( (max4 < max1) )
    {
        max4 = max1;
    } 
    if( (max4 < max3) )
    {
        max4 = max3;
    } 
    double max5 = max2;
    if( (max5 < max3) )
    {
        max5 = max3;
    } 
    double max6 = max2;
    if( (max6 < max3) )
    {
        max6 = max3;
    } 
    if( (max5 < max6) )
    {
        max5 = max6;
    } 
    double max7 = max3;
    if( (max7 < fabs(p3_1_p0_1)) )
    {
        max7 = fabs(p3_1_p0_1);
    } 
    if( (max7 < fabs(p3_0_p0_0)) )
    {
        max7 = fabs(p3_0_p0_0);
    } 
    if( (max5 < max7) )
    {
        max5 = max7;
    } 
    if( (max4 < max5) )
    {
        max4 = max5;
    } 
    if( (max4 < max6) )
    {
        max4 = max6;
    } 
    if( (max4 < max7) )
    {
        max4 = max7;
    } 
    int r_sign;
    int int_tmp_result_FFWKCAA;
    lower_bound_1 = max2;
    upper_bound_1 = max2;
    if( (max1 < lower_bound_1) )
    {
        lower_bound_1 = max1;
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
    if( (max6 < lower_bound_1) )
    {
        lower_bound_1 = max6;
    } 
    if( (max7 < lower_bound_1) )
    {
        lower_bound_1 = max7;
    } 
    if( (lower_bound_1 < 6.01986729486167248087e-50) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 2.59614842926741294957e+33) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (4.67544471613800658534e-13 * (((((max7 * max2) * max1) * max6) * max5) * max4));
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


inline int side3_3d_filter( const double* p0, const double* p1, const double* p2, const double* p3, const double* q0, const double* q1, const double* q2) {
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
    double p3_0_p0_0 = (p3[0] - p0[0]);
    double p3_1_p0_1 = (p3[1] - p0[1]);
    double p3_2_p0_2 = (p3[2] - p0[2]);
    double l3;
    l3 = (1 * (((p3_0_p0_0 * p3_0_p0_0) + (p3_1_p0_1 * p3_1_p0_1)) + (p3_2_p0_2 * p3_2_p0_2)));
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
    double q2_0_p0_0 = (q2[0] - p0[0]);
    double q2_1_p0_1 = (q2[1] - p0[1]);
    double q2_2_p0_2 = (q2[2] - p0[2]);
    double a12;
    a12 = (2 * (((p1_0_p0_0 * q2_0_p0_0) + (p1_1_p0_1 * q2_1_p0_1)) + (p1_2_p0_2 * q2_2_p0_2)));
    double a20;
    a20 = (2 * (((p2_0_p0_0 * q0_0_p0_0) + (p2_1_p0_1 * q0_1_p0_1)) + (p2_2_p0_2 * q0_2_p0_2)));
    double a21;
    a21 = (2 * (((p2_0_p0_0 * q1_0_p0_0) + (p2_1_p0_1 * q1_1_p0_1)) + (p2_2_p0_2 * q1_2_p0_2)));
    double a22;
    a22 = (2 * (((p2_0_p0_0 * q2_0_p0_0) + (p2_1_p0_1 * q2_1_p0_1)) + (p2_2_p0_2 * q2_2_p0_2)));
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
    double max1 = fabs(p1_1_p0_1);
    if( (max1 < fabs(p1_2_p0_2)) )
    {
        max1 = fabs(p1_2_p0_2);
    } 
    if( (max1 < fabs(p1_0_p0_0)) )
    {
        max1 = fabs(p1_0_p0_0);
    } 
    double max2 = fabs(q0_2_p0_2);
    if( (max2 < fabs(q0_0_p0_0)) )
    {
        max2 = fabs(q0_0_p0_0);
    } 
    if( (max2 < fabs(q0_1_p0_1)) )
    {
        max2 = fabs(q0_1_p0_1);
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
    if( (max2 < fabs(q2_0_p0_0)) )
    {
        max2 = fabs(q2_0_p0_0);
    } 
    if( (max2 < fabs(q2_1_p0_1)) )
    {
        max2 = fabs(q2_1_p0_1);
    } 
    if( (max2 < fabs(q2_2_p0_2)) )
    {
        max2 = fabs(q2_2_p0_2);
    } 
    double max3 = fabs(p2_2_p0_2);
    if( (max3 < fabs(p2_0_p0_0)) )
    {
        max3 = fabs(p2_0_p0_0);
    } 
    if( (max3 < fabs(p2_1_p0_1)) )
    {
        max3 = fabs(p2_1_p0_1);
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
        eps = (8.99983341597279045654e-14 * (((max1 * max2) * max3) * max2));
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
    double max4 = max1;
    if( (max4 < max2) )
    {
        max4 = max2;
    } 
    if( (max4 < max3) )
    {
        max4 = max3;
    } 
    double max5 = max1;
    if( (max5 < max2) )
    {
        max5 = max2;
    } 
    double max6 = max1;
    if( (max6 < fabs(p3_0_p0_0)) )
    {
        max6 = fabs(p3_0_p0_0);
    } 
    if( (max6 < fabs(p3_1_p0_1)) )
    {
        max6 = fabs(p3_1_p0_1);
    } 
    if( (max6 < fabs(p3_2_p0_2)) )
    {
        max6 = fabs(p3_2_p0_2);
    } 
    if( (max5 < max6) )
    {
        max5 = max6;
    } 
    double max7 = max1;
    if( (max7 < max2) )
    {
        max7 = max2;
    } 
    if( (max5 < max7) )
    {
        max5 = max7;
    } 
    if( (max4 < max5) )
    {
        max4 = max5;
    } 
    if( (max4 < max6) )
    {
        max4 = max6;
    } 
    if( (max4 < max7) )
    {
        max4 = max7;
    } 
    int r_sign;
    int int_tmp_result_FFWKCAA;
    lower_bound_1 = max2;
    upper_bound_1 = max2;
    if( (max3 < lower_bound_1) )
    {
        lower_bound_1 = max3;
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
    if( (max6 < lower_bound_1) )
    {
        lower_bound_1 = max6;
    } 
    if( (max7 < lower_bound_1) )
    {
        lower_bound_1 = max7;
    } 
    if( (lower_bound_1 < 4.84416636653081796592e-50) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 2.59614842926741294957e+33) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (1.72198804259438718181e-12 * (((((max6 * max2) * max3) * max7) * max5) * max4));
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


inline int side3_4d_filter( const double* p0, const double* p1, const double* p2, const double* p3, const double* q0, const double* q1, const double* q2) {
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
    double a20;
    a20 = (2 * ((((p2_0_p0_0 * q0_0_p0_0) + (p2_1_p0_1 * q0_1_p0_1)) + (p2_2_p0_2 * q0_2_p0_2)) + (p2_3_p0_3 * q0_3_p0_3)));
    double a21;
    a21 = (2 * ((((p2_0_p0_0 * q1_0_p0_0) + (p2_1_p0_1 * q1_1_p0_1)) + (p2_2_p0_2 * q1_2_p0_2)) + (p2_3_p0_3 * q1_3_p0_3)));
    double a22;
    a22 = (2 * ((((p2_0_p0_0 * q2_0_p0_0) + (p2_1_p0_1 * q2_1_p0_1)) + (p2_2_p0_2 * q2_2_p0_2)) + (p2_3_p0_3 * q2_3_p0_3)));
    double a30;
    a30 = (2 * ((((p3_0_p0_0 * q0_0_p0_0) + (p3_1_p0_1 * q0_1_p0_1)) + (p3_2_p0_2 * q0_2_p0_2)) + (p3_3_p0_3 * q0_3_p0_3)));
    double a31;
    a31 = (2 * ((((p3_0_p0_0 * q1_0_p0_0) + (p3_1_p0_1 * q1_1_p0_1)) + (p3_2_p0_2 * q1_2_p0_2)) + (p3_3_p0_3 * q1_3_p0_3)));
    double a32;
    a32 = (2 * ((((p3_0_p0_0 * q2_0_p0_0) + (p3_1_p0_1 * q2_1_p0_1)) + (p3_2_p0_2 * q2_2_p0_2)) + (p3_3_p0_3 * q2_3_p0_3)));
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
    double max1 = fabs(p1_3_p0_3);
    if( (max1 < fabs(p1_1_p0_1)) )
    {
        max1 = fabs(p1_1_p0_1);
    } 
    if( (max1 < fabs(p1_2_p0_2)) )
    {
        max1 = fabs(p1_2_p0_2);
    } 
    if( (max1 < fabs(p1_0_p0_0)) )
    {
        max1 = fabs(p1_0_p0_0);
    } 
    double max2 = fabs(p2_3_p0_3);
    if( (max2 < fabs(p2_2_p0_2)) )
    {
        max2 = fabs(p2_2_p0_2);
    } 
    if( (max2 < fabs(p2_0_p0_0)) )
    {
        max2 = fabs(p2_0_p0_0);
    } 
    if( (max2 < fabs(p2_1_p0_1)) )
    {
        max2 = fabs(p2_1_p0_1);
    } 
    double max3 = fabs(q0_1_p0_1);
    if( (max3 < fabs(q0_0_p0_0)) )
    {
        max3 = fabs(q0_0_p0_0);
    } 
    if( (max3 < fabs(q0_2_p0_2)) )
    {
        max3 = fabs(q0_2_p0_2);
    } 
    if( (max3 < fabs(q0_3_p0_3)) )
    {
        max3 = fabs(q0_3_p0_3);
    } 
    if( (max3 < fabs(q1_0_p0_0)) )
    {
        max3 = fabs(q1_0_p0_0);
    } 
    if( (max3 < fabs(q1_1_p0_1)) )
    {
        max3 = fabs(q1_1_p0_1);
    } 
    if( (max3 < fabs(q1_2_p0_2)) )
    {
        max3 = fabs(q1_2_p0_2);
    } 
    if( (max3 < fabs(q1_3_p0_3)) )
    {
        max3 = fabs(q1_3_p0_3);
    } 
    if( (max3 < fabs(q2_0_p0_0)) )
    {
        max3 = fabs(q2_0_p0_0);
    } 
    if( (max3 < fabs(q2_1_p0_1)) )
    {
        max3 = fabs(q2_1_p0_1);
    } 
    if( (max3 < fabs(q2_2_p0_2)) )
    {
        max3 = fabs(q2_2_p0_2);
    } 
    if( (max3 < fabs(q2_3_p0_3)) )
    {
        max3 = fabs(q2_3_p0_3);
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
    if( (lower_bound_1 < 1.89528395402941802921e-74) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 1.29807421463370647479e+33) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (1.72443682410931985179e-13 * (((max1 * max3) * max2) * max3));
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
    double max4 = max1;
    double max5 = max1;
    double max6 = max1;
    if( (max6 < fabs(p3_0_p0_0)) )
    {
        max6 = fabs(p3_0_p0_0);
    } 
    if( (max6 < fabs(p3_3_p0_3)) )
    {
        max6 = fabs(p3_3_p0_3);
    } 
    if( (max6 < fabs(p3_2_p0_2)) )
    {
        max6 = fabs(p3_2_p0_2);
    } 
    if( (max6 < fabs(p3_1_p0_1)) )
    {
        max6 = fabs(p3_1_p0_1);
    } 
    if( (max5 < max6) )
    {
        max5 = max6;
    } 
    if( (max5 < max3) )
    {
        max5 = max3;
    } 
    double max7 = max1;
    if( (max7 < max3) )
    {
        max7 = max3;
    } 
    if( (max5 < max7) )
    {
        max5 = max7;
    } 
    if( (max4 < max5) )
    {
        max4 = max5;
    } 
    if( (max4 < max6) )
    {
        max4 = max6;
    } 
    if( (max4 < max2) )
    {
        max4 = max2;
    } 
    if( (max4 < max3) )
    {
        max4 = max3;
    } 
    if( (max4 < max7) )
    {
        max4 = max7;
    } 
    int r_sign;
    int int_tmp_result_FFWKCAA;
    lower_bound_1 = max5;
    upper_bound_1 = max5;
    if( (max6 < lower_bound_1) )
    {
        lower_bound_1 = max6;
    } 
    if( (max2 < lower_bound_1) )
    {
        lower_bound_1 = max2;
    } 
    if( (max3 < lower_bound_1) )
    {
        lower_bound_1 = max3;
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
    if( (lower_bound_1 < 4.14607644401726239868e-50) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 1.29807421463370647479e+33) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (4.38046888801178809320e-12 * (((((max6 * max3) * max2) * max7) * max5) * max4));
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


inline int side3_6d_filter( const double* p0, const double* p1, const double* p2, const double* p3, const double* q0, const double* q1, const double* q2) {
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
    double a20;
    a20 = (2 * ((((((p2_0_p0_0 * q0_0_p0_0) + (p2_1_p0_1 * q0_1_p0_1)) + (p2_2_p0_2 * q0_2_p0_2)) + (p2_3_p0_3 * q0_3_p0_3)) + (p2_4_p0_4 * q0_4_p0_4)) + (p2_5_p0_5 * q0_5_p0_5)));
    double a21;
    a21 = (2 * ((((((p2_0_p0_0 * q1_0_p0_0) + (p2_1_p0_1 * q1_1_p0_1)) + (p2_2_p0_2 * q1_2_p0_2)) + (p2_3_p0_3 * q1_3_p0_3)) + (p2_4_p0_4 * q1_4_p0_4)) + (p2_5_p0_5 * q1_5_p0_5)));
    double a22;
    a22 = (2 * ((((((p2_0_p0_0 * q2_0_p0_0) + (p2_1_p0_1 * q2_1_p0_1)) + (p2_2_p0_2 * q2_2_p0_2)) + (p2_3_p0_3 * q2_3_p0_3)) + (p2_4_p0_4 * q2_4_p0_4)) + (p2_5_p0_5 * q2_5_p0_5)));
    double a30;
    a30 = (2 * ((((((p3_0_p0_0 * q0_0_p0_0) + (p3_1_p0_1 * q0_1_p0_1)) + (p3_2_p0_2 * q0_2_p0_2)) + (p3_3_p0_3 * q0_3_p0_3)) + (p3_4_p0_4 * q0_4_p0_4)) + (p3_5_p0_5 * q0_5_p0_5)));
    double a31;
    a31 = (2 * ((((((p3_0_p0_0 * q1_0_p0_0) + (p3_1_p0_1 * q1_1_p0_1)) + (p3_2_p0_2 * q1_2_p0_2)) + (p3_3_p0_3 * q1_3_p0_3)) + (p3_4_p0_4 * q1_4_p0_4)) + (p3_5_p0_5 * q1_5_p0_5)));
    double a32;
    a32 = (2 * ((((((p3_0_p0_0 * q2_0_p0_0) + (p3_1_p0_1 * q2_1_p0_1)) + (p3_2_p0_2 * q2_2_p0_2)) + (p3_3_p0_3 * q2_3_p0_3)) + (p3_4_p0_4 * q2_4_p0_4)) + (p3_5_p0_5 * q2_5_p0_5)));
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
    double max1 = fabs(p1_0_p0_0);
    if( (max1 < fabs(p1_2_p0_2)) )
    {
        max1 = fabs(p1_2_p0_2);
    } 
    if( (max1 < fabs(p1_1_p0_1)) )
    {
        max1 = fabs(p1_1_p0_1);
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
    double max2 = fabs(p2_0_p0_0);
    if( (max2 < fabs(p2_1_p0_1)) )
    {
        max2 = fabs(p2_1_p0_1);
    } 
    if( (max2 < fabs(p2_2_p0_2)) )
    {
        max2 = fabs(p2_2_p0_2);
    } 
    if( (max2 < fabs(p2_3_p0_3)) )
    {
        max2 = fabs(p2_3_p0_3);
    } 
    if( (max2 < fabs(p2_4_p0_4)) )
    {
        max2 = fabs(p2_4_p0_4);
    } 
    if( (max2 < fabs(p2_5_p0_5)) )
    {
        max2 = fabs(p2_5_p0_5);
    } 
    double max3 = fabs(q0_0_p0_0);
    if( (max3 < fabs(q0_1_p0_1)) )
    {
        max3 = fabs(q0_1_p0_1);
    } 
    if( (max3 < fabs(q0_2_p0_2)) )
    {
        max3 = fabs(q0_2_p0_2);
    } 
    if( (max3 < fabs(q0_3_p0_3)) )
    {
        max3 = fabs(q0_3_p0_3);
    } 
    if( (max3 < fabs(q0_4_p0_4)) )
    {
        max3 = fabs(q0_4_p0_4);
    } 
    if( (max3 < fabs(q0_5_p0_5)) )
    {
        max3 = fabs(q0_5_p0_5);
    } 
    if( (max3 < fabs(q1_0_p0_0)) )
    {
        max3 = fabs(q1_0_p0_0);
    } 
    if( (max3 < fabs(q1_1_p0_1)) )
    {
        max3 = fabs(q1_1_p0_1);
    } 
    if( (max3 < fabs(q1_2_p0_2)) )
    {
        max3 = fabs(q1_2_p0_2);
    } 
    if( (max3 < fabs(q1_3_p0_3)) )
    {
        max3 = fabs(q1_3_p0_3);
    } 
    if( (max3 < fabs(q1_4_p0_4)) )
    {
        max3 = fabs(q1_4_p0_4);
    } 
    if( (max3 < fabs(q1_5_p0_5)) )
    {
        max3 = fabs(q1_5_p0_5);
    } 
    if( (max3 < fabs(q2_0_p0_0)) )
    {
        max3 = fabs(q2_0_p0_0);
    } 
    if( (max3 < fabs(q2_1_p0_1)) )
    {
        max3 = fabs(q2_1_p0_1);
    } 
    if( (max3 < fabs(q2_2_p0_2)) )
    {
        max3 = fabs(q2_2_p0_2);
    } 
    if( (max3 < fabs(q2_3_p0_3)) )
    {
        max3 = fabs(q2_3_p0_3);
    } 
    if( (max3 < fabs(q2_4_p0_4)) )
    {
        max3 = fabs(q2_4_p0_4);
    } 
    if( (max3 < fabs(q2_5_p0_5)) )
    {
        max3 = fabs(q2_5_p0_5);
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
    if( (lower_bound_1 < 1.49958502193059513986e-74) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 1.29807421463370647479e+33) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (4.40007476026583916019e-13 * (((max1 * max3) * max2) * max3));
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
    double max4 = max1;
    if( (max4 < max2) )
    {
        max4 = max2;
    } 
    if( (max4 < max3) )
    {
        max4 = max3;
    } 
    double max5 = max1;
    if( (max5 < fabs(p3_1_p0_1)) )
    {
        max5 = fabs(p3_1_p0_1);
    } 
    if( (max5 < fabs(p3_2_p0_2)) )
    {
        max5 = fabs(p3_2_p0_2);
    } 
    if( (max5 < fabs(p3_0_p0_0)) )
    {
        max5 = fabs(p3_0_p0_0);
    } 
    if( (max5 < fabs(p3_3_p0_3)) )
    {
        max5 = fabs(p3_3_p0_3);
    } 
    if( (max5 < fabs(p3_4_p0_4)) )
    {
        max5 = fabs(p3_4_p0_4);
    } 
    if( (max5 < fabs(p3_5_p0_5)) )
    {
        max5 = fabs(p3_5_p0_5);
    } 
    if( (max4 < max5) )
    {
        max4 = max5;
    } 
    double max6 = max1;
    if( (max6 < max3) )
    {
        max6 = max3;
    } 
    if( (max6 < max5) )
    {
        max6 = max5;
    } 
    double max7 = max1;
    if( (max7 < max3) )
    {
        max7 = max3;
    } 
    if( (max6 < max7) )
    {
        max6 = max7;
    } 
    if( (max4 < max6) )
    {
        max4 = max6;
    } 
    if( (max4 < max7) )
    {
        max4 = max7;
    } 
    int r_sign;
    int int_tmp_result_FFWKCAA;
    lower_bound_1 = max2;
    upper_bound_1 = max2;
    if( (max3 < lower_bound_1) )
    {
        lower_bound_1 = max3;
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
    if( (max6 < lower_bound_1) )
    {
        lower_bound_1 = max6;
    } 
    if( (max7 < lower_bound_1) )
    {
        lower_bound_1 = max7;
    } 
    if( (lower_bound_1 < 3.31864264949884013629e-50) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 1.29807421463370647479e+33) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (1.66564133587113197628e-11 * (((((max5 * max3) * max2) * max7) * max6) * max4));
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


inline int side3_7d_filter( const double* p0, const double* p1, const double* p2, const double* p3, const double* q0, const double* q1, const double* q2) {
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
    double a20;
    a20 = (2 * (((((((p2_0_p0_0 * q0_0_p0_0) + (p2_1_p0_1 * q0_1_p0_1)) + (p2_2_p0_2 * q0_2_p0_2)) + (p2_3_p0_3 * q0_3_p0_3)) + (p2_4_p0_4 * q0_4_p0_4)) + (p2_5_p0_5 * q0_5_p0_5)) + (p2_6_p0_6 * q0_6_p0_6)));
    double a21;
    a21 = (2 * (((((((p2_0_p0_0 * q1_0_p0_0) + (p2_1_p0_1 * q1_1_p0_1)) + (p2_2_p0_2 * q1_2_p0_2)) + (p2_3_p0_3 * q1_3_p0_3)) + (p2_4_p0_4 * q1_4_p0_4)) + (p2_5_p0_5 * q1_5_p0_5)) + (p2_6_p0_6 * q1_6_p0_6)));
    double a22;
    a22 = (2 * (((((((p2_0_p0_0 * q2_0_p0_0) + (p2_1_p0_1 * q2_1_p0_1)) + (p2_2_p0_2 * q2_2_p0_2)) + (p2_3_p0_3 * q2_3_p0_3)) + (p2_4_p0_4 * q2_4_p0_4)) + (p2_5_p0_5 * q2_5_p0_5)) + (p2_6_p0_6 * q2_6_p0_6)));
    double a30;
    a30 = (2 * (((((((p3_0_p0_0 * q0_0_p0_0) + (p3_1_p0_1 * q0_1_p0_1)) + (p3_2_p0_2 * q0_2_p0_2)) + (p3_3_p0_3 * q0_3_p0_3)) + (p3_4_p0_4 * q0_4_p0_4)) + (p3_5_p0_5 * q0_5_p0_5)) + (p3_6_p0_6 * q0_6_p0_6)));
    double a31;
    a31 = (2 * (((((((p3_0_p0_0 * q1_0_p0_0) + (p3_1_p0_1 * q1_1_p0_1)) + (p3_2_p0_2 * q1_2_p0_2)) + (p3_3_p0_3 * q1_3_p0_3)) + (p3_4_p0_4 * q1_4_p0_4)) + (p3_5_p0_5 * q1_5_p0_5)) + (p3_6_p0_6 * q1_6_p0_6)));
    double a32;
    a32 = (2 * (((((((p3_0_p0_0 * q2_0_p0_0) + (p3_1_p0_1 * q2_1_p0_1)) + (p3_2_p0_2 * q2_2_p0_2)) + (p3_3_p0_3 * q2_3_p0_3)) + (p3_4_p0_4 * q2_4_p0_4)) + (p3_5_p0_5 * q2_5_p0_5)) + (p3_6_p0_6 * q2_6_p0_6)));
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
    double max1 = fabs(p1_1_p0_1);
    if( (max1 < fabs(p1_3_p0_3)) )
    {
        max1 = fabs(p1_3_p0_3);
    } 
    if( (max1 < fabs(p1_5_p0_5)) )
    {
        max1 = fabs(p1_5_p0_5);
    } 
    if( (max1 < fabs(p1_4_p0_4)) )
    {
        max1 = fabs(p1_4_p0_4);
    } 
    if( (max1 < fabs(p1_6_p0_6)) )
    {
        max1 = fabs(p1_6_p0_6);
    } 
    if( (max1 < fabs(p1_2_p0_2)) )
    {
        max1 = fabs(p1_2_p0_2);
    } 
    if( (max1 < fabs(p1_0_p0_0)) )
    {
        max1 = fabs(p1_0_p0_0);
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
    if( (max2 < fabs(p2_3_p0_3)) )
    {
        max2 = fabs(p2_3_p0_3);
    } 
    if( (max2 < fabs(p2_4_p0_4)) )
    {
        max2 = fabs(p2_4_p0_4);
    } 
    if( (max2 < fabs(p2_5_p0_5)) )
    {
        max2 = fabs(p2_5_p0_5);
    } 
    if( (max2 < fabs(p2_6_p0_6)) )
    {
        max2 = fabs(p2_6_p0_6);
    } 
    double max3 = fabs(q0_0_p0_0);
    if( (max3 < fabs(q0_1_p0_1)) )
    {
        max3 = fabs(q0_1_p0_1);
    } 
    if( (max3 < fabs(q0_2_p0_2)) )
    {
        max3 = fabs(q0_2_p0_2);
    } 
    if( (max3 < fabs(q0_3_p0_3)) )
    {
        max3 = fabs(q0_3_p0_3);
    } 
    if( (max3 < fabs(q0_4_p0_4)) )
    {
        max3 = fabs(q0_4_p0_4);
    } 
    if( (max3 < fabs(q0_5_p0_5)) )
    {
        max3 = fabs(q0_5_p0_5);
    } 
    if( (max3 < fabs(q0_6_p0_6)) )
    {
        max3 = fabs(q0_6_p0_6);
    } 
    if( (max3 < fabs(q1_0_p0_0)) )
    {
        max3 = fabs(q1_0_p0_0);
    } 
    if( (max3 < fabs(q1_1_p0_1)) )
    {
        max3 = fabs(q1_1_p0_1);
    } 
    if( (max3 < fabs(q1_2_p0_2)) )
    {
        max3 = fabs(q1_2_p0_2);
    } 
    if( (max3 < fabs(q1_3_p0_3)) )
    {
        max3 = fabs(q1_3_p0_3);
    } 
    if( (max3 < fabs(q1_4_p0_4)) )
    {
        max3 = fabs(q1_4_p0_4);
    } 
    if( (max3 < fabs(q1_5_p0_5)) )
    {
        max3 = fabs(q1_5_p0_5);
    } 
    if( (max3 < fabs(q1_6_p0_6)) )
    {
        max3 = fabs(q1_6_p0_6);
    } 
    if( (max3 < fabs(q2_0_p0_0)) )
    {
        max3 = fabs(q2_0_p0_0);
    } 
    if( (max3 < fabs(q2_1_p0_1)) )
    {
        max3 = fabs(q2_1_p0_1);
    } 
    if( (max3 < fabs(q2_2_p0_2)) )
    {
        max3 = fabs(q2_2_p0_2);
    } 
    if( (max3 < fabs(q2_3_p0_3)) )
    {
        max3 = fabs(q2_3_p0_3);
    } 
    if( (max3 < fabs(q2_4_p0_4)) )
    {
        max3 = fabs(q2_4_p0_4);
    } 
    if( (max3 < fabs(q2_5_p0_5)) )
    {
        max3 = fabs(q2_5_p0_5);
    } 
    if( (max3 < fabs(q2_6_p0_6)) )
    {
        max3 = fabs(q2_6_p0_6);
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
    if( (lower_bound_1 < 1.36918881183883509035e-74) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 1.29807421463370647479e+33) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (6.33127335329798996022e-13 * (((max1 * max3) * max2) * max3));
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
    double max4;
    double max7 = max1;
    if( (max7 < max3) )
    {
        max7 = max3;
    } 
    max4 = max7;
    if( (max4 < max2) )
    {
        max4 = max2;
    } 
    double max5 = max1;
    if( (max5 < fabs(p3_0_p0_0)) )
    {
        max5 = fabs(p3_0_p0_0);
    } 
    if( (max5 < fabs(p3_1_p0_1)) )
    {
        max5 = fabs(p3_1_p0_1);
    } 
    if( (max5 < fabs(p3_2_p0_2)) )
    {
        max5 = fabs(p3_2_p0_2);
    } 
    if( (max5 < fabs(p3_3_p0_3)) )
    {
        max5 = fabs(p3_3_p0_3);
    } 
    if( (max5 < fabs(p3_4_p0_4)) )
    {
        max5 = fabs(p3_4_p0_4);
    } 
    if( (max5 < fabs(p3_5_p0_5)) )
    {
        max5 = fabs(p3_5_p0_5);
    } 
    if( (max5 < fabs(p3_6_p0_6)) )
    {
        max5 = fabs(p3_6_p0_6);
    } 
    if( (max4 < max5) )
    {
        max4 = max5;
    } 
    if( (max4 < max1) )
    {
        max4 = max1;
    } 
    if( (max4 < max3) )
    {
        max4 = max3;
    } 
    double max6 = max7;
    if( (max6 < max5) )
    {
        max6 = max5;
    } 
    if( (max6 < max1) )
    {
        max6 = max1;
    } 
    if( (max6 < max3) )
    {
        max6 = max3;
    } 
    if( (max4 < max6) )
    {
        max4 = max6;
    } 
    int r_sign;
    int int_tmp_result_FFWKCAA;
    lower_bound_1 = max7;
    upper_bound_1 = max7;
    if( (max2 < lower_bound_1) )
    {
        lower_bound_1 = max2;
    } 
    if( (max5 < lower_bound_1) )
    {
        lower_bound_1 = max5;
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
    if( (max3 < lower_bound_1) )
    {
        lower_bound_1 = max3;
    } 
    if( (max6 < lower_bound_1) )
    {
        lower_bound_1 = max6;
    } 
    if( (lower_bound_1 < 3.04548303565602498901e-50) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 1.29807421463370647479e+33) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (2.78873548804336160566e-11 * (((((max5 * max3) * max2) * max7) * max6) * max4));
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


inline int side3_8d_filter( const double* p0, const double* p1, const double* p2, const double* p3, const double* q0, const double* q1, const double* q2) {
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
    double a20;
    a20 = (2 * ((((((((p2_0_p0_0 * q0_0_p0_0) + (p2_1_p0_1 * q0_1_p0_1)) + (p2_2_p0_2 * q0_2_p0_2)) + (p2_3_p0_3 * q0_3_p0_3)) + (p2_4_p0_4 * q0_4_p0_4)) + (p2_5_p0_5 * q0_5_p0_5)) + (p2_6_p0_6 * q0_6_p0_6)) + (p2_7_p0_7 * q0_7_p0_7)));
    double a21;
    a21 = (2 * ((((((((p2_0_p0_0 * q1_0_p0_0) + (p2_1_p0_1 * q1_1_p0_1)) + (p2_2_p0_2 * q1_2_p0_2)) + (p2_3_p0_3 * q1_3_p0_3)) + (p2_4_p0_4 * q1_4_p0_4)) + (p2_5_p0_5 * q1_5_p0_5)) + (p2_6_p0_6 * q1_6_p0_6)) + (p2_7_p0_7 * q1_7_p0_7)));
    double a22;
    a22 = (2 * ((((((((p2_0_p0_0 * q2_0_p0_0) + (p2_1_p0_1 * q2_1_p0_1)) + (p2_2_p0_2 * q2_2_p0_2)) + (p2_3_p0_3 * q2_3_p0_3)) + (p2_4_p0_4 * q2_4_p0_4)) + (p2_5_p0_5 * q2_5_p0_5)) + (p2_6_p0_6 * q2_6_p0_6)) + (p2_7_p0_7 * q2_7_p0_7)));
    double a30;
    a30 = (2 * ((((((((p3_0_p0_0 * q0_0_p0_0) + (p3_1_p0_1 * q0_1_p0_1)) + (p3_2_p0_2 * q0_2_p0_2)) + (p3_3_p0_3 * q0_3_p0_3)) + (p3_4_p0_4 * q0_4_p0_4)) + (p3_5_p0_5 * q0_5_p0_5)) + (p3_6_p0_6 * q0_6_p0_6)) + (p3_7_p0_7 * q0_7_p0_7)));
    double a31;
    a31 = (2 * ((((((((p3_0_p0_0 * q1_0_p0_0) + (p3_1_p0_1 * q1_1_p0_1)) + (p3_2_p0_2 * q1_2_p0_2)) + (p3_3_p0_3 * q1_3_p0_3)) + (p3_4_p0_4 * q1_4_p0_4)) + (p3_5_p0_5 * q1_5_p0_5)) + (p3_6_p0_6 * q1_6_p0_6)) + (p3_7_p0_7 * q1_7_p0_7)));
    double a32;
    a32 = (2 * ((((((((p3_0_p0_0 * q2_0_p0_0) + (p3_1_p0_1 * q2_1_p0_1)) + (p3_2_p0_2 * q2_2_p0_2)) + (p3_3_p0_3 * q2_3_p0_3)) + (p3_4_p0_4 * q2_4_p0_4)) + (p3_5_p0_5 * q2_5_p0_5)) + (p3_6_p0_6 * q2_6_p0_6)) + (p3_7_p0_7 * q2_7_p0_7)));
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
    double max1 = fabs(p2_1_p0_1);
    if( (max1 < fabs(p2_0_p0_0)) )
    {
        max1 = fabs(p2_0_p0_0);
    } 
    if( (max1 < fabs(p2_3_p0_3)) )
    {
        max1 = fabs(p2_3_p0_3);
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
    if( (max1 < fabs(p2_5_p0_5)) )
    {
        max1 = fabs(p2_5_p0_5);
    } 
    if( (max1 < fabs(p2_6_p0_6)) )
    {
        max1 = fabs(p2_6_p0_6);
    } 
    double max2 = fabs(p1_4_p0_4);
    if( (max2 < fabs(p1_1_p0_1)) )
    {
        max2 = fabs(p1_1_p0_1);
    } 
    if( (max2 < fabs(p1_0_p0_0)) )
    {
        max2 = fabs(p1_0_p0_0);
    } 
    if( (max2 < fabs(p1_3_p0_3)) )
    {
        max2 = fabs(p1_3_p0_3);
    } 
    if( (max2 < fabs(p1_2_p0_2)) )
    {
        max2 = fabs(p1_2_p0_2);
    } 
    if( (max2 < fabs(p1_5_p0_5)) )
    {
        max2 = fabs(p1_5_p0_5);
    } 
    if( (max2 < fabs(p1_6_p0_6)) )
    {
        max2 = fabs(p1_6_p0_6);
    } 
    if( (max2 < fabs(p1_7_p0_7)) )
    {
        max2 = fabs(p1_7_p0_7);
    } 
    double max3 = fabs(q0_0_p0_0);
    if( (max3 < fabs(q0_1_p0_1)) )
    {
        max3 = fabs(q0_1_p0_1);
    } 
    if( (max3 < fabs(q0_2_p0_2)) )
    {
        max3 = fabs(q0_2_p0_2);
    } 
    if( (max3 < fabs(q0_3_p0_3)) )
    {
        max3 = fabs(q0_3_p0_3);
    } 
    if( (max3 < fabs(q0_4_p0_4)) )
    {
        max3 = fabs(q0_4_p0_4);
    } 
    if( (max3 < fabs(q0_5_p0_5)) )
    {
        max3 = fabs(q0_5_p0_5);
    } 
    if( (max3 < fabs(q0_6_p0_6)) )
    {
        max3 = fabs(q0_6_p0_6);
    } 
    if( (max3 < fabs(q0_7_p0_7)) )
    {
        max3 = fabs(q0_7_p0_7);
    } 
    if( (max3 < fabs(q1_0_p0_0)) )
    {
        max3 = fabs(q1_0_p0_0);
    } 
    if( (max3 < fabs(q1_1_p0_1)) )
    {
        max3 = fabs(q1_1_p0_1);
    } 
    if( (max3 < fabs(q1_2_p0_2)) )
    {
        max3 = fabs(q1_2_p0_2);
    } 
    if( (max3 < fabs(q1_3_p0_3)) )
    {
        max3 = fabs(q1_3_p0_3);
    } 
    if( (max3 < fabs(q1_4_p0_4)) )
    {
        max3 = fabs(q1_4_p0_4);
    } 
    if( (max3 < fabs(q1_5_p0_5)) )
    {
        max3 = fabs(q1_5_p0_5);
    } 
    if( (max3 < fabs(q1_6_p0_6)) )
    {
        max3 = fabs(q1_6_p0_6);
    } 
    if( (max3 < fabs(q1_7_p0_7)) )
    {
        max3 = fabs(q1_7_p0_7);
    } 
    if( (max3 < fabs(q2_0_p0_0)) )
    {
        max3 = fabs(q2_0_p0_0);
    } 
    if( (max3 < fabs(q2_1_p0_1)) )
    {
        max3 = fabs(q2_1_p0_1);
    } 
    if( (max3 < fabs(q2_2_p0_2)) )
    {
        max3 = fabs(q2_2_p0_2);
    } 
    if( (max3 < fabs(q2_3_p0_3)) )
    {
        max3 = fabs(q2_3_p0_3);
    } 
    if( (max3 < fabs(q2_4_p0_4)) )
    {
        max3 = fabs(q2_4_p0_4);
    } 
    if( (max3 < fabs(q2_5_p0_5)) )
    {
        max3 = fabs(q2_5_p0_5);
    } 
    if( (max3 < fabs(q2_6_p0_6)) )
    {
        max3 = fabs(q2_6_p0_6);
    } 
    if( (max3 < fabs(q2_7_p0_7)) )
    {
        max3 = fabs(q2_7_p0_7);
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
    if( (lower_bound_1 < 1.26419510663115923609e-74) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 1.29807421463370647479e+33) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (8.71140112255785451890e-13 * (((max2 * max3) * max1) * max3));
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
    double max4 = max1;
    if( (max4 < max2) )
    {
        max4 = max2;
    } 
    if( (max4 < max3) )
    {
        max4 = max3;
    } 
    double max5 = max2;
    if( (max5 < fabs(p3_0_p0_0)) )
    {
        max5 = fabs(p3_0_p0_0);
    } 
    if( (max5 < fabs(p3_1_p0_1)) )
    {
        max5 = fabs(p3_1_p0_1);
    } 
    if( (max5 < fabs(p3_2_p0_2)) )
    {
        max5 = fabs(p3_2_p0_2);
    } 
    if( (max5 < fabs(p3_3_p0_3)) )
    {
        max5 = fabs(p3_3_p0_3);
    } 
    if( (max5 < fabs(p3_4_p0_4)) )
    {
        max5 = fabs(p3_4_p0_4);
    } 
    if( (max5 < fabs(p3_5_p0_5)) )
    {
        max5 = fabs(p3_5_p0_5);
    } 
    if( (max5 < fabs(p3_6_p0_6)) )
    {
        max5 = fabs(p3_6_p0_6);
    } 
    if( (max5 < fabs(p3_7_p0_7)) )
    {
        max5 = fabs(p3_7_p0_7);
    } 
    if( (max4 < max5) )
    {
        max4 = max5;
    } 
    double max6 = max2;
    if( (max6 < max3) )
    {
        max6 = max3;
    } 
    if( (max6 < max5) )
    {
        max6 = max5;
    } 
    double max7 = max2;
    if( (max7 < max3) )
    {
        max7 = max3;
    } 
    if( (max6 < max7) )
    {
        max6 = max7;
    } 
    if( (max4 < max6) )
    {
        max4 = max6;
    } 
    if( (max4 < max7) )
    {
        max4 = max7;
    } 
    int r_sign;
    int int_tmp_result_FFWKCAA;
    lower_bound_1 = max1;
    upper_bound_1 = max1;
    if( (max3 < lower_bound_1) )
    {
        lower_bound_1 = max3;
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
    if( (max6 < lower_bound_1) )
    {
        lower_bound_1 = max6;
    } 
    if( (max7 < lower_bound_1) )
    {
        lower_bound_1 = max7;
    } 
    if( (lower_bound_1 < 2.82528483194754087282e-50) )
    {
        return FPG_UNCERTAIN_VALUE;
    } 
    else 
    {
        if( (upper_bound_1 > 1.29807421463370647479e+33) )
        {
            return FPG_UNCERTAIN_VALUE;
        } 
        eps = (4.37492894694731169807e-11 * (((((max5 * max3) * max1) * max7) * max6) * max4));
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

