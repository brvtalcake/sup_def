# TODO: Use PCHs (`target_precompile_headers`).

cmake_minimum_required(VERSION 3.24.0 FATAL_ERROR)
cmake_policy(SET CMP0144 NEW)

include(CMakeDependentOption)
include(CheckIPOSupported)
include(FindBoost)

add_compile_definitions(BUILDING_SUPDEF=1)
add_compile_options(-ftrack-macro-expansion=0)

if ($ENV{MAINTAINER_MODE})
    set(MAINTAINER_MODE $ENV{MAINTAINER_MODE})
else()
    set(MAINTAINER_MODE OFF)
endif()

if (${MAINTAINER_MODE} STREQUAL "ON")
    message(STATUS "Maintainer mode enabled")
    set(COMP_TOOLCHAIN_DIR "/usr/local/bin/")
    set(BINUTILS_TOOLCHAIN_DIR "/toolchains/native/x64/custom-")
    set(CMAKE_C_COMPILER "${COMP_TOOLCHAIN_DIR}gcc")
    set(CMAKE_CXX_COMPILER "${COMP_TOOLCHAIN_DIR}g++")
    #set(CMAKE_CXX_COMPILER "clang++")
    set(CMAKE_AR "${COMP_TOOLCHAIN_DIR}gcc-ar")
    set(CMAKE_RANLIB "${COMP_TOOLCHAIN_DIR}gcc-ranlib")
    set(CMAKE_LINKER "${BINUTILS_TOOLCHAIN_DIR}ld")
    set(CMAKE_NM "${COMP_TOOLCHAIN_DIR}gcc-nm")
    set(CMAKE_OBJCOPY "${BINUTILS_TOOLCHAIN_DIR}objcopy")
    set(CMAKE_OBJDUMP "${BINUTILS_TOOLCHAIN_DIR}objdump")
    set(CMAKE_STRIP "${BINUTILS_TOOLCHAIN_DIR}strip")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdiagnostics-color=always")

    message(STATUS "C compiler: ${CMAKE_C_COMPILER}")
    message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}")

    set(BOOST_VER_NEEDED "1.83.0")
else ()
    set(BOOST_VER_NEEDED "1.72.0")
endif()

if (NOT CMAKE_BUILD_TYPE)
    # Release with debug info
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build" FORCE)
    message(STATUS "Build type not specified: defaulting to ${CMAKE_BUILD_TYPE}")
else()
    message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
endif()

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_DEBUG_PYTHON OFF)
if (${MAINTAINER_MODE} STREQUAL "ON")
    set(BOOST_ROOT "/usr/local/")
    set(Boost_USE_STATIC_RUNTIME ON)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(Boost_USE_DEBUG_LIBS ON)
        set(Boost_USE_RELEASE_LIBS OFF)
        set(Boost_USE_DEBUG_RUNTIME ON)
    else()
        set(Boost_USE_DEBUG_LIBS OFF)
        set(Boost_USE_RELEASE_LIBS ON)
        set(Boost_USE_DEBUG_RUNTIME OFF)
    endif()
else()
    set(Boost_USE_DEBUG_RUNTIME OFF)
    set(Boost_USE_DEBUG_LIBS OFF)
    set(Boost_USE_RELEASE_LIBS ON)
endif()

set(PROJVER_MAJ 0)
set(PROJVER_MIN 1)
set(PROJVER_PATCH 0)

set(EXTERNAL_TOOL_NAME "supdef")

set(CMAKE_CXX_STANDARD 23)
#set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(SupDef VERSION ${PROJVER_MAJ}.${PROJVER_MIN}.${PROJVER_PATCH} LANGUAGES CXX C)

check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_NOT_SUPPORTED LANGUAGES CXX)

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

option(SUPDEF_STD_STACKTRACE "Use standard library stacktrace" OFF)
cmake_dependent_option(SUPDEF_BOOST_STACKTRACE "Use Boost stacktrace" ON "NOT SUPDEF_STD_STACKTRACE" OFF)

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