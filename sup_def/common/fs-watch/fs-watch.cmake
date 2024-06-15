include("${SUPDEF_PROJECT_CMAKE_DIR}/get_header_list.cmake")

set(
    FS_WATCH_SOURCES_LIST

    "${CMAKE_CURRENT_LIST_DIR}/fs-watch.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/fs-watch.cpp"
)

get_header_list(
    OUTPUT_VAR FS_WATCH_HEADERS_LIST
    SOURCES_LIST ${FS_WATCH_SOURCES_LIST}
    VERIFY_EXISTS
)