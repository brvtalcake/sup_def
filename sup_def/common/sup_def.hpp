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

// TODO: Use MAP_LIST macro instead of BOOST_PP_SEQ_FOR_EACH if possible
// TODO: Move class declarations to different headers as with a one-class-per-header organization
// TODO: Create a Util namespace and move all the utility / helper functions there

#if !defined(_GNU_SOURCE)
    #define _GNU_SOURCE 1
#endif

#if defined(_WIN32)
    #include <windows.h>
#endif

#include <coroutine>

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

#ifdef COMPILING_EXTERNAL
    #include <sup_def/external/external.hpp>
#endif

namespace SupDef
{
    // To be deleted
    /* enum ParserState : parser_state_underlying_type
    {
        OK = 0,
        INTERNAL_ERROR = 1 << 0,
        INVALID_PRAGMA_ERROR = 1 << 1,
        INVALID_MACRO_ARGC_ERROR = 1 << 2
    }; */

    enum class ExcType : uint8_t
    {
        NO_ERROR = 0,
        INTERNAL_ERROR = 1,
        INVALID_FILE_PATH_ERROR = 2,
        NO_INPUT_FILE_ERROR = 3,
        SYNTAX_ERROR = 4,
        UNSPECIFIED_ERROR = 255
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
    inline 
    std::string 
    format_error
    (
        ExcType type,
        const std::string& error_msg,
        std::optional<std::string> error_type,
        std::optional<U> filepath,
        std::optional<string_size_type<T>> line,
        std::optional<string_size_type<T>> col,
        std::optional<std::basic_string<T>> context
    ) noexcept
    {
        std::string result;
        if (type != ExcType::NO_ERROR)
            result = FG(BRIGHT_RED) + TXT(BOLD) + "[ ERROR n°" + std::to_string(SupDef::Util::get_errcount()) + " ]" + FG(DEFAULT) + (error_type.has_value() ? "  (error type: " + error_type.value() + ")" : "") + "  " + TXT(RESET) + error_msg + "\n";
        else
            result = FG(BRIGHT_MAGENTA) + TXT(BOLD) + "[ WARNING n°" + std::to_string(SupDef::Util::get_warncount()) + " ]" + FG(DEFAULT) + "  " + TXT(RESET) + error_msg + "\n";
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
                    if (context.value().at(i) == ' ' || context.value().at(i) == '\t')
                    {
                        start = i + 1;
                        break;
                    }
                }
                for (string_size_type<T> i = col.value() + 1; i < context.value().size(); ++i)
                {
                    if (context.value().at(i) == ' ' || context.value().at(i) == '\t')
                    {
                        end = i;
                        break;
                    }
                }
                result += " " + TXT(BOLD) + "|" + TXT(RESET) + "  " + context.value().substr(0, start);
                assert(static_cast<long long>(end) - static_cast<long long>(start) >= 0LL);
                result += FG(BRIGHT_RED) + TXT(BOLD) + context.value().substr(start, end - start) + FG(DEFAULT) + TXT(RESET);
                if (static_cast<long long>(context.value().size()) - static_cast<long long>(end) > 0LL)
                    result += context.value().substr(end, context.value().size() - end) + "\n";
                else
                    result += "\n";
                uint16_t start_term_col = std::string(" |  ").size();
                for (string_size_type<T> i = 0; i < start; ++i)
                {
                    if (context.value()[i] == '\t')
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
                result += "  | " + context.value() + "\n";
        }
        return result;
    }

    template <typename T, typename U>
        requires CharacterType<T> && FilePath<U>
    class Exception : public std::exception
    {
        private:
            ExcType type;
            std::string msg;
            std::optional<U> filepath;
            std::optional<string_size_type<T>> line;
            std::optional<string_size_type<T>> col;
            std::optional<std::basic_string<T>> context;

            inline std::optional<std::string> get_type_str(void) const noexcept
            {
                switch (this->type)
                {
                    case ExcType::INTERNAL_ERROR:
                        return std::string("Internal error");
                    case ExcType::INVALID_FILE_PATH_ERROR:
                        return std::string("Invalid file path");
                    case ExcType::NO_INPUT_FILE_ERROR:
                        return std::string("No input file");
                    case ExcType::SYNTAX_ERROR:
                        return std::string("Syntax error");

                    case ExcType::UNSPECIFIED_ERROR:
                        [[fallthrough]];
                    case ExcType::NO_ERROR:
                        [[fallthrough]];
                    default:
                        return std::nullopt;
                }
            }
        public:
            Exception() = default;
            Exception(ExcType type) : type(type), filepath(std::nullopt), line(std::nullopt), col(std::nullopt), context(std::nullopt)
            { this->msg = "An error occured"; }
            Exception(std::string msg) : msg(msg), filepath(std::nullopt), line(std::nullopt), col(std::nullopt), context(std::nullopt)
            { this->type = ExcType::UNSPECIFIED_ERROR; }
            Exception(ExcType type, std::string msg) : type(type), msg(msg), filepath(std::nullopt), line(std::nullopt), col(std::nullopt), context(std::nullopt)
            { }
            Exception(ExcType type, std::string msg, string_size_type<T> line, string_size_type<T> col, std::basic_string<T> context) : type(type), msg(msg), filepath(std::nullopt), line(line), col(col), context(context)
            { }
            Exception(ExcType type, std::string msg, U filepath, string_size_type<T> line, string_size_type<T> col, std::basic_string<T> context) : type(type), msg(msg), filepath(filepath), line(line), col(col), context(context)
            { }

            ~Exception() = default;

            constexpr inline const char* what() const noexcept override { return this->msg.c_str(); }
            constexpr inline ExcType get_type() const noexcept { return this->type; }
            constexpr inline Exception& set_type(ExcType type) noexcept { this->type = type; return *this; }
            inline void report(std::ostream& os = std::cerr) const noexcept
            {
                if (this->type != ExcType::NO_ERROR)
                    SupDef::Util::reg_error();
                else
                    SupDef::Util::reg_warning();
                os << format_error<T, U>(this->type, this->msg, this->get_type_str(), this->filepath, this->line, this->col, this->context);
            }
    };

    template <typename T, typename U>
        requires CharacterType<T> && FilePath<U>
    using Error = Exception<T, U>;

    template <typename T, typename E>
        requires SPECIALIZATION_OF(E, SupDef::Error)
    class Result : public std::variant<T, E>
    {
        private:
            bool null;
        public:
            Result() : std::variant<T, E>(), null(true) {}
            Result(nullptr_t) : std::variant<T, E>(), null(true) {}
            Result(const Result&) = default;
            Result(Result&&) = default;
            Result(T value) : std::variant<T, E>(value) {}
            Result(T& value) : std::variant<T, E>(value) {}
            Result(T&& value) : std::variant<T, E>(std::move(value)) {}
            Result(E error) : std::variant<T, E>(error) {}
            Result(E& error) : std::variant<T, E>(error) {}
            Result(E&& error) : std::variant<T, E>(std::move(error)) {}

            ~Result() = default;

            Result& operator=(const Result&) = default;
            Result& operator=(Result&&) = default;
            Result& operator=(T value) { return *this = Result(value); }
            Result& operator=(T& value) { return *this = Result(value); }
            Result& operator=(T&& value) { return *this = Result(std::move(value)); }
            Result& operator=(E error) { return *this = Result(error); }
            Result& operator=(E& error) { return *this = Result(error); }
            Result& operator=(E&& error) { return *this = Result(std::move(error)); }

            inline bool is_ok(void) const noexcept { return std::holds_alternative<T>(*this); }
            inline bool is_err(void) const noexcept { return std::holds_alternative<E>(*this); }
            inline bool is_null(void) const noexcept { return this->null; }

            template <typename FuncType, typename... Args>
                requires std::invocable<FuncType, E, Args...> && std::same_as<std::invoke_result_t<FuncType, E, Args...>, T>
            constexpr inline T unwrap_or_else(FuncType&& func, Args&&... args) const noexcept(std::is_nothrow_invocable_v<FuncType, E, Args...>)
            {
                if (this->is_ok())
                    return std::get<T>(*this);
                else
                    return std::invoke(std::forward<FuncType>(func), std::get<E>(*this), std::forward<Args>(args)...);
            }

            constexpr inline T unwrap_or(T&& value) const noexcept
            {
                if (this->is_ok())
                    return std::get<T>(*this);
                else
                    return std::move(value);
            }

            constexpr inline T unwrap_or(T& value) const noexcept
            {
                if (this->is_ok())
                    return std::get<T>(*this);
                else
                    return value;
            }

            constexpr inline T unwrap(void) const
            {
                if (this->is_ok())
                    return std::get<T>(*this);
                else
                    throw std::get<E>(*this);
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
                if (this->is_ok())
                    return Result(std::invoke(std::forward<FuncType>(func), std::get<T>(*this), std::forward<Args>(args)...));
                else
                    return Result(std::get<E>(*this));
            }
    };

    inline void set_app_locale(void)
    {
        #if defined(_WIN32)
            LCID lcid = GetThreadLocale();
            wchar_t name[LOCALE_NAME_MAX_LENGTH];
            if (LCIDToLocaleName(lcid, name, LOCALE_NAME_MAX_LENGTH, 0) == 0)
                { std::cerr << "Failed to set locale to current system-locale: error code: " << GetLastError() << std::endl; }
            std::locale::global(name);
        #else
            std::locale::global(std::locale(""));
        #endif
    }

    template <typename T, typename U>
        requires CharacterType<T> && std::convertible_to<U, T>
    inline std::vector<std::basic_string<T>> split_string(std::basic_string<T> str, U delimiter) noexcept(std::is_nothrow_convertible_v<U, T>)
    {
        std::vector<std::basic_string<T>> result{};
        std::basic_stringstream<T> ss(str);
        std::basic_string<T> token;
        T converted_delim;
        if constexpr (std::is_nothrow_convertible_v<U, T>)
            converted_delim = static_cast<T>(delimiter);
        else
        {
            try
            {
                converted_delim = static_cast<T>(delimiter);
            }
            catch (const std::bad_cast& e)
            {
#if defined(__GNUC__)
                throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "Failed to convert delimiter to string character type in function " + std::string(__PRETTY_FUNCTION__) + " (caught exception: " + e.what() + ")");
#else
                throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "Failed to convert delimiter to string character type in function " + std::string(__func__) + " (caught exception: " + e.what() + ")");
#endif
                UNREACHABLE();
                return result;
            }
        
        }
        while (std::getline(ss, token, converted_delim))
            result.push_back(token);
        return result;
    }

    /**
     * @fn inline constexpr std::array<C, S> any_string(const char (&literal)[S])
     * @brief Utility function to ease manipulating strings.
     * 
     * @tparam C The character type of the returned array
     * @tparam S The size of the returned array (deduced from the string literal)
     * @param[in] literal The string literal to convert to an `std::array<C, S>`. Must be of the form: `char[S]` (possibly cv-qualified)
     * @return Returns an `std::array<C, S>` (where `C` is a character type) from a string literal of character type `char`
     * @todo Put it in `SupDef::Util` namespace
     */
    template <typename C, size_t S>
        requires CharacterType<C>
    inline constexpr auto any_string(const char (&literal)[S]) -> std::array<C, S>
    {
        std::array<C, S> r = {};

        for (size_t i = 0; i < S; i++)
                r[i] = static_cast<C>(literal[i]);

        return r;
    }

#if defined(ANY_STRING)
    #undef ANY_STRING
#endif
/**
 * @def ANY_STRING(TYPE, LIT)
 * @brief A wrapper equivalent to `std::basic_string<TYPE>(any_string<TYPE>(LIT).data())`
 * @param TYPE The character type of the string
 * @param LIT The string literal to convert to a `std::basic_string<TYPE>`
 */
#define ANY_STRING(TYPE, LIT) (std::basic_string<TYPE>(any_string<TYPE>(LIT).data()))

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

    /**
     * @brief This class is meant to be the type of parsing coroutines so they can suspend themselves while indicating potential errors to the "master" function calling them.
     */
    template <typename T>
    class Coro
    {
        private:
            // To handle the case where using the iterator with corountine which co_return a value
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
                    constexpr inline std::suspend_always initial_suspend() noexcept { return {}; }
                    constexpr inline std::suspend_always final_suspend() noexcept { return {}; }
                    [[noreturn]] inline void unhandled_exception() { std::rethrow_exception(std::move(std::current_exception())); }
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
            Coro(Coro&& other) : handle(other.handle), done(other.done) { other.handle = nullptr; }
            Coro(const Coro&) = delete;
            ~Coro() 
            {
                if (this->handle)
                    this->handle.destroy();
            }
            Coro& operator=(Coro&& other) 
            {
                this->handle = other.handle;
                this->done = other.done;
                other.handle = nullptr;
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

            struct end_iterator {};

            class Iterator
            {
                public:
                    using iterator_category = std::input_iterator_tag;
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

                    inline bool operator==(const end_iterator& other) const noexcept
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
            end_iterator end_sentinel{};
    };

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

    /**
     * @class PragmaDef
     * @brief A class representing a SupDef pragma definition
     * @tparam T The character type of the pragma (char, wchar_t, char8_t, char16_t, char32_t)
     */
    template <typename T>
        requires CharacterType<T>
    struct PragmaDef
    {
        private:
            std::shared_ptr<std::basic_string<T>> name;
            std::vector<std::shared_ptr<std::basic_string<T>>> body_lines;
            std::vector<std::shared_ptr<std::basic_string<T>>> args;

        public:
            PragmaDef();
            PragmaDef(const std::vector<std::basic_string<T>>& full_pragma);
            ~PragmaDef() noexcept;

            std::shared_ptr<std::basic_string<T>> get_name() const noexcept;
            std::basic_string<T> get_body() const noexcept;
            std::vector<std::shared_ptr<std::basic_string<T>>> get_args() const noexcept;
            std::vector<std::shared_ptr<std::basic_string<T>>>::size_type get_argc() const noexcept;
            std::basic_string<T> substitute() const noexcept;
            std::basic_string<T> substitute(std::basic_string<T> arg1, ...) noexcept(__cpp_lib_unreachable >= 202202L);
    };

    template <typename T, typename U>
        requires CharacterType<T> && FilePath<U>
    class SrcFile;

    template <typename T, typename U>
        requires CharacterType<T> && FilePath<U>
    struct PragmaInc
    {
        private:
            std::shared_ptr<SrcFile<T, U>> included;

        public:
            PragmaInc() = default;
            PragmaInc(U path) : included(std::make_shared<SrcFile<T, U>>(path)) {}
            ~PragmaInc() noexcept = default;

            inline std::shared_ptr<SrcFile<T, U>> get_included() const noexcept
            {
                return this->included;
            }

            inline auto get_path() const noexcept
            {
                return this->included->get_path();
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
    concept FileStream = OutputFileStream<T> || InputFileStream<T>;

    template <typename T>
        requires FileStream<T>
    struct File
    {
        public:
            std::filesystem::path path;
            std::optional<std::unique_ptr<T>> stream;

            File() : path(std::filesystem::path()), stream(std::nullopt) {}
            File(std::filesystem::path path) : path(path), stream(std::make_unique<T>(T(path.c_str()))) {}
            File(std::filesystem::path path, T stream) : path(path), stream(std::unique_ptr<T>(stream)) {}
            File(std::filesystem::path path, std::unique_ptr<T> stream) : path(path), stream(std::move(stream)) {}
            File(const File& other) : path(other.path)
            {
                if (other->stream.has_value() && other->stream.value() != nullptr && other->stream.value()->is_open())
                {
                    auto tmp_stream = other->stream.value()->get();
                    this->stream = std::make_unique<T>(*tmp_stream);
                }
                else
                    this->stream = std::nullopt;
            }
            File(File&& other) noexcept
            {
                this->path = other.path;
                this->stream = std::move(other.stream);
            }
            ~File() noexcept
            {
                if (this->stream.has_value() && this->stream.value() != nullptr && this->stream.value()->is_open())
                    this->stream.value()->close();
            }

            inline auto operator=(const File& other)
            {
                this->path = other.path;
                this->stream = std::make_unique<T>(*other.stream);
                return *this;
            }
            inline auto operator=(File&& other) noexcept
            {
                this->path = other.path;
                this->stream = std::move(other.stream);
                return *this;
            }
            inline auto operator<<(const std::basic_string<T>& str)
            {
                return *this->stream << str;
            }
            template <typename U>
                requires (CharacterType<U> || TermControl<U>) && OutputFileStream<T>
            inline auto operator<<(const U& obj)
            {
                std::string str = "";
                if constexpr (TermControl<U>)
                    str += obj;
                else
                    str = std::to_string(obj);
                return *this->stream << str;
            }

            inline void restart() noexcept
            {
                this->~File();
                this->path = std::filesystem::path();
                this->stream = std::nullopt;
            }

            inline void restart(std::filesystem::path path) noexcept
            {
                this->restart();
                this->path = path;
                this->stream = std::make_unique<T>(T(path.c_str()));
            }

            inline void restart(std::filesystem::path path, T stream) noexcept
            {
                this->restart();
                this->path = path;
                this->stream = std::make_unique<T>(stream);
            }

            inline void restart(std::filesystem::path path, std::unique_ptr<T> stream) noexcept
            {
                this->restart();
                this->path = path;
                this->stream = std::move(stream);
            }

            inline auto get_path() const noexcept
            {
                return this->path;
            }

            inline auto is_open() const noexcept
            {
                return this->stream.has_value() && this->stream.value() != nullptr && this->stream.value()->is_open();
            }
    };

    template <typename T, typename U>
        requires CharacterType<T> && FilePath<U>
    class SrcFile
    {
        private:
            std::shared_ptr<U> path;
            std::vector<PragmaDef<T>> super_defines;
            std::vector<PragmaInc<T, U>> includes;

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
                                      includes(other->get_includes()),
                                      parser(std::make_unique<Parser<T>>(*(other->get_path()))),
                                      file(std::make_unique<File<std::basic_ifstream<T>>>(*(other->get_path())))
            { }
            ~SrcFile() noexcept
            {
                if (this->file->stream.has_value() && this->file->stream.value() != nullptr && this->file->stream.value()->is_open())
                    this->file->stream.value()->close();
            }

            inline std::shared_ptr<U> get_path() const noexcept
            {
                return this->path;
            }

            inline std::vector<PragmaDef<T>> get_super_defines() const noexcept
            {
                return this->super_defines;
            }

            inline std::vector<PragmaInc<T, U>> get_includes() const noexcept
            {
                return this->includes;
            }

            inline void add_super_define(const PragmaDef<T>& super_define) noexcept
            {
                this->super_defines.push_back(super_define);
            }

            inline void add_include(const PragmaInc<T, U>& include) noexcept
            {
                this->includes.push_back(include);
            }

            inline void restart() noexcept
            {
                this->path.reset();
                this->super_defines.clear();
                this->includes.clear();
                this->parser.reset();
                this->file.reset();
            }

            inline void restart(U path) noexcept
            {
                this->restart();
                this->path = std::make_shared<U>(path);
                this->super_defines.clear();
                this->includes.clear();
                this->parser = std::make_unique<Parser<T>>(path);
                this->file = std::make_unique<File<std::basic_ifstream<T>>>(path);
            }

            inline auto operator=(SrcFile&& other) noexcept
            {
                this->path = other.path;
                this->super_defines = other.super_defines;
                this->includes = other.includes;
                this->parser = std::move(other.parser);
                this->file = std::move(other.file);
                return *this;
            }
    };

    // Must have the same behavior as std::less
    template <typename T>
        requires CharacterType<T>
    class PragmaLocCompare
    {
        public:
            bool operator()(const std::vector<string_size_type<T>>& lhs, const std::vector<string_size_type<T>>& rhs) const noexcept
            {
                auto min = std::min(lhs.size(), rhs.size());
                for (typename std::vector<string_size_type<T>>::size_type i = 0; i < min; ++i)
                    if (lhs[i] < rhs[i])
                        return true;
                return false;
            }
    };

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
            /* typedef T parsed_char_t; */
            /* typedef std::vector<string_size_type<parsed_char_t>> pragma_loc_t; */

            std::shared_ptr<std::basic_string<T>> file_content_raw;
            std::vector<std::basic_string<T>> lines_raw;

            Parser();
            Parser(std::filesystem::path file_path);
            ~Parser() noexcept = default;

            std::shared_ptr<std::basic_string<T>> slurp_file();
            Parser& strip_comments(void);
#if 0
            std::vector<std::shared_ptr<std::basic_string<T>>> search_includes(void);
#else
            Coro<Result<std::shared_ptr<std::basic_string<T>>, Error<T, std::filesystem::path>>> search_includes(void);
#endif
#if defined(SUPDEF_DEBUG)
            template <typename Stream>
            void print_content(Stream& s) const;
#endif
            /* Parser& process_file(void); */
            /* std::vector<std::vector<string_size_type<parsed_char_t>> locate_supdefs(void); */
        private:
            typedef std::basic_ifstream<T>::pos_type pos_type;
            typedef std::basic_string<T>::size_type location_type;

            std::unique_ptr<std::basic_ifstream<T>> file;
            std::filesystem::path file_path;
            std::vector<std::tuple<string_size_type<T>, string_size_type<T>, T>> file_content;
            std::vector<std::vector<std::tuple<string_size_type<T>, string_size_type<T>, T>>> lines;
            /* pos_type last_error_pos; */
            /* std::map<pragma_loc_t, PragmaDef<parsed_char_t>, PragmaLocCompare<parsed_char_t>> pragmas; */
            /***/
            
            std::basic_string<T> remove_cstr_lit(void);
            std::shared_ptr<std::basic_string<T>> reassemble_lines(void);
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
    class Engine
    {
        public:
            
        private:
            File<std::basic_ofstream<P1>> tmp_file;

        public:
            static std::vector<std::filesystem::path> include_paths;
            SrcFile<P1, P2> src_file;
            File<std::basic_ofstream<P1>> dst_file;
            
            Engine();
            template <typename T, typename U>
                requires FilePath<T> && FilePath<U>
            Engine(T src_file_name, U dst_file_name);

            ~Engine() noexcept;

            template <typename T>
                requires FilePath<T>
            static void add_include_path(T path)
            {
                if (!std::filesystem::exists(path))
                    throw std::runtime_error("Include path does not exist");
                include_paths.push_back(path);
            }
#ifdef ADD_INC_PATH
    #undef ADD_INC_PATH
#endif
#define ADD_INC_PATH(PATH)                                                  \
    SupDef::Engine<char, std::filesystem::path>::add_include_path(PATH);    \
    SupDef::Engine<wchar_t, std::filesystem::path>::add_include_path(PATH); \
    SupDef::Engine<char8_t, std::filesystem::path>::add_include_path(PATH); \
    SupDef::Engine<char16_t, std::filesystem::path>::add_include_path(PATH);\
    SupDef::Engine<char32_t, std::filesystem::path>::add_include_path(PATH);

            template <typename T>
                requires FilePath<T>
            static void remove_include_path(T path)
            {
                if (!std::filesystem::exists(path))
                    throw std::runtime_error("Include path does not exist");
                auto it = std::find(include_paths.begin(), include_paths.end(), path);
                if (it != include_paths.end())
                    include_paths.erase(it);
            }
#ifdef DEL_INC_PATH
    #undef DEL_INC_PATH
#endif
#define DEL_INC_PATH(PATH)                                                      \
    SupDef::Engine<char, std::filesystem::path>::remove_include_path(PATH);     \
    SupDef::Engine<wchar_t, std::filesystem::path>::remove_include_path(PATH);  \
    SupDef::Engine<char8_t, std::filesystem::path>::remove_include_path(PATH);  \
    SupDef::Engine<char16_t, std::filesystem::path>::remove_include_path(PATH); \
    SupDef::Engine<char32_t, std::filesystem::path>::remove_include_path(PATH);


            static void clear_include_paths(void)
            {
                include_paths.clear();
            }
#ifdef CLR_INC_PATH
    #undef CLR_INC_PATH
#endif
#define CLR_INC_PATH()                                                      \
    SupDef::Engine<char, std::filesystem::path>::clear_include_paths();     \
    SupDef::Engine<wchar_t, std::filesystem::path>::clear_include_paths();  \
    SupDef::Engine<char8_t, std::filesystem::path>::clear_include_paths();  \
    SupDef::Engine<char16_t, std::filesystem::path>::clear_include_paths(); \
    SupDef::Engine<char32_t, std::filesystem::path>::clear_include_paths();

            static auto get_include_paths(void)
            {
                return include_paths;
            }
#ifdef GET_INC_PATH
    #undef GET_INC_PATH
#endif
#define GET_INC_PATH() SupDef::Engine<char, std::filesystem::path>::get_include_paths()

            void restart();
            
            template <typename T, typename U>
                requires FilePath<T> && FilePath<U>
            void restart(T src_file_name, U dst_file_name);
    };

#undef NEED_Engine_TEMPLATES
#define NEED_Engine_TEMPLATES 1
#include <sup_def/common/engine.cpp>

    template <typename T, bool authorize_numbers = true>
        requires CharacterType<T>
    consteval inline bool is_ident_char(T c) noexcept [[gnu::const]]
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
                table[i] = static_cast<T>(lowercase_alphabet[i]);
                table[i + 26] = static_cast<T>(uppercase_alphabet[i]);
            }
            if (!authorize_numbers) return table;
            for (size_t i = 0; i < 10; ++i)
                table[i + 2*26] = static_cast<T>(numbers[i]);
            return table;
        };
        auto is_in = [&](T c) -> bool
        {
            for (auto ch : gen_table())
                if (c == ch)
                    return true;
            return false;
        };
        return c == static_cast<T>('_') || is_in(c);
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

#endif


#include <sup_def/common/end_header.h>