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

#include <sup_def/common/config.h>

#if NEED_TPP_INC(Parser) == 0
#include <sup_def/common/sup_def.hpp>

#include <cassert>
#include <cstring>
#include <locale>
#include <regex>
#include <sstream>
#include <string>
#include <vector>
#include <map>

namespace SupDef
{
#if 0
    template <typename T>
        requires CharacterType<T>
    Parser<T>::~Parser() noexcept
    { }
#endif



    

#if 0
    // Locates pragmas' start line and pragmas' end line in this->file_content string
    std::vector<std::vector<string_size_type<Parser::parsed_char_t>>> Parser::locate_supdefs(void)
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
#endif

#if 0
    Parser& Parser::process_file(void)
    {
        try
        {
            this->slurp_file();
            this->strip_comments();
            /* this->pragmas.clear(); */
            // TODO: Locate pragmas and feed this->pragmas with them
            auto pragma_locs = this->locate_supdefs();
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

        return *this;
    }
#endif

    // Explicitely instantiate Parser class for all needed character types
    EXP_INST_CLASS(Parser, (char), (wchar_t), (char8_t), (char16_t), (char32_t))
#if defined(SUPDEF_DEBUG)
    /* EXP_INST_FUNC(Parser<char>::print_content, (void, (std::ostream&))); */
    // Explicitely instantiate print_content 
    template void Parser<char>::print_content<std::ostream>(std::ostream&) const;
#endif
}

#else

#define INCLUDED_FROM_SUPDEF_SOURCE 1
#include <sup_def/common/parser.tpp>
#undef INCLUDED_FROM_SUPDEF_SOURCE

#endif