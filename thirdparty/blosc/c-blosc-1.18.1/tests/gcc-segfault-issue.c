/*
    Copyright (C) 2016  Francesc Alted
    http://blosc.org
    License: MIT (see LICENSE.txt)

    Test program trying to replicate the python-blosc issue:

    https://github.com/Blosc/python-blosc/issues/110

    Apparently this only affects to blosc-powered Python extensions.

    To compile this program:

    $ gcc -O3 gcc-segfault-issue.c -o gcc-segfault-issue -lblosc

    To run:

    $ ./gcc-segfault-issue
    Blosc version info: 1.8.1.dev ($Date:: 2016-03-31 #$)
    Compression: 8000000 -> 73262 (109.2x)

    To check that everything goes well:

    $ time for i in {1..1000}; do ./gcc-segfault-issue > p ; done

    real    0m4.590s
    user    0m2.516s
    sys     0m1.884s

    If you don't see any "Segmentation fault (core dumped)", the
    C-Blosc library itself is probably not a victim of the infamous
    issue above that only seems to affect Python extensions.

*/

#include <stdio.h>
#include <blosc.h>

#define SIZE 1000*1000

int main(){
  static double data[SIZE];
  static double data_out[SIZE];
  static double data_dest[SIZE];
  int isize = SIZE*sizeof(double), osize = SIZE*sizeof(double);
  int dsize = SIZE*sizeof(double), csize;
  int i;

  for(i=0; i<SIZE; i++){
    data[i] = i;
  }

  /* Register the filter with the library */
  printf("Blosc version info: %s (%s)\n",
	 BLOSC_VERSION_STRING, BLOSC_VERSION_DATE);

  /* Initialize the gobal Blosc context */
  blosc_init();

  /* Use multithreading */
  blosc_set_nthreads(3);

  /* Compress with clevel=9 and shuffle active */
  csize = blosc_compress(9, 1, sizeof(double), isize, data, data_out, osize);
  if (csize == 0) {
    printf("Buffer is uncompressible.  Giving up.\n");
    return 1;
  }
  else if (csize < 0) {
    printf("Compression error.  Error code: %d\n", csize);
    return csize;
  }

  printf("Compression: %d -> %d (%.1fx)\n", isize, csize, (1.*isize) / csize);

  /* Destroy the global Blosc context */
  blosc_destroy();

  return 0;
}
