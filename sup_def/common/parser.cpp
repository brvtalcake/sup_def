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

#include <cassert>
#include <cstring>
#include <locale>
#include <regex>
#include <sstream>
#include <string>
#include <vector>
#include <map>

#include <sup_def/common/sup_def.hpp>

namespace SupDef
{
    Parser::Parser()
    {
        try
        {
            this->file = new std::basic_ifstream<parsed_char_t>();
            this->file_content = new std::basic_string<parsed_char_t>();
            this->lines = nullptr;
            this->last_error_pos = 0;
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
            this->file = new std::basic_ifstream<parsed_char_t>(file_path);
            this->file_content = new std::basic_string<parsed_char_t>();
            this->lines = nullptr;
            this->last_error_pos = 0;
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error("Failed to initialize parser: " + std::string(e.what()) + "\n");
        }
    }

    Parser::~Parser() noexcept
    {
        if (this->file)
            delete this->file;
        if (this->file_content)
            delete this->file_content;
        if (this->lines)
            delete this->lines;
    }

    std::basic_string<Parser::parsed_char_t>* Parser::slurp_file()
    {
        if (!this->file_content)
            throw std::runtime_error("File content is null");
        if (!this->file->is_open())
            throw std::runtime_error("File is not open");

        this->file->seekg(0, std::basic_ios<parsed_char_t>::end);
        try
        {
            this->file_content->reserve(this->file->tellg());
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error("Failed to reserve file content: " + std::string(e.what()) + "\n");
        }
        this->file->seekg(0, std::basic_ios<parsed_char_t>::beg);

        try
        {
            this->file_content->clear();
            std::basic_ostringstream<parsed_char_t> sstr;
            sstr << this->file->rdbuf();
            this->file_content->assign(sstr.str());
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error("Failed to slurp file content: " + std::string(e.what()) + "\n");
        }
        this->lines = new std::vector<std::basic_string<parsed_char_t>>(split_string(*this->file_content, '\n'));
        return this->file_content;
    }

    // Strip C and C++ style comments from this->file_content string.
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

    // Locates pragmas' start line and pragmas' end line in this->file_content string
    std::vector<std::vector<string_size_type<Parser::parsed_char_t>>> Parser::locate_pragmas(void)
    {
        if (!this->file_content)
            throw std::runtime_error("File content is null");
        if (this->file_content->empty())
            throw std::runtime_error("File content is empty");

        using loc_t = std::vector<string_size_type<Parser::parsed_char_t>>;

        string_size_type<Parser::parsed_char_t> pragma_start_pos = 0;
        string_size_type<Parser::parsed_char_t> pragma_end_pos = 0;
        std::vector<loc_t> pragma_locations;

        std::vector<std::basic_string<Parser::parsed_char_t>>
            splitted_file_content   =
                this->lines         ?
                *this->lines        :
                split_string(*this->file_content, '\n');
        bool in_supdef = false;
        using pos_t = string_size_type<Parser::parsed_char_t>;
        pos_t line_num = 0;
        for (std::basic_string<Parser::parsed_char_t> line : splitted_file_content)
        {
            pos_t p = line.find("#pragma");
            if (p == std::basic_string<Parser::parsed_char_t>::npos)
            {
                ++line_num;
                continue;
            }

            using regex_t = std::basic_regex<Parser::parsed_char_t>;
            using match_res_t = std::match_results<std::basic_string<Parser::parsed_char_t>::const_iterator>;

            regex_t pragma_start_regex(R"((#(\s*)pragma(\s+)supdef(\s+)start(\s+)(.*)))", std::regex_constants::ECMAScript);
            regex_t pragma_end_regex(R"((#(\s*)pragma(\s+)supdef(\s+)end(\s+)(.*)))", std::regex_constants::ECMAScript);
            match_res_t match_res;

            try
            {
                if (std::regex_match(line, match_res, pragma_start_regex))
                {
                    if (in_supdef)
                    {
                        this->last_error_pos = pragma_start_pos;
                        throw std::runtime_error("Pragma start found while already in a supdef block");
                    }
                    pragma_start_pos = line_num;
                    in_supdef = true;
                }
                else if (std::regex_match(line, match_res, pragma_end_regex))
                {
                    if (!in_supdef)
                    {
                        this->last_error_pos = pragma_end_pos;
                        throw std::runtime_error("Pragma end found while not in a supdef block");
                    }
                    else if (match_res[0].first != line.begin())
                    {
                        this->last_error_pos = pragma_start_pos;
                        throw std::runtime_error("Pragma end found not at start of line");
                    }
                    pragma_end_pos = line_num;
                    // Add pragma start and end to location vector
                    pragma_locations.push_back(loc_t{pragma_start_pos, pragma_end_pos});
                    in_supdef = false;
                }
            }
            catch (const std::exception& e)
            {
                throw std::runtime_error("Failed to locate pragmas: " + std::string(e.what()) + "\n");
            }
            ++line_num;
        }
        return pragma_locations;
    }

    Parser& Parser::process_file(void)
    {
        try
        {
            this->slurp_file();
            this->strip_comments();
            /* this->pragmas.clear(); */
            // TODO: Locate pragmas and feed this->pragmas with them
            auto pragma_locs = this->locate_pragmas();
            for (SupDef::Parser::pragma_loc_t loc : pragma_locs)
            {
                std::basic_string<Parser::parsed_char_t> pragma_content;
                for (size_t i = loc[0]; i < loc[1]; i++)
                {
                    pragma_content += (*this->lines)[i];
                    pragma_content += '\n';
                }
                /* this->pragmas.emplace(std::make_pair(loc, pragma_content)); */
            }
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error("Failed to process file: " + std::string(e.what()) + "\n");
        }
        // Next, locate supdefs instantiations and replace them with their content
    }
}

