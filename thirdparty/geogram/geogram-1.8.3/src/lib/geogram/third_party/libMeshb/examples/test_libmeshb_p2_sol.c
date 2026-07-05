
// libMeshb 7.5 basic example: read a high order solution file

#include <stdio.h>
#include <stdlib.h>
#include <libmeshb7.h>


int main()
{
   int i, j, NmbSol, ver, dim, SolSiz, NmbTyp, TypTab[ GmfMaxTyp ], deg, NmbNod;
   long long InpMsh;
   double *SolTab;

   if(!(InpMsh = GmfOpenMesh("../sample_meshes/edges_p2.sol", GmfRead, &ver, &dim)))
      return(1);

   printf("InpMsh: idx = %lld, version = %d, dimension = %d\n", InpMsh, ver, dim);

   if(ver < 2)
      return(1);

   // Read the number vertices and associated solution size for memory allocation
   NmbSol = (int)GmfStatKwd(InpMsh, GmfHOSolAtEdgesP2, &NmbTyp, &SolSiz, TypTab, &deg, &NmbNod);
   printf("NmbSol = %d, NmbTyp = %d, SolSiz = %d, degree = %d, NmbNod = %d\n",
            NmbSol, NmbTyp, SolSiz, deg, NmbNod);
   SolTab = malloc( (NmbSol+1) * SolSiz * sizeof(double));

   // solution field block reading
   GmfGetBlock(InpMsh, GmfHOSolAtEdgesP2, 1, NmbSol, 0, NULL, NULL,
               GmfDoubleVec, SolSiz, &SolTab[ 1 * SolSiz ], &SolTab[ NmbSol * SolSiz ]);

   // Print each solutions of each vertices
   for(i=1;i<=NmbSol;i++)
      for(j=0;j<SolSiz;j++)
         printf("%d, %d = %g\n", i, j, SolTab[ i * SolSiz + j ]);

   // Close the mesh file and free memory
   GmfCloseMesh(InpMsh);
   free(SolTab);

   return(0);
}
