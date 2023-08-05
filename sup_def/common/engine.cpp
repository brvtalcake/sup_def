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

#include <sup_def/common/sup_def.hpp>

namespace SupDef
{
    std::vector<std::filesystem::path> Engine::include_paths;

    Engine::Engine() : tmp_file(), src_files(), dst_file(), parser() { set_app_locale(); }

    template <typename T, typename U>
        requires FilePath<T> && FilePath<U>
    Engine::Engine(T src_file_name, U dst_file_name)
    {
        set_app_locale();
        auto src_file_path = std::filesystem::path(src_file_name);
        auto dst_file_path = std::filesystem::path(dst_file_name);
        auto tmp_file_path = TmpFile::get_tmp_file();

        if (!std::filesystem::exists(src_file_path))
            throw std::runtime_error("Source file does not exist");
        if (std::filesystem::exists(dst_file_path))
            std::filesystem::remove(dst_file_path);

        this->src_files.push_back({src_file_path, std::make_unique<std::ifstream>(src_file_path)});
        this->dst_file = { dst_file_path, std::make_unique<std::ofstream>(dst_file_path) };
        this->tmp_file = { tmp_file_path, std::make_unique<std::ofstream>(tmp_file_path) };

        this->parser = std::make_unique<Parser>(this->src_files[0].path);
    }

    Engine::~Engine() noexcept 
    {
        if (this->src_files.size() > 0)
        for (auto& src_file : this->src_files)
            if (src_file.stream.has_value())
                src_file.stream.value()->close();
        this->src_files.clear();
        if (this->dst_file.stream.has_value())
            this->dst_file.stream.value()->close();
        if (this->tmp_file.stream.has_value())
            this->tmp_file.stream.value()->close();
    }

    void Engine::restart()
    {
        set_app_locale();
        if (this->src_files.size() > 0)
        for (auto& src_file : this->src_files)
            if (src_file.stream.has_value())
                { src_file.stream.value()->close(); src_file.stream->reset(); src_file.stream.reset(); }
        this->src_files.clear();
        if (this->dst_file.stream.has_value())
            { this->dst_file.stream.value()->close(); this->dst_file.stream->reset(); this->dst_file.stream.reset(); }
        if (this->tmp_file.stream.has_value())
            { this->tmp_file.stream.value()->close(); this->tmp_file.stream->reset(); this->tmp_file.stream.reset(); }
        this->parser.reset();
    }

    template <typename T, typename U>
        requires FilePath<T> && FilePath<U>
    void Engine::restart(T src_file_name, U dst_file_name)
    {
        this->restart();
        
        auto src_file_path = std::filesystem::path(src_file_name);
        auto dst_file_path = std::filesystem::path(dst_file_name);
        auto tmp_file_path = TmpFile::get_tmp_file();

        if (!std::filesystem::exists(src_file_path))
            throw std::runtime_error("Source file does not exist");
        if (std::filesystem::exists(dst_file_path))
            std::filesystem::remove(dst_file_path);

        this->src_files.push_back({src_file_path, std::make_unique<std::ifstream>(src_file_path)});
        this->dst_file = { dst_file_path, std::make_unique<std::ofstream>(dst_file_path) };
        this->tmp_file = { tmp_file_path, std::make_unique<std::ofstream>(tmp_file_path) };

        this->parser = std::make_unique<Parser>(this->src_files[0].path);
    }
}