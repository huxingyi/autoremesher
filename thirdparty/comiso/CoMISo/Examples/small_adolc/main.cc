
//== COMPILE-TIME PACKAGE REQUIREMENTS ========================================
#include <CoMISo/Config/config.hh>
#include <iostream>

#if (COMISO_ADOLC_AVAILABLE && COMISO_EIGEN3_AVAILABLE)

#include <CoMISo/Utils/StopWatch.hh>
#include <vector>
#include <CoMISo/NSolver/LinearConstraint.hh>
#include <CoMISo/NSolver/NPDerivativeChecker.hh>
#include <CoMISo/NSolver/NProblemInterfaceADOLC.hh>
#include <CoMISo/NSolver/IPOPTSolver.hh>


// solve nonlinear Problem f(x,y) = (x-1)^2*(y-2)^4

class Problem1 : public COMISO::NProblemInterfaceADOLC
{
public:

  Problem1() : COMISO::NProblemInterfaceADOLC()
  {}

  virtual int n_unknowns()
  {
    return 2;
  }

  virtual void initial_x(double* _x)
  {
    _x[0]= 0.0;
    _x[1]= 0.0;
  }

  virtual adouble eval_f_adouble(const adouble* _x)
  {
    return pow(_x[0]-1.0,2)*pow(_x[1]-2.0,4) + pow(_x[0]-4.0,6);
  }

  virtual void store_result(const double* _x)
  {
    std::cerr << "result: x=" << _x[0] << ", y=" << _x[1] << std::endl;
  }

  virtual bool constant_gradient() const { return false; }
  virtual bool constant_hessian()  const { return false; }
  virtual bool sparse_gradient() { return false;}
  virtual bool sparse_hessian () { return true;}

};

// Example main
int main(void)
{
  std::cout << "---------- 1) Get an instance of Problem1..." << std::endl;

  Problem1 p1;

  std::cout << "---------- 2) (optional for debugging) Check derivatives of problem..." << std::endl;
  COMISO::NPDerivativeChecker npd;
  npd.check_all(&p1);

// check if IPOPT solver available in current configuration
#if( COMISO_IPOPT_AVAILABLE)
  std::cout << "---------- 3) Get IPOPT solver... " << std::endl;
  COMISO::IPOPTSolver ipsol;

  std::cout << "---------- 4) Solve..." << std::endl;
  // there are no constraints -> provide an empty vector
  std::vector<COMISO::NConstraintInterface*> constraints;
  ipsol.solve(&p1, constraints);
#endif

  // the solution is printed after optimization automatically
  
  return 0;
}

#else

int main(void)
{
  std::cerr << "Warning: Example cannot be executed since either EIGEN3 or ADOLC is not available..." << std::endl;
  return 0;
}

#endif

