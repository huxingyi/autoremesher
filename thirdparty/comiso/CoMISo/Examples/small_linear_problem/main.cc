/*===========================================================================*\
 *                                                                           *
 *                               CoMISo                                      *
 *      Copyright (C) 2008-2009 by Computer Graphics Group, RWTH Aachen      *
 *                           www.rwth-graphics.de                            *
 *                                                                           *
 *---------------------------------------------------------------------------* 
 *  This file is part of CoMISo.                                             *
 *                                                                           *
 *  CoMISo is free software: you can redistribute it and/or modify           *
 *  it under the terms of the GNU General Public License as published by     *
 *  the Free Software Foundation, either version 3 of the License, or        *
 *  (at your option) any later version.                                      *
 *                                                                           *
 *  CoMISo is distributed in the hope that it will be useful,                *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU General Public License for more details.                             *
 *                                                                           *
 *  You should have received a copy of the GNU General Public License        *
 *  along with CoMISo.  If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                           *
\*===========================================================================*/ 

#include <CoMISo/Config/config.hh>

#include <CoMISo/Utils/StopWatch.hh>
#include <vector>
#include <CoMISo/NSolver/LinearProblem.hh>
#include <CoMISo/NSolver/LinearConstraint.hh>
#include <CoMISo/NSolver/VariableType.hh>
#include <CoMISo/NSolver/NPDerivativeChecker.hh>
#include <CoMISo/NSolver/CPLEXSolver.hh>
#include <CoMISo/NSolver/GUROBISolver.hh>
#include <CoMISo/NSolver/CBCSolver.hh>


// minimize linear problem E = 8*x + 2*y + 3*z subject to x+y+z >= 2 and z-y >= 1 and x, y, z binary

// Example main
int main(void)
{
  std::cout << "---------- 1) Get an instance of a LinearProblem..." << std::endl;
  // number of unknowns
  const int n = 3;
  COMISO::LinearProblem lp(n);

  //setup linear energy E = 8*x + 2*y + 3*z
  lp.coeffs()[0] = 8.0;
  lp.coeffs()[1] = 2.0;
  lp.coeffs()[2] = 3.0;

  // first constraint to x+y+z >= 2
  COMISO::LinearConstraint::SVectorNC v(n);
  v.coeffRef(0) = 1.0;
  v.coeffRef(1) = 1.0;
  v.coeffRef(2) = 1.0;
  COMISO::LinearConstraint c0(v,-2.0,COMISO::NConstraintInterface::NC_GREATER_EQUAL);


  // second constraint z-y >= 1
  v.setZero();
  v.coeffRef(1) = -1.0;
  v.coeffRef(2) =  1.0;
  COMISO::LinearConstraint c1(v,-1.0,COMISO::NConstraintInterface::NC_GREATER_EQUAL);

  // add constraints to vector
  std::vector<COMISO::NConstraintInterface*> constraints;
  constraints.push_back(&c0);
  constraints.push_back(&c1);

  // setup binary constraints
  std::vector<COMISO::PairIndexVtype> dc;
  dc.push_back(COMISO::PairIndexVtype(0,COMISO::Binary));
  dc.push_back(COMISO::PairIndexVtype(1,COMISO::Binary));
  dc.push_back(COMISO::PairIndexVtype(2,COMISO::Binary));
  
  std::cout << "---------- 2) (optional for debugging) Check derivatives of problem..." << std::endl;
  COMISO::NPDerivativeChecker npd;
  npd.check_all(&lp);

// check if CPLEX solver available in current configuration
#if( COMISO_CPLEX_AVAILABLE)
  std::cout << "---------- 3) Get CPLEX and optimize... " << std::endl;
  COMISO::CPLEXSolver csol;
  csol.solve(&lp, constraints, dc);
#endif

  std::cout << "---------- 4) Print solution..." << std::endl;
  for( int i=0; i<n; ++i)
    std::cerr << "x_" << i << " = " << lp.x()[i] << std::endl;

  // check if GUROBI solver available in current configuration
#if( COMISO_GUROBI_AVAILABLE)
    std::cout << "---------- 5) Get GUROBI and optimize... " << std::endl;
    COMISO::GUROBISolver gsol;
    gsol.solve(&lp, constraints, dc);
#endif

  std::cout << "---------- 6) Print solution..." << std::endl;
  for( int i=0; i<n; ++i)
    std::cerr << "x_" << i << " = " << lp.x()[i] << std::endl;


    // check if CBC solver available in current configuration
#if( COMISO_CBC_AVAILABLE)
    std::cout << "---------- 6) Get CBC and optimize... " << std::endl;
    COMISO::CBCSolver cbc_sol;
    cbc_sol.solve(&lp, constraints, dc);
#endif

  std::cout << "---------- 8) Print solution..." << std::endl;
  for( int i=0; i<n; ++i)
    std::cerr << "x_" << i << " = " << lp.x()[i] << std::endl;

  
  return 0;
}

