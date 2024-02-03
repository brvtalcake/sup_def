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

#if defined(SUPDEF_DEBUG)
template <typename T>
    requires CharacterType<T>
template <typename Stream>
void Parser<T>::print_content(Stream& s) const
{
    if (this->file_content_raw.empty())
        throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "File content is empty\n");
    std::string content = {};
    for (auto&& pc : this->file_content)
        content += CONVERT(char, pc.val());
    s << content;
    if constexpr (std::same_as<Stream, std::ostream>)
    {
        if (
            SAME_ANY(s, std::cout) ||
            SAME_ANY(s, std::cerr) ||
            SAME_ANY(s, std::clog)
        )
            s << "\n";
        s.flush();
    }
}
#endif