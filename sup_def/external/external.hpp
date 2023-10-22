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


#ifndef EXTERNAL_HPP
#define EXTERNAL_HPP

#include <sup_def/common/config.h>
#include <sup_def/common/sup_def.hpp>

#include <filesystem>
#include <string>
#include <vector>
#include <array>
#include <cstring>
/* #include <unistd.h> */

namespace SupDef
{
    namespace External
    {
        template <typename T>
        concept CStrFilePath = FilePath<T> && CStrType<T>;

        template <typename T>
        concept StdStrFilePath = FilePath<T> && StdStringType<T>;
        /**
         * @class CmdLine
         * @brief A class representing the command line arguments, and used to parse them
         * @details A typical invocation of SupDef is: @code sup_def [-I "<include_paths>"]* [-o <output_file>] <input_file> @endcode, with:
         * - @code -I <include_paths> @endcode being an array of include paths, separated by a colon (:) (and as one argument, i.e. surrounded by quotes).
         * There may be multiple -I arguments
         * - @code -o <output_file> @endcode being the output file
         * - @code <input_file> @endcode being the input file being processed
         * 
         * Example:
         * @code sup_def -I "$HOME/my/project/path/include:../../other/project/include" -o ./output_file.c ./input_file.c @endcode
         */
        template <typename T>
            requires FilePath<T>
        class SD_EXTERNAL_API CmdLine
        {
            public:
                typedef std::conditional_t<CStrType<T>, std::array<std::remove_all_extents_t<std::remove_pointer_t<std::remove_cvref_t<T>>>, 2048>, T> path_type;
                std::vector<path_type> include_paths;
                path_type input_file;
                path_type output_file;

                CmdLine(int argc, char** argv);
                CmdLine(int argc, const char* argv[]);
                ~CmdLine() = default;

                inline constexpr void parse(void);
                inline constexpr void update_engine(void);

            private:
                int argc;
                char** argv;
        };
#if 0

        template <typename T>
            requires CStrFilePath<T>
        class SD_EXTERNAL_API CmdLine<T>
        {
            public:
                std::vector<T> include_paths;
                const T input_file;
                const T output_file;

                CmdLine(int argc, char** argv);
                CmdLine(int argc, const char* argv[]);
                ~CmdLine() = default;

                void parse(void);
                void update_engine(void);

            private:
                int argc;
                char** argv;
        };

        template <typename T>
            requires StdStrFilePath<T>
        class SD_EXTERNAL_API CmdLine<T>
        {
            public:
                std::vector<T> include_paths;
                T input_file;
                T output_file;

                CmdLine(int argc, char** argv);
                CmdLine(int argc, const char* argv[]);
                ~CmdLine() = default;

                void parse(void);
                void update_engine(void);

            private:
                int argc;
                char** argv;
        };

#endif

#undef NEED_CmdLine_TEMPLATES
#define NEED_CmdLine_TEMPLATES 1
#include <sup_def/external/cmdline.cpp>

        SD_EXTERNAL_API
        void init(int argc, char** argv);

        SD_EXTERNAL_API
        void init(int argc, const char** argv);

        SD_EXTERNAL_API
        const std::string& get_program_name(void);

        SD_EXTERNAL_API
        int get_program_argc(void);

        SD_EXTERNAL_API
        const std::vector<std::string>& get_program_argv();

        SD_EXTERNAL_API
        const std::string& get_program_arg(int index);

        SD_EXTERNAL_API
        const std::string& get_program_arg(unsigned int index);

        SD_EXTERNAL_API
        const std::string& get_program_arg(size_t index);

        SD_EXTERNAL_API
        const std::string& get_program_arg(const std::string& arg);

        SD_EXTERNAL_API
        bool has_program_arg(const std::string& arg);

        SD_EXTERNAL_API
        bool has_program_arg(int index);

        SD_EXTERNAL_API
        bool has_program_arg(const char* arg);

        SD_EXTERNAL_API
        int main_ret(void);
    }
}

#endif

#include <sup_def/common/end_header.h>