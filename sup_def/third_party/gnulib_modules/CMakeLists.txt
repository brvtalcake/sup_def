include(FindPython3)

find_package(Python3 COMPONENTS Interpreter REQUIRED)

set(WANTED_GNULIB_MODULES "stdckdint" #[[ "alloca" "malloca" "memset_explicit" ]])

# TODO: Run the build_gnulib_modules target only when processing the CMakeLists.txt file
# i.e. it should rather be a `execute_process` command
#[[ add_custom_target(
    build_gnulib_modules
    ALL
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/gen_gnulib ${WANTED_GNULIB_MODULES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    COMMENT "Generating gnulib modules"
    VERBATIM
    COMMAND_EXPAND_LISTS
) ]]
execute_process(
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/gen_gnulib ${WANTED_GNULIB_MODULES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    RESULT_VARIABLE GNULIB_GEN_RESULT
)
if(NOT GNULIB_GEN_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to generate gnulib modules")
endif()

set(GNULIB_INCDIRS ${CMAKE_CURRENT_LIST_DIR}/ ${CMAKE_CURRENT_LIST_DIR}/gnulib_build/ ${CMAKE_CURRENT_LIST_DIR}/gnulib_build/gllib/ PARENT_SCOPE)
set(GNULIB_LINKDIRS "${CMAKE_CURRENT_LIST_DIR}/gnulib_build/gllib/" PARENT_SCOPE)
set(GNULIB_LIBS "gnu" PARENT_SCOPE) # libgnu.a