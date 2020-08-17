#
# Searches for OpenGL package
# Sets variable OPENGL_LIBRARIES to the list of GL and GLU libraries
#

set(OpenGL_GL_PREFERENCE GLVND)

find_package(OpenGL)

if(OPENGL_FOUND)
    include_directories(${OPENGL_INCLUDE_DIR})
else()
    if(WIN32)
        list(APPEND OPENGL_LIBRARIES opengl32)
    else()
        list(APPEND OPENGL_LIBRARIES GL)
    endif()
endif()

if(OPENGL_GLU_FOUND)
    include_directories(${GLU_INCLUDE_DIR})
else()
    if(WIN32)
        list(APPEND OPENGL_LIBRARIES glu32)
    else()
        list(APPEND OPENGL_LIBRARIES GLU)
    endif()
endif()

if(WIN32)
    # freeglut uses some timer functions defined in winmm
    list(APPEND OPENGL_LIBRARIES winmm)

    # Vorpaline's drag and drop support in freeglut requires shell32   
    list(APPEND OPENGL_LIBRARIES shell32)   
elseif(APPLE)
    # Nothing special to use OpenGL and Vorpaline on APPLE
else()
    find_package(X11)
    list(APPEND OPENGL_LIBRARIES X11)
    
    # GLAD needs the dynamic linker to query symbols in OpenGL lib
    list(APPEND OPENGL_LIBRARIES dl)
    
    # GLFW needs these ones
    list(APPEND OPENGL_LIBRARIES Xxf86vm Xrandr Xcursor Xinerama Xi)
endif()

# Emscripten has buitin OpenGL and GLFW support.
if(GEOGRAM_WITH_EMSCRIPTEN)
  set(OPENGL_LIBRARIES "")
  set(GLFW_LIBRARIES "")
else()
  set(GLFW_LIBRARIES glfw ${OPENGL_LIBRARIES})
endif()
