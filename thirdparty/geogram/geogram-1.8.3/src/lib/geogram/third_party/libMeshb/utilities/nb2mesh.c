/* 
  conversion from xxx.noboite to xxx.mesh(b) or xxx.(no)boite(b)
  and conversely
 
  Authored by Frederic Alauzet, INRIA, 2008
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "libmesh5.h"

#define IMAX(a,b) ((a) > (b) ? (a) : (b))
#define IMIN(a,b) ((a) < (b) ? (a) : (b))
#define DMAX(a,b) ((a) > (b) ? (a) : (b))
#define DMIN(a,b) ((a) < (b) ? (a) : (b))


// -- structure de donnees
typedef double Vertex[3];
typedef int    Tetrahedron[4];
typedef int    Triangle[3];

int bin;
int b64;
int amdba;


typedef struct mesh_t 
{  
  int  NbrVer;
  int  NbrTet;
  int  NbrTfr;
  int  NbrVfr;

  int  Sub[5];
     
  Vertex      *Crd;
  int         *VerRef;
  Tetrahedron *Tet;
  int         *TetRef;
  Triangle    *Tfr;
  int         *TfrRef;

} Mesh, *pMesh;



// --- declaration fonctions
void usage();
static Mesh*  loadMesh(char *InNam);
static int    openInputMesh(char *InNam, int *FilVer);
static Mesh* loadAmdba(char *MshNam);
static Mesh*  loadNoboite(char *InNam);
static void   freeMesh(Mesh *msh);

static void writeMesh(char *OutNam, Mesh* msh);
static void writeNoboiteFormat(char *OutNam, Mesh* msh);
static void writeNoboite(char *OutNam, Mesh* msh);
static void writeNoboiteb(char *outFil, Mesh* msh);
static void writeNoboite8(char *OutNam, Mesh* msh);
static void writeNoboiteb8(char *outFil, Mesh* msh);





void usage()
{
  printf("\n   -- NB2MESH  \n");
  printf("      Copyright (c) INRIA, 2008\n");
  printf("      Version compiled %s at %s\n\n",__DATE__,__TIME__);
  
  printf("      USAGE : nb2mesh filein fileout [-f] [-f64/-f32]\n");
  printf("              filein :  xxx.[no]boite[b] or xxx.mesh[b]\n");  
  printf("              fileout:  yyy.mesh[b]      or yyy.[no]boite[b]\n");
  printf("              -f     :  write binary file\n");
  printf("              -f32/-f64 :  write 32-bits/64-bits file\n");
  
   
  printf("\n");
  
  fflush(stdout);
  exit(1);   
}






int main(int argc, char **argv) 
{  
  int    nb2msh,msh2nb,msh2msh,i,end;
  char   *inNam,*outNam;
  char   *sub;
  Mesh   *msh;
 
  //-- variables globales
  nb2msh = msh2nb = msh2msh = 0;
  b64 = 0;
  bin = 0;
  end = 0;
  amdba = 0;
   
  if( argc < 3 || argc > 5 )
   usage();
 
  printf("\n   -- NB2MESH  \n");
  printf("      Copyright (c) INRIA, 2008\n");
  printf("      Version compiled %s at %s\n\n",__DATE__,__TIME__);

  if ( argc != 3 ) {
    for (i=3; i<argc; ++i) {
      if ( !strcmp(argv[i],"-f") )
        bin = 1;
      else if ( !strcmp(argv[i],"-f32") )
        b64 = 0;
      else if ( !strcmp(argv[i],"-f64") )
        b64 = 1;
      else {
        printf("  ## Unknown keyword : %s\n",argv[3]);
        usage();
      }
    }
  }

  inNam = strdup(argv[1]);

  //--- check if msh2nb or msh2msh
  sub = strstr(inNam,".mesh");
  if ( sub != NULL ) {
    if ( strcmp(sub,".mesh") && strcmp(sub,".meshb") ) {
      printf("  ## ERROR: Wrong infile name extension : %s !!!\n",inNam);
      end = 1;
    }
    inNam[sub-inNam] = '\0';
    
    
    //--- Is out a mesh file
    outNam = strdup(argv[2]);
    sub = strstr(outNam,".mesh");
    if ( sub != NULL ) {
      if ( !strcmp(sub,".mesh") ) {
        msh2msh = 1;
        bin     = 0;
        outNam[sub-outNam] = '\0';
        goto done; 
      }
      else if ( !strcmp(sub,".meshb") ) {
        msh2msh = 1;
        bin     = 1;
        outNam[sub-outNam] = '\0';
        goto done; 
      }
    }

    //--- else check noboite
    msh2nb = 1;
    
    outNam = strdup(argv[2]);

    sub = strstr(outNam,".noboite");
    if ( sub != NULL ) {
      if ( !strcmp(sub,".noboite") ) {
        bin = 0;
        b64 = 0;
      }
      else if ( !strcmp(sub,".noboiteb") ) {
        bin = 1;
        b64 = 0;
      }
      else if ( !strcmp(sub,".noboite8") ) {
        bin = 0;
        b64 = 1;
      }
      else if ( !strcmp(sub,".noboiteb8") ) {
        bin = 1;
        b64 = 1;
      }
      else {
        printf("  ## ERROR: Wrong outfile name extension : %s !!!\n",outNam);
        end = 1;
      }
      outNam[sub-outNam] = '\0';
    }
    else {
      printf("  ## ERROR: outfile name extension is not a noboite : %s !!!\n",outNam);
      end = 1;
    }
  }
  
  //--- check if nb2msh
  else {

    sub = strstr(inNam,".noboite");

    if ( sub != NULL ) {
      if ( !strcmp(sub,".noboite") || !strcmp(sub,".noboiteb") ) 
        b64 = 0;
      else if ( !strcmp(sub,".noboite8") || !strcmp(sub,".noboiteb8") ) 
        b64 = 1;
      else {
        printf("  ## ERROR: Wrong infile name extension : %s !!!\n",inNam);
        end = 1;
      }
      inNam[sub-inNam] = '\0';
      nb2msh = 1;

      outNam = strdup(argv[2]);
      sub = strstr(outNam,".mesh");
      if ( sub != NULL ) {
        if ( !strcmp(sub,".mesh") ) 
          bin = 0;
        else if ( !strcmp(sub,".meshb") ) 
          bin = 1;
        else {
          printf("  ## ERROR: Wrong outfile name extension : %s !!!\n",outNam);
          end = 1;
        }
        outNam[sub-outNam] = '\0';
      }
    }
    else {
      sub = strstr(inNam,".tri");
      if ( sub != NULL ) {
        amdba = 1;

        outNam = strdup(argv[2]);
        sub = strstr(outNam,".mesh");
        if ( sub != NULL ) {
          if ( !strcmp(sub,".mesh") ) 
            bin = 0;
          else if ( !strcmp(sub,".meshb") ) 
            bin = 1;
          else {
            printf("  ## ERROR: Wrong outfile name extension : %s !!!\n",outNam);
            end = 1;
          }
          outNam[sub-outNam] = '\0';
        }
      }
      else {
        printf("  ## ERROR: Input file is not a mesh file nor a noboite file : %s !!!\n",inNam);
        end = 1;
      }
    }
  }
 
  if ( end == 1 ) usage();
 
  done:

  printf(" nb2msh/msh2nb/msh2msh = %d %d %d\t bin = %d \t f32/64 = %d\n",nb2msh,msh2nb,msh2msh,bin,b64);
 
  if ( amdba == 1 ) {
    msh = loadAmdba(inNam);
    writeMesh(outNam,msh);    
  }
 
 
 
  if ( msh2msh == 1 ) {
    msh = loadMesh(inNam);
    writeMesh(outNam,msh);
  }
  
  else if ( nb2msh == 1 ) {
    printf("  ## NOT YET IMPLEMENTED. SORRY.\n");
    exit(1);
    msh = loadNoboite(inNam);
    writeMesh(outNam,msh);
  }

  else if ( msh2nb == 1 ) {
    msh = loadMesh(inNam);
    writeNoboiteFormat(outNam,msh);
  }

  freeMesh(msh);

  fflush(stdout);		   	
 
  return(0);
}





static Mesh* loadMesh(char *InNam)
{  
  int    InpMsh,FilVer;
  int    i,j,iref;
  float  bufFlt[3];
  double bufDbl[3];
  
  //--- Get the global mesh structure
  Mesh *msh = malloc(sizeof(Mesh));
  msh->NbrVer = 0;
  msh->NbrTet = 0;
  msh->NbrTfr = 0;
  msh->NbrVfr = 0;

  //--- define the input file name and open mesh file
  InpMsh = openInputMesh(InNam,&FilVer);
  
  //--- Get number of entities of the mesh file
  msh->NbrVer = GmfStatKwd(InpMsh, GmfVertices);
  msh->NbrTfr = GmfStatKwd(InpMsh, GmfTriangles);
  msh->NbrTet = GmfStatKwd(InpMsh, GmfTetrahedra);
  

  if ( msh->NbrVer <= 0 ) {
    fprintf(stderr,"\n  ## ERROR: NO VERTICES. IGNORED\n");
    fprintf(stderr,"     Number of vertices = %lld\n",(long long)msh->NbrVer);
    exit(1);
  }
  if ( msh->NbrTfr <= 0 ) {
    fprintf(stderr,"\n  ## ERROR: NO BOUNDARY FACES. IGNORED\n");
    fprintf(stderr,"     Number of bdry faces = %lld\n",(long long)msh->NbrTfr);
    exit(1);
  }
  if ( msh->NbrTet <= 0 ) {
    fprintf(stderr,"\n  ## ERROR: NO TETRAHEDRA. IGNORED\n");
    fprintf(stderr,"     Number of tetrahedra = %lld\n",(long long)msh->NbrTet);
    exit(1);
  }

  msh->Crd = malloc(sizeof(Vertex)*(msh->NbrVer+1));
  if ( msh->Crd == NULL ) {
    fprintf(stderr,"\n  ## ERROR: CANNOT ALLOCATE VERTICES.\n");
    exit(1);
  }
  msh->VerRef = malloc(sizeof(int)*(msh->NbrVer+1));
  if ( msh->VerRef == NULL ) {
    fprintf(stderr,"\n  ## ERROR: CANNOT ALLOCATE VERTICES REF.\n");
    exit(1);
  }
  
  msh->Tet = malloc(sizeof(Tetrahedron)*(msh->NbrTet+1));
  if ( msh->Tet == NULL ) {
    fprintf(stderr,"\n  ## ERROR: CANNOT ALLOCATE TETRAHEDRA.\n");
    exit(1);
  }
  msh->TetRef = malloc(sizeof(int)*(msh->NbrTet+1));
  if ( msh->TetRef == NULL ) {
    fprintf(stderr,"\n  ## ERROR: CANNOT ALLOCATE TETRHEDRA REF.\n");
    exit(1);
  }
 
  msh->Tfr = malloc(sizeof(Triangle)*(msh->NbrTfr+1));
  if ( msh->Tfr == NULL ) {
    fprintf(stderr,"\n  ## ERROR: CANNOT ALLOCATE BOUNDARY TRIANGLES.\n");
    exit(1);
  }
  msh->TfrRef = malloc(sizeof(int)*(msh->NbrTfr+1));
  if ( msh->TfrRef == NULL ) {
    fprintf(stderr,"\n  ## ERROR: CANNOT ALLOCATE BOUNDARY TRIANGLES REF.\n");
    exit(1);
  }
  
  
  printf("    Vertices              %d\n", msh->NbrVer);
  printf("    Boundary Triangles    %d\n", msh->NbrTfr);
  printf("    Tetrahedra            %d\n", msh->NbrTet);
  
  
  
  GmfGotoKwd(InpMsh, GmfVertices);
	if ( FilVer == GmfFloat ) {		// read 32 bits float
    for (i=1; i<msh->NbrVer+1; ++i) {
      GmfGetLin(InpMsh, GmfVertices, &bufFlt[0], &bufFlt[1], &bufFlt[2], &iref);
      msh->Crd[i][0] = (double) bufFlt[0];
      msh->Crd[i][1] = (double) bufFlt[1];
      msh->Crd[i][2] = (double) bufFlt[2];
      msh->VerRef[i] = iref;
      if (i<10) printf(" crd[%d] = %lf %lf %lf\n",i,msh->Crd[i][0],msh->Crd[i][1],msh->Crd[i][2]);
      if (i==2372 || i==8582) printf(" crd[%d] = %lf %lf %lf\n",i,msh->Crd[i][0],msh->Crd[i][1],msh->Crd[i][2]);
    }
  }
  else if ( FilVer == GmfDouble ) {	// read 64 bits float
    for (i=1; i<msh->NbrVer+1; ++i) {
      GmfGetLin(InpMsh, GmfVertices, &bufDbl[0], &bufDbl[1], &bufDbl[2], &iref);
      msh->Crd[i][0] =  bufDbl[0];
      msh->Crd[i][1] =  bufDbl[1];
      msh->Crd[i][2] =  bufDbl[2];
      msh->VerRef[i] = iref;
      if (i<10) printf(" crd[%d] = %lf %lf %lf\n",i,msh->Crd[i][0],msh->Crd[i][1],msh->Crd[i][2]);
      if (i==2372 || i==8582) printf(" crd[%d] = %lf %lf %lf\n",i,msh->Crd[i][0],msh->Crd[i][1],msh->Crd[i][2]);
    }  
  }
  else {
    fprintf(stderr,"\n  ## ERROR: WRONG MESH FORMAT VERSION. IGNORED\n");
    exit(1);  
  }

  GmfGotoKwd(InpMsh, GmfTetrahedra);
  for (i=1; i<msh->NbrTet+1; i++) 
    GmfGetLin(InpMsh, GmfTetrahedra, &msh->Tet[i][0], &msh->Tet[i][1], &msh->Tet[i][2], &msh->Tet[i][3],&msh->TetRef[i]);
  
  //--- Read Tfr and get NbrVfr. Assume that all boundary vertices are first in the list.
  GmfGotoKwd(InpMsh, GmfTriangles);
  for (i=1; i<msh->NbrTfr+1; i++) {
    GmfGetLin(InpMsh, GmfTriangles, &msh->Tfr[i][0], &msh->Tfr[i][1], &msh->Tfr[i][2],&msh->TfrRef[i]);
    for (j=0; j<3; ++j)
      msh->NbrVfr = IMAX(msh->NbrVfr,msh->Tfr[i][j]);
  }


  if ( !GmfCloseMesh(InpMsh) ) {
    fprintf(stderr,"  ## ERROR: Cannot close mesh file %s.mesh[b] ! \n",InNam);
    exit(1);
  }

  //--- Set noboite subdomain coef
  msh->Sub[0] = 1; 
  msh->Sub[1] = msh->Sub[2] = msh->Sub[3] = 0;
  msh->Sub[4] = 1;

  return msh;
}





/*
  
  Open mesh file for reading and verif
  
*/
static int openInputMesh(char *InNam, int *FilVer)
{
  int  InpMsh,dim;
  char InpFil[512];
  char *ptr = NULL;

  strcpy(InpFil,InNam);
  ptr = strstr(InpFil,".mesh");		// strstr :locate a substring in a string 
  if ( !ptr ) ptr = "null";				// if found return a pointer to the 1st char, otherwise return NULL 
  
  if ( strcmp(ptr,".mesh") == 0 || strcmp(ptr,".meshb") == 0 ) {
    if ( !(InpMsh = GmfOpenMesh(InpFil,GmfRead,FilVer,&dim)) ) {
      fprintf(stderr,"  ## ERROR: Mesh data file %s not found ! \n",InpFil);
      exit(1);
    }
  }
  else {
    strcat(InpFil,".meshb");
    if ( !(InpMsh = GmfOpenMesh(InpFil,GmfRead,FilVer,&dim)) ) {
      strcpy(InpFil,InNam);
      strcat(InpFil,".mesh");
      if ( !(InpMsh = GmfOpenMesh(InpFil,GmfRead,FilVer,&dim)) ) {
        fprintf(stderr,"  ## ERROR: Mesh data file %s.mesh[b] not found !! \n",InNam);
        exit(1);
      }    
    }
  }

  printf("  %% %s OPENED, VERSION %d\n",InpFil,*FilVer);

  if ( dim != 3 ) {
	  printf(" ## ERROR: INVALID DIMENSION. READ %d, 3 EXPECTED \n",dim);
	  exit(1);
  }

  return InpMsh;
}



static Mesh* loadAmdba(char *MshNam)
{
  int i;
  
  //--- Get the global mesh structure
  Mesh *msh = malloc(sizeof(Mesh));
  msh->NbrVer = 0;
  msh->NbrTet = 0;
  msh->NbrTfr = 0;

  //--- read amdba file
  FILE* f = fopen(MshNam,"r");
  printf(" read %s\n",MshNam);

  fscanf(f,"%d %d",&msh->NbrVer,&msh->NbrTfr);
  printf(" nbv %d  nbf %d\n",msh->NbrVer,msh->NbrTfr);

  msh->Crd = malloc(sizeof(Vertex)*(msh->NbrVer+1));
  if ( msh->Crd == NULL ) {
    fprintf(stderr,"\n  ## ERROR: CANNOT ALLOCATE VERTICES.\n");
    exit(1);
  }
  msh->VerRef = malloc(sizeof(int)*(msh->NbrVer+1));
  if ( msh->VerRef == NULL ) {
    fprintf(stderr,"\n  ## ERROR: CANNOT ALLOCATE VERTICES REF.\n");
    exit(1);
  }
 
  msh->Tfr = malloc(sizeof(Triangle)*(msh->NbrTfr+1));
  if ( msh->Tfr == NULL ) {
    fprintf(stderr,"\n  ## ERROR: CANNOT ALLOCATE BOUNDARY TRIANGLES.\n");
    exit(1);
  }
  msh->TfrRef = malloc(sizeof(int)*(msh->NbrTfr+1));
  if ( msh->TfrRef == NULL ) {
    fprintf(stderr,"\n  ## ERROR: CANNOT ALLOCATE BOUNDARY TRIANGLES REF.\n");
    exit(1);
  }

  for (i=1; i<=msh->NbrVer; ++i) {
    fscanf(f,"%lf %lf %lf",&msh->Crd[i][0],&msh->Crd[i][1],&msh->Crd[i][2]);  
    msh->VerRef[i] = 0.;
    //printf("ver %d = %lf %lf %lf\n",i,msh->Crd[i][0],msh->Crd[i][1],msh->Crd[i][2]);
    //exit(1);
  }
  
  for (i=1; i<=msh->NbrTfr; ++i) {
    fscanf(f,"%d %d %d",&msh->Tfr[i][0],&msh->Tfr[i][1],&msh->Tfr[i][2]);
    msh->TfrRef[i] = 3;
  }

  fclose(f);

  
  return msh;
}



static Mesh*  loadNoboite(char *MshNam)
{

  //--- Get the global mesh structure
  Mesh *msh = malloc(sizeof(Mesh));
  msh->NbrVer = 0;
  msh->NbrTet = 0;
  msh->NbrTfr = 0;
  
  return msh;
}



static void freeMesh(Mesh *msh)
{  
  if ( msh->Crd != NULL  ) 
    free( (void *) msh->Crd);
  if ( msh->Tfr != NULL  ) 
    free( (void *) msh->Tfr);
  if ( msh->Tet != NULL  ) 
    free( (void *) msh->Tet);
  if ( msh->TfrRef != NULL  ) 
    free( (void *) msh->TfrRef);  
}





static void writeMesh(char *OutNam, Mesh *msh)
{
  int   OutMsh;
  int   i,j;
  float bufFlt[3];
  char  outFil[512];
  

  //--- set output file name
  strcpy(outFil,OutNam);

  if ( bin == 1 ) 
    strcat(outFil,".meshb");
  else 
    strcat(outFil,".mesh");


  //--- open mesh file 
  if ( b64 == 1 ) {
    if ( !(OutMsh = GmfOpenMesh(outFil,GmfWrite,GmfDouble,3)) ) {
      fprintf(stderr,"  ## ERROR: Cannot open mesh file %s ! \n",outFil);
      exit(1);
    }
    printf("  %% %s OPENED, VERSION %d\n",outFil,GmfDouble);
  }
  else {
    if ( !(OutMsh = GmfOpenMesh(outFil,GmfWrite,GmfFloat,3)) ) {
      fprintf(stderr,"  ## ERROR: Cannot open mesh file %s ! \n",outFil);
      exit(1);
    }
    printf("  %% %s OPENED, VERSION %d\n",outFil,GmfFloat);
  }
  
  
  //--- write Vertices
  GmfSetKwd(OutMsh, GmfVertices,msh->NbrVer);
  if ( b64 == 1 ) {
    for (i=1; i<=msh->NbrVer; i++) 
      GmfSetLin(OutMsh, GmfVertices, msh->Crd[i][0], msh->Crd[i][1], msh->Crd[i][2], msh->VerRef[i]);
  }
  else {
    for (i=1; i<=msh->NbrVer; i++) {
      for (j=0; j<3; ++j)
        bufFlt[j] = (float)msh->Crd[i][j];
        
      GmfSetLin(OutMsh, GmfVertices, bufFlt[0], bufFlt[1], bufFlt[2], msh->VerRef[i]);
    }  
  }
  
  //--- write triangles
  GmfSetKwd(OutMsh, GmfTriangles,msh->NbrTfr);
  for (i=1; i<=msh->NbrTfr; i++) 
    GmfSetLin(OutMsh, GmfTriangles, msh->Tfr[i][0], msh->Tfr[i][1], msh->Tfr[i][2], msh->TfrRef[i]);
  
  //--- write tetrahedra
  GmfSetKwd(OutMsh, GmfTetrahedra,msh->NbrTet);
  for (i=1; i<=msh->NbrTet; i++) 
    GmfSetLin(OutMsh, GmfTetrahedra, msh->Tet[i][0], msh->Tet[i][1], msh->Tet[i][2], msh->Tet[i][3], msh->TetRef[i]);



  if ( !GmfCloseMesh(OutMsh) ) {
    fprintf(stderr,"  ## ERROR: Cannot close mesh file %s.mesh[b] ! \n",outFil);
    exit(1);
  }

}








static void writeNoboiteFormat(char *OutNam, Mesh* msh)
{
  char outFil[512];
  
  strcpy(outFil,OutNam);

  if ( bin == 1 ) {
    if ( b64 == 1 ) {
      strcat(outFil,".noboiteb8");
      writeNoboiteb8(outFil,msh);
    }
    else {
      strcat(outFil,".noboiteb");
      writeNoboiteb(outFil,msh);
    }
  }
  else {
    if ( b64 == 1 ) {
      strcat(outFil,".noboite8");
      writeNoboite8(outFil,msh);
    }
    else {
      strcat(outFil,".noboite");
      writeNoboite(outFil,msh);
    }
  }  
  
}




static void writeNoboite(char *outFil, Mesh* msh)
{
  FILE *out;
  int  iVer,iTet,k;

  int nbele,loele,nbelef,loelef;
  int nbpoi,lopoi,nbpoif,lopoif;
  int nbhlo,lohlo,nbhlof,lohlof;
  int npfixe,icube,npbli;

  out = fopen(outFil,"w");
  if ( !out ) {
    fprintf(stderr,"  ## ERROR: Unalble to open file %s ! \n",outFil);
    exit(1);
  }
  fprintf(stdout,"  %% Writing file %s\n",outFil);

  //--- define noboite parameters
  nbele = 1;  loele = 4*msh->NbrTet;  nbelef = loelef = 0;
  nbpoi = 1;  lopoi = 3*msh->NbrVer;  nbpoif = lopoif = 0;
  nbhlo = 1;  lohlo =   msh->NbrVer;  nbhlof = lohlof = 0;

  npfixe = msh->NbrVfr;
  icube  = 0;
  npbli  = 0;

  fprintf(out,"%d %d ",msh->NbrTet,msh->NbrVer);
  fprintf(out,"%d %d %d ",npfixe,icube,npbli);
  fprintf(out,"%d %d %d %d ",nbele,loele,nbelef,loelef);
  fprintf(out,"%d %d %d %d ",nbpoi,lopoi,nbpoif,lopoif);
  fprintf(out,"%d %d %d %d\n",nbhlo,lohlo,nbhlof,lohlof);

  
  fprintf(stdout,"  -- Writing simplices\n");
  for (iTet=1; iTet<=msh->NbrTet; ++iTet) {
    fprintf(out,"%d %d %d %d ",msh->Tet[iTet][0],msh->Tet[iTet][1],msh->Tet[iTet][2],msh->Tet[iTet][3]);
    if ( iTet % 3 == 0 ) 
      fprintf(out,"\n");
  }
  if ( (iTet-1) % 3 != 0 )  fprintf(out,"\n");

  
  fprintf(stdout,"  -- Writing coords\n");
  for (iVer=1; iVer<=msh->NbrVer; iVer++) {
    fprintf(out,"%g %g %g ",(float)msh->Crd[iVer][0],(float)msh->Crd[iVer][1],(float)msh->Crd[iVer][2]);
    if ( iVer % 3 == 0 ) 
      fprintf(out,"\n");
  }
  if ( (iVer-1) % 3 != 0 )  fprintf(out,"\n");

  //--- write coefficients 
  for (k=0; k<=4; k++) 
    fprintf(out,"%d ",msh->Sub[k]);
  fprintf(out,"\n");

  fclose(out);
}




static void writeNoboiteb(char *outFil, Mesh* msh)
{
  FILE *out;
  int  sizBlk,deb,fin,j,k,l,idx;

  int nbele,loele,nbelef,loelef;
  int nbpoi,lopoi,nbpoif,lopoif;
  int nbhlo,lohlo,nbhlof,lohlof;
  int npfixe,icube,npbli,no;
  float crd[3];

  out = fopen(outFil,"w");
  if ( !out ) {
    fprintf(stderr,"  ## ERROR: Unalble to open file %s ! \n",outFil);
    exit(1);
  }
  fprintf(stdout,"  %% Writing file %s\n",outFil);

  //--- Binary size block
  sizBlk = 12 * 16384;

  //--- define noboite parameters : split or not the record
  if ( msh->NbrTet < sizBlk ) {   // only one block
    nbele = 1;  loele = 4*msh->NbrTet;  nbelef = loelef = 0;  // elements
    nbpoi = 1;  lopoi = 3*msh->NbrVer;  nbpoif = lopoif = 0;  // vertices
    nbhlo = 1;  lohlo =   msh->NbrVer;  nbhlof = lohlof = 0;  // sub-domain
  }

  else {   // split the record in a series of block of size sizBlk
    //printf("  ## Splitting the records %d\n",sizBlk);
    nbele  = 4*msh->NbrTet / sizBlk;       // nbr of blk
    loele  = sizBlk;                       // siz of  blk
    nbelef = 1;                            // more than one blk
    loelef = 4*msh->NbrTet - sizBlk*nbele; // size of the residual blk

    nbpoi  = 3*msh->NbrVer / sizBlk;
    lopoi  = sizBlk;
    nbpoif = 1;
    lopoif = 3*msh->NbrVer - sizBlk*nbpoi;
    
    if ( msh->NbrVer < sizBlk ) {
      nbhlo = 1;  lohlo = msh->NbrVer;  nbhlof = lohlof = 0;  
    }
    else {
      nbhlo  = msh->NbrVer / sizBlk;
      lohlo  = sizBlk;
      nbhlof = 1;
      lohlof = msh->NbrVer - sizBlk*nbhlo;
    }
  }

  npfixe = msh->NbrVfr;
  icube  = 0;
  npbli  = 0;
  no     = 68;  // =17*4 as we write 17 int

  fwrite(&no,sizeof(int),1,out);
  fwrite(&msh->NbrTet,sizeof(int),1,out);
  fwrite(&msh->NbrVer,sizeof(int),1,out);

  fwrite(&npfixe,sizeof(int),1,out);
  fwrite(&icube,sizeof(int),1,out);
  fwrite(&npbli,sizeof(int),1,out);

  fwrite(&nbele,sizeof(int),1,out);  fwrite(&loele,sizeof(int),1,out);
  fwrite(&nbelef,sizeof(int),1,out); fwrite(&loelef,sizeof(int),1,out);

  fwrite(&nbpoi,sizeof(int),1,out);  fwrite(&lopoi,sizeof(int),1,out);
  fwrite(&nbpoif,sizeof(int),1,out); fwrite(&lopoif,sizeof(int),1,out);

  fwrite(&nbhlo,sizeof(int),1,out);  fwrite(&lohlo,sizeof(int),1,out);
  fwrite(&nbhlof,sizeof(int),1,out); fwrite(&lohlof,sizeof(int),1,out);

  fwrite(&no,sizeof(int),1,out);


  fprintf(stdout,"  -- Writing simplices\n");
  deb = 1;
  fin = loele;
  idx = 1;

  //--- write each block
  for (j=1; j<=nbele; j++) {
    no = (fin-deb+1)*sizeof(int);
    fwrite(&no,sizeof(int),1,out);
    for (k=deb; k<=fin; k+=4) {
      for (l=0; l<4; ++l)
        fwrite(&msh->Tet[idx][l],sizeof(int),1,out);
      //fwrite(&msh->Tet[idx],sizeof(int),4,out);
      idx++;
    }
    deb += loele;
    fin += loele;
    fwrite(&no,sizeof(int),1,out);
  }
  
  //--- write the last block (residual block)
  if ( deb < 4*msh->NbrTet ) {
    no = (4*msh->NbrTet-deb+1)*sizeof(int);
    fwrite(&no,sizeof(int),1,out);
    for (k=deb; k<=4*msh->NbrTet; k+=4) {
      for (l=0; l<4; ++l)
        fwrite(&msh->Tet[idx][l],sizeof(int),1,out);
      //fwrite(&msh->Tet[idx],sizeof(int),4,out);
      idx++;
    }
    fwrite(&no,sizeof(int),1,out);
  }

  
  fprintf(stdout,"  -- Writing coords\n");
  deb = 1;
  fin = lopoi;
  idx = 1;

  //--- write each block
  for (j=1; j<=nbpoi; j++) {
    no = (fin-deb+1)*sizeof(float);
    if (j==1) printf(" no = %d\n",no);
    fwrite(&no,sizeof(int),1,out);
    for (k=deb; k<=fin; k+=3) {
      if (j==1&&k<=30) printf(" crd[%d] = %lf %lf %lf\n",idx,msh->Crd[idx][0],msh->Crd[idx][1],msh->Crd[idx][2]);
      if (idx==2372 || idx==8582) printf(" crd[%d] = %lf %lf %lf\n",idx,msh->Crd[idx][0],msh->Crd[idx][1],msh->Crd[idx][2]);
      for (l=0; l<3; ++l)
        crd[l] = (float)msh->Crd[idx][l];
      fwrite(&crd[0],3*sizeof(float),1,out);
      idx++;
    }
    deb += lopoi;
    fin += lopoi;
    fwrite(&no,sizeof(int),1,out);
  }

  //--- write the last block (residual block)
  if ( deb < 3*msh->NbrVer ) {
    no = (3*msh->NbrVer-deb+1)*sizeof(float);
    fwrite(&no,sizeof(int),1,out);
    for (k=deb; k<=3*msh->NbrVer; k+=3) {
      for (l=0; l<3; ++l)
        crd[l] = (float)msh->Crd[idx][l];
      fwrite(&crd[0],3*sizeof(float),1,out);
      idx++;
    }
    fwrite(&no,sizeof(int),1,out);
  }


  //--- write coefficients 
  no = 5*sizeof(int);
  fwrite(&no,sizeof(int),1,out);
  fwrite(&msh->Sub,sizeof(int),5,out);
  
  fwrite(&no,sizeof(int),1,out);

  fclose(out);
}




static void writeNoboite8(char *outFil, Mesh* msh)
{
  FILE *out;
  long long int  iVer,iTet,k;

  long long int nbele,loele,nbelef,loelef;
  long long int nbpoi,lopoi,nbpoif,lopoif;
  long long int nbhlo,lohlo,nbhlof,lohlof;
  long long int npfixe,icube,npbli;

  out = fopen(outFil,"w");
  if ( !out ) {
    fprintf(stderr,"  ## ERROR: Unalble to open file %s ! \n",outFil);
    exit(1);
  }
  fprintf(stdout,"  %% Writing file %s\n",outFil);

  //--- define noboite parameters
  nbele = 1;  loele = 4*msh->NbrTet;  nbelef = loelef = 0;
  nbpoi = 1;  lopoi = 3*msh->NbrVer;  nbpoif = lopoif = 0;
  nbhlo = 1;  lohlo =   msh->NbrVer;  nbhlof = lohlof = 0;

  npfixe = msh->NbrVfr;
  icube  = 0;
  npbli  = 0;

  fprintf(out,"%d %d ",msh->NbrTet,msh->NbrVer);
  fprintf(out,"%lld %lld %lld ",npfixe,icube,npbli);
  fprintf(out,"%lld %lld %lld %lld ",nbele,loele,nbelef,loelef);
  fprintf(out,"%lld %lld %lld %lld ",nbpoi,lopoi,nbpoif,lopoif);
  fprintf(out,"%lld %lld %lld %lld\n",nbhlo,lohlo,nbhlof,lohlof);

  
  fprintf(stdout,"  -- Writing simplices\n");
  for (iTet=1; iTet<=msh->NbrTet; iTet++) {
    fprintf(out,"%lld %lld %lld %lld ",
    (long long int)msh->Tet[iTet][0],(long long int)msh->Tet[iTet][1],
    (long long int)msh->Tet[iTet][2],(long long int)msh->Tet[iTet][3]);
    if ( iTet % 3 == 0 ) 
      fprintf(out,"\n");
  }
  if ( (iTet-1) % 3 != 0 )  fprintf(out,"\n");

  
  fprintf(stdout,"  -- Writing coords\n");
  for (iVer=1; iVer<=msh->NbrVer; iVer++) {
    fprintf(out,"%.15lg %.15lg %.15lg ",msh->Crd[iVer][0],msh->Crd[iVer][1],msh->Crd[iVer][2]);
    if ( iVer % 3 == 0 ) 
      fprintf(out,"\n");
  }
  if ( (iVer-1) % 3 != 0 )  fprintf(out,"\n");

  //--- write coefficients 
  for (k=0; k<=4; k++) 
    fprintf(out,"%lld ",(long long int)msh->Sub[k]);
  fprintf(out,"\n");

  fclose(out);
}





static void writeNoboiteb8(char *outFil, Mesh* msh)
{
  FILE *out;
  long long int  sizBlk,deb,fin,j,k,idx;

  long long int nbele,loele,nbelef,loelef;
  long long int nbpoi,lopoi,nbpoif,lopoif;
  long long int nbhlo,lohlo,nbhlof,lohlof;
  long long int npfixe,icube,npbli,no;

  out = fopen(outFil,"w");
  if ( !out ) {
    fprintf(stderr,"  ## ERROR: Unalble to open file %s ! \n",outFil);
    exit(1);
  }
  fprintf(stdout,"  %% Writing file %s\n",outFil);

  //--- Binary size block
  sizBlk = 12 * 16384;

  //--- define noboite parameters : split or not the record
  if ( msh->NbrTet < sizBlk ) {   // only one block
    nbele = 1;  loele = 4*msh->NbrTet;  nbelef = loelef = 0;  // elements
    nbpoi = 1;  lopoi = 3*msh->NbrVer;  nbpoif = lopoif = 0;  // vertices
    nbhlo = 1;  lohlo =   msh->NbrVer;  nbhlof = lohlof = 0;  // sub-domain
  }

  else {   // split the record in a series of block of size sizBlk
    //printf("  ## Splitting the records %d\n",sizBlk);
    nbele  = 4*msh->NbrTet / sizBlk;       // nbr of blk
    loele  = sizBlk;                       // siz of  blk
    nbelef = 1;                            // more than one blk
    loelef = 4*msh->NbrTet - sizBlk*nbele; // size of the residual blk

    nbpoi  = 3*msh->NbrVer / sizBlk;
    lopoi  = sizBlk;
    nbpoif = 1;
    lopoif = 3*msh->NbrVer - sizBlk*nbpoi;
    
    if ( msh->NbrVer < sizBlk ) {
      nbhlo = 1;  lohlo = msh->NbrVer;  nbhlof = lohlof = 0;  
    }
    else {
      nbhlo  = msh->NbrVer / sizBlk;
      lohlo  = sizBlk;
      nbhlof = 1;
      lohlof = msh->NbrVer - sizBlk*nbhlo;
    }
  }

  npfixe = msh->NbrVfr;
  icube  = 0;
  npbli  = 0;
  no     = 136;  // =17*8 as we write 17 long long int

// NO EN INT OU LONG LONG INT ??????

  fwrite(&no,sizeof(long long int),1,out);
  fwrite(&msh->NbrTet,sizeof(long long int),1,out);
  fwrite(&msh->NbrVer,sizeof(long long int),1,out);

  fwrite(&npfixe,sizeof(long long int),1,out);
  fwrite(&icube,sizeof(long long int),1,out);
  fwrite(&npbli,sizeof(long long int),1,out);

  fwrite(&nbele,sizeof(long long int),1,out);  fwrite(&loele,sizeof(long long int),1,out);
  fwrite(&nbelef,sizeof(long long int),1,out); fwrite(&loelef,sizeof(long long int),1,out);

  fwrite(&nbpoi,sizeof(long long int),1,out);  fwrite(&lopoi,sizeof(long long int),1,out);
  fwrite(&nbpoif,sizeof(long long int),1,out); fwrite(&lopoif,sizeof(long long int),1,out);

  fwrite(&nbhlo,sizeof(long long int),1,out);  fwrite(&lohlo,sizeof(long long int),1,out);
  fwrite(&nbhlof,sizeof(long long int),1,out); fwrite(&lohlof,sizeof(long long int),1,out);

  fwrite(&no,sizeof(long long int),1,out);


  fprintf(stdout,"  -- Writing simplices\n");
  deb = 1;
  fin = loele;
  idx = 1;

  //--- write each block
  for (j=1; j<=nbele; j++) {
    no = (fin-deb+1)*sizeof(long long int);
    fwrite(&no,sizeof(long long int),1,out);
    for (k=deb; k<=fin; k+=4) {
      fwrite(&msh->Tet[idx],sizeof(long long int),4,out);
      idx++;
    }
    deb += loele;
    fin += loele;
    fwrite(&no,sizeof(long long int),1,out);
  }
  
  //--- write the last block (residual block)
  if ( deb < 4*msh->NbrTet ) {
    no = (4*msh->NbrTet-deb+1)*sizeof(long long int);
    fwrite(&no,sizeof(long long int),1,out);
    for (k=deb; k<=4*msh->NbrTet; k+=4) {
      fwrite(&msh->Tet[idx],sizeof(long long int),4,out);
      idx++;
    }
    fwrite(&no,sizeof(long long int),1,out);
  }

  
  fprintf(stdout,"  -- Writing coords\n");
  deb = 1;
  fin = lopoi;
  idx = 1;

  //--- write each block
  for (j=1; j<=nbpoi; j++) {
    no = (fin-deb+1)*sizeof(double);
    fwrite(&no,sizeof(long long int),1,out);
    for (k=deb; k<=fin; k+=3) {
      fwrite(&msh->Crd[idx],3*sizeof(double),1,out);
      idx++;
    }
    deb += lopoi;
    fin += lopoi;
    fwrite(&no,sizeof(long long int),1,out);
  }

  //--- write the last block (residual block)
  if ( deb < 3*msh->NbrVer ) {
    no = (3*msh->NbrVer-deb+1)*sizeof(double);
    fwrite(&no,sizeof(long long int),1,out);
    for (k=deb; k<=3*msh->NbrVer; k+=3) {
      fwrite(&msh->Crd[idx],3*sizeof(double),1,out);
      idx++;
    }
    fwrite(&no,sizeof(long long int),1,out);
  }


  //--- write coefficients 
  no = 5*sizeof(long long int);
  fwrite(&no,sizeof(long long int),1,out);
  fwrite(&msh->Sub,sizeof(long long int),5,out);
  
  fwrite(&no,sizeof(long long int),1,out);

  fclose(out);
}




