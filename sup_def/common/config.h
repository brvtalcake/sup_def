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

#ifndef _GNU_SOURCE
    #define _GNU_SOURCE 1
#endif

#ifndef _FORTIFY_SOURCE
    #define _FORTIFY_SOURCE 2
#endif

#ifndef NEED_TPP_INC
    #define NEED_TPP_INC(...) 0
#endif

#include <sup_def/common/util/platform.hpp>

#include <sup_def/third_party/map/map.h>
#include <sup_def/third_party/empty_macro/detect.h>

#if defined(ID)
    #undef ID
#endif
#define ID(...) ID_(ID_(ID_(ID_(ID_(__VA_ARGS__)))))

#if defined(ID_)
    #undef ID_
#endif
#define ID_(...) ID__(ID__(ID__(ID__(ID__(__VA_ARGS__)))))

#if defined(ID__)
    #undef ID__
#endif
#define ID__(...) __VA_ARGS__

#ifdef PP_STRINGIZE
    #undef PP_STRINGIZE
#endif
#define PP_STRINGIZE(...) PP_STRINGIZE_(__VA_ARGS__)

#ifdef PP_STRINGIZE_
    #undef PP_STRINGIZE_
#endif
#define PP_STRINGIZE_(...) #__VA_ARGS__

#if defined(PP_EAT)
    #undef PP_EAT
#endif
#define PP_EAT(...)

#if defined(PP_CAT)
    #undef PP_CAT
#endif
#define PP_CAT(a, b) PP_CAT_(a, b)

#if defined(PP_CAT_)
    #undef PP_CAT_
#endif
#define PP_CAT_(a, b) a ## b

#if defined(PP_IF)
    #undef PP_IF
#endif
#define PP_IF(cond) PP_IF_(cond)

#if defined(PP_IF_)
    #undef PP_IF_
#endif
#define PP_IF_(cond) ID(PP_CAT(PP_IF_, cond))

#if defined(PP_IF_0)
    #undef PP_IF_0
#endif
#define PP_IF_0(...) ID

#if defined(PP_IF_1)
    #undef PP_IF_1
#endif
#define PP_IF_1(...) __VA_ARGS__ PP_EAT

#if defined(VA_COUNT)
    #undef VA_COUNT
#endif
#define VA_COUNT(...) VA_COUNT_(__VA_ARGS__)

#if defined(VA_COUNT_)
    #undef VA_COUNT_
#endif
#define VA_COUNT_(...)                                                      \
    ID(                                                                     \
        PP_IF                                                               \
        (                                                                   \
            ISEMPTY(__VA_ARGS__)                                            \
        )                                                                   \
        (0)                                                                 \
        (                                                                   \
            VA_COUNT__(                                                     \
                __VA_ARGS__, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11,        \
                10, 9, 8, 7, 6, 5, 4, 3, 2, 1                               \
            )                                                               \
        )                                                                   \
    )

#if defined(VA_COUNT__)
    #undef VA_COUNT__
#endif
#define VA_COUNT__(a20, a19, a18, a17, a16, a15, a14, a13, a12, a11, a10, a9, a8, a7, a6, a5, a4, a3, a2, a1, count, ...) count

#if defined(EXPAND_ONE_TUPLE)
    #undef EXPAND_ONE_TUPLE
#endif
#define EXPAND_ONE_TUPLE(tuple) ID tuple

#if defined(EXPAND_TUPLES)
    #undef EXPAND_TUPLES
#endif
#define EXPAND_TUPLES(...) MAP(EXPAND_ONE_TUPLE, __VA_ARGS__)

#if defined(TUPLE_FIRST_ARG)
    #undef TUPLE_FIRST_ARG
#endif
#define TUPLE_FIRST_ARG(tuple) FIRST_ARG(EXPAND_ONE_TUPLE(tuple))

#if defined(FIRST_ARG)
    #undef FIRST_ARG
#endif
#define FIRST_ARG(...) FIRST_ARG_(__VA_ARGS__)

#if defined(FIRST_ARG_)
    #undef FIRST_ARG_
#endif
#define FIRST_ARG_(first, ...) first

#if defined(TUPLE_OTHER_ARGS)
    #undef TUPLE_OTHER_ARGS
#endif
#define TUPLE_OTHER_ARGS(tuple) OTHER_ARGS(EXPAND_ONE_TUPLE(tuple))

#if defined(OTHER_ARGS)
    #undef OTHER_ARGS
#endif
#define OTHER_ARGS(...) OTHER_ARGS_(__VA_ARGS__)

#if defined(OTHER_ARGS_)
    #undef OTHER_ARGS_
#endif
#define OTHER_ARGS_(first, ...) __VA_ARGS__

#if defined(LAST_ARG)
    #undef LAST_ARG
#endif
#define LAST_ARG(...) LAST_ARG_(REVERSE(__VA_ARGS__))

#if defined(LAST_ARG_)
    #undef LAST_ARG_
#endif
#define LAST_ARG_(...) FIRST_ARG(__VA_ARGS__)

#if defined(REVERSE)
    #undef REVERSE
#endif
#define REVERSE(...) REVERSE_(__VA_ARGS__)

#if defined(REVERSE_)
    #undef REVERSE_
#endif
#define REVERSE_(...) BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_REVERSE(BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)))

#if defined(EXP_INST_CLASS_ONE)
    #undef EXP_INST_CLASS_ONE
#endif
#define EXP_INST_CLASS_ONE(tuple) template TUPLE_FIRST_ARG(tuple)<EXPAND_ONE_TUPLE(TUPLE_OTHER_ARGS(tuple))>;

#if defined(MAKE_TUPLE_WITH)
    #undef MAKE_TUPLE_WITH
#endif
#define MAKE_TUPLE_WITH(...) MAKE_TUPLE_WITH_(__VA_ARGS__)

#if defined(MAKE_TUPLE_WITH_)
    #undef MAKE_TUPLE_WITH_
#endif
#define MAKE_TUPLE_WITH_(...) (__VA_ARGS__, ID_AND_RIGHT_PAREN

#if defined(ID_AND_RIGHT_PAREN)
    #undef ID_AND_RIGHT_PAREN
#endif
#define ID_AND_RIGHT_PAREN(...) ID(__VA_ARGS__))

#if defined(EXP_INST_MAKE_TUPLE_LOOP_MACRO)
    #undef EXP_INST_MAKE_TUPLE_LOOP_MACRO
#endif
#define EXP_INST_MAKE_TUPLE_LOOP_MACRO(r, data, i, elem) BOOST_PP_COMMA_IF(i) MAKE_TUPLE_WITH(data)(elem)

#if defined(EXP_INST_CLASS_STRUCT_MAKE_TUPLE)
    #undef EXP_INST_CLASS_STRUCT_MAKE_TUPLE
#endif
#define EXP_INST_CLASS_STRUCT_MAKE_TUPLE(T, ...) BOOST_PP_SEQ_FOR_EACH_I(EXP_INST_MAKE_TUPLE_LOOP_MACRO, T, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))

#if defined(EXP_INST_CLASS_LOOP_MACRO)
    #undef EXP_INST_CLASS_LOOP_MACRO
#endif
#define EXP_INST_CLASS_LOOP_MACRO(r, data, i, elem) EXP_INST_CLASS_ONE(elem)

#if defined(EXP_INST_CLASS)
    #undef EXP_INST_CLASS
#endif
// Example use:
// EXP_INST_CLASS(Parser, (char), (wchar_t))
// --> template class Parser<char>; template class Parser<wchar_t>;
//#define EXP_INST_CLASS(T, ...) BOOST_PP_SEQ_FOR_EACH_I(EXP_INST_CLASS_LOOP_MACRO, T, BOOST_PP_VARIADIC_TO_SEQ(EXP_INST_CLASS_STRUCT_MAKE_TUPLE(T, __VA_ARGS__)))
#define EXP_INST_CLASS(T, ...) MAP(EXP_INST_CLASS_ONE, EXP_INST_CLASS_STRUCT_MAKE_TUPLE(class T, __VA_ARGS__))

#if defined(EXP_INST_STRUCT)
    #undef EXP_INST_STRUCT
#endif
#define EXP_INST_STRUCT(T, ...) MAP(EXP_INST_CLASS_ONE, EXP_INST_CLASS_STRUCT_MAKE_TUPLE(struct T, __VA_ARGS__))

#if defined(EXP_INST_FUNC_ONE)
    #undef EXP_INST_FUNC_ONE
#endif
// Input tuple for this macro is as follows:
// (function_name, return_type, (arg1_type, arg2_type, ...))
#define EXP_INST_FUNC_ONE(tuple) template TUPLE_SECOND_ARG(tuple) TUPLE_FIRST_ARG(tuple)<EXPAND_ONE_TUPLE(TUPLE_THIRD_ARG(tuple))>(EXPAND_ONE_TUPLE(TUPLE_THIRD_ARG(tuple)));

#if defined(TUPLE_SECOND_ARG)
    #undef TUPLE_SECOND_ARG
#endif
#define TUPLE_SECOND_ARG(tuple) SECOND_ARG(EXPAND_ONE_TUPLE(tuple))

#if defined(TUPLE_DROP_FIRST)
    #undef TUPLE_DROP_FIRST
#endif
#define TUPLE_DROP_FIRST(tuple) DROP_FIRST(EXPAND_ONE_TUPLE(tuple))

#if defined(DROP_FIRST)
    #undef DROP_FIRST
#endif
#define DROP_FIRST(...) DROP_FIRST_(__VA_ARGS__)

#if defined(DROP_FIRST_)
    #undef DROP_FIRST_
#endif
#define DROP_FIRST_(first, ...) __VA_ARGS__

#if defined(SECOND_ARG)
    #undef SECOND_ARG
#endif
#define SECOND_ARG(...) FIRST_ARG(DROP_FIRST(__VA_ARGS__))

#if defined(TUPLE_THIRD_ARG)
    #undef TUPLE_THIRD_ARG
#endif
#define TUPLE_THIRD_ARG(tuple) THIRD_ARG(EXPAND_ONE_TUPLE(tuple))

#if defined(THIRD_ARG)
    #undef THIRD_ARG
#endif
#define THIRD_ARG(...) SECOND_ARG(DROP_FIRST(__VA_ARGS__))

#if defined(EXP_INST_FUNC_MAKE_TUPLE_LOOP_MACRO)
    #undef EXP_INST_FUNC_MAKE_TUPLE_LOOP_MACRO
#endif
#define EXP_INST_FUNC_MAKE_TUPLE_LOOP_MACRO(r, data, i, elem) BOOST_PP_COMMA_IF(i) MAKE_TUPLE_WITH(data)(EXPAND_ONE_TUPLE(elem))

#if defined(EXP_INST_FUNC_MAKE_TUPLE)
    #undef EXP_INST_FUNC_MAKE_TUPLE
#endif
#define EXP_INST_FUNC_MAKE_TUPLE(F, ...) BOOST_PP_SEQ_FOR_EACH_I(EXP_INST_FUNC_MAKE_TUPLE_LOOP_MACRO, F, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))

#if defined(EXP_INST_FUNC)
    #undef EXP_INST_FUNC
#endif
// Example use:
// EXP_INST_FUNC(my_func, (ret_type, (arg1_type, arg2_type, ...)), (ret_type2, (arg1_type2, arg2_type2, ...)))
// --> template ret_type my_func<arg1_type, arg2_type, ...>(arg1_type, arg2_type, ...); template ret_type2 my_func<arg1_type2, arg2_type2, ...>(arg1_type2, arg2_type2, ...);
#define EXP_INST_FUNC(F, ...) MAP(EXP_INST_FUNC_ONE, EXP_INST_FUNC_MAKE_TUPLE(F, __VA_ARGS__))

#ifdef SAVE_MACRO
    #undef SAVE_MACRO
#endif
#define SAVE_MACRO(MACNAME) _Pragma(PP_STRINGIZE(push_macro(#MACNAME)))

#ifdef RESTORE_MACRO
    #undef RESTORE_MACRO
#endif
#define RESTORE_MACRO(MACNAME) _Pragma(PP_STRINGIZE(pop_macro(#MACNAME)))

#if defined(SUPDEF_PRAGMA_NAME)
    #undef SUPDEF_PRAGMA_NAME
#endif
#define SUPDEF_PRAGMA_NAME "supdef"

#if defined(SUPDEF_PRAGMA_DEFINE_BEGIN)
    #undef SUPDEF_PRAGMA_DEFINE_BEGIN
#endif
#define SUPDEF_PRAGMA_DEFINE_BEGIN "begin"

#if defined(SUPDEF_PRAGMA_DEFINE_END)
    #undef SUPDEF_PRAGMA_DEFINE_END
#endif
#define SUPDEF_PRAGMA_DEFINE_END "end"

#if defined(SUPDEF_MACRO_ID_REGEX)
    #undef SUPDEF_MACRO_ID_REGEX
#endif
// Any valid C identifier
#define SUPDEF_MACRO_ID_REGEX "\\w+"

#if defined(SUPDEF_PRAGMA_DEF_BEG_REGEX)
    #undef SUPDEF_PRAGMA_DEF_BEG_REGEX
#endif
// '#' at the beginning of the line, followed by any number of spaces >= 0
// then 'pragma' followed by any number of spaces > 0
// then 'supdef' followed by any number of spaces > 0
// then 'start' followed by any number of spaces > 0
// then the name of the define followed by any number of spaces >= 0
#define SUPDEF_PRAGMA_DEF_BEG_REGEX "^\\s*#\\s*pragma\\s+" SUPDEF_PRAGMA_NAME "\\s+" SUPDEF_PRAGMA_DEFINE_BEGIN "\\s+" SUPDEF_MACRO_ID_REGEX "\\s*$"

#if defined(SUPDEF_PRAGMA_DEF_END_REGEX)
    #undef SUPDEF_PRAGMA_DEF_END_REGEX
#endif
// '#' at the beginning of the line, followed by any number of spaces >= 0
// then 'pragma' followed by any number of spaces > 0
// then 'supdef' followed by any number of spaces > 0
// then 'end' followed by any number of spaces >= 0
#define SUPDEF_PRAGMA_DEF_END_REGEX "^\\s*#\\s*pragma\\s+" SUPDEF_PRAGMA_NAME "\\s+" SUPDEF_PRAGMA_DEFINE_END "\\s*$"

#if defined(SUPDEF_PRAGMA_INCLUDE)
    #undef SUPDEF_PRAGMA_INCLUDE
#endif
#define SUPDEF_PRAGMA_INCLUDE "include"

#if defined(SUPDEF_PRAGMA_INCLUDE_REGEX)
    #undef SUPDEF_PRAGMA_INCLUDE_REGEX
#endif
// '#' at the beginning of the line, followed by any number of spaces >= 0
// then 'pragma' followed by any number of spaces > 0
// then 'supdef' followed by any number of spaces > 0
// then 'include' followed by any number of spaces > 0
// then a C string literal followed by any number of spaces >= 0
#define SUPDEF_PRAGMA_INCLUDE_REGEX "^\\s*#\\s*pragma\\s+" SUPDEF_PRAGMA_NAME "\\s+" SUPDEF_PRAGMA_INCLUDE "\\s+[<>]*\"([^\"]*)\"[<>]*\\s*$"
#ifdef SUPDEF_PRAGMA_INCLUDE_REGEX_ANGLE_BRACKETS
    #undef SUPDEF_PRAGMA_INCLUDE_REGEX_ANGLE_BRACKETS
#endif
#define SUPDEF_PRAGMA_INCLUDE_REGEX_ANGLE_BRACKETS "^\\s*#\\s*pragma\\s+" SUPDEF_PRAGMA_NAME "\\s+" SUPDEF_PRAGMA_INCLUDE "\\s+[\"]*<([^>]*)>[\"]*\\s*$"
#ifdef SUPDEF_PRAGMA_INCLUDE_REGEX_NO_QUOTES
    #undef SUPDEF_PRAGMA_INCLUDE_REGEX_NO_QUOTES
#endif
// This one is without `""` nor `<>`
#define SUPDEF_PRAGMA_INCLUDE_REGEX_NO_QUOTES "^\\s*#\\s*pragma\\s+" SUPDEF_PRAGMA_NAME "\\s+" SUPDEF_PRAGMA_INCLUDE "\\s+([^\\s]+)\\s*$"
#ifdef SUPDEF_PRAGMA_INCLUDE_REGEX_NO_PATH
    #undef SUPDEF_PRAGMA_INCLUDE_REGEX_NO_PATH
#endif
// This one is just invalid
#define SUPDEF_PRAGMA_INCLUDE_REGEX_NO_PATH "^\\s*#\\s*pragma\\s+" SUPDEF_PRAGMA_NAME "\\s+" SUPDEF_PRAGMA_INCLUDE "\\s+([^\\s]*)\\s*$"

#include <version>
#if !defined( __cpp_lib_coroutine) || __cpp_lib_coroutine  != 201902L || \
    !defined(__cpp_impl_coroutine) || __cpp_impl_coroutine != 201902L
    #error "This library requires C++20 coroutines"
#endif


#if __cplusplus < 202002L// 202302L
    //#error "This library requires C++23"
    #error "This library requires at least C++20"
#endif

#ifndef __cpp_lib_unreachable
    #define __cpp_lib_unreachable 0L
#endif

#if SUPDEF_COMPILER == 1
    #include <bits/stdc++.h>
#endif