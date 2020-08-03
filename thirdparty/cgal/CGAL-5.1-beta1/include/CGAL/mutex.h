// Copyright (c) 2016 GeometryFactory (France)
//  All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Installation/include/CGAL/mutex.h $
// $Id: mutex.h 52164b1 2019-10-19T15:34:59+02:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial

#ifndef CGAL_MUTEX_H
#define CGAL_MUTEX_H

#include <CGAL/config.h>

#ifdef CGAL_HAS_THREADS
#ifdef CGAL_CAN_USE_CXX11_MUTEX
#include <mutex>
#define CGAL_MUTEX std::mutex
#define CGAL_SCOPED_LOCK(M) std::unique_lock<std::mutex> scoped_lock(M)
#else
#include <boost/thread/mutex.hpp>
#define CGAL_MUTEX boost::mutex
#define CGAL_SCOPED_LOCK(M) boost::mutex::scoped_lock scoped_lock(M)
#endif
#endif
#endif // CGAL_MUTEX_H
