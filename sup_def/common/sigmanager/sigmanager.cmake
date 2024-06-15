include("${SUPDEF_PROJECT_CMAKE_DIR}/get_header_list.cmake")

set(
    SIGMANAGER_SOURCES_LIST

    "${CMAKE_CURRENT_LIST_DIR}/sigmanager.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/sigmanager.cpp"
)

get_header_list(
    OUTPUT_VAR SIGMANAGER_HEADERS_LIST
    SOURCES_LIST ${SIGMANAGER_SOURCES_LIST}
    VERIFY_EXISTS
)