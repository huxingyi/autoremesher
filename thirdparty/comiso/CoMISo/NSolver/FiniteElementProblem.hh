//=============================================================================
//
//  CLASS FiniteElementProblem
//
//=============================================================================


#ifndef COMISO_FINITEELEMENTPROBLEM_HH
#define COMISO_FINITEELEMENTPROBLEM_HH


//== INCLUDES =================================================================

#include <CoMISo/Config/CoMISoDefines.hh>
#include "NProblemInterface.hh"


//== FORWARDDECLARATIONS ======================================================

//== NAMESPACES ===============================================================

namespace COMISO { 

//== CLASS DEFINITION =========================================================

	      
/** \class FiniteElementProblem

    Brief Description.
  
    A more elaborate description follows.
*/


class FiniteElementSetBase
{
public:

  typedef Eigen::Triplet<double> Triplet;

  virtual ~FiniteElementSetBase() {}

  virtual double eval_f( const double* _x ) = 0;

  virtual void accumulate_gradient( const double* _x , double* _g) = 0;

  virtual void accumulate_hessian ( const double* _x , std::vector<Triplet>& _triplets) = 0;
};


// Concepts

//class FiniteElementType
//{
//  // define dimensions of variables and constants
//  const static int NV = 3;
//  const static int NC = 1;
//
//  typedef Eigen::Matrix<size_t,NV,1> VecI;
//  typedef Eigen::Matrix<double,NV,1> VecV;
//  typedef Eigen::Matrix<double,NC,1> VecC;
//  typedef Eigen::Triplet<double> Triplet;
//
//  inline double eval_f       (const VecV& _x, const VecC& _c) const;
//  inline void   eval_gradient(const VecV& _x, const VecC& _c, VecV& _g) const;
//  inline void   eval_hessian (const VecV& _x, const VecC& _c, std::vector<Triplet>& _triplets) const;
//};


//class FiniteElementInstancesType
//{
//
//  typedef Eigen::Matrix<double,NC,1> VecC;
//
//  size_t size() const;
//
//  size_t index( const size_t _instance, const size_t _nr) const;
//
//  const VecC& c( const size_t _instance) const;
//
//};

template<class FiniteElementType>
class FiniteElementInstancesVector
{
public:
  const static int NV = FiniteElementType::NV;
  const static int NC = FiniteElementType::NC;

  typedef typename FiniteElementType::VecI VecI;
  typedef typename FiniteElementType::VecC VecC;

  void add_element(const VecI& _indices, const VecC& _constants)
  {
    indices_.push_back(_indices);
    constants_.push_back( _constants);
  }

  void clear_elements()
  {
    indices_.clear();
    constants_.clear();
  }

  size_t size() const
  {
    return indices_.size();
  }

  size_t index( const size_t _instance, const size_t _nr) const
  {
    return indices_[_instance][_nr];
  }

  const VecC& c( const size_t _instance) const
  {
    return constants_[_instance];
  }

private:
  std::vector<VecI> indices_;
  std::vector<VecC> constants_;
};


template<class FiniteElementType, class FiniteElementInstancesType = FiniteElementInstancesVector<FiniteElementType> >
class FiniteElementSet : public FiniteElementSetBase
{
public:

  // export dimensions of element
  const static int NV = FiniteElementType::NV;
  const static int NC = FiniteElementType::NC;

  typedef typename FiniteElementType::VecI VecI;
  typedef typename FiniteElementType::VecV VecV;
  typedef typename FiniteElementType::VecC VecC;

  // access element for setting constants per element etc.
  FiniteElementType& element() { return element_;}

  // access instances for adding etc.
  FiniteElementInstancesType& instances() { return instances_;}

  virtual double eval_f( const double* _x )
  {
    double f(0.0);
    for(unsigned int i=0; i<instances_.size(); ++i)
    {
      // get local x vector
      for(unsigned int j=0; j<NV; ++j)
        x_[j] = _x[instances_.index(i,j)];

      f += element_.eval_f(x_, instances_.c(i));
    }
    return f;
  }

  virtual void accumulate_gradient( const double* _x , double* _g)
  {
    for(unsigned int i=0; i<instances_.size(); ++i)
    {
      // get local x vector
      for(unsigned int j=0; j<NV; ++j)
        x_[j] = _x[instances_.index(i,j)];

      element_.eval_gradient(x_, instances_.c(i), g_);

      // accumulate into global gradient
      for(unsigned int j=0; j<NV; ++j)
        _g[instances_.index(i,j)] += g_[j];
    }
  }

  virtual void accumulate_hessian ( const double* _x , std::vector<Triplet>& _triplets)
  {
    for(unsigned int i=0; i<instances_.size(); ++i)
    {
      // get local x vector
      for(unsigned int j=0; j<NV; ++j)
        x_[j] = _x[instances_.index(i,j)];

      element_.eval_hessian(x_, instances_.c(i), triplets_);

      for(unsigned int j=0; j<triplets_.size(); ++j)
      {
        // add re-indexed Triplet
        _triplets.push_back(Triplet( instances_.index(i,triplets_[j].row()),
                                     instances_.index(i,triplets_[j].col()),
                                     triplets_[j].value()                   ));
      }
    }
  }

private:

  FiniteElementType element_;

  FiniteElementInstancesType instances_;

  VecV x_;
  VecV g_;

  std::vector<Triplet> triplets_;
};


class COMISODLLEXPORT FiniteElementProblem : public NProblemInterface
{
public:

//  typedef Eigen::SparseMatrix<double,Eigen::ColMajor> SMatrixNP;

  typedef FiniteElementSetBase::Triplet Triplet;

  /// Default constructor
  FiniteElementProblem(const unsigned int _n);

  /// Destructor
  virtual ~FiniteElementProblem();

  void add_set(FiniteElementSetBase* _fe_set);

  void clear_sets();

  std::vector<double>& x();

  // problem definition
  virtual int    n_unknowns   (                                );

  virtual void   initial_x    (       double* _x               );

  virtual double eval_f       ( const double* _x );

  virtual void   eval_gradient( const double* _x, double*    _g);

  virtual void   eval_hessian ( const double* _x, SMatrixNP& _H);


  virtual void   store_result ( const double* _x );

  // advanced properties (ToDo better handling)
  virtual bool   constant_gradient() const;
  virtual bool   constant_hessian()  const;

private:

  // number of unknowns
  unsigned int n_;

  // current/initial solution
  std::vector<double> x_;

  // finite element sets
  std::vector<FiniteElementSetBase*> fe_sets_;

  // vector of triplets (avoid re-allocation)
  std::vector<Triplet> triplets_;
};


//=============================================================================
} // namespace COMISO
//=============================================================================
#endif // COMISO_FINITEELEMENTPROBLEM_HH defined
//=============================================================================

