#include <libmesh3.h>

main()
{
	LM_mesh_struct inmesh, outmesh;
	int i, ref, p1, p2, p3;
	float x, y, z;

	/* Tente d'ouvrir le mesh en lecture, sinon fin */

	if(!LM_open_mesh("test.mesh", LM_READ, &inmesh))
		return(1);

	/* Si le mesh n'est pas de la dimension souhaitee : fin */

	if(inmesh.dimension != 3)
		return(1);

	/* Si le mesh ne contient pas de point : fin */

	if(inmesh.kw_counters[ LM_Vertices ] < 1)
		return(1);

	/* Tente d'ouvrir le mesh en ecriture, sinon fin */

	if(!LM_open_mesh("test2.mesh", LM_WRITE, &outmesh, 3))
		return(1);

	/* Boucle copiant tous les points */

	for(i=1; i<=inmesh.kw_counters[ LM_Vertices ]; i++)
	{
		LM_read_line(&inmesh, LM_Vertices, &x, &y, &z, &ref);
		LM_write_line(&outmesh, LM_Vertices, &x, &y, &z, &ref);
		printf("vertex %d : %f %f %f %d\n", i, x, y, z, ref);
	}

	/* On fait de meme pour les triangles */

	if(inmesh.kw_counters[ LM_Triangles ] < 1)
		return(1);

	for(i=1; i<=inmesh.kw_counters[ LM_Triangles ]; i++)
	{
		LM_read_line(&inmesh, LM_Triangles, &p1, &p2, &p3, &ref);
		LM_write_line(&outmesh, LM_Triangles, &p1, &p2, &p3, &ref);
		printf("triangle %d : %d %d %d %d\n", i, p1, p2, p3, ref);
	}

	/* Fermeture des fichiers */

	if(!LM_close_mesh(&inmesh))
		return(1);

	if(!LM_close_mesh(&outmesh))
		return(1);

	return(0);
}
