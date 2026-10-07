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
#include <QCheckBox>
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
#if defined(Q_OS_WIN32) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QWinTaskbarButton>
#include <QWinTaskbarProgress>
#elif defined(Q_OS_WIN32)
#include <QWindow>
#include <shobjidl.h>
#endif
#include "aboutwidget.h"
#include "floatnumberwidget.h"
#include "graphicscontainerwidget.h"
#include "intnumberwidget.h"
#include "logbrowser.h"
#include "mainwindow.h"
#include "preferences.h"
#include "previewmeshgenerator.h"
#include "quadmeshgenerator.h"
#include "rendermeshgenerator.h"
#include "theme.h"
#include "util.h"
#include "version.h"
#include "meshio.h"
#include <QPointer>
#include <QDialog>
#include <QDialogButtonBox>

namespace {
LogBrowser* getLogBrowser()
{
    static LogBrowser* s_logBrowser = new LogBrowser;
    return s_logBrowser;
}
QPointer<QTextBrowser> s_acknowledgementsWidget;
QPointer<QTextBrowser> s_supportersWidget;
QPointer<QTextBrowser> s_contributorsWidget;
QPointer<AboutWidget> s_aboutWidget;
std::map<MainWindow*, QUuid> s_windows;
}

void outputMessage(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    getLogBrowser()->outputMessage(type, msg, context.file, context.line);
}

size_t MainWindow::total()
{
    return s_windows.size();
}

MainWindow::MainWindow()
{
    static bool s_msgHandlerInstalled = false;
    if (!s_msgHandlerInstalled) {
        qInstallMessageHandler(&outputMessage);
        s_msgHandlerInstalled = true;
    }

    s_windows.insert({ this, QUuid::createUuid() });

#if defined(Q_OS_WIN32) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
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

    QMenu* editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(tr("&Undo"), this, &MainWindow::undo, QKeySequence::Undo);
    editMenu->addAction(tr("&Redo"), this, &MainWindow::redo, QKeySequence::Redo);

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

    QAction* showStatisticsAction = new QAction(tr("Statistics"), this);
    connect(showStatisticsAction, &QAction::triggered, this, &MainWindow::showStatistics);
    helpMenu->addAction(showStatisticsAction);

    QAction* showDebugDialogAction = new QAction(tr("Debug"), this);
    connect(showDebugDialogAction, &QAction::triggered, getLogBrowser(), &LogBrowser::showDialog);
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
    // PREVIEW BUTTONS — [source] [decimated] [isotropic] [param] [remesh]
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
            "  padding: 0 6px;"
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
    m_previewDecimateButton = makePreviewButton(tr("Decimated"));
    m_previewIsotropicButton = makePreviewButton(tr("Isotropic"));
    m_previewParamButton = makePreviewButton(tr("Param"));
    m_previewRemeshButton = makePreviewButton(tr("Remeshed"));

    m_previewSourceButton->setEnabled(false);
    m_previewDecimateButton->setEnabled(false);
    m_previewIsotropicButton->setEnabled(false);
    m_previewParamButton->setEnabled(false);
    m_previewRemeshButton->setEnabled(false);

    connect(m_previewSourceButton, &QPushButton::clicked, this, &MainWindow::switchToSourceView);
    connect(m_previewDecimateButton, &QPushButton::clicked, this, &MainWindow::switchToDecimateView);
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

    m_anisotropyWidget = new FloatNumberWidget(this, false);
    m_anisotropyWidget->setItemName(tr("Anisotropy"));
    m_anisotropyWidget->setRange(0.0, 1.0);
    m_anisotropyWidget->setValue(m_anisotropy);
    m_anisotropyWidget->setToolTip(tr("Curvature-adaptive quad elongation. 0 = square quads, 1 = quads stretched along the flatter direction (long on tubes and ridges, square on spheres)."));
    connect(m_anisotropyWidget, &FloatNumberWidget::valueChanged, [=](float value) {
        m_anisotropy = value;
    });

    m_targetQuadCountWidget = new IntNumberWidget(this, false);
    m_targetQuadCountWidget->setItemName(tr("Target Quads"));
    m_targetQuadCountWidget->setRange(1000, 1000000);
    m_targetQuadCountWidget->setValue(m_targetQuadCount);
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

    m_modelTypeSelectBox = new QComboBox(this);
    m_modelTypeSelectBox->addItem(tr("Organic"));
    m_modelTypeSelectBox->addItem(tr("Hard surface"));
    connect(m_modelTypeSelectBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this](int index) {
        m_modelType = 1 == index ? AutoRemesher::ModelType::HardSurface : AutoRemesher::ModelType::Organic;
    });
    m_modelTypeSelectBox->setCurrentIndex(AutoRemesher::ModelType::HardSurface == m_modelType ? 1 : 0);

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
    connect(regenerateButton, &QPushButton::clicked, this, [this]() {
        if (m_inProgress) {
            cancelQuadMesh();
        } else {
            generateQuadMesh();
        }
    });
    m_regenerateButton = regenerateButton;

    // --- Controls panel layout ---
    QVBoxLayout* controlsLayout = new QVBoxLayout;
    controlsLayout->setSpacing(2);
    controlsLayout->setContentsMargins(6, 6, 6, 6);

    controlsLayout->addWidget(m_sharpEdgeDegreesWidget);
    controlsLayout->addWidget(m_smoothNormalDegreesWidget);
    controlsLayout->addWidget(m_adaptivityWidget);
    controlsLayout->addWidget(m_anisotropyWidget);
    controlsLayout->addWidget(m_targetQuadCountWidget);
    controlsLayout->addWidget(m_targetScalingWidget);
    controlsLayout->addWidget(m_modelTypeSelectBox);

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

    // The 2px bar alone cannot say which of the twenty odd steps is running, and
    // the longer ones take seconds, so name the step next to the percentage.
    m_progressStatusLabel = new QLabel(this);
    m_progressStatusLabel->setStyleSheet("color: #9a9a9a; font-size: 11px; padding: 2px 0;");
    m_progressStatusLabel->setWordWrap(true);
    m_progressStatusLabel->hide();

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

    controlsLayout->addWidget(m_progressStatusLabel);
    controlsLayout->addWidget(m_quadCountLabel);
    controlsLayout->addWidget(m_nonQuadCountLabel);
    controlsLayout->addWidget(m_vertexCountLabel);

    // Preview overlay buttons in a row at bottom of sidebar
    controlsLayout->addSpacing(8);
    QHBoxLayout* previewButtonsLayout = new QHBoxLayout;
    previewButtonsLayout->setSpacing(4);
    previewButtonsLayout->addWidget(m_previewSourceButton);
    previewButtonsLayout->addWidget(m_previewDecimateButton);
    previewButtonsLayout->addWidget(m_previewIsotropicButton);
    previewButtonsLayout->addWidget(m_previewParamButton);
    previewButtonsLayout->addWidget(m_previewRemeshButton);
    controlsLayout->addLayout(previewButtonsLayout);

    QWidget* controlsPanel = new QWidget;
    controlsPanel->setLayout(controlsLayout);
    controlsPanel->setFixedWidth(400);
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
        m_anisotropyWidget->setEnabled(true);
        if (m_modelTypeSelectBox)
            m_modelTypeSelectBox->setEnabled(true);
        if (nullptr != m_remeshedQuads) {
            m_saveMeshButton->show();
        } else {
            m_saveMeshButton->hide();
        }
        if (!m_originalVertices.empty()) {
            m_regenerateButton->setText(tr("Regenerate"));
            m_regenerateButton->show();
            m_regenerateButton->setEnabled(true);
        } else {
            m_regenerateButton->hide();
        }
        m_progressBar->hide();
        m_progressStatusLabel->hide();
    } else {
        m_loadModelButton->setEnabled(false);
        m_saveMeshButton->hide();
        m_regenerateButton->setText(tr("Cancel"));
        m_regenerateButton->show();
        m_regenerateButton->setEnabled(true);
        m_targetScalingWidget->setDisabled(true);
        m_targetQuadCountWidget->setDisabled(true);
        m_sharpEdgeDegreesWidget->setDisabled(true);
        m_smoothNormalDegreesWidget->setDisabled(true);
        m_adaptivityWidget->setDisabled(true);
        m_anisotropyWidget->setDisabled(true);
        if (m_modelTypeSelectBox)
            m_modelTypeSelectBox->setDisabled(true);
    }

    // Update preview button availability
    m_previewSourceButton->setEnabled(m_sourceRenderMesh != nullptr);
    m_previewDecimateButton->setEnabled(m_decimatedRenderMesh != nullptr);
    m_previewIsotropicButton->setEnabled(m_isotropicRenderMesh != nullptr);
    m_previewParamButton->setEnabled(m_paramRenderMesh != nullptr);
    m_previewRemeshButton->setEnabled(m_remeshRenderMesh != nullptr);
}

bool MainWindow::loadObj(const QString& filename)
{
    std::string err;
    std::vector<AutoRemesher::Vector3> loadedVertices;
    std::vector<std::vector<size_t>> loadedFaces;

    qDebug() << "loadMesh:" << filename;

    bool loadSuccess = MeshIO::loadMesh(filename, loadedVertices, loadedFaces, err);
    if (!err.empty()) {
        qDebug() << "MeshIO notice:" << QString::fromStdString(err);
    }
    if (!loadSuccess) {
        qWarning() << "Failed to load mesh:" << QString::fromStdString(err);
        return false;
    }

    // Reset preview state for new model
    delete m_sourceRenderMesh;
    m_sourceRenderMesh = nullptr;
    delete m_decimatedRenderMesh;
    m_decimatedRenderMesh = nullptr;
    delete m_isotropicRenderMesh;
    m_isotropicRenderMesh = nullptr;
    delete m_paramRenderMesh;
    m_paramRenderMesh = nullptr;
    delete m_remeshRenderMesh;
    m_remeshRenderMesh = nullptr;
    m_decimatedVertices.clear();
    m_decimatedTriangles.clear();
    m_isotropicVertices.clear();
    m_isotropicTriangles.clear();
    m_isotropicTriangleUvs.clear();
    m_isotropicOriginalTriangleUvs.clear();
    m_isotropicExtractedConnectionMoved.clear();
    m_isotropicSingularVertices.clear();
    m_isotropicExtractedConnections.clear();
    delete m_remeshedVertices;
    m_remeshedVertices = nullptr;
    delete m_remeshedQuads;
    m_remeshedQuads = nullptr;
    m_undoStack.clear();
    m_redoStack.clear();
    m_previewMode = PreviewSource;
    m_previewSourceButton->setChecked(false);
    m_previewDecimateButton->setChecked(false);
    m_previewIsotropicButton->setChecked(false);
    m_previewParamButton->setChecked(false);
    m_previewRemeshButton->setChecked(false);

    m_originalVertices = std::move(loadedVertices);
    m_originalTriangles = std::move(loadedFaces);

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
        tr("Supported Mesh Files (*.obj *.stl *.ply);;Wavefront OBJ (*.obj);;Stereolithography (*.stl);;Polygon File Format (*.ply);;All Files (*.*)"));
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
        tr("Wavefront OBJ (*.obj);;Stereolithography (*.stl);;Polygon File Format (*.ply)"));
    if (filename.isEmpty()) {
        return;
    }

    saveMeshToFile(filename);
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
    m_previewDecimateButton->setChecked(false);
    m_previewIsotropicButton->setChecked(false);
    m_previewParamButton->setChecked(false);
    m_previewRemeshButton->setChecked(false);
    if (m_sourceRenderMesh)
        m_modelRenderWidget->updateMesh(new ModelShaderMesh(*m_sourceRenderMesh));
}

void MainWindow::switchToDecimateView()
{
    m_previewMode = PreviewDecimate;
    m_previewSourceButton->setChecked(false);
    m_previewDecimateButton->setChecked(true);
    m_previewIsotropicButton->setChecked(false);
    m_previewParamButton->setChecked(false);
    m_previewRemeshButton->setChecked(false);
    if (m_decimatedRenderMesh)
        m_modelRenderWidget->updateMesh(new ModelShaderMesh(*m_decimatedRenderMesh));
}

void MainWindow::switchToIsotropicView()
{
    m_previewMode = PreviewIsotropic;
    m_previewSourceButton->setChecked(false);
    m_previewDecimateButton->setChecked(false);
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
    m_previewDecimateButton->setChecked(false);
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
    m_previewDecimateButton->setChecked(false);
    m_previewIsotropicButton->setChecked(false);
    m_previewParamButton->setChecked(false);
    m_previewRemeshButton->setChecked(true);
    if (m_remeshRenderMesh)
        m_modelRenderWidget->updateMesh(new ModelShaderMesh(*m_remeshRenderMesh));
}

void MainWindow::updateProgress(float progress)
{
#if defined(Q_OS_WIN32) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    m_taskbarButton->progress()->setValue((int)(progress * 100));
#elif defined(Q_OS_WIN32)
    if (nullptr != m_taskbarList) {
        QWindow* handle = windowHandle();
        if (nullptr != handle) {
            HWND hwnd = reinterpret_cast<HWND>(handle->winId());
            if (progress >= 1.0f) {
                m_taskbarList->SetProgressState(hwnd, TBPF_NOPROGRESS);
            } else {
                m_taskbarList->SetProgressState(hwnd, TBPF_NORMAL);
                m_taskbarList->SetProgressValue(hwnd, static_cast<ULONGLONG>(progress * 1000), 1000);
            }
        }
    }
#endif
}

void MainWindow::updateProgressDetailed(float progress, const QString& status)
{
    const int percent = (int)(progress * 100);
    m_progressBar->setValue(percent);
    m_progressBar->show();
    m_progressStatusLabel->setText(status.isEmpty()
            ? QString("%1%").arg(percent)
            : QString("%1%  %2").arg(percent).arg(status));
    m_progressStatusLabel->show();
}

MainWindow::~MainWindow()
{
    s_windows.erase(this);
    delete m_sourceRenderMesh;
    delete m_decimatedRenderMesh;
    delete m_isotropicRenderMesh;
    delete m_paramRenderMesh;
    delete m_remeshRenderMesh;
#if defined(Q_OS_WIN32) && QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if (nullptr != m_taskbarList) {
        m_taskbarList->Release();
        m_taskbarList = nullptr;
    }
#endif
}

ModelShaderWidget* MainWindow::modelRenderWidget() const
{
    return m_modelRenderWidget;
}

void MainWindow::showSupporters()
{
    if (!s_supportersWidget) {
        s_supportersWidget = new QTextBrowser;
        s_supportersWidget->setWindowTitle(unifiedWindowTitle(tr("Supporters")));
        s_supportersWidget->setMinimumSize(QSize(320, 280));
        QFile supporters(":/SUPPORTERS");
        supporters.open(QFile::ReadOnly | QFile::Text);
        s_supportersWidget->setHtml("<h1>SUPPORTERS</h1><pre>" + supporters.readAll() + "</pre>");
    }
    s_supportersWidget->show();
    s_supportersWidget->activateWindow();
    s_supportersWidget->raise();
}

void MainWindow::showContributors()
{
    if (!s_contributorsWidget) {
        s_contributorsWidget = new QTextBrowser;
        s_contributorsWidget->setWindowTitle(unifiedWindowTitle(tr("Contributors")));
        s_contributorsWidget->setMinimumSize(QSize(320, 280));
        QFile authors(":/AUTHORS");
        authors.open(QFile::ReadOnly | QFile::Text);
        QFile contributors(":/CONTRIBUTORS");
        contributors.open(QFile::ReadOnly | QFile::Text);
        s_contributorsWidget->setHtml("<h1>AUTHORS</h1><pre>" + authors.readAll() + "</pre><h1>CONTRIBUTORS</h1><pre>" + contributors.readAll() + "</pre>");
    }
    s_contributorsWidget->show();
    s_contributorsWidget->activateWindow();
    s_contributorsWidget->raise();
}

void MainWindow::showAcknowlegements()
{
    if (!s_acknowledgementsWidget) {
        s_acknowledgementsWidget = new QTextBrowser;
        s_acknowledgementsWidget->setWindowTitle(unifiedWindowTitle(tr("Acknowlegements")));
        s_acknowledgementsWidget->setMinimumSize(QSize(640, 380));
        QFile file(":/ACKNOWLEDGEMENTS.html");
        file.open(QFile::ReadOnly | QFile::Text);
        QTextStream stream(&file);
        s_acknowledgementsWidget->setHtml(stream.readAll());
    }
    s_acknowledgementsWidget->show();
    s_acknowledgementsWidget->activateWindow();
    s_acknowledgementsWidget->raise();
}

void MainWindow::showStatistics()
{
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Remeshing Statistics"));
    dialog.setMinimumSize(450, 350);
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    QTextBrowser* browser = new QTextBrowser(&dialog);
    if (m_lastPhaseReport.empty()) {
        browser->setPlainText(tr("No statistics available yet. Run remeshing first."));
    } else {
        QString text;
        for (const auto& line : m_lastPhaseReport) {
            text += QString::fromStdString(line) + "\n";
        }
        browser->setPlainText(text);
    }
    layout->addWidget(browser);
    QDialogButtonBox* box = new QDialogButtonBox(QDialogButtonBox::Ok, &dialog);
    connect(box, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    layout->addWidget(box);
    dialog.exec();
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
    if (!s_aboutWidget) {
        s_aboutWidget = new AboutWidget;
    }
    s_aboutWidget->show();
    s_aboutWidget->activateWindow();
    s_aboutWidget->raise();
}

void MainWindow::showEvent(QShowEvent* event)
{
#if defined(Q_OS_WIN32) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    m_taskbarButton->setWindow(windowHandle());
    m_taskbarButton->progress()->setVisible(true);
#elif defined(Q_OS_WIN32)
    if (nullptr == m_taskbarList) {
        if (SUCCEEDED(CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_INPROC_SERVER,
                IID_ITaskbarList3, reinterpret_cast<void**>(&m_taskbarList)))) {
            if (FAILED(m_taskbarList->HrInit())) {
                m_taskbarList->Release();
                m_taskbarList = nullptr;
            }
        }
    }
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
        m_previewDecimateButton->setChecked(false);
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

void MainWindow::generatePreviewMeshes()
{
    if (nullptr != m_previewMeshGenerator)
        return;

    QThread* thread = new QThread;
    m_previewMeshGenerator = new PreviewMeshGenerator(
        m_decimatedVertices, m_decimatedTriangles,
        m_isotropicVertices, m_isotropicTriangles, m_isotropicTriangleUvs,
        m_isotropicOriginalTriangleUvs,
        m_isotropicSingularVertices, m_isotropicExtractedConnections,
        m_isotropicExtractedConnectionMoved);
    m_previewMeshGenerator->moveToThread(thread);
    connect(thread, &QThread::started, m_previewMeshGenerator, &PreviewMeshGenerator::process);
    connect(m_previewMeshGenerator, &PreviewMeshGenerator::finished, this, &MainWindow::previewMeshesReady);
    connect(m_previewMeshGenerator, &PreviewMeshGenerator::finished, thread, &QThread::quit);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
}

void MainWindow::previewMeshesReady()
{
    delete m_decimatedRenderMesh;
    m_decimatedRenderMesh = m_previewMeshGenerator->takeDecimatedMesh();

    delete m_isotropicRenderMesh;
    m_isotropicRenderMesh = m_previewMeshGenerator->takeIsotropicMesh();

    delete m_paramRenderMesh;
    m_paramRenderMesh = m_previewMeshGenerator->takeParamMesh();

    delete m_previewMeshGenerator;
    m_previewMeshGenerator = nullptr;

    // Enable preview buttons
    m_previewDecimateButton->setEnabled(nullptr != m_decimatedRenderMesh);
    m_previewIsotropicButton->setEnabled(true);
    m_previewParamButton->setEnabled(true);
    m_previewRemeshButton->setEnabled(true);

    // Show the remesh result by default (master copy stays in m_remeshRenderMesh)
    m_previewMode = PreviewRemesh;
    m_previewSourceButton->setChecked(false);
    m_previewDecimateButton->setChecked(false);
    m_previewIsotropicButton->setChecked(false);
    m_previewParamButton->setChecked(false);
    m_previewRemeshButton->setChecked(true);
    m_modelRenderWidget->updateMesh(
        m_remeshRenderMesh ? new ModelShaderMesh(*m_remeshRenderMesh) : new ModelShaderMesh);
}

void MainWindow::setHeadlessParams(const QString& inputPath, const QString& outputPath,
    int targetQuads, double edgeScaling,
    double sharpEdgeDegrees, double smoothNormalDegrees,
    double adaptivity,
    double anisotropy,
    AutoRemesher::ModelType modelType)
{
    m_headlessMode = true;
    m_headlessOutputPath = outputPath;
    m_currentFilename = inputPath;
    m_targetQuadCount = targetQuads;
    m_targetScaling = static_cast<float>(edgeScaling);
    m_sharpEdgeDegrees = static_cast<float>(sharpEdgeDegrees);
    m_smoothNormalDegrees = static_cast<float>(smoothNormalDegrees);
    m_adaptivity = static_cast<float>(adaptivity);
    m_anisotropy = static_cast<float>(anisotropy);
    m_modelType = modelType;
}

void MainWindow::saveMeshToFile(const QString& filename)
{
    if (nullptr == m_remeshedVertices || nullptr == m_remeshedQuads)
        return;

    std::string err;
    if (!MeshIO::saveMesh(filename, *m_remeshedVertices, *m_remeshedQuads, err)) {
        qWarning() << "Failed to save mesh:" << QString::fromStdString(err);
    }
}

void MainWindow::cancelQuadMesh()
{
    if (m_quadMeshGenerator) {
        m_quadMeshGenerator->cancel();
        m_progressStatusLabel->setText(tr("Cancelling..."));
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
    parameters.anisotropy = m_anisotropy;
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
    m_progressStatusLabel->setText(tr("0%  Initializing"));
    m_progressStatusLabel->show();

    QThread* thread = new QThread;

    QuadMeshGenerator::Parameters parameters;

    parameters.targetTriangleCount = m_targetQuadCount * 2;
    parameters.scaling = m_targetScaling;
    parameters.modelType = m_modelType;
    parameters.adaptivity = m_adaptivity;
    parameters.anisotropy = m_anisotropy;
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

    m_lastPhaseReport = m_quadMeshGenerator->phaseReport();
    for (const auto& line : m_lastPhaseReport) {
        qDebug() << QString::fromStdString(line);
    }

    bool wasCancelled = m_quadMeshGenerator->isCancelled();

    m_saved = false;
    m_inProgress = false;

    // Capture intermediate isotropic mesh data for preview overlays
    m_decimatedVertices = m_quadMeshGenerator->decimatedVertices();
    m_decimatedTriangles = m_quadMeshGenerator->decimatedTriangles();
    m_isotropicVertices = m_quadMeshGenerator->isotropicVertices();
    m_isotropicTriangles = m_quadMeshGenerator->isotropicTriangles();
    m_isotropicTriangleUvs = m_quadMeshGenerator->isotropicTriangleUvs();
    m_isotropicOriginalTriangleUvs = m_quadMeshGenerator->isotropicOriginalTriangleUvs();
    m_isotropicExtractedConnectionMoved = m_quadMeshGenerator->isotropicExtractedConnectionMoved();
    m_isotropicSingularVertices = m_quadMeshGenerator->isotropicSingularVertices();
    m_isotropicExtractedConnections = m_quadMeshGenerator->isotropicExtractedConnections();

    delete m_quadMeshGenerator;
    m_quadMeshGenerator = nullptr;

    if (wasCancelled) {
        m_progressStatusLabel->setText(tr("Cancelled"));
        m_progressStatusLabel->show();
        updateButtonStates();
        updateTitle();
        return;
    }

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

        saveHistoryState();

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

void MainWindow::saveHistoryState()
{
    if (!m_remeshedVertices || !m_remeshedQuads)
        return;

    HistoryItem item;
    item.targetQuads = m_targetQuadCount;
    item.scaling = m_targetScaling;
    item.sharpEdge = m_sharpEdgeDegrees;
    item.smoothNormal = m_smoothNormalDegrees;
    item.adaptivity = m_adaptivity;
    item.anisotropy = m_anisotropy;
    item.modelType = m_modelType;
    item.vertices = *m_remeshedVertices;
    item.quads = *m_remeshedQuads;

    m_undoStack.push_back(std::move(item));
    m_redoStack.clear();

    if (m_undoStack.size() > 20) {
        m_undoStack.erase(m_undoStack.begin());
    }
}

void MainWindow::undo()
{
    if (m_undoStack.size() <= 1)
        return;

    m_redoStack.push_back(std::move(m_undoStack.back()));
    m_undoStack.pop_back();

    const auto& prev = m_undoStack.back();
    m_targetQuadCount = prev.targetQuads;
    m_targetScaling = prev.scaling;
    m_sharpEdgeDegrees = prev.sharpEdge;
    m_smoothNormalDegrees = prev.smoothNormal;
    m_adaptivity = prev.adaptivity;
    m_anisotropy = prev.anisotropy;
    m_modelType = prev.modelType;

    m_targetQuadCountWidget->setValue(m_targetQuadCount);
    m_targetScalingWidget->setValue(m_targetScaling);
    m_sharpEdgeDegreesWidget->setValue(m_sharpEdgeDegrees);
    m_smoothNormalDegreesWidget->setValue(m_smoothNormalDegrees);
    m_adaptivityWidget->setValue(m_adaptivity);
    m_anisotropyWidget->setValue(m_anisotropy);
    if (m_modelTypeSelectBox) {
        m_modelTypeSelectBox->setCurrentIndex(m_modelType == AutoRemesher::ModelType::HardSurface ? 1 : 0);
    }

    if (!m_remeshedVertices)
        m_remeshedVertices = new std::vector<AutoRemesher::Vector3>;
    if (!m_remeshedQuads)
        m_remeshedQuads = new std::vector<std::vector<size_t>>;

    *m_remeshedVertices = prev.vertices;
    *m_remeshedQuads = prev.quads;

    m_renderQueue.push({ *m_remeshedVertices, *m_remeshedQuads });
    checkRenderQueue();
    updateButtonStates();
}

void MainWindow::redo()
{
    if (m_redoStack.empty())
        return;

    auto next = std::move(m_redoStack.back());
    m_redoStack.pop_back();

    m_targetQuadCount = next.targetQuads;
    m_targetScaling = next.scaling;
    m_sharpEdgeDegrees = next.sharpEdge;
    m_smoothNormalDegrees = next.smoothNormal;
    m_adaptivity = next.adaptivity;
    m_anisotropy = next.anisotropy;
    m_modelType = next.modelType;

    m_targetQuadCountWidget->setValue(m_targetQuadCount);
    m_targetScalingWidget->setValue(m_targetScaling);
    m_sharpEdgeDegreesWidget->setValue(m_sharpEdgeDegrees);
    m_smoothNormalDegreesWidget->setValue(m_smoothNormalDegrees);
    m_adaptivityWidget->setValue(m_adaptivity);
    m_anisotropyWidget->setValue(m_anisotropy);
    if (m_modelTypeSelectBox) {
        m_modelTypeSelectBox->setCurrentIndex(m_modelType == AutoRemesher::ModelType::HardSurface ? 1 : 0);
    }

    if (!m_remeshedVertices)
        m_remeshedVertices = new std::vector<AutoRemesher::Vector3>;
    if (!m_remeshedQuads)
        m_remeshedQuads = new std::vector<std::vector<size_t>>;

    *m_remeshedVertices = next.vertices;
    *m_remeshedQuads = next.quads;

    m_undoStack.push_back(std::move(next));

    m_renderQueue.push({ *m_remeshedVertices, *m_remeshedQuads });
    checkRenderQueue();
    updateButtonStates();
}
