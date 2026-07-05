

/*----------------------------------------------------------------------------*/
/*                                                                            */
/*                               TRANSMESH V 5.2                              */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Description:         convert mesh file from/to ascii/bin                   */
/* Author:              Loic MARECHAL                                         */
/* Creation date:       mar 08 2004                                           */
/* Last modification:   aug 05 2021                                           */
/*                                                                            */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Defines                                                                    */
/*----------------------------------------------------------------------------*/

#define TRANSMESH 1
#define StrSiz 1024
#define DefMaxRef 1024
#define MaxGrp 1024
#define MaxTok 1024

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

/*----------------------------------------------------------------------------*/
/* Includes                                                                   */
/*----------------------------------------------------------------------------*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "libmeshb7.h"


/*----------------------------------------------------------------------------*/
/* Prototypes of structures                                                   */
/*----------------------------------------------------------------------------*/

typedef struct
{
   int idx, NmbRef, *RefTab, typ, sgn;
   char *NfoStr;
}GrpSct;


/*----------------------------------------------------------------------------*/
/* Return the kwd's integer from its string                                   */
/*----------------------------------------------------------------------------*/

int GetKwdIdx(char *KwdStr)
{
   int KwdIdx;

   for(KwdIdx=1; KwdIdx<=GmfMaxKwd; KwdIdx++)
      if(!strcmp(GmfKwdFmt[ KwdIdx ][0], KwdStr))
         return(KwdIdx);

   return(0);
}


/*----------------------------------------------------------------------------*/
/* Check args and copy each field from in-file to out-file                    */
/*----------------------------------------------------------------------------*/

int main(int ArgCnt, char **ArgVec)
{
   int      NmbTyp, SolSiz, TypTab[ GmfMaxTyp ], FilVer = 0, InpVer, OutVer = 1;
   int      i, j, dim, MaxRef, ArgIdx = 1, NmbGrp = 0, pos, deg, NmbNod;
   int64_t  NmbLin, InpIdx, OutIdx;
   char     *InpNam, *OutNam, *PtrArg, *FlgTab;
   GrpSct   GrpTab[ MaxGrp ], *grp;
#ifndef WIN32
   float    f;
   double   d;
   int      NmbTok, NmbTok2, TokTab[ MaxTok ], TokTab2[ MaxTok ];
   char     *MshNam[ StrSiz ], *TmpStr, *TmpStr2, *TmpStr3;
#endif

   if(ArgCnt == 1)
   {
      puts("\nTRANSMESH v5.2, july 19 2017, Loic MARECHAL / INRIA\n");
      puts(" Usage    : transmesh source_name destination_name (-options)\n");
      puts(" optional : -v output_file_version");
      puts(" version 1: 32 bits integers, 32 bits reals, file size < 2 GigaBytes");
      puts(" version 2: 32 bits integers, 64 bits reals, file size < 2 GigaBytes");
      puts(" version 3: 32 bits integers, 64 bits reals, file size < 8 ExaBytes");
      puts(" version 4: 64 bits integers, 64 bits reals, file size < 8 ExaBytes\n");
#ifndef WIN32
      puts(" optional : -igroup inflow 1 Triangles 3,5,6,9-14");
      puts("            Defines a group named \"inflow\" with index 1 that includes");
      puts("            a set of comma-seprated triangle references.");
      puts("            You may also specify ranges between two dash-separated indices.\n");
      puts(" optional : -egroup wings 3 Quadrilaterals 6-20");
      puts("            Defines a group named \"wings\" with index 3 that excludes");
      puts("            a set of comma-seprated quadrilateral references.");
      puts("            You may also specify ranges between two dash-separated indices.\n");
#endif
      exit(0);
   }

   InpNam = ArgVec[ ArgIdx++ ];
   OutNam = ArgVec[ ArgIdx++ ];

   if(!strcmp(InpNam, OutNam))
   {
      puts("The output mesh cannot overwrite the input mesh.");
      exit(1);
   }

   memset(GmfMaxRefTab, 0, (GmfMaxKwd + 1) * sizeof(int));

   while(ArgIdx < ArgCnt)
   {
      PtrArg = ArgVec[ ArgIdx++ ];

      if(!strcmp(PtrArg, "-v"))
      {
         FilVer = atoi(ArgVec[ ArgIdx++ ]);

         if( (FilVer < 1) || (FilVer > 4) )
         {
            printf("Wrong size type : %d\n", FilVer);
            exit(1);
         }
      }
#ifndef WIN32
      if(!strcmp(PtrArg, "-igroup") || !strcmp(PtrArg, "-egroup"))
      {
         grp = &GrpTab[ NmbGrp++ ];
         grp->NfoStr = ArgVec[ ArgIdx++ ];
         grp->idx = atoi(ArgVec[ ArgIdx++ ]);
         grp->typ = GetKwdIdx(ArgVec[ ArgIdx++ ]);
         grp->NmbRef = 0;
         NmbTok = 0;
         TmpStr = ArgVec[ ArgIdx++ ];

         while( (NmbTok < MaxTok) && ((TmpStr2 = strsep(&TmpStr, ","))) )
         {
            NmbTok2 = 0;

            while( (NmbTok2 < 2) && ((TmpStr3 = strsep(&TmpStr2, "-"))) )
               TokTab2[ NmbTok2++ ] = atoi(TmpStr3);

            if(NmbTok2 == 1)
               TokTab[ NmbTok++ ] = TokTab2[0];
            else if(NmbTok2 == 2)
               for(i=TokTab2[0]; i<=TokTab2[1]; i++)
                  TokTab[ NmbTok++ ] = i;
            else
               exit(1);
         }

         grp->NmbRef = NmbTok;
         grp->RefTab = malloc(grp->NmbRef * sizeof(int));

         for(i=0;i<grp->NmbRef;i++)
            grp->RefTab[i] = TokTab[i];

         if(!strcmp(PtrArg, "-igroup"))
            grp->sgn = 1;
         else
            grp->sgn = -1;
      }
#endif
   }

   if(!(InpIdx = GmfOpenMesh(InpNam, GmfRead, &InpVer, &dim)))
   {
      fprintf(stderr,"Source of error : TRANSMESH / OPEN_MESH\n");
      fprintf(stderr,"Cannot open %s\n", InpNam);
      return(1);
   }

   if(FilVer)
      OutVer = FilVer;
   else
      OutVer = InpVer;

   if(!(OutIdx = GmfOpenMesh(OutNam, GmfWrite, OutVer, dim)))
   {
      fprintf(stderr,"Source of error : TRANSMESH / OPEN_MESH\n");
      fprintf(stderr,"Cannot open %s\n", OutNam);
      return(1);
   }

   for(i=0;i<=GmfMaxKwd;i++)
   {
      if(!strcmp(GmfKwdFmt[i][0], "Reserved") || !strcmp(GmfKwdFmt[i][0], "End"))
         continue;

      if(!GmfGotoKwd(InpIdx, i))
         continue;

      if(strcmp("i", GmfKwdFmt[i][1]))
      {
         if((NmbLin = GmfStatKwd(InpIdx, i)))
            GmfSetKwd(OutIdx, i, 0);
         else
            continue;
      }
      else if(strcmp("sr", GmfKwdFmt[i][2]) && strcmp("hr", GmfKwdFmt[i][2]))
      {
         if((NmbLin = GmfStatKwd(InpIdx, i)))
            GmfSetKwd(OutIdx, i, NmbLin);
         else
            continue;
      }
      else
      {
         if(!strcmp("sr", GmfKwdFmt[i][2]))
         {
            if((NmbLin = GmfStatKwd(InpIdx, i, &NmbTyp, &SolSiz, TypTab)))
               GmfSetKwd(OutIdx, i,  NmbLin, NmbTyp, TypTab);
            else
               continue;
         }
         else if(!strcmp("hr", GmfKwdFmt[i][2]))
         {
            if((NmbLin = GmfStatKwd(InpIdx, i, &NmbTyp, &SolSiz, TypTab, &deg, &NmbNod)))
               GmfSetKwd(OutIdx, i,  NmbLin, NmbTyp, TypTab, deg, NmbNod);
            else
               continue;
         }
      }

      printf("Parsing %s : "INT64_T_FMT" item\n", GmfKwdFmt[i][0], NmbLin);

      for(j=1;j<=NmbLin;j++)
         GmfCpyLin(InpIdx, OutIdx, i);
   }

   if(NmbGrp)
   {
      GmfSetKwd(OutIdx, GmfIRefGroups, NmbGrp);
      pos = 1;

      for(i=0;i<NmbGrp;i++)
      {
         grp = &GrpTab[i];

         if(grp->sgn == -1)
         {
            MaxRef = GmfMaxRefTab[ grp->typ ] ? GmfMaxRefTab[ grp->typ ] : DefMaxRef;
            FlgTab = malloc(MaxRef+1);

            for(i=1;i<=MaxRef;i++)
               FlgTab[i] = 1;

            for(i=0;i<grp->NmbRef;i++)
               FlgTab[ grp->RefTab[i] ] = 0;

            free(grp->RefTab);
            grp->NmbRef = MaxRef - grp->NmbRef;
            grp->RefTab = malloc(grp->NmbRef * sizeof(int));
            pos = 0;

            for(i=1;i<=MaxRef;i++)
               if(FlgTab[i])
                  grp->RefTab[ pos++ ] = i;

            free(FlgTab);
         }

         GmfSetLin(OutIdx, GmfIRefGroups, grp->NfoStr, grp->idx, grp->typ, grp->NmbRef, pos);
         pos += grp->NmbRef;
      }

      GmfSetKwd(OutIdx, GmfDRefGroups, pos - 1);

      for(i=0;i<NmbGrp;i++)
      {
         grp = &GrpTab[i];

         for(j=0;j<grp->NmbRef;j++)
            GmfSetLin(OutIdx, GmfDRefGroups, grp->typ, grp->RefTab[j], grp->idx);
      }
   }

   if(!GmfCloseMesh(InpIdx))
      return(1);

   if(!GmfCloseMesh(OutIdx))
      return(1);

   return(0);
}
