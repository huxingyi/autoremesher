/* -*- c++ -*- (enables emacs c++ mode) */
/*===========================================================================
 
 Copyright (C) 2004-2012 Yves Renard
 
 This file is a part of GETFEM++
 
 Getfem++  is  free software;  you  can  redistribute  it  and/or modify it
 under  the  terms  of the  GNU  Lesser General Public License as published
 by  the  Free Software Foundation;  either version 3 of the License,  or
 (at your option) any later version along with the GCC Runtime Library
 Exception either version 3.1 or (at your option) any later version.
 This program  is  distributed  in  the  hope  that it will be useful,  but
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or  FITNESS  FOR  A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 License and GCC Runtime Library Exception for more details.
 You  should  have received a copy of the GNU Lesser General Public License
 along  with  this program;  if not, write to the Free Software Foundation,
 Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301, USA.
 
 As a special exception, you  may use  this file  as it is a part of a free
 software  library  without  restriction.  Specifically,  if   other  files
 instantiate  templates  or  use macros or inline functions from this file,
 or  you compile this  file  and  link  it  with other files  to produce an
 executable, this file  does  not  by itself cause the resulting executable
 to be covered  by the GNU Lesser General Public License.  This   exception
 does not  however  invalidate  any  other  reasons why the executable file
 might be covered by the GNU Lesser General Public License.
 
===========================================================================*/
#ifndef GMM_PRECOND_H
#define GMM_PRECOND_H

#include "gmm_kernel.h"

/** @file gmm_precond.h
    @author  Yves Renard <Yves.Renard@insa-lyon.fr>
    @date March 29, 2004.
    @brief gmm preconditioners.
 */

/* Preconditioner concept :                                                */
/*                                                                         */
/* A the matrix, P the preconditioner PA well conditioned.                 */
/* PRECOND precontioner type.                                              */
/* mult(P, v, w) :  w <- P v                                               */
/* transposed_mult(P, v, w)       : w <- transposed(P) v                   */
/* left_mult(P, v, w)             : see qmr solver                         */
/* right_mult(P, v, w)            : see qmr solver                         */
/* transposed_left_mult(P, v, w)  : see qmr solver                         */
/* transposed_right_mult(P, v, w) : see qmr solver                         */
/*                                                                         */
/* PRECOND P() : empty preconditioner.                                     */
/* PRECOND P(A, ...) : preconditioner for the matrix A, with optional      */
/*                     parameters                                          */
/* PRECOND(...)  : empty precondtioner with parameters set.                */
/* P.build_with(A) : build a precondtioner for A.                          */
/*                                                                         */
/* *********************************************************************** */




#endif 

