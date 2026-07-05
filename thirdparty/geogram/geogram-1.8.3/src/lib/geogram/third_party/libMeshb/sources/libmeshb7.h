

/*----------------------------------------------------------------------------*/
/*                                                                            */
/*                               LIBMESHB V7.61                               */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*    Description:        handle .meshb file format I/O                       */
/*    Author:             Loic MARECHAL                                       */
/*    Creation date:      dec 09 1999                                         */
/*    Last modification:  sep 27 2021                                         */
/*                                                                            */
/*----------------------------------------------------------------------------*/

// [Bruno] get PRINTF_INT64_MODIFIER
// #include <geogram/third_party/pstdint.h>

#include <stdint.h>

#ifndef LIBMESHB7_H
#define LIBMESHB7_H

/*----------------------------------------------------------------------------*/
/* Defines                                                                    */
/*----------------------------------------------------------------------------*/

#define GmfStrSiz    1024
#define GmfMaxTyp    1000
#define GmfMaxKwd    GmfLastKeyword - 1
#define GmfMshVer    1
#define GmfRead      1
#define GmfWrite     2
#define GmfSca       1
#define GmfVec       2
#define GmfSymMat    3
#define GmfMat       4
#define GmfFloat     1
#define GmfDouble    2
#define GmfInt       3
#define GmfLong      4
#define GmfFloatVec  5
#define GmfDoubleVec 6
#define GmfIntTab    7
#define GmfIntVec    7
#define GmfLongTab   8
#define GmfLongVec   8
#define GmfArgTab    100
#define GmfArgLst    101

enum GmfKwdCod
{
   GmfReserved1,
   GmfVersionFormatted,
   GmfReserved2,
   GmfDimension,
   GmfVertices,
   GmfEdges,
   GmfTriangles,
   GmfQuadrilaterals,
   GmfTetrahedra,
   GmfPrisms,
   GmfHexahedra,
   GmfReserved3,
   GmfReserved4,
   GmfCorners,
   GmfRidges,
   GmfRequiredVertices,
   GmfRequiredEdges,
   GmfRequiredTriangles,
   GmfRequiredQuadrilaterals,
   GmfTangentAtEdgeVertices,
   GmfNormalAtVertices,
   GmfNormalAtTriangleVertices,
   GmfNormalAtQuadrilateralVertices,
   GmfAngleOfCornerBound,
   GmfTrianglesP2,
   GmfEdgesP2,
   GmfSolAtPyramids,
   GmfQuadrilateralsQ2,
   GmfISolAtPyramids,
   GmfSubDomainFromGeom,
   GmfTetrahedraP2,
   GmfFault_NearTri,
   GmfFault_Inter,
   GmfHexahedraQ2,
   GmfExtraVerticesAtEdges,
   GmfExtraVerticesAtTriangles,
   GmfExtraVerticesAtQuadrilaterals,
   GmfExtraVerticesAtTetrahedra,
   GmfExtraVerticesAtPrisms,
   GmfExtraVerticesAtHexahedra,
   GmfVerticesOnGeometricVertices,
   GmfVerticesOnGeometricEdges,
   GmfVerticesOnGeometricTriangles,
   GmfVerticesOnGeometricQuadrilaterals,
   GmfEdgesOnGeometricEdges,
   GmfFault_FreeEdge,
   GmfPolyhedra,
   GmfPolygons,
   GmfFault_Overlap,
   GmfPyramids,
   GmfBoundingBox,
   GmfReserved5,
   GmfPrivateTable,
   GmfFault_BadShape,
   GmfEnd,
   GmfTrianglesOnGeometricTriangles,
   GmfTrianglesOnGeometricQuadrilaterals,
   GmfQuadrilateralsOnGeometricTriangles,
   GmfQuadrilateralsOnGeometricQuadrilaterals,
   GmfTangents,
   GmfNormals,
   GmfTangentAtVertices,
   GmfSolAtVertices,
   GmfSolAtEdges,
   GmfSolAtTriangles,
   GmfSolAtQuadrilaterals,
   GmfSolAtTetrahedra,
   GmfSolAtPrisms,
   GmfSolAtHexahedra,
   GmfDSolAtVertices,
   GmfISolAtVertices,
   GmfISolAtEdges,
   GmfISolAtTriangles,
   GmfISolAtQuadrilaterals,
   GmfISolAtTetrahedra,
   GmfISolAtPrisms,
   GmfISolAtHexahedra,
   GmfIterations,
   GmfTime,
   GmfFault_SmallTri,
   GmfCoarseHexahedra,
   GmfComments,
   GmfPeriodicVertices,
   GmfPeriodicEdges,
   GmfPeriodicTriangles,
   GmfPeriodicQuadrilaterals,
   GmfPrismsP2,
   GmfPyramidsP2,
   GmfQuadrilateralsQ3,
   GmfQuadrilateralsQ4,
   GmfTrianglesP3,
   GmfTrianglesP4,
   GmfEdgesP3,
   GmfEdgesP4,
   GmfIRefGroups,
   GmfDRefGroups,
   GmfTetrahedraP3,
   GmfTetrahedraP4,
   GmfHexahedraQ3,
   GmfHexahedraQ4,
   GmfPyramidsP3,
   GmfPyramidsP4,
   GmfPrismsP3,
   GmfPrismsP4,
   GmfHOSolAtEdgesP1,
   GmfHOSolAtEdgesP2,
   GmfHOSolAtEdgesP3,
   GmfHOSolAtTrianglesP1,
   GmfHOSolAtTrianglesP2,
   GmfHOSolAtTrianglesP3,
   GmfHOSolAtQuadrilateralsQ1,
   GmfHOSolAtQuadrilateralsQ2,
   GmfHOSolAtQuadrilateralsQ3,
   GmfHOSolAtTetrahedraP1,
   GmfHOSolAtTetrahedraP2,
   GmfHOSolAtTetrahedraP3,
   GmfHOSolAtPyramidsP1,
   GmfHOSolAtPyramidsP2,
   GmfHOSolAtPyramidsP3,
   GmfHOSolAtPrismsP1,
   GmfHOSolAtPrismsP2,
   GmfHOSolAtPrismsP3,
   GmfHOSolAtHexahedraQ1,
   GmfHOSolAtHexahedraQ2,
   GmfHOSolAtHexahedraQ3,
   GmfBezierBasis,
   GmfByteFlow,
   GmfEdgesP2Ordering,
   GmfEdgesP3Ordering,
   GmfTrianglesP2Ordering,
   GmfTrianglesP3Ordering,
   GmfQuadrilateralsQ2Ordering,
   GmfQuadrilateralsQ3Ordering,
   GmfTetrahedraP2Ordering,
   GmfTetrahedraP3Ordering,
   GmfPyramidsP2Ordering,
   GmfPyramidsP3Ordering,
   GmfPrismsP2Ordering,
   GmfPrismsP3Ordering,
   GmfHexahedraQ2Ordering,
   GmfHexahedraQ3Ordering,
   GmfEdgesP1Ordering,
   GmfEdgesP4Ordering,
   GmfTrianglesP1Ordering,
   GmfTrianglesP4Ordering,
   GmfQuadrilateralsQ1Ordering,
   GmfQuadrilateralsQ4Ordering,
   GmfTetrahedraP1Ordering,
   GmfTetrahedraP4Ordering,
   GmfPyramidsP1Ordering,
   GmfPyramidsP4Ordering,
   GmfPrismsP1Ordering,
   GmfPrismsP4Ordering,
   GmfHexahedraQ1Ordering,
   GmfHexahedraQ4Ordering,
   GmfFloatingPointPrecision,
   GmfHOSolAtEdgesP4,
   GmfHOSolAtTrianglesP4,
   GmfHOSolAtQuadrilateralsQ4,
   GmfHOSolAtTetrahedraP4,
   GmfHOSolAtPyramidsP4,
   GmfHOSolAtPrismsP4,
   GmfHOSolAtHexahedraQ4,
   GmfHOSolAtEdgesP1NodesPositions,
   GmfHOSolAtEdgesP2NodesPositions,
   GmfHOSolAtEdgesP3NodesPositions,
   GmfHOSolAtEdgesP4NodesPositions,
   GmfHOSolAtTrianglesP1NodesPositions,
   GmfHOSolAtTrianglesP2NodesPositions,
   GmfHOSolAtTrianglesP3NodesPositions,
   GmfHOSolAtTrianglesP4NodesPositions,
   GmfHOSolAtQuadrilateralsQ1NodesPositions,
   GmfHOSolAtQuadrilateralsQ2NodesPositions,
   GmfHOSolAtQuadrilateralsQ3NodesPositions,
   GmfHOSolAtQuadrilateralsQ4NodesPositions,
   GmfHOSolAtTetrahedraP1NodesPositions,
   GmfHOSolAtTetrahedraP2NodesPositions,
   GmfHOSolAtTetrahedraP3NodesPositions,
   GmfHOSolAtTetrahedraP4NodesPositions,
   GmfHOSolAtPyramidsP1NodesPositions,
   GmfHOSolAtPyramidsP2NodesPositions,
   GmfHOSolAtPyramidsP3NodesPositions,
   GmfHOSolAtPyramidsP4NodesPositions,
   GmfHOSolAtPrismsP1NodesPositions,
   GmfHOSolAtPrismsP2NodesPositions,
   GmfHOSolAtPrismsP3NodesPositions,
   GmfHOSolAtPrismsP4NodesPositions,
   GmfHOSolAtHexahedraQ1NodesPositions,
   GmfHOSolAtHexahedraQ2NodesPositions,
   GmfHOSolAtHexahedraQ3NodesPositions,
   GmfHOSolAtHexahedraQ4NodesPositions,
   GmfEdgesReferenceElement,
   GmfTriangleReferenceElement,
   GmfQuadrilateralReferenceElement,
   GmfTetrahedronReferenceElement,
   GmfPyramidReferenceElement,
   GmfPrismReferenceElement,
   GmfHexahedronReferenceElement,
   GmfBoundaryLayers,
   GmfReferenceStrings,
   GmfPrisms9,
   GmfHexahedra12,
   GmfQuadrilaterals6,
   GmfBoundaryPolygonHeaders,
   GmfBoundaryPolygonVertices,
   GmfInnerPolygonHeaders,
   GmfInnerPolygonVertices,
   GmfPolyhedraHeaders,
   GmfPolyhedraFaces,
   GmfDomains,
   GmfVerticesGID,
   GmfEdgesGID,
   GmfTrianglesGID,
   GmfQuadrilateralsGID,
   GmfTetrahedraGID,
   GmfPyramidsGID,
   GmfPrismsGID,
   GmfHexahedraGID,
   GmfLastKeyword
};


#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------*/
/* Public procedures                                                          */
/*----------------------------------------------------------------------------*/

extern int64_t GmfOpenMesh          (const char *, int, ...);
extern int     GmfCloseMesh         (int64_t);
extern int64_t GmfStatKwd           (int64_t, int, ...);
extern int     GmfSetKwd            (int64_t, int, int64_t, ...);
extern int     GmfGotoKwd           (int64_t, int);
extern int     GmfGetLin            (int64_t, int, ...);
extern int     GmfSetLin            (int64_t, int, ...);
extern int     GmfGetBlock          (int64_t, int, int64_t, int64_t, int, void *, void *, ...);
extern int     GmfSetBlock          (int64_t, int, int64_t, int64_t, int, void *, void *, ...);
extern char   *GmfReadByteFlow      (int64_t, int *);
extern int     GmfWriteByteFlow     (int64_t, char *, int);
extern int     GmfSetHONodesOrdering(int64_t, int, int *, int *);
extern int     GmfGetFloatPrecision (int64_t);
extern void    GmfSetFloatPrecision (int64_t, int);


/*----------------------------------------------------------------------------*/
/* Transmesh private API                                                      */
/*----------------------------------------------------------------------------*/

#ifdef TRANSMESH
extern int GmfMaxRefTab[ GmfMaxKwd + 1 ];
extern const char *GmfKwdFmt[ GmfMaxKwd + 1 ][3];
extern int GmfCpyLin(int64_t, int64_t, int);
#endif

#ifdef __cplusplus
} // end extern "C"
#endif

#endif
