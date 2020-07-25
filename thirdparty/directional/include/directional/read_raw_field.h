// This file is part of Directional, a library for directional field processing.
// Copyright (C) 2018 Amir Vaxman <avaxman@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
#ifndef DIRECTIONAL_READ_RAW_FIELD_H
#define DIRECTIONAL_READ_RAW_FIELD_H
#include <cmath>
#include <Eigen/Core>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>


namespace directional
{
  
  // Reads a raw field from a file
  // Inputs:
  //   fileName: The to be loaded file.
  // Outputs:
  //   N: The degree of the field
  //   rawField: the read field in raw #F by 3*N xyzxyz format
  // Return:
  //   Whether or not the file was read successfully
  bool IGL_INLINE read_raw_field(const std::string &fileName,
                                 int& N,
                                 Eigen::MatrixXd& rawField)
  {
    try
    {
      std::ifstream f(fileName);
      int numF;
      f>>N;
      f>>numF;
      rawField.conservativeResize(numF, 3*N);
      
      //Can we do better than element-wise reading?
      for (int i=0;i<rawField.rows();i++)
        for (int j=0;j<rawField.cols();j++)
          f>>rawField(i,j);
      
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
