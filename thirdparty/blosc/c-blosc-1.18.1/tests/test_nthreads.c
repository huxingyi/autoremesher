/*********************************************************************
  Blosc - Blocked Shuffling and Compression Library

  Unit tests for BLOSC_NTHREADS environment variable in Blosc.

  Creation date: 2016-04-25
  Author: Francesc Alted <francesc@blosc.org>

  See LICENSES/BLOSC.txt for details about copyright and rights to use.
**********************************************************************/

#include "test_common.h"

int tests_run = 0;

/* Global vars */
void *src, *srccpy, *dest, *dest2;
int nbytes, cbytes;
int clevel = 1;
int doshuffle = 1;
size_t typesize = 4;
size_t size = 4 * 1000 * 1000;             /* must be divisible by 4 */


/* Check just compressing */
static const char *test_compress(void) {
  int nthreads;

  /* Before any blosc_compress() or blosc_decompress() the number of
     threads must be 1 */
  nthreads = blosc_get_nthreads();
  mu_assert("ERROR: get_nthreads (compress, before) incorrect", nthreads == 1);

  /* Get a compressed buffer */
  cbytes = blosc_compress(clevel, doshuffle, typesize, size, src,
                          dest, size + BLOSC_MAX_OVERHEAD);
  mu_assert("ERROR: cbytes is not correct", cbytes < size);

  nthreads = blosc_get_nthreads();
  mu_assert("ERROR: get_nthreads (compress, after) incorrect", nthreads == 3);

  return 0;
}


/* Check compressing + decompressing */
static const char *test_compress_decompress(void) {
  int nthreads;

  nthreads = blosc_get_nthreads();
  mu_assert("ERROR: get_nthreads incorrect", nthreads == 3);

  /* Get a compressed buffer */
  cbytes = blosc_compress(clevel, doshuffle, typesize, size, src,
                          dest, size + BLOSC_MAX_OVERHEAD);
  mu_assert("ERROR: cbytes is not correct", cbytes < size);

  nthreads = blosc_get_nthreads();
  mu_assert("ERROR: get_nthreads incorrect", nthreads == 3);

  /* Decompress the buffer */
  nbytes = blosc_decompress(dest, dest2, size);
  mu_assert("ERROR: nbytes incorrect(1)", nbytes == size);

  nthreads = blosc_get_nthreads();
  mu_assert("ERROR: get_nthreads incorrect", nthreads == 3);

  return 0;
}


static const char *all_tests(void) {
  mu_run_test(test_compress);
  mu_run_test(test_compress_decompress);

  return 0;
}

#define BUFFER_ALIGN_SIZE   32

int main(int argc, char **argv) {
  int32_t *_src;
  const char *result;
  size_t i;

  printf("STARTING TESTS for %s", argv[0]);

  /* Activate the BLOSC_NTHREADS variable */
  setenv("BLOSC_NTHREADS", "3", 1);

  blosc_init();
  blosc_set_nthreads(1);

  /* Initialize buffers */
  src = blosc_test_malloc(BUFFER_ALIGN_SIZE, size);
  srccpy = blosc_test_malloc(BUFFER_ALIGN_SIZE, size);
  dest = blosc_test_malloc(BUFFER_ALIGN_SIZE, size + BLOSC_MAX_OVERHEAD);
  dest2 = blosc_test_malloc(BUFFER_ALIGN_SIZE, size);
  _src = (int32_t *)src;
  for (i=0; i < (size/4); i++) {
    _src[i] = (int32_t)i;
  }
  memcpy(srccpy, src, size);

  /* Run all the suite */
  result = all_tests();
  if (result != 0) {
    printf(" (%s)\n", result);
  }
  else {
    printf(" ALL TESTS PASSED");
  }
  printf("\tTests run: %d\n", tests_run);

  blosc_test_free(src);
  blosc_test_free(srccpy);
  blosc_test_free(dest);
  blosc_test_free(dest2);

  blosc_destroy();

  return result != 0;
}
