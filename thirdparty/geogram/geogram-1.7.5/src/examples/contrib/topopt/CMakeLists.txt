CMake_Minimum_Required(VERSION 3.0)

# Module path
Set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

If (EMSCRIPTEN)
	Include(SetEmscripten)
Endif ()

# Project name
Project(topopt_demo)

# Default to Release mode
If (NOT CMAKE_BUILD_TYPE)
	Message (STATUS "No build type selected, default to Release")
	Set (CMAKE_BUILD_TYPE "Release")
Endif ()

# Target executable
Add_Executable(${PROJECT_NAME} main.cpp mesh_primitives.cpp topopt.cpp)

# Link to the standard thread library
Find_Package (Threads REQUIRED)
Target_Link_Libraries (${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})

# Link to Geogram
#Find_Package(Geogram)
#Target_Link_Libraries(${PROJECT_NAME} Geogram::geogram_gfx Geogram::geogram)

Target_Link_Libraries(${PROJECT_NAME} geogram_gfx geogram)

# Link to Eigen
Find_Package(Eigen3)
Target_Include_Directories(${PROJECT_NAME} SYSTEM PUBLIC ${EIGEN3_INCLUDE_DIR})

# Link to Cholmod
If (ENABLE_CHOLMOD)
	Find_Package (SuiteSparse)
Endif ()
If (CHOLMOD_FOUND)
	Target_Include_Directories(${PROJECT_NAME} PUBLIC ${CHOLMOD_INCLUDE_DIR})
	Target_Link_Libraries (${PROJECT_NAME} ${CHOLMOD_LIBRARY})
	Target_Compile_Definitions (${PROJECT_NAME} PUBLIC -DUSE_CHOLMOD)
Endif ()

# Use C++11
Set_Target_Properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 11)
Set_Target_Properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD_REQUIRED ON)

# Embed files
if (EMSCRIPTEN)
	#set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/assets/@/")
	#set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")
endif ()
