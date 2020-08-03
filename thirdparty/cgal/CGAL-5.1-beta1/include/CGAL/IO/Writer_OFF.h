// Copyright (c) 1997
// Utrecht University (The Netherlands),
// ETH Zurich (Switzerland),
// INRIA Sophia-Antipolis (France),
// Max-Planck-Institute Saarbruecken (Germany),
// and Tel-Aviv University (Israel).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Stream_support/include/CGAL/IO/Writer_OFF.h $
// $Id: Writer_OFF.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Lutz Kettner  <kettner@mpi-sb.mpg.de>

#ifndef CGAL_IO_WRITER_OFF_H
#define CGAL_IO_WRITER_OFF_H 1
#include <CGAL/IO/File_writer_OFF.h>
#include <CGAL/IO/Generic_writer.h>

namespace CGAL {

template < class Pt >
class Writer_OFF : public Generic_writer<File_writer_OFF,Pt> {
public:
    Writer_OFF( std::ostream& out,
                std::size_t   v,
                std::size_t   h,
                std::size_t   f,
                bool          verbose = false)
    : Generic_writer<File_writer_OFF,Pt>(
        File_writer_OFF( verbose), out, v, h, f
    ) {}
    Writer_OFF( std::ostream& out,
                std::size_t   v,
                std::size_t   h,
                std::size_t   f,
                const File_header_OFF& header)
    : Generic_writer<File_writer_OFF,Pt>(
        File_writer_OFF( header), out, v, h, f
    ) {}
};

} //namespace CGAL
#endif // CGAL_IO_WRITER_OFF_H //
// EOF //
