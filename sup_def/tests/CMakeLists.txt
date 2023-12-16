include("../cmake/test_utils.cmake")
include(FindBoost)

set(SUPDEF_TEST_EXECUTABLES_ARGS " --log_level=test_suite --detect_memory_leaks" CACHE STRING "Arguments for test executables")
set(SUPDEF_TEST_EXECUTABLES "") # List of test executables
set(SUPDEF_TEST_TARGETS "") # List of test targets

#[[
find_package(Boost ${BOOST_VER_NEEDED} REQUIRED COMPONENTS unit_test_framework HINTS "/usr/include/" "/usr/local/include/" "/usr/include/boost/" "/usr/local/include/boost/")
if (NOT Boost_FOUND)
    message(FATAL_ERROR "Boost not found")
endif()
if (NOT Boost_UNIT_TEST_FRAMEWORK_FOUND)
    message(FATAL_ERROR "Boost unit test framework not found")
endif()
]]

add_subdirectory("./common")
file(GLOB sdcommon_test_SOURCES_MAIN LIST_DIRECTORIES FALSE "test_common*.cpp")
add_test_foreach_source(${sdcommon_test_SOURCES_MAIN})

if (NOT ${SUPDEF_BUILD_LIB_ONLY})

    if (${SUPDEF_BUILD_EXTERNAL} AND IS_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/external)
        add_subdirectory("./external")
    endif()

    if (${SUPDEF_BUILD_GCC_PLUGIN} AND IS_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/gcc)
        add_subdirectory("./gcc")
    endif()

    if (${SUPDEF_BUILD_CLANG_PLUGIN} AND IS_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/clang)
        add_subdirectory("./clang")
    endif()

endif()

create_test_command(test_command)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/test_runner.sh "#!/bin/bash\n${test_command}\n")
exec_program("chmod" ARGS "+x ${CMAKE_CURRENT_BINARY_DIR}/test_runner.sh")

# Add a `tests` target to build and execute all tests
add_custom_target(tests
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/test_runner.sh
    DEPENDS ${SUPDEF_TEST_TARGETS}
    COMMENT "Build and execute all tests"
    #[[ VERBATIM ]]
)