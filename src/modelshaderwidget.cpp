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
#include "modelshaderwidget.h"

// QMouseEvent::globalPos() was removed in Qt 6 in favor of
// globalPosition().toPoint(). This helper keeps the code building on both
// Qt 5 and Qt 6.
static inline QPoint mouseEventGlobalPos(QMouseEvent *event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return event->globalPosition().toPoint();
#else
    return event->globalPos();
#endif
}

bool ModelShaderWidget::m_transparent = true;
float ModelShaderWidget::m_minZoomRatio = 5.0;
float ModelShaderWidget::m_maxZoomRatio = 80.0;

int ModelShaderWidget::m_defaultXRotation = 30 * 16;
int ModelShaderWidget::m_defaultYRotation = -45 * 16;
int ModelShaderWidget::m_defaultZRotation = 0;
QVector3D ModelShaderWidget::m_defaultEyePosition = QVector3D(0, 0, -4.0);

ModelShaderWidget::ModelShaderWidget(QWidget *parent) :
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

const QVector3D &ModelShaderWidget::eyePosition()
{
	return m_eyePosition;
}

const QVector3D &ModelShaderWidget::moveToPosition()
{
    return m_moveToPosition;
}

void ModelShaderWidget::setEyePosition(const QVector3D &eyePosition)
{
    m_eyePosition = eyePosition;
    emit eyePositionChanged(m_eyePosition);
    update();
}

void ModelShaderWidget::reRender()
{
    emit renderParametersChanged();
    update();
}

int ModelShaderWidget::xRotation()
{
    return m_xRotation;
}

int ModelShaderWidget::yRotation()
{
    return m_yRotation;
}

int ModelShaderWidget::zRotation()
{
    return m_zRotation;
}

ModelShaderWidget::~ModelShaderWidget()
{
    cleanup();
}

void ModelShaderWidget::normalizeAngle(int &angle)
{
    while (angle < 0)
        angle += 360 * 16;
    while (angle > 360 * 16)
        angle -= 360 * 16;
}

void ModelShaderWidget::setXRotation(int angle)
{
    normalizeAngle(angle);
    if (angle != m_xRotation) {
        m_xRotation = angle;
        emit xRotationChanged(angle);
        emit renderParametersChanged();
        update();
    }
}

void ModelShaderWidget::setYRotation(int angle)
{
    normalizeAngle(angle);
    if (angle != m_yRotation) {
        m_yRotation = angle;
        emit yRotationChanged(angle);
        emit renderParametersChanged();
        update();
    }
}

void ModelShaderWidget::setZRotation(int angle)
{
    normalizeAngle(angle);
    if (angle != m_zRotation) {
        m_zRotation = angle;
        emit zRotationChanged(angle);
        emit renderParametersChanged();
        update();
    }
}

void ModelShaderWidget::cleanup()
{
    if (m_program == nullptr)
        return;
    makeCurrent();
    m_meshBinder.cleanup();
    delete m_program;
    m_program = nullptr;
    if (m_monochromeProgram) {
        delete m_monochromeProgram;
        m_monochromeProgram = nullptr;
    }
    doneCurrent();
}

void ModelShaderWidget::initializeGL()
{
    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &ModelShaderWidget::cleanup);

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
        QColor bgcolor(0x3D, 0x3D, 0x3D);
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
        
    m_program = new ModelShaderProgram(isCoreProfile);
    m_monochromeProgram = new MonochromeOpenGLProgram();
    m_monochromeProgram->load(isCoreProfile);

    m_meshBinder.initialize();

    m_program->release();
    m_monochromeProgram->release();
}

void ModelShaderWidget::disableCullFace()
{
    m_enableCullFace = false;
}

void ModelShaderWidget::setMoveToPosition(const QVector3D &moveToPosition)
{
    m_moveToPosition = moveToPosition;
}

void ModelShaderWidget::paintGL()
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
    m_program->setEyePositionValue(m_eyePosition);
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

    // Draw wireframe overlay using dedicated monochrome shader
    if (m_meshBinder.isWireframeVisible()) {
        m_monochromeProgram->bind();
        m_monochromeProgram->setUniformValue(
            m_monochromeProgram->getUniformLocationByName("projectionMatrix"), m_projection);
        m_monochromeProgram->setUniformValue(
            m_monochromeProgram->getUniformLocationByName("modelMatrix"), m_world);
        m_monochromeProgram->setUniformValue(
            m_monochromeProgram->getUniformLocationByName("viewMatrix"), m_camera);
        if (m_monochromeProgram->isCoreProfile()) {
            m_monochromeProgram->setUniformValue("viewportSize",
                QVector2D(m_widthInPixels, m_heightInPixels));
        }
        m_meshBinder.paintWireframe();
    }
}

void ModelShaderWidget::updateProjectionMatrix()
{
    m_projection.setToIdentity();
    m_projection.translate(m_moveToPosition.x(), m_moveToPosition.y(), m_moveToPosition.z());
    m_projection.perspective(45.0f, GLfloat(width()) / height(), 0.01f, 100.0f);
}

void ModelShaderWidget::resizeGL(int w, int h)
{
	m_widthInPixels = w * window()->devicePixelRatio();
	m_heightInPixels = h * window()->devicePixelRatio();
    updateProjectionMatrix();
    emit renderParametersChanged();
}

std::pair<QVector3D, QVector3D> ModelShaderWidget::mousePositionToMouseRay(const QPoint &mousePosition)
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

void ModelShaderWidget::toggleWireframe()
{
    if (m_meshBinder.isWireframeVisible())
        m_meshBinder.hideWireframe();
    else
        m_meshBinder.showWireframe();
    update();
}

bool ModelShaderWidget::isWireframeVisible()
{
    return m_meshBinder.isWireframeVisible();
}

void ModelShaderWidget::toggleRotation()
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

bool ModelShaderWidget::inputMousePressEventFromOtherWidget(QMouseEvent *event)
{
    bool shouldStartMove = false;
    if (event->button() == Qt::LeftButton) {
        if (QGuiApplication::queryKeyboardModifiers().testFlag(Qt::AltModifier) &&
                !QGuiApplication::queryKeyboardModifiers().testFlag(Qt::ControlModifier)) {
            shouldStartMove = m_moveEnabled;
        }
        if (!shouldStartMove/* && !m_mousePickTargetPositionInModelSpace.isNull()*/)
            emit mousePressed(mouseEventGlobalPos(event));
    } else if (event->button() == Qt::MiddleButton) {
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

bool ModelShaderWidget::inputMouseReleaseEventFromOtherWidget(QMouseEvent *event)
{
    Q_UNUSED(event);
    if (m_moveStarted) {
        m_moveStarted = false;
        return true;
    }
    if (event->button() == Qt::LeftButton) {
        if (m_mousePickingEnabled)
            emit mouseReleased(mouseEventGlobalPos(event));
    }
    return false;
}

void ModelShaderWidget::canvasResized()
{
    resize(parentWidget()->size());
}

bool ModelShaderWidget::inputMouseMoveEventFromOtherWidget(QMouseEvent *event)
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

    if ((event->buttons() & Qt::MiddleButton) ||
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

QPoint ModelShaderWidget::convertInputPosFromOtherWidget(QMouseEvent *event)
{
    return mapFromGlobal(mouseEventGlobalPos(event));
}

bool ModelShaderWidget::inputWheelEventFromOtherWidget(QWheelEvent *event)
{
    if (m_moveStarted)
        return true;
    
    if (m_mousePickingEnabled) {
        if (QGuiApplication::queryKeyboardModifiers().testFlag(Qt::ShiftModifier)) {
			if (event->angleDelta().y() > 0)
				emit addMouseRadius(0.001f);
			else if (event->angleDelta().y() < 0)
				emit addMouseRadius(-0.001f);
            return true;
        }
    }
    
    if (!m_zoomEnabled)
        return false;

    qreal delta = geometry().height() * 0.1f;
    if (event->angleDelta().y() < 0)
        delta = -delta;
    zoom(delta);
    
    return true;
}

void ModelShaderWidget::zoom(float delta)
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

void ModelShaderWidget::setMousePickTargetPositionInModelSpace(QVector3D position)
{
    m_mousePickTargetPositionInModelSpace = position;
    update();
}

void ModelShaderWidget::setMousePickRadius(float radius)
{
    m_mousePickRadius = radius;
    update();
}

void ModelShaderWidget::updateMesh(ModelShaderMesh *mesh)
{
    m_meshBinder.updateMesh(mesh);
    emit renderParametersChanged();
    update();
}

void ModelShaderWidget::fetchCurrentToonNormalAndDepthMaps(QImage *normalMap, QImage *depthMap)
{
    m_meshBinder.fetchCurrentToonNormalAndDepthMaps(normalMap, depthMap);
}

void ModelShaderWidget::updateToonNormalAndDepthMaps(QImage *normalMap, QImage *depthMap)
{
    m_meshBinder.updateToonNormalAndDepthMaps(normalMap, depthMap);
    update();
}

int ModelShaderWidget::widthInPixels()
{
	return m_widthInPixels;
}

int ModelShaderWidget::heightInPixels()
{
	return m_heightInPixels;
}

void ModelShaderWidget::enableMove(bool enabled)
{
    m_moveEnabled = enabled;
}

void ModelShaderWidget::enableZoom(bool enabled)
{
    m_zoomEnabled = enabled;
}

void ModelShaderWidget::enableMousePicking(bool enabled)
{
    m_mousePickingEnabled = enabled;
}

void ModelShaderWidget::setMoveAndZoomByWindow(bool byWindow)
{
    m_moveAndZoomByWindow = byWindow;
}

void ModelShaderWidget::mousePressEvent(QMouseEvent *event)
{
    inputMousePressEventFromOtherWidget(event);
}

void ModelShaderWidget::mouseMoveEvent(QMouseEvent *event)
{
    inputMouseMoveEventFromOtherWidget(event);
}

void ModelShaderWidget::wheelEvent(QWheelEvent *event)
{
    inputWheelEventFromOtherWidget(event);
}

void ModelShaderWidget::mouseReleaseEvent(QMouseEvent *event)
{
    inputMouseReleaseEventFromOtherWidget(event);
}

