include_guard(GLOBAL)

function (assert_fail message)
    message(FATAL_ERROR "Assertion failed: ${message}")
endfunction()

function (assert_eq expected actual)
    if (NOT (expected EQUAL actual))
        assert_fail("assert_eq: expected ${expected}, got ${actual}")
    endif()
endfunction()

function (assert_neq expected actual)
    if (expected EQUAL actual)
        assert_fail("assert_neq: expected ${expected}, got ${actual}")
    endif()
endfunction()

function (assert_gt expected actual)
    if (NOT (actual GREATER expected))
        assert_fail("assert_gt: expected greater than ${expected}, got ${actual}")
    endif()
endfunction()

function (assert_geq expected actual)
    if (NOT (actual GREATER_EQUAL expected))
        assert_fail("assert_geq: expected greater or equal to ${expected}, got ${actual}")
    endif()
endfunction()

function (assert_lt expected actual)
    if (NOT (actual LESS expected))
        assert_fail("assert_lt: expected less than ${expected}, got ${actual}")
    endif()
endfunction()

function (assert_leq expected actual)
    if (NOT (actual LESS_EQUAL expected))
        assert_fail("assert_leq: expected less or equal to ${expected}, got ${actual}")
    endif()
endfunction()

function (assert_list_size_eq list expected_size)
    list(LENGTH list list_size)
    if (NOT (list_size EQUAL expected_size))
        assert_fail("assert_list_size_eq: expected size ${expected_size}, got ${list_size}")
    endif()
endfunction()

function (assert_argc_eq expected_size)
    set(size 0)
    foreach (arg ${ARGN})
        math(EXPR size "${size} + 1")
    endforeach()
    if (NOT (size EQUAL expected_size))
        assert_fail("assert_argc_eq: expected size ${expected_size}, got ${size}")
    endif()
endfunction()
