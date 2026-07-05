

/*----------------------------------------------------------*/
/*															*/
/*							SOL2BB							*/
/*															*/
/*----------------------------------------------------------*/
/*															*/
/*	Description:		convert a .sol(b) file into a .BB	*/
/*	Author:				Loic MARECHAL						*/
/*	Creation date:		jul 21 2004							*/
/*	Last modification:	feb 26 2007							*/
/*															*/
/*----------------------------------------------------------*/


/*----------------------------------------------------------*/
/* Includes													*/
/*----------------------------------------------------------*/

#include <libmesh5.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


/*----------------------------------------------------------*/
/* Local defines											*/
/*----------------------------------------------------------*/

#define bb 1
#define BB 2


/*----------------------------------------------------------*/
/* Check args and copy each fields from infile to outfile	*/
/*----------------------------------------------------------*/

int main(int ArgCnt, char **ArgVec)
{
	int i, j, SolTyp=0, idx=0, OutTyp, siz, SolIdx, NmbTyp, SolSiz, TypTab[ GmfMaxTyp ], FilVer, NmbLin, dim;
	float f, FltBuf[ GmfMaxTyp ];
	double d, DblBuf[ GmfMaxTyp ];
	char *InpNam, *OutNam, *TypStr;
	FILE *hdl;

	/* Parse command-line arguments */

	switch(ArgCnt)
	{
		case 1 :
		{
			printf("\nSOL2BB v1.1,   feb 26 2007,   written by Loic MARECHAL\n");
			printf(" Usage : sol2bb source_name.solb destination_name.BB support_element\n");
			printf(" where support_element is : ver | edg | qad | tet | pen | hex\n\n");
			return(1);
		}break;

		case 2 : case 3 :
		{
			fprintf(stderr,"Wrong number of arguments.\n");
			return(1);
		}break;

		case 4 :
		{
			InpNam = *++ArgVec;
			OutNam = *++ArgVec;
			TypStr = *++ArgVec;

			if(strstr(TypStr, "ver"))
				SolTyp = GmfSolAtVertices;
			else if(strstr(TypStr, "edg"))
				SolTyp = GmfSolAtEdges;
			else if(strstr(TypStr, "tri"))
				SolTyp = GmfSolAtTriangles;
			else if(strstr(TypStr, "qad"))
				SolTyp = GmfSolAtQuadrilaterals;
			else if(strstr(TypStr, "tet"))
				SolTyp = GmfSolAtTetrahedra;
			else if(strstr(TypStr, "pen"))
				SolTyp = GmfSolAtPentahedra;
			else if(strstr(TypStr, "hex"))
				SolTyp = GmfSolAtHexahedra;
			else
			{
				fprintf(stderr,"Wrong type of element : %s\n", TypStr);
				return(1);
			}
		}break;
	}

	if(strstr(OutNam, ".bb"))
		OutTyp = bb;
	else if(strstr(OutNam, ".BB"))
		OutTyp = BB;
	else
	{
		fprintf(stderr,"Source of error : BB2SOL / OPEN_BB\n");
		fprintf(stderr,"%s is not a .bb nor a .BB file\n", InpNam);
		return(1);
	}

	/* Open the .solb file for reading */

	if(!(SolIdx = GmfOpenMesh(InpNam, GmfRead, &FilVer, &dim)))
	{
		fprintf(stderr,"Cannot open %s\n", InpNam);
		return(1);
	}

	/* Check if the sol contains the required kw */

	if(!(NmbLin = GmfStatKwd(SolIdx, SolTyp, &NmbTyp, &SolSiz, TypTab)))
	{
		fprintf(stderr,"No such element in sol file.\n");
		return(1);
	}

	/* Create the bb file and write the header */

	if(!(hdl = fopen(OutNam, "wb")))
	{
		fprintf(stderr,"Cannot open %s\n", OutNam);
		return(1);
	}

	if(OutTyp == bb)
	{
		if(TypTab[0] == GmfSca)
			siz = 1;
		else if(TypTab[0] == GmfVec)
			siz = dim;
		else if(TypTab[0] == GmfSymMat)
			siz = dim * (dim + 1) / 2;
		else if(TypTab[0] == GmfMat)
			siz = dim * dim;

		if(SolTyp == GmfSolAtVertices)
			fprintf(hdl, "%d %d %d 2\n", dim, siz, NmbLin);
		else
			fprintf(hdl, "%d %d %d 1\n", dim, siz, NmbLin);
	}
	else
	{
		fprintf(hdl, "%d %d ", dim, NmbTyp);

		for(i=0;i<NmbTyp;i++)
		{
			if(TypTab[i] == GmfSca)
				fprintf(hdl, "1 ");
			else if(TypTab[i] == GmfVec)
				fprintf(hdl, "%d ", dim);
			else if(TypTab[i] == GmfSymMat)
				fprintf(hdl, "%d ", dim * (dim + 1) / 2);
			else if(TypTab[i] == GmfMat)
				fprintf(hdl, "%d ", dim * dim);
			else
			{
				fprintf(stderr,"Wrong solution type : %d\n", TypTab[i]);
				return(1);
			}
		}

		if(SolTyp == GmfSolAtVertices)
			fprintf(hdl, "%d %d\n", NmbLin, 2);
		else
			fprintf(hdl, "%d %d\n", NmbLin, 1);
	}

	/* Then copy the solutions */

	GmfGotoKwd(SolIdx, SolTyp);

	if(FilVer == GmfFloat)
		for(i=0;i<NmbLin; i++)
		{
			GmfGetLin(SolIdx, SolTyp, FltBuf);

			for(j=0;j<SolSiz;j++)
				fprintf(hdl, "%.6g ", FltBuf[j]);

			fprintf(hdl, "\n");
		}
	else if(FilVer == GmfDouble)
		for(i=0;i<NmbLin; i++)
		{
			GmfGetLin(SolIdx, SolTyp, DblBuf);

			for(j=0;j<SolSiz;j++)
				fprintf(hdl, "%.15g ", DblBuf[j]);

			fprintf(hdl, "\n");
		}

	if(GmfStatKwd(SolIdx, GmfIterations))
	{
		GmfGotoKwd(SolIdx, GmfIterations);
		GmfGetLin(SolIdx, GmfIterations, &i);
		fprintf(hdl, "%d\n", i);
	}

	if(GmfStatKwd(SolIdx, GmfTime))
	{
		GmfGotoKwd(SolIdx, GmfTime);

		if(FilVer == GmfFloat)
		{
			GmfGetLin(SolIdx, GmfTime, &f);
			fprintf(hdl, "%.6g\n", f);
		}
		else if(FilVer == GmfDouble)
		{
			GmfGetLin(SolIdx, GmfTime, &d);
			fprintf(hdl, "%.15g\n", d);
		}
	}

	/* Close both files */

	fclose(hdl);

	if(!GmfCloseMesh(SolIdx))
		return(1);

	return(0);
}
