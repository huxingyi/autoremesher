#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libmeshb7.h"


/* simulate exception */
#include <setjmp.h>
jmp_buf ex_buf__;

extern const char *GmfKwdFmt[ GmfMaxKwd + 1 ][3];

int main(int argc, char *argv[])
{
  int        NmbVer, dim, deg, FilVer, NmbCor, NmbRidg;
  int        NmbTet, NmbHex, NmbPri, NmbPyr, NmbTri, NmbQua, NmbEdg;
  int        NmbTetP2, NmbHexQ2, NmbPriP2, NmbPyrP2, NmbTriP2, NmbQuaQ2, NmbEdgP2, NmbNod;
  int        NmbTetP3, NmbHexQ3, NmbPriP3, NmbPyrP3, NmbTriP3, NmbQuaQ3, NmbEdgP3;
  int        NmbTetP4, NmbHexQ4, NmbPriP4, NmbPyrP4, NmbTriP4, NmbQuaQ4, NmbEdgP4;
  int        OrdTet, OrdHex, OrdPri, OrdPyr, OrdTri, OrdQua, OrdEdg;
  int        OrdTetP2, OrdHexQ2, OrdPriP2, OrdPyrP2, OrdTriP2, OrdQuaQ2, OrdEdgP2;
  int        OrdTetP3, OrdHexQ3, OrdPriP3, OrdPyrP3, OrdTriP3, OrdQuaQ3, OrdEdgP3;
  int        OrdTetP4, OrdHexQ4, OrdPriP4, OrdPyrP4, OrdTriP4, OrdQuaQ4, OrdEdgP4;
  int64_t    InpMsh = 0, InpSol = 0;
  int        i, j, ite = 0;
  float      flt;
  double     dbl, time = 0.0;
  int        NbrLin, SolSiz, NbrTyp, TypTab[ GmfMaxTyp ];
  

  deg    = 1;
  NmbNod = 0;


  if (argc <= 1) {
    printf(" USAGE : mshinfo name [-check]\n");
    exit(1);
  }

  char name[1024];
  char sol[1024];

  strcpy(name,argv[1]);
  strcpy(sol,argv[1]);


  char *sub = NULL;
  sub = strstr(name,".mesh"); // check if it has the extension .mesh[b]


  char *subsol = NULL;
  subsol = strstr(name,".sol"); // check if it has the extension .sol[b]

// Mesh part

//-- test the reading of the file as a mesh
  if (subsol == NULL) { //--- no extension sol[b], check if it is a mesh name without extnsion
    if(sub != NULL) sol[sub-name]='\0'; // change file name to be opened as sol even if mesh extension is given

    if (sub != NULL)
      InpMsh = GmfOpenMesh(name, GmfRead, &FilVer, &dim);
    else {
      strcat(name,".meshb");
      InpMsh = GmfOpenMesh(name, GmfRead, &FilVer, &dim);
      if(InpMsh == 0){
        int ln = strlen(name);
        name[ln -1] = '\0';
        InpMsh = GmfOpenMesh(name, GmfRead, &FilVer, &dim);
      }
    }
  }

  if ( InpMsh != 0 && subsol == NULL ) {
    printf("Mesh informations :\n");
   /* Get number of entities*/
       
    NmbVer   = GmfStatKwd(InpMsh, GmfVertices);
    NmbCor   = GmfStatKwd(InpMsh, GmfCorners);
    NmbRidg  = GmfStatKwd(InpMsh, GmfRidges);
    NmbTet   = GmfStatKwd(InpMsh, GmfTetrahedra);
    NmbHex   = GmfStatKwd(InpMsh, GmfHexahedra);
    NmbPyr   = GmfStatKwd(InpMsh, GmfPyramids);
    NmbPri   = GmfStatKwd(InpMsh, GmfPrisms);
    NmbTri   = GmfStatKwd(InpMsh, GmfTriangles);
    NmbQua   = GmfStatKwd(InpMsh, GmfQuadrilaterals);
    NmbEdg   = GmfStatKwd(InpMsh, GmfEdges);
    NmbTetP2 = GmfStatKwd(InpMsh, GmfTetrahedraP2);
    NmbHexQ2 = GmfStatKwd(InpMsh, GmfHexahedraQ2);
    NmbPyrP2 = GmfStatKwd(InpMsh, GmfPyramidsP2);
    NmbPriP2 = GmfStatKwd(InpMsh, GmfPrismsP2);
    NmbTriP2 = GmfStatKwd(InpMsh, GmfTrianglesP2);
    NmbQuaQ2 = GmfStatKwd(InpMsh, GmfQuadrilateralsQ2);
    NmbEdgP2 = GmfStatKwd(InpMsh, GmfEdgesP2);
    NmbTetP3 = GmfStatKwd(InpMsh, GmfTetrahedraP3);
    NmbHexQ3 = GmfStatKwd(InpMsh, GmfHexahedraQ3);
    NmbPyrP3 = GmfStatKwd(InpMsh, GmfPyramidsP3);
    NmbPriP3 = GmfStatKwd(InpMsh, GmfPrismsP3);
    NmbTriP3 = GmfStatKwd(InpMsh, GmfTrianglesP3);
    NmbQuaQ3 = GmfStatKwd(InpMsh, GmfQuadrilateralsQ3);
    NmbEdgP3 = GmfStatKwd(InpMsh, GmfEdgesP3);
    NmbTetP4 = GmfStatKwd(InpMsh, GmfTetrahedraP4);
    NmbHexQ4 = GmfStatKwd(InpMsh, GmfHexahedraQ4);
    NmbPyrP4 = GmfStatKwd(InpMsh, GmfPyramidsP4);
    NmbPriP4 = GmfStatKwd(InpMsh, GmfPrismsP4);
    NmbTriP4 = GmfStatKwd(InpMsh, GmfTrianglesP4);
    NmbQuaQ4 = GmfStatKwd(InpMsh, GmfQuadrilateralsQ4);
    NmbEdgP4 = GmfStatKwd(InpMsh, GmfEdgesP4);

    //-- Nodes ordering
    OrdTet   = GmfStatKwd(InpMsh, GmfTetrahedraP1Ordering);
    OrdHex   = GmfStatKwd(InpMsh, GmfHexahedraQ1Ordering);
    OrdPyr   = GmfStatKwd(InpMsh, GmfPyramidsP1Ordering);
    OrdPri   = GmfStatKwd(InpMsh, GmfPrismsP1Ordering);
    OrdTri   = GmfStatKwd(InpMsh, GmfTrianglesP1Ordering);
    OrdQua   = GmfStatKwd(InpMsh, GmfQuadrilateralsQ1Ordering);
    OrdEdg   = GmfStatKwd(InpMsh, GmfEdgesP1Ordering);
    OrdTetP2 = GmfStatKwd(InpMsh, GmfTetrahedraP2Ordering);
    OrdHexQ2 = GmfStatKwd(InpMsh, GmfHexahedraQ2Ordering);
    OrdPyrP2 = GmfStatKwd(InpMsh, GmfPyramidsP2Ordering);
    OrdPriP2 = GmfStatKwd(InpMsh, GmfPrismsP2Ordering);
    OrdTriP2 = GmfStatKwd(InpMsh, GmfTrianglesP2Ordering);
    OrdQuaQ2 = GmfStatKwd(InpMsh, GmfQuadrilateralsQ2Ordering);
    OrdEdgP2 = GmfStatKwd(InpMsh, GmfEdgesP2Ordering);
    OrdTetP3 = GmfStatKwd(InpMsh, GmfTetrahedraP3Ordering);
    OrdHexQ3 = GmfStatKwd(InpMsh, GmfHexahedraQ3Ordering);
    OrdPyrP3 = GmfStatKwd(InpMsh, GmfPyramidsP3Ordering);
    OrdPriP3 = GmfStatKwd(InpMsh, GmfPrismsP3Ordering);
    OrdTriP3 = GmfStatKwd(InpMsh, GmfTrianglesP3Ordering);
    OrdQuaQ3 = GmfStatKwd(InpMsh, GmfQuadrilateralsQ3Ordering);
    OrdEdgP3 = GmfStatKwd(InpMsh, GmfEdgesP3Ordering);
    OrdTetP4 = GmfStatKwd(InpMsh, GmfTetrahedraP4Ordering);
    OrdHexQ4 = GmfStatKwd(InpMsh, GmfHexahedraQ4Ordering);
    OrdPyrP4 = GmfStatKwd(InpMsh, GmfPyramidsP4Ordering);
    OrdPriP4 = GmfStatKwd(InpMsh, GmfPrismsP4Ordering);
    OrdTriP4 = GmfStatKwd(InpMsh, GmfTrianglesP4Ordering);
    OrdQuaQ4 = GmfStatKwd(InpMsh, GmfQuadrilateralsQ4Ordering);
    OrdEdgP4 = GmfStatKwd(InpMsh, GmfEdgesP4Ordering);
 
    printf("version = %d; dim = %d; nbv = %d; (nbc = %d)\n", FilVer, dim, NmbVer, NmbCor);
    
    if ( NmbTet > 0 || NmbTri > 0 || NmbEdg > 0 || NmbHex > 0|| NmbPri > 0 || NmbPyr > 0|| NmbQua > 0 ) {
      printf("P1/Q1 elements :\n");
      if ( NmbTet >0 || NmbTri > 0 || NmbEdg > 0 ) 
        printf("nbtet = %d; nbtri = %d; nbedg = %d; (nbrdg = %d)\n", NmbTet, NmbTri, NmbEdg, NmbRidg);
      if ( NmbHex > 0|| NmbPri > 0 || NmbPyr > 0|| NmbQua > 0 )
        printf("nbhex = %d; nbpri = %d; nbpyr = %d; nbqua = %d\n", NmbHex, NmbPri, NmbPyr, NmbQua);
    }
    
    if ( OrdTet >0 || OrdTri > 0 || OrdEdg > 0 || OrdHex > 0|| OrdPri > 0 || OrdPyr > 0|| OrdQua > 0 ) {
      printf("P1/Q1 ordering :\n");
      if ( OrdTet > 0 || OrdTri > 0 || OrdEdg > 0 ) {
        if ( OrdTet > 0 )
          printf("tet ");
        if ( OrdTri > 0 )
          printf("tri ");
        if ( OrdEdg > 0 )
          printf("edg ");
        printf("\n");
      }
      if ( OrdHex > 0|| OrdPri > 0 || OrdPyr > 0|| OrdQua > 0 ) {
        if ( OrdHex > 0 )
          printf("hex ");
        if ( OrdPri > 0 )
          printf("pri ");
        if ( OrdPyr > 0 )
          printf("pyr ");
        if ( OrdQua > 0 )
          printf("qua ");
        printf("\n");
      }
    }

    if ( NmbTetP2 > 0 || NmbTriP2 > 0 || NmbEdgP2 > 0 || NmbHexQ2 > 0|| NmbPriP2 > 0 || NmbPyrP2 > 0|| NmbQuaQ2 > 0 ) {
      printf("P2/Q2 elements :\n"); 
      if ( NmbTetP2 > 0 || NmbTriP2 > 0 || NmbEdgP2 > 0 )
        printf("nbtetP2 = %d; nbtriP2 = %d; nbedgP2 = %d; (nbrdg = %d)\n", NmbTetP2, NmbTriP2, NmbEdgP2, NmbRidg);
      if ( NmbHexQ2 > 0|| NmbPriP2 > 0 || NmbPyrP2 > 0|| NmbQuaQ2 > 0 )
        printf("nbhexQ2 = %d; nbpriP2 = %d; nbpyrP2 = %d; nbquaQ2 = %d\n", NmbHexQ2, NmbPriP2, NmbPyrP2, NmbQuaQ2);
    }
    
    if ( OrdTetP2 >0 || OrdTriP2 > 0 || OrdEdgP2 > 0 || OrdHexQ2 > 0|| OrdPriP2 > 0 || OrdPyrP2 > 0|| OrdQuaQ2 > 0 ) {
      printf("P2/Q2 ordering :\n");
      if ( OrdTetP2 > 0 || OrdTriP2 > 0 || OrdEdgP2 > 0 ) {
        if ( OrdTetP2 > 0 )
          printf("tetP2 ");
        if ( OrdTriP2 > 0 )
          printf("triP2 ");
        if ( OrdEdgP2 > 0 )
          printf("edgP2 ");
        printf("\n");
      }
      if ( OrdHexQ2 > 0|| OrdPriP2 > 0 || OrdPyrP2 > 0|| OrdQuaQ2 > 0 ) {
        if ( OrdHexQ2 > 0 )
          printf("hexQ2 ");
        if ( OrdPriP2 > 0 )
          printf("priP2 ");
        if ( OrdPyrP2 > 0 )
          printf("pyrP2 ");
        if ( OrdQuaQ2 > 0 )
          printf("quaQ2 ");
        printf("\n");
      }
    }

    if ( NmbTetP3 > 0 || NmbTriP3 > 0 || NmbEdgP3 > 0 || NmbHexQ3 > 0|| NmbPriP3 > 0 || NmbPyrP3 > 0|| NmbQuaQ3 > 0 ) {
      printf("P3/Q3 elements :\n"); 
      if ( NmbTetP3 > 0 || NmbTriP3 > 0 || NmbEdgP3 > 0 )
        printf("nbtetP3 = %d; nbtriP3 = %d; nbedgP3 = %d; (nbrdg = %d)\n", NmbTetP3, NmbTriP3, NmbEdgP3, NmbRidg);
      if ( NmbHexQ3 > 0|| NmbPriP3 > 0 || NmbPyrP3 > 0|| NmbQuaQ3 > 0 )
        printf("nbhexQ3 = %d; nbpriP3 = %d; nbpyrP3 = %d; nbquaQ3 = %d\n", NmbHexQ3, NmbPriP3, NmbPyrP3, NmbQuaQ3);
    }

    if ( OrdTetP3 >0 || OrdTriP3 > 0 || OrdEdgP3 > 0 || OrdHexQ3 > 0|| OrdPriP3 > 0 || OrdPyrP3 > 0|| OrdQuaQ3 > 0 ) {
      printf("P3/Q3 ordering :\n");
      if ( OrdTetP3 > 0 || OrdTriP3 > 0 || OrdEdgP3 > 0 ) {
        if ( OrdTetP3 > 0 )
          printf("tetP3 ");
        if ( OrdTriP3 > 0 )
          printf("triP3 ");
        if ( OrdEdgP3 > 0 )
          printf("edgP3 ");
        printf("\n");
      }
      if ( OrdHexQ3 > 0|| OrdPriP3 > 0 || OrdPyrP3 > 0|| OrdQuaQ3 > 0 ) {
        if ( OrdHexQ3 > 0 )
          printf("hexQ3 ");
        if ( OrdPriP3 > 0 )
          printf("priP3 ");
        if ( OrdPyrP3 > 0 )
          printf("pyrP3 ");
        if ( OrdQuaQ3 > 0 )
          printf("quaQ3 ");
        printf("\n");
      }
    }

    if ( NmbTetP4 > 0 || NmbTriP4 > 0 || NmbEdgP4 > 0 || NmbHexQ4 > 0|| NmbPriP4 > 0 || NmbPyrP4 > 0|| NmbQuaQ4 > 0 ) {
      printf("P4/Q4 elements :\n"); 
      if ( NmbTetP4 > 0 || NmbTriP4 > 0 || NmbEdgP4 > 0 )
        printf("nbtetP4 = %d; nbtriP4 = %d; nbedgP4 = %d; (nbrdg = %d)\n", NmbTetP4, NmbTriP4, NmbEdgP4, NmbRidg);
      if ( NmbHexQ4 > 0|| NmbPriP4 > 0 || NmbPyrP4 > 0|| NmbQuaQ4 > 0 )
        printf("nbhexQ4 = %d; nbpriP4 = %d; nbpyrP4 = %d; nbquaQ4 = %d\n", NmbHexQ4, NmbPriP4, NmbPyrP4, NmbQuaQ4);
    }

    if ( OrdTetP4 >0 || OrdTriP4 > 0 || OrdEdgP4 > 0 || OrdHexQ4 > 0|| OrdPriP4 > 0 || OrdPyrP4 > 0|| OrdQuaQ4 > 0 ) {
      printf("P4/Q4 ordering :\n");
      if ( OrdTetP4 > 0 || OrdTriP4 > 0 || OrdEdgP4 > 0 ) {
        if ( OrdTetP4 > 0 )
          printf("tetP4 ");
        if ( OrdTriP4 > 0 )
          printf("triP4 ");
        if ( OrdEdgP4 > 0 )
          printf("edgP4 ");
        printf("\n");
      }
      if ( OrdHexQ4 > 0|| OrdPriP4 > 0 || OrdPyrP4 > 0|| OrdQuaQ4 > 0 ) {
        if ( OrdHexQ4 > 0 )
          printf("hexQ4 ");
        if ( OrdPriP4 > 0 )
          printf("priP4 ");
        if ( OrdPyrP4 > 0 )
          printf("pyrP4 ");
        if ( OrdQuaQ4 > 0 )
          printf("quaQ4 ");
        printf("\n");
      }
    }

    GmfCloseMesh(InpMsh);
     
  }
  

  // solution part

//-- test the reading of the file as a solution
  if (subsol != NULL) { //--- a file containing char ".sol" is given in input
    InpSol = GmfOpenMesh(sol, GmfRead, &FilVer, &dim);
    if ( InpSol == 0 ) {
      strcat(sol,"b");
      InpSol = GmfOpenMesh(sol, GmfRead, &FilVer, &dim);
    }
  }
  else { //-- check if a solution with the same name as the mesh one exists
    strcat(sol,".sol");
    InpSol = GmfOpenMesh(sol, GmfRead, &FilVer, &dim);
    if ( InpSol == 0 ) {
      strcat(sol,"b");
      InpSol = GmfOpenMesh(sol, GmfRead, &FilVer, &dim);
    }
  }

  if (InpSol!= 0 ) {
    printf("Solution informations :\n");
    if ( GmfStatKwd(InpSol, GmfTime) ) {
      GmfGotoKwd(InpSol, GmfTime);
      if ( FilVer == GmfFloat ) {	// read 32 bits float
       GmfGetLin(InpSol, GmfTime, &flt);
       time = (double)flt;
      }  
      else if ( FilVer == GmfDouble ) {	// read 64 bits float
        GmfGetLin(InpSol, GmfTime, &dbl);
        time = dbl;
      }
    }
    if ( GmfStatKwd(InpSol, GmfIterations) ) {
      GmfGotoKwd(InpSol, GmfIterations);
      GmfGetLin( InpSol, GmfIterations, &ite);  
    }  
    printf("dim = %d; ite = %d; time = %lg\n",dim, ite, time);
    for(i=1; i<=GmfMaxKwd; i++) {
      if( ( (!strcmp(GmfKwdFmt[i][2], "sr")  || !strcmp(GmfKwdFmt[i][2], "hr")) ) && ( (NbrLin = GmfStatKwd(InpSol, i, &NbrTyp, &SolSiz, TypTab, &deg, &NmbNod)) ) ) {
        printf("%s = %d\n", GmfKwdFmt[i][0], NbrLin);
        if ( deg != 1 && NmbNod != 0 )
          printf("deg = %d; nbnod = %d\n",deg, NmbNod);
        printf("type = [");
        for(j=0; j<NbrTyp; j++) {
          if (      TypTab[j] == 1 ) printf(" scalar ");
          else if ( TypTab[j] == 2 ) printf(" vector ");
          else if ( TypTab[j] == 3 ) printf(" metric ");
          else if ( TypTab[j] == 4 ) printf(" matrix ");
        }
        printf("]\n");
        deg = 1;
        NmbNod = 0;
      }
    }
    GmfCloseMesh(InpSol);
  }

  if ( InpSol == 0 && InpMsh == 0 ) {
    printf("cannot open mesh/solution file %s\n", argv[1]);
    exit(1);
  }

  return(0);

}