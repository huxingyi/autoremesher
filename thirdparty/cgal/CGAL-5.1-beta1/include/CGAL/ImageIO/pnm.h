// Copyright (c) 2005-2008 ASCLEPIOS Project, INRIA Sophia-Antipolis (France)
// All rights reserved.
//
// This file is part of the ImageIO Library, and as been adapted for CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/CGAL_ImageIO/include/CGAL/ImageIO/pnm.h $
// $Id: pnm.h 07c4ada 2019-10-19T15:50:09+02:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//
// Author(s)     :  ASCLEPIOS Project (INRIA Sophia-Antipolis), Laurent Rineau

#ifndef PNM_H
#define PNM_H

#include <stdio.h>

#include <CGAL/ImageIO.h>


int readPpmImage(const char *name,_image *im);
int writePpmImage(char *name, _image *im);
int readPgmAsciiImage(const char *name,_image *im);
int readPgmImage(const char *name,_image *im);
int writePgmImage(char *name,  _image *im);
int testPgmAsciiHeader(char *magic,const char *name);
int testPgmHeader(char *magic,const char *name);
int testPpmHeader(char *magic,const char *name);
PTRIMAGE_FORMAT createPgmFormat();
PTRIMAGE_FORMAT createPgmAscIIFormat();
PTRIMAGE_FORMAT createPpmFormat();

#ifdef CGAL_HEADER_ONLY
#include <CGAL/ImageIO/pnm_impl.h>
#endif // CGAL_HEADER_ONLY

#endif
