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
#include <QFile>
#include <map>
#include "modelshaderprogram.h"

const QString &ModelShaderProgram::loadShaderSource(const QString &name)
{
    static std::map<QString, QString> s_shaderSources;
    auto findShader = s_shaderSources.find(name);
    if (findShader != s_shaderSources.end())
        return findShader->second;
    QFile file(name);
    file.open(QFile::ReadOnly | QFile::Text);
    QTextStream stream(&file);
    auto insertResult = s_shaderSources.insert({name, stream.readAll()});
    return insertResult.first->second;
}

bool ModelShaderProgram::isCoreProfile()
{
    return m_isCoreProfile;
}

ModelShaderProgram::ModelShaderProgram(bool isCoreProfile)
{
    if (isCoreProfile) {
        this->addShaderFromSourceCode(QOpenGLShader::Vertex, loadShaderSource(":/shaders/model.core.vert"));
        this->addShaderFromSourceCode(QOpenGLShader::Fragment, loadShaderSource(":/shaders/model.core.frag"));
        m_isCoreProfile = true;
    } else {
        this->addShaderFromSourceCode(QOpenGLShader::Vertex, loadShaderSource(":/shaders/model.vert"));
        this->addShaderFromSourceCode(QOpenGLShader::Fragment, loadShaderSource(":/shaders/model.frag"));
    }
    this->bindAttributeLocation("vertex", 0);
    this->bindAttributeLocation("normal", 1);
    this->bindAttributeLocation("color", 2);
    this->bindAttributeLocation("texCoord", 3);
    this->bindAttributeLocation("metalness", 4);
    this->bindAttributeLocation("roughness", 5);
    this->bindAttributeLocation("tangent", 6);
    this->bindAttributeLocation("alpha", 7);
    this->link();

    this->bind();
    m_projectionMatrixLoc = this->uniformLocation("projectionMatrix");
    m_modelMatrixLoc = this->uniformLocation("modelMatrix");
    m_normalMatrixLoc = this->uniformLocation("normalMatrix");
    m_viewMatrixLoc = this->uniformLocation("viewMatrix");
    m_eyePositionLoc = this->uniformLocation("eyePosition");
    m_textureIdLoc = this->uniformLocation("textureId");
    m_textureEnabledLoc = this->uniformLocation("textureEnabled");
    m_normalMapIdLoc = this->uniformLocation("normalMapId");
    m_normalMapEnabledLoc = this->uniformLocation("normalMapEnabled");
    m_metalnessMapEnabledLoc = this->uniformLocation("metalnessMapEnabled");
    m_roughnessMapEnabledLoc = this->uniformLocation("roughnessMapEnabled");
    m_aoMapEnabledLoc = this->uniformLocation("aoMapEnabled");
    m_metalnessRoughnessAoMapIdLoc = this->uniformLocation("metalnessRoughnessAoMapId");
    m_mousePickEnabledLoc = this->uniformLocation("mousePickEnabled");
    m_mousePickTargetPositionLoc = this->uniformLocation("mousePickTargetPosition");
    m_mousePickRadiusLoc = this->uniformLocation("mousePickRadius");
}

int ModelShaderProgram::projectionMatrixLoc()
{
    return m_projectionMatrixLoc;
}

int ModelShaderProgram::modelMatrixLoc()
{
    return m_modelMatrixLoc;
}

int ModelShaderProgram::normalMatrixLoc()
{
    return m_normalMatrixLoc;
}

int ModelShaderProgram::viewMatrixLoc()
{
    return m_viewMatrixLoc;
}

int ModelShaderProgram::eyePositionLoc()
{
    return m_eyePositionLoc;
}

int ModelShaderProgram::textureEnabledLoc()
{
    return m_textureEnabledLoc;
}

int ModelShaderProgram::textureIdLoc()
{
    return m_textureIdLoc;
}

int ModelShaderProgram::normalMapEnabledLoc()
{
    return m_normalMapEnabledLoc;
}

int ModelShaderProgram::normalMapIdLoc()
{
    return m_normalMapIdLoc;
}

int ModelShaderProgram::metalnessMapEnabledLoc()
{
    return m_metalnessMapEnabledLoc;
}

int ModelShaderProgram::roughnessMapEnabledLoc()
{
    return m_roughnessMapEnabledLoc;
}

int ModelShaderProgram::aoMapEnabledLoc()
{
    return m_aoMapEnabledLoc;
}

int ModelShaderProgram::metalnessRoughnessAoMapIdLoc()
{
    return m_metalnessRoughnessAoMapIdLoc;
}

int ModelShaderProgram::mousePickEnabledLoc()
{
    return m_mousePickEnabledLoc;
}

int ModelShaderProgram::mousePickTargetPositionLoc()
{
    return m_mousePickTargetPositionLoc;
}

int ModelShaderProgram::mousePickRadiusLoc()
{
    return m_mousePickRadiusLoc;
}

void ModelShaderProgram::setProjectionMatrixValue(const QMatrix4x4 &value)
{
    if (value == m_projectionMatrixValue)
        return;
    m_projectionMatrixValue = value;
    setUniformValue(m_projectionMatrixLoc, m_projectionMatrixValue);
}

void ModelShaderProgram::setModelMatrixValue(const QMatrix4x4 &value)
{
    if (value == m_modelMatrixValue)
        return;
    m_modelMatrixValue = value;
    setUniformValue(m_modelMatrixLoc, m_modelMatrixValue);
}

void ModelShaderProgram::setNormalMatrixValue(const QMatrix3x3 &value)
{
    if (value == m_normalMatrixValue)
        return;
    m_normalMatrixValue = value;
    setUniformValue(m_normalMatrixLoc, m_normalMatrixValue);
}

void ModelShaderProgram::setViewMatrixValue(const QMatrix4x4 &value)
{
    if (value == m_viewMatrixValue)
        return;
    m_viewMatrixValue = value;
    setUniformValue(m_viewMatrixLoc, m_viewMatrixValue);
}

void ModelShaderProgram::setEyePositionValue(const QVector3D &value)
{
    if (qFuzzyCompare(value, m_eyePositionValue))
        return;
    m_eyePositionValue = value;
    setUniformValue(m_eyePositionLoc, m_eyePositionValue);
}

void ModelShaderProgram::setTextureIdValue(int value)
{
    if (value == m_textureIdValue)
        return;
    m_textureIdValue = value;
    setUniformValue(m_textureIdLoc, m_textureIdValue);
}

void ModelShaderProgram::setTextureEnabledValue(int value)
{
    if (value == m_textureEnabledValue)
        return;
    m_textureEnabledValue = value;
    setUniformValue(m_textureEnabledLoc, m_textureEnabledValue);
}

void ModelShaderProgram::setNormalMapEnabledValue(int value)
{
    if (value == m_normalMapEnabledValue)
        return;
    m_normalMapEnabledValue = value;
    setUniformValue(m_normalMapEnabledLoc, m_normalMapEnabledValue);
}

void ModelShaderProgram::setNormalMapIdValue(int value)
{
    if (value == m_normalMapIdValue)
        return;
    m_normalMapIdValue = value;
    setUniformValue(m_normalMapIdLoc, m_normalMapIdValue);
}

void ModelShaderProgram::setMetalnessMapEnabledValue(int value)
{
    if (value == m_metalnessMapEnabledValue)
        return;
    m_metalnessMapEnabledValue = value;
    setUniformValue(m_metalnessMapEnabledLoc, m_metalnessMapEnabledValue);
}

void ModelShaderProgram::setRoughnessMapEnabledValue(int value)
{
    if (value == m_roughnessMapEnabledValue)
        return;
    m_roughnessMapEnabledValue = value;
    setUniformValue(m_roughnessMapEnabledLoc, m_roughnessMapEnabledValue);
}

void ModelShaderProgram::setAoMapEnabledValue(int value)
{
    if (value == m_aoMapEnabledValue)
        return;
    m_aoMapEnabledValue = value;
    setUniformValue(m_aoMapEnabledLoc, m_aoMapEnabledValue);
}

void ModelShaderProgram::setMetalnessRoughnessAoMapIdValue(int value)
{
    if (value == m_metalnessRoughnessAoMapIdValue)
        return;
    m_metalnessRoughnessAoMapIdValue = value;
    setUniformValue(m_metalnessRoughnessAoMapIdLoc, m_metalnessRoughnessAoMapIdValue);
}

void ModelShaderProgram::setMousePickEnabledValue(int value)
{
    if (value == m_mousePickEnabledValue)
        return;
    m_mousePickEnabledValue = value;
    setUniformValue(m_mousePickEnabledLoc, m_mousePickEnabledValue);
}

void ModelShaderProgram::setMousePickTargetPositionValue(const QVector3D &value)
{
    if (qFuzzyCompare(value, m_mousePickTargetPositionValue))
        return;
    m_mousePickTargetPositionValue = value;
    setUniformValue(m_mousePickTargetPositionLoc, m_mousePickTargetPositionValue);
}

void ModelShaderProgram::setMousePickRadiusValue(float value)
{
    if (qFuzzyCompare(value, m_mousePickRadiusValue))
        return;
    m_mousePickRadiusValue = value;
    setUniformValue(m_mousePickRadiusLoc, m_mousePickRadiusValue);
}