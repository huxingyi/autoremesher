/*********************************************************************
  Blosc - Blocked Shuffling and Compression Library

  Unit tests for basic features in Blosc.

  Creation date: 2010-06-07
  Author: Francesc Alted <francesc@blosc.org>

  See LICENSES/BLOSC.txt for details about copyright and rights to use.
**********************************************************************/

#ifndef BLOSC_TEST_COMMON_H
#define BLOSC_TEST_COMMON_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#if defined(_WIN32) && !defined(__MINGW32__)
  #include <time.h>
  #include "win32/stdint-windows.h"
#else
  #include <stdint.h>
  #include <unistd.h>
  #include <sys/time.h>
#endif
#include <math.h>
#include "../blosc/blosc.h"

#if defined(_WIN32)
  /* MSVC and MinGW do not have setenv */
  #define setenv(name, value, overwrite) do {_putenv_s(name, value);} while(0)
#endif


/* This is MinUnit in action (http://www.jera.com/techinfo/jtns/jtn002.html) */
#define mu_assert(message, test) do { if (!(test)) return message; } while (0)
#define mu_run_test(test) do \
    { const char *message = test(); tests_run++;                          \
      if (message) { printf("%c", 'F'); return message;}            \
      else printf("%c", '.'); } while (0)

extern int tests_run;

#define KB  1024
#define MB  (1024*KB)
#define GB  (1024*MB)

/*
  Memory functions.
*/

/** Allocates a block of memory with the specified size and alignment.
    The allocated memory is 'cleaned' before returning to avoid
    accidental re-use of data within or between tests.
 */
static void* blosc_test_malloc(const size_t alignment, const size_t size)
{
  const int32_t clean_value = 0x99;
  void *block = NULL;
  int32_t res = 0;

#if defined(_ISOC11_SOURCE) || (defined(__FreeBSD__) && __STDC_VERSION__ >= 201112L)
  /* C11 aligned allocation. 'size' must be a multiple of the alignment. */
  block = aligned_alloc(alignment, size);
#elif defined(_WIN32)
  /* A (void *) cast needed for avoiding a warning with MINGW :-/ */
  block = (void *)_aligned_malloc(size, alignment);
#elif _POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600
  /* Platform does have an implementation of posix_memalign */
  res = posix_memalign(&block, alignment, size);
#elif defined(__APPLE__)
  /* Mac OS X guarantees 16-byte alignment in small allocs */
  block = malloc(size);
#else
  #error Cannot determine how to allocate aligned memory on the target platform.
#endif

  if (block == NULL || res != 0) {
    fprintf(stderr, "Error allocating memory!");
    return NULL;
  }

  /* Clean the allocated memory before returning. */
  memset(block, clean_value, size);

  return block;
}

/** Frees memory allocated by blosc_test_malloc. */
static void blosc_test_free(void* ptr)
{
#if defined(_WIN32)
    _aligned_free(ptr);
#else
    free(ptr);
#endif  /* _WIN32 */
}

/** Fills a buffer with random values. */
static void blosc_test_fill_random(void* const ptr, const size_t size)
{
  size_t k;
  uint8_t* const byte_ptr = (uint8_t*)ptr;
  for (k = 0; k < size; k++) {
    byte_ptr[k] = rand();
  }
}

/*
  Argument parsing.
*/

/** Parse a `int32_t` value from a string, checking for overflow. */
static int blosc_test_parse_uint32_t(const char* const str, uint32_t* value)
{
  char* str_end;
  int32_t signed_value = strtol(str, &str_end, 10);
  if (signed_value < 0 || *str_end)
  {
    return 0;
  }
  else
  {
    *value = (uint32_t)signed_value;
    return 1;
  }
}

/*
  Error message functions.
*/

/** Print an error message when a test program has been invoked
    with an invalid number of arguments. */
static void blosc_test_print_bad_argcount_msg(
  const int32_t num_expected_args, const int32_t num_actual_args)
{
  fprintf(stderr, "Invalid number of arguments specified.\nExpected %d arguments but was given %d.",
    num_expected_args, num_actual_args);
}

/** Print an error message when a test program has been invoked
    with an invalid argument value. */
static void blosc_test_print_bad_arg_msg(const int32_t arg_index)
{
  fprintf(stderr, "Invalid value specified for argument at index %d.\n", arg_index);
}

#endif  /* !defined(BLOSC_TEST_COMMON_H) */
