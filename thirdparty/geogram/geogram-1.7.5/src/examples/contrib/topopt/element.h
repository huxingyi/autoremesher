#pragma once

#include "layout.h"
#include <Eigen/Dense>

namespace SquareElement {
	using Eigen::Vector2i;

	// Stiffness matrix
	inline Eigen::Matrix<double, 8, 8> K();

	// Mapping local -> global node index
	inline Vector2i delta(int i);
	inline int nodeIndex(int corner, Vector2i lowerCorner, Vector2i nodeGridSize);

	// Per-element nodal displacement vector
	template<typename EigenMatrix>
	inline Eigen::Matrix<double, 8, 1> getDisplacements(
		const EigenMatrix &u, Vector2i lowerCorner, Vector2i nodeGridSize);
}

// -----------------------------------------------------------------------------

// Precomputed stiffness matrix (for a Poisson's ratio of 1/3)
inline Eigen::Matrix<double, 8, 8> SquareElement::K() {
	return (Eigen::Matrix<double, 8, 8>() <<
		1./2., 3./16., -5./16., 0., -1./4., -3./16., 1./16., 0.,
		3./16., 1./2., 0., 1./16., -3./16., -1./4., 0., -5./16.,
		-5./16., 0., 1./2., -3./16., 1./16., 0., -1./4., 3./16.,
		0., 1./16., -3./16., 1./2., 0., -5./16., 3./16., -1./4.,
		-1./4., -3./16., 1./16., 0., 1./2., 3./16., -5./16., 0.,
		-3./16., -1./4., 0., -5./16., 3./16., 1./2., 0., 1./16.,
		1./16., 0., -1./4., 3./16., -5./16., 0., 1./2., -3./16.,
		0., -5./16., 3./16., -1./4., 0., 1./16., -3./16., 1./2.
	).finished();
}

// Map local corner index (between 0 and 3) to the actual corner of the unit
// square [0,1]^2. The corners are obtained in this order:
// (0, 0), (1, 0), (1, 1), (0, 1)
inline Eigen::Vector2i SquareElement::delta(int i) {
	return Eigen::Vector2i( (i & 1) ^ ((i >> 1) & 1), (i >> 1) & 1 );
}

// Mapping local -> global node index
inline int SquareElement::nodeIndex(int corner, Eigen::Vector2i lowerCorner, Eigen::Vector2i nodeGridSize) {
	const Eigen::Vector2i currentCorner = lowerCorner + delta(corner);
	return Layout::to_index(currentCorner, nodeGridSize);
}

// Per-element nodal displacement vector
template<typename EigenMatrix>
inline Eigen::Matrix<double, 8, 1>  SquareElement::getDisplacements(
	const EigenMatrix &u, Eigen::Vector2i lowerCorner, Eigen::Vector2i nodeGridSize)
{
	Eigen::Matrix<double, 8, 1> u_e;
	for (int localIndex = 0; localIndex < 4; ++localIndex) {
		const int globalIndex = nodeIndex(localIndex, lowerCorner, nodeGridSize);
		for (int i = 0; i < 2; ++i) {
			u_e(2*localIndex+i) = u(2*globalIndex+i);
		}
	}
	return u_e;
}
