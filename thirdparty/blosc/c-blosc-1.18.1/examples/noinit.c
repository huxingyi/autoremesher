/*
    Copyright (C) 2016  Francesc Alted
    http://blosc.org
    License: MIT (see LICENSE.txt)

    Example program demonstrating that from 1.9.0 on, Blosc does not
    need to be initialized (although it is recommended).

    To compile this program:

    $ gcc noinit.c -o noinit -lblosc

    or, if you don't have the blosc library installed yet:

    $ gcc -O3 -msse2 noinit.c -I../blosc -o noinit -L../build/blosc
    $ export LD_LIBRARY_PATH=../build/blosc

    Using MSVC on Windows:

    $ cl /arch:SSE2 /Ox /Fenoinit.exe /Iblosc examples\noinit.c blosc\blosc.c blosc\blosclz.c blosc\shuffle.c blosc\shuffle-sse2.c blosc\shuffle-generic.c blosc\bitshuffle-generic.c blosc\bitshuffle-sse2.c

    To run:

    $ ./noinit
    Blosc version info: 1.8.2.dev ($Date:: 2016-04-08 #$)
    Compression: 4000000 -> 158788 (25.2x)
    Decompression succesful!
    Succesful roundtrip!

*/

#include <stdio.h>
#include <blosc.h>

#define SIZE 100*100*100

int main(){
  static float data[SIZE];
  static float data_out[SIZE];
  static float data_dest[SIZE];
  int isize = SIZE*sizeof(float), osize = SIZE*sizeof(float);
  int dsize = SIZE*sizeof(float), csize;
  int i;

  for(i=0; i<SIZE; i++){
    data[i] = i;
  }

  /* Register the filter with the library */
  printf("Blosc version info: %s (%s)\n",
	 BLOSC_VERSION_STRING, BLOSC_VERSION_DATE);

  /* From 1.9 on, we don't need to initialize the Blosc compressor anymore */
  /* blosc_init(); */

  /* Compress with clevel=5 and shuffle active  */
  csize = blosc_compress(5, 1, sizeof(float), isize, data, data_out, osize);
  if (csize == 0) {
    printf("Buffer is uncompressible.  Giving up.\n");
    return 1;
  }
  else if (csize < 0) {
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

  printf("Decompression succesful!\n");

  for(i=0;i<SIZE;i++){
    if(data[i] != data_dest[i]) {
      printf("Decompressed data differs from original!\n");
      return -1;
    }
  }

  printf("Succesful roundtrip!\n");
  return 0;
}
