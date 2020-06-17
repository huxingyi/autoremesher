/*
 * Copyright 2013 Computer Graphics Group, RWTH Aachen University
 * Authors: Hans-Christian Ebke <ebke@cs.rwth-aachen.de>
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


#ifndef MESHDECIMATORT_HH_
#define MESHDECIMATORT_HH_

#include "../interfaces/c/qex.h"
#include "Globals.hh"
#include <iostream>

namespace QEx {

/**
 * @brief Collapses edges degenerated in the parameter domain.
 */
template<class MeshT>
class MeshDecimator {
    public:
        MeshDecimator(
                MeshT &mesh, std::vector<double> &uvs,
                std::vector<unsigned int> &external_valences) :

                mesh(mesh), uvs(uvs),
                external_valences(external_valences) {
        }

        void transfer_uvs(
                typename MeshT::HalfedgeHandle he_from,
                typename MeshT::HalfedgeHandle he_to) {

            std::copy(uvs.begin() + he_from.idx() * 2,
                      uvs.begin() + he_from.idx() * 2 + 2,
                      uvs.begin() + he_to.idx() * 2);
        }

        bool one_ring_intersection(
                typename MeshT::VertexHandle v0,
                typename MeshT::VertexHandle v1,
                typename MeshT::VertexHandle vl,
                typename MeshT::VertexHandle vr) {

            for (typename MeshT::VVIter vv_it = mesh.vv_begin(v0),
                    vv_end = mesh.vv_end(v0); vv_it != vv_end; ++vv_it) {
                mesh.status(*vv_it).set_tagged(false);
            }

            for (typename MeshT::VVIter vv_it = mesh.vv_begin(v1),
                    vv_end = mesh.vv_end(v1); vv_it != vv_end; ++vv_it) {
                mesh.status(*vv_it).set_tagged(true);
            }

            for (typename MeshT::VVIter vv_it = mesh.vv_begin(v0),
                    vv_end = mesh.vv_end(v0); vv_it != vv_end; ++vv_it) {
                if (mesh.status(*vv_it).tagged() &&
                        *vv_it != vl && *vv_it != vr)
                    return true;
            }

            return false;
        }

        size_t decimate() {
            mesh.request_vertex_status();
            mesh.request_halfedge_status();
            mesh.request_edge_status();
            mesh.request_face_status();
            size_t dec_counter = 0;
            for (typename MeshT::EdgeIter e_it = mesh.edges_sbegin(),
                    e_end = mesh.edges_end(); e_it != e_end; ++e_it) {

                const typename MeshT::HalfedgeHandle
                    he0 = mesh.halfedge_handle(*e_it, 0),
                    he0p = mesh.prev_halfedge_handle(he0),
                    he1 = mesh.opposite_halfedge_handle(he0),
                    he1p = mesh.prev_halfedge_handle(he1);

                const int
                    idx0 = he0.idx(), idx0p = he0p.idx(),
                    idx1 = he1.idx(), idx1p = he1p.idx();

                OpenMesh::Vec2d
                    uv0(uvs[2 * idx0], uvs[2 * idx0 + 1]),
                    uv0p(uvs[2 * idx0p], uvs[2 * idx0p + 1]),
                    uv1(uvs[2 * idx1], uvs[2 * idx1 + 1]),
                    uv1p(uvs[2 * idx1p], uvs[2 * idx1p + 1]);

                // Disregard UVs of boundary edges.
                if (mesh.is_boundary(he0)) {
                    uv0 = uv1p;
                    uv0p = uv1;
                } else if (mesh.is_boundary(he1)) {
                    uv1 = uv0p;
                    uv1p = uv0;
                }

                if (uv0 == uv0p || uv1 == uv1p) {
                    const typename MeshT::HalfedgeHandle
                        he0n = mesh.next_halfedge_handle(he0),
                        he1n = mesh.next_halfedge_handle(he1);

                    if (one_ring_intersection(
                            mesh.to_vertex_handle(he0),
                            mesh.to_vertex_handle(he1),
                            mesh.to_vertex_handle(he0n),
                            mesh.to_vertex_handle(he1n))) {
#ifndef NDEBUG
                        std::cout << "\x1b[41mDecimation: Got one ring "
                                "intersection case.\x1b[0m" << std::endl;
#endif
                    } else if (!mesh.is_collapse_ok(he0)) {
                        std::cout << "\x1b[41mSkipping non-ok collapse.\x1b[0m" << std::endl;
                    } else {
                        ++dec_counter;

                        /*
                         * Fix valences.
                         */
                        if (!external_valences.empty()) {
                            const int
                                vidx0 = mesh.to_vertex_handle(he0).idx(),
                                vidx1 = mesh.from_vertex_handle(he0).idx();
                            const int idx_to = 4 - static_cast<int>(
                                    external_valences[vidx0]);
                            const int idx_from = 4 - static_cast<int>(
                                    external_valences[vidx1]);
                            external_valences[vidx0] = 4 + idx_from + idx_to;
                        }

                        /*
                         * Fix UVs
                         */
                        const typename MeshT::HalfedgeHandle
                            he0n_opp = mesh.opposite_halfedge_handle(he0n),
                            he1n_opp = mesh.opposite_halfedge_handle(he1n),
                            he0p_opp = mesh.opposite_halfedge_handle(he0p),
                            he1p_opp = mesh.opposite_halfedge_handle(he1p);

                        transfer_uvs(he0n_opp, he0p);
                        transfer_uvs(he0p_opp, he0n);
                        transfer_uvs(he1n_opp, he1p);
                        transfer_uvs(he1p_opp, he1n);

#ifndef NDEBUG
                        if ((uv0-uv0p).sqrnorm() > 0 ||
                                (uv1-uv1p).sqrnorm() > 0) {
                            std::cout << "\x1b[43mCollapsing inconsistent edge. "
                                    "Length of he0 = "
                                    << (uv0-uv0p).sqrnorm()
                                    << ", length of he1 = "
                                    << (uv1-uv1p).sqrnorm()
                                    << std::endl
                                    << "If this happens prior to truncation "
                                       "it's ok. If it happens afterwards, "
                                       "this is a bug."
                                    << "\x1b[0m" << std::endl;
                        }

#endif

                        mesh.collapse(he0);
                    }
                }
            }
#ifndef NDEBUG
            std::cout << "\x1b[1;32m"
                    "MeshDecimator::decimate() collapsed " << dec_counter
                    << " edges.\x1b[0m" << std::endl;
#endif
            return dec_counter;
        }

    private:
        MeshT &mesh;
        std::vector<double> &uvs;
        std::vector<unsigned int> &external_valences;
};

} /* namespace QEx */

#endif /* MESHDECIMATORT_HH_ */
