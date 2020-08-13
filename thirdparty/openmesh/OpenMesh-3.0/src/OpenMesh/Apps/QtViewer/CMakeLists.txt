include (ACGCommon)

include_directories (
  ../../..
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${GLUT_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)

set (targetName QtViewer)

# source code directories
set (directories 
  .
)

# collect all header and source files
acg_append_files (headers "*.hh" ${directories})
acg_append_files (sources "*.cc" ${directories})
acg_append_files (ui "*.ui" ${directories})

# remove template cc files from source file list
acg_drop_templates (sources)

# genereate uic and moc targets
acg_qt4_autouic (uic_targets ${ui})
acg_qt4_automoc (moc_targets ${headers})


if (WIN32)
  acg_add_executable (${targetName} WIN32 ${uic_targets} ${sources} ${headers} ${moc_targets})
  # link to qtmain library to get WinMain function for a non terminal app
  target_link_libraries (${targetName} ${QT_QTMAIN_LIBRARY})
else ()
  acg_add_executable (${targetName} ${uic_targets} ${sources} ${headers} ${moc_targets})
endif ()

target_link_libraries (${targetName}
  OpenMeshCore
  OpenMeshTools
  ${QT_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${GLUT_LIBRARIES}
)

