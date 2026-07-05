
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif

#include <libmeshb7.h>


/*----------------------------------------------------------------------------*/
/* Starts or stops the given timer                                            */
/*----------------------------------------------------------------------------*/

double GetWallClock()
{
#ifdef WIN32
   struct __timeb64 tb;
   _ftime64(&tb);
   return((double)tb.time + (double)tb.millitm/1000.);
#else
   struct timeval tp;
   gettimeofday(&tp, NULL);
   return(tp.tv_sec + tp.tv_usec / 1000000.);
#endif
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

int main()
{
   int   i, NmbVer, NmbTet, ver, dim, *RefTab, (*TetTab)[5];
   int64_t InpMsh, OutMsh;
   double (*VerTab)[3];
   double timer;


   /*-----------------------------------*/
   /* Open mesh file "tets.meshb"       */
   /*-----------------------------------*/

   timer = GetWallClock();

   if(!(InpMsh = GmfOpenMesh("tets.meshb", GmfRead, &ver, &dim)))
      return(1);

   printf("InpMsh : idx = %lld, version = %d, dimension = %d\n", InpMsh, ver, dim);

   // Read the number of vertices and allocate memory
   NmbVer = GmfStatKwd(InpMsh, GmfVertices);
   printf("InpMsh : nmb vertices = %d\n", NmbVer);
   VerTab = malloc((size_t)(NmbVer+1) * 3 * sizeof(double));
   RefTab = malloc((size_t)(NmbVer+1)     * sizeof(int));

   // Read the number of tets and allocate memory
   NmbTet = GmfStatKwd(InpMsh, GmfTetrahedra);
   printf("InpMsh : nmb tets = %d\n", NmbTet);
   TetTab = malloc((size_t)(NmbTet+1) * 5 * sizeof(int));

   // Read the vertices
   GmfGetBlock(InpMsh, GmfVertices, 1, NmbVer, 0, NULL, NULL,
               GmfDoubleVec, 3, &VerTab[1][0], &VerTab[ NmbVer ][0],
               GmfInt,         &RefTab[1],    &RefTab[ NmbVer ] );

   // Read the tets
   GmfGetBlock(InpMsh, GmfTetrahedra, 1, NmbTet, 0, NULL, NULL,
               GmfIntVec, 5, &TetTab[1][0], &TetTab[ NmbTet ][0]);

   // Close the tet mesh
   GmfCloseMesh(InpMsh);

   printf("Time for reading: %g seconds\n", GetWallClock() - timer);


   /*-----------------------------------*/
   /* Write the mesh back               */
   /*-----------------------------------*/

   timer = GetWallClock();

   if(!(OutMsh = GmfOpenMesh("tets_out.meshb", GmfWrite, ver, dim)))
      return(1);

   // Write the vertices
   GmfSetKwd(OutMsh, GmfVertices, NmbVer);
   GmfSetBlock(OutMsh, GmfVertices, 1, NmbVer, 0, NULL, NULL,
               GmfDoubleVec, 3, &VerTab[1][0], &VerTab[ NmbVer ][0],
               GmfInt,         &RefTab[1],    &RefTab[ NmbVer ] );

   // Write the tets
   GmfSetKwd(OutMsh, GmfTetrahedra, NmbTet);
   GmfSetBlock(InpMsh, GmfTetrahedra, 1, NmbTet, 0, NULL, NULL,
               GmfIntVec, 5, &TetTab[1][0], &TetTab[ NmbTet ][0]);

   // Do not forget to close the mesh file
   GmfCloseMesh(OutMsh);

   printf("Time for writing: %g seconds\n", GetWallClock() - timer);

   free(TetTab);
   free(RefTab);
   free(VerTab);

   return(0);
}
