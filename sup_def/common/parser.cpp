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
    Parser::Parser()
    {
        try
        {
            this->file = new std::basic_ifstream<private_parsed_char_t>();
            this->file_content = new std::basic_string<private_parsed_char_t>();
            this->last_error_pos = 0;
            this->pragmas = new std::unordered_map<Parser::location_type, Pragma<private_parsed_char_t>>();
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error("Failed to initialize parser: " + std::string(e.what()) + "\n");
        }
    }

    Parser::Parser(std::filesystem::path file_path)
    {
        try
        {
            this->file = new std::basic_ifstream<private_parsed_char_t>(file_path);
            this->file_content = new std::basic_string<private_parsed_char_t>();
            this->last_error_pos = 0;
            this->slurp_file();
            this->strip_comments();
            // TODO: Locate pragmas and feed this->pragmas with them
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error("Failed to initialize parser: " + std::string(e.what()) + "\n");
        }
    }

    Parser::~Parser() noexcept
    {
        delete this->file;
        delete this->file_content;
        delete this->pragmas;
    }

    std::basic_string<Parser::parsed_char_t>* Parser::slurp_file()
    {
        if (!this->file_content)
            throw std::runtime_error("File content is null");
        if (!this->file->is_open())
            throw std::runtime_error("File is not open");

        this->file->seekg(0, std::basic_ios<private_parsed_char_t>::end);
        try
        {
            this->file_content->reserve(this->file->tellg());
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error("Failed to reserve file content: " + std::string(e.what()) + "\n");
        }
        this->file->seekg(0, std::basic_ios<private_parsed_char_t>::beg);

        try
        {
            this->file_content->clear();
            std::basic_ostringstream<private_parsed_char_t> sstr;
            sstr << this->file->rdbuf();
            this->file_content->assign(sstr.str());
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error("Failed to slurp file content: " + std::string(e.what()) + "\n");
        }
        return this->file_content;
    }

    // Strip C- and C++- style comments from this->file_content string.
    Parser& Parser::strip_comments(void)
    {
        if (!this->file_content)
            throw std::runtime_error("File content is null");
        if (this->file_content->empty())
            throw std::runtime_error("File content is empty");
        
        bool in_str_lit = false; // Are we in a string literal ?

        for (size_t i = 0; i < this->file_content->size(); i++)
        {
            if (this->file_content->at(i) == '"')
            {
                if (i > 0 && this->file_content->at(i - 1) != '\\')
                    in_str_lit = !in_str_lit;
            }
            else if (!in_str_lit)
            {
                if (this->file_content->at(i) == '/')
                {
                    if (i + 1 < this->file_content->size())
                    {
                        if (this->file_content->at(i + 1) == '/')
                        {
                            // Check for `\\` and erase the following line if present
                            size_t next_nl_pos = this->file_content->find('\n', i);
                            size_t next_bsl_pos = this->file_content->find('\\', i);
                            while ((next_bsl_pos != std::basic_string<Parser::parsed_char_t>::npos  || 
                                    next_nl_pos  != std::basic_string<Parser::parsed_char_t>::npos) &&
                                    next_bsl_pos < next_nl_pos)
                            {
                                next_nl_pos = this->file_content->find('\n', next_nl_pos + 1);
                                next_bsl_pos = this->file_content->find('\\', next_bsl_pos + 1);
                            }
                            if (next_nl_pos != std::basic_string<Parser::parsed_char_t>::npos)
                            {
                                this->file_content->erase(i, next_nl_pos - i + 1);
                                // Erase the possible '\t' or ' ' before the erased content
                                while (i > 0 && (this->file_content->at(i - 1) == '\t' || this->file_content->at(i - 1) == ' '))
                                {
                                    this->file_content->erase(i - 1, 1);
                                    i--;
                                }                                
                            }
                            else // erase until the end of the file
                                this->file_content->erase(i, this->file_content->size() - i);

                            i--;
                        }
                        else if (this->file_content->at(i + 1) == '*')
                        {
                            size_t end_comment_pos = this->file_content->find("*/", i);
                            if (end_comment_pos == std::basic_string<Parser::parsed_char_t>::npos)
                            {
                                this->last_error_pos = i;
                                throw std::runtime_error("Unterminated comment");
                            }
                            this->file_content->erase(i, end_comment_pos - i + 2);
                        }
                    }
                }
            }
        }
        return *this;
    }

    // Locates pragma start and pragma end positions in this->file_content string.
    std::vector<std::vector<std::basic_string<Parser::parsed_char_t>::size_type>> Parser::locate_pragmas(void)
    {
        if (!this->file_content)
            throw std::runtime_error("File content is null");
        if (this->file_content->empty())
            throw std::runtime_error("File content is empty");

        std::basic_string<Parser::parsed_char_t>::size_type pragma_start_pos = 0;
        std::basic_string<Parser::parsed_char_t>::size_type pragma_end_pos = 0;
        while ((pragma_start_pos = this->file_content->find("#pragma", pragma_start_pos)) != std::basic_string<Parser::parsed_char_t>::npos)
        {
            pragma_start_pos = skip_whitespaces<Parser::parsed_char_t>(*(this->file_content), pragma_start_pos);
            std::basic_string<Parser::parsed_char_t> substr = this->file_content->substr(pragma_start_pos, std::strlen(SUPDEF_PRAGMA_NAME));
            if (substr.compare(SUPDEF_PRAGMA_NAME) == 0)
            {
                pragma_start_pos += std::strlen(SUPDEF_PRAGMA_NAME);
                pragma_start_pos = skip_whitespaces<Parser::parsed_char_t>(*(this->file_content), pragma_start_pos);
                
            }
            else
                pragma_start_pos++;
        }
    }
}

