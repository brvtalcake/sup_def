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

#if defined(__INTELLISENSE__)
    #define TRUE_VAL 1
#endif

#if NEED_TPP_INC(Parser) == 0 || TRUE_VAL
#undef TRUE_VAL
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

    template <typename T>
        requires CharacterType<T>
    Coro<Result<typename Parser<T>::pragma_loc_type, Error<T, std::filesystem::path>>> Parser<T>::search_super_defines(void)
    {
        typedef typename std::basic_regex<T> regex_t;
        typedef std::match_results<typename std::basic_string<T>::const_iterator> match_res_t;
        typedef typename decltype(this->lines)::size_type line_num_t;
        typedef typename string_size_type<T> pos_t;

        auto mk_valid_pragmaloc = [ ](
            const std::basic_string<T>& content, /* A path, in our case here */
            const string_size_type<T>& start_line, /* Start line */
            const string_size_type<T>& end_line /* End line */
        ) -> Parser<T>::pragma_loc_type {
            return std::make_tuple(content, start_line, end_line); 
        };

        using RetType = Result<typename Parser<T>::pragma_loc_type, Error<T, std::filesystem::path>>;
        RetType ret;

        if (this->file_content.empty() || this->lines.empty())
        {
            ret = Error<T, std::filesystem::path>(ExcType::INTERNAL_ERROR, "File content is empty");
            co_return ret;
        }
        
        bool in_supdef_body = false;
        for (line_num_t i = 0; i < this->lines.size(); ++i)
        {
            const std::vector<std::tuple<string_size_type<T>, string_size_type<T>, T>>& line_ = this->lines.at(i);
            std::basic_string<T> line;
            line.reserve(line_.size());
            if (line_.empty())
                continue;
            else
                curr_line = std::get<0>(line_.at(0)) + 1;
            for (auto& c : line_)
                line += std::get<2>(c);
            regex_t pragma_start_regex(SUPDEF_PRAGMA_DEF_BEG_REGEX, std::regex_constants::ECMAScript);
            regex_t pragma_end_regex(SUPDEF_PRAGMA_DEF_END_REGEX, std::regex_constants::ECMAScript);
            match_res_t match_res;
            try
            {
                // TODO
            }
            catch (const std::exception& e)
            {
                ret = Error<T, std::filesystem::path>(
                    ExcType::INTERNAL_ERROR,
                    "Caught exception while parsing supdefinitions (in function `" + std::string(__PRETTY_FUNCTION__) + "`): " + std::string(e.what()) + "\n"
                );
                co_return ret;
            }
            catch (...)
            {
                ret = Error<T, std::filesystem::path>(
                    ExcType::INTERNAL_ERROR,
                    "Caught unknown exception while parsing supdefinitions (in function `" + std::string(__PRETTY_FUNCTION__) + "`)\n"
                );
            }
        }
    }

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