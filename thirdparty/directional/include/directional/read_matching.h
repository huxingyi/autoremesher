// This file is part of Directional, a library for directional field processing.
// Copyright (C) 2018 Amir Vaxman <avaxman@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
#ifndef DIRECTIONAL_READ_MATCHING_H
#define DIRECTIONAL_READ_MATCHING_H
#include <cmath>
#include <Eigen/Core>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>


namespace directional
{

	// Reads a list of singularities from a file
	// Inputs:
	//   fileName: The to be loaded file.
	// Outputs:
	//   singularities: The vector containing the singularities
	//   N:             The degree of the field
  //   singVertices:  The singular vertices.
  //   singIndices:   Rhe index of the singularities, where the actual fractional index is singIndices/N.
	// Return:
	//   Whether or not the file was written successfully
  bool IGL_INLINE read_matching(const std::string &fileName,
                                Eigen::VectorXi& matching,
                                Eigen::MatrixXi& EV,
                                Eigen::MatrixXi& FE,
                                Eigen::MatrixXi& EF)
	{
		try
		{
			std::ifstream f(fileName);
			int numEdges, numFaces;
			f >> numFaces >> numEdges;
      
      matching.conservativeResize(numEdges);
      EV.conservativeResize(numEdges,2);
      FE.conservativeResize(numFaces,3);
      EF.conservativeResize(numEdges,2);

      for (int i=0;i<numEdges;i++)
        f >> EV(i,0)>> EV(i,1);
      
      for (int i=0;i<numFaces;i++)
        f >> FE(i,0)>> FE(i,1)>> FE(i,2);
      
      for (int i=0;i<numEdges;i++)
        f >> EF(i,0)>> EF(i,1);
      
      for (int i=0;i<numEdges;i++)
        f >> matching(i);
      
			f.close();
			return f.fail();
		}
		catch (std::exception e)
		{
			return false;
		}
	}
}

#endif
