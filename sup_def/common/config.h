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

#define UNISTREAMS_STRING_USE_STD_BASIC_STRING 1

#include <gnulib.h>
#undef unreachable
#undef assume
#undef noreturn

#include <sup_def/common/util/platform.hpp>

#include <sup_def/third_party/map/map.h>
#include <sup_def/third_party/empty_macro/detect.h>

#include <chaos/preprocessor/algorithm/filter.h>
#include <chaos/preprocessor/algorithm/for_each.h>
#include <chaos/preprocessor/algorithm/elem.h>
#include <chaos/preprocessor/algorithm/enumerate.h>
#include <chaos/preprocessor/algorithm/reverse.h>
#include <chaos/preprocessor/algorithm/size.h>
#include <chaos/preprocessor/algorithm/transform.h>
#include <chaos/preprocessor/arbitrary/add.h>
#include <chaos/preprocessor/arbitrary/bool.h>
#include <chaos/preprocessor/arbitrary/dec.h>
#include <chaos/preprocessor/arbitrary/demote.h>
#include <chaos/preprocessor/arbitrary/div.h>
#include <chaos/preprocessor/arbitrary/equal.h>
#include <chaos/preprocessor/arbitrary/greater.h>
#include <chaos/preprocessor/arbitrary/inc.h>
#include <chaos/preprocessor/arbitrary/less.h>
#include <chaos/preprocessor/arbitrary/mod.h>
#include <chaos/preprocessor/arbitrary/mul.h>
#include <chaos/preprocessor/arbitrary/neg.h>
#include <chaos/preprocessor/arbitrary/not_equal.h>
#include <chaos/preprocessor/arbitrary/promote.h>
#include <chaos/preprocessor/arbitrary/sign.h>
#include <chaos/preprocessor/arbitrary/sub.h>
#include <chaos/preprocessor/comparison/equal.h>
#include <chaos/preprocessor/control/unless.h>
#include <chaos/preprocessor/control/variadic_if.h>
#include <chaos/preprocessor/control/while.h>
#include <chaos/preprocessor/debug/failure.h>
#include <chaos/preprocessor/detection/is_numeric.h>
#include <chaos/preprocessor/extended/variadic_cat.h>
#include <chaos/preprocessor/facilities/push.h>
#include <chaos/preprocessor/generics/strip.h>
#include <chaos/preprocessor/lambda/ops.h>
#include <chaos/preprocessor/limits.h>
#include <chaos/preprocessor/logical/and.h>
#include <chaos/preprocessor/logical/bool.h>
#include <chaos/preprocessor/logical/not.h>
#include <chaos/preprocessor/logical/or.h>
#include <chaos/preprocessor/recursion/expr.h>
#include <chaos/preprocessor/seq/core.h>
#include <chaos/preprocessor/seq/drop.h>
#include <chaos/preprocessor/seq/enumerate.h>
#include <chaos/preprocessor/tuple/core.h>

#undef MAKE_TUPLE
#define MAKE_TUPLE(...) (__VA_ARGS__)

#undef PP_CONSTRUCT_FLOAT
#undef PP_CONSTRUCT_FLOAT_IMPL
#define PP_CONSTRUCT_FLOAT(integral, decimal, abs_exponent, suffix, negative_exp)   \
    PP_CONSTRUCT_FLOAT_IMPL(integral, decimal, abs_exponent, suffix, negative_exp)
#define PP_CONSTRUCT_FLOAT_IMPL(integral, decimal, abs_exponent, suffix, negative_exp)  \
    PP_IF(negative_exp)                                                                 \
    (                                                                                   \
        VARIADIC_CAT(                                                                   \
            CHAOS_PP_CLEAN(integral),                                                   \
            CHAOS_PP_CLEAN(.),                                                          \
            CHAOS_PP_CLEAN(decimal),                                                    \
            CHAOS_PP_CLEAN(e),                                                          \
            CHAOS_PP_CLEAN(-),                                                          \
            CHAOS_PP_CLEAN(abs_exponent),                                               \
            CHAOS_PP_CLEAN(suffix)                                                      \
        )                                                                               \
    )                                                                                   \
    (                                                                                   \
        VARIADIC_CAT(                                                                   \
            CHAOS_PP_CLEAN(integral),                                                   \
            CHAOS_PP_CLEAN(.),                                                          \
            CHAOS_PP_CLEAN(decimal),                                                    \
            CHAOS_PP_CLEAN(e),                                                          \
            CHAOS_PP_CLEAN(abs_exponent),                                               \
            CHAOS_PP_CLEAN(suffix)                                                      \
        )                                                                               \
    )


#undef VARIADIC_AND
#define VARIADIC_AND(...) VARIADIC_AND_IMPL(__VA_ARGS__)

#undef VARIADIC_AND_IMPL
#define VARIADIC_AND_IMPL(...)                                  \
    CHAOS_PP_NOT(                                               \
        CHAOS_PP_BOOL(                                          \
            VA_COUNT(                                           \
                CHAOS_PP_ENUMERATE(                             \
                    CHAOS_PP_EXPR(                              \
                        CHAOS_PP_FILTER(                        \
                            CHAOS_PP_NOT_(                      \
                                CHAOS_PP_BOOL_(CHAOS_PP_ARG(1)) \
                            ),                                  \
                            (CHAOS_PP_TUPLE) (__VA_ARGS__)      \
                        )                                       \
                    )                                           \
                )                                               \
            )                                                   \
        )                                                       \
    )

#undef TRUE_FOR_ALL
#define TRUE_FOR_ALL(pred, ...) TRUE_FOR_ALL_IMPL(pred, __VA_ARGS__)

#undef TRUE_FOR_ALL_IMPL
#define TRUE_FOR_ALL_IMPL(pred, ...) VARIADIC_AND(CHAOS_PP_ENUMERATE(CHAOS_PP_EXPR(CHAOS_PP_TRANSFORM(pred, (CHAOS_PP_TUPLE) (__VA_ARGS__)))))

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

#undef PP_SWITCH
#undef PP_SWITCH_IMPL
#undef PP_SWITCH_IMPL_SEARCH_CASE
#define PP_SWITCH(integer, defaultcase, ...)    \
    CHAOS_PP_UNLESS(                            \
        CHAOS_PP_IS_NUMERIC(integer)            \
    )(CHAOS_PP_FAILURE())                       \
    PP_SWITCH_IMPL(                             \
        integer,                                \
        defaultcase __VA_OPT__(,)               \
        __VA_ARGS__                             \
    )
#define PP_SWITCH_IMPL(integer, defaultcase, ...)   \
    PP_IF(                                          \
        CHAOS_PP_AND                                \
        (                                           \
            CHAOS_PP_OR                             \
            (                                       \
                ISEMPTY(                            \
                    __VA_ARGS__                     \
                )                                   \
            )(                                      \
                ISEMPTY(                            \
                    PP_SWITCH_IMPL_SEARCH_CASE(     \
                        integer,                    \
                        __VA_ARGS__                 \
                    )                               \
                )                                   \
            )                                       \
        )                                           \
        (                                           \
            CHAOS_PP_IS_NUMERIC(integer)            \
        )                                           \
    )(                                              \
        EXPAND_ONE_TUPLE(                           \
            defaultcase                             \
        )                                           \
    )(                                              \
        EXPAND_ONE_TUPLE(                           \
            PP_SWITCH_IMPL_SEARCH_CASE(             \
                integer,                            \
                __VA_ARGS__                         \
            )                                       \
        )                                           \
    )
#define PP_SWITCH_IMPL_SEARCH_CASE(integer, ...)    \
    CHAOS_PP_EXPR(                                  \
        CHAOS_PP_FOR_EACH(                          \
            CHAOS_PP_LAMBDA(                        \
                CHAOS_PP_VARIADIC_IF_(              \
                    CHAOS_PP_EQUAL_(                \
                        integer,                    \
                        CHAOS_PP_ELEM_(             \
                            0,                      \
                            (CHAOS_PP_TUPLE)        \
                                CHAOS_PP_ARG(1)     \
                        )                           \
                    )                               \
                )(                                  \
                    CHAOS_PP_ELEM_(                 \
                        1,                          \
                        (CHAOS_PP_TUPLE)            \
                            CHAOS_PP_ARG(1)         \
                    )                               \
                )(                                  \
                    CHAOS_PP_EMPTY()                \
                )                                   \
            ),                                      \
            (CHAOS_PP_TUPLE) (__VA_ARGS__)          \
        )                                           \
    )

#ifdef __INTELLISENSE__
    #undef PP_SWITCH_IMPL_SEARCH_CASE
    #define PP_SWITCH_IMPL_SEARCH_CASE(...) (0)
#endif

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

#undef PP_PACK
#undef PP_PACK_IMPL
#define PP_PACK(...) PP_PACK_IMPL(__VA_ARGS__)
#define PP_PACK_IMPL(...) CHAOS_PP_PUSH(__VA_ARGS__)

#undef ARBITRARY_ABS
#undef ARBITRARY_ABS_IMPL
#define ARBITRARY_ABS(chaospp_arbitrary) ARBITRARY_ABS_IMPL(chaospp_arbitrary)
#define ARBITRARY_ABS_IMPL(chaospp_arbitrary)           \
    PP_IF(                                              \
        CHAOS_PP_ARBITRARY_SIGN(chaospp_arbitrary)      \
    )                                                   \
    (                                                   \
        EXPAND_ONE_TUPLE(chaospp_arbitrary)             \
    )                                                   \
    (                                                   \
        chaospp_arbitrary                               \
    )

#undef ARBITRARY_DIGIT_COUNT
#undef ARBITRARY_DIGIT_COUNT_IMPL
#define ARBITRARY_DIGIT_COUNT(chaospp_arbitrary) ARBITRARY_DIGIT_COUNT_IMPL(chaospp_arbitrary)
#define ARBITRARY_DIGIT_COUNT_IMPL(chaospp_arbitrary)       \
    CHAOS_PP_ELEM(                                          \
        1,                                                  \
        CHAOS_PP_EXPR(                                      \
            CHAOS_PP_WHILE(                                 \
                CHAOS_PP_ARBITRARY_BOOL_(                   \
                    CHAOS_PP_ELEM_(                         \
                        0,                                  \
                        CHAOS_PP_ARG(1)                     \
                    )                                       \
                ),                                          \
                CHAOS_PP_LAMBDA(                            \
                    (CHAOS_PP_TUPLE)(                       \
                        CHAOS_PP_ARBITRARY_DIV_(            \
                            CHAOS_PP_ELEM_(                 \
                                0,                          \
                                CHAOS_PP_ARG(1)             \
                            ),                              \
                            CHAOS_PP_ARBITRARY_PROMOTE(10)  \
                        ),                                  \
                        CHAOS_PP_ARBITRARY_INC_(            \
                            CHAOS_PP_ELEM_(                 \
                                1,                          \
                                CHAOS_PP_ARG(1)             \
                            )                               \
                        )                                   \
                    )                                       \
                ),                                          \
                (CHAOS_PP_TUPLE)(                           \
                    ARBITRARY_ABS(                          \
                        chaospp_arbitrary                   \
                    ),                                      \
                    CHAOS_PP_ARBITRARY_PROMOTE(0)           \
                )                                           \
            )                                               \
        )                                                   \
    )

#undef ARBITRARY_EXP
#undef ARBITRARY_EXP_IMPL
#define ARBITRARY_EXP(chaospp_arbitrary_base, chaospp_arbitrary_exp) ARBITRARY_EXP_IMPL(chaospp_arbitrary_base, chaospp_arbitrary_exp)
#define ARBITRARY_EXP_IMPL(chaospp_arbitrary_base, chaospp_arbitrary_exp)   \
    CHAOS_PP_ELEM(                                                          \
        0,                                                                  \
        CHAOS_PP_EXPR(                                                      \
            CHAOS_PP_WHILE(                                                 \
                CHAOS_PP_ARBITRARY_BOOL_(                                   \
                    CHAOS_PP_ELEM_(                                         \
                        1,                                                  \
                        CHAOS_PP_ARG(1)                                     \
                    )                                                       \
                ),                                                          \
                CHAOS_PP_LAMBDA(                                            \
                    (CHAOS_PP_TUPLE)(                                       \
                        CHAOS_PP_ARBITRARY_MUL_(                            \
                            CHAOS_PP_ELEM_(                                 \
                                0,                                          \
                                CHAOS_PP_ARG(1)                             \
                            ),                                              \
                            chaospp_arbitrary_base                          \
                        ),                                                  \
                        CHAOS_PP_ARBITRARY_DEC_(                            \
                            CHAOS_PP_ELEM_(                                 \
                                1,                                          \
                                CHAOS_PP_ARG(1)                             \
                            )                                               \
                        )                                                   \
                    )                                                       \
                ),                                                          \
                (CHAOS_PP_TUPLE)(                                           \
                    CHAOS_PP_ARBITRARY_PROMOTE(1),                          \
                    chaospp_arbitrary_exp                                   \
                )                                                           \
            )                                                               \
        )                                                                   \
    )

#ifdef __INTELLISENSE__
    #undef ARBITRARY_EXP
    #define ARBITRARY_EXP(...) (0)
#endif

#undef VARIADIC_CAT
#define VARIADIC_CAT(...) CHAOS_PP_VARIADIC_CAT(__VA_ARGS__)

#undef CCAST
#define CCAST(type, ...) ((type)(__VA_ARGS__))

#undef CPPCAST
#define CPPCAST(type, ...) (static_cast<type>(__VA_ARGS__))

#undef CPPCAST_CONSTRUCT
#define CPPCAST_CONSTRUCT(type, ...) (type(__VA_ARGS__))

#undef PP_PAIR
#define PP_PAIR(x, y) (CHAOS_PP_TUPLE)(x, y)

#undef PP_FLOAT
#define PP_FLOAT(integralized, pow10) PP_PAIR(integralized, pow10)

#undef PP_FLOAT_PROMOTE
#undef PP_FLOAT_PROMOTE_IMPL
#define PP_FLOAT_PROMOTE(integral, decimal, ...) PP_FLOAT_PROMOTE_IMPL(integral, decimal, __VA_ARGS__)
/**
 * @def PP_FLOAT_PROMOTE_IMPL(integral, decimal, optional_pow10)
 * @brief Promotes a floating point number (described as <integral>.<decimal>) to a `PP_PAIR` such that : a*10^b == integral.decimal (a and b being integers composing the pair)
 */
#define PP_FLOAT_PROMOTE_IMPL(integral, decimal, ...)   \
    PP_FLOAT(                                           \
        PP_IF(                                          \
            CHAOS_PP_ARBITRARY_SIGN(                    \
                CHAOS_PP_ARBITRARY_PROMOTE(integral)    \
            )                                           \
        )(                                              \
            CHAOS_PP_ARBITRARY_NEG(                     \
                PP_FLOAT_PROMOTE_IMPL_INT_PART(         \
                    integral,                           \
                    decimal                             \
                )                                       \
            )                                           \
        )(                                              \
            PP_FLOAT_PROMOTE_IMPL_INT_PART(             \
                integral,                               \
                decimal                                 \
            )                                           \
        ),                                              \
        PP_IF(                                          \
            ISEMPTY(__VA_ARGS__)                        \
        )(                                              \
            PP_FLOAT_PROMOTE_IMPL_POW10_PART(           \
                integral,                               \
                decimal                                 \
            )                                           \
        )(                                              \
            CHAOS_PP_ARBITRARY_ADD(                     \
                PP_FLOAT_PROMOTE_IMPL_POW10_PART(       \
                    integral,                           \
                    decimal                             \
                ),                                      \
                CHAOS_PP_ARBITRARY_PROMOTE(             \
                    FIRST_ARG(__VA_ARGS__)              \
                )                                       \
            )                                           \
        )                                               \
    )

#undef PP_FLOAT_PROMOTE_IMPL_INT_PART
#define PP_FLOAT_PROMOTE_IMPL_INT_PART(integral, decimal)   \
    CHAOS_PP_ARBITRARY_ADD(                                 \
        ARBITRARY_ABS(                                      \
            CHAOS_PP_ARBITRARY_PROMOTE(decimal)             \
        ),                                                  \
        CHAOS_PP_ARBITRARY_MUL(                             \
            ARBITRARY_ABS(                                  \
                CHAOS_PP_ARBITRARY_PROMOTE(integral)        \
            ),                                              \
            ARBITRARY_EXP(                                  \
                CHAOS_PP_ARBITRARY_PROMOTE(10),             \
                ARBITRARY_DIGIT_COUNT(                      \
                    ARBITRARY_ABS(                          \
                        CHAOS_PP_ARBITRARY_PROMOTE(decimal) \
                    )                                       \
                )                                           \
            )                                               \
        )                                                   \
    )

#undef PP_FLOAT_PROMOTE_IMPL_POW10_PART
#define PP_FLOAT_PROMOTE_IMPL_POW10_PART(integral, decimal) \
    CHAOS_PP_ARBITRARY_NEG(                                 \
        ARBITRARY_DIGIT_COUNT(                              \
            ARBITRARY_ABS(                                  \
                CHAOS_PP_ARBITRARY_PROMOTE(decimal)         \
            )                                               \
        )                                                   \
    )

#ifdef __INTELLISENSE__
    #undef PP_FLOAT_PROMOTE
    #define PP_FLOAT_PROMOTE(...) PP_FLOAT((0), (0))
#endif

#undef PP_FLOAT_DEMOTE
#undef PP_FLOAT_DEMOTE_IMPL
#define PP_FLOAT_DEMOTE(pair, ...) PP_FLOAT_DEMOTE_IMPL(pair, __VA_ARGS__ /* Optional type */)
#define PP_FLOAT_DEMOTE_IMPL(pair, ...) \
    PP_IF(ISEMPTY(__VA_ARGS__))         \
    (                                   \
        PP_FLOAT_DEMOTE_IMPL_1(         \
            pair                        \
        )                               \
    )                                   \
    (                                   \
        PP_FLOAT_DEMOTE_IMPL_2(         \
            pair,                       \
            FIRST_ARG(__VA_ARGS__)      \
        )                               \
    )

#include <stdfloat>

#undef PP_FLOAT_DEMOTE_HAS_FLOAT128
#if __STDCPP_FLOAT128_T__
    #define PP_FLOAT_DEMOTE_HAS_FLOAT128 1
#else
    #define PP_FLOAT_DEMOTE_HAS_FLOAT128 0
#endif

#undef PP_FLOAT_DEMOTE_IMPL_1
#define PP_FLOAT_DEMOTE_IMPL_1(pair)                    \
    PP_IF(PP_FLOAT_DEMOTE_HAS_FLOAT128)                 \
    (                                                   \
        PP_CONSTRUCT_FLOAT(                             \
            PP_FLOAT_DEMOTE_INT_PART(pair),             \
            PP_FLOAT_DEMOTE_DEC_PART(pair),             \
            PP_FLOAT_DEMOTE_EXP_PART(pair),             \
            F128,                                       \
            PP_FLOAT_DEMOTE_EXP_PART_IS_NEG(pair)       \
        )                                               \
    )                                                   \
    (                                                   \
        PP_CONSTRUCT_FLOAT(                             \
            PP_FLOAT_DEMOTE_INT_PART(pair),             \
            PP_FLOAT_DEMOTE_DEC_PART(pair),             \
            PP_FLOAT_DEMOTE_EXP_PART(pair),             \
            L,                                          \
            PP_FLOAT_DEMOTE_EXP_PART_IS_NEG(pair)       \
        )                                               \
    )

#undef PP_FLOAT_DEMOTE_IMPL_2
#define PP_FLOAT_DEMOTE_IMPL_2(pair, type) CCAST(type, PP_FLOAT_DEMOTE_IMPL_1(pair))

#undef PP_FLOAT_DEMOTE_INT_PART
#undef PP_FLOAT_DEMOTE_INT_PART_IMPL_SIGN
#undef PP_FLOAT_DEMOTE_INT_PART_IMPL_NOSIGN
#define PP_FLOAT_DEMOTE_INT_PART_IMPL_SIGN(integral_chaospp_arbitrary)  \
    -PP_FLOAT_DEMOTE_INT_PART_IMPL_NOSIGN(                              \
        EXPAND_ONE_TUPLE(                                               \
            integral_chaospp_arbitrary                                  \
        )                                                               \
    )
#define PP_FLOAT_DEMOTE_INT_PART_IMPL_NOSIGN(integral_chaospp_arbitrary) CHAOS_PP_ELEM(0, (CHAOS_PP_SEQ) integral_chaospp_arbitrary)
#define PP_FLOAT_DEMOTE_INT_PART(pair)          \
    PP_IF(                                      \
        CHAOS_PP_ARBITRARY_SIGN(                \
            CHAOS_PP_ELEM(0, pair)              \
        )                                       \
    )                                           \
    (                                           \
        PP_FLOAT_DEMOTE_INT_PART_IMPL_SIGN(     \
            CHAOS_PP_ELEM(0, pair)              \
        )                                       \
    )                                           \
    (                                           \
        PP_FLOAT_DEMOTE_INT_PART_IMPL_NOSIGN(   \
            CHAOS_PP_ELEM(0, pair)              \
        )                                       \
    )

#undef PP_FLOAT_DEMOTE_DEC_PART
#undef PP_FLOAT_DEMOTE_DEC_PART_IMPL
#define PP_FLOAT_DEMOTE_DEC_PART(pair) PP_FLOAT_DEMOTE_DEC_PART_IMPL(ARBITRARY_ABS(CHAOS_PP_ELEM(0, pair)))
#define PP_FLOAT_DEMOTE_DEC_PART_IMPL(integral_chaospp_arbitrary) VARIADIC_CAT(CHAOS_PP_SEQ_ENUMERATE(CHAOS_PP_SEQ_DROP(1, integral_chaospp_arbitrary)))

#undef PP_FLOAT_DEMOTE_EXP_PART_IS_NEG
#undef PP_FLOAT_DEMOTE_EXP_PART_IS_NEG_IMPL
#undef PP_FLOAT_DEMOTE_EXP_PART
#undef PP_FLOAT_DEMOTE_EXP_PART_IMPL
#define PP_FLOAT_DEMOTE_EXP_PART_IS_NEG(pair) PP_FLOAT_DEMOTE_EXP_PART_IS_NEG_IMPL(CHAOS_PP_ELEM(0, pair), CHAOS_PP_ELEM(1, pair))
#define PP_FLOAT_DEMOTE_EXP_PART_IS_NEG_IMPL(integral_chaospp_arbitrary, pow10_chaospp_arbitrary)   \
    CHAOS_PP_ARBITRARY_SIGN(                                                                        \
        CHAOS_PP_ARBITRARY_ADD(                                                                     \
            CHAOS_PP_ARBITRARY_DEC(                                                                 \
                ARBITRARY_DIGIT_COUNT(                                                              \
                    integral_chaospp_arbitrary                                                      \
                )                                                                                   \
            ),                                                                                      \
            pow10_chaospp_arbitrary                                                                 \
        )                                                                                           \
    )
#define PP_FLOAT_DEMOTE_EXP_PART(pair) PP_FLOAT_DEMOTE_EXP_PART_IMPL(CHAOS_PP_ELEM(0, pair), CHAOS_PP_ELEM(1, pair))
#define PP_FLOAT_DEMOTE_EXP_PART_IMPL(integral_chaospp_arbitrary, pow10_chaospp_arbitrary)  \
    CHAOS_PP_ARBITRARY_DEMOTE(                                                              \
        ARBITRARY_ABS(                                                                      \
            CHAOS_PP_ARBITRARY_ADD(                                                         \
                CHAOS_PP_ARBITRARY_DEC(                                                     \
                    ARBITRARY_DIGIT_COUNT(                                                  \
                        integral_chaospp_arbitrary                                          \
                    )                                                                       \
                ),                                                                          \
                pow10_chaospp_arbitrary                                                     \
            )                                                                               \
        )                                                                                   \
    )

#undef PP_FLOAT_ADAPT
#undef PP_FLOAT_ADAPT_IMPL
#undef PP_FLOAT_ADAPT_LOOP_OP
/**
 * @def PP_FLOAT_ADAPT(pair_a, pair_b)
 * @brief Adapts two floating point numbers to the same exponent
 * 
 */
#define PP_FLOAT_ADAPT(pair_a, pair_b) PP_FLOAT_ADAPT_IMPL(pair_a, pair_b)
#define PP_FLOAT_ADAPT_IMPL(pair_a, pair_b) \
    CHAOS_PP_EXPR(                          \
        CHAOS_PP_WHILE(                     \
            CHAOS_PP_ARBITRARY_NOT_EQUAL_(  \
                CHAOS_PP_ELEM_(             \
                    1,                      \
                    CHAOS_PP_ARG(1)         \
                ),                          \
                CHAOS_PP_ELEM_(             \
                    1,                      \
                    CHAOS_PP_ARG(2)         \
                )                           \
            ),                              \
            PP_FLOAT_ADAPT_LOOP_OP,         \
            pair_a,                         \
            pair_b                          \
        )                                   \
    )
#define PP_FLOAT_ADAPT_LOOP_OP(_, pair_a, pair_b)   \
    PP_IF(                                          \
        CHAOS_PP_ARBITRARY_GREATER(                 \
            CHAOS_PP_ELEM(                          \
                1,                                  \
                pair_a                              \
            ),                                      \
            CHAOS_PP_ELEM(                          \
                1,                                  \
                pair_b                              \
            )                                       \
        )                                           \
    )(                                              \
        PP_FLOAT(                                   \
            CHAOS_PP_ARBITRARY_MUL(                 \
                CHAOS_PP_ELEM(                      \
                    0,                              \
                    pair_a                          \
                ),                                  \
                CHAOS_PP_ARBITRARY_PROMOTE(10)      \
            ),                                      \
            CHAOS_PP_ARBITRARY_DEC(                 \
                CHAOS_PP_ELEM(                      \
                    1,                              \
                    pair_a                          \
                )                                   \
            )                                       \
        ),                                          \
        pair_b                                      \
    )(                                              \
        pair_a,                                     \
        PP_FLOAT(                                   \
            CHAOS_PP_ARBITRARY_MUL(                 \
                CHAOS_PP_ELEM(                      \
                    0,                              \
                    pair_b                          \
                ),                                  \
                CHAOS_PP_ARBITRARY_PROMOTE(10)      \
            ),                                      \
            CHAOS_PP_ARBITRARY_DEC(                 \
                CHAOS_PP_ELEM(                      \
                    1,                              \
                    pair_b                          \
                )                                   \
            )                                       \
        )                                           \
    )

#undef PP_FLOAT_ADD
#undef PP_FLOAT_ADD_IMPL
#define PP_FLOAT_ADD(pair_a, pair_b) PP_FLOAT_ADD_IMPL(pair_a, pair_b)
#define PP_FLOAT_ADD_IMPL(pair_a, pair_b)   \
    PP_FLOAT(                               \
        CHAOS_PP_ARBITRARY_ADD(             \
            CHAOS_PP_ELEM(                  \
                0,                          \
                FIRST_ARG(                  \
                    PP_FLOAT_ADAPT(         \
                        pair_a,             \
                        pair_b              \
                    )                       \
                )                           \
            ),                              \
            CHAOS_PP_ELEM(                  \
                0,                          \
                SECOND_ARG(                 \
                    PP_FLOAT_ADAPT(         \
                        pair_a,             \
                        pair_b              \
                    )                       \
                )                           \
            )                               \
        ),                                  \
        CHAOS_PP_ELEM(                      \
            1,                              \
            FIRST_ARG(                      \
                PP_FLOAT_ADAPT(             \
                    pair_a,                 \
                    pair_b                  \
                )                           \
            )                               \
        )                                   \
    )

#undef PP_FLOAT_NEG
#undef PP_FLOAT_NEG_IMPL
#define PP_FLOAT_NEG(pair) PP_FLOAT_NEG_IMPL(pair)
#define PP_FLOAT_NEG_IMPL(pair) PP_FLOAT(CHAOS_PP_ARBITRARY_NEG(CHAOS_PP_ELEM(0, pair)), CHAOS_PP_ELEM(1, pair))

#undef PP_FLOAT_SUB
#undef PP_FLOAT_SUB_IMPL
#define PP_FLOAT_SUB(pair_a, pair_b) PP_FLOAT_SUB_IMPL(pair_a, pair_b)
#define PP_FLOAT_SUB_IMPL(pair_a, pair_b) PP_FLOAT_ADD(pair_a, PP_FLOAT_NEG(pair_b))

#undef PP_FLOAT_MUL
#undef PP_FLOAT_MUL_IMPL
#define PP_FLOAT_MUL(pair_a, pair_b) PP_FLOAT_MUL_IMPL(pair_a, pair_b)
#define PP_FLOAT_MUL_IMPL(pair_a, pair_b)       \
    PP_IF(                                      \
        CHAOS_PP_OR                             \
        (                                       \
            CHAOS_PP_ARBITRARY_EQUAL(           \
                CHAOS_PP_ELEM(                  \
                    0,                          \
                    pair_a                      \
                ),                              \
                CHAOS_PP_ARBITRARY_PROMOTE(0)   \
            )                                   \
        )(                                      \
            CHAOS_PP_ARBITRARY_EQUAL(           \
                CHAOS_PP_ELEM(                  \
                    0,                          \
                    pair_b                      \
                ),                              \
                CHAOS_PP_ARBITRARY_PROMOTE(0)   \
            )                                   \
        )                                       \
    )(                                          \
        PP_FLOAT_ZERO()                         \
    )(                                          \
        PP_FLOAT(                               \
            CHAOS_PP_ARBITRARY_MUL(             \
                CHAOS_PP_ELEM(                  \
                    0,                          \
                    pair_a                      \
                ),                              \
                CHAOS_PP_ELEM(                  \
                    0,                          \
                    pair_b                      \
                )                               \
            ),                                  \
            CHAOS_PP_ARBITRARY_ADD(             \
                CHAOS_PP_ELEM(                  \
                    1,                          \
                    pair_a                      \
                ),                              \
                CHAOS_PP_ELEM(                  \
                    1,                          \
                    pair_b                      \
                )                               \
            )                                   \
        )                                       \
    )

#undef PP_FLOAT_DIV
#undef PP_FLOAT_DIV_IMPL
#define PP_FLOAT_DIV(pair_a, pair_b, ...) PP_FLOAT_DIV_IMPL(pair_a, pair_b, PP_IF(ISEMPTY(__VA_ARGS__))(5)(FIRST_ARG(__VA_ARGS__)))
/* TODO : Replace with a modified version of PP_FLOAT_INV_IMPL */
#define PP_FLOAT_DIV_IMPL(pair_a, pair_b, precision) PP_FLOAT_MUL(pair_a, PP_FLOAT_INV(pair_b, precision))

#undef PP_FLOAT_ONE
#define PP_FLOAT_ONE(...) PP_FLOAT_PROMOTE(1, 0 __VA_OPT__(, FIRST_ARG(__VA_ARGS__)))

#undef PP_FLOAT_DECIMAL_DIGIT_COUNT
#undef PP_FLOAT_DECIMAL_DIGIT_COUNT_IMPL
/**
 * @def PP_FLOAT_DECIMAL_DIGIT_COUNT(pair)
 * @brief Returns the number of decimal digits in a PP_FLOAT, when the power of 10 is 0
 * 
 */
#define PP_FLOAT_DECIMAL_DIGIT_COUNT(pair) PP_FLOAT_DECIMAL_DIGIT_COUNT_IMPL(pair)
#define PP_FLOAT_DECIMAL_DIGIT_COUNT_IMPL(pair) \
    PP_IF(                                      \
        CHAOS_PP_ARBITRARY_SIGN(                \
            CHAOS_PP_ELEM(                      \
                1,                              \
                pair                            \
            )                                   \
        )                                       \
    )(                                          \
        ARBITRARY_ABS(                          \
            CHAOS_PP_ELEM(                      \
                1,                              \
                pair                            \
            )                                   \
        )                                       \
    )(                                          \
        CHAOS_PP_ARBITRARY_PROMOTE(0)           \
    )

#undef PP_FLOAT_ABS
#undef PP_FLOAT_ABS_IMPL
#define PP_FLOAT_ABS(pair) PP_FLOAT_ABS_IMPL(pair)
#define PP_FLOAT_ABS_IMPL(pair) PP_FLOAT(ARBITRARY_ABS(CHAOS_PP_ELEM(0, pair)), CHAOS_PP_ELEM(1, pair))

#undef PP_FLOAT_SIGN
#undef PP_FLOAT_SIGN_IMPL
#define PP_FLOAT_SIGN(pair) PP_FLOAT_SIGN_IMPL(pair)
#define PP_FLOAT_SIGN_IMPL(pair) CHAOS_PP_ARBITRARY_SIGN(CHAOS_PP_ELEM(0, pair))

#undef PP_FLOAT_ZERO
#define PP_FLOAT_ZERO() PP_FLOAT((0), (0))

#undef PP_FLOAT_INV
#undef PP_FLOAT_INV_IMPL
#undef PP_FLOAT_INV_IMPL_WRAPPER
#undef PP_FLOAT_INV_IMPL_WHILE_BODY
#define PP_FLOAT_INV(pair, ...) PP_FLOAT_INV_IMPL(pair, PP_IF(ISEMPTY(__VA_ARGS__))(5)(FIRST_ARG(__VA_ARGS__)))
#define PP_FLOAT_INV_IMPL(pair, precision)                              \
    CHAOS_PP_UNLESS(                                                    \
        CHAOS_PP_ARBITRARY_BOOL(                                        \
            CHAOS_PP_ELEM(                                              \
                0,                                                      \
                pair                                                    \
            )                                                           \
        )                                                               \
    )(CHAOS_PP_FAILURE())                                               \
    PP_FLOAT_INV_IMPL_WRAPPER(                                          \
        PP_FLOAT_SIGN(pair),                                            \
        pair,                                                           \
                                                                        \
        PP_FLOAT_INV_IMPL_WHILE_BODY(pair, precision)                   \
    )
#define PP_FLOAT_INV_IMPL_WRAPPER(boolean, pair_b, ...) \
    PP_FLOAT_INV_IMPL_WRAPPER_END_RESULT(               \
        boolean,                                        \
        pair_b,                                         \
        FIRST_ARG(                                      \
            CHAOS_PP_EXPR(                              \
                CHAOS_PP_WHILE(                         \
                    __VA_ARGS__                         \
                )                                       \
            )                                           \
        )                                               \
    )
#define PP_FLOAT_INV_IMPL_WHILE_BODY(pair, precision)           \
    PP_FLOAT_INV_IMPL_WHILE_BODY_COND /* While cond */,         \
    PP_FLOAT_COMPUTE_DIV_STEP /* While op */,                   \
    PP_FLOAT_ZERO() /* Result (arg 1) */,                       \
    PP_FLOAT_ONE() /* Pair a (arg 2) */,                        \
    PP_FLOAT_ABS(pair) /* Pair b (arg 3) */,                    \
    CHAOS_PP_BOOL(0) /* Division finished (arg 4) */,           \
    CHAOS_PP_ARBITRARY_PROMOTE(0) /* Last remainder (arg 5) */, \
    CHAOS_PP_ARBITRARY_PROMOTE(0) /* Step count (arg 6) */,     \
    CHAOS_PP_ARBITRARY_PROMOTE(precision) /* Precision (arg 7) */

#undef PP_FLOAT_INV_IMPL_WHILE_BODY_COND
#define PP_FLOAT_INV_IMPL_WHILE_BODY_COND(_, res, p_a, p_b, finished, last_rem, step_count, precision)  \
    CHAOS_PP_AND                                                                                        \
    (                                                                                                   \
        CHAOS_PP_ARBITRARY_LESS(                                                                        \
            PP_FLOAT_DECIMAL_DIGIT_COUNT(                                                               \
                PP_FLOAT_INV_IMPL_WRAPPER_END_RESULT(                                                   \
                    PP_FLOAT_SIGN(p_b),                                                                 \
                    p_b,                                                                                \
                    res                                                                                 \
                )                                                                                       \
            ),                                                                                          \
            precision                                                                                   \
        )                                                                                               \
    )(                                                                                                  \
        CHAOS_PP_NOT(finished)                                                                          \
    )
#undef PP_FLOAT_COMPUTE_DIV_STEP_LAST_REMAINDER
/**
 * @def PP_FLOAT_COMPUTE_DIV_STEP_LAST_REMAINDER(prev_step_result, pair_a, pair_b, last_remainder, step_count)
 * @brief Computes the last remainder of a division step (and multiplies it by 10)
 * 
 */
#define PP_FLOAT_COMPUTE_DIV_STEP_LAST_REMAINDER(prev_step_result, pair_a, pair_b, last_remainder, step_count)  \
    CHAOS_PP_ARBITRARY_MUL(                                                                                     \
        PP_IF(CHAOS_PP_ARBITRARY_EQUAL(step_count, 0))                                                          \
        (                                                                                                       \
            CHAOS_PP_ARBITRARY_MOD(                                                                             \
                CHAOS_PP_ELEM(0, pair_a),                                                                       \
                CHAOS_PP_ELEM(0, pair_b)                                                                        \
            )                                                                                                   \
        )(                                                                                                      \
            CHAOS_PP_ARBITRARY_MOD(                                                                             \
                last_remainder,                                                                                 \
                CHAOS_PP_ELEM(0, pair_b)                                                                        \
            )                                                                                                   \
        ),                                                                                                      \
        CHAOS_PP_ARBITRARY_PROMOTE(10)                                                                          \
    )

#undef PP_FLOAT_COMPUTE_DIV_STEP_STEP_RESULT
#define PP_FLOAT_COMPUTE_DIV_STEP_STEP_RESULT(prev_step_result, pair_a, pair_b, last_remainder, step_count) \
    PP_FLOAT_ADD(                                                                                           \
        PP_IF(CHAOS_PP_ARBITRARY_EQUAL(step_count, 0))                                                      \
        (                                                                                                   \
            PP_FLOAT(                                                                                       \
                CHAOS_PP_ARBITRARY_DIV(                                                                     \
                    CHAOS_PP_ELEM(0, pair_a),                                                               \
                    CHAOS_PP_ELEM(0, pair_b)                                                                \
                ),                                                                                          \
                CHAOS_PP_ARBITRARY_PROMOTE(0)                                                               \
            )                                                                                               \
        )(                                                                                                  \
            PP_FLOAT(                                                                                       \
                CHAOS_PP_ARBITRARY_DIV(                                                                     \
                    last_remainder,                                                                         \
                    CHAOS_PP_ELEM(0, pair_b)                                                                \
                ),                                                                                          \
                CHAOS_PP_ARBITRARY_NEG(step_count)                                                          \
            )                                                                                               \
        ),                                                                                                  \
        prev_step_result                                                                                    \
    )

#undef PP_FLOAT_INV_IMPL_WRAPPER_END_RESULT
#define PP_FLOAT_INV_IMPL_WRAPPER_END_RESULT(boolean, pair_b, res)  \
    PP_IF(boolean)                                                  \
    (                                                               \
        PP_FLOAT_NEG(                                               \
            PP_FLOAT_MUL(                                           \
                res,                                                \
                PP_FLOAT(                                           \
                    CHAOS_PP_ARBITRARY_PROMOTE(1),                  \
                    CHAOS_PP_ARBITRARY_NEG(                         \
                        CHAOS_PP_ELEM(                              \
                            1,                                      \
                            pair_b                                  \
                        )                                           \
                    )                                               \
                )                                                   \
            )                                                       \
        )                                                           \
    )(                                                              \
        PP_FLOAT_MUL(                                               \
            res,                                                    \
            PP_FLOAT(                                               \
                CHAOS_PP_ARBITRARY_PROMOTE(1),                      \
                CHAOS_PP_ARBITRARY_NEG(                             \
                    CHAOS_PP_ELEM(                                  \
                        1,                                          \
                        pair_b                                      \
                    )                                               \
                )                                                   \
            )                                                       \
        )                                                           \
    )

#undef PP_FLOAT_COMPUTE_DIV_STEP
#define PP_FLOAT_COMPUTE_DIV_STEP(_, prev_step_result, pair_a, pair_b, div_finished, last_remainder, step_count, precision) \
    PP_FLOAT_COMPUTE_DIV_STEP_STEP_RESULT(prev_step_result, pair_a, pair_b, last_remainder, step_count),                    \
    pair_a,                                                                                                                 \
    pair_b,                                                                                                                 \
    CHAOS_PP_ARBITRARY_EQUAL(                                                                                               \
        PP_FLOAT_COMPUTE_DIV_STEP_LAST_REMAINDER(prev_step_result, pair_a, pair_b, last_remainder, step_count),             \
        CHAOS_PP_ARBITRARY_PROMOTE(0)                                                                                       \
    ),                                                                                                                      \
    PP_FLOAT_COMPUTE_DIV_STEP_LAST_REMAINDER(prev_step_result, pair_a, pair_b, last_remainder, step_count),                 \
    CHAOS_PP_ARBITRARY_INC(step_count),                                                                                     \
    precision

#ifdef __INTELLISENSE__
    #undef PP_FLOAT_INV
    #define PP_FLOAT_INV(...) PP_FLOAT((0), (0))

    #undef PP_FLOAT_COMPUTE_DIV_STEP
    #define PP_FLOAT_COMPUTE_DIV_STEP(_, prev_step_result, pair_a, pair_b, div_finished, last_remainder, step_count, precision) \
        (0), (0), (0), (0), (0), (0), (0)
#endif

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
        ( 0 )                                                               \
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

#ifdef PP_DETECT
    #undef PP_DETECT
#endif
#define PP_DETECT(IDENT, IN) BOOST_PP_EQUAL(VA_COUNT(PP_DETECTOR_ ## IDENT ## _ ## IN), 2)

#if defined(REVERSE)
    #undef REVERSE
#endif
#define REVERSE(...) REVERSE_(__VA_ARGS__)

#if defined(REVERSE_)
    #undef REVERSE_
#endif
#define REVERSE_(...) EXPAND_ONE_TUPLE(CHAOS_PP_STRIP(CHAOS_PP_REVERSE((CHAOS_PP_TUPLE) (__VA_ARGS__))))

#if defined(EXP_INST_CLASS_ONE)
    #undef EXP_INST_CLASS_ONE
#endif
#define EXP_INST_CLASS_ONE(tuple) template TUPLE_FIRST_ARG(tuple)<EXPAND_ONE_TUPLE(TUPLE_OTHER_ARGS(tuple))>;

#undef DECL_EXP_INST_CLASS_ONE
#define DECL_EXP_INST_CLASS_ONE(tuple) extern EXP_INST_CLASS_ONE(tuple)

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

#undef DECL_EXP_INST_CLASS
#undef DECL_EXP_INST_STRUCT
#define DECL_EXP_INST_CLASS(T, ...) MAP(DECL_EXP_INST_CLASS_ONE, EXP_INST_CLASS_STRUCT_MAKE_TUPLE(class T, __VA_ARGS__))
#define DECL_EXP_INST_STRUCT(T, ...) MAP(DECL_EXP_INST_CLASS_ONE, EXP_INST_CLASS_STRUCT_MAKE_TUPLE(struct T, __VA_ARGS__))

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
#define SUPDEF_PRAGMA_DEF_BEG_REGEX "^\\s*#\\s*pragma\\s+" SUPDEF_PRAGMA_NAME "\\s+" SUPDEF_PRAGMA_DEFINE_BEGIN "\\s+(" SUPDEF_MACRO_ID_REGEX ")\\s*$"

#if defined(SUPDEF_PRAGMA_DEF_END_REGEX)
    #undef SUPDEF_PRAGMA_DEF_END_REGEX
#endif
// '#' at the beginning of the line, followed by any number of spaces >= 0
// then 'pragma' followed by any number of spaces > 0
// then 'supdef' followed by any number of spaces > 0
// then 'end' followed by any number of spaces > 0
#define SUPDEF_PRAGMA_DEF_END_REGEX "^\\s*#\\s*pragma\\s+" SUPDEF_PRAGMA_NAME "\\s+" SUPDEF_PRAGMA_DEFINE_END "\\s+(" SUPDEF_MACRO_ID_REGEX ")\\s*$"

#if defined(SUPDEF_PRAGMA_IMPORT)
    #undef SUPDEF_PRAGMA_IMPORT
#endif
#define SUPDEF_PRAGMA_IMPORT "import"

#if defined(SUPDEF_PRAGMA_IMPORT_REGEX)
    #undef SUPDEF_PRAGMA_IMPORT_REGEX
#endif
// '#' at the beginning of the line, followed by any number of spaces >= 0
// then 'pragma' followed by any number of spaces > 0
// then 'supdef' followed by any number of spaces > 0
// then 'import' followed by any number of spaces > 0
// then a C string literal followed by any number of spaces >= 0
#define SUPDEF_PRAGMA_IMPORT_REGEX "^\\s*#\\s*pragma\\s+" SUPDEF_PRAGMA_NAME "\\s+" SUPDEF_PRAGMA_IMPORT "\\s+[<>]*\"([^\"]*)\"[<>]*\\s*$"
#ifdef SUPDEF_PRAGMA_IMPORT_REGEX_ANGLE_BRACKETS
    #undef SUPDEF_PRAGMA_IMPORT_REGEX_ANGLE_BRACKETS
#endif
#define SUPDEF_PRAGMA_IMPORT_REGEX_ANGLE_BRACKETS "^\\s*#\\s*pragma\\s+" SUPDEF_PRAGMA_NAME "\\s+" SUPDEF_PRAGMA_IMPORT "\\s+[\"]*<([^>]*)>[\"]*\\s*$"
#ifdef SUPDEF_PRAGMA_IMPORT_REGEX_NO_QUOTES
    #undef SUPDEF_PRAGMA_IMPORT_REGEX_NO_QUOTES
#endif
// This one is without `""` nor `<>`
#define SUPDEF_PRAGMA_IMPORT_REGEX_NO_QUOTES "^\\s*#\\s*pragma\\s+" SUPDEF_PRAGMA_NAME "\\s+" SUPDEF_PRAGMA_IMPORT "\\s+([^\\s]+)\\s*$"
#ifdef SUPDEF_PRAGMA_IMPORT_REGEX_NO_PATH
    #undef SUPDEF_PRAGMA_IMPORT_REGEX_NO_PATH
#endif
// This one is just invalid
#define SUPDEF_PRAGMA_IMPORT_REGEX_NO_PATH "^\\s*#\\s*pragma\\s+" SUPDEF_PRAGMA_NAME "\\s+" SUPDEF_PRAGMA_IMPORT "\\s+([^\\s]*)\\s*$"

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
    #include <bits/extc++.h>
#endif
#include <execution>

#ifdef GCC_VERSION_AT_LEAST
    #undef GCC_VERSION_AT_LEAST
#endif
#if SUPDEF_COMPILER != 1
    #define GCC_VERSION_AT_LEAST(major, minor, patch) 0
#else
    #define GCC_VERSION_AT_LEAST(major, minor, patch) \
        (__GNUC__ > major || (__GNUC__ == major && (__GNUC_MINOR__ > minor || (__GNUC_MINOR__ == minor && __GNUC_PATCHLEVEL__ >= patch))))
#endif

// TODO: PUSH_MACRO and POP_MACRO are already defined under the respective names SAVE_MACRO and RESTORE_MACRO
#ifdef PUSH_MACRO_IMPL
    #undef PUSH_MACRO_IMPL
#endif
#define PUSH_MACRO_IMPL(MACNAME) _Pragma(PP_STRINGIZE(push_macro(MACNAME)))

#ifdef POP_MACRO_IMPL
    #undef POP_MACRO_IMPL
#endif
#define POP_MACRO_IMPL(MACNAME) _Pragma(PP_STRINGIZE(pop_macro(MACNAME)))

#ifdef PUSH_MACRO
    #undef PUSH_MACRO
#endif
#define PUSH_MACRO(MACNAME) PUSH_MACRO_IMPL(#MACNAME)

#ifdef POP_MACRO
    #undef POP_MACRO
#endif
#define POP_MACRO(MACNAME) POP_MACRO_IMPL(#MACNAME)

#if defined(__cplusplus) && !defined(restrict)
    #if SUPDEF_COMPILER == 1 // GCC
        #define restrict __restrict__
    #elif SUPDEF_COMPILER == 2 // Clang
        #define restrict __restrict__
    #elif SUPDEF_COMPILER == 3 // MSVC
        #define restrict __restrict
    #else
        #define restrict
    #endif
#endif

#ifndef TODO_DEFAULT_MSG
    #define TODO_DEFAULT_MSG "\nTODO at line " PP_STRINGIZE(__LINE__) " in file " __FILE__
#endif

#ifndef STATIC_TODO_RAW
    #define STATIC_TODO_RAW(...)                                    \
    PP_IF(                                                          \
        ISEMPTY(__VA_ARGS__)                                        \
    )                                                               \
    (                                                               \
        _Pragma(PP_STRINGIZE(message (TODO_DEFAULT_MSG)))           \
    )                                                               \
    (                                                               \
        _Pragma(PP_STRINGIZE(message ("\nTODO: " #__VA_ARGS__)))    \
    );
#endif

#ifndef STATIC_TODO
    #define STATIC_TODO(...)                                        \
    PP_IF(                                                          \
        ISEMPTY(__VA_ARGS__)                                        \
    )                                                               \
    (                                                               \
        _Pragma(PP_STRINGIZE(message (TODO_DEFAULT_MSG)))           \
    )                                                               \
    (                                                               \
        _Pragma(PP_STRINGIZE(message ("\nTODO: " __VA_ARGS__)))     \
    );
#endif

#ifndef TODO_RAW
    #define TODO_RAW(...)                                                           \
    PP_IF(                                                                          \
        ISEMPTY(__VA_ARGS__)                                                        \
    )                                                                               \
    (                                                                               \
        _Pragma(PP_STRINGIZE(message (TODO_DEFAULT_MSG)));                          \
        UNREACHABLE(                                                                \
            "Code block around line ",                                              \
            __LINE__,                                                               \
            " in file ",                                                            \
            __FILE__,                                                               \
            " contains a TODO.",                                                    \
            " Please fix this before using this function / class / macro / etc. "   \
        )                                                                           \
    )                                                                               \
    (                                                                               \
        _Pragma(PP_STRINGIZE(message ("\nTODO: " #__VA_ARGS__)));                   \
        UNREACHABLE(                                                                \
            "Code block around line ",                                              \
            __LINE__,                                                               \
            " in file ",                                                            \
            __FILE__,                                                               \
            " contains a TODO.",                                                    \
            " Please fix this before using this function / class / macro / etc. ",  \
            "TODO: " #__VA_ARGS__                                                   \
        )                                                                           \
    );
#endif

#ifndef TODO
    #define TODO(...)                                                               \
    PP_IF(                                                                          \
        ISEMPTY(__VA_ARGS__)                                                        \
    )                                                                               \
    (                                                                               \
        _Pragma(PP_STRINGIZE(message (TODO_DEFAULT_MSG)));                          \
        UNREACHABLE(                                                                \
            "Code block around line ",                                              \
            __LINE__,                                                               \
            " in file ",                                                            \
            __FILE__,                                                               \
            " contains a TODO.",                                                    \
            " Please fix this before using this function / class / macro / etc. "   \
        )                                                                           \
    )                                                                               \
    (                                                                               \
        _Pragma(PP_STRINGIZE(message ("\nTODO: " __VA_ARGS__)));                    \
        UNREACHABLE(                                                                \
            "Code block around line ",                                              \
            __LINE__,                                                               \
            " in file ",                                                            \
            __FILE__,                                                               \
            " contains a TODO.",                                                    \
            " Please fix this before using this function / class / macro / etc. ",  \
            "TODO: " __VA_ARGS__                                                    \
        )                                                                           \
    );
#endif

#undef PP_INVOKE
#define PP_INVOKE(macro, ...) ID(PP_INVOKE_IMPL(macro, __VA_ARGS__))

#undef PP_INVOKE_IMPL
#define PP_INVOKE_IMPL(macro, ...) macro (__VA_ARGS__)

#undef MAKE_PP_OPTIONAL
#define MAKE_PP_OPTIONAL(...) MAKE_PP_OPTIONAL_IMPL(__VA_ARGS__)

#undef MAKE_PP_OPTIONAL_IMPL
#define MAKE_PP_OPTIONAL_IMPL(...) \
    PP_IF(                         \
        ISEMPTY(__VA_ARGS__)       \
    )                              \
    (                              \
        (PP_NULLOPT)               \
    )                              \
    (                              \
        (PP_OPT, (__VA_ARGS__))    \
    )

#undef PP_GET_OPT
#define PP_GET_OPT(opt) PP_GET_OPT_IMPL(opt)

#undef PP_GET_OPT_IMPL
#define PP_GET_OPT_IMPL(opt) EXPAND_ONE_TUPLE(TUPLE_SECOND_ARG(opt))

#undef PP_IS_NULLOPT
#define PP_IS_NULLOPT(opt) PP_IS_NULLOPT_IMPL(TUPLE_FIRST_ARG(opt))

#undef PP_IS_NULLOPT_IMPL
#define PP_IS_NULLOPT_IMPL(kw) PP_DETECT(kw, PP_OPTIONAL)

#undef PP_DETECTOR_PP_NULLOPT_PP_OPTIONAL
#define PP_DETECTOR_PP_NULLOPT_PP_OPTIONAL ~, ~

#undef PP_IF_OPT
#define PP_IF_OPT(opt, if_true, if_false) PP_IF_OPT_IMPL(opt, if_true, if_false)

#undef PP_IF_OPT_IMPL
#define PP_IF_OPT_IMPL(opt, if_true, if_false) \
    PP_IF(                                     \
        PP_IS_NULLOPT(opt)                     \
    )                                          \
    (                                          \
        if_false                               \
    )                                          \
    (                                          \
        if_true                                \
    )

#include <boost/contract.hpp>

#ifndef CONTRACT
    #define CONTRACT(type, ...) ID(PP_INVOKE(PP_CAT(PP_CAT(CONTRACT_, type), _IMPL), MAP_LIST(MAKE_PP_OPTIONAL, __VA_ARGS__,,,,,,,,,,)))
#endif
#ifndef CONTRACT_FN_IMPL
    #define CONTRACT_FN_IMPL(opt_check_name, opt_precond, opt_old, opt_postcond, opt_except, ...) /* __VA_ARGS__ ignored */ \
        boost::contract::check PP_IF_OPT(opt_check_name, PP_GET_OPT(opt_check_name), PP_CAT(contract_check_, __LINE__)) =   \
            boost::contract::function().precondition(PP_IF_OPT(opt_precond, PP_GET_OPT(opt_precond), [](){}))               \
                                       .old(PP_IF_OPT(opt_old, PP_GET_OPT(opt_old), [](){}))                                \
                                       .postcondition(PP_IF_OPT(opt_postcond, PP_GET_OPT(opt_postcond), [](){}))            \
                                       .except(PP_IF_OPT(opt_except, PP_GET_OPT(opt_except), [](){}))
#endif

#ifdef SUPDEF_CONTRACT_VIOLATION
    #undef SUPDEF_CONTRACT_VIOLATION
#endif
#define SUPDEF_CONTRACT_VIOLATION(type) (::SupDef::ContractViolation(::SupDef::ContractType::type))

#undef IS_PURE
#define IS_PURE(expr) \
    (static_cast<bool>(__builtin_object_size( ( ((void)(expr)), "" ) , 2)))

#ifdef ASSUME
    #undef ASSUME
#endif
#ifdef __has_cpp_attribute
    #if __has_cpp_attribute(assume)
        #define ASSUME(expr) [[assume((expr))]]
    #endif
#endif
#ifndef ASSUME
    #if SUPDEF_COMPILER == 1
        #ifdef __has_attribute
            #if __has_attribute(assume)
                #define ASSUME(expr) __attribute__((assume((expr))))
            #endif
        #endif
        #ifndef ASSUME
            #define ASSUME(expr)                    \
                (IS_PURE((expr))                    \
                    ? ((expr)                       \
                        ? ((void) 0)                \
                        : __builtin_unreachable())  \
                    : UNREACHABLE("Do not use `ASSUME` with non-pure expressions"))
        #endif
    #elif SUPDEF_COMPILER == 2
        #define ASSUME(expr) __builtin_assume((expr))
    #elif SUPDEF_COMPILER == 3
        #define ASSUME(expr) __assume((expr))
    #else
        #define ASSUME(expr)
    #endif
#endif

#undef LIKELY_BRANCH_IMPL
#ifdef likely_if
    #undef likely_if
#endif
#ifdef likely_else_if
    #undef likely_else_if
#endif
#ifdef likely_else
    #undef likely_else
#endif
#ifdef likely_while
    #undef likely_while
#endif

#define likely_if(expr) if LIKELY_BRANCH_IMPL(expr)
#define likely_else_if(expr) else if LIKELY_BRANCH_IMPL(expr)
#define likely_while(expr) while LIKELY_BRANCH_IMPL(expr)

#ifdef __has_cpp_attribute
    #if __has_cpp_attribute(likely)
        #define LIKELY_BRANCH_IMPL(expr) (bool((expr))) [[likely]]
        #define likely_else else [[likely]]
    #endif
#endif
#ifndef likely_else
    #define likely_else else
#endif
#ifndef LIKELY_BRANCH_IMPL
    #if SUPDEF_COMPILER == 1
        #ifdef __has_builtin
            #if __has_builtin(__builtin_expect)
                #define LIKELY_BRANCH_IMPL(expr) (__builtin_expect(long(bool((expr))), long(true)))
            #endif
        #endif
        #ifndef LIKELY_BRANCH_IMPL
            #define LIKELY_BRANCH_IMPL(expr) ((expr))
        #endif
    #elif SUPDEF_COMPILER == 2
        #define LIKELY_BRANCH_IMPL(expr) (__builtin_expect(long(bool((expr))), long(true)))
    #elif SUPDEF_COMPILER == 3
        #define LIKELY_BRANCH_IMPL(expr) ((expr))
    #else
        #define LIKELY_BRANCH_IMPL(expr) ((expr))
    #endif
#endif

#undef UNLIKELY_BRANCH_IMPL
#ifdef unlikely_if
    #undef unlikely_if
#endif
#ifdef unlikely_else_if
    #undef unlikely_else_if
#endif
#ifdef unlikely_else
    #undef unlikely_else
#endif
#ifdef unlikely_while
    #undef unlikely_while
#endif

#define unlikely_if(expr) if UNLIKELY_BRANCH_IMPL(expr)
#define unlikely_else_if(expr) else if UNLIKELY_BRANCH_IMPL(expr)
#define unlikely_while(expr) while UNLIKELY_BRANCH_IMPL(expr)

#ifdef __has_cpp_attribute
    #if __has_cpp_attribute(unlikely)
        #define UNLIKELY_BRANCH_IMPL(expr) (bool((expr))) [[unlikely]]
        #define unlikely_else else [[unlikely]]
    #endif
#endif
#ifndef unlikely_else
    #define unlikely_else else
#endif
#ifndef UNLIKELY_BRANCH_IMPL
    #if SUPDEF_COMPILER == 1
        #ifdef __has_builtin
            #if __has_builtin(__builtin_expect)
                #define UNLIKELY_BRANCH_IMPL(expr) (__builtin_expect(long(bool((expr))), long(false)))
            #endif
        #endif
        #ifndef UNLIKELY_BRANCH_IMPL
            #define UNLIKELY_BRANCH_IMPL(expr) ((expr))
        #endif
    #elif SUPDEF_COMPILER == 2
        #define UNLIKELY_BRANCH_IMPL(expr) (__builtin_expect(long(bool((expr))), long(false)))
    #elif SUPDEF_COMPILER == 3
        #define UNLIKELY_BRANCH_IMPL(expr) ((expr))
    #else
        #define UNLIKELY_BRANCH_IMPL(expr) ((expr))
    #endif
#endif

#undef PROBABLY_BRANCH_IMPL
#ifdef probably_if
    #undef probably_if
#endif
#ifdef probably_else_if
    #undef probably_else_if
#endif
#ifdef probably_else
    #undef probably_else
#endif
#ifdef probably_while
    #undef probably_while
#endif

#define probably_if(expr, percentage, ...) if PROBABLY_BRANCH_IMPL(expr, percentage, PP_IF(ISEMPTY(__VA_ARGS__))(CLASSIC)(__VA_ARGS__))
#define probably_else_if(expr, percentage, ...) else if PROBABLY_BRANCH_IMPL(expr, percentage, PP_IF(ISEMPTY(__VA_ARGS__))(CLASSIC)(__VA_ARGS__))
#define probably_else(percentage, ...) else
#define probably_while(expr, percentage, ...) while PROBABLY_BRANCH_IMPL(expr, percentage, PP_IF(ISEMPTY(__VA_ARGS__))(CLASSIC)(__VA_ARGS__))

#if (SUPDEF_COMPILER == 1 || SUPDEF_COMPILER == 2) && !defined(__INTELLISENSE__)
    #ifdef __has_builtin
        #if __has_builtin(__builtin_expect_with_probability)
            #define PROBABLY_BRANCH_IMPL(expr, percentage, backend)     \
                (                                                       \
                    __builtin_expect_with_probability(                  \
                        long(                                           \
                            bool(                                       \
                                (expr)                                  \
                            )                                           \
                        ),                                              \
                        long(true),                                     \
                        double(                                         \
                            PP_IF(                                      \
                                COMPTIME_SCALE_GET_BACKEND(backend)     \
                            )(                                          \
                                COMPTIME_SCALE(                         \
                                    double,                             \
                                    percentage,                         \
                                    (0, 100),                           \
                                    (0, 1),                             \
                                    CHAOS                               \
                                )                                       \
                            )(                                          \
                                COMPTIME_SCALE(                         \
                                    double,                             \
                                    (percentage),                       \
                                    (0, 100),                           \
                                    (0, 1),                             \
                                    CLASSIC                             \
                                )                                       \
                            )                                           \
                        )                                               \
                    )                                                   \
                )
        #endif
    #endif
#endif
#ifndef PROBABLY_BRANCH_IMPL
    #define PROBABLY_BRANCH_IMPL(expr, percentage, ...) ((expr))
#endif

#undef until
#define until(expr) while (!bool((expr)))

#undef likely_until
#define likely_until(expr) while LIKELY_BRANCH_IMPL(!(expr))

#undef unlikely_until
#define unlikely_until(expr) while UNLIKELY_BRANCH_IMPL(!(expr))

#undef probably_until
#define probably_until(expr, percentage, ...) while PROBABLY_BRANCH_IMPL(!(expr), percentage, PP_IF(ISEMPTY(__VA_ARGS__))(CLASSIC)(__VA_ARGS__))

#undef unless
#undef else_unless
#define unless(expr) if (!bool((expr)))
#define else_unless(expr) else if (!bool((expr)))

#undef likely_unless
#undef likely_else_unless
#define likely_unless(expr) if LIKELY_BRANCH_IMPL(!(expr))
#define likely_else_unless(expr) else if LIKELY_BRANCH_IMPL(!(expr))

#undef unlikely_unless
#undef unlikely_else_unless
#define unlikely_unless(expr) if UNLIKELY_BRANCH_IMPL(!(expr))
#define unlikely_else_unless(expr) else if UNLIKELY_BRANCH_IMPL(!(expr))

#undef probably_unless
#undef probably_else_unless
#define probably_unless(expr, percentage, ...) if PROBABLY_BRANCH_IMPL(!(expr), percentage, PP_IF(ISEMPTY(__VA_ARGS__))(CLASSIC)(__VA_ARGS__))
#define probably_else_unless(expr, percentage, ...) else if PROBABLY_BRANCH_IMPL(!(expr), percentage, PP_IF(ISEMPTY(__VA_ARGS__))(CLASSIC)(__VA_ARGS__))

#undef COMPTIME_ABS
#define COMPTIME_ABS(value) ( [](auto x) consteval { return x < 0 ? -x : x; }(value) )

#undef COMPTIME_SCALE
/**
 * @def COMPTIME_SCALE(type, value, min_max_from, min_max_new)
 * @brief Scales (or normalizes) a value from one range to another at compile time.
 * @param type The type used for computations.
 * @param value The value to scale.
 * @param min_max_from The tupled minimum and maximum values of the original range.
 * @param min_max_new The tupled minimum and maximum values of the new range.
 * @param backend Whether to try to use the chaos-pp backend for this invocation.
 * @return The scaled value.
 * @example COMPTIME_SCALE(double, 50, (0, 100), (0, 1)) will return 0.5.
 * @example COMPTIME_SCALE(float, 0.25, (0, 1), (-100, 0)) will return -75.0f.
 * @example COMPTIME_SCALE(double, 0.5, (0, 1), (-2000, 2000)) will return 0.0.
 */
#define COMPTIME_SCALE(type, value, min_max_from, min_max_new, backend) \
    CCAST(                                                              \
        type,                                                           \
        PP_IF(                                                          \
            CHAOS_PP_AND                                                \
            (                                                           \
                COMPTIME_SCALE_GET_BACKEND(backend)                     \
            )(                                                          \
                COMPTIME_SCALE_HELPER_COND(                             \
                    value,                                              \
                    FIRST_ARG(EXPAND_ONE_TUPLE(min_max_from)),          \
                    LAST_ARG(EXPAND_ONE_TUPLE(min_max_from)),           \
                    FIRST_ARG(EXPAND_ONE_TUPLE(min_max_new)),           \
                    LAST_ARG(EXPAND_ONE_TUPLE(min_max_new))             \
                )                                                       \
            )                                                           \
        )                                                               \
        (                                                               \
            COMPTIME_SCALE_IMPL2(                                       \
                type,                                                   \
                value,                                                  \
                FIRST_ARG(EXPAND_ONE_TUPLE(min_max_from)),              \
                LAST_ARG(EXPAND_ONE_TUPLE(min_max_from)),               \
                FIRST_ARG(EXPAND_ONE_TUPLE(min_max_new)),               \
                LAST_ARG(EXPAND_ONE_TUPLE(min_max_new))                 \
            )                                                           \
        )(                                                              \
            COMPTIME_SCALE_IMPL1(                                       \
                CCAST(type, value),                                     \
                CCAST(type, FIRST_ARG(EXPAND_ONE_TUPLE(min_max_from))), \
                CCAST(type, LAST_ARG(EXPAND_ONE_TUPLE(min_max_from))),  \
                CCAST(type, FIRST_ARG(EXPAND_ONE_TUPLE(min_max_new))),  \
                CCAST(type, LAST_ARG(EXPAND_ONE_TUPLE(min_max_new)))    \
            )                                                           \
        )                                                               \
    )

#undef COMPTIME_SCALE_GET_BACKEND_IMPL
#define COMPTIME_SCALE_GET_BACKEND_IMPL(backend) VARIADIC_CAT(COMPTIME_SCALE_, backend, _BACKEND)

#undef COMPTIME_SCALE_GET_BACKEND
#define COMPTIME_SCALE_GET_BACKEND(backend) COMPTIME_SCALE_GET_BACKEND_IMPL(backend)

#undef COMPTIME_SCALE_CLASSIC_BACKEND
#define COMPTIME_SCALE_CLASSIC_BACKEND CHAOS_PP_BOOL(0)

#undef COMPTIME_SCALE_CHAOS_BACKEND
#define COMPTIME_SCALE_CHAOS_BACKEND CHAOS_PP_BOOL(1)

#undef COMPTIME_SCALE_HELPER_COND_VALID_ARG
#define COMPTIME_SCALE_HELPER_COND_VALID_ARG(arg)   \
    CHAOS_PP_OR                                     \
    (                                               \
        CHAOS_PP_IS_NUMERIC(arg)                    \
    )                                               \
    (                                               \
        CHAOS_PP_IS_NUMERIC(PP_EAT arg)             \
    )

#undef COMPTIME_SCALE_HELPER_COND
#define COMPTIME_SCALE_HELPER_COND(value, min_from, max_from, min_new, max_new) \
    TRUE_FOR_ALL(                                                               \
        CHAOS_PP_LAMBDA(COMPTIME_SCALE_HELPER_COND_VALID_ARG)(CHAOS_PP_ARG(1)), \
        value,                                                                  \
        min_from,                                                               \
        max_from,                                                               \
        min_new,                                                                \
        max_new                                                                 \
    )

#undef COMPTIME_SCALE_IMPL1
#define COMPTIME_SCALE_IMPL1(value, min_from, max_from, min_new, max_new)   \
    (                                                                       \
        (                                                                   \
            (                                                               \
                (value - min_from) * (max_new - min_new)                    \
            ) / (max_from - min_from)                                       \
        ) + min_new                                                         \
    )

#undef COMPTIME_SCALE_IMPL2
/**
 * @brief This implementation is selected if chaos-pp can work with provided value to calculate the scaled output using only macros.
 * 
 */
#define COMPTIME_SCALE_IMPL2(type, value, min_from, max_from, min_new, max_new) \
    PP_FLOAT_DEMOTE(                                                            \
        COMPTIME_SCALE_IMPL2_FORMULA(                                           \
            PP_FLOAT_PROMOTE(value, 0),                                         \
            PP_FLOAT_PROMOTE(min_from, 0),                                      \
            PP_FLOAT_PROMOTE(max_from, 0),                                      \
            PP_FLOAT_PROMOTE(min_new, 0),                                       \
            PP_FLOAT_PROMOTE(max_new, 0)                                        \
        ),                                                                      \
        type                                                                    \
    )

#undef COMPTIME_SCALE_IMPL2_FORMULA
#define COMPTIME_SCALE_IMPL2_FORMULA(value, min_from, max_from, min_new, max_new)   \
    PP_FLOAT_ADD(                                                                   \
        min_new,                                                                    \
        PP_FLOAT_DIV(                                                               \
            PP_FLOAT_MUL(                                                           \
                PP_FLOAT_SUB(                                                       \
                    value,                                                          \
                    min_from                                                        \
                ),                                                                  \
                PP_FLOAT_SUB(                                                       \
                    max_new,                                                        \
                    min_new                                                         \
                )                                                                   \
            ),                                                                      \
            PP_FLOAT_SUB(                                                           \
                max_from,                                                           \
                min_from                                                            \
            )                                                                       \
        )                                                                           \
    )

#undef COMPTIME_FLOAT_EQ
#define COMPTIME_FLOAT_EQ(type, a, b, epsilon) bool(COMPTIME_ABS(type(type(a) - type(b))) <= type(epsilon))

#undef ABS_UNSAFE
#define ABS_UNSAFE(value) ((value) < 0 ? -(value) : (value))

#undef FLOAT_EQ
#define FLOAT_EQ(type, a, b, epsilon) bool(ABS_UNSAFE(type(type(a) - type(b))) <= type(epsilon))

static_assert(COMPTIME_FLOAT_EQ(double, 0.5, 0.5, 0.0001));
static_assert(COMPTIME_FLOAT_EQ(float,  0.5, 0.5, 0.0001));
static_assert(FLOAT_EQ(double, 0.5, 0.5, 0.0001));
static_assert(FLOAT_EQ(float,  0.5, 0.5, 0.0001));

#undef ATTRIBUTE_ALIGNED
#undef aligned_at
#if SUPDEF_COMPILER == 1
    #define ATTRIBUTE_ALIGNED(size) [[__gnu__::__aligned__(size)]]
#elif SUPDEF_COMPILER == 2 // Clang
    #define ATTRIBUTE_ALIGNED(size) [[clang::aligned(size)]]
#else
    #define ATTRIBUTE_ALIGNED(size) __declspec(align(size))
#endif
#define aligned_at(size) ATTRIBUTE_ALIGNED(size)

#undef ATTRIBUTE_COLD
#undef symbol_cold
#if SUPDEF_COMPILER == 1
    #define ATTRIBUTE_COLD [[__gnu__::__cold__]]
#elif SUPDEF_COMPILER == 2 // Clang
    #define ATTRIBUTE_COLD [[clang::cold]]
#else
    #define ATTRIBUTE_COLD
#endif
#define symbol_cold ATTRIBUTE_COLD

#undef ATTRIBUTE_HOT
#undef symbol_hot
#if SUPDEF_COMPILER == 1
    #define ATTRIBUTE_HOT [[__gnu__::__hot__]]
#elif SUPDEF_COMPILER == 2 // Clang
    #define ATTRIBUTE_HOT [[clang::hot]]
#else
    #define ATTRIBUTE_HOT ATTRIBUTE_USED
#endif
#define symbol_hot ATTRIBUTE_HOT

#undef ATTRIBUTE_USED
#undef symbol_keep
#if SUPDEF_COMPILER == 1
    #define ATTRIBUTE_USED [[__gnu__::__used__]]
#elif SUPDEF_COMPILER == 2 // Clang
    #define ATTRIBUTE_USED [[clang::used]]
#else
    #define ATTRIBUTE_USED
#endif
#define symbol_keep ATTRIBUTE_USED

#undef ATTRIBUTE_UNUSED
#undef symbol_unused
#ifdef __has_cpp_attribute
    #ifdef maybe_unused
        #error "maybe_unused shouldn't be defined"
    #endif
    #if __has_cpp_attribute(maybe_unused)
        #define ATTRIBUTE_UNUSED [[maybe_unused]]
    #endif
#endif
#ifndef ATTRIBUTE_UNUSED
    #if SUPDEF_COMPILER == 1
        #define ATTRIBUTE_UNUSED [[__gnu__::__unused__]]
    #elif SUPDEF_COMPILER == 2 // Clang
        #define ATTRIBUTE_UNUSED [[clang::unused]]
    #else
        #define ATTRIBUTE_UNUSED
    #endif
#endif
#define symbol_unused ATTRIBUTE_UNUSED

#undef ATTRIBUTE_FALLTHROUGH
#undef case_fallthrough
#ifdef __has_cpp_attribute
    #ifdef fallthrough
        #error "fallthrough shouldn't be defined"
    #endif
    #if __has_cpp_attribute(fallthrough)
        #define ATTRIBUTE_FALLTHROUGH [[fallthrough]]
    #endif
#endif
#ifndef ATTRIBUTE_FALLTHROUGH
    #if SUPDEF_COMPILER == 1
        #define ATTRIBUTE_FALLTHROUGH [[__gnu__::__fallthrough__]]
    #elif SUPDEF_COMPILER == 2 // Clang
        #define ATTRIBUTE_FALLTHROUGH [[clang::fallthrough]]
    #else
        #define ATTRIBUTE_FALLTHROUGH
    #endif
#endif
#define case_fallthrough ATTRIBUTE_FALLTHROUGH

#undef ATTRIBUTE_NODISCARD
#undef warn_unused_result
#ifdef __has_cpp_attribute
    #ifdef nodiscard
        #error "nodiscard shouldn't be defined"
    #endif
    #if __has_cpp_attribute(nodiscard)
        #define ATTRIBUTE_NODISCARD(...) [[nodiscard __VA_OPT__((__VA_ARGS__))]]
    #endif
#endif
#ifndef ATTRIBUTE_NODISCARD
    #if SUPDEF_COMPILER == 1
        #define ATTRIBUTE_NODISCARD(...) [[__gnu__::__warn_unused_result__]]
    #elif SUPDEF_COMPILER == 2 // Clang
        #define ATTRIBUTE_NODISCARD(...) [[clang::__warn_unused_result__]]
    #else
        #define ATTRIBUTE_NODISCARD(...)
    #endif
#endif
#define warn_unused_result(...) ATTRIBUTE_NODISCARD(__VA_ARGS__)

#undef ATTRIBUTE_WARNING
#undef warn_usage_suggest_alternative
#if SUPDEF_COMPILER == 1
    #define ATTRIBUTE_WARNING(msg) [[__gnu__::__warning__(msg)]]
#elif SUPDEF_COMPILER == 2 // Clang
    #define ATTRIBUTE_WARNING(msg) [[clang::warning(msg)]]
#else
    #define ATTRIBUTE_WARNING(msg) STATIC_TODO("Attribute warning not implemented for this compiler")
#endif
#define warn_usage_suggest_alternative(msg, ...)    \
    PP_IF(ISEMPTY(__VA_ARGS__))(                    \
        ATTRIBUTE_WARNING(                          \
            "Usage of this function is "            \
            "discouraged: use \"" msg "\" instead"  \
        )                                           \
    )(                                              \
        ATTRIBUTE_WARNING(                          \
            "Usage of this function is "            \
            "discouraged: use one of [ "            \
            PP_STRINGIZE(                           \
                MAP_LIST(                           \
                    ID, msg, __VA_ARGS__            \
                )                                   \
            )                                       \
            " ] instead"                            \
        )                                           \
    )

#undef ATTRIBUTE_INIT_PRIORITY
#undef static_init_priority
#if SUPDEF_COMPILER == 1
    #define ATTRIBUTE_INIT_PRIORITY(priority) [[__gnu__::__init_priority__(priority)]]
#elif SUPDEF_COMPILER == 2 // Clang
    #define ATTRIBUTE_INIT_PRIORITY(priority) [[clang::init_priority(priority)]]
#else
    #define ATTRIBUTE_INIT_PRIORITY(priority) STATIC_TODO("Attribute init priority not implemented for this compiler")
#endif
#define static_init_priority(priority) ATTRIBUTE_INIT_PRIORITY(priority)

#undef ATTRIBUTE_RETURNS_NONNULL
#undef returns_nonnull
#if SUPDEF_COMPILER == 1
    #define ATTRIBUTE_RETURNS_NONNULL [[__gnu__::__returns_nonnull__]]
#elif SUPDEF_COMPILER == 2 // Clang
    #define ATTRIBUTE_RETURNS_NONNULL [[clang::returns_nonnull]]
#else
    #define ATTRIBUTE_RETURNS_NONNULL
#endif
#define returns_nonnull ATTRIBUTE_RETURNS_NONNULL

#undef ATTRIBUTE_NONNULL_PARAMS
#undef nonnull_params
#if SUPDEF_COMPILER == 1
    #define ATTRIBUTE_NONNULL_PARAMS(...) [[__gnu__::__nonnull__(__VA_ARGS__)]]
#elif SUPDEF_COMPILER == 2 // Clang
    #define ATTRIBUTE_NONNULL_PARAMS(...) [[clang::nonnull(__VA_ARGS__)]]
#else
    #define ATTRIBUTE_NONNULL_PARAMS(...)
#endif
#define nonnull_params(...) ATTRIBUTE_NONNULL_PARAMS(__VA_ARGS__)

#undef ATTRIBUTE_MALLOC
#undef ATTRIBUTE_ALLOC_SIZE
#undef ATTRIBUTE_ALLOC_ALIGN
#undef malloc_like
#undef calloc_like
#undef realloc_like
#undef reallocarray_like
#undef free_like
#if SUPDEF_COMPILER == 1
    #define ATTRIBUTE_MALLOC [[__gnu__::__malloc__]]
    #define ATTRIBUTE_ALLOC_SIZE(index, ...) [[__gnu__::__alloc_size__(index __VA_OPT__(,) __VA_ARGS__)]]
    #define ATTRIBUTE_ALLOC_ALIGN(index) [[__gnu__::__alloc_align__(index)]]
#elif SUPDEF_COMPILER == 2 // Clang
    #define ATTRIBUTE_MALLOC [[clang::malloc]]
    #define ATTRIBUTE_ALLOC_SIZE(index, ...) [[clang::alloc_size(index __VA_OPT__(,) __VA_ARGS__)]]
    #define ATTRIBUTE_ALLOC_ALIGN(index) [[clang::alloc_align(index)]]
#else
    #define ATTRIBUTE_MALLOC
    #define ATTRIBUTE_ALLOC_SIZE(index, ...)
    #define ATTRIBUTE_ALLOC_ALIGN(index)
#endif
#define malloc_like(size_index, ...)        \
    ATTRIBUTE_MALLOC                        \
    ATTRIBUTE_NODISCARD(                    \
        "Allocated pointer should be used"  \
    )                                       \
    ATTRIBUTE_RETURNS_NONNULL               \
    ATTRIBUTE_ALLOC_SIZE(                   \
        size_index                          \
    )                                       \
    __VA_OPT__(                             \
        ATTRIBUTE_ALLOC_ALIGN(              \
            FIRST_ARG(                      \
                __VA_ARGS__                 \
            )                               \
        )                                   \
    )
#define calloc_like(size_index, count_index, ...)   \
    ATTRIBUTE_MALLOC                                \
    ATTRIBUTE_NODISCARD(                            \
        "Allocated pointer should be used"          \
    )                                               \
    ATTRIBUTE_RETURNS_NONNULL                       \
    ATTRIBUTE_ALLOC_SIZE(                           \
        size_index,                                 \
        count_index                                 \
    )                                               \
    __VA_OPT__(                                     \
        ATTRIBUTE_ALLOC_ALIGN(                      \
            FIRST_ARG(                              \
                __VA_ARGS__                         \
            )                                       \
        )                                           \
    )
#define realloc_like(size_index, ...)               \
    ATTRIBUTE_NODISCARD(                            \
        "Allocated pointer should be used"          \
    )                                               \
    ATTRIBUTE_RETURNS_NONNULL                       \
    ATTRIBUTE_ALLOC_SIZE(                           \
        size_index                                  \
    )                                               \
    __VA_OPT__(                                     \
        ATTRIBUTE_ALLOC_ALIGN(                      \
            FIRST_ARG(                              \
                __VA_ARGS__                         \
            )                                       \
        )                                           \
    )
#define reallocarray_like(size_index, count_index, ...) \
    ATTRIBUTE_NODISCARD(                                \
        "Allocated pointer should be used"              \
    )                                                   \
    ATTRIBUTE_RETURNS_NONNULL                           \
    ATTRIBUTE_ALLOC_SIZE(                               \
        size_index,                                     \
        count_index                                     \
    )                                                   \
    __VA_OPT__(                                         \
        ATTRIBUTE_ALLOC_ALIGN(                          \
            FIRST_ARG(                                  \
                __VA_ARGS__                             \
            )                                           \
        )                                               \
    )
#define free_like(ptr_index)            \
    ATTRIBUTE_NONNULL_PARAMS(ptr_index)

#undef ATTRIBUTE_NO_UNIQUE_ADDRESS
#undef optimize_space
#ifdef __has_cpp_attribute
    #ifdef no_unique_address
        #error "no_unique_address shouldn't be defined"
    #endif
    #if __has_cpp_attribute(no_unique_address)
        #define ATTRIBUTE_NO_UNIQUE_ADDRESS [[no_unique_address]]
    #endif
#endif
#ifndef ATTRIBUTE_NO_UNIQUE_ADDRESS
    #if SUPDEF_COMPILER == 1
        #define ATTRIBUTE_NO_UNIQUE_ADDRESS
    #elif SUPDEF_COMPILER == 2 // Clang
        #define ATTRIBUTE_NO_UNIQUE_ADDRESS
    #else
        #define ATTRIBUTE_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
    #endif
#endif
#define optimize_space ATTRIBUTE_NO_UNIQUE_ADDRESS

#undef ATTRIBUTE_MAY_ALIAS
#undef aliasing_type
#undef declare_aliasing_type
#if SUPDEF_COMPILER == 1
    #define ATTRIBUTE_MAY_ALIAS __attribute__((__may_alias__))//[[__gnu__::__may_alias__]]
#elif SUPDEF_COMPILER == 2 && defined(__has_attribute)
    #if __has_attribute(may_alias)
        #define ATTRIBUTE_MAY_ALIAS [[clang::may_alias]]
    #else
        #define ATTRIBUTE_MAY_ALIAS
    #endif
#else
    #define ATTRIBUTE_MAY_ALIAS
#endif
#define aliasing_type(type) PP_CAT(aliasing_, type)
#define declare_aliasing_type(type, ...) typedef type PP_IF(ISEMPTY(__VA_ARGS__))(aliasing_type(type))(__VA_ARGS__) ATTRIBUTE_MAY_ALIAS

#undef ATTRIBUTE_EXPECTED_THROW
#undef should_throw
#if SUPDEF_COMPILER == 1 && defined(__has_attribute)
    #if __has_attribute(expected_throw)
        #define ATTRIBUTE_EXPECTED_THROW [[__gnu__::__expected_throw__]]
    #else
        #define ATTRIBUTE_EXPECTED_THROW
    #endif
#elif SUPDEF_COMPILER == 2 && defined(__has_attribute)
    #if __has_attribute(expected_throw)
        #define ATTRIBUTE_EXPECTED_THROW [[clang::expected_throw]]
    #else
        #define ATTRIBUTE_EXPECTED_THROW
    #endif
#else
    #define ATTRIBUTE_EXPECTED_THROW
#endif
#define should_throw ATTRIBUTE_EXPECTED_THROW

#undef ATTRIBUTE_FLATTEN
#undef flatten
#if SUPDEF_COMPILER == 1
    #define ATTRIBUTE_FLATTEN [[__gnu__::__flatten__]]
#elif SUPDEF_COMPILER == 2 && defined(__has_attribute)
    #if __has_attribute(flatten)
        #define ATTRIBUTE_FLATTEN [[clang::flatten]]
    #else
        #define ATTRIBUTE_FLATTEN
    #endif
#else
    #define ATTRIBUTE_FLATTEN
#endif
#define flatten ATTRIBUTE_FLATTEN

#undef ATTRIBUTE_UNINITIALIZED
#undef no_init
#if SUPDEF_COMPILER == 1
    #define ATTRIBUTE_UNINITIALIZED [[__gnu__::__uninitialized__]]
#elif SUPDEF_COMPILER == 2 && defined(__has_attribute)
    #if __has_attribute(uninitialized)
        #define ATTRIBUTE_UNINITIALIZED [[clang::uninitialized]]
    #else
        #define ATTRIBUTE_UNINITIALIZED
    #endif
#else
    #define ATTRIBUTE_UNINITIALIZED
#endif
#define no_init ATTRIBUTE_UNINITIALIZED

#undef ATTRIBUTE_NORETURN
#undef no_return
#ifdef __has_cpp_attribute
    #ifdef noreturn
        #error "noreturn shouldn't be defined"
    #endif
    #if __has_cpp_attribute(noreturn)
        #define ATTRIBUTE_NORETURN [[noreturn]]
    #endif
#endif
#ifndef ATTRIBUTE_NORETURN
    #if SUPDEF_COMPILER == 1
        #define ATTRIBUTE_NORETURN [[__gnu__::__noreturn__]]
    #elif SUPDEF_COMPILER == 2 // Clang
        #define ATTRIBUTE_NORETURN [[clang::noreturn]]
    #else
        #define ATTRIBUTE_NORETURN __declspec(noreturn)
    #endif
#endif
#define no_return ATTRIBUTE_NORETURN

#undef ATTRIBUTE_NOINLINE
#undef no_inline
#ifdef __has_cpp_attribute
    #ifdef noinline
        #error "noinline shouldn't be defined"
    #endif
    #if __has_cpp_attribute(noinline)
        #define ATTRIBUTE_NOINLINE [[noinline]]
    #endif
#endif
#ifndef ATTRIBUTE_NOINLINE
    #if SUPDEF_COMPILER == 1
        #define ATTRIBUTE_NOINLINE [[__gnu__::__noinline__]]
    #elif SUPDEF_COMPILER == 2 // Clang
        #define ATTRIBUTE_NOINLINE [[clang::noinline]]
    #else
        #define ATTRIBUTE_NOINLINE __declspec(noinline)
    #endif
#endif
#define no_inline ATTRIBUTE_NOINLINE

#undef ATTRIBUTE_ALWAYS_INLINE
#undef always_inline
#ifdef __has_cpp_attribute
    #ifdef always_inline
        #error "always_inline shouldn't be defined"
    #endif
    #if __has_cpp_attribute(always_inline) && 0
        #define ATTRIBUTE_ALWAYS_INLINE [[always_inline]]
    #endif
#endif
#ifndef ATTRIBUTE_ALWAYS_INLINE
    #if SUPDEF_COMPILER == 1
        #define ATTRIBUTE_ALWAYS_INLINE [[__gnu__::__always_inline__]]
    #elif SUPDEF_COMPILER == 2 // Clang
        #define ATTRIBUTE_ALWAYS_INLINE [[clang::always_inline]]
    #else
        #define ATTRIBUTE_ALWAYS_INLINE __forceinline
    #endif
#endif
#define always_inline ATTRIBUTE_ALWAYS_INLINE

#undef ATTRIBUTE_CONST
#undef ATTRIBUTE_PURE
#undef no_side_effects
#if SUPDEF_COMPILER == 1
    #define ATTRIBUTE_CONST [[__gnu__::__const__]]
    #define ATTRIBUTE_PURE  [[__gnu__::__pure__]]
#elif SUPDEF_COMPILER == 2 // Clang
    #define ATTRIBUTE_CONST [[clang::const]]
    #define ATTRIBUTE_PURE  [[clang::pure]]
#else
    #define ATTRIBUTE_CONST
    #define ATTRIBUTE_PURE
#endif
#define no_side_effects(lvl) PP_SWITCH(lvl, (CHAOS_PP_EMPTY()), (0, (CHAOS_PP_EMPTY())), (1, (ATTRIBUTE_CONST)), (2, (ATTRIBUTE_PURE)))

#undef ENUM_CLASS_OPERATORS
#define ENUM_CLASS_OPERATORS(enum_name)                             \
    inline enum_name operator|(enum_name lhs, enum_name rhs)        \
    {                                                               \
        return static_cast<enum_name>(                              \
            std::to_underlying(lhs) |                               \
            std::to_underlying(rhs)                                 \
        );                                                          \
    }                                                               \
    inline enum_name operator&(enum_name lhs, enum_name rhs)        \
    {                                                               \
        return static_cast<enum_name>(                              \
            std::to_underlying(lhs) &                               \
            std::to_underlying(rhs)                                 \
        );                                                          \
    }                                                               \
    inline enum_name operator^(enum_name lhs, enum_name rhs)        \
    {                                                               \
        return static_cast<enum_name>(                              \
            std::to_underlying(lhs) ^                               \
            std::to_underlying(rhs)                                 \
        );                                                          \
    }                                                               \
    inline enum_name operator~(enum_name rhs)                       \
    {                                                               \
        return static_cast<enum_name>(                              \
            ~std::to_underlying(rhs)                                \
        );                                                          \
    }                                                               \
    inline enum_name& operator|=(enum_name& lhs, enum_name rhs)     \
    {                                                               \
        lhs = lhs | rhs;                                            \
        return lhs;                                                 \
    }                                                               \
    inline enum_name& operator&=(enum_name& lhs, enum_name rhs)     \
    {                                                               \
        lhs = lhs & rhs;                                            \
        return lhs;                                                 \
    }                                                               \
    inline enum_name& operator^=(enum_name& lhs, enum_name rhs)     \
    {                                                               \
        lhs = lhs ^ rhs;                                            \
        return lhs;                                                 \
    }

    

#if 0
#include <boost/parameter.hpp>

namespace BoostParameterConfig
{
    namespace parameter = boost::parameter;
    BOOST_PARAMETER_NAME(rm_nl);
    BOOST_PARAMETER_NAME(only_trailing_and_leading);
}

using namespace BoostParameterConfig;
#endif

static_assert(COMPTIME_ABS(-5) == 5);
static_assert(COMPTIME_ABS(5) == 5);

static_assert(COMPTIME_FLOAT_EQ(double, COMPTIME_SCALE(double, 50,   (0, 100), (0, 1),        CLASSIC),    0.5, 0.0001));
static_assert(COMPTIME_FLOAT_EQ(float,  COMPTIME_SCALE(float,  0.25, (0, 1),   (-100, 0),     CLASSIC), -75.0f, 0.0001));
static_assert(COMPTIME_FLOAT_EQ(double, COMPTIME_SCALE(double, 0.5,  (0, 1),   (-2000, 2000), CLASSIC),    0.0, 0.0001));

/* PP_FLOAT_DEMOTE(PP_FLOAT_PROMOTE(1, 2), float); */
/* PP_FLOAT_DEMOTE(PP_FLOAT_PROMOTE((-)1, 2), float); */
/* PP_FLOAT_DEMOTE(PP_FLOAT_PROMOTE((-)1, 2), long double); */

/* CHAOS_PP_ARBITRARY_DEMOTE(ARBITRARY_DIGIT_COUNT((1)(0)(0)(0)(0)(0)(0)(0)(0))) */
/* CHAOS_PP_ARBITRARY_DEMOTE((1)(0)(0)(0)(0)(0)(0)(0)(0)) */
/* CHAOS_PP_ARBITRARY_DEMOTE((0)(0)(0)(0)(0)(0)(0)(0)(0)) */
/* CHAOS_PP_ARBITRARY_PROMOTE(100) // (1)(0)(0) */
/* CHAOS_PP_ARBITRARY_PROMOTE((-)100) // ((1)(0)(0)) */
/* PP_FLOAT_DEMOTE_INT_PART(PP_PAIR((1)(0)(2), unused)) // 1 */
/* PP_FLOAT_DEMOTE_INT_PART(PP_PAIR(((1)(0)(2)), unused)) // -1 */

/* PP_FLOAT_DEMOTE_DEC_PART(PP_PAIR((1)(0)(2), unused)) // 02 */
/* PP_FLOAT_DEMOTE_DEC_PART(PP_PAIR(((1)(0)(2)), unused)) // 02 */

/* PP_FLOAT_DEMOTE_EXP_PART(PP_PAIR((1)(0)(2), (1))) // 3 */
/* PP_FLOAT_DEMOTE_EXP_PART(PP_PAIR((1)(0)(2), ((1)))) // 1 */

/* ARBITRARY_ABS((1)(0)(0)(0)(0)(0)(0)(0)(0)) // (1)(0)(0)(0)(0)(0)(0)(0)(0) */
/* ARBITRARY_ABS(((1)(0)(0)(0)(0)(0)(0)(0)(0))) // (1)(0)(0)(0)(0)(0)(0)(0)(0) */

/* PP_FLOAT_DEMOTE(PP_FLOAT((1)(0)(2), (1))) // 1.02e3F128 */
/* PP_FLOAT_DEMOTE(PP_FLOAT((1)(0)(2), ((1)))) // 1.02e1F128 */

/* ARBITRARY_EXP(CHAOS_PP_ARBITRARY_PROMOTE(10), CHAOS_PP_ARBITRARY_PROMOTE(0))  // (1) */
/* ARBITRARY_EXP(CHAOS_PP_ARBITRARY_PROMOTE(10), CHAOS_PP_ARBITRARY_PROMOTE(2))  // (1) (0) (0) */
/* ARBITRARY_EXP(CHAOS_PP_ARBITRARY_PROMOTE(10), CHAOS_PP_ARBITRARY_PROMOTE(10)) // (1) (0) (0) (0) (0) (0) (0) (0) (0) (0) (0) */

/* ARBITRARY_EXP(CHAOS_PP_ARBITRARY_PROMOTE((-)2), CHAOS_PP_ARBITRARY_PROMOTE(0)) // (1) */
/* ARBITRARY_EXP(CHAOS_PP_ARBITRARY_PROMOTE((-)2), CHAOS_PP_ARBITRARY_PROMOTE(1)) // ((2)) */
/* ARBITRARY_EXP(CHAOS_PP_ARBITRARY_PROMOTE((-)2), CHAOS_PP_ARBITRARY_PROMOTE(2)) // (4) */
/* ARBITRARY_EXP(CHAOS_PP_ARBITRARY_PROMOTE((-)2), CHAOS_PP_ARBITRARY_PROMOTE(3)) // ((8)) */
/* ARBITRARY_EXP(CHAOS_PP_ARBITRARY_PROMOTE((-)2), CHAOS_PP_ARBITRARY_PROMOTE(4)) // (1) (6) */
/* ARBITRARY_EXP(CHAOS_PP_ARBITRARY_PROMOTE((-)2), CHAOS_PP_ARBITRARY_PROMOTE(5)) // ((3) (2)) */

/* PP_FLOAT_PROMOTE(1, 2); // represents 1.2 */
/* PP_FLOAT_PROMOTE((-)1, 2); // represents -1.2 */
/* PP_FLOAT_PROMOTE((-)1, 002); // represents -1.002 (doesn't work) */
/* PP_FLOAT_PROMOTE(2, 0, (-)3); // 0.002 */


/* PP_FLOAT_DEMOTE(PP_FLOAT_PROMOTE(1, 2)) */
/* PP_FLOAT_DEMOTE(PP_FLOAT_PROMOTE((-)1, 2)) */
/* PP_FLOAT_DEMOTE(PP_FLOAT_PROMOTE(2, 0, (-)3)) */

/* PP_IF(1)(blah, blah, blah)(bla, bla, bla) */
/* PP_IF(0)(blah, blah, blah)(bla, bla, bla) */

/* PP_FLOAT_ADAPT(PP_FLOAT_PROMOTE(0, 299), PP_FLOAT_PROMOTE(0, 299)) */
/* PP_FLOAT_ADAPT(PP_FLOAT_PROMOTE(0, 299), PP_FLOAT_PROMOTE(100, 2)) */
/* PP_FLOAT_PROMOTE(0, 299), PP_FLOAT_PROMOTE(100, 2) */
/* PP_FLOAT_DEMOTE(PP_FLOAT_PROMOTE(0, 299)), PP_FLOAT_DEMOTE(PP_FLOAT_PROMOTE(100, 2)) */

/* PP_FLOAT_DEMOTE(PP_FLOAT_ADD(PP_FLOAT_PROMOTE(0, 299), PP_FLOAT_PROMOTE(100, 2))) // 1.00499e2F128 */
/* PP_FLOAT_DEMOTE(PP_FLOAT_ADD(PP_FLOAT_PROMOTE(0, 299), PP_FLOAT_PROMOTE((-)100, 2))) // -9.9901e1F128 */

/* PP_FLOAT_PROMOTE(0, 299) */
/* CHAOS_PP_ARBITRARY_DEMOTE(PP_FLOAT_DECIMAL_DIGIT_COUNT(PP_FLOAT_PROMOTE(0, 299))) // 3 */

/* PP_FLOAT_PROMOTE(100, 2) */
/* CHAOS_PP_ARBITRARY_DEMOTE(PP_FLOAT_DECIMAL_DIGIT_COUNT(PP_FLOAT_PROMOTE(100, 2))) // 1 */

/* PP_FLOAT_PROMOTE(100, 0) */
/* CHAOS_PP_ARBITRARY_DEMOTE(PP_FLOAT_DECIMAL_DIGIT_COUNT(PP_FLOAT_PROMOTE(100, 0))) // 0 */

/* PP_FLOAT_PROMOTE(100,) */
/* CHAOS_PP_ARBITRARY_DEMOTE(PP_FLOAT_DECIMAL_DIGIT_COUNT(PP_FLOAT_PROMOTE(100, )))  // 0 */

/* PP_FLOAT_DEMOTE(PP_FLOAT_ABS(PP_FLOAT_PROMOTE(100, 0))) */
/* PP_FLOAT_DEMOTE(PP_FLOAT_ABS(PP_FLOAT_PROMOTE((-)100, 0))) */

/* PP_FLOAT_DEMOTE(PP_FLOAT_NEG(PP_FLOAT_PROMOTE(100, 0))) */
/* PP_FLOAT_DEMOTE(PP_FLOAT_NEG(PP_FLOAT_PROMOTE((-)100, 0))) */

/* PP_FLOAT_COMPUTE_DIV_STEP(
    unused,
    PP_FLOAT_PROMOTE(0, 0),
    PP_FLOAT_PROMOTE(1, 0),
    PP_FLOAT_PROMOTE(0, 5),
    CHAOS_PP_BOOL(0),
    CHAOS_PP_ARBITRARY_PROMOTE(0),
    CHAOS_PP_ARBITRARY_PROMOTE(0),
    CHAOS_PP_ARBITRARY_PROMOTE(5)
) */
/* PP_FLOAT_INV_IMPL_WHILE_BODY_COND(
    unused,
    PP_FLOAT_PROMOTE(0, 0),
    PP_FLOAT_PROMOTE(1, 0),
    PP_FLOAT_PROMOTE(0, 5),
    CHAOS_PP_BOOL(0),
    CHAOS_PP_ARBITRARY_PROMOTE(0),
    CHAOS_PP_ARBITRARY_PROMOTE(0),
    CHAOS_PP_ARBITRARY_PROMOTE(5)
) */

/* PP_FLOAT_COMPUTE_DIV_STEP(
    unused,
    (CHAOS_PP_TUPLE)((0), (0)), (CHAOS_PP_TUPLE)((1), (0)), (CHAOS_PP_TUPLE)((5), ((1))), 0, (1) (0), (1),
    unused
) */

/* PP_FLOAT_DEMOTE(PP_FLOAT_INV(PP_FLOAT_PROMOTE(0, 5))) // 2.e0F128 */
/* PP_FLOAT_DEMOTE(PP_FLOAT_INV(PP_FLOAT_PROMOTE(0, 299))) // 3.34448e0F128 */
/* PP_FLOAT_DEMOTE(PP_FLOAT_INV(PP_FLOAT_PROMOTE(2, 5))) // 4.e-1F128 */
/* PP_FLOAT_DEMOTE(PP_FLOAT_INV(PP_FLOAT_PROMOTE(2, 299))) // 4.3497e-1F128 */

/* COMPTIME_SCALE(double, 50,   (0, 100), (0, 1), CHAOS) */

/* probably_if(blah, 23) */
/* probably_if(blah, 23, CLASSIC) */
/* probably_if(blah, 23, CHAOS) */

#ifndef STATIC_MEMPOOL_SIZE
    #define STATIC_MEMPOOL_SIZE ((1024 * 1024) * 10) // 1MB * 10 = 10MB
#endif
static_assert(
    std::integral_constant<
        size_t,
        STATIC_MEMPOOL_SIZE
    >::value
    ==
    STATIC_MEMPOOL_SIZE
); // Just to make sure the value is a correct compile-time constant

/* #include <iconv.h> */

STATIC_TODO(
    "Add tests for the following macros: all PP_FLOAT* macros, all macros relying on them (COMPTIME_SCALE for example), probably_if and friends, etc..."
);

#ifdef __cplusplus

#include <decimal/decimal>

namespace stdfs = std::filesystem;
namespace stdexec = std::execution;
namespace stdpmr = std::pmr;
namespace stddec = std::decimal;
namespace stdx = std::experimental;

namespace stdabi = abi;         // Not standard at all but let's pretend it is
namespace stdgnu = __gnu_cxx;   // Not standard at all but let's pretend it is
namespace stdpbds = __gnu_pbds; // Not standard at all but let's pretend it is

namespace stdtr1 = std::tr1;
namespace stdtr2 = std::tr2;

#endif

#undef UNIQUE_ID
#define UNIQUE_ID(name) PP_CAT(name, __LINE__)

#undef TRY_SYSCALL_WHILE_EINTR
#undef TRY_SYSCALL_WHILE_EINTR_WITH_THIS

#ifdef __cplusplus
    #define TRY_SYSCALL_WHILE_EINTR(func, tupled_args)      \
        [&]() -> decltype(auto)                             \
        {                                                   \
            decltype(                                       \
                func(EXPAND_ONE_TUPLE(tupled_args))         \
            ) UNIQUE_ID(retval);                            \
            do                                              \
            {                                               \
                UNIQUE_ID(retval) = std::invoke(            \
                    &(func),                                \
                    EXPAND_ONE_TUPLE(tupled_args)           \
                );                                          \
            } while (                                       \
                UNIQUE_ID(retval) == -1 && errno == EINTR   \
            );                                              \
            return UNIQUE_ID(retval);                       \
        }()
    #define TRY_SYSCALL_WHILE_EINTR_WITH_THIS(func, tupled_args)    \
        [&, this]() -> decltype(auto)                               \
        {                                                           \
            decltype(                                               \
                func(EXPAND_ONE_TUPLE(tupled_args))                 \
            ) UNIQUE_ID(retval);                                    \
            do                                                      \
            {                                                       \
                UNIQUE_ID(retval) = std::invoke(                    \
                    &(func),                                        \
                    EXPAND_ONE_TUPLE(tupled_args)                   \
                );                                                  \
            } while (                                               \
                UNIQUE_ID(retval) == -1 && errno == EINTR           \
            );                                                      \
            return UNIQUE_ID(retval);                               \
        }()
#else
    #define TRY_SYSCALL_WHILE_EINTR(func, tupled_args)      \
        __extension__ ({                                    \
            __typeof__(func(EXPAND_ONE_TUPLE(tupled_args))) \
                UNIQUE_ID(retval);                          \
            do                                              \
            {                                               \
                UNIQUE_ID(retval) = func(                   \
                    EXPAND_ONE_TUPLE(tupled_args)           \
                );                                          \
            } while (                                       \
                UNIQUE_ID(retval) == -1 && errno == EINTR   \
            );                                              \
            UNIQUE_ID(retval);                              \
        })
    #define TRY_SYSCALL_WHILE_EINTR_WITH_THIS(func, tupled_args)    \
        TRY_SYSCALL_WHILE_EINTR(func, tupled_args)
#endif