#
# Macros for generating the documentation
#

#! The minimum doxygen version number allowed here
set(DOXYGEN_MINIMUM_VERSION 1.7.0)

# Check for doxygen and dot

find_package(Doxygen ${DOXYGEN_MINIMUM_VERSION} QUIET)
if(NOT DOXYGEN_FOUND)
    message(STATUS "Doxygen >= ${DOXYGEN_MINIMUM_VERSION} not found, cannot generate documentation")
    unset(DOXYGEN_EXECUTABLE CACHE)
    return()
endif()

if(DOXYGEN_DOT_FOUND)
    set(DOXYGEN_HAVE_DOT YES)
else()
    set(DOXYGEN_HAVE_DOT NO)
endif()


if(DOXYGEN_FOUND)

# Generate a target for generating a specific documentation type

  function(add_doc_target doc_type)

    set(doc_output_dir ${CMAKE_CURRENT_BINARY_DIR})

    configure_file(${doc_type}.dox.in ${doc_type}.dox @ONLY)

    if(doc_type STREQUAL "doc")
      set(doc_target doc)
    else()
      set(doc_target doc-${doc_type})
    endif()
    
    add_custom_target(
        ${doc_target}
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${doc_output_dir}/${doc_type}
        COMMAND ${DOXYGEN_EXECUTABLE} ${doc_type}.dox # > ${doc_type}.log 2>&1
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generate ${doc_type} documentation for ${PROJECT_NAME}"
    )

    set_property(
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${doc_output_dir}/${doc_type}
    )

    set_target_properties(
        ${doc_target} PROPERTIES
	FOLDER "DOC"
    )

  endfunction()

endif()