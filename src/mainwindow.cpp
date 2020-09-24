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
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTextBrowser>
#include <QAction>
#include <QMenuBar>
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>
#include <QPushButton>
#include <QDockWidget>
#include <QLabel>
#include <QImage>
#include <QFileDialog>
#include <QApplication>
#include <QThread>
#include <QDebug>
#include <QTextStream>
#include <QFile>
#include <QComboBox>
#include <QUuid>
#ifdef Q_OS_WIN32
#include <QWinTaskbarButton>
#include <QWinTaskbarProgress>
#endif
#include "mainwindow.h"
#include "graphicscontainerwidget.h"
#include "aboutwidget.h"
#include "updatescheckwidget.h"
#include "util.h"
#include "version.h"
#include "preferences.h"
#include "theme.h"
#include "rendermeshgenerator.h"
#include "quadmeshgenerator.h"
#include "spinnableawesomebutton.h"
#include "logbrowser.h"
#include "floatnumberwidget.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

LogBrowser *g_logBrowser = nullptr;
QTextBrowser *g_acknowlegementsWidget = nullptr;
QTextBrowser *g_supportersWidget = nullptr;
AboutWidget *g_aboutWidget = nullptr;
UpdatesCheckWidget *g_updatesCheckWidget = nullptr;
std::map<MainWindow *, QUuid> g_windows;

void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
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

    g_windows.insert({this, QUuid::createUuid()});
    
#ifdef Q_OS_WIN32
    m_taskbarButton = new QWinTaskbarButton(this);
#endif
    
    GraphicsWidget *graphicsWidget = new GraphicsWidget;
    
    GraphicsContainerWidget *containerWidget = new GraphicsContainerWidget;
    containerWidget->setGraphicsWidget(graphicsWidget);
    QGridLayout *containerLayout = new QGridLayout;
    containerLayout->setSpacing(0);
    containerLayout->setContentsMargins(1, 0, 0, 0);
    containerLayout->addWidget(graphicsWidget);
    containerWidget->setLayout(containerLayout);
    containerWidget->setMinimumSize(400, 400);
    
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    
    QAction *gotoHomepageAction = new QAction(tr("Homepage"), this);
    connect(gotoHomepageAction, &QAction::triggered, this, &MainWindow::gotoHomepage);
    helpMenu->addAction(gotoHomepageAction);

    QAction *viewSourceAction = new QAction(tr("Source Code"), this);
    connect(viewSourceAction, &QAction::triggered, this, &MainWindow::viewSource);
    helpMenu->addAction(viewSourceAction);
    
    QAction *checkForUpdatesAction = new QAction(tr("Check for Updates..."), this);
    connect(checkForUpdatesAction, &QAction::triggered, this, &MainWindow::checkForUpdates);
    helpMenu->addAction(checkForUpdatesAction);

    helpMenu->addSeparator();

    QAction *aboutAction = new QAction(tr("About"), this);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAbout);
    helpMenu->addAction(aboutAction);

    QAction *reportIssuesAction = new QAction(tr("Report Issues"), this);
    connect(reportIssuesAction, &QAction::triggered, this, &MainWindow::reportIssues);
    helpMenu->addAction(reportIssuesAction);
    
    QAction *showDebugDialogAction = new QAction(tr("Debug"), this);
    connect(showDebugDialogAction, &QAction::triggered, g_logBrowser, &LogBrowser::showDialog);
    helpMenu->addAction(showDebugDialogAction);
    
    helpMenu->addSeparator();
    
    QAction *seeSupportersAction = new QAction(tr("Supporters"), this);
    connect(seeSupportersAction, &QAction::triggered, this, &MainWindow::showSupporters);
    helpMenu->addAction(seeSupportersAction);
    
    QAction *showAcknowlegementsAction = new QAction(tr("Acknowlegements"), this);
    connect(showAcknowlegementsAction, &QAction::triggered, this, &MainWindow::showAcknowlegements);
    helpMenu->addAction(showAcknowlegementsAction);
    
    m_modelRenderWidget = new PbrShaderWidget(containerWidget);
    m_modelRenderWidget->setMoveAndZoomByWindow(false);
    m_modelRenderWidget->move(0, 0);
    m_modelRenderWidget->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_modelRenderWidget->toggleWireframe();
    m_modelRenderWidget->enableEnvironmentLight();
    m_modelRenderWidget->disableCullFace();
    m_modelRenderWidget->setEyePosition(QVector3D(0.0, 0.0, -4.0));
    
    connect(containerWidget, &GraphicsContainerWidget::containerSizeChanged,
        m_modelRenderWidget, &PbrShaderWidget::canvasResized);
    
    graphicsWidget->setModelWidget(m_modelRenderWidget);
    containerWidget->setModelWidget(m_modelRenderWidget);
    
    QHBoxLayout *toolLayout = new QHBoxLayout;
    
    m_targetTriangleCountWidget = new FloatNumberWidget;
    m_targetTriangleCountWidget->setItemName(tr("Density"));
    m_targetTriangleCountWidget->setRange(0.0, 1.0);
    m_targetTriangleCountWidget->setValue(m_targetDensity);
    connect(m_targetTriangleCountWidget, &FloatNumberWidget::valueChanged, [=](float value) {
        m_targetDensity = value;
    });
    
    /*
    m_targetScalingWidget = new FloatNumberWidget;
    m_targetScalingWidget->setItemName(tr("Edge Scaling"));
    m_targetScalingWidget->setRange(0.5, 5.0);
    m_targetScalingWidget->setValue(m_targetScaling);
    connect(m_targetScalingWidget, &FloatNumberWidget::valueChanged, [=](float value) {
        m_targetScaling = value;
    });
    */
    
    //m_modelTypeSelectBox = new QComboBox;
    //m_modelTypeSelectBox->addItem(tr("Organic"));
    //m_modelTypeSelectBox->addItem(tr("Hard surface"));
    //connect(m_modelTypeSelectBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [&](int index) {
    //    m_modelType = 1 == index ? AutoRemesher::ModelType::HardSurface : AutoRemesher::ModelType::Organic;
    //});
    
    //m_modelTypeSelectBox->setCurrentIndex(AutoRemesher::ModelType::HardSurface == m_modelType ? 1 : 0);
    
    QLabel *edgeScalingLabel = new QLabel(tr("Edge scaling:"));
    
    m_edgeScalingSelectBox = new QComboBox;
    m_edgeScalingSelectBox->addItem(tr("1.0"));
    m_edgeScalingSelectBox->addItem(tr("2.0"));
    m_edgeScalingSelectBox->addItem(tr("3.0"));
    m_edgeScalingSelectBox->addItem(tr("4.0"));
    m_edgeScalingSelectBox->setCurrentIndex((int)m_targetScaling - 1);
    connect(m_edgeScalingSelectBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [&](int index) {
        m_targetScaling = (float)(index + 1);
    });
    
    SpinnableAwesomeButton *loadModelButton = new SpinnableAwesomeButton();
    loadModelButton->setAwesomeIcon(QChar(fa::folderopeno));
    connect(loadModelButton->button(), &QPushButton::clicked, this, &MainWindow::loadModel);
    
    m_loadModelButton = loadModelButton;
    
    SpinnableAwesomeButton *saveMeshButton = new SpinnableAwesomeButton();
    saveMeshButton->hide();
    saveMeshButton->setAwesomeIcon(QChar(fa::save));
    connect(saveMeshButton->button(), &QPushButton::clicked, this, &MainWindow::saveMesh);
    
    m_saveMeshButton = saveMeshButton;
    
    toolLayout->addStretch();
    toolLayout->addWidget(m_targetTriangleCountWidget);
    toolLayout->addSpacing(5);
    toolLayout->addWidget(edgeScalingLabel);
    toolLayout->addWidget(m_edgeScalingSelectBox);
    //toolLayout->addWidget(m_targetScalingWidget);
    //toolLayout->addWidget(m_modelTypeSelectBox);
    toolLayout->addSpacing(10);
    toolLayout->addWidget(loadModelButton);
    toolLayout->addWidget(saveMeshButton);
    
    QLabel *verticalLogoLabel = new QLabel;
    QImage verticalLogoImage;
    verticalLogoImage.load(":/resources/dust3d-vertical.png");
    verticalLogoLabel->setPixmap(QPixmap::fromImage(verticalLogoImage));

    QHBoxLayout *logoLayout = new QHBoxLayout;
    logoLayout->addWidget(verticalLogoLabel);
    logoLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *mainLeftLayout = new QVBoxLayout;
    mainLeftLayout->setSpacing(0);
    mainLeftLayout->setContentsMargins(0, 0, 0, 0);
    mainLeftLayout->addStretch();
    mainLeftLayout->addLayout(logoLayout);
    mainLeftLayout->addSpacing(10);
    
    QHBoxLayout *canvasLayout = new QHBoxLayout;
    canvasLayout->setSpacing(0);
    canvasLayout->setContentsMargins(0, 0, 0, 0);
    canvasLayout->addLayout(mainLeftLayout);
    canvasLayout->addWidget(containerWidget);
    canvasLayout->addSpacing(3);
    
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(toolLayout);
    mainLayout->addLayout(canvasLayout);

    QWidget *centralWidget = new QWidget;
    centralWidget->setLayout(mainLayout);

    setCentralWidget(centralWidget);
    updateTitle();
}

void MainWindow::updateButtonStates()
{
    if (nullptr == m_quadMeshGenerator &&
            !m_quadMeshResultIsDirty) {
        m_loadModelButton->showSpinner(false);
        //m_targetScalingWidget->setEnabled(true);
        m_targetTriangleCountWidget->setEnabled(true);
        m_edgeScalingSelectBox->setEnabled(true);
        //m_modelTypeSelectBox->setEnabled(true);
        if (nullptr != m_remeshedQuads) {
            m_saveMeshButton->show();
        } else {
            m_saveMeshButton->hide();
        }
    } else {
        m_loadModelButton->showSpinner(true);
        m_saveMeshButton->hide();
        //m_targetScalingWidget->setDisabled(true);
        m_targetTriangleCountWidget->setDisabled(true);
        m_edgeScalingSelectBox->setDisabled(true);
        //m_modelTypeSelectBox->setDisabled(true);
    }
}

bool MainWindow::loadObj(const QString &filename)
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
        auto &dest = m_originalVertices[i];
        dest.setX(attributes.vertices[j++]);
        dest.setY(attributes.vertices[j++]);
        dest.setZ(attributes.vertices[j++]);
    }
    
    m_originalTriangles.clear();
    for (const auto &shape: shapes) {
        for (size_t i = 0; i < shape.mesh.indices.size(); i += 3) {
            m_originalTriangles.push_back(std::vector<size_t> {
                (size_t)shape.mesh.indices[i + 0].vertex_index,
                (size_t)shape.mesh.indices[i + 1].vertex_index,
                (size_t)shape.mesh.indices[i + 2].vertex_index
            });
        }
    }
    
    qDebug() << "m_originalVertices.size():" << m_originalVertices.size();
    qDebug() << "m_originalTriangles.size():" << m_originalTriangles.size();
    
    m_renderQueue.push({
        m_originalVertices,
        m_originalTriangles
    });
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

void MainWindow::setCurrentFilename(const QString &filename)
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
        stream << "# " << APP_NAME << " " << APP_HUMAN_VER << endl;
        stream << "# " << APP_HOMEPAGE_URL << endl;
        for (std::vector<AutoRemesher::Vector3>::const_iterator it = m_remeshedVertices->begin() ; it != m_remeshedVertices->end(); ++it) {
            stream << "v " << (*it).x() << " " << (*it).y() << " " << (*it).z() << endl;
        }
        for (std::vector<std::vector<size_t>>::const_iterator it = m_remeshedQuads->begin() ; it != m_remeshedQuads->end(); ++it) {
            stream << "f";
            for (std::vector<size_t>::const_iterator subIt = (*it).begin() ; subIt != (*it).end(); ++subIt) {
                stream << " " << (1 + *subIt);
            }
            stream << endl;
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

MainWindow::~MainWindow()
{
    g_windows.erase(this);
}

PbrShaderWidget *MainWindow::modelRenderWidget() const
{
    return m_modelRenderWidget;
}

void MainWindow::showSupporters()
{
    if (!g_supportersWidget) {
        g_supportersWidget = new QTextBrowser;
        g_supportersWidget->setWindowTitle(unifiedWindowTitle(tr("Supporters")));
        g_supportersWidget->setMinimumSize(QSize(320, 250));
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

void MainWindow::showEvent(QShowEvent *event)
{
#ifdef Q_OS_WIN32
    m_taskbarButton->setWindow(windowHandle());
    m_taskbarButton->progress()->setVisible(true);
#endif

    event->accept();
}

void MainWindow::closeEvent(QCloseEvent *event)
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
    
    QThread *thread = new QThread;
    
    const auto &item = m_renderQueue.front();
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
    PbrShaderMesh *renderMesh = m_renderMeshGenerator->takeRenderMesh();
    
    qDebug() << "Render mesh ready";
    
    delete m_renderMeshGenerator;
    m_renderMeshGenerator = nullptr;
    
    m_modelRenderWidget->updateMesh(renderMesh);
    
    checkRenderQueue();
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
    
    QThread *thread = new QThread;
    
    QuadMeshGenerator::Parameters parameters;
    
    {
        const int base = 100000;
        const int range = 500000;
        parameters.targetTriangleCount = base + range * m_targetDensity;
    }
    {
        parameters.scaling = m_targetScaling;
    }
    parameters.modelType = m_modelType;
    
    m_quadMeshGenerator = new QuadMeshGenerator(m_originalVertices, m_originalTriangles);
    connect(m_quadMeshGenerator, &QuadMeshGenerator::reportProgress, this, &MainWindow::updateProgress);
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
        m_renderQueue.push({
            *m_remeshedVertices,
            *m_remeshedQuads
        });
        checkRenderQueue();
    } else {
        m_renderQueue.push({
            std::vector<AutoRemesher::Vector3>(),
            std::vector<std::vector<size_t>>()
        });
        checkRenderQueue();
    }
    
    if (m_quadMeshResultIsDirty)
        generateQuadMesh();
    
    updateButtonStates();
    updateTitle();
}
