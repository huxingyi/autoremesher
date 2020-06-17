
//=============================================================================
//
//  HELPER for OpenMP include
//
//=============================================================================


#ifndef COMISO_OMP_HH
#define COMISO_OMP_HH


// use openmp if available
#if defined(_OPENMP)
  #include <omp.h>
#else // define behavior of single thread
  typedef int omp_int_t;
  inline omp_int_t omp_get_thread_num() { return 0;}
  inline omp_int_t omp_get_max_threads() { return 1;}
  inline omp_int_t omp_get_num_procs() { return 1;}
  inline void      omp_set_dynamic            (omp_int_t _n) {}
  inline void      omp_set_num_threads_nested (omp_int_t _n) {}
  inline void      omp_set_nested             (omp_int_t _n) {}
#endif


//=============================================================================
#endif // COMISO_OMP_HH defined
//=============================================================================

