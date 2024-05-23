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

#if !INCLUDED_FROM_UNISTREAMS_SOURCE
    #error "This file shall not be included from outside the unistreams library"
#endif

#include <concepts>

namespace uni
{
    namespace detail
    {
        template <typename T>
        static T* memfill(T* restrict const s, const T& restrict c, size_t n) noexcept
        {
            constexpr size_t tsize  = sizeof (std::remove_cvref_t<T>);
            constexpr size_t talign = alignof(std::remove_cvref_t<T>);

            unlikely_if (n == 0)
                return s;
            
            if constexpr (std::is_trivial_v<std::remove_cvref_t<T>>)
            {
                #pragma GCC ivdep
                for (size_t i = 0; i < n; ++i)
                    std::memcpy(
                        std::assume_aligned<talign>(s + i),
                        std::assume_aligned<talign>(std::addressof(c)),
                        tsize
                    );
            }
            else
            {
                #pragma GCC ivdep
                for (size_t i = 0; i < n; ++i)
                    std::construct_at(
                        std::assume_aligned<talign>(s + i),
                        c
                    );
            }

            return s;
        }

        template <typename CharT>
        consteval CharT* char_traits_move(CharT* s1, const CharT* s2, size_t n) noexcept
        {
            using traits_type = ::uni::char_traits<CharT>;

            const auto end = s2 + n - 1;
            bool overlap = false;
            for (size_t i = 0; i < n - 1; ++i)
            {
                if (s1 + i == end)
                {
                    overlap = true;
                    break;
                }
            }
            if (overlap)
            {
                do
                {
                    --n;
                    traits_type::assign(s1[n], s2[n]);
                } while (n > 0);
            }
            else
                traits_type::copy(s1, s2, n);
            return s1;
        }

        template <typename CU>
        static constexpr byte as_byte(const CU& c, size_t i = 0) noexcept
        {
            likely_if (i < sizeof(CU))
            {
                if constexpr (sizeof(CU) == 1)
                    return reinterpret_cast<const byte&>(c);
                else
                {
                    using unsigned_t = std::make_unsigned_t<CU>;
                    const size_t shiftr = sizeof(CU) - i - 1;
                    const byte b = static_cast<const byte>(
                        (reinterpret_cast<const unsigned_t&>(c) >> (shiftr * 8)) & unsigned_t(0xFF)
                    );
                    return b;
                }
            }
            std::terminate();
        }
        static constexpr bool test_as_byte()
        {
            char8_t c8 = 'A';
            char16_t c16 = u'丰';

            return
                as_byte(c8) == byte('A') &&
                as_byte(c8, 1) == byte(0) &&
                as_byte(c16) == byte(
                    get_literal_endianness<char16_t>() == endianness::big
                    ? byte(0x4E)
                    : byte(0x30)
                ) &&
                as_byte(c16, 1) == byte(
                    get_literal_endianness<char16_t>() == endianness::big
                    ? byte(0x30)
                    : byte(0x4E)
                );
        }

        static constexpr bool is_surrogate(unicode_code_point cp) noexcept
        { return cp >= 0xD800 && cp <= 0xDFFF; }

        // Is high surrogate ?
        static constexpr bool is_high_surrogate(uint16_t c) noexcept
        { return c >= 0xD800 && c <= 0xDBFF; }
        static constexpr bool is_high_surrogate(utf16_char::code_unit c) noexcept
        { return is_high_surrogate(reinterpret_cast<uint16_t&>(c)); }

        // Is low surrogate ?
        static constexpr bool is_low_surrogate(uint16_t c) noexcept
        { return c >= 0xDC00 && c <= 0xDFFF; }
        static constexpr bool is_low_surrogate(utf16_char::code_unit c) noexcept
        { return is_low_surrogate(reinterpret_cast<uint16_t&>(c)); }

        // Find next utf-8 encoded character
        static constexpr const char* find_next_u8(const char* s, const char* const last)
        {
            const uint8_t* pbytes = reinterpret_cast<const uint8_t*>(s);
            const uint8_t* const plast = reinterpret_cast<const uint8_t* const>(last);
            do
            {
                if (pbytes >= plast)
                    return nullptr;
                ++pbytes;
            } while ((*pbytes & 0xC0) == 0x80);
            return reinterpret_cast<const char*>(pbytes);
        }

        // Find previous utf-8 encoded character
        static constexpr const char* find_prev_u8(const char* s, const char* const first)
        {
            const uint8_t* pbytes = reinterpret_cast<const uint8_t*>(s);
            const uint8_t* const pfirst = reinterpret_cast<const uint8_t* const>(first);
            do
            {
                if (pbytes <= pfirst)
                    return nullptr;
                --pbytes;
            } while ((*pbytes & 0xC0) == 0x80);
            return reinterpret_cast<const char*>(pbytes);
        }

        // Find next utf-16be encoded character (considering surrogate pairs)
        static constexpr const char* find_next_u16be(const char* s, const char* const last)
        {
            const uint8_t* pbytes = reinterpret_cast<const uint8_t*>(s);
            const uint8_t* const plast = reinterpret_cast<const uint8_t* const>(last);
            uint16_t hi;
            uint16_t lo;
            if (pbytes + 2 > plast)
                return nullptr;
            hi = (uint16_t(*pbytes) << 8) | uint16_t(*(pbytes + 1));
            if (is_high_surrogate(hi))
            {
                if (pbytes + 4 > plast)
                    return nullptr;
                lo = (uint16_t(*(pbytes + 2)) << 8) | uint16_t(*(pbytes + 3));
                if (is_low_surrogate(lo))
                    return reinterpret_cast<const char*>(pbytes + 4);
                else
                    return nullptr;
            }
            return reinterpret_cast<const char*>(pbytes + 2);
        }

        // Find previous utf-16be encoded character (considering surrogate pairs)
        static constexpr const char* find_prev_u16be(const char* s, const char* const first)
        {
            const uint8_t* pbytes = reinterpret_cast<const uint8_t*>(s);
            const uint8_t* const pfirst = reinterpret_cast<const uint8_t* const>(first);
            uint16_t hi;
            uint16_t lo;
            if (pbytes - 2 < pfirst)
                return nullptr;
            lo = (uint16_t(*(pbytes - 2)) << 8) | uint16_t(*(pbytes - 1));
            if (is_low_surrogate(lo))
            {
                if (pbytes - 4 < pfirst)
                    return nullptr;
                hi = (uint16_t(*(pbytes - 4)) << 8) | uint16_t(*(pbytes - 3));
                if (is_high_surrogate(hi))
                    return reinterpret_cast<const char*>(pbytes - 4);
                else
                    return nullptr;
            }
            return reinterpret_cast<const char*>(pbytes - 2);
        }

        // Find next utf-16le encoded character (considering surrogate pairs)
        static constexpr const char* find_next_u16le(const char* s, const char* const last)
        {
            const uint8_t* pbytes = reinterpret_cast<const uint8_t*>(s);
            const uint8_t* const plast = reinterpret_cast<const uint8_t* const>(last);
            uint16_t hi;
            uint16_t lo;
            if (pbytes + 2 > plast)
                return nullptr;
            hi = (uint16_t(*(pbytes + 1)) << 8) | uint16_t(*pbytes);
            if (is_high_surrogate(hi))
            {
                if (pbytes + 4 > plast)
                    return nullptr;
                lo = (uint16_t(*(pbytes + 3)) << 8) | uint16_t(*(pbytes + 2));
                if (is_low_surrogate(lo))
                    return reinterpret_cast<const char*>(pbytes + 4);
                else
                    return nullptr;
            }
            return reinterpret_cast<const char*>(pbytes + 2);
        }

        // Find previous utf-16le encoded character (considering surrogate pairs)
        static constexpr const char* find_prev_u16le(const char* s, const char* const first)
        {
            const uint8_t* pbytes = reinterpret_cast<const uint8_t*>(s);
            const uint8_t* const pfirst = reinterpret_cast<const uint8_t* const>(first);
            uint16_t hi;
            uint16_t lo;
            if (pbytes - 2 < pfirst)
                return nullptr;
            lo = (uint16_t(*(pbytes - 1)) << 8) | uint16_t(*(pbytes - 2));
            if (is_low_surrogate(lo))
            {
                if (pbytes - 4 < pfirst)
                    return nullptr;
                hi = (uint16_t(*(pbytes - 3)) << 8) | uint16_t(*(pbytes - 4));
                if (is_high_surrogate(hi))
                    return reinterpret_cast<const char*>(pbytes - 4);
                else
                    return nullptr;
            }
            return reinterpret_cast<const char*>(pbytes - 2);
        }

        // Find next utf-32be encoded character
        static constexpr const char* find_next_u32be(const char* s, const char* const last)
        {
            const uint8_t* pbytes = reinterpret_cast<const uint8_t*>(s);
            const uint8_t* const plast = reinterpret_cast<const uint8_t* const>(last);
            if (pbytes + 4 > plast)
                return nullptr;
            return reinterpret_cast<const char*>(pbytes + 4);
        }

        // Find previous utf-32be encoded character
        static constexpr const char* find_prev_u32be(const char* s, const char* const first)
        {
            const uint8_t* pbytes = reinterpret_cast<const uint8_t*>(s);
            const uint8_t* const pfirst = reinterpret_cast<const uint8_t* const>(first);
            if (pbytes - 4 < pfirst)
                return nullptr;
            return reinterpret_cast<const char*>(pbytes - 4);
        }

        // Find next utf-32le encoded character
        static constexpr const char* find_next_u32le(const char* s, const char* const last)
        {
            const uint8_t* pbytes = reinterpret_cast<const uint8_t*>(s);
            const uint8_t* const plast = reinterpret_cast<const uint8_t* const>(last);
            if (pbytes + 4 > plast)
                return nullptr;
            return reinterpret_cast<const char*>(pbytes + 4);
        }

        // Find previous utf-32le encoded character
        static constexpr const char* find_prev_u32le(const char* s, const char* const first)
        {
            const uint8_t* pbytes = reinterpret_cast<const uint8_t*>(s);
            const uint8_t* const pfirst = reinterpret_cast<const uint8_t* const>(first);
            if (pbytes - 4 < pfirst)
                return nullptr;
            return reinterpret_cast<const char*>(pbytes - 4);
        }

        template <typename T, typename U>
        static constexpr T* take_as(U* p) noexcept
        {
            return reinterpret_cast<T*>(p);
        }
        template <typename T, typename U>
        static constexpr const T* take_as(const U* p) noexcept
        {
            return reinterpret_cast<const T*>(p);
        }
        template <typename T, typename U>
        static constexpr T& take_as(U& r) noexcept
        {
            return reinterpret_cast<T&>(r);
        }
        template <typename T, typename U>
        static constexpr const T& take_as(const U& r) noexcept
        {
            return reinterpret_cast<const T&>(r);
        }

        template <typename T>
        static consteval T all_bits() noexcept
        {
            T result = 0;
            for (size_t i = 0; i < sizeof(T) * 8; ++i)
                result |= T(1) << i;
            return result;
        }
        template <typename T>
        static consteval T all_bits(size_t n) noexcept
        {
            T result = 0;
            for (size_t i = 0; i < std::min(n, sizeof(T) * 8); ++i)
                result |= T(1) << i;
            return result;
        }

        template <std::integral T>
        static constexpr bool odd(T n) noexcept
        {
            return bool(n % 2);
        }
        template <std::integral T>
            requires std::unsigned_integral<T>
        static constexpr bool odd(T n) noexcept
        {
            return bool(n & 1);
        }

        /* template <
            ::uni::uni_char CharT,
            std::integral InputT = typename CharT::code_unit
        > */
        static ::SupDef::Coro<std::optional<utf8_char>> parse_code_units(
            const utf8_char::code_unit* const cstr,
            const size_t n
        )
        {
            using cu_t = utf8_char::code_unit;

            std::optional<utf8_char> co_res;

            disable_strict_aliasing();
            const char* const limit = take_as<char>(cstr + n);
            for (const char* p = take_as<char>(cstr); p < limit;)
            {
                const char* const base_p = p;

                p = find_next_u8(base_p, limit);

                hard_assert(base_p <= p);

                if (p == nullptr || p == base_p || p - base_p > 4)
                    co_res = std::nullopt;
                else
                {
                    const size_t ncu = p - base_p;

                    const cu_t cu0 = *take_as<cu_t>(base_p) & all_bits<cu_t>(8UL);
                    const std::optional<cu_t> cu1 = ncu > 1
                                                  ? std::optional<cu_t>(*take_as<cu_t>(base_p + 1) & all_bits<cu_t>(8UL))
                                                  : std::nullopt;
                    const std::optional<cu_t> cu2 = ncu > 2
                                                  ? std::optional<cu_t>(*take_as<cu_t>(base_p + 2) & all_bits<cu_t>(8UL))
                                                  : std::nullopt;
                    const std::optional<cu_t> cu3 = ncu > 3
                                                  ? std::optional<cu_t>(*take_as<cu_t>(base_p + 3) & all_bits<cu_t>(8UL))
                                                  : std::nullopt;
                    co_res = utf8_char(cu0, cu1, cu2, cu3);
                }
                if (!co_res.has_value())
                    throw InternalError("uni::detail::parse_code_units: Invalid utf-8 code unit sequence");
                co_yield co_res;
            }
            reset_strict_aliasing();
            co_return std::nullopt;
        }

        static ::SupDef::Coro<std::optional<utf16_char>> parse_code_units(
            const utf16_char::code_unit* const cstr,
            const size_t n,
            endianness end = endianness::undefined
        )
        {
            using cu_t = utf16_char::code_unit;

            std::optional<utf16_char> co_res;

            if (end == endianness::undefined)
                throw InternalError("uni::detail::parse_code_units: Undefined endianness");

            disable_strict_aliasing();
            const char* const limit = take_as<char>(cstr + n);
            for (const char* p = take_as<char>(cstr); p < limit;)
            {
                const char* const base_p = p;

                p = end == endianness::big
                  ? find_next_u16be(base_p, limit)
                  : find_next_u16le(base_p, limit);

                hard_assert(base_p <= p);

                if (p == nullptr || p == base_p || p - base_p > 4 || odd(uintptr_t(p - base_p)))
                    co_res = std::nullopt;
                else
                {
                    const size_t ncu = size_t(p - base_p) / 2;

                    const cu_t cu0 = *take_as<cu_t>(base_p) & all_bits<cu_t>(16UL);
                    const std::optional<cu_t> cu1 = ncu > 1
                                                  ? std::optional<cu_t>(*take_as<cu_t>(base_p + 2) & all_bits<cu_t>(16UL))
                                                  : std::nullopt;
                    co_res = utf16_char(cu0, cu1, end);
                }
                if (!co_res.has_value())
                    throw InternalError("uni::detail::parse_code_units: Invalid utf-16 code unit sequence");
                co_yield co_res;
            }
            reset_strict_aliasing();
            co_return std::nullopt;
        }

        static ::SupDef::Coro<std::optional<utf32_char>> parse_code_units(
            const utf32_char::code_unit* const cstr,
            const size_t n,
            endianness end = endianness::undefined
        )
        {
            using cu_t = utf32_char::code_unit;

            std::optional<utf32_char> co_res;

            if (end == endianness::undefined)
                throw InternalError("uni::detail::parse_code_units: Undefined endianness");

            disable_strict_aliasing();
            const char* const limit = take_as<char>(cstr + n);
            for (const char* p = take_as<char>(cstr); p < limit;)
            {
                const char* const base_p = p;

                p = end == endianness::big
                  ? find_next_u32be(base_p, limit)
                  : find_next_u32le(base_p, limit);

                hard_assert(base_p <= p);

                if (p == nullptr || size_t(p - base_p) != 4)
                    co_res = std::nullopt;
                else
                {
                    const cu_t cu0 = *take_as<cu_t>(base_p) & all_bits<cu_t>(32UL);
                    co_res = utf32_char(cu0, end);
                }
                if (!co_res.has_value())
                    throw InternalError("uni::detail::parse_code_units: Invalid utf-32 code unit sequence");
                co_yield co_res;
            }
            reset_strict_aliasing();
            co_return std::nullopt;
        }

        static constexpr unicode_code_point to_code_point(const utf8_char& c) noexcept
        {
            size_t ncu = 1;
            uint_fast32_t cp = 0;

            if (c.get<1>() && *(c.get<1>()))
            {
                ++ncu;
                if (c.get<2>() && *(c.get<2>()))
                {
                    ++ncu;
                    if (c.get<3>() && *(c.get<3>()))
                        ++ncu;
                }
            }

            switch (ncu)
            {
                case 1:
                    cp  = uint_fast32_t(  c.get<0>()  & 0x7F);
                    break;
                case 2:
                    cp  = uint_fast32_t(  c.get<0>()  & 0x1F) << 6;
                    cp |= uint_fast32_t(*(c.get<1>()) & 0x3F);
                    break;
                case 3:
                    cp  = uint_fast32_t(  c.get<0>()  & 0x0F) << 12;
                    cp |= uint_fast32_t(*(c.get<1>()) & 0x3F) << 6;
                    cp |= uint_fast32_t(*(c.get<2>()) & 0x3F);
                    break;
                case 4:
                    cp  = uint_fast32_t(  c.get<0>()  & 0x07) << 18;
                    cp |= uint_fast32_t(*(c.get<1>()) & 0x3F) << 12;
                    cp |= uint_fast32_t(*(c.get<2>()) & 0x3F) << 6;
                    cp |= uint_fast32_t(*(c.get<3>()) & 0x3F);
                    break;
                default:
                    UNREACHABLE();
            }

            return cp;
        }

        static constexpr unicode_code_point to_code_point(const utf16_char& c)
        {
            uint_fast32_t cp = 0;
            c.set_endianness(endianness::big);

            if (is_high_surrogate(c.get<0>()))
            {
                if (!c.get<1>())
                    throw InternalError("uni::detail::to_code_point: Invalid utf-16 high surrogate");
                if (!is_low_surrogate(*(c.get<1>())))
                    throw InternalError("uni::detail::to_code_point: Invalid utf-16 low surrogate");
                cp  = uint_fast32_t(  c.get<0>()  & 0x03FF) << 10;
                cp |= uint_fast32_t(*(c.get<1>()) & 0x03FF);
                cp += 0x10000;
            }
            else
            {
                cp = uint_fast32_t(c.get<0>());
                hard_assert(!c.get<1>());
            }

            return cp;
        }

        static constexpr unicode_code_point to_code_point(const utf32_char& c) noexcept
        {
            return uint_fast32_t(c.get());
        }

        template <::uni::uni_char CharT>
        static constexpr CharT from_code_point(
            unicode_code_point cp,
            symbol_unused endianness end = std::conditional_t<
                std::same_as<CharT, utf8_char>,
                std::integral_constant<endianness, endianness::undefined>,
                std::integral_constant<endianness, endianness::big>
            >::value
        )
        {
            using cu_t  = typename CharT::code_unit;
            using ucp_t = unicode_code_point;

            unlikely_if (cp > 0x10FFFF || is_surrogate(cp))
                throw InternalError("uni::detail::from_code_point: Invalid unicode code point");

            if (cp == 0)
                return CharT();

            if constexpr (std::same_as<CharT, utf8_char>)
            {
                size_t ncu = 1;
                probably_if (cp > 0x7F, 30, CHAOS) // 30% chance
                {
                    ++ncu;
                    if (cp > 0x7FF)
                    {
                        ++ncu;
                        if (cp > 0xFFFF)
                            ++ncu;
                    }
                }
                switch (ncu)
                {
                    case 1:
                        assert(cp & 0x7F == cp);
                        cu_t cu1 = cu_t(cp);
                        return CharT(cu1, std::nullopt, std::nullopt, std::nullopt);
                    case 2:
                        cu_t cu1 = cu_t(((cp >> 6) & ucp_t(0x1F)) | ucp_t(0xC0));
                        cu_t cu2 = cu_t(( cp       & ucp_t(0x3F)) | ucp_t(0x80));
                        return CharT(cu1, cu2, std::nullopt, std::nullopt);
                    case 3:
                        cu_t cu1 = cu_t(((cp >> 12) & ucp_t(0x0F)) | ucp_t(0xE0));
                        cu_t cu2 = cu_t(((cp >>  6) & ucp_t(0x3F)) | ucp_t(0x80));
                        cu_t cu3 = cu_t(( cp        & ucp_t(0x3F)) | ucp_t(0x80));
                        return CharT(cu1, cu2, cu3, std::nullopt);
                    case 4:
                        cu_t cu1 = cu_t(((cp >> 18) & ucp_t(0x07)) | ucp_t(0xF0));
                        cu_t cu2 = cu_t(((cp >> 12) & ucp_t(0x3F)) | ucp_t(0x80));
                        cu_t cu3 = cu_t(((cp >>  6) & ucp_t(0x3F)) | ucp_t(0x80));
                        cu_t cu4 = cu_t(( cp        & ucp_t(0x3F)) | ucp_t(0x80));
                        return CharT(cu1, cu2, cu3, cu4);
                    default:
                        break;
                }
            }
            else if constexpr (std::same_as<CharT, utf16_char>)
            {
                likely_if (end == endianness::big || end == endianness::little)
                {
                    probably_if (cp <= 0xFFFF, 70, CHAOS) // 70% chance
                    {
                        cu_t cu1 = cu_t(cp);
                        if (end == endianness::big)
                            return CharT(cu1, std::nullopt);
                        else
                            return CharT(std::byteswap(cu1), std::nullopt);
                    }
                    else
                    {
                        cp -= 0x10000;
                        cu_t cu1 = cu_t((cp >> 10)    + 0xD800);
                        cu_t cu2 = cu_t((cp & 0x03FF) + 0xDC00);
                        if (end == endianness::big)
                            return CharT(cu1, cu2);
                        else
                            return CharT(std::byteswap(cu1), std::byteswap(cu2));
                    }
                }
                unlikely_else
                    throw InternalError("uni::detail::from_code_point: Invalid endianness");
            }
            else if constexpr (std::same_as<CharT, utf32_char>)
            {
                if (end == endianness::big)
                {
                    cu_t cu1 = cu_t(cp);
                    return CharT(cu1);
                }
                else if (end == endianness::little)
                {
                    cu_t cu1 = cu_t(std::byteswap(cp));
                    return CharT(cu1);
                }
                else
                    throw InternalError("uni::detail::from_code_point: Invalid endianness");
            }
            UNREACHABLE();
        }
    }

    template <typename CharT>
    struct char_traits : public std::char_traits<CharT>
    {
        static constexpr bool is_utf8 = false;
        static constexpr bool is_utf16 = false;
        static constexpr bool is_utf32 = false;
        typedef CharT base_char_type;

        using typename std::char_traits<CharT>::char_type;
        using typename std::char_traits<CharT>::int_type;
        using typename std::char_traits<CharT>::off_type;
        using typename std::char_traits<CharT>::pos_type;
        using typename std::char_traits<CharT>::state_type;
    };

    template <>
    struct char_traits<utf8_char>
    {
        static constexpr bool is_utf8 = true;
        static constexpr bool is_utf16 = false;
        static constexpr bool is_utf32 = false;
        
        typedef utf8_char::code_unit base_char_type;

        typedef utf8_char char_type;
        typedef detail::unicode_code_point_signed int_type;

        typedef std::char_traits<char8_t>::off_type off_type;
        typedef std::char_traits<char8_t>::pos_type pos_type;

        typedef void state_type; // Unused

        static constexpr bool is_null(const char_type& c) noexcept
        {
            return c.get<0>() == 0            &&
                   c.get<1>() == std::nullopt &&
                   c.get<2>() == std::nullopt &&
                   c.get<3>() == std::nullopt;
        }
        static constexpr bool is_null(const int_type& c) noexcept
        {
            return c == 0;
        }
        static constexpr std::vector<base_char_type> to_base_char(const char_type* s, size_t n)
        {
            std::vector<base_char_type> result;
            for (size_t i = 0; i < n; ++i)
            {
                result.push_back(s[i].get<0>());
                
                if (s[i].get<1>() && s[i].get<1>().value() != 0)
                    result.push_back(s[i].get<1>().value());
                else continue;

                if (s[i].get<2>() && s[i].get<2>().value() != 0)
                    result.push_back(s[i].get<2>().value());
                else continue;

                if (s[i].get<3>() && s[i].get<3>().value() != 0)
                    result.push_back(s[i].get<3>().value());
                else continue;
            }
            result.push_back(base_char_type());
            return result;
        }
        static constexpr std::vector<char_type> from_base_char(const base_char_type* s, size_t n)
        {
            std::vector<char_type> result(n);
            for (std::optional<char_type> c : detail::parse_code_units(s, n))
            {
                if (c)
                    result.push_back(*c);
            }
            result.push_back(char_type());
            return result;
        }

        static constexpr bool eq(char_type c1, char_type c2) noexcept
        {
            return c1.get<0>() == c2.get<0>() &&
                   c1.get<1>() == c2.get<1>() &&
                   c1.get<2>() == c2.get<2>() &&
                   c1.get<3>() == c2.get<3>();
        }
        static constexpr bool lt(char_type c1, char_type c2) noexcept
        {
            return detail::to_code_point(c1) < detail::to_code_point(c2);
        }
        static constexpr int compare(const char_type* s1, const char_type* s2, size_t n) noexcept
        {
            for (size_t i = 0; i < n; ++i)
            {
                if (lt(s1[i], s2[i]))
                    return -1;
                if (lt(s2[i], s1[i]))
                    return 1;
            }
            return 0;
        }
        static constexpr size_t length(const char_type* s) noexcept
        {
            size_t i = 0;
            while (!is_null(s[i]))
                ++i;
            return i;
        }
        static constexpr const char_type* find(const char_type* s, size_t n, const char_type& a) noexcept
        {
            // Can't rely on the value of eventual padding bits of utf8_char's being 0,
            // so we cannot simply use `::memmem`
#if 0
            if consteval
            {
                for (size_t i = 0; i < n; ++i)
                {
                    if (eq(s[i], a))
                        return s + i;
                }
                return nullptr;
            }
            else
            {
                return static_cast<const char_type*>(
                    ::memmem(
                        s,
                        n * sizeof(char_type),
                        &a,
                        sizeof(char_type)
                    )
                );
            }
#else
            for (size_t i = 0; i < n; ++i)
            {
                if (eq(s[i], a))
                    return s + i;
            }
            return nullptr;
#endif
        }
        static constexpr char_type* move(char_type* s1, const char_type* s2, size_t n) noexcept
        {
            unlikely_if (n == 0)
                return s1;
            unlikely_if (s1 == s2)
                return s1;
            if consteval
            {
                return ::uni::detail::char_traits_move(s1, s2, n);
            }
            else
            {
                return static_cast<char_type*>(
                    std::memmove(
                        std::assume_aligned<alignof(char_type)>(s1),
                        std::assume_aligned<alignof(char_type)>(s2),
                        n * sizeof(char_type)
                    )
                );
            }
        }
        static constexpr char_type* copy(char_type* restrict s1, const char_type* restrict s2, size_t n) noexcept
        {
            unlikely_if (n == 0)
                return s1;
            if consteval
            {
                for (size_t i = 0; i < n; ++i)
                    assign(s1[i], s2[i]);
                return s1;
            }
            else
            {
                return static_cast<char_type*>(
                    std::memcpy(
                        std::assume_aligned<alignof(char_type)>(s1),
                        std::assume_aligned<alignof(char_type)>(s2),
                        n * sizeof(char_type)
                    )
                );
            }
        }
        static constexpr void assign(char_type& c1, const char_type& c2) noexcept(noexcept(std::construct_at(&c1, c2)))
        {
            std::construct_at(&c1, c2);
        }
        static constexpr char_type* assign(char_type* s, size_t n, char_type a) noexcept(noexcept(assign(s[0], a)))
        {
            if consteval
            {
                for (size_t i = 0; i < n; ++i)
                    assign(s[i], a);
                return s;
            }
            else
            {
                /* TODO("Test this"); */
                if (n >= ::SupDef::Util::saturated_mul<decltype(n)>(1024, 1024))
                    std::for_each_n(
                        stdexec::par_unseq,
                        s,
                        n,
                        [&a](char_type& c) { assign(c, a); }
                    );
                else
                {
                    // Probably not worth using parallel algorithms for relatively small strings
                    ::uni::detail::memfill(s, a, n);
                }
                return s;
            }
        }
        static constexpr int_type not_eof(int_type c) noexcept
        {
            return eq_int_type(c, eof()) ? int_type(0) : c;
        }
        static constexpr char_type to_char_type(int_type c) noexcept
        {
            if (c < 0 || c > 0x10FFFF)
                return char_type();
            return detail::from_code_point<utf8_char>(
                static_cast<detail::unicode_code_point>(c)
            );
        }
        static constexpr int_type to_int_type(char_type c) noexcept
        {
            return detail::to_code_point(c);
        }
        static constexpr bool eq_int_type(int_type c1, int_type c2) noexcept
        {
            return c1 == c2;
        }
        static constexpr int_type eof() noexcept
        {
            return int_type{-1};
        }
    };

    template <>
    struct char_traits<utf16_char>
    {
        static constexpr bool is_utf8 = false;
        static constexpr bool is_utf16 = true;
        static constexpr bool is_utf32 = false;

        typedef utf16_char::code_unit base_char_type;

        typedef utf16_char char_type;
        typedef detail::tuple<detail::unicode_code_point_signed, endianness> int_type;

        typedef std::char_traits<char16_t>::off_type off_type;
        typedef std::char_traits<char16_t>::pos_type pos_type;

        typedef void state_type; // Unused

        static constexpr endianness get_endianness(const char_type* restrict const c) noexcept
        {
            return c->get_endianness();
        }
        static constexpr endianness get_endianness(const int_type* restrict const c) noexcept
        {
            return std::get<1>(*c);
        }

        static constexpr endianness get_endianness(const char_type& c) noexcept
        {
            return c.get_endianness();
        }
        static constexpr endianness get_endianness(const int_type& c) noexcept
        {
            return std::get<1>(c);
        }
        static constexpr void set_endianness(char_type& c, endianness end)
        {
            c.set_endianness(end);
        }
        static constexpr void set_endianness(int_type& c, endianness end)
        {
            unlikely_if (end == endianness::undefined)
                throw InternalError("uni::char_traits<utf16_char>::set_endianness: Undefined endianness");
            std::get<1>(c) = end;
        }

        static constexpr bool is_null(const char_type& c) noexcept
        {
            return c.get<0>() == 0 &&
                   c.get<1>() == std::nullopt;
        }
        static constexpr bool is_null(const int_type& c) noexcept
        {
            return std::get<0>(c) == 0;
        }
        static constexpr std::vector<base_char_type> to_base_char(const char_type* s, size_t n)
        {
            std::vector<base_char_type> result;
            for (size_t i = 0; i < n; ++i)
            {
                result.push_back(s[i].get<0>());
                if (s[i].get<1>() && s[i].get<1>().value() != 0)
                    result.push_back(s[i].get<1>().value());
                else continue;
            }
            result.push_back(base_char_type());
            return result;
        }
        static constexpr std::vector<char_type> from_base_char(const base_char_type* s, size_t n, endianness end = endianness::undefined)
        {
            std::vector<char_type> result(n);
            for (std::optional<char_type> c : detail::parse_code_units(s, n, end))
            {
                if (c)
                    result.push_back(*c);
            }
            result.push_back(char_type());
            return result;
        }

        static constexpr bool eq(char_type c1, char_type c2) noexcept
        {
            if (is_null(c1))
                return is_null(c2);
            if (is_null(c2))
                return false;

            return c1.get<0>()         == c2.get<0>() &&
                   c1.get<1>()         == c2.get<1>() &&
                   c1.get_endianness() == c2.get_endianness();
        }
        static constexpr bool lt(char_type c1, char_type c2) noexcept
        {
            return detail::to_code_point(c1) < detail::to_code_point(c2);
        }
        static constexpr int compare(const char_type* s1, const char_type* s2, size_t n) noexcept
        {
            for (size_t i = 0; i < n; ++i)
            {
                if (lt(s1[i], s2[i]))
                    return -1;
                if (lt(s2[i], s1[i]))
                    return 1;
            }
            return 0;
        }
        static constexpr size_t length(const char_type* s) noexcept
        {
            size_t i = 0;
            while (!is_null(s[i]))
                ++i;
            return i;
        }
        static constexpr const char_type* find(const char_type* s, size_t n, const char_type& a) noexcept
        {
            // Can't rely on the value of eventual padding bits of utf16_char's being 0,
            // so we cannot simply use `::memmem`
            for (size_t i = 0; i < n; ++i)
            {
                if (eq(s[i], a))
                    return s + i;
            }
            return nullptr;
        }
        static constexpr char_type* move(char_type* s1, const char_type* s2, size_t n) noexcept
        {
            unlikely_if (n == 0)
                return s1;
            unlikely_if (s1 == s2)
                return s1;
            if consteval
            {
                return ::uni::detail::char_traits_move(s1, s2, n);
            }
            else
            {
                return static_cast<char_type*>(
                    std::memmove(
                        std::assume_aligned<alignof(char_type)>(s1),
                        std::assume_aligned<alignof(char_type)>(s2),
                        n * sizeof(char_type)
                    )
                );
            }
        }
        static constexpr char_type* copy(char_type* restrict s1, const char_type* restrict s2, size_t n) noexcept
        {
            unlikely_if (n == 0)
                return s1;
            if consteval
            {
                for (size_t i = 0; i < n; ++i)
                    assign(s1[i], s2[i]);
                return s1;
            }
            else
            {
                return static_cast<char_type*>(
                    std::memcpy(
                        std::assume_aligned<alignof(char_type)>(s1),
                        std::assume_aligned<alignof(char_type)>(s2),
                        n * sizeof(char_type)
                    )
                );
            }
        }
        static constexpr void assign(char_type& c1, const char_type& c2) noexcept(noexcept(std::construct_at(&c1, c2)))
        {
            std::construct_at(&c1, c2);
        }
        static constexpr char_type* assign(char_type* s, size_t n, char_type a) noexcept(noexcept(assign(s[0], a)))
        {
            if consteval
            {
                for (size_t i = 0; i < n; ++i)
                    assign(s[i], a);
                return s;
            }
            else
            {
                /* TODO("Test this"); */
                if (n >= ::SupDef::Util::saturated_mul<decltype(n)>(1024, 1024))
                    std::for_each_n(
                        stdexec::par_unseq,
                        s,
                        n,
                        [&a](char_type& c) { assign(c, a); }
                    );
                else
                {
                    // Probably not worth using parallel algorithms for relatively small strings
                    ::uni::detail::memfill(s, a, n);
                }
                return s;
            }
        }
        static constexpr int_type not_eof(int_type c) noexcept
        {
            constexpr int_type neofc{0};
            return eq_int_type(c, eof()) ? neofc : c;
        }
        static constexpr char_type to_char_type(int_type c) noexcept
        {
            if (std::get<0>(c) < 0 || std::get<0>(c) > 0x10FFFF || std::get<1>(c) == endianness::undefined)
                return char_type();
            return detail::from_code_point<utf16_char>(
                static_cast<detail::unicode_code_point>(std::get<0>(c)),
                std::get<1>(c)
            );
        }
        static constexpr int_type to_int_type(char_type c) noexcept
        {
            return int_type{detail::to_code_point(c), c.get_endianness()};
        }
        static constexpr bool eq_int_type(int_type c1, int_type c2) noexcept
        {
            if (is_null(c1))
                return is_null(c2);
            if (is_null(c2))
                return false;

            return std::get<0>(c1) == std::get<0>(c2) &&
                   std::get<1>(c1) == std::get<1>(c2);
        }
        static constexpr int_type eof() noexcept
        {
            constexpr int_type eofc{-1, endianness::undefined};
            return eofc;
        }
    };
    
    template <>
    struct char_traits<utf32_char>
    {
        static constexpr bool is_utf8 = false;
        static constexpr bool is_utf16 = false;
        static constexpr bool is_utf32 = true;

        typedef utf32_char::code_unit base_char_type;

        typedef utf32_char char_type;
        typedef detail::tuple<detail::unicode_code_point_signed, endianness> int_type;

        typedef std::char_traits<char32_t>::off_type off_type;
        typedef std::char_traits<char32_t>::pos_type pos_type;

        typedef void state_type; // Unused

        static constexpr endianness get_endianness(const char_type* restrict const c) noexcept
        {
            return c->get_endianness();
        }
        static constexpr endianness get_endianness(const int_type* restrict const c) noexcept
        {
            return std::get<1>(*c);
        }

        static constexpr endianness get_endianness(const char_type& c) noexcept
        {
            return c.get_endianness();
        }
        static constexpr endianness get_endianness(const int_type& c) noexcept
        {
            return std::get<1>(c);
        }
        static constexpr void set_endianness(char_type& c, endianness end)
        {
            c.set_endianness(end);
        }
        static constexpr void set_endianness(int_type& c, endianness end)
        {
            unlikely_if (end == endianness::undefined)
                throw InternalError("uni::char_traits<utf32_char>::set_endianness: Undefined endianness");
            std::get<1>(c) = end;
        }

        static constexpr bool is_null(const char_type& c) noexcept
        {
            return c.get() == 0;
        }
        static constexpr bool is_null(const int_type& c) noexcept
        {
            return std::get<0>(c) == 0;
        }
        static constexpr std::vector<base_char_type> to_base_char(const char_type* s, size_t n)
        {
            std::vector<base_char_type> result;
            for (size_t i = 0; i < n; ++i)
                result.push_back(s[i].get());
            result.push_back(base_char_type());
            return result;
        }
        static constexpr std::vector<char_type> from_base_char(const base_char_type* s, size_t n, endianness end = endianness::undefined)
        {
            std::vector<char_type> result(n);
            for (std::optional<char_type> c : detail::parse_code_units(s, n, end))
            {
                if (c)
                    result.push_back(*c);
            }
            result.push_back(char_type());
            return result;
        }

        static constexpr bool eq(char_type c1, char_type c2) noexcept
        {
            return c1.get() == c2.get();
        }
        static constexpr bool lt(char_type c1, char_type c2) noexcept
        {
            return detail::to_code_point(c1) < detail::to_code_point(c2);
        }
        static constexpr int compare(const char_type* s1, const char_type* s2, size_t n) noexcept
        {
            for (size_t i = 0; i < n; ++i)
            {
                if (lt(s1[i], s2[i]))
                    return -1;
                if (lt(s2[i], s1[i]))
                    return 1;
            }
            return 0;
        }
        static constexpr size_t length(const char_type* s) noexcept
        {
            size_t i = 0;
            while (!is_null(s[i]))
                ++i;
            return i;
        }
        static constexpr const char_type* find(const char_type* s, size_t n, const char_type& a) noexcept
        {
            // Can't rely on the value of eventual padding bits of utf32_char's being 0,
            // so we cannot simply use `::memmem`
            for (size_t i = 0; i < n; ++i)
            {
                if (eq(s[i], a))
                    return s + i;
            }
            return nullptr;
        }
        static constexpr char_type* move(char_type* s1, const char_type* s2, size_t n) noexcept
        {
            unlikely_if (n == 0)
                return s1;
            unlikely_if (s1 == s2)
                return s1;
            if consteval
            {
                return ::uni::detail::char_traits_move(s1, s2, n);
            }
            else
            {
                return static_cast<char_type*>(
                    std::memmove(
                        std::assume_aligned<alignof(char_type)>(s1),
                        std::assume_aligned<alignof(char_type)>(s2),
                        n * sizeof(char_type)
                    )
                );
            }
        }
        static constexpr char_type* copy(char_type* restrict s1, const char_type* restrict s2, size_t n) noexcept
        {
            unlikely_if (n == 0)
                return s1;
            if consteval
            {
                for (size_t i = 0; i < n; ++i)
                    assign(s1[i], s2[i]);
                return s1;
            }
            else
            {
                return static_cast<char_type*>(
                    std::memcpy(
                        std::assume_aligned<alignof(char_type)>(s1),
                        std::assume_aligned<alignof(char_type)>(s2),
                        n * sizeof(char_type)
                    )
                );
            }
        }
        static constexpr void assign(char_type& c1, const char_type& c2) noexcept(noexcept(std::construct_at(&c1, c2)))
        {
            std::construct_at(&c1, c2);
        }
        static constexpr char_type* assign(char_type* s, size_t n, char_type a) noexcept(noexcept(assign(s[0], a)))
        {
            if consteval
            {
                for (size_t i = 0; i < n; ++i)
                    assign(s[i], a);
                return s;
            }
            else
            {
                /* TODO("Test this"); */
                if (n >= ::SupDef::Util::saturated_mul<decltype(n)>(1024, 1024))
                    std::for_each_n(
                        stdexec::par_unseq,
                        s,
                        n,
                        [&a](char_type& c) { assign(c, a); }
                    );
                else
                {
                    // Probably not worth using parallel algorithms for relatively small strings
                    ::uni::detail::memfill(s, a, n);
                }
                return s;
            }
        }
        static constexpr int_type not_eof(int_type c) noexcept
        {
            constexpr int_type neofc{0};
            return eq_int_type(c, eof()) ? neofc : c;
        }
        static constexpr char_type to_char_type(int_type c) noexcept
        {
            if (std::get<0>(c) < 0 || std::get<0>(c) > 0x10FFFF || std::get<1>(c) == endianness::undefined)
                return char_type();
            return detail::from_code_point<utf32_char>(
                static_cast<detail::unicode_code_point>(std::get<0>(c)),
                std::get<1>(c)
            );
        }
        static constexpr int_type to_int_type(char_type c) noexcept
        {
            return int_type{detail::to_code_point(c), c.get_endianness()};
        }
        static constexpr bool eq_int_type(int_type c1, int_type c2) noexcept
        {
            if (is_null(c1))
                return is_null(c2);
            if (is_null(c2))
                return false;

            return std::get<0>(c1) == std::get<0>(c2) &&
                   std::get<1>(c1) == std::get<1>(c2);
        }
        static constexpr int_type eof() noexcept
        {
            constexpr int_type eofc{-1, endianness::undefined};
            return eofc;
        }
    };
}