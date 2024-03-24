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

using namespace std::literals;

namespace SupDef
{

#ifdef MK_CHAR
    #undef MK_CHAR
#endif
#define MK_CHAR(ORIGINAL_LINE, ORIGINAL_COL, CHAR) ParsedChar<T>(std::forward<string_size_type<T>>(ORIGINAL_LINE), std::forward<string_size_type<T>>(ORIGINAL_COL), std::forward<T>(CHAR))

    template <typename T>
        requires CharacterType<T>
    Parser<T>::Parser()
    {
        try
        {
            this->file = std::basic_ifstream<T>();
            this->file_content_raw = std::basic_string<T>();
            this->lines_raw.clear();
            this->file_content.clear();
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
            this->file = std::basic_ifstream<T>(file_path);
            this->file_content_raw = std::basic_string<T>();
            this->lines_raw.clear();
            this->file_path = file_path;
            this->file_content.clear();
            this->lines.clear();
        }
        catch (const std::exception& e)
        {
            throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "Failed to initialize parser: " + std::string(e.what()) + "\n");
        }
    }

    template <typename T>
        requires CharacterType<T>
    Parser<T>::Parser(std::filesystem::path file_path, std::basic_ifstream<T>& file_stream)
    {
        try
        {
            this->file = std::ref(file_stream);
            this->file_content_raw = std::basic_string<T>();
            this->lines_raw.clear();
            this->file_path = file_path;
            this->file_content.clear();
            this->lines.clear();
        }
        catch (const std::exception& e)
        {
            throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "Failed to initialize parser: " + std::string(e.what()) + "\n");
        }
    }

    template <typename T>
        requires CharacterType<T>
    std::basic_ifstream<T>& Parser<T>::get_file_stream(void)
    {
        if (std::holds_alternative<file_stream_type1>(this->file))
            return std::get<file_stream_type1>(this->file);
        else if (std::holds_alternative<file_stream_type2>(this->file))
            return std::get<file_stream_type2>(this->file).get();
        else
            throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "Invalid file type\n");
    }

    template <typename T>
        requires CharacterType<T>
    std::basic_string<T> Parser<T>::slurp_file()
    {
        if (!this->get_file_stream().is_open())
            throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "File is not open\n");

        this->get_file_stream().seekg(0, std::basic_ios<T>::end);
        try
        {
            this->file_content_raw.reserve(this->get_file_stream().tellg());
        }
        catch (const std::exception& e)
        {
            throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "Failed to reserve file content: " + std::string(e.what()) + "\n");
        }
        this->get_file_stream().seekg(0, std::basic_ios<T>::beg);

        try
        {
            this->file_content_raw.clear();
            std::basic_ostringstream<T> sstr;
            sstr << this->get_file_stream().rdbuf();
            this->file_content_raw.assign(sstr.str());
        }
        catch (const std::exception& e)
        {
            throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "Failed to read file content: " + std::string(e.what()) + "\n");
        }
        this->lines_raw.clear();
#if defined(__cpp_lib_containers_ranges) && __cpp_lib_containers_ranges >= 202202L && 0 // For now, disable this
        // Not even sure this works
        this->lines_raw.assign_range(split_string(this->file_content_raw, '\n'));
#else
        this->lines_raw = split_string(this->file_content_raw, CONVERT(T, '\n'));
#endif
        this->file_content.clear();
        this->lines.clear();
        string_size_type<T> linecount = 0;
        string_size_type<T> colcount = 0;
        std::vector<ParsedChar<T>> line;
        for (auto&& c : this->file_content_raw)
        {
            if (SAME(c, '\n'))
            {
                this->lines.push_back(line);
                line.push_back(MK_CHAR(linecount, colcount, c));
                this->file_content.insert(this->file_content.end(), line.begin(), line.end());
                line.clear();
                linecount++;
                colcount = 0;
            }
            else
            {
                line.push_back(MK_CHAR(linecount, colcount, c));
                colcount++;
            }
        }
        // Reset file state to freshly opened
        this->get_file_stream().seekg(0, std::basic_ios<T>::beg);

        return this->file_content_raw;
    }

    // Strip C and C++ style comments from this->file_content string.
    template <typename T>
        requires CharacterType<T>
    Parser<T>& Parser<T>::strip_comments(void)
    {
#if defined(FILE_CONTENT)
    #undef FILE_CONTENT
#endif
#define FILE_CONTENT(POS) (this->file_content_raw.at(POS))

        if (this->file_content_raw.empty() || this->file_content.empty())
            throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "File content is empty\n");
        
        [[maybe_unused]]
        size_t pp_if_level = 0;
        bool in_str_lit = false; // Are we in a string literal ?
        std::basic_string<T> file_content_raw_copy{ this->file_content_raw };

        typedef std::basic_string<T> local_string_type;
        typedef typename local_string_type::size_type local_size_type;
        
#undef FILE_CONTENT
#define FILE_CONTENT(POS) (file_content_raw_copy.at(POS))
        try
        {
            #pragma GCC novector
            for (local_size_type i = 0; i < file_content_raw_copy.size(); ++i)
            {
                if (SAME(FILE_CONTENT(i), '"'))
                {
                    if (i > 0 && DIFFERENT(FILE_CONTENT(i - 1), '\\'))
                        in_str_lit = !in_str_lit;
                }
                else if (!in_str_lit)
                {
                    if (SAME(FILE_CONTENT(i), '/'))
                    {
                        if (i + 1 < file_content_raw_copy.size())
                        {
                            if (SAME(FILE_CONTENT(i + 1), '/'))
                            {
                                // Check for `\\` and erase the following line if present
                                local_size_type next_nl_pos = file_content_raw_copy.find(CONVERT(T, '\n'), i);
                                local_size_type next_bsl_pos = file_content_raw_copy.find(CONVERT(T, '\\'), i);
                                while ((next_bsl_pos != std::basic_string<T>::npos  || 
                                        next_nl_pos  != std::basic_string<T>::npos) &&
                                        next_bsl_pos < next_nl_pos                  &&
                                        next_nl_pos + 1 < file_content_raw_copy.size())
                                {
                                    next_nl_pos = file_content_raw_copy.find(CONVERT(T, '\n'), next_nl_pos + 1);
                                    next_bsl_pos = file_content_raw_copy.find(CONVERT(T, '\\'), next_bsl_pos + 1);
                                }
                                if (next_nl_pos != std::basic_string<T>::npos)
                                {
                                    file_content_raw_copy.erase(i, next_nl_pos - i);
                                    this->file_content.erase(this->file_content.begin() + i, this->file_content.begin() + next_nl_pos);
                                    // Erase the possible '\t' or ' ' before the erased content
                                    while (i > 0 && (SAME(FILE_CONTENT(i - 1), '\t') || SAME(FILE_CONTENT(i - 1), ' ')))
                                    {
                                        file_content_raw_copy.erase(i - 1, 1);
                                        this->file_content.erase(this->file_content.begin() + i - 1);
                                        i--;
                                    }
                                }
                                else // erase until the end of the file
                                {
                                    file_content_raw_copy.erase(i, file_content_raw_copy.size() - i);
                                    this->file_content.erase(this->file_content.begin() + i, this->file_content.end());
                                }
                                i--;
                            }
                            else if (SAME(FILE_CONTENT(i + 1), '*'))
                            {
                                auto to_find = ANY_STRING(T, "*/");
                                local_size_type end_comment_pos = file_content_raw_copy.find(to_find.data(), i);
                                if (end_comment_pos == std::basic_string<T>::npos)
                                {
                                    // Put the line, column and line content in the exception
                                    size_t line = 1;
                                    size_t column = 1;
                                    for (size_t j = 0; j < i; j++)
                                    {
                                        if (SAME(FILE_CONTENT(j), '\n'))
                                        {
                                            line++;
                                            column = 1;
                                        }
                                        else
                                            column++;
                                    }
                                    throw Exception<T, std::filesystem::path>(ExcType::SYNTAX_ERROR, "Unterminated comment\n", this->file_path, line, column, this->lines_raw.at(line - 1));
                                }
                                // If end_comment_pos == end of file, just delete until the end of the file and then add the space
                                if (end_comment_pos + 2 >= file_content_raw_copy.size())
                                {
                                    file_content_raw_copy.erase(file_content_raw_copy.begin() + i, file_content_raw_copy.end());
                                    this->file_content.erase(this->file_content.begin() + i, this->file_content.end());
                                    
                                    for (auto&& cch : CONVERT(T, ' '))
                                        file_content_raw_copy.push_back(cch);
                                    auto good_pos = [&]() -> string_size_type<T> { return i == this->file_content.size() ? i - 1 : i; };
                                    for (auto&& cch : CONVERT(T, ' '))
                                        this->file_content.push_back(MK_CHAR(std::get<0>(this->file_content.at(good_pos())), std::get<1>(this->file_content.at(good_pos())), cch));
                                }
                                else
                                {
                                    file_content_raw_copy.erase(i, end_comment_pos - i + 2);
                                    this->file_content.erase(this->file_content.begin() + i, this->file_content.begin() + end_comment_pos + 2);
                                    // Add a space instead, as specified in the standard
                                    file_content_raw_copy.insert(i, ANY_STRING(T, " ").data());
                                    std::vector<ParsedChar<T>> to_insert{};
                                    auto good_pos = [&]() -> string_size_type<T> { return i == this->file_content.size() ? i - 1 : i; };
                                    for (auto&& cch : CONVERT(T, ' '))
                                        to_insert.push_back(MK_CHAR(std::get<0>(this->file_content.at(good_pos())), std::get<1>(this->file_content.at(good_pos())), cch));
                                    this->file_content.insert(this->file_content.begin() + i, to_insert.begin(), to_insert.end());
                                }
                                i--;
                            }
                        }
                    }
                }
            }
        }
        catch (const Exception<T, std::filesystem::path>& e)
        {
            throw e;
        }
        catch (const std::exception& e)
        {
            throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "Failed to strip comments: " + std::string(e.what()) + "\n");
        }
        catch (...)
        {
            throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "Failed to strip comments: Unknown exception\n");
        }

        if (file_content_raw_copy.size() != this->file_content.size())
            throw Exception<T, std::filesystem::path>(ExcType::INTERNAL_ERROR, "Parsing failed");
        for (string_size_type<T> i = 0; i < file_content_raw_copy.size(); ++i)
        {
            if (DIFFERENT(FILE_CONTENT(i), std::get<2>(this->file_content.at(i))))
                throw Exception<T, std::filesystem::path>(ExcType::INTERNAL_ERROR, "Parsing failed");
        }
    
#undef FILE_CONTENT
#define FILE_CONTENT(POS) (this->file_content_raw.at(POS))
/*
        this->lines_raw.clear();
#if defined(__cpp_lib_containers_ranges) && __cpp_lib_containers_ranges >= 202202L && 0 // For now, disable this
        // Not even sure this works
        this->lines_raw.assign_range(split_string(this->file_content_raw, '\n'));
#else
        this->lines_raw = split_string(this->file_content_raw, CONVERT(T, '\n'));
#endif
*/
        // Also resplit the lines
        this->lines.clear();
        string_size_type<T> linecount = 0;
        string_size_type<T> colcount = 0;
        std::vector<ParsedChar<T>> line;
        for (auto&& c : this->file_content)
        {
            if (SAME(std::get<2>(c), '\n'))
            {
                this->lines.push_back(line);
                line.clear();
                linecount++;
                colcount = 0;
            }
            else
            {
                line.push_back(c);
                colcount++;
            }
        }
        return *this;
    }

    /**
     * @brief Remove all string literals from the file content
     * @details Does not modify the content of the class, but returns a new string
     * 
     * @tparam T 
     * @return std::basic_string<T>
     */
    template <typename T>
        requires CharacterType<T>
    std::basic_string<T> Parser<T>::remove_cstr_lit(void)
    {
        if (this->file_content_raw.empty() || this->file_content.empty())
            throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "File content is empty\n");

        std::basic_string<T> res;
        res.reserve(this->file_content_raw.size());
        bool in_str_lit = false;
        for (size_t i = 0; i < this->file_content_raw.size(); ++i)
        {
            if (SAME(FILE_CONTENT(i), '"'))
            {
                if (i > 0 && DIFFERENT(FILE_CONTENT(i - 1), '\\'))
                    in_str_lit = !in_str_lit;
            }
            else if (!in_str_lit)
                res += FILE_CONTENT(i);
        }
        return res;
    }

    /* template <typename T>
        requires CharacterType<T>
    std::shared_ptr<std::basic_string<T>> Parser<T>::reassemble_lines(void)
    {
        if (this->lines_raw.empty())
            throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "Lines are empty\n");
        std::basic_string<T> res;
        res.reserve(this->file_content_raw.size());
        for (auto& line : this->lines_raw)
            res += line + CONVERT(T, '\n');
        return std::make_shared<std::basic_string<T>>(res);
    } */


#if SUPDEF_WORKAROUND_GCC_INTERNAL_ERROR
    // Just do the same but as a friend function
    template <typename T>
        requires CharacterType<T>
    Coro<Result<std::shared_ptr<std::basic_string<T>>, Error<T, std::filesystem::path>>> search_imports(Parser<T>& parser);
#else
    // TODO: When parsing, verify that there is nothing appearing before the import pragma
    template <typename T>
        requires CharacterType<T>
    Coro<Result<typename Parser<T>::pragma_loc_type, Error<T, std::filesystem::path>>> Parser<T>::search_imports(void)
    {
        Result<Parser<T>::pragma_loc_type, Error<T, std::filesystem::path>> ret;
        string_size_type<T> curr_line = 0;

        auto mk_valid_pragmaloc = [ ](
            const std::basic_string<T>& content, /* A path, in our case here */
            const string_size_type<T>& line /* Start line and end line are the same here */
        ) -> Parser<T>::pragma_loc_type
        {
            return std::make_tuple(content, line, line);
        };
        auto mk_expected_ret = [&mk_valid_pragmaloc](
            const std::basic_string<T>& content, /* A path, in our case here */
            const string_size_type<T>& line /* Start line and end line are the same here */
        ) -> Result<Parser<T>::pragma_loc_type, Error<T, std::filesystem::path>>
        {
            return ::SupDef::make_ok_result<Parser<T>::pragma_loc_type, Error<T, std::filesystem::path>>(mk_valid_pragmaloc(content, line));
        };
        auto mk_null_ret = [ ](void) -> Result<Parser<T>::pragma_loc_type, Error<T, std::filesystem::path>>
        {
            return ::SupDef::make_null_result<Parser<T>::pragma_loc_type, Error<T, std::filesystem::path>>();
        };
        auto mk_unexpected_ret = [ ](
            const Error<T, std::filesystem::path>& err
        ) -> Result<Parser<T>::pragma_loc_type, Error<T, std::filesystem::path>>
        {
            return ::SupDef::make_err_result<Parser<T>::pragma_loc_type, Error<T, std::filesystem::path>>(err);
        };

        if (this->file_content.empty() || this->lines.empty())
        {
            ret = mk_unexpected_ret(Error<T, std::filesystem::path>(ExcType::INTERNAL_ERROR, "File content is empty or lines are empty\n"));
            co_return ret;
        }

        typedef typename std::basic_regex<T> regex_t;
        typedef std::match_results<typename std::basic_string<T>::const_iterator> match_res_t;
        typedef typename decltype(this->lines)::size_type line_num_t;
        typedef string_size_type<T> pos_t;

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
            const regex_t inc_regex(ANY_STRING(T, SUPDEF_PRAGMA_IMPORT_REGEX), std::regex_constants::ECMAScript);
            const regex_t inc_regex_angle_brackets(ANY_STRING(T, SUPDEF_PRAGMA_IMPORT_REGEX_ANGLE_BRACKETS), std::regex_constants::ECMAScript);
            const regex_t inc_regex_no_quotes(ANY_STRING(T, SUPDEF_PRAGMA_IMPORT_REGEX_NO_QUOTES), std::regex_constants::ECMAScript);
            // This is invalid and must be reported as a syntax error
            const regex_t inc_regex_no_path(ANY_STRING(T, SUPDEF_PRAGMA_IMPORT_REGEX_NO_PATH), std::regex_constants::ECMAScript);
            // Invalid too
            const regex_t inc_regex_with_anything(ANY_STRING(T, "^\\s*#\\s*pragma\\s+" SUPDEF_PRAGMA_NAME "\\s+" SUPDEF_PRAGMA_IMPORT ".*$"), std::regex_constants::ECMAScript);
            match_res_t match_res;
            if (std::regex_match(line, match_res, inc_regex))
            {
                pos_t l_bracket_pos = line.find(CONVERT(T, '<'));
                pos_t r_bracket_pos = line.find(CONVERT(T, '>'));
                if (l_bracket_pos != std::basic_string<T>::npos || r_bracket_pos != std::basic_string<T>::npos)
                {
                    auto where = (l_bracket_pos != std::basic_string<T>::npos) ? l_bracket_pos : r_bracket_pos;
                    pos_t err_line = line_.at(where).line();
                    pos_t err_col = line_.at(where).col();
                    auto real_line = this->lines_raw.at(err_line);
                    ret = mk_unexpected_ret(Error<T, std::filesystem::path>(
                        ExcType::SYNTAX_ERROR,
                        "You cannot mix `<>` with `\"\"` in import pragmas",
                        this->file_path,
                        err_line + 1,
                        err_col + 1,
                        real_line
                    ));
                    co_yield ret;
                    continue;
                }
#if SUPDEF_DEBUG
                std::string converted = std::string(match_res[1].first, match_res[1].second);
                std::cerr << "[ DEBUG LOG ]  Found import: " << converted.c_str() << std::endl;
#endif
                std::basic_string<T> inc_path = remove_whitespaces(match_res[1].str());
                if (inc_path.empty())
                {
                    auto where = line.find(CONVERT(T, '"'));
                    pos_t err_line = line_.at(where).line();
                    pos_t err_col = line_.at(where).col();
                    auto real_line = this->lines_raw.at(err_line);
                    ret = mk_unexpected_ret(Error<T, std::filesystem::path>(
                        ExcType::SYNTAX_ERROR,
                        "Empty import path",
                        this->file_path,
                        err_line + 1,
                        err_col + 1,
                        real_line
                    ));
                    co_yield ret;
                    continue;
                }
                ret = mk_expected_ret(inc_path, curr_line);
                // Delete the full line from this->lines (since the pragma is supposed to take the full line)
                /* this->lines_raw.erase(this->lines_raw.begin() + i); */
                this->lines.erase(this->lines.begin() + i);
                i--;
                co_yield ret;
                continue;
            }
            else if (std::regex_match(line, match_res, inc_regex_angle_brackets))
            {
                pos_t quote_pos = line.find(CONVERT(T, '"'));
                if (quote_pos != std::basic_string<T>::npos)
                {
                    pos_t err_line = line_.at(quote_pos).line();
                    pos_t err_col = line_.at(quote_pos).col();
                    auto real_line = this->lines_raw.at(err_line);
                    ret = mk_unexpected_ret(Error<T, std::filesystem::path>(
                        ExcType::SYNTAX_ERROR,
                        "You cannot mix `\"\"` with `<>` in import pragmas",
                        this->file_path,
                        err_line + 1,
                        err_col + 1,
                        real_line
                    ));
                    co_yield ret;
                    continue;
                }
#if SUPDEF_DEBUG
                std::string converted = std::string(match_res[1].first, match_res[1].second);
                std::cerr << "[ DEBUG LOG ]  Found import: " << converted.c_str() << std::endl;
#endif
                std::basic_string<T> inc_path = remove_whitespaces(match_res[1].str());
                if (inc_path.empty())
                {
                    auto where = line.find(CONVERT(T, '"'));
                    pos_t err_line = line_.at(where).line();
                    pos_t err_col = line_.at(where).col();
                    auto real_line = this->lines_raw.at(err_line);
                    ret = mk_unexpected_ret(Error<T, std::filesystem::path>(
                        ExcType::SYNTAX_ERROR,
                        "Empty import path",
                        this->file_path,
                        err_line + 1,
                        err_col + 1,
                        real_line
                    ));
                    co_yield ret;
                    continue;
                }
                ret = mk_expected_ret(inc_path, curr_line);
                // Delete the full line from this->lines (since the pragma is supposed to take the full line)
                /* this->lines_raw.erase(this->lines_raw.begin() + i); */
                this->lines.erase(this->lines.begin() + i);
                i--;
                co_yield ret;
                continue;
            }
            else if (std::regex_match(line, match_res, inc_regex_no_quotes))
            {
                // Error if there are some `<>"` in the line
                if (line.find(CONVERT(T, '"')) != std::basic_string<T>::npos ||
                    line.find(CONVERT(T, '<')) != std::basic_string<T>::npos ||
                    line.find(CONVERT(T, '>')) != std::basic_string<T>::npos)
                {
                    auto where = (line.find(CONVERT(T, '"')) != std::basic_string<T>::npos) ? line.find(CONVERT(T, '"')) :
                                 (line.find(CONVERT(T, '<')) != std::basic_string<T>::npos) ? line.find(CONVERT(T, '<')) :
                                 line.find(CONVERT(T, '>'));
                    pos_t err_line = line_.at(where).line();
                    pos_t err_col = line_.at(where).col();
                    auto real_line = this->lines_raw.at(err_line);
                    ret = mk_unexpected_ret(Error<T, std::filesystem::path>(
                        ExcType::SYNTAX_ERROR,
                        "Invalid import path",
                        this->file_path,
                        err_line + 1,
                        err_col + 1,
                        real_line
                    ));
                    co_yield ret;
                    continue;
                }
#if SUPDEF_DEBUG
                std::string converted = std::string(match_res[1].first, match_res[1].second);
                std::cerr << "[ DEBUG LOG ]  Found import: " << converted.c_str() << std::endl;
#endif
                std::basic_string<T> inc_path = remove_whitespaces(match_res[1].str());
                if (inc_path.empty())
                    SupDef::Util::unreachable();
                ret = mk_expected_ret(inc_path, curr_line);
                // Delete the full line from this->lines (since the pragma is supposed to take the full line)
                /* this->lines_raw.erase(this->lines_raw.begin() + i); */
                this->lines.erase(this->lines.begin() + i);
                i--;
                co_yield ret;
                continue;
            }
            else if (std::regex_match(line, match_res, inc_regex_no_path))
            {
                auto include_keyword_pos = line.find(ANY_STRING(T, SUPDEF_PRAGMA_IMPORT));
                if (include_keyword_pos == std::basic_string<T>::npos)
                    SupDef::Util::unreachable();
                pos_t err_line = line_.at(include_keyword_pos).line();
                pos_t err_col = line_.at(include_keyword_pos).col();
                auto real_line = this->lines_raw.at(err_line);
                ret = mk_unexpected_ret(Error<T, std::filesystem::path>(
                    ExcType::SYNTAX_ERROR,
                    "Include pragmas must have a path",
                    this->file_path,
                    err_line + 1,
                    err_col + 1,
                    real_line
                ));
                co_yield ret;
                continue;
            }
            else if (std::regex_match(line, match_res, inc_regex_with_anything))
            {
                auto include_keyword_pos = line.find(ANY_STRING(T, SUPDEF_PRAGMA_IMPORT));
                if (include_keyword_pos == std::basic_string<T>::npos)
                    SupDef::Util::unreachable();
                pos_t err_line = line_.at(include_keyword_pos).line();
                pos_t err_col = line_.at(include_keyword_pos).col();
                auto real_line = this->lines_raw.at(err_line);
                ret = mk_unexpected_ret(Error<T, std::filesystem::path>(
                    ExcType::SYNTAX_ERROR,
                    "Include pragmas must have a path",
                    this->file_path,
                    err_line + 1,
                    err_col + 1,
                    real_line
                ));
                co_yield ret;
                continue;
            }
        }
        /* this->file_content_raw = this->reassemble_lines(); */
        // Do the same for this->file_content

        this->file_content.clear();
        for (size_t counter = 0; counter < this->lines.size(); ++counter)
        {
            auto& line = this->lines[counter];
            for (auto& c : line)
                this->file_content.push_back(c);
            if (!this->file_content.empty())
            {
                for (auto& c : CONVERT(T, '\n'))
                    this->file_content.push_back(MK_CHAR(std::get<0>(
                        this->file_content.at(this->file_content.size() - 1)),
                        std::get<1>(this->file_content.at(this->file_content.size() - 1)),
                        c
                    ));
            }
            else
            {
                for (auto& c : CONVERT(T, '\n'))
                    this->file_content.push_back(MK_CHAR(0, 0, c));
            }
        }
        ret = mk_null_ret();
        co_return ret;
    }
#endif

#undef FILE_CONTENT
#undef MK_CHAR

    // TODO: Verify that supdef name doesn't start with a number
    // TODO: Verify that the pragma end is the end of the previously started supdef
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

        auto mk_expected_ret = [&mk_valid_pragmaloc](
            const std::basic_string<T>& content, /* A path, in our case here */
            const string_size_type<T>& start_line, /* Start line */
            const string_size_type<T>& end_line /* End line */
        ) -> RetType {
            return ::SupDef::make_ok_result<typename Parser<T>::pragma_loc_type, Error<T, std::filesystem::path>>(mk_valid_pragmaloc(content, start_line, end_line));
        };
        auto mk_null_ret = [ ](void) -> RetType {
            return ::SupDef::make_null_result<typename Parser<T>::pragma_loc_type, Error<T, std::filesystem::path>>();
        };
        auto mk_unexpected_ret = [ ](
            const Error<T, std::filesystem::path>& err
        ) -> RetType {
            return ::SupDef::make_err_result<typename Parser<T>::pragma_loc_type, Error<T, std::filesystem::path>>(err);
        };

        if (this->file_content.empty() || this->lines.empty())
        {
            ret = mk_unexpected_ret(Error<T, std::filesystem::path>(ExcType::INTERNAL_ERROR, "File content is empty"));
            co_return ret;
        }
        
        pos_t curr_line = 0;
        pos_t pragma_start_pos = 0;
        pos_t pragma_end_pos = 0;
        std::basic_string<T> pragma_content; // Pragma name will be prepended as the first line of the pragma content
        std::basic_string<T> supdef_name;
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
            //    "^\\s*#\\s*pragma\\s+" "supdef" "\\s+" "begin" "\\s+(" "\\w+" ")\\s*$"
            regex_t pragma_start_regex(ANY_STRING(T, SUPDEF_PRAGMA_DEF_BEG_REGEX), std::regex_constants::ECMAScript);
            // With `SUPDEF_PRAGMA_DEF_END_REGEX` expanding to:
            //    "^\\s*#\\s*pragma\\s+" "supdef" "\\s+" "end" "\\s+(" "\\w+" ")\\s*$"
            regex_t pragma_end_regex(ANY_STRING(T, SUPDEF_PRAGMA_DEF_END_REGEX), std::regex_constants::ECMAScript);
            match_res_t match_res;
            try
            {
                if (std::regex_match(line, match_res, pragma_start_regex))
                {
#if 0
                    std::cout << "Matched line: " << CONVERT(char, line) << std::endl;
#endif
                    if (in_supdef_body)
                    {
                        string_size_type<T> begin_kwd_pos = line.find(ANY_STRING(T, SUPDEF_PRAGMA_DEFINE_BEGIN));
                        auto err_line = line_.at(begin_kwd_pos).line();
                        auto err_col = line_.at(begin_kwd_pos).col();
                        auto real_line = this->lines_raw.at(err_line);
                        ret = mk_unexpected_ret(Error<T, std::filesystem::path>(
                            ExcType::SYNTAX_ERROR,
                            "Pragma start found while already in a supdef block",
                            this->file_path,
                            err_line + 1,
                            err_col + 1,
                            real_line
                        ));
                        // For now, straight up `co_return` as we don't want to parse anymore, since it would be useless
                        // and probably cause more errors
                        co_return ret;
                    }
                    pragma_start_pos = curr_line;
                    in_supdef_body = true;
                    supdef_name = remove_whitespaces(match_res[1].str(), true);
                    pragma_content.clear();
                    pragma_content += supdef_name;
                    pragma_content += CONVERT(T, '\n');
                    // Remove the line from this->lines
                    this->lines.erase(this->lines.begin() + i);
                    /* this->lines_raw.erase(this->lines_raw.begin() + i); */
                    i--;
                }
                else if (std::regex_match(line, match_res, pragma_end_regex))
                {
                    if (match_res[0].first != line.begin())
                    {
                        auto err_line = line_.at(0).line();
                        auto err_col = line_.at(0).col();
                        auto real_line = this->lines_raw.at(err_line);
                        ret = mk_unexpected_ret(Error<T, std::filesystem::path>(
                            ExcType::SYNTAX_ERROR,
                            "Pragma end found not at start of line",
                            this->file_path,
                            err_line + 1,
                            err_col + 1,
                            real_line
                        ));
                        co_return ret;
                    }
                    if (!in_supdef_body)
                    {
                        string_size_type<T> end_kwd_pos = line.find(ANY_STRING(T, SUPDEF_PRAGMA_DEFINE_END));
                        auto err_line = line_.at(end_kwd_pos).line();
                        auto err_col = line_.at(end_kwd_pos).col();
                        auto real_line = this->lines_raw.at(err_line);
                        ret = mk_unexpected_ret(Error<T, std::filesystem::path>(
                            ExcType::SYNTAX_ERROR,
                            "Pragma end found while not in a supdef block",
                            this->file_path,
                            err_line + 1,
                            err_col + 1,
                            real_line
                        ));
                        co_return ret; // (same comment as above)
                    }
                    if (remove_whitespaces(match_res[1].str(), true) != supdef_name)
                    {
                        // Find position of `remove_whitespaces(match_res[1].str(), true)`, after the `end` keyword
                        string_size_type<T> supdef_name_pos = line.rfind(remove_whitespaces(match_res[1].str(), true));
                        auto err_line = line_.at(supdef_name_pos).line();
                        auto err_col = line_.at(supdef_name_pos).col();
                        auto real_line = this->lines_raw.at(err_line);
                        auto get_first_line = [ ](const std::basic_string<T>& str) -> std::basic_string<T> {
                            std::basic_string<T> res;
                            res.reserve(str.size());
                            for (auto& c : str)
                            {
                                if (SAME(c, '\n'))
                                    break;
                                res += c;
                            }
                            return res;
                        };
                        ret = mk_unexpected_ret(Error<T, std::filesystem::path>(
                            ExcType::SYNTAX_ERROR,
                            "Pragma end found with a different supdef name than the start (should be `"s + CONVERT(char, remove_whitespaces(get_first_line(pragma_content), true)) + "` instead)"s,
                            this->file_path,
                            err_line + 1,
                            err_col + 1,
                            real_line
                        ));
                        co_return ret;
                    }
                    pragma_end_pos = curr_line;
                    // Remove the line from this->lines
                    this->lines.erase(this->lines.begin() + i);
                    /* this->lines_raw.erase(this->lines_raw.begin() + i); */
                    i--;
                    // Add pragma start and end to location vector
                    ret = mk_expected_ret(pragma_content, pragma_start_pos, pragma_end_pos);
                    co_yield ret;
                    in_supdef_body = false;
                }
                else if (in_supdef_body)
                {
                    pragma_content += line;
                    pragma_content += CONVERT(T, '\n');
                    // Remove the line from this->lines
                    this->lines.erase(this->lines.begin() + i);
                    /* this->lines_raw.erase(this->lines_raw.begin() + i); */
                    i--;
                }
            }
            catch (const std::exception& e)
            {
                ret = mk_unexpected_ret(Error<T, std::filesystem::path>(
                    ExcType::INTERNAL_ERROR,
                    "Caught exception while parsing supdefinitions (in function `" + std::string(__PRETTY_FUNCTION__) + "`): " + std::string(e.what()) + "\n"
                ));
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
        for (size_t counter = 0; counter < this->lines.size(); ++counter)
        {
            auto& line = this->lines[counter];
            for (auto& c : line)
                this->file_content.push_back(c);
            if (!this->file_content.empty())
            {
                for (auto& c : CONVERT(T, '\n'))
                    this->file_content.push_back(MK_CHAR(std::get<0>(
                        this->file_content.at(this->file_content.size() - 1)),
                        std::get<1>(this->file_content.at(this->file_content.size() - 1)),
                        c
                    ));
            }
            else
            {
                for (auto& c : CONVERT(T, '\n'))
                    this->file_content.push_back(MK_CHAR(0, 0, c));
            }
        }
        ret = mk_null_ret();
#undef MK_CHAR
        co_return ret;
    }

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