/* 
 * MIT License
 * 
 * Copyright (c) 2023 Axel PASCON
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#undef SUPDEF_ON_WINDOWS
#undef SUPDEF_ON_UNIX
#undef SD_COMMON_API
#undef SUPDEF_COMPILER 

#if defined(__INTELLISENSE__)
    #define COMPILING_EXTERNAL 1
    #define BUILDING_SUPDEF 1
#endif

#if defined(__GNUC__)
    #define SUPDEF_COMPILER 1
#elif defined(__clang__)
    #define SUPDEF_COMPILER 2
#elif defined(_MSC_VER)
    #define SUPDEF_COMPILER 3
#else
    #define SUPDEF_COMPILER 0
    #warning "Unsupported compiler, assuming GCC-like or Clang-like"
#endif

#if defined(_WIN32) || defined(_WIN64)
    #define SUPDEF_ON_WINDOWS 1
#endif

#if  defined(__unix__)  || defined(__unix)   \
 ||  defined(__linux__) || defined(__APPLE__)
    #define SUPDEF_ON_UNIX 1
#endif

#if !defined(SUPDEF_ON_WINDOWS) && !defined(SUPDEF_ON_UNIX)
    #error "Unsupported platform"
#endif

#if defined(SUPDEF_ON_WINDOWS)
    #if defined(BUILDING_SUPDEF)
        #if SUPDEF_COMPILER == 3 // MSVC
            #define SD_COMMON_API __declspec(dllexport)
        #else // GCC-like or Clang-like
            #define SD_COMMON_API __attribute__((__dllexport__))
        #endif
    #else
        #if SUPDEF_COMPILER == 3 // MSVC
            #define SD_COMMON_API __declspec(dllimport)
        #else // GCC-like or Clang-like
            #define SD_COMMON_API __attribute__((__dllimport__))
        #endif
    #endif
#else
    #define SD_COMMON_API __attribute__((__visibility__("default")))
#endif

#if defined(SD_EXTERNAL_API)
    #undef SD_EXTERNAL_API
#endif
#define SD_EXTERNAL_API SD_COMMON_API

#undef SUPDEF_EXIT

#if COMPILING_EXTERNAL
    #define SUPDEF_EXIT(...) std::exit(::SupDef::Util::main_ret())
#else
    #define SUPDEF_EXIT(...) STATIC_TODO("SUPDEF_EXIT is not yet implemented for GCC and Clang SupDef plugins")
#endif

#undef ATTRIBUTE_USED

#if SUPDEF_COMPILER == 3 // MSVC
    #define ATTRIBUTE_USED STATIC_TODO("ATTRIBUTE_USED is not yet implemented for MSVC");
#else // GCC-like or Clang-like
    #define ATTRIBUTE_USED __attribute__((__used__))
#endif