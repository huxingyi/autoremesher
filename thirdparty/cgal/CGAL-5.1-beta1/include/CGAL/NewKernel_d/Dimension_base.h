// Copyright (c) 2014
// INRIA Saclay-Ile de France (France)
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/NewKernel_d/include/CGAL/NewKernel_d/Dimension_base.h $
// $Id: Dimension_base.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
// Author(s)     : Marc Glisse

#ifndef CGAL_KD_DIMENSION_BASE_h
#define CGAL_KD_DIMENSION_BASE_h
#include <CGAL/Dimension.h>
#include <CGAL/assertions.h>
#include <CGAL/NewKernel_d/utils.h>
#include <CGAL/use.h>
namespace CGAL {
struct Store_dimension_base {
        //TODO: add some assertions
        Store_dimension_base(int dim=UNKNOWN_DIMENSION):dim_(dim){}
        int dimension()const{return dim_;}
        void set_dimension(int dim){dim_=dim;}
        private:
        int dim_;
};
template<class=Dynamic_dimension_tag>
struct Dimension_base {
        Dimension_base(int = UNKNOWN_DIMENSION){}
        int dimension() const { return UNKNOWN_DIMENSION; }
        void set_dimension(int) {}
};
template<int dim_>
struct Dimension_base<Dimension_tag<dim_> > {
        Dimension_base(){}
        Dimension_base(int CGAL_assertion_code(dim)){CGAL_assertion(dim_==dim);}
        int dimension()const{return dim_;}
        void set_dimension(int dim){
          CGAL_assertion(dim_==dim);
          CGAL_USE(dim);
        }
};
}
#endif

