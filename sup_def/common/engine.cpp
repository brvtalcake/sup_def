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

    Engine::Engine()
    {
        set_app_locale();
        this->src_file = std::pair<std::filesystem::path, std::ifstream*>(std::filesystem::path(), nullptr);
        this->dst_file = std::pair<std::filesystem::path, std::ofstream*>(std::filesystem::path(), nullptr);
        this->tmp_file = std::pair<std::filesystem::path, std::ofstream*>(std::filesystem::path(), nullptr);
        this->parser = new Parser();
    }

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

        auto* src_ifstream = new std::ifstream(src_file_path);
        auto* dst_ofstream = new std::ofstream(dst_file_path);
        auto* tmp_ofstream = new std::ofstream(tmp_file_path);
        if (src_ifstream == nullptr || dst_ofstream == nullptr || tmp_ofstream == nullptr)
            throw std::runtime_error("Failed to allocate memory for file streams");
        this->src_file = std::pair<std::filesystem::path, std::ifstream*>(src_file_path, src_ifstream);
        this->dst_file = std::pair<std::filesystem::path, std::ofstream*>(dst_file_path, dst_ofstream);
        this->tmp_file = std::pair<std::filesystem::path, std::ofstream*>(tmp_file_path, tmp_ofstream);

        this->parser = new Parser(this->src_file.first);
    }

    Engine::~Engine() noexcept 
    {
        delete this->parser;
        if (this->src_file.second != nullptr)
            delete this->src_file.second;
        if (this->dst_file.second != nullptr)
            delete this->dst_file.second;
        if (this->tmp_file.second != nullptr)
            delete this->tmp_file.second;
        this->src_file = std::pair<std::filesystem::path, std::ifstream*>(std::filesystem::path(), nullptr);
        this->dst_file = std::pair<std::filesystem::path, std::ofstream*>(std::filesystem::path(), nullptr);
        this->tmp_file = std::pair<std::filesystem::path, std::ofstream*>(std::filesystem::path(), nullptr);
    }

    void Engine::restart()
    {
        delete this->parser;
        if (this->src_file.second != nullptr)
            delete this->src_file.second;
        if (this->dst_file.second != nullptr)
            delete this->dst_file.second;
        if (this->tmp_file.second != nullptr)
            delete this->tmp_file.second;
        this->src_file = std::pair<std::filesystem::path, std::ifstream*>(std::filesystem::path(), nullptr);
        this->dst_file = std::pair<std::filesystem::path, std::ofstream*>(std::filesystem::path(), nullptr);
        this->tmp_file = std::pair<std::filesystem::path, std::ofstream*>(std::filesystem::path(), nullptr);
        this->parser = new Parser();
    }
    template <typename T, typename U>
        requires FilePath<T> && FilePath<U>
    void Engine::restart(T src_file_name, U dst_file_name)
    {
        delete this->parser;
        if (this->src_file.second != nullptr)
            delete this->src_file.second;
        if (this->dst_file.second != nullptr)
            delete this->dst_file.second;
        if (this->tmp_file.second != nullptr)
            delete this->tmp_file.second;
        
        set_app_locale();
        auto src_file_path = std::filesystem::path(src_file_name);
        auto dst_file_path = std::filesystem::path(dst_file_name);
        auto tmp_file_path = TmpFile::get_tmp_file();

        if (!std::filesystem::exists(src_file_path))
            throw std::runtime_error("Source file does not exist");
        if (std::filesystem::exists(dst_file_path))
            std::filesystem::remove(dst_file_path);

        auto* src_ifstream = new std::ifstream(src_file_path);
        auto* dst_ofstream = new std::ofstream(dst_file_path);
        auto* tmp_ofstream = new std::ofstream(tmp_file_path);
        if (src_ifstream == nullptr || dst_ofstream == nullptr || tmp_ofstream == nullptr)
            throw std::runtime_error("Failed to allocate memory for file streams");
        this->src_file = std::pair<std::filesystem::path, std::ifstream*>(src_file_path, src_ifstream);
        this->dst_file = std::pair<std::filesystem::path, std::ofstream*>(dst_file_path, dst_ofstream);
        this->tmp_file = std::pair<std::filesystem::path, std::ofstream*>(tmp_file_path, tmp_ofstream);

        this->parser = new Parser(this->src_file.first);
    }
}