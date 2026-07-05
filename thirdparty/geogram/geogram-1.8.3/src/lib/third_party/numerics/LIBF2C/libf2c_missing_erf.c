#include <math.h>


/*
 * VisualC++ versions older than MSVC2013
 * do not have erf() and erfc(),
 * therefore we include here an implementation.
 */
#ifdef _MSC_VER
#if _MSC_VER < 1800
#define MISSING_ERF
#endif
#endif


#ifdef MISSING_ERF

/*
 * grabbed from:
 * http://www.johndcook.com/blog/cpp_erf/
 * License: BSD
 */
double erf(double x) {
    /* constants */
    double a1 =  0.254829592;
    double a2 = -0.284496736;
    double a3 =  1.421413741;
    double a4 = -1.453152027;
    double a5 =  1.061405429;
    double p  =  0.3275911;

    /* Save the sign of x */
    int sign = 1;
    double t,y;
    
    if (x < 0)
        sign = -1;
    x = fabs(x);

    /* A&S formula 7.1.26 */
    t = 1.0/(1.0 + p*x);
    y = 1.0 - (((((a5*t + a4)*t) + a3)*t + a2)*t + a1)*t*exp(-x*x);

    return sign*y;
}

double erfc(double x) {
    return 1.0 - erf(x);
}


#endif
