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

namespace uni
{
    namespace detail
    {
        template <typename CharT>
        consteval CharT* char_traits_move(CharT* s1, const CharT* s2, size_t n) noexcept
        {
            using traits_type = char_traits<CharT>;

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
    }

    template <typename CharT>
    struct char_traits : public std::char_traits<CharT>
    {
        static constexpr bool is_utf8 = false;
        static constexpr bool is_utf16 = false;
        static constexpr bool is_utf32 = false;
    };

    template <>
    struct char_traits<utf8_char>
    {
        static constexpr bool is_utf8 = true;
        static constexpr bool is_utf16 = false;
        static constexpr bool is_utf32 = false;

        typedef utf8_char char_type;
        typedef uint_least16_t int_type;
        typedef std::char_traits<char8_t>::off_type off_type;
        typedef std::char_traits<char8_t>::pos_type pos_type;
        typedef std::char_traits<char8_t>::state_type state_type;

        static constexpr bool eq(char_type c1, char_type c2) noexcept
        {
            return c1 == c2;
        }
        static constexpr bool lt(char_type c1, char_type c2) noexcept
        {
            return c1 < c2;
        }
        static constexpr int compare(const char_type* s1, const char_type* s2, size_t n) noexcept
        {
            if consteval
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
            else
                return std::memcmp(s1, s2, n * sizeof(char_type));
            UNREACHABLE();
        }
        static constexpr size_t length(const char_type* s) noexcept
        {
            size_t i = 0;
            while (!eq(s[i], char_type()))
                ++i;
            return i;
        }
        static constexpr const char_type* find(const char_type* s, size_t n, const char_type& a) noexcept
        {
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
                return static_cast<const char_type*>(std::memchr(s, a, n * sizeof(char_type)));
            }
            UNREACHABLE();
        }
        static constexpr char_type* move(char_type* s1, const char_type* s2, size_t n) noexcept
        {
            unlikely_if (n == 0)
                return s1;
            unlikely_if (s1 == s2)
                return s1;
            if consteval
                return ::uni::detail::char_traits_move(s1, s2, n);
            else
                return static_cast<char_type*>(std::memmove(s1, s2, n * sizeof(char_type)));
            UNREACHABLE();
        }
        static constexpr char_type* copy(char_type* s1, const char_type* s2, size_t n) noexcept
        {
            unlikely_if (n == 0)
                return s1;
            unlikely_if (s1 == s2)
                return s1;
            if consteval
            {
                for (size_t i = 0; i < n; ++i)
                    assign(s1[i], s2[i]);
                return s1;
            }
            else
                return static_cast<char_type*>(std::memcpy(s1, s2, n * sizeof(char_type)));
            UNREACHABLE();
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
                std::for_each_n(
                    std::execution::par_unseq,
                    s,
                    n,
                    [&a](char_type& c) { assign(c, a); }
                );
                return s;
            }
            UNREACHABLE();
        }
        static constexpr int_type not_eof(int_type c) noexcept
        {
            return eq_int_type(c, eof()) ? int_type(0) : c;
        }
        static constexpr char_type to_char_type(int_type c) noexcept
        {
            if (c > std::numeric_limits<char_type>::max())
                return char_type();
            return static_cast<char_type>(c);
        }
        static constexpr int_type to_int_type(char_type c) noexcept
        {
            return static_cast<int_type>(c);
        }
        static constexpr bool eq_int_type(int_type c1, int_type c2) noexcept
        {
            return c1 == c2;
        }
        static constexpr int_type eof() noexcept
        {
            return static_cast<int_type>(-1);
        }
    };

    template <>
    struct char_traits<utf16_char>
    {
        static constexpr bool is_utf8 = false;
        static constexpr bool is_utf16 = true;
        static constexpr bool is_utf32 = false;

        typedef utf16_char char_type;
        struct int_type
        {
            uint_least32_t value;
            endianness endian;

            int_type() noexcept
                : value(0), endian(endianness::undefined) { }
            int_type(uint_least32_t value) noexcept
                : value(value), endian(endianness::undefined) { }
        };
        typedef std::char_traits<char16_t>::off_type off_type;
        typedef std::char_traits<char16_t>::pos_type pos_type;
        typedef std::char_traits<char16_t>::state_type state_type;

        static constexpr endianness get_endianness(char_type c) noexcept
        {
            return c.endian;
        }
        static constexpr endianness get_endianness(int_type c) noexcept
        {
            return c.endian;
        }
        static constexpr endianness get_endianness(const char_type* s)
        {
            return s->endian;
        }
        static constexpr endianness get_endianness(const int_type* s)
        {
            return s->endian;
        }
        static constexpr bool is_null(char_type c) noexcept
        {
            return eq(c, char_type{0, endianness::undefined}) || eq(c, char_type{0, get_endianness(c)});
        }
        static constexpr bool is_null(int_type c) noexcept
        {
            return eq_int_type(c, int_type{0, endianness::undefined}) || eq_int_type(c, int_type{0, get_endianness(c)});
        }

        static constexpr bool eq(char_type c1, char_type c2) noexcept
        {
            return c1.value == c2.value && c1.endian == c2.endian;
        }
        static constexpr bool lt(char_type c1, char_type c2) noexcept
        {
            return c1.value < c2.value || (c1.value == c2.value && std::to_underlying(c1.endian) < std::to_underlying(c2.endian));
        }
        static constexpr int compare(const char_type* s1, const char_type* s2, size_t n) noexcept
        {
            if consteval
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
            else
                return std::memcmp(s1, s2, n * sizeof(char_type));
            UNREACHABLE();
        }
        static constexpr size_t length(const char_type* s) noexcept
        {
            size_t i = 0;
#if 0
            char_type null1 = char_type(0);
            char_type null2 = char_type(0, get_endianness(s));
            while (!eq(s[i], null1) && !eq(s[i], null2))
#else
            while (!is_null(s[i]))
#endif
                ++i;
            return i;
        }
        static constexpr const char_type* find(const char_type* s, size_t n, const char_type& a) noexcept
        {
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
                return ::uni::detail::char_traits_move(s1, s2, n);
            else
                return static_cast<char_type*>(std::memmove(s1, s2, n * sizeof(char_type)));
            UNREACHABLE();
        }
        static constexpr char_type* copy(char_type* s1, const char_type* s2, size_t n) noexcept
        {
            unlikely_if (n == 0)
                return s1;
            unlikely_if (s1 == s2)
                return s1;
            if consteval
            {
                for (size_t i = 0; i < n; ++i)
                    assign(s1[i], s2[i]);
                return s1;
            }
            else
                return static_cast<char_type*>(std::memcpy(s1, s2, n * sizeof(char_type)));
            UNREACHABLE();
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
                std::for_each_n(
                    std::execution::par_unseq,
                    s,
                    n,
                    [&a](char_type& c) { assign(c, a); }
                );
                return s;
            }
            UNREACHABLE();
        }
        static constexpr int_type not_eof(int_type c) noexcept
        {
            return eq_int_type(c, eof()) ? int_type{ 0, endianness::undefined } : c;
        }
        static constexpr char_type to_char_type(int_type c) noexcept
        {
            if (c.value > std::numeric_limits<char_type>::max())
                return char_type(0, c.endian);
            return char_type(c.value, c.endian);
        }
        static constexpr int_type to_int_type(char_type c) noexcept
        {
            return int_type{ c.value, c.endian };
        }
        static constexpr bool eq_int_type(int_type c1, int_type c2) noexcept
        {
            return c1.value == c2.value && c1.endian == c2.endian;
        }
        static constexpr int_type eof() noexcept
        {
            return int_type{ -1, endianness::undefined };
        }
    };

    template <>
    struct char_traits<utf32_char>
    {
        static constexpr bool is_utf8 = false;
        static constexpr bool is_utf16 = false;
        static constexpr bool is_utf32 = true;

        typedef utf32_char char_type;
        struct int_type
        {
            uint_least64_t value;
            endianness endian;

            int_type() noexcept
                : value(0), endian(endianness::undefined) { }
            int_type(uint_least64_t value) noexcept
                : value(value), endian(endianness::undefined) { }
        };
        typedef std::char_traits<char32_t>::off_type off_type;
        typedef std::char_traits<char32_t>::pos_type pos_type;
        typedef std::char_traits<char32_t>::state_type state_type;

        static constexpr endianness get_endianness(char_type c) noexcept
        {
            return c.endian;
        }
        static constexpr endianness get_endianness(int_type c) noexcept
        {
            return c.endian;
        }
        static constexpr endianness get_endianness(const char_type* s)
        {
            return s->endian;
        }
        static constexpr endianness get_endianness(const int_type* s)
        {
            return s->endian;
        }
        static constexpr bool is_null(char_type c) noexcept
        {
            return eq(c, char_type{0, endianness::undefined}) || eq(c, char_type{0, get_endianness(c)});
        }
        static constexpr bool is_null(int_type c) noexcept
        {
            return eq_int_type(c, int_type{0, endianness::undefined}) || eq_int_type(c, int_type{0, get_endianness(c)});
        }

        static constexpr bool eq(char_type c1, char_type c2) noexcept
        {
            return c1.value == c2.value && c1.endian == c2.endian;
        }
        static constexpr bool lt(char_type c1, char_type c2) noexcept
        {
            return c1.value < c2.value || (c1.value == c2.value && std::to_underlying(c1.endian) < std::to_underlying(c2.endian));
        }
        static constexpr int compare(const char_type* s1, const char_type* s2, size_t n) noexcept
        {
            if consteval
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
            else
                return std::memcmp(s1, s2, n * sizeof(char_type));
            UNREACHABLE();
        }
        static constexpr size_t length(const char_type* s) noexcept
        {
            size_t i = 0;
#if 0
            char_type null1 = char_type(0);
            char_type null2 = char_type(0, get_endianness(s));
            while (!eq(s[i], null1) && !eq(s[i], null2))
#else
            while (!is_null(s[i]))
#endif
                ++i;
            return i;
        }
        static constexpr const char_type* find(const char_type* s, size_t n, const char_type& a) noexcept
        {
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
                return ::uni::detail::char_traits_move(s1, s2, n);
            else
                return static_cast<char_type*>(std::memmove(s1, s2, n * sizeof(char_type)));
            UNREACHABLE();
        }
        static constexpr char_type* copy(char_type* s1, const char_type* s2, size_t n) noexcept
        {
            unlikely_if (n == 0)
                return s1;
            unlikely_if (s1 == s2)
                return s1;
            if consteval
            {
                for (size_t i = 0; i < n; ++i)
                    assign(s1[i], s2[i]);
                return s1;
            }
            else
                return static_cast<char_type*>(std::memcpy(s1, s2, n * sizeof(char_type)));
            UNREACHABLE();
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
                std::for_each_n(
                    std::execution::par_unseq,
                    s,
                    n,
                    [&a](char_type& c) { assign(c, a); }
                );
                return s;
            }
            UNREACHABLE();
        }
        static constexpr int_type not_eof(int_type c) noexcept
        {
            return eq_int_type(c, eof()) ? int_type{ 0, endianness::undefined } : c;
        }
        static constexpr char_type to_char_type(int_type c) noexcept
        {
            if (c.value > std::numeric_limits<char_type>::max())
                return char_type(0, c.endian);
            return char_type(c.value, c.endian);
        }
        static constexpr int_type to_int_type(char_type c) noexcept
        {
            return int_type{ c.value, c.endian };
        }
        static constexpr bool eq_int_type(int_type c1, int_type c2) noexcept
        {
            return c1.value == c2.value && c1.endian == c2.endian;
        }
        static constexpr int_type eof() noexcept
        {
            return int_type{ -1, endianness::undefined };
        }
    };
}