/****************************THIS FILE IS DEPRECATED************************/

#ifndef DRAW_CYCLES
#define DRAW_CYCLES
#include <igl/igl_inline.h>
#include <igl/barycenter.h>
#include <igl/per_face_normals.h>
#include <directional/representative_to_raw.h>
#include <directional/point_spheres.h>
#include <directional/line_cylinders.h>
#include <Eigen/Core>
#include <igl/avg_edge_length.h>


namespace directional
{
	// Turns all faces that are part of a particular cycle into a given color.
	// Inputs:
    //  EF: #E by 2 edges to faces indices
	//  basisCycles: #Cycles by #E the oriented (according to EF) cycles 
	//  cycleColor: The color to draw the cycle
	//  index: The index of the cycle you wish to draw
	//  color: The already filled color matrix that should be modified to add the cycles
	void IGL_INLINE draw_cycles(const Eigen::MatrixXi& EF, 
		const Eigen::SparseMatrix<double, Eigen::RowMajor>& cycleMat,
		const Eigen::VectorXd& cycleColor,
		int index,
		Eigen::MatrixXd& color)
	{
		for (Eigen::SparseMatrix<double, Eigen::RowMajor>::InnerIterator it(cycleMat, index); it; ++it)
		{
			color.row(EF(it.col(), 0)) = cycleColor;
			color.row(EF(it.col(), 1)) = cycleColor;
		}
	}

}

#endif
