/*********************************************************************
  Print versions for Blosc and all its internal compressors.
*********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../blosc/blosc.h"


int main(int argc, char *argv[]) {

  char *name = NULL, *version = NULL;
  int ret;

  printf("Blosc version: %s (%s)\n", BLOSC_VERSION_STRING, BLOSC_VERSION_DATE);

  printf("List of supported compressors in this build: %s\n",
         blosc_list_compressors());

  printf("Supported compression libraries:\n");
  ret = blosc_get_complib_info("blosclz", &name, &version);
  if (ret >= 0) printf("  %s: %s\n", name, version);
  ret = blosc_get_complib_info("lz4", &name, &version);
  if (ret >= 0) printf("  %s: %s\n", name, version);
  ret = blosc_get_complib_info("snappy", &name, &version);
  if (ret >= 0) printf("  %s: %s\n", name, version);
  ret = blosc_get_complib_info("zlib", &name, &version);
  if (ret >= 0) printf("  %s: %s\n", name, version);
  ret = blosc_get_complib_info("zstd", &name, &version);
  if (ret >= 0) printf("  %s: %s\n", name, version);

  return(0);
}
