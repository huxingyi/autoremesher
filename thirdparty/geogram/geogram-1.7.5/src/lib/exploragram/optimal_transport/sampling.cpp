
/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000-2009 INRIA - Project ALICE
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  If you modify this software, you should include a notice giving the
 *  name of the person performing the modification, the date of modification,
 *  and the reason for such modification.
 *
 *  Contact: Bruno Levy - levy@loria.fr
 *
 *     Project ALICE
 *     LORIA, INRIA Lorraine, 
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX 
 *     FRANCE
 *
 *  Note that the GNU General Public License does not permit incorporating
 *  the Software into proprietary programs. 
 *
 * As an exception to the GPL, Graphite can be linked with the following (non-GPL) libraries:
 *     Qt, SuperLU, WildMagic and CGAL
 */

#include <exploragram/optimal_transport/sampling.h>
#include <geogram/voronoi/CVT.h>
#include <geogram/mesh/mesh.h>
#include <geogram/mesh/mesh_AABB.h>
#include <geogram/mesh/mesh_tetrahedralize.h>
#include <geogram/mesh/mesh_repair.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/mesh_reorder.h>
#include <geogram/basic/command_line.h>
#include <geogram/basic/permutation.h>
#include <geogram/basic/progress.h>

#ifdef GEOGRAM_WITH_VORPALINE
#include <vorpalib/voronoi/LpCVT.h>
#endif

namespace {
    using namespace GEO;

    
    /**
     * \brief Reorders the points in a Centroidal Voronoi Tesselation
     *  in such a way that continguous index ranges correspond to
     *  multiple resolutions.
     * \param[in,out] CVT the CentroidalVoronoiTesselation
     * \param[out] levels sample indices that correspond to level l are
     *   in the range levels[l] (included) ... levels[l+1] (excluded)
     * \param[in] ratio number of samples ratio between two consecutive
     *   levels
     * \param[in] threshold minimum number of samples in a level
     */
    void BRIO_reorder(
        CentroidalVoronoiTesselation& CVT,
        vector<index_t>& levels,
        double ratio,
        index_t threshold
    ) {
        vector<index_t> sorted_indices;
        compute_BRIO_order(
            CVT.nb_points(), CVT.embedding(0), sorted_indices,
            CVT.dimension(), CVT.dimension(), threshold, ratio, &levels
        );
        Permutation::apply(
            CVT.embedding(0), sorted_indices, 
            index_t(CVT.dimension() * sizeof(double))
        );
    }

    /**
     * \brief Internal implementation function for
     *   compute_hierarchical_sampling().
     * \param[in,out] CVT the CentroidalVoronoiTesselation, initialized
     *  with the volume to be sampled. On output, it stores the samples
     * \param[in] nb_samples total number of samples to generate
     * \param[out] levels sample indices that correspond to level l are
     *   in the range levels[l] (included) ... levels[l+1] (excluded)
     * \param[in] ratio number of samples ratio between two consecutive
     *   levels
     * \param[in] threshold minimum number of samples in a level
     * \param[in] b first element of the level to be generated
     * \param[in] e one position past the last element of the
     *  level to be generated
     * \param[in,out] points work vector allocated by caller,
     *  of size 3*nb_samples
     */
    void compute_hierarchical_sampling_recursive(
        CentroidalVoronoiTesselation& CVT,
        index_t nb_samples,
        vector<index_t>& levels,
        double ratio,
        index_t threshold,
        index_t b, index_t e,
        vector<double>& points
    ) {
        index_t m = b;

        // Recurse in [b...m) range
        if(e - b > threshold) {
            m = b + index_t(double(e - b) * ratio);
            compute_hierarchical_sampling_recursive(
                CVT, nb_samples, levels, ratio, threshold, b, m, points
            );
        }

        // Initialize random points in [m...e) range
        CVT.RVD()->compute_initial_sampling(&points[3 * m], e - m);

        //  Set the points in [b...e) range
        CVT.set_points(e - b, &points[0]);

        // Lock [b...m) range
        {
            for(index_t i = b; i < e; ++i) {
                if(i < m) {
                    CVT.lock_point(i);
                } else {
                    CVT.unlock_point(i);
                }
            }
        }

        Logger::div(
            std::string("Generating level ") +
            String::to_string(levels.size())
        );

        Logger::out("Sample") << " generating a level with " << e - m
            << " samples" << std::endl;

        try {
            ProgressTask progress("Lloyd", 100);
            CVT.set_progress_logger(&progress);
            CVT.Lloyd_iterations(CmdLine::get_arg_uint("opt:nb_Lloyd_iter"));
        }
        catch(const TaskCanceled&) {
        }

        try {
            ProgressTask progress("Newton", 100);
            CVT.set_progress_logger(&progress);
            CVT.Newton_iterations(CmdLine::get_arg_uint("opt:nb_Newton_iter"));
        }
        catch(const TaskCanceled&) {
        }

        levels.push_back(e);
    }

    /**
     * \brief Computes a hierarchical sampling of a volume.
     * \param[in,out] CVT the CentroidalVoronoiTesselation, initialized
     *  with the volume to be sampled. On output, it stores the samples
     * \param[in] nb_samples total number of samples to generate
     * \param[out] levels sample indices that correspond to level l are
     *   in the range levels[l] (included) ... levels[l+1] (excluded)
     * \param[in] ratio number of samples ratio between two consecutive
     *   levels
     * \param[in] threshold minimum number of samples in a level
     */
    void compute_hierarchical_sampling(
        CentroidalVoronoiTesselation& CVT,
        index_t nb_samples,
        vector<index_t>& levels,
        double ratio = 0.125,
        index_t threshold = 300
    ) {
        levels.push_back(0);
        vector<double> points(nb_samples * 3);
        compute_hierarchical_sampling_recursive(
            CVT, nb_samples, levels, ratio, threshold,
            0, nb_samples,
            points
        );
        CVT.unlock_all_points();
    }

    /**
     * \brief Computes a sampling of a volume.
     * \param[in,out] CVT the CentroidalVoronoiTesselation, initialized
     *  with the volume to be sampled. On output, it stores the samples
     * \param[in] nb_samples total number of samples to generate
     */
    void compute_single_level_sampling(
        CentroidalVoronoiTesselation& CVT,
        index_t nb_samples
    ) {

        CVT.compute_initial_sampling(nb_samples);

        try {
            ProgressTask progress("Lloyd", 100);
            CVT.set_progress_logger(&progress);
            CVT.Lloyd_iterations(CmdLine::get_arg_uint("opt:nb_Lloyd_iter"));
        }
        catch(const TaskCanceled&) {
        }

        try {
            ProgressTask progress("Newton", 100);
            CVT.set_progress_logger(&progress);
            CVT.Newton_iterations(CmdLine::get_arg_uint("opt:nb_Newton_iter"));
        }
        catch(const TaskCanceled&) {
        }
    }

    /**
     * \brief Projects the points of a volumetric sampling
     *  onto the border of the volume.
     */
    void project_sampling_on_border(
        CentroidalVoronoiTesselation& CVT
    ) {
        try {
            ProgressTask progress("Surf. Lloyd", 100);
            CVT.set_progress_logger(&progress);
            CVT.set_volumetric(false);
            CVT.Lloyd_iterations(
                CmdLine::get_arg_uint("opt:nb_Lloyd_iter") * 2
            );
        }
        catch(const TaskCanceled&) {
        }

        {

#ifdef GEOGRAM_WITH_VORPALINExxx
            CVT.done_current();
            {
                LpCentroidalVoronoiTesselation LpCVT(
                    CVT.mesh(), 0
                );
                LpCVT.set_points(CVT.nb_points(), CVT.embedding(0));
                try {
                    ProgressTask progress("LpCVT", 100);
                    LpCVT.set_progress_logger(&progress);
                    LpCVT.set_normal_anisotropy(5.0);
                    LpCVT.Newton_iterations(30, 7);
                }
                catch(const TaskCanceled&) {
                }
                CVT.set_points(LpCVT.nb_points(), LpCVT.embedding(0));
            }
            CVT.make_current();
#endif
        }

        vector<double> mg(3 * CVT.nb_points());
        vector<double> m(CVT.nb_points());
        CVT.RVD()->compute_centroids(&mg[0], &m[0]);
        for(index_t i = 0; i < CVT.nb_points(); ++i) {
            if(m[i] == 0.0) {
                CVT.unlock_point(i);
            } else {
                CVT.lock_point(i);
            }
        }

        CVT.set_volumetric(true);

        try {
            ProgressTask progress("Relax. vol.", 100);
            CVT.set_progress_logger(&progress);
            CVT.Lloyd_iterations(
                CmdLine::get_arg_uint("opt:nb_Lloyd_iter") * 2
            );
        }
        catch(const TaskCanceled&) {
        }
    }
    
}

namespace GEO {

    void recenter_mesh(const Mesh& M1, Mesh& M2) {
        double xyzmin1[3];
        double xyzmax1[3];
        double xyzmin2[3];
        double xyzmax2[3];
        double xlat[3];
        get_bbox(M1, xyzmin1, xyzmax1);
        get_bbox(M2, xyzmin2, xyzmax2);
        for(coord_index_t c=0; c<3; ++c) {
            xlat[c] = 0.5*
                ((xyzmin1[c] + xyzmax1[c]) - (xyzmin2[c] + xyzmax2[c]));
        }
        for(index_t v=0; v<M2.vertices.nb(); ++v) {
            for(coord_index_t c=0; c<3; ++c) {
                M2.vertices.point_ptr(v)[c] += xlat[c];
            }
        }
    }
    
    double mesh_tets_volume(const Mesh& M) {
        double result = 0.0;
        for(index_t t = 0; t < M.cells.nb(); ++t) {
            result += Geom::tetra_volume<3>(
                M.vertices.point_ptr(M.cells.tet_vertex(t, 0)),
                M.vertices.point_ptr(M.cells.tet_vertex(t, 1)),
                M.vertices.point_ptr(M.cells.tet_vertex(t, 2)),
                M.vertices.point_ptr(M.cells.tet_vertex(t, 3))
            );
        }
        return result;
    }

    void rescale_mesh(const Mesh& M1, Mesh& M2) {
        double xyzmin[3];
        double xyzmax[3];
        get_bbox(M2, xyzmin, xyzmax);
        double s = pow(mesh_tets_volume(M1)/mesh_tets_volume(M2), 1.0/3.0);
        for(unsigned int v=0; v<M2.vertices.nb(); ++v) {
            for(index_t c=0; c<3; ++c) {
                double gc = 0.5*(xyzmin[c]+xyzmax[c]);
                M2.vertices.point_ptr(v)[c] =
                    gc + s * (M2.vertices.point_ptr(v)[c] - gc);
            }
        }
    }

    enum DensityFunction {
        DENSITY_X=0,
        DENSITY_Y=1,
        DENSITY_Z=2,
        DENSITY_R,
        DENSITY_SIN,
        DENSITY_DIST
    };
    
    void set_density(
        Mesh& M, double mass1, double mass2, const std::string& function_str_in,
        Mesh* density_distance_reference
    ) {
        std::string function_str = function_str_in;

        if(mass1 == mass2) {
            return;
        }

        bool minus = false;
        if(function_str.length() > 1 && function_str[0] == '-') {
            minus = true;
            function_str = function_str.substr(1,function_str.length()-1);
        }
        double density_pow = 1.0;
        {
            std::size_t found = function_str.find('^');
            if(found != std::string::npos) {
                std::string pow_str =
                    function_str.substr(found+1, function_str.length()-found-1);
                density_pow = String::to_double(pow_str);
                function_str = function_str.substr(0,found);
            }
        }

        Logger::out("OTM")
            << "Using density: "
            << (minus ? "-" : "+")
            << function_str << "^"
            << density_pow
            << " rescaled to ("
            << mass1 << "," << mass2
            << ")"
            << std::endl;
        
        DensityFunction function;
        if(function_str == "X") {
            function = DENSITY_X;
        } else if(function_str == "Y") {
            function = DENSITY_Y;            
        } else if(function_str == "Z") {
            function = DENSITY_Z;            
        } else if(function_str == "R") {
            function = DENSITY_R;            
        } else if(function_str == "sin") {
            function = DENSITY_SIN;
        } else if(function_str == "dist") {
            function = DENSITY_DIST;
        } else {
            Logger::err("OTM") << function_str << ": no such density function"
                               << std::endl;
            return;
        }
        
        Attribute<double> mass(M.vertices.attributes(),"weight");

        switch(function) {
        case DENSITY_X:
        case DENSITY_Y:
        case DENSITY_Z: {
            for(index_t v=0; v<M.vertices.nb(); ++v) {
                mass[v] = M.vertices.point_ptr(v)[index_t(function)];
            }
        } break;
        case DENSITY_R: {
            double xyz_min[3];
            double xyz_max[3];
            get_bbox(M, xyz_min, xyz_max);
            for(index_t v=0; v<M.vertices.nb(); ++v) {
                double r=0;
                const double* p = M.vertices.point_ptr(v);
                for(coord_index_t c=0; c<3; ++c) {
                    r += geo_sqr(p[c] - 0.5*(xyz_min[c] + xyz_max[c]));
                }
                r = ::sqrt(r);
                mass[v] = r;
            }
        } break;
        case DENSITY_SIN: {
            double xyz_min[3];
            double xyz_max[3];
            get_bbox(M, xyz_min, xyz_max);
            for(index_t v=0; v<M.vertices.nb(); ++v) {
                double f = 1.0;
                const double* p = M.vertices.point_ptr(v);
                for(coord_index_t c=0; c<3; ++c) {
                    double coord =
                        (p[c] - xyz_min[c]) / (xyz_max[c] - xyz_min[c]);
                    f *= sin(coord *  M_PI * 2.0 * 2.0);
                }
                mass[v] = f;                
            }
        } break;
        case DENSITY_DIST: {
            if(density_distance_reference != nullptr)  {
                MeshFacetsAABB AABB(*density_distance_reference);
                for(index_t v=0; v<M.vertices.nb(); ++v) {
                    mass[v] =
                        ::sqrt(AABB.squared_distance(
                                   vec3(M.vertices.point_ptr(v)))
                        );
                }
            } else {
                MeshFacetsAABB AABB(M);
                for(index_t v=0; v<M.vertices.nb(); ++v) {
                    mass[v] = ::sqrt(
                        AABB.squared_distance(vec3(M.vertices.point_ptr(v)))
                    );
                }
            }
        } break;
        }

        // Compute min and max mass
        double mass_min = Numeric::max_float64();
        double mass_max = Numeric::min_float64();
        for(index_t v=0; v<M.vertices.nb(); ++v) {
            mass_min = std::min(mass_min, mass[v]);
            mass_max = std::max(mass_max, mass[v]);
        }

        // Normalize mass, apply power, and rescale to (mass1 - mass2)
        for(index_t v=0; v<M.vertices.nb(); ++v) {
            double f = (mass[v] - mass_min) / (mass_max - mass_min);
            if(minus) {
                f = 1.0 - f;
            }
            f = ::pow(f,density_pow);
            mass[v] = mass1 + f*(mass2 - mass1);
        }
    }

    void sample(
        CentroidalVoronoiTesselation& CVT,
        index_t nb_points, bool project_on_border,
        bool BRIO, bool multilevel, double ratio,
        vector<index_t>* levels_out
    ) {
        vector<index_t> levels;
        multilevel = multilevel | BRIO;
        if(CmdLine::get_arg_bool("RVD_iter") && multilevel) {
            Logger::warn("OTM") << "Deactivating multilevel mode" << std::endl;
            Logger::warn("OTM") << "(because RVD_iter is set)" << std::endl;
            multilevel = false;
        }
        if(multilevel) {
            if(BRIO) {
                compute_single_level_sampling(CVT, nb_points);
                BRIO_reorder(CVT, levels, ratio, 300);
            } else {
                compute_hierarchical_sampling(
                    CVT, nb_points,levels,ratio
                );
            }
        } else {
            compute_single_level_sampling(CVT, nb_points);
        }
        if(levels_out != nullptr) {
            *levels_out = levels;
        }
        if(project_on_border) {
            project_sampling_on_border(CVT);
        }
    }
    
    
}
