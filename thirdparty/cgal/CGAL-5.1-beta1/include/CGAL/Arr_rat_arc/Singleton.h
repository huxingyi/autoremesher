// Copyright (c) 2011 Tel-Aviv University (Israel), INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Arrangement_on_surface_2/include/CGAL/Arr_rat_arc/Singleton.h $
// $Id: Singleton.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Oren Salzman <orenzalz@post.tau.ac.il >
//                 Michael Hemmer <Michael.Hemmer@sophia.inria.fr>



#ifndef CGAL_SINGLETON_H_
#define SINGLETON_H_

#include <CGAL/license/Arrangement_on_surface_2.h>


#include <CGAL/assertions.h>

namespace CGAL {
namespace Arr_rational_arc {
template <class T>
class Singleton
{
public:
  static T* instance()
  {
    if(!m_pInstance)
      m_pInstance = new T;
    CGAL_assertion(m_pInstance !=nullptr);
    return m_pInstance;
  }

  static void DestroyInstance()
  {
    delete m_pInstance;
    m_pInstance = nullptr;
  };
private:
  Singleton();          // ctor hidden
  ~Singleton();          // dtor hidden
private:
  static T* m_pInstance;
};

template <class T> T* Singleton<T>::m_pInstance=nullptr;

}   // namespace Arr_rational_arc
}   //namespace CGAL {
#endif // CGAL_SINGLETON_H_
