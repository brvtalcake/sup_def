include_guard(GLOBAL)

function(expand_prop_values var target prop)
    if(DEFINED ${target}_${prop} AND NOT "${${target}_${prop}}" STREQUAL "")
        foreach(value ${${target}_${prop}})
            string(APPEND ${var} "${value} ")
        endforeach()
    endif()
    # If ${ARGN} is not empty, recurse to process the other properties
    if(NOT "${ARGN}" STREQUAL "")
        expand_prop_values(${var} ${target} ${ARGN})
    endif()
    if(DEFINED ${var} AND ${var})
        set(${var} ${${var}} PARENT_SCOPE)
    endif()
endfunction()

function(print_if_defined whatisvar var)
    if(DEFINED ${var} AND ${var})
        message(NOTICE "${whatisvar}: ${${var}}")
    else()
        message(NOTICE "${whatisvar}: not defined")
    endif()
endfunction()

function(print_props target)
    message(NOTICE "-------------------")
    message(NOTICE "Target: " ${target})
    print_if_defined("`-Werror`" ${target}_COMPILE_WARNING_AS_ERROR_VALUES)
    print_if_defined("C Standard" ${target}_C_STANDARD_VALUES)
    print_if_defined("CXX Standard" ${target}_CXX_STANDARD_VALUES)
    print_if_defined("Compile Definitions" ${target}_COMPILE_DEFINITIONS_VALUES)
    print_if_defined("Compile Flags" ${target}_COMPILE_FLAGS_VALUES)
    print_if_defined("Compile Options" ${target}_COMPILE_OPTIONS_VALUES)
    print_if_defined("Link Libraries" ${target}_LINK_LIBRARIES_VALUES)
    print_if_defined("Link Flags" ${target}_LINK_FLAGS_VALUES)
    print_if_defined("Link Options" ${target}_LINK_OPTIONS_VALUES)
    print_if_defined("Link Directories" ${target}_LINK_DIRECTORIES_VALUES)
    print_if_defined("Include Directories" ${target}_INCLUDE_DIRS_VALUES)
    message(NOTICE "-------------------")
endfunction()


function(report_target_properties target)
    get_target_property(${target}_COMPILE_WARNING_AS_ERROR ${target} COMPILE_WARNING_AS_ERROR)
    expand_prop_values(${target}_COMPILE_WARNING_AS_ERROR_VALUES ${target} COMPILE_WARNING_AS_ERROR)

    get_target_property(${target}_C_STANDARD ${target} C_STANDARD)
    expand_prop_values(${target}_C_STANDARD_VALUES ${target} C_STANDARD)

    get_target_property(${target}_CXX_STANDARD ${target} CXX_STANDARD)
    expand_prop_values(${target}_CXX_STANDARD_VALUES ${target} CXX_STANDARD)

    get_target_property(${target}_COMPILE_DEFINITIONS ${target} COMPILE_DEFINITIONS)
    get_target_property(${target}_INTERFACE_COMPILE_DEFINITIONS ${target} INTERFACE_COMPILE_DEFINITIONS)
    expand_prop_values(${target}_COMPILE_DEFINITIONS_VALUES ${target} COMPILE_DEFINITIONS INTERFACE_COMPILE_DEFINITIONS)

    get_target_property(${target}_COMPILE_FLAGS ${target} COMPILE_FLAGS)
    expand_prop_values(${target}_COMPILE_FLAGS_VALUES ${target} COMPILE_FLAGS)

    get_target_property(${target}_COMPILE_OPTIONS ${target} COMPILE_OPTIONS)
    get_target_property(${target}_INTERFACE_COMPILE_OPTIONS ${target} INTERFACE_COMPILE_OPTIONS)
    expand_prop_values(${target}_COMPILE_OPTIONS_VALUES ${target} COMPILE_OPTIONS INTERFACE_COMPILE_OPTIONS)

    get_target_property(${target}_LINK_LIBRARIES ${target} LINK_LIBRARIES)
    get_target_property(${target}_INTERFACE_LINK_LIBRARIES ${target} INTERFACE_LINK_LIBRARIES)
    expand_prop_values(${target}_LINK_LIBRARIES_VALUES ${target} LINK_LIBRARIES INTERFACE_LINK_LIBRARIES)
    
    get_target_property(${target}_LINK_FLAGS ${target} LINK_FLAGS)
    expand_prop_values(${target}_LINK_FLAGS_VALUES ${target} LINK_FLAGS)

    get_target_property(${target}_LINK_OPTIONS ${target} LINK_OPTIONS)
    get_target_property(${target}_INTERFACE_LINK_OPTIONS ${target} INTERFACE_LINK_OPTIONS)
    expand_prop_values(${target}_LINK_OPTIONS_VALUES ${target} LINK_OPTIONS INTERFACE_LINK_OPTIONS)

    get_target_property(${target}_LINK_DIRECTORIES ${target} LINK_DIRECTORIES)
    get_target_property(${target}_INTERFACE_LINK_DIRECTORIES ${target} INTERFACE_LINK_DIRECTORIES)
    expand_prop_values(${target}_LINK_DIRECTORIES_VALUES ${target} LINK_DIRECTORIES INTERFACE_LINK_DIRECTORIES)
    
    get_target_property(${target}_INCLUDE_DIRS ${target} INCLUDE_DIRECTORIES)
    get_target_property(${target}_INTERFACE_INCLUDE_DIRS ${target} INTERFACE_INCLUDE_DIRECTORIES)
    expand_prop_values(${target}_INCLUDE_DIRS_VALUES ${target} INCLUDE_DIRECTORIES INTERFACE_INCLUDE_DIRECTORIES)

    print_props(${target})
endfunction()