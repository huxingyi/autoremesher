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
#include <QMouseEvent>
#include <QOpenGLShaderProgram>
#include <QCoreApplication>
#include <QGuiApplication>
#include <cmath>
#include <QVector4D>
#include <QSurfaceFormat>
#include "pbrshaderwidget.h"

bool PbrShaderWidget::m_transparent = true;
float PbrShaderWidget::m_minZoomRatio = 5.0;
float PbrShaderWidget::m_maxZoomRatio = 80.0;

int PbrShaderWidget::m_defaultXRotation = 30 * 16;
int PbrShaderWidget::m_defaultYRotation = -45 * 16;
int PbrShaderWidget::m_defaultZRotation = 0;
QVector3D PbrShaderWidget::m_defaultEyePosition = QVector3D(0, 0, -4.0);

PbrShaderWidget::PbrShaderWidget(QWidget *parent) :
    QOpenGLWidget(parent)
{
    if (m_transparent) {
        setAttribute(Qt::WA_AlwaysStackOnTop);
        setAttribute(Qt::WA_TranslucentBackground);
        QSurfaceFormat fmt = format();
        fmt.setAlphaBufferSize(8);
        fmt.setSamples(8);
        setFormat(fmt);
    } else {
        QSurfaceFormat fmt = format();
        fmt.setSamples(8);
        setFormat(fmt);
    }
    setContextMenuPolicy(Qt::CustomContextMenu);
    
    m_widthInPixels = width() * window()->devicePixelRatio();
	m_heightInPixels = height() * window()->devicePixelRatio();
	
    zoom(200);
}

const QVector3D &PbrShaderWidget::eyePosition()
{
	return m_eyePosition;
}

const QVector3D &PbrShaderWidget::moveToPosition()
{
    return m_moveToPosition;
}

void PbrShaderWidget::setEyePosition(const QVector3D &eyePosition)
{
    m_eyePosition = eyePosition;
    emit eyePositionChanged(m_eyePosition);
    update();
}

void PbrShaderWidget::reRender()
{
    emit renderParametersChanged();
    update();
}

int PbrShaderWidget::xRotation()
{
    return m_xRotation;
}

int PbrShaderWidget::yRotation()
{
    return m_yRotation;
}

int PbrShaderWidget::zRotation()
{
    return m_zRotation;
}

PbrShaderWidget::~PbrShaderWidget()
{
    cleanup();
}

void PbrShaderWidget::normalizeAngle(int &angle)
{
    while (angle < 0)
        angle += 360 * 16;
    while (angle > 360 * 16)
        angle -= 360 * 16;
}

void PbrShaderWidget::setXRotation(int angle)
{
    normalizeAngle(angle);
    if (angle != m_xRotation) {
        m_xRotation = angle;
        emit xRotationChanged(angle);
        emit renderParametersChanged();
        update();
    }
}

void PbrShaderWidget::setYRotation(int angle)
{
    normalizeAngle(angle);
    if (angle != m_yRotation) {
        m_yRotation = angle;
        emit yRotationChanged(angle);
        emit renderParametersChanged();
        update();
    }
}

void PbrShaderWidget::setZRotation(int angle)
{
    normalizeAngle(angle);
    if (angle != m_zRotation) {
        m_zRotation = angle;
        emit zRotationChanged(angle);
        emit renderParametersChanged();
        update();
    }
}

void PbrShaderWidget::cleanup()
{
    if (m_program == nullptr)
        return;
    makeCurrent();
    m_meshBinder.cleanup();
    delete m_program;
    m_program = nullptr;
    doneCurrent();
}

void PbrShaderWidget::initializeGL()
{
    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &PbrShaderWidget::cleanup);

    initializeOpenGLFunctions();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	if (m_enableCullFace)
		glEnable(GL_CULL_FACE);
#ifdef GL_LINE_SMOOTH
	glEnable(GL_LINE_SMOOTH);
#endif
    if (m_transparent) {
        glClearColor(0, 0, 0, 0);
    } else {
        QColor bgcolor = QWidget::palette().color(QWidget::backgroundRole());
        glClearColor(bgcolor.redF(), bgcolor.greenF(), bgcolor.blueF(), 1);
    }
    
    bool isCoreProfile = false;
    const char *versionString = (const char *)glGetString(GL_VERSION);
    if (nullptr != versionString &&
            '\0' != versionString[0] &&
            0 == strstr(versionString, "Mesa")) {
        isCoreProfile = format().profile() == QSurfaceFormat::CoreProfile;
    }
    qDebug() << "isCoreProfile:" << isCoreProfile << "versionString:" << versionString;
        
    m_program = new PbrShaderProgram(isCoreProfile);

    m_meshBinder.initialize();

    m_program->release();
}

void PbrShaderWidget::disableCullFace()
{
    m_enableCullFace = false;
}

void PbrShaderWidget::setMoveToPosition(const QVector3D &moveToPosition)
{
    m_moveToPosition = moveToPosition;
}

void PbrShaderWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, m_widthInPixels, m_heightInPixels);

    m_world.setToIdentity();
    m_world.rotate(m_xRotation / 16.0f, 1, 0, 0);
    m_world.rotate(m_yRotation / 16.0f, 0, 1, 0);
    m_world.rotate(m_zRotation / 16.0f, 0, 0, 1);
    
    m_camera.setToIdentity();
    m_camera.translate(m_eyePosition.x(), m_eyePosition.y(), m_eyePosition.z());

    m_program->bind();
    m_program->setEyePosValue(m_eyePosition);
    m_program->setProjectionMatrixValue(m_projection);
    m_program->setModelMatrixValue(m_world);
    QMatrix3x3 normalMatrix = m_world.normalMatrix();
    m_program->setNormalMatrixValue(normalMatrix);
    m_program->setViewMatrixValue(m_camera);
    m_program->setTextureEnabledValue(0);
    m_program->setNormalMapEnabledValue(0);
    
    if (m_mousePickingEnabled && !m_mousePickTargetPositionInModelSpace.isNull()) {
        m_program->setMousePickEnabledValue(1);
        m_program->setMousePickTargetPositionValue(m_world * m_mousePickTargetPositionInModelSpace);
    } else {
        m_program->setMousePickEnabledValue(0);
        m_program->setMousePickTargetPositionValue(QVector3D());
    }
    m_program->setMousePickRadiusValue(m_mousePickRadius);
    
    m_meshBinder.paint(m_program);

    m_program->release();
}

void PbrShaderWidget::updateProjectionMatrix()
{
    m_projection.setToIdentity();
    m_projection.translate(m_moveToPosition.x(), m_moveToPosition.y(), m_moveToPosition.z());
    m_projection.perspective(45.0f, GLfloat(width()) / height(), 0.01f, 100.0f);
}

void PbrShaderWidget::resizeGL(int w, int h)
{
	m_widthInPixels = w * window()->devicePixelRatio();
	m_heightInPixels = h * window()->devicePixelRatio();
    updateProjectionMatrix();
    emit renderParametersChanged();
}

std::pair<QVector3D, QVector3D> PbrShaderWidget::mousePositionToMouseRay(const QPoint &mousePosition)
{
    auto modelView = m_camera * m_world;
    float x = qMax(qMin(mousePosition.x(), width() - 1), 0);
    float y = qMax(qMin(mousePosition.y(), height() - 1), 0);
    QVector3D nearScreen = QVector3D(x, height() - y, 0.0);
    QVector3D farScreen = QVector3D(x, height() - y, 1.0);
    auto viewPort = QRect(0, 0, width(), height());
    auto nearPosition = nearScreen.unproject(modelView, m_projection, viewPort);
    auto farPosition = farScreen.unproject(modelView, m_projection, viewPort);
    return std::make_pair(nearPosition, farPosition);
}

void PbrShaderWidget::toggleWireframe()
{
    if (m_meshBinder.isWireframeVisible())
        m_meshBinder.hideWireframe();
    else
        m_meshBinder.showWireframe();
    update();
}

bool PbrShaderWidget::isWireframeVisible()
{
    return m_meshBinder.isWireframeVisible();
}

void PbrShaderWidget::enableEnvironmentLight()
{
    m_meshBinder.enableEnvironmentLight();
    update();
}

bool PbrShaderWidget::isEnvironmentLightEnabled()
{
    return m_meshBinder.isEnvironmentLightEnabled();
}

void PbrShaderWidget::toggleRotation()
{
    if (nullptr != m_rotationTimer) {
        delete m_rotationTimer;
        m_rotationTimer = nullptr;
    } else {
        m_rotationTimer = new QTimer(this);
        m_rotationTimer->setInterval(42);
        m_rotationTimer->setSingleShot(false);
        connect(m_rotationTimer, &QTimer::timeout, this, [&]() {
            setYRotation(m_yRotation - 8);
        });
        m_rotationTimer->start();
    }
}

bool PbrShaderWidget::inputMousePressEventFromOtherWidget(QMouseEvent *event)
{
    bool shouldStartMove = false;
    if (event->button() == Qt::LeftButton) {
        if (QGuiApplication::queryKeyboardModifiers().testFlag(Qt::AltModifier) &&
                !QGuiApplication::queryKeyboardModifiers().testFlag(Qt::ControlModifier)) {
            shouldStartMove = m_moveEnabled;
        }
        if (!shouldStartMove/* && !m_mousePickTargetPositionInModelSpace.isNull()*/)
            emit mousePressed(event->globalPos());
    } else if (event->button() == Qt::MidButton) {
        shouldStartMove = m_moveEnabled;
    }
    if (shouldStartMove) {
        m_lastPos = convertInputPosFromOtherWidget(event);
        if (!m_moveStarted) {
            m_moveStartPos = mapToParent(convertInputPosFromOtherWidget(event));
            m_moveStartGeometry = geometry();
            m_moveStarted = true;
        }
        return true;
    }
    return false;
}

bool PbrShaderWidget::inputMouseReleaseEventFromOtherWidget(QMouseEvent *event)
{
    Q_UNUSED(event);
    if (m_moveStarted) {
        m_moveStarted = false;
        return true;
    }
    if (event->button() == Qt::LeftButton) {
        if (m_mousePickingEnabled)
            emit mouseReleased(event->globalPos());
    }
    return false;
}

void PbrShaderWidget::canvasResized()
{
    resize(parentWidget()->size());
}

bool PbrShaderWidget::inputMouseMoveEventFromOtherWidget(QMouseEvent *event)
{
    QPoint pos = convertInputPosFromOtherWidget(event);
    
    if (m_mousePickingEnabled) {
        auto segment = mousePositionToMouseRay(pos);
        emit mouseRayChanged(segment.first, segment.second);
    }

    if (!m_moveStarted) {
        return false;
    }
    
    int dx = pos.x() - m_lastPos.x();
    int dy = pos.y() - m_lastPos.y();

    if ((event->buttons() & Qt::MidButton) ||
            (m_moveStarted && (event->buttons() & Qt::LeftButton))) {
        if (QGuiApplication::queryKeyboardModifiers().testFlag(Qt::ShiftModifier)) {
            if (m_moveStarted) {
                if (m_moveAndZoomByWindow) {
                    QPoint posInParent = mapToParent(pos);
                    QRect rect = m_moveStartGeometry;
                    rect.translate(posInParent.x() - m_moveStartPos.x(), posInParent.y() - m_moveStartPos.y());
                    setGeometry(rect);
                } else {
                    m_moveToPosition.setX(m_moveToPosition.x() + (float)2 * dx / width());
                    m_moveToPosition.setY(m_moveToPosition.y() + (float)2 * -dy / height());
                    if (m_moveToPosition.x() < -1.5)
                        m_moveToPosition.setX(-1.5);
                    if (m_moveToPosition.x() > 1.5)
                        m_moveToPosition.setX(1.5);
                    if (m_moveToPosition.y() < -1.5)
                        m_moveToPosition.setY(-1.5);
                    if (m_moveToPosition.y() > 1.5)
                        m_moveToPosition.setY(1.5);
                    updateProjectionMatrix();
                    emit moveToPositionChanged(m_moveToPosition);
                    emit renderParametersChanged();
                    update();
                }
            }
        } else {
            setXRotation(m_xRotation + 8 * dy);
            setYRotation(m_yRotation + 8 * dx);
        }
    }
    m_lastPos = pos;
    
    return true;
}

QPoint PbrShaderWidget::convertInputPosFromOtherWidget(QMouseEvent *event)
{
    return mapFromGlobal(event->globalPos());
}

bool PbrShaderWidget::inputWheelEventFromOtherWidget(QWheelEvent *event)
{
    if (m_moveStarted)
        return true;
    
    if (m_mousePickingEnabled) {
        if (QGuiApplication::queryKeyboardModifiers().testFlag(Qt::ShiftModifier)) {
			if (event->delta() > 0)
				emit addMouseRadius(0.001f);
			else if (event->delta() < 0)
				emit addMouseRadius(-0.001f);
            return true;
        }
    }
    
    if (!m_zoomEnabled)
        return false;

    qreal delta = geometry().height() * 0.1f;
    if (event->delta() < 0)
        delta = -delta;
    zoom(delta);
    
    return true;
}

void PbrShaderWidget::zoom(float delta)
{
    if (m_moveAndZoomByWindow) {
        QMargins margins(delta, delta, delta, delta);
        if (0 == m_modelInitialHeight) {
            m_modelInitialHeight = height();
        } else {
            float ratio = (float)height() / m_modelInitialHeight;
            if (ratio <= m_minZoomRatio) {
                if (delta < 0)
                    return;
            } else if (ratio >= m_maxZoomRatio) {
                if (delta > 0)
                    return;
            }
        }
        setGeometry(geometry().marginsAdded(margins));
        emit renderParametersChanged();
        update();
        return;
    } else {
        m_eyePosition += QVector3D(0, 0, m_eyePosition.z() * (delta > 0 ? -0.1 : 0.1));
        if (m_eyePosition.z() < -15)
            m_eyePosition.setZ(-15);
        else if (m_eyePosition.z() > -0.1)
            m_eyePosition.setZ(-0.1f);
        emit eyePositionChanged(m_eyePosition);
        emit renderParametersChanged();
        update();
    }
}

void PbrShaderWidget::setMousePickTargetPositionInModelSpace(QVector3D position)
{
    m_mousePickTargetPositionInModelSpace = position;
    update();
}

void PbrShaderWidget::setMousePickRadius(float radius)
{
    m_mousePickRadius = radius;
    update();
}

void PbrShaderWidget::updateMesh(PbrShaderMesh *mesh)
{
    m_meshBinder.updateMesh(mesh);
    emit renderParametersChanged();
    update();
}

void PbrShaderWidget::fetchCurrentToonNormalAndDepthMaps(QImage *normalMap, QImage *depthMap)
{
    m_meshBinder.fetchCurrentToonNormalAndDepthMaps(normalMap, depthMap);
}

void PbrShaderWidget::updateToonNormalAndDepthMaps(QImage *normalMap, QImage *depthMap)
{
    m_meshBinder.updateToonNormalAndDepthMaps(normalMap, depthMap);
    update();
}

int PbrShaderWidget::widthInPixels()
{
	return m_widthInPixels;
}

int PbrShaderWidget::heightInPixels()
{
	return m_heightInPixels;
}

void PbrShaderWidget::enableMove(bool enabled)
{
    m_moveEnabled = enabled;
}

void PbrShaderWidget::enableZoom(bool enabled)
{
    m_zoomEnabled = enabled;
}

void PbrShaderWidget::enableMousePicking(bool enabled)
{
    m_mousePickingEnabled = enabled;
}

void PbrShaderWidget::setMoveAndZoomByWindow(bool byWindow)
{
    m_moveAndZoomByWindow = byWindow;
}

void PbrShaderWidget::mousePressEvent(QMouseEvent *event)
{
    inputMousePressEventFromOtherWidget(event);
}

void PbrShaderWidget::mouseMoveEvent(QMouseEvent *event)
{
    inputMouseMoveEventFromOtherWidget(event);
}

void PbrShaderWidget::wheelEvent(QWheelEvent *event)
{
    inputWheelEventFromOtherWidget(event);
}

void PbrShaderWidget::mouseReleaseEvent(QMouseEvent *event)
{
    inputMouseReleaseEventFromOtherWidget(event);
}

