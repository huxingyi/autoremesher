# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the OpenEXR Project.

ilmbase_define_library(IlmThread
  PRIV_EXPORT ILMTHREAD_EXPORTS
  CURDIR ${CMAKE_CURRENT_SOURCE_DIR}
  SOURCES
    IlmThread.cpp
    IlmThreadMutex.cpp
    IlmThreadMutexPosix.cpp
    IlmThreadPool.cpp
    IlmThreadPosix.cpp
    IlmThreadSemaphore.cpp
    IlmThreadSemaphorePosixCompat.cpp
    IlmThreadSemaphorePosix.cpp
    IlmThreadSemaphoreOSX.cpp
    IlmThreadMutexWin32.cpp
    IlmThreadSemaphoreWin32.cpp
    IlmThreadWin32.cpp
  HEADERS
    IlmThreadPool.h
    IlmThread.h
    IlmThreadSemaphore.h
    IlmThreadMutex.h
    IlmThreadNamespace.h
    IlmThreadExport.h
    IlmThreadForward.h
  DEPENDENCIES
    IlmBase::Iex
    Threads::Threads
  )
