  This directory contains two includes from CGAL, included here to 
make it easy to compile FPG under various systems. They are completely
standalone (it is not necassary to install CGAL, neither BOOST 
or other CGAL dependancies).

  B. Levy, Fri Mar 21 23:10:34 CET 2014

=================================================================

FPU.h: 
   controls floating point unit operations, rounding modes etc...

Static_filter_error.h: 
   the number_type class that computes error bounds for predicate filters.
   
Both files are licensed under the LGPL v3 (included here)

basic.h:
   Replacements for the CGAL macros and functions used by FPG