// Copyright (c) 2000  Max-Planck-Institute Saarbruecken (Germany).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Partition_2/include/CGAL/Partition_2/Matrix.h $
// $Id: Matrix.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Susan Hert <hert@mpi-sb.mpg.de>

#ifndef   CGAL_PARTITION_MATRIX_H
#define   CGAL_PARTITION_MATRIX_H

#include <CGAL/license/Partition_2.h>


#include <vector>
#include <iostream>
#include <cstddef>

namespace CGAL {

template <class T>
class  Matrix : public std::vector< std::vector<T> >
{
public:
   Matrix(std::size_t x = 0, std::size_t y = 0) :
      std::vector< std::vector<T> > (x, std::vector<T>(y)),
      _rows(x), _columns(y)
   {}

   std::size_t rows() const { return _rows; }
   std::size_t columns() const { return _columns; }


protected:
   std::size_t _rows;
   std::size_t _columns;
};

template <class T>
std::ostream& operator<<(std::ostream& os, const Matrix<T>& m)
{
   typedef typename Matrix<T>::size_type size_type;

   for (size_type i = 0; i < m.rows(); i++)
   {
      os << std::endl << i << " : ";
      for (size_type j = 0; j < m.columns(); j++)
      {
         os << m[i][j] << " ";
      }
   }
   return os;
}

}

#endif // CGAL_PARTITION_MATRIX_H
