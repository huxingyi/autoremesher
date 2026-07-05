/*

Adrien Loseille, INRIA, 2016

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libmeshb7.h>


#define max(a,b) ((a)>(b))?(a):(b)

typedef           int  mint;
typedef long long int lint;

typedef double double3[3];
typedef  mint     int3[3];
typedef  mint     int4[4];
typedef  mint     int5[5];
typedef  mint     int6[6];
typedef  mint     int8[8];

int littleEndian()
{
  int n = 1;
  // little endian if true
  if(*(char *)&n == 1) return 1;
  else                 return 0;
}


double swapd(double d)
{
   double a;
   unsigned char *dst = (unsigned char *)&a;
   unsigned char *src = (unsigned char *)&d;

   if ( ! littleEndian() ) return d;

   dst[0] = src[7];
   dst[1] = src[6];
   dst[2] = src[5];
   dst[3] = src[4];
   dst[4] = src[3];
   dst[5] = src[2];
   dst[6] = src[1];
   dst[7] = src[0];

   return a;
}

int swapi4(int d)
{
   mint a;
   unsigned char *dst = (unsigned char *)&a;
   unsigned char *src = (unsigned char *)&d;

   if ( ! littleEndian() ) return d;

   dst[0] = src[3];
   dst[1] = src[2];
   dst[2] = src[1];
   dst[3] = src[0];
  
   return a;
}

lint swapi8(lint d)
{
   lint a;
   unsigned char *dst = (unsigned char *)&a;
   unsigned char *src = (unsigned char *)&d;
   
   if ( ! littleEndian() ) return d;

   dst[0] = src[7];
   dst[1] = src[6];
   dst[2] = src[5];
   dst[3] = src[4];
   dst[4] = src[3];
   dst[5] = src[2];
   dst[6] = src[1];
   dst[7] = src[0];
  
   return a;
}







mint main(int argc, char *argv[])
{
  
  mint nbrVer=0, nbrTri=0, nbrQua=0, nbrTet = 0, nbrPyr = 0, nbrPri = 0, nbrHex = 0;
  int3 *tri        = NULL;
  int4 *qua        = NULL;
  int4 *tet        = NULL;
  int5 *pyr        = NULL;
  int6 *pri        = NULL;
  int8 *hex        = NULL;
  
  mint  *lsurf     = NULL;
  double3 *crd     = NULL;
  
  int    writeSurf = 0;
  
  mint iVer,iTri,iQua,iTet,iPyr,iPri,iHex,i,ix;
  
  if ( argc < 3 ) {
    printf("  Usage : ugrid2mesh file.ugrid file.mesh[b] [ -surf -info ]\n");
    return 0;
  }
  
  /* write surface only */
  if ( argc >= 4 ) {
    if( strcmp(argv[3],"-surf") == 0 ) writeSurf = 1;
  }
  
  
  printf("  *************************************** \n");
  printf("\n");
  printf("  UGRID2MESH Conversion, version 1.0 \n");
  printf("\n");
  printf("  *************************************** \n");
  
  FILE *f = fopen(argv[1],"r");
  if ( f == NULL ) {
    printf("  ## ERROR: CANNOT OPEN FILE %s \n", argv[1]);
    return 0;
  }
  
  //--- should be used to see if it is necessary to swaps the bytes....
  printf("  ** Local system is in  %s \n",(littleEndian() == 1)?"Little Endian encoding":"Big Endian encoding \n");
  
  //printf("  f= %p \n",f);
  unsigned int size[7];
  fread(size,sizeof(int),7,f); 
  
  //printf(" size %d %d %d %d %d %d %d \n",swapi4(size[0]),swapi4(size[1]),swapi4(size[2]),swapi4(size[3]),swapi4(size[4]),swapi4(size[5]),swapi4(size[6]));
  
  printf("  %% %s OPENED \n",argv[1]);
  printf("  Number of Nodes           %15d\n",swapi4(size[0]));
  printf("  Number of Triangles       %15d\n",swapi4(size[1]));
  printf("  Number of Quads           %15d\n",swapi4(size[2]));
  printf("  Number of Tetrahedra      %15d\n",swapi4(size[3]));
  printf("  Number of Pyramids        %15d\n",swapi4(size[4]));
  printf("  Number of Prisms          %15d\n",swapi4(size[5]));
  printf("  Number of Hexes           %15d\n",swapi4(size[6]));
  
  
  //printf(" size of the file %lg Gb\n",(swapi4(size[0])*3.0*8.0 + swapi4(size[1])*4.*4. + swapi4(size[3])*4.*4.)/(1024*1024*1024));
  
  /* exist here if just a query on the mesh */
  printf(" argc %d \n",argc);
  
  if ( argc > 2 ) {
    if ( strcmp(argv[2],"-info") == 0 ) return 1;
    if ( argc > 3  ) {
      if ( strcmp(argv[3],"-info") == 0 ) return 1; 
    }
  }
  
  printf(" -- allocating vertices \n");
  nbrVer   = swapi4(size[0]);
  crd = (double3 *)malloc(sizeof(double)*3*nbrVer);
  
  fread(crd,sizeof(double),3*nbrVer,f);
  
  printf(" -- reading vertices %d\n",nbrVer);
  for(iVer=0; iVer<nbrVer; iVer++) {
    for(i=0; i<3; i++) {
      double sx  = swapd(crd[iVer][i]);
      crd[iVer][i] = sx;
    }
    //if ( iVer <= 10 )        printf("iVer[%15d] =  %lg %lg %lg \n", iVer+1, crd[iVer][0],crd[iVer][1],crd[iVer][2]);
    //if ( iVer >= nbrVer-10 ) printf("iVer[%15d] =  %lg %lg %lg \n", iVer+1,crd[iVer][0],crd[iVer][1],crd[iVer][2]);
    //printf("iVer[%d] =  %lg %lg %lg \n", iVer+1, crd[iVer][0],crd[iVer][1],crd[iVer][2]);
  }
    
  //--- write only surface mesh ?
  printf(" -- allocating surface link \n");
  mint  mark  = 1;
  int  *pmark = malloc(sizeof(mint)*(nbrVer+1));
  
  mint npsrf  = 0;
  mint *lpsrf = malloc(sizeof(mint)*(nbrVer+1));

  memset(pmark,0,sizeof(mint)*(nbrVer+1));
    
  mint maxid = 0;
  
  printf(" -- reading triangles \n");
  nbrTri  = swapi4(size[1]);
  if ( nbrTri > 0 ) {
    tri   = (int3 *)malloc(sizeof(mint)*3*nbrTri);
    fread(tri,sizeof(mint),3*nbrTri,f);
    for(iTri=0; iTri<nbrTri; iTri++) {
      //printf(" iTri %d \n",iTri);
      for(i=0; i<3; i++) {
        ix  = swapi4(tri[iTri][i]);
        tri[iTri][i] = ix;
        maxid = max(maxid,ix);
        if ( pmark[ix] == 0 ) {
          pmark[ix] = mark;
          lpsrf[npsrf] = ix;
          npsrf = npsrf + 1;
          pmark[ix]    = npsrf;
        }
        
      }
      //prmintf(" %lg %lg %lg \n",swapd(crd[0]),swapd(crd[1]),swapd(crd[2]));
    }
  }
  
  printf(" -- reading quads \n");
  nbrQua  = swapi4(size[2]);
  if ( nbrQua > 0 ) {
    qua   = (int4 *)malloc(sizeof(mint)*4*nbrQua);
    fread(qua,sizeof(mint),4*nbrQua,f);
    for(iQua=0; iQua<nbrQua; iQua++) {
      for(i=0; i<4; i++) {
        ix  = swapi4(qua[iQua][i]);
        qua[iQua][i] = ix;
        maxid = max(maxid,ix);
        if ( pmark[ix] == 0 ) {
          pmark[ix] = mark;
          lpsrf[npsrf] = ix;
          npsrf = npsrf + 1;
          pmark[ix]    = npsrf;
        }
      }
      //prmintf(" %lg %lg %lg \n",swapd(crd[0]),swapd(crd[1]),swapd(crd[2]));
    }
  }
  
  printf(" -- setting up surface id \n");
  lsurf = malloc(sizeof(mint)*(nbrTri+nbrQua));
  fread(lsurf,sizeof(mint),nbrTri+nbrQua,f);
  for(iTri=0; iTri<(nbrTri+nbrQua); iTri++) {
     mint ix      = swapi4(lsurf[iTri]);
     lsurf[iTri] = ix;
  }
  
  printf(" ... end reading ugrid file \n");
  
  printf("  Number of Boundary  Nodes %15d \n",npsrf);
  
  int FilVer = 3;
  int64_t OutMsh = GmfOpenMesh(argv[2], GmfWrite, FilVer, 3);
  if ( OutMsh <= 0 ) {
    printf("  ## ERROR: CANNOT CREATE FILE %s \n", argv[2]);
    return 0;
  }
  
  printf(" %% %s CREATED \n",argv[2]);
  
  if ( writeSurf == 1 ) {
    printf("  outputting surface only \n");
    GmfSetKwd(OutMsh, GmfVertices,npsrf);
    for (iVer=0; iVer<npsrf; iVer++) {
      GmfSetLin(OutMsh, GmfVertices, crd[lpsrf[iVer]-1][0],crd[lpsrf[iVer]-1][1],crd[lpsrf[iVer]-1][2],0); 
    }
    GmfSetKwd(OutMsh, GmfTriangles,nbrTri);
    for (iTri=0; iTri<nbrTri; iTri++) {
      GmfSetLin(OutMsh, GmfTriangles, pmark[tri[iTri][0]], pmark[tri[iTri][1]], pmark[tri[iTri][2]], lsurf[iTri]); 
    }
    GmfSetKwd(OutMsh, GmfQuadrilaterals,nbrQua);
    for (iQua=0; iQua<nbrQua; iQua++) {
      GmfSetLin(OutMsh, GmfQuadrilaterals, pmark[qua[iQua][0]], pmark[qua[iQua][1]], pmark[qua[iQua][2]],  pmark[qua[iQua][2]], lsurf[nbrTri+iQua]); 
    }
  }
  else {
    printf("  outputting volume only \n");
    GmfSetKwd(OutMsh, GmfVertices, nbrVer);
    for (iVer=0; iVer<nbrVer; iVer++) {
      GmfSetLin(OutMsh, GmfVertices, crd[iVer][0],crd[iVer][1],crd[iVer][2],0); 
    }
    GmfSetKwd(OutMsh, GmfTriangles,nbrTri);
    for (iTri=0; iTri<nbrTri; iTri++) {
      GmfSetLin(OutMsh, GmfTriangles, tri[iTri][0], tri[iTri][1], tri[iTri][2], lsurf[iTri]); 
    }
    GmfSetKwd(OutMsh, GmfQuadrilaterals,nbrQua);
    for (iQua=0; iQua<nbrQua; iQua++) {
      GmfSetLin(OutMsh, GmfQuadrilaterals, qua[iQua][0], qua[iQua][1], qua[iQua][2],  qua[iQua][2], lsurf[nbrTri+iQua]); 
    }
    
    nbrTet  = swapi4(size[3]);
    if ( nbrTet > 0 ) {
      tet   = (int4 *)malloc(sizeof(mint)*4*nbrTet);
      fread(tet,sizeof(mint),4*nbrTet,f);
      for(iTet=0; iTet<nbrTet; iTet++) {
        for(i=0; i<4; i++) {
          ix  = swapi4(tet[iTet][i]);
          tet[iTet][i] = ix;
        }
      }
    }
    GmfSetKwd(OutMsh, GmfTetrahedra,nbrTet);
    for (iTet=0; iTet<nbrQua; iTet++) {
      GmfSetLin(OutMsh, GmfTetrahedra, tet[iTet][0], tet[iTet][1], tet[iTet][2],  tet[iTet][3], 0); 
    }
    if ( tet ) {free(tet); tet = NULL; }
    
    
    nbrPyr  = swapi4(size[4]);
    if ( nbrPyr > 0 ) {
      pyr   = (int5 *)malloc(sizeof(mint)*5*nbrPyr);
      fread(pyr,sizeof(mint),5*nbrPyr,f);
      for(iPyr=0; iPyr<nbrPyr; iPyr++) {
        for(i=0; i<5; i++) {
          ix  = swapi4(pyr[iPyr][i]);
          pyr[iPyr][i] = ix;
        }
      }
    }
    GmfSetKwd(OutMsh, GmfPyramids,nbrPyr);
    for (iPyr=0; iPyr<nbrQua; iPyr++) {
      GmfSetLin(OutMsh, GmfPyramids, pyr[iPyr][0], pyr[iPyr][1], pyr[iPyr][2],  pyr[iPyr][3], pyr[iPyr][4] , 0); 
    }
    if ( pyr ) {free(pyr); pyr = NULL; }
    
    
    nbrPri  = swapi4(size[5]);
    if ( nbrPri > 0 ) {
      pri   = (int6 *)malloc(sizeof(mint)*6*nbrPri);
      fread(pri,sizeof(mint),6*nbrPri,f);
      for(iPri=0; iPri<nbrPri; iPri++) {
        for(i=0; i<6; i++) {
          ix  = swapi4(pri[iPri][i]);
          pri[iPri][i] = ix;
        }
      }
    }
    GmfSetKwd(OutMsh, GmfPrisms,nbrPri);
    for (iPri=0; iPri<nbrPri; iPri++) {
      GmfSetLin(OutMsh, GmfPrisms, pri[iPri][0], pri[iPri][1], pri[iPri][2],  pri[iPri][3], pri[iPri][4] , pri[iPri][5], 0); 
    }
    if ( pri ) {free(pri); pri = NULL; }
    
    
    nbrHex  = swapi4(size[6]);
    if ( nbrHex > 0 ) {
      hex   = (int8 *)malloc(sizeof(mint)*8*nbrHex);
      fread(hex,sizeof(mint),8*nbrHex,f);
      for(iHex=0; iHex<nbrHex; iHex++) {
        for(i=0; i<8; i++) {
          ix  = swapi4(hex[iHex][i]);
          hex[iHex][i] = ix;
        }
      }
    }
    GmfSetKwd(OutMsh, GmfHexahedra,nbrHex);
    for (iHex=0; iHex<nbrQua; iHex++) {
      GmfSetLin(OutMsh, GmfHexahedra, hex[iHex][0], hex[iHex][1], hex[iHex][2],  hex[iHex][3], hex[iHex][4], hex[iHex][5], hex[iHex][6],  hex[iHex][7], 0); 
    }
    if ( hex ) {free(hex); hex = NULL; }
    
  }
  
  GmfCloseMesh(OutMsh);
  
  
  if ( crd ) { free(crd); crd = NULL; }
  if ( tri ) { free(tri); tri = NULL; }
  if ( qua ) { free(qua); qua = NULL; }
  if ( tet ) { free(tet); tet = NULL; }
  if ( pyr ) { free(pyr); pyr = NULL; }
  if ( pri ) { free(pri); pri = NULL; }
  if ( hex ) { free(hex); hex = NULL; }
  
  fclose(f);
  
  
  return 0;
}
