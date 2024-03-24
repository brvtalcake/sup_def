file(GLOB_RECURSE SUPDEF_EXECUTABLE_SOURCES LIST_DIRECTORIES FALSE "*.h" "*.hpp" "*.c" "*.cpp")
file(GLOB_RECURSE SUPDEF_EXECUTABLE_HEADERS LIST_DIRECTORIES FALSE "*.h" ".hpp")
foreach (f ${SUPDEF_EXECUTABLE_SOURCES})
    if (NOT f MATCHES ".*main.*")
        list(APPEND sdexternal_sources ${f})
    endif()
endforeach()
add_library(sdexternal STATIC ${sdexternal_sources})
target_link_libraries(sdexternal PUBLIC libsdcommon::external sdthirdparty)
target_compile_definitions(sdexternal PRIVATE COMPILING_EXTERNAL=1)

list(APPEND sdexternal_sources "${CMAKE_CURRENT_LIST_DIR}/main.cpp")
add_executable(${EXTERNAL_TOOL_NAME} ${sdexternal_sources})
target_link_libraries(${EXTERNAL_TOOL_NAME} PUBLIC libsdcommon::external sdexternal sdthirdparty)
target_compile_definitions(${EXTERNAL_TOOL_NAME} PRIVATE COMPILING_EXTERNAL=1)
# Add custom command to copy produced executable at the top directory
add_custom_command(TARGET ${EXTERNAL_TOOL_NAME} POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${EXTERNAL_TOOL_NAME}> ${CMAKE_CURRENT_LIST_DIR}/../../bin/${EXTERNAL_TOOL_NAME}
COMMENT "Copying ${EXTERNAL_TOOL_NAME} to bin directory (${CMAKE_CURRENT_LIST_DIR}/../../bin/${EXTERNAL_TOOL_NAME})"
)
# (previous directory where it was copied was ${CMAKE_BINARY_DIR}/bin/${EXTERNAL_TOOL_NAME})

# Add --gc-* flags to link options
target_link_options(${EXTERNAL_TOOL_NAME} PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wl,--gc-sections>)
#target_precompile_headers(${EXTERNAL_TOOL_NAME} PRIVATE ${SUPDEF_EXECUTABLE_HEADERS})