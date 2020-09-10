/*
    Copyright (C) 2014  Francesc Alted
    http://blosc.org
    License: MIT (see LICENSE.txt)

    Example program demonstrating use of the Blosc filter from C code.

    To compile this program using gcc or clang:

    gcc/clang multithread.c -o multithread -lblosc -lpthread

    or, if you don't have the blosc library installed:

    gcc -O3 -msse2 multithread.c ../blosc/!(*avx2*)*.c  -I../blosc -o multithread -lpthread

    Using MSVC on Windows:

    cl /Ox /Femultithread.exe /Iblosc multithread.c blosc\*.c

    To run:

    $ ./multithread
    Blosc version info: 1.4.2.dev ($Date:: 2014-07-08 #$)
    Using 1 threads (previously using 1)
    Compression: 4000000 -> 158494 (25.2x)
    Succesful roundtrip!
    Using 2 threads (previously using 1)
    Compression: 4000000 -> 158494 (25.2x)
    Succesful roundtrip!
    Using 3 threads (previously using 2)
    Compression: 4000000 -> 158494 (25.2x)
    Succesful roundtrip!
    Using 4 threads (previously using 3)
    Compression: 4000000 -> 158494 (25.2x)
    Succesful roundtrip!

*/

#include <stdio.h>
#include <blosc.h>

#define SIZE 1000*1000


int main(){
  static float data[SIZE];
  static float data_out[SIZE];
  static float data_dest[SIZE];
  int isize = SIZE*sizeof(float), osize = SIZE*sizeof(float);
  int dsize = SIZE*sizeof(float), csize;
  int nthreads, pnthreads, i;

  for(i=0; i<SIZE; i++){
    data[i] = i;
  }

  /* Register the filter with the library */
  printf("Blosc version info: %s (%s)\n",
         BLOSC_VERSION_STRING, BLOSC_VERSION_DATE);

  /* Initialize the Blosc compressor */
  blosc_init();

  /* Tell Blosc to use some number of threads */
  for (nthreads=1; nthreads <= 4; nthreads++) {

    pnthreads = blosc_set_nthreads(nthreads);
    printf("Using %d threads (previously using %d)\n", nthreads, pnthreads);

    /* Compress with clevel=5 and shuffle active  */
    csize = blosc_compress(5, 1, sizeof(float), isize, data, data_out, osize);
    if (csize < 0) {
      printf("Compression error.  Error code: %d\n", csize);
      return csize;
    }

    printf("Compression: %d -> %d (%.1fx)\n", isize, csize, (1.*isize) / csize);

    /* Decompress  */
    dsize = blosc_decompress(data_out, data_dest, dsize);
    if (dsize < 0) {
        printf("Decompression error.  Error code: %d\n", dsize);
        return dsize;
    }

    for(i=0;i<SIZE;i++){
      if(data[i] != data_dest[i]) {
        printf("Decompressed data differs from original!\n");
        return -1;
      }
    }

    printf("Succesful roundtrip!\n");
  }

  /* After using it, destroy the Blosc environment */
  blosc_destroy();

  return 0;
}
