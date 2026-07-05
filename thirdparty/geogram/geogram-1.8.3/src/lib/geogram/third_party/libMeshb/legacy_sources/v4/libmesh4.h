

/*----------------------------------------------------------*/
/*															*/
/*						LIBMESH V 4.0						*/
/*															*/
/*----------------------------------------------------------*/
/*															*/
/*	Description:		handle .meshb file format I/O		*/
/*	Author:				Loic MARECHAL						*/
/*	Creation date:		aug 02 2003							*/
/*	Last modification:	jan 25 2006							*/
/*															*/
/*----------------------------------------------------------*/


/*----------------------------------------------------------*/
/* Includes													*/
/*----------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>


/*----------------------------------------------------------*/
/* Defines													*/
/*----------------------------------------------------------*/

#define GmfStrSiz 1024
#define GmfMaxTyp 20
#define GmfMaxKwd 79
#define GmfMshVer 1
#define GmfRead 1
#define GmfWrite 2
#define GmfSca 1
#define GmfVec 2
#define GmfSymMat 3
#define GmfMat 4

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
	GmfPentahedra, \
	GmfHexahedra, \
	GmfReserved3, \
	GmfReserved4, \
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
	GmfReserved5, \
	GmfReserved6, \
	GmfReserved7, \
	GmfReserved8, \
	GmfReserved9, \
	GmfReserved10, \
	GmfReserved11, \
	GmfReserved12, \
	GmfReserved13, \
	GmfReserved14, \
	GmfReserved15, \
	GmfReserved16, \
	GmfReserved17, \
	GmfReserved18, \
	GmfReserved19, \
	GmfReserved20, \
	GmfReserved21, \
	GmfReserved22, \
	GmfReserved23, \
	GmfReserved24, \
	GmfReserved25, \
	GmfReserved26, \
	GmfReserved27, \
	GmfReserved28, \
	GmfReserved29, \
	GmfReserved30, \
	GmfBoundingBox, \
	GmfReserved31, \
	GmfReserved32, \
	GmfReserved33, \
	GmfEnd, \
	GmfReserved34, \
	GmfReserved35, \
	GmfReserved36, \
	GmfReserved37, \
	GmfTangents, \
	GmfNormals, \
	GmfTangentAtVertices, \
	GmfSolAtVertices, \
	GmfSolAtEdges, \
	GmfSolAtTriangles, \
	GmfSolAtQuadrilaterals, \
	GmfSolAtTetrahedra, \
	GmfSolAtPentahedra, \
	GmfSolAtHexahedra, \
	GmfDSolAtVertices, \
	GmfISolAtVertices, \
	GmfISolAtEdges, \
	GmfISolAtTriangles, \
	GmfISolAtQuadrilaterals, \
	GmfISolAtTetrahedra, \
	GmfISolAtPentahedra, \
	GmfISolAtHexahedra, \
	GmfIterations, \
	GmfTime, \
	GmfVertexHack
};

/********************************************************************/
/*  SI VOTRE COMPILATEUR FORTRAN PREFERE GENERE UN _ APRES          */
/*  LES NOMS DES SUBROUTINES, ALORS PRENEZ CETTE DEFINITION DE proc */
/* #define call(x)  name2(x,_)                                      */
/*  SINON PRENEZ CELLE CI                                           */
/* #define call(x)  x                                               */
/********************************************************************/

#ifndef _UNDERSCORE_H_
#define _UNDERSCORE_H_

#if defined(__ANSI_CPP__) || defined(__linux__) || defined(_HPUX_SOURCE) || defined(__macos__)
#  define name2(a,b) a ## b
#else
#  ifdef BSD  /* BSD way: ok pour dn10000 */
#    define name2(a,b) a\
b

#  else /* System V way: */
#    define name2(a,b) a/**/b
#  endif 
#endif

#define name22(a,b) name2(a,b)

#ifdef F77_NO_UNDER_SCORE 
#  define call(x) x
#else
#  if defined(__linux__)  || defined(__macos__)
#    define call(x) name2(x,_)
#  else
#    define call(x) x
#  endif
#endif

#endif


/*----------------------------------------------------------*/
/* Structures												*/
/*----------------------------------------------------------*/

typedef struct
{
	/* Public */
	int SolSiz, NmbLin, NmbTyp, TypTab[ GmfMaxTyp ];

	/* Private */
	size_t pos;
	int CurLin, typ;
	char fmt[ GmfMaxTyp ];
}KwdSct;

typedef struct
{
	/* Public */

	int dim, ver, iter;
	float angle, bbox[3][2], time;
	KwdSct KwdTab[ GmfMaxKwd + 1 ];

	/* Private */

	int mod, typ, cod;
	FILE *hdl;
	char FilNam[ GmfStrSiz ];
}GmfMshSct;


/*----------------------------------------------------------*/
/* External procedures										*/
/*----------------------------------------------------------*/

extern GmfMshSct *GmfOpenMesh(char *, int, ...);
extern int GmfCloseMesh(GmfMshSct *);
extern int GmfReadField(GmfMshSct *, int, float *, int *);
extern int GmfWriteField(GmfMshSct *, int, int, float *, int *, ...);
extern int GmfReadLine(GmfMshSct *, int, ...);
extern int GmfWriteLine(GmfMshSct *, int, ...);

/* Fortran 77 API */

int call(gmfopenmeshf77)(char *, int *, int *, int [ GmfMaxKwd ][ GmfMaxTyp + 2 ], int);
int call(gmfclosemeshf77)(int *);
int call(gmfreadfieldf77)(int *, int *, float *, int *);
int call(gmfwritefieldf77)(int *, int *, int *, int *, int *, float *, int *);
