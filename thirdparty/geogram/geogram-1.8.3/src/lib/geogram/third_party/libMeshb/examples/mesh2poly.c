

/*----------------------------------------------------------------------------*/
/* Includes                                                                   */
/*----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libmeshb7.h>


/*----------------------------------------------------------------------------*/
/* Topologic tables                                                           */
/*----------------------------------------------------------------------------*/

int tettvpe[6][2] = { {0,1}, {1,2}, {2,0}, {3,0}, {3,1}, {3,2} };
int tettvpf[4][3] = { {1,2,3}, {2,0,3}, {3,0,1}, {0,2,1} };
int pyrtvpe[8][2] = { {0,1}, {3,2}, {0,3}, {1,2}, {0,4}, {1,4}, {2,4}, {3,4} };
int pyrtvpf[5][4] = { {0,1,4,-1}, {1,2,4,-1}, {2,3,4,-1}, {3,0,4,-1}, {3,2,1,0} };
int pritvpe[9][2] = { {0,1}, {1,2}, {2,0}, {3,4}, {4,5}, {5,3}, {0,3}, {1,4}, {2,5} };
int pritvpf[5][4] = { {3,5,2,0}, {1,2,5,4}, {0,1,4,3}, {2,1,0,-1}, {3,4,5,-1} };
int hextvpe[12][2] = {  {3,2}, {0,1}, {4,5}, {7,6}, {3,7}, {2,6},
                        {1,5}, {0,4}, {3,0}, {7,4}, {6,5}, {2,1} };
int hextvpf[6][4] = {   {3,0,4,7}, {6,5,1,2}, {3,2,1,0},
                        {4,5,6,7},{3,7,6,2}, {1,5,4,0} };


/*----------------------------------------------------------------------------*/
/* Transform a surface mesh into polygons and a tet mesh into polyhedra       */ 
/*----------------------------------------------------------------------------*/

int main(int ArgCnt, char **ArgVec)
{
   int      i, j, k, ver, dim, ref, nod[8], ArgIdx = 1;
   int      NmbVer, NmbTri, NmbQad, NmbTet, NmbPyr, NmbPri, NmbHex;
   int64_t  InpMsh, OutMsh;
   float    xf, yf, zf;
   double   xd, yd, zd;
   char    *InpNam, *OutNam;

   if(ArgCnt != 3)
   {
      puts("\nMESH2POLY 1.1, september 15 2021, Loic MARECHAL / INRIA\n");
      puts(" Usage    : mesh2poly source_name destination_name\n");
      exit(0);
   }

   InpNam = ArgVec[ ArgIdx++ ];
   OutNam = ArgVec[ ArgIdx++ ];

   if(!strlen(InpNam))
   {
      puts("Missing input name.");
      exit(1);
   }

   if(!strlen(OutNam))
   {
      puts("Missing output name.");
      exit(2);
   }

   if(!strcmp(InpNam, OutNam))
   {
      puts("The output mesh cannot overwrite the input mesh.");
      exit(3);
   }

   if(!(InpMsh = GmfOpenMesh(InpNam, GmfRead, &ver, &dim)))
   {
      printf("Cannot open mesh file %s.\n", InpNam);
      return(4);
   }

   if(!(OutMsh = GmfOpenMesh(OutNam, GmfWrite, ver, dim)))
   {
      printf("Cannot open mesh file %s.\n", OutNam);
      return(5);
   }

   NmbVer = (int)GmfStatKwd(InpMsh, GmfVertices);
   NmbTri = (int)GmfStatKwd(InpMsh, GmfTriangles);
   NmbQad = (int)GmfStatKwd(InpMsh, GmfQuadrilaterals);
   NmbTet = (int)GmfStatKwd(InpMsh, GmfTetrahedra);
   NmbPri = (int)GmfStatKwd(InpMsh, GmfPrisms);
   NmbPyr = (int)GmfStatKwd(InpMsh, GmfPyramids);
   NmbHex = (int)GmfStatKwd(InpMsh, GmfHexahedra);

   printf(" InpMsh: idx = %lld, version: %d, dimension: %d\n", InpMsh, ver, dim);

   // Copy all the vertices
   if(NmbVer)
   {
      printf(" Copying %d vertices\n", NmbVer);

      GmfGotoKwd(InpMsh, GmfVertices);
      GmfSetKwd(OutMsh, GmfVertices, NmbVer);

      for(i=1;i<=NmbVer;i++)
      {
         if(ver <= 1)
         {
            GmfGetLin(InpMsh, GmfVertices, &xf, &yf, &zf, &ref);
            GmfSetLin(OutMsh, GmfVertices,  xf,  yf,  zf,  ref);
         }
         else
         {
            GmfGetLin(InpMsh, GmfVertices, &xd, &yd, &zd, &ref);
            GmfSetLin(OutMsh, GmfVertices,  xd,  yd,  zd,  ref);
         }
      }
   }

   // Transform triangles and quads into boundary polygons
   if(NmbTri || NmbQad)
   {
      GmfSetKwd(OutMsh, GmfBoundaryPolygonHeaders, NmbTri + NmbQad);

      if(NmbTri)
      {
         printf(" Adding  %d triangles to the polygons headers\n", NmbTri);

         for(i=1;i<=NmbTri;i++)
            GmfSetLin(OutMsh, GmfBoundaryPolygonHeaders, (i-1) * 3 + 1, ref);
      }

      if(NmbQad)
      {
         printf(" Adding  %d quads to the polygons headers\n", NmbQad);

         for(i=1;i<=NmbQad;i++)
            GmfSetLin(OutMsh, GmfBoundaryPolygonHeaders, (i-1) * 4 + NmbTri * 3 + 1, ref);
      }

      GmfSetKwd(OutMsh, GmfBoundaryPolygonVertices, NmbTri * 3 + NmbQad * 4);

      if(NmbTri)
      {
         printf(" Adding  %d triangles to the polygons faces\n", NmbTri);
         GmfGotoKwd(InpMsh, GmfTriangles);

         for(i=1;i<=NmbTri;i++)
         {
            GmfGetLin(InpMsh, GmfTriangles, &nod[0], &nod[1], &nod[2], &ref);

            for(j=0;j<3;j++)
               GmfSetLin(OutMsh, GmfBoundaryPolygonVertices, nod[j]);
         }
      }

      if(NmbQad)
      {
         printf(" Adding  %d quads to the polygons faces\n", NmbQad);
         GmfGotoKwd(InpMsh, GmfQuadrilaterals);

         for(i=1;i<=NmbQad;i++)
         {
            GmfGetLin(InpMsh, GmfQuadrilaterals, &nod[0], &nod[1], &nod[2], &nod[3], &ref);

            for(j=0;j<4;j++)
               GmfSetLin(OutMsh, GmfBoundaryPolygonVertices, nod[j]);
         }
      }
   }

   // Transform tets into inner polygons and polyhedra
   if(NmbTet)
   {
      GmfSetKwd(OutMsh, GmfInnerPolygonHeaders, 4 * NmbTet);

      if(NmbTet)
      {
         printf(" Adding  %d tet's faces to the polygons headers\n", 4 * NmbTet);

         for(i=0;i<NmbTet;i++)
            for(j=0;j<4;j++)
               GmfSetLin(OutMsh, GmfInnerPolygonHeaders, i * 12 + j * 3 + 1, ref);
      }

      GmfSetKwd(OutMsh, GmfInnerPolygonVertices, 12 * NmbTet);

      if(NmbTet)
      {
         printf(" Adding  %d tet's faces to the polygons faces\n", NmbTet);
         GmfGotoKwd(InpMsh, GmfTetrahedra);

         for(i=1;i<=NmbTet;i++)
         {
            GmfGetLin(InpMsh, GmfTetrahedra, &nod[0], &nod[1], &nod[2], &nod[3], &ref);

            for(j=0;j<4;j++)
               for(k=0;k<3;k++)
                  GmfSetLin(OutMsh, GmfInnerPolygonVertices, nod[ tettvpf[j][k] ]);
         }
      }

      GmfSetKwd(OutMsh, GmfPolyhedraHeaders, NmbTet);

      if(NmbTet)
      {
         printf(" Adding  %d tets to the polyhedra headers\n", NmbTet);
         GmfGotoKwd(InpMsh, GmfTetrahedra);

         for(i=0;i<NmbTet;i++)
         {
            GmfGetLin(InpMsh, GmfTetrahedra, &nod[0], &nod[1], &nod[2], &nod[3], &ref);
            GmfSetLin(OutMsh, GmfPolyhedraHeaders, i * 4 + 1, ref);
         }
      }

      GmfSetKwd(OutMsh, GmfPolyhedraFaces, NmbTet * 4);

      if(NmbTet)
      {
         printf(" Adding  %d tet's face to the polyhedra faces\n", NmbTet * 4);

         for(i=0;i<NmbTet;i++)
            for(j=0;j<4;j++)
               GmfSetLin(OutMsh, GmfPolyhedraFaces, i * 4 + j + 1);
      }
   }

   GmfCloseMesh(InpMsh);
   GmfCloseMesh(OutMsh);

   return(0);
}
