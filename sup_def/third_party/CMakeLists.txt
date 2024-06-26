include(${CMAKE_CURRENT_LIST_DIR}/../cmake/report_target_properties.cmake)

include(FetchContent)

find_package(Boost ${BOOST_VER_NEEDED} REQUIRED)

if (NOT Boost_FOUND)
    message(FATAL_ERROR "Boost not found")
endif()

# TODO: Add a FetchContent_Declare for `simdutf` and link sdthirdparty to it
FetchContent_Declare(
    simdutf
    SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/simdutf"
)
FetchContent_MakeAvailable(simdutf)

FetchContent_Declare(
    magic_enum
    SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/magic_enum"
)
FetchContent_MakeAvailable(magic_enum)

add_subdirectory("./empty_macro")
add_subdirectory("./map")
add_subdirectory("./gnulib_modules")
add_library(sdthirdparty INTERFACE)
add_dependencies(sdthirdparty build_gnulib_modules)
target_link_libraries(sdthirdparty INTERFACE sdthirdparty_empty_macro sdthirdparty_map)
target_link_libraries(sdthirdparty INTERFACE simdutf::simdutf)
target_link_libraries(sdthirdparty INTERFACE magic_enum::magic_enum)
target_include_directories(sdthirdparty INTERFACE "${CMAKE_CURRENT_LIST_DIR}/")
target_include_directories(sdthirdparty INTERFACE "${CMAKE_CURRENT_LIST_DIR}/chaos-pp/")
target_include_directories(sdthirdparty INTERFACE "${CMAKE_CURRENT_LIST_DIR}/../")
target_include_directories(sdthirdparty INTERFACE "${CMAKE_CURRENT_LIST_DIR}/../../")
target_include_directories(sdthirdparty INTERFACE "${CMAKE_CURRENT_LIST_DIR}/patterns/include/")
target_include_directories(sdthirdparty INTERFACE "${CMAKE_CURRENT_LIST_DIR}/magic_enum/include/")

target_include_directories(sdthirdparty INTERFACE ${GNULIB_INCDIRS})
target_link_libraries(sdthirdparty INTERFACE ${GNULIB_LIBS})
target_link_directories(sdthirdparty INTERFACE ${GNULIB_LINKDIRS})
#[[ link_directories(BEFORE ${GNULIB_LIBDIRS}) ]]

# Include Boost
target_include_directories(sdthirdparty INTERFACE "/${Boost_INCLUDE_DIRS}/")
target_compile_definitions(sdthirdparty INTERFACE SUPDEF_DEBUG=$<IF:$<NOT:$<CONFIG:Release>>,1,0>)
target_compile_definitions(sdthirdparty INTERFACE SUPDEF_ADDITIONAL_CHECKS=1)
target_compile_definitions(sdthirdparty INTERFACE SUPDEF_REMOVE_IF_0_BLOCKS=$<IF:$<NOT:$<CONFIG:Release>>,1,0>)
target_compile_definitions(sdthirdparty INTERFACE _GLIBCXX_CONCEPT_CHECKS=1 _GLIBCXX_ASSERTIONS=1)
if (FALSE)
    target_compile_definitions(sdthirdparty INTERFACE _GLIBCXX_DEBUG=1)
elseif($<NOT:$<CONFIG:Release>>)
    target_compile_definitions(sdthirdparty INTERFACE _GLIBCXX_DEBUG=1)
endif()
target_compile_definitions(sdthirdparty INTERFACE _GNU_SOURCE=1)
target_compile_definitions(sdthirdparty INTERFACE _FORTIFY_SOURCE=2)
target_compile_definitions(sdthirdparty INTERFACE _LARGEFILE_SOURCE=1)
target_compile_definitions(sdthirdparty INTERFACE _LARGEFILE64_SOURCE=1)
target_compile_definitions(sdthirdparty INTERFACE _FILE_OFFSET_BITS=64)
target_compile_definitions(sdthirdparty INTERFACE _TIME_BITS=64)
target_compile_options(sdthirdparty INTERFACE -fconcepts-diagnostics-depth=10)

# TODO: Remove this workaround when GCC bug is fixed, and fix other bugs in my own code
target_compile_definitions(sdthirdparty INTERFACE SUPDEF_WORKAROUND_GCC_INTERNAL_ERROR=0)

if (${SUPDEF_CREATOR_MODE} STREQUAL "ON")
    # TODO: Add other options I like to use (-fweb, -fgraphite, -fgraphite-identity, ...)
    target_compile_options(sdthirdparty INTERFACE -Wall -Wextra -march=alderlake -static-libgcc -static-libstdc++)
    # Link manually to libiconv
    target_link_libraries(sdthirdparty INTERFACE iconv)
endif()

target_compile_options(sdthirdparty INTERFACE $<IF:$<NOT:$<CONFIG:Release>>,-ggdb3,>)

set(
    STDCXX_BACKTRACE_TEST_PROG
"
#include <iostream>
#include <stacktrace>
#include <cassert>
int nested_func(int c)
{ std::cout << std::stacktrace::current() << '\\n'; return c + 1; }
int func(int b)
{ return nested_func(b + 1); }
int main()
{
    assert(func(777) == 779); 
#if _GLIBCXX_HAVE_STACKTRACE
    return 1;
#else
    return 0;
#endif
}
"
)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/backtrace_support/stdcxx_backtrace_test.cpp" "${STDCXX_BACKTRACE_TEST_PROG}")
# Try to compile with `${CMAKE_CXX_COMPILER} -std=gnu++23 -lstdc++_libbacktrace -o stdcxx_backtrace_test stdcxx_backtrace_test.cpp`
# If no error at link-time and if 1 returned, then we can use C++23 stacktraces
function(is_cxx23_stacktrace_available RESULT)
    execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} -std=gnu++23 -o stdcxx_backtrace_test stdcxx_backtrace_test.cpp -lstdc++_libbacktrace
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/backtrace_support"
        RESULT_VARIABLE STDCXX_BACKTRACE_TEST_RESULT
        OUTPUT_QUIET
        ERROR_QUIET
    )
    if (NOT STDCXX_BACKTRACE_TEST_RESULT EQUAL 0)
        message(STATUS "C++23 stacktraces not available")
        set(${RESULT} 0 PARENT_SCOPE)
        return()
    endif()
    execute_process(
        COMMAND "${CMAKE_CURRENT_BINARY_DIR}/backtrace_support/stdcxx_backtrace_test"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/backtrace_support"
        RESULT_VARIABLE STDCXX_BACKTRACE_TEST_RESULT
        OUTPUT_QUIET
        ERROR_QUIET
    )
    if (NOT STDCXX_BACKTRACE_TEST_RESULT EQUAL 1)
        message(STATUS "C++23 stacktraces not available")
        set(${RESULT} 0 PARENT_SCOPE)
        return()
    endif()
    message(STATUS "C++23 stacktraces available")
    set(${RESULT} 1 PARENT_SCOPE)
    return()
endfunction()

function(is_boost_stacktrace_available RESULT)
    # Try to find Boost.Stacktrace
    find_package(Boost ${BOOST_VER_NEEDED} REQUIRED COMPONENTS stacktrace_basic HINTS "/usr/include/" "/usr/local/include/" "/usr/include/boost/" "/usr/local/include/boost/")
    if (NOT Boost_stacktrace_basic_FOUND)
        message(STATUS "Boost.Stacktrace not available")
        set(${RESULT} 0 PARENT_SCOPE)
        return()
    endif()
    message(STATUS "Boost.Stacktrace available")
    set(${RESULT} 1 PARENT_SCOPE)
    return()
endfunction()

set(CXX23_STACKTRACE_AVAILABLE 0)
set(BOOST_STACKTRACE_AVAILABLE 0)
is_cxx23_stacktrace_available(CXX23_STACKTRACE_AVAILABLE)
is_boost_stacktrace_available(BOOST_STACKTRACE_AVAILABLE)

if (CXX23_STACKTRACE_AVAILABLE AND NOT SUPDEF_BOOST_STACKTRACE)
    target_compile_definitions(sdthirdparty INTERFACE SUPDEF_USE_CXX23_STACKTRACE=1)
    target_link_options(sdthirdparty INTERFACE -lstdc++_libbacktrace)
elseif (BOOST_STACKTRACE_AVAILABLE AND NOT SUPDEF_STD_STACKTRACE)
    # TODO: Also search for WinDbg_Cached implementation
    find_package(Boost ${BOOST_VER_NEEDED} COMPONENTS
        stacktrace_addr2line stacktrace_windbg stacktrace_noop stacktrace_windebug stacktrace_backtrace stacktrace_windbg_cached stacktrace_basic
        HINTS "/usr/include/" "/usr/local/include/" "/usr/include/boost/" "/usr/local/include/boost/")

    target_include_directories(sdthirdparty INTERFACE "/${Boost_INCLUDE_DIRS}/")
    target_link_libraries(sdthirdparty INTERFACE ${Boost_LIBRARIES})

    if (Boost_stacktrace_backtrace_FOUND)
        message(STATUS "Boost.Stacktrace backtrace available")
        target_compile_definitions(sdthirdparty INTERFACE SUPDEF_HAVE_BOOST_STACKTRACE_BACKTRACE=1)
        target_compile_definitions(sdthirdparty INTERFACE BOOST_STACKTRACE_USE_BACKTRACE=1)
    endif()

    if (Boost_stacktrace_addr2line_FOUND)
        message(STATUS "Boost.Stacktrace addr2line available")
        target_compile_definitions(sdthirdparty INTERFACE SUPDEF_HAVE_BOOST_STACKTRACE_ADDR2LINE=1)
        if (NOT Boost_stacktrace_backtrace_FOUND)
            target_compile_definitions(sdthirdparty INTERFACE BOOST_STACKTRACE_USE_ADDR2LINE=1)
        endif()
    endif()

    if (Boost_stacktrace_windbg_FOUND)
        message(STATUS "Boost.Stacktrace windbg available")
        target_compile_definitions(sdthirdparty INTERFACE SUPDEF_HAVE_BOOST_STACKTRACE_WINDBG=1)
        if (NOT Boost_stacktrace_backtrace_FOUND AND NOT Boost_stacktrace_addr2line_FOUND)
            target_compile_definitions(sdthirdparty INTERFACE BOOST_STACKTRACE_USE_WINDBG=1)
        endif()
    endif()

    if (Boost_stacktrace_windebug_FOUND)
        message(STATUS "Boost.Stacktrace windebug available")
        target_compile_definitions(sdthirdparty INTERFACE SUPDEF_HAVE_BOOST_STACKTRACE_WINDEBUG=1)
        if (NOT Boost_stacktrace_backtrace_FOUND AND NOT Boost_stacktrace_addr2line_FOUND AND NOT Boost_stacktrace_windbg_FOUND)
            target_compile_definitions(sdthirdparty INTERFACE BOOST_STACKTRACE_USE_WINDBG=1)
        endif()
    endif()
    target_compile_definitions(sdthirdparty INTERFACE SUPDEF_USE_BOOST_STACKTRACE=1)
    target_link_libraries(sdthirdparty INTERFACE Boost::stacktrace_basic)

    if (Boost_stacktrace_noop_FOUND)
        message(STATUS "Boost.Stacktrace noop available")
        target_compile_definitions(sdthirdparty INTERFACE SUPDEF_HAVE_BOOST_STACKTRACE_NOOP=1)
    endif()
else()
    message(FATAL_ERROR "No stacktrace library available")
endif()

find_package(Boost ${BOOST_VER_NEEDED} REQUIRED COMPONENTS contract HINTS "/usr/include/" "/usr/local/include/" "/usr/include/boost/" "/usr/local/include/boost/")
if (Boost_contract_FOUND)
    message(STATUS "Boost.Contract available")
    target_link_libraries(sdthirdparty INTERFACE Boost::contract)
else()
    message(FATAL_ERROR "Boost.Contract not available")
endif()

# Get GCC version
execute_process(
    COMMAND ${CMAKE_CXX_COMPILER} -dumpversion
    OUTPUT_VARIABLE GCC_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(THIS_OS ${CMAKE_SYSTEM_NAME})
set(GXX_LIBS_HINTS "")
if (THIS_OS STREQUAL "Linux")
    exec_program(which ARGS ${CMAKE_CXX_COMPILER} OUTPUT_VARIABLE FULL_CXX_PATH)
    # Get base directory of CXX compiler and remove the potential "bin" directory if present
    # For instance "/usr/local/bin/g++" ---> "/usr/local"
    string(REGEX REPLACE "/bin/g\\+\\+$" "" CXX_BASE_DIR "${FULL_CXX_PATH}")
    set(POTENTIAL_LIBDIR "${CXX_BASE_DIR}/lib")
    set(POTENTIAL_LIB64DIR "${CXX_BASE_DIR}/lib64")
    set(POTENTIAL_LIBGCCDIR "${POTENTIAL_LIBDIR}/gcc/x86_64-pc-linux-gnu/${GCC_VERSION}")
    set(POTENTIAL_LIBSTDCXXDIR "${POTENTIAL_LIBDIR}/gcc/x86_64-pc-linux-gnu/${GCC_VERSION}/include")
    set(POTENTIAL_LIBSTDCXXDIR2 "${POTENTIAL_LIBDIR}/gcc/x86_64-pc-linux-gnu/${GCC_VERSION}/include-fixed")

    set(GXX_LIBS_HINTS "${POTENTIAL_LIBDIR}" "${POTENTIAL_LIB64DIR}" "${POTENTIAL_LIBGCCDIR}" "${POTENTIAL_LIBSTDCXXDIR}" "${POTENTIAL_LIBSTDCXXDIR2}")
endif()

find_library(STDCXXEXP_LIB stdc++exp HINTS ${GXX_LIBS_HINTS})
if (STDCXXEXP_LIB)
    message(STATUS "libstdc++exp.a found")
    target_link_libraries(sdthirdparty INTERFACE -lstdc++exp)
else()
    message(STATUS "libstdc++exp.a not found")
endif()

find_library(STDCXXFS_LIB stdc++fs HINTS ${GXX_LIBS_HINTS})
if (STDCXXFS_LIB)
    message(STATUS "libstdc++fs.a found")
    target_link_libraries(sdthirdparty INTERFACE -lstdc++fs)
else()
    message(STATUS "libstdc++fs.a not found")
endif()

find_library(TBB_LIB tbb HINTS ${GXX_LIBS_HINTS})
if (TBB_LIB)
    message(STATUS "libtbb found")
    target_link_libraries(sdthirdparty INTERFACE -ltbb)
else()
    message(STATUS "libtbb not found")
endif()
# TODO: test if possible to link to libstdc++exp.a. If yes, link to it
# TODO: Test if C++23 stacktraces are available (i.e. test if we can link to libstdc++_libbacktrace). If yes, use them, else find and use Boost.Stacktrace
# TODO: Link to libatomic.a
# TODO: Link to libstdc++fs.a (if possible)
# TODO: Link to libtbb (-ltbb) (if possible)
# TODO: Rebuild custom toolchain with additional GCC configure flags

# Add -ffunction-sections and -fdata-sections to reduce binary size
target_compile_options(sdthirdparty INTERFACE -ffunction-sections -fdata-sections -Wall -Wextra -Wno-unknown-pragmas)

# Get `USER_NPROC`
include(ProcessorCount)
ProcessorCount(USER_NPROC)

if (NOT USER_NPROC EQUAL 0)
    message(STATUS "Number of processors/cores: ${USER_NPROC}")
else()
    message(WARNING "Unable to get number of processors/cores")
endif()


if ((NOT SUPDEF_CREATOR) OR (NOT (${SUPDEF_CREATOR} STREQUAL "Unknown")))
    message(STATUS "Maintainer is ${SUPDEF_CREATOR}")
endif()

if (${SUPDEF_CREATOR} STREQUAL "Axel PASCON")
    target_compile_options(sdthirdparty INTERFACE -march=alderlake -mtune=alderlake -fexceptions)
    if (CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(sdthirdparty INTERFACE -fstrict-aliasing -ffinite-loops -ffold-simple-inlines -fgcse-las -fgcse-sm -fgraphite -fgraphite-identity -fipa-pta -fira-loop-pressure -fira-region=mixed -flive-range-shrinkage -floop-nest-optimize -fmodulo-sched -fmodulo-sched-allow-regmoves -fprefetch-loop-arrays -frename-registers -frtti -fsched-pressure -fschedule-insns -fschedule-insns2 -fthreadsafe-statics -ftracer -ftree-cselim -ftree-loop-if-convert -ftree-lrs -ftree-parallelize-loops=${USER_NPROC} -ftree-vectorize -funreachable-traps -fvariable-expansion-in-unroller -fweb -maccumulate-outgoing-args -malign-data=cacheline)
    endif()
endif()