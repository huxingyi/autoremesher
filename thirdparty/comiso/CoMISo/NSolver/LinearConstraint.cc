//=============================================================================
//
//  CLASS LinearConstraint
//
//=============================================================================


#ifndef COMISO_LINEARCONSTRAINT_CC
#define COMISO_LINEARCONSTRAINT_CC


//== INCLUDES =================================================================

#include <CoMISo/Config/CoMISoDefines.hh>
#include "NConstraintInterface.hh"
#include "LinearConstraint.hh"

//== FORWARDDECLARATIONS ======================================================

//== NAMESPACES ===============================================================

namespace COMISO {

//== Implementation =========================================================

      
/// Default constructor
LinearConstraint::LinearConstraint(const ConstraintType _type) : NConstraintInterface(_type)
{}

// linear equation of the form -> coeffs_^T *x  + b_=_type= 0
LinearConstraint::LinearConstraint(const SVectorNC& _coeffs, const double _b, const ConstraintType _type) : NConstraintInterface(_type)
{
  coeffs_ = _coeffs;
  b_ = _b;
}

/// Destructor
LinearConstraint::~LinearConstraint() {}

int LinearConstraint::n_unknowns()
{
  return coeffs_.innerSize();
}

void LinearConstraint::resize(const unsigned int _n)
{
  if(coeffs_.innerSize() != (int)_n)
  {
    // resize while maintaining all values in range
    SVectorNC coeffs_new(_n);
    coeffs_new.setZero();
    coeffs_new.reserve(coeffs_.nonZeros());

    SVectorNC::InnerIterator it(coeffs_);
    for(; it; ++it)
      if(it.index() < SVectorNC::Index(_n))
        coeffs_new.insertBack(it.index()) = it.value();

    coeffs_.swap(coeffs_new);
    //  coeffs_.m_size = _n;
    //  coeffs_.resize(_n);
  }
}

void LinearConstraint::clear()
{
  coeffs_.setZero();
  b_ = 0.0;
}


const LinearConstraint::SVectorNC& LinearConstraint::coeffs() const
{
  return coeffs_;
}
LinearConstraint::SVectorNC& LinearConstraint::coeffs() 
{ 
  return coeffs_;
}
const double&    LinearConstraint::b() const
{
  return b_;
}
double&    LinearConstraint::b()      
{ 
  return b_;
}

double LinearConstraint::eval_constraint ( const double* _x )
{
  double v = b_;

  SVectorNC::InnerIterator c_it(coeffs_);
  for(; c_it; ++c_it)
    v += c_it.value()*_x[c_it.index()];

  return v;
}

void LinearConstraint::eval_gradient( const double* _x, SVectorNC& _g      )
{
  _g = coeffs_;
}

void LinearConstraint::eval_hessian    ( const double* _x, SMatrixNC& _h      )
{
  _h.clear();
  _h.resize(coeffs_.innerSize(), coeffs_.innerSize());
}


//=============================================================================
} // namespace COMISO
//=============================================================================
#endif // ACG_LINEARCONSTRAINT_HH defined
//=============================================================================

