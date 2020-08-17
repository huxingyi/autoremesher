  

/*----------------------------------------------------------*/
/*                                                          */
/*                        LIBMESH V 7.12                    */
/*                                                          */
/*----------------------------------------------------------*/
/*                                                          */
/*    Description:        handle .meshb file format I/O     */
/*    Author:             Loic MARECHAL                     */
/*    Creation date:      dec 08 2015                       */
/*    Last modification:  may 30 2016                       */
/*                                                          */
/*----------------------------------------------------------*/

/*----------------------------------------------------------*/
/* Headers' macros                                          */
/*----------------------------------------------------------*/

#ifdef F77API

#ifdef F77_NO_UNDER_SCORE
#define NAMF77(c,f) f
#define APIF77(x) x
#else
#define NAMF77(c,f) f ## _
#define APIF77(x) x ## _
#endif

#define VALF77(v) *v
#define TYPF77(t) t*
#define PRCF77(p) *((int *)p)

#else

#define NAMF77(c,f) c
#define VALF77(v) v
#define TYPF77(t) t
#define PRCF77(p) p

#endif


/*----------------------------------------------------------*/
/* Includes                                                 */
/*----------------------------------------------------------*/

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
 *    Under Windows, 
 *  1)   _O_BINARY should be set in the flags.
 *  2) 'mode' has a completely different meaning
 */

#if defined(__unix__) || defined(__linux__) || defined(__APPLE__) || defined(__EMSCRIPTEN__)

#include <unistd.h>

#define OPEN_READ_FLAGS    O_RDONLY
#define OPEN_WRITE_FLAGS   O_CREAT | O_WRONLY | O_TRUNC
#define OPEN_READ_MODE     0666
#define OPEN_WRITE_MODE    0666    
    
#elif defined(WIN32) || defined(_WIN64)

#define GMF_WINDOWS

#include <windows.h>
#include <io.h>
#include <sys/stat.h>

#define OPEN_READ_FLAGS   O_RDONLY | _O_BINARY
#define OPEN_WRITE_FLAGS  O_CREAT | O_WRONLY | O_TRUNC | _O_BINARY
#define OPEN_READ_MODE    _S_IREAD
#define OPEN_WRITE_MODE   _S_IREAD | S_IWRITE 

#endif

#include <errno.h>
#include <geogram/third_party/LM7/libmeshb7.h>

/* [Bruno] Using portable printf modifier from pstdint.h        */
/* (alternative: use "%zd" under Linux and "%Id" under Windows) */

#ifdef PRINTF_INT64_MODIFIER
#define INT64_T_FMT "%" PRINTF_INT64_MODIFIER "d"
#else
#   ifdef GMF_WINDOWS
#     define INT64_T_FMT "%Id"
#   else
#     define INT64_T_FMT "%zd"
#   endif
#endif

/* [Bruno] Made asynchronous I/O optional */
#ifdef WITH_AIO
#include <aio.h>
#endif



/*----------------------------------------------------------*/
/* Defines                                                  */
/*----------------------------------------------------------*/

#define Asc 1
#define Bin 2
#define MshFil 4
#define SolFil 8
#define InfKwd 1
#define RegKwd 2
#define SolKwd 3
#define CmtKwd 4
#define WrdSiz 4
#define FilStrSiz 64
#define BufSiz 10000
#define MaxArg 20


/*----------------------------------------------------------*/
/* Structures                                               */
/*----------------------------------------------------------*/

typedef struct
{
    int typ, SolSiz, NmbWrd, NmbTyp, TypTab[ GmfMaxTyp ];
    int64_t NmbLin, pos;
    char fmt[ GmfMaxTyp*9 ];
}KwdSct;

typedef struct
{
    int dim, ver, mod, typ, cod, FilDes;
    int64_t NexKwdPos, siz, pos;
    jmp_buf err;
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
/* Global variables                                         */
/*----------------------------------------------------------*/

const char *GmfKwdFmt[ GmfMaxKwd + 1 ][4] = 
{    {"Reserved", "", "", ""},
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
    {"EdgesP4", "EdgeP4", "i", "iiiiii"},
    {"IRefGroups", "IRefGroup", "i", "c,i,i,i"},
    {"DRefGroups", "DRefGroup", "i", "i,i"}
 };

#ifdef TRANSMESH
int GmfMaxRefTab[ GmfMaxKwd + 1 ];
#endif


/*----------------------------------------------------------*/
/* Prototypes of local procedures                           */
/*----------------------------------------------------------*/

static void ScaWrd(GmfMshSct *, void *);
static void ScaDblWrd(GmfMshSct *, void *);
static int64_t GetPos(GmfMshSct *);
static void RecWrd(GmfMshSct *, const void *);
static void RecDblWrd(GmfMshSct *, const void *);
static void RecBlk(GmfMshSct *, const void *, int);
static void SetPos(GmfMshSct *, int64_t);
static int ScaKwdTab(GmfMshSct *);
static void ExpFmt(GmfMshSct *, int);
static void ScaKwdHdr(GmfMshSct *, int);
static void SwpWrd(char *, int);
static int SetFilPos(GmfMshSct *, int64_t);
static int64_t GetFilPos(GmfMshSct *msh);
static int64_t GetFilSiz(GmfMshSct *);
#ifdef F77API
static void CalF77Prc(int64_t, int64_t, void *, int, void **);
#endif


/*----------------------------------------------------------*/
/* Fscanf and fgets checking for errors                     */
/*----------------------------------------------------------*/

#define safe_fscanf(hdl, format, ptr, JmpErr) \
    do { \
        if( fscanf(hdl, format, ptr) != 1 ) \
            longjmp( JmpErr, -1); \
    } while(0)


#define safe_fgets(ptr, siz, hdl, JmpErr) \
    do { \
        if( fgets(ptr, siz, hdl) == NULL ) \
            longjmp( JmpErr, -1); \
    } while(0)


/*----------------------------------------------------------*/
/* Open a mesh file in read or write mod                    */
/*----------------------------------------------------------*/

int64_t GmfOpenMesh(char *FilNam, int mod, ...)
{
    int KwdCod, res, *PtrVer, *PtrDim;
    int64_t MshIdx;
    char str[ GmfStrSiz ];
    va_list VarArg;
    GmfMshSct *msh;

    /*---------------------*/
    /* MESH STRUCTURE INIT */
    /*---------------------*/

    if(!(msh = calloc(1, sizeof(GmfMshSct))))
        return(0);

    MshIdx = (int64_t)msh;

    /* Save the current stack environment for longjmp */

    if(setjmp(msh->err) != 0)
    {
        if(msh->hdl != NULL)
            fclose(msh->hdl);

        if(msh->FilDes != 0)
            close(msh->FilDes);

        free(msh);
        return(0);
    }

    /* Copy the FilNam into the structure */

    if(strlen(FilNam) + 7 >= GmfStrSiz)
        longjmp(msh->err, -1);

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
        longjmp(msh->err, -1);

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

        /* Read the endian coding tag, the mesh version and the mesh dimension (mandatory kwd) */

        if(msh->typ & Bin)
        {
            /* Create the name string and open the file */

            /* [Bruno] added binary flag (necessary under Windows) */
            msh->FilDes = open(msh->FilNam, OPEN_READ_FLAGS, OPEN_READ_MODE);

            if(msh->FilDes <= 0)
                longjmp(msh->err, -1);

            /* Read the endian coding tag, the mesh version and the mesh dimension (mandatory kwd) */

            if(read(msh->FilDes, &msh->cod, WrdSiz) != WrdSiz)
                longjmp(msh->err, -1);

            if( (msh->cod != 1) && (msh->cod != 16777216) )
                longjmp(msh->err, -1);

            ScaWrd(msh, (unsigned char *)&msh->ver);

            if( (msh->ver < 1) || (msh->ver > 4) )
                longjmp(msh->err, -1);

            if( (msh->ver >= 3) && (sizeof(int64_t) != 8) )
                longjmp(msh->err, -1);

            ScaWrd(msh, (unsigned char *)&KwdCod);

            if(KwdCod != GmfDimension)
                longjmp(msh->err, -1);

            GetPos(msh);
            ScaWrd(msh, (unsigned char *)&msh->dim);
        }
        else
        {
            /* Create the name string and open the file */

            if(!(msh->hdl = fopen(msh->FilNam, "rb")))
                longjmp(msh->err, -1);

            do
            {
                res = fscanf(msh->hdl, "%s", str);
            }while( (res != EOF) && strcmp(str, "MeshVersionFormatted") );

            if(res == EOF)
                longjmp(msh->err, -1);

            safe_fscanf(msh->hdl, "%d", &msh->ver, msh->err);

            if( (msh->ver < 1) || (msh->ver > 4) )
                longjmp(msh->err, -1);

            do
            {
                res = fscanf(msh->hdl, "%s", str);
            }while( (res != EOF) && strcmp(str, "Dimension") );

            if(res == EOF)
                longjmp(msh->err, -1);

            safe_fscanf(msh->hdl, "%d", &msh->dim, msh->err);
        }

        if( (msh->dim != 2) && (msh->dim != 3) )
            longjmp(msh->err, -1);

        (*PtrVer) = msh->ver;
        (*PtrDim) = msh->dim;

        /*------------*/
        /* KW READING */
        /*------------*/

        /* Read the list of kw present in the file */

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

        /* Check if the user provided a valid version number and dimension */

        va_start(VarArg, mod);
        msh->ver = va_arg(VarArg, int);
        msh->dim = va_arg(VarArg, int);
        va_end(VarArg);

        if( (msh->ver < 1) || (msh->ver > 4) )
            longjmp(msh->err, -1);

        if( (msh->ver >= 3) && (sizeof(int64_t) != 8) )
            longjmp(msh->err, -1);

        if( (msh->dim != 2) && (msh->dim != 3) )
            longjmp(msh->err, -1);

        /* Create the mesh file */

        if(msh->typ & Bin) 
        {
            /* 
             * [Bruno] replaced previous call to creat():
             * with a call to open(), because Windows needs the
             * binary flag to be specified.
             */
            msh->FilDes = open(msh->FilNam, OPEN_WRITE_FLAGS, OPEN_WRITE_MODE);

            if(msh->FilDes <= 0)
                longjmp(msh->err, -1);
        }
        else if(!(msh->hdl = fopen(msh->FilNam, "wb")))
            longjmp(msh->err, -1);


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
/* Close a meshfile in the right way                        */
/*----------------------------------------------------------*/

int GmfCloseMesh(int64_t MshIdx)
{
    int res = 1;
    GmfMshSct *msh = (GmfMshSct *)MshIdx;

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

    if(msh->typ & Bin)
        close(msh->FilDes);
    else if(fclose(msh->hdl))
        res = 0;

    free(msh);

    return(res);
}


/*----------------------------------------------------------*/
/* Read the number of lines and set the position to this kwd*/
/*----------------------------------------------------------*/

int64_t GmfStatKwd(int64_t MshIdx, int KwdCod, ...)
{
    int i, *PtrNmbTyp, *PtrSolSiz, *TypTab;
    GmfMshSct *msh = (GmfMshSct *)MshIdx;
    KwdSct *kwd;
    va_list VarArg;

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
/* Set the current file position to a given kwd              */
/*----------------------------------------------------------*/

int GmfGotoKwd(int64_t MshIdx, int KwdCod)
{
    GmfMshSct *msh = (GmfMshSct *)MshIdx;
    KwdSct *kwd = &msh->KwdTab[ KwdCod ];

    if( (KwdCod < 1) || (KwdCod > GmfMaxKwd) || !kwd->NmbLin )
        return(0);

    return(SetFilPos(msh, kwd->pos));
}


/*----------------------------------------------------------*/
/* Write the kwd and set the number of lines                */
/*----------------------------------------------------------*/

int GmfSetKwd(int64_t MshIdx, int KwdCod, ...)
{
    int i, *TypTab;
    int64_t NmbLin=0, CurPos;
    va_list VarArg;
    GmfMshSct *msh = (GmfMshSct *)MshIdx;
    KwdSct *kwd;

    RecBlk(msh, msh->buf, 0);

    if( (KwdCod < 1) || (KwdCod > GmfMaxKwd) )
        return(0);

    kwd = &msh->KwdTab[ KwdCod ];

    /* Read further arguments if this kw has a header */

    if(strlen(GmfKwdFmt[ KwdCod ][2]))
    {
        va_start(VarArg, KwdCod);
        NmbLin = va_arg(VarArg, int64_t);

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
        CurPos = GetFilPos(msh);

        if(!SetFilPos(msh, msh->NexKwdPos))
            return(0);

        SetPos(msh, CurPos);

        if(!SetFilPos(msh, CurPos))
            return(0);
    }

    /* Write the header */

    if(msh->typ & Asc)
    {
        fprintf(msh->hdl, "\n%s\n", GmfKwdFmt[ KwdCod ][0]);

        if(kwd->typ != InfKwd)
            fprintf(msh->hdl, INT64_T_FMT"\n", kwd->NmbLin);

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

    return(1);
}


/*----------------------------------------------------------*/
/* Read a full line from the current kwd                    */
/*----------------------------------------------------------*/

extern int NAMF77(GmfGetLin, gmfgetlin)(TYPF77(int64_t) MshIdx, TYPF77(int) KwdCod, ...)
{
    int i, j;
    float *FltSolTab;
    double *DblSolTab;
    va_list VarArg;
    GmfMshSct *msh = (GmfMshSct *) VALF77(MshIdx);
    KwdSct *kwd = &msh->KwdTab[ VALF77(KwdCod) ];

    if( (VALF77(KwdCod) < 1) || (VALF77(KwdCod) > GmfMaxKwd) )
        return(0);

    /* Save the current stack environment for longjmp */

    if(setjmp(msh->err) != 0)
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
                            safe_fscanf(msh->hdl, "%f", va_arg(VarArg, float *), msh->err);
                        else
                            safe_fscanf(msh->hdl, "%lf", va_arg(VarArg, double *), msh->err);
                    else if(kwd->fmt[i] == 'i')
                        if(msh->ver <= 3)
                            safe_fscanf(msh->hdl, "%d", va_arg(VarArg, int *), msh->err);
                        else
                            /* [Bruno] %ld -> INT64_T_FMT */
                            safe_fscanf(msh->hdl, INT64_T_FMT, va_arg(VarArg, int64_t *), msh->err);
                    else if(kwd->fmt[i] == 'c')
                        safe_fgets(va_arg(VarArg, char *), WrdSiz * FilStrSiz, msh->hdl, msh->err);
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
                            ScaDblWrd(msh, (unsigned char *)va_arg(VarArg, int64_t *));
                    else if(kwd->fmt[i] == 'c')
                        /* [Bruno] added error control */
                        if(fread(va_arg(VarArg, char *), WrdSiz, FilStrSiz, msh->hdl) != FilStrSiz) {
                            longjmp(msh->err, -1);
                        }
            }
        }break;

        case SolKwd :
        {
            if(msh->ver == 1)
            {
                FltSolTab = va_arg(VarArg, float *);

                if(msh->typ & Asc)
                    for(j=0;j<kwd->SolSiz;j++)
                        safe_fscanf(msh->hdl, "%f", &FltSolTab[j], msh->err);
                else
                    for(j=0;j<kwd->SolSiz;j++)
                        ScaWrd(msh, (unsigned char *)&FltSolTab[j]);
            }
            else
            {
                DblSolTab = va_arg(VarArg, double *);

                if(msh->typ & Asc)
                    for(j=0;j<kwd->SolSiz;j++)
                        safe_fscanf(msh->hdl, "%lf", &DblSolTab[j], msh->err);
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
/* Write a full line from the current kwd                   */
/*----------------------------------------------------------*/

extern int NAMF77(GmfSetLin, gmfsetlin)(TYPF77(int64_t) MshIdx, TYPF77(int) KwdCod, ...)
{
    int i, j, pos, *IntBuf;
    int64_t *LngBuf;
    float *FltSolTab, *FltBuf;
    double *DblSolTab, *DblBuf;
    va_list VarArg;
    GmfMshSct *msh = (GmfMshSct *) VALF77(MshIdx);
    KwdSct *kwd = &msh->KwdTab[ VALF77(KwdCod) ];

    if( ( VALF77(KwdCod) < 1) || ( VALF77(KwdCod) > GmfMaxKwd) )
        return(0);

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
                        fprintf(msh->hdl, "%g ", VALF77(va_arg(VarArg, TYPF77(double))));
                    else
                        fprintf(msh->hdl, "%.15g ", VALF77(va_arg(VarArg, TYPF77(double))));
                }
                else if(kwd->fmt[i] == 'i')
                {
                    if(msh->ver <= 3)
                        fprintf(msh->hdl, "%d ", VALF77(va_arg(VarArg, TYPF77(int))));
                    else
                        /* [Bruno] %ld -> INT64_T_FMT */
                        fprintf(msh->hdl, INT64_T_FMT " ", VALF77(va_arg(VarArg, TYPF77(int64_t))));
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
                    if(msh->ver <= 1)
                    {
                        FltBuf = (void *)&msh->buf[ pos ];
                        *FltBuf = (float) VALF77(va_arg(VarArg, TYPF77(double)));
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

    return(1);
}


/*----------------------------------------------------------*/
/* Private procedure for transmesh : copy a whole line      */
/*----------------------------------------------------------*/

#ifdef TRANSMESH

int GmfCpyLin(int64_t InpIdx, int64_t OutIdx, int KwdCod)
{
        char s[ WrdSiz * FilStrSiz ];
        double d;
        float f;
        int i, a;
        int64_t l;
        GmfMshSct *InpMsh = (GmfMshSct *)InpIdx, *OutMsh = (GmfMshSct *)OutIdx;
        KwdSct *kwd = &InpMsh->KwdTab[ KwdCod ];

        /* Save the current stack environment for longjmp */

        if(setjmp(InpMsh->err) != 0)
                return(0);

        for(i=0;i<kwd->SolSiz;i++)
        {
                if(kwd->fmt[i] == 'r')
                {
                        if(InpMsh->ver == 1)
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
                                read(InpMsh->FilDes, s, WrdSiz * FilStrSiz);

                        if(OutMsh->typ & Asc)
                                fprintf(OutMsh->hdl, "%s ", s);
                        else
                                write(OutMsh->FilDes, s, WrdSiz * FilStrSiz);
                }
        }

        if(OutMsh->typ & Asc)
                fprintf(OutMsh->hdl, "\n");

        return(1);
}

#endif

/* [Bruno] Made asynchronous I/O optional */
#ifdef WITH_AIO

/*----------------------------------------------------------*/
/* Bufferized asynchronous reading of all keyword's lines   */
/*----------------------------------------------------------*/

extern int NAMF77(GmfGetBlock, gmfgetblock)(TYPF77(int64_t) MshIdx, TYPF77(int) KwdCod, void *prc, ...)
{
    char *UsrDat[ GmfMaxTyp ], *FilBuf=NULL, *FrtBuf=NULL, *BckBuf=NULL, *FilPos, **SolTab1, **SolTab2;
    /* [Bruno] "%lld" -> INT64_T_FMT */
    char *StrTab[5] = { "", "%f", "%lf", "%d", INT64_T_FMT };
    int b, i, j, LinSiz, *FilPtrI32, *UsrPtrI32, FilTyp[ GmfMaxTyp ], UsrTyp[ GmfMaxTyp ];
    int NmbBlk, NmbArg, SizTab[5] = {0,4,8,4,8}, err, ret, typ, SolTabTyp = 0;
    int64_t NmbLin, *FilPtrI64, *UsrPtrI64, BegIdx, EndIdx=0;
    float *FilPtrR32, *UsrPtrR32;
    double *FilPtrR64, *UsrPtrR64;
    void (*UsrPrc)(int64_t, int64_t, void *) = NULL, *UsrArg, *ArgTab[ MaxArg ];
    size_t UsrLen[ GmfMaxTyp ], SolTypSiz;
    va_list VarArg;
    GmfMshSct *msh = (GmfMshSct *) VALF77(MshIdx);
    KwdSct *kwd = &msh->KwdTab[ VALF77(KwdCod) ];
    struct aiocb aio;

    /* Save the current stack environment for longjmp */
    if(setjmp(msh->err) != 0)
    {
        if(BckBuf)
            free(BckBuf);

        if(FrtBuf)
            free(FrtBuf);

        return(0);
    }

    /* Check mesh and keyword */
    if( (VALF77(KwdCod) < 1) || (VALF77(KwdCod) > GmfMaxKwd) || !kwd->NmbLin )
        return(0);

    /* Make sure it's not a simple information keyword */
    if( (kwd->typ != RegKwd) && (kwd->typ != SolKwd) )
        return(0);

    /* Start decoding the arguments */
    va_start(VarArg, prc);
    LinSiz = 0;

    /* Get the user's preporcessing procedure and argument adresses, if any */
#ifdef F77API
    if(PRCF77(prc))
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
    for(i=0;i<kwd->SolSiz;i++)
    {
        /* Get the user's data type and pointers to first and second adress to compute the stride */

        if(!SolTabTyp)
        {
            typ = VALF77(va_arg(VarArg, TYPF77(int)));

            if( (typ == GmfFloatTable) || (typ == GmfDoubleTable) )
            {
                if(typ == GmfFloatTable)
                {
                    SolTabTyp = GmfFloat;
                    SolTypSiz = sizeof(float);
                }
                else
                {
                    SolTabTyp = GmfDouble;
                    SolTypSiz = sizeof(double);
                }

                SolTab1 = va_arg(VarArg, char **);
                SolTab2 = va_arg(VarArg, char **);
            }
        }

        if(SolTabTyp)
        {
            UsrTyp[i] = SolTabTyp;
            UsrDat[i] = *(SolTab1 + i);
            UsrLen[i] = (size_t)(SolTab2[i] - SolTab1[i]);
        }
        else
        {
            UsrTyp[i] = typ;
            UsrDat[i] = va_arg(VarArg, char *);
            UsrLen[i] = (size_t)(va_arg(VarArg, char *) - UsrDat[i]);
        }

        /* Get the file's data type */
        if(kwd->fmt[i] == 'r')
            if(msh->ver <= 1)
                FilTyp[i] = GmfFloat;
            else
                FilTyp[i] = GmfDouble;
        else
            if(msh->ver <= 3)
                FilTyp[i] = GmfInt;
            else
                FilTyp[i] = GmfLong;

        /* Compute the file stride */
        LinSiz += SizTab[ FilTyp[i] ];
    }

    va_end(VarArg);

    /* Move file pointer to the keyword data */
    SetFilPos(msh, kwd->pos);

    /* Read the whole kwd data */

    if(msh->typ & Asc)
    {
        for(i=0;i<kwd->NmbLin;i++)
            for(j=0;j<kwd->SolSiz;j++)
            {
                safe_fscanf(msh->hdl, StrTab[ UsrTyp[j] ], UsrDat[j], msh->err);
                UsrDat[j] += UsrLen[j];
            }

        /* Call the user's preprocessing procedure */
        if(UsrPrc)
#ifdef F77API
            CalF77Prc(1, kwd->NmbLin, UsrPrc, NmbArg, ArgTab);
#else
            UsrPrc(1, kwd->NmbLin, UsrArg);
#endif
    }
    else
    {
        /* Allocate both front and back buffers */
        if(!(BckBuf = malloc((size_t)BufSiz * (size_t)LinSiz)))
            return(0);

        if(!(FrtBuf = malloc((size_t)BufSiz * (size_t)LinSiz)))
            return(0);

        /* Setup the ansynchonous parameters */
        memset(&aio, 0, sizeof(struct aiocb));
        FilBuf = BckBuf;
        aio.aio_buf = BckBuf;
        aio.aio_fildes = msh->FilDes;
        aio.aio_offset = GetFilPos(msh);

        NmbBlk = kwd->NmbLin / BufSiz;

        /* Loop over N+1 blocks */
        for(b=0;b<=NmbBlk+1;b++)
        {
            /* Wait for the previous block read to complete except for the first loop interation */
            if(b)
            {
                while(aio_error(&aio) == EINPROGRESS);

                err = aio_error(&aio);
                ret = aio_return(&aio);

                if (err != 0) {
                  printf (" Error at aio_error() : %s\n", strerror (err));
                  exit(1);
                }

                if (ret != aio.aio_nbytes) {
                  printf(" Error at aio_return()\n");
                  exit(1);
                }

                /* Increment the reading position */
                aio.aio_offset += aio.aio_nbytes;

                /* and swap the buffers */
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
 
            /* Read a chunk of data except for the last loop interarion */
            if(b <= NmbBlk)
            {
                /* The last block is shorter than the others */
                if(b == NmbBlk)
                    NmbLin = kwd->NmbLin - b * BufSiz;
                else
                    NmbLin = BufSiz;

                aio.aio_nbytes = NmbLin * LinSiz;

                if(aio_read(&aio) == -1)
                {
                    printf("aio_fildes = %d\n",aio.aio_fildes);
                    printf("aio_buf = %p\n",aio.aio_buf);
                    printf("aio_offset = %lld\n",aio.aio_offset);
                    printf("aio_nbytes = %ld\n",aio.aio_nbytes);
                    printf("errno = %d\n",errno);
                    exit(1);
                }
            }

            /* Then decode the block and store it in the user's data structure
             except for the first loop interation */
            if(b)
            {
                /* The last block is shorter than the others */
                if(b-1 == NmbBlk)
                    NmbLin = kwd->NmbLin - (b-1) * BufSiz;
                else
                    NmbLin = BufSiz;

                BegIdx = EndIdx+1;
                EndIdx += NmbLin;
                FilPos = FilBuf;

                for(i=0;i<NmbLin;i++)
                {
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

                        FilPos += SizTab[ FilTyp[j] ];
                        UsrDat[j] += UsrLen[j];
                    }
                }

                /* Call the user's preprocessing procedure */
                if(UsrPrc)
#ifdef F77API
                    CalF77Prc(BegIdx, EndIdx, UsrPrc, NmbArg, ArgTab);
#else
                    UsrPrc(BegIdx, EndIdx, UsrArg);
#endif
            }
        }

        free(BckBuf);
        free(FrtBuf);
    }

    return(1);
}


/*----------------------------------------------------------*/
/* Bufferized writing of all keyword's lines                */
/*----------------------------------------------------------*/

extern int NAMF77(GmfSetBlock, gmfsetblock)(TYPF77(int64_t) MshIdx, TYPF77(int) KwdCod, void *prc, ...)
{
    char *UsrDat[ GmfMaxTyp ], *FilBuf=NULL, *FrtBuf=NULL, *BckBuf=NULL, *FilPos;
    char *StrTab[5] = { "", "%g", "%.15g", "%d", "%lld" };
    int i, j, LinSiz, *FilPtrI32, *UsrPtrI32, FilTyp[ GmfMaxTyp ], UsrTyp[ GmfMaxTyp ];
    int NmbBlk, NmbArg, NmbLin, b, SizTab[5] = {0,4,8,4,8}, err, ret;
    int64_t *FilPtrI64, *UsrPtrI64, BegIdx, EndIdx=0;
    float *FilPtrR32, *UsrPtrR32;
    double *FilPtrR64, *UsrPtrR64;
    void (*UsrPrc)(int64_t, int64_t, void *) = NULL, *UsrArg, *ArgTab[ MaxArg ];
    size_t UsrLen[ GmfMaxTyp ];
    va_list VarArg;
    GmfMshSct *msh = (GmfMshSct *) VALF77(MshIdx);
    KwdSct *kwd = &msh->KwdTab[ VALF77(KwdCod) ];
    struct aiocb aio;

    /* Save the current stack environment for longjmp */
    if(setjmp(msh->err) != 0)
    {
        if(FilBuf)
            free(FilBuf);

        return(0);
    }

    /* Check mesh and keyword */
    if( (VALF77(KwdCod) < 1) || (VALF77(KwdCod) > GmfMaxKwd) || !kwd->NmbLin )
        return(0);

    /* Make sure it's not a simple information keyword */
    if( (kwd->typ != RegKwd) && (kwd->typ != SolKwd) )
        return(0);

    /* Start decoding the arguments */
    va_start(VarArg, prc);
    LinSiz = 0;

    /* Get the user's postprocessing procedure and argument adresses, if any */
#ifdef F77API
    if(PRCF77(prc))
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
    for(i=0;i<kwd->SolSiz;i++)
    {
        /* Get the user's data type and pointers to first and second adress to compute the stride */
        UsrTyp[i] = VALF77(va_arg(VarArg, TYPF77(int)));
        UsrDat[i] = va_arg(VarArg, char *);
        UsrLen[i] = (size_t)(va_arg(VarArg, char *) - UsrDat[i]);

        /* Get the file's data type */
        if(kwd->fmt[i] == 'r')
            if(msh->ver <= 1)
                FilTyp[i] = GmfFloat;
            else
                FilTyp[i] = GmfDouble;
        else
            if(msh->ver <= 3)
                FilTyp[i] = GmfInt;
            else
                FilTyp[i] = GmfLong;

        /* Compute the file stride */
        LinSiz += SizTab[ FilTyp[i] ];
    }

    va_end(VarArg);

    /* Write the whole kwd data */
    if(msh->typ & Asc)
    {
        if(UsrPrc)
#ifdef F77API
            CalF77Prc(1, kwd->NmbLin, UsrPrc, NmbArg, ArgTab);
#else
            UsrPrc(1, kwd->NmbLin, UsrArg);
#endif

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
                    UsrPtrI64 = (int64_t *)UsrDat[j];
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
        /* Allocate the front and back buffers */
        if(!(BckBuf = malloc((size_t)BufSiz * (size_t)LinSiz)))
            return(0);

        if(!(FrtBuf = malloc((size_t)BufSiz * (size_t)LinSiz)))
            return(0);

        /* Setup the asynchronous parameters */
        memset(&aio, 0, sizeof(struct aiocb));
        FilBuf = BckBuf;
        aio.aio_fildes = msh->FilDes;
        aio.aio_offset = GetFilPos(msh);

        NmbBlk = kwd->NmbLin / BufSiz;

        /* Loop over N+1 blocks */
        for(b=0;b<=NmbBlk+1;b++)
        {
            /* Launch an asynchronous block write except at the first loop iteration */
            if(b)
            {
                aio.aio_nbytes = NmbLin * LinSiz;
                
                if(aio_write(&aio) == -1)
                {
                    printf("aio_fildes = %d\n",aio.aio_fildes);
                    printf("aio_buf = %p\n",aio.aio_buf);
                    printf("aio_offset = %lld\n",aio.aio_offset);
                    printf("aio_nbytes = %ld\n",aio.aio_nbytes);
                    printf("errno = %d\n",errno);
                    exit(1);
                }
            }

            /* Parse the block data except at the last loop iteration */
            if(b<=NmbBlk)
            {
                /* The last block is shorter */
                if(b == NmbBlk)
                    NmbLin = kwd->NmbLin - b * BufSiz;
                else
                    NmbLin = BufSiz;

                FilPos = FilBuf;
                BegIdx = EndIdx+1;
                EndIdx += NmbLin;

                /* Call user's preprocessing first */
                if(UsrPrc)
#ifdef F77API
                    CalF77Prc(BegIdx, EndIdx, UsrPrc, NmbArg, ArgTab);
#else
                    UsrPrc(BegIdx, EndIdx, UsrArg);
#endif

                /* Then copy it's data to the file buffer */
                for(i=0;i<NmbLin;i++)
                {
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

                        FilPos += SizTab[ FilTyp[j] ];
                        UsrDat[j] += UsrLen[j];
                    }
                }
            }

            /* Wait for write completion execpt at the first loop iteration */
            if(b)
            {
                while(aio_error(&aio) == EINPROGRESS);

                err = aio_error(&aio);
                ret = aio_return(&aio);

                if (err != 0) {
                  printf (" Error at aio_error() : %s\n", strerror (err));
                  exit(1);
                }

                if (ret != aio.aio_nbytes) {
                  printf(" Error at aio_return()\n");
                  exit(1);
                }

                /* Move the write position */
                aio.aio_offset += aio.aio_nbytes;
            }

            /* Swap the buffers */
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

#endif

/*----------------------------------------------------------*/
/* Find every kw present in a meshfile                      */
/*----------------------------------------------------------*/

static int ScaKwdTab(GmfMshSct *msh)
{
    int KwdCod, c;
    int64_t  NexPos, EndPos;
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

        EndPos = GetFilSiz(msh);

        /* Jump through kwd positions in the file */

        do
        {
            /* Get the kwd code and the next kwd position */

            ScaWrd(msh, ( char *)&KwdCod);
            NexPos = GetPos(msh);

            if(NexPos > EndPos)
                longjmp(msh->err, -1);

            /* Check if this kwd belongs to this mesh version */

            if( (KwdCod >= 1) && (KwdCod <= GmfMaxKwd) )
                ScaKwdHdr(msh, KwdCod);

            /* Go to the next kwd */

            if(NexPos && !(SetFilPos(msh, NexPos)))
                longjmp(msh->err, -1);
        }while(NexPos && (KwdCod != GmfEnd));
    }

    return(1);
}


/*----------------------------------------------------------*/
/* Read and setup the keyword's header                      */
/*----------------------------------------------------------*/

static void ScaKwdHdr(GmfMshSct *msh, int KwdCod)
{
    int i;
    KwdSct *kwd = &msh->KwdTab[ KwdCod ];

    if(!strcmp("i", GmfKwdFmt[ KwdCod ][2]))
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

    if(!strcmp("sr", GmfKwdFmt[ KwdCod ][3]))
    {
        if(msh->typ & Asc)
        {
            safe_fscanf(msh->hdl, "%d", &kwd->NmbTyp, msh->err);

            for(i=0;i<kwd->NmbTyp;i++)
                safe_fscanf(msh->hdl, "%d", &kwd->TypTab[i], msh->err);
        }
        else
        {
            ScaWrd(msh, (unsigned char *)&kwd->NmbTyp);

            for(i=0;i<kwd->NmbTyp;i++)
                ScaWrd(msh, (unsigned char *)&kwd->TypTab[i]);
        }
    }

    ExpFmt(msh, KwdCod);
    kwd->pos = GetFilPos(msh);
}


/*----------------------------------------------------------*/
/* Expand the compacted format and compute the line size    */
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
/* Read a four bytes word from a mesh file                  */
/*----------------------------------------------------------*/

static void ScaWrd(GmfMshSct *msh, void *ptr)
{
    if(read(msh->FilDes, ptr, WrdSiz) != WrdSiz)
        longjmp(msh->err, -1);

    if(msh->cod != 1)
        SwpWrd((char *)ptr, WrdSiz);
}


/*----------------------------------------------------------*/
/* Read an eight bytes word from a mesh file                */
/*----------------------------------------------------------*/

static void ScaDblWrd(GmfMshSct *msh, void *ptr)
{
    if(read(msh->FilDes, ptr, WrdSiz * 2) != WrdSiz * 2)
        longjmp(msh->err, -1);

    if(msh->cod != 1)
        SwpWrd((char *)ptr, 2 * WrdSiz);
}


/*----------------------------------------------------------*/
/* Read a 4 or 8 bytes position in mesh file                */
/*----------------------------------------------------------*/

static int64_t GetPos(GmfMshSct *msh)
{
    int IntVal;
    int64_t pos;

    if(msh->ver >= 3)
        ScaDblWrd(msh, (unsigned char*)&pos);
    else
    {
        ScaWrd(msh, (unsigned char*)&IntVal);
        pos = (int64_t)IntVal;
    }

    return(pos);
}


/*----------------------------------------------------------*/
/* Write a four bytes word to a mesh file                   */
/*----------------------------------------------------------*/

static void RecWrd(GmfMshSct *msh, const void *wrd)
{
    /* [Bruno] added error control */
    if(write(msh->FilDes, wrd, WrdSiz) != WrdSiz) {
        longjmp(msh->err,-1);
    }
}


/*----------------------------------------------------------*/
/* Write an eight bytes word to a mesh file                 */
/*----------------------------------------------------------*/

static void RecDblWrd(GmfMshSct *msh, const void *wrd)
{
    /* [Bruno] added error control */    
    if(write(msh->FilDes, wrd, WrdSiz * 2) != WrdSiz*2) {
        longjmp(msh->err,-1);
    }
}


/*----------------------------------------------------------*/
/* Write a block of four bytes word to a mesh file          */
/*----------------------------------------------------------*/

static void RecBlk(GmfMshSct *msh, const void *blk, int siz)
{
    /* Copy this line-block into the main mesh buffer */

    if(siz)
    {
        memcpy(&msh->blk[ msh->pos ], blk, (size_t)(siz * WrdSiz));
        msh->pos += siz * WrdSiz;
    }

    /* When the buffer is full or this procedure is APIF77ed with a 0 size, flush the cache on disk */

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
        if((int64_t)write(msh->FilDes, msh->blk, (int)msh->pos) != msh->pos) {
            longjmp(msh->err,-1);
        }
#else        
        if(write(msh->FilDes, msh->blk, msh->pos) != msh->pos) {
            longjmp(msh->err,-1);
        }
#endif        
        msh->pos = 0;
    }
}


/*----------------------------------------------------------*/
/* Write a 4 or 8 bytes position in a mesh file             */
/*----------------------------------------------------------*/

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


/*----------------------------------------------------------*/
/* Endianness conversion                                    */
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
/* Set current position in a file                           */
/*----------------------------------------------------------*/

static int SetFilPos(GmfMshSct *msh, int64_t pos)
{
    if(msh->typ & Bin)
        return((lseek(msh->FilDes, (off_t)pos, 0) != -1));
    else
        return((fseek(msh->hdl, (off_t)pos, SEEK_SET) == 0));
}


/*----------------------------------------------------------*/
/* Get current position in a file                           */
/*----------------------------------------------------------*/

static int64_t GetFilPos(GmfMshSct *msh)
{
    if(msh->typ & Bin)
        return(lseek(msh->FilDes, 0, 1));
    else
        return(ftell(msh->hdl));
}


/*----------------------------------------------------------*/
/* Move the position to the end of file and return the size */
/*----------------------------------------------------------*/

static int64_t GetFilSiz(GmfMshSct *msh)
{
    int64_t CurPos, EndPos = 0;

    if(msh->typ & Bin)
    {
        CurPos = lseek(msh->FilDes, 0, 1);
        EndPos = lseek(msh->FilDes, 0, 2);
        lseek(msh->FilDes, (off_t)CurPos, 0);
    }
    else
    {
        CurPos = ftell(msh->hdl);

        if(fseek(msh->hdl, 0, SEEK_END) != 0)
            longjmp(msh->err, -1);

        EndPos = ftell(msh->hdl);

        if(fseek(msh->hdl, (off_t)CurPos, SEEK_SET) != 0)
            longjmp(msh->err, -1);
    }

    return(EndPos);
}


/*----------------------------------------------------------*/
/* Fortran 77 API                                           */
/*----------------------------------------------------------*/

#ifdef F77API

int64_t APIF77(gmfopenmesh)(char *FilNam, int *mod, int *ver, int *dim, int StrSiz)
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

int APIF77(gmfclosemesh)(int64_t *idx)
{
    return(GmfCloseMesh(*idx));
}

int APIF77(gmfgotokwd)(int64_t *MshIdx, int *KwdIdx)
{
    return(GmfGotoKwd(*MshIdx, *KwdIdx));
}

int APIF77(gmfstatkwd)(int64_t *MshIdx, int *KwdIdx, int *NmbTyp, int *SolSiz, int *TypTab)
{
    if(!strcmp(GmfKwdFmt[ *KwdIdx ][3], "sr"))
        return(GmfStatKwd(*MshIdx, *KwdIdx, NmbTyp, SolSiz, TypTab));
    else
        return(GmfStatKwd(*MshIdx, *KwdIdx));
}

int APIF77(gmfsetkwd)(int64_t *MshIdx, int *KwdIdx, int *NmbLin, int *NmbTyp, int *TypTab)
{
    if(!strcmp(GmfKwdFmt[ *KwdIdx ][3], "sr"))
        return(GmfSetKwd(*MshIdx, *KwdIdx, *NmbLin, *NmbTyp, TypTab));
    else if(strlen(GmfKwdFmt[ *KwdIdx ][2]))
        return(GmfSetKwd(*MshIdx, *KwdIdx, *NmbLin));
    else
        return(GmfSetKwd(*MshIdx, *KwdIdx));
}


/*----------------------------------------------------------*/
/* Duplication macros                                       */
/*----------------------------------------------------------*/

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


/*----------------------------------------------------------*/
/* Call a fortran thread with 1 to 20 arguments             */
/*----------------------------------------------------------*/

static void CalF77Prc(int64_t BegIdx, int64_t EndIdx, void *prc, int NmbArg, void **ArgTab)
{
    switch(NmbArg)
    {
        case 1 :
        {
            void (*prc1)(int64_t *, int64_t *, DUP(void *, 1)) = \
                (void (*)(int64_t *, int64_t *, DUP(void *, 1)))prc;
            prc1(&BegIdx, &EndIdx, ARG(ArgTab, 1));
        }break;

        case 2 :
        {
            void (*prc1)(int64_t *, int64_t *, DUP(void *, 2)) = \
                (void (*)(int64_t *, int64_t *, DUP(void *, 2)))prc;
            prc1(&BegIdx, &EndIdx, ARG(ArgTab, 2));
        }break;

        case 3 :
        {
            void (*prc1)(int64_t *, int64_t *, DUP(void *, 3)) = \
                (void (*)(int64_t *, int64_t *, DUP(void *, 3)))prc;
            prc1(&BegIdx, &EndIdx, ARG(ArgTab, 3));
        }break;

        case 4 :
        {
            void (*prc1)(int64_t *, int64_t *, DUP(void *, 4)) = \
                (void (*)(int64_t *, int64_t *, DUP(void *, 4)))prc;
            prc1(&BegIdx, &EndIdx, ARG(ArgTab, 4));
        }break;

        case 5 :
        {
            void (*prc1)(int64_t *, int64_t *, DUP(void *, 5)) = \
                (void (*)(int64_t *, int64_t *, DUP(void *, 5)))prc;
            prc1(&BegIdx, &EndIdx, ARG(ArgTab, 5));
        }break;

        case 6 :
        {
            void (*prc1)(int64_t *, int64_t *, DUP(void *, 6)) = \
                (void (*)(int64_t *, int64_t *, DUP(void *, 6)))prc;
            prc1(&BegIdx, &EndIdx, ARG(ArgTab, 6));
        }break;

        case 7 :
        {
            void (*prc1)(int64_t *, int64_t *, DUP(void *, 7)) = \
                (void (*)(int64_t *, int64_t *, DUP(void *, 7)))prc;
            prc1(&BegIdx, &EndIdx, ARG(ArgTab, 7));
        }break;

        case 8 :
        {
            void (*prc1)(int64_t *, int64_t *, DUP(void *, 8)) = \
                (void (*)(int64_t *, int64_t *, DUP(void *, 8)))prc;
            prc1(&BegIdx, &EndIdx, ARG(ArgTab, 8));
        }break;

        case 9 :
        {
            void (*prc1)(int64_t *, int64_t *, DUP(void *, 9)) = \
                (void (*)(int64_t *, int64_t *, DUP(void *, 9)))prc;
            prc1(&BegIdx, &EndIdx, ARG(ArgTab, 9));
        }break;

        case 10 :
        {
            void (*prc1)(int64_t *, int64_t *, DUP(void *, 10)) = \
                (void (*)(int64_t *, int64_t *, DUP(void *, 10)))prc;
            prc1(&BegIdx, &EndIdx, ARG(ArgTab, 10));
        }break;

        case 11 :
        {
            void (*prc1)(int64_t *, int64_t *, DUP(void *, 11)) = \
                (void (*)(int64_t *, int64_t *, DUP(void *, 11)))prc;
            prc1(&BegIdx, &EndIdx, ARG(ArgTab, 11));
        }break;

        case 12 :
        {
            void (*prc1)(int64_t *, int64_t *, DUP(void *, 12)) = \
                (void (*)(int64_t *, int64_t *, DUP(void *, 12)))prc;
            prc1(&BegIdx, &EndIdx, ARG(ArgTab, 12));
        }break;

        case 13 :
        {
            void (*prc1)(int64_t *, int64_t *, DUP(void *, 13)) = \
                (void (*)(int64_t *, int64_t *, DUP(void *, 13)))prc;
            prc1(&BegIdx, &EndIdx, ARG(ArgTab, 13));
        }break;

        case 14 :
        {
            void (*prc1)(int64_t *, int64_t *, DUP(void *, 14)) = \
                (void (*)(int64_t *, int64_t *, DUP(void *, 14)))prc;
            prc1(&BegIdx, &EndIdx, ARG(ArgTab, 14));
        }break;

        case 15 :
        {
            void (*prc1)(int64_t *, int64_t *, DUP(void *, 15)) = \
                (void (*)(int64_t *, int64_t *, DUP(void *, 15)))prc;
            prc1(&BegIdx, &EndIdx, ARG(ArgTab, 15));
        }break;

        case 16 :
        {
            void (*prc1)(int64_t *, int64_t *, DUP(void *, 16)) = \
                (void (*)(int64_t *, int64_t *, DUP(void *, 16)))prc;
            prc1(&BegIdx, &EndIdx, ARG(ArgTab, 16));
        }break;

        case 17 :
        {
            void (*prc1)(int64_t *, int64_t *, DUP(void *, 17)) = \
                (void (*)(int64_t *, int64_t *, DUP(void *, 17)))prc;
            prc1(&BegIdx, &EndIdx, ARG(ArgTab, 17));
        }break;

        case 18 :
        {
            void (*prc1)(int64_t *, int64_t *, DUP(void *, 18)) = \
                (void (*)(int64_t *, int64_t *, DUP(void *, 18)))prc;
            prc1(&BegIdx, &EndIdx, ARG(ArgTab, 18));
        }break;

        case 19 :
        {
            void (*prc1)(int64_t *, int64_t *, DUP(void *, 19)) = \
                (void (*)(int64_t *, int64_t *, DUP(void *, 19)))prc;
            prc1(&BegIdx, &EndIdx, ARG(ArgTab, 19));
        }break;

        case 20 :
        {
            void (*prc1)(int64_t *, int64_t *, DUP(void *, 20)) = \
                (void (*)(int64_t *, int64_t *, DUP(void *, 20)))prc;
            prc1(&BegIdx, &EndIdx, ARG(ArgTab, 20));
        }break;
    }
}

#endif
