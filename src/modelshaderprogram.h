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
#ifndef AUTO_REMESHER_MODEL_SHADER_PROGRAM_H
#define AUTO_REMESHER_MODEL_SHADER_PROGRAM_H
#include <QOpenGLShaderProgram>
#include <QString>

class ModelShaderProgram : public QOpenGLShaderProgram {
public:
    ModelShaderProgram(bool isCoreProfile);
    int projectionMatrixLoc();
    int modelMatrixLoc();
    int normalMatrixLoc();
    int viewMatrixLoc();
    int eyePositionLoc();
    int textureIdLoc();
    int textureEnabledLoc();
    int normalMapEnabledLoc();
    int normalMapIdLoc();
    int metalnessMapEnabledLoc();
    int roughnessMapEnabledLoc();
    int aoMapEnabledLoc();
    int metalnessRoughnessAoMapIdLoc();
    int mousePickEnabledLoc();
    int mousePickTargetPositionLoc();
    int mousePickRadiusLoc();
    bool isCoreProfile();
    static const QString& loadShaderSource(const QString& name);
    void setProjectionMatrixValue(const QMatrix4x4& value);
    void setModelMatrixValue(const QMatrix4x4& value);
    void setNormalMatrixValue(const QMatrix3x3& value);
    void setViewMatrixValue(const QMatrix4x4& value);
    void setEyePositionValue(const QVector3D& value);
    void setTextureIdValue(int value);
    void setTextureEnabledValue(int value);
    void setNormalMapEnabledValue(int value);
    void setNormalMapIdValue(int value);
    void setMetalnessMapEnabledValue(int value);
    void setRoughnessMapEnabledValue(int value);
    void setAoMapEnabledValue(int value);
    void setMetalnessRoughnessAoMapIdValue(int value);
    void setMousePickEnabledValue(int value);
    void setMousePickTargetPositionValue(const QVector3D& value);
    void setMousePickRadiusValue(float value);

private:
    bool m_isCoreProfile = false;
    int m_projectionMatrixLoc = 0;
    int m_modelMatrixLoc = 0;
    int m_normalMatrixLoc = 0;
    int m_viewMatrixLoc = 0;
    int m_eyePositionLoc = 0;
    int m_textureIdLoc = 0;
    int m_textureEnabledLoc = 0;
    int m_normalMapEnabledLoc = 0;
    int m_normalMapIdLoc = 0;
    int m_metalnessMapEnabledLoc = 0;
    int m_roughnessMapEnabledLoc = 0;
    int m_aoMapEnabledLoc = 0;
    int m_metalnessRoughnessAoMapIdLoc = 0;
    int m_mousePickEnabledLoc = 0;
    int m_mousePickTargetPositionLoc = 0;
    int m_mousePickRadiusLoc = 0;
    QMatrix4x4 m_projectionMatrixValue;
    QMatrix4x4 m_modelMatrixValue;
    QMatrix3x3 m_normalMatrixValue;
    QMatrix4x4 m_viewMatrixValue;
    QVector3D m_eyePositionValue;
    int m_textureIdValue = -1;
    int m_textureEnabledValue = -1;
    int m_normalMapEnabledValue = -1;
    int m_normalMapIdValue = -1;
    int m_metalnessMapEnabledValue = -1;
    int m_roughnessMapEnabledValue = -1;
    int m_aoMapEnabledValue = -1;
    int m_metalnessRoughnessAoMapIdValue = -1;
    int m_mousePickEnabledValue = -1;
    QVector3D m_mousePickTargetPositionValue;
    float m_mousePickRadiusValue = 0.0;
};

#endif