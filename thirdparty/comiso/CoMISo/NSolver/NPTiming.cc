//=============================================================================
//
//  CLASS NPTiming
//
//=============================================================================


//== INCLUDES =================================================================

#include <iostream>
#include <iomanip>

#include "NPTiming.hh"

//== FORWARDDECLARATIONS ======================================================

//== NAMESPACES ===============================================================

namespace COMISO {

//== CLASS DEFINITION =========================================================

/// Default constructor
NPTiming::NPTiming(NProblemInterface* _base) : base_(_base) {start_timing();}

/// Destructor
NPTiming::~NPTiming() {}

int NPTiming::n_unknowns   ()
{
  return base_->n_unknowns();
}

void NPTiming::initial_x( double* _x )
{
  base_->initial_x(_x);
}

double NPTiming::eval_f( const double* _x )
{
  ++n_eval_f_;
  sw_.start();
  double f = base_->eval_f(_x);
  timing_eval_f_ += sw_.stop();
  return f;
}

void NPTiming::eval_gradient( const double* _x, double*    _g)
{
  ++n_eval_gradient_;
  sw_.start();
  base_->eval_gradient(_x, _g);
  timing_eval_gradient_ += sw_.stop();
}

void NPTiming::eval_hessian ( const double* _x, SMatrixNP& _H)
{
  ++n_eval_hessian_;
  sw_.start();
  base_->eval_hessian(_x, _H);
  timing_eval_hessian_ += sw_.stop();
}

void NPTiming::store_result ( const double* _x )
{
  base_->store_result(_x);
  print_statistics();
}

bool NPTiming::constant_gradient() const
{
  return base_->constant_gradient();
}

bool NPTiming::constant_hessian()  const
{
  return base_->constant_hessian();
}


void NPTiming::start_timing()
{
  swg_.start();

  timing_eval_f_ = 0.0;
  timing_eval_gradient_ = 0.0;
  timing_eval_hessian_ = 0.0;

  n_eval_f_ = 0;
  n_eval_gradient_ = 0;
  n_eval_hessian_ = 0;
}


void NPTiming::print_statistics()
{
  double time_total = swg_.stop();

  double time_np = timing_eval_f_ + timing_eval_gradient_ + timing_eval_hessian_;



  std::cerr << "######## NP-Timings ########" << std::endl;
  std::cerr << "total time    : " << time_total/1000.0 << "s\n";
  std::cerr << "total time NP : " << time_np/1000.0 << "s  (" << time_np/time_total*100.0 << " %)\n";

  double timing_eval_f_avg        = timing_eval_f_/double(n_eval_f_);
  double timing_eval_gradient_avg = timing_eval_gradient_/double(n_eval_gradient_);
  double timing_eval_hessian_avg  = timing_eval_hessian_/double(n_eval_hessian_);

  std::cerr << std::fixed << std::setprecision(5)
  << "eval_f time   : " << timing_eval_f_/1000.0
  << "s  ( #evals: " << n_eval_f_ << " -> avg "
  << timing_eval_f_avg/1000.0 << "s )\n"
  << "eval_grad time: " << timing_eval_gradient_/1000.0
  << "s  ( #evals: " << n_eval_gradient_ << " -> avg "
  << timing_eval_gradient_avg/1000.0 << "s, factor: " <<  timing_eval_gradient_avg / timing_eval_f_avg << ")\n"
  << "eval_hess time: " << timing_eval_hessian_/1000.0
  << "s  ( #evals: " << n_eval_hessian_ << " -> avg "
  << timing_eval_hessian_avg/1000.0 << "s, factor: " <<  timing_eval_hessian_avg / timing_eval_f_avg << ")\n";
}


//=============================================================================
} // namespace COMISO
//=============================================================================

