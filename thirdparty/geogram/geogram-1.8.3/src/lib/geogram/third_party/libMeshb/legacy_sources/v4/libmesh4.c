

/*----------------------------------------------------------*/
/*															*/
/*						LIBMESH V 4.0						*/
/*															*/
/*----------------------------------------------------------*/
/*															*/
/*	Description:		handle .meshb file format I/O		*/
/*	Author:				Loic MARECHAL						*/
/*	Creation date:		aug 02 2003							*/
/*	Last modification:	nov 14 2006							*/
/*															*/
/*----------------------------------------------------------*/


/*----------------------------------------------------------*/
/* Includes													*/
/*----------------------------------------------------------*/

#include <string.h>
#include <float.h>
#include <math.h>
#include <ctype.h>
#include "libmesh4.h"


/*----------------------------------------------------------*/
/* Defines													*/
/*----------------------------------------------------------*/

#define WrdSiz 4
#define Asc 1
#define Bin 2
#define MshFil 4
#define SolFil 8
#define MaxMsh 10
#define InfKwd 1
#define FldKwd 2
#define SolKwd 3


/*----------------------------------------------------------*/
/* Global variables											*/
/*----------------------------------------------------------*/

char *KwdFmt[ GmfMaxKwd + 1 ][3] = 
{	{"Reserved", "", ""},
	{"MeshVersionFormatted", "", "i"},
	{"Reserved", "", ""},
	{"Dimension", "", "i"},
	{"Vertices", "i", "dri"},
	{"Edges", "i", "iii"},
	{"Triangles", "i", "iiii"},
	{"Quadrilaterals", "i", "iiiii"},
	{"Tetrahedra", "i", "iiiii"},
	{"Pentahedra", "i", "iiiiiii"},
	{"Hexahedra", "i", "iiiiiiiii"},
	{"Reserved", "", ""},
	{"Reserved", "", ""},
	{"Corners", "i", "i"},
	{"Ridges", "i", "i"},
	{"RequiredVertices", "i", "i"},
	{"RequiredEdges", "i", "i"},
	{"RequiredTriangles", "i", "i"},
	{"RequiredQuadrilaterals", "i", "i"},
	{"TangentAtEdgeVertices", "i", "iii"},
	{"NormalAtVertices", "i", "ii"},
	{"NormalAtTriangleVertices", "i", "iii"},
	{"NormalAtQuadrilateralVertices", "i", "iiii"},
	{"AngleOfCornerBound", "", "r"},
	{"Reserved", "", ""},
	{"Reserved", "", ""},
	{"Reserved", "", ""},
	{"Reserved", "", ""},
	{"Reserved", "", ""},
	{"Reserved", "", ""},
	{"Reserved", "", ""},
	{"Reserved", "", ""},
	{"Reserved", "", ""},
	{"Reserved", "", ""},
	{"Reserved", "", ""},
	{"Reserved", "", ""},
	{"Reserved", "", ""},
	{"Reserved", "", ""},
	{"Reserved", "", ""},
	{"Reserved", "", ""},
	{"Reserved", "", ""},
	{"Reserved", "", ""},
	{"Reserved", "", ""},
	{"Reserved", "", ""},
	{"Reserved", "", ""},
	{"Reserved", "", ""},
	{"Reserved", "", ""},
	{"Reserved", "", ""},
	{"Reserved", "", ""},
	{"Reserved", "", ""},
	{"BoundingBox", "", "drdr"},
	{"Reserved", "", ""},
	{"Reserved", "", ""},
	{"Reserved", "", ""},
	{"End", "", ""},
	{"Reserved", "", ""},
	{"Reserved", "", ""},
	{"Reserved", "", ""},
	{"Reserved", "", ""},
	{"Tangents", "i", "dr"},
	{"Normals", "i", "dr"},
	{"TangentAtVertices", "i", "ii"},
	{"SolAtVertices", "i", "sr"},
	{"SolAtEdges", "i", "sr"},
	{"SolAtTriangles", "i", "sr"},
	{"SolAtQuadrilaterals", "i", "sr"},
	{"SolAtTetrahedra", "i", "sr"},
	{"SolAtPentahedra", "i", "sr"},
	{"SolAtHexahedra", "i", "sr"},
	{"DSolAtVertices", "i", "sr"},
	{"ISolAtVertices", "i", "i"},
	{"ISolAtEdges", "i", "ii"},
	{"ISolAtTriangles", "i", "iii"},
	{"ISolAtQuadrilaterals", "i", "iiii"},
	{"ISolAtTetrahedra", "i", "iiii"},
	{"ISolAtPentahedra", "i", "iiiiii"},
	{"ISolAtHexahedra", "i", "iiiiiiii"},
	{"Iterations","","i"},
	{"Time","","r"},
	{"VertexHack","","drdr"}
 };

int IniGmf = 0;
GmfMshSct *MshTab[ MaxMsh + 1 ];


/*----------------------------------------------------------*/
/* Prototypes of local procedures							*/
/*----------------------------------------------------------*/

static void ScaWrd(GmfMshSct *, unsigned char *);
static void RecWrd(GmfMshSct *, unsigned char *);
static void SwpWrd(unsigned char *);
static int ScaKwdTab(GmfMshSct *);
static void ExpFmt(GmfMshSct *, int);
static void ScaKwdHdr(GmfMshSct *, int);


/*----------------------------------------------------------*/
/* Open a mesh file in read or write mod					*/
/*----------------------------------------------------------*/

GmfMshSct *GmfOpenMesh(char *FilNam, int mod, ...)
{
	int KwdCod, res;
	char str[256];
	va_list par;
	GmfMshSct *msh;

	/*---------------------*/
	/* MESH STRUCTURE INIT */
	/*---------------------*/

	if(!(msh = calloc(1, sizeof(GmfMshSct))))
		return(NULL);

	/* Copy the FilNam into the structure */

	if(strlen(FilNam) + 7 >= GmfStrSiz)
		return(NULL);

	strcpy(msh->FilNam, FilNam);

	/* Store the opening mod (read or write) and guess the filetype (binary or ascii) depending on the extension */

	msh->mod = mod;
	msh->cod = 1;

	if(strstr(msh->FilNam, ".meshb"))
		msh->typ |= (Bin | MshFil);
	else if(strstr(msh->FilNam, ".mesh"))
		msh->typ |= (Asc | MshFil);
	else if(strstr(msh->FilNam, ".solb"))
		msh->typ |= (Bin | SolFil);
	else if(strstr(msh->FilNam, ".sol"))
		msh->typ |= (Asc | SolFil);
	else
		return(NULL);

	/* Open the file in the required mod and initialyse the mesh structure */

	if(msh->mod == GmfRead)
	{

		/*-----------------------*/
		/* OPEN FILE FOR READING */
		/*-----------------------*/

		/* Create the name string and open the file */

		if(!(msh->hdl = fopen(msh->FilNam, "rb")))
			return(NULL);

		/* Read the endian coding tag, the mesh version and the mesh dimension (mandatory kwd) */

		if(msh->typ & Bin)
		{
			fread((unsigned char *)&msh->cod, WrdSiz, 1, msh->hdl);

			if( (msh->cod != 1) && (msh->cod != 16777216) )
				return(NULL);

			ScaWrd(msh, (unsigned char *)&msh->ver);
			ScaWrd(msh, (unsigned char *)&KwdCod);

			if(KwdCod != GmfDimension)
				return(NULL);

			ScaWrd(msh, (unsigned char *)&KwdCod);
			ScaWrd(msh, (unsigned char *)&msh->dim);
		}
		else
		{
			do
			{
				res = fscanf(msh->hdl, "%s", str);
			}while( (res != EOF) && strcmp(str, "MeshVersionFormatted") );

			if(res == EOF)
				return(NULL);

			fscanf(msh->hdl, "%d", &msh->ver);

			do
			{
				res = fscanf(msh->hdl, "%s", str);
			}while( (res != EOF) && strcmp(str, "Dimension") );

			if(res == EOF)
				return(NULL);

			fscanf(msh->hdl, "%d", &msh->dim);
		}

		if( (msh->dim != 2) && (msh->dim != 3) )
			return(NULL);


		/*------------*/
		/* KW READING */
		/*------------*/

		/* Read the list of kw present in the file */

		if(!ScaKwdTab(msh))
			return(NULL);

		/* Read in some special keywords at opening time */

		if(msh->KwdTab[ GmfIterations ].NmbLin)
			GmfReadField(msh, GmfIterations, NULL, &msh->iter);

		if(msh->KwdTab[ GmfTime ].NmbLin)
			GmfReadField(msh, GmfTime, &msh->time, NULL);

		if(msh->KwdTab[ GmfAngleOfCornerBound ].NmbLin)
			GmfReadField(msh, GmfAngleOfCornerBound, &msh->angle, NULL);

		if(msh->KwdTab[ GmfBoundingBox ].NmbLin)
			GmfReadField(msh, GmfBoundingBox, &msh->bbox[0][0], NULL);
	}
	else if(msh->mod == GmfWrite)
	{

		/*-----------------------*/
		/* OPEN FILE FOR WRITING */
		/*-----------------------*/

		/* Check if the user provided a valid dimension */

		va_start(par, mod);
		msh->dim = va_arg(par, int);
		va_end(par);

		if( (msh->dim != 2) && (msh->dim != 3) )
			return(NULL);

		/* Create the mesh file */

		if(!(msh->hdl = fopen(msh->FilNam, "wb")))
			return(NULL);


		/*------------*/
		/* KW WRITING */
		/*------------*/

		/* Initialyse the required fields. The kw will be stored afterward. */

		msh->ver = GmfMshVer;
		msh->cod = 1;

		/* Write the mesh version */

		if(msh->typ & Asc)
			GmfWriteField(msh, GmfVersionFormatted, 0, NULL, &msh->ver);
		else
		{
			RecWrd(msh, (unsigned char *)&msh->cod);
			RecWrd(msh, (unsigned char *)&msh->ver);
		}

		/* Write the mesh dimension */

		GmfWriteField(msh, GmfDimension, 0, NULL, &msh->dim);
	}
	else
		return(NULL);

	return(msh);
}


/*----------------------------------------------------------*/
/* Close a meshfile in the right way						*/
/*----------------------------------------------------------*/

int GmfCloseMesh(GmfMshSct *msh)
{
	int EndCod = GmfEnd, nul=0;

	/* Write down some special keywords before closing the file */

	if(msh->iter)
		GmfWriteField(msh, GmfIterations, 0, NULL, &msh->iter);

	if(msh->time)
		GmfWriteField(msh, GmfTime, 0, &msh->time, NULL);

	if(msh->angle)
		GmfWriteField(msh, GmfAngleOfCornerBound, 0, &msh->angle, NULL);

	if(msh->bbox[0][0])
		GmfWriteField(msh, GmfBoundingBox, 0, &msh->bbox[0][0], NULL);

	/* In write down the "End" kw in write mode */

	if(msh->mod == GmfWrite)
		if(msh->typ & Asc)
			fprintf(msh->hdl, "\n%s\n", KwdFmt[ GmfEnd ][0]);
		else
		{
			RecWrd(msh, (unsigned char *)&EndCod);
			RecWrd(msh, (unsigned char *)&nul);
		}

	/* Close the file and free the mesh structure */

	if(fclose(msh->hdl))
		return(0);
	else
		return(1);

	free(msh);
}


/*----------------------------------------------------------*/
/* Bufferized read of a whole keyword's field				*/
/*----------------------------------------------------------*/

int GmfReadField(GmfMshSct *msh, int KwdCod, float *FltBuf, int *IntBuf)
{
	int i, j, IntIdx=0, FltIdx=0, LocIdx=0, *LocIntBuf;
	float *LocFltBuf;
	KwdSct *kwd = &msh->KwdTab[ KwdCod ];

	/* Check if the kw code is valid */

	if( (KwdCod < 1) || (KwdCod > GmfMaxKwd) || !kwd->typ)
		return(0);

	/* Set the curent position in file to the begining of this kw's data */

	fseek(msh->hdl, kwd->pos, SEEK_SET);

	/* Start reading the field */

	if(msh->typ & Asc)
	{
		for(i=0;i<kwd->NmbLin;i++)
			for(j=0;j<kwd->SolSiz;j++)
				if(kwd->fmt[j] == 'i')
					fscanf(msh->hdl, "%d", &IntBuf[ IntIdx++ ]);
				else
					fscanf(msh->hdl, "%g", &FltBuf[ FltIdx++ ]);
	}
	else
	{
		if(!(LocIntBuf = malloc(kwd->NmbLin * kwd->SolSiz * WrdSiz)))
			return(0);

		LocFltBuf = (float *)LocIntBuf;

		fread(LocIntBuf, WrdSiz, kwd->NmbLin * kwd->SolSiz, msh->hdl);

		if(msh->cod != 1)
			for(i=0;i<kwd->NmbLin * kwd->SolSiz;i++)
				SwpWrd((unsigned char *)&LocIntBuf[i]);

		for(i=0;i<kwd->NmbLin;i++)
			for(j=0;j<kwd->SolSiz;j++)
				if(kwd->fmt[j] == 'i')
					IntBuf[ IntIdx++ ] = LocIntBuf[ LocIdx++ ];
				else if(kwd->fmt[j] == 'r')
					FltBuf[ FltIdx++ ] = LocFltBuf[ LocIdx++ ];

		free(LocIntBuf);
	}

	return(kwd->NmbLin);
}


/*----------------------------------------------------------*/
/* Bufferized write of a whole keyword's field				*/
/*----------------------------------------------------------*/

int GmfWriteField(GmfMshSct *msh, int KwdCod, int NmbLin, float *FltBuf, int *IntBuf, ...)
{
	int i, j, NexPos, CurPos, IntIdx=0, FltIdx=0, LocIdx=0, *LocIntBuf;
	float *LocFltBuf;
	va_list par;
	KwdSct *kwd = &msh->KwdTab[ KwdCod ];

	/* Check if the kw code is valid and if it has not already been written */

	if( (KwdCod < 1) || (KwdCod > GmfMaxKwd) || kwd->NmbLin)
		return(0);

	/* Read further arguments if this kw is solution field and the extra header was not provided by the user */

	if(!strcmp(KwdFmt[ KwdCod ][2], "sr") && !kwd->NmbTyp)
	{
		va_start(par, IntBuf);

		kwd->NmbTyp = va_arg(par, int);

		for(i=0;i<kwd->NmbTyp;i++)
			kwd->TypTab[i] = va_arg(par, int);

		va_end(par);
	}

	/* Setup the kwd info */

	ExpFmt(msh, KwdCod);

	if(!kwd->typ)
		return(0);
	else if(kwd->typ == InfKwd)
		kwd->NmbLin = 1;
	else
		kwd->NmbLin = NmbLin;

	/* Write the header */

	if(msh->typ & Asc)
	{
		fprintf(msh->hdl, "\n%s\n", KwdFmt[ KwdCod ][0]);

		if(kwd->typ != InfKwd)
			fprintf(msh->hdl, "%d\n", kwd->NmbLin);

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
		NexPos = ftell(msh->hdl);
		RecWrd(msh, (unsigned char *)&NexPos);

		if(kwd->typ != InfKwd)
			RecWrd(msh, (unsigned char *)&kwd->NmbLin);

		/* In case of solution field, write the extended header at once */

		if(kwd->typ == SolKwd)
		{
			RecWrd(msh, (unsigned char *)&kwd->NmbTyp);

			for(i=0;i<kwd->NmbTyp;i++)
				RecWrd(msh, (unsigned char *)&kwd->TypTab[i]);
		}
	}

	/* Write the field */

	if(msh->typ & Asc)
		for(i=0;i<kwd->NmbLin;i++)
		{
			for(j=0;j<kwd->SolSiz;j++)
				if(kwd->fmt[j] == 'i')
					fprintf(msh->hdl, "%d ", IntBuf[ IntIdx++ ]);
				else
					fprintf(msh->hdl, "%g ", FltBuf[ FltIdx++ ]);

			fprintf(msh->hdl, "\n");
		}
	else
	{
		if(!(LocIntBuf = malloc(kwd->NmbLin * kwd->SolSiz * WrdSiz)))
			return(0);

		LocFltBuf = (float *)LocIntBuf;

		for(i=0;i<kwd->NmbLin;i++)
			for(j=0;j<kwd->SolSiz;j++)
				if(kwd->fmt[j] == 'i')
					LocIntBuf[ LocIdx++ ] = IntBuf[ IntIdx++ ];
				else
					LocFltBuf[ LocIdx++ ] = FltBuf[ FltIdx++ ];

		fwrite(LocIntBuf, WrdSiz, kwd->NmbLin * kwd->SolSiz, msh->hdl);
		free(LocIntBuf);
	}

	/* Store the next kwd position in binary file */

	if(msh->typ & Bin)
	{
		CurPos = ftell(msh->hdl);
		fseek(msh->hdl, NexPos, SEEK_SET);
		RecWrd(msh, (unsigned char *)&CurPos);
		fseek(msh->hdl, CurPos, SEEK_SET);
	}

	return(kwd->NmbLin);
}


/*----------------------------------------------------------*/
/* Easy reading of a single kwd's line						*/
/*----------------------------------------------------------*/

int GmfReadLine(GmfMshSct *msh, int KwdCod, ...)
{
	float *FltPtr;
	int i, *IntPtr;
	va_list par;
	KwdSct *kwd = &msh->KwdTab[ KwdCod ];

	/* Check if the kw code is valid */

	if( (KwdCod < 1) || (KwdCod > GmfMaxKwd) || !kwd->typ || (kwd->typ == InfKwd) || (kwd->CurLin > kwd->NmbLin) )
		return(0);

	fseek(msh->hdl, kwd->pos, SEEK_SET);

	/* Start decoding the arguments */

	va_start(par, KwdCod);

	for(i=0;i<kwd->SolSiz;i++)
	{
		if(kwd->fmt[i] == 'r')
		{
			FltPtr = va_arg(par, float *);

			if(msh->typ & Asc)
				fscanf(msh->hdl, "%f", FltPtr);
			else
				ScaWrd(msh, (unsigned char *)FltPtr);
		}
		else
		{
			IntPtr = va_arg(par, int *);

			if(msh->typ & Asc)
				fscanf(msh->hdl, "%d", IntPtr);
			else
				ScaWrd(msh, (unsigned char *)IntPtr);
		}
	}

	va_end(par);

	kwd->CurLin++;
	kwd->pos = ftell(msh->hdl);

	/* return the number of arguments filled */

	return(kwd->SolSiz);
}


/*----------------------------------------------------------*/
/* Easy writing of a single kwd's line						*/
/*----------------------------------------------------------*/

int GmfWriteLine(GmfMshSct *msh, int KwdCod, ...)
{
	float *FltPtr;
	int i, *IntPtr;
	va_list par;
	KwdSct *kwd = &msh->KwdTab[ KwdCod ];

	/* Check if the kw code is valid */

	if( (KwdCod < 1) || (KwdCod > GmfMaxKwd) || !kwd->typ || (kwd->typ == InfKwd) || (kwd->CurLin > kwd->NmbLin) )
		return(0);

	fseek(msh->hdl, kwd->pos, SEEK_SET);

	/* Start decoding the arguments */

	va_start(par, KwdCod);

	for(i=0;i<kwd->SolSiz;i++)
	{
		if(kwd->fmt[i] == 'r')
		{
			FltPtr = va_arg(par, float *);

			if(msh->typ & Asc)
				fprintf(msh->hdl, "%f", *FltPtr);
			else
				RecWrd(msh, (unsigned char *)FltPtr);
		}
		else
		{
			IntPtr = va_arg(par, int *);

			if(msh->typ & Asc)
				fprintf(msh->hdl, "%d", *IntPtr);
			else
				RecWrd(msh, (unsigned char *)IntPtr);
		}
	}

	va_end(par);

	kwd->CurLin++;
	kwd->pos = ftell(msh->hdl);

	/* return the number of arguments filled */

	return(kwd->SolSiz);
}


/*----------------------------------------------------------*/
/* Find every kw present in a meshfile						*/
/*----------------------------------------------------------*/

static int ScaKwdTab(GmfMshSct *msh)
{
	int KwdCod, NexPos, CurPos, EndPos;
	char str[256];

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
					if(!strcmp(str, KwdFmt[ KwdCod ][0]))
					{
						ScaKwdHdr(msh, KwdCod);
						break;
					}
			}
			else if(str[0] == '#')
				while(fgetc(msh->hdl) != '\n');
		}
	}
	else
	{
		/* Get file size */

		CurPos = ftell(msh->hdl);
		fseek(msh->hdl, 0, SEEK_END);
		EndPos = ftell(msh->hdl);
		fseek(msh->hdl, CurPos, SEEK_SET);

		/* Jump through kwd positions in the file */

		do
		{
			/* Get the kwd code and the next kwd position */

			ScaWrd(msh, (unsigned char *)&KwdCod);
			ScaWrd(msh, (unsigned char *)&NexPos);

			if(NexPos > EndPos)
				return(0);

			/* Check if this kwd belongs to this mesh version */

			if( (KwdCod >= 1) && (KwdCod <= GmfMaxKwd) )
				ScaKwdHdr(msh, KwdCod);

			/* Go to the next kwd */

			if(NexPos)
				fseek(msh->hdl, NexPos, SEEK_SET);
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

	if(!strcmp("i", KwdFmt[ KwdCod ][1]))
	{
		if(msh->typ & Asc)
			fscanf(msh->hdl, "%d", &kwd->NmbLin);
		else
			ScaWrd(msh, (unsigned char *)&kwd->NmbLin);
	}
	else
		kwd->NmbLin = 1;

	if(!strcmp("sr", KwdFmt[ KwdCod ][2]))
	{
		if(msh->typ & Asc)
		{
			fscanf(msh->hdl, "%d", &kwd->NmbTyp);

			for(i=0;i<kwd->NmbTyp;i++)
				fscanf(msh->hdl, "%d", &kwd->TypTab[i]);
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
	int i, j, TmpSiz=0;
	char chr, *InpFmt = KwdFmt[ KwdCod ][2];
	KwdSct *kwd = &msh->KwdTab[ KwdCod ];

	/* Check if this kw has a format */

	if(!strlen(InpFmt))
		return;

	/* Set the kwd's type */

	if(!strlen(KwdFmt[ KwdCod ][1]))
		kwd->typ = InfKwd;
	else if(!strcmp(InpFmt, "sr"))
		kwd->typ = SolKwd;
	else
		kwd->typ = FldKwd;

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

	i = 0;

	while(i < strlen(InpFmt))
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
}


/*----------------------------------------------------------*/
/* Read a four bytes word in a mesh file					*/
/*----------------------------------------------------------*/

static void ScaWrd(GmfMshSct *msh, unsigned char *wrd)
{
	fread(wrd, WrdSiz, 1, msh->hdl);

	if(msh->cod != 1)
		SwpWrd(wrd);
}


/*----------------------------------------------------------*/
/* Write a four bytes word in a mesh file					*/
/*----------------------------------------------------------*/

static void RecWrd(GmfMshSct *msh, unsigned char *wrd)
{
	fwrite(wrd, WrdSiz, 1, msh->hdl);
}


/*----------------------------------------------------------*/
/* Convert little endian word <-> big endian				*/
/*----------------------------------------------------------*/

static void SwpWrd(unsigned char *wrd)
{
	unsigned char swp;

	swp = wrd[3];
	wrd[3] = wrd[0];
	wrd[0] = swp;

	swp = wrd[2];
	wrd[2] = wrd[1];
	wrd[1] = swp;
}


/*----------------------------------------------------------*/
/* Open a mesh file from fortran77							*/
/*----------------------------------------------------------*/

int call(gmfopenmeshf77)(char *FilNam, int *mod, int *dim, int KwdTab[ GmfMaxKwd ][ GmfMaxTyp + 2 ], int StrSiz)
{
	int i, j, idx=0;
	GmfMshSct *msh;
	char TmpNam[256];

	/* Clear the mesh table at first run */

	if(!IniGmf)
	{
		for(i=1;i<=MaxMsh;i++)
			MshTab[i] = NULL;

		IniGmf = 1;
	}

	/* Allocate a new mesh struct associated with an index */

	for(idx=1;idx<=MaxMsh;idx++)
		if(!MshTab[ idx ])
			break;

	/* Copy the fortran filename into a c string */

	for(i=0;i<StrSiz;i++)
		TmpNam[i] = FilNam[i];

	TmpNam[ StrSiz ] = 0;

	if(*mod == GmfRead)
	{
		if(!(msh = GmfOpenMesh(TmpNam, GmfRead)))
			return(0);

		MshTab[ idx ] = msh;
		*dim = msh->dim;

		for(i=1;i<GmfMaxKwd;i++)
		{
			KwdTab[i-1][0] = msh->KwdTab[i].NmbLin;
			KwdTab[i-1][1] = msh->KwdTab[i].NmbTyp;

			for(j=0;j<GmfMaxTyp;j++)
				KwdTab[i-1][ j+2 ] = msh->KwdTab[i].TypTab[j];
		}
	}
	else
	{
		if(!(msh = GmfOpenMesh(TmpNam, GmfWrite, *dim)))
			return(0);

		MshTab[ idx ] = msh;
	}

	return(idx);
}


/*----------------------------------------------------------*/
/* Close a meshfile (from f77)								*/ 
/*----------------------------------------------------------*/

int call(gmfclosemeshf77)(int *idx)
{
	GmfMshSct *msh = MshTab[ *idx ];

	if( (*idx < 1) || (*idx > MaxMsh) )
		return(0);

	MshTab[ *idx ] = NULL;

	return(GmfCloseMesh(msh));
}


/*----------------------------------------------------------*/
/* Read a whole kwd field from f77							*/
/*----------------------------------------------------------*/

int call(gmfreadfieldf77)(int *idx, int *kwd, float *FltTab, int *IntTab)
{
	if( (*idx < 1) || (*idx > MaxMsh) )
		return(0);

	return(GmfReadField(MshTab[ *idx ], *kwd, FltTab, IntTab));
}


/*----------------------------------------------------------*/
/* Write a whole kwd field from f77							*/
/*----------------------------------------------------------*/

int call(gmfwritefieldf77)(int *idx, int *kwd, int *NmbLin, int *NmbTyp, int *TypTab, float *FltTab, int *IntTab)
{
	int i;

	if( (*idx < 1) || (*idx > MaxMsh) )
		return(0);

	if(*NmbTyp)
	{
		if( (*kwd < 1) || (*kwd > GmfMaxKwd) || strcmp(KwdFmt[ *kwd ][2], "sr") )
			return(0);

		MshTab[ *idx ]->KwdTab[ *kwd ].NmbTyp = *NmbTyp;

		for(i=0;i<*NmbTyp;i++)
			MshTab[ *idx ]->KwdTab[ *kwd ].TypTab[i] = TypTab[i];
	}

	return(GmfWriteField(MshTab[ *idx ], *kwd, *NmbLin, FltTab, IntTab));
}
