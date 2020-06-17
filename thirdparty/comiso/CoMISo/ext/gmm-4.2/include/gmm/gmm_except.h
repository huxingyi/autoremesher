/* -*- c++ -*- (enables emacs c++ mode) */
/*===========================================================================
 
 Copyright (C) 2002-2012 Yves Renard
 
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

/** @file gmm_except.h 
    @author Yves Renard <Yves.Renard@insa-lyon.fr>
    @author Julien Pommier <Julien.Pommier@insa-toulouse.fr>
    @date September 01, 2002.
    @brief Definition of basic exceptions.
*/

#ifndef GMM_EXCEPT_H__
#define GMM_EXCEPT_H__

#include "gmm_std.h"

namespace gmm {

/* *********************************************************************** */
/*	Getfem++ generic errors.                     			   */
/* *********************************************************************** */

  class gmm_error: public std::logic_error {
  public:
    gmm_error(const std::string& what_arg): std::logic_error (what_arg) {}
  };

#ifdef GETFEM_HAVE_PRETTY_FUNCTION
#  define GMM_PRETTY_FUNCTION __PRETTY_FUNCTION__
#else 
#  define GMM_PRETTY_FUNCTION ""
#endif

  // Errors : GMM_THROW should not be used on its own.
  //          GMM_ASSERT1 : Non-maskable errors. Typically for in/ouput and
  //               when the test do not significantly reduces the performance.
  //          GMM_ASSERT2 : All tests which are potentially performance
  //               consuming. Not hidden by default. Hidden when NDEBUG is
  //               defined.
  //          GMM_ASSERT3 : For internal checks. Hidden by default. Active
  //               only when DEBUG_MODE is defined.
// __EXCEPTIONS is defined by gcc, _CPPUNWIND is defined by visual c++
#if defined(__EXCEPTIONS) || defined(_CPPUNWIND)
  inline void short_error_throw(const char *file, int line, const char *func,
				const char *errormsg) {
    std::stringstream msg;
    msg << "Error in " << file << ", line " << line << " " << func
	<< ": \n" << errormsg << std::ends;
    throw gmm::gmm_error(msg.str());	
  }
# define GMM_THROW_(type, errormsg) {					\
    std::stringstream msg;						\
    msg << "Error in " << __FILE__ << ", line "				\
	<< __LINE__ << " " << GMM_PRETTY_FUNCTION << ": \n"		\
	<< errormsg << std::ends;						\
    throw (type)(msg.str());						\
  }
#else
#ifndef _MSC_VER
# define abort_no_return() ::abort()
#else
// apparently ::abort() on windows is not declared with __declspec(noreturn) so the compiler spits a lot of warnings when abort is used.
# define abort_no_return() { assert("GMM ABORT"==0); throw "GMM ABORT"; }
#endif

  inline void short_error_throw(const char *file, int line, const char *func,
				const char *errormsg) {
    std::stringstream msg;
    msg << "Error in " << file << ", line " << line << " " << func
	<< ": \n" << errormsg << std::ends;
    std::cerr << msg.str() << std::endl;
    abort_no_return();	
  }

# define GMM_THROW_(type, errormsg) {					\
    std::stringstream msg;						\
    msg << "Error in " << __FILE__ << ", line "				\
	<< __LINE__ << " " << GMM_PRETTY_FUNCTION << ": \n"		\
	<< errormsg;						\
    std::cerr << msg.str() << std::endl;                                \
    abort_no_return();							\
  }
#endif

# define GMM_ASSERT1(test, errormsg)		        		\
  { if (!(test)) GMM_THROW_(gmm::gmm_error, errormsg); }

  // inline void GMM_THROW() IS_DEPRECATED;
  inline void GMM_THROW() {}
#define GMM_THROW(a, b) { GMM_THROW_(a,b); gmm::GMM_THROW(); }

#if defined(NDEBUG)
# define GMM_ASSERT2(test, errormsg) {}
# define GMM_ASSERT3(test, errormsg) {}
#elif !defined(GMM_FULL_NDEBUG)
# define GMM_ASSERT2(test, errormsg)				        \
  { if (!(test)) gmm::short_error_throw(__FILE__, __LINE__,		\
				   GMM_PRETTY_FUNCTION, errormsg); }
# define GMM_ASSERT3(test, errormsg)				        \
  { if (!(test)) gmm::short_error_throw(__FILE__, __LINE__,		\
				   GMM_PRETTY_FUNCTION, errormsg); }
#else
# define GMM_ASSERT2(test, errormsg)          				\
  { if (!(test)) gmm::short_error_throw(__FILE__, __LINE__,		\
				   GMM_PRETTY_FUNCTION, errormsg); }
# define GMM_ASSERT3(test, errormsg)
#endif

/* *********************************************************************** */
/*	Getfem++ warnings.                         			   */
/* *********************************************************************** */

  // This allows to dynamically hide warnings
  struct warning_level {
    static int level(int l = -2)
    { static int level_ = 3; return (l != -2) ? (level_ = l) : level_; }
  };

  inline void set_warning_level(int l) { warning_level::level(std::max(0,l)); }
  inline int  get_warning_level(void)  { return warning_level::level(-2); }

  // This allow not too compile some Warnings
#ifndef GMM_WARNING_LEVEL
# define GMM_WARNING_LEVEL 4
#endif

  // Warning levels : 0 always printed
  //                  1 very important : specify a possible error in the code.
  //                  2 important : specify a default of optimization for inst.
  //                  3 remark
  //                  4 ignored by default.

#define GMM_WARNING_MSG(level_, thestr)  {			       \
      std::stringstream msg;                                           \
      msg << "Level " << level_ << " Warning in " << __FILE__ << ", line " \
          << __LINE__ << ": " << thestr;		       \
       std::cerr << msg.str() << std::endl;                            \
    }

#define GMM_WARNING0(thestr) GMM_WARNING_MSG(0, thestr)

#if GMM_WARNING_LEVEL > 0
# define GMM_WARNING1(thestr)                                           \
  { if (1 <= gmm::warning_level::level()) GMM_WARNING_MSG(1, thestr) }
#else
# define GMM_WARNING1(thestr) {}
#endif

#if GMM_WARNING_LEVEL > 1
# define GMM_WARNING2(thestr)                                           \
  { if (2 <= gmm::warning_level::level()) GMM_WARNING_MSG(2, thestr) } 
#else
# define GMM_WARNING1(thestr) {}
#endif

#if GMM_WARNING_LEVEL > 2
# define GMM_WARNING3(thestr)                                           \
  { if (3 <= gmm::warning_level::level()) GMM_WARNING_MSG(3, thestr) } 
#else
# define GMM_WARNING1(thestr) {}
#endif

#if GMM_WARNING_LEVEL > 3
# define GMM_WARNING4(thestr)                                           \
  { if (4 <= gmm::warning_level::level()) GMM_WARNING_MSG(4, thestr) } 
#else
# define GMM_WARNING1(thestr) {}
#endif

/* *********************************************************************** */
/*	Getfem++ traces.                         			   */
/* *********************************************************************** */

  // This allows to dynamically hide traces
  struct traces_level {
    static int level(int l = -2)
    { static int level_ = 3; return (l != -2) ? (level_ = l) : level_; }
  };

  inline void set_traces_level(int l) { traces_level::level(std::max(0,l)); }

  // This allow not too compile some Warnings
#ifndef GMM_TRACES_LEVEL
# define GMM_TRACES_LEVEL 3
#endif

  // Traces levels : 0 always printed
  //                 1 Susceptible to occur once in a program.
  //                 2 Susceptible to occur occasionnaly in a program (10).
  //                 3 Susceptible to occur often (100).
  //                 4 Susceptible to occur very often (>1000).

#define GMM_TRACE_MSG_MPI     // for Parallelized version
#define GMM_TRACE_MSG(level_, thestr)  {			       \
    GMM_TRACE_MSG_MPI {						       \
      std::stringstream msg;                                           \
      msg << "Trace " << level_ << " in " << __FILE__ << ", line "     \
          << __LINE__ << ": " << thestr;        		       \
      std::cout << msg.str() << std::endl;			       \
    }                                                                  \
  }        

#define GMM_TRACE0(thestr) GMM_TRACE_MSG(0, thestr)

#if GMM_TRACES_LEVEL > 0
# define GMM_TRACE1(thestr)						\
  { if (1 <= gmm::traces_level::level()) GMM_TRACE_MSG(1, thestr) }
#else
# define GMM_TRACE1(thestr) {}
#endif
  
#if GMM_TRACES_LEVEL > 1
# define GMM_TRACE2(thestr)						\
  { if (2 <= gmm::traces_level::level()) GMM_TRACE_MSG(2, thestr) } 
#else
# define GMM_TRACE2(thestr) {}
#endif
  
#if GMM_TRACES_LEVEL > 2
# define GMM_TRACE3(thestr)						\
  { if (3 <= gmm::traces_level::level()) GMM_TRACE_MSG(3, thestr) } 
#else
# define GMM_TRACE3(thestr) {}
#endif
  
#if GMM_TRACES_LEVEL > 3
# define GMM_TRACE4(thestr)						\
  { if (4 <= gmm::traces_level::level()) GMM_TRACE_MSG(4, thestr) } 
#else
# define GMM_TRACE4(thestr) {}
#endif
  
  
  /* ********************************************************************* */
  /*    Definitions for compatibility with old versions.        	   */
  /* ********************************************************************* */ 
  
  using std::invalid_argument;
  
  struct dimension_error : public std::logic_error
  { dimension_error(const std::string& w): std::logic_error(w) {} };
  struct file_not_found_error : public std::logic_error
  { file_not_found_error(const std::string& w): std::logic_error (w) {} };
  struct internal_error : public std::logic_error
  { internal_error(const std::string& w): std::logic_error(w) {} };
  struct failure_error : public std::logic_error
  { failure_error(const std::string& w): std::logic_error (w) {} };
  struct not_linear_error : public std::logic_error
  { not_linear_error(const std::string& w): std::logic_error (w) {} };
  struct to_be_done_error : public std::logic_error
  { to_be_done_error(const std::string& w): std::logic_error (w) {} };

#define GMM_STANDARD_CATCH_ERROR   catch(std::logic_error e)	\
    {								\
      std::cerr << "============================================\n";	\
      std::cerr << "|      An error has been detected !!!      |\n";	\
      std::cerr << "============================================\n";	\
      std::cerr << e.what() << std::endl << std::endl;				\
      exit(1);							\
    }								\
  catch(std::runtime_error e)					\
    {								\
      std::cerr << "============================================\n";	\
      std::cerr << "|      An error has been detected !!!      |\n";	\
      std::cerr << "============================================\n";	\
      std::cerr << e.what() << std::endl << std::endl;				\
      exit(1);							\
    }								\
  catch(std::bad_alloc) {					\
    std::cerr << "============================================\n";	\
    std::cerr << "|  A bad allocation has been detected !!!  |\n";	\
    std::cerr << "============================================\n";	\
    exit(1);							\
  }								\
  catch(std::bad_typeid) {					\
    std::cerr << "============================================\n";	\
    std::cerr << "|  A bad typeid     has been detected !!!  |\n";	\
    std::cerr << "============================================\n";	\
    exit(1);							\
  }								\
  catch(std::bad_exception) {					\
    std::cerr << "============================================\n";	\
    std::cerr << "|  A bad exception  has been detected !!!  |\n";	\
    std::cerr << "============================================\n";	\
    exit(1);							\
  }								\
  catch(std::bad_cast) {					\
    std::cerr << "============================================\n";	\
    std::cerr << "|    A bad cast  has been detected !!!     |\n";	\
    std::cerr << "============================================\n";	\
    exit(1);							\
  }								\
  catch(...) {							\
    std::cerr << "============================================\n";	\
    std::cerr << "|  An unknown error has been detected !!!  |\n";	\
    std::cerr << "============================================\n";	\
    exit(1);							\
  }
  //   catch(ios_base::failure) { 
  //     std::cerr << "============================================\n";
  //     std::cerr << "| A ios_base::failure has been detected !!!|\n";
  //     std::cerr << "============================================\n";
  //     exit(1);
  //   } 

#if defined(__GNUC__) && (__GNUC__ > 3)
# define GMM_SET_EXCEPTION_DEBUG				\
  std::set_terminate(__gnu_cxx::__verbose_terminate_handler);
#else
# define GMM_SET_EXCEPTION_DEBUG
#endif

}


#endif /* GMM_EXCEPT_H__ */
