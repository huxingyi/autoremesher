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

#define QEX_QUADEXTRACTORPOSTPROCT_C

#include "QuadExtractorPostprocT.hh"

#include <map>
#include <stack>
#include <functional>
#include <stdlib.h>
#include <cstdlib>

namespace QEx {

template<class MeshT>
void QuadExtractorPostprocT<MeshT>::generate_collapse_graph(Graph &out_collapseGraph) {
    out_collapseGraph.clear();
    out_collapseGraph.resize(mesh_.n_vertices());

    for (typename MeshT::FaceIter f_it = mesh_.faces_begin(), f_end = mesh_.faces_end();
            f_it != f_end; ++f_it) {

        typedef std::multimap<Vec2i, VH> CollapseGroups;
        CollapseGroups collapseGroups;

        /*
         * Collect face's vertices ordered by their UV coordinates.
         */
        for (typename MeshT::FHIter fh_it = mesh_.fh_begin(*f_it), fh_end = mesh_.fh_end(*f_it);
                fh_it != fh_end; ++fh_it) {

            const Vec2i &localUv = localUvsProp[*fh_it];
            collapseGroups.insert(std::pair<Vec2i, VH>(localUv, mesh_.to_vertex_handle(*fh_it)));
        }

        /*
         * Make vertices with same UV coordinates inside face adjacent.
         */
        for (typename CollapseGroups::iterator cg_it = collapseGroups.begin(), last_cg_it; cg_it != collapseGroups.end(); last_cg_it = cg_it++) {
            if (cg_it == collapseGroups.begin()) continue;

#ifndef NDEBUG
            if (!(cg_it->second.idx() >= 0 && (size_t)cg_it->second.idx() < out_collapseGraph.size())) {
                std::cerr << "Assertion cg_it->second.idx() > 0 && (size_t)cg_it->second.idx() < collapseGraph.size() is going to fail." << std::endl
                        << "cg_it->second.idx(): " << cg_it->second.idx() << ", collapseGraph.size(): " << out_collapseGraph.size() << std::endl;
            }
#endif
            assert(cg_it->second.idx() >= 0 && (size_t)cg_it->second.idx() < out_collapseGraph.size());
            if (cg_it->first == last_cg_it->first) {
                out_collapseGraph[last_cg_it->second.idx()].push_back(cg_it->second);
                out_collapseGraph[cg_it->second.idx()].push_back(last_cg_it->second);
            }
        }
    }
}

#if 0
template<class MeshT>
void QuadExtractorPostprocT<MeshT>::viz_components() {
    /*
     * Construct collapse graph.
     */
    Graph collapseGraph;
    generate_collapse_graph(collapseGraph);

    /*
     * Traverse components and color vertices.
     */
    mesh_.request_vertex_colors();

    // Initialize vertex color to black;
    for (typename MeshT::VertexIter v_it = mesh_.vertices_begin(), v_end = mesh_.vertices_end(); v_it != v_end; ++v_it)
        mesh_.set_color(*v_it, Vec4f(0, 0, 0, 1));

    // Color Components
    ACG::ColorGenerator colorGenerator;
    int vertexIdx = 0;
    for (typename Graph::const_iterator root_node_it = collapseGraph.begin(); root_node_it != collapseGraph.end(); ++root_node_it, ++vertexIdx) {
        if (root_node_it->empty()) continue;

        std::stack<VH> bfs;
        bfs.push(mesh_.vertex_handle(vertexIdx));
        ACG::Vec4f component_color = colorGenerator.generateNextColor();

        while (!bfs.empty()) {
            VH cur = bfs.top(); bfs.pop();

            mesh_.set_color(cur, component_color);

            for (typename AdjacencyList::const_iterator it = collapseGraph[cur.idx()].begin(), it_end = collapseGraph[cur.idx()].end();
                    it != it_end; ++it) {
                bfs.push(*it);
            }

            // Mark as visited by clearing adjacency list.
            collapseGraph[cur.idx()].clear();
        }
    }
}
#endif

template<class MeshT>
size_t QuadExtractorPostprocT<MeshT>::create_consolidated_vertices_and_vertex_map(Graph &collapseGraph, std::vector<int> &out_vertex_map) {

    /*
     * Start off with identity map.
     */
    out_vertex_map.clear();
    out_vertex_map.reserve(mesh_.n_vertices());
    for (size_t i = 0; i < mesh_.n_vertices(); ++i) out_vertex_map.push_back(i);

    size_t createdVertices = 0;
    int vertexIdx = 0;
    for (typename Graph::const_iterator root_node_it = collapseGraph.begin(); root_node_it != collapseGraph.end(); ++root_node_it, ++vertexIdx) {
        if (root_node_it->empty()) continue;

        typename MeshT::Point position(0, 0, 0);

        const VH newVertex = mesh_.add_vertex(position);
        mesh_.set_color(newVertex, typename MeshT::Color(0.0, 1.0, 0.0, 1.0));
        mesh_.status(newVertex).set_selected(true);
        ++createdVertices;
        const int newVertexIdx = newVertex.idx();

        typedef std::pair<VH, int> PVHI;
        std::stack<PVHI> bfs;
        bfs.push(PVHI(mesh_.vertex_handle(vertexIdx), newVertexIdx));

        int magnitude = 0;

        while (!bfs.empty()) {
            PVHI cur = bfs.top(); bfs.pop();

            out_vertex_map[cur.first.idx()] = cur.second;
            position += mesh_.point(cur.first);
            ++magnitude;

            for (typename AdjacencyList::const_iterator it = collapseGraph[cur.first.idx()].begin(), it_end = collapseGraph[cur.first.idx()].end();
                    it != it_end; ++it) {
                bfs.push(PVHI(*it, cur.second));
            }

            // Mark as visited by clearing adjacency list.
            collapseGraph[cur.first.idx()].clear();
        }

        // FIXME: Uncomment this to set correct positions.
        mesh_.point(newVertex) = position / magnitude;
    }

    return createdVertices;
}

template<class MeshT>
bool QuadExtractorPostprocT<MeshT>::compute_face_image(FH face, const std::vector<int> &vertex_map, std::vector<std::pair<VH, Vec2i> > &out_newFaceVertices) {

    out_newFaceVertices.clear();
    bool identity = true;

    for (typename MeshT::FaceHalfedgeIter fh_it = mesh_.fh_begin(face), fh_end = mesh_.fh_end(face);
            fh_it != fh_end; ++fh_it) {

        assert(mesh_.to_vertex_handle(*fh_it).idx() >= 0 &&
               (size_t)mesh_.to_vertex_handle(*fh_it).idx() < vertex_map.size());
        const int preImage = mesh_.to_vertex_handle(*fh_it).idx();
        const int image = vertex_map[preImage];
        if (preImage != image) identity = false;
        assert(image >= 0 && (size_t)image < mesh_.n_vertices());
        const VH vh = mesh_.vertex_handle(image);
        assert(vh.is_valid());
        out_newFaceVertices.push_back(std::pair<VH, Vec2i>(vh, localUvsProp[*fh_it]));
    }

    return identity;
}

template<class MeshT>
template<class OIT>
bool QuadExtractorPostprocT<MeshT>::simplify_face(const std::vector<std::pair<VH, Vec2i> > &out_newFaceVertices, OIT faceOutIt) {

    bool result = false;

    typedef std::pair<VH, Vec2i> PVHV2I;
    typedef std::vector<PVHV2I> VEC_PVH2I;

    std::stack<VEC_PVH2I> bfs;
    bfs.push(out_newFaceVertices);

    while (!bfs.empty()) {
        VEC_PVH2I currentFace = bfs.top(); bfs.pop();

        ptrdiff_t sequenceLength = static_cast<ptrdiff_t>(currentFace.size());
        const ptrdiff_t idealLength = sequenceLength / 2;
        typename VEC_PVH2I::iterator seq_begin = currentFace.begin(),
                seq_end = currentFace.end();

        for (typename VEC_PVH2I::iterator v1_it = currentFace.begin(), v1_end = currentFace.end() - 1; v1_it != v1_end; ++v1_it) {
            for (typename VEC_PVH2I::iterator v2_it = v1_it + 1, v2_end = currentFace.end(); v2_it != v2_end; ++v2_it) {
                if (v1_it->first != v2_it->first) continue;

                const ptrdiff_t dist = std::distance(v1_it, v2_it);
                assert(dist > 0);
                if (std::labs(idealLength - dist) < std::labs(idealLength - sequenceLength)) {
                    sequenceLength = dist;
                    seq_begin = v1_it;
                    seq_end = v2_it;
                }

                if (std::labs(idealLength - (currentFace.size() - dist)) < std::labs(idealLength - sequenceLength)) {
                    assert(static_cast<size_t>(dist) <= currentFace.size());
                    sequenceLength = currentFace.size() - dist;
                    seq_begin = v2_it;
                    seq_end = v1_it;
                }
            }
        }

        if (seq_end == currentFace.end()) {
            assert(seq_begin == currentFace.begin());
            assert(static_cast<size_t>(sequenceLength) == currentFace.size());
#ifndef XNDEBUG
            if (sequenceLength != 4 && sequenceLength > 2) {
                std::cerr << "WARNING: Subsequence length was " << sequenceLength << " out of " << currentFace.size() << std::endl;
                std::cerr << "Entire sequence: ";
                for (typename VEC_PVH2I::iterator v1_it = currentFace.begin(), v1_end = currentFace.end(); v1_it != v1_end; v1_it++) {
                    std::cerr << " " << v1_it->first.idx();
                }
                std::cerr << std::endl;
                std::cerr << "Subsequence: [" << std::distance(currentFace.begin(), seq_begin)
                    << ", " << std::distance(currentFace.begin(), seq_end) << ")" << std::endl;
            }
#endif

            if (sequenceLength > 2) {
                *faceOutIt++ = currentFace;
                result = true;
            }
        } else {
            // Split face.

            assert(static_cast<size_t>(sequenceLength) < currentFace.size());
            std::rotate(currentFace.begin(), seq_end, currentFace.end());

            bfs.push(VEC_PVH2I(currentFace.end() - sequenceLength, currentFace.end()));
            currentFace.resize(currentFace.size() - sequenceLength);
            bfs.push(currentFace);
        }
    }

    return result;
}

template<class MeshT>
typename QuadExtractorPostprocT<MeshT>::FH QuadExtractorPostprocT<MeshT>::create_face(const std::vector<std::pair<VH, Vec2i> > &newFaceVertices) {

    std::vector<VH> face;
    face.reserve(newFaceVertices.size());
    for (typename std::vector<std::pair<VH, Vec2i> >::const_iterator it = newFaceVertices.begin(); it != newFaceVertices.end(); ++it)
        face.push_back(it->first);

#ifndef NDEBUG
    /*
     * Extensive checking.
     */
    for (typename std::vector<VH>::const_iterator v1_it = face.begin(), v0_it = v1_it++, v_end = face.end();
            v0_it != v_end; ++v0_it, (++v1_it == v_end ? v1_it = face.begin() : v1_it)) {
        assert(!mesh_.status(*v0_it).deleted());
        assert(!mesh_.status(*v1_it).deleted());

        for (typename std::vector<VH>::const_iterator v2_it = v0_it + 1; v2_it != v_end; ++v2_it) {
            assert(*v0_it != *v2_it);
            assert(v0_it->idx() != v2_it->idx());
        }

        assert(v0_it->idx() != v1_it->idx());

        if (mesh_.is_isolated(*v0_it)) continue;

        for (typename MeshT::VOHIter voh_it = mesh_.voh_begin(*v0_it), voh_end = mesh_.voh_end(*v0_it);
                voh_it != voh_end; ++voh_it) {
            if (mesh_.status(*voh_it).deleted()) continue;

            if (mesh_.to_vertex_handle(*voh_it) == *v1_it) {
                assert(mesh_.is_boundary(*voh_it));
            }
        }
    }
#endif

    const typename MeshT::FaceHandle fh = mesh_.add_face(face);
    //assert(fh.is_valid());
    if (!fh.is_valid()) return fh;

    /*
     * TODO: Transfer Local UV property.
     */

#ifndef NDEBUG
    if (newFaceVertices.size() != 4)
        mesh_.set_color(fh, typename MeshT::Color(1.0, 0, 0, 1.0));
    else
        mesh_.set_color(fh, typename MeshT::Color(1.0, 1.0, 1.0, 1.0));
#endif

    return fh;
}

template<class MeshT>
size_t QuadExtractorPostprocT<MeshT>::delete_old_vertices(const std::vector<int> &vertex_map) {
    int idx = 0;
    size_t deletedVertices = 0;

    for (std::vector<int>::const_iterator it = vertex_map.begin(), it_end = vertex_map.end(); it != it_end; ++it, ++idx) {
        if (idx != *it) {
            mesh_.delete_vertex(mesh_.vertex_handle(idx), false);
            ++deletedVertices;
        }
    }

    return deletedVertices;
}

template<class MeshT>
size_t QuadExtractorPostprocT<MeshT>::delete_obsolete_faces_and_create_new_ones(const std::vector<int> &vertex_map, std::vector<std::vector<std::pair<VH, Vec2i> > > &out_newFaces) {

    typedef std::pair<VH, Vec2i> PVHV2I;

    size_t deletedFaces = 0;

    out_newFaces.clear();
    for (typename MeshT::FaceIter f_it = mesh_.faces_begin(), f_end = mesh_.faces_end();
            f_it != f_end; ++f_it) {

        std::vector<PVHV2I> newFaceVertices; newFaceVertices.reserve(4);

        /*
         * Determine image of face.
         * Faces with identity mapping can stay as they were (i.e. we skip them).
         */
        if (compute_face_image(*f_it, vertex_map, newFaceVertices)) continue;

        /*
         * Simplify image of face.
         */
        if (!simplify_face(newFaceVertices, std::back_inserter(out_newFaces)))
            ++deletedFaces;

        /*
         * Remove old face.
         */
        assert(f_it->is_valid());
        mesh_.delete_face(*f_it, false);
    }

    /*
     * Delete edges without faces.
     */
    for (typename MeshT::EdgeIter e_it = mesh_.edges_begin(), e_end = mesh_.edges_end();
            e_it != e_end; ++e_it) {

        typename MeshT::HalfedgeHandle
            heh0 = mesh_.halfedge_handle(*e_it, 0),
            heh1 = mesh_.halfedge_handle(*e_it, 1);

        if (mesh_.is_boundary(heh0) && mesh_.is_boundary(heh1))
            mesh_.delete_edge(*e_it, false);
    }

#ifndef NDEBUG
    for (typename std::vector<std::vector<PVHV2I> >::iterator f_it = out_newFaces.begin(), f_end = out_newFaces.end();
            f_it != f_end; ++f_it) {

        for (typename std::vector<PVHV2I>::iterator v_it = f_it->begin(), v_end = f_it->end(); v_it != v_end; ++v_it) {
            for (typename std::vector<PVHV2I>::iterator v2_it = v_it + 1; v2_it != v_end; ++v2_it) {
                assert(v_it->first != v2_it->first);
                assert(v_it->first.idx() != v2_it->first.idx());
            }
        }
    }
#endif

    return deletedFaces;
}

template<class MeshT>
size_t QuadExtractorPostprocT<MeshT>::remove_double_faces(std::vector<FACE_UV> &in_out_newFaces) {

    if (in_out_newFaces.size() <= 1) return 0;

    typedef std::vector<INDEXED_FACE_UV> INDEXED_FACE_UV_VEC;
    typedef std::vector<FACE_UV> FACE_UV_VEC;

    /*
     * Normalize faces;
     */
    INDEXED_FACE_UV_VEC normalized_faces;

    size_t face_idx = 0;
    for (typename FACE_UV_VEC::const_iterator f_it = in_out_newFaces.begin(), f_end = in_out_newFaces.end(); f_it != f_end; ++f_it, ++face_idx) {

        assert(!f_it->empty());

        /*
         * Find smallest index.
         */
        typename FACE_UV::const_iterator v_min = f_it->begin();
        for (typename FACE_UV::const_iterator v_it = f_it->begin(), v_end = f_it->end(); v_it != v_end; ++v_it) {
            assert(v_it->first.is_valid());
            if (v_it->first.idx() < v_min->first.idx())
                v_min = v_it;
        }

        /*
         * Rotate face so that it starts with the vertex with the smallest index.
         * begin, ..., min-1, min, min+1, ..., end => min, min+1, ..., end, begin, ..., min-1
         */
        FACE_UV normalizedFace;
        assert(v_min >= f_it->begin() && v_min < f_it->end());
        std::copy(v_min, f_it->end(), std::back_inserter(normalizedFace));
        std::copy(f_it->begin(), v_min, std::back_inserter(normalizedFace));

        assert(normalizedFace.size() == f_it->size());

        /*
         * Bring face into normalized orientation.
         */
        if (normalizedFace.size() > 1 && normalizedFace.back().first < normalizedFace[1].first)
            std::reverse(normalizedFace.begin(), normalizedFace.end());

        normalized_faces.push_back(INDEXED_FACE_UV(face_idx, normalizedFace));

    }


    /*
     * Sort normalized faces lexicographically.
     */

    std::sort(normalized_faces.begin(), normalized_faces.end(), OrderFacesByVertexIndices());


    assert(normalized_faces.size() == in_out_newFaces.size());

    /*
     * Collect indices of duplicates.
     */
    std::vector<int> kill_list;
    for (typename INDEXED_FACE_UV_VEC::const_iterator it = normalized_faces.begin(), last_it = it++, it_end = normalized_faces.end();
            it != it_end; ++it, ++last_it) {

        if (it->second == last_it->second) kill_list.push_back(it->first);
    }

    /*
     * Sort in descending order.
     */
    std::sort(kill_list.begin(), kill_list.end(), std::greater<int>());

    /*
     * Remove duplicates back to front.
     */
    for (std::vector<int>::const_iterator it = kill_list.begin(); it != kill_list.end(); ++it) {
        in_out_newFaces.erase(in_out_newFaces.begin() + *it);
    }

    return kill_list.size();
}

template<class MeshT>
size_t QuadExtractorPostprocT<MeshT>::remove_isolated_vertices(std::vector<std::vector<std::pair<VH, Vec2i> > > &in_out_newFaces) {

    std::vector<bool> used; used.resize(mesh_.n_vertices(), false);

    /*
     * Mark vertices used by old faces.
     */
    for (typename MeshT::FaceIter f_it = mesh_.faces_begin(), f_end = mesh_.faces_end(); f_it != f_end; ++f_it) {
        if (mesh_.status(*f_it).deleted()) continue;
#ifndef NDEBUG
        int innerCnt = 0;
#endif
        for (typename MeshT::FVIter fv_it = mesh_.fv_begin(*f_it), fv_end = mesh_.fv_end(*f_it); fv_it != fv_end; ++fv_it) {
            assert(fv_it->idx() >= 0 && (size_t)fv_it->idx() < used.size());
            used[fv_it->idx()] = true;
#ifndef NDEBUG
            if (++innerCnt == 10) {
                std::cerr << "Face " << f_it->idx() << " with >= 10 vertices? That seems rather unlikely." << std::endl;
            }

            if (innerCnt >= 10 && innerCnt <= 20) {
                std::cerr << "fv_it->idx(): " << fv_it->idx() << std::endl;
            } else if (innerCnt > 20) {
                abort();
            }
#endif
        }
    }

    /*
     * Mark vertices used by new faces.
     */
    typedef std::pair<VH, Vec2i> VERTEX;
    typedef std::vector<VERTEX> FACE;
    typedef std::vector<FACE> FACE_VEC;
    for (typename FACE_VEC::const_iterator f_it = in_out_newFaces.begin(), f_end = in_out_newFaces.end(); f_it != f_end; ++f_it) {
        for (typename FACE::const_iterator v_it = f_it->begin(), v_end = f_it->end(); v_it != v_end; ++v_it) {
            assert(v_it->first.idx() >= 0 && (size_t)v_it->first.idx() < used.size());
            used[v_it->first.idx()] = true;
        }
    }

    /*
     * Delete unmarked vertices.
     */
    int deletedVertices = 0;
    int idx = 0;
    for (std::vector<bool>::const_iterator it = used.begin(); it != used.end(); ++it, ++idx) {
        if (!*it) {
            mesh_.delete_vertex(mesh_.vertex_handle(idx));
            ++deletedVertices;
        }
    }

    return deletedVertices;
}

template<class MeshT>
size_t QuadExtractorPostprocT<MeshT>::create_faces(std::vector<std::vector<std::pair<VH, Vec2i> > > &in_out_newFaces) {
    /*
     * Create a dictionary that maps edges to their incident faces.
     *
     * We'll use it later to make sure we don't add faces that would result in
     * non-manifold configuration, i.e. edges with more than two incident faces.
     */
    typedef std::pair<VH, VH> EDGE;
    typedef std::pair<FH, FH> INCIDENT_FACES;
    typedef std::map<EDGE, INCIDENT_FACES> EDGE_INCIDENTS;

    EDGE_INCIDENTS edge_incidents;

    for (typename MeshT::EdgeIter it = mesh_.edges_begin(), it_end = mesh_.edges_end(); it != it_end; ++it) {
        if (mesh_.status(*it).deleted()) continue;
        typename MeshT::HalfedgeHandle
            he0 = mesh_.halfedge_handle(*it, 0),
            he1 = mesh_.halfedge_handle(*it, 1);

        VH v0 = mesh_.to_vertex_handle(he0);
        VH v1= mesh_.to_vertex_handle(he1);

        if (v0.idx() > v1.idx()) {
            std::swap(v0, v1);
            std::swap(he0, he1);
        }

        FH f0, f1;

        if (!mesh_.is_boundary(he0)) f0 = mesh_.face_handle(he0);
        if (!mesh_.is_boundary(he1)) f1 = mesh_.face_handle(he1);

        if (!f0.is_valid() && f1.is_valid()) std::swap(f0, f1);

        std::pair<typename EDGE_INCIDENTS::iterator, bool> insertRes =
                edge_incidents.insert(std::pair<EDGE, INCIDENT_FACES>(EDGE(v0, v1), INCIDENT_FACES(f0, f1)));
        if (insertRes.second == false) {
#ifndef NDEBUG
            std::cerr << "Double edge case." << std::endl;
#endif
            if (!insertRes.first->second.first.is_valid())
                insertRes.first->second = INCIDENT_FACES(f0, f1);
            else if (!insertRes.first->second.second.is_valid())
                insertRes.first->second.second = f0;
        }
    }

    /*
     * Create faces but only those that don't introduce non-manifoldness.
     */
    typedef std::pair<VH, Vec2i> VH_LOCALUV;
    typedef std::vector<VH_LOCALUV> FACE;
    typedef std::vector<FACE> FACE_VEC;

    size_t skippedFaces = 0;

    for (typename FACE_VEC::const_iterator f_it = in_out_newFaces.begin(), f_it_end = in_out_newFaces.end();
            f_it != f_it_end; ++f_it) {

        if (f_it->size() <= 1) {
            assert(f_it->size() > 1);
            std::cerr << __FILE__": Warning: New face with less than two vertices. Skipping." << std::endl;
            ++skippedFaces;
            continue;
        }

        bool skip = false;

        /*
         * Check whether any of the edges is incident to two faces already.
         * v1_it wraps around for the last iteraton.
         */
#ifndef NDEBUG
        size_t edge_count = 0;
#endif
        for (typename FACE::const_iterator v1_it = f_it->begin(), v0_it = v1_it++, v_end = f_it->end();
                v0_it != v_end; ++v0_it, ((++v1_it == v_end) ? (v1_it = f_it->begin()) : v1_it)) {

#ifndef NDEBUG
            ++edge_count;
#endif

            VH v0 = v0_it->first, v1 = v1_it->first;

            assert(!mesh_.status(v0).deleted());
            assert(!mesh_.status(v1).deleted());

            /*
             * Test: Boundary vertex?
             */
            if (!mesh_.is_boundary(v0) || !mesh_.is_boundary(v1)) {
                skip = true;
                if (!mesh_.is_boundary(v0))
                    mesh_.set_color(v0, typename MeshT::Color(1.0, 0.0, 0.0, 1.0));
                else
                    mesh_.set_color(v1, typename MeshT::Color(1.0, 0.0, 0.0, 1.0));
                std::cerr << "create_faces: Skipping. Reason: Non-boundary vertex." << std::endl;
                break;
            }

            if (!mesh_.is_isolated(v0)) {
                for (typename MeshT::VOHIter voh_it = mesh_.voh_begin(v0), voh_end = mesh_.voh_end(v0);
                        voh_it != voh_end; ++voh_it) {
                    if (mesh_.status(*voh_it).deleted()) continue;
                    if (mesh_.to_vertex_handle(*voh_it) == v1 &&
                            !mesh_.is_boundary(*voh_it)) {
                        skip = true;
    #ifndef NDEBUG
                        // Highlight perpetrators.
                        mesh_.set_color(mesh_.face_handle(*voh_it), typename MeshT::Color(1.0, 0.0, 0.0, 1.0));
    #endif
    #ifdef NDEBUG
                        break;
    #endif
                    }
                }
            }

            /*
             * Test: Boundary edge?
             */
            if (v0.idx() > v1.idx()) std::swap(v0, v1);
            typename EDGE_INCIDENTS::iterator edge_incident_it = edge_incidents.find(EDGE(v0, v1));

            if (
                    // If it's not a new edge and ...
                    edge_incident_it != edge_incidents.end() &&

                    // ... if it has two incident faces, we can't add the new face.
                    edge_incident_it->second.first.is_valid() && edge_incident_it->second.second.is_valid()) {

#ifndef NDEBUG
                // Highlight perpetrators.
                mesh_.set_color(edge_incident_it->second.first, typename MeshT::Color(1.0, 0.0, 0.0, 1.0));
                mesh_.set_color(edge_incident_it->second.second, typename MeshT::Color(1.0, 0.0, 0.0, 1.0));
#endif

                skip = true;
                std::cerr << "create_faces: Skipping. Reason: Non-boundary edge." << std::endl;
                break;
            } else if (skip) {
                std::cerr << "First check recognized non-manifold, second one didn't!" << std::endl;
            }

            /*
             * Redundant boundary check.
             */
            if (!mesh_.is_isolated(v0)) {
                for (typename MeshT::VOHIter voh_it = mesh_.voh_begin(v0), voh_end = mesh_.voh_end(v0); voh_it != voh_end; ++voh_it) {
                    assert(voh_it->is_valid());
                    if (mesh_.to_vertex_handle(*voh_it) == v1) {
                        assert(mesh_.is_boundary(*voh_it) || mesh_.is_boundary(mesh_.opposite_halfedge_handle(*voh_it)));
                    }
                }
            }
        }
#ifndef NDEBUG
        if (!skip && edge_count != f_it->size()) {
            std::cerr << "Assertion is going to fail. edge_count = " << edge_count << ", f_it->size() = " << f_it->size() << std::endl;
        }
#endif
        assert(skip || edge_count == f_it->size());
        /*
         * Check end.
         */

        if (skip) {
            ++skippedFaces;
        } else {
            FH newFh = create_face(*f_it);
            if (!newFh.is_valid()) {
                std::cerr << "\x1b[41mcreate_face() failed. Most probable reason: Non-manifold face not filtered out.\x1b[0m" << std::endl;
                std::cerr << "Failing face vertices were: ";
                for (typename FACE::const_iterator v_it = f_it->begin(), v_end = f_it->end(); v_it != v_end; ++v_it) {
                    if (v_it != f_it->begin()) std::cerr << ", ";
                    std::cerr << v_it->first.idx();
                }
                std::cerr << std::endl;
                continue;
            }
            assert(newFh.is_valid());
#ifndef NDEBUG
            mesh_.set_color(newFh, typename MeshT::Color(0, .4, 0, 1.0));
#endif

            /*
             * Register new face on all of its edges.
             */
            for (typename FACE::const_iterator v1_it = f_it->begin(), v0_it = v1_it++, v_end = f_it->end();
                    v0_it != v_end; ++v0_it, (++v1_it == v_end) ? (v1_it = f_it->begin()) : v1_it) {

                VH v0 = v0_it->first, v1 = v1_it->first;

                if (v0.idx() > v1.idx()) std::swap(v0, v1);

                /*
                 * Try to insert edge.
                 */
                std::pair<typename EDGE_INCIDENTS::iterator, bool> insert_res = edge_incidents.insert(
                        typename EDGE_INCIDENTS::value_type(EDGE(v0, v1), INCIDENT_FACES(newFh, FH())));

                if (insert_res.second == true) {

                    /*
                     * Edge didn't exist before. -> We're done.
                     */
                    continue;
                } else {

                    /*
                     * Edge already exists. Add new face.
                     */
                    const typename EDGE_INCIDENTS::iterator edge_incident_it = insert_res.first;

                    if (!edge_incident_it->second.first.is_valid()) {
                        edge_incident_it->second.first = newFh;
                    } else if (!edge_incident_it->second.second.is_valid()) {
                        edge_incident_it->second.second = newFh;
                    } else {
                        std::cerr << __FILE__": Logic error." << std::endl;
                        abort();
                    }
                }
            }
            /*
             * Registration end.
             */
        }
    }

    return skippedFaces;
}

template<class MeshT>
void QuadExtractorPostprocT<MeshT>::ngons_to_quads() {

    /*
     * Color existing faces in gray.
     */
#ifndef NDEBUG
    mesh_.request_face_colors();
    for (typename MeshT::FaceIter f_it = mesh_.faces_begin(), f_end = mesh_.faces_end(); f_it != f_end; ++f_it)
        mesh_.set_color(*f_it, typename MeshT::Color(.4, .4, .4, 1.0));
#endif

    mesh_.request_vertex_colors();
    for (typename MeshT::VertexIter v_it = mesh_.vertices_begin(), v_end = mesh_.vertices_end(); v_it != v_end; ++v_it)
        mesh_.set_color(*v_it, typename MeshT::Color(0, 0, 0, 1.0));

    /*
     * Construct collapse graph.
     */
    Graph collapseGraph;
    generate_collapse_graph(collapseGraph);

    /*
     * Generate new vertices.
     * Generate (non-injective) mapping old vertex -> new vertex
     */
    std::vector<int> vertex_map;
#ifndef NDEBUG
    const size_t createdVertices =
#endif
            create_consolidated_vertices_and_vertex_map(collapseGraph, vertex_map);

    /*
     * Delete and rebuild all faces that do not have an identity mapping.
     * Restore localUvsProp.
     */

    typedef std::pair<VH, Vec2i> PVHV2I;

    std::vector<std::vector<PVHV2I> > newFaces;

#ifndef NDEBUG
    const size_t deletedFaces_initial =
#endif
            delete_obsolete_faces_and_create_new_ones(vertex_map, newFaces);
#ifndef NDEBUG
    const size_t updatedFaces_initial =
#endif
            newFaces.size();
    size_t deletedFaces_cleanup = 0;
    size_t deletedVertices_cleanup = 0;

    for (size_t facesDeleted = 1337; facesDeleted > 0; deletedFaces_cleanup += facesDeleted) {
        facesDeleted = remove_double_faces(newFaces);
        deletedVertices_cleanup += remove_isolated_vertices(newFaces);
    }

#ifndef NDEBUG
    size_t skippedFaces =
#endif
            create_faces(newFaces);

    /*
     * Delete all vertices that were mapped somewhere else.
     */
#ifndef NDEBUG
    const size_t deletedVertices =
#endif
            delete_old_vertices(vertex_map);

#ifndef NDEBUG
    std::cout
            << "perform_initial_collapses:" << std::endl
            << "  Initially deleted " << deletedVertices << " vertices, created " << createdVertices << " vertices." << std::endl
            << "  Initially deleted " << deletedFaces_initial << " faces, updated " << updatedFaces_initial << " faces." << std::endl
            << "  During cleanup deleted " << deletedFaces_cleanup << " faces, deleted " << deletedVertices_cleanup << " vertices." << std::endl
            << "  During face materialization skipped " << skippedFaces << " faces due to non-manifoldness." << std::endl;
#endif

    mesh_.garbage_collection();
    mesh_.update_normals();

#ifndef NDEBUG
    std::cout << "Garbage collection done." << std::endl;
#endif
}

template<class MeshT>
void QuadExtractorPostprocT<MeshT>::output_localuvs_of_selected_faces() {
    for (typename MeshT::FaceIter f_it = mesh_.faces_begin(), f_end = mesh_.faces_end(); f_it != f_end; ++f_it) {
        if (mesh_.status(*f_it).selected()) {
            std::cout << "Face " << f_it->idx() << " [<vertex idx>(<local uv>)]: ";
            for (typename MeshT::FHIter fh_begin = mesh_.fh_begin(*f_it), fh_it = fh_begin, fh_end = mesh_.fh_end(*f_it); fh_it != fh_end; ++fh_it) {
                if (fh_it != fh_begin) std::cout << ", ";
                std::cout << mesh_.to_vertex_handle(*fh_it) << "(" << localUvsProp[*fh_it] << ")";
            }
            std::cout << std::endl;
        }
    }
}

} // namespace QEx
