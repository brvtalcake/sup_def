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

namespace SupDef
{
    namespace External
    {
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
        class SD_EXTERNAL_API CmdLine
        {
            public:
                std::vector<std::filesystem::path> include_paths;
                std::filesystem::path input_file;
                std::filesystem::path output_file;

                CmdLine(int argc, char** argv);
                CmdLine(int argc, const char* argv[]);
                ~CmdLine() = default;

                void parse(void);
                void update_engine(void);

            private:
                int argc;
                char** argv;
        };

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