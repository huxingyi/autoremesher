

/*----------------------------------------------------------*/
/*                                                          */
/*                        LIBMESH V 7.12                    */
/*                                                          */
/*----------------------------------------------------------*/
/*                                                          */
/*    Description:        handle .meshb file format I/O     */
/*    Author:             Loic MARECHAL                     */
/*    Creation date:      dec 08 2015                       */
/*    Last modification:  may 20 2015                       */
/*                                                          */
/*----------------------------------------------------------*/

/* [Bruno] get int64_t and PRINTF_INT64_MODIFIER */
#include <geogram/third_party/pstdint.h> 

/*----------------------------------------------------------*/
/* Defines                                                  */
/*----------------------------------------------------------*/

#define GmfStrSiz 1024
#define GmfMaxTyp 1000
#define GmfMaxKwd GmfLastKeyword - 1
#define GmfMshVer 1
#define GmfRead 1
#define GmfWrite 2
#define GmfSca 1
#define GmfVec 2
#define GmfSymMat 3
#define GmfMat 4
#define GmfFloat 1
#define GmfDouble 2
#define GmfInt 3
#define GmfLong 4
#define GmfDoubleTable 5
#define GmfFloatTable 6

enum GmfKwdCod
{
    GmfReserved1, \
    GmfVersionFormatted, \
    GmfReserved2, \
    GmfDimension, \
    GmfVertices, \
    GmfEdges, \
    GmfTriangles, \
    GmfQuadrilaterals, \
    GmfTetrahedra, \
    GmfPrisms, \
    GmfHexahedra, \
    GmfIterationsAll, \
    GmfTimesAll, \
    GmfCorners, \
    GmfRidges, \
    GmfRequiredVertices, \
    GmfRequiredEdges, \
    GmfRequiredTriangles, \
    GmfRequiredQuadrilaterals, \
    GmfTangentAtEdgeVertices, \
    GmfNormalAtVertices, \
    GmfNormalAtTriangleVertices, \
    GmfNormalAtQuadrilateralVertices, \
    GmfAngleOfCornerBound, \
    GmfTrianglesP2, \
    GmfEdgesP2, \
    GmfSolAtPyramids, \
    GmfQuadrilateralsQ2, \
    GmfISolAtPyramids, \
    GmfSubDomainFromGeom, \
    GmfTetrahedraP2, \
    GmfFault_NearTri, \
    GmfFault_Inter, \
    GmfHexahedraQ2, \
    GmfExtraVerticesAtEdges, \
    GmfExtraVerticesAtTriangles, \
    GmfExtraVerticesAtQuadrilaterals, \
    GmfExtraVerticesAtTetrahedra, \
    GmfExtraVerticesAtPrisms, \
    GmfExtraVerticesAtHexahedra, \
    GmfVerticesOnGeometricVertices, \
    GmfVerticesOnGeometricEdges, \
    GmfVerticesOnGeometricTriangles, \
    GmfVerticesOnGeometricQuadrilaterals, \
    GmfEdgesOnGeometricEdges, \
    GmfFault_FreeEdge, \
    GmfPolyhedra, \
    GmfPolygons, \
    GmfFault_Overlap, \
    GmfPyramids, \
    GmfBoundingBox, \
    GmfBody, \
    GmfPrivateTable, \
    GmfFault_BadShape, \
    GmfEnd, \
    GmfTrianglesOnGeometricTriangles, \
    GmfTrianglesOnGeometricQuadrilaterals, \
    GmfQuadrilateralsOnGeometricTriangles, \
    GmfQuadrilateralsOnGeometricQuadrilaterals, \
    GmfTangents, \
    GmfNormals, \
    GmfTangentAtVertices, \
    GmfSolAtVertices, \
    GmfSolAtEdges, \
    GmfSolAtTriangles, \
    GmfSolAtQuadrilaterals, \
    GmfSolAtTetrahedra, \
    GmfSolAtPrisms, \
    GmfSolAtHexahedra, \
    GmfDSolAtVertices, \
    GmfISolAtVertices, \
    GmfISolAtEdges, \
    GmfISolAtTriangles, \
    GmfISolAtQuadrilaterals, \
    GmfISolAtTetrahedra, \
    GmfISolAtPrisms, \
    GmfISolAtHexahedra, \
    GmfIterations, \
    GmfTime, \
    GmfFault_SmallTri, \
    GmfCoarseHexahedra, \
    GmfComments, \
    GmfPeriodicVertices, \
    GmfPeriodicEdges, \
    GmfPeriodicTriangles, \
    GmfPeriodicQuadrilaterals, \
    GmfPrismsP2, \
    GmfPyramidsP2, \
    GmfQuadrilateralsQ3, \
    GmfQuadrilateralsQ4, \
    GmfTrianglesP3, \
    GmfTrianglesP4, \
    GmfEdgesP3, \
    GmfEdgesP4, \
    GmfIRefGroups, \
    GmfDRefGroups, \
    GmfLastKeyword
};


/*----------------------------------------------------------*/
/* Public procedures                                        */
/*----------------------------------------------------------*/

extern int64_t   GmfOpenMesh(char *, int, ...);
extern int       GmfCloseMesh(int64_t);
extern int64_t   GmfStatKwd(int64_t, int, ...);
extern int       GmfSetKwd(int64_t, int, ...);
extern int       GmfGotoKwd(int64_t, int);
extern int       GmfGetLin(int64_t, int, ...);
extern int       GmfSetLin(int64_t, int, ...);
extern int       GmfGetBlock(int64_t, int, void *, ...);
extern int       GmfSetBlock(int64_t, int, void *, ...);


/*----------------------------------------------------------*/
/* Transmesh private API                                    */
/*----------------------------------------------------------*/

#ifdef TRANSMESH

extern int GmfMaxRefTab[ GmfMaxKwd + 1 ];
extern const char *GmfKwdFmt[ GmfMaxKwd + 1 ][4];
extern int GmfCpyLin(int64_t, int64_t, int);

#endif
