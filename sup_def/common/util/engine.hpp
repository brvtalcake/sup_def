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
#include <sup_def/common/config.h>

#ifndef UTIL_ENGINE_HPP
#define UTIL_ENGINE_HPP

#include <sup_def/common/util/util.hpp>

#include <optional>

namespace SupDef 
{

    class EngineBaseHelper
    {
        public:
            EngineBaseHelper() = default;
            ~EngineBaseHelper() = default;

        protected:
            static std::vector<std::filesystem::path> include_paths;
    };

    class EngineBase : private EngineBaseHelper
    {
        public:
            EngineBase() = default;
            ~EngineBase() = default;

            EngineBase(const EngineBase&) = delete;
            EngineBase(EngineBase&&) = delete;
            EngineBase& operator=(const EngineBase&) = delete;
            EngineBase& operator=(EngineBase&&) = delete;

            template <typename... U>
                requires (FilePath<U> && ...)
            EngineBase(U&&... paths)
            {
                (this->add_include_path(paths), ...);
            }
            
            template <typename U>
                requires FilePath<U>
            static void add_include_path(U path)
            {
                if (!std::filesystem::exists(path))
                    throw Exception<char, std::filesystem::path>(ExcType::INVALID_PATH_ERROR, "Include path does not exist");
                include_paths.push_back(path);
            }

            template <typename U>
                requires FilePath<U>
            static void remove_include_path(U path)
            {
                if (!std::filesystem::exists(path))
                    throw Exception<char, std::filesystem::path>(ExcType::INVALID_PATH_ERROR, "Include path does not exist");
                auto it = std::find(include_paths.begin(), include_paths.end(), path);
                if (it != include_paths.end())
                    include_paths.erase(it);
            }

            static void clear_include_paths(void)
            {
                include_paths.clear();
            }

            static std::vector<std::filesystem::path>& get_include_paths(void)
            {
                return include_paths;
            }

            static void set_include_paths(const std::vector<std::filesystem::path>& paths)
            {
                include_paths = paths;
            }

            static void set_include_paths(std::vector<std::filesystem::path>&& paths)
            {
                include_paths = std::move(paths);
            }
    };

    template <typename P1, typename P2>
        requires CharacterType<P1> && FilePath<P2>
    class Engine;

    namespace Util
    {
        inline std::optional<std::filesystem::path> get_included_fpath(std::filesystem::path file_path)
        {
            for (auto& include_path : ::SupDef::EngineBase::get_include_paths())
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