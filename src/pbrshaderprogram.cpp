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
#include "pbrshaderprogram.h"

const QString &PbrShaderProgram::loadShaderSource(const QString &name)
{
    static std::map<QString, QString> s_shaderSources;
    auto findShader = s_shaderSources.find(name);
    if (findShader != s_shaderSources.end()) {
        return findShader->second;
    }
    QFile file(name);
    file.open(QFile::ReadOnly | QFile::Text);
    QTextStream stream(&file);
    auto insertResult = s_shaderSources.insert({name, stream.readAll()});
    return insertResult.first->second;
}

bool PbrShaderProgram::isCoreProfile()
{
    return m_isCoreProfile;
}

PbrShaderProgram::PbrShaderProgram(bool isCoreProfile)
{
    if (isCoreProfile) {
        this->addShaderFromSourceCode(QOpenGLShader::Vertex, loadShaderSource(":/shaders/pbr.core.vert"));
        this->addShaderFromSourceCode(QOpenGLShader::Fragment, loadShaderSource(":/shaders/pbr.core.frag"));
        m_isCoreProfile = true;
    } else {
        this->addShaderFromSourceCode(QOpenGLShader::Vertex, loadShaderSource(":/shaders/pbr.vert"));
        this->addShaderFromSourceCode(QOpenGLShader::Fragment, loadShaderSource(":/shaders/pbr.frag"));
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
    m_eyePosLoc = this->uniformLocation("eyePos");
    m_textureIdLoc = this->uniformLocation("textureId");
    m_textureEnabledLoc = this->uniformLocation("textureEnabled");
    m_normalMapIdLoc = this->uniformLocation("normalMapId");
    m_normalMapEnabledLoc = this->uniformLocation("normalMapEnabled");
    m_metalnessMapEnabledLoc = this->uniformLocation("metalnessMapEnabled");
    m_roughnessMapEnabledLoc = this->uniformLocation("roughnessMapEnabled");
    m_ambientOcclusionMapEnabledLoc = this->uniformLocation("ambientOcclusionMapEnabled");
    m_metalnessRoughnessAmbientOcclusionMapIdLoc = this->uniformLocation("metalnessRoughnessAmbientOcclusionMapId");
    m_mousePickEnabledLoc = this->uniformLocation("mousePickEnabled");
    m_mousePickTargetPositionLoc = this->uniformLocation("mousePickTargetPosition");
    m_mousePickRadiusLoc = this->uniformLocation("mousePickRadius");
    if (m_isCoreProfile) {
        m_environmentIrradianceMapIdLoc = this->uniformLocation("environmentIrradianceMapId");
        m_environmentIrradianceMapEnabledLoc = this->uniformLocation("environmentIrradianceMapEnabled");
        m_environmentSpecularMapIdLoc = this->uniformLocation("environmentSpecularMapId");
        m_environmentSpecularMapEnabledLoc = this->uniformLocation("environmentSpecularMapEnabled");
    }
}

int PbrShaderProgram::projectionMatrixLoc()
{
    return m_projectionMatrixLoc;
}

int PbrShaderProgram::modelMatrixLoc()
{
    return m_modelMatrixLoc;
}

int PbrShaderProgram::normalMatrixLoc()
{
    return m_normalMatrixLoc;
}

int PbrShaderProgram::viewMatrixLoc()
{
    return m_viewMatrixLoc;
}

int PbrShaderProgram::eyePosLoc()
{
    return m_eyePosLoc;
}

int PbrShaderProgram::textureEnabledLoc()
{
    return m_textureEnabledLoc;
}

int PbrShaderProgram::textureIdLoc()
{
    return m_textureIdLoc;
}

int PbrShaderProgram::normalMapEnabledLoc()
{
    return m_normalMapEnabledLoc;
}

int PbrShaderProgram::normalMapIdLoc()
{
    return m_normalMapIdLoc;
}

int PbrShaderProgram::metalnessMapEnabledLoc()
{
    return m_metalnessMapEnabledLoc;
}

int PbrShaderProgram::roughnessMapEnabledLoc()
{
    return m_roughnessMapEnabledLoc;
}

int PbrShaderProgram::ambientOcclusionMapEnabledLoc()
{
    return m_ambientOcclusionMapEnabledLoc;
}

int PbrShaderProgram::metalnessRoughnessAmbientOcclusionMapIdLoc()
{
    return m_metalnessRoughnessAmbientOcclusionMapIdLoc;
}

int PbrShaderProgram::mousePickEnabledLoc()
{
    return m_mousePickEnabledLoc;
}

int PbrShaderProgram::mousePickTargetPositionLoc()
{
    return m_mousePickTargetPositionLoc;
}

int PbrShaderProgram::mousePickRadiusLoc()
{
    return m_mousePickRadiusLoc;
}

int PbrShaderProgram::environmentIrradianceMapIdLoc()
{
    return m_environmentIrradianceMapIdLoc;
}

int PbrShaderProgram::environmentIrradianceMapEnabledLoc()
{
    return m_environmentIrradianceMapEnabledLoc;
}

int PbrShaderProgram::environmentSpecularMapIdLoc()
{
    return m_environmentSpecularMapIdLoc;
}

int PbrShaderProgram::environmentSpecularMapEnabledLoc()
{
    return m_environmentSpecularMapEnabledLoc;
}

void PbrShaderProgram::setProjectionMatrixValue(const QMatrix4x4 &value)
{
    if (value == m_projectionMatrixValue)
        return;
    m_projectionMatrixValue = value;
    setUniformValue(m_projectionMatrixLoc, m_projectionMatrixValue);
}

void PbrShaderProgram::setModelMatrixValue(const QMatrix4x4 &value)
{
    if (value == m_modelMatrixValue)
        return;
    m_modelMatrixValue = value;
    setUniformValue(m_modelMatrixLoc, m_modelMatrixValue);
}

void PbrShaderProgram::setNormalMatrixValue(const QMatrix3x3 &value)
{
    if (value == m_normalMatrixValue)
        return;
    m_normalMatrixValue = value;
    setUniformValue(m_normalMatrixLoc, m_normalMatrixValue);
}

void PbrShaderProgram::setViewMatrixValue(const QMatrix4x4 &value)
{
    if (value == m_viewMatrixValue)
        return;
    m_viewMatrixValue = value;
    setUniformValue(m_viewMatrixLoc, m_viewMatrixValue);
}

void PbrShaderProgram::setEyePosValue(const QVector3D &value)
{
    if (qFuzzyCompare(value, m_eyePosValue))
        return;
    m_eyePosValue = value;
    setUniformValue(m_eyePosLoc, m_eyePosValue);
}

void PbrShaderProgram::setTextureIdValue(int value)
{
    if (value == m_textureIdValue)
        return;
    m_textureIdValue = value;
    setUniformValue(m_textureIdLoc, m_textureIdValue);
}

void PbrShaderProgram::setTextureEnabledValue(int value)
{
    if (value == m_textureEnabledValue)
        return;
    m_textureEnabledValue = value;
    setUniformValue(m_textureEnabledLoc, m_textureEnabledValue);
}

void PbrShaderProgram::setNormalMapEnabledValue(int value)
{
    if (value == m_normalMapEnabledValue)
        return;
    m_normalMapEnabledValue = value;
    setUniformValue(m_normalMapEnabledLoc, m_normalMapEnabledValue);
}

void PbrShaderProgram::setNormalMapIdValue(int value)
{
    if (value == m_normalMapIdValue)
        return;
    m_normalMapIdValue = value;
    setUniformValue(m_normalMapIdLoc, m_normalMapIdValue);
}

void PbrShaderProgram::setMetalnessMapEnabledValue(int value)
{
    if (value == m_metalnessMapEnabledValue)
        return;
    m_metalnessMapEnabledValue = value;
    setUniformValue(m_metalnessMapEnabledLoc, m_metalnessMapEnabledValue);
}

void PbrShaderProgram::setRoughnessMapEnabledValue(int value)
{
    if (value == m_roughnessMapEnabledValue)
        return;
    m_roughnessMapEnabledValue = value;
    setUniformValue(m_roughnessMapEnabledLoc, m_roughnessMapEnabledValue);
}

void PbrShaderProgram::setAmbientOcclusionMapEnabledValue(int value)
{
    if (value == m_ambientOcclusionMapEnabledValue)
        return;
    m_ambientOcclusionMapEnabledValue = value;
    setUniformValue(m_ambientOcclusionMapEnabledLoc, m_ambientOcclusionMapEnabledValue);
}

void PbrShaderProgram::setMetalnessRoughnessAmbientOcclusionMapIdValue(int value)
{
    if (value == m_metalnessRoughnessAmbientOcclusionMapIdValue)
        return;
    m_metalnessRoughnessAmbientOcclusionMapIdValue = value;
    setUniformValue(m_metalnessRoughnessAmbientOcclusionMapIdLoc, m_metalnessRoughnessAmbientOcclusionMapIdValue);
}

void PbrShaderProgram::setMousePickEnabledValue(int value)
{
    if (value == m_mousePickEnabledValue)
        return;
    m_mousePickEnabledValue = value;
    setUniformValue(m_mousePickEnabledLoc, m_mousePickEnabledValue);
}

void PbrShaderProgram::setMousePickTargetPositionValue(const QVector3D &value)
{
    if (qFuzzyCompare(value, m_mousePickTargetPositionValue))
        return;
    m_mousePickTargetPositionValue = value;
    setUniformValue(m_mousePickTargetPositionLoc, m_mousePickTargetPositionValue);
}

void PbrShaderProgram::setMousePickRadiusValue(float value)
{
    if (qFuzzyCompare(value, m_mousePickRadiusValue))
        return;
    m_mousePickRadiusValue = value;
    setUniformValue(m_mousePickRadiusLoc, m_mousePickRadiusValue);
}

void PbrShaderProgram::setEnvironmentIrradianceMapIdValue(int value)
{
    if (value == m_environmentIrradianceMapIdValue)
        return;
    m_environmentIrradianceMapIdValue = value;
    setUniformValue(m_environmentIrradianceMapIdLoc, m_environmentIrradianceMapIdValue);
}

void PbrShaderProgram::setEnvironmentIrradianceMapEnabledValue(int value)
{
    if (value == m_environmentIrradianceMapEnabledValue)
        return;
    m_environmentIrradianceMapEnabledValue = value;
    setUniformValue(m_environmentIrradianceMapEnabledLoc, m_environmentIrradianceMapEnabledValue);
}

void PbrShaderProgram::setEnvironmentSpecularMapIdValue(int value)
{
    if (value == m_environmentSpecularMapIdValue)
        return;
    m_environmentSpecularMapIdValue = value;
    setUniformValue(m_environmentSpecularMapIdLoc, m_environmentSpecularMapIdValue);
}

void PbrShaderProgram::setEnvironmentSpecularMapEnabledValue(int value)
{
    if (value == m_environmentSpecularMapEnabledValue)
        return;
    m_environmentSpecularMapEnabledValue = value;
    setUniformValue(m_environmentSpecularMapEnabledLoc, m_environmentSpecularMapEnabledValue);
}