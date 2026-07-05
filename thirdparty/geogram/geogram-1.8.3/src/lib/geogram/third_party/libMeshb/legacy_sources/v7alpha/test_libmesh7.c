
/* Exemple d'utilisation de la libmesh7 : transformation de quadrangles en triangles dans un maillage surfacique */

#include <stdio.h>
#include <stdlib.h>
#include "libmesh7.h"


int main()
{
	int i, NmbVer, NmbQad, ver, dim, *RefTab, (*QadTab)[5], (*TriTab)[4];
	long long InpMsh, OutMsh;
	float (*VerTab)[3];


	/*-----------------------------------*/
	/* Ouverture du maillage "quad.mesh" */
	/*-----------------------------------*/

	if(!(InpMsh = GmfOpenMesh("quad.meshb", GmfRead, &ver, &dim)))
		return(1);

	printf("InpMsh : idx = %d, version = %d, dimension = %d\n", InpMsh, ver, dim);

	if(dim != 3)
		exit(1);

	/* Lecture des nombres d'elements et de noeuds pour l'allocation de memoire */
	NmbVer = GmfStatKwd(InpMsh, GmfVertices);
	printf("InpMsh : nmb vertices = %d\n", NmbVer);
	VerTab = malloc((NmbVer+1) * 3 * sizeof(float));
	RefTab = malloc((NmbVer+1) * sizeof(int));

	NmbQad = GmfStatKwd(InpMsh, GmfQuadrilaterals);
	printf("InpMsh : nmb quads = %d\n", NmbQad);
	QadTab = malloc((NmbQad+1) * 5 * sizeof(int));
	TriTab = malloc((NmbQad+1) * 2 * 4 * sizeof(int));

	/* Lecture des noeuds */
	GmfGetBlock(	InpMsh, GmfVertices, NULL, \
					GmfFloat, &VerTab[1][0], &VerTab[2][0], \
					GmfFloat, &VerTab[1][1], &VerTab[2][1], \
					GmfFloat, &VerTab[1][2], &VerTab[2][2], \
					GmfInt, &RefTab[1], &RefTab[2] );

	/* Lecture des quadrangles */
	GmfGetBlock(	InpMsh, GmfQuadrilaterals, NULL, \
					GmfInt, &QadTab[1][0], &QadTab[2][0], \
					GmfInt, &QadTab[1][1], &QadTab[2][1], \
					GmfInt, &QadTab[1][2], &QadTab[2][2], \
					GmfInt, &QadTab[1][3], &QadTab[2][3], \
					GmfInt, &QadTab[1][4], &QadTab[2][4] );

	/* Fermeture du maillage quad */
	GmfCloseMesh(InpMsh);


	/*-----------------------------------*/
	/* Creation du maillage en triangles */
	/*-----------------------------------*/

	if(!(OutMsh = GmfOpenMesh("tri.meshb", GmfWrite, ver, dim)))
		return(1);

	/* Ecriture du nombre de noeuds */
	GmfSetKwd(OutMsh, GmfVertices, NmbVer);

	/* Puis ecriture des noeuds */
	GmfSetBlock(OutMsh, GmfVertices, NULL, \
					GmfFloat, &VerTab[1][0], &VerTab[2][0], \
					GmfFloat, &VerTab[1][1], &VerTab[2][1], \
					GmfFloat, &VerTab[1][2], &VerTab[2][2], \
					GmfInt, &RefTab[1], &RefTab[2] );


	/*  Ecriture du nombre de triangles */
	GmfSetKwd(OutMsh, GmfTriangles, 2*NmbQad);
	printf("OutMsh : nmb triangles = %d\n", 2*NmbQad);

	/* Puis boucle de conversion des quads en deux triangles */

	for(i=1;i<=NmbQad;i++)
	{
		TriTab[i*2-1][0] = QadTab[i][0];
		TriTab[i*2-1][1] = QadTab[i][1];
		TriTab[i*2-1][2] = QadTab[i][2];
		TriTab[i*2-1][3] = QadTab[i][4];

		TriTab[i*2][0] = QadTab[i][0];
		TriTab[i*2][1] = QadTab[i][2];
		TriTab[i*2][2] = QadTab[i][3];
		TriTab[i*2][3] = QadTab[i][4];
	}

	/* Ecriture des triangles */
	GmfSetBlock(OutMsh, GmfTriangles, NULL, \
					GmfInt, &TriTab[1][0], &TriTab[2][0], \
					GmfInt, &TriTab[1][1], &TriTab[2][1], \
					GmfInt, &TriTab[1][2], &TriTab[2][2], \
					GmfInt, &TriTab[1][3], &TriTab[2][3] );

	/* Ne pas oublier de fermer le fichier */
	GmfCloseMesh(OutMsh);

	free(QadTab);
	free(TriTab);
	free(RefTab);
	free(VerTab);

	return(0);
}
