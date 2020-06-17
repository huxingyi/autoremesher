//=============================================================================
//
//  CLASS GUROBISolver
//
//=============================================================================


#ifndef COMISO_GUROBISOLVER_HH
#define COMISO_GUROBISOLVER_HH


//== COMPILE-TIME PACKAGE REQUIREMENTS ========================================
#include <CoMISo/Config/config.hh>
#if COMISO_GUROBI_AVAILABLE

//== INCLUDES =================================================================

#include <CoMISo/Config/CoMISoDefines.hh>
#include <vector>
#include <string>
#include "NProblemInterface.hh"
#include "NConstraintInterface.hh"
#include "VariableType.hh"
#include "GurobiHelper.hh"

#include <gurobi_c++.h>

//== FORWARDDECLARATIONS ======================================================

//== NAMESPACES ===============================================================

namespace COMISO {

//== CLASS DEFINITION =========================================================

	      

/** \class NewtonSolver GUROBISolver.hh

    Brief Description.
  
    A more elaborate description follows.
*/
class COMISODLLEXPORT GUROBISolver
{
public:

  /// Default constructor
  GUROBISolver();
 
  /// Destructor
  ~GUROBISolver() {}

  // ********** SOLVE **************** //
  bool solve(NProblemInterface*                  _problem,                // problem instance
             std::vector<NConstraintInterface*>& _constraints,            // linear constraints
             std::vector<PairIndexVtype>&        _discrete_constraints,   // discrete constraints
             const double                        _time_limit = 60     ); // time limit in seconds

  bool solve(NProblemInterface*                  _problem,                // problem instance
             std::vector<NConstraintInterface*>& _constraints,            // linear constraints
             const double                        _time_limit = 60     ) // time limit in seconds
  {
    std::vector<PairIndexVtype> dc; return solve(_problem, _constraints, dc, _time_limit);
  }

  // same as previous but more control over stopping criteria
  // the optimizer runs in two phases
  // phase 1) stops if solution with a MIP gap lower than _gap0 is found or _time_limit0 is reached
  // phase 2) starts only if in phase 1 no solution with a MIP gap lower than _gap1 was found and
  //          uses _gap1 and _time_limit2 as parameters
  bool solve_two_phase(NProblemInterface*                  _problem,                // problem instance
             std::vector<NConstraintInterface*>& _constraints,            // linear constraints
             std::vector<PairIndexVtype>&        _discrete_constraints,   // discrete constraints
             const double                        _time_limit0 = 60, // time limit phase 1 in seconds
             const double                        _gap0 = 0.001,     // MIP gap phase 1
             const double                        _time_limit1 = 120, // time limit phase 2 in seconds
             const double                        _gap1 = 0.2 );       // MIP gap phase 2
  
  bool solve_two_phase(NProblemInterface*                  _problem,                // problem instance
                       std::vector<NConstraintInterface*>& _constraints,            // linear constraints
                       std::vector<PairIndexVtype>&        _discrete_constraints,   // discrete constraints
                       const double                        _time_limit0, // time limit phase 1 in seconds
                       const double                        _gap0 ,     // MIP gap phase 1
                       const double                        _time_limit1, // time limit phase 2 in seconds
                       const double                        _gap1,  // MIP gap phase 2
                       double&                             _final_gap);  //return final gap


  // optimization with additional lazy constraints that are only added iteratively to the problem if not satisfied
  bool solve(NProblemInterface*                        _problem,
                    const std::vector<NConstraintInterface*>& _constraints,
                    const std::vector<NConstraintInterface*>& _lazy_constraints,
                    std::vector<PairIndexVtype>&              _discrete_constraints,   // discrete constraints
                    const double                              _almost_infeasible = 0.5,
                    const int                                 _max_passes        = 5,
                    const double                              _time_limit = 60,
                    const bool                                _silent = false);


  // same as above with additional lazy constraints that are only added iteratively to the problem if not satisfied
  bool solve(NProblemInterface*                        _problem,
                    const std::vector<NConstraintInterface*>& _constraints,
                    const std::vector<NConstraintInterface*>& _lazy_constraints,
                    const double                              _almost_infeasible = 0.5,
                    const int                                 _max_passes        = 5,
                    const double                              _time_limit = 60,
                    const bool                                _silent = false)
  {
    std::vector<PairIndexVtype> dc; return solve(_problem, _constraints, _lazy_constraints, dc, _almost_infeasible, _max_passes, _time_limit, _silent);
  }


  void set_problem_output_path    ( const std::string &_problem_output_path);
  void set_problem_env_output_path( const std::string &_problem_env_output_path);
  void set_solution_input_path    ( const std::string &_solution_input_path);

protected:
  double* P(std::vector<double>& _v)
  {
    if( !_v.empty())
      return ((double*)&_v[0]);
    else
      return 0;
  }

private:
  void add_constraint_to_model(COMISO::NConstraintInterface* _constraint, GRBModel& _model, std::vector<GRBVar>& _vars, double * _x);

private:

  // filenames for exporting/importing gurobi solutions
  // if string is empty nothing is imported or exported
  std::string problem_output_path_;
  std::string problem_env_output_path_;
  std::string solution_input_path_;
};



//=============================================================================
} // namespace COMISO

//=============================================================================
#endif // COMISO_GUROBI_AVAILABLE
//=============================================================================
#endif // ACG_GUROBISOLVER_HH defined
//=============================================================================

