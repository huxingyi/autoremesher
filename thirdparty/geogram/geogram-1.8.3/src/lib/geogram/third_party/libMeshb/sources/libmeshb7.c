

/*----------------------------------------------------------------------------*/
/*                                                                            */
/*                               LIBMESHB V7.62                               */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*   Description:        handles .meshb file format I/O                       */
/*   Author:             Loic MARECHAL                                        */
/*   Creation date:      dec 09 1999                                          */
/*   Last modification:  jan 07 2022                                          */
/*                                                                            */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Headers' macros                                                            */
/*----------------------------------------------------------------------------*/

// Silent Visual Studio warnings on string functions
#define _CRT_SECURE_NO_WARNINGS

#ifdef F77API

// Add a final underscore to Fortran procedure names
#ifdef F77_NO_UNDER_SCORE
#define NAMF77(c,f)  f
#define APIF77(x)    x
#else
#define NAMF77(c,f)  f ## _
#define APIF77(x)    x ## _
#endif

// Pass parameters as pointers in Fortran
#define VALF77(v)    *v
#define TYPF77(t)    t*

#else

// Pass parameters as values in C
#define NAMF77(c,f)  c
#define VALF77(v)    v
#define TYPF77(t)    t

#endif


/*----------------------------------------------------------------------------*/
/* Includes                                                                   */
/*----------------------------------------------------------------------------*/

#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <ctype.h>
#include <setjmp.h>
#include <fcntl.h>

 
/*
 * [Bruno] include the headers with the prototypes for 
 *  open()/close()/write()/lseek() 
 *  and define the constants to be used to open() a file. 
 *   Under Windows, 
 *  1)   _O_BINARY should be set in the flags.
 *  2) 'mode' has a completely different meaning
 */

#if defined(__unix__) || defined(__linux__) || defined(__APPLE__) || defined(__EMSCRIPTEN__)

#include <unistd.h>

#define OPEN_READ_FLAGS   O_RDONLY
#define OPEN_WRITE_FLAGS  O_CREAT | O_WRONLY | O_TRUNC
#define OPEN_READ_MODE    0666
#define OPEN_WRITE_MODE   0666   
   
#elif defined(_WIN32) || defined(_WIN64)

#define GMF_WINDOWS

#include <windows.h>
#include <io.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <wchar.h>

#define OPEN_READ_FLAGS    O_RDONLY | _O_BINARY
#define OPEN_WRITE_FLAGS   O_CREAT | O_WRONLY | O_TRUNC | _O_BINARY
#define OPEN_READ_MODE     _S_IREAD
#define OPEN_WRITE_MODE    _S_IREAD | S_IWRITE 

#endif


#if defined(_WIN64)
#define MYFTELL(s) (int64_t)_ftelli64(s)
#define MYFSEEK(s,o,w) _fseeki64(s,(__int64)o,w)
#else
#define MYFTELL(s) ftell(s)
#define MYFSEEK(s,o,w) fseek(s,o,w)
#endif


#include <errno.h>
#include <libmeshb7.h>

// [Bruno] Using portable printf modifier from pstdint.h
// (alternative: use "%zd" under Linux and "%Id" under Windows)

#ifdef PRINTF_INT64_MODIFIER
#define INT64_T_FMT "%" PRINTF_INT64_MODIFIER "d"
#else
#   ifdef GMF_WINDOWS
#    define INT64_T_FMT "%Id"
#   else
#    include <inttypes.h>
#    define INT64_T_FMT "%" PRId64
#   endif
#endif


// AIO: hardware or software mockup are both encapsulated into my_aio functions

#ifdef WITH_GMF_AIO

#include <aio.h>

int    my_aio_error (const struct aiocb *aiocbp){return(aio_error (aiocbp));}
int    my_aio_read  (      struct aiocb *aiocbp){return(aio_read  (aiocbp));}
size_t my_aio_return(      struct aiocb *aiocbp){return(aio_return(aiocbp));}
int    my_aio_write (      struct aiocb *aiocbp){return(aio_write (aiocbp));}

#else

// Mockup aio library

struct aiocb
{
   FILE   *aio_fildes;         // File descriptor
   off_t  aio_offset;          // File offset
   void   *aio_buf;            // Location of buffer
   size_t aio_nbytes;          // Length of transfer
   int    aio_lio_opcode;      // Operation to be performed
};

int my_aio_error(const struct aiocb *aiocbp)
{
   return(aiocbp->aio_lio_opcode);
}

// Set the file position and read a block of data
int my_aio_read(struct aiocb *aiocbp)
{
   if( (MYFSEEK(aiocbp->aio_fildes, (off_t)aiocbp->aio_offset, SEEK_SET) == 0)
   &&  (fread(aiocbp->aio_buf, 1, aiocbp->aio_nbytes, aiocbp->aio_fildes)
       == aiocbp->aio_nbytes) )
   {
      aiocbp->aio_lio_opcode = 0;
   }
   else
   {
      aiocbp->aio_lio_opcode = -1;
   }

   return(aiocbp->aio_lio_opcode);
}

size_t my_aio_return(struct aiocb *aiocbp)
{
   return(aiocbp->aio_nbytes);
}

// Set the file position and write a block of data
int my_aio_write(struct aiocb *aiocbp)
{
   if( (MYFSEEK(aiocbp->aio_fildes, (off_t)aiocbp->aio_offset, SEEK_SET) == 0)
   &&  (fwrite(aiocbp->aio_buf, 1, aiocbp->aio_nbytes, aiocbp->aio_fildes)
       == aiocbp->aio_nbytes) )
   {
      aiocbp->aio_lio_opcode = 0;
   }
   else
   {
      aiocbp->aio_lio_opcode = -1;
   }

   return(aiocbp->aio_lio_opcode);
}

#endif


/*----------------------------------------------------------------------------*/
/* Defines                                                                    */
/*----------------------------------------------------------------------------*/

#define Asc       1
#define Bin       2
#define MshFil    4
#define SolFil    8
#define InfKwd    1
#define RegKwd    2
#define SolKwd    3
#define CmtKwd    4
#define WrdSiz    4
#define FilStrSiz 64
#define BufSiz    10000L
#define MaxArg    20



/*----------------------------------------------------------------------------*/
/* Structures                                                                 */
/*----------------------------------------------------------------------------*/

typedef struct
{
   int      typ, deg, NmbNod, SolSiz, NmbWrd, NmbTyp, TypTab[ GmfMaxTyp ];
   int      *OrdTab;
   int64_t  NmbLin;
   size_t   pos;
   char     fmt[ GmfMaxTyp*9 ];
}KwdSct;

typedef struct
{
   int      dim, ver, mod, typ, cod, FilDes, FltSiz;
   int64_t  NexKwdPos, siz;
   size_t   pos;
   jmp_buf  err;
   KwdSct   KwdTab[ GmfMaxKwd + 1 ];
   FILE     *hdl;
   int      *IntBuf;
   float    *FltBuf;
   char     *buf;
   char     FilNam[ GmfStrSiz ];
   double   DblBuf[1000/8];
   unsigned char blk[ BufSiz + 1000L ];
}GmfMshSct;


/*----------------------------------------------------------------------------*/
/* Global variables                                                           */
/*----------------------------------------------------------------------------*/

const char *GmfKwdFmt[ GmfMaxKwd + 1 ][3] = 
{
   {"Reserved",                                 "", ""},
   {"MeshVersionFormatted",                     "", "i"},
   {"Reserved",                                 "", ""},
   {"Dimension",                                "", "i"},
   {"Vertices",                                 "i", "dri"},
   {"Edges",                                    "i", "iii"},
   {"Triangles",                                "i", "iiii"},
   {"Quadrilaterals",                           "i", "iiiii"},
   {"Tetrahedra",                               "i", "iiiii"},
   {"Prisms",                                   "i", "iiiiiii"},
   {"Hexahedra",                                "i", "iiiiiiiii"},
   {"Reserved",                                 "",  ""},
   {"Reserved",                                 "",  ""},
   {"Corners",                                  "i", "i"},
   {"Ridges",                                   "i", "i"},
   {"RequiredVertices",                         "i", "i"},
   {"RequiredEdges",                            "i", "i"},
   {"RequiredTriangles",                        "i", "i"},
   {"RequiredQuadrilaterals",                   "i", "i"},
   {"TangentAtEdgeVertices",                    "i", "iii"},
   {"NormalAtVertices",                         "i", "ii"},
   {"NormalAtTriangleVertices",                 "i", "iii"},
   {"NormalAtQuadrilateralVertices",            "i", "iiii"},
   {"AngleOfCornerBound",                       "",  "r"},
   {"TrianglesP2",                              "i", "iiiiiii"},
   {"EdgesP2",                                  "i", "iiii"},
   {"SolAtPyramids",                            "i", "sr"},
   {"QuadrilateralsQ2",                         "i", "iiiiiiiiii"},
   {"ISolAtPyramids",                           "i", "iiiii"},
   {"SubDomainFromGeom",                        "i", "iii"},
   {"TetrahedraP2",                             "i", "iiiiiiiiiii"},
   {"Fault_NearTri",                            "i", "i"},
   {"Fault_Inter",                              "i", "i"},
   {"HexahedraQ2",                              "i", "iiiiiiiiiiiiiiiiiiiiiiiiiiii"},
   {"ExtraVerticesAtEdges",                     "i", "in"},
   {"ExtraVerticesAtTriangles",                 "i", "in"},
   {"ExtraVerticesAtQuadrilaterals",            "i", "in"},
   {"ExtraVerticesAtTetrahedra",                "i", "in"},
   {"ExtraVerticesAtPrisms",                    "i", "in"},
   {"ExtraVerticesAtHexahedra",                 "i", "in"},
   {"VerticesOnGeometricVertices",              "i", "ii"},
   {"VerticesOnGeometricEdges",                 "i", "iirr"},
   {"VerticesOnGeometricTriangles",             "i", "iirrr"},
   {"VerticesOnGeometricQuadrilaterals",        "i", "iirrr"},
   {"EdgesOnGeometricEdges",                    "i", "ii"},
   {"Fault_FreeEdge",                           "i", "i"},
   {"Polyhedra",                                "i", "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"},
   {"Polygons",                                 "",  "iiiiiiiii"},
   {"Fault_Overlap",                            "i", "i"},
   {"Pyramids",                                 "i", "iiiiii"},
   {"BoundingBox",                              "",  "drdr"},
   {"Reserved",                                 "",  ""},
   {"PrivateTable",                             "i", "i"},
   {"Fault_BadShape",                           "i", "i"},
   {"End",                                      "",  ""},
   {"TrianglesOnGeometricTriangles",            "i", "ii"},
   {"TrianglesOnGeometricQuadrilaterals",       "i", "ii"},
   {"QuadrilateralsOnGeometricTriangles",       "i", "ii"},
   {"QuadrilateralsOnGeometricQuadrilaterals",  "i", "ii"},
   {"Tangents",                                 "i", "dr"},
   {"Normals",                                  "i", "dr"},
   {"TangentAtVertices",                        "i", "ii"},
   {"SolAtVertices",                            "i", "sr"},
   {"SolAtEdges",                               "i", "sr"},
   {"SolAtTriangles",                           "i", "sr"},
   {"SolAtQuadrilaterals",                      "i", "sr"},
   {"SolAtTetrahedra",                          "i", "sr"},
   {"SolAtPrisms",                              "i", "sr"},
   {"SolAtHexahedra",                           "i", "sr"},
   {"DSolAtVertices",                           "i", "sr"},
   {"ISolAtVertices",                           "i", "i"},
   {"ISolAtEdges",                              "i", "ii"},
   {"ISolAtTriangles",                          "i", "iii"},
   {"ISolAtQuadrilaterals",                     "i", "iiii"},
   {"ISolAtTetrahedra",                         "i", "iiii"},
   {"ISolAtPrisms",                             "i", "iiiiii"},
   {"ISolAtHexahedra",                          "i", "iiiiiiii"},
   {"Iterations",                               "",  "i"},
   {"Time",                                     "",  "r"},
   {"Fault_SmallTri",                           "i", "i"},
   {"CoarseHexahedra",                          "i", "i"},
   {"Comments",                                 "i", "c"},
   {"PeriodicVertices",                         "i", "ii"},
   {"PeriodicEdges",                            "i", "ii"},
   {"PeriodicTriangles",                        "i", "ii"},
   {"PeriodicQuadrilaterals",                   "i", "ii"},
   {"PrismsP2",                                 "i", "iiiiiiiiiiiiiiiiiii"},
   {"PyramidsP2",                               "i", "iiiiiiiiiiiiiii"},
   {"QuadrilateralsQ3",                         "i", "iiiiiiiiiiiiiiiii"},
   {"QuadrilateralsQ4",                         "i", "iiiiiiiiiiiiiiiiiiiiiiiiii"},
   {"TrianglesP3",                              "i", "iiiiiiiiiii"},
   {"TrianglesP4",                              "i", "iiiiiiiiiiiiiiii"},
   {"EdgesP3",                                  "i", "iiiii"},
   {"EdgesP4",                                  "i", "iiiiii"},
   {"IRefGroups",                               "i", "ciii"},
   {"DRefGroups",                               "i", "iii"},
   {"TetrahedraP3",                             "i", "iiiiiiiiiiiiiiiiiiiii"},
   {"TetrahedraP4",                             "i", "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"},
   {"HexahedraQ3",                              "i", "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"},
   {"HexahedraQ4",                              "i", "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"},
   {"PyramidsP3",                               "i", "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"},
   {"PyramidsP4",                               "i", "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"},
   {"PrismsP3",                                 "i", "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"},
   {"PrismsP4",                                 "i", "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"},
   {"HOSolAtEdgesP1",                           "i", "hr"},
   {"HOSolAtEdgesP2",                           "i", "hr"},
   {"HOSolAtEdgesP3",                           "i", "hr"},
   {"HOSolAtTrianglesP1",                       "i", "hr"},
   {"HOSolAtTrianglesP2",                       "i", "hr"},
   {"HOSolAtTrianglesP3",                       "i", "hr"},
   {"HOSolAtQuadrilateralsQ1",                  "i", "hr"},
   {"HOSolAtQuadrilateralsQ2",                  "i", "hr"},
   {"HOSolAtQuadrilateralsQ3",                  "i", "hr"},
   {"HOSolAtTetrahedraP1",                      "i", "hr"},
   {"HOSolAtTetrahedraP2",                      "i", "hr"},
   {"HOSolAtTetrahedraP3",                      "i", "hr"},
   {"HOSolAtPyramidsP1",                        "i", "hr"},
   {"HOSolAtPyramidsP2",                        "i", "hr"},
   {"HOSolAtPyramidsP3",                        "i", "hr"},
   {"HOSolAtPrismsP1",                          "i", "hr"},
   {"HOSolAtPrismsP2",                          "i", "hr"},
   {"HOSolAtPrismsP3",                          "i", "hr"},
   {"HOSolAtHexahedraQ1",                       "i", "hr"},
   {"HOSolAtHexahedraQ2",                       "i", "hr"},
   {"HOSolAtHexahedraQ3",                       "i", "hr"},
   {"BezierBasis",                              "",  "i"},
   {"ByteFlow",                                 "i", "i"},
   {"EdgesP2Ordering",                          "i", "i"},
   {"EdgesP3Ordering",                          "i", "i"},
   {"TrianglesP2Ordering",                      "i", "iii"},
   {"TrianglesP3Ordering",                      "i", "iii"},
   {"QuadrilateralsQ2Ordering",                 "i", "ii"},
   {"QuadrilateralsQ3Ordering",                 "i", "ii"},
   {"TetrahedraP2Ordering",                     "i", "iiii"},
   {"TetrahedraP3Ordering",                     "i", "iiii"},
   {"PyramidsP2Ordering",                       "i", "iii"},
   {"PyramidsP3Ordering",                       "i", "iii"},
   {"PrismsP2Ordering",                         "i", "iiii"},
   {"PrismsP3Ordering",                         "i", "iiii"},
   {"HexahedraQ2Ordering",                      "i", "iii"},
   {"HexahedraQ3Ordering",                      "i", "iii"},
   {"EdgesP1Ordering",                          "i", "i"},
   {"EdgesP4Ordering",                          "i", "i"},
   {"TrianglesP1Ordering",                      "i", "iii"},
   {"TrianglesP4Ordering",                      "i", "iii"},
   {"QuadrilateralsQ1Ordering",                 "i", "ii"},
   {"QuadrilateralsQ4Ordering",                 "i", "ii"},
   {"TetrahedraP1Ordering",                     "i", "iiii"},
   {"TetrahedraP4Ordering",                     "i", "iiii"},
   {"PyramidsP1Ordering",                       "i", "iii"},
   {"PyramidsP4Ordering",                       "i", "iii"},
   {"PrismsP1Ordering",                         "i", "iiii"},
   {"PrismsP4Ordering",                         "i", "iiii"},
   {"HexahedraQ1Ordering",                      "i", "iii"},
   {"HexahedraQ4Ordering",                      "i", "iii"},
   {"FloatingPointPrecision",                   "",  "i"},
   {"HOSolAtEdgesP4",                           "i", "hr"},
   {"HOSolAtTrianglesP4",                       "i", "hr"},
   {"HOSolAtQuadrilateralsQ4",                  "i", "hr"},
   {"HOSolAtTetrahedraP4",                      "i", "hr"},
   {"HOSolAtPyramidsP4",                        "i", "hr"},
   {"HOSolAtPrismsP4",                          "i", "hr"},
   {"HOSolAtHexahedraQ4",                       "i", "hr"},
   {"HOSolAtEdgesP1NodesPositions",             "i", "rr"},
   {"HOSolAtEdgesP2NodesPositions",             "i", "rr"},
   {"HOSolAtEdgesP3NodesPositions",             "i", "rr"},
   {"HOSolAtEdgesP4NodesPositions",             "i", "rr"},
   {"HOSolAtTrianglesP1NodesPositions",         "i", "rrr"},
   {"HOSolAtTrianglesP2NodesPositions",         "i", "rrr"},
   {"HOSolAtTrianglesP3NodesPositions",         "i", "rrr"},
   {"HOSolAtTrianglesP4NodesPositions",         "i", "rrr"},
   {"HOSolAtQuadrilateralsQ1NodesPositions",    "i", "rr"},
   {"HOSolAtQuadrilateralsQ2NodesPositions",    "i", "rr"},
   {"HOSolAtQuadrilateralsQ3NodesPositions",    "i", "rr"},
   {"HOSolAtQuadrilateralsQ4NodesPositions",    "i", "rr"},
   {"HOSolAtTetrahedraP1NodesPositions",        "i", "rrrr"},
   {"HOSolAtTetrahedraP2NodesPositions",        "i", "rrrr"},
   {"HOSolAtTetrahedraP3NodesPositions",        "i", "rrrr"},
   {"HOSolAtTetrahedraP4NodesPositions",        "i", "rrrr"},
   {"HOSolAtPyramidsP1NodesPositions",          "i", "rrr"},
   {"HOSolAtPyramidsP2NodesPositions",          "i", "rrr"},
   {"HOSolAtPyramidsP3NodesPositions",          "i", "rrr"},
   {"HOSolAtPyramidsP4NodesPositions",          "i", "rrr"},
   {"HOSolAtPrismsP1NodesPositions",            "i", "rrrr"},
   {"HOSolAtPrismsP2NodesPositions",            "i", "rrrr"},
   {"HOSolAtPrismsP3NodesPositions",            "i", "rrrr"},
   {"HOSolAtPrismsP4NodesPositions",            "i", "rrrr"},
   {"HOSolAtHexahedraQ1NodesPositions",         "i", "rrr"},
   {"HOSolAtHexahedraQ2NodesPositions",         "i", "rrr"},
   {"HOSolAtHexahedraQ3NodesPositions",         "i", "rrr"},
   {"HOSolAtHexahedraQ4NodesPositions",         "i", "rrr"},
   {"EdgesReferenceElement",                    "",  "rr"},
   {"TriangleReferenceElement",                 "",  "rrrrrr"},
   {"QuadrilateralReferenceElement",            "",  "rrrrrrrr"},
   {"TetrahedronReferenceElement",              "",  "rrrrrrrrrrrr"},
   {"PyramidReferenceElement",                  "",  "rrrrrrrrrrrrrrr"},
   {"PrismReferenceElement",                    "",  "rrrrrrrrrrrrrrrrrr"},
   {"HexahedronReferenceElement",               "",  "rrrrrrrrrrrrrrrrrrrrrrrr"},
   {"BoundaryLayers",                           "i", "iii"},
   {"ReferenceStrings",                         "i", "iic"},
   {"Prisms9",                                  "i", "iiiiiiiiii"},
   {"Hexahedra12",                              "i", "iiiiiiiiiiiii"},
   {"Quadrilaterals6",                          "i", "iiiiiii"},
   {"BoundaryPolygonHeaders",                   "i", "ii"},
   {"BoundaryPolygonVertices",                  "i", "i"},
   {"InnerPolygonHeaders",                      "i", "ii"},
   {"InnerPolygonVertices",                     "i", "i"},
   {"PolyhedraHeaders",                         "i", "ii"},
   {"PolyhedraFaces",                           "i", "i"},
   {"Domains",                                  "",  "ii"},
   {"VerticesGID",                              "i", "iii"},
   {"EdgesGID",                                 "i", "iii"},
   {"TrianglesGID",                             "i", "iii"},
   {"QuadrilateralsGID",                        "i", "iii"},
   {"TetrahedraGID",                            "i", "iii"},
   {"PyramidsGID",                              "i", "iii"},
   {"PrismsGID",                                "i", "iii"},
   {"HexahedraGID",                             "i", "iii"},
};

#ifdef TRANSMESH
int GmfMaxRefTab[ GmfMaxKwd + 1 ];
#endif


/*----------------------------------------------------------------------------*/
/* Prototypes of local procedures                                             */
/*----------------------------------------------------------------------------*/

static void    ScaWrd   (GmfMshSct *, void *);
static void    ScaDblWrd(GmfMshSct *, void *);
static int64_t GetPos   (GmfMshSct *);
static void    RecWrd   (GmfMshSct *, const void *);
static void    RecDblWrd(GmfMshSct *, const void *);
static void    RecBlk   (GmfMshSct *, const void *, int);
static void    SetPos   (GmfMshSct *, int64_t);
static int     ScaKwdTab(GmfMshSct *);
static void    ExpFmt   (GmfMshSct *, int);
static void    ScaKwdHdr(GmfMshSct *, int);
static void    SwpWrd   (char *, int);
static int     SetFilPos(GmfMshSct *, int64_t);
static int64_t GetFilPos(GmfMshSct *msh);
static int64_t GetFilSiz(GmfMshSct *);
#ifdef F77API
static void    CalF77Prc(int64_t, int64_t, void *, int, void **);
#endif


/*----------------------------------------------------------------------------*/
/* Fscanf and fgets checking for errors                                       */
/*----------------------------------------------------------------------------*/

#define safe_fscanf(hdl, format, ptr, JmpErr) \
   do { \
      if( fscanf(hdl, format, ptr) != 1 ) \
         longjmp( JmpErr, -1); \
   } while(0)


#define safe_fgets(ptr, siz, hdl, JmpErr) \
   do { \
      if( fgets(ptr, siz, hdl) == NULL ) \
         longjmp( JmpErr, -2); \
   } while(0)


#define safe_fread(ptr, siz, nit, str, JmpErr) \
   do { \
      if( fread(ptr, siz, nit, str) != nit ) \
         longjmp( JmpErr, -3); \
   } while(0)


/*----------------------------------------------------------------------------*/
/* Open a mesh file in read or write mode                                     */
/*----------------------------------------------------------------------------*/

int64_t GmfOpenMesh(const char *FilNam, int mod, ...)
{
   int      KwdCod, res, *PtrVer, *PtrDim, err;
   int64_t  MshIdx;
   char     str[ GmfStrSiz ];
   va_list  VarArg;
   GmfMshSct *msh;

   /*---------------------*/
   /* MESH STRUCTURE INIT */
   /*---------------------*/

   if(!(msh = calloc(1, sizeof(GmfMshSct))))
      return(0);

   MshIdx = (int64_t)msh;

   // Save the current stack environment for longjmp
   if( (err = setjmp(msh->err)) != 0)
   {
#ifdef GMFDEBUG
      printf("libMeshb : mesh %p : error %d\n", msh, err);
#endif
      if(msh->hdl != NULL)
         fclose(msh->hdl);

      if(msh->FilDes != 0)
#ifdef GMF_WINDOWS
         _close(msh->FilDes);
#else
         close(msh->FilDes);
#endif

      free(msh);
      return(0);
   }

   // Copy the FilNam into the structure
   if(strlen(FilNam) + 7 >= GmfStrSiz)
      longjmp(msh->err, -4);

   strcpy(msh->FilNam, FilNam);

   // Store the opening mod (read or write) and guess
   // the filetype (binary or ascii) depending on the extension
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
      longjmp(msh->err, -5);

   // Open the file in the required mod and initialize the mesh structure
   if(msh->mod == GmfRead)
   {

      /*-----------------------*/
      /* OPEN FILE FOR READING */
      /*-----------------------*/

      va_start(VarArg, mod);
      PtrVer = va_arg(VarArg, int *);
      PtrDim = va_arg(VarArg, int *);
      va_end(VarArg);

      // Read the endian coding tag, the mesh version
      // and the mesh dimension (mandatory kwd)
      if(msh->typ & Bin)
      {
         // Create the name string and open the file
#ifdef WITH_GMF_AIO
         // [Bruno] added binary flag (necessary under Windows)
         msh->FilDes = open(msh->FilNam, OPEN_READ_FLAGS, OPEN_READ_MODE);

         if(msh->FilDes <= 0)
            longjmp(msh->err, -6);

         // Read the endian coding tag
         if(read(msh->FilDes, &msh->cod, WrdSiz) != WrdSiz)
            longjmp(msh->err, -7);
#else
         // [Bruno] added binary flag (necessary under Windows)
         if(!(msh->hdl = fopen(msh->FilNam, "rb")))
            longjmp(msh->err, -8);

         // Read the endian coding tag
         safe_fread(&msh->cod, WrdSiz, 1, msh->hdl, msh->err);
#endif

         // Read the mesh version and the mesh dimension (mandatory kwd)
         if( (msh->cod != 1) && (msh->cod != 16777216) )
            longjmp(msh->err, -9);

         ScaWrd(msh, (unsigned char *)&msh->ver);

         if( (msh->ver < 1) || (msh->ver > 4) )
            longjmp(msh->err, -10);

         if( (msh->ver >= 3) && (sizeof(int64_t) != 8) )
            longjmp(msh->err, -11);

         ScaWrd(msh, (unsigned char *)&KwdCod);

         if(KwdCod != GmfDimension)
            longjmp(msh->err, -12);

         GetPos(msh);
         ScaWrd(msh, (unsigned char *)&msh->dim);
      }
      else
      {
         // Create the name string and open the file
         if(!(msh->hdl = fopen(msh->FilNam, "rb")))
            longjmp(msh->err, -13);

         do
         {
            res = fscanf(msh->hdl, "%100s", str);
         }while( (res != EOF) && strcmp(str, "MeshVersionFormatted") );

         if(res == EOF)
            longjmp(msh->err, -14);

         safe_fscanf(msh->hdl, "%d", &msh->ver, msh->err);

         if( (msh->ver < 1) || (msh->ver > 4) )
            longjmp(msh->err, -15);

         do
         {
            res = fscanf(msh->hdl, "%100s", str);
         }while( (res != EOF) && strcmp(str, "Dimension") );

         if(res == EOF)
            longjmp(msh->err, -16);

         safe_fscanf(msh->hdl, "%d", &msh->dim, msh->err);
      }

      if( (msh->dim != 2) && (msh->dim != 3) )
         longjmp(msh->err, -17);

      (*PtrVer) = msh->ver;
      (*PtrDim) = msh->dim;

      // Set default real numbers size
      if(msh->ver == 1)
         msh->FltSiz = 32;
      else
         msh->FltSiz = 64;

      /*------------*/
      /* KW READING */
      /*------------*/

      // Read the list of kw present in the file
      if(!ScaKwdTab(msh))
         return(0);

      return(MshIdx);
   }
   else if(msh->mod == GmfWrite)
   {

      /*-----------------------*/
      /* OPEN FILE FOR WRITING */
      /*-----------------------*/

      msh->cod = 1;

      // Check if the user provided a valid version number and dimension
      va_start(VarArg, mod);
      msh->ver = va_arg(VarArg, int);
      msh->dim = va_arg(VarArg, int);
      va_end(VarArg);

      if( (msh->ver < 1) || (msh->ver > 4) )
         longjmp(msh->err, -18);

      if( (msh->ver >= 3) && (sizeof(int64_t) != 8) )
         longjmp(msh->err, -19);

      if( (msh->dim != 2) && (msh->dim != 3) )
         longjmp(msh->err, -20);

      // Set default real numbers size
      if(msh->ver == 1)
         msh->FltSiz = 32;
      else
         msh->FltSiz = 64;

      // Create the mesh file
      if(msh->typ & Bin) 
      {
         /* 
          * [Bruno] replaced previous call to creat():
          * with a call to open(), because Windows needs the
          * binary flag to be specified.
          */
#ifdef WITH_GMF_AIO
         msh->FilDes = open(msh->FilNam, OPEN_WRITE_FLAGS, OPEN_WRITE_MODE);

         if(msh->FilDes <= 0)
            longjmp(msh->err, -21);
#else
         if(!(msh->hdl = fopen(msh->FilNam, "wb")))
            longjmp(msh->err, -22);
#endif
      }
      else if(!(msh->hdl = fopen(msh->FilNam, "wb")))
         longjmp(msh->err, -23);


      /*------------*/
      /* KW WRITING */
      /*------------*/

      // Write the mesh version and dimension
      if(msh->typ & Asc)
      {
         fprintf(msh->hdl, "%s %d\n\n",
               GmfKwdFmt[ GmfVersionFormatted ][0], msh->ver);
         fprintf(msh->hdl, "%s %d\n",
               GmfKwdFmt[ GmfDimension ][0], msh->dim);
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


/*----------------------------------------------------------------------------*/
/* Close a meshfile in the right way                                          */
/*----------------------------------------------------------------------------*/

int GmfCloseMesh(int64_t MshIdx)
{
   int i, res = 1;
   GmfMshSct *msh = (GmfMshSct *)MshIdx;

   RecBlk(msh, msh->buf, 0);

   // In write down the "End" kw in write mode
   if(msh->mod == GmfWrite)
   {
      if(msh->typ & Asc)
         fprintf(msh->hdl, "\n%s\n", GmfKwdFmt[ GmfEnd ][0]);
      else
         GmfSetKwd(MshIdx, GmfEnd, 0);
   }

   // Close the file and free the mesh structure
   if(msh->typ & Bin)
#ifdef WITH_GMF_AIO
      close(msh->FilDes);
#else
      fclose(msh->hdl);
#endif
   else if(fclose(msh->hdl))
      res = 0;

   // Free optional H.O. renumbering tables
   for(i=0;i<GmfLastKeyword;i++)
      if(msh->KwdTab[i].OrdTab)
         free(msh->KwdTab[i].OrdTab);

   free(msh);

   return(res);
}


/*----------------------------------------------------------------------------*/
/* Read the number of lines and set the position to this kwd                  */
/*----------------------------------------------------------------------------*/

int64_t GmfStatKwd(int64_t MshIdx, int KwdCod, ...)
{
   int         i, *PtrNmbTyp, *PtrSolSiz, *TypTab, *PtrDeg, *PtrNmbNod;
   GmfMshSct   *msh = (GmfMshSct *)MshIdx;
   KwdSct      *kwd;
   va_list     VarArg;

   if( (KwdCod < 1) || (KwdCod > GmfMaxKwd) )
      return(0);

   kwd = &msh->KwdTab[ KwdCod ];

   if(!kwd->NmbLin)
      return(0);

   // Read further arguments if this kw is a sol
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

      // Add two extra paramaters for HO elements: degree and nmb nodes
      if(!strcmp("hr", GmfKwdFmt[ KwdCod ][2]) )
      {
         PtrDeg = va_arg(VarArg, int *);
         *PtrDeg = kwd->deg;
         
         PtrNmbNod = va_arg(VarArg, int *);
         *PtrNmbNod = kwd->NmbNod;
      }

      va_end(VarArg);
   }

   return(kwd->NmbLin);
}


/*----------------------------------------------------------------------------*/
/* Set the current file position to a given kwd                               */
/*----------------------------------------------------------------------------*/

int GmfGotoKwd(int64_t MshIdx, int KwdCod)
{
   GmfMshSct   *msh = (GmfMshSct *)MshIdx;
   KwdSct      *kwd = &msh->KwdTab[ KwdCod ];

   if( (KwdCod < 1) || (KwdCod > GmfMaxKwd) || !kwd->NmbLin )
      return(0);

   return(SetFilPos(msh, kwd->pos));
}


/*----------------------------------------------------------------------------*/
/* Write the kwd and set the number of lines                                  */
/*----------------------------------------------------------------------------*/

int GmfSetKwd(int64_t MshIdx, int KwdCod, int64_t NmbLin, ...)
{
   int         i, *TypTab;
   int64_t     CurPos;
   va_list     VarArg;
   GmfMshSct   *msh = (GmfMshSct *)MshIdx;
   KwdSct      *kwd;

   RecBlk(msh, msh->buf, 0);

   if( (KwdCod < 1) || (KwdCod > GmfMaxKwd) )
      return(0);

   kwd = &msh->KwdTab[ KwdCod ];

   // Read further arguments if this kw is a solution
   if(!strcmp(GmfKwdFmt[ KwdCod ][2], "sr")
   || !strcmp(GmfKwdFmt[ KwdCod ][2], "hr"))
   {
      va_start(VarArg, NmbLin);

      kwd->NmbTyp = va_arg(VarArg, int);
      TypTab = va_arg(VarArg, int *);

      for(i=0;i<kwd->NmbTyp;i++)
         kwd->TypTab[i] = TypTab[i];

      // Add two extra paramaters for HO elements: degree and nmb nodes
      if(!strcmp("hr", GmfKwdFmt[ KwdCod ][2]))
      {
         kwd->deg = va_arg(VarArg, int);
         kwd->NmbNod = va_arg(VarArg, int);
      }

      va_end(VarArg);
   }

   // Setup the kwd info
   ExpFmt(msh, KwdCod);

   if(!kwd->typ)
      return(0);
   else if(kwd->typ == InfKwd)
      kwd->NmbLin = 1;
   else
      kwd->NmbLin = NmbLin;

   // Store the next kwd position in binary file
   if( (msh->typ & Bin) && msh->NexKwdPos )
   {
      CurPos = GetFilPos(msh);

      if(!SetFilPos(msh, msh->NexKwdPos))
         return(0);

      SetPos(msh, CurPos);

      if(!SetFilPos(msh, CurPos))
         return(0);
   }

   // Write the header
   if(msh->typ & Asc)
   {
      fprintf(msh->hdl, "\n%s\n", GmfKwdFmt[ KwdCod ][0]);

      if(kwd->typ != InfKwd)
         fprintf(msh->hdl, INT64_T_FMT"\n", kwd->NmbLin);

      // In case of solution field, write the extended header
      if(kwd->typ == SolKwd)
      {
         fprintf(msh->hdl, "%d ", kwd->NmbTyp);

         for(i=0;i<kwd->NmbTyp;i++)
            fprintf(msh->hdl, "%d ", kwd->TypTab[i]);

         fprintf(msh->hdl, "\n");
      }

      if(!strcmp("hr", GmfKwdFmt[ KwdCod ][2]))
         fprintf(msh->hdl, "%d %d\n", kwd->deg, kwd->NmbNod);
   }
   else
   {
      RecWrd(msh, (unsigned char *)&KwdCod);
      msh->NexKwdPos = GetFilPos(msh);
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

      // In case of solution field, write the extended header at once
      if(kwd->typ == SolKwd)
      {
         RecWrd(msh, (unsigned char *)&kwd->NmbTyp);

         for(i=0;i<kwd->NmbTyp;i++)
            RecWrd(msh, (unsigned char *)&kwd->TypTab[i]);

         if(!strcmp("hr", GmfKwdFmt[ KwdCod ][2]))
         {
            RecWrd(msh, (unsigned char *)&kwd->deg);
            RecWrd(msh, (unsigned char *)&kwd->NmbNod);
         }
      }
   }

   // Reset write buffer position
   msh->pos = 0;

   // Compute the total file size and check if it crosses the 2GB threshold
   msh->siz += kwd->NmbLin * kwd->NmbWrd * WrdSiz;

   return(1);
}


/*----------------------------------------------------------------------------*/
/* Read a full line from the current kwd                                      */
/*----------------------------------------------------------------------------*/

int NAMF77(GmfGetLin, gmfgetlin)(TYPF77(int64_t)MshIdx, TYPF77(int)KwdCod, ...)
{
   int         i, err;
   float       *FltSolTab, FltVal, *PtrFlt;
   double      *DblSolTab, *PtrDbl;
   va_list     VarArg;
   GmfMshSct   *msh = (GmfMshSct *) VALF77(MshIdx);
   KwdSct      *kwd = &msh->KwdTab[ VALF77(KwdCod) ];

   if( (VALF77(KwdCod) < 1) || (VALF77(KwdCod) > GmfMaxKwd) )
      return(0);

   // Save the current stack environment for longjmp
   if( (err = setjmp(msh->err)) != 0)
   {
#ifdef GMFDEBUG
      printf("libMeshb : mesh %p : error %d\n", msh, err);
#endif
      return(0);
   }

   // Start decoding the arguments
   va_start(VarArg, KwdCod);

   switch(kwd->typ)
   {
      case InfKwd : case RegKwd : case CmtKwd :
      {
         if(msh->typ & Asc)
         {
            for(i=0;i<kwd->SolSiz;i++)
            {
               if(kwd->fmt[i] == 'r')
               {
                  if(msh->FltSiz == 32)
                  {
                     safe_fscanf(msh->hdl, "%f", &FltVal, msh->err);
                     PtrDbl = va_arg(VarArg, double *);
                     PtrFlt = (float *)PtrDbl;
                     *PtrFlt = FltVal;
                  }                     
                  else
                  {
                     safe_fscanf(msh->hdl, "%lf",
                              va_arg(VarArg, double *), msh->err);
                  }
               }
               else if(kwd->fmt[i] == 'i')
               {
                  if(msh->ver <= 3)
                  {
                     safe_fscanf(msh->hdl, "%d",
                        va_arg(VarArg, int *), msh->err);
                  }
                  else
                  {
                     // [Bruno] %ld -> INT64_T_FMT
                     safe_fscanf(msh->hdl, INT64_T_FMT,
                              va_arg(VarArg, int64_t *), msh->err);
                  }
               }
               else if(kwd->fmt[i] == 'c')
               {
                  safe_fgets( va_arg(VarArg, char *),
                              WrdSiz * FilStrSiz, msh->hdl, msh->err);
               }
            }
         }
         else
         {
            for(i=0;i<kwd->SolSiz;i++)
               if(kwd->fmt[i] == 'r')
                  if(msh->FltSiz == 32)
                     ScaWrd(msh, (unsigned char *)va_arg(VarArg, float *));
                  else
                     ScaDblWrd(msh, (unsigned char *)va_arg(VarArg, double *));
               else if(kwd->fmt[i] == 'i')
                  if(msh->ver <= 3)
                     ScaWrd(msh, (unsigned char *)va_arg(VarArg, int *));
                  else
                     ScaDblWrd(msh, (unsigned char *)va_arg(VarArg, int64_t *));
               else if(kwd->fmt[i] == 'c')
                  // [Bruno] added error control
                  safe_fread(va_arg(VarArg, char *), WrdSiz, FilStrSiz, msh->hdl, msh->err);
         }
      }break;

      case SolKwd :
      {
         if(msh->FltSiz == 32)
         {
            FltSolTab = va_arg(VarArg, float *);

            if(msh->typ & Asc)
               for(i=0; i<kwd->SolSiz; i++)
                  safe_fscanf(msh->hdl, "%f", &FltSolTab[i], msh->err);
            else
               for(i=0; i<kwd->SolSiz; i++)
                  ScaWrd(msh, (unsigned char *)&FltSolTab[i]);
         }
         else
         {
            DblSolTab = va_arg(VarArg, double *);

            if(msh->typ & Asc)
               for(i=0; i<kwd->SolSiz; i++)
                  safe_fscanf(msh->hdl, "%lf", &DblSolTab[i], msh->err);
            else
               for(i=0; i<kwd->SolSiz; i++)
                  ScaDblWrd(msh, (unsigned char *)&DblSolTab[i]);
         }
      }break;
   }

   va_end(VarArg);

   return(1);
}


/*----------------------------------------------------------------------------*/
/* Write a full line from the current kwd                                     */
/*----------------------------------------------------------------------------*/

int NAMF77(GmfSetLin, gmfsetlin)(TYPF77(int64_t) MshIdx, TYPF77(int) KwdCod, ...)
{
   int         i, pos, *IntBuf, err;
   int64_t     *LngBuf;
   float       *FltSolTab, *FltBuf;
   double      *DblSolTab, *DblBuf;
   va_list     VarArg;
   GmfMshSct   *msh = (GmfMshSct *) VALF77(MshIdx);
   KwdSct      *kwd = &msh->KwdTab[ VALF77(KwdCod) ];

   if( ( VALF77(KwdCod) < 1) || ( VALF77(KwdCod) > GmfMaxKwd) )
      return(0);
 
   // Save the current stack environment for longjmp
   // This is needed in RecBlk()
   if( (err = setjmp(msh->err)) != 0)
   {
#ifdef GMFDEBUG
      printf("libMeshb : mesh %p : error %d\n", msh, err);
#endif
      return(0);
   }

   // Save the current stack environment for longjmp
   // This is needed in RecBlk()
   if( (err = setjmp(msh->err)) != 0)
   {
#ifdef GMFDEBUG
      printf("libMeshb : mesh %p : error %d\n", msh, err);
#endif
      return(0);
   }

   // Start decoding the arguments
   va_start(VarArg, KwdCod);

   if(kwd->typ != SolKwd)
   {
      if(msh->typ & Asc)
      {
         for(i=0;i<kwd->SolSiz;i++)
         {
            if(kwd->fmt[i] == 'r')
            {
               if(msh->FltSiz == 32)
#ifdef F77API
                  fprintf(msh->hdl, "%.9g ", *(va_arg(VarArg, float *)));
#else
                  fprintf(msh->hdl, "%.9g ", va_arg(VarArg, double));
#endif
               else
                  fprintf(msh->hdl, "%.17g ", VALF77(va_arg(VarArg, TYPF77(double))));
            }
            else if(kwd->fmt[i] == 'i')
            {
               if(msh->ver <= 3)
                  fprintf(msh->hdl, "%d ", VALF77(va_arg(VarArg, TYPF77(int))));
               else
               {
                  // [Bruno] %ld -> INT64_T_FMT
                  fprintf( msh->hdl, INT64_T_FMT " ",
                           VALF77(va_arg(VarArg, TYPF77(int64_t))));
               }
            }
            else if(kwd->fmt[i] == 'c')
               fprintf(msh->hdl, "%s ", va_arg(VarArg, char *));
         }
      }
      else
      {
         pos = 0;

         for(i=0;i<kwd->SolSiz;i++)
         {
            if(kwd->fmt[i] == 'r')
            {
               if(msh->FltSiz == 32)
               {
                  FltBuf = (void *)&msh->buf[ pos ];
#ifdef F77API
                  *FltBuf = (float)*(va_arg(VarArg, float *));
#else
                  *FltBuf = (float)va_arg(VarArg, double);
#endif
                  pos += 4;
               }
               else
               {
                  DblBuf = (void *)&msh->buf[ pos ];
                  *DblBuf = VALF77(va_arg(VarArg, TYPF77(double)));
                  pos += 8;
               }
            }
            else if(kwd->fmt[i] == 'i')
            {
               if(msh->ver <= 3)
               {
                  IntBuf = (void *)&msh->buf[ pos ];
                  *IntBuf = VALF77(va_arg(VarArg, TYPF77(int)));
                  pos += 4;
               }
               else
               {
                  LngBuf = (void *)&msh->buf[ pos ];
                  *LngBuf = VALF77(va_arg(VarArg, TYPF77(int64_t)));
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
      if(msh->FltSiz == 32)
      {
         FltSolTab = va_arg(VarArg, float *);

         if(msh->typ & Asc)
            for(i=0; i<kwd->SolSiz; i++)
               fprintf(msh->hdl, "%.9g ", (double)FltSolTab[i]);
         else
            RecBlk(msh, (unsigned char *)FltSolTab, kwd->NmbWrd);
      }
      else
      {
         DblSolTab = va_arg(VarArg, double *);

         if(msh->typ & Asc)
            for(i=0; i<kwd->SolSiz; i++)
               fprintf(msh->hdl, "%.17g ", DblSolTab[i]);
         else
            RecBlk(msh, (unsigned char *)DblSolTab, kwd->NmbWrd);
      }
   }

   va_end(VarArg);

   if(msh->typ & Asc)
      fprintf(msh->hdl, "\n");

   return(1);
}


/*----------------------------------------------------------------------------*/
/* Private procedure for mesh : copy a whole line                             */
/*----------------------------------------------------------------------------*/

#ifdef TRANSMESH

int GmfCpyLin(int64_t InpIdx, int64_t OutIdx, int KwdCod)
{
   char        s[ WrdSiz * FilStrSiz ];
   double      d;
   float       f;
   int         i, a, err;
   int64_t     l;
   GmfMshSct   *InpMsh = (GmfMshSct *)InpIdx, *OutMsh = (GmfMshSct *)OutIdx;
   KwdSct      *kwd = &InpMsh->KwdTab[ KwdCod ];

   // Save the current stack environment for longjmp
   if( (err = setjmp(InpMsh->err)) != 0)
   {
#ifdef GMFDEBUG
      printf("libMeshb : mesh %p : error %d\n", InpMsh, err);
#endif
      return(0);
   }

   for(i=0;i<kwd->SolSiz;i++)
   {
      if(kwd->fmt[i] == 'r')
      {
         if(InpMsh->FltSiz == 32)
         {
            if(InpMsh->typ & Asc)
               safe_fscanf(InpMsh->hdl, "%f", &f, InpMsh->err);
            else
               ScaWrd(InpMsh, (unsigned char *)&f);

            d = (double)f;
         }
         else
         {
            if(InpMsh->typ & Asc)
               safe_fscanf(InpMsh->hdl, "%lf", &d, InpMsh->err);
            else
               ScaDblWrd(InpMsh, (unsigned char *)&d);

            f = (float)d;
         }

         if(OutMsh->FltSiz == 32)
            if(OutMsh->typ & Asc)
               fprintf(OutMsh->hdl, "%.9g ", (double)f);
            else
               RecWrd(OutMsh, (unsigned char *)&f);
         else
            if(OutMsh->typ & Asc)
               fprintf(OutMsh->hdl, "%.17g ", d);
            else
               RecDblWrd(OutMsh, (unsigned char *)&d);
      }
      else if(kwd->fmt[i] == 'i')
      {
         if(InpMsh->ver <= 3)
         {
            if(InpMsh->typ & Asc)
               safe_fscanf(InpMsh->hdl, "%d", &a, InpMsh->err);
            else
               ScaWrd(InpMsh, (unsigned char *)&a);

            l = (int64_t)a;
         }
         else
         {
            if(InpMsh->typ & Asc)
               safe_fscanf(InpMsh->hdl, INT64_T_FMT, &l, InpMsh->err);
            else
               ScaDblWrd(InpMsh, (unsigned char *)&l);

            a = (int)l;
         }

         if( (i == kwd->SolSiz-1) && (a > GmfMaxRefTab[ KwdCod ]) )
            GmfMaxRefTab[ KwdCod ] = a;

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
               fprintf(OutMsh->hdl, INT64_T_FMT" ", l);
            else
               RecDblWrd(OutMsh, (unsigned char *)&l);
         }
      }
      else if(kwd->fmt[i] == 'c')
      {
         memset(s, 0, FilStrSiz * WrdSiz);

         if(InpMsh->typ & Asc)
            safe_fgets(s, WrdSiz * FilStrSiz, InpMsh->hdl, InpMsh->err);
         else
#ifdef WITH_GMF_AIO
            read(InpMsh->FilDes, s, WrdSiz * FilStrSiz);
#else
            safe_fread(s, WrdSiz, FilStrSiz, InpMsh->hdl, InpMsh->err);
#endif
         if(OutMsh->typ & Asc)
            fprintf(OutMsh->hdl, "%s ", s);
         else
#ifdef WITH_GMF_AIO
            write(OutMsh->FilDes, s, WrdSiz * FilStrSiz);
#else
            fwrite(s, WrdSiz, FilStrSiz, OutMsh->hdl);
#endif
      }
   }

   if(OutMsh->typ & Asc)
      fprintf(OutMsh->hdl, "\n");

   return(1);
}

#endif


// [Bruno] Made asynchronous I/O optional
#ifndef WITHOUT_AIO

/*----------------------------------------------------------------------------*/
/* Bufferized asynchronous reading of all keyword's lines                     */
/*----------------------------------------------------------------------------*/

int NAMF77(GmfGetBlock, gmfgetblock)(  TYPF77(int64_t) MshIdx,
                                       TYPF77(int)     KwdCod,
                                       TYPF77(int64_t) BegIdx,
                                       TYPF77(int64_t) EndIdx,
                                       TYPF77(int)     MapTyp,
                                       void           *MapTab,
                                       void           *prc, ... )
{
   char        *UsrDat[ GmfMaxTyp ], *UsrBas[ GmfMaxTyp ], *FilPos, *EndUsrDat;
   char        *FilBuf = NULL, *FrtBuf = NULL, *BckBuf = NULL, *BegUsrDat;
   char        *StrTab[5] = { "", "%f", "%lf", "%d", INT64_T_FMT };
   char        **BegTab, **EndTab;
   int         i, j, k, *FilPtrI32, *UsrPtrI32, FilTyp[ GmfMaxTyp ];
   int         UsrTyp[ GmfMaxTyp ], TypSiz[5] = {0,4,8,4,8};
   int         *IntMapTab = NULL, err, TotSiz = 0, IniFlg = 1, mod = GmfArgLst;
   int         *TypTab, *SizTab, typ, VecCnt, ArgCnt = 0;
   float       *FilPtrR32, *UsrPtrR32;
   double      *FilPtrR64, *UsrPtrR64;
   int64_t     BlkNmbLin, *FilPtrI64, *UsrPtrI64, BlkBegIdx, BlkEndIdx = 0;
   int64_t     *LngMapTab = NULL, OldIdx = 0, UsrNmbLin, VecLen;
   size_t      FilBegIdx = VALF77(BegIdx), FilEndIdx = VALF77(EndIdx);
   void        (*UsrPrc)(int64_t, int64_t, void *) = NULL;
   size_t      UsrLen[ GmfMaxTyp ], ret, LinSiz, b, NmbBlk;
   va_list     VarArg;
   GmfMshSct   *msh = (GmfMshSct *) VALF77(MshIdx);
   KwdSct      *kwd = &msh->KwdTab[ VALF77(KwdCod) ];
   struct      aiocb aio;
#ifdef F77API
   int         NmbArg = 0;
   void        *ArgTab[ MaxArg ];
#else
   char        *UsrArg = NULL;
#endif

   // Save the current stack environment for longjmp
   if( (err = setjmp(msh->err)) != 0)
   {
#ifdef GMFDEBUG
      printf("libMeshb : mesh %p : error %d\n", msh, err);
#endif
      if(BckBuf)
         free(BckBuf);

      if(FrtBuf)
         free(FrtBuf);

      return(0);
   }

   // Check mesh and keyword
   if( (VALF77(KwdCod) < 1) || (VALF77(KwdCod) > GmfMaxKwd) || !kwd->NmbLin )
      return(0);

   // Make sure it's not a simple information keyword
   if( (kwd->typ != RegKwd) && (kwd->typ != SolKwd) )
      return(0);

   // Check user's bounds
   if( (FilBegIdx < 1) || (FilBegIdx > FilEndIdx) || (FilEndIdx > (size_t)kwd->NmbLin) )
      return(0);

   // Compute the number of lines to be read
   UsrNmbLin = FilEndIdx - FilBegIdx + 1;

   // Get the renumbering map if any
   if(VALF77(MapTyp) == GmfInt)
      IntMapTab = (int *)MapTab;
   else if(VALF77(MapTyp) == GmfLong)
      LngMapTab = (int64_t *)MapTab;

   // Start decoding the arguments
   va_start(VarArg, prc);
   LinSiz = 0;

   // Get the user's preprocessing procedure and argument adresses, if any
#ifdef F77API
   if(prc)
   {
      UsrPrc = (void (*)(int64_t, int64_t, void *))prc;
      NmbArg = *(va_arg(VarArg, int *));

      for(i=0;i<NmbArg;i++)
         ArgTab[i] = va_arg(VarArg, void *);
   }
#else
   if(prc)
   {
      UsrPrc = (void (*)(int64_t, int64_t, void *))prc;
      UsrArg = va_arg(VarArg, void *);
   }
#endif

   if( (kwd->typ != RegKwd) && (kwd->typ != SolKwd) )
      return(0);

   // Read the first data type to select between list and table mode
   typ = VALF77(va_arg(VarArg, TYPF77(int)));

   // If the table mode is selected, read the four additional tables
   // containing the arguments: type, vector size, begin and end pointers
   if(typ == GmfArgTab)
   {
      mod = GmfArgTab;
      TypTab = va_arg(VarArg, int *);
      SizTab = va_arg(VarArg, int *);
      BegTab = va_arg(VarArg, char **);
      EndTab = va_arg(VarArg, char **);
   }

   // Read the arguments until to total size reaches the keyword's size
   while(TotSiz < kwd->SolSiz)
   {
      // In list mode all arguments are read from the variable argument buffer
      if(mod == GmfArgLst)
      {
         // Do not read the type argument for the first iteration because
         // it was read befeore the loop begins to get the argument mode
         if(IniFlg)
            IniFlg = 0;
         else
            typ = VALF77(va_arg(VarArg, TYPF77(int)));

         // In case the type is a vector. get its size and change the type
         // for the corresponding scalar type
         if(typ >= GmfFloatVec && typ <= GmfLongVec)
         {
            typ -= 4;
            VecCnt = VALF77(va_arg(VarArg, TYPF77(int)));
         }
         else
            VecCnt = 1;

         BegUsrDat = va_arg(VarArg, char *);
         EndUsrDat = va_arg(VarArg, char *);
      }
      else
      {
         // Do exactly the same as above but the arguments are read from
         // the tables instead of VarArgs
         typ = TypTab[ ArgCnt ];

         if(typ >= GmfFloatVec && typ <= GmfLongVec)
         {
            typ -= 4;
            VecCnt = SizTab[ ArgCnt ];
         }
         else
            VecCnt = 1;

         BegUsrDat = (char *)BegTab[ ArgCnt ];
         EndUsrDat = (char *)EndTab[ ArgCnt ];
         ArgCnt++;
      }

      if(UsrNmbLin > 1)
         VecLen = (size_t)(EndUsrDat - BegUsrDat) / (UsrNmbLin - 1);
      else
         VecLen = 0;

      // Compute the consecutive begin / end adresses for vector data types
      for(i=0;i<VecCnt;i++)
      {
         UsrTyp[ TotSiz ]  = typ;
         UsrBas[ TotSiz ]  = BegUsrDat + i * TypSiz[ typ ];
         UsrDat[ TotSiz ]  = UsrBas[ TotSiz ];
         UsrLen[ TotSiz ]  = VecLen;
         TotSiz++;
      }
   }

   // Get the file's data type
   for(i=0;i<kwd->SolSiz;i++)
   {
      if(kwd->fmt[i] == 'r')
         if(msh->FltSiz == 32)
            FilTyp[i] = GmfFloat;
         else
            FilTyp[i] = GmfDouble;
      else
         if(msh->ver <= 3)
            FilTyp[i] = GmfInt;
         else
            FilTyp[i] = GmfLong;

      // Compute the file stride
      LinSiz += TypSiz[ FilTyp[i] ];
   }

   va_end(VarArg);

   // Move file pointer to the keyword data
   SetFilPos(msh, kwd->pos);

   // Read the whole kwd data
   if(msh->typ & Asc)
   {
      OldIdx = 1;

      for(i=1;i<=FilEndIdx;i++)
      {
         for(j=0;j<kwd->SolSiz;j++)
         {
            // Reorder HO nodes on the fly
            if(kwd->OrdTab && (j != kwd->SolSiz-1))
               k = kwd->OrdTab[j];
            else
               k = j;

            // Move to the next user's data line only when the desired
            // begining position in the ascii file has been reached since
            // we cannot move directly to an arbitrary position
            if(IntMapTab)
               UsrDat[j] = UsrBas[k] + (IntMapTab[ OldIdx ] - 1) * UsrLen[k];
            else if(LngMapTab)
               UsrDat[j] = UsrBas[k] + (LngMapTab[ OldIdx ] - 1) * UsrLen[k];
            else
               UsrDat[j] = UsrBas[k] + (OldIdx - 1) * UsrLen[k];

            safe_fscanf(msh->hdl, StrTab[ UsrTyp[j] ], UsrDat[j], msh->err);
         }

         if(i >= FilBegIdx)
            OldIdx++;

         // Call the user's preprocessing procedure
         if(UsrPrc)
#ifdef F77API
            CalF77Prc(1, kwd->NmbLin, UsrPrc, NmbArg, ArgTab);
#else
            UsrPrc(1, kwd->NmbLin, UsrArg);
#endif
      }
   }
   else
   {
      // Allocate both front and back buffers
      if(!(BckBuf = malloc(BufSiz * LinSiz)))
         return(0);

      if(!(FrtBuf = malloc(BufSiz * LinSiz)))
         return(0);

      // Setup the ansynchonous parameters
      memset(&aio, 0, sizeof(struct aiocb));
      FilBuf = BckBuf;
      aio.aio_buf = BckBuf;
#ifdef WITH_GMF_AIO
      aio.aio_fildes = msh->FilDes;
#else
      aio.aio_fildes = msh->hdl;
#endif
      aio.aio_offset = (off_t)(GetFilPos(msh) + (FilBegIdx-1) * LinSiz);

      NmbBlk = UsrNmbLin / BufSiz;

      // Loop over N+1 blocks
      for(b=0;b<=NmbBlk+1;b++)
      {
         // Wait for the previous block read to complete except
         // for the first loop interation
         if(b)
         {
            while(my_aio_error(&aio) == EINPROGRESS);

            err = my_aio_error(&aio);
            ret = my_aio_return(&aio);

            if (err != 0) {
              printf (" Error at aio_error() : %s\n", strerror (err));
              exit(1);
            }

            if (ret != aio.aio_nbytes) {
              printf(" Error at aio_return()\n");
              exit(1);
            }

            // Increment the reading position
            aio.aio_offset += (off_t)aio.aio_nbytes;

            // and swap the buffers
            if(aio.aio_buf == BckBuf)
            {
               aio.aio_buf = FrtBuf;
               FilBuf = BckBuf;
            }
            else
            {
               aio.aio_buf = BckBuf;
               FilBuf = FrtBuf;
            }
         }
 
         // Read a chunk of data except for the last loop interarion
         if(b <= NmbBlk)
         {
            // The last block is shorter than the others
            if(b == NmbBlk)
               BlkNmbLin = UsrNmbLin - b * BufSiz;
            else
               BlkNmbLin = BufSiz;

            aio.aio_nbytes = BlkNmbLin * LinSiz;

            if(my_aio_read(&aio) == -1)
            {
               printf("block      = %zd / %zd\n", b+1, NmbBlk+1);
               printf("size       = "INT64_T_FMT" lines\n", BlkNmbLin);
#ifdef WITH_GMF_AIO
               printf("aio_fildes = %d\n",aio.aio_fildes);
#else
               printf("aio_fildes = %p\n",aio.aio_fildes);
#endif
               printf("aio_buf    = %p\n",aio.aio_buf);
               printf("aio_offset = " INT64_T_FMT "\n",(int64_t)aio.aio_offset);
               printf("aio_nbytes = " INT64_T_FMT "\n",(int64_t)aio.aio_nbytes);
               printf("errno      = %d\n",errno);
               exit(1);
            }
         }

         // Then decode the block and store it in the user's data structure
         // except for the first loop interation
         if(b)
         {
            // The last block is shorter than the others
            if(b-1 == NmbBlk)
               BlkNmbLin = UsrNmbLin - (b-1) * BufSiz;
            else
               BlkNmbLin = BufSiz;

            BlkBegIdx = BlkEndIdx+1;
            BlkEndIdx += BlkNmbLin;
            FilPos = FilBuf;

            for(i=0;i<BlkNmbLin;i++)
            {
               OldIdx++;

               for(j=0;j<kwd->SolSiz;j++)
               {
                  if(msh->cod != 1)
                     SwpWrd(FilPos, TypSiz[ FilTyp[j] ]);

                  // Reorder HO nodes on the fly
                  if(kwd->OrdTab && (j != kwd->SolSiz-1))
                     k = kwd->OrdTab[j];
                  else
                     k = j;

                  if(IntMapTab)
                     UsrDat[j] = UsrBas[k] + (IntMapTab[ OldIdx ] - 1) * UsrLen[k];
                  else if(LngMapTab)
                     UsrDat[j] = UsrBas[k] + (LngMapTab[ OldIdx ] - 1) * UsrLen[k];
                  else
                     UsrDat[j] = UsrBas[k] + (OldIdx - 1) * UsrLen[k];

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
                        UsrPtrI64 = (int64_t *)UsrDat[j];
                        *UsrPtrI64 = (int64_t)*FilPtrI32;
                     }
                  }
                  else if(FilTyp[j] == GmfLong)
                  {
                     FilPtrI64 = (int64_t *)FilPos;

                     if(UsrTyp[j] == GmfLong)
                     {
                        UsrPtrI64 = (int64_t *)UsrDat[j];
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

                  FilPos += TypSiz[ FilTyp[j] ];
               }
            }

            // Call the user's preprocessing procedure
            if(UsrPrc)
#ifdef F77API
               CalF77Prc(BlkBegIdx, BlkEndIdx, UsrPrc, NmbArg, ArgTab);
#else
               UsrPrc(BlkBegIdx, BlkEndIdx, UsrArg);
#endif
         }
      }

      free(BckBuf);
      free(FrtBuf);
   }

   return(1);
}


/*----------------------------------------------------------------------------*/
/* Bufferized writing of all keyword's lines                                  */
/*----------------------------------------------------------------------------*/

int NAMF77(GmfSetBlock, gmfsetblock)(  TYPF77(int64_t) MshIdx,
                                       TYPF77(int)     KwdCod,
                                       TYPF77(int64_t) BegIdx,
                                       TYPF77(int64_t) EndIdx,
                                       TYPF77(int)     MapTyp,
                                       void           *MapTab,
                                       void           *prc, ... )
{
   char        *UsrDat[ GmfMaxTyp ], *UsrBas[ GmfMaxTyp ];
   char        *StrTab[5] = { "", "%.9g", "%.17g", "%d", "%lld" }, *FilPos;
   char        *FilBuf = NULL, *FrtBuf = NULL, *BckBuf = NULL;
   char        **BegTab, **EndTab, *BegUsrDat, *EndUsrDat;
   int         i, j, *FilPtrI32, *UsrPtrI32, FilTyp[ GmfMaxTyp ];
   int         UsrTyp[ GmfMaxTyp ], TypSiz[5] = {0,4,8,4,8};
   int         err, *IntMapTab = NULL, typ, mod = GmfArgLst;
   int         *TypTab, *SizTab, IniFlg = 1, TotSiz = 0, VecCnt, ArgCnt = 0;
   float       *FilPtrR32, *UsrPtrR32;
   double      *FilPtrR64, *UsrPtrR64;
   int64_t     UsrNmbLin, BlkNmbLin = 0, BlkBegIdx, BlkEndIdx = 0;
   int64_t     *FilPtrI64, *UsrPtrI64, *LngMapTab = NULL, OldIdx = 0;
   size_t      FilBegIdx = VALF77(BegIdx), FilEndIdx = VALF77(EndIdx);
   void        (*UsrPrc)(int64_t, int64_t, void *) = NULL;
   size_t      UsrLen[ GmfMaxTyp ], ret, LinSiz, VecLen, s, b, NmbBlk;
   va_list     VarArg;
   GmfMshSct   *msh = (GmfMshSct *) VALF77(MshIdx);
   KwdSct      *kwd = &msh->KwdTab[ VALF77(KwdCod) ];
   struct      aiocb aio;
#ifdef F77API
   int         NmbArg = 0;
   void        *ArgTab[ MaxArg ];
#else
   char        *UsrArg = NULL;
#endif

   // Save the current stack environment for longjmp
   if( (err = setjmp(msh->err)) != 0)
   {
#ifdef GMFDEBUG
      printf("libMeshb : mesh %p : error %d\n", msh, err);
#endif
      if(FilBuf)
         free(FilBuf);

      return(0);
   }

   // Check mesh and keyword
   if( (VALF77(KwdCod) < 1) || (VALF77(KwdCod) > GmfMaxKwd) || !kwd->NmbLin )
      return(0);

   // Make sure it's not a simple information keyword
   if( (kwd->typ != RegKwd) && (kwd->typ != SolKwd) )
      return(0);

   // Temporarily overwright the given begin and end values
   // as arbitrary position block write is not yet implemented
   FilBegIdx = 1;
   FilEndIdx = kwd->NmbLin;

   // Check user's bounds
   if( (FilBegIdx < 1) || (FilBegIdx > FilEndIdx) || (FilEndIdx > (size_t)kwd->NmbLin) )
      return(0);

   // Compute the number of lines to be written
   UsrNmbLin = FilEndIdx - FilBegIdx + 1;

   // Get the renumbering map if any
   if(VALF77(MapTyp) == GmfInt)
      IntMapTab = (int *)MapTab;
   else if(VALF77(MapTyp) == GmfLong)
      LngMapTab = (int64_t *)MapTab;

   // Start decoding the arguments
   va_start(VarArg, prc);
   LinSiz = 0;

   // Get the user's postprocessing procedure and argument adresses, if any
#ifdef F77API
   if(prc)
   {
      UsrPrc = (void (*)(int64_t, int64_t, void *))prc;
      NmbArg = *(va_arg(VarArg, int *));

      for(i=0;i<NmbArg;i++)
         ArgTab[i] = va_arg(VarArg, void *);
   }
#else
   if(prc)
   {
      UsrPrc = (void (*)(int64_t, int64_t, void *))prc;
      UsrArg = va_arg(VarArg, void *);
   }
#endif

   if( (kwd->typ != RegKwd) && (kwd->typ != SolKwd) )
      return(0);

   // Read the first data type to select between list and table mode
   typ = VALF77(va_arg(VarArg, TYPF77(int)));

   // If the table mode is selected, read the four additional tables
   // containing the arguments: type, vector size, begin and end pointers
   if(typ == GmfArgTab)
   {
      mod = GmfArgTab;
      TypTab = va_arg(VarArg, int *);
      SizTab = va_arg(VarArg, int *);
      BegTab = va_arg(VarArg, char **);
      EndTab = va_arg(VarArg, char **);
   }

   // Read the arguments until to total size reaches the keyword's size
   while(TotSiz < kwd->SolSiz)
   {
      // In list mode all arguments are read from the variable argument buffer
      if(mod == GmfArgLst)
      {
         // Do not read the type argument for the first iteration because
         // it was read befeore the loop begins to get the argument mode
         if(IniFlg)
            IniFlg = 0;
         else
            typ = VALF77(va_arg(VarArg, TYPF77(int)));

         // In case the type is a vector. get its size and change the type
         // for the corresponding scalar type
         if(typ >= GmfFloatVec && typ <= GmfLongVec)
         {
            typ -= 4;
            VecCnt = VALF77(va_arg(VarArg, TYPF77(int)));
         }
         else
            VecCnt = 1;

         BegUsrDat = va_arg(VarArg, char *);
         EndUsrDat = va_arg(VarArg, char *);
      }
      else
      {
         // Do exactly the same as above but the arguments are read from
         // the tables instead of VarArgs
         typ = TypTab[ ArgCnt ];

         if(typ >= GmfFloatVec && typ <= GmfLongVec)
         {
            typ -= 4;
            VecCnt = SizTab[ ArgCnt ];
         }
         else
            VecCnt = 1;

         BegUsrDat = (char *)BegTab[ ArgCnt ];
         EndUsrDat = (char *)EndTab[ ArgCnt ];
         ArgCnt++;
      }

      if(UsrNmbLin > 1)
         VecLen = (size_t)(EndUsrDat - BegUsrDat) / (UsrNmbLin - 1);
      else
         VecLen = 0;

      // Compute the consecutive begin / end adresses for vector data types
      for(i=0;i<VecCnt;i++)
      {
         UsrTyp[ TotSiz ]  = typ;
         UsrBas[ TotSiz ]  = BegUsrDat + i * TypSiz[ typ ];
         UsrDat[ TotSiz ]  = UsrBas[ TotSiz ];
         UsrLen[ TotSiz ]  = VecLen;
         TotSiz++;
      }
   }

   // Get the file's data type
   for(i=0;i<kwd->SolSiz;i++)
   {
      if(kwd->fmt[i] == 'r')
         if(msh->FltSiz == 32)
            FilTyp[i] = GmfFloat;
         else
            FilTyp[i] = GmfDouble;
      else
         if(msh->ver <= 3)
            FilTyp[i] = GmfInt;
         else
            FilTyp[i] = GmfLong;

      // Compute the file stride
      LinSiz += TypSiz[ FilTyp[i] ];
   }

   va_end(VarArg);

   // Write the whole kwd data
   if(msh->typ & Asc)
   {
      if(UsrPrc)
#ifdef F77API
         CalF77Prc(1, kwd->NmbLin, UsrPrc, NmbArg, ArgTab);
#else
         UsrPrc(1, kwd->NmbLin, UsrArg);
#endif

      for(s=FilBegIdx; s<=FilEndIdx; s++)
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
               UsrPtrI64 = (int64_t *)UsrDat[j];
               fprintf(msh->hdl, StrTab[ UsrTyp[j] ], *UsrPtrI64);
            }

            if(j < kwd->SolSiz -1)
               fprintf(msh->hdl, " ");
            else
               fprintf(msh->hdl, "\n");

            //UsrDat[j] += UsrLen[j];
            if(IntMapTab)
               UsrDat[j] = UsrBas[j] + IntMapTab[s] * UsrLen[j];
            else if(LngMapTab)
               UsrDat[j] = UsrBas[j] + LngMapTab[s] * UsrLen[j];
            else
               UsrDat[j] = UsrBas[j] + s * UsrLen[j];
         }
   }
   else
   {
      // Allocate the front and back buffers
      if(!(BckBuf = malloc(BufSiz * LinSiz)))
         return(0);

      if(!(FrtBuf = malloc(BufSiz * LinSiz)))
         return(0);

      // Setup the asynchronous parameters
      memset(&aio, 0, sizeof(struct aiocb));
      FilBuf = BckBuf;
#ifdef WITH_GMF_AIO
      aio.aio_fildes = msh->FilDes;
#else
      aio.aio_fildes = msh->hdl;
#endif
      aio.aio_offset = (off_t)GetFilPos(msh);

      NmbBlk = UsrNmbLin / BufSiz;

      // Loop over N+1 blocks
      for(b=0;b<=NmbBlk+1;b++)
      {
         // Launch an asynchronous block write
         // except for the first loop iteration
         if(b)
         {
            aio.aio_nbytes = BlkNmbLin * LinSiz;
            
            if(my_aio_write(&aio) == -1)
            {
#ifdef WITH_GMF_AIO
               printf("aio_fildes = %d\n",aio.aio_fildes);
#else
               printf("aio_fildes = %p\n",aio.aio_fildes);
#endif
               printf("aio_buf    = %p\n",aio.aio_buf);
               printf("aio_offset = " INT64_T_FMT "\n",(int64_t)aio.aio_offset);
               printf("aio_nbytes = " INT64_T_FMT "\n",(int64_t)aio.aio_nbytes);
               printf("errno      = %d\n",errno);
               exit(1);
            }
         }

         // Parse the block data except at the last loop iteration
         if(b<=NmbBlk)
         {
            // The last block is shorter
            if(b == NmbBlk)
               BlkNmbLin = UsrNmbLin - b * BufSiz;
            else
               BlkNmbLin = BufSiz;

            FilPos = FilBuf;
            BlkBegIdx = BlkEndIdx+1;
            BlkEndIdx += BlkNmbLin;

            // Call user's preprocessing first
            if(UsrPrc)
#ifdef F77API
               CalF77Prc(BlkBegIdx, BlkEndIdx, UsrPrc, NmbArg, ArgTab);
#else
               UsrPrc(BlkBegIdx, BlkEndIdx, UsrArg);
#endif

            // Then copy it's data to the file buffer
            for(i=0;i<BlkNmbLin;i++)
            {
               OldIdx++;

               for(j=0;j<kwd->SolSiz;j++)
               {
                  if(IntMapTab)
                     UsrDat[j] = UsrBas[j] + (IntMapTab[ OldIdx ] - 1) * UsrLen[j];
                  else if(LngMapTab)
                     UsrDat[j] = UsrBas[j] + (LngMapTab[ OldIdx ] - 1) * UsrLen[j];
                  else
                     UsrDat[j] = UsrBas[j] + (OldIdx - 1) * UsrLen[j];

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
                        UsrPtrI64 = (int64_t *)UsrDat[j];
                        *FilPtrI32 = (int)*UsrPtrI64;
                     }
                  }
                  else if(FilTyp[j] == GmfLong)
                  {
                     FilPtrI64 = (int64_t *)FilPos;

                     if(UsrTyp[j] == GmfLong)
                     {
                        UsrPtrI64 = (int64_t *)UsrDat[j];
                        *FilPtrI64 = *UsrPtrI64;
                     }
                     else
                     {
                        UsrPtrI32 = (int *)UsrDat[j];
                        *FilPtrI64 = (int64_t)*UsrPtrI32;
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

                  FilPos += TypSiz[ FilTyp[j] ];
               }
            }
         }

         // Wait for write completion execpt at the first loop iteration
         if(b)
         {
            while(my_aio_error(&aio) == EINPROGRESS);

            err = my_aio_error(&aio);
            ret = my_aio_return(&aio);

            if (err != 0) {
              printf (" Error at aio_error() : %s\n", strerror (err));
              exit(1);
            }

            if (ret != aio.aio_nbytes) {
              printf(" Error at aio_return()\n");
              exit(1);
            }

            // Move the write position
            aio.aio_offset += (off_t)aio.aio_nbytes;
         }

         // Swap the buffers
         if(FilBuf == BckBuf)
         {
            aio.aio_buf = BckBuf;
            FilBuf = FrtBuf;
         }
         else
         {
            aio.aio_buf = FrtBuf;
            FilBuf = BckBuf;
         }
      }

      SetFilPos(msh, aio.aio_offset);
      free(BckBuf);
      free(FrtBuf);
   }

   return(1);
}


/*----------------------------------------------------------------------------*/
/* Map two HO element's nodes numbering orders                                */
/*----------------------------------------------------------------------------*/

int GmfSetHONodesOrdering(int64_t MshIdx, int KwdCod, int *BasTab, int *OrdTab)
{
   int i, j, k, flg, NmbNod, NmbCrd;
   GmfMshSct   *msh = (GmfMshSct *)MshIdx;
   KwdSct      *kwd;
   
   // printf("\n\tGmfSetHONodesOrdering 0\n");

   if( (KwdCod < 1) || (KwdCod > GmfMaxKwd) )
      return(0);

   kwd = &msh->KwdTab[ KwdCod ];

   // Find the Bezier indices dimension according to the element's kind
   switch(KwdCod)
   {
      case GmfEdges   :          NmbNod =  2; NmbCrd = 1; break;
      case GmfEdgesP2 :          NmbNod =  3; NmbCrd = 1; break;
      case GmfEdgesP3 :          NmbNod =  4; NmbCrd = 1; break;
      case GmfEdgesP4 :          NmbNod =  5; NmbCrd = 1; break;
      case GmfTriangles   :      NmbNod =  3; NmbCrd = 3; break;
      case GmfTrianglesP2 :      NmbNod =  6; NmbCrd = 3; break;
      case GmfTrianglesP3 :      NmbNod = 10; NmbCrd = 3; break;
      case GmfTrianglesP4 :      NmbNod = 15; NmbCrd = 3; break;
      case GmfQuadrilaterals   : NmbNod =  4; NmbCrd = 2; break;
      case GmfQuadrilateralsQ2 : NmbNod =  9; NmbCrd = 2; break;
      case GmfQuadrilateralsQ3 : NmbNod = 16; NmbCrd = 2; break;
      case GmfQuadrilateralsQ4 : NmbNod = 25; NmbCrd = 2; break;
      case GmfTetrahedra   :     NmbNod =  4; NmbCrd = 4; break;
      case GmfTetrahedraP2 :     NmbNod = 10; NmbCrd = 4; break;
      case GmfTetrahedraP3 :     NmbNod = 20; NmbCrd = 4; break;
      case GmfTetrahedraP4 :     NmbNod = 35; NmbCrd = 4; break;
      case GmfPyramids   :       NmbNod =  5; NmbCrd = 3; break;
      case GmfPyramidsP2 :       NmbNod = 14; NmbCrd = 3; break;
      case GmfPyramidsP3 :       NmbNod = 30; NmbCrd = 3; break;
      case GmfPyramidsP4 :       NmbNod = 55; NmbCrd = 3; break;
      case GmfPrisms   :         NmbNod =  6; NmbCrd = 4; break;
      case GmfPrismsP2 :         NmbNod = 18; NmbCrd = 4; break;
      case GmfPrismsP3 :         NmbNod = 40; NmbCrd = 4; break;
      case GmfPrismsP4 :         NmbNod = 75; NmbCrd = 4; break;
      case GmfHexahedra   :      NmbNod =  8; NmbCrd = 3; break;
      case GmfHexahedraQ2 :      NmbNod = 27; NmbCrd = 3; break;
      case GmfHexahedraQ3 :      NmbNod = 64; NmbCrd = 3; break;
      case GmfHexahedraQ4 :      NmbNod =125; NmbCrd = 3; break;
      default : return(0);
   }

   // Free and rebuild the mapping table if there were already one
   if(kwd->OrdTab)
      free(kwd->OrdTab);

   if(!(kwd->OrdTab = malloc(NmbNod * sizeof(int))))
      return(0);

   // Find the corresponding Bezier coordinates from the source table
   for(i=0;i<NmbNod;i++)
   {
      for(j=0;j<NmbNod;j++)
      {
         flg = 1;

         for(k=0;k<NmbCrd;k++)
            if(BasTab[ i * NmbCrd + k ] != OrdTab[ j * NmbCrd + k ])
            {
               flg = 0;
               break;
            }

         if(flg)
            kwd->OrdTab[j] = i;
      }
   }

   // Check the ordering consistency
   for(i=0;i<NmbNod;i++)
   {
      flg = 0;

      for(j=0;j<NmbNod;j++)
         if(kwd->OrdTab[j] == i)
         {
            flg = 1;
            break;
         }

      if(!flg)
      {
         for(j=0;j<NmbNod;j++)
            kwd->OrdTab[j] = j;

         return(0);
      }
   }

   return(1);
}

#endif


#ifndef F77API


/*----------------------------------------------------------------------------*/
/* Read an EGADS binary CAD and return the byte flow and its exact byte size  */
/*----------------------------------------------------------------------------*/

char *GmfReadByteFlow(int64_t MshIdx, int *NmbByt)
{
   int         cod, *WrdTab;
   size_t      i, NmbWrd;
   GmfMshSct   *msh = (GmfMshSct *)MshIdx;

   // Read and allocate the number of 4-byte words in the byteflow
   if(!(NmbWrd = GmfStatKwd(MshIdx, GmfByteFlow)))
      return(NULL);

   if(!(WrdTab = malloc(NmbWrd * WrdSiz)))
      return(NULL);

   // Disable the endianess conversion
   cod = msh->cod;
   msh->cod = 1;

   // Read the exact number of bytes in the byteflow
   GmfGotoKwd(MshIdx, GmfByteFlow);
   GmfGetLin(MshIdx, GmfByteFlow, NmbByt);

   // Read the byteflow as 4-byte blocks
   for(i=0;i<NmbWrd;i++)
      GmfGetLin(MshIdx, GmfByteFlow, &WrdTab[i]);

   // Enable endianess convertion
   msh->cod = cod;

   return((char *)WrdTab);
}


/*----------------------------------------------------------------------------*/
/* Write an EGADS binary CAD as an integer table whose first entry is the size*/
/*----------------------------------------------------------------------------*/

int GmfWriteByteFlow(int64_t MshIdx, char *BytTab, int NmbByt)
{
   int i, PadWrd = 0, *WrdTab = (int *)BytTab, NmbWrd = NmbByt / WrdSiz;

   // Add an extra padding word at the end if needed
   if(NmbByt > NmbWrd * 4)
      PadWrd = 1;

   // Create the keyword with the number of words, not bytes
   if(!GmfSetKwd(MshIdx, GmfByteFlow, NmbWrd + PadWrd))
      return(0);

   // Reacord the exact number of bytes
   GmfSetLin(MshIdx, GmfByteFlow, NmbByt);

   // Write the byteflow as 4-byte words, missing up to 3 endding bytes
   for(i=0;i<NmbWrd;i++)
      GmfSetLin(MshIdx, GmfByteFlow, WrdTab[i]);

   // Write the extra 1,2 or 3 ending bytes
   if(PadWrd)
   {
      PadWrd = 0;

      // Copy the last bytes in an integer
      for(i=0; i<NmbByt - NmbWrd * 4; i++)
         PadWrd |= BytTab[ NmbWrd * 4 + i ] << (i*8);

      // And write it as the last line
      GmfSetLin(MshIdx, GmfByteFlow, PadWrd);
   }

   return(1);
}


/*----------------------------------------------------------------------------*/
/* Override the floating point precision deduced form the file version        */
/* with the one read from the GmfFloatingPointPrecision field                 */
/*----------------------------------------------------------------------------*/

int GmfGetFloatPrecision(int64_t MshIdx)
{
   int FltSiz;
   GmfMshSct *msh = (GmfMshSct *)MshIdx;

   if(GmfStatKwd(MshIdx, GmfFloatingPointPrecision))
   {
      GmfGotoKwd(MshIdx, GmfFloatingPointPrecision);
      GmfGetLin(MshIdx, GmfFloatingPointPrecision, &FltSiz);

      if(FltSiz == 32 || FltSiz == 64)
         msh->FltSiz = FltSiz;
   }

   return(msh->FltSiz);
}


/*----------------------------------------------------------------------------*/
/* Set the floating point precision arbitrarily, regardless the file version  */
/*----------------------------------------------------------------------------*/

void GmfSetFloatPrecision(int64_t MshIdx , int FltSiz)
{
   GmfMshSct *msh = (GmfMshSct *)MshIdx;

   if(FltSiz != 32 && FltSiz != 64)
      return;

   msh->FltSiz = FltSiz;
   GmfSetKwd(MshIdx, GmfFloatingPointPrecision, 1);
   GmfSetLin(MshIdx, GmfFloatingPointPrecision, FltSiz);
}

#endif


/*----------------------------------------------------------------------------*/
/* Find every kw present in a meshfile                                        */
/*----------------------------------------------------------------------------*/

static int ScaKwdTab(GmfMshSct *msh)
{
   int      KwdCod, c;
   int64_t  NexPos, EndPos, LstPos;
   char     str[ GmfStrSiz ];

   if(msh->typ & Asc)
   {
      // Scan each string in the file until the end
      while(fscanf(msh->hdl, "%100s", str) != EOF)
      {
         // Fast test in order to reject quickly the numeric values
         if(isalpha(str[0]))
         {
            // Search which kwd code this string is associated with, then get its
            // header and save the curent position in file (just before the data)
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
      // Get file size
      EndPos = GetFilSiz(msh);
      LstPos = -1;

      // Jump through kwd positions in the file
      do
      {
         // Get the kwd code and the next kwd position
         ScaWrd(msh, ( char *)&KwdCod);
         NexPos = GetPos(msh);

         // Make sure the flow does not move beyond the file size
         if(NexPos > EndPos)
            longjmp(msh->err, -24);

         // And check that it does not move back
         if(NexPos && (NexPos <= LstPos))
            longjmp(msh->err, -30);

         LstPos = NexPos;

         // Check if this kwd belongs to this mesh version
         if( (KwdCod >= 1) && (KwdCod <= GmfMaxKwd) )
            ScaKwdHdr(msh, KwdCod);

         // Go to the next kwd
         if(NexPos && !(SetFilPos(msh, NexPos)))
            longjmp(msh->err, -25);

      }while(NexPos && (KwdCod != GmfEnd));
   }

   return(1);
}


/*----------------------------------------------------------------------------*/
/* Read and setup the keyword's header                                        */
/*----------------------------------------------------------------------------*/

static void ScaKwdHdr(GmfMshSct *msh, int KwdCod)
{
   int      i;
   KwdSct   *kwd = &msh->KwdTab[ KwdCod ];

   if(!strcmp("i", GmfKwdFmt[ KwdCod ][1]))
      if(msh->typ & Asc)
         safe_fscanf(msh->hdl, INT64_T_FMT, &kwd->NmbLin, msh->err);
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

   if(!strcmp("sr", GmfKwdFmt[ KwdCod ][2])
   || !strcmp("hr", GmfKwdFmt[ KwdCod ][2]) )
   {
      if(msh->typ & Asc)
      {
         safe_fscanf(msh->hdl, "%d", &kwd->NmbTyp, msh->err);

         for(i=0;i<kwd->NmbTyp;i++)
            safe_fscanf(msh->hdl, "%d", &kwd->TypTab[i], msh->err);

         // Scan two extra fields for HO solutions: deg and nmb Nodes
         if(!strcmp("hr", GmfKwdFmt[ KwdCod ][2]))
         {
            safe_fscanf(msh->hdl, "%d", &kwd->deg, msh->err);
            safe_fscanf(msh->hdl, "%d", &kwd->NmbNod, msh->err);
         }
         else
         {
            kwd->deg = 0;
            kwd->NmbNod = 1;
         }

      }
      else
      {
         ScaWrd(msh, (unsigned char *)&kwd->NmbTyp);

         for(i=0;i<kwd->NmbTyp;i++)
            ScaWrd(msh, (unsigned char *)&kwd->TypTab[i]);

         // Scan two extra fields for HO solutions: deg and nmb Nodes
         if(!strcmp("hr", GmfKwdFmt[ KwdCod ][2]))
         {
            ScaWrd(msh, (unsigned char *)&kwd->deg);
            ScaWrd(msh, (unsigned char *)&kwd->NmbNod);
         }
         else
         {
            kwd->deg = 0;
            kwd->NmbNod = 1;
         }
      }
   }

   ExpFmt(msh, KwdCod);
   kwd->pos = GetFilPos(msh);
}


/*----------------------------------------------------------------------------*/
/* Expand the compacted format and compute the line size                      */
/*----------------------------------------------------------------------------*/

static void ExpFmt(GmfMshSct *msh, int KwdCod)
{
   int         i, j, TmpSiz=0, IntWrd, FltWrd;
   char        chr;
   const char  *InpFmt = GmfKwdFmt[ KwdCod ][2];
   KwdSct      *kwd = &msh->KwdTab[ KwdCod ];

   // Set the kwd's type
   if(!strlen(GmfKwdFmt[ KwdCod ][1]))
      kwd->typ = InfKwd;
   else if( !strcmp(InpFmt, "sr") || !strcmp(InpFmt, "hr") )
      kwd->typ = SolKwd;
   else
      kwd->typ = RegKwd;

   // Get the solution-field's size
   if(kwd->typ == SolKwd)
      for(i=0;i<kwd->NmbTyp;i++)
         switch(kwd->TypTab[i])
         {
            case GmfSca    : TmpSiz += 1; break;
            case GmfVec    : TmpSiz += msh->dim; break;
            case GmfSymMat : TmpSiz += (msh->dim * (msh->dim+1)) / 2; break;
            case GmfMat    : TmpSiz += msh->dim * msh->dim; break;
         }

   // Scan each character from the format string
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
      else if((chr == 's')||(chr == 'h'))
      {
         chr = InpFmt[i++];

         for(j=0;j<TmpSiz;j++)
            kwd->fmt[ kwd->SolSiz++ ] = chr;
      }
      else
         kwd->fmt[ kwd->SolSiz++ ] = chr;
   }

   if(msh->FltSiz == 32)
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

   // HO solution: duplicate the format as many times as the number of nodes
   if( !strcmp(InpFmt, "hr") && (kwd->NmbNod > 1) )
   {
      for(i=1;i<=kwd->NmbNod;i++)
         for(j=0;j<kwd->SolSiz;j++)
            kwd->fmt[ i * kwd->SolSiz + j ] = kwd->fmt[j];

      kwd->SolSiz *= kwd->NmbNod;
      kwd->NmbWrd *= kwd->NmbNod;
   }
}


/*----------------------------------------------------------------------------*/
/* Read a four bytes word from a mesh file                                    */
/*----------------------------------------------------------------------------*/

static void ScaWrd(GmfMshSct *msh, void *ptr)
{
#ifdef WITH_GMF_AIO
   if(read(msh->FilDes, ptr, WrdSiz) != WrdSiz)
#else
   if(fread(ptr, WrdSiz, 1, msh->hdl) != 1)
#endif
      longjmp(msh->err, -26);

   if(msh->cod != 1)
      SwpWrd((char *)ptr, WrdSiz);
}


/*----------------------------------------------------------------------------*/
/* Read an eight bytes word from a mesh file                                  */
/*----------------------------------------------------------------------------*/

static void ScaDblWrd(GmfMshSct *msh, void *ptr)
{
#ifdef WITH_GMF_AIO
   if(read(msh->FilDes, ptr, WrdSiz * 2) != WrdSiz * 2)
#else
   if( fread(ptr, WrdSiz, 2, msh->hdl) != 2 )
#endif
      longjmp(msh->err, -27);

   if(msh->cod != 1)
      SwpWrd((char *)ptr, 2 * WrdSiz);
}


/*----------------------------------------------------------------------------*/
/* Read a 4 or 8 bytes position in mesh file                                  */
/*----------------------------------------------------------------------------*/

static int64_t GetPos(GmfMshSct *msh)
{
   int      IntVal;
   int64_t  pos;

   if(msh->ver >= 3)
      ScaDblWrd(msh, (unsigned char*)&pos);
   else
   {
      ScaWrd(msh, (unsigned char*)&IntVal);
      pos = (int64_t)IntVal;
   }

   return(pos);
}


/*----------------------------------------------------------------------------*/
/* Write a four bytes word to a mesh file                                     */
/*----------------------------------------------------------------------------*/

static void RecWrd(GmfMshSct *msh, const void *wrd)
{
   // [Bruno] added error control
#ifdef WITH_GMF_AIO
   if(write(msh->FilDes, wrd, WrdSiz) != WrdSiz)
#else
   if(fwrite(wrd, WrdSiz, 1, msh->hdl) != 1)
#endif
      longjmp(msh->err,-28);
}


/*----------------------------------------------------------------------------*/
/* Write an eight bytes word to a mesh file                                   */
/*----------------------------------------------------------------------------*/

static void RecDblWrd(GmfMshSct *msh, const void *wrd)
{
   // [Bruno] added error control
#ifdef WITH_GMF_AIO
   if(write(msh->FilDes, wrd, WrdSiz * 2) != WrdSiz*2)
#else
   if(fwrite(wrd, WrdSiz, 2, msh->hdl) != 2)
#endif
      longjmp(msh->err,-29);
}


/*----------------------------------------------------------------------------*/
/* Write a block of four bytes word to a mesh file                            */
/*----------------------------------------------------------------------------*/

static void RecBlk(GmfMshSct *msh, const void *blk, int siz)
{
   // Copy this line-block into the main mesh buffer
   if(siz)
   {
      memcpy(&msh->blk[ msh->pos ], blk, (size_t)(siz * WrdSiz));
      msh->pos += siz * WrdSiz;
   }

   // When the buffer is full or this procedure is APIF77ed with a 0 size,
   // flush the cache on disk

   if( (msh->pos > BufSiz) || (!siz && msh->pos) )
   {
#ifdef GMF_WINDOWS
      /*
       *   [Bruno] TODO: check that msh->pos is smaller
       * than 4G (fits in 32 bits).
       *   Note: for now, when trying to write more than 4Gb, it will
       * trigger an error (longjmp).
       *   As far as I understand:
       *   Given that this function just flushes the cache, and given that
       * the cache size is 10000 words, this is much much smaller than 4Gb
       * so there is probably no problem.
       */
#ifdef WITH_GMF_AIO
      if(write(msh->FilDes, msh->blk, (int)msh->pos) != (ssize_t)msh->pos)
#else      
      if(fwrite(msh->blk, 1, (size_t)msh->pos, msh->hdl) != msh->pos)
#endif      
         longjmp(msh->err, -30);
#else      
#ifdef WITH_GMF_AIO
      if(write(msh->FilDes, msh->blk, msh->pos) != (ssize_t)msh->pos)
#else      
      if(fwrite(msh->blk, 1, msh->pos, msh->hdl) != msh->pos)
#endif      
         longjmp(msh->err, -31);
#endif      
      msh->pos = 0;
   }
}


/*----------------------------------------------------------------------------*/
/* Write a 4 or 8 bytes position in a mesh file                               */
/*----------------------------------------------------------------------------*/

static void SetPos(GmfMshSct *msh, int64_t pos)
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


/*----------------------------------------------------------------------------*/
/* Endianness conversion                                                      */
/*----------------------------------------------------------------------------*/

static void SwpWrd(char *wrd, int siz)
{
   char  swp;
   int   i;

   for(i=0;i<siz/2;i++)
   {
      swp = wrd[ siz-i-1 ];
      wrd[ siz-i-1 ] = wrd[i];
      wrd[i] = swp;
   }
}


/*----------------------------------------------------------------------------*/
/* Set current position in a file                                             */
/*----------------------------------------------------------------------------*/

static int SetFilPos(GmfMshSct *msh, int64_t pos)
{
#ifdef WITH_GMF_AIO
   if(msh->typ & Bin)
      return((lseek(msh->FilDes, (off_t)pos, 0) != -1));
   else
      return((MYFSEEK(msh->hdl, (off_t)pos, SEEK_SET) == 0));
#else
   return((MYFSEEK(msh->hdl, (off_t)pos, SEEK_SET) == 0));
#endif
}


/*----------------------------------------------------------------------------*/
/* Get current position in a file                                             */
/*----------------------------------------------------------------------------*/

static int64_t GetFilPos(GmfMshSct *msh)
{
#ifdef WITH_GMF_AIO
   if(msh->typ & Bin)
      return(lseek(msh->FilDes, 0, 1));
   else
      return(MYFTELL(msh->hdl));
#else
   return(MYFTELL(msh->hdl));
#endif
}


/*----------------------------------------------------------------------------*/
/* Move the position to the end of file and return the size                   */
/*----------------------------------------------------------------------------*/

static int64_t GetFilSiz(GmfMshSct *msh)
{
   int64_t CurPos, EndPos = 0;

   if(msh->typ & Bin)
   {
#ifdef WITH_GMF_AIO
      CurPos = lseek(msh->FilDes, 0, 1);
      EndPos = lseek(msh->FilDes, 0, 2);
      lseek(msh->FilDes, (off_t)CurPos, 0);
#else
      CurPos = MYFTELL(msh->hdl);

      if(MYFSEEK(msh->hdl, 0, SEEK_END) != 0)
         longjmp(msh->err, -32);

      EndPos = MYFTELL(msh->hdl);

      if(MYFSEEK(msh->hdl, (off_t)CurPos, SEEK_SET) != 0)
         longjmp(msh->err, -33);
#endif
   }
   else
   {
      CurPos = MYFTELL(msh->hdl);

      if(MYFSEEK(msh->hdl, 0, SEEK_END) != 0)
         longjmp(msh->err, -34);

      EndPos = MYFTELL(msh->hdl);

      if(MYFSEEK(msh->hdl, (off_t)CurPos, SEEK_SET) != 0)
         longjmp(msh->err, -35);
   }

   return(EndPos);
}


/*----------------------------------------------------------------------------*/
/* Fortran 77 API                                                             */
/*----------------------------------------------------------------------------*/

#ifdef F77API

int64_t APIF77(gmfopenmesh)(  char *FilNam, int *mod,
                              int *ver, int *dim, int StrSiz )
{
   int   i = 0;
   char  TmpNam[ GmfStrSiz ];

   if(StrSiz <= 0)
      return(0);

   // Trim trailing spaces from the fortran string
   while(isspace(FilNam[ StrSiz-1 ]))
      StrSiz--;

   for(i=0;i<StrSiz;i++)
      TmpNam[i] = FilNam[i];

   TmpNam[ StrSiz ] = 0;

   if(*mod == GmfRead)
      return(GmfOpenMesh(TmpNam, *mod, ver, dim));
   else
      return(GmfOpenMesh(TmpNam, *mod, *ver, *dim));
}

int APIF77(gmfclosemesh)(int64_t *idx)
{
   return(GmfCloseMesh(*idx));
}

int APIF77(gmfgotokwd)(int64_t *MshIdx, int *KwdIdx)
{
   return(GmfGotoKwd(*MshIdx, *KwdIdx));
}

int APIF77(gmfstatkwd)( int64_t *MshIdx, int *KwdIdx, int *NmbTyp,
                        int *SolSiz, int *TypTab,  int *deg, int *NmbNod)
{
   if(!strcmp(GmfKwdFmt[ *KwdIdx ][2], "hr"))
      return(GmfStatKwd(*MshIdx, *KwdIdx, NmbTyp, SolSiz, TypTab, deg, NmbNod));
   else if(!strcmp(GmfKwdFmt[ *KwdIdx ][2], "sr"))
      return(GmfStatKwd(*MshIdx, *KwdIdx, NmbTyp, SolSiz, TypTab));
   else
      return(GmfStatKwd(*MshIdx, *KwdIdx));
}

int APIF77(gmfsetkwd)(  int64_t *MshIdx, int *KwdIdx, int *NmbLin,
                        int *NmbTyp, int *TypTab, int *deg, int *NmbNod)
{
   if(!strcmp(GmfKwdFmt[ *KwdIdx ][2], "hr"))
      return(GmfSetKwd(*MshIdx, *KwdIdx, *NmbLin, *NmbTyp, TypTab, *deg, *NmbNod));
   else if(!strcmp(GmfKwdFmt[ *KwdIdx ][2], "sr"))
      return(GmfSetKwd(*MshIdx, *KwdIdx, *NmbLin, *NmbTyp, TypTab));
   else
      return(GmfSetKwd(*MshIdx, *KwdIdx, *NmbLin));
}


int APIF77(gmfsethonodesordering)(int64_t *MshIdx, int *KwdCod, int *BasTab, int *OrdTab)
{
   return(GmfSetHONodesOrdering(*MshIdx, *KwdCod, BasTab, OrdTab));
}
/*
int APIF77(gmfreadbyteflow)(int64_t *MshIdx, char *BytFlo, int *NmbByt)
{
   int TmpNmb;
   char *TmpFlo;

   TmpFlo = GmfReadByteFlow(*MshIdx, &TmpNmb);

   if(!TmpFlo || NmbByt <= 0 || !BytFlo || TmpNmb > *NmbByt)
      return(0);

   *NmbByt = TmpNmb;
   memcpy(BytFlo, TmpFlo, *NmbByt);
   free(TmpFlo);

   return(TmpNmb);
}

int APIF77(gmfwritebyteflow)(int64_t *MshIdx, char *BytFlo, int *NmbByt)
{
   return(GmfWriteByteFlow(*MshIdx, BytFlo, *NmbByt));
}

int APIF77(gmfgetfloatprecision)(int64_t *MshIdx)
{
   return(GmfGetFloatPrecision(*MshIdx));
}

int APIF77(gmfsetfloatprecision)(int64_t *MshIdx, int *FltSiz)
{
   GmfSetFloatPrecision(*MshIdx, *FltSiz);
   return(0);
}
*/

/*----------------------------------------------------------------------------*/
/* Duplication macros                                                         */
/*----------------------------------------------------------------------------*/

#define DUP(s,n) DUP ## n (s)
#define DUP1(s) s
#define DUP2(s) DUP1(s),s
#define DUP3(s) DUP2(s),s
#define DUP4(s) DUP3(s),s
#define DUP5(s) DUP4(s),s
#define DUP6(s) DUP5(s),s
#define DUP7(s) DUP6(s),s
#define DUP8(s) DUP7(s),s
#define DUP9(s) DUP8(s),s
#define DUP10(s) DUP9(s),s
#define DUP11(s) DUP10(s),s
#define DUP12(s) DUP11(s),s
#define DUP13(s) DUP12(s),s
#define DUP14(s) DUP13(s),s
#define DUP15(s) DUP14(s),s
#define DUP16(s) DUP15(s),s
#define DUP17(s) DUP16(s),s
#define DUP18(s) DUP17(s),s
#define DUP19(s) DUP18(s),s
#define DUP20(s) DUP19(s),s


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


/*----------------------------------------------------------------------------*/
/* Call a fortran thread with 1 to 20 arguments                               */
/*----------------------------------------------------------------------------*/

static void CalF77Prc(  int64_t BegIdx, int64_t EndIdx,
                        void *prc, int NmbArg, void **ArgTab )
{
   switch(NmbArg)
   {
      case 1 :
      {
         void (*prc1)(int64_t *, int64_t *, DUP(void *, 1)) =
            (void (*)(int64_t *, int64_t *, DUP(void *, 1)))prc;
         prc1(&BegIdx, &EndIdx, ARG(ArgTab, 1));
      }break;

      case 2 :
      {
         void (*prc1)(int64_t *, int64_t *, DUP(void *, 2)) =
            (void (*)(int64_t *, int64_t *, DUP(void *, 2)))prc;
         prc1(&BegIdx, &EndIdx, ARG(ArgTab, 2));
      }break;

      case 3 :
      {
         void (*prc1)(int64_t *, int64_t *, DUP(void *, 3)) =
            (void (*)(int64_t *, int64_t *, DUP(void *, 3)))prc;
         prc1(&BegIdx, &EndIdx, ARG(ArgTab, 3));
      }break;

      case 4 :
      {
         void (*prc1)(int64_t *, int64_t *, DUP(void *, 4)) =
            (void (*)(int64_t *, int64_t *, DUP(void *, 4)))prc;
         prc1(&BegIdx, &EndIdx, ARG(ArgTab, 4));
      }break;

      case 5 :
      {
         void (*prc1)(int64_t *, int64_t *, DUP(void *, 5)) =
            (void (*)(int64_t *, int64_t *, DUP(void *, 5)))prc;
         prc1(&BegIdx, &EndIdx, ARG(ArgTab, 5));
      }break;

      case 6 :
      {
         void (*prc1)(int64_t *, int64_t *, DUP(void *, 6)) =
            (void (*)(int64_t *, int64_t *, DUP(void *, 6)))prc;
         prc1(&BegIdx, &EndIdx, ARG(ArgTab, 6));
      }break;

      case 7 :
      {
         void (*prc1)(int64_t *, int64_t *, DUP(void *, 7)) =
            (void (*)(int64_t *, int64_t *, DUP(void *, 7)))prc;
         prc1(&BegIdx, &EndIdx, ARG(ArgTab, 7));
      }break;

      case 8 :
      {
         void (*prc1)(int64_t *, int64_t *, DUP(void *, 8)) =
            (void (*)(int64_t *, int64_t *, DUP(void *, 8)))prc;
         prc1(&BegIdx, &EndIdx, ARG(ArgTab, 8));
      }break;

      case 9 :
      {
         void (*prc1)(int64_t *, int64_t *, DUP(void *, 9)) =
            (void (*)(int64_t *, int64_t *, DUP(void *, 9)))prc;
         prc1(&BegIdx, &EndIdx, ARG(ArgTab, 9));
      }break;

      case 10 :
      {
         void (*prc1)(int64_t *, int64_t *, DUP(void *, 10)) =
            (void (*)(int64_t *, int64_t *, DUP(void *, 10)))prc;
         prc1(&BegIdx, &EndIdx, ARG(ArgTab, 10));
      }break;

      case 11 :
      {
         void (*prc1)(int64_t *, int64_t *, DUP(void *, 11)) =
            (void (*)(int64_t *, int64_t *, DUP(void *, 11)))prc;
         prc1(&BegIdx, &EndIdx, ARG(ArgTab, 11));
      }break;

      case 12 :
      {
         void (*prc1)(int64_t *, int64_t *, DUP(void *, 12)) =
            (void (*)(int64_t *, int64_t *, DUP(void *, 12)))prc;
         prc1(&BegIdx, &EndIdx, ARG(ArgTab, 12));
      }break;

      case 13 :
      {
         void (*prc1)(int64_t *, int64_t *, DUP(void *, 13)) =
            (void (*)(int64_t *, int64_t *, DUP(void *, 13)))prc;
         prc1(&BegIdx, &EndIdx, ARG(ArgTab, 13));
      }break;

      case 14 :
      {
         void (*prc1)(int64_t *, int64_t *, DUP(void *, 14)) =
            (void (*)(int64_t *, int64_t *, DUP(void *, 14)))prc;
         prc1(&BegIdx, &EndIdx, ARG(ArgTab, 14));
      }break;

      case 15 :
      {
         void (*prc1)(int64_t *, int64_t *, DUP(void *, 15)) =
            (void (*)(int64_t *, int64_t *, DUP(void *, 15)))prc;
         prc1(&BegIdx, &EndIdx, ARG(ArgTab, 15));
      }break;

      case 16 :
      {
         void (*prc1)(int64_t *, int64_t *, DUP(void *, 16)) =
            (void (*)(int64_t *, int64_t *, DUP(void *, 16)))prc;
         prc1(&BegIdx, &EndIdx, ARG(ArgTab, 16));
      }break;

      case 17 :
      {
         void (*prc1)(int64_t *, int64_t *, DUP(void *, 17)) =
            (void (*)(int64_t *, int64_t *, DUP(void *, 17)))prc;
         prc1(&BegIdx, &EndIdx, ARG(ArgTab, 17));
      }break;

      case 18 :
      {
         void (*prc1)(int64_t *, int64_t *, DUP(void *, 18)) =
            (void (*)(int64_t *, int64_t *, DUP(void *, 18)))prc;
         prc1(&BegIdx, &EndIdx, ARG(ArgTab, 18));
      }break;

      case 19 :
      {
         void (*prc1)(int64_t *, int64_t *, DUP(void *, 19)) =
            (void (*)(int64_t *, int64_t *, DUP(void *, 19)))prc;
         prc1(&BegIdx, &EndIdx, ARG(ArgTab, 19));
      }break;

      case 20 :
      {
         void (*prc1)(int64_t *, int64_t *, DUP(void *, 20)) =
            (void (*)(int64_t *, int64_t *, DUP(void *, 20)))prc;
         prc1(&BegIdx, &EndIdx, ARG(ArgTab, 20));
      }break;
   }
}

#endif
