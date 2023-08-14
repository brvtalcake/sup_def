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
    template <typename P1, typename P2>
        requires CharacterType<P1> && FilePath<P2>
    std::vector<std::filesystem::path> Engine<P1, P2>::include_paths;

    template <typename P1, typename P2>
        requires CharacterType<P1> && FilePath<P2>
    Engine<P1, P2>::Engine() : tmp_file(), src_file(), dst_file() { set_app_locale(); }

    template <typename P1, typename P2>
        requires CharacterType<P1> && FilePath<P2>
    template <typename T, typename U>
        requires FilePath<T> && FilePath<U>
    Engine<P1, P2>::Engine(T src_file_name, U dst_file_name)
    {
        set_app_locale();
        auto src_file_path = std::filesystem::path(src_file_name);
        auto dst_file_path = std::filesystem::path(dst_file_name);
        auto tmp_file_path = TmpFile::get_tmp_file();

        if (!std::filesystem::exists(src_file_path))
            throw std::runtime_error("Source file does not exist");
        if (std::filesystem::exists(dst_file_path))
            std::filesystem::remove(dst_file_path);

        this->src_file = SrcFile<P1, P2>(src_file_path);
        this->dst_file = { dst_file_path, std::make_unique<std::basic_ofstream<P1>>(dst_file_path) };
        this->tmp_file = { tmp_file_path, std::make_unique<std::basic_ofstream<P1>>(tmp_file_path) };
    }

    template <typename P1, typename P2>
        requires CharacterType<P1> && FilePath<P2>
    Engine<P1, P2>::~Engine() noexcept 
    { }

    template <typename P1, typename P2>
        requires CharacterType<P1> && FilePath<P2>
    void Engine<P1, P2>::restart()
    {
        set_app_locale();
        this->src_file.restart();
        this->dst_file.restart();
        this->tmp_file.restart();
    }

    template <typename P1, typename P2>
        requires CharacterType<P1> && FilePath<P2>
    template <typename T, typename U>
        requires FilePath<T> && FilePath<U>
    void Engine<P1, P2>::restart(T src_file_name, U dst_file_name)
    {
        this->restart();
        
        auto src_file_path = std::filesystem::path(src_file_name);
        auto dst_file_path = std::filesystem::path(dst_file_name);
        auto tmp_file_path = TmpFile::get_tmp_file();

        if (!std::filesystem::exists(src_file_path))
            throw std::runtime_error("Source file does not exist");
        if (std::filesystem::exists(dst_file_path))
            std::filesystem::remove(dst_file_path);

        this->src_file = SrcFile<P1, P2>(src_file_path);
        this->dst_file = { dst_file_path, std::make_unique<std::basic_ofstream<P1>>(dst_file_path) };
        this->tmp_file = { tmp_file_path, std::make_unique<std::basic_ofstream<P1>>(tmp_file_path) };
    }
}