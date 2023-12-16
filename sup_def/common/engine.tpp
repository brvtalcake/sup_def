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

#ifndef INCLUDED_FROM_SUPDEF_SOURCE
    #error "This file may only be included from a C++ SupDef source file, and may not be compiled directly."
#endif
#include <sup_def/common/config.h>

template <typename P1, typename P2>
    requires CharacterType<P1> && FilePath<P2>
template <typename T, typename U>
    requires FilePath<std::remove_cvref_t<T>> && FilePath<std::remove_cvref_t<U>>
Engine<P1, P2>::Engine(T&& src, U&& dst) : EngineBase(), tmp_file(TmpFile::get_tmp_file()), parser_pool(), src_file(), dst_file()
{
    typedef SrcFile<P1, P2> SrcFileType;
    typedef File<std::basic_ofstream<P1>> DstFileType;

    this->src_file = std::make_shared<SrcFileType>(CONVERT(P2, std::forward<T>(src)));
    this->dst_file = DstFileType(CONVERT(std::filesystem::path, std::forward<U>(dst)));
    auto [_, ok] = this->parser_pool.emplace({ this->src_file, Parser<P1>(CONVERT(std::filesystem::path, std::forward<T&>(src)), static_cast<std::basic_ifstream<P1>>(*(this->src_file))) });

}