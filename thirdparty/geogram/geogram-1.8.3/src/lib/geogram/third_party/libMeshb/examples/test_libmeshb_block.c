
// libMeshb 7.5 basic example: read a quad mesh, split it into triangles
// and write the result back using fast block transfer

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
   int   i, NmbVer, NmbQad, ver, dim, *RefTab, (*QadTab)[5], (*TriTab)[4];
   //int   TypTab[ GmfMaxTyp ], SizTab[ GmfMaxTyp ];
   //void *BegTab[ GmfMaxTyp ], *EndTab[ GmfMaxTyp ];
   int64_t InpMsh, OutMsh;
   float (*VerTab)[3];


   /*-----------------------------------*/
   /* Open mesh file "quad.meshb"       */
   /*-----------------------------------*/

   if(!(InpMsh = GmfOpenMesh("../sample_meshes/quad.meshb", GmfRead, &ver, &dim)))
      return(1);

   printf("InpMsh : idx = "INT64_T_FMT", version = %d, dimension = %d\n", InpMsh, ver, dim);

   if(dim != 3)
      exit(1);

   // Read the number of vertices and allocate memory
   NmbVer = (int)GmfStatKwd(InpMsh, GmfVertices);
   printf("InpMsh : nmb vertices = %d\n", NmbVer);
   VerTab = malloc((NmbVer+1) * 3 * sizeof(float));
   RefTab = malloc((NmbVer+1) * sizeof(int));

   // Read the number of quads and allocate memory
   NmbQad = (int)GmfStatKwd(InpMsh, GmfQuadrilaterals);
   printf("InpMsh : nmb quads = %d\n", NmbQad);
   QadTab = malloc((NmbQad+1) * 5 * sizeof(int));
   TriTab = malloc((NmbQad+1) * 2 * 4 * sizeof(int));

   // Read the vertices: choose one of the four available methods

   // First method: argument list with scalar pointers
   GmfGetBlock(InpMsh, GmfVertices, 1, NmbVer, 0, NULL, NULL,
            GmfFloat, &VerTab[1][0], &VerTab[ NmbVer ][0],
            GmfFloat, &VerTab[1][1], &VerTab[ NmbVer ][1],
            GmfFloat, &VerTab[1][2], &VerTab[ NmbVer ][2],
            GmfInt,   &RefTab[1],    &RefTab[ NmbVer ] );

   // 2nd method: argument list with vector pointers
   /*GmfGetBlock(InpMsh, GmfVertices, 1, NmbVer, 0, NULL, NULL,
               GmfFloatVec, 3, &VerTab[1][0], &VerTab[ NmbVer ][0],
               GmfInt,         &RefTab[1],    &RefTab[ NmbVer ] );*/

   // 3rd method: argument table with scalar pointers
   /*TypTab[0] = GmfFloat;
   BegTab[0] = (void *)&VerTab[      1 ][0];
   EndTab[0] = (void *)&VerTab[ NmbVer ][0];

   TypTab[1] = GmfFloat;
   BegTab[1] = (void *)&VerTab[      1 ][1];
   EndTab[1] = (void *)&VerTab[ NmbVer ][1];

   TypTab[2] = GmfFloat;
   BegTab[2] = (void *)&VerTab[      1 ][2];
   EndTab[2] = (void *)&VerTab[ NmbVer ][2];

   TypTab[3] = GmfInt;
   BegTab[3] = (void *)&RefTab[      1 ];
   EndTab[3] = (void *)&RefTab[ NmbVer ];

   GmfGetBlock(InpMsh, GmfVertices, 1, NmbVer, 0, NULL, NULL,
               GmfArgTab, TypTab, SizTab, BegTab, EndTab );*/

   // 4th method: argument table with vector pointers
   /*TypTab[0] = GmfFloatVec;
   SizTab[0] = 3;
   BegTab[0] = (void *)&VerTab[      1 ][0];
   EndTab[0] = (void *)&VerTab[ NmbVer ][0];

   TypTab[1] = GmfInt;
   BegTab[1] = (void *)&RefTab[      1 ];
   EndTab[1] = (void *)&RefTab[ NmbVer ];

   GmfGetBlock(InpMsh, GmfVertices, 1, NmbVer, 0, NULL, NULL,
               GmfArgTab, TypTab, SizTab, BegTab, EndTab );*/

   // Read the quads
   GmfGetBlock(InpMsh, GmfQuadrilaterals, 1, NmbQad, 0, NULL, NULL,
               GmfIntVec, 5, &QadTab[1][0], &QadTab[ NmbQad ][0]);

   // Close the quad mesh
   GmfCloseMesh(InpMsh);


   /*-----------------------------------*/
   /* Create the triangluated mesh      */
   /*-----------------------------------*/

   for(i=1;i<=NmbQad;i++)
   {
      TriTab[i*2-1][0] = QadTab[i][0];
      TriTab[i*2-1][1] = QadTab[i][1];
      TriTab[i*2-1][2] = QadTab[i][2];
      TriTab[i*2-1][3] = QadTab[i][4];

      TriTab[i*2][0] = QadTab[i][0];
      TriTab[i*2][1] = QadTab[i][2];
      TriTab[i*2][2] = QadTab[i][3];
      TriTab[i*2][3] = QadTab[i][4];
   }


   /*-----------------------------------*/
   /* Write the triangle mesh           */
   /*-----------------------------------*/

   if(!(OutMsh = GmfOpenMesh("tri.meshb", GmfWrite, ver, dim)))
      return(1);

   // Write the vertices
   GmfSetKwd(OutMsh, GmfVertices, NmbVer);
   GmfSetBlock(OutMsh, GmfVertices, 1, NmbVer, 0, NULL, NULL,
               GmfFloat, &VerTab[1][0], &VerTab[ NmbVer ][0],
               GmfFloat, &VerTab[1][1], &VerTab[ NmbVer ][1],
               GmfFloat, &VerTab[1][2], &VerTab[ NmbVer ][2],
               GmfInt,   &RefTab[1],    &RefTab[ NmbVer ] );

   // Write the triangles
   GmfSetKwd(OutMsh, GmfTriangles, 2*NmbQad);
   GmfSetBlock(OutMsh, GmfTriangles, 1, 2*NmbQad, 0, NULL, NULL,
               GmfInt, &TriTab[1][0], &TriTab[ 2*NmbQad ][0],
               GmfInt, &TriTab[1][1], &TriTab[ 2*NmbQad ][1],
               GmfInt, &TriTab[1][2], &TriTab[ 2*NmbQad ][2],
               GmfInt, &TriTab[1][3], &TriTab[ 2*NmbQad ][3] );

   // Do not forget to close the mesh file
   GmfCloseMesh(OutMsh);
   printf("OutMsh : nmb triangles = %d\n", 2*NmbQad);

   free(QadTab);
   free(TriTab);
   free(RefTab);
   free(VerTab);

   return(0);
}
