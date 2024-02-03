include(FindPython3)

find_package(Python3 COMPONENTS Interpreter REQUIRED)

set(WANTED_GNULIB_MODULES "stdckdint" #[[ "alloca" "malloca" "memset_explicit" ]])
add_custom_target(
    build_gnulib_modules
    ALL
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/gen_gnulib ${WANTED_GNULIB_MODULES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    COMMENT "Generating gnulib modules"
    VERBATIM
    COMMAND_EXPAND_LISTS
)
set(GNULIB_INCDIRS ${CMAKE_CURRENT_LIST_DIR}/ ${CMAKE_CURRENT_LIST_DIR}/gnulib_build/ ${CMAKE_CURRENT_LIST_DIR}/gnulib_build/gllib/ PARENT_SCOPE)
set(GNULIB_LINKDIRS "${CMAKE_CURRENT_LIST_DIR}/gnulib_build/gllib/" PARENT_SCOPE)
set(GNULIB_LIBS "gnu" PARENT_SCOPE) # libgnu.a