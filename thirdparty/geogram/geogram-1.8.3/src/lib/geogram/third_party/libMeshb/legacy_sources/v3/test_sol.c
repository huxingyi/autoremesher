#include <libmesh3.h>

main()
{
	LM_mesh_struct mesh, mesh2;
	int i, j, entier;
	float *buffer, *ptr_buffer, reel, *ptr_entier = (float *)&entier, *ptr_reel = &reel;

	/* Tente d'ouvrir le mesh en lecture, sinon fin */

	if(!LM_open_mesh("test.sol", LM_READ, &mesh))
		return(1);

	/* Si le mesh n'est pas de la dimension souhaitee : fin */

	if(mesh.dimension != 3)
		return(1);

	/* Si le mesh ne contient pas de solutions aux points : fin */

	if(mesh.kw_counters[ LM_SolAtVertices ] < 1)
		return(1);

	/* Tente d'ouvrir le mesh en ecriture, sinon fin */

	if(!LM_open_mesh("test.solb", LM_WRITE, &mesh2, 3))
		return(1);

	/* Alloue un buffer suffisamment grand pour contenir toutes les solutions */

	if(!(buffer = malloc(mesh.kw_counters[ LM_SolAtVertices ] * (mesh.sol_headers[ LM_SolAtVertices ][1] * sizeof(float)))))
		return(1);

	/* Lecture de toutes les solutions en bloc */

	if(!LM_read_field(&mesh, LM_SolAtVertices, mesh.kw_counters[ LM_SolAtVertices ], buffer))
		return(1);

	/* Ecriture des 3 solutions pour chaque vertex : un scalaire, un vecteur et une matrice symetrique */

	if(!LM_write_field(&mesh2, LM_SolAtVertices, mesh.kw_counters[ LM_SolAtVertices ], buffer, 3, 1, 2, 3))
		return(1);

	/* Boucle affichant toutes les solutions */

	ptr_buffer = buffer;

	for(i=1; i<=mesh.kw_counters[ LM_SolAtVertices ]; i++)
	{
		printf("vertex %d : ", i);

		for(j=1;j<=mesh.sol_headers[ LM_SolAtVertices ][1];j++)
		{
			*ptr_reel = *(ptr_buffer++);
			printf("%f ",reel);
		}

		puts("");
	}

	/* Libere la memoire du buffer */

	free(buffer);

	/* Fermeture des fichiers */

	if(!LM_close_mesh(&mesh))
		return(1);

	if(!LM_close_mesh(&mesh2))
		return(1);

	return(0);
}
