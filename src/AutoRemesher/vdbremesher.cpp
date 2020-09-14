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
#include <AutoRemesher/VdbRemesher>
#include <openvdb/openvdb.h>
#include <openvdb/tools/MeshToVolume.h>
#include <openvdb/tools/VolumeToMesh.h>
#include <openvdb/tools/LevelSetFilter.h>
#include <openvdb/tools/Morphology.h>
#include <openvdb/tools/Mask.h>
#include <openvdb/tools/Clip.h>
#include <openvdb/tools/LevelSetRebuild.h>
#include <openvdb/tools/MultiResGrid.h>
#include <openvdb/tools/Interpolation.h>
#include <openvdb/tools/LevelSetUtil.h>
#if AUTO_REMESHER_DEBUG
#include <QDebug>
#endif

namespace AutoRemesher
{

bool VdbRemesher::remesh()
{
    auto &vertices = *m_vertices;

    std::vector<openvdb::Vec3s> inputPoints(vertices.size());
	std::vector<openvdb::Vec3I> inputTriangles;
	std::vector<openvdb::Vec4I> inputQuads;
	for (size_t i = 0; i < vertices.size(); ++i) {
		const auto &src = vertices[i];
		inputPoints[i] = openvdb::Vec3s(src.x(), src.y(), src.z());
	}
	for (const auto &face: *m_triangles) {
		if (3 == face.size()) {
			inputTriangles.push_back(openvdb::Vec3I(face[0], face[1], face[2]));
		} else if (4 == face.size()) {
			inputQuads.push_back(openvdb::Vec4I(face[0], face[1], face[2], face[3]));
		} else if (face.size() > 4) {
			Vector3 center;
			for (const auto &vertexIndex: face) {
				center += vertices[vertexIndex];
			}
			center /= face.size();
			size_t centerIndex = inputPoints.size();
			inputPoints.push_back(openvdb::Vec3s(center.x(), center.y(), center.z()));
			for (size_t j = 0; j < face.size(); ++j) {
				size_t k = (j + 1) % face.size();
				inputTriangles.push_back(openvdb::Vec3I(face[j], face[k], centerIndex));
			}
		}
	}
    openvdb::math::Transform::Ptr transform = openvdb::math::Transform::createLinearTransform(m_voxelSize);
	openvdb::FloatGrid::Ptr grid = openvdb::tools::meshToLevelSet<openvdb::FloatGrid>(
		*transform, inputPoints, inputTriangles, inputQuads, 3);

	//openvdb::tools::LevelSetFilter<openvdb::FloatGrid> filter(*grid);
	//filter.laplacian();
	
    double isovalue = 0.0;
	double adaptivity = 0.0;
	bool relaxDisorientedTriangles = false;
	std::vector<openvdb::Vec3s> outputPoints;
	std::vector<openvdb::Vec3I> outputTriangles;
	std::vector<openvdb::Vec4I> outputQuads;
    
    openvdb::tools::volumeToMesh<openvdb::FloatGrid>(*grid, outputPoints, outputTriangles, outputQuads,
        isovalue, adaptivity, relaxDisorientedTriangles);
    
    delete m_vdbVertices;
	m_vdbVertices = new std::vector<Vector3>(outputPoints.size());
	for (size_t i = 0; i < outputPoints.size(); ++i) {
		const auto &src = outputPoints[i];
		(*m_vdbVertices)[i] = Vector3(src.x(), src.y(), src.z());
	}
    delete m_vdbTriangles;
    m_vdbTriangles = new std::vector<std::vector<size_t>>;
    m_vdbTriangles->reserve(outputTriangles.size() + outputQuads.size() * 2);
	for (const auto &it: outputTriangles) {
		m_vdbTriangles->push_back({it.z(), it.y(), it.x()});
	}
	for (const auto &it: outputQuads) {
		m_vdbTriangles->push_back({it.z(), it.y(), it.x()});
		m_vdbTriangles->push_back({it.x(), it.w(), it.z()});
	}
    
    return true;
}

}
