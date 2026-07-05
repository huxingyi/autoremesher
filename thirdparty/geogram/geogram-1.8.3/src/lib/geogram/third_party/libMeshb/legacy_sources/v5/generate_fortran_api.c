

/*----------------------------------------------------------*/
/*															*/
/*						LIBMESH V 5.41						*/
/*															*/
/*----------------------------------------------------------*/
/*															*/
/*	Description:		generate automatically fortran API	*/
/*	Author:				Loic MARECHAL						*/
/*	Creation date:		jun 15 2009							*/
/*	Last modification:	may 17 2011							*/
/*															*/
/*----------------------------------------------------------*/


/*----------------------------------------------------------*/
/* Includes													*/
/*----------------------------------------------------------*/


#include "libmesh5.c"


/*----------------------------------------------------------*/
/* Generate fortran api automatically						*/
/*----------------------------------------------------------*/

int main()
{
	int i, j, k, l, m, DimMod, FltMod;
	char *ActStr[2][2] = { {"Get","Set"}, {"", "*"} };
	char *DimStr[2][2] = { {"", ""}, {"2d", "3d"} };
	char *FltStr[4][2] = { {"", ""}, {"R4", "R8"}, {"float", "double"}, {"real(4)", "real(8)"} };
	char PrcNam[256], PrcPar[256], F77Nam[256], F77Par[256];
	KwdSct *kwd;
	FILE *hdl, *F90Hdl, *F77Hdl;
	GmfMshSct msh;

	/* Create ".c" and a ".h" files */

	system("cp head_c libmesh5_fortran_api.c");
	hdl = fopen("libmesh5_fortran_api.c", "a");

	system("cp head_f90 M_libmesh5_api.f90");
	F90Hdl = fopen("M_libmesh5_api.f90", "a");
	F77Hdl = fopen("libmesh5_api.ins", "w");

	/* Write headers */

	fprintf(hdl, "/* Generated automatically by libmesh5.2 */\n\n");

	fprintf(F77Hdl, "c Generated automatically by libmesh5.2 \n\n");

	fprintf(F77Hdl, "      external GmfOpenMeshF77\n");
	fprintf(F77Hdl, "      external GmfCloseMeshF77\n");
	fprintf(F77Hdl, "      external GmfStatKwdF77\n");
	fprintf(F77Hdl, "      external GmfSetKwdF77\n");
	fprintf(F77Hdl, "      external GmfGotoKwdF77\n");
	fprintf(F77Hdl, "\n");

	fprintf(F77Hdl, "      integer GmfOpenMeshF77\n");
	fprintf(F77Hdl, "      integer GmfCloseMeshF77\n");
	fprintf(F77Hdl, "      integer GmfStatKwdF77\n");
	fprintf(F77Hdl, "      integer GmfSetKwdF77\n");
	fprintf(F77Hdl, "      integer GmfGotoKwdF77\n");
	fprintf(F77Hdl, "\n");

	/* Start generating keyword's prototypes */

	for(i=1;i<=GmfMaxKwd;i++)
	{
		if(strcmp(GmfKwdFmt[i][2], "i"))
			continue;

		kwd = &msh.KwdTab[i];

		if(strchr(GmfKwdFmt[i][3], 'd'))
			DimMod = 1;
		else
			DimMod = 0;

		if(strchr(GmfKwdFmt[i][3], 'r'))
			FltMod = 1;
		else
			FltMod = 0;

		for(j=0;j<2;j++)
			for(k=0;k<=DimMod;k++)
				for(l=0;l<=FltMod;l++)
				{
					/* Generate function name's strings */

					sprintf(F77Nam, "Gmf%s%s%s%s",ActStr[0][j], GmfKwdFmt[i][1], DimStr[ DimMod ][k], FltStr[ FltMod ][l]);
					sprintf(PrcNam, "Gmf%sLin",ActStr[0][j]);

					for(m=0;m<strlen(F77Nam);m++)
						F77Nam[m] = tolower(F77Nam[m]);


					/* Write f77api.c */

					fprintf(hdl, "void call(%s)(int *MshIdx", F77Nam);

					msh.dim = k+2;
					ExpFmt(&msh, i);

					if(kwd->typ == RegKwd)
					{
						for(m=0;m<kwd->SolSiz;m++)
							if(kwd->fmt[m] == 'i')
								fprintf(hdl, ", int *i%d", m);
							else if(kwd->fmt[m] == 'r')
								fprintf(hdl, ", %s *r%d", FltStr[2][l], m);
					}
					else if(kwd->typ == SolKwd)
					{
						fprintf(hdl, ", %s *SolTab", FltStr[2][l]);
					}

					fprintf(hdl, ")\n");

					fprintf(hdl, "{\n	%s(*MshIdx, Gmf%s",PrcNam,GmfKwdFmt[i][0]);

					if(kwd->typ == RegKwd)
					{
						for(m=0;m<kwd->SolSiz;m++)
							if(kwd->fmt[m] == 'i')
								fprintf(hdl, ", %si%d", ActStr[1][j], m);
							else if(kwd->fmt[m] == 'r')
								fprintf(hdl, ", %sr%d", ActStr[1][j], m);
					}
					else if(kwd->typ == SolKwd)
					{
						fprintf(hdl, ", SolTab", FltStr[2][l]);
					}

					fprintf(hdl, ");\n}\n\n");


					/* Write f77api.f90 */

					fprintf(F90Hdl, "\ninterface\n");
					fprintf(F90Hdl, "  subroutine %s(MshIdx", F77Nam);

					msh.dim = k+2;
					ExpFmt(&msh, i);

					if(kwd->typ == RegKwd)
					{
						for(m=0;m<kwd->SolSiz;m++)
							if(kwd->fmt[m] == 'i')
								fprintf(F90Hdl, ", i%d",m);
							else if(kwd->fmt[m] == 'r')
								fprintf(F90Hdl, ", r%d", m);
					}
					else if(kwd->typ == SolKwd)
					{
						fprintf(F90Hdl, ", SolTab");
					}

					fprintf(F90Hdl, ")\n");

					fprintf(F90Hdl, "  integer :: MshIdx\n");

					if(kwd->typ == RegKwd)
					{
						for(m=0;m<kwd->SolSiz;m++)
							if(kwd->fmt[m] == 'i')
								fprintf(F90Hdl, "  integer :: i%d\n",m);
							else if(kwd->fmt[m] == 'r')
								fprintf(F90Hdl, "  %s :: r%d\n", FltStr[3][l],m);
					}
					else if(kwd->typ == SolKwd)
						fprintf(F90Hdl, "  %s :: SolTab(*)\n", FltStr[3][l]);

					fprintf(F90Hdl, "  end subroutine %s\n", F77Nam);
					fprintf(F90Hdl, "end interface\n");


					/* Write f77api.ins */

					fprintf(F77Hdl, "      external %s\n", F77Nam);
				}
	}

	/* Generate f90 keywords */

	fprintf(F90Hdl, "\n");
	fprintf(F90Hdl, "integer,parameter :: GmfMaxTyp=%d\n", GmfMaxTyp);
	fprintf(F90Hdl, "integer,parameter :: GmfMaxKwd=%d\n", GmfMaxKwd);
	fprintf(F90Hdl, "integer,parameter :: GmfRead=%d\n", GmfRead);
	fprintf(F90Hdl, "integer,parameter :: GmfWrite=%d\n", GmfWrite);
	fprintf(F90Hdl, "integer,parameter :: GmfSca=%d\n", GmfSca);
	fprintf(F90Hdl, "integer,parameter :: GmfVec=%d\n", GmfVec);
	fprintf(F90Hdl, "integer,parameter :: GmfSymMat=%d\n", GmfSymMat);
	fprintf(F90Hdl, "integer,parameter :: GmfMat=%d\n", GmfMat);
	fprintf(F90Hdl, "\n");

	for(i=1;i<=GmfMaxKwd;i++)
		if(strcmp(GmfKwdFmt[i][0], "Reserved"))
			fprintf(F90Hdl, "integer,parameter :: Gmf%s=%d\n", GmfKwdFmt[i][0], i);

	fprintf(F90Hdl, "\nend module M_libmesh5_api\n\n");

	/* Generate f77 keywords */

	fprintf(F77Hdl, "\n");
	fprintf(F77Hdl, "      integer GmfMaxTyp\n");
	fprintf(F77Hdl, "      integer GmfMaxKwd\n");
	fprintf(F77Hdl, "      integer GmfRead\n");
	fprintf(F77Hdl, "      integer GmfWrite\n");
	fprintf(F77Hdl, "      integer GmfSca\n");
	fprintf(F77Hdl, "      integer GmfVec\n");
	fprintf(F77Hdl, "      integer GmfSymMat\n");
	fprintf(F77Hdl, "      integer GmfMat\n");
	fprintf(F77Hdl, "      integer GmfFloat\n");
	fprintf(F77Hdl, "      integer GmfDouble\n");
	fprintf(F77Hdl, "\n");

	fprintf(F77Hdl, "      parameter (GmfMaxTyp=%d)\n", GmfMaxTyp);
	fprintf(F77Hdl, "      parameter (GmfMaxKwd=%d)\n", GmfMaxKwd);
	fprintf(F77Hdl, "      parameter (GmfRead=%d)\n", GmfRead);
	fprintf(F77Hdl, "      parameter (GmfWrite=%d)\n", GmfWrite);
	fprintf(F77Hdl, "      parameter (GmfSca=%d)\n", GmfSca);
	fprintf(F77Hdl, "      parameter (GmfVec=%d)\n", GmfVec);
	fprintf(F77Hdl, "      parameter (GmfSymMat=%d)\n", GmfSymMat);
	fprintf(F77Hdl, "      parameter (GmfMat=%d)\n", GmfMat);
	fprintf(F77Hdl, "      parameter (GmfFloat=%d)\n", GmfFloat);
	fprintf(F77Hdl, "      parameter (GmfDouble=%d)\n", GmfDouble);
	fprintf(F77Hdl, "\n");

	for(i=1;i<=GmfMaxKwd;i++)
		if(strcmp(GmfKwdFmt[i][0], "Reserved"))
			fprintf(F77Hdl, "      integer Gmf%s\n", GmfKwdFmt[i][0]);

	fprintf(F77Hdl, "\n");

	for(i=1;i<=GmfMaxKwd;i++)
		if(strcmp(GmfKwdFmt[i][0], "Reserved"))
			fprintf(F77Hdl, "      parameter (Gmf%s=%d)\n", GmfKwdFmt[i][0], i);

	fclose(hdl);
	fclose(F90Hdl);
	fclose(F77Hdl);
}
