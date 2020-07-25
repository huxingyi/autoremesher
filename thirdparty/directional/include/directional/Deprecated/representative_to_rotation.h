#ifndef REPRESENTATIVE_TO_ROTATION_H
#define REPRESENTATIVE_TO_ROTATION_H
#include <directional/rotation_to_representative.h>
#include <directional/representative_to_raw.h>
#include <igl/local_basis.h>
#include <igl/parallel_transport_angles.h>
#include <igl/edge_topology.h>
#include <Eigen/Core>

namespace directional
{
	// Computes the rotation angle field given the representative field.
	// Inputs:
	//  V: #V by 3 vertex coordinates.
	//  F: #F by 3 face vertex indices.
	//  EV: #E by 2 edges 2 vertices indices.
	//  FE: #F by 3 indices of the edges surrounding a face.
	//  EF: #E by 2 edges 2 faces indices.
	//  representative: #F by 3 coordinates of representative vectors.
	//  N: the degree of the field.
	// Outputs:
	//  adjustAngles: #E angles that encode deviation from parallel transport.
	//  globalRotation: The angle between the vector on the first face and its basis in radians.
	IGL_INLINE void representative_to_rotation(const Eigen::MatrixXd& V,
		const Eigen::MatrixXi& F,
		const Eigen::MatrixXi& EV,
		const Eigen::MatrixXi& FE,
		const Eigen::MatrixXi& EF,
		const Eigen::MatrixXd& representative,
		int N,
		Eigen::VectorXd& adjustAngles,
		double& globalRotation)
	{
		Eigen::MatrixXd B1, B2, B3;
		Eigen::VectorXd local, parallel;
		igl::local_basis(V, F, B1, B2, B3);
		adjustAngles.resize(EV.rows(), 1);

		local.resize(F.rows());

		//Obtain angles for parallel transport
		igl::parallel_transport_angles(V, F, B3, EF, FE, parallel);

		//Calculate translations in local base
		for (int i = 0; i < F.rows(); i++)
		{
			double dot = representative.row(i).dot(B1.row(i));
			Eigen::Matrix3d mat;
			mat << representative.row(i),
				B1.row(i),
				B3.row(i);
			double det = mat.determinant();
			local[i] = atan2(det, dot);
		}

		globalRotation = local(0);

		for (int i = 0; i < EF.rows(); i++)
		{
			if (EF(i, 0) == -1 || EF(i, 1) == -1)
				continue;

			adjustAngles(i) = local(EF(i,0)) - local(EF(i,1)) -parallel(i);
			int matching =(int)(adjustAngles(i) / (2*igl::PI / N));
			adjustAngles(i) -= (2 * igl::PI / N) * matching;

			if (adjustAngles(i) > igl::PI/N)
				adjustAngles(i) -= 2 * igl::PI/N;
		}
	}

	// Computes the rotation angle field given the representative field.
	// Inputs:
	//  V: #V by 3 vertex coordinates.
	//  F: #F by 3 face vertex indices.
	//  representative: #F by 3 coordinates of representative vectors.
	//  N: the degree of the field.
	// Outputs:
	//  adjustAngles: #E angles that encode deviation from parallel transport.
	//  globalRotation: The angle between the vector on the first face and its basis in radians.
	IGL_INLINE void representative_to_rotation(const Eigen::MatrixXd& V,
		const Eigen::MatrixXi& F,
		const Eigen::MatrixXd& representative,
		int N,
		Eigen::VectorXd& adjustAngles,
		double& globalRotation)
	{
		Eigen::MatrixXi EV, FE, EF;
		igl::edge_topology(V, F, EV, FE, EF);
		representative_to_rotation(V, F, EV, FE, EF, representative, N, adjustAngles, globalRotation);
	}
}

#endif
