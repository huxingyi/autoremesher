
/**
 * Replacement of some functions and macros needed by CGAL
 * B. Levy, Fri Mar 21 23:10:34 CET 2014
 */

#ifndef CGAL_BASIC
#define CGAL_BASIC
#include <iostream>
#include <cassert>
#include <string>

inline void CGAL_error_msg(const std::string& msg) {
   std::cerr << "CGAL error:" << msg << std::endl;
}

inline void CGAL_warning_msg(bool cond, const std::string& msg) {
   if(!cond) 
     {
	std::cerr << "CGAL warning:" << msg << std::endl;
     }
}

inline void CGAL_warning_msg(const std::string& msg) {
   std::cerr << "CGAL warning:" << msg << std::endl;
}

inline void CGAL_error() 
{
   std::cerr << "CGAL error" << std::endl;
}

#define CGAL_expensive_assertion(x) assert(x)

#define CGAL_assertion(x) assert(x)

#define CGAL_BEGIN_NAMESPACE namespace CGAL {
#define CGAL_END_NAMESPACE }

#endif
