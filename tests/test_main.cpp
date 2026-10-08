#include <AutoRemesher/AutoRemesher>
#include <AutoRemesher/Vector3>
#include "meshio.h"
#include "quadmeshgenerator.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <QCoreApplication>
#include <QDir>
#include <QTemporaryDir>

static int g_testsPassed = 0;
static int g_testsFailed = 0;

#define TEST_ASSERT(condition, msg)                                                \
    do {                                                                           \
        if (condition) {                                                           \
            g_testsPassed++;                                                       \
        } else {                                                                   \
            g_testsFailed++;                                                       \
            std::cerr << "FAILED: " << msg << " (" << __FILE__ << ":" << __LINE__ \
                      << ")" << std::endl;                                         \
        }                                                                          \
    } while (0)

#define TEST_ASSERT_NEAR(a, b, eps, msg)                                           \
    do {                                                                           \
        if (std::abs((a) - (b)) <= (eps)) {                                        \
            g_testsPassed++;                                                       \
        } else {                                                                   \
            g_testsFailed++;                                                       \
            std::cerr << "FAILED: " << msg << " (" << (a) << " vs " << (b)        \
                      << ", diff=" << std::abs((a) - (b)) << ") at "               \
                      << __FILE__ << ":" << __LINE__ << std::endl;                 \
        }                                                                          \
    } while (0)

void testVector3Math()
{
    std::cout << "[RUN] testVector3Math..." << std::endl;

    AutoRemesher::Vector3 a(1.0, 2.0, 3.0);
    AutoRemesher::Vector3 b(4.0, 5.0, 6.0);

    AutoRemesher::Vector3 sum = a + b;
    TEST_ASSERT_NEAR(sum.x(), 5.0, 1e-6, "Vector3 operator+ x");
    TEST_ASSERT_NEAR(sum.y(), 7.0, 1e-6, "Vector3 operator+ y");
    TEST_ASSERT_NEAR(sum.z(), 9.0, 1e-6, "Vector3 operator+ z");

    AutoRemesher::Vector3 diff = b - a;
    TEST_ASSERT_NEAR(diff.x(), 3.0, 1e-6, "Vector3 operator- x");
    TEST_ASSERT_NEAR(diff.y(), 3.0, 1e-6, "Vector3 operator- y");
    TEST_ASSERT_NEAR(diff.z(), 3.0, 1e-6, "Vector3 operator- z");

    double dotPerp = AutoRemesher::Vector3::dotProduct(AutoRemesher::Vector3(1.0, 0.0, 0.0),
        AutoRemesher::Vector3(0.0, 1.0, 0.0));
    TEST_ASSERT_NEAR(dotPerp, 0.0, 1e-6, "Vector3 dotProduct orthogonal");

    double dotParallel = AutoRemesher::Vector3::dotProduct(AutoRemesher::Vector3(2.0, 0.0, 0.0),
        AutoRemesher::Vector3(3.0, 0.0, 0.0));
    TEST_ASSERT_NEAR(dotParallel, 6.0, 1e-6, "Vector3 dotProduct parallel");

    AutoRemesher::Vector3 cross = AutoRemesher::Vector3::crossProduct(AutoRemesher::Vector3(1.0, 0.0, 0.0),
        AutoRemesher::Vector3(0.0, 1.0, 0.0));
    TEST_ASSERT_NEAR(cross.x(), 0.0, 1e-6, "Vector3 crossProduct x");
    TEST_ASSERT_NEAR(cross.y(), 0.0, 1e-6, "Vector3 crossProduct y");
    TEST_ASSERT_NEAR(cross.z(), 1.0, 1e-6, "Vector3 crossProduct z");

    AutoRemesher::Vector3 v0(0.0, 0.0, 0.0);
    AutoRemesher::Vector3 v1(1.0, 0.0, 0.0);
    AutoRemesher::Vector3 v2(0.0, 1.0, 0.0);

    double triArea = AutoRemesher::Vector3::area(v0, v1, v2);
    TEST_ASSERT_NEAR(triArea, 0.5, 1e-6, "Vector3::area right triangle");

    AutoRemesher::Vector3 triNormal = AutoRemesher::Vector3::normal(v0, v1, v2);
    TEST_ASSERT_NEAR(triNormal.x(), 0.0, 1e-6, "Vector3::normal x");
    TEST_ASSERT_NEAR(triNormal.y(), 0.0, 1e-6, "Vector3::normal y");
    TEST_ASSERT_NEAR(triNormal.z(), 1.0, 1e-6, "Vector3::normal z");
}

void testMeshIOFormatDetection()
{
    std::cout << "[RUN] testMeshIOFormatDetection..." << std::endl;

    TEST_ASSERT(MeshIO::detectFormat("mesh.obj") == MeshIO::Format::OBJ, "detectFormat .obj");
    TEST_ASSERT(MeshIO::detectFormat("mesh.OBJ") == MeshIO::Format::OBJ, "detectFormat .OBJ uppercase");
    TEST_ASSERT(MeshIO::detectFormat("/path/to/mesh.stl") == MeshIO::Format::STL, "detectFormat .stl");
    TEST_ASSERT(MeshIO::detectFormat("part.StL") == MeshIO::Format::STL, "detectFormat .StL mixed case");
    TEST_ASSERT(MeshIO::detectFormat("pointcloud.ply") == MeshIO::Format::PLY, "detectFormat .ply");
    TEST_ASSERT(MeshIO::detectFormat("model.PLY") == MeshIO::Format::PLY, "detectFormat .PLY uppercase");
    TEST_ASSERT(MeshIO::detectFormat("unknown.fbx") == MeshIO::Format::Unknown, "detectFormat .fbx unknown");
}

static void makeCubeMesh(std::vector<AutoRemesher::Vector3>& vertices,
    std::vector<std::vector<size_t>>& faces)
{
    vertices = {
        { -0.5, -0.5, -0.5 },
        { 0.5, -0.5, -0.5 },
        { 0.5, 0.5, -0.5 },
        { -0.5, 0.5, -0.5 },
        { -0.5, -0.5, 0.5 },
        { 0.5, -0.5, 0.5 },
        { 0.5, 0.5, 0.5 },
        { -0.5, 0.5, 0.5 }
    };
    faces = {
        { 0, 1, 2 }, { 0, 2, 3 }, // bottom
        { 4, 6, 5 }, { 4, 7, 6 }, // top
        { 0, 4, 5 }, { 0, 5, 1 }, // front
        { 1, 5, 6 }, { 1, 6, 2 }, // right
        { 2, 6, 7 }, { 2, 7, 3 }, // back
        { 3, 7, 4 }, { 3, 4, 0 }  // left
    };
}

void testMeshIORoundtrips()
{
    std::cout << "[RUN] testMeshIORoundtrips..." << std::endl;

    QTemporaryDir tempDir;
    TEST_ASSERT(tempDir.isValid(), "Temporary directory created");

    std::vector<AutoRemesher::Vector3> originalVertices;
    std::vector<std::vector<size_t>> originalFaces;
    makeCubeMesh(originalVertices, originalFaces);

    std::string err;

    // 1. OBJ roundtrip
    {
        QString objPath = tempDir.filePath("cube.obj");
        bool saveOk = MeshIO::saveMesh(objPath, originalVertices, originalFaces, err);
        TEST_ASSERT(saveOk, "saveMesh OBJ");

        std::vector<AutoRemesher::Vector3> loadedVertices;
        std::vector<std::vector<size_t>> loadedFaces;
        bool loadOk = MeshIO::loadMesh(objPath, loadedVertices, loadedFaces, err);
        TEST_ASSERT(loadOk, "loadMesh OBJ");
        TEST_ASSERT(loadedVertices.size() == originalVertices.size(), "OBJ vertex count matches");
        TEST_ASSERT(loadedFaces.size() == originalFaces.size(), "OBJ face count matches");
    }

    // 2. STL ASCII roundtrip
    {
        QString stlAsciiPath = tempDir.filePath("cube_ascii.stl");
        bool saveOk = MeshIO::saveSTL(stlAsciiPath, originalVertices, originalFaces, err, false);
        TEST_ASSERT(saveOk, "saveSTL ASCII");

        std::vector<AutoRemesher::Vector3> loadedVertices;
        std::vector<std::vector<size_t>> loadedFaces;
        bool loadOk = MeshIO::loadMesh(stlAsciiPath, loadedVertices, loadedFaces, err);
        TEST_ASSERT(loadOk, "loadMesh STL ASCII");
        TEST_ASSERT(loadedFaces.size() == 12, "STL ASCII triangle count matches");
        TEST_ASSERT(loadedVertices.size() == 8, "STL ASCII vertex welding welded to 8 vertices");
    }

    // 3. STL binary roundtrip
    {
        QString stlBinPath = tempDir.filePath("cube_bin.stl");
        bool saveOk = MeshIO::saveSTL(stlBinPath, originalVertices, originalFaces, err, true);
        TEST_ASSERT(saveOk, "saveSTL binary");

        std::vector<AutoRemesher::Vector3> loadedVertices;
        std::vector<std::vector<size_t>> loadedFaces;
        bool loadOk = MeshIO::loadMesh(stlBinPath, loadedVertices, loadedFaces, err);
        TEST_ASSERT(loadOk, "loadMesh STL binary");
        TEST_ASSERT(loadedFaces.size() == 12, "STL binary triangle count matches");
        TEST_ASSERT(loadedVertices.size() == 8, "STL binary vertex welding welded to 8 vertices");
    }

    // 4. PLY ASCII roundtrip
    {
        QString plyAsciiPath = tempDir.filePath("cube_ascii.ply");
        bool saveOk = MeshIO::savePLY(plyAsciiPath, originalVertices, originalFaces, err, false);
        TEST_ASSERT(saveOk, "savePLY ASCII");

        std::vector<AutoRemesher::Vector3> loadedVertices;
        std::vector<std::vector<size_t>> loadedFaces;
        bool loadOk = MeshIO::loadMesh(plyAsciiPath, loadedVertices, loadedFaces, err);
        TEST_ASSERT(loadOk, "loadMesh PLY ASCII");
        TEST_ASSERT(loadedVertices.size() == originalVertices.size(), "PLY ASCII vertex count matches");
        TEST_ASSERT(loadedFaces.size() == originalFaces.size(), "PLY ASCII face count matches");
    }

    // 5. PLY binary roundtrip
    {
        QString plyBinPath = tempDir.filePath("cube_bin.ply");
        bool saveOk = MeshIO::savePLY(plyBinPath, originalVertices, originalFaces, err, true);
        TEST_ASSERT(saveOk, "savePLY binary");

        std::vector<AutoRemesher::Vector3> loadedVertices;
        std::vector<std::vector<size_t>> loadedFaces;
        bool loadOk = MeshIO::loadMesh(plyBinPath, loadedVertices, loadedFaces, err);
        TEST_ASSERT(loadOk, "loadMesh PLY binary");
        TEST_ASSERT(loadedVertices.size() == originalVertices.size(), "PLY binary vertex count matches");
        TEST_ASSERT(loadedFaces.size() == originalFaces.size(), "PLY binary face count matches");
    }

    // 6. Error handling
    {
        std::vector<AutoRemesher::Vector3> dummyV;
        std::vector<std::vector<size_t>> dummyF;
        bool loadOk = MeshIO::loadMesh(tempDir.filePath("non_existent_file.obj"), dummyV, dummyF, err);
        TEST_ASSERT(!loadOk, "loadMesh non-existent file returns false");
        TEST_ASSERT(!err.empty(), "loadMesh non-existent file sets error string");
    }
}

void testAutoRemesherConfigAndCancellation()
{
    std::cout << "[RUN] testAutoRemesherConfigAndCancellation..." << std::endl;

    std::vector<AutoRemesher::Vector3> vertices;
    std::vector<std::vector<size_t>> faces;
    makeCubeMesh(vertices, faces);

    AutoRemesher::AutoRemesher remesher(vertices, faces);

    // Initial cancellation state
    TEST_ASSERT(!remesher.isCancelled(), "AutoRemesher starts not cancelled");
    remesher.cancel();
    TEST_ASSERT(remesher.isCancelled(), "AutoRemesher isCancelled() after cancel()");

    // Remesh iterations
    remesher.setRemeshIterations(3);
    TEST_ASSERT(remesher.remeshIterations() == 3, "AutoRemesher remeshIterations()");
    remesher.setRemeshIterations(5);
    TEST_ASSERT(remesher.remeshIterations() == 5, "AutoRemesher remeshIterations() updated");

    // Model type
    remesher.setModelType(AutoRemesher::ModelType::HardSurface);
    // Should not crash
    TEST_ASSERT(true, "AutoRemesher setModelType HardSurface executed");
    remesher.setModelType(AutoRemesher::ModelType::Organic);
    TEST_ASSERT(true, "AutoRemesher setModelType Organic executed");

    // QuadMeshGenerator cancellation propagation
    QuadMeshGenerator generator(vertices, faces);
    TEST_ASSERT(!generator.isCancelled(), "QuadMeshGenerator starts not cancelled");
    generator.cancel();
    TEST_ASSERT(generator.isCancelled(), "QuadMeshGenerator isCancelled() after cancel()");
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    std::cout << "========================================" << std::endl;
    std::cout << "AutoRemesher Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;

    testVector3Math();
    testMeshIOFormatDetection();
    testMeshIORoundtrips();
    testAutoRemesherConfigAndCancellation();

    std::cout << "========================================" << std::endl;
    std::cout << "Results: " << g_testsPassed << " passed, "
              << g_testsFailed << " failed." << std::endl;
    std::cout << "========================================" << std::endl;

    return g_testsFailed == 0 ? 0 : 1;
}
