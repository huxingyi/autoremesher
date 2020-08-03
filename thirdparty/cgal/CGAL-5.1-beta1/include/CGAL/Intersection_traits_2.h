// Copyright (c) 2011 GeometryFactory (France). All rights reserved.
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Intersections_2/include/CGAL/Intersection_traits_2.h $
// $Id: Intersection_traits_2.h 8b41189 2020-03-26T18:58:21+01:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Philipp Möller

#ifndef CGAL_INTERSECTION_TRAITS_2_H
#define CGAL_INTERSECTION_TRAITS_2_H

#include <CGAL/Intersection_traits.h>

#include <boost/variant.hpp>
#include <boost/optional.hpp>
#include <vector>

namespace CGAL {

CGAL_INTERSECTION_TRAITS_2(Line_2, Line_2, Point_2, Line_2)

CGAL_INTERSECTION_TRAITS_2(Segment_2, Line_2, Point_2, Segment_2)
CGAL_INTERSECTION_TRAITS_2(Line_2, Segment_2, Point_2, Segment_2)

CGAL_INTERSECTION_TRAITS_2(Segment_2, Segment_2, Point_2, Segment_2)

CGAL_INTERSECTION_TRAITS_2(Ray_2, Line_2, Point_2, Ray_2)
CGAL_INTERSECTION_TRAITS_2(Line_2, Ray_2, Point_2, Ray_2)

CGAL_INTERSECTION_TRAITS_2(Ray_2, Segment_2, Point_2, Segment_2)
CGAL_INTERSECTION_TRAITS_2(Segment_2, Ray_2, Point_2, Segment_2)

CGAL_INTERSECTION_TRAITS_3(Ray_2, Ray_2, Point_2, Segment_2, Ray_2)

CGAL_INTERSECTION_TRAITS_2(Triangle_2, Line_2, Point_2, Segment_2)
CGAL_INTERSECTION_TRAITS_2(Line_2, Triangle_2, Point_2, Segment_2)

CGAL_INTERSECTION_TRAITS_2(Triangle_2, Segment_2, Point_2, Segment_2)
CGAL_INTERSECTION_TRAITS_2(Segment_2, Triangle_2, Point_2, Segment_2)

CGAL_INTERSECTION_TRAITS_2(Triangle_2, Ray_2, Point_2, Segment_2)
CGAL_INTERSECTION_TRAITS_2(Ray_2, Triangle_2, Point_2, Segment_2)

template<typename K>
struct Intersection_traits<K, typename K::Triangle_2, typename K::Triangle_2>  {
  typedef typename
  boost::variant< typename K::Point_2, typename K::Segment_2,
                  typename K::Triangle_2, typename std::vector< typename K::Point_2 > > variant_type;
  typedef typename boost::optional< variant_type > result_type;
};


CGAL_INTERSECTION_TRAITS_2(Iso_rectangle_2, Line_2, Point_2, Segment_2)
CGAL_INTERSECTION_TRAITS_2(Line_2, Iso_rectangle_2, Point_2, Segment_2)

CGAL_INTERSECTION_TRAITS_2(Iso_rectangle_2, Segment_2, Point_2, Segment_2)
CGAL_INTERSECTION_TRAITS_2(Segment_2, Iso_rectangle_2, Point_2, Segment_2)

CGAL_INTERSECTION_TRAITS_2(Iso_rectangle_2, Ray_2, Point_2, Segment_2)
CGAL_INTERSECTION_TRAITS_2(Ray_2, Iso_rectangle_2, Point_2, Segment_2)

// undocumented

// Variants of one for backwards compatibility
template<typename K>
struct Intersection_traits<K, typename K::Iso_rectangle_2, typename K::Iso_rectangle_2>  {
  typedef typename boost::variant<typename K::Iso_rectangle_2> variant_type;
  typedef boost::optional<variant_type> result_type;
};


// Point_2 is special
template<typename K, typename B>
struct Intersection_traits<K, typename K::Point_2, B> {
  typedef typename boost::variant<typename K::Point_2> variant_type;
  typedef boost::optional<variant_type> result_type;
};

template<typename K, typename A>
struct Intersection_traits<K, A, typename K::Point_2> {
  typedef typename boost::variant<typename K::Point_2> variant_type;
  typedef boost::optional<variant_type> result_type;
};

template<typename K>
struct Intersection_traits<K, typename K::Point_2, typename K::Point_2> {
  typedef typename boost::variant<typename K::Point_2> variant_type;
  typedef boost::optional<variant_type> result_type;
};

template<typename K>
struct Intersection_traits<K, typename K::Iso_rectangle_2, typename K::Triangle_2>
{
  typedef typename boost::variant<typename K::Segment_2, typename K::Triangle_2,
                                  typename K::Point_2,
                                  typename std::vector< typename K::Point_2 > > variant_type;
  typedef typename boost::optional < variant_type > result_type;
};

template<typename K>
struct Intersection_traits<K, typename K::Triangle_2, typename K::Iso_rectangle_2>
  : public Intersection_traits<K, typename K::Iso_rectangle_2, typename K::Triangle_2> {};

template<typename K, class B>
struct Intersection_traits<K, CGAL::Bbox_2, B>  {
  typedef typename Intersection_traits<K,typename K::Iso_rectangle_2, B>::result_type result_type;
};

template<typename K, class A>
struct Intersection_traits<K, A, CGAL::Bbox_2>  {
  typedef typename Intersection_traits<K,typename K::Iso_rectangle_2, A>::result_type result_type;
};

template<typename K>
struct Intersection_traits<K, CGAL::Bbox_2, typename K::Point_2>  {
  typedef typename boost::variant<typename K::Point_2> variant_type;
  typedef boost::optional<variant_type> result_type;
};

template<typename K>
struct Intersection_traits<K, typename K::Point_2, CGAL::Bbox_2>  {
  typedef typename boost::variant<typename K::Point_2> variant_type;
  typedef boost::optional<variant_type> result_type;
};

} // namespace CGAL

#endif /* CGAL_INTERSECTION_TRAITS_2_H */

