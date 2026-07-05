

/*----------------------------------------------------------------------------*/
/*                                                                            */
/*                            LIBMESHB-HELPERS V0.91                          */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Description:         Set of helpers functions useful for the libMeshb      */
/* Author:              Loic MARECHAL                                         */
/* Creation date:       mar 24 2021                                           */
/* Last modification:   nov 05 2021                                           */
/*                                                                            */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Includes                                                                   */
/*----------------------------------------------------------------------------*/

#include "libmeshb7_helpers.h"
#include "libmeshb7.h"
#include <stdio.h>
#include <stdlib.h>

/*----------------------------------------------------------------------------*/
/* Defines                                                                    */
/*----------------------------------------------------------------------------*/

#ifdef INT64
#define itg int64_t
#else
#define itg int32_t
#endif

#ifdef REAL32
#define fpn float
#else
#define fpn double
#endif

/*----------------------------------------------------------------------------*/
/* Defintion of macro commands                                                */
/*----------------------------------------------------------------------------*/

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define POW(a) ((a) * (a))

/*----------------------------------------------------------------------------*/
/* Allocate a structure to hold all polygons and polyhedra related data       */
/*----------------------------------------------------------------------------*/

PolMshSct *GmfAllocatePolyhedralStructure(int64_t MshIdx)
{
   int SrfFlg = 0, VolFlg = 0, ErrFlg = 0;
   itg NmbBndHdr, NmbBndVer, NmbInrHdr, NmbInrVer, NmbVolHdr, NmbVolFac;
   PolMshSct *pol;

   if (!MshIdx)
      return (NULL);

   // Get all the polyhedra related field's sizes
   NmbBndHdr = (itg)GmfStatKwd(MshIdx, GmfBoundaryPolygonHeaders);
   NmbBndVer = (itg)GmfStatKwd(MshIdx, GmfBoundaryPolygonVertices);
   NmbInrHdr = (itg)GmfStatKwd(MshIdx, GmfInnerPolygonHeaders);
   NmbInrVer = (itg)GmfStatKwd(MshIdx, GmfInnerPolygonVertices);
   NmbVolHdr = (itg)GmfStatKwd(MshIdx, GmfPolyhedraHeaders);
   NmbVolFac = (itg)GmfStatKwd(MshIdx, GmfPolyhedraFaces);

   // There are surface polygons
   if (NmbBndHdr && NmbBndVer)
      SrfFlg = 1;

   // There are volume polyhedra
   if (NmbInrHdr && NmbInrVer && NmbVolHdr && NmbVolFac)
      VolFlg = 1;

   if (!SrfFlg && !VolFlg)
      return (NULL);

   pol = malloc(sizeof(PolMshSct));

   if (!pol)
      return (NULL);

   pol->MshIdx = MshIdx;

   // Store surface information and allocate Headers and data tables
   if (SrfFlg)
   {
      pol->NmbBndHdr = NmbBndHdr;
      pol->NmbBndVer = NmbBndVer;

      pol->BndHdrTab = malloc((NmbBndHdr + 1) * 2 * sizeof(itg));
      pol->BndVerTab = malloc((NmbBndVer + 1) * sizeof(itg));

      if (!pol->BndHdrTab || !pol->BndVerTab)
         ErrFlg = 1;
   }
   else
   {
      pol->NmbBndHdr = 0;
      pol->NmbBndVer = 0;
   }

   // Store volume information and allocate Headers and data tables
   // for both inner polygons and polyhedra
   if (VolFlg)
   {
      pol->NmbInrHdr = NmbInrHdr;
      pol->NmbInrVer = NmbInrVer;
      pol->NmbVolHdr = NmbVolHdr;
      pol->NmbVolFac = NmbVolFac;

      pol->InrHdrTab = malloc((NmbInrHdr + 1) * 2 * sizeof(itg));
      pol->InrVerTab = malloc((NmbInrVer + 1) * sizeof(itg));
      pol->VolHdrTab = malloc((NmbVolHdr + 1) * 2 * sizeof(itg));
      pol->VolFacTab = malloc((NmbVolFac + 1) * sizeof(itg));

      if (!pol->InrHdrTab || !pol->InrVerTab || !pol->VolHdrTab || !pol->VolFacTab)
         ErrFlg = 1;
   }
   else
   {
      pol->NmbInrHdr = 0;
      pol->NmbInrVer = 0;
      pol->NmbVolHdr = 0;
      pol->NmbVolFac = 0;
   }

   // If something went wrong, free all memories
   if (ErrFlg)
   {
      GmfFreePolyghedralStructure(pol);
      return (NULL);
   }

   return (pol);
}

/*----------------------------------------------------------------------------*/
/* Free all polyhedral tables and the head structure itself                   */
/*----------------------------------------------------------------------------*/

void GmfFreePolyghedralStructure(PolMshSct *pol)
{
   if (!pol)
      return;

   if (pol->BndHdrTab)
      free(pol->BndHdrTab);

   if (pol->BndVerTab)
      free(pol->BndVerTab);

   if (pol->InrHdrTab)
      free(pol->InrHdrTab);

   if (pol->InrVerTab)
      free(pol->InrVerTab);

   if (pol->VolHdrTab)
      free(pol->VolHdrTab);

   if (pol->VolFacTab)
      free(pol->VolFacTab);

   free(pol);
}

/*----------------------------------------------------------------------------*/
/* Read the surface polygons from an already opened mesh file                 */
/* and store the data in the provided polyhedral structure                    */
/*----------------------------------------------------------------------------*/

int GmfReadBoundaryPolygons(PolMshSct *pol)
{
   int ret;

   if (!pol || !pol->BndHdrTab || !pol->BndVerTab)
      return (0);

   ret = GmfGetBlock(pol->MshIdx, GmfBoundaryPolygonHeaders,
                     1, pol->NmbBndHdr, 0, NULL, NULL, GmfIntVec, 2,
                     pol->BndHdrTab[1], pol->BndHdrTab[pol->NmbBndHdr]);

   if (!ret)
      return (0);

   ret = GmfGetBlock(pol->MshIdx, GmfBoundaryPolygonVertices,
                     1, pol->NmbBndVer, 0, NULL, NULL, GmfInt,
                     &pol->BndVerTab[1], &pol->BndVerTab[pol->NmbBndVer]);

   if (!ret)
      return (0);

   return (1);
}

/*----------------------------------------------------------------------------*/
/* Read the polyhedra from an already opened mesh file                        */
/* and store the data in the provided polyhedral structure                    */
/*----------------------------------------------------------------------------*/

int GmfReadPolyhedra(PolMshSct *pol)
{
   int ret;

   if (!pol || !pol->InrHdrTab || !pol->InrVerTab || !pol->VolHdrTab || !pol->VolFacTab)
   {
      return (0);
   }

   ret = GmfGetBlock(pol->MshIdx, GmfInnerPolygonHeaders,
                     1, pol->NmbInrHdr, 0, NULL, NULL, GmfIntVec, 2,
                     pol->InrHdrTab[1], pol->InrHdrTab[pol->NmbInrHdr]);

   if (!ret)
      return (0);

   ret = GmfGetBlock(pol->MshIdx, GmfInnerPolygonVertices,
                     1, pol->NmbInrVer, 0, NULL, NULL, GmfInt,
                     &pol->InrVerTab[1], &pol->InrVerTab[pol->NmbInrVer]);

   if (!ret)
      return (0);

   ret = GmfGetBlock(pol->MshIdx, GmfPolyhedraHeaders,
                     1, pol->NmbVolHdr, 0, NULL, NULL, GmfIntVec, 2,
                     pol->VolHdrTab[1], pol->VolHdrTab[pol->NmbVolHdr]);

   if (!ret)
      return (0);

   ret = GmfGetBlock(pol->MshIdx, GmfPolyhedraFaces,
                     1, pol->NmbVolFac, 0, NULL, NULL, GmfInt,
                     &pol->VolFacTab[1], &pol->VolFacTab[pol->NmbVolFac]);

   if (!ret)
      return (0);

   return (1);
}

/*----------------------------------------------------------------------------*/
/* Copy one surface polygon list of vertices in the table and return its size */
/*----------------------------------------------------------------------------*/

itg GmfGetBoundaryPolygon(PolMshSct *pol, itg EleIdx, itg *UsrTab)
{
   itg i, BegIdx, EndIdx;

   if (!pol || (EleIdx <= 0) || (EleIdx > pol->NmbBndHdr))
      return (0);

   BegIdx = pol->BndHdrTab[EleIdx][0];

   if (EleIdx == pol->NmbBndHdr)
      EndIdx = pol->NmbBndVer;
   else
      EndIdx = pol->BndHdrTab[EleIdx + 1][0] - 1;

   EndIdx = MIN(EndIdx, BegIdx + 256);

   for (i = BegIdx; i <= EndIdx; i++)
      UsrTab[i - BegIdx] = pol->BndVerTab[i];

   return (EndIdx - BegIdx + 1);
}

/*----------------------------------------------------------------------------*/
/* Copy one inner polygon list of vertices in the table and return its size   */
/*----------------------------------------------------------------------------*/

itg GmfGetInnerPolygon(PolMshSct *pol, itg EleIdx, itg *UsrTab)
{
   itg i, BegIdx, EndIdx, ord = 1;

   if(EleIdx < 0)
   {
      EleIdx = -EleIdx;
      ord = -1;
   }

   if (!pol || (EleIdx <= 0) || (EleIdx > pol->NmbInrHdr))
      return (0);

   BegIdx = pol->InrHdrTab[EleIdx][0];

   if (EleIdx == pol->NmbInrHdr)
      EndIdx = pol->NmbInrVer;
   else
      EndIdx = pol->InrHdrTab[EleIdx + 1][0] - 1;

   EndIdx = MIN(EndIdx, BegIdx + 256);

   if(ord == 1)
      for (i = BegIdx; i <= EndIdx; i++)
         UsrTab[i - BegIdx] = pol->InrVerTab[i];
   else
      for (i = EndIdx; i >= BegIdx; i--)
         UsrTab[EndIdx - i] = pol->InrVerTab[i];

   return (EndIdx - BegIdx + 1);
}

/*----------------------------------------------------------------------------*/
/* Copy one polyhedra list of polygonal faces in the table and return its size*/
/*----------------------------------------------------------------------------*/

itg GmfGetPolyhedron(PolMshSct *pol, itg EleIdx, itg *UsrTab)
{
   itg i, BegIdx, EndIdx;

   if (!pol || (EleIdx <= 0) || (EleIdx > pol->NmbVolHdr))
      return (0);

   BegIdx = pol->VolHdrTab[EleIdx][0];

   if (EleIdx == pol->NmbVolHdr)
      EndIdx = pol->NmbVolFac;
   else
      EndIdx = pol->VolHdrTab[EleIdx + 1][0] - 1;

   EndIdx = MIN(EndIdx, BegIdx + 256);

   for (i = BegIdx; i <= EndIdx; i++)
      UsrTab[i - BegIdx] = pol->VolFacTab[i];

   return (EndIdx - BegIdx + 1);
}

/*----------------------------------------------------------------------------*/
/* Create the tessellation in triangles of a polygon                          */
/* Output: number of triangles, list of trangles and visibility of egdes      */
/*----------------------------------------------------------------------------*/

itg GmfTesselatePolygon(PolMshSct *pol, itg EleIdx, itg (*Tri)[3], itg (*VisEdg)[3])
{
   itg NmbTri, deg1, i, buf1[256];

   deg1 = GmfGetBoundaryPolygon(pol, EleIdx, buf1);

   NmbTri = deg1 - 2;

   //-- First triangle
   Tri[0][0] = buf1[0];
   Tri[0][1] = buf1[1];
   Tri[0][2] = buf1[2];
   VisEdg[0][0] = 1;
   VisEdg[0][1] = 0;
   VisEdg[0][2] = 1;

   //-- Other triangles
   for (i = 1; i < NmbTri - 1; i++)
   {
      Tri[i][0] = buf1[0];
      Tri[i][1] = buf1[i + 1];
      Tri[i][2] = buf1[i + 2];
      VisEdg[i][0] = 1;
      VisEdg[i][1] = 0;
      VisEdg[i][2] = 0;
   }

   //-- Last triangle
   Tri[NmbTri - 1][0] = buf1[0];
   Tri[NmbTri - 1][1] = buf1[deg1 - 2];
   Tri[NmbTri - 1][2] = buf1[deg1 - 1];
   VisEdg[NmbTri - 1][0] = 1;
   VisEdg[NmbTri - 1][1] = 1;
   VisEdg[NmbTri - 1][2] = 0;

   // for (i = 0; i < NmbTri; i++) {
   //   printf("GmfTesselatePolygon: Tri[%d] %d %d %d \n", i, Tri[i][0], Tri[i][1], Tri[i][2]);
   //   printf("GmfTesselatePolygon: VisEdg[%d] %d %d %d \n", i, VisEdg[i][0], VisEdg[i][1], VisEdg[i][2]);
   // }

   return NmbTri;
}


/*----------------------------------------------------------------------------*/
/* Return the percentage of cache hit while doing element to node accesses    */
/*----------------------------------------------------------------------------*/

float GmfEvaluateNumbering(int NmbEle, int NmbNod, int *PtrEle, int *PtrEnd)
{
   int i, j, EleStp, HshTab[256] = {0}, NmbHit = 0;

   if(!NmbEle || !NmbNod || !PtrEle || !PtrEnd || (PtrEnd < PtrEle))
      return(0.);

   // Compute the user data structure stride between two lines of elements
   EleStp = (NmbEle > 1) ? (PtrEnd - PtrEle) / (NmbEle - 1) : 0;

   // Simulate the search and insert in a 256 entries hash table
   for(i=0;i<NmbEle;i++)
   {
      // Get the next element's pointer on its nodes
      PtrEle += EleStp;

      // Search for each node in the hash table:
      // if its found, increase the hit counter and insert it otherwise
      for(j=0;j<NmbNod;j++)
         if(HshTab[ PtrEle[j] & 0xff ] == PtrEle[j])
            NmbHit++;
         else
            HshTab[ PtrEle[j] & 0xff ] = PtrEle[j];
   }

   // Return the percentage of cache hits
   return( (100. * NmbHit) / (NmbEle * NmbNod) );
}
