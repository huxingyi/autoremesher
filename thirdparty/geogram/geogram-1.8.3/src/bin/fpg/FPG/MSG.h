#ifndef MSG_H
#define MSG_H

#include <iostream>

#ifdef DEBUG
  #define MSG(v)  {std::cout <<__PRETTY_FUNCTION__ <<":" << v << std::endl; }
  #define TRACE() {std::cerr <<__PRETTY_FUNCTION__ << std::endl; }
#else
  #define MSG(v)  {}
  #define TRACE() {}
#endif

#define VERBOSE_ERROR(v) { std::cerr << "!! " << __FILE__ << ":" << __LINE__ << " (" << __PRETTY_FUNCTION__ << ") " << v << std::endl; }
#define ERROR(v) VERBOSE_ERROR(v)
#define VERBOSE_FAIL(v)  { ERROR(v); *((int*)0) = 4711; }
#define FAIL(v) { ERROR(v); exit(-1); }

#endif


