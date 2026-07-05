
// libMeshb7 basic example: write an EGADS CAD model stored as a byte flow

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <libmeshb7.h>

int main(int argc, char **argv)
{
   int NmbBytes = 9;
   char *OutNam, cad[9] = {1,2,3,4,5,6,7,8,9};
   int64_t OutMsh;

   // Print usage and exit
   if(argc == 1)
   {
      puts("\nUSAGE:");
      puts("  write_egads cad_file.meshb\n");
      exit(0);
   }

   // Get the file name
   OutNam = *++argv;

   // Open the mesh file for writing
   if(!(OutMsh = GmfOpenMesh(OutNam, GmfWrite, 2, 3)))
      return(1);

   // Write egads tree stored as a raw byte flow
   GmfWriteByteFlow(OutMsh, cad, NmbBytes);

   // Do not forget to close the mesh file
   GmfCloseMesh(OutMsh);

   return(0);
}
