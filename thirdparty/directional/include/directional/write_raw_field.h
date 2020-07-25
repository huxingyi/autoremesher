// This file is part of Directional, a library for directional field processing.
// Copyright (C) 2018 Amir Vaxman <avaxman@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
#ifndef DIRECTIONAL_WRITE_RAW_FIELD_H
#define DIRECTIONAL_WRITE_RAW_FIELD_H

#include <Eigen/Core>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>

namespace directional
{

  // Writes a a directional field in raw format to file
  // Inputs:
  //   filename: The name used for the mesh and singularity file, without extention
  //   rawField: #F by 3*N in xyzxyz format (N is derived from F.cols())
  // Returns:
  //   Whether or not the file was written successfully
  bool IGL_INLINE write_raw_field(const std::string fileName, Eigen::MatrixXd& rawField)
  {
      std::ofstream f(fileName);
      int N = rawField.cols() / 3;
      assert(3 * N == rawField.cols());
      f << N << " " << rawField.rows() << std::endl;
      for (int i=0;i<rawField.rows();i++)
      {
        for (int j=0;j<rawField.cols();j++)
          f << rawField(i,j) << " ";
        f << std::endl;
      }
      f.close();
      return !f.fail();
  }
}

#endif
