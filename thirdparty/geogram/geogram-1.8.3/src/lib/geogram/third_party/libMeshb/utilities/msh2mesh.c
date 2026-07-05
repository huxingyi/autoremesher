

/*----------------------------------------------------------*/
/*															*/
/*							MSH2SMESH						*/
/*															*/
/*					Loic MARECHAL 21/10/99					*/
/*															*/
/*	lecture d'un .faces/.points et ecriture d'un .mesh		*/
/*															*/
/*----------------------------------------------------------*/


/*----------------------------------------------------------*/
/* Includes													*/
/*----------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <libmesh.h>
#include <string.h>


/*----------------------------------------------------------*/
/* Protos													*/
/*----------------------------------------------------------*/

void mouliner(char *,char *);


/*----------------------------------------------------------*/
/* Parsing des arguments									*/
/*----------------------------------------------------------*/

main(argc,argv)

int argc;
char **argv;

{
	char *nom_source=0,*nom_destination=0;

	switch(argc)
	{
		case 1 :
		{
			printf("\nMSH2MESH v1.1   04/01/2000   Loic MARECHAL\n");
			printf(" Utilisation  : msh2mesh source destination\n");
			printf("  source      : nom du maillage au format .POINTS/.FACES sans l'extension\n");
			printf("  destination : nom du maillage au format MESH\n\n");
			exit(1);
		}break;

		case 2 :
		{
			if(argc==2)
			{
				fprintf(stderr,"Nombre d'arguments incorrect!\n");
				exit(1);
			}
		}break;

		case 3 :
		{
			nom_source=*++argv;
			nom_destination=*++argv;
		}break;
	}

	mouliner(nom_source,nom_destination);
}


/*----------------------------------------------------------*/
/* Moulinette : lit et ecrit a la volee						*/
/*----------------------------------------------------------*/

void mouliner(nom_source,nom_destination)

char *nom_source,*nom_destination;

{
	FILE *h_points,*h_faces,*dest;
	int nbv=0,nbf=0,nbp,nbe=0,nbt=0,nbq=0,ref,i,j,v0,v1,v2,v3,vide,meshversion=1,pos;
	double x,y,z;
	char nom[256],sx[256],sy[256],sz[256],*ptr;

	/* ouverture du .points/.faces */

	strcpy(nom, nom_source);
	strcat(nom, ".points");

	if(!(h_points = fopen(nom, "r")))
	{
		fprintf(stderr, "Impossible d'ouvrir %s\n",nom);
		exit(1);
	}

	strcpy(nom, nom_source);
	strcat(nom, ".faces");

	if(!(h_faces = fopen(nom, "r")))
	{
		fprintf(stderr, "Impossible d'ouvrir %s\n",nom);
		exit(1);
	}

	/* ouverture du mesh de destination */

	if(!(dest = ouvrir_mesh(nom_destination,"w",&meshversion)))
	{
		fprintf(stderr,"Impossible d'ouvrir %s\n",nom_destination);
		exit(1);
	}

	/* lecture des dimensions dans le .points/.faces */

	fscanf(h_points, "%d", &nbv);
	fscanf(h_faces, "%d", &nbf);

	/* conversion des points a la volee */

	ecrire_commentaire(dest, "Conversion d'un fichier msh2 (.points/.faces) par MSH2MESH v 1.0 de Loic MARECHAL");
	ecrire_mot_clef(dest, MeshDimension);
	ecrire_int(dest, 3);
	formater(dest);
 
	ecrire_mot_clef(dest, Vertices);
	ecrire_int(dest, nbv);
	formater(dest);

 	for(i=1;i<=nbv;i++)
	{
		/* Lecture des coordonnees dans des chaines */

		fscanf(h_points, "%s %s %s %d", &sx, &sy, &sz, &ref);

		/* Les 'd' ou 'D' dur fortran sont remplaces par des 'e' pour le C */

		if(ptr=strpbrk(sx,"dD"))
			*ptr = 'e';

		if(ptr=strpbrk(sy,"dD"))
			*ptr = 'e';

		if(ptr=strpbrk(sz,"dD"))
			*ptr = 'e';

		/* Lecture C standard des chaines filtrees */

		sscanf(sx, "%lf", &x);
		sscanf(sy, "%lf", &y);
		sscanf(sz, "%lf", &z);

		ecrire_reel(dest, x);
		ecrire_reel(dest, y);
		ecrire_reel(dest, z);
		ecrire_int(dest, ref);
		formater(dest);
	}

	/* compte le nombre d'edges, de triangles et de quadrangles */

	pos = ftell(h_faces);

 	for(i=1;i<=nbf;i++)
	{
		fscanf(h_faces, "%d", &nbp);

		switch(nbp)
		{
			case 2 :
			{
				nbe++;
				for(j=1;j<=5;j++)
					fscanf(h_faces, "%d", &vide);
			}break;

			case 3 :
			{
				nbt++;
				for(j=1;j<=7;j++)
					fscanf(h_faces, "%d", &vide);
			}break;

			case 4 :
			{
				nbq++;
				for(j=1;j<=9;j++)
					fscanf(h_faces, "%d", &vide);
			}break;
		}
	}

	/* conversion des edges a la volee */

	if(nbe)
	{
		fseek(h_faces, pos, SEEK_SET);

		ecrire_mot_clef(dest, Edges);
		ecrire_int(dest, nbe);
		formater(dest);

 	 	for(i=1;i<=nbf;i++)
		{
			fscanf(h_faces, "%d", &nbp);

			switch(nbp)
			{
				case 2 :
				{
					fscanf(h_faces, "%d %d %d %d %d", &v0, &v1, &ref, &vide, &vide);
					ecrire_int(dest, v0);
					ecrire_int(dest, v1);
					ecrire_int(dest, ref);
					formater(dest);
				}break;

				case 3 :
				{
					for(j=1;j<=7;j++)
						fscanf(h_faces, "%d", &vide);
				}break;

				case 4 :
				{
					for(j=1;j<=9;j++)
						fscanf(h_faces, "%d", &vide);
				}break;
			}
		}
	}

	/* conversion des triangles a la volee */

	if(nbt)
	{
		fseek(h_faces, pos, SEEK_SET);

		ecrire_mot_clef(dest, Triangles);
		ecrire_int(dest, nbt);
		formater(dest);

 	 	for(i=1;i<=nbf;i++)
		{
			fscanf(h_faces, "%d", &nbp);

			switch(nbp)
			{
				case 2 :
				{
					for(j=1;j<=5;j++)
						fscanf(h_faces, "%d", &vide);
				}break;

				case 3 :
				{
					fscanf(h_faces, "%d %d %d %d %d %d %d", \
							&v0, &v1, &v2, &ref, &vide, &vide, &vide);
					ecrire_int(dest, v0);
					ecrire_int(dest, v1);
					ecrire_int(dest, v2);
					ecrire_int(dest, ref);
					formater(dest);
				}break;

				case 4 :
				{
					for(j=1;j<=9;j++)
						fscanf(h_faces, "%d", &vide);
				}break;
			}
		}
	}

	/* conversion des quadrangles a la volee */

	if(nbq)
	{
		fseek(h_faces, pos, SEEK_SET);

		ecrire_mot_clef(dest, Quadrilaterals);
		ecrire_int(dest, nbq);
		formater(dest);

 	 	for(i=1;i<=nbf;i++)
		{
			fscanf(h_faces, "%d", &nbp);

			switch(nbp)
			{
				case 2 :
				{
					for(j=1;j<=5;j++)
						fscanf(h_faces, "%d", &vide);
				}break;

				case 3 :
				{
					for(j=1;j<=7;j++)
						fscanf(h_faces, "%d", &vide);
				}break;

				case 4 :
				{
					fscanf(h_faces, "%d %d %d %d %d %d %d %d %d", \
							&v0, &v1, &v2, &v3, &ref, &vide, &vide, &vide, &vide);
					ecrire_int(dest, v0);
					ecrire_int(dest, v1);
					ecrire_int(dest, v2);
					ecrire_int(dest, v3);
					ecrire_int(dest, ref);
					formater(dest);
				}break;
			}
		}
	}

	/* fermeture des fichiers */

	ecrire_mot_clef(dest, End);

	fclose(h_points);
	fclose(h_faces);
	fclose(dest);
}
