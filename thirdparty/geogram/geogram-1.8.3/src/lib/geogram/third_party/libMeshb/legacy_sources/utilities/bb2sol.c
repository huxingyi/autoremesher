

/*----------------------------------------------------------*/
/*															*/
/*							BB2SOL							*/
/*															*/
/*----------------------------------------------------------*/
/*															*/
/*	Description:		convert a .bb or .BB file in .sol	*/
/*	Author:				Loic MARECHAL						*/
/*	Creation date:		mar  8 2004							*/
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

main(int ArgCnt, char **ArgVec)
{
	int i, j, NmbLin, NmbTyp, TypTab[ GmfMaxTyp ], InpTyp, SolEleTyp, SolSiz=0, dim, idx=0, SolIdx, BBIter;
	float buf[ GmfMaxTyp ], BBTime;
	char *InpNam, *OutNam, *TypStr;
	FILE *hdl;

	/* Parse command-line arguments */

	switch(ArgCnt)
	{
		case 1 :
		{
			printf("\nBB2SOL v1.2,   feb 26 2007,   written by Loic MARECHAL\n");
			printf(" Usage : bb2sol source_name.bb destination_name.solb support_element\n");
			printf(" where support_element is : ver | edg | qad | tet | pen | hex\n\n");
			return(0);
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
				SolEleTyp = GmfSolAtVertices;
			else if(strstr(TypStr, "edg"))
				SolEleTyp = GmfSolAtEdges;
			else if(strstr(TypStr, "tri"))
				SolEleTyp = GmfSolAtTriangles;
			else if(strstr(TypStr, "qad"))
				SolEleTyp = GmfSolAtQuadrilaterals;
			else if(strstr(TypStr, "tet"))
				SolEleTyp = GmfSolAtTetrahedra;
			else if(strstr(TypStr, "pen"))
				SolEleTyp = GmfSolAtPentahedra;
			else if(strstr(TypStr, "hex"))
				SolEleTyp = GmfSolAtHexahedra;
			else
			{
				fprintf(stderr,"Wrong type of element : %s\n", TypStr);
				return(1);
			}
		}break;
	}

	/* Open the .bb or .BB file for reading */

	if(strstr(InpNam, ".bb"))
		InpTyp = bb;
	else if(strstr(InpNam, ".BB"))
		InpTyp = BB;
	else
	{
		fprintf(stderr,"Source of error : BB2SOL / OPEN_BB\n");
		fprintf(stderr,"%s is not a .bb nor a .BB file\n", InpNam);
		return(1);
	}

	if(!(hdl = fopen(InpNam, "r")))
	{
		fprintf(stderr,"Source of error : BB2SOL / OPEN_BB\n");
		fprintf(stderr,"Cannot open %s\n", InpNam);
		return(1);
	}

	/* Read and check the dimension */

	fscanf(hdl, "%d", &dim);

	if( (dim != 3) && (dim != 2) )
	{
		fprintf(stderr,"Source of error : BB2SOL / READ_BB\n");
		fprintf(stderr,"Wrong dimension : %d\n", dim);
		return(1);
	}

	/* Create a .sol file */

	if(!(SolIdx = GmfOpenMesh(OutNam, GmfWrite, GmfFloat, dim)))
	{
		fprintf(stderr,"Source of error : BB2SOL / OPEN_MESH\n");
		fprintf(stderr,"Cannot open %s\n", OutNam);
		return(1);
	}

	/* bb case */

	if(InpTyp == bb)
	{
		NmbTyp = 1;

		fscanf(hdl, "%d", &InpTyp);

		if(InpTyp == 1)
			TypTab[0] = GmfSca;
		else if(InpTyp == dim)
			TypTab[0] = GmfVec;
		else if(InpTyp == (dim * (dim + 1)) / 2)
			TypTab[0] = GmfSymMat;
		else if(InpTyp == dim * dim)
			TypTab[0] = GmfMat;
		else
		{
			fprintf(stderr,"Source of error : BB2SOL / READ_BB\n");
			fprintf(stderr,"Wrong solution size : %d\n", InpTyp);
			return(1);
		}
	}
	else /* BB case */
	{
		/* Read and check the number of types */

		fscanf(hdl, "%d", &NmbTyp);

		if( (NmbTyp < 1) || (NmbTyp > GmfMaxTyp) )
		{
			fprintf(stderr,"Source of error : BB2SOL / READ_BB\n");
			fprintf(stderr,"Wrong number of types : %d\n", NmbTyp);
			return(1);
		}

		/* Read each types, store them in the sol header */

		for(i=0;i<NmbTyp;i++)
		{
			fscanf(hdl, "%d", &j);

			if(j == 1)
				TypTab[i] = GmfSca;
			else if( (j == 2) && (dim == 2) )
				TypTab[i] = GmfVec;
			else if( (j == 3) && (dim == 2) )
				TypTab[i] = GmfSymMat;
			else if( (j == 3) && (dim == 3) )
				TypTab[i] = GmfVec;
			else if( (j == 4) && (dim == 2) )
				TypTab[i] = GmfMat;
			else if( (j == 6) && (dim == 3) )
				TypTab[i] = GmfSymMat;
			else if( (j == 9) && (dim == 3) )
				TypTab[i] = GmfMat;
			else
			{
				fprintf(stderr,"Source of error : BB2SOL / READ_BB\n");
				fprintf(stderr,"Wrong solution type : %d\n", j);
				return(1);
			}
		}
	}

	/* Read and check the number of solutions */

	fscanf(hdl, "%d", &NmbLin);

	if(NmbLin < 1)
	{
		fprintf(stderr,"Source of error : BB2SOL / READ_BB\n");
		fprintf(stderr,"Wrong number of solutions : %d\n", NmbLin);
		return(1);
	}

	/* Jump over the solution's reference element */

	fscanf(hdl, "%d", &i);

	/* Compute the solution's size */

	for(i=0;i<NmbTyp;i++)
		switch(TypTab[i])
		{
			case GmfSca : SolSiz++;break;
			case GmfVec : SolSiz += dim;break;
			case GmfSymMat : SolSiz += (dim * (dim+1)) / 2;break;
			case GmfMat : SolSiz += dim * dim;break;
		}

	/* Start reading the solutions */

	GmfSetKwd(SolIdx, SolEleTyp, NmbLin, NmbTyp, TypTab);

	for(i=1;i<=NmbLin;i++)
	{
		for(j=0;j<SolSiz;j++)
			fscanf(hdl, "%f", &buf[j]);

		GmfSetLin(SolIdx, SolEleTyp, buf);
	}

	/* read the optional time and iter ending information */

	if(fscanf(hdl, "%d %f", &BBIter, &BBTime) == 2)
	{
		GmfSetKwd(SolIdx, GmfIterations);
		GmfSetLin(SolIdx, GmfIterations, BBIter);

		GmfSetKwd(SolIdx, GmfTime);
		GmfSetLin(SolIdx, GmfTime, BBTime);
	}

	/* Close both files */

	fclose(hdl);

	if(!GmfCloseMesh(SolIdx))
		return(1);

	return(0);
}
