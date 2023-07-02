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

#ifndef SUP_DEF_HPP
#define SUP_DEF_HPP

#if !defined(_GNU_SOURCE)
    #define _GNU_SOURCE
#endif

#if defined(_WIN32)
    #include <windows.h>
#endif

#include <cstdint>

#include <fstream>
#include <filesystem>
#include <concepts>
#include <type_traits>
#include <vector>
#include <string>
#include <stdexcept>
#include <exception>
#include <unordered_map>

#if defined(SUPDEF_PRAGMA_NAME)
    #undef SUPDEF_PRAGMA_NAME
#endif
#define SUPDEF_PRAGMA_NAME "supdef"

#if defined(SUPDEF_PRAGMA_START)
    #undef SUPDEF_PRAGMA_START
#endif
#define SUPDEF_PRAGMA_START "start"

#if defined(SUPDEF_PRAGMA_END)
    #undef SUPDEF_PRAGMA_END
#endif
#define SUPDEF_PRAGMA_END "end"

namespace SupDef
{
    template <typename T>
    concept CharacterType = std::same_as<char, T>       ||
                            std::same_as<wchar_t, T>    ||
                            std::same_as<char8_t, T>    ||
                            std::same_as<char16_t, T>   ||
                            std::same_as<char32_t, T>;

    template <typename T>
    concept FilePath = std::same_as<std::filesystem::path, T>   || 
                       CharacterType<std::remove_cv_t<T>>;

    template <typename T>
        requires CharacterType<T>
    using string_size_type = std::basic_string<T>::size_type;
    /*
     * A SupDef pragma has the form:
     *
     * #pragma supdef start <super_define_name>(<super_define_args>)
     * <super_define_body>
     * #pragma supdef end
     * 
     * Where:
     * - <super_define_name> is the name of the super define (it is a classic C(++) identifier / macro name)
     * - <super_define_args> is a comma separated list of arguments (it is a classic C(++) macro argument list)
     * - <super_define_body> is the body of the super define, and has the form:
     *  <super_define_body> ::= <super_define_body_line>*
     * <super_define_body_line> ::= ["] <super_define_body_line_content> ["] [\n]
     * <super_define_body_line_content> ::= <super_define_body_line_content_char>*
     * <super_define_body_line_content_char> ::= [anything except " (which has to be escaped with \") and \n]
    */
    template <typename T>
        requires CharacterType<T>
    class Pragma
    {
        private:
            std::basic_string<T>* name;
            std::basic_string<T>* body;
            std::vector<std::basic_string<T>>* args;

        public:
            Pragma();
            Pragma(const std::basic_string<T>& full_pragma);
            ~Pragma() noexcept;

            std::basic_string<T> get_name() const noexcept;
            std::basic_string<T> get_body() const noexcept;
            std::vector<std::basic_string<T>> get_args() const noexcept;
            std::vector<std::basic_string<T>>::size_type get_argc() const noexcept;
    };

    class Parser
    {
        private:
            typedef char private_parsed_char_t;
            typedef std::basic_ifstream<private_parsed_char_t>::pos_type pos_type;
            typedef std::basic_string<private_parsed_char_t>::size_type location_type;

            std::basic_ifstream<private_parsed_char_t>* file;
            pos_type last_error_pos;
            std::unordered_map<location_type, Pragma<private_parsed_char_t>>* pragmas;
            enum class State : uint64_t
            {
                OK = 0,
                INTERNAL_ERROR = 1 << 0,
                INVALID_PRAGMA_ERROR = 1 << 1,
                INVALID_MACRO_ARGC_ERROR = 1 << 2
            };

        public:
            typedef private_parsed_char_t parsed_char_t;
            
            std::basic_string<parsed_char_t>* file_content;

            Parser();
            Parser(std::filesystem::path file_path);
            ~Parser() noexcept;
            
            std::basic_string<parsed_char_t>* slurp_file();
            Parser& strip_comments(void);
            std::vector<std::vector<std::basic_string<parsed_char_t>::size_type>> locate_pragmas(void);
    };

    class Engine
    {
        private:
            std::filesystem::path tmp_file_path;
            class Parser* parser;

        public:
            std::filesystem::path src_file_path;
            std::filesystem::path dst_file_path;
            
            Engine();
            template <typename T, typename U>
                requires FilePath<T> && FilePath<U>
            Engine(const T *const src_file_name, const U *const dst_file_name);

            ~Engine() noexcept;

            void restart();
            template <typename T, typename U>
                requires FilePath<T> && FilePath<U>
            void restart(const T *const src_file_name, const U *const dst_file_name);
    };

    template <typename T>
        requires CharacterType<T>
    inline bool is_ident_char(T c, bool authorize_numbers = true) noexcept
    {
        return c == static_cast<T>('_') || std::isalpha(c, std::locale()) || (authorize_numbers && std::isdigit(c, std::locale()));
    }

    template <typename T>
        requires CharacterType<T>
    inline string_size_type<T> skip_whitespaces(std::basic_string<T> str, string_size_type<T> cursor_pos) noexcept
    {
        while (cursor_pos < str.size() && std::isspace(str[cursor_pos], std::locale()))
            ++cursor_pos;
        return cursor_pos;
    }

    /* template <>
    inline string_size_type<char> skip_whitespaces(std::basic_string<char> str, string_size_type<char> cursor_pos) noexcept
    {
        while (cursor_pos < str.size() && std::isspace(str[cursor_pos], std::locale()))
            ++cursor_pos;
        return cursor_pos;
    }

    template <>
    inline string_size_type<wchar_t> skip_whitespaces(std::basic_string<wchar_t> str, string_size_type<wchar_t> cursor_pos) noexcept
    {
        while (cursor_pos < str.size() && std::iswspace(str[cursor_pos], std::locale()))
            ++cursor_pos;
        return cursor_pos;
    } */

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
}

#endif
