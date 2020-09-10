/*
    Copyright (C) 2015  Francesc Alted
    http://blosc.org
    License: MIT (see LICENSE.txt)

    Example program demonstrating use of the Blosc filter using the Windows Run-Time Dynamic Linking technique:
    
    https://msdn.microsoft.com/en-us/library/windows/desktop/ms686944(v=vs.85).aspx
    
    This allows to link your app in run-time with DLLs made with different compatible compilers
    (e.g. VS2013 and mingw-w64).

    To compile this program (be aware that you should match your compiler 32-bit/64-bit with your DLL):

    cl /Ox /Fewin-dynamic-linking.exe /I..\blosc win-dynamic-linking.c

    To run:

    $ win-dynamic-linking.exe
    Blosc version info: 1.7.0.dev
    Compression: 400000000 -> 19928862 (20.1x)
    Decompression succesful!
    Succesful roundtrip!

*/

#include <stdio.h>
#include <blosc.h>
#include <windows.h>

#define SIZE 100*1000*1000
#define SHAPE {100,1000,1000}
#define CHUNKSHAPE {1,1000,1000}

/* Definition for the compression and decompression blosc routines */
typedef int (__cdecl *COMPRESS_CTX)(int clevel, int doshuffle, size_t typesize,
                                        size_t nbytes, const void* src, void* dest,
                                        size_t destsize, const char* compressor,
                                        size_t blocksize, int numinternalthreads);

typedef int (__cdecl *DECOMPRESS_CTX)(const void *src, void *dest,
                                          size_t destsize, int numinternalthreads);
typedef char* (__cdecl *GET_VERSION_STRING)(void);


int main(){
  HINSTANCE BDLL;                       /* Handle to DLL */
  COMPRESS_CTX blosc_compress_ctx;      /* Function pointer for compression */
  DECOMPRESS_CTX blosc_decompress_ctx;  /* Function pointer for decompression */
  GET_VERSION_STRING blosc_get_version_string;

  static float data[SIZE];
  static float data_out[SIZE];
  static float data_dest[SIZE];
  int isize = SIZE*sizeof(float), osize = SIZE*sizeof(float);
  int dsize = SIZE*sizeof(float), csize;
  int i;

  BDLL = LoadLibrary(TEXT("myblosc.dll"));
  if (BDLL == NULL) {
    printf("Cannot find myblosc.dll library!\n");
    goto out;
  }

  blosc_compress_ctx = (COMPRESS_CTX)GetProcAddress(BDLL, "blosc_compress_ctx");
  if (!blosc_compress_ctx) {
    // handle the error
    printf("Cannot find blosc_compress_ctx() function!\n");
    goto out;
  }

  blosc_decompress_ctx = (DECOMPRESS_CTX)GetProcAddress(BDLL, "blosc_decompress_ctx");
  if (!blosc_decompress_ctx) {
    // handle the error
    printf("Cannot find blosc_decompress_ctx() function!\n");
    goto out;
  }

  blosc_get_version_string = (GET_VERSION_STRING)GetProcAddress(BDLL, "blosc_get_version_string");
  if (!blosc_get_version_string) {
    // handle the error
    printf("Cannot find blosc_get_version_string() function!\n");
    goto out;
  }

  for(i=0; i<SIZE; i++){
    data[i] = i;
  }

  /* Register the filter with the library */
  printf("Blosc version info: %s\n", blosc_get_version_string());

  /* Compress with clevel=3, shuffle active, 16-bytes data size, blosclz and 2 threads */
  csize = blosc_compress_ctx(3, 1, 16, isize, data, data_out, osize, "blosclz", 0, 2);
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
  dsize = blosc_decompress_ctx(data_out, data_dest, dsize, 1);
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
  
out:
  FreeLibrary(BDLL);
  return -1;
}
