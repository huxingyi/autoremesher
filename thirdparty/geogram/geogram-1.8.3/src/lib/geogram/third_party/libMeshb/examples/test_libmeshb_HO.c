
// libMeshb 7 basic example:
// read a Q2 quad mesh while using the automatic HO reordering feature,
// split it into P2 triangles and write the result back using fast block transfer

#include <stdio.h>
#include <stdlib.h>
#include <libmeshb7.h>

#ifdef PRINTF_INT64_MODIFIER
#define INT64_T_FMT "%" PRINTF_INT64_MODIFIER "d"
#else
#   ifdef GMF_WINDOWS
#    define INT64_T_FMT "%Id"
#   else
#    include <inttypes.h>
#    define INT64_T_FMT "%" PRId64
#   endif
#endif

int main()
{
   int i, NmbVer, NmbQad, ver, dim, *RefTab, (*QadTab)[10], (*TriTab)[7];
   int BasOrd[9][2] = { {0,0}, {1,0}, {2,0}, {0,1}, {1,1}, {2,1}, {0,2}, {1,2}, {2,2} };
   int FilOrd[9][2];
   int64_t InpMsh, OutMsh;
   float (*VerTab)[3];


   /*-----------------------------------*/
   /* Open mesh file "quad_q2.meshb"    */
   /*-----------------------------------*/

   if(!(InpMsh = GmfOpenMesh("../sample_meshes/quad_q2.meshb", GmfRead, &ver, &dim)))
      return(1);

   printf("InpMsh : idx = "INT64_T_FMT", version = %d, dimension = %d\n", InpMsh, ver, dim);

   if(dim != 3)
      exit(1);

   // Read the number of vertices and allocate memory
   NmbVer = (int)GmfStatKwd(InpMsh, GmfVertices);
   printf("InpMsh : nmb vertices = %d\n", NmbVer);
   VerTab = malloc((NmbVer+1) * 3 * sizeof(float));
   RefTab = malloc((NmbVer+1) * sizeof(int));
   
   // Read the number of Q2 quads and allocate memory
   NmbQad = (int)GmfStatKwd(InpMsh, GmfQuadrilateralsQ2);
   printf("InpMsh : nmb Q2 quads = %d\n", NmbQad);
   QadTab = malloc((NmbQad+1) * 10 * sizeof(int));
   TriTab = malloc((NmbQad+1) * 2 * 7 * sizeof(int));

   // Read the optional Q2 quad node ordering information and pass it
   // to the library along our internal coding to automatically reorder the nodes
   if(GmfStatKwd(InpMsh, GmfQuadrilateralsQ2Ordering))
   {
      GmfGetBlock(InpMsh, GmfQuadrilateralsQ2Ordering, 1, 9, 0, NULL, NULL,
                  GmfIntTab, 9, FilOrd[0], FilOrd[8]);

      GmfSetHONodesOrdering(InpMsh, GmfQuadrilateralsQ2, (int *)BasOrd, (int *)FilOrd);
   }
   
   /*
   for(i=0;i<9;i++){
     printf ("i=%d BasOrd: i=%d j=%d - FilOrd: i=%d j=%d \n", i+1,BasOrd[i][0],BasOrd[i][1], FilOrd[i][0],FilOrd[i][1]);
   }
   */
   
   // Read the vertices
   GmfGetBlock(InpMsh, GmfVertices, 1, NmbVer, 0, NULL, NULL,
               GmfFloat, &VerTab[1][0], &VerTab[ NmbVer ][0],
               GmfFloat, &VerTab[1][1], &VerTab[ NmbVer ][1],
               GmfFloat, &VerTab[1][2], &VerTab[ NmbVer ][2],
               GmfInt,   &RefTab[1],    &RefTab[ NmbVer ] );



   // Read the Q2 quads
   GmfGetBlock(InpMsh, GmfQuadrilateralsQ2, 1, NmbQad, 0, NULL, NULL,
               GmfIntTab, 10, QadTab[1], QadTab[ NmbQad ]);
   
   /*
   i=1;
   printf ("Quad1 %d %d %d %d %d %d %d %d %d\n", QadTab[i][0], QadTab[i][1], QadTab[i][2], QadTab[i][3], QadTab[i][4], QadTab[i][5], QadTab[i][6], QadTab[i][7], QadTab[i][8]);
   */
   
   // Close the quad mesh
   GmfCloseMesh(InpMsh);


   /*-----------------------------------*/
   /* Create the triangulated P2 mesh   */
   /*-----------------------------------*/

   for(i=1;i<=NmbQad;i++)
   {
      TriTab[i*2-1][0] = QadTab[i][0];
      TriTab[i*2-1][1] = QadTab[i][2];
      TriTab[i*2-1][2] = QadTab[i][8];
      TriTab[i*2-1][3] = QadTab[i][1];
      TriTab[i*2-1][4] = QadTab[i][5];
      TriTab[i*2-1][5] = QadTab[i][4];
      TriTab[i*2-1][6] = QadTab[i][9];

      TriTab[i*2][0] = QadTab[i][0];
      TriTab[i*2][1] = QadTab[i][8];
      TriTab[i*2][2] = QadTab[i][6];
      TriTab[i*2][3] = QadTab[i][4];
      TriTab[i*2][4] = QadTab[i][7];
      TriTab[i*2][5] = QadTab[i][3];
      TriTab[i*2][6] = QadTab[i][9];
   }


   /*-----------------------------------*/
   /* Write the P2 triangle mesh        */
   /*-----------------------------------*/

   if(!(OutMsh = GmfOpenMesh("tri_p2.meshb", GmfWrite, ver, dim)))
      return(1);

   // Write the vertices
   GmfSetKwd(OutMsh, GmfVertices, NmbVer);
   GmfSetBlock(OutMsh, GmfVertices, 1, NmbVer, 0, NULL, NULL,
               GmfFloat, &VerTab[1][0], &VerTab[ NmbVer ][0],
               GmfFloat, &VerTab[1][1], &VerTab[ NmbVer ][1],
               GmfFloat, &VerTab[1][2], &VerTab[ NmbVer ][2],
               GmfInt,   &RefTab[1],    &RefTab[ NmbVer ] );

   // Write the triangles
   GmfSetKwd(OutMsh, GmfTrianglesP2, 2*NmbQad);
   GmfSetBlock(OutMsh, GmfTrianglesP2, 1, 2*NmbQad, 0, NULL, NULL,
               GmfIntTab, 7, TriTab[1], TriTab[ 2*NmbQad ]);

   // Do not forget to close the mesh file
   GmfCloseMesh(OutMsh);
   printf("OutMsh : nmb triangles = %d\n", 2*NmbQad);

   free(QadTab);
   free(TriTab);
   free(RefTab);
   free(VerTab);

   return(0);
}
