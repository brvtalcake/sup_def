include_guard(GLOBAL)

# Usage: get_header_list(OUTPUT_VAR <output_var> SOURCES_LIST <sources_list1> [<sources_list2> ...] [VERIFY_EXISTS])
function (get_header_list)
    set(options VERIFY_EXISTS)
    set(oneValueArgs OUTPUT_VAR)
    set(multiValueArgs SOURCES_LIST)
    cmake_parse_arguments(GET_HEADER_LIST "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(TMP_HEADERS_LIST)
    foreach(source_file ${GET_HEADER_LIST_SOURCES_LIST})
        if (NOT EXISTS ${source_file})
            if (GET_HEADER_LIST_VERIFY_EXISTS)
                message(FATAL_ERROR "Source file ${source_file} does not exist")
            endif()
        endif()
        if (NOT (${source_file} MATCHES "^.*\.(c|cpp|cxx|cc)$"))
            list(APPEND TMP_HEADERS_LIST ${source_file})
        endif()
    endforeach()

    set(
        ${GET_HEADER_LIST_OUTPUT_VAR}

        ${TMP_HEADERS_LIST}

        PARENT_SCOPE
    )
endfunction()