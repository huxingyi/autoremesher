/*
 * Copyright 2013 Computer Graphics Group, RWTH Aachen University
 * Authors: David Bommes <bommes@cs.rwth-aachen.de>
 *          Hans-Christian Ebke <ebke@cs.rwth-aachen.de>
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

/// @file
#ifndef QEX_MESHEXTRACTORT_HH
#define QEX_MESHEXTRACTORT_HH

//== INCLUDES =================================================================

#include "../interfaces/c/qex.h"
#include "TransitionFunction.hh"
#include "Algebra.hh"
#include "Vector.hh"
#include "ExactPredicates.h"
#include "Globals.hh"

#include <OpenMesh/Core/Utils/Property.hh>

#include <vector>
#include <complex>
#include <limits>
#include <ostream>
#ifndef NDEBUG
#include <iostream>
#endif

namespace QEx {

/**
 * @brief Helper class to ensure initialization of exact predicates.
 *
 * This class ensures that the exact predicates are
 * initialized before any class in this translation
 * unit is used. It also wastes sizeof(void*) bytes
 * of memory. :-)
 *
 * @see exactinit
 */
class ExactPredicatesInitializer {
    private:
        ExactPredicatesInitializer() {
            exactinit();
#ifndef NDEBUG
            std::cout << "Exact predicates initialized." << std::endl;
#endif
        }

        static ExactPredicatesInitializer instance;
};

#ifndef NDEBUG
class HalfedgeParamInfo {
    public:
        HalfedgeParamInfo(Vec2i uv_from, Vec2i uv_to_wrt_from, TransitionFunctionInt accumulated_tf) :
                uv_from(uv_from), uv_to_wrt_from(uv_to_wrt_from), accumulated_tf(accumulated_tf) {
        }

        HalfedgeParamInfo() :
                uv_from(std::numeric_limits<int>::max(), std::numeric_limits<int>::max()), uv_to_wrt_from(
                        std::numeric_limits<int>::max(), std::numeric_limits<int>::max()), accumulated_tf(
                        TransitionFunctionInt::IDENTITY) {
        }

        Vec2i uv_from, uv_to_wrt_from;
        TransitionFunctionInt accumulated_tf;

        static const char *HANDLE_NAME() {
            return "Plugin-MIQParameterize.ParamInfo";
        }
};
#endif

/**
 * @brief The core quad extractor functionality.
 *
 * Usage example:
 * @code{.cpp}
 * TriMesh triMesh;
 * QuadMesh quadMesh;
 * std::vector<double> uvs; uvs.resize(triMesh.n_halfedges() * 2);
 * QEx::MeshExtractorT<TriMesh> qme(triMesh);
 * qme.extract(uvs, quadMesh, false, true);
 * @endcode
 */
template<class TMeshT>
class MeshExtractorT {
    public:

        // Triangle Mesh type
        typedef TMeshT TMesh;

        // Base class to use mesh extraction_with_mapping
        class MappingBase {
            public:
                virtual typename TMesh::Point map_to_surface(
                        const double _u, const double _v, const typename TMesh::FaceHandle _fh) = 0;
        };

        /**
         * @brief Constructor.
         * @param _tri_mesh The triangle mesh which to construct a quad mesh from.
         */
        MeshExtractorT(const TMesh& _tri_mesh);

        /// Destructor
        ~MeshExtractorT();

        /**
         * @brief Extract quadmesh based on _uv_coords given per halfedge.
         *
         * Real boundaries are tagged vertex-wise [ mesh.status(vh).tagged() : true -> input-mesh boundary]
         * -> undesired holes can be identified by their missing vertex-tags
         */
        template<class PolyMeshT>
        void extract(
                std::vector<double>& _uv_coords,   // input uv-coordinates
                typename PropMgr<PolyMeshT>::LocalUvsPropertyManager &heLocalUvProp,
                PolyMeshT& _quad_mesh, const std::vector<unsigned int> * const _external_valences = 0);

        /**
         * @brief Analyze parametrization and return some statistics.
         *
         * This method used to be called analyze_after_truncation, in case you
         * want to port legacy code.
         *
         * @return Parametrization statistics.
         */
        std::string getParametrizationStats(std::vector<double>& _uv_coords);

    public:

        // shorter names for mesh access
        typedef typename TMesh::VertexHandle VH;
        typedef typename TMesh::FaceHandle FH;
        typedef typename TMesh::EdgeHandle EH;
        typedef typename TMesh::HalfedgeHandle HEH;
        typedef typename TMesh::VertexIter VIter;
        typedef typename TMesh::EdgeIter EIter;
        typedef typename TMesh::FaceIter FIter;
        typedef typename TMesh::VIHIter VIHIter;
        typedef typename TMesh::CVIHIter CVIHIter;
        typedef typename TMesh::VFIter VFIter;
        typedef typename TMesh::Point Point;

        typedef std::complex<double> Complex;

        typedef std::pair<int, int> PairII;

        // shortname
        typedef TransitionFunctionInt TF;

        // structure to hold all relevant information of an edge locally emanating from a grid-vertex

        struct LocalEdgeInfo {
                enum LEConnectivityInfo {
                    LECI_Connected_Thresh = 0,
                    LECI_No_Connection = -1,
                    LECI_Traced_Into_Boundary = -2,
                    LECI_Traced_Into_Degeneracy = -3,
                    LECI_Tracing_Error = -4
                };

                // default constructor
                LocalEdgeInfo(const FH _fh_from, const Point_2 _uv_from, const Point_2 _uv_to) :
                        fh_from(_fh_from), uv_from(_uv_from), uv_intended_to(_uv_to),
						uv_to(std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN()), connected_to_idx(LECI_No_Connection), orientation_idx(-1),
                        face_constructed(false), accumulated_tf(TF::IDENTITY), halfedgeIndex(-1)
#ifndef NDEBUG
                        , primary(false), id(nextId++)
#endif
                {}

                LocalEdgeInfo() :
                        uv_from(std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN()), uv_intended_to(std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN()), uv_to(std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN()),
                        connected_to_idx(LECI_No_Connection), orientation_idx(-1),
                        face_constructed(false), accumulated_tf(TF::IDENTITY), halfedgeIndex(-1)
#ifndef NDEBUG
                        , primary(false), id(nextId++)
#endif
                {}

                inline bool is_valid() const {
                    return fh_from.is_valid();
                }

                inline bool isUnconnected() const {
                    return connected_to_idx == LECI_No_Connection;
                }
                inline bool isConnected() const {
                    return connected_to_idx >= LECI_Connected_Thresh;
                }

                inline bool isUnconnectedOrSignal() const {
                    return connected_to_idx < LECI_Connected_Thresh;
                }

                inline void completeInformation(int connected_to_idx, int orientation_idx,
                        Point_2 uv_to, TF accumulated_tf) {
                    this->connected_to_idx = connected_to_idx;
                    this->orientation_idx = orientation_idx;
                    this->uv_to = uv_to;
                    this->accumulated_tf = accumulated_tf;
                }

                static bool vecNan(const Point_2 &pt) {
                    return pt[0] != pt[0] && pt[1] != pt[1];
                }

                inline bool operator==(const LocalEdgeInfo &rhs) const {
                    return fh_from == rhs.fh_from && uv_from == rhs.uv_from
                            && uv_intended_to == rhs.uv_intended_to
                            && ((vecNan(uv_to) && vecNan(rhs.uv_to)) || (uv_to == rhs.uv_to))
                            && connected_to_idx == rhs.connected_to_idx
                            && orientation_idx == rhs.orientation_idx
                            && face_constructed == rhs.face_constructed
                            && accumulated_tf == rhs.accumulated_tf
                            && halfedgeIndex == rhs.halfedgeIndex
#ifndef NDEBUG
                            && primary == rhs.primary
#endif
                    ;
                }

#ifndef NDEBUG
                operator HalfedgeParamInfo() const {
                    return HalfedgeParamInfo(Vec2i(uv_from[0], uv_from[1]),
                            Vec2i(uv_to[0], uv_to[1]), accumulated_tf);
                }
#endif

                // face handle of chart for uv_from and uv_to
                FH fh_from;

                // parameter-coords of start point of outgoing edge (w.r.t. fh_from)
                Point_2 uv_from;

                /*
                 * parameter-coords of the intended end point of outgoing edge (w.r.t. fh_from)
                 * The actual end point might differ if the edge is a reversing edge (i.e. connects a negative and a positive area).
                 */
                Point_2 uv_intended_to;
                Point_2 uv_to;

                // information of the connection to (which_vertex,which_local_edge)
                // connected_to.first >= 0  -> global GridVertex idx
                // connected_to.first == -1 -> no connection
                // connected_to.first == -2 -> tracing ran into boundary
                // connected_to.first == -3 -> tracing ran into degeneracy
                // connected_to.first == -4 -> tracing ran into an error, need debugging
                int connected_to_idx;

                static const char *connected_to_idx_failure_string(int connected_to_idx) {
                    if (connected_to_idx >= LECI_Connected_Thresh)
                        return "Ok";

                    switch (connected_to_idx) {
                        case LECI_No_Connection:
                            return "No Trace Performed";
                        case LECI_Traced_Into_Boundary:
                            return "Traced Into Boundary";
                        case LECI_Traced_Into_Degeneracy:
                            return "Traced Into Degeneracy";
                        case LECI_Tracing_Error:
                            return "Tracing Error";
                    }

                    return "Unknown";
                }

                int orientation_idx;

                // has face of outgoing halfedge has been constructed
                bool face_constructed;

                TF accumulated_tf;
                int halfedgeIndex;

#ifndef NDEBUG
                bool primary;

            public:
                std::string getIdHeIdx() const {
                    if (this == 0)
                        return "null";

                    std::ostringstream oss;
                    oss << "\x1b[32m" << id << "\x1b[0m(" << halfedgeIndex << ")";
                    return oss.str();
                }
                static size_t nextId;
                size_t id;
#endif

                template<typename STREAM>
                inline friend STREAM &operator<<(STREAM &s, const LocalEdgeInfo &self) {
                    s << "LocalEdgeInfo { fh_from.idx(): " << self.fh_from.idx()
                            << ", uv_from: (" << self.uv_from
                            << "), uv_intended_to: (" << self.uv_intended_to
                            << "), uv_to: (" << self.uv_to
                            << "), connected_to_idx: " << self.connected_to_idx
                            << ", orientation_idx: " << self.orientation_idx
                            << ", face_constructed: " << self.face_constructed
                            << ", accumulated_tf: " << self.accumulated_tf
#ifndef NDEBUG
                            << ", primary: " << self.primary
                            << ", id: " << self.id
#endif
                            << " }";
                    return s;
                }
        };

        class FindPathResult {

            public:

                FindPathResult(int connected_to_idx, int orientation_idx, const Point_2 &from_uv,
                        const Point_2 &to_uv_wrt_from, const TF &accumulated_tf) :
                        connected_to_idx(connected_to_idx), orientation_idx(orientation_idx), from_uv(
                                from_uv), to_uv_wrt_from(to_uv_wrt_from), accumulated_tf(
                                accumulated_tf) {

                    assert(from_uv[0] == std::floor(from_uv[0]) &&
                            from_uv[1] == std::floor(from_uv[1]) &&
                            to_uv_wrt_from[0] == std::floor(to_uv_wrt_from[0]) &&
                            to_uv_wrt_from[1] == std::floor(to_uv_wrt_from[1]));
                }

                int connected_to_idx;
                int orientation_idx;

                Point_2 from_uv;

                /**
                 * UV coordinates of end vertex expressed in the
                 * coordinate frame of the start vertex.
                 */
                Point_2 to_uv_wrt_from;

                TF accumulated_tf;

                static inline FindPathResult Error() {
                    static FindPathResult error(LocalEdgeInfo::LECI_Tracing_Error, 0, Point_2(0, 0),
                            Point_2(0, 0), TF::IDENTITY);
                    return error;
                }

                static inline FindPathResult Signal(
                        typename LocalEdgeInfo::LEConnectivityInfo connInfo) {
                    return FindPathResult(connInfo, 0, Point_2(0, 0), Point_2(0, 0), TF::IDENTITY);
                }

                inline void applyToLocalEdgeInfo(LocalEdgeInfo &lei) {
                    lei.completeInformation(connected_to_idx, orientation_idx, to_uv_wrt_from,
                            accumulated_tf);
                    /*
                     lei.connected_to_idx = connected_to_idx;
                     lei.orientation_idx = orientation_idx;
                     lei.accumulated_tf = accumulated_tf;
                     lei.uv_to = to_uv_wrt_from;
                     */
#ifndef NDEBUG
                    lei.primary = true;
#endif

                }
        };

        // structure to hold the information of a single grid vertex in uv-space
        struct GridVertex {
                // GridVertices may have different types, depending on where they are living
                enum GVType {
                    OnVertex = 0, OnEdge = 1, OnFace = 2
                };

                // default constructor
                GridVertex() {}

                // additional constructor
                GridVertex(const GVType _type, const HEH& _heh, const Point_2& _p_uv,
                        const Point& _p_3d, const bool _is_boundary) :
                        type(_type), is_boundary(_is_boundary), missing_leis(0),
                        heh(_heh), position_uv(_p_uv), position_3d(_p_3d)
#ifndef NDEBUG
                        , verbose(false)
#endif
                {}

                // number of local outgoing edges (ordered CCW)
                unsigned int n_edges() {
                    return local_edges.size();
                }

                // get local edge (allowing cyclical access)
                LocalEdgeInfo& local_edge(int _i) {
                    int n = n_edges();
                    _i = ((_i % n) + n) % n;
                    return local_edges[_i];
                }

                /// type on Vertex/Edge/Face
                GVType type;

                /// does vertex belong to quadmesh boundary?
                bool is_boundary;
                int missing_leis;

                // HalfedgeHandle(FaceHandle) of defining chart
                // for GridVertices on Edges/Vertices heh belongs to/points to the edge/vertex
                HEH heh;
                // position in parametrization
                Point_2 position_uv;
                // position in 3d quadmesh
                Point position_3d;
                // local edge information CCW-ordered
                std::vector<LocalEdgeInfo> local_edges;

#ifndef NDEBUG
                bool verbose;
#endif

                const char *typeAsString() const {
                    switch (type) {
                        case OnVertex:
                            return "OnVertex";
                        case OnEdge:
                            return "OnEdge";
                        case OnFace:
                            return "OnFace";
                        default:
                            return "InvalidType";
                    }
                }

                template<class STREAM>
                friend STREAM &operator<<(STREAM &s, const GridVertex &self) {
                    s << "GridVertex {" << self.typeAsString();
                    s << ", " << (self.is_boundary ? "boundary" : "non-boundary")
                            << ", heh: " << self.heh.idx()
                            << ", uv pos: (" << self.position_uv << ")"
                            << ", 3d pos: (" << self.position_3d << ")"
                            << ", local_edges: [" << std::endl;

                    for (typename std::vector<LocalEdgeInfo>::const_iterator it = self.local_edges.begin(), it_end = self.local_edges.end(); it != it_end; ++it) {
                        s << "    " << *it;
                        if (it + 1 != it_end)
                            s << ",";
                        s << std::endl;
                    }

                    s << "] }";

                    return s;
                }
        };

    public:
        template<class PolyMeshT>
        static void print_quad_mesh_metrics(const PolyMeshT& _quad_mesh);

#ifdef TESTING
    public:
#else
    private:
#endif
        // extract transition functions from uv-coords
        void extract_transition_functions(const std::vector<double>& _uv_coords);

        void perturbate_degenerate_faces(std::vector<double>& _uv_coords);

        // truncate uv-coords in order to make transition functions exact
        void consistent_truncation(std::vector<double>& _uv_coords);

        // use vertex and edge informatin created so far to construct faces
        // and to store the resulting mesh in _quad_mesh
        template<class PolyMeshT>
        void generate_faces_and_store_quadmesh(PolyMeshT& _quad_mesh,
                typename PropMgr<PolyMeshT>::LocalUvsPropertyManager &heLocalUvProp);

        // generate GridVertices and their local edge information
        // vertex_to_face_ and edge_to_face_ are initialized for use in generate_edges
        template<typename EMBEDDING>
        void generate_vertices(std::vector<double>& _uv_coords,
                               const std::vector<unsigned int> * const _external_valences,
                               EMBEDDING embedding);

        // generate connections between GridVertices (connect local outgoing edges)
        void generate_connections(std::vector<double>& _uv_coords);

        void try_connect_incomplete_gvertices();

        // in debug mode check all connections
        void check_connections();

        // find path for outgoing local edge via tracing
        FindPathResult find_path(const GridVertex& _gv, const LocalEdgeInfo& lei,
                std::vector<double>& _uv_coords);

        // find local connection where it is known that the endpoint intersects the triangle
        FindPathResult find_local_connection(const Point_2& _uv_from,
                const Point_2& _uv_original_from, const Point_2& _uv_to, const Triangle_2& _tri,
                const HEH _heh0, const HEH _heh1, const HEH _heh2, const BOUNDEDNESS& _bs,
                TF &accumulated_tf, std::vector<double>& _uv_coords);

        // find local connection where it is known that the endpoint intersects the edge given by _heh
        FindPathResult find_local_connection_at_edge(const Point_2& _uv_from,
                const Point_2& _uv_original_from, const Point_2& _uv_to, const HEH _heh,
                TF &accumulated_tf);

        // find local connection where it is known that the endpoint intersects the vertex pointed to by _heh
        // _tri is in local ordering _tri[0] = vertex(_heh->to)
        FindPathResult find_local_connection_at_vertex(const Point_2& _uv_from,
                const Point_2& _uv_original_from, const Point_2& _uv_to, const HEH _heh,
                const Triangle_2& _tri, TF &accumulated_tf);

        ORIENTATION triangleUvOrientation(FH fh, const std::vector<double> &uv_coords);

        // construct local information of outgoing edges
        void construct_local_edge_information_face(GridVertex& _gv,
                const std::vector<double>& _uv_coords);
        void construct_local_edge_information_edge(GridVertex& _gv,
                const std::vector<double>& _uv_coords);
        void construct_local_edge_information_vertex(GridVertex& _gv,
                const std::vector<double>& _uv_coords,
                const std::vector<unsigned int> * const _external_valences);

        inline void reverseApply(Point_2 &p1, Point_2 &p2, const TF &accumulated_tf) {
            TF inverse_tf = accumulated_tf.inverse();
            inverse_tf.transform_point(p1);
            inverse_tf.transform_point(p2);
        }

        // convert the vector into orientation as used in cartesian_orientations_
        //-----------------------------------------------------------------------------
        inline int ori_to_idx(const Vector_2& _dir) const {
            // Vector has to be
            assert(
                    _dir == Vector_2(1, 0) || _dir == Vector_2(0, 1) || _dir == Vector_2(-1, 0)
                            || _dir == Vector_2(0, -1));

            if (_dir[0] != 0)
                return ((_dir[0] > 0) ? 0 : 2);
            else
                return ((_dir[1] > 0) ? 1 : 3);
        }

        inline int ori_to_idx_inverse(const Vector_2& _dir) const {
            return 3 - ori_to_idx(_dir);
        }

        // get uv-coordinates of halfedge
        Complex uv_as_complex(const HEH& _heh, const std::vector<double>& _uv_coords) {
            return Complex(_uv_coords[2 * _heh.idx()], _uv_coords[2 * _heh.idx() + 1]);
        }

        Vec2d uv_as_vec2d(const HEH &_heh, const std::vector<double>& _uv_coords) {
            return Vec2d(_uv_coords[2 * _heh.idx()], _uv_coords[2 * _heh.idx() + 1]);
        }

        // get transition function around vertex (if non-boundary vertex, otherwise return identity)
        // chart for transition function is beginning of VIHIter
        TF transition(const VH& _vh) const;

        // get transition function from triangle(heh) -> triangle(opp_heh)
        TF transition(const HEH& _heh) const {
            const EH eh = tri_mesh_.edge_handle(_heh);
            if (tri_mesh_.halfedge_handle(eh, 0) == _heh)
                return tf_[eh.idx()];
            else
                return tf_[eh.idx()].inverse();
        }

        // barycentric mapping of p between _tri and (_a,_b,_c)
        inline Matrix_3 get_mapping(const Triangle_2& _tri, const Point& _a, const Point& _b, const Point& _c) const;

        inline Point applyMapping(const Matrix_3 &M, double x, double y) {
            Point p3d(0, 0, 0);
            for (unsigned int i = 0; i < 3; ++i)
                p3d[i] += M(i, 0) * x + M(i, 1) * y + M(i, 2);
            return p3d;
        }

        // barycentric mapping of p between _seg and (_a,_b)
        Matrix_3 get_mapping(const Segment_2& _seg, const Point& _a, const Point& _b) const;

        TF intra_gv_transition(FH from_fh, FH to_fh, const GridVertex &gv, bool returnIdentityIfSameFh);

        /**
         * @brief Adds a face to a poly mesh.
         *
         * This add_face routine - in contrast to the one supplied by OpenMesh - is
         * capable of adding a face with ``double edges'', e.g. a-b-c-b-d-e.
         *
         * OpenMesh still doesn't support non-manifold meshes. Prior to adding the
         * face, this method checks whether a non-manifold configuration would result.
         * In such cases the face cannot be added and an invalid face handle is returned.
         *
         * @return The handle to the new face. This handle is invalid if the face could not be added.
         */
        template<class PolyMeshT>
        typename PolyMeshT::FaceHandle add_face(PolyMeshT &qmesh,
                std::vector<LocalEdgeInfo*> &leis);

        LocalEdgeInfo *getNextConnectedLeiWithHE(int connected_to_idx, int orientation_idx,
                int direction);

        void increment_opposite_connected_to_idx(
                typename std::vector<LocalEdgeInfo>::iterator first,
                typename std::vector<LocalEdgeInfo>::iterator last);
        bool notConnected(GridVertex &gv1, GridVertex &gv2);

#ifdef TESTING
        public:
#else
    private:
#endif

        // reference to triangle mesh
        TMesh tri_mesh_;

        // transition functions per edge
        // convention: from face(first_halfedge) -> face(second_halfedge)
        std::vector<TF> tf_;

        // assign vertices and edges uniquely to one neighboring face
        std::vector<HEH> vertex_to_halfedge_;
        std::vector<HEH> edge_to_halfedge_;

        // cache whether or not a triangle/edge is valid in parameter space
        std::vector<bool> edge_valid_;

        // store for each face/edge/vertex references to its grid-vertices
        std::vector<std::vector<int> > face_gvertices_;
        std::vector<std::vector<int> > edge_gvertices_;
        std::vector<std::vector<int> > vertex_gvertices_;

        // vector of grid vertices
        std::vector<GridVertex> gvertices_;

        // constant cartesian directions
        const Vector_2 du_;
        const Vector_2 dv_;
        std::vector<Vector_2> cartesian_orientations_;
};

#ifndef NDEBUG
template<class TMeshT>
size_t MeshExtractorT<TMeshT>::LocalEdgeInfo::nextId = 0;
#endif

} // namespace QEx

#if !defined(QEX_QUADMESHEXTRACTORT_C)
#define QEX_QUADMESHEXTRACTORT_TEMPLATES
#include "MeshExtractorT.cc"
#endif
#endif // QEX_MESHEXTRACTORT_HH
