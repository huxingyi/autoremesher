// Copyright (c) 2000  Max-Planck-Institute Saarbruecken (Germany).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Partition_2/include/CGAL/Partition_2/Turn_reverser.h $
// $Id: Turn_reverser.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Susan Hert <hert@mpi-sb.mpg.de>

#ifndef   CGAL_TURN_REVERSER_H
#define   CGAL_TURN_REVERSER_H

#include <CGAL/license/Partition_2.h>


namespace CGAL {

template <class Point_2, class TurnPredicate>
class Turn_reverser
{
public:
   Turn_reverser() {}
   Turn_reverser( const TurnPredicate& t ): turn(t) {}

   bool operator() (const Point_2& p1,
                    const Point_2& p2,
                    const Point_2& p3) const
   {   return turn(p2, p1, p3); }

private:
   TurnPredicate turn;
};


}

#endif // CGAL_TURN_REVERSER_H
