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
#include <CoMISo/NSolver/FiniteElementProblem.hh>
#include <CoMISo/NSolver/NPDerivativeChecker.hh>
#include <CoMISo/NSolver/IPOPTSolver.hh>


// create a simple finite element (x-y)^2
class SimpleElement
{
public:

  // define dimensions
  const static int NV = 2;
  const static int NC = 0;

  typedef Eigen::Matrix<size_t,NV,1> VecI;
  typedef Eigen::Matrix<double,NV,1> VecV;
  typedef Eigen::Matrix<double,NC,1> VecC;
  typedef Eigen::Triplet<double> Triplet;

  inline double eval_f       (const VecV& _x, const VecC& _c) const
  {
    return std::pow(_x[0]-_x[1],2);
  }

  inline void   eval_gradient(const VecV& _x, const VecC& _c, VecV& _g) const
  {
    _g[0] = 2.0*(_x[0]-_x[1]);
    _g[1] = -_g[0];
  }

  inline void   eval_hessian (const VecV& _x, const VecC& _c, std::vector<Triplet>& _triplets) const
  {
    _triplets.clear();
    _triplets.push_back(Triplet(0,0,2));
    _triplets.push_back(Triplet(1,1,2));
    _triplets.push_back(Triplet(0,1,-2));
    _triplets.push_back(Triplet(1,0,-2));
  }
};

// create a simple finite element (x-c)^2
class SimpleElement2
{
public:

  // define dimensions
  const static int NV = 1;
  const static int NC = 1;

  typedef Eigen::Matrix<size_t,NV,1> VecI;
  typedef Eigen::Matrix<double,NV,1> VecV;
  typedef Eigen::Matrix<double,NC,1> VecC;
  typedef Eigen::Triplet<double> Triplet;

  inline double eval_f(const VecV& _x, const VecC& _c) const
  {
    return std::pow(_x[0]-_c[0],2);
  }

  inline void   eval_gradient(const VecV& _x, const VecC& _c, VecV& _g) const
  {
    _g[0] = 2.0*(_x[0]-_c[0]);
  }

  inline void   eval_hessian (const VecV& _x, const VecC& _c, std::vector<Triplet>& _triplets) const
  {
    _triplets.clear();
    _triplets.push_back(Triplet(0,0,2));
  }
};


//------------------------------------------------------------------------------------------------------

// Example main
int main(void)
{
  std::cout << "---------- 1) Get an instance of a FiniteElementProblem..." << std::endl;

  // first create sets of different finite elements
  COMISO::FiniteElementSet<SimpleElement> fe_set;

  fe_set.instances().add_element(SimpleElement::VecI(0,1), SimpleElement::VecC());
  fe_set.instances().add_element(SimpleElement::VecI(1,2), SimpleElement::VecC());

  // second set for boundary conditions
  COMISO::FiniteElementSet<SimpleElement2> fe_set2;
  SimpleElement2::VecI vi;
  SimpleElement2::VecV v;
  vi[0] = 0;
  v [0] = 0;
  fe_set2.instances().add_element(vi, v);
  vi[0] = 2;
  v [0] = 2;
  fe_set2.instances().add_element(vi, v);

  // then create finite element problem and add sets
  COMISO::FiniteElementProblem fe_problem(3);
  fe_problem.add_set(&fe_set );
  fe_problem.add_set(&fe_set2);

  std::cout << "---------- 2) (optional for debugging) Check derivatives of problem..." << std::endl;
  COMISO::NPDerivativeChecker npd;
  npd.check_all(&fe_problem);

  // check if IPOPT solver available in current configuration
  #if( COMISO_IPOPT_AVAILABLE)
    std::cout << "---------- 3) Get IPOPT solver... " << std::endl;
    COMISO::IPOPTSolver ipsol;

    std::cout << "---------- 4) Solve..." << std::endl;
    // there are no constraints -> provide an empty vector
    std::vector<COMISO::NConstraintInterface*> constraints;
    ipsol.solve(&fe_problem, constraints);
  #endif

  // print result
  for(unsigned int i=0; i<fe_problem.x().size(); ++i)
    std::cerr << "x[" << i << "] = " << fe_problem.x()[i] << std::endl;

  return 0;
}

