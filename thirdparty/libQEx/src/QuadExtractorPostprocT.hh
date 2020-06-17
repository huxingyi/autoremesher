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

#ifndef QEX_QUADEXTRACTORPOSTPROCT_HH
#define QEX_QUADEXTRACTORPOSTPROCT_HH

#include "Globals.hh"
#include "Vector.hh"

#include <vector>

namespace QEx {

template<class MeshT>
class QuadExtractorPostprocT {
    private:
        typedef typename PropMgr<MeshT>::LocalUvsPropertyManager LocalUvsPropertyManager;
        typedef typename MeshT::VertexHandle VH;
        typedef typename MeshT::FaceHandle FH;
        typedef std::vector<VH> AdjacencyList;
        typedef std::vector<AdjacencyList> Graph;

        typedef std::pair<VH, Vec2i> VERTEX_UV;
        typedef std::vector<VERTEX_UV> FACE_UV;
        typedef std::pair<int, FACE_UV> INDEXED_FACE_UV;

        class OrderFacesByVertexIndices {
            public:
                bool operator() (const INDEXED_FACE_UV &a, const INDEXED_FACE_UV &b) {

                    for (typename FACE_UV::const_iterator v_it = a.second.begin(), v_end = a.second.end(); v_it != v_end; ++v_it) {
                        assert(v_it->first.is_valid());
                        assert(v_it->first.idx() >= 0);
                    }
                    for (typename FACE_UV::const_iterator v_it = b.second.begin(), v_end = b.second.end(); v_it != v_end; ++v_it) {
                        assert(v_it->first.is_valid());
                        assert(v_it->first.idx() >= 0);
                    }

                    typename FACE_UV::const_iterator a_it, b_it, a_end, b_end;
                    for (a_it = a.second.begin(), b_it = b.second.begin(), a_end = a.second.end(), b_end = b.second.end();
                            a_it != a_end && b_it != b_end; ++a_it, ++b_it) {

                        assert(a_it->first.is_valid());
                        assert(b_it->first.is_valid());
                        const int ai = a_it->first.idx();
                        const int bi = b_it->first.idx();
                        if (ai == bi) continue;
                        else return ai < bi;
                    }
                    return a_it == a_end && b_it != b_end;
                }
        };

    public:

        /// Constructor
        QuadExtractorPostprocT(MeshT& _mesh, const LocalUvsPropertyManager &localUvsProp) :
                mesh_(_mesh), localUvsProp(localUvsProp) {
        }

        /// Destructor
        ~QuadExtractorPostprocT() {
        }

#if 0
        void viz_components();
#endif

        void ngons_to_quads();
        void output_localuvs_of_selected_faces();

    private:
        void generate_collapse_graph(Graph &out_collapseGraph);
        size_t create_consolidated_vertices_and_vertex_map(Graph &collapseGraph, std::vector<int> &out_vertex_map);
        bool compute_face_image(FH face, const std::vector<int> &vertex_map, FACE_UV &out_newFaceVertices);
        template<class OIT>
        bool simplify_face(const FACE_UV &out_newFaceVertices, OIT oit);
        FH create_face(const FACE_UV &newFaceVertices);
        size_t delete_old_vertices(const std::vector<int> &vertex_map);
        size_t delete_obsolete_faces_and_create_new_ones(const std::vector<int> &vertex_map, std::vector<FACE_UV> &out_newFaces);
        size_t remove_double_faces(std::vector<FACE_UV> &in_out_newFaces);
        size_t remove_isolated_vertices(std::vector<FACE_UV> &in_out_newFaces);
        size_t create_faces(std::vector<FACE_UV> &in_out_newFaces);

    private:

        MeshT& mesh_;
        const LocalUvsPropertyManager &localUvsProp;
};

}// namespace QEx

#if !defined(QEX_QUADEXTRACTORPOSTPROCT_C)
#define QEX_QUADEXTRACTORPOSTPROC_TEMPLATES
#include "QuadExtractorPostprocT.cc"
#endif

#endif // QEX_QUADEXTRACTORPOSTPROCT_HH defined
