

/*----------------------------------------------------------*/
/*															*/
/*						LIBMESH V 6.05						*/
/*															*/
/*----------------------------------------------------------*/
/*															*/
/*	Description:		handle .meshb file format I/O		*/
/*	Author:				Loic MARECHAL						*/
/*	Creation date:		feb 16 2007							*/
/*	Last modification:	jan 18 2016							*/
/*															*/
/*----------------------------------------------------------*/


/*----------------------------------------------------------*/
/* Includes													*/
/*----------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <ctype.h>
#include <setjmp.h>
#include "libmesh6.h"


/*----------------------------------------------------------*/
/* Defines													*/
/*----------------------------------------------------------*/

#define Asc 1
#define Bin 2
#define MshFil 4
#define SolFil 8
#define MaxMsh 100
#define InfKwd 1
#define RegKwd 2
#define SolKwd 3
#define CmtKwd 4
#define WrdSiz 4
#define FilStrSiz 64
#define BufSiz 10000


/*----------------------------------------------------------*/
/* Fortran 77 API											*/
/*----------------------------------------------------------*/

#if defined(F77_NO_UNDER_SCORE)
#define call(x) x
#else
#define call(x) x ## _
#endif

#define ARG(a,n) ARG ## n (a)
#define ARG1(a) a[0]
#define ARG2(a) ARG1(a),a[1]
#define ARG3(a) ARG2(a),a[2]
#define ARG4(a) ARG3(a),a[3]
#define ARG5(a) ARG4(a),a[4]
#define ARG6(a) ARG5(a),a[5]
#define ARG7(a) ARG6(a),a[6]
#define ARG8(a) ARG7(a),a[7]
#define ARG9(a) ARG8(a),a[8]
#define ARG10(a) ARG9(a),a[9]
#define ARG11(a) ARG10(a),a[10]
#define ARG12(a) ARG11(a),a[11]
#define ARG13(a) ARG12(a),a[12]
#define ARG14(a) ARG13(a),a[13]
#define ARG15(a) ARG14(a),a[14]
#define ARG16(a) ARG15(a),a[15]
#define ARG17(a) ARG16(a),a[16]
#define ARG18(a) ARG17(a),a[17]
#define ARG19(a) ARG18(a),a[18]
#define ARG20(a) ARG19(a),a[19]

#define TARG(a,b,c,n) TARG ## n (a,b,c)
#define TARG1(a,b,c) a[0],b[0],c[0]
#define TARG2(a,b,c) TARG1(a,b,c),a[1],b[1],c[1]
#define TARG3(a,b,c) TARG2(a,b,c),a[2],b[2],c[2]
#define TARG4(a,b,c) TARG3(a,b,c),a[3],b[3],c[3]
#define TARG5(a,b,c) TARG4(a,b,c),a[4],b[4],c[4]
#define TARG6(a,b,c) TARG5(a,b,c),a[5],b[5],c[5]
#define TARG7(a,b,c) TARG6(a,b,c),a[6],b[6],c[6]
#define TARG8(a,b,c) TARG7(a,b,c),a[7],b[7],c[7]
#define TARG9(a,b,c) TARG8(a,b,c),a[8],b[8],c[8]
#define TARG10(a,b,c) TARG9(a,b,c),a[9],b[9],c[9]
#define TARG11(a,b,c) TARG10(a,b,c),a[10],b[10],c[10]
#define TARG12(a,b,c) TARG11(a,b,c),a[11],b[11],c[11]
#define TARG13(a,b,c) TARG12(a,b,c),a[12],b[12],c[12]
#define TARG14(a,b,c) TARG13(a,b,c),a[13],b[13],c[13]
#define TARG15(a,b,c) TARG14(a,b,c),a[14],b[14],c[14]
#define TARG16(a,b,c) TARG15(a,b,c),a[15],b[15],c[15]
#define TARG17(a,b,c) TARG16(a,b,c),a[16],b[16],c[16]
#define TARG18(a,b,c) TARG17(a,b,c),a[17],b[17],c[17]
#define TARG19(a,b,c) TARG18(a,b,c),a[18],b[18],c[18]
#define TARG20(a,b,c) TARG19(a,b,c),a[19],b[19],c[19]


/*----------------------------------------------------------*/
/* Structures												*/
/*----------------------------------------------------------*/

typedef struct
{
	int typ, SolSiz, NmbWrd, NmbTyp, TypTab[ GmfMaxTyp ];
	long NmbLin;
	long pos;
	char fmt[ GmfMaxTyp*9 ];
}KwdSct;

typedef struct
{
	int dim, ver, mod, typ, cod;
	long NexKwdPos, siz, pos;
	KwdSct KwdTab[ GmfMaxKwd + 1 ];
	FILE *hdl;
	int *IntBuf;
	float *FltBuf;
	char *buf;
	char FilNam[ GmfStrSiz ];
	double DblBuf[1000/8];
	unsigned char blk[ BufSiz + 1000 ];
}GmfMshSct;


/*----------------------------------------------------------*/
/* Global variables											*/
/*----------------------------------------------------------*/

static jmp_buf GmfEnv;
static int GmfIniFlg=0;
static GmfMshSct *GmfMshTab[ MaxMsh + 1 ];
const char *GmfKwdFmt[ GmfMaxKwd + 1 ][4] = 
{	{"Reserved", "", "", ""},
	{"MeshVersionFormatted", "", "", "i"},
	{"Reserved", "", "", ""},
	{"Dimension", "", "", "i"},
	{"Vertices", "Vertex", "i", "dri"},
	{"Edges", "Edge", "i", "iii"},
	{"Triangles", "Triangle", "i", "iiii"},
	{"Quadrilaterals", "Quadrilateral", "i", "iiiii"},
	{"Tetrahedra", "Tetrahedron", "i", "iiiii"},
	{"Prisms", "Prism", "i", "iiiiiii"},
	{"Hexahedra", "Hexahedron", "i", "iiiiiiiii"},
	{"IterationsAll", "IterationAll","","i"},
	{"TimesAll", "TimeAll","","r"},					
	{"Corners", "Corner", "i", "i"},
	{"Ridges", "Ridge", "i", "i"},
	{"RequiredVertices", "RequiredVertex", "i", "i"},
	{"RequiredEdges", "RequiredEdge", "i", "i"},
	{"RequiredTriangles", "RequiredTriangle", "i", "i"},
	{"RequiredQuadrilaterals", "RequiredQuadrilateral", "i", "i"},
	{"TangentAtEdgeVertices", "TangentAtEdgeVertex", "i", "iii"},
	{"NormalAtVertices", "NormalAtVertex", "i", "ii"},
	{"NormalAtTriangleVertices", "NormalAtTriangleVertex", "i", "iii"},
	{"NormalAtQuadrilateralVertices", "NormalAtQuadrilateralVertex", "i", "iiii"},
	{"AngleOfCornerBound", "", "", "r"},
	{"TrianglesP2", "TriangleP2", "i", "iiiiiii"},
	{"EdgesP2", "EdgeP2", "i", "iiii"},
	{"SolAtPyramids", "SolAtPyramid", "i", "sr"},
	{"QuadrilateralsQ2", "QuadrilateralQ2", "i", "iiiiiiiiii"},
	{"ISolAtPyramids", "ISolAtPyramid", "i", "iiiii"},
	{"SubDomainFromGeom", "SubDomainFromGeom", "i", "iii"},
	{"TetrahedraP2", "TetrahedronP2", "i", "iiiiiiiiiii"},
	{"Fault_NearTri", "Fault_NearTri", "i", "i"},
	{"Fault_Inter", "Fault_Inter", "i", "i"},
	{"HexahedraQ2", "HexahedronQ2", "i", "iiiiiiiiiiiiiiiiiiiiiiiiiiii"},
	{"ExtraVerticesAtEdges", "ExtraVerticesAtEdge", "i", "in"},
	{"ExtraVerticesAtTriangles", "ExtraVerticesAtTriangle", "i", "in"},
	{"ExtraVerticesAtQuadrilaterals", "ExtraVerticesAtQuadrilateral", "i", "in"},
	{"ExtraVerticesAtTetrahedra", "ExtraVerticesAtTetrahedron", "i", "in"},
	{"ExtraVerticesAtPrisms", "ExtraVerticesAtPrism", "i", "in"},
	{"ExtraVerticesAtHexahedra", "ExtraVerticesAtHexahedron", "i", "in"},
	{"VerticesOnGeometricVertices", "VertexOnGeometricVertex", "i", "iir"},
	{"VerticesOnGeometricEdges", "VertexOnGeometricEdge", "i", "iirr"},
	{"VerticesOnGeometricTriangles", "VertexOnGeometricTriangle", "i", "iirrr"},
	{"VerticesOnGeometricQuadrilaterals", "VertexOnGeometricQuadrilateral", "i", "iirrr"},
	{"EdgesOnGeometricEdges", "EdgeOnGeometricEdge", "i", "iir"},
	{"Fault_FreeEdge", "Fault_FreeEdge", "i", "i"},
	{"Polyhedra", "Polyhedron", "i", "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"},
	{"Polygons", "Polygon", "", "iiiiiiiii"},
	{"Fault_Overlap", "Fault_Overlap", "i", "i"},
	{"Pyramids", "Pyramid", "i", "iiiiii"},
	{"BoundingBox", "", "", "drdr"},
	{"Body","i", "drdrdrdr"},
	{"PrivateTable", "PrivateTable", "i", "i"},
	{"Fault_BadShape", "Fault_BadShape", "i", "i"},
	{"End", "", "", ""},
	{"TrianglesOnGeometricTriangles", "TriangleOnGeometricTriangle", "i", "iir"},
	{"TrianglesOnGeometricQuadrilaterals", "TriangleOnGeometricQuadrilateral", "i", "iir"},
	{"QuadrilateralsOnGeometricTriangles", "QuadrilateralOnGeometricTriangle", "i", "iir"},
	{"QuadrilateralsOnGeometricQuadrilaterals", "QuadrilateralOnGeometricQuadrilateral", "i", "iir"},
	{"Tangents", "Tangent", "i", "dr"},
	{"Normals", "Normal", "i", "dr"},
	{"TangentAtVertices", "TangentAtVertex", "i", "ii"},
	{"SolAtVertices", "SolAtVertex", "i", "sr"},
	{"SolAtEdges", "SolAtEdge", "i", "sr"},
	{"SolAtTriangles", "SolAtTriangle", "i", "sr"},
	{"SolAtQuadrilaterals", "SolAtQuadrilateral", "i", "sr"},
	{"SolAtTetrahedra", "SolAtTetrahedron", "i", "sr"},
	{"SolAtPrisms", "SolAtPrism", "i", "sr"},
	{"SolAtHexahedra", "SolAtHexahedron", "i", "sr"},
	{"DSolAtVertices", "DSolAtVertex", "i", "sr"},
	{"ISolAtVertices", "ISolAtVertex", "i", "i"},
	{"ISolAtEdges", "ISolAtEdge", "i", "ii"},
	{"ISolAtTriangles", "ISolAtTriangle", "i", "iii"},
	{"ISolAtQuadrilaterals", "ISolAtQuadrilateral", "i", "iiii"},
	{"ISolAtTetrahedra", "ISolAtTetrahedron", "i", "iiii"},
	{"ISolAtPrisms", "ISolAtPrism", "i", "iiiiii"},
	{"ISolAtHexahedra", "ISolAtHexahedron", "i", "iiiiiiii"},
	{"Iterations", "","","i"},
	{"Time", "","","r"},
	{"Fault_SmallTri", "Fault_SmallTri","i","i"},
	{"CoarseHexahedra", "CoarseHexahedron", "i", "i"},
	{"Comments", "Comment", "i", "c"},
	{"PeriodicVertices", "PeriodicVertex", "i", "ii"},
	{"PeriodicEdges", "PeriodicEdge", "i", "ii"},
	{"PeriodicTriangles", "PeriodicTriangle", "i", "ii"},
	{"PeriodicQuadrilaterals", "PeriodicQuadrilateral", "i", "ii"},
	{"PrismsP2", "PrismP2", "i", "iiiiiiiiiiiiiiiiiii"},
	{"PyramidsP2", "PyramidP2", "i", "iiiiiiiiiiiiiii"},
	{"QuadrilateralsQ3", "QuadrilateralQ3", "i", "iiiiiiiiiiiiiiiii"},
	{"QuadrilateralsQ4", "QuadrilateralQ4", "i", "iiiiiiiiiiiiiiiiiiiiiiiiii"},
	{"TrianglesP3", "TriangleP3", "i", "iiiiiiiiiii"},
	{"TrianglesP4", "TriangleP4", "i", "iiiiiiiiiiiiiiii"},
	{"EdgesP3", "EdgeP3", "i", "iiiii"},
	{"EdgesP4", "EdgeP4", "i", "iiiiii"}
 };


/*----------------------------------------------------------*/
/* Prototypes of local procedures							*/
/*----------------------------------------------------------*/

static void ScaWrd(GmfMshSct *, void *);
static void ScaDblWrd(GmfMshSct *, void *);
static long GetPos(GmfMshSct *);
static void RecWrd(GmfMshSct *, const void *);
static void RecDblWrd(GmfMshSct *, const void *);
static void RecBlk(GmfMshSct *, const void *, int);
static void SetPos(GmfMshSct *, long);
static int ScaKwdTab(GmfMshSct *);
static void ExpFmt(GmfMshSct *, int);
static void ScaKwdHdr(GmfMshSct *, int);
static void SwpWrd(char *, int);

#define safe_fscanf(hdl, format, ptr) \
	do { \
		if( fscanf(hdl, format, ptr) != 1 ) \
			longjmp(GmfEnv, -1); \
	} while(0)


#define safe_fgets(ptr, siz, hdl) \
	do { \
		if( fgets(ptr, siz, hdl) == NULL ) \
			longjmp(GmfEnv, -1); \
	} while(0)


/*----------------------------------------------------------*/
/* Open a mesh file in read or write mod					*/
/*----------------------------------------------------------*/

int GmfOpenMesh(char *FilNam, int mod, ...)
{
	int i, KwdCod, res, *PtrVer, *PtrDim, MshIdx=0;
	char str[ GmfStrSiz ];
	va_list VarArg;
	GmfMshSct *msh;

	if(!GmfIniFlg)
	{
		for(i=0;i<=MaxMsh;i++)
			GmfMshTab[i] = NULL;

		GmfIniFlg = 1;
	}

	/*---------------------*/
	/* MESH STRUCTURE INIT */
	/*---------------------*/

	for(i=1;i<=MaxMsh;i++)
		if(!GmfMshTab[i])
		{
			MshIdx = i;
			break;
		}

	if( !MshIdx || !(msh = calloc(1, sizeof(GmfMshSct))) )
		return(0);

	/* Save the current stack environment for longjmp */

	if(setjmp(GmfEnv) != 0)
	{
		if(msh->hdl != NULL)
			fclose(msh->hdl);
		free(msh);
		return(0);
	}

	/* Copy the FilNam into the structure */

	if(strlen(FilNam) + 7 >= GmfStrSiz)
		longjmp(GmfEnv, -1);

	strcpy(msh->FilNam, FilNam);

	/* Store the opening mod (read or write) and guess the filetype (binary or ascii) depending on the extension */

	msh->mod = mod;
	msh->buf = (void *)msh->DblBuf;
	msh->FltBuf = (void *)msh->DblBuf;
	msh->IntBuf = (void *)msh->DblBuf;

	if(strstr(msh->FilNam, ".meshb"))
		msh->typ |= (Bin | MshFil);
	else if(strstr(msh->FilNam, ".mesh"))
		msh->typ |= (Asc | MshFil);
	else if(strstr(msh->FilNam, ".solb"))
		msh->typ |= (Bin | SolFil);
	else if(strstr(msh->FilNam, ".sol"))
		msh->typ |= (Asc | SolFil);
	else
		longjmp(GmfEnv, -1);

	/* Open the file in the required mod and initialyse the mesh structure */

	if(msh->mod == GmfRead)
	{

		/*-----------------------*/
		/* OPEN FILE FOR READING */
		/*-----------------------*/

		va_start(VarArg, mod);
		PtrVer = va_arg(VarArg, int *);
		PtrDim = va_arg(VarArg, int *);
		va_end(VarArg);

		/* Create the name string and open the file */

		if(!(msh->hdl = fopen(msh->FilNam, "rb")))
			longjmp(GmfEnv, -1);

		/* Read the endian coding tag, the mesh version and the mesh dimension (mandatory kwd) */

		if(msh->typ & Bin)
		{
			if( fread(&msh->cod, WrdSiz, 1, msh->hdl) != 1 )
				longjmp(GmfEnv, -1);

			if( (msh->cod != 1) && (msh->cod != 16777216) )
				longjmp(GmfEnv, -1);

			ScaWrd(msh, (unsigned char *)&msh->ver);

			if( (msh->ver < 1) || (msh->ver > 4) )
				longjmp(GmfEnv, -1);

			if( (msh->ver >= 3) && (sizeof(long) == 4) )
				longjmp(GmfEnv, -1);

			ScaWrd(msh, (unsigned char *)&KwdCod);

			if(KwdCod != GmfDimension)
				longjmp(GmfEnv, -1);

			GetPos(msh);
			ScaWrd(msh, (unsigned char *)&msh->dim);
		}
		else
		{
			do
			{
				res = fscanf(msh->hdl, "%s", str);
			}while( (res != EOF) && strcmp(str, "MeshVersionFormatted") );

			if(res == EOF)
				longjmp(GmfEnv, -1);

			safe_fscanf(msh->hdl, "%d", &msh->ver);

			if( (msh->ver < 1) || (msh->ver > 4) )
				longjmp(GmfEnv, -1);

			do
			{
				res = fscanf(msh->hdl, "%s", str);
			}while( (res != EOF) && strcmp(str, "Dimension") );

			if(res == EOF)
				longjmp(GmfEnv, -1);

			safe_fscanf(msh->hdl, "%d", &msh->dim);
		}

		if( (msh->dim != 2) && (msh->dim != 3) )
			longjmp(GmfEnv, -1);

		(*PtrVer) = msh->ver;
		(*PtrDim) = msh->dim;

		/*------------*/
		/* KW READING */
		/*------------*/

		/* Read the list of kw present in the file */

		if(!ScaKwdTab(msh))
			return(0);

		GmfMshTab[ MshIdx ] = msh;

		return(MshIdx);
	}
	else if(msh->mod == GmfWrite)
	{

		/*-----------------------*/
		/* OPEN FILE FOR WRITING */
		/*-----------------------*/

		msh->cod = 1;

		/* Check if the user provided a valid version number and dimension */

		va_start(VarArg, mod);
		msh->ver = va_arg(VarArg, int);
		msh->dim = va_arg(VarArg, int);
		va_end(VarArg);

		if( (msh->ver < 1) || (msh->ver > 4) )
			longjmp(GmfEnv, -1);

		if( (msh->ver >= 3) && (sizeof(long) == 4) )
			longjmp(GmfEnv, -1);

		if( (msh->dim != 2) && (msh->dim != 3) )
			longjmp(GmfEnv, -1);

		/* Create the mesh file */

		if(!(msh->hdl = fopen(msh->FilNam, "wb")))
			longjmp(GmfEnv, -1);

		GmfMshTab[ MshIdx ] = msh;


		/*------------*/
		/* KW WRITING */
		/*------------*/

		/* Write the mesh version and dimension */

		if(msh->typ & Asc)
		{
			fprintf(msh->hdl, "%s %d\n\n", GmfKwdFmt[ GmfVersionFormatted ][0], msh->ver);
			fprintf(msh->hdl, "%s %d\n", GmfKwdFmt[ GmfDimension ][0], msh->dim);
		}
		else
		{
			RecWrd(msh, (unsigned char *)&msh->cod);
			RecWrd(msh, (unsigned char *)&msh->ver);
			GmfSetKwd(MshIdx, GmfDimension, 0);
			RecWrd(msh, (unsigned char *)&msh->dim);
		}

		return(MshIdx);
	}
	else
	{
		free(msh);
		return(0);
	}
}


/*----------------------------------------------------------*/
/* Close a meshfile in the right way						*/
/*----------------------------------------------------------*/

int GmfCloseMesh(int MshIdx)
{
	int res = 1;
	GmfMshSct *msh;

	if( (MshIdx < 1) || (MshIdx > MaxMsh) )
		return(0);

	msh = GmfMshTab[ MshIdx ];
	RecBlk(msh, msh->buf, 0);

	/* In write down the "End" kw in write mode */

	if(msh->mod == GmfWrite)
	{
		if(msh->typ & Asc)
			fprintf(msh->hdl, "\n%s\n", GmfKwdFmt[ GmfEnd ][0]);
		else
			GmfSetKwd(MshIdx, GmfEnd, 0);
	}

	/* Close the file and free the mesh structure */

	if(fclose(msh->hdl))
		res = 0;

	free(msh);
	GmfMshTab[ MshIdx ] = NULL;

	return(res);
}


/*----------------------------------------------------------*/
/* Read the number of lines and set the position to this kwd*/
/*----------------------------------------------------------*/

long GmfStatKwd(int MshIdx, int KwdCod, ...)
{
	int i, *PtrNmbTyp, *PtrSolSiz, *TypTab;
	GmfMshSct *msh;
	KwdSct *kwd;
	va_list VarArg;

	if( (MshIdx < 1) || (MshIdx > MaxMsh) )
		return(0);

	msh = GmfMshTab[ MshIdx ];

	if( (KwdCod < 1) || (KwdCod > GmfMaxKwd) )
		return(0);

	kwd = &msh->KwdTab[ KwdCod ];

	if(!kwd->NmbLin)
		return(0);

	/* Read further arguments if this kw is a sol */

	if(kwd->typ == SolKwd)
	{
		va_start(VarArg, KwdCod);

		PtrNmbTyp = va_arg(VarArg, int *);
		*PtrNmbTyp = kwd->NmbTyp;

		PtrSolSiz = va_arg(VarArg, int *);
		*PtrSolSiz = kwd->SolSiz;

		TypTab = va_arg(VarArg, int *);

		for(i=0;i<kwd->NmbTyp;i++)
			TypTab[i] = kwd->TypTab[i];

		va_end(VarArg);
	}

	return(kwd->NmbLin);
}


/*----------------------------------------------------------*/
/* Set the current file position to a given kwd				*/
/*----------------------------------------------------------*/

int GmfGotoKwd(int MshIdx, int KwdCod)
{
	GmfMshSct *msh;
	KwdSct *kwd;

	if( (MshIdx < 1) || (MshIdx > MaxMsh) )
		return(0);

	msh = GmfMshTab[ MshIdx ];

	if( (KwdCod < 1) || (KwdCod > GmfMaxKwd) )
		return(0);

	kwd = &msh->KwdTab[ KwdCod ];

	if(!kwd->NmbLin)
		return(0);

	return(fseek(msh->hdl, kwd->pos, SEEK_SET) == 0);
}


/*----------------------------------------------------------*/
/* Write the kwd and set the number of lines				*/
/*----------------------------------------------------------*/

long GmfSetKwd(int MshIdx, int KwdCod, ...)
{
	int i, *TypTab;
	long NmbLin=0, CurPos;
	va_list VarArg;
	GmfMshSct *msh;
	KwdSct *kwd;

	if( (MshIdx < 1) || (MshIdx > MaxMsh) )
		return(0);

	msh = GmfMshTab[ MshIdx ];
	RecBlk(msh, msh->buf, 0);

	if( (KwdCod < 1) || (KwdCod > GmfMaxKwd) )
		return(0);

	kwd = &msh->KwdTab[ KwdCod ];

	/* Read further arguments if this kw has a header */

	if(strlen(GmfKwdFmt[ KwdCod ][2]))
	{
		va_start(VarArg, KwdCod);
		NmbLin = va_arg(VarArg, long);

		if(!strcmp(GmfKwdFmt[ KwdCod ][3], "sr"))
		{
			kwd->NmbTyp = va_arg(VarArg, int);
			TypTab = va_arg(VarArg, int *);

			for(i=0;i<kwd->NmbTyp;i++)
				kwd->TypTab[i] = TypTab[i];
		}

		va_end(VarArg);
	}

	/* Setup the kwd info */

	ExpFmt(msh, KwdCod);

	if(!kwd->typ)
		return(0);
	else if(kwd->typ == InfKwd)
		kwd->NmbLin = 1;
	else
		kwd->NmbLin = NmbLin;

	/* Store the next kwd position in binary file */

	if( (msh->typ & Bin) && msh->NexKwdPos )
	{
		CurPos = ftell(msh->hdl);

		if(fseek(msh->hdl, msh->NexKwdPos, SEEK_SET) != 0)
			return(0);

		SetPos(msh, CurPos);

		if(fseek(msh->hdl, CurPos, SEEK_SET) != 0)
			return(0);
	}

	/* Write the header */

	if(msh->typ & Asc)
	{
		fprintf(msh->hdl, "\n%s\n", GmfKwdFmt[ KwdCod ][0]);

		if(kwd->typ != InfKwd)
			fprintf(msh->hdl, "%ld\n", kwd->NmbLin);

		/* In case of solution field, write the extended header */

		if(kwd->typ == SolKwd)
		{
			fprintf(msh->hdl, "%d ", kwd->NmbTyp);

			for(i=0;i<kwd->NmbTyp;i++)
				fprintf(msh->hdl, "%d ", kwd->TypTab[i]);

			fprintf(msh->hdl, "\n\n");
		}
	}
	else
	{
		RecWrd(msh, (unsigned char *)&KwdCod);
		msh->NexKwdPos = ftell(msh->hdl);
		SetPos(msh, 0);

		if(kwd->typ != InfKwd)
		{
			if(msh->ver < 4)
			{
				i = (int)kwd->NmbLin;
				RecWrd(msh, (unsigned char *)&i);
			}
			else
				RecDblWrd(msh, (unsigned char *)&kwd->NmbLin);
		}

		/* In case of solution field, write the extended header at once */

		if(kwd->typ == SolKwd)
		{
			RecWrd(msh, (unsigned char *)&kwd->NmbTyp);

			for(i=0;i<kwd->NmbTyp;i++)
				RecWrd(msh, (unsigned char *)&kwd->TypTab[i]);
		}
	}

	/* Reset write buffer position */
	msh->pos = 0;

	/* Estimate the total file size and check whether it crosses the 2GB threshold */

	msh->siz += kwd->NmbLin * kwd->NmbWrd * WrdSiz;
	return(kwd->NmbLin);
}


/*----------------------------------------------------------*/
/* Read a full line from the current kwd					*/
/*----------------------------------------------------------*/

int GmfGetLin(int MshIdx, int KwdCod, ...)
{
	int i, j;
	float *FltSolTab;
	double *DblSolTab;
	va_list VarArg;
	GmfMshSct *msh = GmfMshTab[ MshIdx ];
	KwdSct *kwd = &msh->KwdTab[ KwdCod ];

	/* Save the current stack environment for longjmp */

	if(setjmp(GmfEnv) != 0)
		return(0);

	/* Start decoding the arguments */

	va_start(VarArg, KwdCod);

	switch(kwd->typ)
	{
		case InfKwd : case RegKwd : case CmtKwd :
		{
			if(msh->typ & Asc)
			{
				for(i=0;i<kwd->SolSiz;i++)
					if(kwd->fmt[i] == 'r')
						if(msh->ver <= 1)
							safe_fscanf(msh->hdl, "%f", va_arg(VarArg, float *));
						else
							safe_fscanf(msh->hdl, "%lf", va_arg(VarArg, double *));
					else if(kwd->fmt[i] == 'i')
						if(msh->ver <= 3)
							safe_fscanf(msh->hdl, "%d", va_arg(VarArg, int *));
						else
							safe_fscanf(msh->hdl, "%ld", va_arg(VarArg, long *));
					else if(kwd->fmt[i] == 'c')
						safe_fgets(va_arg(VarArg, char *), WrdSiz * FilStrSiz, msh->hdl);
			}
			else
			{
				for(i=0;i<kwd->SolSiz;i++)
					if(kwd->fmt[i] == 'r')
						if(msh->ver <= 1)
							ScaWrd(msh, (unsigned char *)va_arg(VarArg, float *));
						else
							ScaDblWrd(msh, (unsigned char *)va_arg(VarArg, double *));
					else if(kwd->fmt[i] == 'i')
						if(msh->ver <= 3)
							ScaWrd(msh, (unsigned char *)va_arg(VarArg, int *));
						else
							ScaDblWrd(msh, (unsigned char *)va_arg(VarArg, long *));
					else if(kwd->fmt[i] == 'c')
						fread(va_arg(VarArg, char *), WrdSiz, FilStrSiz, msh->hdl);
			}
		}break;

		case SolKwd :
		{
			if(msh->ver == 1)
			{
				FltSolTab = va_arg(VarArg, float *);

				if(msh->typ & Asc)
					for(j=0;j<kwd->SolSiz;j++)
						safe_fscanf(msh->hdl, "%f", &FltSolTab[j]);
				else
					for(j=0;j<kwd->SolSiz;j++)
						ScaWrd(msh, (unsigned char *)&FltSolTab[j]);
			}
			else
			{
				DblSolTab = va_arg(VarArg, double *);

				if(msh->typ & Asc)
					for(j=0;j<kwd->SolSiz;j++)
						safe_fscanf(msh->hdl, "%lf", &DblSolTab[j]);
				else
					for(j=0;j<kwd->SolSiz;j++)
						ScaDblWrd(msh, (unsigned char *)&DblSolTab[j]);
			}
		}break;
	}

	va_end(VarArg);

	return(1);
}


/*----------------------------------------------------------*/
/* Write a full line from the current kwd					*/
/*----------------------------------------------------------*/

void GmfSetLin(int MshIdx, int KwdCod, ...)
{
	int i, j, pos, *IntBuf;
	long *LngBuf;
	float *FltSolTab, *FltBuf;
	double *DblSolTab, *DblBuf;
	va_list VarArg;
	GmfMshSct *msh = GmfMshTab[ MshIdx ];
	KwdSct *kwd = &msh->KwdTab[ KwdCod ];

	/* Start decoding the arguments */

	va_start(VarArg, KwdCod);

	if(kwd->typ != SolKwd)
	{
		if(msh->typ & Asc)
		{
			for(i=0;i<kwd->SolSiz;i++)
			{
				if(kwd->fmt[i] == 'r')
				{
					if(msh->ver <= 1)
						fprintf(msh->hdl, "%g ", va_arg(VarArg, double));
					else
						fprintf(msh->hdl, "%.15g ", va_arg(VarArg, double));
				}
				else if(kwd->fmt[i] == 'i')
				{
					if(msh->ver <= 3)
						fprintf(msh->hdl, "%d ", va_arg(VarArg, int));
					else
						fprintf(msh->hdl, "%ld ", va_arg(VarArg, long));
				}
				else if(kwd->fmt[i] == 'c')
					fprintf(msh->hdl, "%s", va_arg(VarArg, char *));
			}
		}
		else
		{
			pos = 0;

			for(i=0;i<kwd->SolSiz;i++)
			{
				if(kwd->fmt[i] == 'r')
				{
					if(msh->ver <= 1)
					{
						FltBuf = (void *)&msh->buf[ pos ];
						*FltBuf = (float)va_arg(VarArg, double);
						pos += 4;
					}
					else
					{
						DblBuf = (void *)&msh->buf[ pos ];
						*DblBuf = va_arg(VarArg, double);
						pos += 8;
					}
				}
				else if(kwd->fmt[i] == 'i')
				{
					if(msh->ver <= 3)
					{
						IntBuf = (void *)&msh->buf[ pos ];
						*IntBuf = va_arg(VarArg, int);
						pos += 4;
					}
					else
					{
						LngBuf = (void *)&msh->buf[ pos ];
						*LngBuf = va_arg(VarArg, long);
						pos += 8;
					}
				}
				else if(kwd->fmt[i] == 'c')
				{
					memset(&msh->buf[ pos ], 0, FilStrSiz * WrdSiz);
					strncpy(&msh->buf[ pos ], va_arg(VarArg, char *), FilStrSiz * WrdSiz);
					pos += FilStrSiz;
				}
			}

			RecBlk(msh, msh->buf, kwd->NmbWrd);
		}
	}
	else
	{
		if(msh->ver == 1)
		{
			FltSolTab = va_arg(VarArg, float *);

			if(msh->typ & Asc)
				for(j=0;j<kwd->SolSiz;j++)
					fprintf(msh->hdl, "%g ", (double)FltSolTab[j]);
			else
				RecBlk(msh, (unsigned char *)FltSolTab, kwd->NmbWrd);
		}
		else
		{
			DblSolTab = va_arg(VarArg, double *);

			if(msh->typ & Asc)
				for(j=0;j<kwd->SolSiz;j++)
					fprintf(msh->hdl, "%.15g ", DblSolTab[j]);
			else
				RecBlk(msh, (unsigned char *)DblSolTab, kwd->NmbWrd);
		}
	}

	va_end(VarArg);

	if(msh->typ & Asc)
		fprintf(msh->hdl, "\n");
}


/*----------------------------------------------------------*/
/* Private procedure for transmesh : copy a whole line		*/
/*----------------------------------------------------------*/

int GmfCpyLin(int InpIdx, int OutIdx, int KwdCod)
{
	char s[ WrdSiz * FilStrSiz ];
	double d;
	float f;
	int i, a;
	long l;
	GmfMshSct *InpMsh = GmfMshTab[ InpIdx ], *OutMsh = GmfMshTab[ OutIdx ];
	KwdSct *kwd = &InpMsh->KwdTab[ KwdCod ];

	/* Save the current stack environment for longjmp */

	if(setjmp(GmfEnv) != 0)
		return(0);

	for(i=0;i<kwd->SolSiz;i++)
	{
		if(kwd->fmt[i] == 'r')
		{
			if(InpMsh->ver == 1)
			{
				if(InpMsh->typ & Asc)
					safe_fscanf(InpMsh->hdl, "%f", &f);
				else
					ScaWrd(InpMsh, (unsigned char *)&f);

				d = (double)f;
			}
			else
			{
				if(InpMsh->typ & Asc)
					safe_fscanf(InpMsh->hdl, "%lf", &d);
				else
					ScaDblWrd(InpMsh, (unsigned char *)&d);

				f = (float)d;
			}

			if(OutMsh->ver == 1)
				if(OutMsh->typ & Asc)
					fprintf(OutMsh->hdl, "%g ", (double)f);
				else
					RecWrd(OutMsh, (unsigned char *)&f);
			else
				if(OutMsh->typ & Asc)
					fprintf(OutMsh->hdl, "%.15g ", d);
				else
					RecDblWrd(OutMsh, (unsigned char *)&d);
		}
		else if(kwd->fmt[i] == 'i')
		{
			if(InpMsh->ver <= 3)
			{
				if(InpMsh->typ & Asc)
					safe_fscanf(InpMsh->hdl, "%d", &a);
				else
					ScaWrd(InpMsh, (unsigned char *)&a);

				l = (long)a;
			}
			else
			{
				if(InpMsh->typ & Asc)
					safe_fscanf(InpMsh->hdl, "%ld", &l);
				else
					ScaDblWrd(InpMsh, (unsigned char *)&l);

				a = (int)l;
			}

			if(OutMsh->ver <= 3)
			{
				if(OutMsh->typ & Asc)
					fprintf(OutMsh->hdl, "%d ", a);
				else
					RecWrd(OutMsh, (unsigned char *)&a);
			}
			else
			{
				if(OutMsh->typ & Asc)
					fprintf(OutMsh->hdl, "%ld ", l);
				else
					RecDblWrd(OutMsh, (unsigned char *)&l);
			}
		}
		else if(kwd->fmt[i] == 'c')
		{
			memset(s, 0, FilStrSiz * WrdSiz);

			if(InpMsh->typ & Asc)
				safe_fgets(s, WrdSiz * FilStrSiz, InpMsh->hdl);
			else
				fread(s, WrdSiz, FilStrSiz, InpMsh->hdl);

			if(OutMsh->typ & Asc)
				fprintf(OutMsh->hdl, "%s", s);
			else
				fwrite(s, WrdSiz, FilStrSiz, OutMsh->hdl);
		}
	}

	if(OutMsh->typ & Asc)
		fprintf(OutMsh->hdl, "\n");

	return(1);
}


/*----------------------------------------------------------*/
/* Bufferized reading of all keyword's lines				*/
/*----------------------------------------------------------*/

extern int GmfGetBlock(int MshIdx, int KwdCod, ...)
{
	char *UsrDat[ GmfMaxTyp ], *FilBuf=NULL, *FilPos;
	char *StrTab[5] = { "", "%f", "%lf", "%d", "%ld" };
	int b, i, j, LinSiz, *FilPtrI32, *UsrPtrI32, FilTyp[ GmfMaxTyp ], UsrTyp[ GmfMaxTyp ], SizTab[5] = {0,4,8,4,8};
	long NmbLin, *FilPtrI64, *UsrPtrI64;
	float *FilPtrR32, *UsrPtrR32;
	double *FilPtrR64, *UsrPtrR64;
	size_t UsrLen[ GmfMaxTyp ];
	va_list VarArg;
	GmfMshSct *msh = GmfMshTab[ MshIdx ];
	KwdSct *kwd = &msh->KwdTab[ KwdCod ];

	/* Save the current stack environment for longjmp */

	if(setjmp(GmfEnv) != 0)
	{
		if(FilBuf)
			free(FilBuf);

		return(0);
	}

	/* Check mesh and keyword */

	if( (MshIdx < 1) || (MshIdx > MaxMsh) )
		return(0);

	if( (KwdCod < 1) || (KwdCod > GmfMaxKwd) )
		return(0);

	if(!kwd->NmbLin)
		return(0);

	/* Make shure it's not a simple information keyword */

	if( (kwd->typ != RegKwd) && (kwd->typ != SolKwd) )
		return(0);

	/* Start decoding the arguments */

	va_start(VarArg, KwdCod);
	LinSiz = 0;

	if(kwd->typ == RegKwd)
	{
		for(i=0;i<kwd->SolSiz;i++)
		{
			/* Get the user's data type */

			UsrTyp[i] = va_arg(VarArg, int);

			if(kwd->fmt[i] == 'r')
			{
				/* Get the data pointer */

				if(UsrTyp[i] == GmfFloat)
					UsrDat[i] = (char *)va_arg(VarArg, float *);
				else if(UsrTyp[i] == GmfDouble)
					UsrDat[i] = (char *)va_arg(VarArg, double *);
				else
					return(0);

				/* Get the file's data type */

				if(msh->ver <= 1)
					FilTyp[i] = GmfFloat;
				else
					FilTyp[i] = GmfDouble;
			}
			else
			{
				/* Get the data pointer */

				if(UsrTyp[i] == GmfInt)
					UsrDat[i] = (char *)va_arg(VarArg, int *);
				else if(UsrTyp[i] == GmfLong)
					UsrDat[i] = (char *)va_arg(VarArg, long *);
				else
					return(0);

				/* Get the file's data type */

				if(msh->ver <= 3)
					FilTyp[i] = GmfInt;
				else
					FilTyp[i] = GmfLong;
			}

			/* Then get the data second adress and compute the stride */

			UsrLen[i] = (size_t)(va_arg(VarArg, char *) - UsrDat[i]);
			LinSiz += SizTab[ FilTyp[i] ];
		}
	}
	else
	{
		/* Get the user's data type */

		UsrTyp[0] = va_arg(VarArg, int);

		/* Get the data pointer */

		if(UsrTyp[0] == GmfFloat)
			UsrDat[0] = (char *)va_arg(VarArg, float *);
		else if(UsrTyp[0] == GmfDouble)
			UsrDat[0] = (char *)va_arg(VarArg, double *);
		else
			return(0);

		/* Get the file's data type */

		if(msh->ver <= 1)
			FilTyp[0] = GmfFloat;
		else
			FilTyp[0] = GmfDouble;

		/* Then get the data second adress and compute the stride */

		UsrLen[0] = (size_t)(va_arg(VarArg, char *) - UsrDat[0]);

		for(i=1;i<kwd->SolSiz;i++)
		{
			UsrTyp[i] = UsrTyp[0];
			UsrDat[i] = UsrDat[i-1] + SizTab[ UsrTyp[0] ];
			UsrLen[i] = UsrLen[0];
			FilTyp[i] = FilTyp[0];
		}

		LinSiz = kwd->SolSiz * SizTab[ FilTyp[0] ];
	}

	va_end(VarArg);

	/* Read the whole kwd data */

	if(msh->typ & Asc)
	{
		for(i=0;i<kwd->NmbLin;i++)
			for(j=0;j<kwd->SolSiz;j++)
			{
				safe_fscanf(msh->hdl, StrTab[ UsrTyp[j] ], UsrDat[j]);
				UsrDat[j] += UsrLen[j];
			}
	}
	else
	{
		/* Allocate a small buffer and split the main loop into chunks */

		if(!(FilBuf = malloc((size_t)(BufSiz * LinSiz))))
			return(0);

		for(b=0;b<=kwd->NmbLin/BufSiz;b++)
		{
			if(b == kwd->NmbLin/BufSiz)
				NmbLin = kwd->NmbLin - b * BufSiz;
			else
				NmbLin = BufSiz;
 
			/* Read a chunk of data */

			if(fread(FilBuf, (size_t)LinSiz, (size_t)NmbLin, msh->hdl) != NmbLin)
				longjmp(GmfEnv, -1);

			FilPos = FilBuf;

			/* Then decode it and store it in the user's data structure */

			for(i=0;i<NmbLin;i++)
				for(j=0;j<kwd->SolSiz;j++)
				{
					if(msh->cod != 1)
						SwpWrd(FilPos, SizTab[ FilTyp[j] ]);

					if(FilTyp[j] == GmfInt)
					{
						FilPtrI32 = (int *)FilPos;

						if(UsrTyp[j] == GmfInt)
						{
							UsrPtrI32 = (int *)UsrDat[j];
							*UsrPtrI32 = *FilPtrI32;
						}
						else
						{
							UsrPtrI64 = (long *)UsrDat[j];
							*UsrPtrI64 = (long)*FilPtrI32;
						}
					}
					else if(FilTyp[j] == GmfLong)
					{
						FilPtrI64 = (long *)FilPos;

						if(UsrTyp[j] == GmfLong)
						{
							UsrPtrI64 = (long *)UsrDat[j];
							*UsrPtrI64 = *FilPtrI64;
						}
						else
						{
							UsrPtrI32 = (int *)UsrDat[j];
							*UsrPtrI32 = (int)*FilPtrI64;
						}
					}
					else if(FilTyp[j] == GmfFloat)
					{
						FilPtrR32 = (float *)FilPos;

						if(UsrTyp[j] == GmfFloat)
						{
							UsrPtrR32 = (float *)UsrDat[j];
							*UsrPtrR32 = *FilPtrR32;
						}
						else
						{
							UsrPtrR64 = (double *)UsrDat[j];
							*UsrPtrR64 = (double)*FilPtrR32;
						}
					}
					else if(FilTyp[j] == GmfDouble)
					{
						FilPtrR64 = (double *)FilPos;

						if(UsrTyp[j] == GmfDouble)
						{
							UsrPtrR64 = (double *)UsrDat[j];
							*UsrPtrR64 = *FilPtrR64;
						}
						else
						{
							UsrPtrR32 = (float *)UsrDat[j];
							*UsrPtrR32 = (float)*FilPtrR64;
						}
					}

					FilPos += SizTab[ FilTyp[j] ];
					UsrDat[j] += UsrLen[j];
				}
		}

		free(FilBuf);
	}

	return(1);
}


/*----------------------------------------------------------*/
/* Bufferized writing of all keyword's lines				*/
/*----------------------------------------------------------*/

extern int GmfSetBlock(int MshIdx, int KwdCod, ...)
{
	char *UsrDat[ GmfMaxTyp ], *FilBuf, *FilPos;
	char *StrTab[5] = { "", "%g", "%.15g", "%d", "%ld" };
	int i, j, LinSiz, *FilPtrI32, *UsrPtrI32, FilTyp[ GmfMaxTyp ], UsrTyp[ GmfMaxTyp ];
	int SizTab[5] = {0,4,8,4,8};
	long NmbLin, b, *FilPtrI64, *UsrPtrI64;
	float *FilPtrR32, *UsrPtrR32;
	double *FilPtrR64, *UsrPtrR64;
	size_t UsrLen[ GmfMaxTyp ];
	va_list VarArg;
	GmfMshSct *msh = GmfMshTab[ MshIdx ];
	KwdSct *kwd = &msh->KwdTab[ KwdCod ];

	/* Check mesh and keyword */

	if( (MshIdx < 1) || (MshIdx > MaxMsh) )
		return(0);

	if( (KwdCod < 1) || (KwdCod > GmfMaxKwd) )
		return(0);

	if(!kwd->NmbLin)
		return(0);

	if( (kwd->typ != RegKwd) && (kwd->typ != SolKwd) )
		return(0);

	/* Start decoding the arguments */

	va_start(VarArg, KwdCod);
	LinSiz = 0;

	if(kwd->typ == RegKwd)
	{
		for(i=0;i<kwd->SolSiz;i++)
		{
			/* Get the user's data type */

			UsrTyp[i] = va_arg(VarArg, int);

			if(kwd->fmt[i] == 'r')
			{
				/* Get the data pointer */

				if(UsrTyp[i] == GmfFloat)
					UsrDat[i] = (char *)va_arg(VarArg, float *);
				else if(UsrTyp[i] == GmfDouble)
					UsrDat[i] = (char *)va_arg(VarArg, double *);
				else
					return(0);

				/* Get the file's data type */

				if(msh->ver <= 1)
					FilTyp[i] = GmfFloat;
				else
					FilTyp[i] = GmfDouble;
			}
			else
			{
				/* Get the data pointer */

				if(UsrTyp[i] == GmfInt)
					UsrDat[i] = (char *)va_arg(VarArg, int *);
				else if(UsrTyp[i] == GmfLong)
					UsrDat[i] = (char *)va_arg(VarArg, long *);
				else
					return(0);

				/* Get the file's data type */

				if(msh->ver <= 3)
					FilTyp[i] = GmfInt;
				else
					FilTyp[i] = GmfLong;
			}

			/* Then get the data second adress and compute the stride */

			UsrLen[i] = (size_t)(va_arg(VarArg, char *) - UsrDat[i]);
			LinSiz += SizTab[ FilTyp[i] ];
		}
	}
	else
	{
		/* Get the user's data type */

		UsrTyp[0] = va_arg(VarArg, int);

		/* Get the data pointer */

		if(UsrTyp[0] == GmfFloat)
			UsrDat[0] = (char *)va_arg(VarArg, float *);
		else if(UsrTyp[0] == GmfDouble)
			UsrDat[0] = (char *)va_arg(VarArg, double *);
		else
			return(0);

		/* Get the file's data type */

		if(msh->ver <= 1)
			FilTyp[0] = GmfFloat;
		else
			FilTyp[0] = GmfDouble;

		/* Then get the data second adress and compute the stride */

		UsrLen[0] = (size_t)(va_arg(VarArg, char *) - UsrDat[0]);

		for(i=1;i<kwd->SolSiz;i++)
		{
			UsrTyp[i] = UsrTyp[0];
			UsrDat[i] = UsrDat[i-1] + SizTab[ UsrTyp[0] ];
			UsrLen[i] = UsrLen[0];
			FilTyp[i] = FilTyp[0];
		}

		LinSiz = kwd->SolSiz * SizTab[ FilTyp[0] ];
	}

	va_end(VarArg);

	/* Write the whole kwd data */
    
	if(msh->typ & Asc)
	{
		for(i=0;i<kwd->NmbLin;i++)
			for(j=0;j<kwd->SolSiz;j++)
			{
				if(UsrTyp[j] == GmfFloat)
				{
					UsrPtrR32 = (float *)UsrDat[j];
					fprintf(msh->hdl, StrTab[ UsrTyp[j] ], (double)*UsrPtrR32);
				}
				else if(UsrTyp[j] == GmfDouble)
				{
					UsrPtrR64 = (double *)UsrDat[j];
					fprintf(msh->hdl, StrTab[ UsrTyp[j] ], *UsrPtrR64);
				}
				else if(UsrTyp[j] == GmfInt)
				{
					UsrPtrI32 = (int *)UsrDat[j];
					fprintf(msh->hdl, StrTab[ UsrTyp[j] ], *UsrPtrI32);
				}
				else if(UsrTyp[j] == GmfLong)
				{
					UsrPtrI64 = (long *)UsrDat[j];
					fprintf(msh->hdl, StrTab[ UsrTyp[j] ], *UsrPtrI64);
				}

				if(j < kwd->SolSiz -1)
					fprintf(msh->hdl, " ");
				else
					fprintf(msh->hdl, "\n");

				UsrDat[j] += UsrLen[j];
			}
	}
	else
	{
		if(!(FilBuf = malloc((size_t)BufSiz * (size_t)LinSiz)))
			return(0);

		for(b=0;b<=kwd->NmbLin/BufSiz;b++)
		{
			if(b == kwd->NmbLin/BufSiz)
				NmbLin = kwd->NmbLin - b * BufSiz;
			else
				NmbLin = BufSiz;

			FilPos = FilBuf;

			for(i=0;i<NmbLin;i++)
				for(j=0;j<kwd->SolSiz;j++)
				{
					if(FilTyp[j] == GmfInt)
					{
						FilPtrI32 = (int *)FilPos;

						if(UsrTyp[j] == GmfInt)
						{
							UsrPtrI32 = (int *)UsrDat[j];
							*FilPtrI32 = *UsrPtrI32;
						}
						else
						{
							UsrPtrI64 = (long *)UsrDat[j];
							*FilPtrI32 = (int)*UsrPtrI64;
						}
					}
					else if(FilTyp[j] == GmfLong)
					{
						FilPtrI64 = (long *)FilPos;

						if(UsrTyp[j] == GmfLong)
						{
							UsrPtrI64 = (long *)UsrDat[j];
							*FilPtrI64 = *UsrPtrI64;
						}
						else
						{
							UsrPtrI32 = (int *)UsrDat[j];
							*FilPtrI64 = (long)*UsrPtrI32;
						}
					}
					else if(FilTyp[j] == GmfFloat)
					{
						FilPtrR32 = (float *)FilPos;

						if(UsrTyp[j] == GmfFloat)
						{
							UsrPtrR32 = (float *)UsrDat[j];
							*FilPtrR32 = *UsrPtrR32;
						}
						else
						{
							UsrPtrR64 = (double *)UsrDat[j];
							*FilPtrR32 = (float)*UsrPtrR64;
						}
					}
					else if(FilTyp[j] == GmfDouble)
					{
						FilPtrR64 = (double *)FilPos;

						if(UsrTyp[j] == GmfDouble)
						{
							UsrPtrR64 = (double *)UsrDat[j];
							*FilPtrR64 = *UsrPtrR64;
						}
						else
						{
							UsrPtrR32 = (float *)UsrDat[j];
							*FilPtrR64 = (double)*UsrPtrR32;
						}
					}

					FilPos += SizTab[ FilTyp[j] ];
					UsrDat[j] += UsrLen[j];
				}

			if(fwrite(FilBuf, (size_t)LinSiz, (size_t)NmbLin, msh->hdl) != NmbLin)
			{
				free(FilBuf);
				return(0);
			}
		}

		free(FilBuf);
	}

	return(1);
}


/*----------------------------------------------------------*/
/* Find every kw present in a meshfile						*/
/*----------------------------------------------------------*/

static int ScaKwdTab(GmfMshSct *msh)
{
	int KwdCod, c;
	long  NexPos, CurPos, EndPos;
	char str[ GmfStrSiz ];

	if(msh->typ & Asc)
	{
		/* Scan each string in the file until the end */

		while(fscanf(msh->hdl, "%s", str) != EOF)
		{
			/* Fast test in order to reject quickly the numeric values */

			if(isalpha(str[0]))
			{
				/* Search which kwd code this string is associated with, 
					then get its header and save the curent position in file (just before the data) */

				for(KwdCod=1; KwdCod<= GmfMaxKwd; KwdCod++)
					if(!strcmp(str, GmfKwdFmt[ KwdCod ][0]))
					{
						ScaKwdHdr(msh, KwdCod);
						break;
					}
			}
			else if(str[0] == '#')
				while((c = fgetc(msh->hdl)) != '\n' && c != EOF);
		}
	}
	else
	{
		/* Get file size */

		CurPos = ftell(msh->hdl);

		if(fseek(msh->hdl, 0, SEEK_END) != 0)
			longjmp(GmfEnv, -1);

		EndPos = ftell(msh->hdl);

		if(fseek(msh->hdl, CurPos, SEEK_SET) != 0)
			longjmp(GmfEnv, -1);

		/* Jump through kwd positions in the file */

		do
		{
			/* Get the kwd code and the next kwd position */

			ScaWrd(msh, (unsigned char *)&KwdCod);
			NexPos = GetPos(msh);

			if(NexPos > EndPos)
				longjmp(GmfEnv, -1);

			/* Check if this kwd belongs to this mesh version */

			if( (KwdCod >= 1) && (KwdCod <= GmfMaxKwd) )
				ScaKwdHdr(msh, KwdCod);

			/* Go to the next kwd */

			if(NexPos && (fseek(msh->hdl, NexPos, SEEK_SET) != 0))
				longjmp(GmfEnv, -1);
		}while(NexPos && (KwdCod != GmfEnd));
	}

	return(1);
}


/*----------------------------------------------------------*/
/* Read and setup the keyword's header						*/
/*----------------------------------------------------------*/

static void ScaKwdHdr(GmfMshSct *msh, int KwdCod)
{
	int i;
	KwdSct *kwd = &msh->KwdTab[ KwdCod ];

	if(!strcmp("i", GmfKwdFmt[ KwdCod ][2]))
		if(msh->typ & Asc)
			safe_fscanf(msh->hdl, "%ld", &kwd->NmbLin);
		else
			if(msh->ver <= 3)
			{
				ScaWrd(msh, (unsigned char *)&i);
				kwd->NmbLin = i;
			}
			else
				ScaDblWrd(msh, (unsigned char *)&kwd->NmbLin);
	else
		kwd->NmbLin = 1;

	if(!strcmp("sr", GmfKwdFmt[ KwdCod ][3]))
	{
		if(msh->typ & Asc)
		{
			safe_fscanf(msh->hdl, "%d", &kwd->NmbTyp);

			for(i=0;i<kwd->NmbTyp;i++)
				safe_fscanf(msh->hdl, "%d", &kwd->TypTab[i]);
		}
		else
		{
			ScaWrd(msh, (unsigned char *)&kwd->NmbTyp);

			for(i=0;i<kwd->NmbTyp;i++)
				ScaWrd(msh, (unsigned char *)&kwd->TypTab[i]);
		}
	}

	ExpFmt(msh, KwdCod);
	kwd->pos = ftell(msh->hdl);
}


/*----------------------------------------------------------*/
/* Expand the compacted format and compute the line size	*/
/*----------------------------------------------------------*/

static void ExpFmt(GmfMshSct *msh, int KwdCod)
{
	int i, j, TmpSiz=0, IntWrd, FltWrd;
	char chr;
	const char *InpFmt = GmfKwdFmt[ KwdCod ][3];
	KwdSct *kwd = &msh->KwdTab[ KwdCod ];

	/* Set the kwd's type */

	if(!strlen(GmfKwdFmt[ KwdCod ][2]))
		kwd->typ = InfKwd;
	else if(!strcmp(InpFmt, "sr"))
		kwd->typ = SolKwd;
	else
		kwd->typ = RegKwd;

	/* Get the solution-field's size */

	if(kwd->typ == SolKwd)
		for(i=0;i<kwd->NmbTyp;i++)
			switch(kwd->TypTab[i])
			{
				case GmfSca    : TmpSiz += 1; break;
				case GmfVec    : TmpSiz += msh->dim; break;
				case GmfSymMat : TmpSiz += (msh->dim * (msh->dim+1)) / 2; break;
				case GmfMat    : TmpSiz += msh->dim * msh->dim; break;
			}

	/* Scan each character from the format string */

	i = kwd->SolSiz = kwd->NmbWrd = 0;

	while(i < (int)strlen(InpFmt))
	{
		chr = InpFmt[ i++ ];

		if(chr == 'd')
		{
			chr = InpFmt[i++];

			for(j=0;j<msh->dim;j++)
				kwd->fmt[ kwd->SolSiz++ ] = chr;
		}
		else if(chr == 's')
		{
			chr = InpFmt[i++];

			for(j=0;j<TmpSiz;j++)
				kwd->fmt[ kwd->SolSiz++ ] = chr;
		}
		else
			kwd->fmt[ kwd->SolSiz++ ] = chr;
	}

	if(msh->ver <= 1)
		FltWrd = 1;
	else
		FltWrd = 2;

	if(msh->ver <= 3)
		IntWrd = 1;
	else
		IntWrd = 2;

	for(i=0;i<kwd->SolSiz;i++)
		switch(kwd->fmt[i])
		{
			case 'i' : kwd->NmbWrd += IntWrd; break;
			case 'c' : kwd->NmbWrd += FilStrSiz; break;
			case 'r' : kwd->NmbWrd += FltWrd;break;
		}
}


/*----------------------------------------------------------*/
/* Read a four bytes word from a mesh file					*/
/*----------------------------------------------------------*/

static void ScaWrd(GmfMshSct *msh, void *ptr)
{
	if( fread(ptr, WrdSiz, 1, msh->hdl) != 1)
		longjmp(GmfEnv, -1);

	if(msh->cod != 1)
		SwpWrd((char *)ptr, WrdSiz);
}


/*----------------------------------------------------------*/
/* Read an eight bytes word from a mesh file				*/
/*----------------------------------------------------------*/

static void ScaDblWrd(GmfMshSct *msh, void *ptr)
{
	if( fread(ptr, WrdSiz, 2, msh->hdl) != 2 )
		longjmp(GmfEnv, -1);

	if(msh->cod != 1)
		SwpWrd((char *)ptr, 2 * WrdSiz);
}


/*----------------------------------------------------------*/
/* Read a 4 or 8 bytes position in mesh file				*/
/*----------------------------------------------------------*/

static long GetPos(GmfMshSct *msh)
{
	int IntVal;
	long pos;

	if(msh->ver >= 3)
		ScaDblWrd(msh, (unsigned char*)&pos);
	else
	{
		ScaWrd(msh, (unsigned char*)&IntVal);
		pos = (long)IntVal;
	}

	return(pos);
}


/*----------------------------------------------------------*/
/* Write a four bytes word to a mesh file					*/
/*----------------------------------------------------------*/

static void RecWrd(GmfMshSct *msh, const void *wrd)
{
	fwrite(wrd, WrdSiz, 1, msh->hdl);
}


/*----------------------------------------------------------*/
/* Write an eight bytes word to a mesh file					*/
/*----------------------------------------------------------*/

static void RecDblWrd(GmfMshSct *msh, const void *wrd)
{
	fwrite(wrd, WrdSiz, 2, msh->hdl);
}


/*----------------------------------------------------------*/
/* Write a block of four bytes word to a mesh file			*/
/*----------------------------------------------------------*/

static void RecBlk(GmfMshSct *msh, const void *blk, int siz)
{
	/* Copy this line-block into the main mesh buffer */

	if(siz)
	{
		memcpy(&msh->blk[ msh->pos ], blk, (size_t)(siz * WrdSiz));
		msh->pos += siz * WrdSiz;
	}

	/* When the buffer is full or this procedure is called with a 0 size, flush the cache on disk */

	if( (msh->pos > BufSiz) || (!siz && msh->pos) )
	{
		fwrite(msh->blk, 1, (size_t)msh->pos, msh->hdl);
		msh->pos = 0;
	}
}


/*----------------------------------------------------------*/
/* Write a 4 or 8 bytes position in a mesh file				*/
/*----------------------------------------------------------*/

static void SetPos(GmfMshSct *msh, long pos)
{
	int IntVal;

	if(msh->ver >= 3)
		RecDblWrd(msh, (unsigned char*)&pos);
	else
	{
		IntVal = (int)pos;
		RecWrd(msh, (unsigned char*)&IntVal);
	}
}


/*----------------------------------------------------------*/
/* Endianness conversion									*/
/*----------------------------------------------------------*/

static void SwpWrd(char *wrd, int siz)
{
	char swp;
	int i;

	for(i=0;i<siz/2;i++)
	{
		swp = wrd[ siz-i-1 ];
		wrd[ siz-i-1 ] = wrd[i];
		wrd[i] = swp;
	}
}


/*----------------------------------------------------------*/
/* Fortran 77 API											*/
/*----------------------------------------------------------*/

int call(gmfopenmesh)(char *FilNam, int *mod, int *ver, int *dim, int StrSiz)
{
	int i;
	char TmpNam[ GmfStrSiz ];

	for(i=0;i<StrSiz;i++)
		TmpNam[i] = FilNam[i];

	TmpNam[ StrSiz ] = 0;

	if(*mod == GmfRead)
		return(GmfOpenMesh(TmpNam, *mod, ver, dim));
	else
		return(GmfOpenMesh(TmpNam, *mod, *ver, *dim));
}

int call(gmfclosemesh)(int *idx)
{
	return(GmfCloseMesh(*idx));
}

int call(gmfstatkwd)(int *MshIdx, int *KwdIdx, int *NmbTyp, int *SolSiz, int *TypTab)
{
	if(!strcmp(GmfKwdFmt[ *KwdIdx ][3], "sr"))
		return(GmfStatKwd(*MshIdx, *KwdIdx, NmbTyp, SolSiz, TypTab));
	else
		return(GmfStatKwd(*MshIdx, *KwdIdx));
}

int call(gmfgotokwd)(int *MshIdx, int *KwdIdx)
{
	return(GmfGotoKwd(*MshIdx, *KwdIdx));
}

int call(gmfsetkwd)(int *MshIdx, int *KwdIdx, int *NmbLin, int *NmbTyp, int *TypTab)
{
	if(!strcmp(GmfKwdFmt[ *KwdIdx ][3], "sr"))
		return(GmfSetKwd(*MshIdx, *KwdIdx, *NmbLin, *NmbTyp, TypTab));
	else if(strlen(GmfKwdFmt[ *KwdIdx ][2]))
		return(GmfSetKwd(*MshIdx, *KwdIdx, *NmbLin));
	else
		return(GmfSetKwd(*MshIdx, *KwdIdx));
}

int call(gmfgetlin)(int *MshIdx, int *KwdIdx, ...)
{
	int i, ret = 0;
	va_list ArgLst;
	void *ArgTab[20];
	GmfMshSct *msh = GmfMshTab[ *MshIdx ];
	KwdSct *kwd = &msh->KwdTab[ *KwdIdx ];

	va_start(ArgLst, KwdIdx);

	for(i=0;i<kwd->SolSiz;i++)
		ArgTab[i] = va_arg(ArgLst, void *);

	va_end(ArgLst);

	switch(*KwdIdx)
	{
		case GmfVertices :
		{
			if(msh->dim == 2)
				ret = GmfGetLin(*MshIdx, *KwdIdx, ARG(ArgTab,3));
			else
				ret = GmfGetLin(*MshIdx, *KwdIdx, ARG(ArgTab,4));
		}break;

		case GmfEdges          : ret = GmfGetLin(*MshIdx, *KwdIdx, ARG(ArgTab,3)); break;
		case GmfTriangles      : ret = GmfGetLin(*MshIdx, *KwdIdx, ARG(ArgTab,4)); break;
		case GmfQuadrilaterals : ret = GmfGetLin(*MshIdx, *KwdIdx, ARG(ArgTab,5)); break;
		case GmfTetrahedra     : ret = GmfGetLin(*MshIdx, *KwdIdx, ARG(ArgTab,5)); break;
		case GmfPrisms         : ret = GmfGetLin(*MshIdx, *KwdIdx, ARG(ArgTab,7)); break;
		case GmfCorners        : ret = GmfGetLin(*MshIdx, *KwdIdx, ARG(ArgTab,1)); break;
		case GmfRidges         : ret = GmfGetLin(*MshIdx, *KwdIdx, ARG(ArgTab,1)); break;
		case GmfPyramids       : ret = GmfGetLin(*MshIdx, *KwdIdx, ARG(ArgTab,6)); break;
	}

	return(ret);
}

int call(gmfgetblock)(int *MshIdx, int *KwdIdx, ...)
{
	int i, ret = 0;
	va_list ArgLst;
	void *ItmOneTab[20], *ItmTwoTab[20];
	int ItmTypTab[20];
	GmfMshSct *msh = GmfMshTab[ *MshIdx ];
	KwdSct *kwd = &msh->KwdTab[ *KwdIdx ];

	va_start(ArgLst, KwdIdx);

	for(i=0;i<kwd->SolSiz;i++)
	{
		ItmTypTab[i] = *va_arg(ArgLst, int *);
		ItmOneTab[i] = va_arg(ArgLst, void *);
		ItmTwoTab[i] = va_arg(ArgLst, void *);
	}

	va_end(ArgLst);

	switch(*KwdIdx)
	{
		case GmfVertices :
		{
			if(msh->dim == 2)
				ret = GmfGetBlock(*MshIdx, *KwdIdx, TARG(ItmTypTab,ItmOneTab,ItmTwoTab,3));
			else
				ret = GmfGetBlock(*MshIdx, *KwdIdx, TARG(ItmTypTab,ItmOneTab,ItmTwoTab,4));
		}break;

		case GmfEdges          : ret = GmfGetBlock(*MshIdx, *KwdIdx, TARG(ItmTypTab,ItmOneTab,ItmTwoTab,3)); break;
		case GmfTriangles      : ret = GmfGetBlock(*MshIdx, *KwdIdx, TARG(ItmTypTab,ItmOneTab,ItmTwoTab,4)); break;
		case GmfQuadrilaterals : ret = GmfGetBlock(*MshIdx, *KwdIdx, TARG(ItmTypTab,ItmOneTab,ItmTwoTab,5)); break;
		case GmfTetrahedra     : ret = GmfGetBlock(*MshIdx, *KwdIdx, TARG(ItmTypTab,ItmOneTab,ItmTwoTab,5)); break;
		case GmfPrisms         : ret = GmfGetBlock(*MshIdx, *KwdIdx, TARG(ItmTypTab,ItmOneTab,ItmTwoTab,7)); break;
		case GmfCorners        : ret = GmfGetBlock(*MshIdx, *KwdIdx, TARG(ItmTypTab,ItmOneTab,ItmTwoTab,1)); break;
		case GmfRidges         : ret = GmfGetBlock(*MshIdx, *KwdIdx, TARG(ItmTypTab,ItmOneTab,ItmTwoTab,1)); break;
		case GmfPyramids       : ret = GmfGetBlock(*MshIdx, *KwdIdx, TARG(ItmTypTab,ItmOneTab,ItmTwoTab,6)); break;
	}

	return(ret);
}

int call(gmfsetblock)(int *MshIdx, int *KwdIdx, ...)
{
	int i, ret = 0;
	va_list ArgLst;
	void *ItmOneTab[20], *ItmTwoTab[20];
	int ItmTypTab[20];
	GmfMshSct *msh = GmfMshTab[ *MshIdx ];
	KwdSct *kwd = &msh->KwdTab[ *KwdIdx ];

	va_start(ArgLst, KwdIdx);

	for(i=0;i<kwd->SolSiz;i++)
	{
		ItmTypTab[i] = *va_arg(ArgLst, int *);
		ItmOneTab[i] = va_arg(ArgLst, void *);
		ItmTwoTab[i] = va_arg(ArgLst, void *);
	}

	va_end(ArgLst);

	switch(*KwdIdx)
	{
		case GmfVertices :
		{
			if(msh->dim == 2)
				ret = GmfSetBlock(*MshIdx, *KwdIdx, TARG(ItmTypTab,ItmOneTab,ItmTwoTab,3));
			else
				ret = GmfSetBlock(*MshIdx, *KwdIdx, TARG(ItmTypTab,ItmOneTab,ItmTwoTab,4));
		}break;

		case GmfEdges          : ret = GmfSetBlock(*MshIdx, *KwdIdx, TARG(ItmTypTab,ItmOneTab,ItmTwoTab,3)); break;
		case GmfTriangles      : ret = GmfSetBlock(*MshIdx, *KwdIdx, TARG(ItmTypTab,ItmOneTab,ItmTwoTab,4)); break;
		case GmfQuadrilaterals : ret = GmfSetBlock(*MshIdx, *KwdIdx, TARG(ItmTypTab,ItmOneTab,ItmTwoTab,5)); break;
		case GmfTetrahedra     : ret = GmfSetBlock(*MshIdx, *KwdIdx, TARG(ItmTypTab,ItmOneTab,ItmTwoTab,5)); break;
		case GmfPrisms         : ret = GmfSetBlock(*MshIdx, *KwdIdx, TARG(ItmTypTab,ItmOneTab,ItmTwoTab,7)); break;
		case GmfCorners        : ret = GmfSetBlock(*MshIdx, *KwdIdx, TARG(ItmTypTab,ItmOneTab,ItmTwoTab,1)); break;
		case GmfRidges         : ret = GmfSetBlock(*MshIdx, *KwdIdx, TARG(ItmTypTab,ItmOneTab,ItmTwoTab,1)); break;
		case GmfPyramids       : ret = GmfSetBlock(*MshIdx, *KwdIdx, TARG(ItmTypTab,ItmOneTab,ItmTwoTab,6)); break;
	}

	return(ret);
}
