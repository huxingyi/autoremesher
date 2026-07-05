
/* libMeshb 7.2 basic example: read a quad mesh, split it into triangles
   and write the result back */

#include <stdio.h>
#include <stdlib.h>
#include <libmeshb7.h>

int main()
{
   int i, NmbVer, NmbQad, ver, dim, *RefTab, (*QadTab)[5];
   long long InpMsh, OutMsh;
   double (*VerTab)[3];


   /*-----------------------------------*/
   /* Open mesh file "quad.meshb"       */
   /*-----------------------------------*/

   if(!(InpMsh = GmfOpenMesh("../sample_meshes/quad.mesh", GmfRead, &ver, &dim)))
      return(1);

   printf("InpMsh: idx = %lld, version = %d, dimension = %d\n", InpMsh, ver, dim);

   if( (ver != 2) || (dim != 3) )
      exit(1);

   // Read the number of vertices and allocate memory
   NmbVer = (int)GmfStatKwd(InpMsh, GmfVertices);
   printf("InpMsh: nmb vertices = %d\n", NmbVer);
   VerTab = malloc((NmbVer+1) * 3 * sizeof(double));
   RefTab = malloc((NmbVer+1) * sizeof(int));

   // Read the number of quads and allocate memory
   NmbQad = (int)GmfStatKwd(InpMsh, GmfQuadrilaterals);
   printf("InpMsh: nmb quads = %d\n", NmbQad);
   QadTab = malloc((NmbQad+1) * 5 * sizeof(int));

   // Read the vertices
   GmfGotoKwd(InpMsh, GmfVertices);

   for(i=1;i<=NmbVer;i++)
      GmfGetLin(  InpMsh, GmfVertices, &VerTab[i][0], &VerTab[i][1],
                  &VerTab[i][2], &RefTab[i] );

   // Read the quads
   GmfGotoKwd(InpMsh, GmfQuadrilaterals);

   for(i=1;i<=NmbQad;i++)
      GmfGetLin(  InpMsh, GmfQuadrilaterals, &QadTab[i][0], &QadTab[i][1],
                  &QadTab[i][2], &QadTab[i][3], &QadTab[i][4] );

   // Close the quad mesh
   GmfCloseMesh(InpMsh);


   /*-----------------------------------*/
   /* Write the triangle mesh           */
   /*-----------------------------------*/

   if(!(OutMsh = GmfOpenMesh("tri.mesh", GmfWrite, ver, dim)))
      return(1);

   // Write the vertices
   GmfSetKwd(OutMsh, GmfVertices, NmbVer);

   for(i=1;i<=NmbVer;i++)
      GmfSetLin(  OutMsh, GmfVertices, VerTab[i][0],
                  VerTab[i][1], VerTab[i][2], RefTab[i] );

   // Write the triangles
   GmfSetKwd(OutMsh, GmfTriangles, 2*NmbQad);

   // Split each quad into two triangles on the fly
   for(i=1;i<=NmbQad;i++)
   {
      GmfSetLin(  OutMsh, GmfTriangles, QadTab[i][0],
                  QadTab[i][1], QadTab[i][2], QadTab[i][4] );
      GmfSetLin(  OutMsh, GmfTriangles, QadTab[i][0],
                  QadTab[i][2], QadTab[i][3], QadTab[i][4] );
   }

   // Do not forget to close the mesh file
   GmfCloseMesh(OutMsh);
   printf("OutMsh: nmb triangles = %d\n", 2 * NmbQad);

   free(QadTab);
   free(RefTab);
   free(VerTab);

   return(0);
}
