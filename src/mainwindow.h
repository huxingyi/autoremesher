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
#ifndef AUTO_REMESHER_MAIN_WINDOW_H
#define AUTO_REMESHER_MAIN_WINDOW_H
#include <QMainWindow>
#include <QCloseEvent>
#include <QShowEvent>
#include <QString>
#include <queue>
#include <AutoRemesher/AutoRemesher>
#include <AutoRemesher/Vector3>
#include "pbrshaderwidget.h"

class RenderMeshGenerator;
class QuadMeshGenerator;
class SpinnableAwesomeButton;
class FloatNumberWidget;
class QComboBox;
#ifdef Q_OS_WIN32
class QWinTaskbarButton;
#endif

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    struct ResultMesh
    {
        std::vector<AutoRemesher::Vector3> vertices;
        std::vector<std::vector<size_t>> faces;
    };

    MainWindow();
    ~MainWindow();
    PbrShaderWidget *modelRenderWidget() const;
    static size_t total();
protected:
    void closeEvent(QCloseEvent *event);
    void showEvent(QShowEvent *event);
private slots:
    void showSupporters();
    void showAcknowlegements();
    void viewSource();
    void gotoHomepage();
    void reportIssues();
    void showAbout();
    void checkForUpdates();
    void updateTitle();
    void loadModel();
    void saveMesh();
    bool loadObj(const QString &filename);
    void setCurrentFilename(const QString &filename);
    void checkRenderQueue();
    void renderMeshReady();
    void generateQuadMesh();
    void quadMeshReady();
    void updateButtonStates();
    void updateProgress(float progress);
private:
    PbrShaderWidget *m_modelRenderWidget = nullptr;
    AutoRemesher::AutoRemesher *m_autoRemesher = nullptr;
    bool m_inProgress = false;
    bool m_saved = true;
    float m_targetDensity = 0.0;
    float m_targetScaling = 2.0;
    AutoRemesher::ModelType m_modelType = AutoRemesher::ModelType::Organic;
    std::vector<AutoRemesher::Vector3> m_originalVertices;
    std::vector<std::vector<size_t>> m_originalTriangles;
    std::vector<AutoRemesher::Vector3> *m_remeshedVertices = nullptr;
    std::vector<std::vector<size_t>> *m_remeshedQuads = nullptr;
    QString m_currentFilename;
    RenderMeshGenerator *m_renderMeshGenerator = nullptr;
    std::queue<ResultMesh> m_renderQueue;
    bool m_quadMeshResultIsDirty = false;
    QuadMeshGenerator *m_quadMeshGenerator = nullptr;
    SpinnableAwesomeButton *m_loadModelButton = nullptr;
    SpinnableAwesomeButton *m_saveMeshButton = nullptr;
    FloatNumberWidget *m_targetTriangleCountWidget = nullptr;
    FloatNumberWidget *m_targetScalingWidget = nullptr;
    //QComboBox *m_modelTypeSelectBox = nullptr;
    QComboBox *m_edgeScalingSelectBox = nullptr;
#ifdef Q_OS_WIN32
    QWinTaskbarButton *m_taskbarButton = nullptr;
#endif
};

#endif
