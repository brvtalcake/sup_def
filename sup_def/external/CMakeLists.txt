file(GLOB_RECURSE SUPDEF_EXECUTABLE_SOURCES LIST_DIRECTORIES FALSE "*.h" "*.hpp" "*.c" "*.cpp")
add_executable(${EXTERNAL_TOOL_NAME} ${SUPDEF_EXECUTABLE_SOURCES})
target_link_libraries(${EXTERNAL_TOOL_NAME} PUBLIC sdcommon)
# Add custom command to copy produced executable at the top directory
add_custom_command(TARGET ${EXTERNAL_TOOL_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${EXTERNAL_TOOL_NAME}> ${CMAKE_BINARY_DIR}/bin/${EXTERNAL_TOOL_NAME}
    COMMENT "Copying ${EXTERNAL_TOOL_NAME} to ${CMAKE_BINARY_DIR}/bin/${EXTERNAL_TOOL_NAME}"
)