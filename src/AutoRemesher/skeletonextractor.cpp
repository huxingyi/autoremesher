#include <AutoRemesher/SkeletonExtractor>
#include <set>

namespace AutoRemesher
{

SkeletonExtractor::SkeletonExtractor(const std::vector<Vector3> *vertices,
        const std::vector<std::vector<size_t>> *faces) :
    m_vertices(vertices),
    m_faces(faces)
{
}

void SkeletonExtractor::extract()
{
    std::vector<Mesh::Vertex_index> vertices;
    vertices.reserve(m_vertices->size());
    for (const auto &position: *m_vertices)
        vertices.push_back(m_mesh.add_vertex(Point(position.x(), position.y(), position.z())));
    for (const auto &face: *m_faces)
        m_mesh.add_face(vertices[face[0]], vertices[face[1]], vertices[face[2]]);
    
    bool created;
    boost::tie(m_meshPropertyMap, created) = m_mesh.add_property_map<Mesh::Vertex_index, size_t>("v:source", 0);
    for (size_t i = 0; i < vertices.size(); ++i)
        m_meshPropertyMap[vertices[i]] = i;
    
    CGAL::extract_mean_curvature_flow_skeleton(m_mesh, m_skeleton);
}

std::vector<std::vector<Skeleton::vertex_descriptor>> SkeletonExtractor::convertToStrokes()
{
    std::map<Skeleton::vertex_descriptor, std::vector<Skeleton::vertex_descriptor>> neighbors;
    for (Skeleton_edge e : CGAL::make_range(edges(m_skeleton))) {
        const auto &from = source(e, m_skeleton);
        const auto &to = target(e, m_skeleton);
        neighbors[from].push_back(to);
        neighbors[to].push_back(from);
    }
    
    std::vector<Skeleton::vertex_descriptor> endpoints;
    for (auto it = neighbors.begin(); it != neighbors.end(); ) {
        if (it->second.size() > 2) {
            for (const auto &v: it->second)
                endpoints.push_back(v);
            it = neighbors.erase(it);
            continue;
        }
        if (it->second.size() == 1)
            endpoints.push_back(it->first);
        it++;
    }
    
    std::set<Skeleton::vertex_descriptor> visited;
    
    auto findOtherNeighbor = [&](Skeleton::vertex_descriptor v) {
        auto &items = neighbors[v];
        for (const auto &it: items) {
            if (visited.end() == visited.find(it))
                return it;
        }
        return v;
    };
    
    std::vector<std::vector<Skeleton::vertex_descriptor>> strokes;
    for (const auto &endpoint: endpoints) {
        if (visited.end() != visited.find(endpoint))
            continue;
        std::vector<Skeleton::vertex_descriptor> stroke;
        auto v = endpoint;
        while (true) {
            stroke.push_back(v);
            visited.insert(v);
            auto nextV = findOtherNeighbor(v);
            if (nextV == v)
                break;
            v = nextV;
        }
        if (stroke.size() > 1)
            strokes.push_back(stroke);
    }
    
    return strokes;
}

Vector3 SkeletonExtractor::calculateStrokeBaseNormal(const std::vector<Skeleton::vertex_descriptor> &stroke)
{
    if (stroke.size() < 2)
        return Vector3(0.0, 0.0, 1.0);
    
    Vector3 traverseDirection;
    for (size_t i = 1; i < stroke.size(); ++i) {
        size_t h = i - 1;
        auto p = m_skeleton[stroke[i]].point - m_skeleton[stroke[h]].point;
        traverseDirection += Vector3(CGAL::to_double(p.x()), CGAL::to_double(p.y()), CGAL::to_double(p.z()));
    }
    traverseDirection.normalize();
    
    const std::vector<Vector3> axisList = {
        Vector3 {1.0, 0.0, 0.0},
        Vector3 {0.0, 1.0, 0.0},
        Vector3 {0.0, 0.0, 1.0},
    };
    double maxDot = -1;
    size_t nearAxisIndex = 0;
    bool reversed = false;
    for (size_t i = 0; i < axisList.size(); ++i) {
        const auto axis = axisList[i];
        auto dot = Vector3::dotProduct(axis, traverseDirection);
        auto positiveDot = abs(dot);
        if (positiveDot >= maxDot) {
            reversed = dot < 0;
            maxDot = positiveDot;
            nearAxisIndex = i;
        }
    }
    // axisList[nearAxisIndex] align with the traverse direction,
    // So we pick the next axis to do cross product with traverse direction
    const auto& choosenAxis = axisList[(nearAxisIndex + 1) % 3];
    auto baseNormal = Vector3::crossProduct(traverseDirection, choosenAxis).normalized();
    return reversed ? -baseNormal : baseNormal;
}

std::vector<std::vector<Mesh::Vertex_index>> SkeletonExtractor::calculateStrokeSeam(const std::vector<Skeleton::vertex_descriptor> &stroke)
{
    std::vector<std::vector<Mesh::Vertex_index>> seam;
    if (stroke.size() < 2)
        return seam;

    auto baseNormal = calculateStrokeBaseNormal(stroke);
    for (const auto &v: stroke) {
        const auto originPoint = m_skeleton[v].point;
        auto origin = Vector3(originPoint.x(), originPoint.y(), originPoint.z());
        std::vector<Mesh::Vertex_index> seamVertices;
        for (const auto &mv: m_skeleton[v].vertices) {
            auto currentPoint = m_mesh.point(mv);
            auto current = Vector3(currentPoint.x(), currentPoint.y(), currentPoint.z());
            auto direction = (current - origin).normalized();
            if (std::abs(Vector3::dotProduct(direction, baseNormal)) >= 0.866) { //<=30 or >=150 degrees
                seamVertices.push_back(mv);
            }
        }
        if (!seamVertices.empty())
            seam.push_back(seamVertices);
    }
    
    return seam;
}

void SkeletonExtractor::debugExportPolylines(const char *filename)
{
    std::ofstream output(filename);
    std::vector<std::pair<size_t, size_t>> polylines;
    size_t vertexNum = 0;
    for (Skeleton_edge e : CGAL::make_range(edges(m_skeleton))) {
        const Point &s = m_skeleton[source(e, m_skeleton)].point;
        const Point &t = m_skeleton[target(e, m_skeleton)].point;
        output << "v " << s << std::endl;
        output << "v " << t << std::endl;
        polylines.push_back({vertexNum + 1, vertexNum + 2});
        vertexNum += 2;
    }
    for (const auto &it: polylines) {
        output << "l " << it.first << " " << it.second << std::endl;
    }
}

void SkeletonExtractor::debugExportStrokes(const char *filename, const std::vector<std::vector<Skeleton::vertex_descriptor>> &strokes)
{
    std::ofstream output(filename);
    for (const auto &stroke: strokes) {
        for (const auto &v: stroke) {
            output << "v " << m_skeleton[v].point << std::endl;
        }
    }
    size_t vertexNum = 0;
    for (const auto &stroke: strokes) {
        output << "l";
        for (const auto &v: stroke) {
            output << " " << (vertexNum + 1);
            ++vertexNum;
        }
        output << std::endl;
    }
}

Mesh &SkeletonExtractor::mesh()
{
    return m_mesh;
}

Skeleton &SkeletonExtractor::skeleton()
{
    return m_skeleton;
}

Mesh::Property_map<Mesh::Vertex_index, size_t> &SkeletonExtractor::meshPropertyMap()
{
    return m_meshPropertyMap;
}

}