include (ACGCommon)

include_directories (
  ..
  ${CMAKE_CURRENT_SOURCE_DIR}
)

if ( NOT DEFINED OPENMESH_BUILD_UNIT_TESTS)
    set( OPENMESH_BUILD_UNIT_TESTS false CACHE BOOL "Enable or disable unit test builds in OpenMesh." )
endif()

if ( OPENMESH_BUILD_UNIT_TESTS )
  # Search for gtest headers and libraries
  find_package(GTest)

  if(GTEST_FOUND)

    enable_testing()
   
    find_package(EIGEN3)

    # Set correct include paths so that the compiler can find the headers
    include_directories(${GTEST_INCLUDE_DIRS} )


    # set additional link directories
    link_directories(${GTEST_LIBRARY_DIR} )

    
    if (EIGEN3_FOUND)
      add_definitions( -DENABLE_EIGEN3_TEST ) 
      include_directories(${EIGEN3_INCLUDE_DIR})
    endif()

    if ( CMAKE_GENERATOR MATCHES "^Visual Studio 11.*" ) 
      add_definitions( /D _VARIADIC_MAX=10 )
    endif()

    # Create new target named unittests_hexmeshing
    FILE(GLOB UNITTEST_SRC *.cc)
    # Create unittest executable
    acg_add_executable(unittests ${UNITTEST_SRC})
    acg_add_executable(unittests_customvec ${UNITTEST_SRC})
    acg_add_executable(unittests_doublevec ${UNITTEST_SRC})
    target_compile_definitions(unittests_customvec PRIVATE TEST_CUSTOM_TRAITS)
    target_compile_definitions(unittests_doublevec PRIVATE TEST_DOUBLE_TRAITS)
 
    # For the unittest we don't want the install rpath as set by acg_add_executable 
    set_target_properties ( unittests PROPERTIES  BUILD_WITH_INSTALL_RPATH 0 )
    set_target_properties ( unittests_customvec PROPERTIES  BUILD_WITH_INSTALL_RPATH 0 )
    set_target_properties ( unittests_doublevec PROPERTIES  BUILD_WITH_INSTALL_RPATH 0 )

   # Set output directory to ${BINARY_DIR}/Unittests
    set (OUTPUT_DIR "${CMAKE_BINARY_DIR}/Unittests")
      set_target_properties(unittests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
      set_target_properties(unittests_customvec PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
      set_target_properties(unittests_doublevec PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
      foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${CONFIG} UPCONFIG)
        set_target_properties(unittests PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${UPCONFIG} ${OUTPUT_DIR})
        set_target_properties(unittests_customvec PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${UPCONFIG} ${OUTPUT_DIR})
        set_target_properties(unittests_doublevec PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${UPCONFIG} ${OUTPUT_DIR})
    endforeach()

	
	if ( NOT WIN32)
           # Link against all necessary libraries
           target_link_libraries(unittests OpenMeshCore OpenMeshTools ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)
           target_link_libraries(unittests_customvec OpenMeshCore OpenMeshTools ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)
           target_link_libraries(unittests_doublevec OpenMeshCore OpenMeshTools ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)
	  
	  
	
       else()
	    # Link against all necessary libraries
		if ( OPENMESH_BUILD_SHARED )
			add_definitions( -DOPENMESHDLL )

		endif()
      target_link_libraries(unittests OpenMeshCore OpenMeshTools ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES})
      target_link_libraries(unittests_customvec OpenMeshCore OpenMeshTools ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES})
      target_link_libraries(unittests_doublevec OpenMeshCore OpenMeshTools ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES})
    endif()
	

    
	if ( NOT WIN32 )
	    # Set compiler flags
      set_target_properties(unittests PROPERTIES COMPILE_FLAGS "-g -pedantic -Wno-long-long")
      set_target_properties(unittests_customvec PROPERTIES COMPILE_FLAGS "-g -pedantic -Wno-long-long")
      set_target_properties(unittests_doublevec PROPERTIES COMPILE_FLAGS "-g -pedantic -Wno-long-long")
	else()
	    # Set compiler flags
      set_target_properties(unittests PROPERTIES COMPILE_FLAGS "" )
      set_target_properties(unittests_customvec PROPERTIES COMPILE_FLAGS "" )
      set_target_properties(unittests_doublevec PROPERTIES COMPILE_FLAGS "" )
    endif()
    
    if ( OPENMESH_BUILD_SHARED )
	
      #-------- copy dlls to unittests --------
      SET(OPENMESH_TARGETS "OpenMeshTools" "OpenMeshCore")
    
       foreach(TAR ${OPENMESH_TARGETS} )
         add_custom_command(TARGET unittests POST_BUILD 
                            COMMAND "${CMAKE_COMMAND}" -E copy 
                            "$<TARGET_FILE:${TAR}>"
                            "${CMAKE_BINARY_DIR}/Unittests/$<TARGET_FILE_NAME:${TAR}>" 
                            COMMENT "Copying OpenMesh targets to unittests directory")
         add_custom_command(TARGET unittests_customvec POST_BUILD
                            COMMAND "${CMAKE_COMMAND}" -E copy
                            "$<TARGET_FILE:${TAR}>"
                            "${CMAKE_BINARY_DIR}/Unittests/$<TARGET_FILE_NAME:${TAR}>"
                            COMMENT "Copying OpenMesh targets to unittests directory")
         add_custom_command(TARGET unittests_doublevec POST_BUILD
                            COMMAND "${CMAKE_COMMAND}" -E copy
                            "$<TARGET_FILE:${TAR}>"
                            "${CMAKE_BINARY_DIR}/Unittests/$<TARGET_FILE_NAME:${TAR}>"
                            COMMENT "Copying OpenMesh targets to unittests directory")
       endforeach(TAR)
     endif()
	
    acg_copy_after_build(unittests ${CMAKE_CURRENT_SOURCE_DIR}/TestFiles ${CMAKE_BINARY_DIR}/Unittests/)
    acg_copy_after_build(unittests_customvec ${CMAKE_CURRENT_SOURCE_DIR}/TestFiles ${CMAKE_BINARY_DIR}/Unittests/)
    acg_copy_after_build(unittests_doublevec ${CMAKE_CURRENT_SOURCE_DIR}/TestFiles ${CMAKE_BINARY_DIR}/Unittests/)

    add_test(NAME AllTestsIn_OpenMesh_tests WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/Unittests" COMMAND "${CMAKE_BINARY_DIR}/Unittests/unittests")
    add_test(NAME AllTestsIn_OpenMesh_tests_with_minimal_vector WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/Unittests" COMMAND "${CMAKE_BINARY_DIR}/Unittests/unittests_customvec")
    add_test(NAME AllTestsIn_OpenMesh_tests_with_double_vector WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/Unittests" COMMAND "${CMAKE_BINARY_DIR}/Unittests/unittests_doublevec")

  else(GTEST_FOUND)
      message(STATUS "Google testing framework was not found, unittests disabled.")
  endif(GTEST_FOUND)
endif()
