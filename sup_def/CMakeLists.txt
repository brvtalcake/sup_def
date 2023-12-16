add_subdirectory("./third_party")

add_subdirectory("./common")

if (NOT ${SUPDEF_BUILD_LIB_ONLY})

    if (${SUPDEF_BUILD_EXTERNAL})
        add_subdirectory("./external")
    endif()

    if (${SUPDEF_BUILD_GCC_PLUGIN})
        add_subdirectory("./gcc")
    endif()

    if (${SUPDEF_BUILD_CLANG_PLUGIN})
        add_subdirectory("./clang")
    endif()

    if (${SUPDEF_BUILD_TESTS})
        enable_testing()
        add_subdirectory("./tests")
    endif()

endif()