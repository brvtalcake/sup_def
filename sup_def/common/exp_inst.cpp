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

#if NEED_TPP_INC(ExpInst) == 0
#include <sup_def/common/sup_def.hpp>

EXP_INST_CLASS(::SupDef::Engine,
    (char, std::filesystem::path),
    (char, std::string),
    (char, std::wstring),
    (char, std::u8string),
    (char, std::u16string),
    (char, std::u32string),

    (char8_t, std::filesystem::path),
    (char8_t, std::string),
    (char8_t, std::wstring),
    (char8_t, std::u8string),
    (char8_t, std::u16string),
    (char8_t, std::u32string),

    (char16_t, std::filesystem::path),
    (char16_t, std::string),
    (char16_t, std::wstring),
    (char16_t, std::u8string),
    (char16_t, std::u16string),
    (char16_t, std::u32string),

    (char32_t, std::filesystem::path),
    (char32_t, std::string),
    (char32_t, std::wstring),
    (char32_t, std::u8string),
    (char32_t, std::u16string),
    (char32_t, std::u32string)
);

EXP_INST_CLASS(::SupDef::File,
    (std::fstream),
    (std::ifstream),
    (std::ofstream),
      
    (std::wfstream),
    (std::wifstream),
    (std::wofstream),
      
    (std::basic_fstream<char8_t>),
    (std::basic_ifstream<char8_t>),
    (std::basic_ofstream<char8_t>),
      
    (std::basic_fstream<char16_t>),
    (std::basic_ifstream<char16_t>),
    (std::basic_ofstream<char16_t>),
      
    (std::basic_fstream<char32_t>),
    (std::basic_ifstream<char32_t>),
    (std::basic_ofstream<char32_t>)
);

EXP_INST_CLASS(::SupDef::ParsedChar,
    (char),
    (wchar_t),
    (char8_t),
    (char16_t),
    (char32_t)
);

EXP_INST_CLASS(::SupDef::Parser,
    (char),
    (wchar_t),
    (char8_t),
    (char16_t),
    (char32_t)
);

EXP_INST_STRUCT(::SupDef::PragmaDef,
    (char),
    (wchar_t),
    (char8_t),
    (char16_t),
    (char32_t)
);

EXP_INST_CLASS(::SupDef::ThreadPoolBase,
    (::SupDef::ThreadPool, ::SupDef::ThreadPoolAliases)
);

#if COMPILING_EXTERNAL

EXP_INST_CLASS(::SupDef::External::CmdLine,
    (char*),
    (std::string),
    (std::filesystem::path)
);

#endif

#else

DECL_EXP_INST_CLASS(::SupDef::Engine,
    (char, std::filesystem::path),
    (char, std::string),
    (char, std::wstring),
    (char, std::u8string),
    (char, std::u16string),
    (char, std::u32string),

    (char8_t, std::filesystem::path),
    (char8_t, std::string),
    (char8_t, std::wstring),
    (char8_t, std::u8string),
    (char8_t, std::u16string),
    (char8_t, std::u32string),

    (char16_t, std::filesystem::path),
    (char16_t, std::string),
    (char16_t, std::wstring),
    (char16_t, std::u8string),
    (char16_t, std::u16string),
    (char16_t, std::u32string),

    (char32_t, std::filesystem::path),
    (char32_t, std::string),
    (char32_t, std::wstring),
    (char32_t, std::u8string),
    (char32_t, std::u16string),
    (char32_t, std::u32string)
);

DECL_EXP_INST_CLASS(::SupDef::File,
    (std::fstream),
    (std::ifstream),
    (std::ofstream),
      
    (std::wfstream),
    (std::wifstream),
    (std::wofstream),
      
    (std::basic_fstream<char8_t>),
    (std::basic_ifstream<char8_t>),
    (std::basic_ofstream<char8_t>),
      
    (std::basic_fstream<char16_t>),
    (std::basic_ifstream<char16_t>),
    (std::basic_ofstream<char16_t>),
      
    (std::basic_fstream<char32_t>),
    (std::basic_ifstream<char32_t>),
    (std::basic_ofstream<char32_t>)
);

DECL_EXP_INST_CLASS(::SupDef::ParsedChar,
    (char),
    (wchar_t),
    (char8_t),
    (char16_t),
    (char32_t)
);

DECL_EXP_INST_CLASS(::SupDef::Parser,
    (char),
    (wchar_t),
    (char8_t),
    (char16_t),
    (char32_t)
);

DECL_EXP_INST_STRUCT(::SupDef::PragmaDef,
    (char),
    (wchar_t),
    (char8_t),
    (char16_t),
    (char32_t)
);

DECL_EXP_INST_CLASS(::SupDef::ThreadPoolBase,
    (::SupDef::ThreadPool, ::SupDef::ThreadPoolAliases)
);

#if COMPILING_EXTERNAL

DECL_EXP_INST_CLASS(::SupDef::External::CmdLine,
    (char*),
    (std::string),
    (std::filesystem::path)
);

#endif

#endif