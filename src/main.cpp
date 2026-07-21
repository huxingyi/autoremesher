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
#include "version.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QEventLoop>
#include <QFile>
#include <QFontDatabase>
#include <QScreen>
#include <QSettings>
#include <QStyleFactory>
#include <QSurfaceFormat>
#include <QTextStream>
#include <QTimer>
#include <QTranslator>
#include <QtGlobal>
#include <geogram/basic/common.h>
#include <iostream>

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

static HeadlessParams parseHeadlessArgs(QCommandLineParser& parser)
{
    HeadlessParams params;
    params.inputPath = parser.value("input");
    params.outputPath = parser.value("output");
    if (parser.isSet("report"))
        params.reportPath = parser.value("report");
    if (parser.isSet("target-quads"))
        params.targetQuads = parser.value("target-quads").toInt();
    if (parser.isSet("edge-scaling"))
        params.edgeScaling = parser.value("edge-scaling").toDouble();
    if (parser.isSet("sharp-edge"))
        params.sharpEdgeDegrees = parser.value("sharp-edge").toDouble();
    if (parser.isSet("smooth-normal"))
        params.smoothNormalDegrees = parser.value("smooth-normal").toDouble();
    if (parser.isSet("adaptivity"))
        params.adaptivity = parser.value("adaptivity").toDouble();
    return params;
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QCoreApplication::setApplicationName(APP_NAME);
    QCoreApplication::setOrganizationName(APP_COMPANY);
    QCoreApplication::setOrganizationDomain(APP_HOMEPAGE_URL);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

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

    parser.process(app);

    bool headlessMode = parser.isSet("input");

    GEO::initialize();

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

    if (headlessMode) {
        HeadlessParams params = parseHeadlessArgs(parser);
        if (params.outputPath.isEmpty()) {
            std::cerr << "Error: --output is required when --input is specified" << std::endl;
            return 1;
        }

        QObject::connect(mainWindow, &MainWindow::headlessFinished,
            [&](size_t quadCount, size_t nonQuadCount, size_t vertexCount, double elapsedSeconds) {
                std::cout << "=== AutoRemesher Report ===" << std::endl;
                std::cout << "Input: " << params.inputPath.toStdString() << std::endl;
                std::cout << "Output: " << params.outputPath.toStdString() << std::endl;
                std::cout << "Quads: " << quadCount << std::endl;
                std::cout << "Non-quads: " << nonQuadCount << std::endl;
                std::cout << "Vertices: " << vertexCount << std::endl;
                std::cout << "Time: " << elapsedSeconds << " seconds" << std::endl;
                std::cout << "===========================" << std::endl;

                // Write a report file if --report was specified
                if (!params.reportPath.isEmpty()) {
                    QFile reportFile(params.reportPath);
                    if (reportFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
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
                        reportFile.close();
                    }
                }

                QCoreApplication::quit();
            });

        mainWindow->setHeadlessParams(params.inputPath, params.outputPath,
            params.targetQuads, params.edgeScaling,
            params.sharpEdgeDegrees, params.smoothNormalDegrees,
            params.adaptivity);
        mainWindow->runHeadless();

        return app.exec();
    }

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

    return app.exec();
}
