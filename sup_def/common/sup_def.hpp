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

#include <cstdint>

#include <fstream>
#include <filesystem>
#include <concepts>
#include <type_traits>

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
                       

    class Parser
    {
        private:
            typedef char internal_parsed_char_t;
            typedef std::basic_ifstream<internal_parsed_char_t>::pos_type pos_type;

            std::basic_ifstream<internal_parsed_char_t>* file;
            pos_type last_error_pos;

        public:
            typedef internal_parsed_char_t parsed_char_t;
            
            std::basic_string<internal_parsed_char_t>* file_content;
            enum class State : uint64_t
            {
                OK = 0,
                INTERNAL_ERROR = 1 << 0,
                INVALID_PRAGMA_ERROR = 1 << 1,
                INVALID_MACRO_ARGC_ERROR = 1 << 2
            };

            Parser();
            Parser(std::filesystem::path file_path);
            ~Parser() noexcept;
            
            std::basic_string<parsed_char_t>* slurp_file();
            Parser& strip_comments(void);
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
}

#endif
