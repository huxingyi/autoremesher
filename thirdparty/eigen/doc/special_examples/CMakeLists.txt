if(NOT EIGEN_TEST_NOQT)
  find_package(Qt4)
  if(QT4_FOUND)
    include(${QT_USE_FILE})
  endif()
endif()

if(QT4_FOUND)
  add_executable(Tutorial_sparse_example Tutorial_sparse_example.cpp Tutorial_sparse_example_details.cpp)
  target_link_libraries(Tutorial_sparse_example ${EIGEN_STANDARD_LIBRARIES_TO_LINK_TO} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} Eigen3::Eigen)

  add_custom_command(
    TARGET Tutorial_sparse_example
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/../html/
    COMMAND Tutorial_sparse_example ARGS ${CMAKE_CURRENT_BINARY_DIR}/../html/Tutorial_sparse_example.jpeg
  )

  add_dependencies(all_examples Tutorial_sparse_example)
endif()

add_executable(random_cpp11 random_cpp11.cpp)
target_link_libraries(random_cpp11 ${EIGEN_STANDARD_LIBRARIES_TO_LINK_TO} Eigen3::Eigen)
add_dependencies(all_examples random_cpp11)

add_custom_command(
  TARGET random_cpp11
  POST_BUILD
  COMMAND random_cpp11
  ARGS >${CMAKE_CURRENT_BINARY_DIR}/random_cpp11.out
)
