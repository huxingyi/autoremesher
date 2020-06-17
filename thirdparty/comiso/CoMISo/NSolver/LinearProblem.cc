//== COMPILE-TIME PACKAGE REQUIREMENTS ========================================
#include <CoMISo/Config/config.hh>
#if COMISO_EIGEN3_AVAILABLE

//== INCLUDES =================================================================
#include "LinearProblem.hh"


namespace COMISO {

LinearProblem::LinearProblem (unsigned int _dimension)
{
  // resize and zero elements
  coeffs_.resize(_dimension,0.0);
  x_.resize(_dimension, 0.0);
}

LinearProblem::~LinearProblem()
{
}

int  LinearProblem::n_unknowns()
{
  return coeffs_.size();
}

void LinearProblem::initial_x(double* _x)
{
  if(!x_.empty())
    memcpy(_x, &(x_[0]), x_.size()*sizeof(double));
}

double LinearProblem::eval_f( const double* _x)
{
  double d(0.0);
  for(unsigned int i=0; i<coeffs_.size(); ++i)
    d += coeffs_[i]*_x[i];
  return d;
}

void LinearProblem::eval_gradient( const double* _x, double* _g)
{
  if(!coeffs_.empty())
    memcpy(_g, &(coeffs_[0]), coeffs_.size()*sizeof(double));
}

void LinearProblem::eval_hessian ( const double* _x, SMatrixNP& _H)
{
  // resize and set to zero
  _H.resize(n_unknowns(), n_unknowns());
  _H.setZero();
}

void LinearProblem::store_result ( const double* _x )
{
  if(!x_.empty())
    memcpy(&(x_[0]), _x, x_.size()*sizeof(double));
}


bool LinearProblem::constant_gradient() const
{
  return true;
}

bool LinearProblem::constant_hessian()  const
{
  return true;
}

std::vector<double>& LinearProblem::coeffs()
{
  return coeffs_;
}

std::vector<double>& LinearProblem::x()
{
  return x_;
}


//=============================================================================
} // namespace COMISO
//=============================================================================
#endif // COMISO_EIGEN3_AVAILABLE
//=============================================================================

