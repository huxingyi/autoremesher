/*********************************************************************
  Blosc - Blocked Shuffling and Compression Library

  Unit tests for BLOSC_COMPRESSOR environment variable in Blosc.

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
size_t typesize = 8;
size_t size = 8 * 1000 * 1000;  /* must be divisible by typesize */


/* Check compressor */
static const char *test_compressor(void) {
  const char* compressor;

  /* Before any blosc_compress() the compressor must be blosclz */
  compressor = blosc_get_compressor();
  mu_assert("ERROR: get_compressor (compress, before) incorrect",
	    strcmp(compressor, "blosclz") == 0);

  /* Activate the BLOSC_COMPRESSOR variable */
  setenv("BLOSC_COMPRESSOR", "lz4", 0);

  /* Get a compressed buffer */
  cbytes = blosc_compress(clevel, doshuffle, typesize, size, src,
                          dest, size + BLOSC_MAX_OVERHEAD);
  mu_assert("ERROR: cbytes is not correct", cbytes < size);

  compressor = blosc_get_compressor();
  mu_assert("ERROR: get_compressor (compress, after) incorrect",
	    strcmp(compressor, "lz4") == 0);

  /* Reset envvar */
  unsetenv("BLOSC_COMPRESSOR");
  return 0;
}


/* Check compressing + decompressing */
static const char *test_compress_decompress(void) {
  const char* compressor;

  /* Activate the BLOSC_COMPRESSOR variable */
  setenv("BLOSC_COMPRESSOR", "lz4", 0);

  compressor = blosc_get_compressor();
  mu_assert("ERROR: get_compressor incorrect",
	    strcmp(compressor, "lz4") == 0);

  /* Get a compressed buffer */
  cbytes = blosc_compress(clevel, doshuffle, typesize, size, src,
                          dest, size + BLOSC_MAX_OVERHEAD);
  mu_assert("ERROR: cbytes is not correct", cbytes < size);

  compressor = blosc_get_compressor();
  mu_assert("ERROR: get_compressor incorrect",
	    strcmp(compressor, "lz4") == 0);

  /* Decompress the buffer */
  nbytes = blosc_decompress(dest, dest2, size);
  mu_assert("ERROR: nbytes incorrect(1)", nbytes == size);

  compressor = blosc_get_compressor();
  mu_assert("ERROR: get_compressor incorrect",
	    strcmp(compressor, "lz4") == 0);

  /* Reset envvar */
  unsetenv("BLOSC_COMPRESSOR");
  return 0;
}


/* Check compression level */
static const char *test_clevel(void) {
  int cbytes2;

  /* Get a compressed buffer */
  cbytes = blosc_compress(clevel, doshuffle, typesize, size, src,
                          dest, size + BLOSC_MAX_OVERHEAD);
  mu_assert("ERROR: cbytes is not correct", cbytes < size);

  /* Activate the BLOSC_CLEVEL variable */
  setenv("BLOSC_CLEVEL", "9", 0);
  cbytes2 = blosc_compress(clevel, doshuffle, typesize, size, src,
                           dest, size + BLOSC_MAX_OVERHEAD);
  mu_assert("ERROR: BLOSC_CLEVEL does not work correctly", cbytes2 < cbytes);

  /* Reset envvar */
  unsetenv("BLOSC_CLEVEL");
  return 0;
}

/* Check noshuffle */
static const char *test_noshuffle(void) {
  int cbytes2;

  /* Get a compressed buffer */
  cbytes = blosc_compress(clevel, doshuffle, typesize, size, src,
                          dest, size + BLOSC_MAX_OVERHEAD);
  mu_assert("ERROR: cbytes is not correct", cbytes < size);

  /* Activate the BLOSC_SHUFFLE variable */
  setenv("BLOSC_SHUFFLE", "NOSHUFFLE", 0);
  cbytes2 = blosc_compress(clevel, doshuffle, typesize, size, src,
                           dest, size + BLOSC_MAX_OVERHEAD);
  mu_assert("ERROR: BLOSC_SHUFFLE=NOSHUFFLE does not work correctly",
            cbytes2 > cbytes);

  /* Reset env var */
  unsetenv("BLOSC_SHUFFLE");
  return 0;
}


/* Check regular shuffle */
static const char *test_shuffle(void) {
  int cbytes2;

  /* Get a compressed buffer */
  cbytes = blosc_compress(clevel, doshuffle, typesize, size, src,
                          dest, size + BLOSC_MAX_OVERHEAD);
  mu_assert("ERROR: cbytes is not 0", cbytes < size);

  /* Activate the BLOSC_SHUFFLE variable */
  setenv("BLOSC_SHUFFLE", "SHUFFLE", 0);
  cbytes2 = blosc_compress(clevel, doshuffle, typesize, size, src,
                           dest, size + BLOSC_MAX_OVERHEAD);
  mu_assert("ERROR: BLOSC_SHUFFLE=SHUFFLE does not work correctly",
            cbytes2 == cbytes);

  /* Reset env var */
  unsetenv("BLOSC_SHUFFLE");
  return 0;
}

/* Check bitshuffle */
static const char *test_bitshuffle(void) {
  int cbytes2;

  /* Get a compressed buffer */
  blosc_set_compressor("blosclz");  /* avoid lz4 here for now (see #BLOSC_MAX_OVERHEAD8) */
  cbytes = blosc_compress(clevel, doshuffle, typesize, size, src,
                          dest, size + BLOSC_MAX_OVERHEAD);
  mu_assert("ERROR: cbytes is not 0", cbytes < size);

  /* Activate the BLOSC_BITSHUFFLE variable */
  setenv("BLOSC_SHUFFLE", "BITSHUFFLE", 0);
  cbytes2 = blosc_compress(clevel, doshuffle, typesize, size, src,
                           dest, size + BLOSC_MAX_OVERHEAD);
  mu_assert("ERROR: BLOSC_SHUFFLE=BITSHUFFLE does not work correctly",
            cbytes2 < cbytes * 1.5);

  /* Reset env var */
  unsetenv("BLOSC_SHUFFLE");
  return 0;
}


/* Check typesize */
static const char *test_typesize(void) {
  int cbytes2;

  /* Get a compressed buffer */
  cbytes = blosc_compress(clevel, doshuffle, typesize, size, src,
                          dest, size + BLOSC_MAX_OVERHEAD);
  mu_assert("ERROR: cbytes is not correct", cbytes < size);

  /* Activate the BLOSC_TYPESIZE variable */
  setenv("BLOSC_TYPESIZE", "9", 0);
  cbytes2 = blosc_compress(clevel, doshuffle, typesize, size, src,
                           dest, size + BLOSC_MAX_OVERHEAD);
  mu_assert("ERROR: BLOSC_TYPESIZE does not work correctly", cbytes2 > cbytes);

  /* Reset envvar */
  unsetenv("BLOSC_TYPESIZE");
  return 0;
}

/* Check splitmode */
static const char *test_splitmode() {
  int cbytes2;

  /* Get a compressed buffer */
  cbytes = blosc_compress(clevel, doshuffle, typesize, size, src,
                          dest, size + BLOSC_MAX_OVERHEAD);
  mu_assert("ERROR: cbytes is not correct", cbytes < size);

  /* Deactivate the split */
  blosc_set_splitmode(BLOSC_NEVER_SPLIT);
  cbytes2 = blosc_compress(clevel, doshuffle, typesize, size, src,
                           dest, size + BLOSC_MAX_OVERHEAD);
  mu_assert("ERROR: blosc_set_splitmode does not work correctly", cbytes2 > cbytes);
  /* Reset the splitmode */
  blosc_set_splitmode(BLOSC_FORWARD_COMPAT_SPLIT);

  return 0;
}

/* Check splitmode with an environment variable */
static const char *test_splitmode_envvar() {
  int cbytes2;

  /* Get a compressed buffer */
  cbytes = blosc_compress(clevel, doshuffle, typesize, size, src,
                          dest, size + BLOSC_MAX_OVERHEAD);
  mu_assert("ERROR: cbytes is not correct", cbytes < size);

  /* Deactivate the split */
  setenv("BLOSC_SPLITMODE", "NEVER", 0);
  cbytes2 = blosc_compress(clevel, doshuffle, typesize, size, src,
                           dest, size + BLOSC_MAX_OVERHEAD);
  mu_assert("ERROR: BLOSC_SPLITMODE envvar does not work correctly", cbytes2 > cbytes);

  return 0;
}

/* Check for compressing an empty buffer */
static const char *test_empty_buffer() {
  int cbytes1;
  int cbytes2;

  cbytes1 = blosc_compress(1, 1, 1, 0, src, dest, BLOSC_MAX_OVERHEAD);
  mu_assert("ERROR: cbytes is not correct", cbytes1 == BLOSC_MAX_OVERHEAD);

  cbytes2 = blosc_decompress(dest, src, 0);
  mu_assert("ERROR: decompressed bytes is not correct", cbytes2 == 0);

  return 0;
}

/* Check for compressing a very small buffer */
static const char *test_small_buffer() {
  int cbytes1;
  int cbytes2;
  size_t srclen;

  for (srclen = 1; srclen < BLOSC_MAX_OVERHEAD; srclen++) {
      cbytes1 = blosc_compress(1, 1, typesize, srclen, src, dest, srclen + BLOSC_MAX_OVERHEAD);
      mu_assert("ERROR: cbytes is not correct", cbytes1 == srclen + BLOSC_MAX_OVERHEAD);

      cbytes2 = blosc_decompress(dest, src, srclen);
      mu_assert("ERROR: decompressed bytes is not correct", cbytes2 == srclen);
  }
   return 0;
}

/* Check for decompressing into a buffer larger than necessary */
static const char *test_too_long_dest() {
  int cbytes1;
  int cbytes2;
  size_t srclen = 2;

  cbytes1 = blosc_compress(1, 1, typesize, srclen, src, dest, srclen + BLOSC_MAX_OVERHEAD);
  mu_assert("ERROR: cbytes is not correct", cbytes1 == srclen + BLOSC_MAX_OVERHEAD);

  cbytes2 = blosc_decompress(dest, src, srclen + 1021);
  mu_assert("ERROR: decompressed bytes is not correct", cbytes2 == srclen);
  return 0;
}

/* Check for decompressing into a buffer larger than necessary (v2) */
static const char *test_too_long_dest2() {
  int cbytes1;
  int cbytes2;
  size_t srclen = 3069;

  cbytes1 = blosc_compress(1, 1, typesize, srclen, src, dest, srclen + BLOSC_MAX_OVERHEAD);
  mu_assert("ERROR: cbytes is not correct", cbytes1 <= srclen + BLOSC_MAX_OVERHEAD);

  cbytes2 = blosc_decompress(dest, src, srclen + 1021);
  mu_assert("ERROR: decompressed bytes is not correct", cbytes2 == srclen);
  return 0;
}



static const char *all_tests(void) {
  mu_run_test(test_compressor);
  mu_run_test(test_compress_decompress);
  mu_run_test(test_clevel);
  mu_run_test(test_noshuffle);
  mu_run_test(test_shuffle);
  mu_run_test(test_bitshuffle);
  mu_run_test(test_typesize);
  mu_run_test(test_splitmode);
  mu_run_test(test_splitmode_envvar);
  mu_run_test(test_empty_buffer);
  mu_run_test(test_small_buffer);
  mu_run_test(test_too_long_dest);
  mu_run_test(test_too_long_dest2);

  return 0;
}

#define BUFFER_ALIGN_SIZE   32

int main(int argc, char **argv) {
  int64_t *_src;
  const char *result;
  size_t i;

  printf("STARTING TESTS for %s", argv[0]);

  blosc_init();
  blosc_set_compressor("blosclz");

  /* Initialize buffers */
  src = blosc_test_malloc(BUFFER_ALIGN_SIZE, size);
  srccpy = blosc_test_malloc(BUFFER_ALIGN_SIZE, size);
  dest = blosc_test_malloc(BUFFER_ALIGN_SIZE, size + BLOSC_MAX_OVERHEAD);
  dest2 = blosc_test_malloc(BUFFER_ALIGN_SIZE, size);
  _src = (int64_t *)src;
  for (i=0; i < (size / sizeof(int64_t)); i++) {
    _src[i] = (int64_t)i;
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
