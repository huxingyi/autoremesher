/*********************************************************************
  Blosc - Blocked Shuffling and Compression Library

  Unit tests for Blosc API.

  Creation date: 2010-06-07
  Author: Francesc Alted <francesc@blosc.org>

  See LICENSES/BLOSC.txt for details about copyright and rights to use.
**********************************************************************/

#include "test_common.h"

int tests_run = 0;

/* Global vars */
void *src, *srccpy, *dest, *dest2;
size_t nbytes, cbytes;
int clevel = 3;
int doshuffle = 1;
size_t typesize = 4;
size_t size = 1*MB;



static const char *test_cbuffer_sizes(void) {
  size_t nbytes_, cbytes_, blocksize;

  blosc_cbuffer_sizes(dest, &nbytes_, &cbytes_, &blocksize);
  mu_assert("ERROR: nbytes incorrect(1)", nbytes == size);
  mu_assert("ERROR: nbytes incorrect(2)", nbytes_ == nbytes);
  mu_assert("ERROR: cbytes incorrect", cbytes == cbytes_);
  mu_assert("ERROR: blocksize incorrect", blocksize >= 128);
  return 0;
}

static const char *test_get_complib_info(void) {
  char *complib, *version;
  int clibcode;

  clibcode = blosc_get_complib_info("blosclz", &complib, &version);
  mu_assert("ERROR: complib incorrect", strcmp(complib, "BloscLZ") == 0);
  mu_assert("ERROR: clibcode incorrect", clibcode == 0);
  free(complib);
  free(version);
  clibcode = blosc_get_complib_info("non-existing", &complib, &version);
  mu_assert("ERROR: complib should be NULL", complib == NULL);
  mu_assert("ERROR: clibcode incorrect", clibcode == -1);
  clibcode = blosc_get_complib_info("blosclz", NULL, NULL);
  mu_assert("ERROR: clibcode incorrect", clibcode == 0);
  clibcode = blosc_get_complib_info("blosclz", NULL, &version);
  mu_assert("ERROR: clibcode incorrect", clibcode == 0);
  clibcode = blosc_get_complib_info("blosclz", &complib, NULL);
  mu_assert("ERROR: complib incorrect", strcmp(complib, "BloscLZ") == 0);
  mu_assert("ERROR: clibcode incorrect", clibcode == 0);
  return 0;
}

static const char *test_cbuffer_metainfo(void) {
  size_t typesize_;
  int flags;

  blosc_cbuffer_metainfo(dest, &typesize_, &flags);
  mu_assert("ERROR: typesize incorrect", typesize_ == typesize);
  mu_assert("ERROR: shuffle incorrect", (flags & BLOSC_DOSHUFFLE) == doshuffle);
  return 0;
}


static const char *test_cbuffer_versions(void) {
  int version_;
  int versionlz_;

  blosc_cbuffer_versions(dest, &version_, &versionlz_);
  mu_assert("ERROR: version incorrect", version_ == BLOSC_VERSION_FORMAT);
  mu_assert("ERROR: versionlz incorrect", versionlz_ == BLOSC_BLOSCLZ_VERSION_FORMAT);
  return 0;
}


static const char *test_cbuffer_complib(void) {
  const char *complib;

  complib = blosc_cbuffer_complib(dest);
  mu_assert("ERROR: complib incorrect", strcmp(complib, "BloscLZ") == 0);
  return 0;
}


static const char *test_nthreads(void) {
  int nthreads;

  nthreads = blosc_set_nthreads(4);
  mu_assert("ERROR: set_nthreads incorrect", nthreads == 1);
  nthreads = blosc_get_nthreads();
  mu_assert("ERROR: get_nthreads incorrect", nthreads == 4);
  return 0;
}

static const char *test_blocksize(void) {
  int blocksize;

  blocksize = blosc_get_blocksize();
  mu_assert("ERROR: get_blocksize incorrect", blocksize == 0);

  blosc_set_blocksize(4096);
  blocksize = blosc_get_blocksize();
  mu_assert("ERROR: get_blocksize incorrect", blocksize == 4096);
  return 0;
}

static char *test_set_splitmode() {
  blosc_set_splitmode(BLOSC_AUTO_SPLIT);
  return 0;
}


static const char *all_tests(void) {
  mu_run_test(test_cbuffer_sizes);
  mu_run_test(test_get_complib_info);
  mu_run_test(test_cbuffer_metainfo);
  mu_run_test(test_cbuffer_versions);
  mu_run_test(test_cbuffer_complib);
  mu_run_test(test_nthreads);
  mu_run_test(test_blocksize);
  mu_run_test(test_set_splitmode);
  return 0;
}

#define BUFFER_ALIGN_SIZE   8

int main(int argc, char **argv) {
  const char *result;

  printf("STARTING TESTS for %s", argv[0]);

  blosc_init();
  blosc_set_nthreads(1);

  /* Initialize buffers */
  src = blosc_test_malloc(BUFFER_ALIGN_SIZE, size);
  srccpy = blosc_test_malloc(BUFFER_ALIGN_SIZE, size);
  dest = blosc_test_malloc(BUFFER_ALIGN_SIZE, size);
  dest2 = blosc_test_malloc(BUFFER_ALIGN_SIZE, size);
  memset(src, 0, size);
  memcpy(srccpy, src, size);

  /* Get a compressed buffer */
  cbytes = blosc_compress(clevel, doshuffle, typesize, size, src, dest, size);

  /* Get a decompressed buffer */
  nbytes = blosc_decompress(dest, dest2, size);

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

