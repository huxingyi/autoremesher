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
#ifndef AUTO_REMESHER_QUAD_REMESHER_H
#define AUTO_REMESHER_QUAD_REMESHER_H
#include <vector>
#include <cstddef>
#include <map>
#include <AutoRemesher/Vector3>
#include <AutoRemesher/HalfEdge>

namespace AutoRemesher
{

class QuadRemesher
{
public:
    QuadRemesher(HalfEdge::Mesh *mesh, const std::vector<size_t> *vertexValences=nullptr) :
        m_mesh(mesh),
        m_vertexValences(vertexValences)
    {
    }
    
    const std::vector<Vector3> &remeshedVertices()
    {
        return m_remeshedVertices;
    }
    
    const std::vector<std::vector<size_t>> &remeshedQuads()
    {
        return m_remeshedQuads;
    }
    
    bool remesh();
private:
    HalfEdge::Mesh *m_mesh = nullptr;
    const std::vector<size_t> *m_vertexValences = nullptr;
    std::vector<Vector3> m_remeshedVertices;
    std::vector<std::vector<size_t>> m_remeshedQuads;
    
    void fixHoles();
    void createCoonsPatchFrom(const std::vector<size_t> &c0,
        const std::vector<size_t> &c1,
        const std::vector<size_t> &d0,
        const std::vector<size_t> &d1);
};

}

#endif
