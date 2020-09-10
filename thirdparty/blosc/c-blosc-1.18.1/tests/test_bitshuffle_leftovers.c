/*********************************************************************
  Blosc - Blocked Shuffling and Compression Library

  Unit test for the bitshuffle with blocks that are not aligned.
  See https://github.com/Blosc/python-blosc/issues/220
  Probably related: https://github.com/Blosc/c-blosc/issues/240

  Creation date: 2020-02-18
  Author: Francesc Alted <francesc@blosc.org>

  See LICENSES/BLOSC.txt for details about copyright and rights to use.
 **********************************************************************/

#include "test_common.h"


static int test_roundtrip_bitshuffle8(int size, void *data, void *data_out, void *data_dest) {
  /* Compress with bitshuffle active  */
  int isize = size;
  int osize = size + BLOSC_MIN_HEADER_LENGTH;
  int csize = blosc_compress(9, BLOSC_BITSHUFFLE, 8, isize, data, data_out, osize);
  int dsize;
  int exit_code;
  FILE *fout = fopen("test-bitshuffle8-nomemcpy.cdata", "w");

  if (csize == 0) {
    printf("Buffer is uncompressible.  Giving up.\n");
    return 1;
  }
  else if (csize < 0) {
    printf("Compression error.  Error code: %d\n", csize);
    return csize;
  }
  printf("Compression: %d -> %d (%.1fx)\n", isize, csize, (1.*isize) / csize);

  fwrite(data_out, csize, 1, fout);
  fclose(fout);

  /* Decompress  */
  dsize = blosc_decompress(data_out, data_dest, isize);
  if (dsize < 0) {
    printf("Decompression error.  Error code: %d\n", dsize);
    return dsize;
  }

  printf("Decompression succesful!\n");

  exit_code = memcmp(data, data_dest, size) ? EXIT_FAILURE : EXIT_SUCCESS;

  if (exit_code == EXIT_SUCCESS)
    printf("Succesful roundtrip!\n");
  else
    printf("Decompressed data differs from original!\n");

  return exit_code;
}

static int test_roundtrip_bitshuffle4(int size, void *data, void *data_out, void *data_dest) {
  /* Compress with bitshuffle active  */
  int isize = size;
  int osize = size + BLOSC_MIN_HEADER_LENGTH;
  int csize = blosc_compress(9, BLOSC_BITSHUFFLE, 4, isize, data, data_out, osize);
  int dsize;
  int exit_code;
  FILE *fout = fopen("test-bitshuffle4-memcpy.cdata", "w");

  if (csize == 0) {
    printf("Buffer is uncompressible.  Giving up.\n");
    return 1;
  }
  else if (csize < 0) {
    printf("Compression error.  Error code: %d\n", csize);
    return csize;
  }
  printf("Compression: %d -> %d (%.1fx)\n", isize, csize, (1.*isize) / csize);

  fwrite(data_out, csize, 1, fout);
  fclose(fout);

  /* Decompress  */
  dsize = blosc_decompress(data_out, data_dest, isize);
  if (dsize < 0) {
    printf("Decompression error.  Error code: %d\n", dsize);
    return dsize;
  }

  printf("Decompression succesful!\n");

  exit_code = memcmp(data, data_dest, size) ? EXIT_FAILURE : EXIT_SUCCESS;
  if (exit_code == EXIT_SUCCESS)
    printf("Succesful roundtrip!\n");
  else
    printf("Decompressed data differs from original!\n");

  return exit_code;
}

int main() {
  /* `size` below is chosen so that it is not divisible by 8
   * (not supported by bitshuffle) and in addition, it is not
   * divisible by 8 (typesize) again.
   */
  int size = 641091;
  int32_t *data = malloc(size);
  int32_t *data_out = malloc(size + BLOSC_MIN_HEADER_LENGTH);
  int32_t *data_dest = malloc(size);
  int result;
  int i;

  /* Initialize data */
  for (i = 0; i < size / sizeof(int32_t); i++) {
    ((uint32_t*)data)[i] = i;
  }
  /* leftovers */
  for (i = size / sizeof(int32_t) * sizeof(int32_t); i < size; i++) {
    ((uint8_t*)data)[i] = i;
  }

  blosc_init();
  blosc_set_nthreads(1);
  blosc_set_compressor("lz4");
  printf("Blosc version info: %s (%s)\n", BLOSC_VERSION_STRING, BLOSC_VERSION_DATE);
  result = test_roundtrip_bitshuffle4(size, data, data_out, data_dest);
  if (result != EXIT_SUCCESS) {
    goto fail;
  }
  result = test_roundtrip_bitshuffle8(size, data, data_out, data_dest);
  if (result != EXIT_SUCCESS) {
    goto fail;
  }

  free(data);
  free(data_out);
  free(data_dest);

  blosc_destroy();

  fail:
  return result;
}
