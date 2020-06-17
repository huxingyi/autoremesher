//=============================================================================
//
//  CLASS GCPLEXSolver - IMPLEMENTATION
//
//=============================================================================

#define COMISO_CPLEXSOLVER_C

//== INCLUDES =================================================================

//== COMPILE-TIME PACKAGE REQUIREMENTS ========================================
#include "CPLEXSolver.hh"
#include "LinearConstraint.hh"
#include "BoundConstraint.hh"
#include "ConeConstraint.hh"
#include <CoMISo/Utils/StopWatch.hh>

#if COMISO_CPLEX_AVAILABLE
//=============================================================================


#include <stdexcept>

//== NAMESPACES ===============================================================

namespace COMISO {

//== IMPLEMENTATION ========================================================== 



// ********** SOLVE **************** //
bool
CPLEXSolver::
solve2(NProblemInterface*                  _problem,
      std::vector<NConstraintInterface*>& _constraints,
      std::vector<PairIndexVtype>&        _discrete_constraints,
      const double                        _time_limit,
      const bool                          _silent)
{
  try
  {
    //----------------------------------------------
    // 0. set up environment
    //----------------------------------------------
    if(!_silent)
      std::cerr << "cplex -> get environment...\n";
    IloEnv env_;

    if(!_silent)
      std::cerr << "cplex -> get model...\n";
    IloModel model(env_);
    //    model.getEnv().set(GRB_DoubleParam_TimeLimit, _time_limit);

    //----------------------------------------------
    // 1. allocate variables
    //----------------------------------------------
    if(!_silent)
      std::cerr << "cplex -> allocate variables...\n";
    // determine variable types: 0->real, 1->integer, 2->bool
    std::vector<char> vtypes(_problem->n_unknowns(),0);
    for(unsigned int i=0; i<_discrete_constraints.size(); ++i)
      if(_discrete_constraints[i].first < vtypes.size())
      {
        switch(_discrete_constraints[i].second)
        {
          case Integer: vtypes[_discrete_constraints[i].first] = 1; break;
          case Binary : vtypes[_discrete_constraints[i].first] = 2; break;
          default     : break;
        }
      }
      else
        std::cerr << "ERROR: requested a discrete variable which is above the total number of variables"
                  << _discrete_constraints[i].first << " vs " << vtypes.size() << std::endl;

    // CPLEX variables
    std::vector<IloNumVar> vars; vars.reserve(_problem->n_unknowns());
    // first all
    for( int i=0; i<_problem->n_unknowns(); ++i)
      switch(vtypes[i])
      {
        case 0 : vars.push_back( IloNumVar(env_,-IloInfinity, IloInfinity, IloNumVar::Float) ); break;
        case 1 : vars.push_back( IloNumVar(env_,  -IloIntMax,   IloIntMax, IloNumVar::Int)   ); break;
        case 2 : vars.push_back( IloNumVar(env_,           0,           1, IloNumVar::Bool)  ); break;
      }

    if(!_silent)
    {
      std::cerr << "#unknowns       : " << _problem->n_unknowns() << std::endl;
      std::cerr << "#CPLEX variables: " << _problem->n_unknowns() << std::endl;
    }

    // Integrate new variables
//    model.update();

    //----------------------------------------------
    // 2. setup constraints
    //----------------------------------------------
    if(!_silent)
      std::cerr << "cplex -> setup constraints...\n";

    // get zero vector
    std::vector<double> x(_problem->n_unknowns(), 0.0);

    for(unsigned int i=0; i<_constraints.size();  ++i)
    {
      if(!_constraints[i]->is_linear())
        std::cerr << "Warning: CPLEXSolver received a problem with non-linear constraints!!!" << std::endl;

      IloExpr lin_expr(env_);
      NConstraintInterface::SVectorNC gc;
      _constraints[i]->eval_gradient(P(x), gc);

      NConstraintInterface::SVectorNC::InnerIterator v_it(gc);
      for(; v_it; ++v_it)
        lin_expr += vars[v_it.index()]*v_it.value();

      double b = _constraints[i]->eval_constraint(P(x));


      switch(_constraints[i]->constraint_type())
      {
        case NConstraintInterface::NC_EQUAL         : model.add(lin_expr + b == 0); break;
        case NConstraintInterface::NC_LESS_EQUAL    : model.add(lin_expr + b <= 0); break;
        case NConstraintInterface::NC_GREATER_EQUAL : model.add(lin_expr + b >= 0); break;
      }

    }

//    model.update();

    //----------------------------------------------
    // 3. setup energy
    //----------------------------------------------
    if(!_silent)
      std::cerr << "cplex -> setup energy...\n";

    if(!_problem->constant_hessian())
      std::cerr << "Warning: CPLEXSolver received a problem with non-constant hessian!!!" << std::endl;

//    GRBQuadExpr objective;
    IloExpr objective(env_);

    // add quadratic part
    NProblemInterface::SMatrixNP H;
    _problem->eval_hessian(P(x), H);
    for( int i=0; i<H.outerSize(); ++i)
      for (NProblemInterface::SMatrixNP::InnerIterator it(H,i); it; ++it)
        objective += 0.5*it.value()*vars[it.row()]*vars[it.col()];


    // add linear part
    std::vector<double> g(_problem->n_unknowns());
    _problem->eval_gradient(P(x), P(g));
    for(unsigned int i=0; i<g.size(); ++i)
      objective += g[i]*vars[i];

    // add constant part
    objective += _problem->eval_f(P(x));

    model.add(IloMinimize(env_,objective));

//    model.set(GRB_IntAttr_ModelSense, 1);
//    model.setObjective(objective);
//    model.update();


    //----------------------------------------------
    // 4. solve problem
    //----------------------------------------------
    if(!_silent)
      std::cerr << "cplex -> generate model...\n";
    IloCplex cplex(model);
    cplex.setParam(IloCplex::TiLim, _time_limit);
    { // hack
//    0 [CPX_NODESEL_DFS] Depth-first search
//    1 [CPX_NODESEL_BESTBOUND] Best-bound search
//    2 [CPX_NODESEL_BESTEST] Best-estimate search
//    3 [CPX_NODESEL_BESTEST_ALT] Alternative best-estimate search
//      cplex.setParam(IloCplex::NodeSel , 0);
    }
    if(!_silent)
      std::cerr << "cplex -> solve...\n";

    // silent mode?
    if(_silent)
      cplex.setOut(env_.getNullStream());

    IloBool solution_found = cplex.solve();


//    if (solution_input_path_.empty())
//    {
//      if (!problem_env_output_path_.empty())
//      {
//        std::cout << "Writing problem's environment into file \"" << problem_env_output_path_ << "\"." << std::endl;
//        model.getEnv().writeParams(problem_env_output_path_);
//      }
//      if (!problem_output_path_.empty())
//      {
//        std::cout << "Writing problem into file \"" << problem_output_path_ << "\"." << std::endl;
//        GurobiHelper::outputModelToMpsGz(model, problem_output_path_);
//      }
//
//      model.optimize();
//    }
//    else
//    {
//        std::cout << "Reading solution from file \"" << solution_input_path_ << "\"." << std::endl;
//    }
//
    //----------------------------------------------
    // 5. store result
    //----------------------------------------------

    if(solution_found != IloFalse)
    {
      if(!_silent)
        std::cerr << "cplex -> store result...\n";
      // store computed result
      for(unsigned int i=0; i<vars.size(); ++i)
        x[i] = cplex.getValue(vars[i]);

      _problem->store_result(P(x));
    }

/*
    if (solution_input_path_.empty())
    {
      // store computed result
      for(unsigned int i=0; i<vars.size(); ++i)
        x[i] = vars[i].get(GRB_DoubleAttr_X);
    }
*/
//    else
//    {
//        std::cout << "Loading stored solution from \"" << solution_input_path_ << "\"." << std::endl;
//        // store loaded result
//        const size_t oldSize = x.size();
//        x.clear();
//        GurobiHelper::readSolutionVectorFromSOL(x, solution_input_path_);
//        if (oldSize != x.size()) {
//            std::cerr << "oldSize != x.size() <=> " << oldSize << " != " << x.size() << std::endl;
//            throw std::runtime_error("Loaded solution vector doesn't have expected dimension.");
//        }
//    }
//
//    _problem->store_result(P(x));
//
//    // ObjVal is only available if the optimize was called.
//    if (solution_input_path_.empty())
//        std::cout << "GUROBI Objective: " << model.get(GRB_DoubleAttr_ObjVal) << std::endl;
    return solution_found;
  }
  catch (IloException& e)
  {
     cerr << "Concert exception caught: " << e << endl;
     return false;
  }
  catch (...)
  {
     cerr << "Unknown exception caught" << endl;
     return false;
  }

  return false;
}


//-----------------------------------------------------------------------------

bool
CPLEXSolver::
solve(NProblemInterface*                  _problem,
      std::vector<NConstraintInterface*>& _constraints,
      std::vector<PairIndexVtype>&        _discrete_constraints,
      const double                        _time_limit,
      const bool                          _silent)
{
  try
  {
    //----------------------------------------------
    // 0. set up environment
    //----------------------------------------------
    if(!_silent)
      std::cerr << "cplex -> get environment...\n";
    IloEnv env_;

    if(!_silent)
      std::cerr << "cplex -> get model...\n";
    IloModel model(env_);
    //    model.getEnv().set(GRB_DoubleParam_TimeLimit, _time_limit);

    //----------------------------------------------
    // 1. allocate variables and initialize limits
    //----------------------------------------------
    if(!_silent)
      std::cerr << "cplex -> allocate variables...\n";
    // determine variable types: 0->real, 1->integer, 2->bool
    std::vector<char>   vtypes (_problem->n_unknowns(),0);

    for(unsigned int i=0; i<_discrete_constraints.size(); ++i)
      if(_discrete_constraints[i].first < vtypes.size())
      {
        switch(_discrete_constraints[i].second)
        {
          case Integer: vtypes [_discrete_constraints[i].first] = 1;
                        break;
          case Binary : vtypes[_discrete_constraints[i].first]  = 2;
                        break;
          default     : break;
        }
      }
      else
        std::cerr << "ERROR: requested a discrete variable which is above the total number of variables"
                  << _discrete_constraints[i].first << " vs " << vtypes.size() << std::endl;

    // CPLEX variables
    std::vector<IloNumVar> vars; vars.reserve(_problem->n_unknowns());
    // first all
    for( int i=0; i<_problem->n_unknowns(); ++i)
      switch(vtypes[i])
      {
        case 0 : vars.push_back( IloNumVar(env_,-IloInfinity, IloInfinity, IloNumVar::Float) ); break;
        case 1 : vars.push_back( IloNumVar(env_,  -IloIntMax,   IloIntMax, IloNumVar::Int)   ); break;
        case 2 : vars.push_back( IloNumVar(env_,           0,           1, IloNumVar::Bool)  ); break;
        default: std::cerr << "ERROR: unhandled varibalbe type in CPLEXSolver!!!" << std::endl;
      }

    if(!_silent)
    {
      std::cerr << "#unknowns       : " << _problem->n_unknowns() << std::endl;
      std::cerr << "#CPLEX variables: " << vars.size() << std::endl;
    }


    // Integrate new variables
//    model.update();

    //----------------------------------------------
    // 2. setup constraints
    //----------------------------------------------
    if(!_silent)
      std::cerr << "cplex -> setup constraints...\n";

    // get zero vector
    std::vector<double> x(_problem->n_unknowns(), 0.0);

    // handle constraints depending on their tyep
    for(unsigned int i=0; i<_constraints.size();  ++i)
    {
      // is linear constraint?
      LinearConstraint* lin_ptr = dynamic_cast<LinearConstraint*>(_constraints[i]);
      if(lin_ptr)
      {
        IloExpr lin_expr(env_);
        NConstraintInterface::SVectorNC gc;
        _constraints[i]->eval_gradient(P(x), gc);

        NConstraintInterface::SVectorNC::InnerIterator v_it(gc);
        for(; v_it; ++v_it)
        {
          assert(v_it.index() >= 0 && v_it.index() < _problem->n_unknowns());
          lin_expr += vars[v_it.index()]*v_it.value();
        }

        double b = _constraints[i]->eval_constraint(P(x));


        switch(_constraints[i]->constraint_type())
        {
          case NConstraintInterface::NC_EQUAL         : model.add(lin_expr + b == 0); break;
          case NConstraintInterface::NC_LESS_EQUAL    : model.add(lin_expr + b <= 0); break;
          case NConstraintInterface::NC_GREATER_EQUAL : model.add(lin_expr + b >= 0); break;
        }
      }
      else
      {
        BoundConstraint* bnd_ptr = dynamic_cast<BoundConstraint*>(_constraints[i]);
        if(bnd_ptr)
        {
          assert( int(bnd_ptr->idx()) < _problem->n_unknowns());

          switch(bnd_ptr->constraint_type())
          {
            case NConstraintInterface::NC_EQUAL         : vars[bnd_ptr->idx()].setBounds(bnd_ptr->bound(), bnd_ptr->bound()); break;
            case NConstraintInterface::NC_LESS_EQUAL    : vars[bnd_ptr->idx()].setUB(bnd_ptr->bound()); break;
            case NConstraintInterface::NC_GREATER_EQUAL : vars[bnd_ptr->idx()].setLB(bnd_ptr->bound()); break;
          }
        }
        else
        {
          ConeConstraint* cone_ptr = dynamic_cast<ConeConstraint*>(_constraints[i]);
          if(cone_ptr)
          {
            IloExpr soc_lhs(env_);
            IloExpr soc_rhs(env_);

            assert(cone_ptr->i() >= 0 && cone_ptr->i() <= _problem->n_unknowns());

            soc_rhs= 0.5*cone_ptr->c()*vars[cone_ptr->i()]*vars[cone_ptr->i()];

            NConstraintInterface::SMatrixNC::iterator q_it = cone_ptr->Q().begin();
            for(; q_it != cone_ptr->Q().end(); ++q_it)
            {
              assert( int(q_it.col()) < _problem->n_unknowns());
              assert( int(q_it.row()) < _problem->n_unknowns());

              soc_lhs += 0.5*(*q_it)*vars[q_it.col()]*vars[q_it.row()];
            }

            model.add(soc_lhs <= soc_rhs);
          }
          else
            std::cerr << "Warning: CPLEXSolver received a constraint of unknow type!!! -> skipping it" << std::endl;
        }
      }
    }
//    model.update();

    //----------------------------------------------
    // 3. setup energy
    //----------------------------------------------
    if(!_silent)
      std::cerr << "cplex -> setup energy...\n";

    if(!_problem->constant_hessian())
      std::cerr << "Warning: CPLEXSolver received a problem with non-constant hessian!!!" << std::endl;

//    GRBQuadExpr objective;
    IloExpr objective(env_);

    // add quadratic part
    NProblemInterface::SMatrixNP H;
    _problem->eval_hessian(P(x), H);
    for( int i=0; i<H.outerSize(); ++i)
      for (NProblemInterface::SMatrixNP::InnerIterator it(H,i); it; ++it)
      {
        assert(it.row() >= 0 && it.row() < _problem->n_unknowns());
        assert(it.col() >= 0 && it.col() < _problem->n_unknowns());

        objective += 0.5*it.value()*vars[it.row()]*vars[it.col()];
      }


    // add linear part
    std::vector<double> g(_problem->n_unknowns());
    _problem->eval_gradient(P(x), P(g));
    for(unsigned int i=0; i<g.size(); ++i)
    {
      objective += g[i]*vars[i];
    }

    // add constant part
    objective += _problem->eval_f(P(x));

    model.add(IloMinimize(env_,objective));

//    model.set(GRB_IntAttr_ModelSense, 1);
//    model.setObjective(objective);
//    model.update();


    //----------------------------------------------
    // 4. solve problem
    //----------------------------------------------
    if(!_silent)
      std::cerr << "cplex -> generate model...\n";
    IloCplex cplex(model);

    cplex.setParam(IloCplex::TiLim, _time_limit);
    { // hack
//    0 [CPX_NODESEL_DFS] Depth-first search
//    1 [CPX_NODESEL_BESTBOUND] Best-bound search
//    2 [CPX_NODESEL_BESTEST] Best-estimate search
//    3 [CPX_NODESEL_BESTEST_ALT] Alternative best-estimate search
//      cplex.setParam(IloCplex::NodeSel , 0);
    }
    if(!_silent)
      std::cerr << "cplex -> solve...\n";

    // silent mode?
    if(_silent)
      cplex.setOut(env_.getNullStream());

    IloBool solution_found = cplex.solve();


//    if (solution_input_path_.empty())
//    {
//      if (!problem_env_output_path_.empty())
//      {
//        std::cout << "Writing problem's environment into file \"" << problem_env_output_path_ << "\"." << std::endl;
//        model.getEnv().writeParams(problem_env_output_path_);
//      }
//      if (!problem_output_path_.empty())
//      {
//        std::cout << "Writing problem into file \"" << problem_output_path_ << "\"." << std::endl;
//        GurobiHelper::outputModelToMpsGz(model, problem_output_path_);
//      }
//
//      model.optimize();
//    }
//    else
//    {
//        std::cout << "Reading solution from file \"" << solution_input_path_ << "\"." << std::endl;
//    }
//
    //----------------------------------------------
    // 5. store result
    //----------------------------------------------

    if(solution_found != IloFalse)
    {
      if(!_silent)
        std::cerr << "cplex -> store result...\n";
      // store computed result
      for(unsigned int i=0; i<vars.size(); ++i)
        x[i] = cplex.getValue(vars[i]);

      _problem->store_result(P(x));
    }

/*
    if (solution_input_path_.empty())
    {
      // store computed result
      for(unsigned int i=0; i<vars.size(); ++i)
        x[i] = vars[i].get(GRB_DoubleAttr_X);
    }
*/
//    else
//    {
//        std::cout << "Loading stored solution from \"" << solution_input_path_ << "\"." << std::endl;
//        // store loaded result
//        const size_t oldSize = x.size();
//        x.clear();
//        GurobiHelper::readSolutionVectorFromSOL(x, solution_input_path_);
//        if (oldSize != x.size()) {
//            std::cerr << "oldSize != x.size() <=> " << oldSize << " != " << x.size() << std::endl;
//            throw std::runtime_error("Loaded solution vector doesn't have expected dimension.");
//        }
//    }
//
//    _problem->store_result(P(x));
//
//    // ObjVal is only available if the optimize was called.
//    if (solution_input_path_.empty())
//        std::cout << "GUROBI Objective: " << model.get(GRB_DoubleAttr_ObjVal) << std::endl;
    return solution_found;
  }
  catch (IloException& e)
  {
     cerr << "Concert exception caught: " << e << endl;
     return false;
  }
  catch (...)
  {
     cerr << "Unknown exception caught" << endl;
     return false;
  }

  return false;
}


//-----------------------------------------------------------------------------

bool
CPLEXSolver::
solve_two_phase(NProblemInterface*                  _problem,
      std::vector<NConstraintInterface*>& _constraints,
      std::vector<PairIndexVtype>&        _discrete_constraints,
      const double                        _time_limit0,
      const double                        _gap0,
      const double                        _time_limit1,
      const double                        _gap1,
      const bool                          _silent)
{
  try
  {
    //----------------------------------------------
    // 0. set up environment
    //----------------------------------------------
    if(!_silent)
      std::cerr << "cplex -> get environment...\n";
    IloEnv env_;

    if(!_silent)
      std::cerr << "cplex -> get model...\n";
    IloModel model(env_);
    //    model.getEnv().set(GRB_DoubleParam_TimeLimit, _time_limit);

    //----------------------------------------------
    // 1. allocate variables and initialize limits
    //----------------------------------------------
    if(!_silent)
      std::cerr << "cplex -> allocate variables...\n";
    // determine variable types: 0->real, 1->integer, 2->bool
    std::vector<char>   vtypes (_problem->n_unknowns(),0);

    for(unsigned int i=0; i<_discrete_constraints.size(); ++i)
      if(_discrete_constraints[i].first < vtypes.size())
      {
        switch(_discrete_constraints[i].second)
        {
          case Integer: vtypes [_discrete_constraints[i].first] = 1;
                        break;
          case Binary : vtypes[_discrete_constraints[i].first]  = 2;
                        break;
          default     : break;
        }
      }
      else
        std::cerr << "ERROR: requested a discrete variable which is above the total number of variables"
                  << _discrete_constraints[i].first << " vs " << vtypes.size() << std::endl;

    // CPLEX variables
    std::vector<IloNumVar> vars; vars.reserve(_problem->n_unknowns());
    // first all
    for( int i=0; i<_problem->n_unknowns(); ++i)
      switch(vtypes[i])
      {
        case 0 : vars.push_back( IloNumVar(env_,-IloInfinity, IloInfinity, IloNumVar::Float) ); break;
        case 1 : vars.push_back( IloNumVar(env_,  -IloIntMax,   IloIntMax, IloNumVar::Int)   ); break;
        case 2 : vars.push_back( IloNumVar(env_,           0,           1, IloNumVar::Bool)  ); break;
        default: std::cerr << "ERROR: unhandled varibalbe type in CPLEXSolver!!!" << std::endl;
      }

    if(!_silent)
    {
      std::cerr << "#unknowns       : " << _problem->n_unknowns() << std::endl;
      std::cerr << "#CPLEX variables: " << vars.size() << std::endl;
    }


    // Integrate new variables
//    model.update();

    //----------------------------------------------
    // 2. setup constraints
    //----------------------------------------------
    if(!_silent)
      std::cerr << "cplex -> setup constraints...\n";

    // get zero vector
    std::vector<double> x(_problem->n_unknowns(), 0.0);

    // handle constraints depending on their tyep
    for(unsigned int i=0; i<_constraints.size();  ++i)
    {
      // is linear constraint?
      LinearConstraint* lin_ptr = dynamic_cast<LinearConstraint*>(_constraints[i]);
      if(lin_ptr)
      {
        IloExpr lin_expr(env_);
        NConstraintInterface::SVectorNC gc;
        _constraints[i]->eval_gradient(P(x), gc);

        NConstraintInterface::SVectorNC::InnerIterator v_it(gc);
        for(; v_it; ++v_it)
        {
          assert(v_it.index() >= 0 && v_it.index() < _problem->n_unknowns());
          lin_expr += vars[v_it.index()]*v_it.value();
        }

        double b = _constraints[i]->eval_constraint(P(x));


        switch(_constraints[i]->constraint_type())
        {
          case NConstraintInterface::NC_EQUAL         : model.add(lin_expr + b == 0); break;
          case NConstraintInterface::NC_LESS_EQUAL    : model.add(lin_expr + b <= 0); break;
          case NConstraintInterface::NC_GREATER_EQUAL : model.add(lin_expr + b >= 0); break;
        }
      }
      else
      {
        BoundConstraint* bnd_ptr = dynamic_cast<BoundConstraint*>(_constraints[i]);
        if(bnd_ptr)
        {
          assert( int(bnd_ptr->idx()) < _problem->n_unknowns());

          switch(bnd_ptr->constraint_type())
          {
            case NConstraintInterface::NC_EQUAL         : vars[bnd_ptr->idx()].setBounds(bnd_ptr->bound(), bnd_ptr->bound()); break;
            case NConstraintInterface::NC_LESS_EQUAL    : vars[bnd_ptr->idx()].setUB(bnd_ptr->bound()); break;
            case NConstraintInterface::NC_GREATER_EQUAL : vars[bnd_ptr->idx()].setLB(bnd_ptr->bound()); break;
          }
        }
        else
        {
          ConeConstraint* cone_ptr = dynamic_cast<ConeConstraint*>(_constraints[i]);
          if(cone_ptr)
          {
            IloExpr soc_lhs(env_);
            IloExpr soc_rhs(env_);

            assert(cone_ptr->i() >= 0 && cone_ptr->i() <= _problem->n_unknowns());

            soc_rhs= 0.5*cone_ptr->c()*vars[cone_ptr->i()]*vars[cone_ptr->i()];

            NConstraintInterface::SMatrixNC::iterator q_it = cone_ptr->Q().begin();
            for(; q_it != cone_ptr->Q().end(); ++q_it)
            {
              assert( int(q_it.col()) < _problem->n_unknowns());
              assert( int(q_it.row()) < _problem->n_unknowns());

              soc_lhs += 0.5*(*q_it)*vars[q_it.col()]*vars[q_it.row()];
            }

            model.add(soc_lhs <= soc_rhs);
          }
          else
            std::cerr << "Warning: CPLEXSolver received a constraint of unknow type!!! -> skipping it" << std::endl;
        }
      }
    }
//    model.update();

    //----------------------------------------------
    // 3. setup energy
    //----------------------------------------------
    if(!_silent)
      std::cerr << "cplex -> setup energy...\n";

    if(!_problem->constant_hessian())
      std::cerr << "Warning: CPLEXSolver received a problem with non-constant hessian!!!" << std::endl;

//    GRBQuadExpr objective;
    IloExpr objective(env_);

    // add quadratic part
    NProblemInterface::SMatrixNP H;
    _problem->eval_hessian(P(x), H);
    for( int i=0; i<H.outerSize(); ++i)
      for (NProblemInterface::SMatrixNP::InnerIterator it(H,i); it; ++it)
      {
        assert(it.row() >= 0 && it.row() < _problem->n_unknowns());
        assert(it.col() >= 0 && it.col() < _problem->n_unknowns());

        objective += 0.5*it.value()*vars[it.row()]*vars[it.col()];
      }


    // add linear part
    std::vector<double> g(_problem->n_unknowns());
    _problem->eval_gradient(P(x), P(g));
    for(unsigned int i=0; i<g.size(); ++i)
    {
      objective += g[i]*vars[i];
    }

    // add constant part
    objective += _problem->eval_f(P(x));

    model.add(IloMinimize(env_,objective));

//    model.set(GRB_IntAttr_ModelSense, 1);
//    model.setObjective(objective);
//    model.update();


    //----------------------------------------------
    // 4. solve problem
    //----------------------------------------------
    if(!_silent)
      std::cerr << "cplex -> generate model...\n";
    IloCplex cplex(model);

    cplex.setParam(IloCplex::TiLim, _time_limit0);
    cplex.setParam(IloCplex::EpGap, _gap0);
    { // hack
//    0 [CPX_NODESEL_DFS] Depth-first search
//    1 [CPX_NODESEL_BESTBOUND] Best-bound search
//    2 [CPX_NODESEL_BESTEST] Best-estimate search
//    3 [CPX_NODESEL_BESTEST_ALT] Alternative best-estimate search
//      cplex.setParam(IloCplex::NodeSel , 0);
    }
    if(!_silent)
      std::cerr << "cplex -> solve...\n";

    // silent mode?
    if(_silent)
      cplex.setOut(env_.getNullStream());

    IloBool solution_found = cplex.solve();

    // phase 2?
    if(_time_limit1 > 0 && (!solution_found || cplex.getMIPRelativeGap() > _gap1))
    {
      cplex.setParam(IloCplex::TiLim, _time_limit1);
      cplex.setParam(IloCplex::EpGap, _gap1);
      solution_found = cplex.solve();
    }

    //----------------------------------------------
    // 5. store result
    //----------------------------------------------

    if(solution_found != IloFalse)
    {
      if(!_silent)
        std::cerr << "cplex -> store result...\n";
      // store computed result
      for(unsigned int i=0; i<vars.size(); ++i)
        x[i] = cplex.getValue(vars[i]);

      _problem->store_result(P(x));
    }

/*
    if (solution_input_path_.empty())
    {
      // store computed result
      for(unsigned int i=0; i<vars.size(); ++i)
        x[i] = vars[i].get(GRB_DoubleAttr_X);
    }
*/
//    else
//    {
//        std::cout << "Loading stored solution from \"" << solution_input_path_ << "\"." << std::endl;
//        // store loaded result
//        const size_t oldSize = x.size();
//        x.clear();
//        GurobiHelper::readSolutionVectorFromSOL(x, solution_input_path_);
//        if (oldSize != x.size()) {
//            std::cerr << "oldSize != x.size() <=> " << oldSize << " != " << x.size() << std::endl;
//            throw std::runtime_error("Loaded solution vector doesn't have expected dimension.");
//        }
//    }
//
//    _problem->store_result(P(x));
//
//    // ObjVal is only available if the optimize was called.
//    if (solution_input_path_.empty())
//        std::cout << "GUROBI Objective: " << model.get(GRB_DoubleAttr_ObjVal) << std::endl;
    return solution_found;
  }
  catch (IloException& e)
  {
     cerr << "Concert exception caught: " << e << endl;
     return false;
  }
  catch (...)
  {
     cerr << "Unknown exception caught" << endl;
     return false;
  }

  return false;
}


//-----------------------------------------------------------------------------

void
CPLEXSolver::
test()
{
  try
  {
    std::cerr << "cplex2 -> get environment...\n";
    IloEnv env_;

    std::cerr << "cplex2 -> get model...\n";
    IloModel model(env_);

//    // CPLEX variables
//    std::vector<IloNumVar> vars; vars.reserve(_problem->n_unknowns());
//    // first all
//    for( int i=0; i<_problem->n_unknowns(); ++i)
//      vars.push_back( IloNumVar(env_,-IloInfinity, IloInfinity, IloNumVar::Float) );


    IloNumVar x1(env_,-IloInfinity, IloInfinity, IloNumVar::Float);
    IloNumVar x2(env_,-IloInfinity, IloInfinity, IloNumVar::Float);
    IloNumVar x3(env_,-IloInfinity, IloInfinity, IloNumVar::Float);
    IloNumVar x4(env_,-IloInfinity, IloInfinity, IloNumVar::Float);
    IloNumVar x5(env_,-IloInfinity, IloInfinity, IloNumVar::Float);
    IloNumVar x6(env_,-IloInfinity, IloInfinity, IloNumVar::Float);

    IloExpr objective(env_,0);

//    // add constant part
//    objective = x1 + x2 + x3 + x4 + x5 + x6;

    model.add(IloMinimize(env_,objective));

    std::cerr << "cplex2 -> generate model...\n";
    IloCplex cplex(model);
    std::cerr << "cplex2 -> generate model done...\n";
//    cplex.setParam(IloCplex::TiLim, _time_limit);
//    std::cerr << "cplex2 -> solve...\n";
//    cplex.setOut(env_.getNullStream());

    IloBool solution_found = cplex.solve();
  }
  catch (IloException& e)
  {
     cerr << "Concert exception caught: " << e << endl;
  }
  catch (...)
  {
     cerr << "Unknown exception caught" << endl;
  }
}



//-----------------------------------------------------------------------------


bool
CPLEXSolver::
solve(NProblemInterface*                        _problem,
      const std::vector<NConstraintInterface*>& _constraints,
      const std::vector<NConstraintInterface*>& _lazy_constraints,
      std::vector<PairIndexVtype>&              _discrete_constraints,   // discrete constraints
      const double                              _almost_infeasible,
      const int                                 _max_passes,
      const double                              _time_limit,
      const bool                                _silent)
{
//  std::cerr << "Warning: CPLEXSolver does not support lazy constraints yet -> solve with all constraints instead" << std::endl;
//  std::vector<NConstraintInterface*> C;
//  std::copy(_constraints.begin(),_constraints.end(),std::back_inserter(C));
//  return solve(_problem, C, _time_limit, _silent);

  StopWatch sw; sw.start();

  bool feasible_point_found = false;
  int  cur_pass = 0;
  double acceptable_tolerance = 0.01; // hack: read out from ipopt!!!
  std::vector<bool> lazy_added(_lazy_constraints.size(),false);

  // cache statistics of all iterations
  std::vector<int> n_inf;
  std::vector<int> n_almost_inf;

  try
  {
    //----------------------------------------------
    // 0. set up environment
    //----------------------------------------------
    if(!_silent)
      std::cerr << "cplex -> get environment...\n";
    IloEnv env_;

    if(!_silent)
      std::cerr << "cplex -> get model...\n";
    IloModel model(env_);
    //    model.getEnv().set(GRB_DoubleParam_TimeLimit, _time_limit);

    //----------------------------------------------
    // 1. allocate variables and initialize limits
    //----------------------------------------------
    if(!_silent)
      std::cerr << "cplex -> allocate variables...\n";
    // determine variable types: 0->real, 1->integer, 2->bool
    std::vector<char>   vtypes (_problem->n_unknowns(),0);

    for(unsigned int i=0; i<_discrete_constraints.size(); ++i)
      if(_discrete_constraints[i].first < vtypes.size())
      {
        switch(_discrete_constraints[i].second)
        {
          case Integer: vtypes [_discrete_constraints[i].first] = 1;
                        break;
          case Binary : vtypes[_discrete_constraints[i].first]  = 2;
                        break;
          default     : break;
        }
      }
      else
        std::cerr << "ERROR: requested a discrete variable which is above the total number of variables"
                  << _discrete_constraints[i].first << " vs " << vtypes.size() << std::endl;

    // CPLEX variables
    std::vector<IloNumVar> vars; vars.reserve(_problem->n_unknowns());
    // first all
    for( int i=0; i<_problem->n_unknowns(); ++i)
      switch(vtypes[i])
      {
        case 0 : vars.push_back( IloNumVar(env_,-IloInfinity, IloInfinity, IloNumVar::Float) ); break;
        case 1 : vars.push_back( IloNumVar(env_,  -IloIntMax,   IloIntMax, IloNumVar::Int)   ); break;
        case 2 : vars.push_back( IloNumVar(env_,           0,           1, IloNumVar::Bool)  ); break;
        default: std::cerr << "ERROR: unhandled varibalbe type in CPLEXSolver!!!" << std::endl;
      }

    if(!_silent)
    {
      std::cerr << "#unknowns       : " << _problem->n_unknowns() << std::endl;
      std::cerr << "#CPLEX variables: " << vars.size() << std::endl;
    }


    // Integrate new variables
//    model.update();

    //----------------------------------------------
    // 2. setup constraints
    //----------------------------------------------
    if(!_silent)
      std::cerr << "cplex -> setup constraints...\n";

    // get zero vector
    std::vector<double> x(_problem->n_unknowns(), 0.0);

    // handle constraints depending on their type
    for(unsigned int i=0; i<_constraints.size();  ++i)
      add_constraint_to_model( _constraints[i], vars, model, env_, P(x));
//    model.update();

    //----------------------------------------------
    // 3. setup energy
    //----------------------------------------------
    if(!_silent)
      std::cerr << "cplex -> setup energy...\n";

    if(!_problem->constant_hessian())
      std::cerr << "Warning: CPLEXSolver received a problem with non-constant hessian!!!" << std::endl;

//    GRBQuadExpr objective;
    IloExpr objective(env_);

    // add quadratic part
    NProblemInterface::SMatrixNP H;
    _problem->eval_hessian(P(x), H);
    for( int i=0; i<H.outerSize(); ++i)
      for (NProblemInterface::SMatrixNP::InnerIterator it(H,i); it; ++it)
      {
        assert(it.row() >= 0 && it.row() < _problem->n_unknowns());
        assert(it.col() >= 0 && it.col() < _problem->n_unknowns());

        objective += 0.5*it.value()*vars[it.row()]*vars[it.col()];
      }


    // add linear part
    std::vector<double> g(_problem->n_unknowns());
    _problem->eval_gradient(P(x), P(g));
    for(unsigned int i=0; i<g.size(); ++i)
    {
      objective += g[i]*vars[i];
    }

    // add constant part
    objective += _problem->eval_f(P(x));

    model.add(IloMinimize(env_,objective));

//    model.set(GRB_IntAttr_ModelSense, 1);
//    model.setObjective(objective);
//    model.update();


    //----------------------------------------------
    // 4. iteratively solve problem
    //----------------------------------------------
    IloBool solution_found = IloFalse;

    while(!feasible_point_found && cur_pass <(_max_passes-1))
    {
      ++cur_pass;
      //----------------------------------------------------------------------------
      // 1. Create an instance of current Model and optimize
      //----------------------------------------------------------------------------

      if(!_silent)
        std::cerr << "cplex -> setup IloCPlex...\n";
      IloCplex cplex(model);
      cplex.setParam(IloCplex::TiLim, _time_limit);
      // silent mode?
      if(_silent)
        cplex.setOut(env_.getNullStream());

      if(!_silent)
        std::cerr << "cplex -> optimize...\n";
      solution_found = cplex.solve();

      if(solution_found != IloFalse)
      {
        for(unsigned int i=0; i<vars.size(); ++i)
          x[i] = cplex.getValue(vars[i]);

        _problem->store_result(P(x));
      }
      else continue;

      //----------------------------------------------------------------------------
      // 2. Check lazy constraints
      //----------------------------------------------------------------------------
      // check lazy constraints
      n_inf.push_back(0);
      n_almost_inf.push_back(0);
      feasible_point_found = true;
      for(unsigned int i=0; i<_lazy_constraints.size(); ++i)
        if(!lazy_added[i])
        {
          NConstraintInterface* lc = _lazy_constraints[i];

          double v = lc->eval_constraint(P(x));

          bool inf        = false;
          bool almost_inf = false;

          if(lc->constraint_type() == NConstraintInterface::NC_EQUAL)
          {
            v = std::abs(v);
            if(v>acceptable_tolerance)
              inf = true;
            else
              if(v>_almost_infeasible)
                almost_inf = true;
          }
          else
            if(lc->constraint_type() == NConstraintInterface::NC_GREATER_EQUAL)
            {
              if(v<-acceptable_tolerance)
                inf = true;
              else
                if(v<_almost_infeasible)
                  almost_inf = true;
            }
            else
              if(lc->constraint_type() == NConstraintInterface::NC_LESS_EQUAL)
              {
                if(v>acceptable_tolerance)
                  inf = true;
                else
                  if(v>-_almost_infeasible)
                    almost_inf = true;
              }

          // infeasible?
          if(inf)
          {
            add_constraint_to_model( lc, vars, model, env_, P(x));
            lazy_added[i] = true;
            feasible_point_found = false;
            ++n_inf.back();
          }
          else // almost violated or violated? -> add to constraints
            if(almost_inf)
            {
              add_constraint_to_model( lc, vars, model, env_, P(x));
              lazy_added[i] = true;
              ++n_almost_inf.back();
            }
        }
    }

    // no termination after max number of passes?
    if(!feasible_point_found)
    {
      ++cur_pass;

      std::cerr << "*************** could not find feasible point after " << _max_passes-1 << " -> solving with all lazy constraints..." << std::endl;
      for(unsigned int i=0; i<_lazy_constraints.size(); ++i)
        if(!lazy_added[i])
        {
          add_constraint_to_model( _lazy_constraints[i], vars, model, env_, P(x));
        }

      //----------------------------------------------------------------------------
      // 1. Create an instance of current Model and optimize
      //----------------------------------------------------------------------------

      IloCplex cplex(model);
      cplex.setParam(IloCplex::TiLim, _time_limit);
      // silent mode?
      if(_silent)
        cplex.setOut(env_.getNullStream());
      solution_found = cplex.solve();

      if(solution_found != IloFalse)
      {
        for(unsigned int i=0; i<vars.size(); ++i)
          x[i] = cplex.getValue(vars[i]);

        _problem->store_result(P(x));
      }
    }

    const double overall_time = sw.stop()/1000.0;

    //----------------------------------------------------------------------------
    // 4. output statistics
    //----------------------------------------------------------------------------
//    if (solution_found != IloFalse)
//    {
//      // Retrieve some statistics about the solve
//      Ipopt::Index iter_count = app_->Statistics()->IterationCount();
//      printf("\n\n*** IPOPT: The problem solved in %d iterations!\n", iter_count);
//
//      Ipopt::Number final_obj = app_->Statistics()->FinalObjective();
//      printf("\n\n*** IPOPT: The final value of the objective function is %e.\n", final_obj);
//    }

    std::cerr <<"############# CPLEX with lazy constraints statistics ###############" << std::endl;
    std::cerr << "overall time: " << overall_time << "s" << std::endl;
    std::cerr << "#passes     : " << cur_pass << "( of " << _max_passes << ")" << std::endl;
    for(unsigned int i=0; i<n_inf.size(); ++i)
      std::cerr << "pass " << i << " induced " << n_inf[i] << " infeasible and " << n_almost_inf[i] << " almost infeasible" << std::endl;

    return (solution_found != IloFalse);
  }
  catch (IloException& e)
  {
     cerr << "Concert exception caught: " << e << endl;
     return false;
  }
  catch (...)
  {
     cerr << "Unknown exception caught" << endl;
     return false;
  }

  return false;
}



//-----------------------------------------------------------------------------


void
CPLEXSolver::
add_constraint_to_model( NConstraintInterface* _constraint, std::vector<IloNumVar>& _vars, IloModel& _model, IloEnv& _env, double* _x)
{

  // is linear constraint?
  LinearConstraint* lin_ptr = dynamic_cast<LinearConstraint*>(_constraint);
  if(lin_ptr)
  {
    IloExpr lin_expr(_env);
    NConstraintInterface::SVectorNC gc;
    _constraint->eval_gradient(_x, gc);

    NConstraintInterface::SVectorNC::InnerIterator v_it(gc);
    for(; v_it; ++v_it)
    {
      assert(v_it.index() >= 0 && v_it.index() < (int)_vars.size());
      lin_expr += _vars[v_it.index()]*v_it.value();
    }

    double b = _constraint->eval_constraint(_x);


    switch(_constraint->constraint_type())
    {
    case NConstraintInterface::NC_EQUAL         : _model.add(lin_expr + b == 0); break;
    case NConstraintInterface::NC_LESS_EQUAL    : _model.add(lin_expr + b <= 0); break;
    case NConstraintInterface::NC_GREATER_EQUAL : _model.add(lin_expr + b >= 0); break;
    }
  }
  else
  {
    BoundConstraint* bnd_ptr = dynamic_cast<BoundConstraint*>(_constraint);
    if(bnd_ptr)
    {
      assert( int(bnd_ptr->idx()) < (int)_vars.size());

      switch(bnd_ptr->constraint_type())
      {
      case NConstraintInterface::NC_EQUAL         : _vars[bnd_ptr->idx()].setBounds(bnd_ptr->bound(), bnd_ptr->bound()); break;
      case NConstraintInterface::NC_LESS_EQUAL    : _vars[bnd_ptr->idx()].setUB(bnd_ptr->bound()); break;
      case NConstraintInterface::NC_GREATER_EQUAL : _vars[bnd_ptr->idx()].setLB(bnd_ptr->bound()); break;
      }
    }
    else
    {
      ConeConstraint* cone_ptr = dynamic_cast<ConeConstraint*>(_constraint);
      if(cone_ptr)
      {
        IloExpr soc_lhs(_env);
        IloExpr soc_rhs(_env);

        assert(cone_ptr->i() >= 0 && cone_ptr->i() <= (int)_vars.size());

        soc_rhs= 0.5*cone_ptr->c()*_vars[cone_ptr->i()]*_vars[cone_ptr->i()];

        NConstraintInterface::SMatrixNC::iterator q_it = cone_ptr->Q().begin();
        for(; q_it != cone_ptr->Q().end(); ++q_it)
        {
          assert( int(q_it.col()) < (int)_vars.size());
          assert( int(q_it.row()) < (int)_vars.size());

          soc_lhs += 0.5*(*q_it)*_vars[q_it.col()]*_vars[q_it.row()];
        }

        _model.add(soc_lhs <= soc_rhs);
      }
      else
        std::cerr << "Warning: CPLEXSolver received a constraint of unknow type!!! -> skipping it" << std::endl;
    }
  }
}

//-----------------------------------------------------------------------------


//void
//CPLEXSolver::
//set_problem_output_path( const std::string &_problem_output_path)
//{
//  problem_output_path_ = _problem_output_path;
//}
//
//
////-----------------------------------------------------------------------------
//
//
//void
//CPLEXSolver::
//set_problem_env_output_path( const std::string &_problem_env_output_path)
//{
//  problem_env_output_path_ = _problem_env_output_path;
//}
//
//
////-----------------------------------------------------------------------------
//
//
//void
//CPLEXSolver::
//set_solution_input_path(const std::string &_solution_input_path)
//{
//  solution_input_path_ = _solution_input_path;
//}


//=============================================================================
} // namespace COMISO
//=============================================================================
#endif // COMISO_CPLEX_AVAILABLE
//=============================================================================
