//----------------------------------------------------------------------
// File:			kd_pr_search.cpp
// Programmer:		Sunil Arya and David Mount
// Description:		Priority search for kd-trees
// Last modified:	01/04/05 (Version 1.0)
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
//	Revision 0.1  03/04/98
//		Initial release
//----------------------------------------------------------------------

#include "kd_pr_search.h"				// kd priority search declarations

//----------------------------------------------------------------------
//	Approximate nearest neighbor searching by priority search.
//		The kd-tree is searched for an approximate nearest neighbor.
//		The point is returned through one of the arguments, and the
//		distance returned is the SQUARED distance to this point.
//
//		The method used for searching the kd-tree is called priority
//		search.  (It is described in Arya and Mount, ``Algorithms for
//		fast vector quantization,'' Proc. of DCC '93: Data Compression
//		Conference}, eds. J. A. Storer and M. Cohn, IEEE Press, 1993,
//		381--390.)
//
//		The cell of the kd-tree containing the query point is located,
//		and cells are visited in increasing order of distance from the
//		query point.  This is done by placing each subtree which has
//		NOT been visited in a priority queue, according to the closest
//		distance of the corresponding enclosing rectangle from the
//		query point.  The search stops when the distance to the nearest
//		remaining rectangle exceeds the distance to the nearest point
//		seen by a factor of more than 1/(1+eps). (Implying that any
//		point found subsequently in the search cannot be closer by more
//		than this factor.)
//
//		The main entry point is annkPriSearch() which sets things up and
//		then call the recursive routine ann_pri_search().  This is a
//		recursive routine which performs the processing for one node in
//		the kd-tree.  There are two versions of this virtual procedure,
//		one for splitting nodes and one for leaves. When a splitting node
//		is visited, we determine which child to continue the search on
//		(the closer one), and insert the other child into the priority
//		queue.  When a leaf is visited, we compute the distances to the
//		points in the buckets, and update information on the closest
//		points.
//
//		Some trickery is used to incrementally update the distance from
//		a kd-tree rectangle to the query point.  This comes about from
//		the fact that which each successive split, only one component
//		(along the dimension that is split) of the squared distance to
//		the child rectangle is different from the squared distance to
//		the parent rectangle.
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//	annkPriSearch - priority search for k nearest neighbors
//----------------------------------------------------------------------

void ANNkd_tree::annkPriSearch(
	ANNpoint			q,				// query point
	int					k,				// number of near neighbors to return
	ANNidxArray			nn_idx,			// nearest neighbor indices (returned)
	ANNdistArray		dd,				// dist to near neighbors (returned)
	double				eps)			// error bound (ignored)
{

        s_PriSearchParams params;
										// max tolerable squared error
	params.ANNprMaxErr = ANN_POW(1.0 + eps);
	ANN_FLOP(2)							// increment floating ops

	params.ANNprDim = dim;						// copy arguments to static equivs
	params.ANNprQ = q;
	params.ANNprPts = pts;
	params.ANNptsVisited = 0;					// initialize count of points visited

	// [Bruno Levy] Allocated on the stack, more multithread-friendly (and no need to deallocate).
	void* the_nodes = alloca((k+1)*ANNmin_k::NODE_SIZE) ;
	ANNmin_k the_min_k(k, the_nodes) ;    
	params.ANNprPointMK = &the_min_k;		// create set for closest k points
										// distance to root box
	ANNdist box_dist = annBoxDistance(q,
				bnd_box_lo, bnd_box_hi, dim);
	
	ANNpr_queue the_pqueue(n_pts) ;
	params.ANNprBoxPQ = &the_pqueue; // create priority queue for boxes
	params.ANNprBoxPQ->insert(box_dist, root); // insert root in priority queue

	while (params.ANNprBoxPQ->non_empty() &&
		(!(ANNmaxPtsVisited != 0 && params.ANNptsVisited > ANNmaxPtsVisited))) {
		ANNkd_ptr np;					// next box from prior queue

										// extract closest box from queue
		params.ANNprBoxPQ->extr_min(box_dist, (void *&) np);

		ANN_FLOP(2)						// increment floating ops
		if (box_dist*params.ANNprMaxErr >= params.ANNprPointMK->max_key())
			break;

		np->ann_pri_search(box_dist,&params);	// search this subtree.
	}

	for (int i = 0; i < k; i++) {		// extract the k-th closest points
		dd[i] = params.ANNprPointMK->ith_smallest_key(i);
		nn_idx[i] = params.ANNprPointMK->ith_smallest_info(i);
	}
}

//----------------------------------------------------------------------
//	kd_split::ann_pri_search - search a splitting node
//----------------------------------------------------------------------

void ANNkd_split::ann_pri_search(ANNdist box_dist, s_PriSearchParams *params)
{
	ANNdist new_dist;					// distance to child visited later
										// distance to cutting plane
	ANNcoord cut_diff = params->ANNprQ[cut_dim] - cut_val;

	if (cut_diff < 0) {					// left of cutting plane
		ANNcoord box_diff = cd_bnds[ANN_LO] - params->ANNprQ[cut_dim];
		if (box_diff < 0)				// within bounds - ignore
			box_diff = 0;
										// distance to further box
		new_dist = (ANNdist) ANN_SUM(box_dist,
				ANN_DIFF(ANN_POW(box_diff), ANN_POW(cut_diff)));

		if (child[ANN_HI] != KD_TRIVIAL)// enqueue if not trivial
			params->ANNprBoxPQ->insert(new_dist, child[ANN_HI]);
										// continue with closer child
		child[ANN_LO]->ann_pri_search(box_dist,params);
	}
	else {								// right of cutting plane
		ANNcoord box_diff = params->ANNprQ[cut_dim] - cd_bnds[ANN_HI];
		if (box_diff < 0)				// within bounds - ignore
			box_diff = 0;
										// distance to further box
		new_dist = (ANNdist) ANN_SUM(box_dist,
				ANN_DIFF(ANN_POW(box_diff), ANN_POW(cut_diff)));

		if (child[ANN_LO] != KD_TRIVIAL)// enqueue if not trivial
			params->ANNprBoxPQ->insert(new_dist, child[ANN_LO]);
										// continue with closer child
		child[ANN_HI]->ann_pri_search(box_dist,params);
	}
	ANN_SPL(1)							// one more splitting node visited
	ANN_FLOP(8)							// increment floating ops
}

//----------------------------------------------------------------------
//	kd_leaf::ann_pri_search - search points in a leaf node
//
//		This is virtually identical to the ann_search for standard search.
//----------------------------------------------------------------------

void ANNkd_leaf::ann_pri_search(ANNdist /* box_dist */,s_PriSearchParams *params)
{
	ANNdist dist;				// distance to data point
	ANNcoord* pp;				// data coordinate pointer
	ANNcoord* qq;				// query coordinate pointer
	ANNdist min_dist;			// distance to k-th closest point
	ANNcoord t;
	int d;

	min_dist = params->ANNprPointMK->max_key(); // k-th smallest distance so far

	for (int i = 0; i < n_pts; i++) {	// check points in bucket

		pp = params->ANNprPts[bkt[i]];			// first coord of next data point
		qq = params->ANNprQ;					// first coord of query point
		dist = 0;

		for(d = 0; d < params->ANNprDim; d++) {
			ANN_COORD(1)				// one more coordinate hit
			ANN_FLOP(4)					// increment floating ops

			t = *(qq++) - *(pp++);		// compute length and adv coordinate
										// exceeds dist to k-th smallest?
			if( (dist = ANN_SUM(dist, ANN_POW(t))) > min_dist) {
				break;
			}
		}

		if (d >= params->ANNprDim &&					// among the k best?
		   (ANN_ALLOW_SELF_MATCH || dist!=0)) { // and no self-match problem
												// add it to the list
			params->ANNprPointMK->insert(dist, bkt[i]);
			min_dist = params->ANNprPointMK->max_key();
		}
	}
	ANN_LEAF(1)							// one more leaf node visited
	ANN_PTS(n_pts)						// increment points visited
	params->ANNptsVisited += n_pts;				// increment number of points visited
}
