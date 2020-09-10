/*********************************************************************
  Blosc - Blocked Shuffling and Compression Library

  Unit tests for the blosc_getitem() function.

  Creation date: 2010-06-07
  Author: Francesc Alted <francesc@blosc.org>

  See LICENSES/BLOSC.txt for details about copyright and rights to use.
**********************************************************************/

#include "test_common.h"


/** Test the blosc_getitem function. */
static int test_getitem(size_t type_size, size_t num_elements,
  size_t buffer_alignment, int compression_level, int do_shuffle)
{
  size_t buffer_size = type_size * num_elements;
  int exit_code;

  /* Allocate memory for the test. */
  void* original = blosc_test_malloc(buffer_alignment, buffer_size);
  void* intermediate = blosc_test_malloc(buffer_alignment, buffer_size + BLOSC_MAX_OVERHEAD);
  void* result = blosc_test_malloc(buffer_alignment, buffer_size);

  /* Fill the input data buffer with random values. */
  blosc_test_fill_random(original, buffer_size);

  /* Compress the input data, then use blosc_getitem to extract (decompress)
     a range of elements into a new buffer. */
  blosc_compress(compression_level, do_shuffle, type_size, buffer_size,
    original, intermediate, buffer_size + BLOSC_MAX_OVERHEAD);
  blosc_getitem(intermediate, 0, num_elements, result);

  /* The round-tripped data matches the original data when the
     result of memcmp is 0. */
  exit_code = memcmp(original, result, buffer_size) ?
    EXIT_FAILURE : EXIT_SUCCESS;

  /* Free allocated memory. */
  blosc_test_free(original);
  blosc_test_free(intermediate);
  blosc_test_free(result);

  return exit_code;
}

/** Required number of arguments to this test, including the executable name. */
#define TEST_ARG_COUNT  7

int main(int argc, char **argv)
{
  uint32_t type_size;
  uint32_t num_elements;
  uint32_t buffer_align_size;
  uint32_t compression_level;
  uint32_t shuffle_enabled;
  uint32_t blosc_thread_count;
  int result;

  /*  argv[1]: sizeof(element type)
      argv[2]: number of elements
      argv[3]: buffer alignment
      argv[4]: compression level
      argv[5]: shuffle enabled
      argv[6]: thread count
  */

  /*  Verify the correct number of command-line args have been specified. */
  if (TEST_ARG_COUNT != argc)
  {
    blosc_test_print_bad_argcount_msg(TEST_ARG_COUNT, argc);
    return EXIT_FAILURE;
  }

  /* Parse arguments */
  if (!blosc_test_parse_uint32_t(argv[1], &type_size) || (type_size < 1))
  {
    blosc_test_print_bad_arg_msg(1);
    return EXIT_FAILURE;
  }

  if (!blosc_test_parse_uint32_t(argv[2], &num_elements) || (num_elements < 1))
  {
    blosc_test_print_bad_arg_msg(2);
    return EXIT_FAILURE;
  }

  if (!blosc_test_parse_uint32_t(argv[3], &buffer_align_size)
    || (buffer_align_size & (buffer_align_size - 1))
    || (buffer_align_size < sizeof(void*)))
  {
    blosc_test_print_bad_arg_msg(3);
    return EXIT_FAILURE;
  }

  if (!blosc_test_parse_uint32_t(argv[4], &compression_level) || (compression_level > 9))
  {
    blosc_test_print_bad_arg_msg(4);
    return EXIT_FAILURE;
  }

  {
    if (!blosc_test_parse_uint32_t(argv[5], &shuffle_enabled) || (shuffle_enabled > 2))
    {
      blosc_test_print_bad_arg_msg(5);
      return EXIT_FAILURE;
    }
  }

  if (!blosc_test_parse_uint32_t(argv[6], &blosc_thread_count) || (blosc_thread_count < 1))
  {
    blosc_test_print_bad_arg_msg(6);
    return EXIT_FAILURE;
  }

  /* Initialize blosc before running tests. */
  blosc_init();
  blosc_set_nthreads(blosc_thread_count);

  /* Run the test. */
  result = test_getitem(type_size, num_elements, buffer_align_size,
    compression_level, shuffle_enabled);

  /* Cleanup blosc resources. */
  blosc_destroy();

  return result;
}
