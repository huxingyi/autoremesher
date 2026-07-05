//----------------------------------------------------------------------
// File:			kd_fix_rad_search.h
// Programmer:		Sunil Arya and David Mount
// Description:		Standard kd-tree fixed-radius kNN search
// Last modified:	05/03/05 (Version 1.1)
//----------------------------------------------------------------------
// Copyright (c) 1997-2005 University of Maryland and Sunil Arya and
// David Mount.  All Rights Reserved.
// 
// This software and related documentation is part of the Approximate
// Nearest Neighbor Library (ANN).  This software is provided under
// the provisions of the Lesser GNU Public License (LGPL).  See the
// file ../ReadMe.txt for further information.
// 
// The University of Maryland (U.M.) and the authors make no
// representations about the suitability or fitness of this software for
// any purpose.  It is provided "as is" without express or implied
// warranty.
//----------------------------------------------------------------------
// History:
//	Revision 1.1  05/03/05
//		Initial release
//----------------------------------------------------------------------

#ifndef ANN_kd_fix_rad_search_H
#define ANN_kd_fix_rad_search_H

#include "kd_tree.h"					// kd-tree declarations
#include "kd_util.h"					// kd-tree utilities
#include "pr_queue_k.h"					// k-element priority queue

#include "ANNperf.h"				// performance evaluation

// [SL] replaced globals with parameters

//----------------------------------------------------------------------
//	Parameters
//		These are active for the life of each call to
//		annRangeSearch().  They are set to save the number of
//		variables that need to be passed among the various search
//		procedures.
//----------------------------------------------------------------------

struct s_kFRSearchParams 
{
  int			ANNkdFRDim;	   // dimension of space
  ANNpoint		ANNkdFRQ;	   // query point
  ANNdist		ANNkdFRSqRad;	   // squared radius search bound
  double		ANNkdFRMaxErr;	   // max tolerable squared error
  ANNpointArray	        ANNkdFRPts;	   // the points
  ANNmin_k*		ANNkdFRPointMK;	   // set of k closest points
  int			ANNkdFRPtsVisited; // total points visited
  int			ANNkdFRPtsInRange; // number of points in the range
  int	                ANNptsVisited;	   // number of pts visited in search
} ;

#endif
