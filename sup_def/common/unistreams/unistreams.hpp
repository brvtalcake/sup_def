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

#ifndef UNISTREAMS_HPP
#define UNISTREAMS_HPP

#include <sup_def/common/util/util.hpp>

namespace uni
{
    namespace detail
    {
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

    typedef unsigned char byte;
    
    enum class endianness : int_fast8_t
    {
        little = -1,
        big = 1,
        undefined = 0
    };

    namespace detail
    {
        template <typename CharT>
            requires std::same_as<CharT, char16_t> || std::same_as<CharT, char32_t>
        consteval endianness get_literal_endianness()
        {
            if constexpr (std::same_as<CharT, char16_t>)
            {
                constexpr char16_t value[] = u"😀";
                static_assert(
                    ( sizeof(value) / sizeof(char16_t) ) - 1 == 2
                );
                static_assert(
                    sizeof(char16_t) * 2 == 4
                );
                uint8_t buf[4] = {
                    [ 0 ] = static_cast<uint8_t>(( value[0] >> 8 ) & 0xFF),
                    [ 1 ] = static_cast<uint8_t>(value[0] & 0xFF),
                    [ 2 ] = static_cast<uint8_t>(( value[1] >> 8 ) & 0xFF),
                    [ 3 ] = static_cast<uint8_t>(value[1] & 0xFF)
                };                    
                if (
                    buf[0] == 0xD8 &&
                    buf[1] == 0x3D &&
                    buf[2] == 0xDE &&
                    buf[3] == 0x00
                )
                    return endianness::big;
                else if (
                    buf[0] == 0x3D &&
                    buf[1] == 0xD8 &&
                    buf[2] == 0x00 &&
                    buf[3] == 0xDE
                )
                    return endianness::little;
                else
                    return endianness::undefined;
            }
            else
            {
                constexpr char32_t value[] = U"😀";
                static_assert(
                    ( sizeof(value) / sizeof(char32_t) ) - 1 == 1
                );
                static_assert(
                    sizeof(char32_t) == 4
                );
                uint8_t buf[4] = {
                    [ 0 ] = static_cast<uint8_t>(( value[0] >> 24 ) & 0xFF),
                    [ 1 ] = static_cast<uint8_t>(( value[0] >> 16 ) & 0xFF),
                    [ 2 ] = static_cast<uint8_t>(( value[0] >> 8 ) & 0xFF),
                    [ 3 ] = static_cast<uint8_t>(value[0] & 0xFF)
                };
                if (
                    buf[0] == 0x00 &&
                    buf[1] == 0x01 &&
                    buf[2] == 0xF6 &&
                    buf[3] == 0x00
                )
                    return endianness::big;
                else if (
                    buf[0] == 0x00 &&
                    buf[1] == 0xF6 &&
                    buf[2] == 0x01 &&
                    buf[3] == 0x00
                )
                    return endianness::little;
                else
                    return endianness::undefined;
            }
        }
        static_assert(
            std::integral_constant<
                endianness,
                get_literal_endianness<char16_t>()
            >::value == endianness::big
        );
        static_assert(
            std::integral_constant<
                endianness,
                get_literal_endianness<char32_t>()
            >::value == endianness::big
        );
    }
    
    typedef char8_t utf8_char;
    static_assert(
        ::uni::detail::similar_types<
            char8_t,
            uint_least8_t
        >::value &&
        std::unsigned_integral<char8_t>
    );
    
    struct utf16_char
    {
        char16_t value;
        endianness endian;

        utf16_char() noexcept
            : value(0), endian(endianness::undefined)
        { }
        utf16_char(char16_t value) noexcept
            : value(value), endian(endianness::undefined)
        { }
    };
    static_assert(
        ::uni::detail::similar_types<
            char16_t,
            uint_least16_t
        >::value &&
        std::unsigned_integral<char16_t>
    );

    struct utf32_char
    {
        char32_t value;
        endianness endian;

        utf32_char() noexcept
            : value(0), endian(endianness::undefined)
        { }
        utf32_char(char32_t value) noexcept
            : value(value), endian(endianness::undefined)
        { }
    };
    static_assert(
        ::uni::detail::similar_types<
            char32_t,
            uint_least32_t
        >::value &&
        std::unsigned_integral<char32_t>
    );

    template <
        typename CharT,
        typename RealCharT = std::remove_cvref_t<CharT>
    >
    concept is_char = std::same_as<RealCharT, utf8_char> ||
        std::same_as<RealCharT, utf16_char>              ||
        std::same_as<RealCharT, utf32_char>;

    template <typename CharT>
        requires ::uni::is_char<CharT>
    class input_file
    {
        public:
            input_file() = default;
            input_file(const std::filesystem::path& path);

            bool open(const std::filesystem::path& path);
            void close();

            bool is_open() const;
        private:
            std::filebuf file;
    };
}

#define INCLUDED_FROM_UNISTREAMS_SOURCE 1
#include <sup_def/common/unistreams/impl/char_traits.ipp>
#undef INCLUDED_FROM_UNISTREAMS_SOURCE

using ::uni::utf8_char;
using ::uni::utf16_char;
using ::uni::utf32_char;

#endif
