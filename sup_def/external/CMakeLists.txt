file(GLOB_RECURSE SUPDEF_EXECUTABLE_SOURCES LIST_DIRECTORIES FALSE "*.h" "*.hpp" "*.c" "*.cpp")
file(GLOB_RECURSE SUPDEF_EXECUTABLE_HEADERS LIST_DIRECTORIES FALSE "*.h" ".hpp")
add_executable(${EXTERNAL_TOOL_NAME} ${SUPDEF_EXECUTABLE_SOURCES})
target_link_libraries(${EXTERNAL_TOOL_NAME} PUBLIC sdcommon sdthirdparty)
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