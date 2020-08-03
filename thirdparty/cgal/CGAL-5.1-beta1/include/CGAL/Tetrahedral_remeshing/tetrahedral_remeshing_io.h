// Copyright (c) 2020 GeometryFactory (France) and Telecom Paris (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Tetrahedral_remeshing/include/CGAL/Tetrahedral_remeshing/tetrahedral_remeshing_io.h $
// $Id: tetrahedral_remeshing_io.h 3f275b8 2020-04-24T16:03:28+02:00 Jane Tournois
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Jane Tournois, Noura Faraj

#include <CGAL/IO/io.h>

#include <iostream>
#include <fstream>

namespace CGAL
{
template<typename T3>
bool load_triangulation(std::istream& is, T3& t3)
{
  std::string s;
  if (!(is >> s)) return false;
  bool binary = (s == "binary");
  if (binary) {
    if (!(is >> s)) return false;
  }
  if (s != "CGAL" || !(is >> s) || s != "c3t3")
    return false;

  std::getline(is, s);
  if (binary) CGAL::set_binary_mode(is);
  else        CGAL::set_ascii_mode(is);
  is >> t3;
  return bool(is);
}

template<typename T3>
bool save_binary_triangulation(std::ostream& os, const T3& t3)
{
  os << "binary CGAL c3t3\n";
  CGAL::set_binary_mode(os);
  return !!(os << t3);
}

template<typename T3>
bool save_ascii_triangulation(std::ostream& os, const T3& t3)
{
  os << "CGAL c3t3\n";
  CGAL::set_ascii_mode(os);
  return !!(os << t3);
}

}