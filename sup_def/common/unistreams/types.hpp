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

    namespace detail
    {
        typedef uint_fast32_t unicode_code_point;
        typedef int_fast64_t  unicode_code_point_signed;

        template <auto... B>
            requires std::conjunction<
                std::bool_constant<
                    std::convertible_to<decltype(B), bool>
                >...
            >::value
        using sfinae_require_all_of = typename std::enable_if<
            std::conjunction<
                std::bool_constant<static_cast<bool>(B)>...
            >::value
        >::type;

        template <typename... BC>
        using sfinae_require_all_of_types = typename std::enable_if<
            std::conjunction<BC...>::value
        >::type;

        template <auto... B>
            requires std::disjunction<
                std::bool_constant<
                    std::convertible_to<decltype(B), bool>
                >...
            >::value
        using sfinae_require_any_of = typename std::enable_if<
            std::disjunction<
                std::bool_constant<static_cast<bool>(B)>...
            >::value
        >::type;

        template <typename... BC>
        using sfinae_require_any_of_types = typename std::enable_if<
            std::disjunction<BC...>::value
        >::type;

    }
}

#undef INCLUDED_FROM_UNISTREAMS_SOURCE
#define INCLUDED_FROM_UNISTREAMS_SOURCE 1

#include <sup_def/common/unistreams/impl/detail_optional.ipp>
#include <sup_def/common/unistreams/impl/detail_tuple.ipp>

#undef INCLUDED_FROM_UNISTREAMS_SOURCE


namespace uni
{
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
}

namespace uni
{
    template <typename CharT>
    struct char_traits;

    template <
        typename CharT,
        typename Traits = ::uni::char_traits<CharT>,
        typename Allocator = detail::string_default_allocator<CharT>
    >
    class string;

    enum class endianness : int8_t
    {
        undefined = 0,
        
        little = -1,
        big = 1
    };
    static_assert(
        sizeof(::uni::endianness) == 1, "Some code below may rely on `endianness` being 1 byte long"
    );
    static_assert(
        alignof(::uni::endianness) == 1, "Some code below may rely on `endianness` being 1 byte aligned"
    );

    using byte = uint8_t;

    struct utf8_char;
    struct utf16_char;
    struct utf32_char;

    template <
        typename CharT,
        typename RealCharT = std::remove_cvref_t<CharT>
    >
    concept uni_char = std::same_as<RealCharT, utf8_char> ||
        std::same_as<RealCharT, utf16_char>               ||
        std::same_as<RealCharT, utf32_char>;
}

#undef INCLUDED_FROM_UNISTREAMS_SOURCE
#define INCLUDED_FROM_UNISTREAMS_SOURCE 1

#include <sup_def/common/unistreams/impl/utf_chars.ipp>

#undef INCLUDED_FROM_UNISTREAMS_SOURCE

namespace uni
{
    typedef ::uni::string<char, ::uni::char_traits<char>> char_string;
    typedef ::uni::string<wchar_t, ::uni::char_traits<wchar_t>> wchar_string;
    typedef ::uni::string<char8_t, ::uni::char_traits<char8_t>> char8_string;
    typedef ::uni::string<char16_t, ::uni::char_traits<char16_t>> char16_string;
    typedef ::uni::string<char32_t, ::uni::char_traits<char32_t>> char32_string;

    typedef ::uni::string<utf8_char, ::uni::char_traits<utf8_char>> utf8_string;
    typedef ::uni::string<utf16_char, ::uni::char_traits<utf16_char>> utf16_string;
    typedef ::uni::string<utf32_char, ::uni::char_traits<utf32_char>> utf32_string;

    template <typename Allocator = detail::string_default_allocator<char>>
    using char_string_ex = ::uni::string<char, ::uni::char_traits<char>, Allocator>;
    template <typename Allocator = detail::string_default_allocator<wchar_t>>
    using wchar_string_ex = ::uni::string<wchar_t, ::uni::char_traits<wchar_t>, Allocator>;
    template <typename Allocator = detail::string_default_allocator<char8_t>>
    using char8_string_ex = ::uni::string<char8_t, ::uni::char_traits<char8_t>, Allocator>;
    template <typename Allocator = detail::string_default_allocator<char16_t>>
    using char16_string_ex = ::uni::string<char16_t, ::uni::char_traits<char16_t>, Allocator>;
    template <typename Allocator = detail::string_default_allocator<char32_t>>
    using char32_string_ex = ::uni::string<char32_t, ::uni::char_traits<char32_t>, Allocator>;

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
        typename CharT,
        typename RealCharT = std::remove_cvref_t<CharT>
    >
    concept supported_uni_char = uni_char<RealCharT> ||
        std::same_as<RealCharT, char>                ||
        std::same_as<RealCharT, wchar_t>             ||
        std::same_as<RealCharT, char8_t>             ||
        std::same_as<RealCharT, char16_t>            ||
        std::same_as<RealCharT, char32_t>;

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
            ::uni::supported_uni_char<
                typename uni::detail::uni_string_traits_helper<RealStrT>::char_type
            > &&
            std::same_as<
                typename uni::detail::uni_string_traits_helper<RealStrT>::traits_type,
                ::uni::char_traits<
                    typename uni::detail::uni_string_traits_helper<RealStrT>::char_type
                >
            >
        );
          

    namespace detail
    {
        template <
            typename CharT,
            typename Traits = ::uni::char_traits<CharT>
        >
        concept has_endianness = requires(
            const CharT& c1, CharT& c2,
            const Traits::int_type& i1, Traits::int_type& i2,
            endianness e
        )
        {
            { c1.get_endianness() }           -> std::same_as<endianness>;
            { c2.get_endianness() }           -> std::same_as<endianness>;
            { c2.set_endianness(e) }          -> std::same_as<void>;

            { Traits::get_endianness(c1) }    -> std::same_as<endianness>;
            { Traits::get_endianness(c2) }    -> std::same_as<endianness>;
            { Traits::set_endianness(c2, e) } -> std::same_as<void>;

            { Traits::get_endianness(i1) }    -> std::same_as<endianness>;
            { Traits::get_endianness(i2) }    -> std::same_as<endianness>;
            { Traits::set_endianness(i2, e) } -> std::same_as<void>;
        };

        template <
            ::uni::supported_uni_char CharTFrom,
            ::uni::supported_uni_char CharTTo
        >
        struct string_conversions;

        template <typename Allocator, typename CharTTo>
        concept can_rebind_alloc = requires {
            typename std::allocator_traits<Allocator>::template rebind_alloc<CharTTo>;
        };
        

        template <typename...>
        struct rebind_string_impl;

        template <typename...>
        struct rebind_string_ex_impl;

        template <
            typename CharTFrom,
            typename AllocatorFrom,
            typename CharTTo
        > requires !can_rebind_alloc<AllocatorFrom, CharTTo>
        struct rebind_string_impl<
            ::uni::string<CharTFrom, ::uni::char_traits<CharTFrom>, AllocatorFrom>,
            CharTTo
        >
        {
            typedef ::uni::string<CharTTo, ::uni::char_traits<CharTTo>, AllocatorFrom> type;
        };

        template <
            typename CharTFrom,
            typename AllocatorFrom,
            typename CharTTo
        > requires can_rebind_alloc<AllocatorFrom, CharTTo>
        struct rebind_string_impl<
            ::uni::string<CharTFrom, ::uni::char_traits<CharTFrom>, AllocatorFrom>,
            CharTTo
        >
        {
            typedef ::uni::string<CharTTo, ::uni::char_traits<CharTTo>, typename std::allocator_traits<AllocatorFrom>::template rebind_alloc<CharTTo>> type;
        };

        template <
            typename CharTFrom,
            typename AllocatorFrom,
            typename CharTTo,
            typename AllocatorTo
        >
        struct rebind_string_ex_impl<
            ::uni::string<CharTFrom, ::uni::char_traits<CharTFrom>, AllocatorFrom>,
            CharTTo,
            AllocatorTo
        >
        {
            typedef ::uni::string<CharTTo, ::uni::char_traits<CharTTo>, AllocatorTo> type;
        };

        template <typename StrT, typename CharT>
        using rebind_string = typename rebind_string_impl<StrT, CharT>::type;

        // If possible, default to the same as rebind_string if Allocator is not specified
        template <
            typename StrT,
            typename CharT,
            typename Allocator = std::enable_if_t<
                can_rebind_alloc<
                    typename uni::detail::uni_string_traits_helper<StrT>::allocator_type,
                    CharT
                >,
                typename std::allocator_traits<
                    typename uni::detail::uni_string_traits_helper<StrT>::allocator_type
                >::template rebind_alloc<CharT>
            >
        >                
        using rebind_string_ex = typename rebind_string_ex_impl<StrT, CharT, Allocator>::type;
    }
}

#define INCLUDED_FROM_UNISTREAMS_SOURCE 1

#include <sup_def/common/unistreams/impl/char_traits.ipp>
#include <sup_def/common/unistreams/impl/string_conversions.ipp>

#undef INCLUDED_FROM_UNISTREAMS_SOURCE

#endif
