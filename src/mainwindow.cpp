/*
 *  Copyright (c) 2020 Jeremy HU <jeremy-at-dust3d dot org>. All rights reserved.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */
#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QDebug>
#include <QDesktopServices>
#include <QDockWidget>
#include <QFile>
#include <QFileDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QTextBrowser>
#include <QTextStream>
#include <QThread>
#include <QUrl>
#include <QUuid>
#include <QVBoxLayout>
#include <cmath>
#include <iostream>
#ifdef Q_OS_WIN32
#include <QWinTaskbarButton>
#include <QWinTaskbarProgress>
#endif
#include "aboutwidget.h"
#include "floatnumberwidget.h"
#include "graphicscontainerwidget.h"
#include "intnumberwidget.h"
#include "logbrowser.h"
#include "mainwindow.h"
#include "preferences.h"
#include "quadmeshgenerator.h"
#include "rendermeshgenerator.h"
#include "theme.h"
#include "updatescheckwidget.h"
#include "util.h"
#include "version.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

LogBrowser* g_logBrowser = nullptr;
QTextBrowser* g_acknowlegementsWidget = nullptr;
QTextBrowser* g_supportersWidget = nullptr;
AboutWidget* g_aboutWidget = nullptr;
UpdatesCheckWidget* g_updatesCheckWidget = nullptr;
std::map<MainWindow*, QUuid> g_windows;

void outputMessage(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    if (g_logBrowser)
        g_logBrowser->outputMessage(type, msg, context.file, context.line);
}

size_t MainWindow::total()
{
    return g_windows.size();
}

MainWindow::MainWindow()
{
    if (!g_logBrowser) {
        g_logBrowser = new LogBrowser;
        qInstallMessageHandler(&outputMessage);
    }

    g_windows.insert({ this, QUuid::createUuid() });

#ifdef Q_OS_WIN32
    m_taskbarButton = new QWinTaskbarButton(this);
#endif

    GraphicsWidget* graphicsWidget = new GraphicsWidget;

    GraphicsContainerWidget* containerWidget = new GraphicsContainerWidget;
    containerWidget->setGraphicsWidget(graphicsWidget);
    QGridLayout* containerLayout = new QGridLayout;
    containerLayout->setSpacing(0);
    containerLayout->setContentsMargins(1, 0, 0, 0);
    containerLayout->addWidget(graphicsWidget);
    containerWidget->setLayout(containerLayout);
    containerWidget->setMinimumSize(400, 400);

    QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));

    QAction* gotoHomepageAction = new QAction(tr("Homepage"), this);
    connect(gotoHomepageAction, &QAction::triggered, this, &MainWindow::gotoHomepage);
    helpMenu->addAction(gotoHomepageAction);

    QAction* viewSourceAction = new QAction(tr("Source Code"), this);
    connect(viewSourceAction, &QAction::triggered, this, &MainWindow::viewSource);
    helpMenu->addAction(viewSourceAction);

    QAction* checkForUpdatesAction = new QAction(tr("Check for Updates..."), this);
    connect(checkForUpdatesAction, &QAction::triggered, this, &MainWindow::checkForUpdates);
    helpMenu->addAction(checkForUpdatesAction);

    helpMenu->addSeparator();

    QAction* aboutAction = new QAction(tr("About"), this);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAbout);
    helpMenu->addAction(aboutAction);

    QAction* reportIssuesAction = new QAction(tr("Report Issues"), this);
    connect(reportIssuesAction, &QAction::triggered, this, &MainWindow::reportIssues);
    helpMenu->addAction(reportIssuesAction);

    QAction* showDebugDialogAction = new QAction(tr("Debug"), this);
    connect(showDebugDialogAction, &QAction::triggered, g_logBrowser, &LogBrowser::showDialog);
    helpMenu->addAction(showDebugDialogAction);

    helpMenu->addSeparator();

    QAction* seeSupportersAction = new QAction(tr("Supporters"), this);
    connect(seeSupportersAction, &QAction::triggered, this, &MainWindow::showSupporters);
    helpMenu->addAction(seeSupportersAction);

    QAction* showAcknowlegementsAction = new QAction(tr("Acknowlegements"), this);
    connect(showAcknowlegementsAction, &QAction::triggered, this, &MainWindow::showAcknowlegements);
    helpMenu->addAction(showAcknowlegementsAction);

    m_modelRenderWidget = new ModelShaderWidget(containerWidget);
    m_modelRenderWidget->setMoveAndZoomByWindow(false);
    m_modelRenderWidget->move(0, 0);
    m_modelRenderWidget->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_modelRenderWidget->toggleWireframe();
    m_modelRenderWidget->disableCullFace();
    m_modelRenderWidget->setEyePosition(QVector3D(0.0, 0.0, -4.0));

    connect(containerWidget, &GraphicsContainerWidget::containerSizeChanged,
        m_modelRenderWidget, &ModelShaderWidget::canvasResized);

    graphicsWidget->setModelWidget(m_modelRenderWidget);
    containerWidget->setModelWidget(m_modelRenderWidget);

    // ============================================================
    // PROGRESS BAR — thin micro-line at top of window
    // ============================================================

    m_progressBar = new QProgressBar;
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(false);
    m_progressBar->setFixedHeight(2);
    m_progressBar->hide();

    m_progressContainer = new QWidget;
    m_progressContainer->setFixedHeight(2);
    QVBoxLayout* progressLayout = new QVBoxLayout(m_progressContainer);
    progressLayout->setContentsMargins(0, 0, 0, 0);
    progressLayout->setSpacing(0);
    progressLayout->addWidget(m_progressBar);

    // ============================================================
    // RIGHT-SIDE CONTROLS PANEL
    // ============================================================

    m_sharpEdgeDegreesWidget = new FloatNumberWidget(this, false);
    m_sharpEdgeDegreesWidget->setItemName(tr("Sharp Edge"));
    m_sharpEdgeDegreesWidget->setRange(30.0, 180.0);
    m_sharpEdgeDegreesWidget->setValue(m_sharpEdgeDegrees);
    m_sharpEdgeDegreesWidget->setToolTip(tr("Dihedral angle threshold (degrees). Edges sharper than this are preserved as feature edges."));
    connect(m_sharpEdgeDegreesWidget, &FloatNumberWidget::valueChanged, [=](float value) {
        m_sharpEdgeDegrees = value;
    });

    m_smoothNormalDegreesWidget = new FloatNumberWidget(this, false);
    m_smoothNormalDegreesWidget->setItemName(tr("Smooth Normal"));
    m_smoothNormalDegreesWidget->setRange(0.0, 180.0);
    m_smoothNormalDegreesWidget->setValue(m_smoothNormalDegrees);
    m_smoothNormalDegreesWidget->setToolTip(tr("Smooth normal angle threshold (degrees). 0 = faceted (current behavior), larger values produce a smoother surface during remeshing by respecting the original vertex normals."));
    connect(m_smoothNormalDegreesWidget, &FloatNumberWidget::valueChanged, [=](float value) {
        m_smoothNormalDegrees = value;
    });

    m_adaptivityWidget = new FloatNumberWidget(this, false);
    m_adaptivityWidget->setItemName(tr("Adaptivity"));
    m_adaptivityWidget->setRange(0.0, 1.0);
    m_adaptivityWidget->setValue(m_adaptivity);
    m_adaptivityWidget->setToolTip(tr("Curvature-adaptive quad density. 0 = uniform, 1 = full adaptivity (finer quads in high-curvature areas)."));
    connect(m_adaptivityWidget, &FloatNumberWidget::valueChanged, [=](float value) {
        m_adaptivity = value;
    });

    m_targetQuadCountWidget = new IntNumberWidget(this, false);
    m_targetQuadCountWidget->setItemName(tr("Target Quads"));
    m_targetQuadCountWidget->setRange(1000, 1000000);
    m_targetQuadCountWidget->setValue(m_targetQuadCount);
    m_targetQuadCountWidget->setSuffix(tr(" quads"));
    connect(m_targetQuadCountWidget, &IntNumberWidget::valueChanged, [=](int value) {
        m_targetQuadCount = value;
    });

    m_targetScalingWidget = new FloatNumberWidget(this, false);
    m_targetScalingWidget->setItemName(tr("Edge Scaling"));
    m_targetScalingWidget->setRange(1.0, 4.0);
    m_targetScalingWidget->setValue(m_targetScaling);
    connect(m_targetScalingWidget, &FloatNumberWidget::valueChanged, [=](float value) {
        m_targetScaling = value;
    });

    //m_modelTypeSelectBox = new QComboBox;
    //m_modelTypeSelectBox->addItem(tr("Organic"));
    //m_modelTypeSelectBox->addItem(tr("Hard surface"));
    //connect(m_modelTypeSelectBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [&](int index) {
    //    m_modelType = 1 == index ? AutoRemesher::ModelType::HardSurface : AutoRemesher::ModelType::Organic;
    //});
    //m_modelTypeSelectBox->setCurrentIndex(AutoRemesher::ModelType::HardSurface == m_modelType ? 1 : 0);

    // --- Action buttons ---
    QPushButton* loadModelButton = new QPushButton(tr("Open"));
    loadModelButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    connect(loadModelButton, &QPushButton::clicked, this, &MainWindow::loadModel);
    m_loadModelButton = loadModelButton;

    QPushButton* saveMeshButton = new QPushButton(tr("Save"));
    saveMeshButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    saveMeshButton->hide();
    connect(saveMeshButton, &QPushButton::clicked, this, &MainWindow::saveMesh);
    m_saveMeshButton = saveMeshButton;

    QPushButton* regenerateButton = new QPushButton(tr("Regenerate"));
    regenerateButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    regenerateButton->hide();
    connect(regenerateButton, &QPushButton::clicked, this, &MainWindow::generateQuadMesh);
    m_regenerateButton = regenerateButton;

    // --- Controls panel layout ---
    QVBoxLayout* controlsLayout = new QVBoxLayout;
    controlsLayout->setSpacing(2);
    controlsLayout->setContentsMargins(6, 6, 6, 6);

    controlsLayout->addWidget(m_sharpEdgeDegreesWidget);
    controlsLayout->addWidget(m_smoothNormalDegreesWidget);
    controlsLayout->addWidget(m_adaptivityWidget);
    controlsLayout->addWidget(m_targetQuadCountWidget);
    controlsLayout->addWidget(m_targetScalingWidget);
    //controlsLayout->addWidget(m_modelTypeSelectBox);

    // Result mesh stats (hidden until a mesh is generated)
    m_quadCountLabel = new QLabel(this);
    m_quadCountLabel->setStyleSheet("color: #ffffff; font-size: 11px; padding: 2px 0;");
    m_quadCountLabel->hide();

    m_nonQuadCountLabel = new QLabel(this);
    m_nonQuadCountLabel->setStyleSheet("color: #ffffff; font-size: 11px; padding: 2px 0;");
    m_nonQuadCountLabel->hide();

    m_vertexCountLabel = new QLabel(this);
    m_vertexCountLabel->setStyleSheet("color: #ffffff; font-size: 11px; padding: 2px 0;");
    m_vertexCountLabel->hide();

    // Toolbar rows at bottom
    QHBoxLayout* toolbarLayout = new QHBoxLayout;
    toolbarLayout->setSpacing(4);
    toolbarLayout->setContentsMargins(0, 12, 0, 0);
    toolbarLayout->addWidget(loadModelButton, 1);
    toolbarLayout->addWidget(regenerateButton, 1);
    controlsLayout->addLayout(toolbarLayout);

    QHBoxLayout* saveLayout = new QHBoxLayout;
    saveLayout->setSpacing(0);
    saveLayout->setContentsMargins(0, 2, 0, 0);
    saveLayout->addWidget(saveMeshButton, 1);
    controlsLayout->addLayout(saveLayout);

    controlsLayout->addWidget(m_quadCountLabel);
    controlsLayout->addWidget(m_nonQuadCountLabel);
    controlsLayout->addWidget(m_vertexCountLabel);

    controlsLayout->addStretch();

    QWidget* controlsPanel = new QWidget;
    controlsPanel->setLayout(controlsLayout);
    controlsPanel->setFixedWidth(220);
    controlsPanel->setObjectName("controlsPanel");
    controlsPanel->setStyleSheet(
        "#controlsPanel {"
        "  background-color: #242424;"
        "  border: 1px solid #2a2a2a;"
        "  border-radius: 4px;"
        "}");

    // ============================================================
    // CANVAS AREA
    // ============================================================

    QLabel* verticalLogoLabel = new QLabel;
    QImage verticalLogoImage;
    verticalLogoImage.load(":/resources/dust3d-vertical.png");
    verticalLogoLabel->setPixmap(QPixmap::fromImage(verticalLogoImage));

    QHBoxLayout* logoLayout = new QHBoxLayout;
    logoLayout->addWidget(verticalLogoLabel);
    logoLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout* mainLeftLayout = new QVBoxLayout;
    mainLeftLayout->setSpacing(0);
    mainLeftLayout->setContentsMargins(0, 0, 0, 0);
    mainLeftLayout->addStretch();
    mainLeftLayout->addLayout(logoLayout);
    mainLeftLayout->addSpacing(10);

    QHBoxLayout* canvasLayout = new QHBoxLayout;
    canvasLayout->setSpacing(0);
    canvasLayout->setContentsMargins(0, 4, 8, 4);
    canvasLayout->addLayout(mainLeftLayout);
    canvasLayout->addWidget(containerWidget);
    canvasLayout->addSpacing(6);
    canvasLayout->addWidget(controlsPanel);

    // ============================================================
    // MAIN LAYOUT
    // ============================================================

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(m_progressContainer);
    mainLayout->addLayout(canvasLayout, 1);

    QWidget* centralWidget = new QWidget;
    centralWidget->setLayout(mainLayout);

    setCentralWidget(centralWidget);
    updateTitle();
}

void MainWindow::updateButtonStates()
{
    if (nullptr == m_quadMeshGenerator && !m_quadMeshResultIsDirty) {
        m_loadModelButton->setEnabled(true);
        m_targetScalingWidget->setEnabled(true);
        m_targetQuadCountWidget->setEnabled(true);
        m_sharpEdgeDegreesWidget->setEnabled(true);
        m_smoothNormalDegreesWidget->setEnabled(true);
        m_adaptivityWidget->setEnabled(true);
        //m_modelTypeSelectBox->setEnabled(true);
        if (nullptr != m_remeshedQuads) {
            m_saveMeshButton->show();
        } else {
            m_saveMeshButton->hide();
        }
        if (!m_originalVertices.empty()) {
            m_regenerateButton->show();
            m_regenerateButton->setEnabled(true);
        } else {
            m_regenerateButton->hide();
        }
        m_progressBar->hide();
    } else {
        m_loadModelButton->setEnabled(false);
        m_saveMeshButton->hide();
        m_regenerateButton->setEnabled(false);
        m_targetScalingWidget->setDisabled(true);
        m_targetQuadCountWidget->setDisabled(true);
        m_sharpEdgeDegreesWidget->setDisabled(true);
        m_smoothNormalDegreesWidget->setDisabled(true);
        m_adaptivityWidget->setDisabled(true);
        //m_modelTypeSelectBox->setDisabled(true);
    }
}

bool MainWindow::loadObj(const QString& filename)
{
    tinyobj::attrib_t attributes;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    qDebug() << "loadObj:" << filename;

    bool loadSuccess = tinyobj::LoadObj(&attributes, &shapes, &materials, &warn, &err, filename.toUtf8().constData());
    if (!warn.empty()) {
        qDebug() << "WARN:" << warn.c_str();
    }
    if (!err.empty()) {
        qDebug() << err.c_str();
    }
    if (!loadSuccess) {
        return false;
    }

    m_originalVertices.resize(attributes.vertices.size() / 3);
    for (size_t i = 0, j = 0; i < m_originalVertices.size(); ++i) {
        auto& dest = m_originalVertices[i];
        dest.setX(attributes.vertices[j++]);
        dest.setY(attributes.vertices[j++]);
        dest.setZ(attributes.vertices[j++]);
    }

    m_originalTriangles.clear();
    for (const auto& shape : shapes) {
        for (size_t i = 0; i < shape.mesh.indices.size(); i += 3) {
            m_originalTriangles.push_back(std::vector<size_t> {
                (size_t)shape.mesh.indices[i + 0].vertex_index,
                (size_t)shape.mesh.indices[i + 1].vertex_index,
                (size_t)shape.mesh.indices[i + 2].vertex_index });
        }
    }

    qDebug() << "m_originalVertices.size():" << m_originalVertices.size();
    qDebug() << "m_originalTriangles.size():" << m_originalTriangles.size();

    m_renderQueue.push({ m_originalVertices,
        m_originalTriangles });
    checkRenderQueue();

    return true;
}

void MainWindow::loadModel()
{
    if (!m_saved) {
        QMessageBox::StandardButton answer = QMessageBox::question(this,
            APP_NAME,
            tr("Do you really want to load another file and lose the unsaved changes?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
        if (answer != QMessageBox::Yes)
            return;
    }

    if (m_inProgress) {
        QMessageBox::StandardButton answer = QMessageBox::question(this,
            APP_NAME,
            tr("Do you really want to load another file and lose the in progress operations?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
        if (answer != QMessageBox::Yes) {
            return;
        }
    }

    QString filename = QFileDialog::getOpenFileName(this, QString(), QString(),
        tr("Wavefront (*.obj)"));
    if (filename.isEmpty())
        return;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    bool objLoaded = loadObj(filename);
    QApplication::restoreOverrideCursor();

    if (objLoaded) {
        setCurrentFilename(filename);

        generateQuadMesh();
    }
}

void MainWindow::setCurrentFilename(const QString& filename)
{
    m_currentFilename = filename;
    m_saved = true;
    updateTitle();
}

void MainWindow::saveMesh()
{
    if (nullptr == m_remeshedVertices || nullptr == m_remeshedQuads)
        return;

    QString filename = QFileDialog::getSaveFileName(this, QString(), QString(),
        tr("Wavefront (*.obj)"));
    if (filename.isEmpty()) {
        return;
    }

    if (!filename.endsWith(".obj"))
        filename += ".obj";

    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream << "# " << APP_NAME << " " << APP_HUMAN_VER << "\n";
        stream << "# " << APP_HOMEPAGE_URL << "\n";
        for (std::vector<AutoRemesher::Vector3>::const_iterator it = m_remeshedVertices->begin(); it != m_remeshedVertices->end(); ++it) {
            stream << "v " << (*it).x() << " " << (*it).y() << " " << (*it).z() << "\n";
        }
        for (std::vector<std::vector<size_t>>::const_iterator it = m_remeshedQuads->begin(); it != m_remeshedQuads->end(); ++it) {
            stream << "f";
            for (std::vector<size_t>::const_iterator subIt = (*it).begin(); subIt != (*it).end(); ++subIt) {
                stream << " " << (1 + *subIt);
            }
            stream << "\n";
        }
    }
}

void MainWindow::updateTitle()
{
    QString appName = APP_NAME;
    QString appVer = APP_HUMAN_VER;
    setWindowTitle(QString("%1 %2 %3%4").arg(appName).arg(appVer).arg(m_currentFilename).arg(m_saved ? "" : "*"));
}

void MainWindow::updateProgress(float progress)
{
#ifdef Q_OS_WIN32
    m_taskbarButton->progress()->setValue((int)(progress * 100));
#endif
}

void MainWindow::updateProgressDetailed(float progress, const QString& status)
{
    m_progressBar->setValue((int)(progress * 100));
    m_progressBar->show();

    if (progress >= 1.0f) {
        m_progressBar->hide();
    }
}

MainWindow::~MainWindow()
{
    g_windows.erase(this);
}

ModelShaderWidget* MainWindow::modelRenderWidget() const
{
    return m_modelRenderWidget;
}

void MainWindow::showSupporters()
{
    if (!g_supportersWidget) {
        g_supportersWidget = new QTextBrowser;
        g_supportersWidget->setWindowTitle(unifiedWindowTitle(tr("Supporters")));
        g_supportersWidget->setMinimumSize(QSize(320, 280));
        QFile supporters(":/SUPPORTERS");
        supporters.open(QFile::ReadOnly | QFile::Text);
        g_supportersWidget->setHtml("<h1>SUPPORTERS</h1><pre>" + supporters.readAll() + "</pre>");
    }
    g_supportersWidget->show();
    g_supportersWidget->activateWindow();
    g_supportersWidget->raise();
}

void MainWindow::showAcknowlegements()
{
    if (!g_acknowlegementsWidget) {
        g_acknowlegementsWidget = new QTextBrowser;
        g_acknowlegementsWidget->setWindowTitle(unifiedWindowTitle(tr("Acknowlegements")));
        g_acknowlegementsWidget->setMinimumSize(QSize(640, 380));
        QFile file(":/ACKNOWLEDGEMENTS.html");
        file.open(QFile::ReadOnly | QFile::Text);
        QTextStream stream(&file);
        g_acknowlegementsWidget->setHtml(stream.readAll());
    }
    g_acknowlegementsWidget->show();
    g_acknowlegementsWidget->activateWindow();
    g_acknowlegementsWidget->raise();
}

void MainWindow::viewSource()
{
    QString url = APP_REPOSITORY_URL;
    qDebug() << "viewSource:" << url;
    QDesktopServices::openUrl(QUrl(url));
}

void MainWindow::gotoHomepage()
{
    QString url = APP_HOMEPAGE_URL;
    qDebug() << "gotoHomepage:" << url;
    QDesktopServices::openUrl(QUrl(url));
}

void MainWindow::reportIssues()
{
    QString url = APP_ISSUES_URL;
    qDebug() << "reportIssues:" << url;
    QDesktopServices::openUrl(QUrl(url));
}

void MainWindow::showAbout()
{
    if (!g_aboutWidget) {
        g_aboutWidget = new AboutWidget;
    }
    g_aboutWidget->show();
    g_aboutWidget->activateWindow();
    g_aboutWidget->raise();
}

void MainWindow::checkForUpdates()
{
    if (!g_updatesCheckWidget) {
        g_updatesCheckWidget = new UpdatesCheckWidget;
    }
    g_updatesCheckWidget->check();
    g_updatesCheckWidget->show();
    g_updatesCheckWidget->activateWindow();
    g_updatesCheckWidget->raise();
}

void MainWindow::showEvent(QShowEvent* event)
{
#ifdef Q_OS_WIN32
    m_taskbarButton->setWindow(windowHandle());
    m_taskbarButton->progress()->setVisible(true);
#endif

    event->accept();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (!m_saved) {
        QMessageBox::StandardButton answer = QMessageBox::question(this,
            APP_NAME,
            tr("Do you really want to close while there are unsaved changes?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
        if (answer == QMessageBox::No) {
            event->ignore();
            return;
        }
    }

    if (m_inProgress) {
        QMessageBox::StandardButton answer = QMessageBox::question(this,
            APP_NAME,
            tr("Do you really want to close while there are in progress operations?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
        if (answer == QMessageBox::No) {
            event->ignore();
            return;
        }
    }

    QSize saveSize;
    if (!isMaximized())
        saveSize = size();
    Preferences::instance().setMainWindowSize(saveSize);

    event->accept();
}

void MainWindow::checkRenderQueue()
{
    if (nullptr != m_renderMeshGenerator)
        return;

    if (m_renderQueue.empty())
        return;

    qDebug() << "Generate render mesh...";

    QThread* thread = new QThread;

    const auto& item = m_renderQueue.front();
    m_renderMeshGenerator = new RenderMeshGenerator(item.vertices, item.faces);
    m_renderQueue.pop();
    m_renderMeshGenerator->moveToThread(thread);
    connect(thread, &QThread::started, m_renderMeshGenerator, &RenderMeshGenerator::process);
    connect(m_renderMeshGenerator, &RenderMeshGenerator::finished, this, &MainWindow::renderMeshReady);
    connect(m_renderMeshGenerator, &RenderMeshGenerator::finished, thread, &QThread::quit);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
}

void MainWindow::renderMeshReady()
{
    ModelShaderMesh* renderMesh = m_renderMeshGenerator->takeRenderMesh();

    qDebug() << "Render mesh ready";

    delete m_renderMeshGenerator;
    m_renderMeshGenerator = nullptr;

    m_modelRenderWidget->updateMesh(renderMesh);

    checkRenderQueue();
}

void MainWindow::setHeadlessParams(const QString& inputPath, const QString& outputPath,
    int targetQuads, double edgeScaling,
    double sharpEdgeDegrees, double smoothNormalDegrees,
    double adaptivity)
{
    m_headlessMode = true;
    m_headlessOutputPath = outputPath;
    m_currentFilename = inputPath;
    m_targetQuadCount = targetQuads;
    m_targetScaling = static_cast<float>(edgeScaling);
    m_sharpEdgeDegrees = static_cast<float>(sharpEdgeDegrees);
    m_smoothNormalDegrees = static_cast<float>(smoothNormalDegrees);
    m_adaptivity = static_cast<float>(adaptivity);
}

void MainWindow::saveMeshToFile(const QString& filename)
{
    if (nullptr == m_remeshedVertices || nullptr == m_remeshedQuads)
        return;

    QFile file(filename);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << "# " << APP_NAME << " " << APP_HUMAN_VER << "\n";
        stream << "# " << APP_HOMEPAGE_URL << "\n";
        for (std::vector<AutoRemesher::Vector3>::const_iterator it = m_remeshedVertices->begin(); it != m_remeshedVertices->end(); ++it) {
            stream << "v " << (*it).x() << " " << (*it).y() << " " << (*it).z() << "\n";
        }
        for (std::vector<std::vector<size_t>>::const_iterator it = m_remeshedQuads->begin(); it != m_remeshedQuads->end(); ++it) {
            stream << "f";
            for (std::vector<size_t>::const_iterator subIt = (*it).begin(); subIt != (*it).end(); ++subIt) {
                stream << " " << (1 + *subIt);
            }
            stream << "\n";
        }
    }
}

void MainWindow::runHeadless()
{
    m_headlessTimer.start();

    // Load the input file and generate the quad mesh without UI dialogs
    QApplication::setOverrideCursor(Qt::WaitCursor);
    bool objLoaded = loadObj(m_currentFilename);
    QApplication::restoreOverrideCursor();

    if (!objLoaded) {
        std::cerr << "Error: Failed to load " << m_currentFilename.toStdString() << std::endl;
        QCoreApplication::quit();
        return;
    }

    // Start generation
    if (nullptr != m_quadMeshGenerator) {
        m_quadMeshResultIsDirty = true;
        return;
    }

    m_quadMeshResultIsDirty = false;
    m_saved = true;
    m_inProgress = true;

    QThread* thread = new QThread;

    QuadMeshGenerator::Parameters parameters;

    parameters.targetTriangleCount = m_targetQuadCount * 2;
    parameters.scaling = m_targetScaling;
    parameters.modelType = m_modelType;
    parameters.adaptivity = m_adaptivity;
    parameters.sharpEdgeDegrees = m_sharpEdgeDegrees;
    parameters.smoothNormalDegrees = m_smoothNormalDegrees;

    m_quadMeshGenerator = new QuadMeshGenerator(m_originalVertices, m_originalTriangles);
    connect(m_quadMeshGenerator, &QuadMeshGenerator::reportProgress, this, &MainWindow::updateProgress);
    connect(m_quadMeshGenerator, &QuadMeshGenerator::reportProgressDetailed, this, &MainWindow::updateProgressDetailed);
    m_quadMeshGenerator->setParameters(parameters);
    m_quadMeshGenerator->moveToThread(thread);
    connect(thread, &QThread::started, m_quadMeshGenerator, &QuadMeshGenerator::process);
    connect(m_quadMeshGenerator, &QuadMeshGenerator::finished, this, &MainWindow::quadMeshReady);
    connect(m_quadMeshGenerator, &QuadMeshGenerator::finished, thread, &QThread::quit);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
}

void MainWindow::generateQuadMesh()
{
    if (nullptr != m_quadMeshGenerator) {
        m_quadMeshResultIsDirty = true;
        return;
    }

    m_quadMeshResultIsDirty = false;
    m_saved = true;
    m_inProgress = true;

    m_quadCountLabel->hide();
    m_nonQuadCountLabel->hide();
    m_vertexCountLabel->hide();

    m_progressBar->setValue(0);
    m_progressBar->show();

    QThread* thread = new QThread;

    QuadMeshGenerator::Parameters parameters;

    parameters.targetTriangleCount = m_targetQuadCount * 2;
    parameters.scaling = m_targetScaling;
    parameters.modelType = m_modelType;
    parameters.adaptivity = m_adaptivity;
    parameters.sharpEdgeDegrees = m_sharpEdgeDegrees;
    parameters.smoothNormalDegrees = m_smoothNormalDegrees;

    m_quadMeshGenerator = new QuadMeshGenerator(m_originalVertices, m_originalTriangles);
    connect(m_quadMeshGenerator, &QuadMeshGenerator::reportProgress, this, &MainWindow::updateProgress);
    connect(m_quadMeshGenerator, &QuadMeshGenerator::reportProgressDetailed, this, &MainWindow::updateProgressDetailed);
    m_quadMeshGenerator->setParameters(parameters);
    m_quadMeshGenerator->moveToThread(thread);
    connect(thread, &QThread::started, m_quadMeshGenerator, &QuadMeshGenerator::process);
    connect(m_quadMeshGenerator, &QuadMeshGenerator::finished, this, &MainWindow::quadMeshReady);
    connect(m_quadMeshGenerator, &QuadMeshGenerator::finished, thread, &QThread::quit);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();

    updateButtonStates();
    updateTitle();
}

void MainWindow::quadMeshReady()
{
    delete m_remeshedVertices;
    m_remeshedVertices = m_quadMeshGenerator->takeRemeshedVertices();

    delete m_remeshedQuads;
    m_remeshedQuads = m_quadMeshGenerator->takeRemeshedQuads();

    m_saved = false;
    m_inProgress = false;

    delete m_quadMeshGenerator;
    m_quadMeshGenerator = nullptr;

    if (nullptr != m_remeshedVertices && nullptr != m_remeshedQuads) {
        size_t quadCount = 0;
        size_t nonQuadCount = 0;
        for (const auto& face : *m_remeshedQuads) {
            if (face.size() == 4)
                ++quadCount;
            else
                ++nonQuadCount;
        }
        size_t vertexCount = m_remeshedVertices->size();

        if (m_headlessMode) {
            double elapsed = m_headlessTimer.elapsed() / 1000.0;
            saveMeshToFile(m_headlessOutputPath);
            emit headlessFinished(quadCount, nonQuadCount, vertexCount, elapsed);
            return;
        }

        m_quadCountLabel->setText(QString("Quads: %1").arg(quadCount));
        m_nonQuadCountLabel->setText(QString("Non-quads: %1").arg(nonQuadCount));
        m_vertexCountLabel->setText(QString("Vertices: %1").arg(vertexCount));

        m_quadCountLabel->show();
        m_nonQuadCountLabel->show();
        m_vertexCountLabel->show();

        m_renderQueue.push({ *m_remeshedVertices,
            *m_remeshedQuads });
        checkRenderQueue();
    } else {
        if (m_headlessMode) {
            std::cerr << "Error: Remeshing produced no result" << std::endl;
            emit headlessFinished(0, 0, 0, m_headlessTimer.elapsed() / 1000.0);
            return;
        }
        m_renderQueue.push({ std::vector<AutoRemesher::Vector3>(),
            std::vector<std::vector<size_t>>() });
        checkRenderQueue();
    }

    if (m_quadMeshResultIsDirty)
        generateQuadMesh();

    updateButtonStates();
    updateTitle();
}
