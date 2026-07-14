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
#include "util.h"
#include "version.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

LogBrowser* g_logBrowser = nullptr;
QTextBrowser* g_acknowlegementsWidget = nullptr;
QTextBrowser* g_supportersWidget = nullptr;
QTextBrowser* g_contributorsWidget = nullptr;
AboutWidget* g_aboutWidget = nullptr;
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

    QAction* seeContributorsAction = new QAction(tr("Contributors"), this);
    connect(seeContributorsAction, &QAction::triggered, this, &MainWindow::showContributors);
    helpMenu->addAction(seeContributorsAction);

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
    // PREVIEW BUTTONS — [source] [isotropic] [param] [remesh]
    // (placed in the right sidebar layout below)
    // ============================================================

    auto makePreviewButton = [&](const QString& label) -> QPushButton* {
        QPushButton* btn = new QPushButton(label, containerWidget);
        btn->setFixedHeight(22);
        btn->setStyleSheet(
            "QPushButton {"
            "  color: #191919;"
            "  background-color: #aaebc4;"
            "  border: 1px solid #2a2a2a;"
            "  border-radius: 3px;"
            "  padding: 0 10px;"
            "  font-size: 11px;"
            "}"
            "QPushButton:hover {"
            "  background-color: #8ad4a8;"
            "  border-color: #4a4a4a;"
            "}"
            "QPushButton:pressed {"
            "  background-color: #6dbe8e;"
            "}"
            "QPushButton:checked {"
            "  background-color: #aaebc4;"
            "  color: #191919;"
            "  border: 1px solid #2a2a2a;"
            "}"
            "QPushButton:!checked {"
            "  background-color: rgba(42, 42, 42, 180);"
            "  color: #aaaaaa;"
            "  border: 1px solid #3a3a3a;"
            "}"
            "QPushButton:!checked:hover {"
            "  background-color: rgba(60, 60, 60, 200);"
            "  color: #cccccc;"
            "}");
        btn->setCheckable(true);
        btn->show();
        return btn;
    };

    m_previewSourceButton = makePreviewButton(tr("Source"));
    m_previewIsotropicButton = makePreviewButton(tr("Isotropic"));
    m_previewParamButton = makePreviewButton(tr("Param"));
    m_previewRemeshButton = makePreviewButton(tr("Remeshed"));

    m_previewSourceButton->setEnabled(false);
    m_previewIsotropicButton->setEnabled(false);
    m_previewParamButton->setEnabled(false);
    m_previewRemeshButton->setEnabled(false);

    connect(m_previewSourceButton, &QPushButton::clicked, this, &MainWindow::switchToSourceView);
    connect(m_previewIsotropicButton, &QPushButton::clicked, this, &MainWindow::switchToIsotropicView);
    connect(m_previewParamButton, &QPushButton::clicked, this, &MainWindow::switchToParamView);
    connect(m_previewRemeshButton, &QPushButton::clicked, this, &MainWindow::switchToRemeshView);

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

    controlsLayout->addStretch();

    controlsLayout->addWidget(m_quadCountLabel);
    controlsLayout->addWidget(m_nonQuadCountLabel);
    controlsLayout->addWidget(m_vertexCountLabel);

    // Preview overlay buttons in a row at bottom of sidebar
    controlsLayout->addSpacing(8);
    QHBoxLayout* previewButtonsLayout = new QHBoxLayout;
    previewButtonsLayout->setSpacing(4);
    previewButtonsLayout->addWidget(m_previewSourceButton);
    previewButtonsLayout->addWidget(m_previewIsotropicButton);
    previewButtonsLayout->addWidget(m_previewParamButton);
    previewButtonsLayout->addWidget(m_previewRemeshButton);
    controlsLayout->addLayout(previewButtonsLayout);

    QWidget* controlsPanel = new QWidget;
    controlsPanel->setLayout(controlsLayout);
    controlsPanel->setFixedWidth(300);
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

    // Update preview button availability
    m_previewSourceButton->setEnabled(m_sourceRenderMesh != nullptr);
    m_previewIsotropicButton->setEnabled(m_isotropicRenderMesh != nullptr);
    m_previewParamButton->setEnabled(m_paramRenderMesh != nullptr);
    m_previewRemeshButton->setEnabled(m_remeshRenderMesh != nullptr);
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

    // Reset preview state for new model
    delete m_sourceRenderMesh;
    m_sourceRenderMesh = nullptr;
    delete m_isotropicRenderMesh;
    m_isotropicRenderMesh = nullptr;
    delete m_paramRenderMesh;
    m_paramRenderMesh = nullptr;
    delete m_remeshRenderMesh;
    m_remeshRenderMesh = nullptr;
    m_isotropicVertices.clear();
    m_isotropicTriangles.clear();
    m_isotropicTriangleUvs.clear();
    m_isotropicSingularVertices.clear();
    delete m_remeshedVertices;
    m_remeshedVertices = nullptr;
    delete m_remeshedQuads;
    m_remeshedQuads = nullptr;
    m_previewMode = PreviewSource;
    m_previewSourceButton->setChecked(false);
    m_previewIsotropicButton->setChecked(false);
    m_previewParamButton->setChecked(false);
    m_previewRemeshButton->setChecked(false);

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

void MainWindow::switchToSourceView()
{
    m_previewMode = PreviewSource;
    m_previewSourceButton->setChecked(true);
    m_previewIsotropicButton->setChecked(false);
    m_previewParamButton->setChecked(false);
    m_previewRemeshButton->setChecked(false);
    if (m_sourceRenderMesh)
        m_modelRenderWidget->updateMesh(new ModelShaderMesh(*m_sourceRenderMesh));
}

void MainWindow::switchToIsotropicView()
{
    m_previewMode = PreviewIsotropic;
    m_previewSourceButton->setChecked(false);
    m_previewIsotropicButton->setChecked(true);
    m_previewParamButton->setChecked(false);
    m_previewRemeshButton->setChecked(false);
    if (m_isotropicRenderMesh)
        m_modelRenderWidget->updateMesh(new ModelShaderMesh(*m_isotropicRenderMesh));
}

void MainWindow::switchToParamView()
{
    m_previewMode = PreviewParam;
    m_previewSourceButton->setChecked(false);
    m_previewIsotropicButton->setChecked(false);
    m_previewParamButton->setChecked(true);
    m_previewRemeshButton->setChecked(false);
    if (m_paramRenderMesh)
        m_modelRenderWidget->updateMesh(new ModelShaderMesh(*m_paramRenderMesh));
}

void MainWindow::switchToRemeshView()
{
    m_previewMode = PreviewRemesh;
    m_previewSourceButton->setChecked(false);
    m_previewIsotropicButton->setChecked(false);
    m_previewParamButton->setChecked(false);
    m_previewRemeshButton->setChecked(true);
    if (m_remeshRenderMesh)
        m_modelRenderWidget->updateMesh(new ModelShaderMesh(*m_remeshRenderMesh));
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
    delete m_sourceRenderMesh;
    delete m_isotropicRenderMesh;
    delete m_paramRenderMesh;
    delete m_remeshRenderMesh;
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

void MainWindow::showContributors()
{
    if (!g_contributorsWidget) {
        g_contributorsWidget = new QTextBrowser;
        g_contributorsWidget->setWindowTitle(unifiedWindowTitle(tr("Contributors")));
        g_contributorsWidget->setMinimumSize(QSize(320, 280));
        QFile authors(":/AUTHORS");
        authors.open(QFile::ReadOnly | QFile::Text);
        QFile contributors(":/CONTRIBUTORS");
        contributors.open(QFile::ReadOnly | QFile::Text);
        g_contributorsWidget->setHtml("<h1>AUTHORS</h1><pre>" + authors.readAll() + "</pre><h1>CONTRIBUTORS</h1><pre>" + contributors.readAll() + "</pre>");
    }
    g_contributorsWidget->show();
    g_contributorsWidget->activateWindow();
    g_contributorsWidget->raise();
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

    // Save a copy of the mesh being displayed, then pass ownership to the binder
    if (nullptr == m_remeshedVertices && nullptr == m_remeshedQuads) {
        // This is the source mesh being displayed
        delete m_sourceRenderMesh;
        m_sourceRenderMesh = new ModelShaderMesh(*renderMesh);
        m_previewMode = PreviewSource;
        m_previewSourceButton->setChecked(true);
        m_previewIsotropicButton->setChecked(false);
        m_previewParamButton->setChecked(false);
        m_previewRemeshButton->setChecked(false);
    } else {
        // This is the remesh result being displayed — save a copy
        delete m_remeshRenderMesh;
        m_remeshRenderMesh = new ModelShaderMesh(*renderMesh);

        // Now generate isotropic (voxel) and param preview meshes
        generatePreviewMeshes();
    }

    m_modelRenderWidget->updateMesh(renderMesh);

    updateButtonStates();

    checkRenderQueue();
}

static ModelShaderMesh* buildRenderMeshFromTriangles(
    const std::vector<AutoRemesher::Vector3>& vertices,
    const std::vector<std::vector<size_t>>& triangles)
{
    if (vertices.empty() || triangles.empty())
        return new ModelShaderMesh;

    // Normalize vertices to unit cube
    double minX = std::numeric_limits<double>::max();
    double maxX = std::numeric_limits<double>::lowest();
    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();
    double minZ = std::numeric_limits<double>::max();
    double maxZ = std::numeric_limits<double>::lowest();
    for (const auto& v : vertices) {
        if (v.x() < minX)
            minX = v.x();
        if (v.x() > maxX)
            maxX = v.x();
        if (v.y() < minY)
            minY = v.y();
        if (v.y() > maxY)
            maxY = v.y();
        if (v.z() < minZ)
            minZ = v.z();
        if (v.z() > maxZ)
            maxZ = v.z();
    }
    AutoRemesher::Vector3 origin = {
        (maxX + minX) * 0.5, (maxY + minY) * 0.5, (maxZ + minZ) * 0.5
    };
    double maxLength = std::max({ maxX - minX, maxY - minY, maxZ - minZ }) * 0.5;
    if (maxLength < 1e-10)
        maxLength = 1.0;

    std::vector<AutoRemesher::Vector3> normalizedVerts(vertices.size());
    for (size_t i = 0; i < vertices.size(); ++i)
        normalizedVerts[i] = (vertices[i] - origin) / maxLength;

    // Compute per-vertex normals
    std::vector<AutoRemesher::Vector3> normals(vertices.size());
    for (const auto& tri : triangles) {
        AutoRemesher::Vector3 n = AutoRemesher::Vector3::normal(
            normalizedVerts[tri[0]], normalizedVerts[tri[1]], normalizedVerts[tri[2]]);
        normals[tri[0]] += n;
        normals[tri[1]] += n;
        normals[tri[2]] += n;
    }
    for (auto& n : normals)
        n.normalize();

    // Build vertex array (3 verts per triangle)
    int vertexCount = (int)triangles.size() * 3;
    int edgeVertexCount = (int)triangles.size() * 6; // 2 per edge, 3 edges per tri
    ModelShaderVertex* vertData = new ModelShaderVertex[vertexCount];
    ModelShaderVertex* edgeData = new ModelShaderVertex[edgeVertexCount];
    memset(vertData, 0, sizeof(ModelShaderVertex) * vertexCount);
    memset(edgeData, 0, sizeof(ModelShaderVertex) * edgeVertexCount);

    int vi = 0;
    int ei = 0;
    for (const auto& tri : triangles) {
        for (int j = 0; j < 3; ++j) {
            // Edge vertex (2 per edge segment)
            for (int e = 0; e < 2; ++e) {
                auto& ev = edgeData[ei++];
                int idx = (int)tri[(j + e) % 3];
                ev.posX = (float)normalizedVerts[idx].x();
                ev.posY = (float)normalizedVerts[idx].y();
                ev.posZ = (float)normalizedVerts[idx].z();
                ev.normX = (float)normals[idx].x();
                ev.normY = (float)normals[idx].y();
                ev.normZ = (float)normals[idx].z();
                ev.colorR = 0.0f;
                ev.colorG = 0.0f;
                ev.colorB = 0.0f;
                ev.roughness = 1.0f;
                ev.alpha = 1.0f;
            }

            // Triangle vertex
            auto& tv = vertData[vi++];
            int idx = (int)tri[j];
            tv.posX = (float)normalizedVerts[idx].x();
            tv.posY = (float)normalizedVerts[idx].y();
            tv.posZ = (float)normalizedVerts[idx].z();
            tv.normX = (float)normals[idx].x();
            tv.normY = (float)normals[idx].y();
            tv.normZ = (float)normals[idx].z();
            tv.colorR = 1.0f;
            tv.colorG = 0.996f;
            tv.colorB = 0.890f;
            tv.roughness = 1.0f;
            tv.alpha = 1.0f;
        }
    }

    std::vector<AutoRemesher::Vector3>* vertsCopy = new std::vector<AutoRemesher::Vector3>(normalizedVerts);
    std::vector<std::vector<size_t>>* facesCopy = new std::vector<std::vector<size_t>>(triangles);
    return new ModelShaderMesh(vertData, vertexCount, edgeData, ei, vertsCopy, facesCopy);
}

// Forward declarations for static mesh builders
static ModelShaderMesh* buildRenderMeshFromTriangles(
    const std::vector<AutoRemesher::Vector3>& vertices,
    const std::vector<std::vector<size_t>>& triangles);
static ModelShaderMesh* buildUvRenderMesh(
    const std::vector<AutoRemesher::Vector3>& vertices,
    const std::vector<std::vector<size_t>>& triangles,
    const std::vector<std::vector<AutoRemesher::Vector2>>& triangleUvs,
    const std::vector<AutoRemesher::Vector3>& singularVertices = {});

void MainWindow::generatePreviewMeshes()
{
    // Generate isotropic (voxel) preview mesh
    delete m_isotropicRenderMesh;
    m_isotropicRenderMesh = buildRenderMeshFromTriangles(
        m_isotropicVertices, m_isotropicTriangles);

    // Generate param (UV) preview mesh with texture
    delete m_paramRenderMesh;
    if (!m_isotropicVertices.empty() && !m_isotropicTriangles.empty()
        && !m_isotropicTriangleUvs.empty()) {
        // Build a render mesh with UV coordinates and texture
        m_paramRenderMesh = buildUvRenderMesh(
            m_isotropicVertices, m_isotropicTriangles,
            m_isotropicTriangleUvs, m_isotropicSingularVertices);
    } else {
        m_paramRenderMesh = new ModelShaderMesh;
    }

    // Enable preview buttons
    m_previewIsotropicButton->setEnabled(true);
    m_previewParamButton->setEnabled(true);
    m_previewRemeshButton->setEnabled(true);

    // Show the remesh result by default (master copy stays in m_remeshRenderMesh)
    m_previewMode = PreviewRemesh;
    m_previewSourceButton->setChecked(false);
    m_previewIsotropicButton->setChecked(false);
    m_previewParamButton->setChecked(false);
    m_previewRemeshButton->setChecked(true);
    m_modelRenderWidget->updateMesh(
        m_remeshRenderMesh ? new ModelShaderMesh(*m_remeshRenderMesh) : new ModelShaderMesh);
}

static ModelShaderMesh* buildUvRenderMesh(
    const std::vector<AutoRemesher::Vector3>& vertices,
    const std::vector<std::vector<size_t>>& triangles,
    const std::vector<std::vector<AutoRemesher::Vector2>>& triangleUvs,
    const std::vector<AutoRemesher::Vector3>& singularVertices)
{
    if (vertices.empty() || triangles.empty() || triangleUvs.empty())
        return new ModelShaderMesh;

    // Normalize vertices
    double minX = std::numeric_limits<double>::max();
    double maxX = std::numeric_limits<double>::lowest();
    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();
    double minZ = std::numeric_limits<double>::max();
    double maxZ = std::numeric_limits<double>::lowest();
    for (const auto& v : vertices) {
        if (v.x() < minX)
            minX = v.x();
        if (v.x() > maxX)
            maxX = v.x();
        if (v.y() < minY)
            minY = v.y();
        if (v.y() > maxY)
            maxY = v.y();
        if (v.z() < minZ)
            minZ = v.z();
        if (v.z() > maxZ)
            maxZ = v.z();
    }
    AutoRemesher::Vector3 origin = {
        (maxX + minX) * 0.5, (maxY + minY) * 0.5, (maxZ + minZ) * 0.5
    };
    double maxLength = std::max({ maxX - minX, maxY - minY, maxZ - minZ }) * 0.5;
    if (maxLength < 1e-10)
        maxLength = 1.0;

    std::vector<AutoRemesher::Vector3> normalizedVerts(vertices.size());
    for (size_t i = 0; i < vertices.size(); ++i)
        normalizedVerts[i] = (vertices[i] - origin) / maxLength;

    // Compute per-vertex normals
    std::vector<AutoRemesher::Vector3> normals(vertices.size());
    for (const auto& tri : triangles) {
        AutoRemesher::Vector3 n = AutoRemesher::Vector3::normal(
            normalizedVerts[tri[0]], normalizedVerts[tri[1]], normalizedVerts[tri[2]]);
        normals[tri[0]] += n;
        normals[tri[1]] += n;
        normals[tri[2]] += n;
    }
    for (auto& n : normals)
        n.normalize();

    // Map UVs from [-1, 1] to [0, 1] (geogram QuadCover convention)
    auto normalizeUv = [](double x) {
        return 0.5 + x * 0.5;
    };

    // Build vertex array with UV coordinates
    int vertexCount = (int)triangles.size() * 3;
    int edgeVertexCount = (int)triangles.size() * 6;
    ModelShaderVertex* vertData = new ModelShaderVertex[vertexCount];
    ModelShaderVertex* edgeData = new ModelShaderVertex[edgeVertexCount];
    memset(vertData, 0, sizeof(ModelShaderVertex) * vertexCount);
    memset(edgeData, 0, sizeof(ModelShaderVertex) * edgeVertexCount);

    int vi = 0;
    int ei = 0;
    for (size_t i = 0; i < triangles.size(); ++i) {
        const auto& tri = triangles[i];
        const auto& uvTri = (i < triangleUvs.size()) ? triangleUvs[i] : triangleUvs.back();
        for (int j = 0; j < 3; ++j) {
            // Edge vertex
            for (int e = 0; e < 2; ++e) {
                auto& ev = edgeData[ei++];
                int idx = (int)tri[(j + e) % 3];
                ev.posX = (float)normalizedVerts[idx].x();
                ev.posY = (float)normalizedVerts[idx].y();
                ev.posZ = (float)normalizedVerts[idx].z();
                ev.normX = (float)normals[idx].x();
                ev.normY = (float)normals[idx].y();
                ev.normZ = (float)normals[idx].z();
                ev.colorR = 0.0f;
                ev.colorG = 0.0f;
                ev.colorB = 0.0f;
                ev.roughness = 1.0f;
                ev.alpha = 1.0f;
            }

            // Triangle vertex with UV
            auto& tv = vertData[vi++];
            int idx = (int)tri[j];
            tv.posX = (float)normalizedVerts[idx].x();
            tv.posY = (float)normalizedVerts[idx].y();
            tv.posZ = (float)normalizedVerts[idx].z();
            tv.normX = (float)normals[idx].x();
            tv.normY = (float)normals[idx].y();
            tv.normZ = (float)normals[idx].z();
            tv.colorR = 1.0f;
            tv.colorG = 0.996f;
            tv.colorB = 0.890f;
            tv.texU = (float)normalizeUv(uvTri[j].x());
            tv.texV = (float)normalizeUv(uvTri[j].y());
            tv.roughness = 1.0f;
            tv.alpha = 1.0f;
        }
    }

    std::vector<AutoRemesher::Vector3>* vertsCopy = new std::vector<AutoRemesher::Vector3>(normalizedVerts);
    std::vector<std::vector<size_t>>* facesCopy = new std::vector<std::vector<size_t>>(triangles);
    ModelShaderMesh* mesh = new ModelShaderMesh(vertData, vertexCount, edgeData, ei, vertsCopy, facesCopy);

    // Load the cross UV texture
    QImage* textureImage = new QImage(":/resources/crossuv.png");
    mesh->setTextureImage(textureImage);

    // Add small white sphere markers at singular vertex positions
    if (!singularVertices.empty()) {
        // Generate an icosahedron as a small sphere marker
        // Golden ratio for icosahedron vertices
        const float phi = (1.0f + std::sqrt(5.0f)) * 0.5f;
        // Unit icosahedron vertices
        std::vector<AutoRemesher::Vector3> sphereVerts = {
            { -1, phi, 0 }, { 1, phi, 0 }, { -1, -phi, 0 }, { 1, -phi, 0 },
            { 0, -1, phi }, { 0, 1, phi }, { 0, -1, -phi }, { 0, 1, -phi },
            { phi, 0, -1 }, { phi, 0, 1 }, { -phi, 0, -1 }, { -phi, 0, 1 }
        };
        // Normalize to unit sphere
        for (auto& v : sphereVerts) {
            float len = std::sqrt(v.x() * v.x() + v.y() * v.y() + v.z() * v.z());
            if (len > 1e-10f) {
                v.setX(v.x() / len);
                v.setY(v.y() / len);
                v.setZ(v.z() / len);
            }
        }
        // Icosahedron triangle indices (20 faces)
        int indices[20][3] = {
            { 0, 11, 5 }, { 0, 5, 1 }, { 0, 1, 7 }, { 0, 7, 10 }, { 0, 10, 11 },
            { 1, 5, 9 }, { 5, 11, 4 }, { 11, 10, 2 }, { 10, 7, 6 }, { 7, 1, 8 },
            { 3, 9, 4 }, { 3, 4, 2 }, { 3, 2, 6 }, { 3, 6, 8 }, { 3, 8, 9 },
            { 4, 9, 5 }, { 2, 4, 11 }, { 6, 2, 10 }, { 8, 6, 7 }, { 9, 8, 1 }
        };
        // Subdivide once for smoother spheres (each triangle becomes 4)
        std::vector<AutoRemesher::Vector3> subVerts;
        std::vector<int> subIndices;
        for (int f = 0; f < 20; ++f) {
            int i0 = indices[f][0], i1 = indices[f][1], i2 = indices[f][2];
            auto& v0 = sphereVerts[i0];
            auto& v1 = sphereVerts[i1];
            auto& v2 = sphereVerts[i2];
            auto mid = [](const AutoRemesher::Vector3& a, const AutoRemesher::Vector3& b) {
                AutoRemesher::Vector3 m = { (a.x() + b.x()) * 0.5f, (a.y() + b.y()) * 0.5f, (a.z() + b.z()) * 0.5f };
                float len = std::sqrt(m.x() * m.x() + m.y() * m.y() + m.z() * m.z());
                if (len > 1e-10f) {
                    m.setX(m.x() / len);
                    m.setY(m.y() / len);
                    m.setZ(m.z() / len);
                }
                return m;
            };
            AutoRemesher::Vector3 m01 = mid(v0, v1);
            AutoRemesher::Vector3 m12 = mid(v1, v2);
            AutoRemesher::Vector3 m20 = mid(v2, v0);
            int bi = (int)subVerts.size();
            subVerts.push_back(v0);
            subVerts.push_back(v1);
            subVerts.push_back(v2);
            subVerts.push_back(m01);
            subVerts.push_back(m12);
            subVerts.push_back(m20);
            subIndices.push_back(bi + 0);
            subIndices.push_back(bi + 3);
            subIndices.push_back(bi + 5);
            subIndices.push_back(bi + 3);
            subIndices.push_back(bi + 1);
            subIndices.push_back(bi + 4);
            subIndices.push_back(bi + 5);
            subIndices.push_back(bi + 4);
            subIndices.push_back(bi + 2);
            subIndices.push_back(bi + 3);
            subIndices.push_back(bi + 4);
            subIndices.push_back(bi + 5);
        }

        // Compute the sphere radius as a fraction of the mesh bounding box
        double sphereRadius = 0.0025;
        if (sphereRadius < 0.001)
            sphereRadius = 0.001;

        // Build tool vertices for all singular vertex spheres
        int sphereTriCount = (int)subIndices.size() / 3;
        int totalToolVerts = (int)singularVertices.size() * sphereTriCount * 3;
        ModelShaderVertex* toolVerts = new ModelShaderVertex[totalToolVerts];
        memset(toolVerts, 0, sizeof(ModelShaderVertex) * totalToolVerts);

        int toolVi = 0;
        for (const auto& svPos : singularVertices) {
            // Normalize singular vertex position (same normalization as the mesh)
            AutoRemesher::Vector3 normalizedPos = (svPos - origin) / maxLength;
            for (int t = 0; t < sphereTriCount; ++t) {
                for (int j = 0; j < 3; ++j) {
                    auto& tv = toolVerts[toolVi++];
                    int vi = subIndices[t * 3 + j];
                    tv.posX = (float)(normalizedPos.x() + subVerts[vi].x() * sphereRadius);
                    tv.posY = (float)(normalizedPos.y() + subVerts[vi].y() * sphereRadius);
                    tv.posZ = (float)(normalizedPos.z() + subVerts[vi].z() * sphereRadius);
                    tv.normX = subVerts[vi].x();
                    tv.normY = subVerts[vi].y();
                    tv.normZ = subVerts[vi].z();
                    tv.colorR = 1.0f;
                    tv.colorG = 1.0f;
                    tv.colorB = 1.0f;
                    tv.roughness = 0.3f;
                    tv.alpha = 1.0f;
                }
            }
        }

        mesh->updateTool(toolVerts, totalToolVerts);
    }

    return mesh;
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

    // Capture intermediate isotropic mesh data for preview overlays
    m_isotropicVertices = m_quadMeshGenerator->isotropicVertices();
    m_isotropicTriangles = m_quadMeshGenerator->isotropicTriangles();
    m_isotropicTriangleUvs = m_quadMeshGenerator->isotropicTriangleUvs();
    m_isotropicSingularVertices = m_quadMeshGenerator->isotropicSingularVertices();

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
