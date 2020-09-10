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

  /* Initialize the Blosc compressor */
  blosc_init();

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

  /* After using it, destroy the Blosc environment */
  blosc_destroy();

  for(i=0;i<SIZE;i++){
    if(data[i] != data_dest[i]) {
      printf("Decompressed data differs from original!\n");
      return -1;
    }
  }

  printf("Succesful roundtrip!\n");
  return 0;
}
