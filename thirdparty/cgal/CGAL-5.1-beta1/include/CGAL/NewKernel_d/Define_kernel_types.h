// Copyright (c) 2014
// INRIA Saclay-Ile de France (France)
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/NewKernel_d/include/CGAL/NewKernel_d/Define_kernel_types.h $
// $Id: Define_kernel_types.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Marc Glisse

#ifndef CGAL_DEFINE_KERNEL_TYPES_H
#define CGAL_DEFINE_KERNEL_TYPES_H
#include <CGAL/config.h>
#include <CGAL/NewKernel_d/functor_tags.h>
#include <CGAL/typeset.h>

namespace CGAL {
  namespace internal {
    template<class K,class Tag_,bool=iterator_tag_traits<Tag_>::is_iterator>
      struct Type_or_iter : K::template Type<Tag_> {};
    template<class K,class Tag_>
      struct Type_or_iter<K, Tag_, true> : K::template Iterator<Tag_> {};
  }
  template<class K, class Base=K, class List=typename typeset_union<typename K::Object_list,typename K::Iterator_list>::type> struct Define_kernel_types;
  template<class K, class Base>
    struct Define_kernel_types <K, Base, typeset<> > : Base {};
  template<class K>
    struct Define_kernel_types <K, void, typeset<> > {};
  template<class K, class Base, class List>
    struct Define_kernel_types :
      Typedef_tag_type<typename List::head,
        typename internal::Type_or_iter<K,typename List::head>::type,
        Define_kernel_types<K, Base, typename List::tail>
      > {};
}
#endif
