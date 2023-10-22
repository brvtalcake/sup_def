file(GLOB_RECURSE LIBSDCOMMON_SOURCES LIST_DIRECTORIES FALSE "*.h" ".hpp" "*.c" "*.cpp")
add_library(sdcommon STATIC ${LIBSDCOMMON_SOURCES})
target_include_directories(sdcommon PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(sdcommon PUBLIC sdthirdparty)
