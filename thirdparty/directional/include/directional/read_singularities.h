// This file is part of Directional, a library for directional field processing.
// Copyright (C) 2018 Amir Vaxman <avaxman@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
#ifndef DIRECTIONAL_READ_SINGULARITIES_H
#define DIRECTIONAL_READ_SINGULARITIES_H
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
  bool IGL_INLINE read_singularities(const std::string &fileName,
                                     int& N,
                                     Eigen::VectorXi &singVertices,
                                     Eigen::VectorXi& singIndices)
	{
		try
		{
			std::ifstream f(fileName);
			int numSings;
			f >> N;
			f >> numSings;

			singVertices = Eigen::VectorXi::Zero(numSings);
      singIndices = Eigen::VectorXi::Zero(numSings);
      
      for (int i=0;i<numSings;i++)
        f >> singVertices.coeffRef(i)>> singIndices.coeffRef(i);
      
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
