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

#include <cstring>
#include <locale>

#include <sup_def/common/sup_def.hpp>

namespace SupDef
{
    Engine::Engine()
    {
        /* std::locale::global(std::locale("en_US.UTF-8")); */
        set_app_locale();
        this->src_file_path = std::filesystem::path();
        this->dst_file_path = std::filesystem::path();
        this->parser = new Parser();
    }

    template <typename T, typename U>
        requires FilePath<T> && FilePath<U>
    Engine::Engine(const T *const src_file_name, const U *const dst_file_name)
    {
        /* std::locale::global(std::locale("en_US.UTF-8")); */
        set_app_locale();
        this->src_file_path = std::filesystem::path(src_file_name);
        this->dst_file_path = std::filesystem::path(dst_file_name);
        if (!std::filesystem::exists(this->src_file_path))
            throw std::runtime_error("Source file does not exist");
        if (!std::filesystem::exists(this->dst_file_path))
            std::ofstream(this->dst_file_path);
        else
        {
            std::filesystem::remove(this->dst_file_path);
            std::ofstream(this->dst_file_path);
        }
        this->parser = new Parser(this->src_file_path);
    }

    Engine::~Engine() noexcept 
    {
        delete this->parser;
    }

    void Engine::restart()
    {
        delete this->parser;
        this->src_file_path = std::filesystem::path();
        this->dst_file_path = std::filesystem::path();
        this->parser = new Parser();
    }
    template <typename T, typename U>
        requires FilePath<T> && FilePath<U>
    void Engine::restart(const T *const src_file_name, const U *const dst_file_name)
    {
        delete this->parser;
        this->src_file_path = std::filesystem::path(src_file_name);
        this->dst_file_path = std::filesystem::path(dst_file_name);
        if (!std::filesystem::exists(this->src_file_path))
            throw std::runtime_error("Source file does not exist");
        if (!std::filesystem::exists(this->dst_file_path))
            std::ofstream(this->dst_file_path);
        else
        {
            std::filesystem::remove(this->dst_file_path);
            std::ofstream(this->dst_file_path);
        }
        this->parser = new Parser(this->src_file_path);
    }
}