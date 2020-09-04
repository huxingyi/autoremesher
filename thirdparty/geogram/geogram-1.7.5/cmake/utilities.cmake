#!
# @file cmake/utilities.cmake
# @brief Utilities for the Vorpaline build system
#

include(CMakeParseArguments)


#!
# @brief Appends elements to a list
#
# @param[in,out] _list  (required) List name where to add item.
# @param[in] items      (required) Items to add in the list.
#
macro(list_append _list)
    list(APPEND ${_list} ${ARGN})
endmacro()

#!
# @brief Uniquely appends elements to a list
# @details Adds the given @a items to @a _list if they do not already exist in
# the list.
# @param[in,out] _list  (required) List name where to add item.
# @param[in] items      (required) Items to add in the list.
#
macro(list_add_unique _list)
    list(APPEND ${_list} ${ARGN})
    if(${_list})
        list(REMOVE_DUPLICATES ${_list})
    endif()
endmacro()

#!
# @brief Prepends elements to a list
#
# @param[in,out] _list (required) List name where to prepend item(s).
# @param[in] items (required) Items to prepend in the list.
#
macro(list_prepend _list)
    list(INSERT ${_list} 0 ${ARGN})
endmacro()

#!
# @brief Removes elements from a list
# @details This is a convenience shortcut for list(REMOVE_ITEM list values...)
# @param[in,out] _list (required) List name where to prepend item(s).
# @param[in] items (required) Items to remove from the list.
#
macro(list_remove _list)
    list(REMOVE_ITEM ${_list} ${ARGN})
endmacro()

#!
# @brief Joins list elements into a string
# @details
# Joins all strings in list @a list into a single string. Elements in the
# resulting string are separated by @a separator elements.
# @param[out] var name of the variable that receives the result string
# @param[in] separator separator between list elements
# @param[in] list list of string to join.
#
macro(list_join _var _separator)
    set(_sep "")
    set(${_var} "")
    foreach(_arg ${ARGN})
        set(${_var} "${${_var}}${_sep}${_arg}")
        set(_sep ${_separator})
    endforeach()
endmacro()

#!
# @brief Adds values to a string
# @details
# Convenience macro for adding values individually to a string considered as a
# space separated list of values.
# Compared to list_append, the values are not transformed into a semicolon separated list.
# BTW, it ensures that the values are not added if they are already present
# in property_name.
# @param[in,out] _var name of the variable that contains the input string
# @param[in] values list of values to add to @a _var
#
function(add_flags _var)
    string(REPLACE " " ";" flags "${${_var}};${ARGN}")
    list(REMOVE_DUPLICATES flags)
    string(REPLACE ";" " " flags "${flags}")
    set(${_var} ${flags} PARENT_SCOPE)
endfunction()


#!
# @brief Adds values to a string
# @details
# Convenience macro for adding values individually to a string considered as a
# space separated list of values.
# Compared to list_append, the values are not transformed into a semicolon separated list.
# It differs from add_flags because it does not suppress duplicates values
# @param[in,out] _var name of the variable that contains the input string
# @param[in] values list of values to add to @a _var
#
function(add_flags_no_remove_duplicates _var)
    string(REPLACE " " ";" flags "${${_var}};${ARGN}")
    string(REPLACE ";" " " flags "${flags}")
    set(${_var} ${flags} PARENT_SCOPE)
endfunction()


#!
# @brief Removes values from a string
# @details
# Convenience function to remove values individually from a string considered as a
# space separated list of values.
# @param[in,out] _var name of the variable that contains the input string
# @param[in] values list of values to remove from @a _var
#
function(remove_flags _var)
    string(REPLACE " " ";" flags "${${_var}}")
    list(REMOVE_ITEM flags ${ARGN})
    string(REPLACE ";" " " flags "${flags}")
    set(${_var} ${flags} PARENT_SCOPE)
endfunction()

#!
# @brief Replaces a value in a string
# @details
# Convenience function to replace a value by abother one in a string
# considered as a space separated list of values.
# @param[in,out] _var name of the variable that contains the input string
# @param[in] _old_value the value to replace in @a _var
# @param[in] values list of values to add to @a _var
#
function(replace_flag _var _flag)
    string(REPLACE " " ";" flags "${${_var}}")
    list(FIND flags ${_flag} _index)
    if(NOT ${_index} EQUAL -1)
        list(REMOVE_AT flags ${_index})
        list(INSERT flags ${_index} ${ARGN})
        string(REPLACE ";" " " flags "${flags}")
        set(${_var} ${flags} PARENT_SCOPE)
    endif()
endfunction()

#!
# @brief Splits a string to a list of strings
# @details
# Splits the input string @a string into tokens separated by @a separator.
# token is added at the end of this listEach
# @param[out] var name of the variable that receives the result list
# @param[in] string the input string to split
# @param[in] separator seperator used to split the string
#
macro(string_split _var _string _separator)
    string(REPLACE ${_separator} ";" ${_var} "${_string}")
endmacro()

#!
# @brief Wrapper around CMake's @c configure_file that handles native paths
# @details
# pdgm_configure_file_with_native_paths() behaves just like the CMake's built-in
# configure_file() macro with the additional ability to replace path variables
# in the input file by their their native paths instead of the CMake path.
# Path variables can be either specified by the VARIABLES option or
# autodetected in the input file (see option AUTODETECT)
#
# @param[in] input_file   (required) the input file
# @param[in] output_file  (required) the output file
#
# @param[in] OPTIONS conf_file_opt  (optional) the list of options to give to 'configure_file'.
# @param[in] VARIABLES path_list    (optional) the list of path variables to convert to native paths. The
# result is stored in a variable suffixed by 'input_file'. 
# Use this variable in your template.
#
# @param[in] AUTODETECT (optional) if set, the function will attempt to autodetect path variables
# in the template source. Variables placeholders of the form @variable.PATH@
# will be automatically considered as path variables and converted to native
# path, in addition to the variables defined in argument VARIABLES
#
function(configure_file_with_native_paths input_file output_file)
    cmake_parse_arguments(
        ${input_file}
        "AUTODETECT"
        ""
        "OPTIONS;VARIABLES"
        ${ARGN}
    )

    if(${input_file}_AUTODETECT)
        # Read the template text
        file(READ ${input_file} _input)

        # Autodetect variables suffixed by .PATH in the source file
        string(REGEX MATCHALL "@[^-@]+\\.PATH@" _path_variables "${_input}")

        # Extract the list of variable names
        string(REGEX REPLACE "@([^-@]+)\\.PATH@" "\\1" _path_variables "${_path_variables}")

        # Convert paths to native paths
        foreach(_var ${_path_variables})
            file(TO_NATIVE_PATH "${${_var}}" "${_var}.PATH")
        endforeach()
    endif()

    # Convert paths referenced in VARIABLES to native paths
    foreach(_var ${${input_file}_VARIABLES})
        file(TO_NATIVE_PATH "${${_var}}" ${_var})
    endforeach()

    # Make a writable copy of the template and configure
    configure_file(${input_file} ${output_file} ${${input_file}_OPTIONS})
endfunction()

#!
# @brief Add sources from directories
# @details
# Add the sources from the specified \p directories to variable \p var
# and place them in the specified \p folder if non empty.
# @param[out] var name of the variable that receives the result list
# @param[in] folder the name of the folder
# @param[in] directories list of directories to scan
#
function(aux_source_directories var folder)
    set(sources)
    foreach(dir ${ARGN})
        file(GLOB _sources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${dir}/*.[ch]" "${dir}/*.[ch]pp")
        list(APPEND sources ${_sources})
    endforeach()
    
    if( NOT folder STREQUAL "")
        source_group(${folder} FILES ${sources})
    endif()

    #message("\nDEBUG: aux_source_directories: current_source_dir=${CMAKE_CURRENT_SOURCE_DIR} dirs=${ARGN}):\n${sources}\n")
    set(${var} ${${var}} ${sources} PARENT_SCOPE)
endfunction()

#!
# @brief Install targets for the runtime distribution
# @param[in] targets list of targets to install
#
function(install_runtime_targets)
    install(
        TARGETS ${ARGN}
        COMPONENT runtime
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )
endfunction()

#!
# @brief Install targets for the runtime distribution
# @param[in] dir destination directory relative to the root installation
# directory
# @param[in] targets list of files to install
#
function(install_runtime_files dir)
    install(
        FILES ${ARGN}
        DESTINATION ${dir}
        COMPONENT runtime
    )
endfunction()

#!
# @brief Install targets for the developer kit distribution
# @param[in] targets list of targets to install
#
function(install_devkit_targets)
    foreach(component devkit devkit-full)
        install(
            TARGETS ${ARGN}
            COMPONENT ${component}
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
        )
    endforeach()
endfunction()

#!
# @brief Install targets for the developer kit distribution
# @param[in] dir destination directory relative to the root installation
# directory
# @param[in] targets list of files to install
#
function(install_devkit_files dir)
    foreach(component devkit devkit-full)
        install(
            FILES ${ARGN}
            DESTINATION ${dir}
            COMPONENT ${component}
        )
    endforeach()
endfunction()


