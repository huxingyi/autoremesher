

/*----------------------------------------------------------*/
/*															*/
/*					NOBOITEB2MESHB V1.1						*/
/*															*/
/*----------------------------------------------------------*/
/*															*/
/*	Description:		convertd noboiteb to mesh(b)		*/
/*	Author:				Loic MARECHAL						*/
/*	Creation date:		dec 12 1997							*/
/*	Last modification:	feb 23 2007							*/
/*															*/
/*----------------------------------------------------------*/


/*----------------------------------------------------------*/
/* Includes													*/
/*----------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <libmesh5.h>


/*----------------------------------------------------------*/
/* Protos													*/
/*----------------------------------------------------------*/

void convertir(char *,char *);


/*----------------------------------------------------------*/
/* Structures												*/
/*----------------------------------------------------------*/

typedef struct
{
	float x,y,z;
}point_struct;

typedef struct
{
	int v1,v2,v3,v4;
}tetra_struct;


/*----------------------------------------------------------*/
/* Debut													*/
/*----------------------------------------------------------*/

int main(int argc, char **argv)
{
	char *nom_source,*nom_destination;

	switch(argc)
	{
		case 1 :
		{
			printf("NOBOITEB2MESHB v1.1   feb 28 2007   Loic MARECHAL\n");
			printf(" Utilisation	: noboiteb2meshb source destination\n");
			printf("  source        : nom du maillage au format NOBOITEB\n");
			printf("  dest          : nom du maillage au format MESH(B)\n");
			exit(1);
		}break;

		case 2 :
		{
			if(argc==2)
				fprintf(stderr,"Nombre d'arguments incorrect!\n");
		}break;

		case 3 :
		{
			nom_source=*++argv;
			nom_destination=*++argv;
		}break;
	}

	convertir(nom_source,nom_destination);

	return(0);
}


/*----------------------------------------------------------*/
/*  Lire_noboite											*/
/*----------------------------------------------------------*/
/* IN: un pointeur sur un nom de fichier 					*/
/*----------------------------------------------------------*/

void convertir(char * nom_source, char *nom_destination)
{
	FILE *src;
	int vide[10],nbele,loele,loelef,nbelef,nbpoi,lopoi,lopoif,nbpoif,i,j,debut,fin;
	int nb_tetraedres,nb_vertices,meshversion=0,pos_tetras, MshIdx;
	point_struct points;
	tetra_struct tetras;

	if(!(src=fopen(nom_source,"r")))
	{
		fprintf(stderr,"Source de l'erreur : NOBOITEB2MESHB / CHARGER_NOBOITE.\n");
		fprintf(stderr,"Impossible d'ouvrir %s\n",nom_source);
		exit(1);
	}

	if(!(MshIdx = GmfOpenMesh(nom_destination, GmfWrite, GmfFloat, 3)))
	{
		fprintf(stderr,"Source de l'erreur : NOBOITEB2MESHB / CHARGER_NOBOITE.\n");
		fprintf(stderr,"Impossible d'ouvrir %s\n",nom_destination);
		exit(1);
	}

	/* Lecture de l'entete du noboiteb */

	fread(vide,sizeof(int),1,src);

	fread(&nb_tetraedres,sizeof(int),1,src);
	fread(&nb_vertices,sizeof(int),1,src);
	fread(vide,sizeof(int),3,src);

	fread(&nbele,sizeof(int),1,src);
	fread(&loele,sizeof(int),1,src);
	fread(&nbelef,sizeof(int),1,src);
	fread(&loelef,sizeof(int),1,src);

	fread(&nbpoi,sizeof(int),1,src);
	fread(&lopoi,sizeof(int),1,src);
	fread(&nbpoif,sizeof(int),1,src);
	fread(&lopoif,sizeof(int),1,src);

	fread(vide,sizeof(int),5,src);

	/* Le handle est positionne sur les tetras,je le sauve pour y revenir ensuite */

	pos_tetras=ftell(src);

	fseek(src, nbele * (loele*4+8) + loelef*4 + 0 ,SEEK_CUR);

	printf("nbv=%d , nbt=%d\n",nb_vertices,nb_tetraedres);

	/* Lecture et ecriture des vertices */

	lopoi/=3;
	lopoif/=3;
	debut=1;
	fin=lopoi;

	GmfSetKwd(MshIdx, GmfVertices, nb_vertices);

	for(i=1;i<=nbpoi;i++)
	{
		fread(vide,sizeof(int),1,src);
		for(j=debut;j<=fin;j++)
		{
			fread(&points,sizeof(float),3,src);
			GmfSetLin(MshIdx, GmfVertices, points.x, points.y, points.z, 0);
		}
		fread(vide,sizeof(int),1,src);
		debut+=lopoi;
		fin+=lopoi;
	}
	if(lopoif)
	{
		fin=fin-lopoi+lopoif;
		fread(vide,sizeof(int),1,src);
		for(j=debut;j<=fin;j++)
		{
			fread(&points,sizeof(float),3,src);
			GmfSetLin(MshIdx, GmfVertices, points.x, points.y, points.z, 0);
		}
		fread(vide,sizeof(int),1,src);
	}

	/* Lecture et ecriture des tetraedres */

	fseek(src,pos_tetras,SEEK_SET);

	loele/=4;
	loelef/=4;
	debut=1;
	fin=loele;

	GmfSetKwd(MshIdx, GmfTetrahedra, nb_tetraedres);

	for(i=1;i<=nbele;i++)
	{
		fread(vide,sizeof(int),1,src);
		for(j=debut;j<=fin;j++)
		{
			fread(&tetras,sizeof(int),4,src);
			GmfSetLin(MshIdx, GmfTetrahedra, tetras.v1, tetras.v2, tetras.v3, tetras.v4, 0);
		}
		fread(vide,sizeof(int),1,src);
		debut+=loele;
		fin+=loele;
	}
	if(loelef)
	{
		fin=fin-loele+loelef;
		fread(vide,sizeof(int),1,src);
		for(j=debut;j<=fin;j++)
		{
			fread(&tetras,sizeof(int),4,src);
			GmfSetLin(MshIdx, GmfTetrahedra, tetras.v1, tetras.v2, tetras.v3, tetras.v4, 0);
		}
		fread(vide,sizeof(int),1,src);
	}

	GmfCloseMesh(MshIdx);
	fclose(src);
}
