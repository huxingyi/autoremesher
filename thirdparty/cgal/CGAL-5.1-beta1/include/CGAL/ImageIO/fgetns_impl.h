// Copyright (c) 2005-2008 ASCLEPIOS Project, INRIA Sophia-Antipolis (France)
// All rights reserved.
//
// This file is part of the ImageIO Library, and as been adapted for CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/CGAL_ImageIO/include/CGAL/ImageIO/fgetns_impl.h $
// $Id: fgetns_impl.h 07c4ada 2019-10-19T15:50:09+02:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//
// Author(s)     :  ASCLEPIOS Project (INRIA Sophia-Antipolis), Laurent Rineau

#ifdef CGAL_HEADER_ONLY
#define CGAL_INLINE_FUNCTION inline
#else
#define CGAL_INLINE_FUNCTION
#endif

#include <string.h>

/* get a string from a file and discard the ending newline character
   if any */
CGAL_INLINE_FUNCTION
char *fgetns(char *str, int n,  _image *im ) {

  memset( str, 0, n );
  char* ret = ImageIO_gets( im, str, n );

  if(!ret) return nullptr;

  std::size_t l = strlen(str);
  if(l > 0 && str[l-1] == '\n') str[l-1] = '\0';
  return ret;
}
