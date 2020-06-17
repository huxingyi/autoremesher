//=============================================================================
//
//  CLASS LinearProblem
//
//=============================================================================


#ifndef COMISO_LINEARPROBLEM_HH
#define COMISO_LINEARPROBLEM_HH


//== COMPILE-TIME PACKAGE REQUIREMENTS ========================================
#include <CoMISo/Config/config.hh>
#if COMISO_EIGEN3_AVAILABLE

//== INCLUDES =================================================================

#include <stdio.h>
#include <iostream>
#include <vector>

#include <CoMISo/Config/CoMISoDefines.hh>
#include <CoMISo/NSolver/NProblemInterface.hh>

//== FORWARDDECLARATIONS ======================================================

//== NAMESPACES ===============================================================

namespace COMISO {

//== CLASS DEFINITION =========================================================

	      

/** \class LinearProblem LinearProblem.hh <CoMISo/NSolver/LinearProblem.hh>

    Brief Description.
  
    A more elaborate description follows.
*/
class COMISODLLEXPORT LinearProblem : public NProblemInterface
{
public:
  
  /// Default constructor
  LinearProblem (unsigned int _dimension = 0);
 
  /// Destructor
  virtual ~LinearProblem();

  // problem definition
  virtual int    n_unknowns();

  virtual void   initial_x(double* _x);

  virtual double eval_f( const double* _x);

  virtual void   eval_gradient( const double* _x, double* _g);

  virtual void   eval_hessian ( const double* _x, SMatrixNP& _H);

  virtual void   store_result ( const double* _x );

  // advanced properties
  virtual bool   constant_gradient() const;
  virtual bool   constant_hessian()  const;

  // give access to coefficients to define the problem
  std::vector<double>& coeffs();

  // give access to result
  std::vector<double>& x();

private:

  // coefficients of linear problem
  std::vector<double> coeffs_;
  std::vector<double> x_;
};


//=============================================================================
} // namespace COMISO
//=============================================================================
#endif // COMISO_EIGEN3_AVAILABLE
//=============================================================================
#endif // COMISO_NPROBLEMGMMINTERFACE_HH defined
//=============================================================================

