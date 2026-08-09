
project(EigenBlas CXX)

if(EIGEN_BUILD_BLAS)

add_custom_target(blas)

set(EigenBlas_SRCS  single.cpp double.cpp complex_single.cpp complex_double.cpp xerbla.cpp
                    f2c/srotm.c   f2c/srotmg.c  f2c/drotm.c f2c/drotmg.c
                    f2c/lsame.c   f2c/dspmv.c   f2c/ssbmv.c f2c/chbmv.c
                    f2c/sspmv.c   f2c/zhbmv.c   f2c/chpmv.c f2c/dsbmv.c
                    f2c/zhpmv.c   f2c/dtbmv.c   f2c/stbmv.c f2c/ctbmv.c
                    f2c/ztbmv.c   f2c/complexdots.c
   )

set(EIGEN_BLAS_TARGETS "")

add_library(eigen_blas_static STATIC ${EigenBlas_SRCS})
list(APPEND EIGEN_BLAS_TARGETS eigen_blas_static)

if (EIGEN_BUILD_SHARED_LIBS)
  add_library(eigen_blas SHARED ${EigenBlas_SRCS} "eigen_blas.def")
  target_compile_definitions(eigen_blas PUBLIC "EIGEN_BLAS_BUILD_DLL")
  set_target_properties(eigen_blas PROPERTIES CXX_VISIBILITY_PRESET hidden)
  list(APPEND EIGEN_BLAS_TARGETS eigen_blas)
endif()

foreach(target IN LISTS EIGEN_BLAS_TARGETS)
  if(EIGEN_STANDARD_LIBRARIES_TO_LINK_TO)
      target_link_libraries(${target} ${EIGEN_STANDARD_LIBRARIES_TO_LINK_TO})
  endif()

  add_dependencies(blas ${target})
  install(TARGETS ${target}
          RUNTIME DESTINATION bin
          LIBRARY DESTINATION lib
          ARCHIVE DESTINATION lib)
endforeach()

if(EIGEN_BUILD_TESTING)
  if(EIGEN_LEAVE_TEST_IN_ALL_TARGET)
    add_subdirectory(testing) # can't do EXCLUDE_FROM_ALL here, breaks CTest
  else()
    add_subdirectory(testing EXCLUDE_FROM_ALL)
  endif()
endif()

endif()
