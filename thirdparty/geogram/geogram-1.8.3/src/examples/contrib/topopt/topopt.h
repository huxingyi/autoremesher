#pragma once

#include <Eigen/Dense>
#include <Eigen/Sparse>
#ifdef USE_CHOLMOD
#include <Eigen/CholmodSupport>
#endif

////////////////////////////////////////////////////////////////////////////////

enum class FilterType : int { NoFilter, Densities, Sensitivies };

// -----------------------------------------------------------------------------

struct TopoptProblem {
	Eigen::Vector2i nelems;
	Eigen::VectorXd densities;
	Eigen::VectorXd forces;
	Eigen::VectorXd fixed;
	float volfrac;
	float radius;
	double penalty;
	double E_min;
	FilterType filter;

	TopoptProblem()
		: nelems(64, 32)
		, volfrac(0.4)
		, radius(1.5)
		, penalty(3)
		, E_min(1e-9)
		, filter(FilterType::Densities)
	{
		densities.setConstant(volfrac);
		forces.setZero();
		fixed.setZero();
	}

	int nvars() const { return 2*(nelems[0]+1)*(nelems[1]+1); }
	Eigen::Vector2i nnodes() const { return nelems + Eigen::Vector2i(1, 1); }
};

////////////////////////////////////////////////////////////////////////////////

struct TopoptSolver {
	// Typedef
#ifdef USE_CHOLMOD
	typedef Eigen::CholmodSupernodalLLT<Eigen::SparseMatrix<double> > EigenDirectSolver;
#else
	typedef Eigen::SimplicialLDLT<Eigen::SparseMatrix<double> >       EigenDirectSolver;
#endif

	// Description of the FE problem
	TopoptProblem &m_Top;

	Eigen::VectorXd             m_X; // Design variables (densities)
	Eigen::VectorXd&            m_D; // Physical densities (after filtering)
	Eigen::VectorXd             m_U; // Nodal displacements / values
	Eigen::SparseMatrix<double> m_K; // Assembled stiffness matrix
	Eigen::SparseMatrix<double> m_H; // Filtering coefficient matrix
	Eigen::VectorXd             m_S; // Filtering denoms
	Eigen::VectorXd             m_V; // Derivative of the volume
	Eigen::VectorXd             m_Rho; // Penalized densities
	Eigen::VectorXd             m_GradObj;

	// Need to refresh the filter matrices?
	double m_CachedFilterRadius;

	// Solve linear equation
	EigenDirectSolver m_Solver;

	// Constructor
	TopoptSolver(TopoptProblem &top);

	// Number of elements, nodes, and degrees of freedom
	int numElements() const { return (int) m_Rho.size(); }
	int numNodes()    const { return (int) m_Top.nnodes().prod(); }
	int numVars()     const { return (int) m_K.rows(); }

	// Reset design
	void resetDesign();
	void rescaleDesign(double xmin, double xmax);

	// Prepare filtering coefficient matrix
	void prepareFilter();

	// Compute and allocate non-zero pattern of the stiffness matrix
	void prepareStiffnessMatrix();

	// Assemble general stiffness matrix
	void assembleStiffnessMatrix();

	// Apply filter
	void filterDensities();
	void filterGradient();

	// Compute compliance for all set of external loads
	double computeCompliance();

	// Add the contribution of the given displacement vector to the objective gradient
	void computeGradient();

	// Update densities based on objective gradient, using the optimality criteria
	void updateWithOptimalityCriterion();

	// FEM analysis step (no density update). Return the compliance
	double analyze();

	// Perform a design update
	void iter();
};
