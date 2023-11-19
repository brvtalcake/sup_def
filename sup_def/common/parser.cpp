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
    template <typename T>
        requires CharacterType<T>
    Coro<Result<typename Parser<T>::pragma_loc_type, Error<T, std::filesystem::path>>> Parser<T>::search_super_defines(void)
    {
        typedef typename std::basic_regex<T> regex_t;
        typedef std::match_results<typename std::basic_string<T>::const_iterator> match_res_t;
        typedef typename decltype(this->lines)::size_type line_num_t;
        typedef string_size_type<T> pos_t;

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
        
        pos_t curr_line = 0;
        pos_t pragma_start_pos = 0;
        pos_t pragma_end_pos = 0;
        std::basic_string<T> pragma_content; // Pragma name will be prepended as the first line of the pragma content
        std::basic_string<T> pragma_name;
        bool in_supdef_body = false;
        for (line_num_t i = 0; i < this->lines.size(); ++i)
        {
            const std::vector<ParsedChar<T>>& line_ = this->lines.at(i);
            std::basic_string<T> line;
            line.reserve(line_.size());
            if (line_.empty())
                continue;
            else
                curr_line = std::get<0>(line_.at(0)) + 1;
            for (auto& c : line_)
                line += std::get<2>(c);
            // With `SUPDEF_PRAGMA_DEF_BEG_REGEX` expanding to:
            //    "^\\s*#\\s*pragma\\s+" "supdef" "\\s+" "begin" "\\s+" "\\w+" "\\s*$"
            regex_t pragma_start_regex(ANY_STRING(T, SUPDEF_PRAGMA_DEF_BEG_REGEX), std::regex_constants::ECMAScript);
            // With `SUPDEF_PRAGMA_DEF_END_REGEX` expanding to:
            //    "^\\s*#\\s*pragma\\s+" "supdef" "\\s+" "end" "\\s+" "\\w+" "\\s*$"
            regex_t pragma_end_regex(ANY_STRING(T, SUPDEF_PRAGMA_DEF_END_REGEX), std::regex_constants::ECMAScript);
            match_res_t match_res;
            try
            {
                if (std::regex_match(line, match_res, pragma_start_regex))
                {
                    if (in_supdef_body)
                    {
                        auto err_line = std::get<0>(line_.at(0));
                        string_size_type<T> begin_kwd_pos = match_res[0].first - line.begin();
                        // TODO (FIXME): err_col may be wrong if there was a comment in the line (same for search_includes method)
                        auto err_col = std::get<1>(line_.at(begin_kwd_pos));
                        ret = Error<T, std::filesystem::path>(
                            ExcType::SYNTAX_ERROR,
                            "Pragma start found while already in a supdef block",
                            this->file_path,
                            err_line + 1,
                            err_col + 1,
                            line
                        );
                        // For now, straight up `co_return` as we don't want to parse anymore, since it would be useless
                        // and probably cause more errors
                        co_return ret;
                    }
                    pragma_start_pos = curr_line;
                    in_supdef_body = true;
                    pragma_name = match_res[5].str();
                    pragma_content.clear();
                    pragma_content += pragma_name;
                    pragma_content += static_cast<T>('\n');
                    // Remove the line from this->lines
                    this->lines.erase(this->lines.begin() + i);
                    /* this->lines_raw.erase(this->lines_raw.begin() + i); */
                }
                else if (std::regex_match(line, match_res, pragma_end_regex))
                {
                    if (!in_supdef_body)
                    {
                        auto err_line = std::get<0>(line_.at(0));
                        string_size_type<T> end_kwd_pos = match_res[0].first - line.begin();
                        auto err_col = std::get<1>(line_.at(end_kwd_pos));
                        ret = Error<T, std::filesystem::path>(
                            ExcType::SYNTAX_ERROR,
                            "Pragma end found while not in a supdef block",
                            this->file_path,
                            err_line + 1,
                            err_col + 1,
                            line
                        );
                        co_return ret; // (same comment as above)
                    }
                    else if (match_res[0].first != line.begin())
                    {
                        auto err_line = std::get<0>(line_.at(0));
                        auto err_col = std::get<1>(line_.at(0));
                        ret = Error<T, std::filesystem::path>(
                            ExcType::SYNTAX_ERROR,
                            "Pragma end found not at start of line",
                            this->file_path,
                            err_line + 1,
                            err_col + 1,
                            line
                        );
                        // Here this is probably ok to continue parsing
                        // Just ignore what's before the start of the pragma (on the line)
                        co_yield ret;
                        continue;
                    }
                    pragma_end_pos = curr_line;
                    // Remove the line from this->lines
                    this->lines.erase(this->lines.begin() + i);
                    /* this->lines_raw.erase(this->lines_raw.begin() + i); */
                    // Add pragma start and end to location vector
                    ret = mk_valid_pragmaloc(pragma_content, pragma_start_pos, pragma_end_pos);
                    co_yield ret;
                    in_supdef_body = false;
                }
                else if (in_supdef_body)
                {
                    pragma_content += line;
                    pragma_content += static_cast<T>('\n');
                    // Remove the line from this->lines
                    this->lines.erase(this->lines.begin() + i);
                    /* this->lines_raw.erase(this->lines_raw.begin() + i); */
                }
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
#ifdef MK_CHAR
    #undef MK_CHAR
#endif
#define MK_CHAR(ORIGINAL_LINE, ORIGINAL_COL, CHAR) ParsedChar<T>(std::forward<string_size_type<T>>(ORIGINAL_LINE), std::forward<string_size_type<T>>(ORIGINAL_COL), std::forward<T>(CHAR))

        /* this->file_content_raw = this->reassemble_lines(); */
        // Do the same for this->file_content
        this->file_content.clear();
        this->file_content.push_back(MK_CHAR(0, 0, static_cast<T>('\n'))); // dummy line
        for (size_t counter = 0; counter < this->lines.size(); ++counter)
        {
            auto& line = this->lines[counter];
            for (auto& c : line)
                this->file_content.push_back(c);
            this->file_content.push_back(MK_CHAR(std::get<0>(this->file_content.at(this->file_content.size() - 1)), std::get<1>(this->file_content.at(this->file_content.size() - 1)), static_cast<T>('\n')));
        }
        ret = nullptr; // Indicate the end
#undef MK_CHAR
        co_return ret;
    }

    // Explicitely instantiate Parser class for all needed character types
    EXP_INST_CLASS(Parser, (char), (wchar_t), (char8_t), (char16_t), (char32_t))
#if defined(SUPDEF_DEBUG)
    /* EXP_INST_FUNC(Parser<char>::print_content, (void, (std::ostream&))); */
    // Explicitely instantiate print_content 
    template void Parser<char>::print_content<std::ostream>(std::ostream&) const;
    template void Parser<wchar_t>::print_content<std::ostream>(std::ostream&) const;
    template void Parser<char8_t>::print_content<std::ostream>(std::ostream&) const;
    template void Parser<char16_t>::print_content<std::ostream>(std::ostream&) const;
    template void Parser<char32_t>::print_content<std::ostream>(std::ostream&) const;
#endif
}

#else

#define INCLUDED_FROM_SUPDEF_SOURCE 1
#include <sup_def/common/parser.tpp>
#undef INCLUDED_FROM_SUPDEF_SOURCE

#endif