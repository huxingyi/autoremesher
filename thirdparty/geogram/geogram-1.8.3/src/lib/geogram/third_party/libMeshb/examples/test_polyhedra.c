
// Read and prints a polyhedral mesh using the helpers functions

#include <stdio.h>
#include <stdlib.h>
#include <libmeshb7.h>
#include <libmeshb7_helpers.h>

int main()
{
   int         i, j, k, NmbVer, ver, dim, *RefTab, ret;
   int         deg1, buf1[256], deg2, buf2[256];
   int64_t     InpMsh;
   double      (*VerTab)[3];
   PolMshSct   *pol;

   // Open and check the polyhedral mesh file
   if(!(InpMsh = GmfOpenMesh("../sample_meshes/polyhedra.mesh", GmfRead, &ver, &dim)))
   {
      puts("Could not open ../sample_meshes/polyhedra.mesh");
      return(1);
   }

   printf("InpMsh: idx = %lld, version = %d, dimension = %d\n", InpMsh, ver, dim);

   if( (ver != 2) || (dim != 3) )
   {
      printf("Wrong version (%d) or dimension (%d)\n", ver, dim);
      return(1);
   }

   // Read the number of vertices and allocate memory
   NmbVer = (int)GmfStatKwd(InpMsh, GmfVertices);
   printf("InpMsh: nmb vertices = %d\n", NmbVer);
   VerTab = malloc((NmbVer+1) * 3 * sizeof(double));
   RefTab = malloc((NmbVer+1) * sizeof(int));

   // Read the number of polygons/polyhedra and allocate memory
   pol = GmfAllocatePolyhedralStructure(InpMsh);

   if(!pol)
   {
      puts("Could not allocate the polyhedral structure");
      return(1);
   }

   // Read both keyword's fields associated with the boundary polygons
   ret = GmfReadBoundaryPolygons(pol);

   if(!ret)
   {
      puts("Could not read the boundary polygons");
      return(1);
   }

   // Print the polygon's degree and vertex indices
   for(i=1;i<=pol->NmbBndHdr;i++)
   {
      // Extract a polygon's list of vertices to a buffer
      deg1 = GmfGetBoundaryPolygon(pol, i, buf1);
      printf("polygon %d (%d): ", i, deg1);

      for(j=0;j<deg1;j++)
         printf("%d ", buf1[j]);

      puts("");
   }

   // Read all four keyword's fields associated with the polyhedra
   ret = GmfReadPolyhedra(pol);

   if(!ret)
   {
      puts("Could not read the polyhedra");
      return(1);
   }

   // Print the polyhedra's degree and face list,
   // then all the inner faces vertex indices
   for(i=1;i<=pol->NmbVolHdr;i++)
   {
      // Extract a polyhedron's list of inner faces to a buffer
      deg1 = GmfGetPolyhedron(pol, i, buf1);
      printf("polyhedron %d (%d): ", i, deg1);

      for(j=0;j<deg1;j++)
         printf("%d ", buf1[j]);

      puts("");

      // Print each inner polygon's vertex indices
      for(j=0;j<deg1;j++)
      {
         // Extract a polygon's list of vertices to a buffer
         deg2 = GmfGetInnerPolygon(pol, buf1[j], buf2);
         printf("polygon %d (%d): ", buf1[j], deg2);

         for(k=0;k<deg2;k++)
            printf("%d ", buf2[k]);

         puts("");
      }
   }

   // Close the mesh and free memories
   GmfCloseMesh(InpMsh);
   GmfFreePolyghedralStructure(pol);
   free(RefTab);
   free(VerTab);

   return(0);
}
