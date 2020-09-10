/*
    Example program demonstrating use of the Blosc filter from C code.

    To compile this program:

    $ gcc many_compressors.c -o many_compressors -lblosc -lpthread

    or, if you don't have the blosc library installed:

    $ gcc -O3 -msse2 many_compressors.c ../blosc/*.c -I../blosc \
        -o many_compressors -lpthread \
        -DHAVE_ZLIB -lz -DHAVE_LZ4 -llz4 -DHAVE_SNAPPY -lsnappy

    Using MSVC on Windows:

    $ cl /Ox /Femany_compressors.exe /Iblosc many_compressors.c blosc\*.c

    To run:

    $ ./many_compressors
    Blosc version info: 1.10.0.dev ($Date:: 2016-07-20 #$)
    Using 4 threads (previously using 1)
    Using blosclz compressor
    Compression: 4000000 -> 158788 (25.2x)
    Succesful roundtrip!
    Using lz4 compressor
    Compression: 4000000 -> 235419 (17.0x)
    Succesful roundtrip!
    Using lz4hc compressor
    Compression: 4000000 -> 38314 (104.4x)
    Succesful roundtrip!
    Using snappy compressor
    Compression: 4000000 -> 311617 (12.8x)
    Succesful roundtrip!
    Using zlib compressor
    Compression: 4000000 -> 22103 (181.0x)
    Succesful roundtrip!
    Using zstd compressor
    Compression: 4000000 -> 11813 (338.6x)
    Succesful roundtrip!

*/

#include <stdio.h>
#include <blosc.h>

#define SIZE 100*100*100
#define SHAPE {100,100,100}
#define CHUNKSHAPE {1,100,100}

int main(){
  static float data[SIZE];
  static float data_out[SIZE];
  static float data_dest[SIZE];
  int isize = SIZE*sizeof(float), osize = SIZE*sizeof(float);
  int dsize = SIZE*sizeof(float), csize;
  int nthreads, pnthreads, i;
  char* compressors[] = {"blosclz", "lz4", "lz4hc", "snappy", "zlib", "zstd"};
  int ccode, rcode;

  for(i=0; i<SIZE; i++){
    data[i] = i;
  }

  /* Register the filter with the library */
  printf("Blosc version info: %s (%s)\n",
	 BLOSC_VERSION_STRING, BLOSC_VERSION_DATE);

  /* Initialize the Blosc compressor */
  blosc_init();

  nthreads = 4;
  pnthreads = blosc_set_nthreads(nthreads);
  printf("Using %d threads (previously using %d)\n", nthreads, pnthreads);

  /* Tell Blosc to use some number of threads */
  for (ccode=0; ccode <= 5; ccode++) {

    rcode = blosc_set_compressor(compressors[ccode]);
    if (rcode < 0) {
      printf("Error setting %s compressor.  It really exists?",
	     compressors[ccode]);
      return rcode;
    }
    printf("Using %s compressor\n", compressors[ccode]);

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

    /* After using it, destroy the Blosc environment */
    blosc_destroy();

    for(i=0;i<SIZE;i++){
      if(data[i] != data_dest[i]) {
	printf("Decompressed data differs from original!\n");
	return -1;
      }
    }

    printf("Succesful roundtrip!\n");
  }

  return 0;
}
