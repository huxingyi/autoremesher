/*
 * Copyright 2013 Computer Graphics Group, RWTH Aachen University
 * Author: Hans-Christian Ebke <ebke@cs.rwth-aachen.de>
 *
 * This file is part of QEx.
 *
 * QEx is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * QEx is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with QEx.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * This is a minimal usage example of QEx written in C99.
 */

#include <stdio.h>
#include <stdlib.h>
#include <qex.h>

int main(int argc, char *argv[]) {
    qex_TriMesh triMesh;
    qex_QuadMesh quadMesh;
    unsigned int vertex_i, face_i;
    qex_Point3* vertex;


    triMesh.vertex_count = 4;
    triMesh.tri_count = 2;

    triMesh.vertices = (qex_Point3*)malloc(sizeof(qex_Point3) * 4);
    triMesh.tris = (qex_Tri*)malloc(sizeof(qex_Tri) * 2);
    triMesh.uvTris = (qex_UVTri*)malloc(sizeof(qex_UVTri) * 2);

    triMesh.vertices[0] = (qex_Point3) { .x = {-1, 0, -1} };
    triMesh.vertices[1] = (qex_Point3) { .x = {0, 0, 1} };
    triMesh.vertices[2] = (qex_Point3) { .x = {1, 0, 1} };
    triMesh.vertices[3] = (qex_Point3) { .x = {1, 0, 0} };

    triMesh.tris[0] = (qex_Tri) { .indices = {0, 1, 2} };
    triMesh.tris[1] = (qex_Tri) { .indices = {0, 2, 3} };
    triMesh.uvTris[0] = (qex_UVTri) { .uvs = { (qex_Point2) { .x = {-.1, -.1} }, (qex_Point2) { .x = {1.1, -.1} }, (qex_Point2) { .x = {1, 1} } } };
    triMesh.uvTris[1] = (qex_UVTri) { .uvs = { (qex_Point2) { .x = {-.1, -.1} }, (qex_Point2) { .x = {1, 1} }, (qex_Point2) { .x = {-.1, 1.1} } } };

    qex_extractQuadMesh(&triMesh, NULL, &quadMesh);

    printf("Got the following output quad mesh:\n");
    for (face_i = 0; face_i < quadMesh.quad_count; ++face_i) {
        printf("  Face %d: ", face_i);
        for (vertex_i = 0; vertex_i < 4; ++vertex_i) {
            if (vertex_i > 0) printf(", ");
            vertex = quadMesh.vertices + quadMesh.quads[face_i].indices[vertex_i];
            printf("%d (%f, %f, %f)",
                    quadMesh.quads[face_i].indices[vertex_i],
                    vertex->x[0], vertex->x[1], vertex->x[2]);
        }
        printf("\n");
    }

    free(triMesh.vertices);
    free(triMesh.tris);
    free(triMesh.uvTris);

    free(quadMesh.vertices);
    free(quadMesh.quads);

    return 0;
}
