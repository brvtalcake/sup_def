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

#include <optional>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <concepts>
#include <type_traits>
#include <vector>
#include <string>
#include <stdexcept>
#include <map>
#include <algorithm>
#include <iostream>
#include <locale>

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
    concept CharacterType = std::same_as<char, std::remove_cv_t<T>>       ||
                            std::same_as<wchar_t, std::remove_cv_t<T>>    ||
                            std::same_as<char8_t, std::remove_cv_t<T>>    ||
                            std::same_as<char16_t, std::remove_cv_t<T>>   ||
                            std::same_as<char32_t, std::remove_cv_t<T>>;
    
    template <typename T>
    concept StdStringType = requires(T)
    {
        typename T::value_type;
        typename T::size_type;
        typename T::traits_type;
        typename T::allocator_type;
    };

    template <typename T>
    concept StringType = ( std::is_pointer_v<T>                                                         &&
                         CharacterType<std::remove_cv_t<std::remove_pointer_t<std::remove_cv_t<T>>>> )  ||
                         ( StdStringType<std::remove_cv_t<T>>                                           &&
                         CharacterType<typename std::remove_cv_t<T>::value_type> );

    template <typename T>
    concept FilePath = std::same_as<std::filesystem::path, T>   || 
                       StringType<T>;

    template <typename T>
        requires CharacterType<T>
    using string_size_type = std::basic_string<T>::size_type;

    typedef uint64_t parser_state_underlying_type;

    enum ParserState : parser_state_underlying_type
    {
        OK = 0,
        INTERNAL_ERROR = 1 << 0,
        INVALID_PRAGMA_ERROR = 1 << 1,
        INVALID_MACRO_ARGC_ERROR = 1 << 2
    };

    /**
     * @class CmdLine
     * @brief A class representing the command line arguments, and used to parse them
     * @details A typical invocation of SupDef is: @code sup_def [-I "<include_paths>"]* [-o <output_file>] <input_file> @endcode, with:
     * - @code -I <include_paths> @endcode being an array of include paths, separated by a colon (:) (and as one argument, i.e. surrounded by quotes). There may be multiple -I arguments
     * - @code -o <output_file> @endcode being the output file
     * - @code <input_file> @endcode being the input file being processed
     * 
     * Example:
     * @code sup_def -I "$HOME/my/project/path/include:../../other/project/include" -o ./output_file.c ./input_file.c @endcode
     */
    class CmdLine
    {
        public:
            std::vector<std::filesystem::path> include_paths;
            std::filesystem::path input_file;
            std::filesystem::path output_file;

            CmdLine(int argc, char** argv);
            ~CmdLine() = default;

            void parse(void);
            void update_engine(void);

        private:
            int argc;
            char** argv;
    };

    class TmpFile
    {
        private:
            static inline size_t get_counter(void)
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
     * 
     * Example:
     * #pragma supdef start MY_SUPER_DEFINE(arg1, arg2)
     * "#define TEST \"This is the body of my super define which takes 2 arguments: arg1 and arg2\""
     * #pragma supdef end
     * 
     * Result:
     * #define TEST "This is the body of my super define which takes 2 arguments: arg1 and arg2"
     */

    /**
     * @class Pragma
     * @brief A class representing a SupDef pragma definition
     * @tparam T The character type of the pragma (char, wchar_t, char8_t, char16_t, char32_t)
     */
    template <typename T>
        requires CharacterType<T>
    class Pragma
    {
        private:
            std::basic_string<T>* name;
            std::vector<std::basic_string<T>>* body_lines;
            std::vector<std::basic_string<T>>* args;

        public:
            Pragma();
            Pragma(const std::vector<std::basic_string<T>>& full_pragma);
            ~Pragma() noexcept;

            std::basic_string<T> get_name() const noexcept;
            std::basic_string<T> get_body() const noexcept;
            std::vector<std::basic_string<T>> get_args() const noexcept;
            std::vector<std::basic_string<T>>::size_type get_argc() const noexcept;
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
     */
    class Parser
    {
        public:
            typedef char parsed_char_t;
            typedef std::vector<string_size_type<parsed_char_t>> pragma_loc_t;

            std::basic_string<parsed_char_t>* file_content;
            std::vector<std::basic_string<parsed_char_t>>* lines;

            Parser();
            Parser(std::filesystem::path file_path);
            ~Parser() noexcept;
            
            std::basic_string<parsed_char_t>* slurp_file();
            Parser& strip_comments(void);
            Parser& process_file(void);
            std::vector<std::vector<std::basic_string<parsed_char_t>::size_type>> locate_pragmas(void);
        private:
            typedef std::basic_ifstream<parsed_char_t>::pos_type pos_type;
            typedef std::basic_string<parsed_char_t>::size_type location_type;

            std::basic_ifstream<parsed_char_t>* file;
            pos_type last_error_pos;
            std::map<pragma_loc_t, Pragma<parsed_char_t>, PragmaLocCompare<parsed_char_t>> pragmas;
    };

    class Engine
    {
        private:
            std::pair<std::filesystem::path, std::ofstream*> tmp_file;
            class Parser* parser;

        public:
            static std::vector<std::filesystem::path> include_paths;
            std::pair<std::filesystem::path, std::ifstream*> src_file;
            std::pair<std::filesystem::path, std::ofstream*> dst_file;
            
            Engine();
            template <typename T, typename U>
                requires FilePath<T> && FilePath<U>
            Engine(T src_file_name, U dst_file_name);

            ~Engine() noexcept;

            template <typename T>
                requires FilePath<T>
            static inline void add_include_path(T path)
            {
                if (!std::filesystem::exists(path))
                    throw std::runtime_error("Include path does not exist");
                include_paths.push_back(path);
            }

            template <typename T>
                requires FilePath<T>
            static inline void remove_include_path(T path)
            {
                if (!std::filesystem::exists(path))
                    throw std::runtime_error("Include path does not exist");
                auto it = std::find(include_paths.begin(), include_paths.end(), path);
                if (it != include_paths.end())
                    include_paths.erase(it);
            }

            static inline void clear_include_paths(void)
            {
                include_paths.clear();
            }

            void restart();
            
            template <typename T, typename U>
                requires FilePath<T> && FilePath<U>
            void restart(T src_file_name, U dst_file_name);
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

    template <typename T>
        requires CharacterType<T>
    inline std::vector<std::basic_string<T>> split_string(std::basic_string<T> str, T delimiter) noexcept
    {
        std::vector<std::basic_string<T>> result = {};
        std::basic_stringstream<T> ss(str);
        std::basic_string<T> token;
        while (std::getline(ss, token, delimiter))
            result.push_back(token);
        return result;
    }

    inline std::optional<std::filesystem::path> get_included_fpath(std::filesystem::path file_path)
    {
        for (auto& include_path : Engine::include_paths)
        {
            auto include_file_path = include_path / file_path;
            if (std::filesystem::exists(include_file_path))
                return include_file_path;        
        }
        return std::nullopt;
    }
}

#endif
