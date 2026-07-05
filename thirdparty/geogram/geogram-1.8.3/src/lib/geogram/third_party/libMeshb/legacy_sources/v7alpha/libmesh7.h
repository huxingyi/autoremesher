

/*----------------------------------------------------------*/
/*															*/
/*						LIBMESH V 7.0						*/
/*															*/
/*----------------------------------------------------------*/
/*															*/
/*	Description:		handle .meshb file format I/O		*/
/*	Author:				Loic MARECHAL						*/
/*	Creation date:		dec 08 2015							*/
/*	Last modification:	jan 19 2015							*/
/*															*/
/*----------------------------------------------------------*/


/*----------------------------------------------------------*/
/* Defines													*/
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
	GmfLastKeyword
};


/*----------------------------------------------------------*/
/* Public procedures										*/
/*----------------------------------------------------------*/

extern long long GmfOpenMesh(char *, int, ...);
extern int       GmfCloseMesh(long long);
extern long long GmfStatKwd(long long, int, ...);
extern int       GmfSetKwd(long long, int, ...);
extern int       GmfGotoKwd(long long, int);
extern int       GmfGetLin(long long, int, ...);
extern int       GmfSetLin(long long, int, ...);
extern int       GmfGetBlock(long long, int, void *, ...);
extern int       GmfSetBlock(long long, int, void *, ...);


/*----------------------------------------------------------*/
/* Transmesh private API									*/
/*----------------------------------------------------------*/

#ifdef TRANSMESH

extern char *GmfKwdFmt[ GmfMaxKwd + 1 ][4];
extern int GmfCpyLin(int, int, int);

#endif
