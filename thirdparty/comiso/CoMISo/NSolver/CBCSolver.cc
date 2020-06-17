//=============================================================================
//
//  CLASS CBCSolver - IMPLEMENTATION
//
//=============================================================================

//== INCLUDES =================================================================

//== COMPILE-TIME PACKAGE REQUIREMENTS ========================================
#include <CoMISo/Config/config.hh>
#if COMISO_CBC_AVAILABLE

//=============================================================================
#include "CBCSolver.hh"

// For Branch and bound
#include "OsiSolverInterface.hpp"
#include "CbcModel.hpp"
#include "CbcCutGenerator.hpp"
#include "CbcStrategy.hpp"
#include "CbcHeuristicLocal.hpp"
#include "OsiClpSolverInterface.hpp"

// Cuts
#include <CbcModel.hpp>
#include <OsiClpSolverInterface.hpp>

#include "CglGomory.hpp"
#include "CglProbing.hpp"
#include "CglKnapsackCover.hpp"
#include "CglRedSplit.hpp"
#include "CglClique.hpp"
#include "CglFlowCover.hpp"
#include "CglMixedIntegerRounding2.hpp"

// Preprocessing
#include "CglPreProcess.hpp"

// Heuristics
#include "CbcHeuristic.hpp"
#include "CbcCompareDepth.hpp"

#include <stdexcept>

#define CBC_INFINITY COIN_DBL_MAX

//== NAMESPACES ===============================================================

namespace COMISO {

//== IMPLEMENTATION ==========================================================

// These are some "tweaks" for the CbcModel, copied from some sample Cbc code
// I have no idea what any of these do!
void model_tweak(CbcModel& model)
{
  CglProbing generator1;
  generator1.setUsingObjective(true);
  generator1.setMaxPass(1);
  generator1.setMaxPassRoot(5);
  // Number of unsatisfied variables to look at
  generator1.setMaxProbe(10);
  generator1.setMaxProbeRoot(1000);
  // How far to follow the consequences
  generator1.setMaxLook(50);
  generator1.setMaxLookRoot(500);
  // Only look at rows with fewer than this number of elements
  generator1.setMaxElements(200);
  generator1.setRowCuts(3);

  CglGomory generator2;
  // try larger limit
  generator2.setLimit(300);

  CglKnapsackCover generator3;

  CglRedSplit generator4;
  // try larger limit
  generator4.setLimit(200);

  CglClique generator5;
  generator5.setStarCliqueReport(false);
  generator5.setRowCliqueReport(false);

  CglMixedIntegerRounding2 mixedGen;
  CglFlowCover flowGen;

  // Add in generators
  // Experiment with -1 and -99 etc
  //model.addCutGenerator(&generator1, -1, "Probing");
  model.addCutGenerator(&generator2, -1, "Gomory");
  //model.addCutGenerator(&generator3, -1, "Knapsack");
  //model.addCutGenerator(&generator4,-1,"RedSplit");
  //model.addCutGenerator(&generator5, -1, "Clique");
  //model.addCutGenerator(&flowGen, -1, "FlowCover");
  //model.addCutGenerator(&mixedGen, -1, "MixedIntegerRounding");
  // Say we want timings
  int numberGenerators = model.numberCutGenerators();
  int iGenerator;
  for (iGenerator = 0; iGenerator < numberGenerators; iGenerator++)
  {
    CbcCutGenerator* generator = model.cutGenerator(iGenerator);
    generator->setTiming(true);
  }
  //auto osiclp = dynamic_cast<OsiClpSolverInterface*>(model.solver());
  //// go faster stripes
  //if (osiclp)
  //{
  //  // Turn this off if you get problems
  //  // Used to be automatically set
  //  osiclp->setSpecialOptions(128);
  //  if (osiclp->getNumRows() < 300 && osiclp->getNumCols() < 500)
  //  {
  //    //osiclp->setupForRepeatedUse(2,0);
  //    osiclp->setupForRepeatedUse(0, 0);
  //  }
  //}
  // Uncommenting this should switch off all CBC messages
  // model.messagesPointer()->setDetailMessages(10,10000,NULL);
  // Allow rounding heuristic

  CbcRounding heuristic1(model);
  model.addHeuristic(&heuristic1);

  // And local search when new solution found
  //CbcHeuristicLocal heuristic2(model);
  //model.addHeuristic(&heuristic2);
}

#define P(X) ((X).data())

bool solve_impl(
  NProblemInterface*                        _problem,
  const std::vector<NConstraintInterface*>& _constraints,
  const std::vector<PairIndexVtype>&        _discrete_constraints,
  const double                              _time_limit
)
{
  if(!_problem->constant_hessian())
    std::cerr << "Warning: CBCSolver received a problem with non-constant hessian!" << std::endl;
  if(!_problem->constant_gradient())
    std::cerr <<  "Warning: CBCSolver received a problem with non-constant gradient!" << std::endl;

  const int n_rows = _constraints.size(); // Constraints #
  const int n_cols = _problem->n_unknowns(); // Unknowns #

  // expand the variable types from discrete mtrx array
  std::vector<VariableType> var_type(n_cols, Real);
  for (size_t i = 0; i < _discrete_constraints.size(); ++i)
    var_type[_discrete_constraints[i].first] = _discrete_constraints[i].second;

  //----------------------------------------------
  // set up constraints
  //----------------------------------------------
  std::vector<double> x(n_cols, 0.0); // solution

  CoinPackedMatrix mtrx(false, 0, 0);// make a row-ordered, empty mtrx
  mtrx.setDimensions(0, n_cols);

  std::vector<double> row_lb(n_rows, -CBC_INFINITY);
  std::vector<double> row_ub(n_rows,  CBC_INFINITY);

  for (size_t i = 0; i < _constraints.size();  ++i)
  {
    if(!_constraints[i]->is_linear())
      std::cerr << "Warning: constraint " << i << " is non-linear" << std::endl;

    NConstraintInterface::SVectorNC gc;
    _constraints[i]->eval_gradient(P(x), gc);

    // the setup below appears inefficient, the interface is rather restrictive
    CoinPackedVector lin_expr;
    lin_expr.reserve(gc.size());
    for (NConstraintInterface::SVectorNC::InnerIterator v_it(gc); v_it; ++v_it)
      lin_expr.insert(v_it.index(), v_it.value());
    mtrx.appendRow(lin_expr);

    const double b = -_constraints[i]->eval_constraint(P(x));
    switch (_constraints[i]->constraint_type())
    {
    case NConstraintInterface::NC_EQUAL         :
      row_lb[i] = row_ub[i] = b;
      break;
    case NConstraintInterface::NC_LESS_EQUAL    :
      row_ub[i] = b;
      break;
    case NConstraintInterface::NC_GREATER_EQUAL :
      row_lb[i] = b;
      break;
    }

//    TRACE_CBT("Constraint " << i << " is of type " <<
//              _constraints[i]->constraint_type() << "; RHS val", b);
  }

  //----------------------------------------------
  // setup energy
  //----------------------------------------------

  std::vector<double> objective(n_cols);
  _problem->eval_gradient(P(x), P(objective));
//  TRACE_CBT("Objective linear term", objective);

  const double c = _problem->eval_f(P(x));
  // ICGB: Where does objective constant term go in CBC?
  // MCM: I could not find this either: It is possible that the entire model
  // can be translated to accomodate the constant (if != 0). Ask DB!
//  DEB_error_if(c > FLT_EPSILON, "Ignoring a non-zero constant objective term: "
//               << c);
//  TRACE_CBT("Objective constant term", c);

  // CBC Problem initialize
  OsiClpSolverInterface si;
  si.setHintParam(OsiDoReducePrint, true, OsiHintTry);

  const std::vector<double> col_lb(n_cols, -CBC_INFINITY);
  const std::vector<double> col_ub(n_cols,  CBC_INFINITY);
  si.loadProblem(mtrx, P(col_lb), P(col_ub), P(objective), P(row_lb), P(row_ub));

  // set variables
  for (int i = 0; i < n_cols; ++i)
  {
    switch (var_type[i])
    {
    case Real:
      si.setContinuous(i);
      break;
    case Integer:
      si.setInteger(i);
      break;
    case Binary :
      si.setInteger(i);
      si.setColBounds(i, 0.0, 1.0);
      break;
    }
  }
  si.initialSolve();

  // TODO: make this accessible through the DEB system instead
  volatile static bool dump_problem = false; // change on run-time if necessary
  if (dump_problem)
    si.writeMps("CBC_problem_dump"); //output problem as .MPS

  // Pass the OsiSolver with the problem to be solved to CbcModel
  CbcModel model(si);
  model.solver()->setHintParam(OsiDoReducePrint, true, OsiHintTry);
//  model.setMaximumSolutions(4);
//  TRACE_CBT("CbcModel::getMaximumSolutions()", model.getMaximumSolutions());
  model.setMaximumSeconds(_time_limit);
//  TRACE_CBT("CbcModel::getMaximumSeconds()", model.getMaximumSeconds());

  model_tweak(model);

  CbcCompareDepth compare_depth;
  model.setNodeComparison(&compare_depth);

  model.branchAndBound();

  const double* solution = model.bestSolution();
//  THROW_OUTCOME_if(solution == nullptr, UNSPECIFIED_CBC_EXCEPTION);

  // store if solution available
  if(solution != 0)
  {
    _problem->store_result(solution);
    return true;
  }
  else
    return false;
}

#undef P

bool CBCSolver::solve(
  NProblemInterface*                        _problem,
  const std::vector<NConstraintInterface*>& _constraints,
  const std::vector<PairIndexVtype>&        _discrete_constraints,
  const double                              _time_limit
)
{
  bool valid_solution = false;
  try
  {
    valid_solution = solve_impl(_problem, _constraints, _discrete_constraints, _time_limit);
  }
  catch (CoinError& ce)
  {
    std::cerr << "CoinError code = " << ce.message() << std::endl;
//    THROW_OUTCOME(UNSPECIFIED_CBC_EXCEPTION);
  }
  return valid_solution;
}


//=============================================================================
} // namespace COMISO
//=============================================================================

#endif // COMISO_CBC_AVAILABLE
//=============================================================================

