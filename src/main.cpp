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
#include "mainwindow.h"
#include "preferences.h"
#include "theme.h"
#include "tiny_obj_loader.h"
#include "version.h"
#include <AutoRemesher/AutoRemesher>
#include <AutoRemesher/Vector3>
#include <QApplication>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QFileDevice>
#include <QFontDatabase>
#include <QScreen>
#include <QSettings>
#include <QStyleFactory>
#include <QSurfaceFormat>
#include <QTextStream>
#include <QTimer>
#include <QTranslator>
#include <QtGlobal>
#include <cmath>
#include <geogram/basic/common.h>
#include <iostream>
#include <memory>

struct HeadlessParams {
    QString inputPath;
    QString outputPath;
    QString reportPath;
    int targetQuads = 50000;
    double edgeScaling = 1.0;
    double sharpEdgeDegrees = 90.0;
    double smoothNormalDegrees = 0.0;
    double adaptivity = 1.0;
};

struct MeshData {
    std::vector<AutoRemesher::Vector3> vertices;
    std::vector<std::vector<size_t>> triangles;
};

static bool hasHeadlessInputArg(int argc, char** argv)
{
    for (int i = 1; i < argc; ++i) {
        QString arg = QString::fromLocal8Bit(argv[i]);
        if (arg == "-i" || arg == "--input" || arg.startsWith("--input="))
            return true;
    }
    return false;
}

static bool addTriangle(const tinyobj::index_t* indices,
    const std::vector<AutoRemesher::Vector3>& vertices,
    std::vector<std::vector<size_t>>* triangles)
{
    std::vector<size_t> triangle;
    triangle.reserve(3);
    for (size_t i = 0; i < 3; ++i) {
        int vertexIndex = indices[i].vertex_index;
        if (vertexIndex < 0 || static_cast<size_t>(vertexIndex) >= vertices.size()) {
            std::cerr << "Error: Invalid OBJ vertex index " << vertexIndex << std::endl;
            return false;
        }
        triangle.push_back(static_cast<size_t>(vertexIndex));
    }
    if (triangle[0] == triangle[1] || triangle[1] == triangle[2] || triangle[2] == triangle[0])
        return true;
    if (AutoRemesher::Vector3::area(vertices[triangle[0]], vertices[triangle[1]], vertices[triangle[2]]) <= 0.0)
        return true;
    triangles->push_back(triangle);
    return true;
}

static bool loadObjMesh(const QString& filename, MeshData* mesh)
{
    tinyobj::attrib_t attributes;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn;
    std::string err;

    bool loadSuccess = tinyobj::LoadObj(&attributes, &shapes, &materials, &warn, &err, filename.toUtf8().constData());
    if (!warn.empty())
        std::cerr << "Warning: " << warn << std::endl;
    if (!err.empty())
        std::cerr << "Error: " << err << std::endl;
    if (!loadSuccess)
        return false;
    if (attributes.vertices.size() % 3 != 0) {
        std::cerr << "Error: Invalid OBJ vertex array size" << std::endl;
        return false;
    }

    MeshData parsedMesh;
    parsedMesh.vertices.resize(attributes.vertices.size() / 3);
    for (size_t i = 0, j = 0; i < parsedMesh.vertices.size(); ++i) {
        auto& vertex = parsedMesh.vertices[i];
        vertex.setX(attributes.vertices[j++]);
        vertex.setY(attributes.vertices[j++]);
        vertex.setZ(attributes.vertices[j++]);
    }

    for (const auto& shape : shapes) {
        size_t indexOffset = 0;
        if (shape.mesh.num_face_vertices.empty()) {
            if (shape.mesh.indices.size() % 3 != 0) {
                std::cerr << "Error: Invalid OBJ index count" << std::endl;
                return false;
            }
            for (; indexOffset < shape.mesh.indices.size(); indexOffset += 3) {
                if (!addTriangle(&shape.mesh.indices[indexOffset], parsedMesh.vertices, &parsedMesh.triangles))
                    return false;
            }
            continue;
        }

        for (const auto& faceVertexCount : shape.mesh.num_face_vertices) {
            if (indexOffset + faceVertexCount > shape.mesh.indices.size()) {
                std::cerr << "Error: OBJ face/index counts do not match" << std::endl;
                return false;
            }
            if (faceVertexCount != 3) {
                std::cerr << "Error: Expected triangulated OBJ faces" << std::endl;
                return false;
            }
            if (!addTriangle(&shape.mesh.indices[indexOffset], parsedMesh.vertices, &parsedMesh.triangles))
                return false;
            indexOffset += faceVertexCount;
        }
        if (indexOffset != shape.mesh.indices.size()) {
            std::cerr << "Error: OBJ face/index counts do not match" << std::endl;
            return false;
        }
    }

    if (parsedMesh.vertices.empty() || parsedMesh.triangles.empty()) {
        std::cerr << "Error: OBJ contains no remeshable triangles" << std::endl;
        return false;
    }

    *mesh = parsedMesh;
    return true;
}

static bool writeObjFile(const QString& filename,
    const std::vector<AutoRemesher::Vector3>& vertices,
    const std::vector<std::vector<size_t>>& faces)
{
    for (const auto& face : faces) {
        if (face.size() < 3)
            return false;
        for (const auto& vertexIndex : face) {
            if (vertexIndex >= vertices.size())
                return false;
        }
    }

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream stream(&file);
    stream << "# " << APP_NAME << " " << APP_HUMAN_VER << "\n";
    stream << "# " << APP_HOMEPAGE_URL << "\n";
    for (const auto& vertex : vertices) {
        stream << "v " << vertex.x() << " " << vertex.y() << " " << vertex.z() << "\n";
    }
    for (const auto& face : faces) {
        stream << "f";
        for (const auto& vertexIndex : face)
            stream << " " << (1 + vertexIndex);
        stream << "\n";
    }
    stream.flush();
    file.close();
    return stream.status() == QTextStream::Ok && file.error() == QFileDevice::NoError;
}

static int writeHeadlessReport(const HeadlessParams& params,
    size_t quadCount,
    size_t nonQuadCount,
    size_t vertexCount,
    double elapsedSeconds)
{
    std::cout << "=== AutoRemesher Report ===" << std::endl;
    std::cout << "Input: " << params.inputPath.toStdString() << std::endl;
    std::cout << "Output: " << params.outputPath.toStdString() << std::endl;
    std::cout << "Quads: " << quadCount << std::endl;
    std::cout << "Non-quads: " << nonQuadCount << std::endl;
    std::cout << "Vertices: " << vertexCount << std::endl;
    std::cout << "Time: " << elapsedSeconds << " seconds" << std::endl;
    std::cout << "===========================" << std::endl;

    if (params.reportPath.isEmpty())
        return 0;

    QFile reportFile(params.reportPath);
    if (!reportFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        std::cerr << "Error: Failed to write report " << params.reportPath.toStdString() << std::endl;
        return 1;
    }

    QTextStream out(&reportFile);
    out << "AutoRemesher Report\n";
    out << "===================\n\n";
    out << "Input file: " << params.inputPath << "\n";
    out << "Output file: " << params.outputPath << "\n";
    out << "Target quads: " << params.targetQuads << "\n";
    out << "Edge scaling: " << params.edgeScaling << "\n";
    out << "Sharp edge degrees: " << params.sharpEdgeDegrees << "\n";
    out << "Smooth normal degrees: " << params.smoothNormalDegrees << "\n";
    out << "Adaptivity: " << params.adaptivity << "\n\n";
    out << "Results:\n";
    out << "  Quads: " << quadCount << "\n";
    out << "  Non-quads: " << nonQuadCount << "\n";
    out << "  Vertices: " << vertexCount << "\n";
    out << "  Total time: " << elapsedSeconds << " seconds\n";

    out.flush();
    reportFile.close();
    return (out.status() == QTextStream::Ok && reportFile.error() == QFileDevice::NoError) ? 0 : 1;
}

static int runHeadless(const HeadlessParams& params)
{
    MeshData mesh;
    if (!loadObjMesh(params.inputPath, &mesh))
        return 1;

    QElapsedTimer timer;
    timer.start();

    AutoRemesher::AutoRemesher autoRemesher(mesh.vertices, mesh.triangles);
    autoRemesher.setTargetTriangleCount(static_cast<size_t>(params.targetQuads) * 2);
    autoRemesher.setScaling(params.edgeScaling);
    autoRemesher.setSharpEdgeDegrees(params.sharpEdgeDegrees);
    autoRemesher.setSmoothNormalDegrees(params.smoothNormalDegrees);
    autoRemesher.setGradientAdaptivity(params.adaptivity);

    if (!autoRemesher.remesh()) {
        std::cerr << "Error: Remeshing failed" << std::endl;
        return 1;
    }

    const auto& vertices = autoRemesher.remeshedVertices();
    const auto& faces = autoRemesher.remeshedQuads();
    if (!writeObjFile(params.outputPath, vertices, faces)) {
        std::cerr << "Error: Failed to write " << params.outputPath.toStdString() << std::endl;
        return 1;
    }

    size_t quadCount = 0;
    size_t nonQuadCount = 0;
    for (const auto& face : faces) {
        if (face.size() == 4)
            ++quadCount;
        else
            ++nonQuadCount;
    }

    double elapsedSeconds = timer.elapsed() / 1000.0;
    return writeHeadlessReport(params, quadCount, nonQuadCount, vertices.size(), elapsedSeconds);
}

static bool parseIntOption(QCommandLineParser& parser, const QString& optionName,
    int minValue, int maxValue, int* value)
{
    if (!parser.isSet(optionName))
        return true;

    bool ok = false;
    int parsedValue = parser.value(optionName).toInt(&ok);
    if (!ok || parsedValue < minValue || parsedValue > maxValue) {
        std::cerr << "Error: --" << optionName.toStdString()
                  << " must be an integer in range "
                  << minValue << "-" << maxValue << std::endl;
        return false;
    }

    *value = parsedValue;
    return true;
}

static bool parseDoubleOption(QCommandLineParser& parser, const QString& optionName,
    double minValue, double maxValue, double* value)
{
    if (!parser.isSet(optionName))
        return true;

    bool ok = false;
    double parsedValue = parser.value(optionName).toDouble(&ok);
    if (!ok || !std::isfinite(parsedValue) || parsedValue < minValue || parsedValue > maxValue) {
        std::cerr << "Error: --" << optionName.toStdString()
                  << " must be a number in range "
                  << minValue << "-" << maxValue << std::endl;
        return false;
    }

    *value = parsedValue;
    return true;
}

static bool parseHeadlessArgs(QCommandLineParser& parser, HeadlessParams* params)
{
    params->inputPath = parser.value("input");
    params->outputPath = parser.value("output");
    if (parser.isSet("report"))
        params->reportPath = parser.value("report");

    if (!parseIntOption(parser, "target-quads", 1000, 1000000, &params->targetQuads))
        return false;
    if (!parseDoubleOption(parser, "edge-scaling", 1.0, 4.0, &params->edgeScaling))
        return false;
    if (!parseDoubleOption(parser, "sharp-edge", 30.0, 180.0, &params->sharpEdgeDegrees))
        return false;
    if (!parseDoubleOption(parser, "smooth-normal", 0.0, 180.0, &params->smoothNormalDegrees))
        return false;
    if (!parseDoubleOption(parser, "adaptivity", 0.0, 1.0, &params->adaptivity))
        return false;

    return true;
}

int main(int argc, char** argv)
{
    bool requestedHeadlessMode = hasHeadlessInputArg(argc, argv);
    std::unique_ptr<QCoreApplication> app;
    if (requestedHeadlessMode)
        app.reset(new QCoreApplication(argc, argv));
    else
        app.reset(new QApplication(argc, argv));

    QCoreApplication::setApplicationName(APP_NAME);
    QCoreApplication::setOrganizationName(APP_COMPANY);
    QCoreApplication::setOrganizationDomain(APP_HOMEPAGE_URL);

    QCommandLineParser parser;
    parser.setApplicationDescription("AutoRemesher - Automatic quad remeshing tool");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption inputOption(QStringList { "i", "input" },
        QCoreApplication::translate("main", "Input .obj file to remesh"),
        QCoreApplication::translate("main", "file.obj"));
    parser.addOption(inputOption);

    QCommandLineOption outputOption(QStringList { "o", "output" },
        QCoreApplication::translate("main", "Output .obj file path for the remeshed result"),
        QCoreApplication::translate("main", "output.obj"));
    parser.addOption(outputOption);

    QCommandLineOption reportOption(QStringList { "report" },
        QCoreApplication::translate("main", "Path to write a report file with stats (quads, non-quads, vertices, time). If omitted, no report file is written."),
        QCoreApplication::translate("main", "report.txt"));
    parser.addOption(reportOption);

    QCommandLineOption targetQuadsOption(QStringList { "target-quads" },
        QCoreApplication::translate("main", "Target quad count (default: 50000)"),
        QCoreApplication::translate("main", "count"));
    parser.addOption(targetQuadsOption);

    QCommandLineOption edgeScalingOption(QStringList { "edge-scaling" },
        QCoreApplication::translate("main", "Edge scaling factor (default: 1.0, range: 1.0-4.0)"),
        QCoreApplication::translate("main", "factor"));
    parser.addOption(edgeScalingOption);

    QCommandLineOption sharpEdgeOption(QStringList { "sharp-edge" },
        QCoreApplication::translate("main", "Sharp edge dihedral angle threshold in degrees (default: 90.0, range: 30.0-180.0)"),
        QCoreApplication::translate("main", "degrees"));
    parser.addOption(sharpEdgeOption);

    QCommandLineOption smoothNormalOption(QStringList { "smooth-normal" },
        QCoreApplication::translate("main", "Smooth normal angle threshold in degrees (default: 0.0, range: 0.0-180.0)"),
        QCoreApplication::translate("main", "degrees"));
    parser.addOption(smoothNormalOption);

    QCommandLineOption adaptivityOption(QStringList { "adaptivity" },
        QCoreApplication::translate("main", "Curvature-adaptive quad density (default: 1.0, range: 0.0-1.0)"),
        QCoreApplication::translate("main", "value"));
    parser.addOption(adaptivityOption);

    parser.process(*app);

    bool headlessMode = parser.isSet("input");

    GEO::initialize();

    if (headlessMode) {
        HeadlessParams params;
        if (!parseHeadlessArgs(parser, &params))
            return 1;
        if (params.outputPath.isEmpty()) {
            std::cerr << "Error: --output is required when --input is specified" << std::endl;
            return 1;
        }
        return runHeadless(params);
    }

    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setProfile(QSurfaceFormat::OpenGLContextProfile::CoreProfile);
    format.setVersion(3, 3);
    QSurfaceFormat::setDefaultFormat(format);

    qApp->setStyle(QStyleFactory::create("Fusion"));
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, Theme::black);
    darkPalette.setColor(QPalette::WindowText, Theme::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::Text, Theme::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::Text, Theme::black);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Theme::white);
    darkPalette.setColor(QPalette::BrightText, Theme::green);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, Theme::green);
    darkPalette.setColor(QPalette::HighlightedText, Theme::black);
    qApp->setPalette(darkPalette);
    qApp->setStyleSheet(Theme::compactStylesheet());

    QFont font;
    font.setWeight(QFont::Light);
    font.setBold(false);
    QApplication::setFont(font);

    Theme::initAwsomeBaseSizes();

    MainWindow* mainWindow = new MainWindow();
    mainWindow->setAttribute(Qt::WA_DeleteOnClose);

    QSize size = Preferences::instance().mainWindowSize();
    if (size.isValid()) {
        mainWindow->resize(size);
    } else {
        mainWindow->showMaximized();
    }
    mainWindow->show();
    if (!mainWindow->isMaximized()) {
        QScreen* screen = QApplication::primaryScreen();
        if (screen) {
            QRect screenRect = screen->availableGeometry();
            mainWindow->move((screenRect.width() - mainWindow->frameGeometry().width()) / 2,
                (screenRect.height() - mainWindow->frameGeometry().height()) / 2);
        }
    }

    return app->exec();
}
