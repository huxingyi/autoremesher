// Copyright (c) 2012  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/STL_Extension/include/CGAL/CC_safe_handle.h $
// $Id: CC_safe_handle.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Clement Jamin

#ifndef CGAL_CC_SAFE_HANDLE_H
#define CGAL_CC_SAFE_HANDLE_H

namespace CGAL {

// CC_safe_handle is a helper that store a CC handle and its erase
// counter value (value when the CC_safe_handle instance was created).
// The is_zombie() function allows to know if the pointee was erased since.
template <typename CC_iterator>
class CC_safe_handle
{
public:
  CC_safe_handle(CC_iterator iterator)
    : m_handle(iterator)
    , m_erase_counter_value(iterator->erase_counter())
  {
  }

  bool is_zombie() const
  {
    return m_handle->erase_counter() != m_erase_counter_value;
  }

  CC_iterator cc_iterator() const
  {
    return m_handle;
  }

protected:
  CC_iterator     m_handle;
  unsigned int    m_erase_counter_value;
};

template <typename CC_iterator>
CC_safe_handle<CC_iterator> make_cc_safe_handle(CC_iterator iterator)
{
  return CC_safe_handle<CC_iterator>(iterator);
}

} //namespace CGAL

#endif // CGAL_CC_SAFE_HANDLE_H
