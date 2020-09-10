/*********************************************************************
  Blosc - Blocked Shuffling and Compression Library

  Roundtrip tests

  Creation date: 2010-06-07
  Author: Francesc Alted <francesc@blosc.org>

  See LICENSES/BLOSC.txt for details about copyright and rights to use.
**********************************************************************/

#include "test_common.h"
#include "../blosc/shuffle.h"
#include "../blosc/shuffle-generic.h"


/** Roundtrip tests for the generic shuffle/unshuffle. */
static int test_shuffle_roundtrip_generic(size_t type_size, size_t num_elements,
  size_t buffer_alignment)
{
  size_t buffer_size = type_size * num_elements;
  int exit_code;

  /* Allocate memory for the test. */
  void* original = blosc_test_malloc(buffer_alignment, buffer_size);
  void* shuffled = blosc_test_malloc(buffer_alignment, buffer_size);
  void* unshuffled = blosc_test_malloc(buffer_alignment, buffer_size);

  /* Fill the input data buffer with random values. */
  blosc_test_fill_random(original, buffer_size);

  /* Generic shuffle, then generic unshuffle. */
  blosc_internal_shuffle_generic(type_size, buffer_size, original, shuffled);
  blosc_internal_unshuffle_generic(type_size, buffer_size, shuffled, unshuffled);

  /* The round-tripped data matches the original data when the
     result of memcmp is 0. */
  exit_code = memcmp(original, unshuffled, buffer_size) ?
    EXIT_FAILURE : EXIT_SUCCESS;

  /* Free allocated memory. */
  blosc_test_free(original);
  blosc_test_free(shuffled);
  blosc_test_free(unshuffled);

  return exit_code;
}

/** Required number of arguments to this test, including the executable name. */
#define TEST_ARG_COUNT  4

int main(int argc, char **argv)
{
  uint32_t type_size;
  uint32_t num_elements;
  uint32_t buffer_align_size;

  /*  argv[1]: sizeof(element type)
      argv[2]: number of elements
      argv[3]: buffer alignment
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

  /* Run the test. */
  return test_shuffle_roundtrip_generic(type_size, num_elements, buffer_align_size);
}
