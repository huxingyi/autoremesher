#ifndef BLAS_H
#define BLAS_H

#if defined(_WIN32)
#if defined(EIGEN_BLAS_BUILD_DLL)
#define EIGEN_BLAS_API __declspec(dllexport)
#elif defined(EIGEN_BLAS_LINK_DLL)
#define EIGEN_BLAS_API __declspec(dllimport)
#else
#define EIGEN_BLAS_API
#endif
#elif ((defined(__GNUC__) && __GNUC__ >= 4) || defined(__clang__)) && defined(EIGEN_BLAS_BUILD_DLL)
#define EIGEN_BLAS_API __attribute__((visibility("default")))
#else
#define EIGEN_BLAS_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define BLASFUNC(FUNC) FUNC##_

#ifdef __WIN64__
typedef long long BLASLONG;
typedef unsigned long long BLASULONG;
#else
typedef long BLASLONG;
typedef unsigned long BLASULONG;
#endif

EIGEN_BLAS_API int BLASFUNC(lsame)(const char *, const char *);
EIGEN_BLAS_API void BLASFUNC(xerbla)(const char *, int *info);

EIGEN_BLAS_API float BLASFUNC(sdot)(int *, float *, int *, float *, int *);
EIGEN_BLAS_API float BLASFUNC(sdsdot)(int *, float *, float *, int *, float *, int *);

EIGEN_BLAS_API double BLASFUNC(dsdot)(int *, float *, int *, float *, int *);
EIGEN_BLAS_API double BLASFUNC(ddot)(int *, double *, int *, double *, int *);
EIGEN_BLAS_API double BLASFUNC(qdot)(int *, double *, int *, double *, int *);

EIGEN_BLAS_API void BLASFUNC(cdotu)(int *, float *, int *, float *, int *);
EIGEN_BLAS_API void BLASFUNC(cdotc)(int *, float *, int *, float *, int *);
EIGEN_BLAS_API void BLASFUNC(zdotu)(int *, double *, int *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(zdotc)(int *, double *, int *, double *, int *);

EIGEN_BLAS_API void BLASFUNC(cdotuw)(int *, float *, int *, float *, int *, float *);
EIGEN_BLAS_API void BLASFUNC(cdotcw)(int *, float *, int *, float *, int *, float *);
EIGEN_BLAS_API void BLASFUNC(zdotuw)(int *, double *, int *, double *, int *, double *);
EIGEN_BLAS_API void BLASFUNC(zdotcw)(int *, double *, int *, double *, int *, double *);

EIGEN_BLAS_API void BLASFUNC(saxpy)(const int *, const float *, const float *, const int *, float *, const int *);
EIGEN_BLAS_API void BLASFUNC(daxpy)(const int *, const double *, const double *, const int *, double *, const int *);
EIGEN_BLAS_API void BLASFUNC(qaxpy)(const int *, const double *, const double *, const int *, double *, const int *);
EIGEN_BLAS_API void BLASFUNC(caxpy)(const int *, const float *, const float *, const int *, float *, const int *);
EIGEN_BLAS_API void BLASFUNC(zaxpy)(const int *, const double *, const double *, const int *, double *, const int *);
EIGEN_BLAS_API void BLASFUNC(xaxpy)(const int *, const double *, const double *, const int *, double *, const int *);
EIGEN_BLAS_API void BLASFUNC(caxpyc)(const int *, const float *, const float *, const int *, float *, const int *);
EIGEN_BLAS_API void BLASFUNC(zaxpyc)(const int *, const double *, const double *, const int *, double *, const int *);
EIGEN_BLAS_API void BLASFUNC(xaxpyc)(const int *, const double *, const double *, const int *, double *, const int *);

EIGEN_BLAS_API void BLASFUNC(scopy)(int *, float *, int *, float *, int *);
EIGEN_BLAS_API void BLASFUNC(dcopy)(int *, double *, int *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(qcopy)(int *, double *, int *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(ccopy)(int *, float *, int *, float *, int *);
EIGEN_BLAS_API void BLASFUNC(zcopy)(int *, double *, int *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(xcopy)(int *, double *, int *, double *, int *);

EIGEN_BLAS_API void BLASFUNC(sswap)(int *, float *, int *, float *, int *);
EIGEN_BLAS_API void BLASFUNC(dswap)(int *, double *, int *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(qswap)(int *, double *, int *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(cswap)(int *, float *, int *, float *, int *);
EIGEN_BLAS_API void BLASFUNC(zswap)(int *, double *, int *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(xswap)(int *, double *, int *, double *, int *);

EIGEN_BLAS_API float BLASFUNC(sasum)(int *, float *, int *);
EIGEN_BLAS_API float BLASFUNC(scasum)(int *, float *, int *);
EIGEN_BLAS_API double BLASFUNC(dasum)(int *, double *, int *);
EIGEN_BLAS_API double BLASFUNC(qasum)(int *, double *, int *);
EIGEN_BLAS_API double BLASFUNC(dzasum)(int *, double *, int *);
EIGEN_BLAS_API double BLASFUNC(qxasum)(int *, double *, int *);

EIGEN_BLAS_API int BLASFUNC(isamax)(int *, float *, int *);
EIGEN_BLAS_API int BLASFUNC(idamax)(int *, double *, int *);
EIGEN_BLAS_API int BLASFUNC(iqamax)(int *, double *, int *);
EIGEN_BLAS_API int BLASFUNC(icamax)(int *, float *, int *);
EIGEN_BLAS_API int BLASFUNC(izamax)(int *, double *, int *);
EIGEN_BLAS_API int BLASFUNC(ixamax)(int *, double *, int *);

EIGEN_BLAS_API int BLASFUNC(ismax)(int *, float *, int *);
EIGEN_BLAS_API int BLASFUNC(idmax)(int *, double *, int *);
EIGEN_BLAS_API int BLASFUNC(iqmax)(int *, double *, int *);
EIGEN_BLAS_API int BLASFUNC(icmax)(int *, float *, int *);
EIGEN_BLAS_API int BLASFUNC(izmax)(int *, double *, int *);
EIGEN_BLAS_API int BLASFUNC(ixmax)(int *, double *, int *);

EIGEN_BLAS_API int BLASFUNC(isamin)(int *, float *, int *);
EIGEN_BLAS_API int BLASFUNC(idamin)(int *, double *, int *);
EIGEN_BLAS_API int BLASFUNC(iqamin)(int *, double *, int *);
EIGEN_BLAS_API int BLASFUNC(icamin)(int *, float *, int *);
EIGEN_BLAS_API int BLASFUNC(izamin)(int *, double *, int *);
EIGEN_BLAS_API int BLASFUNC(ixamin)(int *, double *, int *);

EIGEN_BLAS_API int BLASFUNC(ismin)(int *, float *, int *);
EIGEN_BLAS_API int BLASFUNC(idmin)(int *, double *, int *);
EIGEN_BLAS_API int BLASFUNC(iqmin)(int *, double *, int *);
EIGEN_BLAS_API int BLASFUNC(icmin)(int *, float *, int *);
EIGEN_BLAS_API int BLASFUNC(izmin)(int *, double *, int *);
EIGEN_BLAS_API int BLASFUNC(ixmin)(int *, double *, int *);

EIGEN_BLAS_API float BLASFUNC(samax)(int *, float *, int *);
EIGEN_BLAS_API double BLASFUNC(damax)(int *, double *, int *);
EIGEN_BLAS_API double BLASFUNC(qamax)(int *, double *, int *);
EIGEN_BLAS_API float BLASFUNC(scamax)(int *, float *, int *);
EIGEN_BLAS_API double BLASFUNC(dzamax)(int *, double *, int *);
EIGEN_BLAS_API double BLASFUNC(qxamax)(int *, double *, int *);

EIGEN_BLAS_API float BLASFUNC(samin)(int *, float *, int *);
EIGEN_BLAS_API double BLASFUNC(damin)(int *, double *, int *);
EIGEN_BLAS_API double BLASFUNC(qamin)(int *, double *, int *);
EIGEN_BLAS_API float BLASFUNC(scamin)(int *, float *, int *);
EIGEN_BLAS_API double BLASFUNC(dzamin)(int *, double *, int *);
EIGEN_BLAS_API double BLASFUNC(qxamin)(int *, double *, int *);

EIGEN_BLAS_API float BLASFUNC(smax)(int *, float *, int *);
EIGEN_BLAS_API double BLASFUNC(dmax)(int *, double *, int *);
EIGEN_BLAS_API double BLASFUNC(qmax)(int *, double *, int *);
EIGEN_BLAS_API float BLASFUNC(scmax)(int *, float *, int *);
EIGEN_BLAS_API double BLASFUNC(dzmax)(int *, double *, int *);
EIGEN_BLAS_API double BLASFUNC(qxmax)(int *, double *, int *);

EIGEN_BLAS_API float BLASFUNC(smin)(int *, float *, int *);
EIGEN_BLAS_API double BLASFUNC(dmin)(int *, double *, int *);
EIGEN_BLAS_API double BLASFUNC(qmin)(int *, double *, int *);
EIGEN_BLAS_API float BLASFUNC(scmin)(int *, float *, int *);
EIGEN_BLAS_API double BLASFUNC(dzmin)(int *, double *, int *);
EIGEN_BLAS_API double BLASFUNC(qxmin)(int *, double *, int *);

EIGEN_BLAS_API void BLASFUNC(sscal)(int *, float *, float *, int *);
EIGEN_BLAS_API void BLASFUNC(dscal)(int *, double *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(qscal)(int *, double *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(cscal)(int *, float *, float *, int *);
EIGEN_BLAS_API void BLASFUNC(zscal)(int *, double *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(xscal)(int *, double *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(csscal)(int *, float *, float *, int *);
EIGEN_BLAS_API void BLASFUNC(zdscal)(int *, double *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(xqscal)(int *, double *, double *, int *);

EIGEN_BLAS_API float BLASFUNC(snrm2)(int *, float *, int *);
EIGEN_BLAS_API float BLASFUNC(scnrm2)(int *, float *, int *);

EIGEN_BLAS_API double BLASFUNC(dnrm2)(int *, double *, int *);
EIGEN_BLAS_API double BLASFUNC(qnrm2)(int *, double *, int *);
EIGEN_BLAS_API double BLASFUNC(dznrm2)(int *, double *, int *);
EIGEN_BLAS_API double BLASFUNC(qxnrm2)(int *, double *, int *);

EIGEN_BLAS_API void BLASFUNC(srot)(int *, float *, int *, float *, int *, float *, float *);
EIGEN_BLAS_API void BLASFUNC(drot)(int *, double *, int *, double *, int *, double *, double *);
EIGEN_BLAS_API void BLASFUNC(qrot)(int *, double *, int *, double *, int *, double *, double *);
EIGEN_BLAS_API void BLASFUNC(csrot)(int *, float *, int *, float *, int *, float *, float *);
EIGEN_BLAS_API void BLASFUNC(zdrot)(int *, double *, int *, double *, int *, double *, double *);
EIGEN_BLAS_API void BLASFUNC(xqrot)(int *, double *, int *, double *, int *, double *, double *);

EIGEN_BLAS_API void BLASFUNC(srotg)(float *, float *, float *, float *);
EIGEN_BLAS_API void BLASFUNC(drotg)(double *, double *, double *, double *);
EIGEN_BLAS_API void BLASFUNC(qrotg)(double *, double *, double *, double *);
EIGEN_BLAS_API void BLASFUNC(crotg)(float *, float *, float *, float *);
EIGEN_BLAS_API void BLASFUNC(zrotg)(double *, double *, double *, double *);
EIGEN_BLAS_API void BLASFUNC(xrotg)(double *, double *, double *, double *);

EIGEN_BLAS_API void BLASFUNC(srotmg)(float *, float *, float *, float *, float *);
EIGEN_BLAS_API void BLASFUNC(drotmg)(double *, double *, double *, double *, double *);

EIGEN_BLAS_API void BLASFUNC(srotm)(int *, float *, int *, float *, int *, float *);
EIGEN_BLAS_API void BLASFUNC(drotm)(int *, double *, int *, double *, int *, double *);
EIGEN_BLAS_API void BLASFUNC(qrotm)(int *, double *, int *, double *, int *, double *);

/* Level 2 routines */

EIGEN_BLAS_API void BLASFUNC(sger)(int *, int *, float *, float *, int *, float *, int *, float *, int *);
EIGEN_BLAS_API void BLASFUNC(dger)(int *, int *, double *, double *, int *, double *, int *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(qger)(int *, int *, double *, double *, int *, double *, int *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(cgeru)(int *, int *, float *, float *, int *, float *, int *, float *, int *);
EIGEN_BLAS_API void BLASFUNC(cgerc)(int *, int *, float *, float *, int *, float *, int *, float *, int *);
EIGEN_BLAS_API void BLASFUNC(zgeru)(int *, int *, double *, double *, int *, double *, int *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(zgerc)(int *, int *, double *, double *, int *, double *, int *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(xgeru)(int *, int *, double *, double *, int *, double *, int *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(xgerc)(int *, int *, double *, double *, int *, double *, int *, double *, int *);

EIGEN_BLAS_API void BLASFUNC(sgemv)(const char *, const int *, const int *, const float *, const float *, const int *,
                                    const float *, const int *, const float *, float *, const int *);
EIGEN_BLAS_API void BLASFUNC(dgemv)(const char *, const int *, const int *, const double *, const double *, const int *,
                                    const double *, const int *, const double *, double *, const int *);
EIGEN_BLAS_API void BLASFUNC(qgemv)(const char *, const int *, const int *, const double *, const double *, const int *,
                                    const double *, const int *, const double *, double *, const int *);
EIGEN_BLAS_API void BLASFUNC(cgemv)(const char *, const int *, const int *, const float *, const float *, const int *,
                                    const float *, const int *, const float *, float *, const int *);
EIGEN_BLAS_API void BLASFUNC(zgemv)(const char *, const int *, const int *, const double *, const double *, const int *,
                                    const double *, const int *, const double *, double *, const int *);
EIGEN_BLAS_API void BLASFUNC(xgemv)(const char *, const int *, const int *, const double *, const double *, const int *,
                                    const double *, const int *, const double *, double *, const int *);

EIGEN_BLAS_API void BLASFUNC(strsv)(const char *, const char *, const char *, const int *, const float *, const int *,
                                    float *, const int *);
EIGEN_BLAS_API void BLASFUNC(dtrsv)(const char *, const char *, const char *, const int *, const double *, const int *,
                                    double *, const int *);
EIGEN_BLAS_API void BLASFUNC(qtrsv)(const char *, const char *, const char *, const int *, const double *, const int *,
                                    double *, const int *);
EIGEN_BLAS_API void BLASFUNC(ctrsv)(const char *, const char *, const char *, const int *, const float *, const int *,
                                    float *, const int *);
EIGEN_BLAS_API void BLASFUNC(ztrsv)(const char *, const char *, const char *, const int *, const double *, const int *,
                                    double *, const int *);
EIGEN_BLAS_API void BLASFUNC(xtrsv)(const char *, const char *, const char *, const int *, const double *, const int *,
                                    double *, const int *);

EIGEN_BLAS_API void BLASFUNC(stpsv)(char *, char *, char *, int *, float *, float *, int *);
EIGEN_BLAS_API void BLASFUNC(dtpsv)(char *, char *, char *, int *, double *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(qtpsv)(char *, char *, char *, int *, double *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(ctpsv)(char *, char *, char *, int *, float *, float *, int *);
EIGEN_BLAS_API void BLASFUNC(ztpsv)(char *, char *, char *, int *, double *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(xtpsv)(char *, char *, char *, int *, double *, double *, int *);

EIGEN_BLAS_API void BLASFUNC(strmv)(const char *, const char *, const char *, const int *, const float *, const int *,
                                    float *, const int *);
EIGEN_BLAS_API void BLASFUNC(dtrmv)(const char *, const char *, const char *, const int *, const double *, const int *,
                                    double *, const int *);
EIGEN_BLAS_API void BLASFUNC(qtrmv)(const char *, const char *, const char *, const int *, const double *, const int *,
                                    double *, const int *);
EIGEN_BLAS_API void BLASFUNC(ctrmv)(const char *, const char *, const char *, const int *, const float *, const int *,
                                    float *, const int *);
EIGEN_BLAS_API void BLASFUNC(ztrmv)(const char *, const char *, const char *, const int *, const double *, const int *,
                                    double *, const int *);
EIGEN_BLAS_API void BLASFUNC(xtrmv)(const char *, const char *, const char *, const int *, const double *, const int *,
                                    double *, const int *);

EIGEN_BLAS_API void BLASFUNC(stpmv)(char *, char *, char *, int *, float *, float *, int *);
EIGEN_BLAS_API void BLASFUNC(dtpmv)(char *, char *, char *, int *, double *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(qtpmv)(char *, char *, char *, int *, double *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(ctpmv)(char *, char *, char *, int *, float *, float *, int *);
EIGEN_BLAS_API void BLASFUNC(ztpmv)(char *, char *, char *, int *, double *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(xtpmv)(char *, char *, char *, int *, double *, double *, int *);

EIGEN_BLAS_API void BLASFUNC(stbmv)(char *, char *, char *, int *, int *, float *, int *, float *, int *);
EIGEN_BLAS_API void BLASFUNC(dtbmv)(char *, char *, char *, int *, int *, double *, int *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(qtbmv)(char *, char *, char *, int *, int *, double *, int *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(ctbmv)(char *, char *, char *, int *, int *, float *, int *, float *, int *);
EIGEN_BLAS_API void BLASFUNC(ztbmv)(char *, char *, char *, int *, int *, double *, int *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(xtbmv)(char *, char *, char *, int *, int *, double *, int *, double *, int *);

EIGEN_BLAS_API void BLASFUNC(stbsv)(char *, char *, char *, int *, int *, float *, int *, float *, int *);
EIGEN_BLAS_API void BLASFUNC(dtbsv)(char *, char *, char *, int *, int *, double *, int *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(qtbsv)(char *, char *, char *, int *, int *, double *, int *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(ctbsv)(char *, char *, char *, int *, int *, float *, int *, float *, int *);
EIGEN_BLAS_API void BLASFUNC(ztbsv)(char *, char *, char *, int *, int *, double *, int *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(xtbsv)(char *, char *, char *, int *, int *, double *, int *, double *, int *);

EIGEN_BLAS_API void BLASFUNC(ssymv)(const char *, const int *, const float *, const float *, const int *, const float *,
                                    const int *, const float *, float *, const int *);
EIGEN_BLAS_API void BLASFUNC(dsymv)(const char *, const int *, const double *, const double *, const int *,
                                    const double *, const int *, const double *, double *, const int *);
EIGEN_BLAS_API void BLASFUNC(qsymv)(const char *, const int *, const double *, const double *, const int *,
                                    const double *, const int *, const double *, double *, const int *);

EIGEN_BLAS_API void BLASFUNC(sspmv)(char *, int *, float *, float *, float *, int *, float *, float *, int *);
EIGEN_BLAS_API void BLASFUNC(dspmv)(char *, int *, double *, double *, double *, int *, double *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(qspmv)(char *, int *, double *, double *, double *, int *, double *, double *, int *);

EIGEN_BLAS_API void BLASFUNC(ssyr)(const char *, const int *, const float *, const float *, const int *, float *,
                                   const int *);
EIGEN_BLAS_API void BLASFUNC(dsyr)(const char *, const int *, const double *, const double *, const int *, double *,
                                   const int *);
EIGEN_BLAS_API void BLASFUNC(qsyr)(const char *, const int *, const double *, const double *, const int *, double *,
                                   const int *);

EIGEN_BLAS_API void BLASFUNC(ssyr2)(const char *, const int *, const float *, const float *, const int *, const float *,
                                    const int *, float *, const int *);
EIGEN_BLAS_API void BLASFUNC(dsyr2)(const char *, const int *, const double *, const double *, const int *,
                                    const double *, const int *, double *, const int *);
EIGEN_BLAS_API void BLASFUNC(qsyr2)(const char *, const int *, const double *, const double *, const int *,
                                    const double *, const int *, double *, const int *);
EIGEN_BLAS_API void BLASFUNC(csyr2)(const char *, const int *, const float *, const float *, const int *, const float *,
                                    const int *, float *, const int *);
EIGEN_BLAS_API void BLASFUNC(zsyr2)(const char *, const int *, const double *, const double *, const int *,
                                    const double *, const int *, double *, const int *);
EIGEN_BLAS_API void BLASFUNC(xsyr2)(const char *, const int *, const double *, const double *, const int *,
                                    const double *, const int *, double *, const int *);

EIGEN_BLAS_API void BLASFUNC(sspr)(char *, int *, float *, float *, int *, float *);
EIGEN_BLAS_API void BLASFUNC(dspr)(char *, int *, double *, double *, int *, double *);
EIGEN_BLAS_API void BLASFUNC(qspr)(char *, int *, double *, double *, int *, double *);

EIGEN_BLAS_API void BLASFUNC(sspr2)(char *, int *, float *, float *, int *, float *, int *, float *);
EIGEN_BLAS_API void BLASFUNC(dspr2)(char *, int *, double *, double *, int *, double *, int *, double *);
EIGEN_BLAS_API void BLASFUNC(qspr2)(char *, int *, double *, double *, int *, double *, int *, double *);
EIGEN_BLAS_API void BLASFUNC(cspr2)(char *, int *, float *, float *, int *, float *, int *, float *);
EIGEN_BLAS_API void BLASFUNC(zspr2)(char *, int *, double *, double *, int *, double *, int *, double *);
EIGEN_BLAS_API void BLASFUNC(xspr2)(char *, int *, double *, double *, int *, double *, int *, double *);

EIGEN_BLAS_API void BLASFUNC(cher)(char *, int *, float *, float *, int *, float *, int *);
EIGEN_BLAS_API void BLASFUNC(zher)(char *, int *, double *, double *, int *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(xher)(char *, int *, double *, double *, int *, double *, int *);

EIGEN_BLAS_API void BLASFUNC(chpr)(char *, int *, float *, float *, int *, float *);
EIGEN_BLAS_API void BLASFUNC(zhpr)(char *, int *, double *, double *, int *, double *);
EIGEN_BLAS_API void BLASFUNC(xhpr)(char *, int *, double *, double *, int *, double *);

EIGEN_BLAS_API void BLASFUNC(cher2)(char *, int *, float *, float *, int *, float *, int *, float *, int *);
EIGEN_BLAS_API void BLASFUNC(zher2)(char *, int *, double *, double *, int *, double *, int *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(xher2)(char *, int *, double *, double *, int *, double *, int *, double *, int *);

EIGEN_BLAS_API void BLASFUNC(chpr2)(char *, int *, float *, float *, int *, float *, int *, float *);
EIGEN_BLAS_API void BLASFUNC(zhpr2)(char *, int *, double *, double *, int *, double *, int *, double *);
EIGEN_BLAS_API void BLASFUNC(xhpr2)(char *, int *, double *, double *, int *, double *, int *, double *);

EIGEN_BLAS_API void BLASFUNC(chemv)(const char *, const int *, const float *, const float *, const int *, const float *,
                                    const int *, const float *, float *, const int *);
EIGEN_BLAS_API void BLASFUNC(zhemv)(const char *, const int *, const double *, const double *, const int *,
                                    const double *, const int *, const double *, double *, const int *);
EIGEN_BLAS_API void BLASFUNC(xhemv)(const char *, const int *, const double *, const double *, const int *,
                                    const double *, const int *, const double *, double *, const int *);

EIGEN_BLAS_API void BLASFUNC(chpmv)(char *, int *, float *, float *, float *, int *, float *, float *, int *);
EIGEN_BLAS_API void BLASFUNC(zhpmv)(char *, int *, double *, double *, double *, int *, double *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(xhpmv)(char *, int *, double *, double *, double *, int *, double *, double *, int *);

EIGEN_BLAS_API void BLASFUNC(snorm)(char *, int *, int *, float *, int *);
EIGEN_BLAS_API void BLASFUNC(dnorm)(char *, int *, int *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(cnorm)(char *, int *, int *, float *, int *);
EIGEN_BLAS_API void BLASFUNC(znorm)(char *, int *, int *, double *, int *);

EIGEN_BLAS_API void BLASFUNC(sgbmv)(char *, int *, int *, int *, int *, float *, float *, int *, float *, int *,
                                    float *, float *, int *);
EIGEN_BLAS_API void BLASFUNC(dgbmv)(char *, int *, int *, int *, int *, double *, double *, int *, double *, int *,
                                    double *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(qgbmv)(char *, int *, int *, int *, int *, double *, double *, int *, double *, int *,
                                    double *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(cgbmv)(char *, int *, int *, int *, int *, float *, float *, int *, float *, int *,
                                    float *, float *, int *);
EIGEN_BLAS_API void BLASFUNC(zgbmv)(char *, int *, int *, int *, int *, double *, double *, int *, double *, int *,
                                    double *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(xgbmv)(char *, int *, int *, int *, int *, double *, double *, int *, double *, int *,
                                    double *, double *, int *);

EIGEN_BLAS_API void BLASFUNC(ssbmv)(char *, int *, int *, float *, float *, int *, float *, int *, float *, float *,
                                    int *);
EIGEN_BLAS_API void BLASFUNC(dsbmv)(char *, int *, int *, double *, double *, int *, double *, int *, double *,
                                    double *, int *);
EIGEN_BLAS_API void BLASFUNC(qsbmv)(char *, int *, int *, double *, double *, int *, double *, int *, double *,
                                    double *, int *);
EIGEN_BLAS_API void BLASFUNC(csbmv)(char *, int *, int *, float *, float *, int *, float *, int *, float *, float *,
                                    int *);
EIGEN_BLAS_API void BLASFUNC(zsbmv)(char *, int *, int *, double *, double *, int *, double *, int *, double *,
                                    double *, int *);
EIGEN_BLAS_API void BLASFUNC(xsbmv)(char *, int *, int *, double *, double *, int *, double *, int *, double *,
                                    double *, int *);

EIGEN_BLAS_API void BLASFUNC(chbmv)(char *, int *, int *, float *, float *, int *, float *, int *, float *, float *,
                                    int *);
EIGEN_BLAS_API void BLASFUNC(zhbmv)(char *, int *, int *, double *, double *, int *, double *, int *, double *,
                                    double *, int *);
EIGEN_BLAS_API void BLASFUNC(xhbmv)(char *, int *, int *, double *, double *, int *, double *, int *, double *,
                                    double *, int *);

/* Level 3 routines */

EIGEN_BLAS_API void BLASFUNC(sgemm)(const char *, const char *, const int *, const int *, const int *, const float *,
                                    const float *, const int *, const float *, const int *, const float *, float *,
                                    const int *);
EIGEN_BLAS_API void BLASFUNC(dgemm)(const char *, const char *, const int *, const int *, const int *, const double *,
                                    const double *, const int *, const double *, const int *, const double *, double *,
                                    const int *);
EIGEN_BLAS_API void BLASFUNC(qgemm)(const char *, const char *, const int *, const int *, const int *, const double *,
                                    const double *, const int *, const double *, const int *, const double *, double *,
                                    const int *);
EIGEN_BLAS_API void BLASFUNC(cgemm)(const char *, const char *, const int *, const int *, const int *, const float *,
                                    const float *, const int *, const float *, const int *, const float *, float *,
                                    const int *);
EIGEN_BLAS_API void BLASFUNC(zgemm)(const char *, const char *, const int *, const int *, const int *, const double *,
                                    const double *, const int *, const double *, const int *, const double *, double *,
                                    const int *);
EIGEN_BLAS_API void BLASFUNC(xgemm)(const char *, const char *, const int *, const int *, const int *, const double *,
                                    const double *, const int *, const double *, const int *, const double *, double *,
                                    const int *);

EIGEN_BLAS_API void BLASFUNC(cgemm3m)(char *, char *, int *, int *, int *, float *, float *, int *, float *, int *,
                                      float *, float *, int *);
EIGEN_BLAS_API void BLASFUNC(zgemm3m)(char *, char *, int *, int *, int *, double *, double *, int *, double *, int *,
                                      double *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(xgemm3m)(char *, char *, int *, int *, int *, double *, double *, int *, double *, int *,
                                      double *, double *, int *);

EIGEN_BLAS_API void BLASFUNC(sge2mm)(char *, char *, char *, int *, int *, float *, float *, int *, float *, int *,
                                     float *, float *, int *);
EIGEN_BLAS_API void BLASFUNC(dge2mm)(char *, char *, char *, int *, int *, double *, double *, int *, double *, int *,
                                     double *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(cge2mm)(char *, char *, char *, int *, int *, float *, float *, int *, float *, int *,
                                     float *, float *, int *);
EIGEN_BLAS_API void BLASFUNC(zge2mm)(char *, char *, char *, int *, int *, double *, double *, int *, double *, int *,
                                     double *, double *, int *);

EIGEN_BLAS_API void BLASFUNC(strsm)(const char *, const char *, const char *, const char *, const int *, const int *,
                                    const float *, const float *, const int *, float *, const int *);
EIGEN_BLAS_API void BLASFUNC(dtrsm)(const char *, const char *, const char *, const char *, const int *, const int *,
                                    const double *, const double *, const int *, double *, const int *);
EIGEN_BLAS_API void BLASFUNC(qtrsm)(const char *, const char *, const char *, const char *, const int *, const int *,
                                    const double *, const double *, const int *, double *, const int *);
EIGEN_BLAS_API void BLASFUNC(ctrsm)(const char *, const char *, const char *, const char *, const int *, const int *,
                                    const float *, const float *, const int *, float *, const int *);
EIGEN_BLAS_API void BLASFUNC(ztrsm)(const char *, const char *, const char *, const char *, const int *, const int *,
                                    const double *, const double *, const int *, double *, const int *);
EIGEN_BLAS_API void BLASFUNC(xtrsm)(const char *, const char *, const char *, const char *, const int *, const int *,
                                    const double *, const double *, const int *, double *, const int *);

EIGEN_BLAS_API void BLASFUNC(strmm)(const char *, const char *, const char *, const char *, const int *, const int *,
                                    const float *, const float *, const int *, float *, const int *);
EIGEN_BLAS_API void BLASFUNC(dtrmm)(const char *, const char *, const char *, const char *, const int *, const int *,
                                    const double *, const double *, const int *, double *, const int *);
EIGEN_BLAS_API void BLASFUNC(qtrmm)(const char *, const char *, const char *, const char *, const int *, const int *,
                                    const double *, const double *, const int *, double *, const int *);
EIGEN_BLAS_API void BLASFUNC(ctrmm)(const char *, const char *, const char *, const char *, const int *, const int *,
                                    const float *, const float *, const int *, float *, const int *);
EIGEN_BLAS_API void BLASFUNC(ztrmm)(const char *, const char *, const char *, const char *, const int *, const int *,
                                    const double *, const double *, const int *, double *, const int *);
EIGEN_BLAS_API void BLASFUNC(xtrmm)(const char *, const char *, const char *, const char *, const int *, const int *,
                                    const double *, const double *, const int *, double *, const int *);

EIGEN_BLAS_API void BLASFUNC(ssymm)(const char *, const char *, const int *, const int *, const float *, const float *,
                                    const int *, const float *, const int *, const float *, float *, const int *);
EIGEN_BLAS_API void BLASFUNC(dsymm)(const char *, const char *, const int *, const int *, const double *,
                                    const double *, const int *, const double *, const int *, const double *, double *,
                                    const int *);
EIGEN_BLAS_API void BLASFUNC(qsymm)(const char *, const char *, const int *, const int *, const double *,
                                    const double *, const int *, const double *, const int *, const double *, double *,
                                    const int *);
EIGEN_BLAS_API void BLASFUNC(csymm)(const char *, const char *, const int *, const int *, const float *, const float *,
                                    const int *, const float *, const int *, const float *, float *, const int *);
EIGEN_BLAS_API void BLASFUNC(zsymm)(const char *, const char *, const int *, const int *, const double *,
                                    const double *, const int *, const double *, const int *, const double *, double *,
                                    const int *);
EIGEN_BLAS_API void BLASFUNC(xsymm)(const char *, const char *, const int *, const int *, const double *,
                                    const double *, const int *, const double *, const int *, const double *, double *,
                                    const int *);

EIGEN_BLAS_API void BLASFUNC(csymm3m)(char *, char *, int *, int *, float *, float *, int *, float *, int *, float *,
                                      float *, int *);
EIGEN_BLAS_API void BLASFUNC(zsymm3m)(char *, char *, int *, int *, double *, double *, int *, double *, int *,
                                      double *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(xsymm3m)(char *, char *, int *, int *, double *, double *, int *, double *, int *,
                                      double *, double *, int *);

EIGEN_BLAS_API void BLASFUNC(ssyrk)(const char *, const char *, const int *, const int *, const float *, const float *,
                                    const int *, const float *, float *, const int *);
EIGEN_BLAS_API void BLASFUNC(dsyrk)(const char *, const char *, const int *, const int *, const double *,
                                    const double *, const int *, const double *, double *, const int *);
EIGEN_BLAS_API void BLASFUNC(qsyrk)(const char *, const char *, const int *, const int *, const double *,
                                    const double *, const int *, const double *, double *, const int *);
EIGEN_BLAS_API void BLASFUNC(csyrk)(const char *, const char *, const int *, const int *, const float *, const float *,
                                    const int *, const float *, float *, const int *);
EIGEN_BLAS_API void BLASFUNC(zsyrk)(const char *, const char *, const int *, const int *, const double *,
                                    const double *, const int *, const double *, double *, const int *);
EIGEN_BLAS_API void BLASFUNC(xsyrk)(const char *, const char *, const int *, const int *, const double *,
                                    const double *, const int *, const double *, double *, const int *);

EIGEN_BLAS_API void BLASFUNC(ssyr2k)(const char *, const char *, const int *, const int *, const float *, const float *,
                                     const int *, const float *, const int *, const float *, float *, const int *);
EIGEN_BLAS_API void BLASFUNC(dsyr2k)(const char *, const char *, const int *, const int *, const double *,
                                     const double *, const int *, const double *, const int *, const double *, double *,
                                     const int *);
EIGEN_BLAS_API void BLASFUNC(qsyr2k)(const char *, const char *, const int *, const int *, const double *,
                                     const double *, const int *, const double *, const int *, const double *, double *,
                                     const int *);
EIGEN_BLAS_API void BLASFUNC(csyr2k)(const char *, const char *, const int *, const int *, const float *, const float *,
                                     const int *, const float *, const int *, const float *, float *, const int *);
EIGEN_BLAS_API void BLASFUNC(zsyr2k)(const char *, const char *, const int *, const int *, const double *,
                                     const double *, const int *, const double *, const int *, const double *, double *,
                                     const int *);
EIGEN_BLAS_API void BLASFUNC(xsyr2k)(const char *, const char *, const int *, const int *, const double *,
                                     const double *, const int *, const double *, const int *, const double *, double *,
                                     const int *);

EIGEN_BLAS_API void BLASFUNC(chemm)(const char *, const char *, const int *, const int *, const float *, const float *,
                                    const int *, const float *, const int *, const float *, float *, const int *);
EIGEN_BLAS_API void BLASFUNC(zhemm)(const char *, const char *, const int *, const int *, const double *,
                                    const double *, const int *, const double *, const int *, const double *, double *,
                                    const int *);
EIGEN_BLAS_API void BLASFUNC(xhemm)(const char *, const char *, const int *, const int *, const double *,
                                    const double *, const int *, const double *, const int *, const double *, double *,
                                    const int *);

EIGEN_BLAS_API void BLASFUNC(chemm3m)(char *, char *, int *, int *, float *, float *, int *, float *, int *, float *,
                                      float *, int *);
EIGEN_BLAS_API void BLASFUNC(zhemm3m)(char *, char *, int *, int *, double *, double *, int *, double *, int *,
                                      double *, double *, int *);
EIGEN_BLAS_API void BLASFUNC(xhemm3m)(char *, char *, int *, int *, double *, double *, int *, double *, int *,
                                      double *, double *, int *);

EIGEN_BLAS_API void BLASFUNC(cherk)(const char *, const char *, const int *, const int *, const float *, const float *,
                                    const int *, const float *, float *, const int *);
EIGEN_BLAS_API void BLASFUNC(zherk)(const char *, const char *, const int *, const int *, const double *,
                                    const double *, const int *, const double *, double *, const int *);
EIGEN_BLAS_API void BLASFUNC(xherk)(const char *, const char *, const int *, const int *, const double *,
                                    const double *, const int *, const double *, double *, const int *);

EIGEN_BLAS_API void BLASFUNC(cher2k)(const char *, const char *, const int *, const int *, const float *, const float *,
                                     const int *, const float *, const int *, const float *, float *, const int *);
EIGEN_BLAS_API void BLASFUNC(zher2k)(const char *, const char *, const int *, const int *, const double *,
                                     const double *, const int *, const double *, const int *, const double *, double *,
                                     const int *);
EIGEN_BLAS_API void BLASFUNC(xher2k)(const char *, const char *, const int *, const int *, const double *,
                                     const double *, const int *, const double *, const int *, const double *, double *,
                                     const int *);
EIGEN_BLAS_API void BLASFUNC(cher2m)(const char *, const char *, const char *, const int *, const int *, const float *,
                                     const float *, const int *, const float *, const int *, const float *, float *,
                                     const int *);
EIGEN_BLAS_API void BLASFUNC(zher2m)(const char *, const char *, const char *, const int *, const int *, const double *,
                                     const double *, const int *, const double *, const int *, const double *, double *,
                                     const int *);
EIGEN_BLAS_API void BLASFUNC(xher2m)(const char *, const char *, const char *, const int *, const int *, const double *,
                                     const double *, const int *, const double *, const int *, const double *, double *,
                                     const int *);

EIGEN_BLAS_API void BLASFUNC(sgemmtr)(const char *, const char *, const char *, const int *, const int *, const float *,
                                      const float *, const int *, const float *, const int *, const float *, float *,
                                      const int *);
EIGEN_BLAS_API void BLASFUNC(dgemmtr)(const char *, const char *, const char *, const int *, const int *,
                                      const double *, const double *, const int *, const double *, const int *,
                                      const double *, double *, const int *);
EIGEN_BLAS_API void BLASFUNC(qgemmtr)(const char *, const char *, const char *, const int *, const int *,
                                      const double *, const double *, const int *, const double *, const int *,
                                      const double *, double *, const int *);
EIGEN_BLAS_API void BLASFUNC(cgemmtr)(const char *, const char *, const char *, const int *, const int *, const float *,
                                      const float *, const int *, const float *, const int *, const float *, float *,
                                      const int *);
EIGEN_BLAS_API void BLASFUNC(zgemmtr)(const char *, const char *, const char *, const int *, const int *,
                                      const double *, const double *, const int *, const double *, const int *,
                                      const double *, double *, const int *);
EIGEN_BLAS_API void BLASFUNC(xgemmtr)(const char *, const char *, const char *, const int *, const int *,
                                      const double *, const double *, const int *, const double *, const int *,
                                      const double *, double *, const int *);

#ifdef __cplusplus
}
#endif

#endif
