//=============================================================================
//
//  CLASS CBCSolver
//
//=============================================================================
#ifndef COMISO_CBCSolver_HH
#define COMISO_CBCSolver_HH

//== COMPILE-TIME PACKAGE REQUIREMENTS ========================================
#include <CoMISo/Config/config.hh>
#if COMISO_CBC_AVAILABLE

//== INCLUDES =================================================================

#include <CoMISo/Config/CoMISoDefines.hh>
#include <vector>
#include <string>
#include "NProblemInterface.hh"
#include "NConstraintInterface.hh"
#include "VariableType.hh"

//== FORWARDDECLARATIONS ======================================================
class GRBModel;
class GRBVar;

//== NAMESPACES ===============================================================

namespace COMISO {

//== CLASS DEFINITION =========================================================

/**
    Solver interface for Coin-or Cbc.

    A more elaborate description follows.
*/
class COMISODLLEXPORT CBCSolver
{
public:
  /// Default constructor
  CBCSolver() {}

  /// Destructor
  ~CBCSolver() {}

  // ********** SOLVE **************** //
  //! \throws Outcome
  bool solve(
    NProblemInterface* _problem, // problem instance
    const std::vector<NConstraintInterface*>& _constraints, // linear constraints
    const std::vector<PairIndexVtype>& _discrete_constraints, // discrete constraints
    const double _time_limit = 60); // time limit in seconds

  //! \throws Outcome
 bool solve(
    NProblemInterface* _problem, // problem instance
    const std::vector<NConstraintInterface*>& _constraints, // linear constraints
    const double _time_limit = 60) // time limit in seconds
  {
    std::vector<PairIndexVtype> dc;
    return solve(_problem, _constraints, dc, _time_limit);
  }

//  // same as previous but more control over stopping criteria
//  // the optimizer runs in two phases
//  // phase 1) stops if solution with a MIP gap lower than _gap0 is found or _time_limit0 is reached
//  // phase 2) starts only if in phase 1 no solution with a MIP gap lower than _gap1 was found and
//  //          uses _gap1 and _time_limit2 as parameters
//  inline bool solve_two_phase(NProblemInterface*                  _problem,                // problem instance
//                    std::vector<NConstraintInterface*>& _constraints,            // linear constraints
//                    std::vector<PairIndexVtype>&        _discrete_constraints,   // discrete constraints
//                    const double                        _time_limit0 = 60, // time limit phase 1 in seconds
//                    const double                        _gap0 = 0.001,     // MIP gap phase 1
//                    const double                        _time_limit1 = 120, // time limit phase 2 in seconds
//                    const double                        _gap1 = 0.2,        // MIP gap phase 2
//                    const bool                          _silent = false);
};


//=============================================================================
} // namespace COMISO

//=============================================================================
#endif // COMISO_CBC_AVAILABLE
//=============================================================================
#endif // COMISO_CBCSolver_HH
//=============================================================================

