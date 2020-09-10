# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the OpenEXR Project.

if(TARGET Python2::Interpreter)
  execute_process(
    COMMAND ${Python2_EXECUTABLE} -c
    "from __future__ import print_function\ntry: import numpy; print(numpy.get_include(), end='')\nexcept: import sys; sys.exit(1)\n"
    RESULT_VARIABLE _NUMPY2_RESULT
    OUTPUT_VARIABLE py_ilmbase_numpy2_path
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  if(NOT _NUMPY2_RESULT MATCHES 0)
    set(NumPy_Py2_FOUND FALSE CACHE INTERNAL "Python2 numpy libraries not found")
    message(WARNING "Unable to import numpy using python ${Python2_VERSION}")
  else()
    execute_process(
      COMMAND ${Python2_EXECUTABLE} -c
      "from __future__ import print_function\ntry: import numpy; print(numpy.__version__, end='')\nexcept: import sys; sys.exit(1)\n"
      RESULT_VARIABLE _NUMPY2_RESULT
      OUTPUT_VARIABLE py_ilmbase_numpy2_version
      OUTPUT_STRIP_TRAILING_WHITESPACE)

    if(NOT _NUMPY2_RESULT MATCHES 0)
      set(NumPy_Py2_FOUND FALSE CACHE INTERNAL "Python2 numpy libraries not found")
      message(WARNING "Found numpy module in python ${Python2_VERSION}, but no version information")
    else()
      find_path(NumPy_Py2_INCLUDE_DIRS numpy/arrayobject.h
        HINTS "${py_ilmbase_numpy2_path}" "${Python2_INCLUDE_DIRS}"
        NO_DEFAULT_PATH
      )
      if(NumPy_Py2_INCLUDE_DIRS)
        set(NumPy_Py2_FOUND TRUE CACHE INTERNAL "Python2 numpy found")
        set(NumPy_Py2_VERSION ${py_ilmbase_numpy2_version})
        add_library(NumPy_Py2 INTERFACE IMPORTED GLOBAL)
        target_include_directories(NumPy_Py2 INTERFACE ${NumPy_Py2_INCLUDE_DIRS})
        add_library(Python2::IlmBaseNumPy ALIAS NumPy_Py2)
        message(STATUS "Found NumPy ${NumPy_Py2_VERSION} for Python ${Python2_VERSION}: ${NumPy_Py2_INCLUDE_DIRS}")
      else()
        set(NumPy_Py2_FOUND FALSE CACHE INTERNAL "Python2 numpy libraries not found")
        message(WARNING "Found numpy version ${py_ilmbase_numpy2_version} in python ${Python2_VERSION}, but unable to locate header files")
      endif()
    endif()
  endif()
endif()

if(TARGET Python3::Interpreter)
  execute_process(
    COMMAND ${Python3_EXECUTABLE} -c
    "from __future__ import print_function\ntry: import numpy; print(numpy.get_include(), end='')\nexcept: import sys; sys.exit(1)\n"
    RESULT_VARIABLE _NUMPY3_RESULT
    OUTPUT_VARIABLE py_ilmbase_numpy3_path
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  if(NOT _NUMPY3_RESULT MATCHES 0)
    set(NumPy_Py3_FOUND FALSE CACHE INTERNAL "Python3 numpy libraries not found")
    message(WARNING "Unable to import numpy using python ${Python3_VERSION}")
  else()
    execute_process(
      COMMAND ${Python3_EXECUTABLE} -c
      "from __future__ import print_function\ntry: import numpy; print(numpy.__version__, end='')\nexcept: import sys; sys.exit(1)\n"
      RESULT_VARIABLE _NUMPY3_RESULT
      OUTPUT_VARIABLE py_ilmbase_numpy3_version
      OUTPUT_STRIP_TRAILING_WHITESPACE)

    if(NOT _NUMPY3_RESULT MATCHES 0)
      set(NumPy_Py3_FOUND FALSE CACHE INTERNAL "Python3 numpy libraries not found")
      message(WARNING "Found numpy module in python ${Python3_VERSION}, but no version information")
    else()
      find_path(NumPy_Py3_INCLUDE_DIRS numpy/arrayobject.h
        HINTS "${py_ilmbase_numpy3_path}" "${Python3_INCLUDE_DIRS}"
        NO_DEFAULT_PATH
      )
      if(NumPy_Py3_INCLUDE_DIRS)
        set(NumPy_Py3_FOUND TRUE CACHE INTERNAL "Python3 numpy found")
        set(NumPy_Py3_VERSION ${py_ilmbase_numpy3_version})
        add_library(NumPy_Py3 INTERFACE IMPORTED GLOBAL)
        target_include_directories(NumPy_Py3 INTERFACE ${NumPy_Py3_INCLUDE_DIRS})
        add_library(Python3::IlmBaseNumPy ALIAS NumPy_Py3)
        message(STATUS "Found NumPy ${NumPy_Py3_VERSION} for Python ${Python3_VERSION}: ${NumPy_Py3_INCLUDE_DIRS}")
      else()
        set(NumPy_Py3_FOUND FALSE CACHE INTERNAL "Python3 numpy libraries not found")
        message(WARNING "Found numpy version ${py_ilmbase_numpy3_version} in python ${Python3_VERSION}, but unable to locate header files")
      endif()
    endif()
  endif()
endif()
