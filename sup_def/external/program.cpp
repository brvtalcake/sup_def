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


#include <sup_def/common/config.h>

#include <sup_def/common/util/util.hpp>

#include <string>
#include <vector>
#include <iostream>

static std::string _prog_name;
static int _prog_argc;
static std::vector<std::string> _prog_argv;


namespace SupDef
{
    namespace External
    {
        SD_EXTERNAL_API
        void init(int argc, char** argv)
        {
            _prog_name = argv[0];
            _prog_argc = argc;
            _prog_argv = std::vector<std::string>(argv + 1, argv + argc);
        }

        SD_EXTERNAL_API
        void init(int argc, const char** argv)
        {
            _prog_name = argv[0];
            _prog_argc = argc;
            _prog_argv = std::vector<std::string>(argv + 1, argv + argc);
        }

        SD_EXTERNAL_API
        const std::string& get_program_name(void)
        {
            return _prog_name;
        }

        SD_EXTERNAL_API
        int get_program_argc(void)
        {
            return _prog_argc;
        }

        SD_EXTERNAL_API
        const std::vector<std::string>& get_program_argv()
        {
            return _prog_argv;
        }

        SD_EXTERNAL_API
        const std::string& get_program_arg(int index)
        {
            return _prog_argv[index];
        }

        SD_EXTERNAL_API
        const std::string& get_program_arg(unsigned int index)
        {
            return _prog_argv[index];
        }

        SD_EXTERNAL_API
        const std::string& get_program_arg(size_t index)
        {
            return _prog_argv[index];
        }

        SD_EXTERNAL_API
        const std::string& get_program_arg(const std::string& arg)
        {
            for (const auto& a : _prog_argv)
                if (a == arg)
                    return a;
            return _prog_argv[0];
        }

        SD_EXTERNAL_API
        bool has_program_arg(const std::string& arg)
        {
            for (const auto& a : _prog_argv)
                if (a == arg)
                    return true;
            return false;
        }

        SD_EXTERNAL_API
        bool has_program_arg(int index)
        {
            return index < _prog_argc;
        }

        SD_EXTERNAL_API
        bool has_program_arg(const char* arg)
        {
            for (const auto& a : _prog_argv)
                if (a == arg)
                    return true;
            return false;
        }

        SD_EXTERNAL_API
        int main_ret(void)
        {
            std::cerr << "\033[97m";
#if defined(COMPILING_EXTERNAL)
            std::cerr << _prog_name << " exited with code " << SupDef::Util::get_errcount() << std::endl;
#else
            std::cerr << "Program exited with code " << SupDef::Util::get_errcount() << std::endl;
#endif
            // Reset std::cerr to default
            std::cerr << "\033[0m";
            return SupDef::Util::get_errcount();
        }
    }
}