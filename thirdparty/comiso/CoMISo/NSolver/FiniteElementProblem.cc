#include "FiniteElementProblem.hh"


namespace COMISO { 


// FiniteElementProblem


/// Default constructor
FiniteElementProblem::FiniteElementProblem(const unsigned int _n)
: NProblemInterface(), n_(_n), x_(_n,0.0)
{
}

/// Destructor
FiniteElementProblem::~FiniteElementProblem()
{
}

void FiniteElementProblem::add_set(FiniteElementSetBase* _fe_set)
{
  fe_sets_.push_back(_fe_set);
}

void FiniteElementProblem::clear_sets()
{
  fe_sets_.clear();
}

std::vector<double>& FiniteElementProblem::x()
{
  return x_;
}


int    FiniteElementProblem::n_unknowns()
{
  return n_;
}

void   FiniteElementProblem::initial_x(double* _x)
{
  if(n_ > 0)
    memcpy(_x, &(x_[0]), n_*sizeof(double));
}

double FiniteElementProblem::eval_f(const double* _x)
{
  double f(0.0);

  for(unsigned int i=0; i<fe_sets_.size(); ++i)
    f += fe_sets_[i]->eval_f(_x);

  return f;
}

void FiniteElementProblem::eval_gradient( const double* _x, double*    _g)
{
  // clear gradient (assume floating point 0 has only zero bits)
  memset(_g, 0, n_*sizeof(double));

  for(unsigned int i=0; i<fe_sets_.size(); ++i)
    fe_sets_[i]->accumulate_gradient(_x, _g);
}

void FiniteElementProblem::eval_hessian ( const double* _x, SMatrixNP& _H)
{
  triplets_.clear();

  for(unsigned int i=0; i<fe_sets_.size(); ++i)
    fe_sets_[i]->accumulate_hessian(_x, triplets_);

  // set data
  _H.resize(n_unknowns(), n_unknowns());
  _H.setFromTriplets(triplets_.begin(), triplets_.end());
}


void FiniteElementProblem::store_result ( const double* _x )
{
  if(n_ > 0)
    memcpy(&(x_[0]), _x, n_*sizeof(double));
}

// advanced properties (ToDo better handling)
bool FiniteElementProblem::constant_gradient() const
{
  return false;
}
bool FiniteElementProblem::constant_hessian()  const
{
  return false;
}

}


