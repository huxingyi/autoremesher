/*****************************************************************

COPYRIGHT NOTIFICATION

This program discloses material protectable under copyright laws of
the United States. Permission to copy and modify this software and its
documentation for internal research use is hereby granted, provided
that this notice is retained thereon and on all copies or modifications.
The University of Chicago makes no representations as to the suitability
and operability of this software for any purpose.
It is provided "as is" without express or implied warranty.

Use of this software for commercial purposes is expressly prohibited
without contacting

Jorge J. More'
Mathematics and Computer Science Division
Argonne National Laboratory
9700 S. Cass Ave.
Argonne, Illinois 60439-4844
e-mail: more@mcs.anl.gov

Argonne National Laboratory with facilities in the states of
Illinois and Idaho, is owned by The United States Government, and
operated by the University of Chicago under provision of a contract
with the Department of Energy.

*****************************************************************/

#ifndef ICFS_H
#define ICFS_H

int dicfs_(int *n, int *nnz, double *a, double *adiag, int *acol_ptr__,
		   int *arow_ind__, double *l, double *ldiag, int *lcol_ptr__,
		   int * lrow_ind__, int *p, double *alpha, int *iwa, double * wa1,
		   double *wa2);

int dicf_(int *n, int *nnz, double *a, double *diag, int *col_ptr__,
		  int *row_ind__, int *p, int *info, int *indr, int *indf, int *list,
		  double *w);

int dsel2_(int *n, double *x, int *keys, int *k);

int insort_(int *n, int *keys);

int ihsort_(int *n, int *keys);

int dstrsol_(int *n, double *l, double *ldiag, int *jptr, int *indr,
			 double *r__, char *task);

#endif
