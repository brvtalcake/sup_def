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

#include <sup_def/common/config.h>

#if NEED_TPP_INC(CmdLine) == 0

#include <sup_def/common/sup_def.hpp>
#include <sup_def/external/external.hpp>

namespace SupDef
{
    namespace External
    {
        template <typename T>
            requires FilePath<T>
        CmdLine<T>::CmdLine(int argc, char** argv)
        {
            this->argc = argc;
            this->argv = argv;
        }

        template <typename T>
            requires FilePath<T>
        CmdLine<T>::CmdLine(int argc, const char* argv[])
        {
            this->argc = argc;
            this->argv = const_cast<char**>(argv);
        }

        EXP_INST_CLASS(CmdLine, (char*), (std::string), (std::filesystem::path))
    };
};

#else

#define INCLUDED_FROM_SUPDEF_SOURCE 1
#include <sup_def/external/cmdline.tpp>
#undef INCLUDED_FROM_SUPDEF_SOURCE

#endif