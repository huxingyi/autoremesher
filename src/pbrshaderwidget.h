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
#ifndef AUTO_REMESHER_PBR_SHADER_WIDGET_H
#define AUTO_REMESHER_PBR_SHADER_WIDGET_H
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include <QMutex>
#include <QVector2D>
#include <QTimer>
#include "pbrshadermesh.h"
#include "pbrshaderprogram.h"
#include "pbrshadermeshbinder.h"

class PbrShaderWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
signals:
    void mouseRayChanged(const QVector3D &near, const QVector3D &far);
    void mousePressed(QPoint globalPos);
    void mouseReleased(QPoint globalPos);
    void addMouseRadius(float radius);
    void renderParametersChanged();
    void xRotationChanged(int angle);
    void yRotationChanged(int angle);
    void zRotationChanged(int angle);
    void eyePositionChanged(const QVector3D &eyePosition);
    void moveToPositionChanged(const QVector3D &moveToPosition);
public:
    PbrShaderWidget(QWidget *parent = 0);
    ~PbrShaderWidget();
    static bool isTransparent()
    {
        return m_transparent;
    }
    static void setTransparent(bool t)
    {
        m_transparent = t;
    }
    void updateMesh(PbrShaderMesh *mesh);
    void toggleWireframe();
    bool isWireframeVisible();
    void toggleRotation();
    void toggleUvCheck();
    void enableEnvironmentLight();
    bool isEnvironmentLightEnabled();
    void enableMove(bool enabled);
    void enableZoom(bool enabled);
    void enableMousePicking(bool enabled);
    void setMoveAndZoomByWindow(bool byWindow);
    void disableCullFace();
    void setMoveToPosition(const QVector3D &moveToPosition);
    bool inputMousePressEventFromOtherWidget(QMouseEvent *event);
    bool inputMouseMoveEventFromOtherWidget(QMouseEvent *event);
    bool inputWheelEventFromOtherWidget(QWheelEvent *event);
    bool inputMouseReleaseEventFromOtherWidget(QMouseEvent *event);
    QPoint convertInputPosFromOtherWidget(QMouseEvent *event);
    void fetchCurrentToonNormalAndDepthMaps(QImage *normalMap, QImage *depthMap);
    void updateToonNormalAndDepthMaps(QImage *normalMap, QImage *depthMap);
    int widthInPixels();
    int heightInPixels();
    int xRotation();
    int yRotation();
    int zRotation();
    const QVector3D &eyePosition();
    const QVector3D &moveToPosition();
    std::pair<QVector3D, QVector3D> mousePositionToMouseRay(const QPoint &mousePosition);
public slots:
    void setXRotation(int angle);
    void setYRotation(int angle);
    void setZRotation(int angle);
    void setEyePosition(const QVector3D &eyePosition);
    void cleanup();
    void zoom(float delta);
    void setMousePickTargetPositionInModelSpace(QVector3D position);
    void setMousePickRadius(float radius);
    void reRender();
    void canvasResized();
protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
private:
    int m_xRotation = m_defaultXRotation;
    int m_yRotation = m_defaultYRotation;
    int m_zRotation = m_defaultZRotation;
    PbrShaderProgram *m_program = nullptr;
    bool m_moveStarted = false;
    bool m_moveEnabled = true;
    bool m_zoomEnabled = true;
    bool m_mousePickingEnabled = false;
    QVector3D m_mousePickTargetPositionInModelSpace;
    QPoint m_lastPos;
    PbrShaderMeshBinder m_meshBinder;
    QMatrix4x4 m_projection;
    QMatrix4x4 m_camera;
    QMatrix4x4 m_world;
    float m_mousePickRadius = 0.0;
    QVector3D m_eyePosition = m_defaultEyePosition;
    static bool m_transparent;
    static float m_minZoomRatio;
    static float m_maxZoomRatio;
    QPoint m_moveStartPos;
    QRect m_moveStartGeometry;
    int m_modelInitialHeight = 0;
    QTimer *m_rotationTimer = nullptr;
    int m_widthInPixels = 0;
    int m_heightInPixels = 0;
    QVector3D m_moveToPosition;
    bool m_moveAndZoomByWindow = true;
    bool m_enableCullFace = true;
    void updateProjectionMatrix();
    void normalizeAngle(int &angle);
public:
    static int m_defaultXRotation;
    static int m_defaultYRotation;
    static int m_defaultZRotation;
    static QVector3D m_defaultEyePosition;
};

#endif
