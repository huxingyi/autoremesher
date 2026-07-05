
/* Exemple d'utilisation de la libmesh6 : transformation de quadrangles en triangles dans un maillage surfacique */

#include <stdio.h>
#include <stdlib.h>
#include "libmesh6.h"


int main()
{
	int i, NmbVer, NmbQad, InpMsh, OutMsh, ver, dim, *RefTab, (*QadTab)[5];
	double (*VerTab)[3];


	/*-----------------------------------*/
	/* Ouverture du maillage "quad.mesh" */
	/*-----------------------------------*/

	if(!(InpMsh = GmfOpenMesh("quad.mesh", GmfRead, &ver, &dim)))
		return(1);

	printf("InpMsh : idx = %d, version = %d, dimension = %d\n", InpMsh, ver, dim);

	if( (ver != GmfDouble) || (dim != 3) )
		exit(1);

	/* Lecture des nombres d'elements et de noeuds pour l'allocation de memoire */

	NmbVer = GmfStatKwd(InpMsh, GmfVertices);
	printf("InpMsh : nmb vertices = %d\n", NmbVer);
	VerTab = malloc((NmbVer+1) * 3 * sizeof(double));
	RefTab = malloc((NmbVer+1) * sizeof(int));

	NmbQad = GmfStatKwd(InpMsh, GmfQuadrilaterals);
	printf("InpMsh : nmb quads = %d\n", NmbQad);
	QadTab = malloc((NmbQad+1) * 5 * sizeof(int));

	/* Lecture des noeuds */

	GmfGotoKwd(InpMsh, GmfVertices);

	for(i=1;i<=NmbVer;i++)
		GmfGetLin(InpMsh, GmfVertices, &VerTab[i][0], &VerTab[i][1], &VerTab[i][2], &RefTab[i]);

	/* Lecture des quadrangles */

	GmfGotoKwd(InpMsh, GmfQuadrilaterals);

	for(i=1;i<=NmbQad;i++)
		GmfGetLin(InpMsh, GmfQuadrilaterals, &QadTab[i][0], &QadTab[i][1], &QadTab[i][2], &QadTab[i][3], &QadTab[i][4]);

	/* Fermeture du maillage quad */

	GmfCloseMesh(InpMsh);


	/*-----------------------------------*/
	/* Creation du maillage en triangles */
	/*-----------------------------------*/

	if(!(OutMsh = GmfOpenMesh("tri.mesh", GmfWrite, ver, dim)))
		return(1);

	/* Ecriture du nombre de noeuds */

	GmfSetKwd(OutMsh, GmfVertices, NmbVer);

	/* Puis boucle d'ecriture sur les noeuds */

	for(i=1;i<=NmbVer;i++)
		GmfSetLin(OutMsh, GmfVertices, VerTab[i][0], VerTab[i][1], VerTab[i][2], RefTab[i]);

	/*  Ecriture du nombre de triangles */

	GmfSetKwd(OutMsh, GmfTriangles, 2*NmbQad);
	printf("OutMsh : nmb triangles = %d\n", 2*NmbQad);

	/* Puis boucle d'ecriture sur les triangles (2 triangles par quad) */

	for(i=1;i<=NmbQad;i++)
	{
		GmfSetLin(OutMsh, GmfTriangles, QadTab[i][0], QadTab[i][1], QadTab[i][2], QadTab[i][4]);
		GmfSetLin(OutMsh, GmfTriangles, QadTab[i][0], QadTab[i][2], QadTab[i][3], QadTab[i][4]);
	}

	/* Ne pas oublier de fermer le fichier */

	GmfCloseMesh(OutMsh);

	free(QadTab);
	free(RefTab);
	free(VerTab);

	return(0);
}
