
#ifdef GEOGRAM_WITH_HLBFGS

#include <geogram/basic/command_line.h>
#include <geogram/basic/command_line_args.h>
#include <geogram/numerics/optimizer.h>
#include <geogram/numerics/lbfgs_optimizers.h>
#include <geogram/third_party/HLBFGS/Lite_Sparse_Matrix.h>
#include <geogram/third_party/HLBFGS/HLBFGS.h>
#include <iostream>

namespace {
    Lite_Sparse_Matrix* m_sparse_matrix = nullptr;

/********************** Initial HLBFGS API ***********************************/

    void evalfunc_C(int N, double* x, double *prev_x, double* f, double* g) {
        GEO::geo_argused(prev_x);
        *f = 0;
        for (int i = 0; i < N; i+=2) {
            double T1 = 1 - x[i];
            double T2 = 10*(x[i+1]-x[i]*x[i]);
            *f += T1*T1+T2*T2;
            g[i+1]   = 20*T2;
            g[i] = -2*(x[i]*g[i+1]+T1);
        }
    }

    void newiteration_C(
        int iter, int call_iter, double *x,
        double* f, double *g,  double* gnorm
    ) {
        GEO::geo_argused(x);
        GEO::geo_argused(g);    
        std::cout << iter <<": " << call_iter <<" "
                  << *f <<" " << *gnorm  << std::endl;
    }
    
    void evalfunc_h_C(
        int N, double *x, double *prev_x, double *f, double *g,
        HESSIAN_MATRIX& hessian
        ) {
        GEO::geo_argused(prev_x);
        
        //the following code is not optimal if the pattern of
        // hessian matrix is fixed.
        if (m_sparse_matrix) {
            delete m_sparse_matrix;
        }

        m_sparse_matrix = new Lite_Sparse_Matrix(
            (unsigned int)N, (unsigned int)N, SYM_LOWER, CCS, FORTRAN_TYPE, true
        );

        m_sparse_matrix->begin_fill_entry();
        
        static bool first = true;
        double *diag = m_sparse_matrix->get_diag();
        
        if (first) {
            // you need to update f and g
            *f = 0;
            double tmp;
            for (unsigned int i = 0; i < (unsigned int)N; i+=2) {
                tmp = x[i]*x[i];
                double T1 = 1 - x[i];
                double T2 = 10*(x[i+1]-tmp);
                *f += T1*T1+T2*T2;
                g[i+1]   = 20*T2;
                g[i] = -2*(x[i]*g[i+1]+T1);
                diag[i] = 2+1200*tmp-400*x[i+1];
                diag[i+1] = 200;
                m_sparse_matrix->fill_entry(i, i+1, -400*x[i]);
            }
        } else {
            for (unsigned int i = 0; i < (unsigned int)N; i+=2) {
                diag[i] = 2+1200*x[i]*x[i]-400*x[i+1];
                diag[i+1] = 200;
                m_sparse_matrix->fill_entry(i, i+1, -400*x[i]);
            }
        }
        m_sparse_matrix->end_fill_entry();
        hessian.set_diag(m_sparse_matrix->get_diag());
        hessian.set_values(m_sparse_matrix->get_values());
        hessian.set_rowind((int*)m_sparse_matrix->get_rowind());
        hessian.set_colptr((int*)m_sparse_matrix->get_colptr());
        hessian.set_nonzeros((int)m_sparse_matrix->get_nonzero());
        first = false;
    }

    void Optimize_by_HLBFGS_C(
        int N, double *init_x, int num_iter, int M, int T, bool with_hessian
    ) {
        double parameter[20];
        int info[20];
        //initialize
        INIT_HLBFGS(parameter, info);
        info[4] = num_iter;
        info[6] = T;
        info[7] = with_hessian?1:0;
        info[10] = 0;
        info[11] = 1;
        
        if (with_hessian) {
            HLBFGS(
                N, M, init_x,
                evalfunc_C, evalfunc_h_C,
                HLBFGS_UPDATE_Hessian, newiteration_C, parameter, info
            );
        } else {
            HLBFGS(
            N, M, init_x,
            evalfunc_C, nullptr,
            HLBFGS_UPDATE_Hessian, newiteration_C, parameter, info
        );
        }
    }

    /********************** Geogram API **************************************/

    void evalfunc(GEO::index_t N, double* x, double& f, double* g) {
        std::cerr << "eval func" << std::endl;
        f = 0.0;
        for (GEO::index_t i = 0; i < N; i+=2) {
            double T1 = 1 - x[i];
            double T2 = 10*(x[i+1]-x[i]*x[i]);
            f += T1*T1+T2*T2;
            g[i+1]   = 20*T2;
            g[i] = -2*(x[i]*g[i+1]+T1);
        }
    }

    void newiteration(
        GEO::index_t iter,
        const double *x, double f, const double *g, double gnorm
    ) {
        GEO::geo_argused(iter);
        GEO::geo_argused(x);
        GEO::geo_argused(g);    
        std::cout << " " << f <<" " << gnorm  << std::endl;
    }

    void evalfunc_h(
        GEO::index_t N, double *x, double& f, double *g,
        HESSIAN_MATRIX& hessian
        ) {
        std::cerr << "eval func with Hessian" << std::endl;
        
        //the following code is not optimal if the pattern of
        // hessian matrix is fixed.
        if (m_sparse_matrix) {
            delete m_sparse_matrix;
        }

        m_sparse_matrix = new Lite_Sparse_Matrix(
            N, N, SYM_LOWER, CCS, FORTRAN_TYPE, true
        );

        m_sparse_matrix->begin_fill_entry();
        
        static bool first = true;
        double *diag = m_sparse_matrix->get_diag();
        
        if (first) {
            // you need to update f and g
            f = 0.0;
            double tmp;
            for (unsigned int i = 0; i < N; i+=2) {
                tmp = x[i]*x[i];
                double T1 = 1 - x[i];
                double T2 = 10*(x[i+1]-tmp);
                f += T1*T1+T2*T2;
                g[i+1]   = 20*T2;
                g[i] = -2*(x[i]*g[i+1]+T1);
                diag[i] = 2+1200*tmp-400*x[i+1];
                diag[i+1] = 200;
                m_sparse_matrix->fill_entry(i, i+1, -400*x[i]);
            }
        } else {
            for (unsigned int i = 0; i < N; i+=2) {
                diag[i] = 2+1200*x[i]*x[i]-400*x[i+1];
                diag[i+1] = 200;
                m_sparse_matrix->fill_entry(i, i+1, -400*x[i]);
            }
        }
        m_sparse_matrix->end_fill_entry();
        hessian.set_diag(m_sparse_matrix->get_diag());
        hessian.set_values(m_sparse_matrix->get_values());
        hessian.set_rowind((int*)m_sparse_matrix->get_rowind());
        hessian.set_colptr((int*)m_sparse_matrix->get_colptr());
        hessian.set_nonzeros((int)m_sparse_matrix->get_nonzero());
        first = false;
    }

    void Optimize_by_HLBFGS(
        int N, double *init_x, int num_iter, int M, int T, bool with_hessian
    ) {
        GEO::Optimizer_var optimizer = with_hessian ?
            GEO::Optimizer::create("HLBFGS_HESS") :
            GEO::Optimizer::create("HLBFGS");


        optimizer->set_newiteration_callback(newiteration);
        if(with_hessian) {
            optimizer->set_evalhessian_callback(evalfunc_h);
        }
        optimizer->set_funcgrad_callback(evalfunc);    
        
        optimizer->set_N((unsigned int)N);
        optimizer->set_M((unsigned int)M);
        optimizer->set_max_iter((unsigned int)num_iter);

        GEO::HLBFGS_HessOptimizer* hess =
            dynamic_cast<GEO::HLBFGS_HessOptimizer*>(
                (GEO::Optimizer*)(optimizer)
            );

        if(hess != nullptr) {
            hess->set_T((unsigned int)T);
        }
        optimizer->optimize(init_x);
    }
}

/****************************************************************************/

int main(int argc, char** argv) {

    GEO::initialize();
    GEO::CmdLine::import_arg_group("standard");
    GEO::CmdLine::declare_arg("Newton",false,"Use Newton solver");
    GEO::CmdLine::declare_arg("C_api",false,"Use HLBFGS C api");    
    GEO::CmdLine::declare_arg("N", 1000, "Nb variables");

    if(!GEO::CmdLine::parse(argc, argv)) {
        return 1;
    }
    
    std::cout.precision(16);
    std::cout << std::scientific;
    
    int N = GEO::CmdLine::get_arg_int("N");
    std::vector<double> x((unsigned int)N);
    
    for (unsigned int i = 0; i < (unsigned int)(N/2); i++) {
        x[2*i]   = -1.2;
        x[2*i+1] =  1.0;
    }
    
    int M = 7;
    int T = 0;

    if(GEO::CmdLine::get_arg_bool("C_api")) {
        if(GEO::CmdLine::get_arg_bool("Newton")) {
            //use Hessian
            // if M = 0, T = 0, it is Newton
            Optimize_by_HLBFGS_C(N, &x[0], 1000, M, T, true);
        } else {
            //without Hessian
            // it is LBFGS(M) actually, T is not used        
            Optimize_by_HLBFGS_C(N, &x[0], 1000, M, T, false);
        }
    } else {
        if(GEO::CmdLine::get_arg_bool("Newton")) {
            //use Hessian
            // if M = 0, T = 0, it is Newton
            Optimize_by_HLBFGS(N, &x[0], 1000, M, T, true);
        } else {
            //without Hessian
            // it is LBFGS(M) actually, T is not used        
            Optimize_by_HLBFGS(N, &x[0], 1000, M, T, false);
        }
    }
    
    if (m_sparse_matrix) {
        delete m_sparse_matrix;
    }

    return 0;
}

#else

#include <iostream>

int main() {
    std::cout << "This geogram was not compiled with HLBFGS support"
	      << std::endl;
    return 0;
}

#endif
