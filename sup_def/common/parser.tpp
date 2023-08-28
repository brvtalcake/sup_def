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

    template <typename T>
        requires CharacterType<T>
    Parser<T>::Parser()
    {
        try
        {
            this->file = std::make_unique<std::basic_ifstream<T>>();
            this->file_content = std::make_shared<std::basic_string<T>>();
            this->lines.clear();
        }
        catch (const std::exception& e)
        {
            throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "Failed to initialize parser: " + std::string(e.what()) + "\n");
        }
    }

    template <typename T>
        requires CharacterType<T>
    Parser<T>::Parser(std::filesystem::path file_path)
    {
        try
        {
            this->file = std::make_unique<std::basic_ifstream<T>>(file_path);
            this->file_content = std::make_shared<std::basic_string<T>>();
            this->lines.clear();
            this->file_path = file_path;
        }
        catch (const std::exception& e)
        {
            throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "Failed to initialize parser: " + std::string(e.what()) + "\n");
        }
    }

#if defined(SUPDEF_DEBUG)
    template <typename T>
        requires CharacterType<T>
    template <typename Stream>
    void Parser<T>::print_content(Stream& s) const
    {
        if (this->file_content == nullptr)
            throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "File content is null\n");
        if (this->file_content.get()->empty())
            throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "File content is empty\n");
        s << *(this->file_content.get()) << std::endl;
    }
#endif

    template <typename T>
        requires CharacterType<T>
    std::shared_ptr<std::basic_string<T>> Parser<T>::slurp_file()
    {
        if (this->file_content == nullptr)
            throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "File content is null\n");
        if (!this->file.get()->is_open())
            throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "File is not open\n");

        this->file.get()->seekg(0, std::basic_ios<T>::end);
        try
        {
            this->file_content.get()->reserve(this->file.get()->tellg());
        }
        catch (const std::exception& e)
        {
            throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "Failed to reserve file content: " + std::string(e.what()) + "\n");
        }
        this->file.get()->seekg(0, std::basic_ios<T>::beg);

        try
        {
            this->file_content.get()->clear();
            std::basic_ostringstream<T> sstr;
            sstr << this->file.get()->rdbuf();
            this->file_content.get()->assign(sstr.str());
        }
        catch (const std::exception& e)
        {
            throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "Failed to read file content: " + std::string(e.what()) + "\n");
        }
        this->lines.clear();
#if defined(__cpp_lib_containers_ranges) && __cpp_lib_containers_ranges >= 202202L && 0 // For now, disable this
        // Not even sure this works
        this->lines.assign_range(split_string(*(this->file_content.get()), '\n'));
#else
        this->lines = split_string(*(this->file_content.get()), static_cast<T>('\n'));
#endif
        return this->file_content;
    }

    // Strip C and C++ style comments from this->file_content string.
    // TODO: Also remove `#if 0` blocks
    template <typename T>
        requires CharacterType<T>
    Parser<T>& Parser<T>::strip_comments(void)
    {
        if (this->file_content == nullptr)
            throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "File content is null\n");
        if (this->file_content.get()->empty())
            throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "File content is empty\n");
        
        bool in_str_lit = false; // Are we in a string literal ?

        for (size_t i = 0; i < this->file_content.get()->size(); i++)
        {
            if (this->file_content.get()->at(i) == '"')
            {
                if (i > 0 && this->file_content.get()->at(i - 1) != '\\')
                    in_str_lit = !in_str_lit;
            }
            else if (!in_str_lit)
            {
                if (this->file_content.get()->at(i) == '/')
                {
                    if (i + 1 < this->file_content.get()->size())
                    {
                        if (this->file_content.get()->at(i + 1) == '/')
                        {
                            // Check for `\\` and erase the following line if present
                            size_t next_nl_pos = this->file_content.get()->find('\n', i);
                            size_t next_bsl_pos = this->file_content.get()->find('\\', i);
                            while ((next_bsl_pos != std::basic_string<T>::npos  || 
                                    next_nl_pos  != std::basic_string<T>::npos) &&
                                    next_bsl_pos < next_nl_pos)
                            {
                                next_nl_pos = this->file_content.get()->find('\n', next_nl_pos + 1);
                                next_bsl_pos = this->file_content.get()->find('\\', next_bsl_pos + 1);
                            }
                            if (next_nl_pos != std::basic_string<T>::npos)
                            {
                                this->file_content.get()->erase(i, next_nl_pos - i + 1);
                                // Erase the possible '\t' or ' ' before the erased content
                                while (i > 0 && (this->file_content.get()->at(i - 1) == '\t' || this->file_content.get()->at(i - 1) == ' '))
                                {
                                    this->file_content.get()->erase(i - 1, 1);
                                    i--;
                                }                                
                            }
                            else // erase until the end of the file
                                this->file_content.get()->erase(i, this->file_content.get()->size() - i);

                            i--;
                        }
                        else if (this->file_content.get()->at(i + 1) == '*')
                        {
                            auto to_find = any_string<T>("*/");
                            size_t end_comment_pos = this->file_content.get()->find(to_find.data(), i);
                            if (end_comment_pos == std::basic_string<T>::npos)
                            {
                                // Put the line, column and line content in the exception
                                size_t line = 1;
                                size_t column = 1;
                                for (size_t j = 0; j < i; j++)
                                {
                                    if (this->file_content.get()->at(j) == '\n')
                                    {
                                        line++;
                                        column = 1;
                                    }
                                    else
                                        column++;
                                }
                                throw Exception<T, std::filesystem::path>(ExcType::SYNTAX_ERROR, "Unterminated comment\n", this->file_path, line, column, this->lines.at(line - 1));
                            }
                            this->file_content.get()->erase(i, end_comment_pos - i + 2);
                        }
                    }
                }
            }
        }
        return *this;
    }