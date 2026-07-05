#include "monochromeopenglprogram.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

static const QString &loadShaderSource(const QString &name)
{
    static std::map<QString, QString> s_shaderSources;
    auto findShader = s_shaderSources.find(name);
    if (findShader != s_shaderSources.end())
        return findShader->second;
    QFile file(name);
    file.open(QFile::ReadOnly | QFile::Text);
    QTextStream stream(&file);
    auto insertResult = s_shaderSources.insert({ name, stream.readAll() });
    return insertResult.first->second;
}

bool MonochromeOpenGLProgram::isCoreProfile() const
{
    return m_isCoreProfile;
}

void MonochromeOpenGLProgram::load(bool isCoreProfile)
{
    if (m_isLoaded)
        return;

    m_isCoreProfile = isCoreProfile;
    if (m_isCoreProfile) {
        addShaderFromSourceCode(QOpenGLShader::Vertex,
            loadShaderSource(":/shaders/monochrome_core.vert"));
        addShaderFromSourceCode(QOpenGLShader::Geometry,
            loadShaderSource(":/shaders/monochrome_core.geom"));
        addShaderFromSourceCode(QOpenGLShader::Fragment,
            loadShaderSource(":/shaders/monochrome_core.frag"));
    } else {
        addShaderFromSourceCode(QOpenGLShader::Vertex,
            loadShaderSource(":/shaders/monochrome.vert"));
        addShaderFromSourceCode(QOpenGLShader::Fragment,
            loadShaderSource(":/shaders/monochrome.frag"));
    }
    bindAttributeLocation("vertex", 0);
    bindAttributeLocation("color", 1);
    bindAttributeLocation("alpha", 2);
    link();
    bind();
    m_isLoaded = true;
}

int MonochromeOpenGLProgram::getUniformLocationByName(const std::string &name)
{
    auto findLocation = m_uniformLocationMap.find(name);
    if (findLocation != m_uniformLocationMap.end())
        return findLocation->second;
    int location = uniformLocation(name.c_str());
    m_uniformLocationMap.insert({ name, location });
    return location;
}