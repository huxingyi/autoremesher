/******************THIS FILE IS DEPRECATED************************/


#ifndef CYCLE_CURVATURE_H
#define CYCLE_CURVATURE_H
#include <igl/igl_inline.h>
#include <igl/gaussian_curvature.h>
#include <igl/local_basis.h>
#include <igl/parallel_transport_angles.h>
#include <igl/edge_topology.h>
#include <igl/is_border_vertex.h>
#include <igl/gaussian_curvature.h>
#include <Eigen/Core>
#include <vector>
#include <cmath>

namespace directional
{
	// Computes the angle defect for each basis cycle in the basis cycle matrix. Contractable cycles around border vertices are set to 0.
	// Inputs:
	//  V: #V X 3 vertex coordinates
	//  F: #F by 3 face vertex indices
	//  EF: #E X 2 edges 2 faces indices
	//  EV: #E by 2 indices of the vertices surrounding each edge
	//  B1, B2: #F by 3 matrices representing the local base of each face.
	//  basisCycles: #basisCycles X #E the oriented (according to EF) basis cycles around which the curvatures are measured
	//               the basis cycles must be arranged so that the first |V| are the vertex cycles (for instance, the result of directional::basis_cycles())
	//				 followed by the border cycles.
	// Outputs:
	//  cycleCurvature: the angle defect for each basis cycle.
    // TODO: this still computes with holonomy and should be replace with angle defect instead!
	IGL_INLINE void cycle_curvature(const Eigen::MatrixXd& V,
                                    const Eigen::MatrixXi& F,
                                    const Eigen::MatrixXi& EV,
                                    const Eigen::MatrixXi& EF,
                                    const Eigen::MatrixXd& B1,
                                    const Eigen::MatrixXd& B2,
                                    const Eigen::SparseMatrix<double, Eigen::RowMajor>& basisCycles,
                                    Eigen::VectorXd& cycleCurvature)
	{
		using namespace Eigen;
		using namespace std;

		vector<bool> border = igl::is_border_vertex(V, F);
		// Start of to be replaced section
		VectorXd edgeParallelAngleChange(basisCycles.cols());  //the difference in the angle representation of edge i from EF(i,0) to EF(i,1)
																		//MatrixXd edgeVectors(columns(columns.size() - 1), 3);

		for (int i = 0, j = 0; i < EF.rows(); i++) {
			//skip border edges
			if (EF(i, 0) == -1 || EF(i, 1) == -1)
				continue;

			RowVectorXd edgeVectors = (V.row(EV(i, 1)) - V.row(EV(i, 0))).normalized();
			double x1 = edgeVectors.dot(B1.row(EF(i, 0)));
			double y1 = edgeVectors.dot(B2.row(EF(i, 0)));
			double x2 = edgeVectors.dot(B1.row(EF(i, 1)));
			double y2 = edgeVectors.dot(B2.row(EF(i, 1)));
			edgeParallelAngleChange(i) = atan2(y2, x2) - atan2(y1, x1);
		}


		cycleCurvature = basisCycles*edgeParallelAngleChange;
		for (int i = 0; i < cycleCurvature.size(); i++) {
			while (cycleCurvature(i) >= M_PI) cycleCurvature(i) -= 2.0*M_PI;
			while (cycleCurvature(i) < -M_PI) cycleCurvature(i) += 2.0*M_PI;
		}

		// End of to be replaced section

		//VectorXd cycleCurvature(reducedCycles.rows());
		VectorXd gc;
		igl::gaussian_curvature(V, F, gc);

		//Copy over for contractible cycles
		for (int i = 0; i < gc.size(); i++)
			if (!border[i])
				cycleCurvature(i) = gc(i);

		//Sum curvature for boundaries
		int i = 0, added = 0, total = count(border.begin(), border.end(), true);
		while (added < total)
		{
			double temp = cycleCurvature(V.rows() + i);
			cycleCurvature(V.rows() + i) = 0;
			//Loop over boundary cycles
			for (SparseMatrix<double, Eigen::RowMajor>::InnerIterator it(basisCycles, V.rows() + i); it; ++it)
				if (it.value())
				{
					int b = (it.value() + 1) / 2;
					if (border[EV(it.col(), b)])
					{
						// Substract Pi, as igl::gaussian_curvature calculates gc as 2 * PI - sum(angles)
						cycleCurvature(V.rows() + i) += gc(EV(it.col(), b)) - igl::PI;
						//Ensure vertices won't be counted twice
						border[EV(it.col(), b)] = false;
						added++;
					}
				}
			i++;
		}
	}

	// Version without explicit bases (just (V,F))
	IGL_INLINE void cycle_curvature(const Eigen::MatrixXd& V,
                                    const Eigen::MatrixXi& F,
                                    const Eigen::SparseMatrix<double, Eigen::RowMajor>& basisCycles,
                                    Eigen::VectorXd& cycleCurvature)
	{
		Eigen::MatrixXi EV, x, EF;
		igl::edge_topology(V, F, EV, x, EF);
		Eigen::MatrixXd B1, B2, B3;
		igl::local_basis(V, F, B1, B2, B3);
		cycle_curvature(V, F, EV, EF, B1, B2, basisCycles, cycleCurvature);
	}
}




#endif
