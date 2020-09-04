#-------------------------------------------------------------------
# Flags common to all Windows based platforms with Visual Studio
#-------------------------------------------------------------------

include(${GEOGRAM_SOURCE_DIR}/cmake/platforms/Windows.cmake)

# Suppresses the display of the copyright banner
add_definitions(/nologo)

# Allow the compilation using multiple processors
add_definitions(/MP)

# Disable C++ exceptions (enabled by default)
#remove_flags(CMAKE_CXX_FLAGS /EHsc)

# Compile with wchar_t support to fix unresolved symbols in Xerces
#remove_definitions(/Zc:wchar_t-)
#add_definitions(/Zc:wchar_t)

# Warning Level 4
remove_definitions(/W3)
add_definitions(/W4)

# Remove warning: non DLL-interface classkey 'identifier' used as base for DLL-interface classkey 'identifier'
add_definitions(/wd4275)

# Remove warning: 'function': was declared deprecated
add_definitions(/wd4996)

# Remove warning: assignment operator could not be generated
add_definitions(/wd4512)

# Increases the number of addressable sections in an .obj file.
add_definitions(/bigobj)

# Exclude some of the less common API declarations, for faster builds
add_definitions(-DWIN32_LEAN_AND_MEAN)
add_definitions(-DVC_EXTRALEAN)

# Disable the macros min and max defined by Visual Studio. This interferes with std::min and std::max
add_definitions(-DNOMINMAX )

# Add _CRT_SECURE_NO_WARNINGS define to disable warning C4996 : This function or variable may be unsafe...
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

# Manage Math definitions (such as M_PI)
add_definitions(-D_USE_MATH_DEFINES)

# GZ is deprecated (replaced by RTC1, which is already added by CMake in debug mode)
remove_flags(CMAKE_CXX_FLAGS_DEBUG /GZ)
remove_flags(CMAKE_C_FLAGS_DEBUG /GZ)

# GX is deprecated (replaced by EHsc)
remove_flags(CMAKE_CXX_FLAGS /GX)

# Change flags for static link
if(VORPALINE_BUILD_DYNAMIC)
# remove warning for multiply defined symbols (caused by multiple
# instanciations of STL templates)
  add_definitions(/wd4251)
else()
  foreach(config ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${config} config)
    string(REPLACE /MD /MT CMAKE_C_FLAGS_${config} "${CMAKE_C_FLAGS_${config}}")
    string(REPLACE /MD /MT CMAKE_CXX_FLAGS_${config} "${CMAKE_CXX_FLAGS_${config}}")
  endforeach()
endif()

# Additional release flags
foreach(config RELEASE RELWITHDEBINFO MINSIZEREL)
    add_flags(CMAKE_CXX_FLAGS_${config} /D_SECURE_SCL=0 /GS- /Ox)
    add_flags(CMAKE_C_FLAGS_${config} /D_SECURE_SCL=0 /GS- /Ox)
endforeach()

# Generate debug information even in release mode
# add_definitions(/Zi)

# Allow debug information in release modes
#foreach(config RELEASE RELWITHDEBINFO MINSIZEREL)
#    add_flags(CMAKE_SHARED_LINKER_FLAGS_${config} /DEBUG)
#    add_flags(CMAKE_EXE_LINKER_FLAGS_${config} /DEBUG)
#endforeach()

# Allow inter-procedural optimization in release modes
foreach(config RELEASE RELWITHDEBINFO MINSIZEREL)
    add_flags(CMAKE_SHARED_LINKER_FLAGS_${config} /OPT:REF,ICF)
    add_flags(CMAKE_EXE_LINKER_FLAGS_${config} /OPT:REF,ICF)
endforeach()

# Disable inter-procedural optimization in debug mode
add_flags(CMAKE_SHARED_LINKER_FLAGS_DEBUG /OPT:NOREF,NOICF)
add_flags(CMAKE_EXE_LINKER_FLAGS_DEBUG /OPT:NOREF,NOICF)

# Reset the warning level for third parties
function(vor_reset_warning_level)
    remove_definitions(/W4)
    add_definitions(/W3)
    add_definitions(/wd4245)
    add_definitions(/wd4389)
endfunction()

# Create a statically linked executable
macro(vor_add_executable)
    add_executable(${ARGN})
endmacro()
 
