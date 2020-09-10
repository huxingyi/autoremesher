# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the OpenEXR Project.

include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckLibraryExists)
include(CheckStructHasMember)

check_include_files(ucontext.h ILMBASE_HAVE_UCONTEXT_H)
if(ILMBASE_HAVE_UCONTEXT_H)
  # TODO: remove this once we cleanly have ILMBASE_ prefix on all #defines
  set(HAVE_UCONTEXT_H ON)
  check_struct_has_member("struct _libc_fpstate" mxcsr ucontext.h ILMBASE_HAVE_CONTROL_REGISTER_SUPPORT)
  if(NOT ILMBASE_HAVE_CONTROL_REGISTER_SUPPORT)
    check_include_files(asm/sigcontext.h ILMBASE_HAVE_SIGCONTEXT_H)
    if(ILMBASE_HAVE_SIGCONTEXT_H)
      check_struct_has_member("struct _fpstate" mxcsr asm/sigcontext.h ILMBASE_HAVE_SIGCONTEXT_CONTROL_REGISTER_SUPPORT)
    endif()
  endif()
endif()

###check_include_files(pthread.h ILMBASE_HAVE_PTHREAD)
if(Threads_FOUND)
  if(CMAKE_HAVE_PTHREAD_H OR CMAKE_USE_PTHREADS_INIT OR CMAKE_HP_PTHREAD_INIT)
    set(ILMBASE_HAVE_PTHREAD ON)
    # TODO: remove this once we cleanly have ILMBASE_ prefix on all #defines
    set(HAVE_PTHREAD ON)
  endif()

  # we have threads, but do we have posix semaphores for sem_init?
  # should be in pthreads
  if(NOT (APPLE OR WIN32))
    check_include_files(semaphore.h ILMBASE_HAVE_SEMAPHORE_H)
    if(ILMBASE_HAVE_SEMAPHORE_H AND ILMBASE_HAVE_PTHREAD)
      # should just be in libc, so no need for check_library_exists
      set(CMAKE_REQUIRED_FLAGS ${CMAKE_THREAD_LIBS_INIT})
      check_library_exists(pthread sem_init "" ILMBASE_HAVE_POSIX_SEMAPHORES)
      # TODO: remove this once we cleanly have ILMBASE_ prefix on all #defines
      if(ILMBASE_HAVE_POSIX_SEMAPHORES)
        set(HAVE_POSIX_SEMAPHORES ON)
      endif()
      set(CMAKE_REQUIRED_FLAGS)
    endif()
  endif()
endif()

if (ILMBASE_ENABLE_LARGE_STACK)
  set(ILMBASE_HAVE_LARGE_STACK ON)
endif()

configure_file(IlmBaseConfig.h.in_cmake ${CMAKE_CURRENT_BINARY_DIR}/IlmBaseConfig.h)
configure_file(IlmBaseConfigInternal.h.in_cmake ${CMAKE_CURRENT_BINARY_DIR}/IlmBaseConfigInternal.h)

add_library(IlmBaseConfig INTERFACE)
target_include_directories(IlmBaseConfig INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${ILMBASE_OUTPUT_SUBDIR}>)

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/IlmBaseConfig.h
  DESTINATION
    ${CMAKE_INSTALL_INCLUDEDIR}/${ILMBASE_OUTPUT_SUBDIR}
)

install(TARGETS IlmBaseConfig EXPORT ${PROJECT_NAME})
add_library(IlmBase::Config ALIAS IlmBaseConfig)

if(ILMBASE_INSTALL_PKG_CONFIG)
  # use a helper function to avoid variable pollution, but pretty simple
  function(ilmbase_pkg_config_help pcinfile)
    set(prefix ${CMAKE_INSTALL_PREFIX})
    set(exec_prefix ${CMAKE_INSTALL_BINDIR})
    set(libdir ${CMAKE_INSTALL_LIBDIR})
    set(includedir ${CMAKE_INSTALL_INCLUDEDIR})
    set(LIB_SUFFIX_DASH ${ILMBASE_LIB_SUFFIX})
    if(TARGET Threads::Threads)
      # hrm, can't use properties as they end up as generator expressions
      # which don't seem to evaluate
      if(THREADS_HAVE_PTHREAD_ARG)
        set(PTHREAD_CFLAGS "-pthread")
      endif()
      set(PTHREAD_LIBS ${CMAKE_THREAD_LIBS_INIT})
    endif()
    string(REPLACE ".in" "" pcout ${pcinfile})
    configure_file(${pcinfile} ${CMAKE_CURRENT_BINARY_DIR}/${pcout} @ONLY)
    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/${pcout}
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
    )
  endfunction()
  ilmbase_pkg_config_help(../IlmBase.pc.in)
else()
  message(NOTICE "-- WARNING pkg-config generation disabled")
endif()

# The main export of the configuration - This is the
# moral equivalent of a pkg-config file for cmake
# and replaces the Find*.cmake of the "old" cmake
include(CMakePackageConfigHelpers)
write_basic_package_version_file("${PROJECT_NAME}ConfigVersion.cmake"
  VERSION ${ILMBASE_VERSION}
  COMPATIBILITY SameMajorVersion
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)
install(EXPORT ${PROJECT_NAME}
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
  FILE ${PROJECT_NAME}Config.cmake
  NAMESPACE ${PROJECT_NAME}::
  EXPORT_LINK_INTERFACE_LIBRARIES
)
