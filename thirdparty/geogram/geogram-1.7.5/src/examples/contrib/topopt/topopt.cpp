////////////////////////////////////////////////////////////////////////////////
#include "topopt.h"
#include "layout.h"
#include "element.h"
// -----------------------------------------------------------------------------
#include <cstdint>
#include <iostream>
#include <limits>
#include <unordered_set>
#include <type_traits>
#include <cassert>
#include <array>
////////////////////////////////////////////////////////////////////////////////

TopoptSolver::TopoptSolver(TopoptProblem &top)
	: m_Top(top)
	, m_X(top.nelems.prod())
	, m_D(top.densities)
	, m_U(top.nvars())
	, m_K(top.nvars(), top.nvars())
	, m_H(top.nelems.prod(), top.nelems.prod())
	, m_S(top.nelems.prod())
	, m_V(top.nelems.prod())
	, m_Rho(top.nelems.prod())
	, m_GradObj(top.nelems.prod())
	, m_CachedFilterRadius(-1)
{
	// Initialize quantities
	m_X = top.densities;
	m_U.setZero();

	// Allocate stiffness matrix and analyze sparsity pattern
	prepareStiffnessMatrix();
	m_Solver.analyzePattern(m_K);

	// Prepare filter coeffs
	prepareFilter();

	// Filter physical densities (if applicable)
	filterDensities();
}

////////////////////////////////////////////////////////////////////////////////

// Reset design
void TopoptSolver::resetDesign() {
	m_X.setConstant(m_Top.volfrac);
	filterDensities();
}

void TopoptSolver::rescaleDesign(double xmin, double xmax) {
	double ymin = m_X.minCoeff();
	double ymax = m_X.maxCoeff();
	m_X = xmin + (xmax - xmin) / std::max(ymax - ymin, 0.01) * (m_X.array() - ymin);
	filterDensities();
}

////////////////////////////////////////////////////////////////////////////////

// Prepare filtering coefficient matrix
void TopoptSolver::prepareFilter() {
	using namespace Eigen;
	const double rmin = m_Top.radius;
	std::vector<Eigen::Triplet<double> > nnz;
	for (int elemIndex = 0; elemIndex < numElements(); ++elemIndex) {
		const Vector2i p = Layout::to_grid(elemIndex, m_Top.nelems);
		const Vector2i lower(
			std::max((int) std::floor(p[0] - rmin), 0),
			std::max((int) std::floor(p[1] - rmin), 0)
		);
		const Vector2i upper(
			std::min((int) std::ceil(p[0] + rmin), m_Top.nelems[0] - 1),
			std::min((int) std::ceil(p[1] + rmin), m_Top.nelems[1] - 1)
		);
		for (int x = lower[0]; x <= upper[0]; ++x) {
		for (int y = lower[1]; y <= upper[1]; ++y) {
			const Vector2i q(x, y);
			assert(Layout::is_valid(x, y, m_Top.nelems));
			const int j = Layout::to_index(q, m_Top.nelems);
			const double val = std::max(0.0, rmin - (p - q).norm());
			if (val > 0.0) {
				nnz.emplace_back(elemIndex, j, val);
			}
		}}
	}

	m_H.setFromTriplets(nnz.begin(), nnz.end());
	m_S = m_H * Eigen::VectorXd::Ones(numElements());

	std::cout << "-- Nonzeros (filter): \t" << m_H.nonZeros() << " / " << nnz.size() << std::endl;
	m_CachedFilterRadius = rmin;
}

////////////////////////////////////////////////////////////////////////////////

// Compute and allocate non-zero pattern of the stiffness matrix
void TopoptSolver::prepareStiffnessMatrix() {
	auto iterateElements = [&] (const std::function<void(int, int)> &action) {
		const auto K_e = SquareElement::K();
		for (int elemIndex = 0; elemIndex < numElements(); ++elemIndex) {
			const Eigen::Vector2i lc = Layout::to_grid(elemIndex, m_Top.nelems);
			for (int c1 = 0; c1 < 4; ++c1) {
				const int n1 = SquareElement::nodeIndex(c1, lc, m_Top.nnodes());
				for (int d1 = 0; d1 < 2; ++d1) {
					const int i1 = 2 * n1 + d1;
					for (int c2 = 0; c2 < 4; ++c2) {
						const int n2 = SquareElement::nodeIndex(c2, lc, m_Top.nnodes());
						for (int d2 = 0; d2 < 2; ++d2) {
							const int i2 = 2 * n2 + d2;
							if (K_e(2*c1+d1, 2*c2+d2) != 0) {
								action(i1, i2);
							}
						}
					}
				}
			}
		}
		for (int i = 0; i < numVars(); ++i) {
			action(i, i);
		}
	};

	// Step1: count number of nonzeros
	size_t numNonZeros = 0;
	iterateElements([&numNonZeros] (int i1, int i2) { ++numNonZeros; });

	// Step2: allocate triplets
	std::vector<Eigen::Triplet<char> > nnz;
	nnz.reserve(numNonZeros);
	iterateElements([&nnz] (int i1, int i2) { nnz.emplace_back(i1, i2, 1); });

	// Step3: build sparse matrix
	m_K.setFromTriplets(nnz.begin(), nnz.end());

	std::cout << "-- Nonzeros (stiffness matrix): " << m_K.nonZeros() << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

namespace {
	int localIndexNodes(Eigen::Vector2i x1, Eigen::Vector2i x2) {
		int res = 0;
		for (int d = 1; d >= 0; --d) {
			assert(std::abs(x1[d] - x2[d]) <= 1);
			res = x2[d] - x1[d] + 1 + 3 * res;
		}
		return res;
	}
}

// -----------------------------------------------------------------------------

// Assemble general stiffness matrix
void TopoptSolver::assembleStiffnessMatrix() {
	m_Rho = m_Top.E_min + m_D.array().pow(m_Top.penalty) * (1.0 - m_Top.E_min);

	// Update the values of the global stiffness matrix
	const auto K_e = SquareElement::K();
	#pragma omp parallel for
	for (int n1 = 0; n1 < numNodes(); ++n1) {
		const Eigen::Vector2i n1Pos = Layout::to_grid(n1, m_Top.nnodes());
		for (int d1 = 0; d1 < 2; ++d1) {
			const int i1 = 2 * n1 + d1;

			if (m_Top.fixed(i1) == 1) {
				for (Eigen::SparseMatrix<double>::InnerIterator it(m_K, i1); it; ++it) {
					assert(it.col() == i1);
					const int i2 = it.row();
					it.valueRef() = (i1 == i2 ? 1.0 : 0.0);
				}
				continue;
			}

			std::array<double, 9*2> values = {};
			for (int c1 = 0; c1 < 4; ++c1) {
				const Eigen::Vector2i elemPos = n1Pos - SquareElement::delta(c1);
				const int elemIndex = Layout::to_index(elemPos, m_Top.nelems);
				if (Layout::is_valid(elemPos[0], elemPos[1], m_Top.nelems)) {
					for (int c2 = 0; c2 < 4; ++c2) {
						const Eigen::Vector2i n2Pos = elemPos + SquareElement::delta(c2);
						const int n2 = Layout::to_index(n2Pos, m_Top.nnodes());
						for (int d2 = 0; d2 < 2; ++d2) {
							const int i2 = 2 * n2 + d2;
							if (m_Top.fixed(i2) == 1) { continue; }
							const int localIndex = localIndexNodes(n1Pos, n2Pos);
							const double val = m_Rho(elemIndex) * K_e(2*c1+d1, 2*c2+d2);
							if (val != 0) {
								values[2*localIndex+d2] += val;
							}
						}
					}
				}
			}

			for (Eigen::SparseMatrix<double>::InnerIterator it(m_K, i1); it; ++it) {
				assert(it.col() == i1);
				const int i2 = it.row();
				const int n2 = (i2 / 2);
				const int d2 = (i2 % 2);
				const Eigen::Vector2i n2Pos = Layout::to_grid(n2, m_Top.nnodes());
				const int localIndex = localIndexNodes(n1Pos, n2Pos);
				it.valueRef() = values[2*localIndex+d2];
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

// Apply filter on densities
void TopoptSolver::filterDensities() {
	if (m_Top.filter == FilterType::Densities) {
		m_D.noalias() = (m_H * m_X).cwiseQuotient(m_S);
	} else {
		m_D = m_X;
	}
}

////////////////////////////////////////////////////////////////////////////////

// Apply filter on compliance gradient
void TopoptSolver::filterGradient() {
	switch (m_Top.filter) {
	case FilterType::NoFilter:
		break;
	case FilterType::Sensitivies:
		m_GradObj = (m_H * m_D.cwiseProduct(m_GradObj)).cwiseQuotient(
			m_S).cwiseQuotient(m_D.cwiseMax(1e-3));
		break;
	case FilterType::Densities:
		m_GradObj = m_H * (m_GradObj.cwiseQuotient(m_S));
		break;
	default:
		throw std::runtime_error("Topopt::filterGradient");
	}
}

////////////////////////////////////////////////////////////////////////////////

// Compute compliance for all set of external loads
double TopoptSolver::computeCompliance() {
	return m_Top.forces.dot(m_U);
}

////////////////////////////////////////////////////////////////////////////////

// Add the contribution of the given displacement vector to the objective gradient
void TopoptSolver::computeGradient() {
	#pragma omp parallel for
	for (int i = 0; i < numElements(); ++i) {
		const auto K_e = SquareElement::K();
		const Eigen::Vector2i lc = Layout::to_grid(i, m_Top.nelems);
		const double deltaRho = m_Top.penalty * (1.0 - m_Top.E_min) * std::pow(m_D(i), m_Top.penalty - 1.0);

		// Retrieve nodal displacements
		const auto u_e = SquareElement::getDisplacements(m_U, lc, m_Top.nnodes());

		// Compute gradient
		m_GradObj(i) = - deltaRho * u_e.dot(K_e * u_e);
	}
}

////////////////////////////////////////////////////////////////////////////////

// Update densities based on objective gradient, using the optimality criteria
void TopoptSolver::updateWithOptimalityCriterion() {
	// See http://www.herrera.unt.edu.ar/iest/stropt/stopttopopt@unt.pdf
	const double moveLimit = 0.2;
	const double damping = 0.5;
	const double q = 2.0; //2.0;

	double l1 = 0;
	double l2 = 1e40;

	m_V.setOnes();
	if (m_Top.filter == FilterType::Densities) {
		m_V = m_H * (m_V.cwiseQuotient(m_S)); // Volume derivative
	}

	Eigen::VectorXd newX(m_X.size());
	while ( l2 > 1e-40 && (l2-l1)/(l2+l1) > 1e-6 ) {
	//while ( l2 - l1 > 1e-6 ) {
		const double lmid = 0.5 * (l1 + l2);

		#pragma omp parallel for
		for (int i = 0; i < numElements(); ++i) {
			const auto Be = std::max(-m_GradObj(i) / m_V(i) / lmid, 1e-10);
			newX(i) = std::max(0.0,
					std::max(m_X(i) - moveLimit,
						std::min(1.0,
							std::min(m_X(i) + moveLimit,
								std::pow(m_X(i) * std::pow(Be, damping), q)
							)
						)
					)
				);
		}
		double newVolume = 0;
		switch (m_Top.filter) {
		case FilterType::NoFilter:;
		case FilterType::Sensitivies:
			newVolume = newX.sum();
			break;
		case FilterType::Densities:
			{
				auto xPhys = m_H * (newX.cwiseQuotient(m_S));
				newVolume = xPhys.sum();
				break;
			}
		default:
			throw std::runtime_error("Topopt::updateWithOptimalityCriterion");
		}

		if (newVolume > (double) numElements() * m_Top.volfrac) {
			l1 = lmid;
		} else {
			l2 = lmid;
		}
	}

	m_X.swap(newX);
}

////////////////////////////////////////////////////////////////////////////////

// FEM analysis step (no density update)
double TopoptSolver::analyze() {
	if (m_Top.radius != m_CachedFilterRadius) {
		prepareFilter();
	}

	assembleStiffnessMatrix();
	m_Solver.factorize(m_K);
	m_U = m_Solver.solve(m_Top.forces);

	double c = computeCompliance();
	std::cerr << "-- Compliance: \t" << c << std::endl;
	return c;
}

////////////////////////////////////////////////////////////////////////////////

// Perform a design update
void TopoptSolver::iter() {
	analyze();

	computeGradient();
	filterGradient();

	updateWithOptimalityCriterion();
	filterDensities();
}
