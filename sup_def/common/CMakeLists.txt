include("${SUPDEF_PROJECT_CMAKE_DIR}/report_target_properties.cmake")

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/unistreams/impl/compiled/")

include("${CMAKE_CURRENT_LIST_DIR}/supdefcommon.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/unistreams/unistreams.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/fs-watch/fs-watch.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/sigmanager/sigmanager.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/threadkiller/threadkiller.cmake")

# Append SUPDEFCOMMON_SOURCES_LIST, UNISTREAMS_SOURCES_LIST, ... etc, to LIBSDCOMMON_SOURCES
set(LIBSDCOMMON_SOURCES
    ${SUPDEFCOMMON_SOURCES_LIST}
    ${UNISTREAMS_SOURCES_LIST}
    ${FS_WATCH_SOURCES_LIST}
    ${SIGMANAGER_SOURCES_LIST}
    ${THREADKILLER_SOURCES_LIST}
)
set(LIBSDCOMMON_HEADERS
    ${SUPDEFCOMMON_HEADERS_LIST}
    ${UNISTREAMS_HEADERS_LIST}
    ${FS_WATCH_HEADERS_LIST}
    ${SIGMANAGER_HEADERS_LIST}
    ${THREADKILLER_HEADERS_LIST}
)

#file(GLOB_RECURSE LIBSDCOMMON_SOURCES LIST_DIRECTORIES FALSE "*.h" ".hpp" "*.i" "*.ipp" "*.tpp" "*.c" "*.cpp")
#file(GLOB_RECURSE LIBSDCOMMON_HEADERS LIST_DIRECTORIES FALSE "*.h" ".hpp" "*.i" "*.ipp" "*.tpp")


add_library(sdcommon STATIC ${LIBSDCOMMON_SOURCES})
target_link_libraries(sdcommon PUBLIC sdthirdparty)
#[[ target_include_directories(sdcommon PRIVATE ${CMAKE_CURRENT_LIST_DIR}) ]]
#target_precompile_headers(sdcommon PRIVATE ${LIBSDCOMMON_HEADERS})


#[[ if(NOT ${SUPDEF_BUILD_LIB_ONLY})
    add_library(sdcommon_test STATIC ${LIBSDCOMMON_SOURCES})
    target_include_directories(sdcommon_test PRIVATE ${CMAKE_CURRENT_LIST_DIR})
    target_link_libraries(sdcommon_test PUBLIC sdthirdparty_test)
    # Change compilation GCC options to SUPDEF_TEST_COMPILE_FLAGS, and override the selected CMAKE_BUILD_TYPE
    set_target_properties(sdcommon_test PROPERTIES COMPILE_FLAGS ${SUPDEF_TEST_COMPILE_FLAGS})
    set_target_properties(sdcommon_test PROPERTIES LINK_FLAGS ${SUPDEF_TEST_LINK_FLAGS})
endif() ]]


add_library(sdcommon_external STATIC ${LIBSDCOMMON_SOURCES})
target_link_libraries(sdcommon_external PUBLIC sdthirdparty)
target_compile_definitions(sdcommon_external PUBLIC COMPILING_EXTERNAL=1)

add_library(libsdcommon::base ALIAS sdcommon)
add_library(libsdcommon::external ALIAS sdcommon_external)

add_library(libsdcommon::test::common ALIAS sdcommon)
add_library(libsdcommon::test::external ALIAS sdcommon_external)