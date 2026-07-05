#ifndef AUTO_REMESHER_MONOCHROME_OPENGL_PROGRAM_H
#define AUTO_REMESHER_MONOCHROME_OPENGL_PROGRAM_H
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <map>

class MonochromeOpenGLProgram : public QOpenGLShaderProgram {
public:
    void load(bool isCoreProfile = false);
    int getUniformLocationByName(const std::string &name);
    bool isCoreProfile() const;
private:
    bool m_isLoaded = false;
    bool m_isCoreProfile = false;
    std::map<std::string, int> m_uniformLocationMap;
};

#endif