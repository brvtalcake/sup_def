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

#include <cstdarg>
#include <regex>
#include <utility>

#include <sup_def/common/config.h>

#if NEED_TPP_INC(Pragmas) == 0
#include <sup_def/common/sup_def.hpp>

namespace SupDef
{
    template <typename T>
        requires CharacterType<T>
    PragmaDef<T>::PragmaDef()
    {
        this->name = nullptr;
        this->body_lines.clear();
        this->args.clear();
    }

    template <typename T>
        requires CharacterType<T>
    PragmaDef<T>::PragmaDef(const std::vector<std::basic_string<T>>& full_pragma)
    {
        this->name = nullptr;
        this->body_lines.clear();
        this->args.clear();

        if (full_pragma.size() < 2)
            throw Exception<char, std::filesystem::path>("Invalid pragma definition being parsed in PragmaDef constructor");

        this->name = std::make_shared<std::basic_string<T>>(full_pragma[0]);
        for (auto it = full_pragma.begin() + 1; it != full_pragma.end(); ++it)
            this->body_lines.push_back(std::make_shared<std::basic_string<T>>(*it));
    }

    template <typename T>
        requires CharacterType<T>
    PragmaDef<T>::~PragmaDef() noexcept
    {
        this->name = nullptr;
        this->body_lines.clear();
        this->args.clear();
    }

    template <typename T>
        requires CharacterType<T>
    std::shared_ptr<std::basic_string<T>> PragmaDef<T>::get_name() const noexcept
    {
        return this->name;
    }

    template <typename T>
        requires CharacterType<T>
    std::basic_string<T> PragmaDef<T>::get_body() const noexcept
    {
        std::basic_string<T> body{};
        for (auto& line : this->body_lines)
            body += *(line) + static_cast<T>('\n');
        return body;
    }

    template <typename T>
        requires CharacterType<T>
    std::vector<std::shared_ptr<std::basic_string<T>>> PragmaDef<T>::get_args() const noexcept
    {
        return this->args;
    }

    template <typename T>
        requires CharacterType<T>
    typename std::vector<std::shared_ptr<std::basic_string<T>>>::size_type PragmaDef<T>::get_argc() const noexcept
    {
        return this->args.size();
    }

    /**
     * @brief Substitute the arguments in the body of the pragma
     * @details The pragma variables appear in the body as $0, $1, $2, etc.
     * @tparam T 
     * @return std::basic_string<T> 
     */
    template <typename T>
        requires CharacterType<T>
    std::basic_string<T> PragmaDef<T>::substitute() const noexcept
    {
        return this->get_body();
    }
    //EXP_INST_STRUCT(PragmaDef, (char), (wchar_t), (char8_t), (char16_t), (char32_t))
    EXP_INST_STRUCT(PragmaDef, (char), (wchar_t), (char8_t), (char16_t), (char32_t))
}

#else

#define INCLUDED_FROM_SUPDEF_SOURCE 1
#include <sup_def/common/pragmas.tpp>
#undef INCLUDED_FROM_SUPDEF_SOURCE

#endif