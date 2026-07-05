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
#ifndef AUTO_REMESHER_MAIN_WINDOW_H
#define AUTO_REMESHER_MAIN_WINDOW_H
#include "modelshaderwidget.h"
#include <AutoRemesher/AutoRemesher>
#include <AutoRemesher/Vector3>
#include <QCloseEvent>
#include <QMainWindow>
#include <QProgressBar>
#include <QPushButton>
#include <QShowEvent>
#include <QString>
#include <queue>

class RenderMeshGenerator;
class QuadMeshGenerator;
class FloatNumberWidget;
class IntNumberWidget;
class QLabel;
#ifdef Q_OS_WIN32
class QWinTaskbarButton;
#endif

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    struct ResultMesh {
        std::vector<AutoRemesher::Vector3> vertices;
        std::vector<std::vector<size_t>> faces;
    };

    MainWindow();
    ~MainWindow();
    ModelShaderWidget* modelRenderWidget() const;
    static size_t total();

protected:
    void closeEvent(QCloseEvent* event);
    void showEvent(QShowEvent* event);
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
    bool loadObj(const QString& filename);
    void setCurrentFilename(const QString& filename);
    void checkRenderQueue();
    void renderMeshReady();
    void generateQuadMesh();
    void quadMeshReady();
    void updateButtonStates();
    void updateProgress(float progress);
    void updateProgressDetailed(float progress, const QString& status);

private:
    ModelShaderWidget* m_modelRenderWidget = nullptr;
    AutoRemesher::AutoRemesher* m_autoRemesher = nullptr;
    bool m_inProgress = false;
    bool m_saved = true;
    int m_targetQuadCount = 50000;
    float m_targetScaling = 1.0;
    float m_sharpEdgeDegrees = 90.0;
    float m_smoothNormalDegrees = 0.0;
    float m_adaptivity = 1.0;
    AutoRemesher::ModelType m_modelType = AutoRemesher::ModelType::Organic;
    std::vector<AutoRemesher::Vector3> m_originalVertices;
    std::vector<std::vector<size_t>> m_originalTriangles;
    std::vector<AutoRemesher::Vector3>* m_remeshedVertices = nullptr;
    std::vector<std::vector<size_t>>* m_remeshedQuads = nullptr;
    QString m_currentFilename;
    RenderMeshGenerator* m_renderMeshGenerator = nullptr;
    std::queue<ResultMesh> m_renderQueue;
    bool m_quadMeshResultIsDirty = false;
    QuadMeshGenerator* m_quadMeshGenerator = nullptr;
    QPushButton* m_loadModelButton = nullptr;
    QPushButton* m_saveMeshButton = nullptr;
    QPushButton* m_regenerateButton = nullptr;
    IntNumberWidget* m_targetQuadCountWidget = nullptr;
    FloatNumberWidget* m_targetScalingWidget = nullptr;
    //QComboBox *m_modelTypeSelectBox = nullptr;
    FloatNumberWidget* m_sharpEdgeDegreesWidget = nullptr;
    FloatNumberWidget* m_smoothNormalDegreesWidget = nullptr;
    FloatNumberWidget* m_adaptivityWidget = nullptr;
    QLabel* m_quadCountLabel = nullptr;
    QLabel* m_nonQuadCountLabel = nullptr;
    QLabel* m_vertexCountLabel = nullptr;
    QProgressBar* m_progressBar = nullptr;
    QWidget* m_progressContainer = nullptr;
#ifdef Q_OS_WIN32
    QWinTaskbarButton* m_taskbarButton = nullptr;
#endif
};

#endif