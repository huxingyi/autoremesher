/*
 *  Copyright (c) 2020 Jeremy HU <jeremy-at-dust3d dot org>. All rights reserved. 
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */
#ifndef AUTO_REMESHER_RELATIVE_HEIGHT_H
#define AUTO_REMESHER_RELATIVE_HEIGHT_H
#include <AutoRemesher/Vector3>
#include <vector>
#include <map>

namespace AutoRemesher
{
    
class RelativeHeight
{
public:
    RelativeHeight(const std::vector<Vector3> *vertices,
            const std::vector<std::vector<size_t>> *triangles) :
        m_vertices(vertices),
        m_triangles(triangles)
    {
    }
    
    ~RelativeHeight()
    {
        delete m_vertexRelativeHeights;
        delete m_vertexNormals;
        delete m_faceAroundVertexMap;
    }
    
    std::vector<double> *takeVertexRelativeHeights()
    {
        std::vector<double> *vertexRelativeHeights = m_vertexRelativeHeights;
        m_vertexRelativeHeights = nullptr;
        return vertexRelativeHeights;
    }
    
    std::vector<Vector3> *takeVertexNormals()
    {
        std::vector<Vector3> *vertexNormals = m_vertexNormals;
        m_vertexNormals = nullptr;
        return vertexNormals;
    }
    
    double averageEdgeLength()
    {
        return m_averageEdgeLength;
    }
    
    std::map<size_t, std::vector<size_t>> *takeFaceAroundVertexMap()
    {
        std::map<size_t, std::vector<size_t>> *faceAroundVertexMap = m_faceAroundVertexMap;
        m_faceAroundVertexMap = nullptr;
        return faceAroundVertexMap;
    }

    bool calculate();
private:
    const std::vector<Vector3> *m_vertices = nullptr;
    const std::vector<std::vector<size_t>> *m_triangles = nullptr;
    std::vector<double> *m_vertexRelativeHeights = nullptr;
    std::vector<Vector3> *m_vertexNormals = nullptr;
    double m_averageEdgeLength = 0.0;
    std::map<size_t, std::vector<size_t>> *m_faceAroundVertexMap = nullptr;
    
    void buildFaceAroundVertexMap(const std::vector<std::vector<size_t>> &triangles,
        std::map<size_t, std::vector<size_t>> *faceAroundVertexMap);
    void calculateVertexNormals(const std::vector<Vector3> &vertices,
        const std::vector<std::vector<size_t>> &triangles,
        std::vector<Vector3> *vertexNormals);
    double calculateAverageEdgeLength(const std::vector<Vector3> &vertices,
        const std::vector<std::vector<size_t>> &triangles);
    void collectNearbyVertices(const std::vector<Vector3> &vertices,
        const std::vector<std::vector<size_t>> &triangles,
        const std::map<size_t, std::vector<size_t>> &faceAroundVertexMap,
        size_t vertexIndex, 
        std::vector<size_t> *nearbyVertexIndices, 
        double maxSquaredDistance);
};
    
}

#endif
