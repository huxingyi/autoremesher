if (EXISTS ${CMAKE_SOURCE_DIR}/${CMAKE_PROJECT_NAME}.cmake)
  include (${CMAKE_SOURCE_DIR}/${CMAKE_PROJECT_NAME}.cmake)
endif ()

# prevent build in source directory
  if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
      message (SEND_ERROR "Building in the source directory is not supported.")
      message (FATAL_ERROR "Please remove the created \"CMakeCache.txt\" file, the \"CMakeFiles\" directory and create a build directory and call \"${CMAKE_COMMAND} <path to the sources>\".")
  endif ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")

# allow only Debug and Release builds
set (CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "")
mark_as_advanced (CMAKE_CONFIGURATION_TYPES)

# set Debus as default build target
if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Debug CACHE STRING
      "Choose the type of build, options are: Debug, Release."
      FORCE)
endif ()

# create our output directroy
if (NOT EXISTS ${CMAKE_BINARY_DIR}/Build)
  file (MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Build)
endif ()

# read version from file
macro (acg_get_version)
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${ARGN}/VERSION")
      file (READ "${CMAKE_CURRENT_SOURCE_DIR}/${ARGN}/VERSION" _file)
    else ()
      file (READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" _file)
    endif ()

    string (
        REGEX REPLACE
        "^.*ID=([^\n]*).*$" "\\1"
        _id ${_file}
    )
    string (
        REGEX REPLACE
        "^.*VERSION=([^\n]*).*$" "\\1"
        _version ${_file}
    )
    string (
        REGEX REPLACE
        "^.*MAJOR=([^\n]*).*$" "\\1"
        _major ${_file}
    )
    string (
        REGEX REPLACE
        "^.*MINOR=([^\n]*).*$" "\\1"
        _minor ${_file}
    )
    string (
        REGEX REPLACE
        "^.*PATCH=([^\n]*).*$" "\\1"
        _patch ${_file}
    )

    set (${_id}_VERSION ${_version})
    set (${_id}_VERSION_MAJOR ${_major})
    set (${_id}_VERSION_MINOR ${_minor})
    set (${_id}_VERSION_PATCH ${_patch})
endmacro ()


# set directory structures for the different platforms
if (CMAKE_HOST_SYSTEM_NAME MATCHES Windows)
  set (ACG_PROJECT_DATADIR ".")
  set (ACG_PROJECT_LIBDIR "lib")
  set (ACG_PROJECT_BINDIR ".")
  set (ACG_PROJECT_PLUGINDIR "Plugins")
  if (NOT EXISTS ${CMAKE_BINARY_DIR}/Build/${ACG_PROJECT_LIBDIR})
    file (MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Build/${ACG_PROJECT_LIBDIR})
  endif ()
elseif (APPLE)
  set (ACG_PROJECT_DATADIR "share/${CMAKE_PROJECT_NAME}")
  set (ACG_PROJECT_LIBDIR "lib${LIB_SUFFIX}")
  set (CMAKE_LIBRARY_OUTPUT_DIR "${CMAKE_BINARY_DIR}/Build/${ACG_PROJECT_LIBDIR}")
  set (ACG_PROJECT_PLUGINDIR "${ACG_PROJECT_LIBDIR}/plugins")
  set (ACG_PROJECT_BINDIR "bin")
else ()
  set (ACG_PROJECT_DATADIR "share/${CMAKE_PROJECT_NAME}")
  set (ACG_PROJECT_LIBDIR "lib${LIB_SUFFIX}")
  set (ACG_PROJECT_PLUGINDIR "${ACG_PROJECT_LIBDIR}/plugins")
  set (ACG_PROJECT_BINDIR "bin")
endif ()

# allow a project to modify the directories
if (COMMAND acg_modify_project_dirs)
  acg_modify_project_dirs ()
endif ()

if (NOT EXISTS ${CMAKE_BINARY_DIR}/Build/${ACG_PROJECT_DATADIR})
 file (MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Build/${ACG_PROJECT_DATADIR})
endif ()


# sets default build properties
macro (acg_set_target_props target)
  if (WIN32)
    set_target_properties (
      ${target} PROPERTIES
      BUILD_WITH_INSTALL_RPATH 1
      SKIP_BUILD_RPATH 0
    )
  elseif (APPLE AND NOT ACG_PROJECT_MACOS_BUNDLE)
    # save rpath
    set_target_properties (
      ${target} PROPERTIES
      INSTALL_RPATH "@executable_path/../${ACG_PROJECT_LIBDIR}"
      MACOSX_RPATH 1
      #BUILD_WITH_INSTALL_RPATH 1
      SKIP_BUILD_RPATH 0
    )  
  elseif (NOT APPLE)

    set_target_properties (
      ${target} PROPERTIES
      INSTALL_RPATH "$ORIGIN/../${ACG_PROJECT_LIBDIR}"
      BUILD_WITH_INSTALL_RPATH 1
      SKIP_BUILD_RPATH 0
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Build/${ACG_PROJECT_BINDIR}"
      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Build/${ACG_PROJECT_LIBDIR}"
    )
  endif ()
endmacro ()

include (AddFileDependencies)
include (ACGCompiler)

# define INCLUDE_TEMPLATES for everything we build
add_definitions (-DINCLUDE_TEMPLATES)

# unsets the given variable
macro (acg_unset var)
    set (${var} "" CACHE INTERNAL "")
endmacro ()

# sets the given variable
macro (acg_set var value)
    set (${var} ${value} CACHE INTERNAL "")
endmacro ()

# append all files with extension "ext" in the "dirs" directories to "ret"
# excludes all files starting with a '.' (dot)
macro (acg_append_files ret ext)
  foreach (_dir ${ARGN})
    file (GLOB _files "${_dir}/${ext}")
    foreach (_file ${_files})
      get_filename_component (_filename ${_file} NAME)
      if (_filename MATCHES "^[.]")
	list (REMOVE_ITEM _files ${_file})
      endif ()
    endforeach ()
    list (APPEND ${ret} ${_files})
  endforeach ()
endmacro ()

# get all files in directory, but ignore svn
macro (acg_get_files_in_dir ret dir)
  file (GLOB_RECURSE __files RELATIVE "${dir}" "${dir}/*")
  foreach (_file ${__files})
    if ( (NOT _file MATCHES ".*svn.*") AND (NOT _file MATCHES ".DS_Store") )
      list (APPEND ${ret} "${_file}")
    endif ()
  endforeach ()
endmacro ()

# copy the whole directory without svn files
function (acg_copy_after_build target src dst)
  acg_unset (_files)
  acg_get_files_in_dir (_files ${src})
  foreach (_file ${_files})
    add_custom_command(TARGET ${target} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${src}/${_file}" "${dst}/${_file}"
    )
  endforeach ()
endfunction ()

# extended version of add_executable that also copies output to out Build directory
function (acg_add_executable _target)
  add_executable (${_target} ${ARGN})

  # set common target properties defined in common.cmake
  acg_set_target_props (${_target})
  
  if (WIN32 OR (APPLE AND NOT ACG_PROJECT_MACOS_BUNDLE))
    add_custom_command (TARGET ${_target} POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E
                        copy_if_different
                          $<TARGET_FILE:${_target}>
                          ${CMAKE_BINARY_DIR}/Build/${ACG_PROJECT_BINDIR}/$<TARGET_FILE_NAME:${_target}>)
  endif (WIN32 OR (APPLE AND NOT ACG_PROJECT_MACOS_BUNDLE))
  
  if (NOT ACG_PROJECT_MACOS_BUNDLE OR NOT APPLE)
    install (TARGETS ${_target} DESTINATION ${ACG_PROJECT_BINDIR})
  endif ()
endfunction ()

# extended version of add_library that also copies output to out Build directory
function (acg_add_library _target _libtype)

  if (${_libtype} STREQUAL SHAREDANDSTATIC)
    set (_type SHARED)
    if (NOT WIN32 OR MINGW)
      set (_and_static 1)
    else ()
      set (_and_static 0)
    endif ()    
  else ()
    set (_type ${_libtype})
    set (_and_static 0)
  endif ()

  add_library (${_target} ${_type} ${ARGN} )

  # set common target properties defined in common.cmake
  acg_set_target_props (${_target})

  if (_and_static)
    add_library (${_target}Static STATIC ${ARGN})

    # set common target properties defined in common.cmake
    acg_set_target_props (${_target}Static)

    set_target_properties(${_target}Static PROPERTIES OUTPUT_NAME ${_target})
    
    if (NOT APPLE)
      set_target_properties (${_target}Static PROPERTIES 
                             LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
                            )
    endif ()
  endif ()

  if ( (WIN32 AND MSVC) OR (APPLE AND NOT ACG_PROJECT_MACOS_BUNDLE))
    if (${_type} STREQUAL SHARED)
      add_custom_command (TARGET ${_target} POST_BUILD
                          COMMAND ${CMAKE_COMMAND} -E
                          copy_if_different
                            $<TARGET_FILE:${_target}>
                            ${CMAKE_BINARY_DIR}/Build/${ACG_PROJECT_LIBDIR}/$<TARGET_FILE_NAME:${_target}>)
    add_custom_command (TARGET ${_target} POST_BUILD
                          COMMAND ${CMAKE_COMMAND} -E
                          copy_if_different
                            $<TARGET_LINKER_FILE:${_target}>
                            ${CMAKE_BINARY_DIR}/Build/${ACG_PROJECT_LIBDIR}/$<TARGET_LINKER_FILE_NAME:${_target}>)
    elseif (${_type} STREQUAL MODULE)
      if (NOT EXISTS ${CMAKE_BINARY_DIR}/Build/${ACG_PROJECT_PLUGINDIR})
        file (MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Build/${ACG_PROJECT_PLUGINDIR})
      endif ()
      add_custom_command (TARGET ${_target} POST_BUILD
                          COMMAND ${CMAKE_COMMAND} -E
                          copy_if_different
                            $<TARGET_FILE:${_target}>
                            ${CMAKE_BINARY_DIR}/Build/${ACG_PROJECT_PLUGINDIR}/$<TARGET_FILE_NAME:${_target}>)
    elseif (${_type} STREQUAL STATIC)
    add_custom_command (TARGET ${_target} POST_BUILD
                          COMMAND ${CMAKE_COMMAND} -E
                          copy_if_different
                            $<TARGET_FILE:${_target}>
                            ${CMAKE_BINARY_DIR}/Build/${ACG_PROJECT_LIBDIR}/$<TARGET_FILE_NAME:${_target}>)
  endif()
  
   
  # make an extra copy for windows into the binary directory
    if (${_type} STREQUAL SHARED AND WIN32)
      add_custom_command (TARGET ${_target} POST_BUILD
                          COMMAND ${CMAKE_COMMAND} -E
                          copy_if_different 
                            $<TARGET_FILE:${_target}>
                            ${CMAKE_BINARY_DIR}/Build/${ACG_PROJECT_BINDIR}/$<TARGET_FILE_NAME:${_target}>)
  endif () 
    
  endif( (WIN32 AND MSVC) OR (APPLE AND NOT ACG_PROJECT_MACOS_BUNDLE))
  
  if (_and_static)
    add_custom_command (TARGET ${_target}Static POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E
                        copy_if_different
                          $<TARGET_FILE:${_target}Static>
                          ${CMAKE_BINARY_DIR}/Build/${ACG_PROJECT_LIBDIR}/$<TARGET_FILE_NAME:${_target}Static>)

  endif ()
 

  # Block installation of libraries by setting ACG_NO_LIBRARY_INSTALL
  if ( NOT ACG_NO_LIBRARY_INSTALL ) 
    if (NOT ACG_PROJECT_MACOS_BUNDLE OR NOT APPLE)
      if (${_type} STREQUAL SHARED OR ${_type} STREQUAL STATIC )
        install (TARGETS ${_target}
                 RUNTIME DESTINATION ${ACG_PROJECT_BINDIR}
                 LIBRARY DESTINATION ${ACG_PROJECT_LIBDIR}
                 ARCHIVE DESTINATION ${ACG_PROJECT_LIBDIR})
        if (_and_static)
          install (TARGETS ${_target}Static
                   DESTINATION ${ACG_PROJECT_LIBDIR})
        endif ()
      elseif (${_type} STREQUAL MODULE)
        install (TARGETS ${_target} DESTINATION ${ACG_PROJECT_PLUGINDIR})
      endif ()
    endif ()
  endif()

endfunction ()

