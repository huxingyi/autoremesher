//=============================================================================
//
//  CLASS NPTiming
//
//=============================================================================


#ifndef COMISO_NPTIMING_HH
#define COMISO_NPTIMING_HH


//== INCLUDES =================================================================

#include <CoMISo/Utils/StopWatch.hh>
#include <gmm/gmm.h>
#include "NProblemInterface.hh"
#include <CoMISo/Config/CoMISoDefines.hh>

//== FORWARDDECLARATIONS ======================================================

//== NAMESPACES ===============================================================

namespace COMISO {

//== CLASS DEFINITION =========================================================

	      

/** \class NProblemGmmInterface NProblemGmmInterface.hh <ACG/.../NProblemGmmInterface.hh>

    Brief Description.
  
    A more elaborate description follows.
*/
class COMISODLLEXPORT NPTiming : public NProblemInterface
{
public:
  
  /// Default constructor
  NPTiming(NProblemInterface* _base);
 
  /// Destructor
  ~NPTiming();

  virtual int    n_unknowns   ();

  virtual void   initial_x( double* _x );

  virtual double eval_f( const double* _x );

  virtual void   eval_gradient( const double* _x, double*    _g);

  virtual void   eval_hessian ( const double* _x, SMatrixNP& _H);

  virtual void   store_result ( const double* _x );

  // advanced properties
  virtual bool   constant_gradient() const;
  virtual bool   constant_hessian()  const;


  void start_timing();

protected:

  void print_statistics();

private:
  NProblemInterface* base_;
  StopWatch swg_;
  StopWatch sw_;

  // timings
  double timing_eval_f_;
  double timing_eval_gradient_;
  double timing_eval_hessian_;

  // number of function executions
  int n_eval_f_;
  int n_eval_gradient_;
  int n_eval_hessian_;
};


//=============================================================================
} // namespace COMISO
//=============================================================================
#endif // COMISO_NPTIMING_HH defined
//=============================================================================

