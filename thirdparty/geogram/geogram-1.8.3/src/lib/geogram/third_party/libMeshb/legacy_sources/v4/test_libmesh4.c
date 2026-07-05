/* Recopie d'un fichier in.mesh dans out.meshb en utilisant la libmesh4 */

#include <libmesh4.h>

main()
{
	/* Variables locales pour stocker les sommets et les triangles du maillage */

	int *RefTab, *TriTab, NmbVer, NmbTri, dim;
	float *CrdTab;

	/* Pointeurs sur des structures GMF pour stocker les infos sur les fichiers d'entree et sortie */

	GmfMshSct *InpMsh, *OutMsh;

	/* L'ouverture du mesh en lecture retourne un pointeur qu'il faudra passer en argument
		a toutes les routines operant sur ce fichier */

	if(!(InpMsh = GmfOpenMesh("in.meshb", GmfRead)))
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

	/* Allocation de mes tables locales */

	RefTab = malloc(NmbVer * sizeof(int));
	CrdTab = malloc(NmbVer * 3 * sizeof(float));

	/* Lecture du champ complet dans le maillage source et ecriture dans la destination.
		Le nombre de lignes a ecrire est indique dans le cas de l'ecriture.
		Les donnees reeles et entieres sont rangees dans deux tableaux separes */

	GmfReadField(InpMsh, GmfVertices, CrdTab, RefTab);
	GmfWriteField(OutMsh, GmfVertices, NmbVer, CrdTab, RefTab);

	free(RefTab);
	free(CrdTab);

	/* On fait de meme pour les triangles, cette fois-ci le tableau de nombres reels n'est pas utilise */

	TriTab = malloc(NmbTri * 4 * sizeof(int));

	GmfReadField(InpMsh, GmfTriangles, NULL, TriTab);
	GmfWriteField(OutMsh, GmfTriangles, NmbTri, NULL, TriTab);

	free(TriTab);

	/* Fermeture des maillages */

	GmfCloseMesh(InpMsh);
	GmfCloseMesh(OutMsh);
}
