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

#ifndef UTIL_ENGINE_HPP
#define UTIL_ENGINE_HPP

#include <sup_def/common/util/util.hpp>

#include <optional>

namespace SupDef 
{
    namespace Util
    {

#include <sup_def/common/config.h>

        template <typename P1, typename P2>
            requires CharacterType<P1> && FilePath<P2>
        class Engine;

        template <typename P1, typename P2>
            requires CharacterType<P1> && FilePath<P2>
        inline std::optional<std::filesystem::path> get_included_fpath(std::filesystem::path file_path)
        {
            for (auto& include_path : Engine<P1, P2>::get_include_paths())
            {
                auto include_file_path = get_normalized_path(include_path) / file_path;
                if (std::filesystem::exists(include_file_path))
                    return include_file_path;        
            }
            return std::nullopt;
        }
    }
}
#endif

#include <sup_def/common/end_header.h>