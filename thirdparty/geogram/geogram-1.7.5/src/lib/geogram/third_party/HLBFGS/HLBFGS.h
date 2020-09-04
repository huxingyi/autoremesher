///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// HLBFGS                                                                    //
// http://www.loria.fr/~liuyang/software/HLBFGS/                             //
//                                                                           //
// HLBFGS is a hybrid L-BFGS optimization framework which unifies L-BFGS     //
// method, Preconditioned L-BFGS method and                                  //
// Preconditioned Conjugate Gradient method.                                 //
//                                                                           //
// Version 1.2                                                               //
// March 09, 2010                                                            //
//                                                                           //
// Copyright (C) 2009--2010                                                  //
// Yang Liu                                                                  //
//                                                                           //
// xueyuhanlang@gmail.com                                                    //
//                                                                           //
// HLBFGS is HLBFGS is freely available for non-commercial purposes.         //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef HLBFGS_H
#define HLBFGS_H

#include <vector>

// [Bruno Levy] added HLBFGS_API declarations to have linkage.
#include <geogram/api/defs.h>
#define HLBFGS_API GEOGRAM_API

//////////////////////////////////////////////////////////////////////////

//! ICFS_INFO stores ICFS's working arrays
class HLBFGS_API ICFS_INFO {
public:
        ICFS_INFO() {
            p = 15;
        }
        ~ICFS_INFO() {
        }
        void allocate_mem(int N_in) {
            if (N_in > 0) {
                size_t N = size_t(N_in);
                lcol_ptr.resize(N + 1);
                ldiag.resize(N);
                iwa.resize(3 * N);
                wa1.resize(N);
                wa2.resize(N);
                r.resize(N);
                p = 15;
                CGP.resize(N);
                CGR.resize(N);
                CGQ.resize(N);
                CGZ.resize(N);
            }
        }
        int * get_lcol_ptr() {
            return &lcol_ptr[0];
        }
        int * get_lrow_ind() {
            return &lrow_ind[0];
        }
        double * get_ldiag() {
            return &ldiag[0];
        }
        double * get_l() {
            return &l[0];
        }
        int * get_iwa() {
            return &iwa[0];
        }
        double * get_wa1() {
            return &wa1[0];
        }
        double * get_wa2() {
            return &wa2[0];
        }
        int & get_p() {
            return p;
        }
        double * get_r() {
            return &r[0];
        }
        double * get_CGP() {
            return &CGP[0];
        }
        double * get_CGQ() {
            return &CGQ[0];
        }
        double * get_CGR() {
            return &CGR[0];
        }
        double * get_CGZ() {
            return &CGZ[0];
        }
        double & get_icfs_alpha() {
            return icfs_alpha;
        }
        void set_lrow_ind_size(int size) {
            lrow_ind.resize(size_t(size));
        }
        void set_l_size(int size) {
            l.resize(size_t(size));
        }
private:
        std::vector<int> lcol_ptr;
        std::vector<int> lrow_ind;
        std::vector<double> ldiag;
        std::vector<double> l;
        std::vector<int> iwa;
        std::vector<double> wa1;
        std::vector<double> wa2;
        int p;
        std::vector<double> r;
        std::vector<double> CGP;
        std::vector<double> CGQ;
        std::vector<double> CGR;
        std::vector<double> CGZ;
        double icfs_alpha;
};
//////////////////////////////////////////////////////////////////////////
//! Stores the pointers of hessian matrix

class HLBFGS_API HESSIAN_MATRIX {
public:
        HESSIAN_MATRIX(int N) {
            n = N;
            nnz = 0;
            values = nullptr;
            rowind = nullptr;
            colptr = nullptr;
            diag   = nullptr;
        }
        ~HESSIAN_MATRIX() {
        }
        void set_dimension(int dim) {
            n = dim;
        }
        void set_nonzeros(int nz) {
            nnz = nz;
        }
        void set_values(double *array_values) {
            values = array_values;
        }
        void set_rowind(int *array_rowind) {
            rowind = array_rowind;
        }
        void set_colptr(int *array_colptr) {
            colptr = array_colptr;
        }
        void set_diag(double *array_diag) {
            diag = array_diag;
        }
        int get_dimension() {
            return n;
        }
        int get_nonzeros() {
            return nnz;
        }
        double * get_values() {
            return values;
        }
        int * get_rowind() {
            return rowind;
        }
        int * get_colptr() {
            return colptr;
        }
        double * get_diag() {
            return diag;
        }
        ICFS_INFO& get_icfs_info() {
            return l_info;
        }
private:
        int n;
        int nnz;
        double *values;
        int *rowind;
        int *colptr;
        double *diag;
        ICFS_INFO l_info;
};

//////////////////////////////////////////////////////////////////////////
//! HLBFGS initialization
//Dimension of arrays: 20, 20
void HLBFGS_API INIT_HLBFGS(double PARAMETERS[], int INFO[]);

void HLBFGS_API HLBFGS_MESSAGE(bool print, int id, const double PARAMETERS[]);

//////////////////////////////////////////////////////////////////////////
void HLBFGS_API HLBFGS_UPDATE_First_Step(
    int N, int M, double *q, double *s, double *y,
    double *rho, double *alpha, int bound, int cur_pos, int iter
 );

void HLBFGS_API HLBFGS_UPDATE_Hessian(
    int N, int M, double *q, double *s, double *y,
    int cur_pos, double *diag, int INFO[]
);

void HLBFGS_API HLBFGS_UPDATE_Second_Step(
    int N, int M, double *q, double *s, double *y,
    double *rho, double *alpha, int bound, int cur_pos, int iter
);

void HLBFGS_API CONJUGATE_GRADIENT_UPDATE(
    int N, double *q, double *prev_q_update,
    double *prev_q_first_stage, int INFO[]
);
//////////////////////////////////////////////////////////////////////////

void HLBFGS_API HLBFGS_BUILD_HESSIAN_INFO(HESSIAN_MATRIX& m_hessian, int INFO[]);
//////////////////////////////////////////////////////////////////////////

//! HLBFGS functions
void HLBFGS_API HLBFGS(
    int N, int M, double *x,
    void EVALFUNC(int, double*, double*, double*, double*),
    void EVALFUNC_H(int, double*, double*, double*, double*, HESSIAN_MATRIX&),
    void USER_DEFINED_HLBFGS_UPDATE_H(int, int, double*, double*, double*, int, double*, int[]),
    void NEWITERATION(int, int, double*, double*, double*, double*),
    double PARAMETERS[],
    int INFO[]
);
//////////////////////////////////////////////////////////////////////////

#endif
