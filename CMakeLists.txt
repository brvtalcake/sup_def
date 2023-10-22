cmake_minimum_required(VERSION 3.24.0 FATAL_ERROR)

include(CMakeDependentOption)
include(CheckIPOSupported)
include(FindBoost)

if (NOT CMAKE_BUILD_TYPE)
    # Release with debug info
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build" FORCE)
    message(STATUS "Build type not specified: defaulting to ${CMAKE_BUILD_TYPE}")
else()
    message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
endif()

set(PROJVER_MAJ 0)
set(PROJVER_MIN 1)
set(PROJVER_PATCH 0)

set(EXTERNAL_TOOL_NAME "supdef")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(SupDef VERSION ${PROJVER_MAJ}.${PROJVER_MIN}.${PROJVER_PATCH} LANGUAGES CXX C)

check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_NOT_SUPPORTED LANGUAGES CXX)

find_package(Boost 1.74.0 REQUIRED)

if (NOT Boost_FOUND)
    message(FATAL_ERROR "Boost not found")
endif()

option(SUPDEF_BUILD_LIB_ONLY "Build only the common library" OFF)
option(SUPDEF_BUILD_TESTS "Build tests" OFF)

if (IPO_SUPPORTED AND ("${CMAKE_BUILD_TYPE}" STREQUAL "Release"))
    option(SUPDEF_ENABLE_LTO "Enable link-time optimization" ON)
    message(STATUS "Link-time optimization is supported by the compiler")
elseif (IPO_SUPPORTED AND NOT (${CMAKE_BUILD_TYPE} STREQUAL "Release"))
    option(SUPDEF_ENABLE_LTO "Enable link-time optimization" OFF)
    message(WARNING "Link-time optimization is only supported in Release mode")
else()
    option(SUPDEF_ENABLE_LTO "Enable link-time optimization" OFF)
    message(WARNING "Link-time optimization is not supported by the compiler: ${IPO_NOT_SUPPORTED}")
endif()
# SUPDEF_BUILD_EXTERNAL is only available if SUPDEF_BUILD_LIB_ONLY is OFF.
# Same goes for SUPDEF_BUILD_GCC_PLUGIN and SUPDEF_BUILD_CLANG_PLUGIN
cmake_dependent_option(SUPDEF_BUILD_EXTERNAL "Build external command-line tool" ON "NOT SUPDEF_BUILD_LIB_ONLY" OFF)
cmake_dependent_option(SUPDEF_BUILD_GCC_PLUGIN "Build GCC plugin" OFF "NOT SUPDEF_BUILD_LIB_ONLY" OFF)
cmake_dependent_option(SUPDEF_BUILD_CLANG_PLUGIN "Build Clang plugin" OFF "NOT SUPDEF_BUILD_LIB_ONLY" OFF)

if (SUPDEF_ENABLE_LTO AND IPO_SUPPORTED AND ("${CMAKE_BUILD_TYPE}" STREQUAL "Release"))
    message(STATUS "Link-time optimization enabled")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(STATUS "Link-time optimization disabled")
endif()

if (SUPDEF_BUILD_LIB_ONLY)
    message(STATUS "Building only the common library")
endif()

if (SUPDEF_BUILD_EXTERNAL)
    message(STATUS "Building external command-line tool")
endif()

if (SUPDEF_BUILD_GCC_PLUGIN)
    message(STATUS "Building GCC plugin")
endif()

if (SUPDEF_BUILD_CLANG_PLUGIN)
    message(STATUS "Building Clang plugin")
endif()

if (SUPDEF_BUILD_TESTS)
    message(STATUS "Building tests")
endif()

add_subdirectory("./sup_def")

if (SUPDEF_BUILD_TESTS)
    add_subdirectory("./tests")
endif()