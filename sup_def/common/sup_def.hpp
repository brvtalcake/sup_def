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

#include <sup_def/common/start_header.h>
#include <sup_def/common/config.h>

#ifndef SUP_DEF_HPP
#define SUP_DEF_HPP

STATIC_TODO(
    "Verify correctness of all std::forward's in the code"
);

// TODO: Use MAP_LIST macro instead of BOOST_PP_SEQ_FOR_EACH if possible
// TODO: Move class declarations to different headers as with a one-class-per-header organization
// TODO: Create a Util namespace and move all the utility / helper functions there
// TODO: Open all files in binary mode

#if !defined(_GNU_SOURCE)
    #define _GNU_SOURCE 1
#endif

#if defined(_WIN32)
    #include <windows.h>
#endif

#include <coroutine>
#include <iterator>
//#include <expected>

#if SUPDEF_COMPILER != 1
#include <cstdint>
#include <cassert>

#include <utility>
#include <optional>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <concepts>
#include <type_traits>
#include <vector>
#include <string>
#include <stdexcept>
#include <array>
#include <algorithm>
#include <iostream>
#include <locale>
#include <memory>
#include <cstdlib>
#include <unordered_map>
#include <map>
#include <variant>
#include <regex>
#include <functional>
#include <version>
#endif


#include <sup_def/third_party/map/map.h>
#include <boost/preprocessor.hpp>

#include <sup_def/common/util/util.hpp>
#include <sup_def/common/util/engine.hpp>

#include <sup_def/common/unistreams/unistreams.hpp>

#if COMPILING_EXTERNAL
    #include <sup_def/external/external.hpp>
#endif

namespace SupDef
{
    template <typename T>
        requires CharacterType<T>
    class ParsedChar : public std::tuple<string_size_type<T>, string_size_type<T>, T>
    {
        public:
            ParsedChar() = default;
            ParsedChar(const ParsedChar&) = default;
            ParsedChar(ParsedChar&&) = default;
            ParsedChar& operator=(const ParsedChar&) = default;
            ParsedChar& operator=(ParsedChar&&) = default;
            ~ParsedChar() = default;

            ParsedChar(const string_size_type<T>& line, const string_size_type<T>& col, const T& c) : std::tuple<string_size_type<T>, string_size_type<T>, T>(line, col, c)
            { }
            ParsedChar(string_size_type<T>&& line, string_size_type<T>&& col, T&& c) : std::tuple<string_size_type<T>, string_size_type<T>, T>(line, col, c)
            { }
            ParsedChar(const std::tuple<string_size_type<T>, string_size_type<T>, T>& t) : std::tuple<string_size_type<T>, string_size_type<T>, T>(t)
            { }
            ParsedChar(std::tuple<string_size_type<T>, string_size_type<T>, T>&& t) : std::tuple<string_size_type<T>, string_size_type<T>, T>(t)
            { }
            ParsedChar(const T& c) : std::tuple<string_size_type<T>, string_size_type<T>, T>(0, 0, c)
            { }

#if __cpp_explicit_this_parameter >= 202110L // "Deducing this" feature from C++23
        private:
            template <typename Self, typename Arg>
            using helper = std::conditional_t<std::is_const_v<std::remove_reference_t<Self>>, Arg, Arg&>;
        public:
            template <typename Self>
            inline ParsedChar<T>::helper<Self, string_size_type<T>>  line(this Self&& self) noexcept { return std::get<0>(self); }
            static_assert(std::same_as<decltype(std::declval<ParsedChar<T>>().line()), string_size_type<T>&>);
            static_assert(std::same_as<decltype(std::declval<const ParsedChar<T>>().line()), string_size_type<T>>);

            template <typename Self>
            inline ParsedChar<T>::helper<Self, string_size_type<T>>  col (this Self&& self) noexcept { return std::get<1>(self); }
            static_assert(std::same_as<decltype(std::declval<ParsedChar<T>>().col()), string_size_type<T>&>);
            static_assert(std::same_as<decltype(std::declval<const ParsedChar<T>>().col()), string_size_type<T>>);

            template <typename Self>
            inline ParsedChar<T>::helper<Self, T>                    val (this Self&& self) noexcept { return std::get<2>(self); }
            static_assert(std::same_as<decltype(std::declval<ParsedChar<T>>().val()), T&>);
            static_assert(std::same_as<decltype(std::declval<const ParsedChar<T>>().val()), T>);
#else
            inline constexpr string_size_type<T>  line(void) const noexcept { return std::get<0>(*this); }
            inline constexpr string_size_type<T>  col (void) const noexcept { return std::get<1>(*this); }
            inline constexpr T                    val (void) const noexcept { return std::get<2>(*this); }

            inline constexpr string_size_type<T>& line(void)       noexcept { return std::get<0>(*this); }
            inline constexpr string_size_type<T>& col (void)       noexcept { return std::get<1>(*this); }
            inline constexpr T&                   val (void)       noexcept { return std::get<2>(*this); }

#endif
            inline constexpr explicit operator T (void) const noexcept { return std::get<2>(*this); }
            inline constexpr explicit operator T&(void)       noexcept { return std::get<2>(*this); }
    };

    template <typename T, typename BaseTraits = std::char_traits<T>>
        requires CharacterType<T>
    class ParsedCharTraits
    {
        private:
            typedef T base_char_type;
            typedef BaseTraits base_traits_type;

            static inline constexpr bool strings_overlap(const base_char_type* s1, const base_char_type* s2, std::size_t n) noexcept
            {
                return (s1 <= s2 && s2 < s1 + n) || (s2 <= s1 && s1 < s2 + n);
            }

        public:
            typedef ParsedChar<T> char_type;
            typedef typename std::tuple<string_size_type<T>, string_size_type<T>, typename base_traits_type::int_type> int_type;
            typedef typename base_traits_type::pos_type pos_type;
            typedef typename base_traits_type::off_type off_type;
            typedef typename base_traits_type::state_type state_type;

            static inline constexpr bool eq(const char_type& c1, const char_type& c2) noexcept
            {
                return std::get<0>(c1) == std::get<0>(c2) && std::get<1>(c1) == std::get<1>(c2) && base_traits_type::eq(std::get<2>(c1), std::get<2>(c2));
            }

            static inline constexpr bool lt(const char_type& c1, const char_type& c2) noexcept
            {
                return base_traits_type::lt(std::get<2>(c1), std::get<2>(c2));
            }

            static inline constexpr int compare(const char_type* s1, const char_type* s2, std::size_t n) noexcept
            {
                for (std::size_t i = 0; i < n; ++i)
                {
                    if (lt(s1[i], s2[i]))
                        return -1;
                    else if (lt(s2[i], s1[i]))
                        return 1;
                }
                return 0;
            }

            static inline constexpr std::size_t length(const char_type* s) noexcept
            {
                std::size_t len = 0;
                while (!base_traits_type::eq(std::get<2>(s[len]), base_char_type()))
                    ++len;
                return len;
            }

            static inline constexpr const char_type* find(const char_type* s, std::size_t n, const char_type& a) noexcept
            {
                for (std::size_t i = 0; i < n; ++i)
                {
                    if (eq(s[i], a))
                        return s + i;
                }
                return nullptr;
            }

            static inline char_type* move(char_type* s1, const char_type* s2, std::size_t n) noexcept
            {
                if (strings_overlap(s1, s2, n))
                {
                    if (s1 < s2)
                        copy(s1, s2, n); 
                    else
                    {
                        for (std::size_t i = n; i > 0; --i)
                            assign(s1[i - 1], s2[i - 1]);
                    }
                }
                else
                    copy(s1, s2, n);
                return s1;
            }

            static inline char_type* copy(char_type* s1, const char_type* s2, std::size_t n) noexcept
            {
                assert(!strings_overlap(s1, s2, n));
                for (std::size_t i = 0; i < n; ++i)
                    assign(s1[i], s2[i]);
                return s1;
            }

            static inline constexpr char_type* assign(char_type& c1, const char_type& c2) noexcept
            {
                std::get<0>(c1) = std::get<0>(c2);
                std::get<1>(c1) = std::get<1>(c2);
                std::get<2>(c1) = std::get<2>(c2);
                return &c1;
            }

            static inline constexpr char_type* assign(char_type& c1, base_char_type c2) noexcept
            {
                std::get<0>(c1) = 0;
                std::get<1>(c1) = 0;
                std::get<2>(c1) = c2;
                return &c1;
            }

            static inline constexpr base_char_type* assign(base_char_type& c1, const char_type& c2) noexcept
            {
                c1 = std::get<2>(c2);
                return &c1;
            }

            static inline constexpr int_type not_eof(const int_type& c) noexcept
            {
                if (!eq_int_type(c, eof()))
                    return c;
                else
                    return std::make_tuple(0, 0, base_traits_type::not_eof(base_traits_type::eof()));
            }

            static inline constexpr int_type eof() noexcept
            {
                return std::make_tuple(std::numeric_limits<string_size_type<T>>::max(), std::numeric_limits<string_size_type<T>>::max(), base_traits_type::eof());
            }

            static inline constexpr int_type to_int_type(const char_type& c) noexcept
            {
                return std::make_tuple(std::get<0>(c), std::get<1>(c), base_traits_type::to_int_type(std::get<2>(c)));
            }

            static inline constexpr char_type to_char_type(const int_type& c) noexcept
            {
                return std::make_tuple(std::get<0>(c), std::get<1>(c), base_traits_type::to_char_type(std::get<2>(c)));
            }

            static inline constexpr bool eq_int_type(const int_type& c1, const int_type& c2) noexcept
            {
                return std::get<0>(c1) == std::get<0>(c2) && std::get<1>(c1) == std::get<1>(c2) && base_traits_type::eq_int_type(std::get<2>(c1), std::get<2>(c2));
            }
    };

    template <typename T>
        requires CharacterType<T>
    class ParsedCharString : public std::basic_string<ParsedChar<T>, ParsedCharTraits<T>>
    {
        private:
            typedef std::basic_string<ParsedChar<T>, ParsedCharTraits<T>> base_type;
        
        public:
            ParsedCharString() : base_type() {}
            ParsedCharString(const ParsedCharString&) = default;
            ParsedCharString(ParsedCharString&&) = default;
            ParsedCharString& operator=(const ParsedCharString&) = default;
            ParsedCharString& operator=(ParsedCharString&&) = default;
            ~ParsedCharString() = default;

            ParsedCharString(const base_type& str) : base_type(str) {}
            ParsedCharString(base_type&& str) : base_type(str) {}
            ParsedCharString& operator=(const base_type& str) { base_type::operator=(str); return *this; }
            ParsedCharString& operator=(base_type&& str) { base_type::operator=(str); return *this; }

            template <typename C>
                requires CharacterType<C>
            ParsedCharString(const std::basic_string<C>& str) : base_type()
            {
                size_t line, col, i;
                for (i = 0, line = 0, col = 0; i < str.size(); ++i)
                {
                    if (SAME(str[i], '\n'))
                    {
                        ++line;
                        col = 0;
                        continue;
                    }
                    this->push_back(ParsedChar<T>(line, col++, CONVERT(T, str[i])));
                }
            }
            template <typename C>
                requires CharacterType<C>
            ParsedCharString(std::basic_string<C>&& str) : base_type()
            {
                size_t line, col, i;
                for (i = 0, line = 0, col = 0; i < str.size(); ++i)
                {
                    if (SAME(str[i], '\n'))
                    {
                        ++line;
                        col = 0;
                        continue;
                    }
                    this->push_back(ParsedChar<T>(line, col++, CONVERT(T, str[i])));
                }
            }
            template <typename C>
                requires CharacterType<C>
            ParsedCharString& operator=(const std::basic_string<C>& str)
            {
                this->clear();
                size_t line, col, i;
                for (i = 0, line = 0, col = 0; i < str.size(); ++i)
                {
                    if (SAME(str[i], '\n'))
                    {
                        ++line;
                        col = 0;
                        continue;
                    }
                    this->push_back(ParsedChar<T>(line, col++, CONVERT(T, str[i])));
                }
                return *this;
            }
            template <typename C>
                requires CharacterType<C>
            ParsedCharString& operator=(std::basic_string<C>&& str)
            {
                this->clear();
                size_t line, col, i;
                for (i = 0, line = 0, col = 0; i < str.size(); ++i)
                {
                    if (SAME(str[i], '\n'))
                    {
                        ++line;
                        col = 0;
                        continue;
                    }
                    this->push_back(ParsedChar<T>(line, col++, CONVERT(T, str[i])));
                }
                return *this;
            }

            template <typename C>
                requires CharacterType<C>
            ParsedCharString(const C* str) : base_type()
            {
                size_t line, col, i;
                for (i = 0, line = 0, col = 0; DIFFERENT(str[i], '\0'); ++i)
                {
                    if (SAME(str[i], '\n'))
                    {
                        ++line;
                        col = 0;
                        continue;
                    }
                    this->push_back(ParsedChar<T>(line, col++, CONVERT(T, str[i])));
                }
            }
            template <typename C>
                requires CharacterType<C>
            ParsedCharString& operator=(const C* str)
            {
                this->clear();
                size_t line, col, i;
                for (i = 0, line = 0, col = 0; DIFFERENT(str[i], '\0'); ++i)
                {
                    if (SAME(str[i], '\n'))
                    {
                        ++line;
                        col = 0;
                        continue;
                    }
                    this->push_back(ParsedChar<T>(line, col++, CONVERT(T, str[i])));
                }
                return *this;
            }

            template <typename C>
                requires CharacterType<C>
            ParsedCharString(const C* str, size_t n) : base_type()
            {
                size_t line, col, i;
                for (i = 0, line = 0, col = 0; i < n; ++i)
                {
                    if (SAME(str[i], '\n'))
                    {
                        ++line;
                        col = 0;
                        continue;
                    }
                    this->push_back(ParsedChar<T>(line, col++, CONVERT(T, str[i])));
                }
            }

            template <typename C>
                requires CharacterType<C>
            ParsedCharString(std::initializer_list<C> il) : base_type()
            {
                size_t line, col, i;
                for (i = 0, line = 0, col = 0; i < il.size(); ++i)
                {
                    if (SAME(il[i], '\n'))
                    {
                        ++line;
                        col = 0;
                        continue;
                    }
                    this->push_back(ParsedChar<T>(line, col++, CONVERT(T, il[i])));
                }
            }
            template <typename C>
                requires CharacterType<C>
            ParsedCharString& operator=(std::initializer_list<C> il)
            {
                this->clear();
                size_t line, col, i;
                for (i = 0, line = 0, col = 0; i < il.size(); ++i)
                {
                    if (SAME(il[i], '\n'))
                    {
                        ++line;
                        col = 0;
                        continue;
                    }
                    this->push_back(ParsedChar<T>(line, col++, CONVERT(T, il[i])));
                }
                return *this;
            }

            // Override c_str() to return a unique_ptr to a C string
            template <typename C = T>
                requires CharacterType<C>
            inline constexpr std::unique_ptr<C[]> c_str(void) const noexcept
            {
                std::unique_ptr<C[]> result(new C[
                    this->size()          +
                    newline_count(*this) +
                    (this->size() > 0 ? (SAME(this->at(this->size() - 1).val(), '\0') ? 0 : 1) : 1)
                ]);
                if (this->size() < 1)
                {
                    result[0] = C('\0');
                    return result;
                }
                result[0] = CONVERT(C, this->at(0).val());
                size_t offset = 0;
                for (size_t i = 1; i < this->size(); ++i)
                {
                    auto prev = this->at(i - 1).line();
                    auto curr = this->at(i).line();
                    if (curr > prev)
                    {
                        for (size_t j = 0; j < curr - prev; ++j)
                            result[i + offset + j] = CONVERT(C, '\n');
                        offset += curr - prev;
                    }
                    result[i + offset] = CONVERT(C, this->at(i).val());
                }
                if (DIFFERENT(this->at(this->size() - 1).val(), '\0'))
                    result[this->size() + newline_count(*this)] = C('\0');
                return result;
            }
            // Override data() to return a unique_ptr to a C string
            // (same as c_str() but without the null terminator, except if the string is empty where we add it)
            template <typename C = T>
                requires CharacterType<C>
            inline constexpr std::unique_ptr<C[]> data(void) const noexcept
            {
                std::unique_ptr<C[]> result(new C[
                    this->size()          +
                    newline_count(*this) +
                    (this->size() > 0 ? 0 : 1)
                ]);
                if (this->size() < 1)
                {
                    result[0] = C('\0');
                    return result;
                }
                result[0] = CONVERT(C, this->at(0).val());
                size_t offset = 0;
                for (size_t i = 1; i < this->size(); ++i)
                {
                    auto prev = this->at(i - 1).line();
                    auto curr = this->at(i).line();
                    if (curr > prev)
                    {
                        for (size_t j = 0; j < curr - prev; ++j)
                            result[i + offset + j] = CONVERT(C, '\n');
                        offset += curr - prev;
                    }
                    result[i + offset] = CONVERT(C, this->at(i).val());
                }
                return result;
            }

        private:
            constexpr static inline size_t newline_count(const ParsedCharString& str) noexcept
            {
                return str.size() > 0 ? str.at(str.size() - 1).line() - str.at(0).line() : 0;
            }
    };

#if defined(ESC)
    #undef ESC
#endif
#define ESC "\033"

    enum class Color : uint8_t
    {
        FG_BLACK = 30,
        FG_RED = 31,
        FG_GREEN = 32,
        FG_YELLOW = 33,
        FG_BLUE = 34,
        FG_MAGENTA = 35,
        FG_CYAN = 36,
        FG_WHITE = 37,
        
        FG_BRIGHT_BLACK = 90,
        FG_BRIGHT_RED = 91,
        FG_BRIGHT_GREEN = 92,
        FG_BRIGHT_YELLOW = 93,
        FG_BRIGHT_BLUE = 94,
        FG_BRIGHT_MAGENTA = 95,
        FG_BRIGHT_CYAN = 96,
        FG_BRIGHT_WHITE = 97,

        FG_DEFAULT = 39,

    
        BG_BLACK = 40,
        BG_RED = 41,
        BG_GREEN = 42,
        BG_YELLOW = 43,
        BG_BLUE = 44,
        BG_MAGENTA = 45,
        BG_CYAN = 46,
        BG_WHITE = 47,

        BG_BRIGHT_BLACK = 100,
        BG_BRIGHT_RED = 101,
        BG_BRIGHT_GREEN = 102,
        BG_BRIGHT_YELLOW = 103,
        BG_BRIGHT_BLUE = 104,
        BG_BRIGHT_MAGENTA = 105,
        BG_BRIGHT_CYAN = 106,
        BG_BRIGHT_WHITE = 107,

        BG_DEFAULT = 49,


        FG_RESET = 0,
        BG_RESET = 0
    };

    enum class Style : uint8_t
    {
        BOLD = 1,
        DIM = 2,
        UNDERLINED = 4,
        BLINK = 5,
        REVERSE = 7,
        HIDDEN = 8,
        STRIKETHROUGH = 9,

        RESET_BOLD = 22,
        RESET_DIM = 22,
        RESET_UNDERLINED = 24,
        RESET_BLINK = 25,
        RESET_REVERSE = 27,
        RESET_HIDDEN = 28,
        RESET_STRIKETHROUGH = 29,

        RESET = 0
    };

#if defined(FG)
    #undef FG
#endif
#define FG(color) Color::FG_##color
#if defined(BG)
    #undef BG
#endif
#define BG(color) Color::BG_##color
#if defined(TXT)
    #undef TXT
#endif
#define TXT(style) Style::style

    inline std::string to_string(Color color)
    {
        return ESC + std::string("[") + std::to_string(static_cast<std::underlying_type_t<Color>>(color)) + "m";
    }

    inline std::string to_string(Style style)
    {
        return ESC + std::string("[") + std::to_string(static_cast<std::underlying_type_t<Style>>(style)) + "m";
    }

    /* For 256 color mode (correspond to <{FG|BG},{256-bit color nb}>) */
    typedef std::pair<uint8_t, uint8_t> Color256;

#if defined(FG_256)
    #undef FG_256
#endif
#define FG_256(value) std::make_pair(38, value)
#if defined(BG_256)
    #undef BG_256
#endif
#define BG_256(value) std::make_pair(48, value)

    inline std::string to_string(Color256 color)
    {
        return ESC + std::string("[") + std::to_string(color.first) + ";5;" + std::to_string(color.second) + "m";
    }

    struct TrueColor
    {
        private:
            uint8_t r;
            uint8_t g;
            uint8_t b;
            uint8_t fg_or_bg;
        public:
            TrueColor(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b), fg_or_bg(38) {}
            TrueColor(uint8_t r, uint8_t g, uint8_t b, uint8_t fg_or_bg) : r(r), g(g), b(b), fg_or_bg(fg_or_bg) {}
            ~TrueColor() = default;

            constexpr inline uint8_t get_r(void) const noexcept { return this->r; }
            constexpr inline uint8_t get_g(void) const noexcept { return this->g; }
            constexpr inline uint8_t get_b(void) const noexcept { return this->b; }
            constexpr inline uint8_t get_fg_or_bg(void) const noexcept { return this->fg_or_bg; }

            constexpr inline void set_r(uint8_t r) noexcept { this->r = r; }
            constexpr inline void set_g(uint8_t g) noexcept { this->g = g; }
            constexpr inline void set_b(uint8_t b) noexcept { this->b = b; }
            constexpr inline void set_fg_or_bg(uint8_t fg_or_bg) noexcept { this->fg_or_bg = fg_or_bg; }
            constexpr inline void set(int16_t r, int16_t g, int16_t b) noexcept
            {
                this->r = r < 0 ? this->r : r;
                this->g = g < 0 ? this->g : g;
                this->b = b < 0 ? this->b : b;
            }

            inline std::string to_string(void)
            {
                return ESC + std::string("[") + std::to_string(this->fg_or_bg) + ";2;" + std::to_string(this->r) + ";" + std::to_string(this->g) + ";" + std::to_string(this->b) + "m";
            }
    };

#if defined(FG_TRUE)
    #undef FG_TRUE
#endif
#define FG_TRUE(R, G, B) TrueColor(R, G, B)
#if defined(BG_TRUE)
    #undef BG_TRUE
#endif
#define BG_TRUE(R, G, B) TrueColor(R, G, B, 48)

    template <typename T>
    concept TextStyle =  std::same_as<Color, T> 
                      || std::same_as<Style, T> 
                      || std::same_as<Color256, T>
                      || std::same_as<TrueColor, T>;

    struct CursorMove
    {
        private:
            bool is_absolute;
            union
            {
                struct { int16_t x; int16_t y; };
                struct { int16_t col; int16_t row; };
            };
        public:
            CursorMove() : is_absolute(false), x(0), y(0) {}
            /* x and y are respectively stored at the same memory location as col and row */
            CursorMove(int16_t col, int16_t row, bool is_absolute = false) : is_absolute(is_absolute), col(col), row(row) {} 
            ~CursorMove() = default;

            constexpr inline bool get_is_absolute(void) const noexcept { return this->is_absolute; }
            constexpr inline int16_t get_x(void) const noexcept { return this->x; }
            constexpr inline int16_t get_y(void) const noexcept { return this->y; }
            constexpr inline int16_t get_col(void) const noexcept { return this->col; }
            constexpr inline int16_t get_row(void) const noexcept { return this->row; }

            constexpr inline void set_is_absolute(bool is_absolute) noexcept { this->is_absolute = is_absolute; }
            constexpr inline void set_x(int16_t x) noexcept { this->x = x; }
            constexpr inline void set_y(int16_t y) noexcept { this->y = y; }
            constexpr inline void set_col(int16_t col) noexcept { this->col = col; }
            constexpr inline void set_row(int16_t row) noexcept { this->row = row; }

            inline std::string to_string(void)
            {
                std::string result = "";
                if (this->is_absolute)
                {
                    result += ESC + std::string("[") + std::to_string(this->row) + ";" + std::to_string(this->col) + "H";
                }
                else
                {
                    if (x != 0 && x < 0)
                        result += ESC + std::string("[") + std::to_string(std::abs(this->x)) + "D";
                    else if (x != 0 && x > 0)
                        result += ESC + std::string("[") + std::to_string(std::abs(this->x)) + "C";
                    if (y != 0 && y < 0)
                        result += ESC + std::string("[") + std::to_string(std::abs(this->y)) + "A";
                    else if (y != 0 && y > 0)
                        result += ESC + std::string("[") + std::to_string(std::abs(this->y)) + "B";
                }
                return result;
            }
    };

#if defined(CURSOR_MOVE)
    #undef CURSOR_MOVE
#endif
#define CURSOR_MOVE(x, y) CursorMove(x, y)
#if defined(CURSOR_MOVE_ABS)
    #undef CURSOR_MOVE_ABS
#endif
#define CURSOR_MOVE_ABS(x, y) CursorMove(x, y, true)

    template <typename T>
    concept CursorControl = std::same_as<CursorMove, T>;


#if defined(IS_CLASS_TYPE)
    #undef IS_CLASS_TYPE
#endif
#define IS_CLASS_TYPE(type) (std::is_class_v<std::remove_cv_t<type>>)

#if defined(HAS_MEMBER)
    #undef HAS_MEMBER
#endif
#define HAS_MEMBER(obj, member) (IS_CLASS_TYPE(obj) && std::is_member_pointer<decltype(&obj::member)>::value)

    template <typename T>
    concept HasToStringMember = HAS_MEMBER(T, to_string) && requires(T t)
    {
        { t.to_string() } -> std::convertible_to<std::string>;
    };

    template <typename T>
    concept HasToStringFunction = requires(T t)
    {
        { to_string(t) } -> std::convertible_to<std::string>;
    };

    template <typename T>
    concept HasToString = HasToStringMember<T> || HasToStringFunction<T>;

    template <typename T>
    concept TermControl = (TextStyle<T> || CursorControl<T>)
                        && HasToString<T>;

    template <typename T>
        requires TermControl<T> && HasToStringMember<T>
    inline std::string operator+(const std::string& str, T style)
    {
        return str + style.to_string();
    }

    template <typename T>
        requires TermControl<T> && HasToStringFunction<T>
    inline std::string operator+(const std::string& str, T style)
    {
        return str + to_string(style);
    }

    template <typename T>
        requires TermControl<T> && HasToStringMember<T>
    inline std::string operator+(T style, const std::string& str)
    {
        return style.to_string() + str;
    }

    template <typename T>
        requires TermControl<T> && HasToStringFunction<T>
    inline std::string operator+(T style, const std::string& str)
    {
        return to_string(style) + str;
    }

    template <typename T>
        requires TermControl<T> && HasToStringMember<T>
    inline std::string& operator+=(std::string& str, T style)
    {
        return str += style.to_string();
    }

    template <typename T>
        requires TermControl<T> && HasToStringFunction<T>
    inline std::string& operator+=(std::string& str, T style)
    {
        return str += to_string(style);
    }

    template <typename T, typename U>
        requires TermControl<T> && TermControl<U> && HasToStringMember<T> && HasToStringMember<U>
    inline std::string operator+(T style1, U style2)
    {
        return style1.to_string() + style2.to_string();
    }

    template <typename T, typename U>
        requires TermControl<T> && TermControl<U> && HasToStringMember<T> && HasToStringFunction<U>
    inline std::string operator+(T style1, U style2)
    {
        return style1.to_string() + to_string(style2);
    }

    template <typename T, typename U>
        requires TermControl<T> && TermControl<U> && HasToStringMember<U> && HasToStringFunction<T>
    inline std::string operator+(T style1, U style2)
    {
        return to_string(style1) + style2.to_string();
    }

    template <typename T, typename U>
        requires TermControl<T> && TermControl<U> && HasToStringFunction<T> && HasToStringFunction<U>
    inline std::string operator+(T style1, U style2)
    {
        return to_string(style1) + to_string(style2);
    }

    /**
     * @fn std::string format_error(const std::string& error_msg, std::optional<std::string> error_type, std::optional<string_size_type<char>> line, std::optional<string_size_type<char>> col, std::optional<std::string> context)
     * @brief Format an error message
     * 
     * @param type The type of the error
     * @param error_msg The main error message
     * @param error_type Additional error type information
     * @param line Line number where the error occured, if any
     * @param col Column number where the error occured, if any
     * @param context Line context where the error occured, if any
     * @tparam T The character type of the parsed file
     * @return The formatted error message
     */
    template <typename T, typename U>
        requires CharacterType<T> && FilePath<U>
    static 
    std::string 
    format_error
    (
        ExcType type,
        const std::string& error_msg,
        std::optional<std::string> error_type,
        std::optional<U> filepath,
        std::optional<string_size_type<T>> line,
        std::optional<string_size_type<T>> col,
        std::optional<std::basic_string<T>> context,
        int err_or_warn_count
    ) noexcept
    {
        std::string result;
        if (type != ExcType::NO_ERROR)
            result = FG(BRIGHT_RED) + TXT(BOLD) + "[ ERROR n°" + std::to_string(err_or_warn_count) + " ]" + FG(DEFAULT) + (error_type.has_value() ? "  (error type: " + error_type.value() + ")" : "") + "  " + TXT(RESET) + error_msg + "\n";
        else
            result = FG(BRIGHT_MAGENTA) + TXT(BOLD) + "[ WARNING n°" + std::to_string(err_or_warn_count) + " ]" + FG(DEFAULT) + "  " + TXT(RESET) + error_msg + "\n";
        if (filepath.has_value())
        {
            result += std::string("In file ") + TXT(BOLD) + std::string(filepath.value());
            if (!line.has_value() || !col.has_value())
                result += ":\n";
            else
                result += ":" + std::to_string(line.value()) + ":" + std::to_string(col.value()) + ":\n";
            result += TXT(RESET);
        }
        else if (line.has_value() && col.has_value())
            result += "At line: " + std::to_string(line.value()) + ", column: " + std::to_string(col.value()) + ":\n";
        if (context.has_value())
        {
            if (col.has_value())
            {
                // Find the "word" in which the error occured
                string_size_type<T> start = 0;
                string_size_type<T> end = context.value().size();
                for (string_size_type<T> i = col.value() - 1; i >= 0; --i)
                {
                    if (SAME(context.value().at(i), ' ') || SAME(context.value().at(i), '\t'))
                    {
                        start = i + 1;
                        break;
                    }
                }
                for (string_size_type<T> i = col.value() - 1; i < context.value().size(); ++i)
                {
                    if (SAME(context.value().at(i), ' ') || SAME(context.value().at(i), '\t'))
                    {
                        end = i;
                        break;
                    }
                }
                result += " " + TXT(BOLD) + "|" + TXT(RESET) + "  " + Util::convert<char, T>(context.value().substr(0, start));
                assert(static_cast<long long>(end) - static_cast<long long>(start) >= 0LL);
                result += FG(BRIGHT_RED) + TXT(BOLD) + Util::convert<char, T>(context.value().substr(start, end - start)) + FG(DEFAULT) + TXT(RESET);
                if (static_cast<long long>(context.value().size()) - static_cast<long long>(end) > 0LL)
                    result += Util::convert<char, T>(context.value().substr(end, context.value().size() - end)) + "\n";
                else
                    result += "\n";
                uint16_t start_term_col = std::string(" |  ").size();
                for (string_size_type<T> i = 0; i < start; ++i)
                {
                    if (SAME(context.value().at(i), '\t'))
                        start_term_col += 4;
                    else
                        ++start_term_col;
                }
                result += TXT(RESET);
                // Move cursor position just under the error position (i.e. relative to the start of the line)
                result += CURSOR_MOVE(start_term_col, 0);
                // Add some '~' to underline the error position, and a '^' to point to the start of the """word"""
                result += FG(BRIGHT_RED) + TXT(BOLD);
                result += '^';
                for (string_size_type<T> i = start + 1; i < end; ++i)
                    result += '~';
                result += FG(DEFAULT) + TXT(RESET);
                result += '\n';
            }
            else
                result += "  | " + Util::convert<char, T>(context.value()) + "\n";
        }
        return result;
    }

#if 0
    template <typename T, typename E, typename FuncType, typename... Args>
    concept OrElseInvocable = std::conditional_t<
                                    std::bool_constant<sizeof...(Args) == 0>::value,
                                    std::bool_constant<std::invocable<FuncType, E>>,
                                    std::bool_constant<std::invocable<FuncType, E, Args...>>
                                >::value;

    template <typename T, typename E, typename FuncType, typename... Args>
    concept OrElseReturnT = std::conditional_t<
                                    std::bool_constant<sizeof...(Args) == 0>::value,
                                    std::bool_constant<std::same_as<
                                            std::remove_cvref_t<std::invoke_result_t<FuncType, E>>, T>
                                        >,
                                    std::bool_constant<std::same_as<
                                            std::remove_cvref_t<std::invoke_result_t<FuncType, E, Args...>>, T>
                                        >
                                >::value;

    template <typename T, typename E, typename FuncType, typename... Args>
    concept OrElseNoexcept = std::conditional_t<
                                    std::bool_constant<sizeof...(Args) == 0>::value,
                                    std::bool_constant<std::is_nothrow_invocable<FuncType, E>::value>,
                                    std::bool_constant<std::is_nothrow_invocable<FuncType, E, Args...>::value>
                                >::value;

    STATIC_TODO("If possible, turn all `Result` variables in supdef into C++23 `std::expected` variables");
    template <typename T, typename E>
        requires SPECIALIZATION_OF(E, Error) || SPECIALIZATION_OF(E, Exception)
    class Result : public std::variant<T, E>
    {
        private:
            bool null = false;
        public:
            typedef T ok_type;
            typedef E err_type;

            Result() : std::variant<T, E>(), null(true) {}
            Result(std::nullptr_t) : std::variant<T, E>(), null(true) {}
            template <typename U>
                requires std::convertible_to<U, std::nullptr_t>
            Result(std::initializer_list<U> list) : std::variant<T, E>(), null(true) {}
            Result(const Result&) = default;
            Result(Result&&) = default;
            Result(T& value) : std::variant<T, E>(value) {}
            Result(T&& value) : std::variant<T, E>(std::move(value)) {}
            template <typename U>
                requires std::convertible_to<U, T>
            Result(std::initializer_list<U> list) : std::variant<T, E>(static_cast<T>(*std::data(list))) {}
            Result(E& error) : std::variant<T, E>(error) {}
            Result(E&& error) : std::variant<T, E>(std::move(error)) {}
            template <typename U>
                requires std::convertible_to<U, E>
            Result(std::initializer_list<U> list) : std::variant<T, E>(static_cast<E>(*std::data(list))) {}

            ~Result() = default;

            [[__gnu__::__nonnull__]]
            Result& operator=(const Result&) = default;
            Result& operator=(Result&&) = default;
            Result& operator=(T& value) { return (*this = Result(value)); }
            Result& operator=(T&& value) { return (*this = Result(std::move(value))); }
            Result& operator=(E& error) { return (*this = Result(error)); }
            Result& operator=(E&& error) { return (*this = Result(std::move(error))); }
            Result& operator=(std::nullptr_t) { return (*this = Result(nullptr)); }

            inline bool is_null(void) const noexcept { return this->null; }
            inline bool is_ok(void) const noexcept { return !this->is_null() && std::holds_alternative<T>(*this); }
            inline bool is_err(void) const noexcept { return !this->is_null() && std::holds_alternative<E>(*this); }

#if 0
            template <typename FuncType, typename... Args>
                requires std::invocable<FuncType, E, Args...> && std::convertible_to<std::invoke_result_t<FuncType, E, Args...>, T>
            constexpr inline T unwrap_or_else(FuncType&& func, Args&&... args) const noexcept(std::is_nothrow_invocable_v<FuncType, E, Args...>)
            {
                if (this->is_ok())
                    return std::get<T>(*this);
                else
                    return std::invoke(std::forward<FuncType>(func), std::get<E>(*this), std::forward<Args>(args)...);
            }
#else
            /**
             * @brief Unwrap the value contained in the `Result<T, E>` if it is Ok, otherwise return the result of the function `func`
             * applied to the error value.
             * 
             */
            template <typename FuncType, typename... Args>
                requires OrElseInvocable<ok_type, err_type, FuncType, Args...> && OrElseReturnT<ok_type, err_type, FuncType, Args...> && (sizeof...(Args) > 0)
            constexpr inline ok_type unwrap_or_else(FuncType&& func, Args&&... args) const noexcept(OrElseNoexcept<ok_type, err_type, FuncType, Args...>)
            {
                if (this->is_null())
                    return ok_type();
                if (this->is_ok())
                    return std::get<T>(*this);
                return std::invoke(std::forward<FuncType>(func), std::get<E>(*this), std::forward<Args>(args)...);
            }

            template <typename FuncType>
                requires OrElseInvocable<ok_type, err_type, FuncType> && OrElseReturnT<ok_type, err_type, FuncType>
            constexpr inline ok_type unwrap_or_else(FuncType&& func) const noexcept(OrElseNoexcept<ok_type, err_type, FuncType>)
            {
                if (this->is_null())
                    return ok_type();
                if (this->is_ok())
                    return std::get<T>(*this);
                return std::invoke(std::forward<FuncType>(func), std::get<E>(*this));
            }


#endif

            constexpr inline T unwrap_or(T&& value) const noexcept
            {
                if (this->is_ok())
                    return (value);
                if (this->is_ok())
                    return std::get<T>(*this);
                return (value);
            }

            constexpr inline T unwrap_or(T& value) const noexcept
            {
                if (this->is_null())
                    return value;
                if (this->is_ok())
                    return std::get<T>(*this);
                return value;
            }

            constexpr inline T unwrap(void) const
            {
                if (this->is_null())
                    throw std::runtime_error("Attempt to unwrap a null Result");
                if (this->is_ok())
                    return std::get<T>(*this);
                if (this->is_err())
                    throw std::get<E>(*this);
                Util::unreachable();
            }

            /**
             * @brief Map a function to the value contained in the `Result<T, E>` if it is Ok, otherwise return `this` unchanged.
             * 
             * @tparam FuncType The type of the function to map
             * @tparam Args The arguments to forward to the function
             * @param func The function to map
             * @param args The arguments to forward to the function
             * @return Result<T, E>
             */
            template <typename FuncType, typename... Args>
                requires std::invocable<FuncType, T, Args...> && std::same_as<std::invoke_result_t<FuncType, T, Args...>, T>
            constexpr inline Result<T, E> map(FuncType&& func, Args&&... args) const noexcept(std::is_nothrow_invocable_v<FuncType, T, Args...>)
            {
                if (this->is_null())
                    return Result<T, E>();
                if (this->is_ok())
                    return Result(std::invoke(std::forward<FuncType>(func), std::get<T>(*this), std::forward<Args>(args)...));
                return Result(std::get<E>(*this));
            }
    };
#else
    template <typename T, typename E>
        requires SPECIALIZATION_OF(E, Error) || SPECIALIZATION_OF(E, Exception)
    class Result : public std::expected<
                        std::variant<T, nullptr_t>, E
                    >
    {
        private:
            using Base = std::expected<std::variant<T, nullptr_t>, E>;
        public:
            using Base::Base;
            using Base::operator=;

            using ok_type = std::variant_alternative_t<0, typename Base::value_type>;
            using err_type = typename Base::error_type;
            using null_type = std::nullptr_t;

            inline bool is_null(void) const noexcept
            {
                return this->has_value() && std::holds_alternative<null_type>(this->value());
            }

            inline bool is_ok(void) const noexcept
            {
                return this->has_value() && std::holds_alternative<ok_type>(this->value());
            }

            inline bool is_err(void) const noexcept
            {
                return !this->has_value();
            }

            T& unwrap(void) const
            {
                if (this->is_err())
                    throw InternalError("Attempt to unwrap an error Result");
                if (this->is_null())
                    throw InternalError("Attempt to unwrap a null Result");
                if (this->is_ok())
                    return std::get<ok_type>(this->value());
                UNREACHABLE();
            }
    };

    template <typename T, typename E>
        requires SPECIALIZATION_OF(E, Error) || SPECIALIZATION_OF(E, Exception)
    always_inline
    inline Result<T, E> make_null_result(void)
    {
        return Result<T, E>(std::nullptr_t());
    }

    template <typename T, typename E>
        requires SPECIALIZATION_OF(E, Error) || SPECIALIZATION_OF(E, Exception)
    always_inline
    inline Result<T, E> make_ok_result(const T& value)
    {
        return Result<T, E>(value);
    }

    template <typename T, typename E>
        requires SPECIALIZATION_OF(E, Error) || SPECIALIZATION_OF(E, Exception)
    always_inline
    inline Result<T, E> make_ok_result(T&& value)
    {
        return Result<T, E>(std::move(value));
    }

    template <typename T, typename E>
        requires SPECIALIZATION_OF(E, Error) || SPECIALIZATION_OF(E, Exception)
    always_inline
    inline Result<T, E> make_err_result(const E& error)
    {
        return Result<T, E>(std::unexpected(error));
    }

    template <typename T, typename E>
        requires SPECIALIZATION_OF(E, Error) || SPECIALIZATION_OF(E, Exception)
    always_inline
    inline Result<T, E> make_err_result(E&& error)
    {
        return Result<T, E>(std::unexpected(std::move(error)));
    }

#endif

    class ErrorPrinterBase
    {
        protected:
            static std::mutex mtx;
    };

    STATIC_TODO(
        "Rewrite ErrorPrinter implementation"
    );
    template <typename E>
        requires SPECIALIZATION_OF(E, Error)         || SPECIALIZATION_OF(E, Exception)
              || std::same_as<E, InternalError>      || std::same_as<E, InternalException>
              || std::derived_from<E, InternalError> || std::derived_from<E, InternalException>
    class ErrorPrinter : private ErrorPrinterBase
    {
        private:
            std::vector<E> errors;
        public:
            ErrorPrinter() = default;
            ErrorPrinter(const ErrorPrinter&) = default;
            ErrorPrinter(ErrorPrinter&&) = default;
            ~ErrorPrinter() = default;

            ErrorPrinter(std::initializer_list<E> list) : errors(list) {}
            ErrorPrinter(std::vector<E>& errors) : errors(errors) {}
            ErrorPrinter(E& error) : errors({error}) {}
            ErrorPrinter(E&& error) : errors({std::move(error)}) {}

            ErrorPrinter& operator=(const ErrorPrinter&) = default;
            ErrorPrinter& operator=(ErrorPrinter&&) = default;
            ErrorPrinter& operator=(std::initializer_list<E> list) { this->errors = list; return *this; }
            ErrorPrinter& operator=(std::vector<E>& errors) { this->errors = errors; return *this; }
            ErrorPrinter& operator=(E& error) { this->errors = {error}; return *this; }
            ErrorPrinter& operator=(E&& error) { this->errors = {std::move(error)}; return *this; }

            inline void add_error(E& error) { this->errors.push_back(error); }
            inline void add_error(E&& error) { this->errors.push_back(std::move(error)); }

            operator std::vector<E>() const { return this->errors; }
            ErrorPrinter& operator+(E& error) { this->errors.push_back(error); return *this; }
            ErrorPrinter& operator+(E&& error) { this->errors.push_back(std::move(error)); return *this; }

            inline void print(std::ostream& os = std::cerr) const noexcept
            {
                std::lock_guard<std::mutex> lock(ErrorPrinterBase::mtx);
                for (const E& error : this->errors)
                    error.report(os);
            }
    };

    // TODO: Fix Windows version
    inline void set_app_locale(void)
    {
        #if defined(_WIN32)
            LCID lcid = GetThreadLocale();
            wchar_t name[LOCALE_NAME_MAX_LENGTH];
            if (LCIDToLocaleName(lcid, name, LOCALE_NAME_MAX_LENGTH, 0) == 0)
                { std::cerr << "Failed to set locale to current system-locale: error code: " << GetLastError() << std::endl; }
            std::locale::global(name);
        #else
            std::locale::global(uni::detail::user_preferred_unicode_aware_locale());
        #endif
    }

    STATIC_TODO(
        "Rewrite all `split_string` functions"
    );
    template <typename T, typename U>
        requires CharacterType<T> && (std::convertible_to<U, T> || CharacterType<U>) && (!std::same_as<T, U>)
    inline
    std::vector<std::basic_string<T>>
        split_string(
            const std::basic_string<T>& str,
            const U& delimiter
        )
        noexcept(std::is_nothrow_convertible_v<U, T> && !CharacterType<U>)
    {
        std::vector<std::basic_string<T>> result{};
        std::basic_stringstream<T> ss(str);
        std::basic_string<T> token;
        T converted_delim;
        if constexpr (std::is_nothrow_convertible_v<U, T> && !CharacterType<U>)
            converted_delim = static_cast<T>(delimiter);
        else
        {
            try
            {
                if constexpr (CharacterType<U>)
                {
                    auto converted_delim_str = CONVERT(T, delimiter);
                    if (converted_delim_str.length() == 1)
                        converted_delim = converted_delim_str.at(0);
                    else
                        return split_string<T, U>(str, converted_delim_str);
                }
                else
                    converted_delim = static_cast<T>(delimiter);
            }
            catch (const std::exception& e)
            {
#if defined(__GNUC__)
                throw InternalError("Failed to convert delimiter to string character type in function " + std::string(__PRETTY_FUNCTION__) + " (caught exception: " + e.what() + ")");
#else
                throw InternalError("Failed to convert delimiter to string character type in function " + std::string(__func__) + " (caught exception: " + e.what() + ")");
#endif
                UNREACHABLE();
                return result;
            }
        
        }
        while (std::getline(ss, token, converted_delim))
            result.push_back(token);
        return result;
    }

    template <typename T, typename U>
        requires CharacterType<T> && std::same_as<T, U>
    inline std::vector<std::basic_string<T>> split_string(const std::basic_string<T>& str, const U& delimiter) noexcept
    {
        std::vector<std::basic_string<T>> result{};
        std::basic_stringstream<T> ss(str);
        std::basic_string<T> token;
        while (std::getline(ss, token, delimiter))
            result.push_back(token);
        return result;
    }

    template <typename T, typename U>
        requires CharacterType<T> && CharacterType<U>
    inline std::vector<std::basic_string<T>> split_string(const std::basic_string<T>& str, const std::basic_string<U>& delim)
    {
        if (CONVERT(T, delim).length() == 1)
            return split_string<T, U>(str, delim.at(0));

        std::vector<std::basic_string<T>> result{};
        std::basic_string<T> converted_delim;
        if constexpr (std::same_as<T, U>)
            converted_delim = delim;
        else
        {
            try
            {
                converted_delim = CONVERT(T, delim);
            }
            catch (const std::exception& e)
            {
#if defined(__GNUC__)
                throw InternalError("Failed to convert delimiter to string character type in function " + std::string(__PRETTY_FUNCTION__) + " (caught exception: " + e.what() + ")");
#else
                throw InternalError("Failed to convert delimiter to string character type in function " + std::string(__func__) + " (caught exception: " + e.what() + ")");
#endif
                UNREACHABLE();
            }
        }
        std::remove_cv_t<decltype(std::basic_string<T>::npos)> pos = 0, prev_pos = 0;

        while ((pos = str.find(converted_delim, prev_pos)) != std::basic_string<T>::npos)
        {
            result.push_back(str.substr(prev_pos, pos - prev_pos));
            prev_pos = pos + converted_delim.length();
        }
        result.push_back(str.substr(prev_pos, pos - prev_pos));

        return result;
    }
    
    /**
     * @brief Utility concept for coroutines
     * 
     * @tparam T The type to check if it is a coroutine
     */
    template <typename T>
    concept Suspendable = requires(T t)
    {
        { t.resume() };
    };

    template <typename T>
    concept StdSuspend = std::same_as<std::suspend_always, T> || std::same_as<std::suspend_never, T>;

    template <typename YieldType, typename ReturnType = YieldType, typename CoroPolicies = void, typename... CoroArgs>
    class Coroutine
    {};

    /**
     * @brief This class is meant to be the type of parsing coroutines so they can suspend themselves while indicating potential errors to the "master" function calling them.
     */
    template <typename T>
    class Coro
    {
        private:
            // To handle the case where using the iterator with coroutine which co_return a value
            // so we can use the last co_returned value
            bool iter_end = false;
        public:
            class Promise
            {
                public:
                    using promise_type = Promise;
                    using handle_type = std::coroutine_handle<promise_type>;
                private:
                    T value;
                    bool done;
                public:
                    inline Coro<T> get_return_object()
                    {
                        return Coro<T>(handle_type::from_promise(*this));
                    }
                    [[noreturn]]
                    static inline Coro<T> get_return_object_on_allocation_failure()
                    {
                        throw InternalError("Failed to allocate memory for return object of coroutine");
                    }
                    constexpr inline std::suspend_always initial_suspend() noexcept { return {}; }
                    constexpr inline std::suspend_always final_suspend() noexcept { return {}; }
                    [[noreturn]] inline void unhandled_exception() { std::rethrow_exception(std::current_exception()); }
                    inline void return_value(T value) 
                    {
                        this->value = value;
                        this->done = true;
                    }
                    inline T get_value() { return this->value; }
                    inline std::suspend_always yield_value(T value)
                    {
                        this->value = value;
                        return {};
                    }
            };
            using promise_type = Promise;
            using handle_type = std::coroutine_handle<promise_type>;
            Coro(handle_type handle) : handle(handle), done(false) {}
            Coro(Coro&& other) : handle(std::exchange(other.handle, nullptr)), done(other.done) {}
            Coro(const Coro&) = delete;
            ~Coro() 
            {
                if (this->handle)
                    this->handle.destroy();
            }
            Coro& operator=(Coro&& other) 
            {
                this->handle = std::exchange(other.handle, nullptr);
                this->done = other.done;
                return *this;
            }
            Coro& operator=(const Coro&) = delete;
            inline bool resume() 
            {
                if (this->done && this->handle.done())
                    return false;
                this->handle.resume();
                this->done = this->handle.done();
                return true;
            }
            inline T get_value() { return this->handle.promise().get_value(); }
            inline bool is_done() { return this->done && this->handle.done(); }
            inline bool is_iter_end() 
            {
                if (this->iter_end)
                    return true;
                bool old_val = this->iter_end;
                if (this->is_done() && !this->iter_end)
                    this->iter_end = true;
                return old_val;
            }
            inline operator bool() { return !this->is_done(); }

            struct end_iterator : public std::default_sentinel_t {};

            class Iterator
            {
                public:
                    using iterator_category = std::input_iterator_tag;
                    using iterator_concept = std::input_iterator_tag;
                    using value_type = T;
                    using difference_type = std::ptrdiff_t;
                    using pointer = T*;
                    using reference = T&;
                private:
                    Coro<T>* coro;
                public:
                    Iterator() : coro(nullptr) {}
                    Iterator(Coro<T>* coro) : coro(coro) {}
                    Iterator(const Iterator&) = default;
                    Iterator(Iterator&&) = default;
                    ~Iterator() = default;
                    Iterator& operator=(const Iterator&) = default;
                    Iterator& operator=(Iterator&&) = default;

                    inline bool operator==(end_iterator) const noexcept
                    {
                        return this->coro ? this->coro->is_iter_end() : true;
                    }

                    inline Iterator& operator++() noexcept
                    {
                        if (this->coro && this->coro->handle)
                            this->coro->resume();
                        return *this;
                    }

                    inline Iterator operator++(int) noexcept
                    {
                        Iterator tmp = *this;
                        ++(*this);
                        return tmp;
                    }
                    
                    inline value_type operator*() const noexcept
                    {
                        if (!this->coro || !this->coro->handle)
                            return {};
                        return this->coro->get_value();                        
                    }
                    
                    inline value_type operator->() const noexcept
                    {
                        if (!this->coro || !this->coro->handle)
                            return {};
                        return this->coro->get_value();
                    }
            };

            typedef Iterator iterator;

            inline iterator begin() noexcept
            {
                iterator it(this);
                return ++it;
            }

            inline end_iterator end() noexcept
            {
                return end_iterator{};
            }

        private:
            handle_type handle;
            bool done;
    };

    static_assert(std::input_iterator<Coro<int>::iterator>);

    template <typename T>
    concept IsCoro = SPECIALIZATION_OF(T, Coro);

    template <typename ExceptionType>
    concept IsHandlableException = SPECIALIZATION_OF(ExceptionType, ::SupDef::Exception)
                                || SPECIALIZATION_OF(ExceptionType, ::SupDef::Error)
                                || std::is_base_of_v<std::exception, ExceptionType>;

    template <typename ExceptionType>
    using IsHandlableExceptionPredicate = std::bool_constant<IsHandlableException<ExceptionType>>;

    // TODO: Implement a coroutine wrapper so coroutine results can be stored in a std::future, and launched in a thread pool
    template <typename...>
    class PackagedCoro;

    struct TimedTaskBase
    {
        struct TimedOut : public InternalException
        {
            TimedOut() : InternalException("Timed out while waiting for task to complete", false, false)
            {
                this->trace = CURRENT_STACKTRACE(1);
                this->init_msg();
            }
        };
    };

    template <
        typename Rep, typename Period,
        typename FuncType
    >
    struct TimedTask<std::chrono::duration<Rep, Period>, FuncType> : public TimedTaskBase
    {
        private:
            static constexpr decltype(auto) to_add = std::chrono::microseconds(100);
            using DurationType = decltype(
                                    std::declval<std::chrono::duration<Rep, Period>>()
                                  + std::declval<decltype(to_add)>()
                                );
            template <typename... Args>
            using ReturnType = std::invoke_result_t<FuncType, Args...>;

            DurationType duration;
            FuncType func;

            template <typename... Args>
                requires std::invocable<FuncType, Args...>
            symbol_hot
            ReturnType<Args...> wrap_func(Args&&... args)
            {
                std::mutex mtx{};
                std::condition_variable cv{};
                bool done = false;
                ReturnType<Args...> result;

                std::thread t([&mtx, &cv, &done, &result, this](auto&&... args) -> void
                {
                    result = std::invoke(this->func, std::forward<Args>(args)...);

                    std::unique_lock<std::mutex> lock(mtx);
                    done = true;
                    lock.unlock();
                    
                    cv.notify_one();
                }, std::forward<Args>(args)...);
                
                {
                    std::unique_lock<std::mutex> lock(mtx);
                    if (!cv.wait_for(lock, this->duration, [&done]{ return done; }))
                    {
                        hard_assert(!done);
                        t.detach();
                        throw TimedOut();
                        UNREACHABLE();
                    }
                }
                
                t.join();
                return result;
            }

        public:
            TimedTask(std::chrono::duration<Rep, Period>&& duration, FuncType&& func)
                : duration(std::forward<std::chrono::duration<Rep, Period>>(duration) + to_add),
                  func(std::forward<FuncType>(func))
            {}

            TimedTask(const TimedTask&) = default;
            TimedTask(TimedTask&&) = default;
            ~TimedTask() = default;

            TimedTask& operator=(const TimedTask&) = default;
            TimedTask& operator=(TimedTask&&) = default;

            symbol_cold
            inline auto get_duration(void) const noexcept { return this->duration; }

            symbol_cold
            inline auto get_func(void) const noexcept { return this->func; }

            template <typename... Args>
                requires std::invocable<FuncType, Args...>
            symbol_hot
            inline ReturnType<Args...> operator()(Args&&... args)
            {
                return this->wrap_func(std::forward<Args>(args)...);
            }
    };

    template <typename Floating, typename FuncType>
        requires std::floating_point<std::remove_cvref_t<Floating>> && std::negation_v<std::is_reference<Floating>>
    struct TimedTask<Floating, FuncType> : public TimedTaskBase
    {
        private:
            static constexpr decltype(auto) to_add = std::chrono::microseconds(100);
            using DurationType = decltype(
                                    std::declval<std::chrono::duration<Floating, std::micro>>()
                                  + std::declval<decltype(to_add)>()
                                );
            template <typename... Args>
            using ReturnType = std::invoke_result_t<FuncType, Args...>;

            DurationType duration;
            FuncType func;

            template <typename... Args>
                requires std::invocable<FuncType, Args...>
            symbol_hot
            ReturnType<Args...> wrap_func(Args&&... args)
            {
                std::mutex mtx{};
                std::condition_variable cv{};
                bool done = false;
                ReturnType<Args...> result;

                std::thread t([&mtx, &cv, &done, &result, this](auto&&... args) -> void
                {
                    result = std::invoke(this->func, std::forward<Args>(args)...);

                    std::unique_lock<std::mutex> lock(mtx);
                    done = true;
                    lock.unlock();
                    
                    cv.notify_one();
                }, std::forward<Args>(args)...);
                
                {
                    std::unique_lock<std::mutex> lock(mtx);
                    if (!cv.wait_for(lock, this->duration, [&done]{ return done; }))
                    {
                        hard_assert(!done);
                        t.detach();
                        throw TimedOut();
                        UNREACHABLE();
                    }
                }
                
                t.join();
                return result;
            }

            symbol_unused
            static consteval intmax_t get_max_10th_power(void) noexcept
            {
                intmax_t max = 1;
                for (intmax_t i = 0; i < std::numeric_limits<intmax_t>::digits10; ++i)
                    max *= ::SupDef::Util::saturated_mul<intmax_t>(max, 10);
                hard_assert(max != std::numeric_limits<intmax_t>::max());
                hard_assert(::SupDef::Util::saturated_mul<intmax_t>(max, 10) == std::numeric_limits<intmax_t>::max());
                return max;
            }
        public:
            /*
             * In all constructors, treat the duration as microseconds
             */
            TimedTask(Floating&& duration, FuncType&& func)
                : duration(std::chrono::duration<Floating, std::micro>(std::forward<Floating>(duration)) + to_add),
                  func(std::forward<FuncType>(func))
            {}

            TimedTask(const TimedTask&) = default;
            TimedTask(TimedTask&&) = default;
            ~TimedTask() = default;

            TimedTask& operator=(const TimedTask&) = default;
            TimedTask& operator=(TimedTask&&) = default;

            symbol_cold
            inline auto get_duration(void) const noexcept { return this->duration; }

            symbol_cold
            inline auto get_func(void) const noexcept { return this->func; }

            template <typename... Args>
                requires std::invocable<FuncType, Args...>
            symbol_hot
            inline ReturnType<Args...> operator()(Args&&... args)
            {
                return this->wrap_func(std::forward<Args>(args)...);
            }
    };

    // Deduction guide
    template <typename Rep, typename Period, typename FuncType>
    TimedTask(std::chrono::duration<Rep, Period>&&, FuncType&&) -> TimedTask<std::chrono::duration<Rep, Period>, FuncType>;

    namespace ThreadPoolUtil
    {
        template <typename TP>
        class ThreadPoolAccessor
        {
            private:
                TP* const tp;
            public:
                ThreadPoolAccessor(TP* const tp) : tp(tp) {}
                ThreadPoolAccessor(const ThreadPoolAccessor&) = delete;
                ThreadPoolAccessor(ThreadPoolAccessor&&) = delete;
                ~ThreadPoolAccessor() = default;

                ThreadPoolAccessor& operator=(const ThreadPoolAccessor&) = delete;
                ThreadPoolAccessor& operator=(ThreadPoolAccessor&&) = delete;

                template <uint8_t AccessType, typename T>
                    requires std::negation_v<std::is_reference<T>>
                inline void get_access(const T& resource) const
                {
                    this->tp->template get_access<AccessType>(resource);
                }

                template <uint8_t AccessType, typename T>
                    requires std::negation_v<std::is_reference<T>>
                inline bool try_get_access(const T& resource) const
                {
                    return this->tp->template try_get_access<AccessType>(resource);
                }

                template <uint8_t AccessType, typename T>
                    requires std::negation_v<std::is_reference<T>>
                inline void release_access(const T& resource) const
                {
                    this->tp->template release_access<AccessType>(resource);
                }
        };
        template <typename TP>
        ThreadPoolAccessor(TP*) -> ThreadPoolAccessor<TP>;

        template <typename TP, typename T>
            requires std::negation_v<std::is_reference<T>>
        class LockGuard_READ
        {
            private:
                static constexpr uint8_t AccessType = ::SupDef::Util::RecursiveSharedMutex::READ;
            public:
                LockGuard_READ(TP* const tp, const T& resource)
                    : locked(false), tp(tp), resource(std::addressof(resource))
                {
                    this->lock();
                }
                LockGuard_READ(TP* const tp, const T& resource, std::defer_lock_t)
                    : locked(false), tp(tp), resource(std::addressof(resource))
                {}
                LockGuard_READ(TP* const tp, const T& resource, std::try_to_lock_t)
                    : locked(false), tp(tp), resource(std::addressof(resource))
                {
                    this->locked = this->try_lock();
                }
                LockGuard_READ(TP* const tp, const T& resource, std::adopt_lock_t)
                    : locked(false), tp(tp), resource(std::addressof(resource))
                {
                    this->locked = true;
                }
                        
                LockGuard_READ(const LockGuard_READ&) = delete;
                LockGuard_READ(LockGuard_READ&&) = delete;

                ~LockGuard_READ()
                {
                    this->unlock();
                }

                inline void lock(void) const
                {
                    unlikely_if (this->locked)
                        return;
                    ThreadPoolAccessor(tp).template get_access<AccessType>(this->resource);
                    this->locked = true;
                }

                inline bool try_lock(void) const
                {
                    unlikely_if (this->locked)
                        return true;
                    if (ThreadPoolAccessor(tp).template try_get_access<AccessType>(this->resource))
                    {
                        this->locked = true;
                        return true;
                    }
                    return false;
                }

                inline void unlock(void) const
                {
                    unlikely_unless (this->locked)
                        return;
                    ThreadPoolAccessor(tp).template release_access<AccessType>(this->resource);
                    this->locked = false;
                }
            private:
                mutable bool locked;
                TP* const tp;
                const T* const resource;
        };

        template <typename TP, typename T>
            requires std::negation_v<std::is_reference<T>>
        class LockGuard_WRITE
        {
            private:
                static constexpr uint8_t AccessType = ::SupDef::Util::RecursiveSharedMutex::WRITE;
            public:
                LockGuard_WRITE(TP* const tp, const T& resource)
                    : locked(false), tp(tp), resource(std::addressof(resource))
                {
                    this->lock();
                }
                LockGuard_WRITE(TP* const tp, const T& resource, std::defer_lock_t)
                    : locked(false), tp(tp), resource(std::addressof(resource))
                {}
                LockGuard_WRITE(TP* const tp, const T& resource, std::try_to_lock_t)
                    : locked(false), tp(tp), resource(std::addressof(resource))
                {
                    this->locked = this->try_lock();
                }
                LockGuard_WRITE(TP* const tp, const T& resource, std::adopt_lock_t)
                    : locked(false), tp(tp), resource(std::addressof(resource))
                {
                    this->locked = true;
                }
                        
                LockGuard_WRITE(const LockGuard_WRITE&) = delete;
                LockGuard_WRITE(LockGuard_WRITE&&) = delete;

                ~LockGuard_WRITE()
                {
                    this->unlock();
                }

                inline void lock(void) const
                {
                    unlikely_if (this->locked)
                        return;
                    ThreadPoolAccessor(tp).template get_access<AccessType>(this->resource);
                    this->locked = true;
                }

                inline bool try_lock(void) const
                {
                    unlikely_if (this->locked)
                        return true;
                    if (ThreadPoolAccessor(tp).template try_get_access<AccessType>(this->resource))
                    {
                        this->locked = true;
                        return true;
                    }
                    return false;
                }

                inline void unlock(void) const
                {
                    unlikely_unless (this->locked)
                        return;
                    ThreadPoolAccessor(tp).template release_access<AccessType>(this->resource);
                    this->locked = false;
                }
            private:
                mutable bool locked;
                TP* const tp;
                const T* const resource;
        };

        template <typename TP, typename T>
            requires std::negation_v<std::is_reference<T>>
        class LockGuard_READ_WRITE
        {
            private:
                static constexpr uint8_t AccessType = ::SupDef::Util::RecursiveSharedMutex::READ_WRITE;
            public:
                LockGuard_READ_WRITE(TP* const tp, const T& resource)
                    : locked(false), tp(tp), resource(std::addressof(resource))
                {
                    this->lock();
                }
                LockGuard_READ_WRITE(TP* const tp, const T& resource, std::defer_lock_t)
                    : locked(false), tp(tp), resource(std::addressof(resource))
                {}
                LockGuard_READ_WRITE(TP* const tp, const T& resource, std::try_to_lock_t)
                    : locked(false), tp(tp), resource(std::addressof(resource))
                {
                    this->locked = this->try_lock();
                }
                LockGuard_READ_WRITE(TP* const tp, const T& resource, std::adopt_lock_t)
                    : locked(false), tp(tp), resource(std::addressof(resource))
                {
                    this->locked = true;
                }
                        
                LockGuard_READ_WRITE(const LockGuard_READ_WRITE&) = delete;
                LockGuard_READ_WRITE(LockGuard_READ_WRITE&&) = delete;

                ~LockGuard_READ_WRITE()
                {
                    this->unlock();
                }

                inline void lock(void) const
                {
                    unlikely_if (this->locked)
                        return;
                    ThreadPoolAccessor(tp).template get_access<AccessType>(this->resource);
                    this->locked = true;
                }

                inline bool try_lock(void) const
                {
                    unlikely_if (this->locked)
                        return true;
                    if (ThreadPoolAccessor(tp).template try_get_access<AccessType>(this->resource))
                    {
                        this->locked = true;
                        return true;
                    }
                    return false;
                }

                inline void unlock(void) const
                {
                    unlikely_unless (this->locked)
                        return;
                    ThreadPoolAccessor(tp).template release_access<AccessType>(this->resource);
                    this->locked = false;
                }
            private:
                mutable bool locked;
                TP* const tp;
                const T* const resource;
        };

        template <typename TP, typename T>
            requires std::negation_v<std::is_reference<T>>
        LockGuard_READ(TP*, const T&) -> LockGuard_READ<TP, T>;
        template <typename TP, typename T>
            requires std::negation_v<std::is_reference<T>>
        LockGuard_READ(TP*, const T&, std::defer_lock_t) -> LockGuard_READ<TP, T>;
        template <typename TP, typename T>
            requires std::negation_v<std::is_reference<T>>
        LockGuard_READ(TP*, const T&, std::try_to_lock_t) -> LockGuard_READ<TP, T>;
        template <typename TP, typename T>
            requires std::negation_v<std::is_reference<T>>
        LockGuard_READ(TP*, const T&, std::adopt_lock_t) -> LockGuard_READ<TP, T>;

        template <typename TP, typename T>
            requires std::negation_v<std::is_reference<T>>
        LockGuard_WRITE(TP*, const T&) -> LockGuard_WRITE<TP, T>;
        template <typename TP, typename T>
            requires std::negation_v<std::is_reference<T>>
        LockGuard_WRITE(TP*, const T&, std::defer_lock_t) -> LockGuard_WRITE<TP, T>;
        template <typename TP, typename T>
            requires std::negation_v<std::is_reference<T>>
        LockGuard_WRITE(TP*, const T&, std::try_to_lock_t) -> LockGuard_WRITE<TP, T>;
        template <typename TP, typename T>
            requires std::negation_v<std::is_reference<T>>
        LockGuard_WRITE(TP*, const T&, std::adopt_lock_t) -> LockGuard_WRITE<TP, T>;

        template <typename TP, typename T>
            requires std::negation_v<std::is_reference<T>>
        LockGuard_READ_WRITE(TP*, const T&) -> LockGuard_READ_WRITE<TP, T>;
        template <typename TP, typename T>
            requires std::negation_v<std::is_reference<T>>
        LockGuard_READ_WRITE(TP*, const T&, std::defer_lock_t) -> LockGuard_READ_WRITE<TP, T>;
        template <typename TP, typename T>
            requires std::negation_v<std::is_reference<T>>
        LockGuard_READ_WRITE(TP*, const T&, std::try_to_lock_t) -> LockGuard_READ_WRITE<TP, T>;
        template <typename TP, typename T>
            requires std::negation_v<std::is_reference<T>>
        LockGuard_READ_WRITE(TP*, const T&, std::adopt_lock_t) -> LockGuard_READ_WRITE<TP, T>;


#undef LOCK_GUARD
#define LOCK_GUARD(LOCK_TYPE) PP_CAT(::SupDef::ThreadPoolUtil::LockGuard_, LOCK_TYPE)

    }

    template <typename ThreadPoolType, typename ThreadPoolRequiredAliases>
    concept ThreadPoolBaseRequireClause = requires {
        typename ThreadPoolRequiredAliases::task_queue_t;
        typename ThreadPoolRequiredAliases::task_queue_t::value_type;
        typename ThreadPoolRequiredAliases::task_queue_t::StopRequired;
        typename ThreadPoolRequiredAliases::function_type;
    };

    template <typename ThreadPoolType, typename ThreadPoolRequiredAliases>
        requires ThreadPoolBaseRequireClause<ThreadPoolType, ThreadPoolRequiredAliases>
    class ThreadPoolBase
    {
        friend class ThreadPoolUtil::ThreadPoolAccessor<ThreadPoolType>;
        friend class ThreadPoolUtil::ThreadPoolAccessor<const ThreadPoolType>;
        friend class ThreadPoolUtil::ThreadPoolAccessor<volatile ThreadPoolType>;
        friend class ThreadPoolUtil::ThreadPoolAccessor<const volatile ThreadPoolType>;

        friend class ThreadPoolUtil::ThreadPoolAccessor<ThreadPoolBase>;
        friend class ThreadPoolUtil::ThreadPoolAccessor<const ThreadPoolBase>;
        friend class ThreadPoolUtil::ThreadPoolAccessor<volatile ThreadPoolBase>;
        friend class ThreadPoolUtil::ThreadPoolAccessor<const volatile ThreadPoolBase>;

        public:
            using task_queue_t = ThreadPoolRequiredAliases::task_queue_t;
            using function_type = ThreadPoolRequiredAliases::function_type;
            
        private:
            using ThisType = ThreadPoolBase<ThreadPoolType, ThreadPoolRequiredAliases>;
            template <typename T>
                requires std::negation_v<std::is_reference<T>>
            RecursiveSharedMutex& get_mtx(const T& resource);
            template <typename T>
                requires std::negation_v<std::is_reference<T>>
            const RecursiveSharedMutex& get_mtx(const T& resource) const;

        public:
            using ::SupDef::Util::RecursiveSharedMutex::READ;
            using ::SupDef::Util::RecursiveSharedMutex::WRITE;
            using ::SupDef::Util::RecursiveSharedMutex::READ_WRITE;

            /**
             * @brief Get access to a resource
             * 
             * @tparam AccessType Either `READ`, `WRITE` or `READ_WRITE`
             * @tparam T The type of the resource
             * @param resource The resource to get access to
             * 
             * @example get_access<READ>(this->task_queues);
             */
            template <uint8_t AccessType, typename T>
                requires std::negation_v<std::is_reference<T>>
            inline void get_access(const T& resource) const;

            template <uint8_t AccessType, typename T>
            inline void get_access(const T* const resource) const
            {
                this->get_access<AccessType>(*resource);
            }

            template <uint8_t AccessType, typename T>
                requires std::negation_v<std::is_reference<T>>
            inline bool try_get_access(const T& resource) const;
            
            template <uint8_t AccessType, typename T>
            inline bool try_get_access(const T* const resource) const
            {
                return this->try_get_access<AccessType>(*resource);
            }

            template <uint8_t AccessType, typename T>
                requires std::negation_v<std::is_reference<T>>
            inline void release_access(const T& resource) const;

            template <uint8_t AccessType, typename T>
            inline void release_access(const T* const resource) const
            {
                this->release_access<AccessType>(*resource);
            }

        private:
            using get_next_task_return_type = task_queue_t::value_type;
            
            get_next_task_return_type get_next_task(std::stop_token stoken);

        protected:
            void thread_main(std::stop_token stoken, size_t index);

            template <typename T>
            struct MovableAtomic : public std::atomic<T>
            {
                MovableAtomic() = default;
                MovableAtomic(MovableAtomic&& other) : std::atomic<T>(other.load()) { other.store(T()); }
                MovableAtomic& operator=(MovableAtomic&& other) { this->exchange(other.load()); other.store(T()); return *this; }

                // Import all std::atomic<T> constructors and assignment operators
                using std::atomic<T>::atomic;
                using std::atomic<T>::operator=;
            };

            std::vector< std::pair< std::jthread, MovableAtomic<bool> > > threads;
            std::unordered_map< std::jthread::id, task_queue_t > task_queues;
            // In which queue is the thread waiting for a task
            std::unordered_map< std::jthread::id, std::jthread::id > waiting_locations;

        private:
            RecursiveSharedMutex threads_mtx;
            RecursiveSharedMutex task_queues_mtx;
            RecursiveSharedMutex waiting_locations_mtx;

#ifdef DECLARE_FRIENDS3
    PUSH_MACRO(DECLARE_FRIENDS3)
#endif
#undef DECLARE_FRIENDS3

#ifdef DECLARE_FRIENDS2
    PUSH_MACRO(DECLARE_FRIENDS2)
#endif
#undef DECLARE_FRIENDS2

#ifdef DECLARE_FRIENDS
    PUSH_MACRO(DECLARE_FRIENDS)
#endif
#undef DECLARE_FRIENDS

#define DECLARE_FRIENDS3(mode, member, t1, t2)      \
    friend class ThreadPoolUtil::LockGuard_##mode < \
        t1,                                         \
        std::remove_cvref_t<                        \
            decltype(                               \
                std::declval<t2>().member           \
            )                                       \
        >                                           \
    >;                                              \
    friend class ThreadPoolUtil::LockGuard_##mode < \
        const t1,                                   \
        std::remove_cvref_t<                        \
            decltype(                               \
                std::declval<t2>().member           \
            )                                       \
        >                                           \
    >;                                              \
    friend class ThreadPoolUtil::LockGuard_##mode < \
        volatile t1,                                \
        std::remove_cvref_t<                        \
            decltype(                               \
                std::declval<t2>().member           \
            )                                       \
        >                                           \
    >;                                              \
    friend class ThreadPoolUtil::LockGuard_##mode < \
        const volatile t1,                          \
        std::remove_cvref_t<                        \
            decltype(                               \
                std::declval<t2>().member           \
            )                                       \
        >                                           \
    >
#define DECLARE_FRIENDS2(mode, member)              \
    DECLARE_FRIENDS3(                               \
        mode,                                       \
        member,                                     \
        ThreadPoolType,                             \
        ThisType                                    \
    );                                              \
    DECLARE_FRIENDS3(                               \
        mode,                                       \
        member,                                     \
        ThisType,                                   \
        ThisType                                    \
    );
#define DECLARE_FRIENDS(mode)                       \
    DECLARE_FRIENDS2(mode, threads);                \
    DECLARE_FRIENDS2(mode, task_queues);            \
    DECLARE_FRIENDS2(mode, waiting_locations)

            DECLARE_FRIENDS(READ);
            DECLARE_FRIENDS(WRITE);
            DECLARE_FRIENDS(READ_WRITE);

#undef DECLARE_FRIENDS
#undef DECLARE_FRIENDS2
#undef DECLARE_FRIENDS3
POP_MACRO(DECLARE_FRIENDS)
POP_MACRO(DECLARE_FRIENDS2)
POP_MACRO(DECLARE_FRIENDS3)
    };

    struct ThreadPoolAliases
    {
#if 1
        using function_type = std::function<void()>;
#else
        using function_type = std::move_only_function<void()>;
#endif
        template <typename T>
        using queue_type = ::SupDef::Util::ThreadSafeQueue<T>;

        typedef queue_type<function_type> task_queue_t;
    };

    // TO BE TESTED
    STATIC_TODO(
        "Test the ThreadPool class and potentially modify it to use more TSS / TLS to avoid too much mutex locking "
        "(look at Boost.Thread for ideas)"
    );
    class ThreadPool
    : private ThreadPoolBase<ThreadPool, ThreadPoolAliases>
    {
        STATIC_TODO(
            "Make it possible to know if there is still tasks running in the thread pool (or in the queues)"
        );
        friend class ThreadPoolBase<ThreadPool, ThreadPoolAliases>;
        friend class ThreadPoolUtil::ThreadPoolAccessor<ThreadPool>;
        friend class ThreadPoolUtil::ThreadPoolAccessor<const ThreadPool>;
        friend class ThreadPoolUtil::ThreadPoolAccessor<volatile ThreadPool>;
        friend class ThreadPoolUtil::ThreadPoolAccessor<const volatile ThreadPool>;

#ifdef DECLARE_FRIENDS3
    PUSH_MACRO(DECLARE_FRIENDS3)
#endif
#undef DECLARE_FRIENDS3

#ifdef DECLARE_FRIENDS2
    PUSH_MACRO(DECLARE_FRIENDS2)
#endif
#undef DECLARE_FRIENDS2

#ifdef DECLARE_FRIENDS
    PUSH_MACRO(DECLARE_FRIENDS)
#endif
#undef DECLARE_FRIENDS

#define DECLARE_FRIENDS3(mode, member, t1, t2)      \
    friend class ThreadPoolUtil::LockGuard_##mode < \
        t1,                                         \
        std::remove_cvref_t<                        \
            decltype(                               \
                std::declval<t2>().member           \
            )                                       \
        >                                           \
    >;                                              \
    friend class ThreadPoolUtil::LockGuard_##mode < \
        const t1,                                   \
        std::remove_cvref_t<                        \
            decltype(                               \
                std::declval<t2>().member           \
            )                                       \
        >                                           \
    >;                                              \
    friend class ThreadPoolUtil::LockGuard_##mode < \
        volatile t1,                                \
        std::remove_cvref_t<                        \
            decltype(                               \
                std::declval<t2>().member           \
            )                                       \
        >                                           \
    >;                                              \
    friend class ThreadPoolUtil::LockGuard_##mode < \
        const volatile t1,                          \
        std::remove_cvref_t<                        \
            decltype(                               \
                std::declval<t2>().member           \
            )                                       \
        >                                           \
    >
#define DECLARE_FRIENDS2(mode, member)              \
    DECLARE_FRIENDS3(                               \
        mode,                                       \
        member,                                     \
        ThreadPool,                                 \
        ThreadPool                                  \
    )
#define DECLARE_FRIENDS(mode)                       \
    DECLARE_FRIENDS2(mode, threads);                \
    DECLARE_FRIENDS2(mode, task_queues);            \
    DECLARE_FRIENDS2(mode, waiting_locations)

        DECLARE_FRIENDS(READ);
        DECLARE_FRIENDS(WRITE);
        DECLARE_FRIENDS(READ_WRITE);

#undef DECLARE_FRIENDS
#undef DECLARE_FRIENDS2
#undef DECLARE_FRIENDS3
POP_MACRO(DECLARE_FRIENDS)
POP_MACRO(DECLARE_FRIENDS2)
POP_MACRO(DECLARE_FRIENDS3)
        
        private:
            using BaseType1 = ThreadPoolAliases;
            using BaseType2 = ThreadPoolBase<ThreadPool, ThreadPoolAliases>;

            std::jthread* get_thread_from_id(std::jthread::id&& id);
            std::jthread* get_thread_from_id(const std::jthread::id& id);
            std::jthread::id get_most_busy_thread(void);
            std::jthread::id get_least_busy_thread(void);

            void request_thread_stop(std::jthread::id&& id);
            void request_thread_stop(const std::jthread::id& id);
            void request_thread_stop(size_t index);

        public:
            using TaskTimeoutError = typename TimedTaskBase::TimedOut;

            void wrap_thread_main(std::stop_token stoken, size_t index);

            explicit ThreadPool(const size_t nb_threads = std::jthread::hardware_concurrency());
            ThreadPool(const ThreadPool&) = delete;
            ThreadPool(ThreadPool&&) = delete;
            ~ThreadPool();

            ThreadPool& operator=(const ThreadPool&) = delete;
            ThreadPool& operator=(ThreadPool&&) = delete;

            // TODO
            void add_threads(size_t nb_threads);
            // TODO
            void remove_threads(size_t nb_threads);
            // TODO
            bool try_remove_threads(size_t nb_threads);
            size_t size(void) const noexcept;

            template <
                typename FuncType, typename... Args,
                typename ReturnType = std::invoke_result_t<FuncType&&, Args&&...>
            >
                requires std::invocable<FuncType, Args...>
                    && (!IsCoro<ReturnType>)
                    && (std::is_copy_constructible_v<std::remove_reference_t<Args>> && ...)
            std::future<ReturnType> enqueue(FuncType&& func, Args&&... args);

            template <
                typename Rep, typename Period,
                typename FuncType, typename... Args,
                typename ReturnType = std::invoke_result_t<FuncType&&, Args&&...>
            >
                requires std::invocable<FuncType, Args...> 
                    && (!IsCoro<ReturnType>)
                    && (std::is_copy_constructible_v<std::remove_reference_t<Args>> && ...)
            std::future<ReturnType> enqueue(std::chrono::duration<Rep, Period>&& timeout, FuncType&& func, Args&&... args);
    };


#undef NEED_ThreadPool_TEMPLATES
#define NEED_ThreadPool_TEMPLATES 1
#include <sup_def/common/thread_pool.cpp>

    class TmpFile
    {
        private:
            static size_t get_counter(void)
            {
                static size_t counter = 0;
                return counter++;
            }
        public:
            static inline std::filesystem::path get_tmp_file(void)
            {
                std::filesystem::path tmp_file_path = std::filesystem::temp_directory_path();
                tmp_file_path /= std::filesystem::path(std::string("sup_def_tmp_") + std::to_string(get_counter()));
                while (std::filesystem::exists(tmp_file_path))
                    tmp_file_path = std::filesystem::temp_directory_path() / std::filesystem::path(std::string("sup_def_tmp_") + std::to_string(get_counter()));
                return tmp_file_path;
            }
    };

    /*
     * A SupDef pragma has the form:
     *
     * #pragma supdef begin <super_define_name>
     * <super_define_body>
     * #pragma supdef end
     * 
     * Where:
     * - <super_define_name> is the name of the super define (it is a classic C(++) identifier / macro name)
     * - <super_define_body> is the body of the super define, and has the form:
     *  <super_define_body> ::= <super_define_body_line>*
     * <super_define_body_line> ::= ["] <super_define_body_line_content> ["] [\n]
     * <super_define_body_line_content> ::= <super_define_body_line_content_char>*
     * <super_define_body_line_content_char> ::= [anything except " (which has to be escaped with \") and \n]
     * 
     * Example:
     * #pragma supdef begin MY_SUPER_DEFINE
     * "#define TEST \"This is the body of my super define named $0 which takes 2 arguments: $1 and $2\""
     * #pragma supdef end
     * 
     * Result of invocation of MY_SUPER_DEFINE(arg1, arg2):
     * #define TEST "This is the body of my super define named MY_SUPER_DEFINE which takes 2 arguments: arg1 and arg2"
     */

#if 0
    enum class PragmaType : uint8_t
    {
        INC = 1 << 0,
        DEF = 1 << 1
    };

    template <typename CharType = char>
        requires CharacterType<CharType>
    class PragmaDefFields
    {
        private:
            std::shared_ptr<std::basic_string<CharType>> id;
            std::shared_ptr<std::basic_string<CharType>> body;

        public:
            PragmaDefFields() = default;
            
            template <typename StdStringType1, typename StdStringType2>
                requires std::same_as<std::remove_cvref_t<StdStringType1>, std::basic_string<CharType>> &&
                         std::same_as<std::remove_cvref_t<StdStringType2>, std::basic_string<CharType>>
            PragmaDefFields(StdStringType1&& id, StdStringType2&& body)
            {
                if constexpr (std::is_rvalue_reference_v<StdStringType1&&>)
                    this->id = std::make_shared<std::basic_string<CharType>>(std::move(id));
                else
                    this->id = std::make_shared<std::basic_string<CharType>>(id);
                if constexpr (std::is_rvalue_reference_v<StdStringType2&&>)
                    this->body = std::make_shared<std::basic_string<CharType>>(std::move(body));
                else
                    this->body = std::make_shared<std::basic_string<CharType>>(body);
            }
            template <typename StdStringType>
                requires std::same_as<std::remove_cvref_t<StdStringType>, std::basic_string<CharType>>
            PragmaDefFields(StdStringType&& id)
            {
                if constexpr (std::is_rvalue_reference_v<StdStringType&&>)
                    this->id = std::make_shared<std::basic_string<CharType>>(std::move(id));
                else
                    this->id = std::make_shared<std::basic_string<CharType>>(id);
            }

            PragmaDefFields(const PragmaDefFields&) = default;
            PragmaDefFields(PragmaDefFields&&) = default;

            ~PragmaDefFields() = default;

            PragmaDefFields& operator=(const PragmaDefFields&) = default;
            PragmaDefFields& operator=(PragmaDefFields&&) = default;

        protected:
            inline std::shared_ptr<std::basic_string<CharType>> get_id() const noexcept
            {
                return this->id;
            }

            inline std::shared_ptr<std::basic_string<CharType>> get_body() const noexcept
            {
                return this->body;
            }

            inline void set_id(std::shared_ptr<std::basic_string<CharType>> id) noexcept
            {
                this->id = id;
            }

            inline void set_body(std::shared_ptr<std::basic_string<CharType>> body) noexcept
            {
                this->body = body;
            }

            template <typename StdStringType>
                requires std::same_as<std::remove_cvref_t<StdStringType>, std::basic_string<CharType>>
            inline void set_id(StdStringType&& id) noexcept
            {
                if constexpr (std::is_rvalue_reference_v<StdStringType&&>)
                    this->id = std::make_shared<std::basic_string<CharType>>(std::move(id));
                else
                    this->id = std::make_shared<std::basic_string<CharType>>(id);
            }

            template <typename StdStringType>
                requires std::same_as<std::remove_cvref_t<StdStringType>, std::basic_string<CharType>>
            inline void set_body(StdStringType&& body) noexcept
            {
                if constexpr (std::is_rvalue_reference_v<StdStringType&&>)
                    this->body = std::make_shared<std::basic_string<CharType>>(std::move(body));
                else
                    this->body = std::make_shared<std::basic_string<CharType>>(body);
            }

            inline void set_id(const CharType* id) noexcept
            {
                this->id = std::make_shared<std::basic_string<CharType>>(id);
            }

            inline void set_body(const CharType* body) noexcept
            {
                this->body = std::make_shared<std::basic_string<CharType>>(body);
            }
    };

    template <typename CharType = char, typename FilePathType = std::filesystem::path>
        requires CharacterType<CharType> && FilePath<FilePathType>
    struct PragmaIncFields
    {
        private:
            std::shared_ptr<SrcFile<CharType, FilePathType>> included;

        public:
            PragmaIncFields() = default;
            PragmaIncFields(const PragmaIncFields&) = default;
            PragmaIncFields(PragmaIncFields&&) = default;
            ~PragmaIncFields() = default;

            PragmaIncFields(FilePathType path) : included(std::make_shared<SrcFile<CharType, FilePathType>>(path)) {}

            PragmaIncFields& operator=(const PragmaIncFields&) = default;
            PragmaIncFields& operator=(PragmaIncFields&&) = default;

            inline std::shared_ptr<SrcFile<CharType, FilePathType>> get_included() const noexcept
            {
                return this->included;
            }

            inline auto get_path() const noexcept
            {
                return this->included->get_path();
            }
    };

    template <PragmaType PragType, typename CharType = char, typename FilePathType = std::filesystem::path>
        requires CharacterType<CharType> && FilePath<FilePathType>
    struct PragmaBase
    {
        private:
            std::shared_ptr<std::basic_string<CharType>> full_pragma;
            std::shared_ptr<std::tuple<string_size_type<CharType>, string_size_type<CharType>>> pos;
    };

    template <PragmaType PragType, typename CharType = char, typename FilePathType = std::filesystem::path>
        requires CharacterType<CharType> && FilePath<FilePathType>
    struct Pragma : private PragmaDefFields<CharType>
    {
        private:
            std::shared_ptr<std::basic_string<CharType>> full_pragma;

        public:
            Pragma() = default;
    };
#endif

    // TODO: Modify PragmaDef
    /**
     * @class PragmaDef
     * @brief A class representing a SupDef pragma definition
     * @tparam CharType The character type of the pragma (char, wchar_t, char8_t, char16_t, char32_t)
     */
    template <typename CharType>
        requires CharacterType<CharType>
    struct PragmaDef
    {
        private:
            std::shared_ptr<std::basic_string<CharType>> id;
            std::shared_ptr<std::basic_string<CharType>> body;
            std::shared_ptr<std::tuple<string_size_type<CharType>, string_size_type<CharType>>> pos;

        public:
            typedef Util::pragma_loc_type<CharType> pragma_loc_type;

            PragmaDef() = default;
            PragmaDef(const PragmaDef&) = default;
            PragmaDef(PragmaDef&&) = default;
            ~PragmaDef() = default;

            template <typename PragLocType>
                requires std::same_as<std::remove_cvref_t<PragLocType>, pragma_loc_type>
            PragmaDef(PragLocType&& pragma_loc)
            {
                if constexpr (std::is_rvalue_reference_v<PragLocType&&>)
                {
                    this->pos = std::make_shared< std::tuple< string_size_type<CharType>, string_size_type<CharType> > >(
                        std::make_tuple(
                            std::get<1>(std::move(pragma_loc)),
                            std::get<2>(std::move(pragma_loc))
                        )
                    );
                    auto first_line = [](const std::basic_string<CharType>& str) -> std::basic_string<CharType>
                    {
                        std::basic_string<CharType> result;
                        for (const CharType& c : str)
                        {
                            if (SAME(c, '\n'))
                                break;
                            result += c;
                        }
                        return result;
                    }(std::get<0>(std::move(pragma_loc)));
                    this->id = std::make_shared<std::basic_string<CharType>>(
                        remove_whitespaces(first_line, true)
                    );
                    auto remaining_lines = [](const std::basic_string<CharType>& str) -> std::basic_string<CharType>
                    {
                        std::basic_string<CharType> result;
                        bool is_first_line = true;
                        for (const CharType& c : str)
                        {
                            if (is_first_line)
                            {
                                if (SAME(c, '\n'))
                                    is_first_line = false;
                            }
                            else
                                result += c;
                        }
                        return result;
                    }(std::get<0>(std::move(pragma_loc)));
                    this->body = std::make_shared<std::basic_string<CharType>>(
                        remove_whitespaces(remaining_lines, true, true)
                    );
                }
                else
                {
                    this->pos = std::make_shared< std::tuple< string_size_type<CharType>, string_size_type<CharType> > >(
                        std::make_tuple(
                            std::get<1>(pragma_loc),
                            std::get<2>(pragma_loc)
                        )
                    );
                    auto first_line = [](const std::basic_string<CharType>& str) -> std::basic_string<CharType>
                    {
                        std::basic_string<CharType> result;
                        for (const CharType& c : str)
                        {
                            if (SAME(c, '\n'))
                                break;
                            result += c;
                        }
                        return result;
                    }(std::get<0>(pragma_loc));
                    this->id = std::make_shared<std::basic_string<CharType>>(
                        remove_whitespaces(first_line, true)
                    );
                    auto remaining_lines = [](const std::basic_string<CharType>& str) -> std::basic_string<CharType>
                    {
                        std::basic_string<CharType> result;
                        bool is_first_line = true;
                        for (const CharType& c : str)
                        {
                            if (is_first_line)
                            {
                                if (SAME(c, '\n'))
                                    is_first_line = false;
                            }
                            else
                                result += c;
                        }
                        return result;
                    }(std::get<0>(pragma_loc));
                    this->body = std::make_shared<std::basic_string<CharType>>(
                        remove_whitespaces(remaining_lines, true, true)
                    );
                }
            }
            template <typename StdStringType1, typename StdStringType2, typename PosType>
                requires std::same_as<std::remove_cvref_t<StdStringType1>, std::basic_string<CharType>> &&
                         std::same_as<std::remove_cvref_t<StdStringType2>, std::basic_string<CharType>> &&
                         std::same_as<std::remove_cvref_t<PosType>, std::tuple<string_size_type<CharType>, string_size_type<CharType>>>
            PragmaDef(StdStringType1&& id, StdStringType2&& body, PosType&& pos = std::tuple<string_size_type<CharType>, string_size_type<CharType>>(0, 0))
            {
                if constexpr (std::is_rvalue_reference_v<StdStringType1&&>)
                    this->id = std::make_shared<std::basic_string<CharType>>(std::move(id));
                else
                    this->id = std::make_shared<std::basic_string<CharType>>(id);
             
                if constexpr (std::is_rvalue_reference_v<StdStringType2&&>)
                    this->body = std::make_shared<std::basic_string<CharType>>(std::move(body));
                else
                    this->body = std::make_shared<std::basic_string<CharType>>(body);
             
                if constexpr (std::is_rvalue_reference_v<PosType&&>)
                    this->pos = std::make_shared< std::tuple< string_size_type<CharType>, string_size_type<CharType> > >(std::move(pos));
                else
                    this->pos = std::make_shared< std::tuple< string_size_type<CharType>, string_size_type<CharType> > >(pos);
            }

            PragmaDef& operator=(const PragmaDef&) = default;
            PragmaDef& operator=(PragmaDef&&) = default;

            inline auto get_pos() const noexcept
            {
                return this->pos;
            }

            inline auto get_id() const noexcept
            {
                return this->id;
            }

            inline auto get_body() const noexcept
            {
                return this->body;
            }

            inline string_size_type<CharType> get_start() const noexcept
            {
                return std::get<0>(*this->pos);
            }

            inline string_size_type<CharType> get_end() const noexcept
            {
                return std::get<1>(*this->pos);
            }

            inline void set_pos(
                std::shared_ptr<std::tuple<string_size_type<CharType>, string_size_type<CharType>>> pos
            ) noexcept
            {
                this->pos = pos;
            }

            inline void set_id(std::shared_ptr<std::basic_string<CharType>> id) noexcept
            {
                this->id = id;
            }

            inline void set_body(std::shared_ptr<std::basic_string<CharType>> body) noexcept
            {
                this->body = body;
            }

            template <typename StdStringType>
                requires std::same_as<std::remove_cvref_t<StdStringType>, std::basic_string<CharType>>
            inline void set_id(StdStringType&& id) noexcept
            {
                if constexpr (std::is_rvalue_reference_v<StdStringType&&>)
                    this->id = std::make_shared<std::basic_string<CharType>>(std::move(id));
                else
                    this->id = std::make_shared<std::basic_string<CharType>>(id);
            }

            template <typename StdStringType>
                requires std::same_as<std::remove_cvref_t<StdStringType>, std::basic_string<CharType>>
            inline void set_body(StdStringType&& body) noexcept
            {
                if constexpr (std::is_rvalue_reference_v<StdStringType&&>)
                    this->body = std::make_shared<std::basic_string<CharType>>(std::move(body));
                else
                    this->body = std::make_shared<std::basic_string<CharType>>(body);
            }

            inline void set_id(const CharType* id) noexcept
            {
                this->id = std::make_shared<std::basic_string<CharType>>(id);
            }

            inline void set_body(const CharType* body) noexcept
            {
                this->body = std::make_shared<std::basic_string<CharType>>(body);
            }

            size_t get_argc(void) const noexcept
            {
                size_t result = 0;
                std::vector<uint8_t> curr_num(1);
                bool in_arg = false;
                bool reached_first_num = false;
                string_size_type<CharType> curr_pos = 0;
                for (const CharType& c : *this->body)
                {
                    if (SAME(c, '$'))
                    {
                        if (in_arg)
                            continue;
                        size_t backslash_count = 0;
                        for (string_size_type<CharType> i = curr_pos - 1; i >= 0; --i)
                        {
                            if (SAME((*this->body)[i], '\\'))
                                ++backslash_count;
                            else
                                break;
                        }
                        if (backslash_count % 2 == 0)
                            in_arg = true;
                    }
                    else if (in_arg)
                    {
                        auto is_number = [](const CharType& c) -> bool
                        {
                            return SAME(c, '0') || SAME(c, '1') || SAME(c, '2') || SAME(c, '3') || SAME(c, '4') || SAME(c, '5') || SAME(c, '6') || SAME(c, '7') || SAME(c, '8') || SAME(c, '9');
                        };
                        if (is_number(c))
                        {
                            if (!reached_first_num)
                                reached_first_num = true;
                            curr_num.push_back(static_cast<uint8_t>(CONVERT_NUM(unsigned short, c)));
                        }
                        else
                        {
                            if (!reached_first_num)
                                continue;
                            curr_num.insert(curr_num.begin(), 0);
                            result = std::max(result, size_t(std::accumulate(curr_num.begin(), curr_num.end(), 0, [](const uint8_t& a, const uint8_t& b) -> uint8_t { return a * 10 + b; })));
                            curr_num.clear();
                            in_arg = false;
                            reached_first_num = false;
                        }
                    }
                    ++curr_pos;
                }
                if (in_arg && reached_first_num)
                {
                    curr_num.insert(curr_num.begin(), 0);
                    result = std::max(result, size_t(std::accumulate(curr_num.begin(), curr_num.end(), 0, [](const uint8_t& a, const uint8_t& b) -> uint8_t { return a * 10 + b; })));
                }
                return result;
            }

            template <typename... Args>
                requires (std::same_as<std::remove_cvref_t<Args>, std::basic_string<CharType>> && ...)
            std::basic_string<CharType> substitute(Args&&... args) const
            {
                std::basic_string<CharType> result;
                std::vector<std::basic_string<CharType>> args_vec{ std::forward<Args>(args)... };
                std::vector<uint8_t> curr_num(1);
                bool in_arg = false;
                bool reached_first_num = false;
                string_size_type<CharType> curr_pos = 0;
                for (const CharType& c : *this->body)
                {
                    if (SAME(c, '$'))
                    {
                        if (in_arg)
                            continue;
                        size_t backslash_count = 0;
                        for (string_size_type<CharType> i = curr_pos - 1; i >= 0; --i)
                        {
                            if (SAME((*this->body)[i], '\\'))
                                ++backslash_count;
                            else
                                break;
                        }
                        if (backslash_count % 2 == 0)
                            in_arg = true;
                    }
                    else if (in_arg)
                    {
                        auto is_number = [](const CharType& c) -> bool
                        {
                            return SAME(c, '0') || SAME(c, '1') || SAME(c, '2') || SAME(c, '3') || SAME(c, '4') || SAME(c, '5') || SAME(c, '6') || SAME(c, '7') || SAME(c, '8') || SAME(c, '9');
                        };
                        if (is_number(c))
                        {
                            if (!reached_first_num)
                                reached_first_num = true;
                            curr_num.push_back(static_cast<uint8_t>(CONVERT_NUM(unsigned short, c)));
                        }
                        else
                        {
                            if (!reached_first_num)
                                continue;
                            curr_num.insert(curr_num.begin(), 0);
                            size_t arg_num = std::accumulate(curr_num.begin(), curr_num.end(), 0, [](const uint8_t& a, const uint8_t& b) -> uint8_t { return a * 10 + b; });
                            if (arg_num > args_vec.size())
                                throw Exception<CharType, std::filesystem::path>(ExcType::INTERNAL_ERROR, "Invalid argument number in super define " + CONVERT(std::string, *this->id) + " (expected: " + std::to_string(args_vec.size()) + ", got: " + std::to_string(arg_num) + ")");
                            result += args_vec[arg_num - 1];
                            curr_num.clear();
                            in_arg = false;
                            reached_first_num = false;
                        }
                    }
                    else
                        result += c;
                    ++curr_pos;
                }
                if (in_arg && reached_first_num)
                {
                    curr_num.insert(curr_num.begin(), 0);
                    size_t arg_num = std::accumulate(curr_num.begin(), curr_num.end(), 0, [](const uint8_t& a, const uint8_t& b) -> uint8_t { return a * 10 + b; });
                    if (arg_num > args_vec.size())
                        throw Exception<CharType, std::filesystem::path>(ExcType::INTERNAL_ERROR, "Invalid argument number in super define " + CONVERT(std::string, *this->id) + " (expected: " + std::to_string(args_vec.size()) + ", got: " + std::to_string(arg_num) + ")");
                    result += args_vec[arg_num - 1];
                }
                return result;
            }

            template <typename... Args>
                requires (std::same_as<std::remove_cvref_t<Args>, std::basic_string<CharType>> && ...)
            std::basic_string<CharType> operator()(Args&&... args) const
            {
                return this->substitute(std::forward<Args>(args)...);
            }
    };

    template <typename T, typename U>
        requires CharacterType<T> && FilePath<U>
    class SrcFile;

    // TODO: Modify PragmaInc
    template <typename CharType, typename FilePathType>
        requires CharacterType<CharType> && FilePath<FilePathType>
    struct PragmaInc
    {
        private:
            std::shared_ptr<std::tuple<string_size_type<CharType>, string_size_type<CharType>>> pos;
            std::shared_ptr<SrcFile<CharType, FilePathType>> included;

        public:
            typedef Util::pragma_loc_type<CharType> pragma_loc_type;
            PragmaInc() = default;
            ~PragmaInc() noexcept;
            
            template <typename PragLocType>
                requires std::same_as<std::remove_cvref_t<PragLocType>, pragma_loc_type>
            PragmaInc(PragLocType&& pragma_loc)
            {
                if constexpr (std::is_rvalue_reference_v<PragLocType&&>)
                {
                    this->pos = std::make_shared< std::tuple< string_size_type<CharType>, string_size_type<CharType> > >(
                        std::make_tuple(
                            std::get<1>(std::move(pragma_loc)),
                            std::get<2>(std::move(pragma_loc))
                        )
                    );
                    this->included = std::make_shared<SrcFile<CharType, FilePathType>>(CONVERT(FilePathType, remove_whitespaces(std::get<0>(std::move(pragma_loc)))));
                }
                else
                {
                    this->pos = std::make_shared< std::tuple< string_size_type<CharType>, string_size_type<CharType> > >(
                        std::make_tuple(
                            std::get<1>(pragma_loc),
                            std::get<2>(pragma_loc)
                        )
                    );
                    this->included = std::make_shared<SrcFile<CharType, FilePathType>>(CONVERT(FilePathType, remove_whitespaces(std::get<0>(pragma_loc))));
                }
            }
            template <typename StdStringType, typename PosType>
                requires std::same_as<std::remove_cvref_t<StdStringType>, std::basic_string<CharType>> &&
                         std::same_as<std::remove_cvref_t<PosType>, std::tuple<string_size_type<CharType>, string_size_type<CharType>>>
            PragmaInc(StdStringType&& path, PosType&& pos)
            {
                if constexpr (std::is_rvalue_reference_v<StdStringType&&>)
                    this->included = std::make_shared<SrcFile<CharType, FilePathType>>(std::move(path));
                else
                    this->included = std::make_shared<SrcFile<CharType, FilePathType>>(path);
                if constexpr (std::is_rvalue_reference_v<PosType&&>)
                    this->pos = std::make_shared< std::tuple< string_size_type<CharType>, string_size_type<CharType> > >(std::move(pos));
                else
                    this->pos = std::make_shared< std::tuple< string_size_type<CharType>, string_size_type<CharType> > >(pos);
            }

            PragmaInc(const PragmaInc&) = default;
            PragmaInc(PragmaInc&&) = default;

            PragmaInc& operator=(const PragmaInc&) = default;
            PragmaInc& operator=(PragmaInc&&) = default;

            inline auto get_pos() const noexcept
            {
                return this->pos;
            }
            
            inline auto get_included() const noexcept
            {
                return this->included;
            }

            inline auto get_path() const noexcept
            {
                return this->included->get_path();
            }

            inline string_size_type<CharType> get_start() const noexcept
            {
                return std::get<0>(*this->pos);
            }
            
            inline string_size_type<CharType> get_end() const noexcept
            {
                return std::get<1>(*this->pos);
            }

            inline void set_pos(std::shared_ptr<std::tuple<string_size_type<CharType>, string_size_type<CharType>>> pos) noexcept
            {
                this->pos = pos;
            }
    };

    template <typename T>
        requires CharacterType<T>
    class Parser;

    template <typename T>
    concept OutputFileStream = std::same_as<std::remove_cvref_t<T>, std::ofstream>                  ||
                               std::same_as<std::remove_cvref_t<T>, std::wofstream>                 ||
                               std::same_as<std::remove_cvref_t<T>, std::basic_ofstream<char8_t>>   ||
                               std::same_as<std::remove_cvref_t<T>, std::basic_ofstream<char16_t>>  ||
                               std::same_as<std::remove_cvref_t<T>, std::basic_ofstream<char32_t>>  ||
                               std::same_as<std::remove_cvref_t<T>, std::basic_ofstream<std::byte>>;

    template <typename T>
    concept InputFileStream = std::same_as<std::remove_cvref_t<T>, std::ifstream>                  ||
                              std::same_as<std::remove_cvref_t<T>, std::wifstream>                 ||
                              std::same_as<std::remove_cvref_t<T>, std::basic_ifstream<char8_t>>   ||
                              std::same_as<std::remove_cvref_t<T>, std::basic_ifstream<char16_t>>  ||
                              std::same_as<std::remove_cvref_t<T>, std::basic_ifstream<char32_t>>  ||
                              std::same_as<std::remove_cvref_t<T>, std::basic_ifstream<std::byte>>;

    template <typename T>
    concept InputOutputStream = std::same_as<std::remove_cvref_t<T>, std::fstream>                  ||
                                std::same_as<std::remove_cvref_t<T>, std::wfstream>                 ||
                                std::same_as<std::remove_cvref_t<T>, std::basic_fstream<char8_t>>   ||
                                std::same_as<std::remove_cvref_t<T>, std::basic_fstream<char16_t>>  ||
                                std::same_as<std::remove_cvref_t<T>, std::basic_fstream<char32_t>>  ||
                                std::same_as<std::remove_cvref_t<T>, std::basic_fstream<std::byte>>;

    template <typename T>
    concept FileStream = OutputFileStream<T> || InputFileStream<T> || InputOutputStream<T>;

#if 0
    template <typename T, typename... Args>
    concept HasGetlineMethod = requires(T&& t, Args&&... args)
    {
        { t.getline(std::forward<Args>(args)...) };
    };

    template <typename T, typename... Args>
    concept HasGetlineFunction = requires(T&& t, Args&&... args)
    {
        { std::getline(std::forward<T>(t), std::forward<Args>(args)...) };
    };

    // TODO: Fix bugs in File and File_StreamMethodsImpl
    template <typename T>
        requires FileStream<T>
    struct File;

    template <typename T, typename... Args>
    struct File_StreamMethodsImplHelper {};

    template <FileStream StreamType, typename... Args>
        requires (!HasGetlineMethod<StreamType, Args...> && !HasGetlineFunction<StreamType, Args...>)
    struct File_StreamMethodsImplHelper<StreamType, Args...>
    {
        // Return type of invoking std::getline(StreamType&, Args...)
        using ResultOfGetline = void;
    };

    template <FileStream StreamType, typename... Args>
        requires HasGetlineMethod<StreamType, Args...>
    struct File_StreamMethodsImplHelper<StreamType, Args...>
    {
        // Return type of invoking std::getline(StreamType&, Args...)
        using ResultOfGetline = decltype(std::declval<StreamType&>().getline(std::declval<Args>()...));
    };

    template <FileStream StreamType, typename... Args>
        requires HasGetlineFunction<StreamType, Args...>
    struct File_StreamMethodsImplHelper<StreamType, Args...>
    {
        // Return type of invoking std::getline(StreamType&, Args...)
        using ResultOfGetline = decltype(std::getline(std::declval<StreamType&>(), std::declval<Args>()...));
    };

    template <typename T>
    struct File_StreamMethodsImpl {};

    template <OutputFileStream StreamType>
    struct File_StreamMethodsImpl<StreamType>
    {
        protected:
            std::optional<StreamType>* stream_ptr;
            typedef typename StreamType::char_type char_type;

        public:
            File_StreamMethodsImpl() = default;
            File_StreamMethodsImpl(std::optional<StreamType>& stream_ref) : stream_ptr(std::addressof(stream_ref)) {}
            File_StreamMethodsImpl(File<StreamType>& file) : stream_ptr(std::addressof(file.get_stream())) {}
            File_StreamMethodsImpl(const File_StreamMethodsImpl&) = default;
            File_StreamMethodsImpl(File_StreamMethodsImpl&&) = default;
            ~File_StreamMethodsImpl() = default;

        protected:
            inline StreamType& operator<<(const char_type* str)
            {
                if (!(*this->stream_ptr).has_value())
                    throw InternalError("Attempt to use operator<< on a File_StreamMethodsImpl object which has no stream");
                if (!(*this->stream_ptr)->is_open())
                    throw InternalError("Attempt to use operator<< on a File_StreamMethodsImpl object which has a closed stream");
                *(*this->stream_ptr) << str;
                return *(*this->stream_ptr);
            }
            inline StreamType& operator<<(const std::basic_string<char_type>& str)
            {
                if (!(*this->stream_ptr).has_value())
                    throw InternalError("Attempt to use operator<< on a File_StreamMethodsImpl object which has no stream");
                if (!(*this->stream_ptr)->is_open())
                    throw InternalError("Attempt to use operator<< on a File_StreamMethodsImpl object which has a closed stream");
                *(*this->stream_ptr) << str;
                return *(*this->stream_ptr);
            }
            inline StreamType& operator<<(const char_type& c)
            {
                if (!(*this->stream_ptr).has_value())
                    throw InternalError("Attempt to use operator<< on a File_StreamMethodsImpl object which has no stream");
                if (!(*this->stream_ptr)->is_open())
                    throw InternalError("Attempt to use operator<< on a File_StreamMethodsImpl object which has a closed stream");
                *(*this->stream_ptr) << c;
                return *(*this->stream_ptr);
            }
            inline StreamType& operator<<(const std::basic_string_view<char_type>& str)
            {
                if (!(*this->stream_ptr).has_value())
                    throw InternalError("Attempt to use operator<< on a File_StreamMethodsImpl object which has no stream");
                if (!(*this->stream_ptr)->is_open())
                    throw InternalError("Attempt to use operator<< on a File_StreamMethodsImpl object which has a closed stream");
                *(*this->stream_ptr) << str;
                return *(*this->stream_ptr);
            }

            // Delete operator>> and getline
            template <typename U>
            StreamType& operator>>(U&&) = delete;

            template <typename U>
            StreamType& getline(U&&) = delete;
    };

    template <InputFileStream StreamType>
    struct File_StreamMethodsImpl<StreamType>
    {
        protected:
            std::optional<StreamType>* stream_ptr;
            typedef typename StreamType::char_type char_type;

        public:
            File_StreamMethodsImpl() = default;
            File_StreamMethodsImpl(std::optional<StreamType>& stream_ref) : stream_ptr(std::addressof(stream_ref)) {}
            File_StreamMethodsImpl(File<StreamType>& file) : stream_ptr(std::addressof(file.get_stream())) {}
            File_StreamMethodsImpl(const File_StreamMethodsImpl&) = default;
            File_StreamMethodsImpl(File_StreamMethodsImpl&&) = default;
            ~File_StreamMethodsImpl() = default;

        protected:
            inline StreamType& operator>>(char_type* str)
            {
                if (!(*this->stream_ptr).has_value())
                    throw InternalError("Attempt to use operator>> on a File_StreamMethodsImpl object which has no stream");
                if (!(*this->stream_ptr)->is_open())
                    throw InternalError("Attempt to use operator>> on a File_StreamMethodsImpl object which has a closed stream");
                *(*this->stream_ptr) >> str;
                return *(*this->stream_ptr);
            }
            inline StreamType& operator>>(std::basic_string<char_type>& str)
            {
                if (!(*this->stream_ptr).has_value())
                    throw InternalError("Attempt to use operator>> on a File_StreamMethodsImpl object which has no stream");
                if (!(*this->stream_ptr)->is_open())
                    throw InternalError("Attempt to use operator>> on a File_StreamMethodsImpl object which has a closed stream");
                *(*this->stream_ptr) >> str;
                return *(*this->stream_ptr);
            }
            inline StreamType& operator>>(char_type& c)
            {
                if (!(*this->stream_ptr).has_value())
                    throw InternalError("Attempt to use operator>> on a File_StreamMethodsImpl object which has no stream");
                if (!(*this->stream_ptr)->is_open())
                    throw InternalError("Attempt to use operator>> on a File_StreamMethodsImpl object which has a closed stream");
                *(*this->stream_ptr) >> c;
                return *(*this->stream_ptr);
            }

            // Same as std::getline
            inline typename File_StreamMethodsImplHelper<StreamType, char_type*, std::streamsize, char_type>::ResultOfGetline getline(char_type* str, std::streamsize count, char_type delim)
            {
                if (!(*this->stream_ptr).has_value())
                    throw InternalError("Attempt to use getline on a File_StreamMethodsImpl object which has no stream");
                if (!(*this->stream_ptr)->is_open())
                    throw InternalError("Attempt to getline on a File_StreamMethodsImpl object which has a closed stream");
                return (*this->stream_ptr)->getline(str, count, delim);
            }
            inline typename File_StreamMethodsImplHelper<StreamType, std::basic_string<char_type>&, char_type>::ResultOfGetline getline(std::basic_string<char_type>& str, char_type delim)
            {
                if (!(*this->stream_ptr).has_value())
                    throw InternalError("Attempt to getline on a File_StreamMethodsImpl object which has no stream");
                if (!(*this->stream_ptr)->is_open())
                    throw InternalError("Attempt to getline on a File_StreamMethodsImpl object which has a closed stream");
                return std::getline(*(*this->stream_ptr), str, delim);
            }
            inline typename File_StreamMethodsImplHelper<StreamType, std::basic_string<char_type>&>::ResultOfGetline getline(std::basic_string<char_type>& str)
            {
                if (!(*this->stream_ptr).has_value())
                    throw InternalError("Attempt to getline on a File_StreamMethodsImpl object which has no stream");
                if (!(*this->stream_ptr)->is_open())
                    throw InternalError("Attempt to getline on a File_StreamMethodsImpl object which has a closed stream");
                return std::getline(*(*this->stream_ptr), str);
            }


            // Delete operator<<
            template <typename U>
            StreamType& operator<<(U&&) = delete;
    };

    template <InputOutputStream StreamType>
    struct File_StreamMethodsImpl<StreamType>
    {
        protected:
            std::optional<StreamType>* stream_ptr;
            typedef typename StreamType::char_type char_type;

        public:
            File_StreamMethodsImpl() = default;
            File_StreamMethodsImpl(std::optional<StreamType>& stream_ref) : stream_ptr(std::addressof(stream_ref)) {}
            File_StreamMethodsImpl(File<StreamType>& file) : stream_ptr(std::addressof(file.get_stream())) {}
            File_StreamMethodsImpl(const File_StreamMethodsImpl&) = default;
            File_StreamMethodsImpl(File_StreamMethodsImpl&&) = default;
            ~File_StreamMethodsImpl() = default;

        protected:
            inline StreamType& operator<<(const char_type* str)
            {
                if (!(*this->stream_ptr).has_value())
                    throw InternalError("Attempt to use operator<< on a File_StreamMethodsImpl object which has no stream");
                if (!(*this->stream_ptr)->is_open())
                    throw InternalError("Attempt to use operator<< on a File_StreamMethodsImpl object which has a closed stream");
                *(*this->stream_ptr) << str;
                return *(*this->stream_ptr);
            }
            inline StreamType& operator<<(const std::basic_string<char_type>& str)
            {
                if (!(*this->stream_ptr).has_value())
                    throw InternalError("Attempt to use operator<< on a File_StreamMethodsImpl object which has no stream");
                if (!(*this->stream_ptr)->is_open())
                    throw InternalError("Attempt to use operator<< on a File_StreamMethodsImpl object which has a closed stream");
                *(*this->stream_ptr) << str;
                return *(*this->stream_ptr);
            }
            inline StreamType& operator<<(const char_type& c)
            {
                if (!(*this->stream_ptr).has_value())
                    throw InternalError("Attempt to use operator<< on a File_StreamMethodsImpl object which has no stream");
                if (!(*this->stream_ptr)->is_open())
                    throw InternalError("Attempt to use operator<< on a File_StreamMethodsImpl object which has a closed stream");
                *(*this->stream_ptr) << c;
                return *(*this->stream_ptr);
            }
            inline StreamType& operator<<(const std::basic_string_view<char_type>& str)
            {
                if (!(*this->stream_ptr).has_value())
                    throw InternalError("Attempt to use operator<< on a File_StreamMethodsImpl object which has no stream");
                if (!(*this->stream_ptr)->is_open())
                    throw InternalError("Attempt to use operator<< on a File_StreamMethodsImpl object which has a closed stream");
                *(*this->stream_ptr) << str;
                return *(*this->stream_ptr);
            }

            inline StreamType& operator>>(char_type* str)
            {
                if (!(*this->stream_ptr).has_value())
                    throw InternalError("Attempt to use operator>> on a File_StreamMethodsImpl object which has no stream");
                if (!(*this->stream_ptr)->is_open())
                    throw InternalError("Attempt to use operator>> on a File_StreamMethodsImpl object which has a closed stream");
                *(*this->stream_ptr) >> str;
                return *(*this->stream_ptr);
            }
            inline StreamType& operator>>(std::basic_string<char_type>& str)
            {
                if (!(*this->stream_ptr).has_value())
                    throw InternalError("Attempt to use operator>> on a File_StreamMethodsImpl object which has no stream");
                if (!(*this->stream_ptr)->is_open())
                    throw InternalError("Attempt to use operator>> on a File_StreamMethodsImpl object which has a closed stream");
                *(*this->stream_ptr) >> str;
                return *(*this->stream_ptr);
            }
            inline StreamType& operator>>(char_type& c)
            {
                if (!(*this->stream_ptr).has_value())
                    throw InternalError("Attempt to use operator>> on a File_StreamMethodsImpl object which has no stream");
                if (!(*this->stream_ptr)->is_open())
                    throw InternalError("Attempt to use operator>> on a File_StreamMethodsImpl object which has a closed stream");
                *(*this->stream_ptr) >> c;
                return *(*this->stream_ptr);
            }

            // Same as std::getline()
            inline typename File_StreamMethodsImplHelper<StreamType, char_type*, std::streamsize, char_type>::ResultOfGetline getline(char_type* str, std::streamsize count, char_type delim)
            {
                if (!(*this->stream_ptr).has_value())
                    throw InternalError("Attempt getline on a File_StreamMethodsImpl object which has no stream");
                if (!(*this->stream_ptr)->is_open())
                    throw InternalError("Attempt getline on a File_StreamMethodsImpl object which has a closed stream");
                return (*this->stream_ptr)->getline(str, count, delim);
            }
            inline typename File_StreamMethodsImplHelper<StreamType, std::basic_string<char_type>&, char_type>::ResultOfGetline getline(std::basic_string<char_type>& str, char_type delim)
            {
                if (!(*this->stream_ptr).has_value())
                    throw InternalError("Attempt getline on a File_StreamMethodsImpl object which has no stream");
                if (!(*this->stream_ptr)->is_open())
                    throw InternalError("Attempt getline on a File_StreamMethodsImpl object which has a closed stream");
                return std::getline(*(*this->stream_ptr), str, delim);
            }
            inline typename File_StreamMethodsImplHelper<StreamType, std::basic_string<char_type>&>::ResultOfGetline getline(std::basic_string<char_type>& str)
            {
                if (!(*this->stream_ptr).has_value())
                    throw InternalError("Attempt getline on a File_StreamMethodsImpl object which has no stream");
                if (!(*this->stream_ptr)->is_open())
                    throw InternalError("Attempt getline on a File_StreamMethodsImpl object which has a closed stream");
                return std::getline(*(*this->stream_ptr), str);
            }
    };

    template <typename T>
        requires FileStream<T>
    struct File : private File_StreamMethodsImpl<T>
    {
        public:
            typedef T stream_type;
            typedef typename T::char_type char_type;

        private:
            std::filesystem::path path;
            std::optional<stream_type> stream;

        public:
            File() : File_StreamMethodsImpl<T>(*this), path(), stream(std::nullopt) {}
            
            File(std::filesystem::path&& p, std::ios_base::openmode mode) : File_StreamMethodsImpl<T>(*this), path(std::move(p)), stream(std::in_place, this->path, mode)
            {
                if (DIFFERENT_ANY(this->stream, (*this->stream_ptr)))
                    throw InternalError("File_StreamMethodsImpl base class has a reference to a different stream than the one in the File class");
            }
            File(std::filesystem::path& p, std::ios_base::openmode mode) : File_StreamMethodsImpl<T>(*this), path(p), stream(std::in_place, this->path, mode)
            {
                if (DIFFERENT_ANY(this->stream, (*this->stream_ptr)))
                    throw InternalError("File_StreamMethodsImpl base class has a reference to a different stream than the one in the File class");
            }
            
            File(const File& other) = delete;
            File(File&& other) : File_StreamMethodsImpl<T>(*this), path(std::move(other.path)), stream(std::move(other.stream))
            {
                if (DIFFERENT_ANY(this->stream, (*this->stream_ptr)))
                    throw InternalError("File_StreamMethodsImpl base class has a reference to a different stream than the one in the File class");
            }

            File& operator=(const File& other) = delete;
            File& operator=(File&& other)
            {
                this->path = std::move(other.path);
                this->stream = std::move(other.stream);
                this->stream_ptr = std::addressof(this->stream);
                if (DIFFERENT_ANY(this->stream, (*this->stream_ptr)))
                    throw InternalError("File_StreamMethodsImpl base class has a reference to a different stream than the one in the File class");
                return *this;
            }

            ~File() noexcept
            {
                if (this->stream.has_value() && this->stream->is_open())
                    this->stream->close();
                this->stream.reset();
            }
            
            constexpr inline std::filesystem::path& get_path() noexcept
            {
                return this->path;
            }

            constexpr inline std::optional<stream_type>& get_stream() noexcept
            {
                return this->stream;
            }

            inline bool is_open() const noexcept
            {
                return this->stream.has_value() && this->stream->is_open();
            }

            void restart()
            {
                this->path.clear();
                if (this->stream.has_value() && this->stream->is_open())
                    this->stream->close();
                this->stream.reset();
                this->stream_ptr = std::addressof(this->stream);
                if (DIFFERENT_ANY(this->stream, (*this->stream_ptr)))
                    throw InternalError("File_StreamMethodsImpl base class has a reference to a different stream than the one in the File class");
            }

            void restart(std::filesystem::path&& p, std::ios_base::openmode mode)
            {
                this->restart();
                this->path = std::move(p);
                this->stream.emplace(this->path, mode);
                this->stream_ptr = std::addressof(this->stream);
                if (DIFFERENT_ANY(this->stream, (*this->stream_ptr)))
                    throw InternalError("File_StreamMethodsImpl base class has a reference to a different stream than the one in the File class");
            }

            void close()
            {
                restart();
            }

            using File_StreamMethodsImpl<T>::operator<<;
            using File_StreamMethodsImpl<T>::operator>>;
            using File_StreamMethodsImpl<T>::getline;
    };
#else
    template <typename StreamType, typename FilePathType = std::filesystem::path>
        requires FileStream<StreamType> && FilePath<FilePathType>
    class File : public StreamType
    {
        private:
            FilePathType path;

        public:
            static constexpr bool output_filestream_p = OutputFileStream<StreamType> || InputOutputStream<StreamType>;
            static constexpr bool input_filestream_p = InputFileStream<StreamType> || InputOutputStream<StreamType>;
            static constexpr bool inputoutput_filestream_p = InputOutputStream<StreamType>;

            File() : StreamType(), path() {}
            
            File(
                FilePathType&& p,
                std::ios_base::openmode mode = 
                    std::conditional_t<
                        bool(inputoutput_filestream_p),
                        std::integral_constant<std::ios_base::openmode, std::ios_base::in | std::ios_base::out | std::ios_base::binary>,
                        std::conditional_t<
                            bool(output_filestream_p),
                            std::integral_constant<std::ios_base::openmode, std::ios_base::out | std::ios_base::binary>,
                            std::integral_constant<std::ios_base::openmode, std::ios_base::in | std::ios_base::binary>
                        >
                    >::value
            ) : StreamType(std::move(p), mode), path(std::move(p))
            { }
            File(
                const FilePathType& p,
                std::ios_base::openmode mode = 
                    std::conditional_t<
                        bool(inputoutput_filestream_p),
                        std::integral_constant<std::ios_base::openmode, std::ios_base::in | std::ios_base::out | std::ios_base::binary>,
                        std::conditional_t<
                            bool(output_filestream_p),
                            std::integral_constant<std::ios_base::openmode, std::ios_base::out | std::ios_base::binary>,
                            std::integral_constant<std::ios_base::openmode, std::ios_base::in | std::ios_base::binary>
                        >
                    >::value
            ) : StreamType(p, mode), path(p)
            { }

            File(const File& other) = delete;
            File(File&& other) : StreamType(std::move(other)), path(std::move(other.path))
            { }

            File& operator=(const File& other) = delete;
            File& operator=(File&& other)
            {
                this->path = std::move(other.path);
                return *this;
            }

            ~File() noexcept
            {
                this->restart();
            }

        protected:
            virtual constexpr inline FilePathType& get_path() noexcept
            {
                return this->path;
            }

            virtual constexpr inline StreamType& get_stream() noexcept
            {
                return *this;
            }

        public:
            virtual void restart()
            {
                this->path.clear();
                if (this->is_open())
                    this->StreamType::close();
            }

            virtual void restart(FilePathType&& p, std::ios_base::openmode mode)
            {
                this->restart();
                FilePathType unconverted = p;
                this->path = std::move(p);
                this->open(CONVERT(std::filesystem::path, std::move(unconverted)), mode);
            }

            virtual void restart(const FilePathType& p, std::ios_base::openmode mode)
            {
                this->restart();
                auto unconverted = p;
                this->path = p;
                this->open(CONVERT(std::filesystem::path, unconverted), mode);
            }
    };
#endif

    // TODO: Modify whole SrcFile implementation to use File as a base class
#if 0
    template <typename T, typename U>
        requires CharacterType<T> && FilePath<U>
    class SrcFile
    {
        private:
            std::shared_ptr<U> path;
            std::vector<PragmaDef<T>> super_defines;
            std::vector<PragmaInc<T, U>> imports;

        public:
            std::unique_ptr<Parser<T>> parser;
            std::unique_ptr<File<std::basic_ifstream<T>>> file;

            SrcFile() = default;
            SrcFile(U path) : path(std::make_shared<U>(path)),
                              parser(std::make_unique<Parser<T>>(path)),
                              file(std::make_unique<File<std::basic_ifstream<T>>>(path))
            { }
            SrcFile(SrcFile&& other) noexcept = default;
            SrcFile(SrcFile& other) : path(other->get_path),
                                      super_defines(other->get_super_defines()),
                                      imports(other->get_imports()),
                                      parser(std::make_unique<Parser<T>>(*(other->get_path()))),
                                      file(std::make_unique<File<std::basic_ifstream<T>>>(*(other->get_path())))
            { }
            ~SrcFile() noexcept
            {
                if (this->file->is_open())
                    this->file->get_stream()->close();
            }

            inline std::shared_ptr<U> get_path() const noexcept
            {
                return this->path;
            }

            inline std::vector<PragmaDef<T>> get_super_defines() const noexcept
            {
                return this->super_defines;
            }

            inline std::vector<PragmaInc<T, U>> get_imports() const noexcept
            {
                return this->imports;
            }

            inline void add_super_define(const PragmaDef<T>& super_define) noexcept
            {
                this->super_defines.push_back(super_define);
            }

            inline void add_include(const PragmaInc<T, U>& import) noexcept
            {
                this->imports.push_back(import);
            }

            inline void restart() noexcept
            {
                this->path.reset();
                this->super_defines.clear();
                this->imports.clear();
                this->parser.reset();
                this->file.reset();
            }

            inline void restart(U path) noexcept
            {
                this->restart();
                this->path = std::make_shared<U>(path);
                this->super_defines.clear();
                this->imports.clear();
                this->parser = std::make_unique<Parser<T>>(path);
                this->file = std::make_unique<File<std::basic_ifstream<T>>>(path);
            }

            inline auto operator=(SrcFile&& other) noexcept
            {
                this->path = other.path;
                this->super_defines = other.super_defines;
                this->imports = other.imports;
                this->parser = std::move(other.parser);
                this->file = std::move(other.file);
                return *this;
            }
    };
#else
    template <typename T = char, typename U = std::filesystem::path>
        requires CharacterType<T> && FilePath<U>
    class SrcFile : public File<std::basic_ifstream<T>, U>
    {
        private:
            std::vector<PragmaDef<T>> super_defines;
            std::vector<PragmaInc<T, U>> imports;

            using Base = File<std::basic_ifstream<T>, U>;
        public:
            SrcFile() : Base(), super_defines(), imports() {}

            template <typename PathType>
                requires FilePath<PathType> && std::constructible_from<Base, PathType&&, std::ios_base::openmode>
            SrcFile(
                PathType&& p,
                std::ios_base::openmode mode = (std::ios_base::in | std::ios_base::binary)
            ) : Base(std::forward<PathType>(p), mode), super_defines(), imports()
            { }
            
            SrcFile(SrcFile& other) = delete;
            SrcFile(SrcFile&& other) noexcept = default;
            
            ~SrcFile() noexcept = default;
            
            inline std::vector<PragmaDef<T>>& get_super_defines() noexcept
            {
                return this->super_defines;
            }
            inline std::vector<PragmaInc<T, U>>& get_imports() noexcept
            {
                return this->imports;
            }

            inline void add_super_define(const PragmaDef<T>& super_define) noexcept
            {
                this->super_defines.push_back(super_define);
            }
            inline void add_include(const PragmaInc<T, U>& import) noexcept
            {
                this->imports.push_back(import);
            }

            inline void restart() override
            {
                this->Base::restart();
                this->super_defines.clear();
                this->imports.clear();
            }

            inline void restart(
                U&& p,
                std::ios_base::openmode mode = (std::ios_base::in | std::ios_base::binary)
            ) override
            {
                if (mode & std::ios_base::out)
                    throw InternalError("Attempt to open a SrcFile in write mode");
                this->Base::restart(std::forward<U>(p), mode);
                this->super_defines.clear();
                this->imports.clear();
            }

            inline void restart(
                const U& p,
                std::ios_base::openmode mode = (std::ios_base::in | std::ios_base::binary)
            ) override
            {
                if (mode & std::ios_base::out)
                    throw InternalError("Attempt to open a SrcFile in write mode");
                this->Base::restart(p, mode);
                this->super_defines.clear();
                this->imports.clear();
            }

            inline SrcFile& operator=(SrcFile&& other) noexcept
            {
                this->super_defines = std::move(other.super_defines);
                this->imports = std::move(other.imports);
                this->Base::operator=(std::move(other));
                return *this;
            }
            inline SrcFile& operator=(SrcFile& other) = delete;

#if 0
            inline void close() override
            {
                this->restart();
            }
#endif
    };
#endif

    // Must have the same behavior as std::less
    template <typename T>
        requires CharacterType<T>
    class PragmaLocCompare
    {
        public:
            using tuple_type = std::tuple<string_size_type<T>, string_size_type<T>>;

            // Are the two tupled types in U convertible to string_size_type<T>?
            template <typename U>
                requires SPECIALIZATION_OF(U, std::tuple)
            using convertible_tuple = std::conjunction<
                std::is_convertible<std::tuple_element_t<0, std::remove_cvref_t<U>>, string_size_type<T>>,
                std::is_convertible<std::tuple_element_t<1, std::remove_cvref_t<U>>, string_size_type<T>>
            >;

            static_assert(convertible_tuple<std::tuple<string_size_type<char>, string_size_type<char>>>::value);
            static_assert(convertible_tuple<std::tuple<int, int>>::value);
            static_assert(convertible_tuple<std::tuple<int, string_size_type<char>>>::value);
            static_assert(convertible_tuple<std::tuple<string_size_type<char>, int>>::value);

            template <typename U>
                requires SPECIALIZATION_OF(U, std::tuple)
            static constexpr bool convertible_tuple_v = convertible_tuple<U>::value;
            
            enum : uint8_t
            {
                START = 1 << 0,
                END = 1 << 1,
                BOTH = START | END
            };

            template <typename U>
                requires std::same_as<tuple_type, std::remove_cvref_t<U>> || convertible_tuple_v<U>
            static constexpr inline bool eq(const U&& lhs, const U&& rhs, uint8_t flags = BOTH) noexcept
            {
                if (flags & BOTH)
                    return std::get<0>(lhs) == std::get<0>(rhs) && std::get<1>(lhs) == std::get<1>(rhs);
                else if (flags & START)
                    return std::get<0>(lhs) == std::get<0>(rhs);
                else if (flags & END)
                    return std::get<1>(lhs) == std::get<1>(rhs);
                else
                    return false;
            }

            template <typename U>
                requires std::same_as<tuple_type, std::remove_cvref_t<U>> || convertible_tuple_v<U>
            static constexpr inline bool lt(const U&& lhs, const U&& rhs, uint8_t flags = BOTH) noexcept
            {
                if (flags & BOTH)
                    return std::get<0>(lhs) < std::get<0>(rhs) && std::get<1>(lhs) < std::get<1>(rhs);
                else if (flags & START)
                    return std::get<0>(lhs) < std::get<0>(rhs);
                else if (flags & END)
                    return std::get<1>(lhs) < std::get<1>(rhs);
                else
                    return false;
            }

            template <typename U>
                requires std::same_as<tuple_type, std::remove_cvref_t<U>> || convertible_tuple_v<U>
            static constexpr inline bool gt(const U&& lhs, const U&& rhs, uint8_t flags = BOTH) noexcept
            {
                if (flags & BOTH)
                    return std::get<0>(lhs) > std::get<0>(rhs) && std::get<1>(lhs) > std::get<1>(rhs);
                else if (flags & START)
                    return std::get<0>(lhs) > std::get<0>(rhs);
                else if (flags & END)
                    return std::get<1>(lhs) > std::get<1>(rhs);
                else
                    return false;
            }

            template <typename U>
                requires std::same_as<tuple_type, std::remove_cvref_t<U>> || convertible_tuple_v<U>
            static constexpr inline bool le(const U&& lhs, const U&& rhs, uint8_t flags = BOTH) noexcept
            {
                if (flags & BOTH)
                    return std::get<0>(lhs) <= std::get<0>(rhs) && std::get<1>(lhs) <= std::get<1>(rhs);
                else if (flags & START)
                    return std::get<0>(lhs) <= std::get<0>(rhs);
                else if (flags & END)
                    return std::get<1>(lhs) <= std::get<1>(rhs);
                else
                    return false;
            }

            template <typename U>
                requires std::same_as<tuple_type, std::remove_cvref_t<U>> || convertible_tuple_v<U>
            static constexpr inline bool ge(const U&& lhs, const U&& rhs, uint8_t flags = BOTH) noexcept
            {
                if (flags & BOTH)
                    return std::get<0>(lhs) >= std::get<0>(rhs) && std::get<1>(lhs) >= std::get<1>(rhs);
                else if (flags & START)
                    return std::get<0>(lhs) >= std::get<0>(rhs);
                else if (flags & END)
                    return std::get<1>(lhs) >= std::get<1>(rhs);
                else
                    return false;
            }

#if 0
            template <typename U>
                requires std::same_as<tuple_type, std::remove_cvref_t<U>> || convertible_tuple_v<U>
            using helper_func_type1 = bool (*)(const U&&, const U&&, uint8_t);

            template <typename U>
                requires std::same_as<tuple_type, std::remove_cvref_t<U>> || convertible_tuple_v<U>
            using helper_func_type2 = std::function<bool(const U&&, const U&&, uint8_t)>;
#endif

            template <typename U, typename Fn>
                requires (std::same_as<tuple_type, std::remove_cvref_t<U>> || convertible_tuple_v<U>) && std::invocable<Fn, const U&&, const U&&, uint8_t>
            constexpr inline bool operator()(Fn&& func, const U&& lhs, const U&& rhs, uint8_t flags = BOTH) const
                noexcept(
                    noexcept(func(std::forward<const U>(lhs), std::forward<const U>(rhs), flags))
                )
            {
                return func(std::forward<const U>(lhs), std::forward<const U>(rhs), flags);
            }

            template <typename U, typename Fn>
                requires (std::same_as<tuple_type, std::remove_cvref_t<U>> || convertible_tuple_v<U>) && std::invocable<Fn, const U&&, const U&&, uint8_t>
            struct Comparator
            {
                Fn func;
                uint8_t flags;

                Comparator(Fn&& func, uint8_t flags = BOTH) : func(std::forward<Fn>(func)), flags(flags) {}
                Comparator(const Fn& func, uint8_t flags = BOTH) : func(func), flags(flags) {}

                constexpr inline bool operator()(const U&& lhs, const U&& rhs) const
                    noexcept(
                        noexcept(func(std::forward<const U>(lhs), std::forward<const U>(rhs), flags))
                    )
                {
                    return func(std::forward<const U>(lhs), std::forward<const U>(rhs), flags);
                }
            };

            template <typename U, typename Fn>
                requires (std::same_as<tuple_type, std::remove_cvref_t<U>> || convertible_tuple_v<U>) && std::invocable<Fn, const U&&, const U&&, uint8_t>
            static constexpr inline Comparator<U, Fn> make_comparator(Fn&& func, uint8_t flags = BOTH)
            {
                return Comparator<U, Fn>(std::forward<Fn>(func), flags);
            }
    };

    static_assert(PragmaLocCompare<char>::eq(std::make_tuple(0, 0), std::make_tuple(0, 0)));
    static_assert(PragmaLocCompare<char>::lt(std::make_tuple(0, 0), std::make_tuple(2, 3)));
    static_assert(!PragmaLocCompare<char>::gt(std::make_tuple(0, 1), std::make_tuple(0, 0)));
    static_assert(PragmaLocCompare<char>()([](auto&& lhs, auto&& rhs, uint8_t flags) { return PragmaLocCompare<char>::eq(std::forward<decltype(lhs)>(lhs), std::forward<decltype(rhs)>(rhs), flags); }, std::make_tuple(0, 0), std::make_tuple(0, 0)));

#ifdef PRAGLOC_COMPARE_REL
    #undef PRAGLOC_COMPARE_REL
#endif
#define PRAGLOC_COMPARE_REL(TP, OP, TUPLE1, TUPLE2, ...) (::SupDef::PragmaLocCompare<TP>::OP((TUPLE1), (TUPLE2) __VA_OPT__(,) __VA_ARGS__))

    static_assert(PRAGLOC_COMPARE_REL(char, eq, std::make_tuple(0, 0), std::make_tuple(0, 0)));
    static_assert(PRAGLOC_COMPARE_REL(char, lt, std::make_tuple(0, 0), std::make_tuple(2, 3)));
    static_assert(!PRAGLOC_COMPARE_REL(char, gt, std::make_tuple(0, 1), std::make_tuple(0, 0)));

#ifdef PRAGLOC_COMPARE_FN
    #undef PRAGLOC_COMPARE_FN
#endif
#define PRAGLOC_COMPARE_FN(TP, FN, TUPLE1, TUPLE2, ...) (::SupDef::PragmaLocCompare<TP>()((FN), (TUPLE1), (TUPLE2) __VA_OPT__(,) __VA_ARGS__))
    
    static_assert(PRAGLOC_COMPARE_FN(char, [](auto&& lhs, auto&& rhs, uint8_t flags) { return PRAGLOC_COMPARE_REL(char, eq, std::forward<decltype(lhs)>(lhs), std::forward<decltype(rhs)>(rhs), flags); }, std::make_tuple(0, 0), std::make_tuple(0, 0)));

#ifdef PRAGLOC_COMPARATOR_FN
    #undef PRAGLOC_COMPARATOR_FN
#endif
#define PRAGLOC_COMPARATOR_FN(TP_CHAR, TP_TUPLED, FN, ...) (::SupDef::PragmaLocCompare<TP_CHAR>::template make_comparator<std::tuple<TP_TUPLED, TP_TUPLED>>((FN) __VA_OPT__(,) __VA_ARGS__))

#ifdef PRAGLOC_COMPARATOR_REL
    #undef PRAGLOC_COMPARATOR_REL
#endif
#define PRAGLOC_COMPARATOR_REL(TP_CHAR, TP_TUPLED, OP, ...) (PRAGLOC_COMPARATOR_FN(TP_CHAR, TP_TUPLED, [](auto&& lhs, auto&& rhs, uint8_t flags) { return PRAGLOC_COMPARE_REL(TP_CHAR, OP, std::forward<decltype(lhs)>(lhs), std::forward<decltype(rhs)>(rhs), flags); } __VA_OPT__(,) __VA_ARGS__))

#if SUPDEF_WORKAROUND_GCC_INTERNAL_ERROR
    template <typename T>
        requires CharacterType<T>
    Coro<Result<std::shared_ptr<std::basic_string<T>>, Error<T, std::filesystem::path>>> search_imports(Parser<T>& parser);
#endif
    /**
     * @class Parser
     * @brief A class representing a SupDef parser
     * @tparam T The character type of the parser (char, wchar_t, char8_t, char16_t, char32_t)
     * @details The @class Parser is used to parse a file and extract SupDef pragmas from it
     * TODO: Keep track of removed lines count to be able to report errors with line numbers properly (DONE)
     * TODO: Completely remove "raw" `file_content` and `lines` vectors
     */
    template <typename T>
        requires CharacterType<T>
    class Parser
    {
        public:
            typedef typename std::basic_string<T> string_type;
            typedef typename std::tuple<string_type, string_size_type<T>, string_size_type<T>> pragma_loc_type;

            std::basic_string<T> file_content_raw;
            std::vector<std::basic_string<T>> lines_raw;

            Parser();
            Parser(std::filesystem::path file_path); // Initialize this->file with std::basic_ifstream<T>
            Parser(std::filesystem::path file_path, std::basic_ifstream<T>& file_stream); // Initialize this->file with std::reference_wrapper<std::basic_ifstream<T>>
            ~Parser() noexcept = default;

            std::basic_string<T> slurp_file();
            Parser& strip_comments(void);
#if SUPDEF_WORKAROUND_GCC_INTERNAL_ERROR
        private:
            friend Coro<Result<std::shared_ptr<std::basic_string<T>>, Error<T, std::filesystem::path>>> search_imports<T>(Parser& parser);
        public:
#else
            // Search for `#pragma supdef import ...` pragmas
            Coro<Result<Parser<T>::pragma_loc_type, Error<T, std::filesystem::path>>> search_imports(void);
#endif
            // Search for `#pragma supdef begin ...` and its corresponding `#pragma supdef end` pragmas
            Coro<Result<Parser<T>::pragma_loc_type, Error<T, std::filesystem::path>>> search_super_defines(void);
#if defined(SUPDEF_DEBUG)
            template <typename Stream = std::ostream>                
            void print_content(Stream& s = std::cout) const;
#endif
        private:
            typedef typename std::basic_ifstream<T>::pos_type pos_type;
            typedef typename std::basic_string<T>::size_type location_type;
            typedef std::basic_ifstream<T> file_stream_type1;
            typedef std::reference_wrapper<std::basic_ifstream<T>> file_stream_type2;
            typedef std::variant<file_stream_type1, file_stream_type2> file_stream_variant_type;

            file_stream_variant_type file;
            
            std::basic_ifstream<T>& get_file_stream(void);
            std::basic_string<T> remove_cstr_lit(void);

            // TODO: Implement the following three methods
            bool is_super_define_start(std::vector<ParsedChar<T>>& line);
            bool is_super_define_end(std::vector<ParsedChar<T>>& line);
            bool is_import(std::vector<ParsedChar<T>>& line);
#if SUPDEF_WORKAROUND_GCC_INTERNAL_ERROR
        public:
#endif
            std::filesystem::path file_path;
            std::vector<ParsedChar<T>> file_content;
            std::vector<std::vector<ParsedChar<T>>> lines;
            //std::shared_ptr<std::basic_string<T>> reassemble_lines(void);
    };

#undef NEED_Parser_TEMPLATES
#define NEED_Parser_TEMPLATES 1
#include <sup_def/common/parser.cpp>

    /**
     * @brief A class representing a SupDef engine
     * 
     * @tparam P1 The character type of the manipulated files (char, wchar_t, char8_t, char16_t, char32_t)
     * @tparam P2 The type of the path of the manipulated files (std::filesystem::path, std::basic_string<char>, std::basic_string<wchar_t>, std::basic_string<char8_t>, std::basic_string<char16_t>, std::basic_string<char32_t>)
     */
    template <typename P1, typename P2>
        requires CharacterType<P1> && FilePath<P2>
    class Engine : public EngineBase
    {
        public:
            using src_file_t = std::shared_ptr<SrcFile<P1, P2>>;
            using dst_file_t = File<std::basic_ofstream<P1>>;
            using tmp_file_t = File<std::basic_fstream<P1>>;
            
            using target_t = std::tuple<src_file_t, dst_file_t, tmp_file_t, bool>;

        private:
            std::unordered_map<std::shared_ptr<SrcFile<P1, P2>>, Parser<P1>> parser_pool;
            ThreadPool thread_pool;            
            std::vector<target_t> targets;

        public:
            Engine();
            template <typename T, typename U>
                requires FilePath<std::remove_cvref_t<T>> && FilePath<std::remove_cvref_t<U>>
            Engine(T&& src, U&& dst);

            ~Engine() noexcept;

#ifdef ADD_INC_PATH
    #undef ADD_INC_PATH
#endif
#define ADD_INC_PATH(PATH) SupDef::EngineBase::add_include_path(PATH)

#ifdef DEL_INC_PATH
    #undef DEL_INC_PATH
#endif
#define DEL_INC_PATH(PATH) SupDef::EngineBase::remove_include_path(PATH)

#ifdef CLR_INC_PATH
    #undef CLR_INC_PATH
#endif
#define CLR_INC_PATH() SupDef::EngineBase::clear_include_paths()

#ifdef GET_INC_PATH
    #undef GET_INC_PATH
#endif
#define GET_INC_PATH() SupDef::EngineBase::get_include_paths()

#ifdef SET_INC_PATH
    #undef SET_INC_PATH
#endif
#define SET_INC_PATH(PATHS) SupDef::EngineBase::set_include_paths(PATHS)

            void restart();
            
            template <typename T, typename U>
                requires FilePath<std::remove_cvref_t<T>> && FilePath<std::remove_cvref_t<U>>
            void restart(T&& src, U&& dst);
    };

#undef NEED_Engine_TEMPLATES
#define NEED_Engine_TEMPLATES 1
#include <sup_def/common/engine.cpp>

#undef NEED_Pragmas_TEMPLATES
#define NEED_Pragmas_TEMPLATES 1
#include <sup_def/common/pragmas.cpp>

    template <typename T, bool authorize_numbers = true>
        requires CharacterType<T>
    [[gnu::const]]
    constexpr inline bool is_ident_char(T c) noexcept
    {
        const std::array<char, 26> lowercase_alphabet = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
                                                          'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
                                                          's', 't', 'u', 'v', 'w', 'x', 'y', 'z' };
        const std::array<char, 26> uppercase_alphabet = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
                                                          'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',
                                                          'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z' };
        const std::array<char, 10> numbers = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

        auto gen_table = [&]() -> const std::array<T, std::conditional_t<authorize_numbers, std::integral_constant<size_t, 62>, std::integral_constant<size_t, 52>>::value>
        {
            std::array<T, std::conditional_t<authorize_numbers, std::integral_constant<size_t, 62>, std::integral_constant<size_t, 52>>::value> table = {};
            for (size_t i = 0; i < 26; ++i)
            {
                table[i] = CONVERT(T, lowercase_alphabet[i]);
                table[i + 26] = CONVERT(T, uppercase_alphabet[i]);
            }
            if (!authorize_numbers) return table;
            for (size_t i = 0; i < 10; ++i)
                table[i + 2*26] = CONVERT(T, numbers[i]);
            return table;
        };
        auto is_in = [&](T c) -> bool
        {
            for (auto&& ch : gen_table())
                if (SAME(c, ch))
                    return true;
            return false;
        };
        return SAME(c, '_') || is_in(c);
    }

    template <typename T>
        requires CharacterType<T>
    inline string_size_type<T> skip_whitespaces(std::basic_string<T> str, string_size_type<T> cursor_pos) noexcept
    {
        while (cursor_pos < str.size() && std::isspace(str[cursor_pos], std::locale()))
            ++cursor_pos;
        return cursor_pos;
    }

}

#if !defined(ENGINE)
    #define ENGINE SupDef::Engine<char, std::filesystem::path>
#endif

#undef NEED_ExpInst_TEMPLATES
#define NEED_ExpInst_TEMPLATES 1
#include <sup_def/common/exp_inst.cpp>

#endif


#include <sup_def/common/end_header.h>