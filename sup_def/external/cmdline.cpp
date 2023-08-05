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

#include <getopt.h>

#include <sup_def/common/sup_def.hpp>
#include <sup_def/external/external.hpp>

namespace SupDef
{
    namespace External
    {
        CmdLine::CmdLine(int argc, char** argv)
        {
            this->argc = argc;
            this->argv = argv;
        }

        CmdLine::CmdLine(int argc, const char* argv[])
        {
            this->argc = argc;
            this->argv = const_cast<char**>(argv);
        }

        void CmdLine::update_engine(void)
        {
            Engine::clear_include_paths();
            for (std::filesystem::path inc : this->include_paths)
                Engine::add_include_path<std::filesystem::path>(inc);
        }

        void CmdLine::parse(void)
        {
            int c;
            while ((c = getopt(this->argc, this->argv, "I:o:")) != -1)
            {
                switch (c)
                {
                    case 'I':
                        if (!std::filesystem::exists(get_normalized_path(std::filesystem::path(optarg))))
                            throw Exception<char>(ExcType::INVALID_FILE_PATH_ERROR, "Path \"" + std::string(optarg) + "\" does not exist");
                        this->include_paths.push_back(get_normalized_path(std::filesystem::path(optarg)));
                        break;
                    case 'o':
                        this->output_file = get_normalized_path(std::filesystem::path(optarg));
                        break;
                    default:
                        break;
                }
            }

            if (optind < this->argc && this->argv[optind] != nullptr 
                    && std::filesystem::exists(get_normalized_path(std::filesystem::path(this->argv[optind]))))
                this->input_file = get_normalized_path(std::filesystem::path(this->argv[optind]));
            else if (optind < this->argc && this->argv[optind] != nullptr)
                throw Exception<char>(ExcType::INVALID_FILE_PATH_ERROR, "Path \"" + std::string(this->argv[optind]) + "\" does not exist");
            else
                throw Exception<char>(ExcType::NO_INPUT_FILE_ERROR, "Please specify an input file");
        }

    }
}