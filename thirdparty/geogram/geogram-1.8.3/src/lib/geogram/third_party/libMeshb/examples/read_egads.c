
// libMeshb7 basic example: read an EGADS CAD model stored as a byte flow

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <libmeshb7.h>

int main(int argc, char **argv)
{
   int i, NmbBytes, ver, dim;
   char *cad, *InpNam;
   int64_t InpMsh;

   // Print usage and exit
   if(argc == 1)
   {
      puts("\nUSAGE:");
      puts("  read_egads cad_file.meshb\n");
      exit(0);
   }

   // Get the file name
   InpNam = *++argv;

   // Open the mesh file for reading
   if(!(InpMsh = GmfOpenMesh(InpNam, GmfRead, &ver, &dim)))
      return(1);

   // Read the egads tree stored as a raw byte flow
   cad = GmfReadByteFlow(InpMsh, &NmbBytes);

   // Print the byte flow
   printf("NmbBytes = %d\n", NmbBytes);
   for(i=0;i<NmbBytes;i++)
      printf("%d : %d\n", i, cad[i]);

   // Do not forget to close the mesh file
   GmfCloseMesh(InpMsh);

   return(0);
}
