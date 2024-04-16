/* 
 * MIT License
 * 
 * Copyright (c) 2023-2024 Axel PASCON
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

#ifndef TYPES_HPP
#define TYPES_HPP

#include <sup_def/common/util/util.hpp>

namespace SupDef
{
    void set_app_locale(void);
}

namespace uni
{
    using ::SupDef::InternalError;

    typedef unsigned char byte;

    namespace detail
    {
        template <typename T>
        using string_default_allocator = std::allocator<T>;

        template <
            typename CharT,
            typename T,
            typename RealCharT = std::remove_cvref_t<CharT>,
            typename RealT = std::remove_cvref_t<T>
        >
        struct similar_types
        : public std::bool_constant<
            std::same_as<RealCharT, RealT> ||
            (
                ( sizeof(RealCharT) == sizeof(RealT) ) &&
                ( std::unsigned_integral<RealCharT> == std::unsigned_integral<RealT> ) &&
                ( alignof(RealCharT) == alignof(RealT) )
            )
        > { };
    }

    template <typename CharT>
    struct char_traits;

    template <
        typename CharT,
        typename Traits = ::uni::char_traits<CharT>,
        typename Allocator = detail::string_default_allocator<CharT>
    >
    class string;

    enum class endianness : int_fast8_t
    {
        little = -1,
        big = 1,

        undefined = 0
    };

    typedef char8_t utf8_char;
    static_assert(
        ::uni::detail::similar_types<
            char8_t,
            uint_least8_t
        >::value &&
        std::unsigned_integral<char8_t> &&
        sizeof(char8_t) == 1 &&
        std::is_trivial<utf8_char>::value &&
        std::is_standard_layout<utf8_char>::value &&
        utf8_char{} == 0
    );
    
    struct utf16_char
    {
        char16_t value;
        endianness endian;

#if 0
        constexpr utf16_char() noexcept
            : value(0), endian(endianness::undefined)
        { }
#else
        constexpr utf16_char() = default;
#endif
        constexpr utf16_char(char16_t value) noexcept
            : value(value), endian(endianness::undefined)
        { }
        constexpr utf16_char(char16_t value, endianness endian) noexcept
            : value(value), endian(endian)
        { }
    };
    static_assert(
        ::uni::detail::similar_types<
            char16_t,
            uint_least16_t
        >::value &&
        std::unsigned_integral<char16_t> &&
        sizeof(char16_t) == 2 &&
        std::is_trivial<utf16_char>::value &&
        std::is_standard_layout<utf16_char>::value &&
        utf16_char{}.endian == endianness::undefined &&
        utf16_char{}.value == 0
    );

    struct utf32_char
    {
        char32_t value;
        endianness endian;

#if 0
        constexpr utf32_char() noexcept
            : value(0), endian(endianness::undefined)
        { }
#else
        constexpr utf32_char() = default;
#endif
        constexpr utf32_char(char32_t value) noexcept
            : value(value), endian(endianness::undefined)
        { }
        constexpr utf32_char(char32_t value, endianness endian) noexcept
            : value(value), endian(endian)
        { }
    };
    static_assert(
        ::uni::detail::similar_types<
            char32_t,
            uint_least32_t
        >::value &&
        std::unsigned_integral<char32_t> &&
        sizeof(char32_t) == 4 &&
        std::is_trivial<utf32_char>::value &&
        std::is_standard_layout<utf32_char>::value &&
        utf32_char{}.endian == endianness::undefined &&
        utf32_char{}.value == 0
    );

    template <
        typename CharT,
        typename RealCharT = std::remove_cvref_t<CharT>
    >
    concept uni_char = std::same_as<RealCharT, utf8_char> ||
        std::same_as<RealCharT, utf16_char>               ||
        std::same_as<RealCharT, utf32_char>;

    typedef ::uni::string<char, ::uni::char_traits<char>> char_string;
    typedef ::uni::string<wchar_t, ::uni::char_traits<wchar_t>> wchar_string;

    typedef ::uni::string<utf8_char, ::uni::char_traits<utf8_char>> utf8_string;
    typedef ::uni::string<utf16_char, ::uni::char_traits<utf16_char>> utf16_string;
    typedef ::uni::string<utf32_char, ::uni::char_traits<utf32_char>> utf32_string;

    template <typename Allocator = detail::string_default_allocator<char>>
    using char_string_ex = ::uni::string<char, ::uni::char_traits<char>, Allocator>;
    template <typename Allocator = detail::string_default_allocator<wchar_t>>
    using wchar_string_ex = ::uni::string<wchar_t, ::uni::char_traits<wchar_t>, Allocator>;

    template <typename Allocator = detail::string_default_allocator<utf8_char>>
    using utf8_string_ex = ::uni::string<utf8_char, ::uni::char_traits<utf8_char>, Allocator>;
    template <typename Allocator = detail::string_default_allocator<utf16_char>>
    using utf16_string_ex = ::uni::string<utf16_char, ::uni::char_traits<utf16_char>, Allocator>;
    template <typename Allocator = detail::string_default_allocator<utf32_char>>
    using utf32_string_ex = ::uni::string<utf32_char, ::uni::char_traits<utf32_char>, Allocator>;

    namespace detail
    {
        template <typename...>
        struct uni_string_traits_helper;

        template <
            typename CharT,
            typename Traits,
            typename Allocator
        >
        struct uni_string_traits_helper<
            ::uni::string<CharT, Traits, Allocator>
        >
        {
            typedef ::uni::string<CharT, Traits, Allocator> string_type;

            typedef CharT char_type;
            typedef typename string_type::value_type real_char_type;
            
            typedef Traits traits_type;
            typedef typename string_type::traits_type real_traits_type;
            
            typedef Allocator allocator_type;
            typedef typename string_type::allocator_type real_allocator_type;
        };

        // RealStrT is a utfX_string, but Allocator differs
        template <
            typename StrT,
            typename RealStrT = std::remove_cvref_t<StrT>
        >
        struct uni_string_allocdiff
        : public std::bool_constant<
            ::uni::uni_char<
                typename uni_string_traits_helper<RealStrT>::char_type
            > &&
            std::same_as<
                typename uni_string_traits_helper<RealStrT>::traits_type,
                ::uni::char_traits<typename uni_string_traits_helper<RealStrT>::char_type>
            > &&
            !std::same_as<
                typename uni_string_traits_helper<RealStrT>::real_allocator_type,
                typename uni_string_traits_helper<
                    ::uni::string<
                        typename uni_string_traits_helper<RealStrT>::char_type,
                        typename uni_string_traits_helper<RealStrT>::traits_type
                    >
                >::real_allocator_type
            >
        > { };
    }

    template <
        typename StrT,
        typename RealStrT = std::remove_cvref_t<StrT>
    >
    concept uni_string = std::same_as<RealStrT, utf8_string>    ||
        std::same_as<RealStrT, utf16_string>                    ||
        std::same_as<RealStrT, utf32_string>                    ||
        detail::uni_string_allocdiff<RealStrT>::value;

    template <
        typename StrT,
        typename RealStrT = std::remove_cvref_t<StrT>
    >
    concept supported_uni_string = uni_string<RealStrT> ||
        (
            SPECIALIZATION_OF( RealStrT, ::uni::string ) &&
            ::SupDef::IsOneOf<
                typename uni::detail::uni_string_traits_helper<RealStrT>::char_type,
                char, wchar_t
            > &&
            ::SupDef::IsOneOf<
                typename uni::detail::uni_string_traits_helper<RealStrT>::traits_type,
                ::uni::char_traits<
                    typename uni::detail::uni_string_traits_helper<RealStrT>::char_type
                >
            >
        );

    template <
        typename CharT,
        typename RealCharT = std::remove_cvref_t<CharT>
    >
    concept supported_uni_char = uni_char<RealCharT> ||
        std::same_as<RealCharT, char>                ||
        std::same_as<RealCharT, wchar_t>;
          

    namespace detail
    {
        template <
            typename CharT,
            typename Traits = ::uni::char_traits<CharT>
        >
        concept has_endianness = requires(const CharT& c1, CharT& c2, endianness e)
        {
            { Traits::get_endianness(c1) } -> std::same_as<endianness>;
            { Traits::set_endianness(c2, e) } -> std::same_as<void>;
        };

        template <
            ::uni::supported_uni_char CharTFrom,
            ::uni::supported_uni_char CharTTo
        >
        struct string_conversions;
    }
}

#define INCLUDED_FROM_UNISTREAMS_SOURCE 1

#include <sup_def/common/unistreams/impl/string_conversions.ipp>

#undef INCLUDED_FROM_UNISTREAMS_SOURCE

#endif
