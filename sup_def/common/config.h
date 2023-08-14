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

#ifndef NEED_TPP_INC
    #define NEED_TPP_INC(...) 0
#endif

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

#if defined(SUPDEF_PRAGMA_NAME)
    #undef SUPDEF_PRAGMA_NAME
#endif
#define SUPDEF_PRAGMA_NAME "supdef"

#if defined(SUPDEF_PRAGMA_DEFINE_START)
    #undef SUPDEF_PRAGMA_DEFINE_START
#endif
#define SUPDEF_PRAGMA_DEFINE_START "start"

#if defined(SUPDEF_PRAGMA_DEFINE_END)
    #undef SUPDEF_PRAGMA_DEFINE_END
#endif
#define SUPDEF_PRAGMA_DEFINE_END "end"

#if defined(SUPDEF_MACRO_ID_REGEX)
    #undef SUPDEF_MACRO_ID_REGEX
#endif
// Any valid C identifier
#define SUPDEF_MACRO_ID_REGEX "\\w+"

#if defined(SUPDEF_PRAGMA_DEFSTART_REGEX)
    #undef SUPDEF_PRAGMA_DEFSTART_REGEX
#endif
// '#' at the beginning of the line, followed by any number of spaces >= 0
// then 'pragma' followed by any number of spaces > 0
// then 'supdef' followed by any number of spaces > 0
// then 'start' followed by any number of spaces > 0
// then the name of the define followed by any number of spaces >= 0
#define SUPDEF_PRAGMA_DEFSTART_REGEX "^#\\s*pragma\\s+" SUPDEF_PRAGMA_NAME "\\s+" SUPDEF_PRAGMA_DEFINE_START "\\s+" SUPDEF_MACRO_ID_REGEX "\\s*$"

#if defined(SUPDEF_PRAGMA_DEFEND_REGEX)
    #undef SUPDEF_PRAGMA_DEFEND_REGEX
#endif
// '#' at the beginning of the line, followed by any number of spaces >= 0
// then 'pragma' followed by any number of spaces > 0
// then 'supdef' followed by any number of spaces > 0
// then 'end' followed by any number of spaces >= 0
#define SUPDEF_PRAGMA_DEFEND_REGEX "^#\\s*pragma\\s+" SUPDEF_PRAGMA_NAME "\\s+" SUPDEF_PRAGMA_DEFINE_END "\\s*$"

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
#define SUPDEF_PRAGMA_INCLUDE_REGEX "^#\\s*pragma\\s+" SUPDEF_PRAGMA_NAME "\\s+" SUPDEF_PRAGMA_INCLUDE "\\s+\"([^\"]*)\"\\s*$"

