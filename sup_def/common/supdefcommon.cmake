include("${SUPDEF_PROJECT_CMAKE_DIR}/get_header_list.cmake")

set(
    SUPDEFCOMMON_SOURCES_LIST

    "${CMAKE_CURRENT_LIST_DIR}/config.h"
    "${CMAKE_CURRENT_LIST_DIR}/end_header.h"
    "${CMAKE_CURRENT_LIST_DIR}/engine.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/engine.tpp"
    "${CMAKE_CURRENT_LIST_DIR}/error_printer.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/exp_inst.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/parsed_char.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/parser.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/parser.tpp"
    "${CMAKE_CURRENT_LIST_DIR}/pragmas.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/pragmas.tpp"
    "${CMAKE_CURRENT_LIST_DIR}/start_header.h"
    "${CMAKE_CURRENT_LIST_DIR}/sup_def.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/thread_pool.cpp"
    
    "${CMAKE_CURRENT_LIST_DIR}/util/concepts.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/util/constructors.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/util/convert_old_impl.ipp"
    "${CMAKE_CURRENT_LIST_DIR}/util/destructors.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/util/engine.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/util/errors.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/util/exception.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/util/fwd_decls.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/util/platform.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/util/static_allocator.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/util/util.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/util/util.hpp"
)

get_header_list(
    OUTPUT_VAR SUPDEFCOMMON_HEADERS_LIST
    SOURCES_LIST ${SUPDEFCOMMON_SOURCES_LIST}
    VERIFY_EXISTS
)
