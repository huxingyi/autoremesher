################################################################################
# Custom settings for compiler flags and similar
################################################################################

if ( WIN32 )
  ################################################################################
  # Windows large memory support
  ################################################################################
  if ( NOT DEFINED WINDOWS_LARGE_MEMORY_SUPPORT )
    set( WINDOWS_LARGE_MEMORY_SUPPORT true CACHE BOOL "Enable or disable binary support for large memory" )
  endif()
  
  set( ADDITIONAL_CMAKE_EXE_LINKER_FLAGS )
  set( ADDITIONAL_CMAKE_SHARED_LINKER_FLAGS )
  set( ADDITIONAL_CMAKE_MODULE_LINKER_FLAGS )
  
  if ( WINDOWS_LARGE_MEMORY_SUPPORT )
    if( NOT "${CMAKE_GENERATOR}" MATCHES "MinGW Makefiles")
	list(APPEND ADDITIONAL_CMAKE_EXE_LINKER_FLAGS       "/LARGEADDRESSAWARE" )
	list(APPEND ADDITIONAL_CMAKE_SHARED_LINKER_FLAGS    "/LARGEADDRESSAWARE" )
	list(APPEND ADDITIONAL_CMAKE_MODULE_LINKER_FLAGS    "/LARGEADDRESSAWARE" )
    endif()
  endif()
  
  
  foreach( flag ${ADDITIONAL_CMAKE_EXE_LINKER_FLAGS} )
    if( NOT CMAKE_EXE_LINKER_FLAGS MATCHES "${flag}" )
      set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${flag} ")
    endif()
  endforeach()
  
  foreach( flag ${ADDITIONAL_CMAKE_SHARED_LINKER_FLAGS} )
    if( NOT CMAKE_SHARED_LINKER_FLAGS MATCHES "${flag}" )
      set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${flag} ")
    endif()
  endforeach()
  
  foreach( flag ${ADDITIONAL_CMAKE_MODULE_LINKER_FLAGS} )
    if( NOT CMAKE_MODULE_LINKER_FLAGS MATCHES "${flag}" )
      set( CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${flag} ")
    endif()
  endforeach()

endif( WIN32 )

if (UNIX)

  set ( ADDITIONAL_CXX_DEBUG_FLAGS )
  set ( ADDITIONAL_CXX_RELEASE_FLAGS )
  set ( ADDITIONAL_CXX_RELWITHDEBINFO_FLAGS )
  
  set ( ADDITIONAL_C_DEBUG_FLAGS )
  set ( ADDITIONAL_C_RELEASE_FLAGS )
  set ( ADDITIONAL_C_RELWITHDEBINFO_FLAGS )

  ################################################################################
  # Defaults
  ################################################################################

  # add our standard flags for Template inclusion
  list(APPEND ADDITIONAL_CXX_DEBUG_FLAGS          "-DINCLUDE_TEMPLATES" )
  list(APPEND ADDITIONAL_CXX_RELEASE_FLAGS        "-DINCLUDE_TEMPLATES" )
  list(APPEND ADDITIONAL_CXX_RELWITHDEBINFO_FLAGS "-DINCLUDE_TEMPLATES" )

  # add our standard flags for Template inclusion
  list(APPEND ADDITIONAL_C_DEBUG_FLAGS            "-DINCLUDE_TEMPLATES" )
  list(APPEND ADDITIONAL_C_RELEASE_FLAGS          "-DINCLUDE_TEMPLATES" )
  list(APPEND ADDITIONAL_C_RELWITHDEBINFO_FLAGS   "-DINCLUDE_TEMPLATES" )
 
#  Deprecated setting. Remove in future release, as the default template depth 
#  should be enough with state of the art compilers 
#  # Increase the template depth as this might be exceeded from time to time
#  IF( NOT CMAKE_SYSTEM MATCHES "SunOS*")
#    list(APPEND ADDITIONAL_CXX_DEBUG_FLAGS          "-ftemplate-depth-100" )
#    list(APPEND ADDITIONAL_CXX_RELEASE_FLAGS        "-ftemplate-depth-100" )
#    list(APPEND ADDITIONAL_CXX_RELWITHDEBINFO_FLAGS "-ftemplate-depth-100" )  
#  ENDIF()
  
  ################################################################################
  # OS Defines
  ################################################################################  
  
  if (APPLE)
    add_definitions( -DARCH_DARWIN )
  endif()
  
  ################################################################################
  # Build/Release Defines
  ################################################################################
  IF( NOT CMAKE_SYSTEM MATCHES "SunOS*")
    list(APPEND ADDITIONAL_CXX_DEBUG_FLAGS          "-DDEBUG" )
    list(APPEND ADDITIONAL_CXX_RELEASE_FLAGS        "-DNDEBUG" )
    list(APPEND ADDITIONAL_CXX_RELWITHDEBINFO_FLAGS "-DDEBUG" )    
    
    list(APPEND ADDITIONAL_C_DEBUG_FLAGS            "-DDEBUG" )
    list(APPEND ADDITIONAL_C_RELEASE_FLAGS          "-DNDEBUG" )
    list(APPEND ADDITIONAL_C_RELWITHDEBINFO_FLAGS   "-DDEBUG" )
  ENDIF()  
  
  ################################################################################
  # Warnings
  ################################################################################
 
  # Add the standard compiler warnings
  if ( NOT COMPILER_WARNINGS )
   
    IF ( APPLE  )
      # Skip unused parameters as it has to be used for the documentation via doxygen and the interfaces
      set ( COMPILER_WARNINGS "-W" "-Wall" "-Wno-unused" "-Wextra" "-Wno-non-virtual-dtor" "-Wno-unused-parameter" "-Wno-variadic-macros" CACHE STRINGLIST "This list contains the warning flags used during compilation " )
    elseif ("${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}" MATCHES ".*clang")
      set ( COMPILER_WARNINGS "-W" "-Wall" "-Wextra" "-Wno-non-virtual-dtor" "-Wno-unused-parameter" "-Wno-variadic-macros" CACHE STRINGLIST "This list contains the warning flags used during compilation " )
    ELSEIF ( CMAKE_SYSTEM MATCHES "SunOS*" )
      set ( COMPILER_WARNINGS "" CACHE STRINGLIST "This list contains the warning flags used during compilation " )
    ELSE ()
      set ( COMPILER_WARNINGS "-W" "-Wall" "-Wno-unused" "-Wextra" "-Wno-variadic-macros" CACHE STRINGLIST "This list contains the warning flags used during compilation " )
    ENDIF()

  endif ( NOT COMPILER_WARNINGS )

  list(APPEND ADDITIONAL_CXX_DEBUG_FLAGS          ${COMPILER_WARNINGS} )
  list(APPEND ADDITIONAL_CXX_RELEASE_FLAGS        ${COMPILER_WARNINGS} )
  list(APPEND ADDITIONAL_CXX_RELWITHDEBINFO_FLAGS ${COMPILER_WARNINGS} )    
    
  list(APPEND ADDITIONAL_C_DEBUG_FLAGS            ${COMPILER_WARNINGS} )
  list(APPEND ADDITIONAL_C_RELEASE_FLAGS          ${COMPILER_WARNINGS} )
  list(APPEND ADDITIONAL_C_RELWITHDEBINFO_FLAGS   ${COMPILER_WARNINGS} )
  
  ################################################################################
  # STL Vector checks
  ################################################################################
  
  # Pre initialize stl vector check variable
  if ( NOT STL_VECTOR_CHECKS )
    set ( STL_VECTOR_CHECKS false CACHE BOOL "Include full stl vector checks in debug mode (This option is only used in debug Mode!)" )
  endif ( NOT STL_VECTOR_CHECKS )
  
  # Add a flag to check stl vectors in debugging mode
  if ( STL_VECTOR_CHECKS AND NOT CMAKE_SYSTEM MATCHES "SunOS*"  )
    list(APPEND ADDITIONAL_CXX_DEBUG_FLAGS          "-D_GLIBCXX_DEBUG" )
    list(APPEND ADDITIONAL_CXX_DEBUG_FLAGS          "-D_GLIBCXX_DEBUG_PEDANTIC")
    list(APPEND ADDITIONAL_CXX_RELWITHDEBINFO_FLAGS "-D_GLIBCXX_DEBUG" )
    list(APPEND ADDITIONAL_CXX_RELWITHDEBINFO_FLAGS "-D_GLIBCXX_DEBUG_PEDANTIC")
    
    list(APPEND ADDITIONAL_C_DEBUG_FLAGS            "-D_GLIBCXX_DEBUG" )
    list(APPEND ADDITIONAL_C_DEBUG_FLAGS            "-D_GLIBCXX_DEBUG_PEDANTIC")
    list(APPEND ADDITIONAL_C_RELWITHDEBINFO_FLAGS   "-D_GLIBCXX_DEBUG" )
    list(APPEND ADDITIONAL_C_RELWITHDEBINFO_FLAGS   "-D_GLIBCXX_DEBUG_PEDANTIC")
  endif()

  ################################################################################
  # Process the additional flags:
  ################################################################################

  # Add the debug flags
  foreach( flag ${ADDITIONAL_CXX_DEBUG_FLAGS} )
    if( NOT CMAKE_CXX_FLAGS_DEBUG MATCHES "${flag}" )
      set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${flag} ")
    endif()
  endforeach()

  # Add the release flags
  foreach( flag ${ADDITIONAL_CXX_RELEASE_FLAGS} )
    if( NOT CMAKE_CXX_FLAGS_RELEASE MATCHES "${flag}" )
      set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${flag} ")
    endif()
  endforeach()

  # Add the release with debug info flags
  foreach( flag ${ADDITIONAL_CXX_RELWITHDEBINFO_FLAGS} )
    if( NOT CMAKE_CXX_FLAGS_RELWITHDEBINFO MATCHES "${flag}" )
      set( CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${flag} ")
    endif()
  endforeach()

  # Add the debug flags
  foreach( flag ${ADDITIONAL_C_DEBUG_FLAGS} )
    if( NOT CMAKE_C_FLAGS_DEBUG MATCHES "${flag}" )
      set( CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${flag} ")
    endif()
  endforeach()

  # Add the release flags
  foreach( flag ${ADDITIONAL_C_RELEASE_FLAGS} )
    if( NOT CMAKE_C_FLAGS_RELEASE MATCHES "${flag}" )
      set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${flag} ")
    endif()
  endforeach()

  # Add the release with debug info flags
  foreach( flag ${ADDITIONAL_C_RELWITHDEBINFO_FLAGS} )
    if( NOT CMAKE_C_FLAGS_RELWITHDEBINFO MATCHES "${flag}" )
      set( CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} ${flag} ")
    endif()
  endforeach()

  #TODO : Test and remove it?!
  IF( CMAKE_SYSTEM MATCHES "SunOS*")
    set (CMAKE_CFLAGS_RELEASE "-xO3")
    set (CMAKE_CXX_FLAGS_RELEASE "-xO3")        
  endif ( CMAKE_SYSTEM MATCHES "SunOS*" ) 
  
endif ()
