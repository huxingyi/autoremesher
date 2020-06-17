include (ACGCommon)

include_directories (
  ../..
  ${CMAKE_CURRENT_SOURCE_DIR}
)

# source code directories
set (directories 
  . 
  Geometry 
  IO 
  IO/exporter 
  IO/importer 
  IO/reader 
  IO/writer
  Mesh 
  Mesh/gen 
  System 
  Utils
)

# collect all header and source files
acg_append_files (headers "*.hh" ${directories})
acg_append_files (sources "*.cc" ${directories})

# Disable Library installation when not building OpenMesh on its own but as part of another project!
if ( NOT ${PROJECT_NAME} MATCHES "OpenMesh")
  set(ACG_NO_LIBRARY_INSTALL true)
endif()


if (WIN32)

  if ( OPENMESH_BUILD_SHARED )
    add_definitions( -DOPENMESHDLL -DBUILDOPENMESHDLL)
    acg_add_library (OpenMeshCore SHARED ${sources} ${headers})
  else()
    # OpenMesh has no dll exports so we have to build a static library on windows
    acg_add_library (OpenMeshCore STATIC ${sources} ${headers})
  endif()

else ()
  acg_add_library (OpenMeshCore SHAREDANDSTATIC ${sources} ${headers})
  set_target_properties (OpenMeshCore PROPERTIES VERSION ${OPENMESH_VERSION_MAJOR}.${OPENMESH_VERSION_MINOR}
                                               SOVERSION ${OPENMESH_VERSION_MAJOR}.${OPENMESH_VERSION_MINOR} )

endif ()

# Add core as dependency before fixbundle 
if ( (${PROJECT_NAME} MATCHES "OpenMesh") AND BUILD_APPS )

  if ( WIN32 )
    if ( NOT "${CMAKE_GENERATOR}" MATCHES "MinGW Makefiles" )
      add_dependencies (fixbundle OpenMeshCore)
    endif()
  endif()

  # Add core as dependency before fixbundle 
  if ( APPLE )
    # let bundle generation depend on targets
    add_dependencies (fixbundle OpenMeshCore)
  endif ()

endif()

# if we build debug and release in the same dir, we want to install both!
if ( ${PROJECT_NAME} MATCHES "OpenMesh")
  if ( WIN32 )
    FILE(GLOB files_install_libs "${CMAKE_BINARY_DIR}/Build/lib/*.lib" )
    FILE(GLOB files_install_dlls "${CMAKE_BINARY_DIR}/Build/*.dll" )
    INSTALL(FILES ${files_install_libs} DESTINATION lib )
    INSTALL(FILES ${files_install_dlls} DESTINATION . )
  endif()
endif()


# Install Header Files (Apple)
if ( NOT ACG_PROJECT_MACOS_BUNDLE AND APPLE )
 FILE(GLOB files_install_Geometry    "${CMAKE_CURRENT_SOURCE_DIR}/Geometry/*.hh" )
 FILE(GLOB files_install_IO          "${CMAKE_CURRENT_SOURCE_DIR}/IO/*.hh" "${CMAKE_CURRENT_SOURCE_DIR}/IO/*.inl"  )
 FILE(GLOB files_install_IO_importer "${CMAKE_CURRENT_SOURCE_DIR}/IO/importer/*.hh" )
 FILE(GLOB files_install_IO_exporter "${CMAKE_CURRENT_SOURCE_DIR}/IO/exporter/*.hh" )
 FILE(GLOB files_install_IO_reader   "${CMAKE_CURRENT_SOURCE_DIR}/IO/reader/*.hh" )
 FILE(GLOB files_install_IO_writer   "${CMAKE_CURRENT_SOURCE_DIR}/IO/writer/*.hh" )
 FILE(GLOB files_install_Mesh        "${CMAKE_CURRENT_SOURCE_DIR}/Mesh/*.hh" )
 FILE(GLOB files_install_Mesh_Gen    "${CMAKE_CURRENT_SOURCE_DIR}/Mesh/gen/*.hh" )
 FILE(GLOB files_install_System      "${CMAKE_CURRENT_SOURCE_DIR}/System/*.hh" "${CMAKE_CURRENT_SOURCE_DIR}/System/config.h" )
 FILE(GLOB files_install_Utils       "${CMAKE_CURRENT_SOURCE_DIR}/Utils/*.hh" )
 INSTALL(FILES ${files_install_Geometry}    DESTINATION include/OpenMesh/Core/Geometry )
 INSTALL(FILES ${files_install_IO}          DESTINATION include/OpenMesh/Core/IO )
 INSTALL(FILES ${files_install_IO_importer} DESTINATION include/OpenMesh/Core/IO/importer )
 INSTALL(FILES ${files_install_IO_exporter} DESTINATION include/OpenMesh/Core/IO/exporter )
 INSTALL(FILES ${files_install_IO_reader}   DESTINATION include/OpenMesh/Core/IO/reader )
 INSTALL(FILES ${files_install_IO_writer}   DESTINATION include/OpenMesh/Core/IO/writer )
 INSTALL(FILES ${files_install_Mesh}        DESTINATION include/OpenMesh/Core/Mesh )
 INSTALL(FILES ${files_install_Mesh_Gen}    DESTINATION include/OpenMesh/Core/Mesh/gen )
 INSTALL(FILES ${files_install_System}      DESTINATION include/OpenMesh/Core/System )
 INSTALL(FILES ${files_install_Utils}       DESTINATION include/OpenMesh/Core/Utils )
endif()


# Only install if the project name matches OpenMesh.
if (NOT APPLE AND ${PROJECT_NAME} MATCHES "OpenMesh")

# Install Header Files)
install(DIRECTORY . 
	DESTINATION include/OpenMesh/Core
        FILES_MATCHING 
	PATTERN "*.hh"
        PATTERN "CVS" EXCLUDE
        PATTERN ".svn" EXCLUDE
        PATTERN "tmp" EXCLUDE
	PATTERN "Templates" EXCLUDE
        PATTERN "Debian*" EXCLUDE)

#install the config file
install(FILES System/config.h DESTINATION include/OpenMesh/Core/System)

#install inlined Files from IO
install(DIRECTORY IO/ 
	DESTINATION include/OpenMesh/Core/IO
	FILES_MATCHING
	PATTERN "*.inl"         
	PATTERN "CVS" EXCLUDE
        PATTERN ".svn" EXCLUDE
        PATTERN "reader" EXCLUDE
        PATTERN "writer" EXCLUDE
        PATTERN "importer" EXCLUDE
        PATTERN "exporter" EXCLUDE
        PATTERN "tmp" EXCLUDE
        PATTERN "Debian*" EXCLUDE )    
endif ()

target_include_directories(OpenMeshCore PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>
  $<INSTALL_INTERFACE:include>)

install(TARGETS OpenMeshCore EXPORT OpenMeshConfig
  ARCHIVE DESTINATION ${ACG_PROJECT_LIBDIR}
  LIBRARY DESTINATION ${ACG_PROJECT_LIBDIR}      
  RUNTIME DESTINATION ${ACG_PROJECT_BINDIR})

