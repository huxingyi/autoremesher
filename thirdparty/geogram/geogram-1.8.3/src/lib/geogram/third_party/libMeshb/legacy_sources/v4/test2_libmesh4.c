/* Recopie d'un fichier in.mesh dans out.meshb en utilisant la libmesh4 */

#include <libmesh4.h>

main()
{
	int i, ref, NmbVer, NmbTri, dim, v1, v2, v3;
	float x, y, z;

	/* Pointeurs sur des structures GMF pour stocker les infos sur les fichiers d'entree et sortie */

	GmfMshSct *InpMsh, *OutMsh;

	/* L'ouverture du mesh en lecture retourne un pointeur qu'il faudra passer en argument
		a toutes les routines operant sur ce fichier */

	if(!(InpMsh = GmfOpenMesh("in.mesh", GmfRead)))
		exit(1);

	/* Lecture des dimensions dans le fichier d'entree. Cela permet d'allouer la memoire des l'ouverture
		du fichier avant la lecture proprement dite */

	dim = InpMsh->dim;
	NmbVer = InpMsh->KwdTab[ GmfVertices ].NmbLin;
	NmbTri = InpMsh->KwdTab[ GmfTriangles ].NmbLin;

	printf("InpMsh : dim = %d, nbv = %d, nbt = %d\n", dim, NmbVer, NmbTri);

	/* Creation du maillage de sortie. Le parametre dimension est ajoute */

	if(!(OutMsh = GmfOpenMesh("out.mesh", GmfWrite, dim)))
		exit(1);

	for(i=0;i<NmbVer;i++)
	{
		GmfReadLine(InpMsh, GmfVertices, &x, &y, &z, &ref);
		GmfWriteLine(OutMsh, GmfVertices, x, y, z, ref);
	}

	for(i=0;i<NmbTri;i++)
	{
		GmfReadLine(InpMsh, GmfTriangles, &v1, &v2, &v3, &ref);
		GmfWriteLine(OutMsh, GmfTriangles, v1, v2, v3, ref);
	}

	/* Fermeture des maillages */

	GmfCloseMesh(InpMsh);
	GmfCloseMesh(OutMsh);
}
