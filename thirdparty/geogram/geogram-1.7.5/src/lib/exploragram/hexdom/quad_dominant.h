#ifndef H_QUAD_DOMINANT_H
#define H_QUAD_DOMINANT_H

#include <exploragram/basic/common.h>
#include <geogram/mesh/mesh.h>

    /**
     * \file exploragram/hexdom/quad_dominant.h
     * \brief Functions to extract a quad-dominant mesh from the
     *  boundary of a parameterized volumetric mesh.
     * \details All informations required to produce a hex-dom mesh 
     *  are exported into a 2D non-manifold mesh with uv coordinates
     *  This new mesh is the only input of the subsequent steps in the
     *  hex dom generation pipeline
     */


namespace GEO {

    void find_degenerate_facets(Mesh* m, vector<index_t> &degenerate);


    /**
     * \brief Creates a surfacic parameterized mesh from the boundary of 
     *  an input volumetric parameterized mesh.
     * \details The parameterization is stored in \p m in a cell corners
     *  attribute of type vec3 called "U". The input parameterization is
     *  supposed to be snapped, i.e. abs(coord - round(coord)) < 0.05 for each
     *  coordinate. The input mesh \p m also has
     *  a boolean cell attribute called "has_param" that indicates whether
     *  each cell has a valid parameterization. 
     * \param[in] m the volumetric parameterized mesh
     * \param[out] hex the generated surfacic parametrized mesh
     * \param[in] uv_name the name of a created facet corner attribute 
     *  with the 2d parameterization as a vec2 attached to the facet corners.
     * \param[in] singtri_name the name of a created facet attribute with
     *  an index_t that indicates whether the facet is singular.
     */
    void export_boundary_with_uv(Mesh* m, Mesh* hex, const char* uv_name, const char* singtri_name);
    
    void imprint(Mesh* m, const char *uv_name, const char *singular_name);

    void split_edges_by_iso_uvs(Mesh* hex, const char * uv_name, const char *singular_name);
    
    void facets_split(Mesh* m, const char *uv_name, const char *singular_name);
    
    void mark_charts(Mesh* m, const char *uv_name, const char *charts_name, const char *singular_name);
    
    void simplify_quad_charts(Mesh* m);
    
    bool export_quadtri_from_charts(Mesh* hex);

    
}
#endif
