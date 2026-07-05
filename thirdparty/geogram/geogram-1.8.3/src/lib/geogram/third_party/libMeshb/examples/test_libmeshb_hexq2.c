
// libMeshb 7.5 basic example:
// read a Q2 hex mesh while using the automatic HO reordering feature,
// and print the renumbered coordinates

#include <stdio.h>
#include <stdlib.h>
#include <libmeshb7.h>

int main()
{
   int i, NmbVer, NmbHex, ver, dim, *RefTab, (*HexTab)[28];
   int BasOrd[27][3] = {  {0, 0, 0},
                          {2, 0, 0},
                          {2, 2, 0},
                          {0, 2, 0},
                          {0, 0, 2},
                          {2, 0, 2},
                          {2, 2, 2},
                          {0, 2, 2},
                          {1, 0, 0},
                          {2, 1, 0},
                          {1, 2, 0},
                          {0, 1, 0},
                          {1, 0, 2},
                          {2, 1, 2},
                          {1, 2, 2},
                          {0, 1, 2},
                          {0, 0, 1},
                          {2, 0, 1},
                          {2, 2, 1},
                          {0, 2, 1},
                          {1, 1, 0},
                          {1, 1, 2},
                          {1, 0, 1},
                          {2, 1, 1},
                          {1, 2, 1},
                          {0, 1, 1},
                          {1, 1, 1}};
   int FilOrd[27][3];
   int64_t InpMsh, OutMsh;
   float (*VerTab)[3];


   /*-----------------------------------*/
   /* Open mesh file "quad_q2.meshb"    */
   /*-----------------------------------*/

   if(!(InpMsh = GmfOpenMesh("../sample_meshes/hexaq2_vizir.mesh", GmfRead, &ver, &dim)))
      return(1);

   printf("InpMsh : idx = %lld, version = %d, dimension = %d\n", InpMsh, ver, dim);

   if(dim != 3)
      exit(1);

   // Read the number of vertices and allocate memory
   NmbVer = GmfStatKwd(InpMsh, GmfVertices);
   printf("InpMsh : nmb vertices = %d\n", NmbVer);
   VerTab = malloc((NmbVer+1) * 3 * sizeof(float));
   RefTab = malloc((NmbVer+1) * sizeof(int));

   // Read the number of Q2 hex and allocate memory
   NmbHex = GmfStatKwd(InpMsh, GmfHexahedraQ2);
   printf("InpMsh : nmb Q2 hexes = %d\n", NmbHex);
   HexTab = malloc((NmbHex+1) * 28 * sizeof(int));

   // Read the optional Q2 quad node ordering information and pass it
   // to the library along our internal coding to automatically reorder the nodes
   if(GmfStatKwd(InpMsh, GmfHexahedraQ2Ordering))
   {
      GmfGetBlock(InpMsh, GmfHexahedraQ2Ordering, 1, 27, 0, NULL, NULL,
                  GmfIntVec, 27, FilOrd[0], FilOrd[26]);

      GmfSetHONodesOrdering(InpMsh, GmfHexahedraQ2, (int *)BasOrd, (int *)FilOrd);
   }

   // Read the vertices
   GmfGetBlock(InpMsh, GmfVertices, 1, NmbVer, 0, NULL, NULL,
               GmfFloatVec, 3, &VerTab[1][0], &VerTab[ NmbVer ][0],
               GmfInt,         &RefTab[1],    &RefTab[ NmbVer ] );

   // Read the Q2 quads
   GmfGetBlock(InpMsh, GmfHexahedraQ2, 1, NmbHex, 0, NULL, NULL,
               GmfIntVec, 28, HexTab[1], HexTab[ NmbHex ]);

   // Close the quad mesh
   GmfCloseMesh(InpMsh);

   for(i=0;i<27;i++)
      printf("%g %g %g 0\n", VerTab [HexTab[1][i] ][0], VerTab [HexTab[1][i] ][1], VerTab [HexTab[1][i] ][2]);

   free(HexTab);
   free(RefTab);
   free(VerTab);

   return(0);
}
