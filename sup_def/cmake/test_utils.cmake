include_guard(GLOBAL)

function(get_test_sources_variable var test_suite_name)
    set(${var} ${test_suite_name}_SOURCES PARENT_SCOPE)
endfunction()

function(add_sources_to_test_suite test_suite_name)
    get_test_sources_variable(TEST_CASE_SOURCES ${test_suite_name})
    foreach(source ${ARGN})
        list(APPEND ${TEST_CASE_SOURCES} ${source})
    endforeach()
    set(${test_suite_name}_SOURCES ${${TEST_CASE_SOURCES}} PARENT_SCOPE)
endfunction()

# To be called at the end, when all other sources are registered
function(add_test_suite_target name)
    get_test_sources_variable(TEST_CASE_SOURCES ${name})
    add_executable(${name} ${TEST_CASE_SOURCES} ${name}.cpp)
    target_link_libraries(${name} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
    target_link_libraries(${name} PUBLIC sdcommon sdthirdparty)
    target_link_options(${name} PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wl,--gc-sections>)
    add_test(${name} COMMAND "./${name}")
endfunction()

function(add_test_foreach_source)
    foreach (test ${SUPDEF_TEST_EXECUTABLES})
        list(APPEND REGISTERED_TEST_EXECUTABLES ${test})
    endforeach()
    foreach (target ${SUPDEF_TEST_TARGETS})
        list(APPEND REGISTERED_TEST_TARGETS ${target})
    endforeach()
    foreach (test_src ${ARGN})
        get_filename_component(test_name ${test_src} NAME_WE)
        add_executable(${test_name} ${test_src})
        target_link_libraries(${test_name} sdthirdparty sdcommon)
        #[[ add_test(NAME ${test_name} COMMAND ${test_name}) ]]
        # Copy test to ${CMAKE_SOURCE_DIR}/bin/tests
        add_custom_command(TARGET ${test_name} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${test_name}> ${CMAKE_SOURCE_DIR}/bin/tests
                COMMENT "Copying test ${test_name} to ${CMAKE_SOURCE_DIR}/bin/tests"
        )
        list(APPEND REGISTERED_TEST_EXECUTABLES ${CMAKE_SOURCE_DIR}/bin/tests/${test_name})
        list(APPEND REGISTERED_TEST_TARGETS ${test_name})
        message(STATUS "Added test ${test_name} from source ${test_src}")
    endforeach()
    set(SUPDEF_TEST_EXECUTABLES ${REGISTERED_TEST_EXECUTABLES} PARENT_SCOPE)
    set(SUPDEF_TEST_TARGETS ${REGISTERED_TEST_TARGETS} PARENT_SCOPE)
endfunction()

function(create_test_command var)
    set(OLD_SUPDEF_TEST_EXECUTABLES ${SUPDEF_TEST_EXECUTABLES})
    list(POP_FRONT SUPDEF_TEST_EXECUTABLES TEST_EXECUTABLE)
    set(OUT_VAR "${TEST_EXECUTABLE} ${SUPDEF_TEST_EXECUTABLES_ARGS}")
    foreach (test ${SUPDEF_TEST_EXECUTABLES})
        set(OUT_VAR "${OUT_VAR} && ${test} ${SUPDEF_TEST_EXECUTABLES_ARGS}")
    endforeach()
    set(${var} ${OUT_VAR} PARENT_SCOPE)
    set(SUPDEF_TEST_EXECUTABLES ${OLD_SUPDEF_TEST_EXECUTABLES} PARENT_SCOPE)
endfunction()
