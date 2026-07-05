

/*----------------------------------------------------------*/
/*															*/
/*							MESH2PATRAN						*/
/*															*/
/*----------------------------------------------------------*/
/*															*/
/*	Description:		convert a hex mesh into bulk patran */
/*	Author:				Loic MARECHAL						*/
/*	Creation date:		15 nov 2001							*/
/*	Last modification:	15 nov 2001							*/
/*															*/
/*----------------------------------------------------------*/


/*----------------------------------------------------------*/
/* Headers													*/
/*----------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include "libmesh.h"

/*----------------------------------------------------------*/
/* Read the mesh and write the bulk on the fly				*/
/*----------------------------------------------------------*/

int main(int argc, char **argv)
{
	int meshversion, keyword, dim;
	char *source_name;
	FILE *input_handle;

	if(argc == 2)
		source_name = *argv[2];
	else
	{
		puts("Source filename missing.");
		exit(1);
	}

	/* Open a mesh file for reading */

	if(!(input_handle = ouvrir_mesh(source_name, "r", &meshversion)))
	{
		puts("Cannot open source file.");
		exit(1);
	}

	if(meshversion < 1)
	{
		puts("Bad mesh version.");
		exit(1);
	}

	/* Create a patran bulk file */

	if(!(output_handle = fopen(strcat(source_name, ".dat"), "w")))
	{
		puts("Cannot create destination file.");
		exit(1);
	}


	/*---------------------------------------------------*/
	/* scan for keywords and sizes for memory allocation */
	/*---------------------------------------------------*/

	do
	{
		keyword = mot_clef_suivant(input_handle);

		switch(keyword)
		{
			case MeshDimension :
			{
				dim = lire_int(input_handle);

				if(dim != 3)
				{
					puts("Not a 3 dimensions mesh.");
					exit(1);
				}
			}break;

			case Vertices :
			{
				nb = lire_int(input_handle);

				for(i=1;i<=nb;i++)
				{
					 x = lire_reel(input_handle);
					 y = lire_reel(input_handle);
					 z = lire_reel(input_handle);
					 ref = lire_int(input_handle);

					fprintf(output_handle, "GRID*   %25d",i, x,y,z
				}

			}break;
		}
	}while(keyword != End);
}
