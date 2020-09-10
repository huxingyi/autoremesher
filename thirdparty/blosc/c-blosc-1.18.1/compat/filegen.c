/*********************************************************************
  Blosc - Blocked Shuffling and Compression Library

  Generator data file for Blosc forward and backward tests.

  Creation date: 2018-02-16
  Author: Elvis Stansvik, Francesc Alted <francesc@blosc.org>

  See LICENSES/BLOSC.txt for details about copyright and rights to use.
**********************************************************************/

#include <stdio.h>
#include <blosc.h>
#include <string.h>

#if defined(_WIN32) && !defined(__MINGW32__)
  #include <windows.h>
  /* stdint.h only available in VS2010 (VC++ 16.0) and newer */
  #if defined(_MSC_VER) && _MSC_VER < 1600
    #include "win32/stdint-windows.h"
  #else
    #include <stdint.h>
  #endif
#else
  #include <stdint.h>
#endif  /* _WIN32 */

#ifdef __HAIKU__
/* int32_t declared here */
#include <stdint.h>
#endif

#define SIZE (1000 * 1000)


int main(int argc, char *argv[]) {
  static int32_t data[SIZE];
  static int32_t data_out[SIZE];
  static int32_t data_dest[SIZE];
  size_t isize = SIZE * sizeof(int32_t);
  size_t osize = SIZE * sizeof(int32_t);
  int dsize = SIZE * sizeof(int32_t);
  int csize;
  long fsize;
  int i;

  FILE *f;

  /* Register the filter with the library */
  printf("Blosc version info: %s (%s)\n", BLOSC_VERSION_STRING, BLOSC_VERSION_DATE);

  /* Initialize the Blosc compressor */
  blosc_init();

  /* Use the argv[2] compressor. The supported ones are "blosclz",
  "lz4", "lz4hc", "snappy", "zlib" and "zstd"*/
  blosc_set_compressor(argv[2]);

  if (strcmp(argv[1], "compress") == 0) {

    for (i = 0; i < SIZE; i++) {
      data[i] = i;
    }

    /* Compress with clevel=9 and shuffle active  */
    csize = blosc_compress(9, 1, sizeof(int32_t), isize, data, data_out, osize);
    if (csize == 0) {
      printf("Buffer is uncompressible.  Giving up.\n");
      return 1;
    } else if (csize < 0) {
      printf("Compression error.  Error code: %d\n", csize);
      return csize;
    }

    printf("Compression: %d -> %d (%.1fx)\n", (int) isize, csize, (1. * isize) / csize);

    /* Write data_out to argv[3] */
    f = fopen(argv[3], "wb+");
    if (fwrite(data_out, 1, (size_t) csize, f) == csize) {
      printf("Wrote %s\n", argv[3]);
    } else {
      printf("Write failed");
    }
  } else {
    /* Read from argv[2] into data_out. */
    f = fopen(argv[2], "rb");
    fseek(f, 0, SEEK_END);
    fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (fread(data_out, 1, (size_t) fsize, f) == fsize) {
      printf("Checking %s\n", argv[2]);
    } else {
      printf("Read failed");
    }

    /* Decompress */
    dsize = blosc_decompress(data_out, data_dest, (size_t) dsize);
    if (dsize < 0) {
      printf("Decompression error.  Error code: %d\n", dsize);
      return dsize;
    }

    printf("Decompression succesful!\n");
  }

  /* After using it, destroy the Blosc environment */
  blosc_destroy();

  return 0;
}
