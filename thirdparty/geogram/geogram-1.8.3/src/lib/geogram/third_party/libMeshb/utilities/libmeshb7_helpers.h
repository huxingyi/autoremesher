

/*----------------------------------------------------------------------------*/
/*                                                                            */
/*                            LIBMESHB-HELPERS V0.91                          */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Description:         libmeshb's helper functions' headers                  */
/* Author:              Loic MARECHAL                                         */
/* Creation date:       mar 24 2021                                           */
/* Last modification:   nov 05 2021                                           */
/*                                                                            */
/*----------------------------------------------------------------------------*/


#ifndef LIBMESHB7_HELPERS_H
#define LIBMESHB7_HELPERS_H


/*----------------------------------------------------------------------------*/
/* Defines                                                                    */
/*----------------------------------------------------------------------------*/

#include <stdint.h>

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
/* Prototypes of public structures                                            */
/*----------------------------------------------------------------------------*/

typedef struct
{
   int64_t MshIdx;
   itg NmbBndHdr, (*BndHdrTab)[2], NmbBndVer, *BndVerTab;
   itg NmbInrHdr, (*InrHdrTab)[2], NmbInrVer, *InrVerTab;
   itg NmbVolHdr, (*VolHdrTab)[2], NmbVolFac, *VolFacTab;
}PolMshSct;

/*----------------------------------------------------------------------------*/
/* Prototypes of public procedures                                            */
/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

extern PolMshSct *GmfAllocatePolyhedralStructure(int64_t);
extern void       GmfFreePolyghedralStructure   (PolMshSct *);
extern int        GmfReadBoundaryPolygons       (PolMshSct *);
extern int        GmfReadPolyhedra              (PolMshSct *);
extern itg        GmfGetBoundaryPolygon         (PolMshSct *, itg, itg *);
extern itg        GmfGetInnerPolygon            (PolMshSct *, itg, itg *);
extern itg        GmfGetPolyhedron              (PolMshSct *, itg, itg *);
extern itg        GmfTesselatePolygon           (PolMshSct *, itg, itg (*)[3], itg (*)[3]);
extern float      GmfEvaluateNumbering          (int, int, int *, int *);

#ifdef __cplusplus
} // end extern "C"
#endif

#endif
