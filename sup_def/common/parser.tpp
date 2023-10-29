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

#ifdef MK_CHAR
    #undef MK_CHAR
#endif
#define MK_CHAR(ORIGINAL_LINE, ORIGINAL_COL, CHAR) std::make_tuple<string_size_type<T>, string_size_type<T>, T>(std::forward<string_size_type<T>>(ORIGINAL_LINE), std::forward<string_size_type<T>>(ORIGINAL_COL), std::forward<T>(CHAR))

template <typename T>
    requires CharacterType<T>
Parser<T>::Parser()
{
    try
    {
        this->file = std::make_unique<std::basic_ifstream<T>>();
        this->file_content_raw = std::make_shared<std::basic_string<T>>();
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
        this->file = std::make_unique<std::basic_ifstream<T>>(file_path);
        this->file_content_raw = std::make_shared<std::basic_string<T>>();
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

#if defined(SUPDEF_DEBUG)
    template <typename T>
        requires CharacterType<T>
    template <typename Stream>
    void Parser<T>::print_content(Stream& s) const
    {
        if (this->file_content_raw == nullptr)
            throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "File content is null\n");
        if (this->file_content_raw.get()->empty())
            throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "File content is empty\n");
        s << *(this->file_content_raw) << std::endl;
    }
#endif

template <typename T>
    requires CharacterType<T>
std::shared_ptr<std::basic_string<T>> Parser<T>::slurp_file()
{
    if (this->file_content_raw == nullptr)
        throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "File content is null\n");
    if (!this->file.get()->is_open())
        throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "File is not open\n");

    this->file.get()->seekg(0, std::basic_ios<T>::end);
    try
    {
        this->file_content_raw.get()->reserve(this->file.get()->tellg());
    }
    catch (const std::exception& e)
    {
        throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "Failed to reserve file content: " + std::string(e.what()) + "\n");
    }
    this->file.get()->seekg(0, std::basic_ios<T>::beg);

    try
    {
        this->file_content_raw.get()->clear();
        std::basic_ostringstream<T> sstr;
        sstr << this->file.get()->rdbuf();
        this->file_content_raw.get()->assign(sstr.str());
    }
    catch (const std::exception& e)
    {
        throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "Failed to read file content: " + std::string(e.what()) + "\n");
    }
    this->lines_raw.clear();
#if defined(__cpp_lib_containers_ranges) && __cpp_lib_containers_ranges >= 202202L && 0 // For now, disable this
    // Not even sure this works
    this->lines_raw.assign_range(split_string(*(this->file_content_raw.get()), '\n'));
#else
    this->lines_raw = split_string(*(this->file_content_raw.get()), static_cast<T>('\n'));
#endif
    this->file_content.clear();
    this->lines.clear();
    string_size_type<T> linecount = 0;
    string_size_type<T> colcount = 0;
    std::vector<std::tuple<string_size_type<T>, string_size_type<T>, T>> line;
    for (T c : *(this->file_content_raw.get()))
    {
        if (c == static_cast<T>('\n'))
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
#define FILE_CONTENT(POS) (this->file_content_raw.get()->at(POS))

    if (this->file_content_raw == nullptr)
        throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "File content is null\n");
    if (this->file_content_raw.get()->empty() || this->file_content.empty())
        throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "File content is empty\n");
    
    [[maybe_unused]]
    size_t pp_if_level = 0;
    bool in_str_lit = false; // Are we in a string literal ?

    typedef T local_char_type;
    typedef std::basic_string<T> local_string_type;
    typedef typename local_string_type::size_type local_size_type;
    
    for (local_size_type i = 0; i < this->file_content_raw.get()->size(); ++i)
    {
        if (FILE_CONTENT(i) == static_cast<local_char_type>('"'))
        {
            if (i > 0 && FILE_CONTENT(i - 1) != static_cast<local_char_type>('\\'))
                in_str_lit = !in_str_lit;
        }
        else if (!in_str_lit)
        {
            if (FILE_CONTENT(i) == static_cast<local_char_type>('/'))
            {
                if (i + 1 < this->file_content_raw.get()->size())
                {
                    if (FILE_CONTENT(i + 1) == static_cast<local_char_type>('/'))
                    {
                        // Check for `\\` and erase the following line if present
                        local_size_type next_nl_pos = this->file_content_raw.get()->find(static_cast<local_char_type>('\n'), i);
                        local_size_type next_bsl_pos = this->file_content_raw.get()->find(static_cast<local_char_type>('\\'), i);
                        while ((next_bsl_pos != std::basic_string<T>::npos  || 
                                next_nl_pos  != std::basic_string<T>::npos) &&
                                next_bsl_pos < next_nl_pos)
                        {
                            next_nl_pos = this->file_content_raw.get()->find(static_cast<local_char_type>('\n'), next_nl_pos + 1);
                            next_bsl_pos = this->file_content_raw.get()->find(static_cast<local_char_type>('\\'), next_bsl_pos + 1);
                        }
                        if (next_nl_pos != std::basic_string<T>::npos)
                        {
                            this->file_content_raw.get()->erase(i, next_nl_pos - i);
                            this->file_content.erase(this->file_content.begin() + i, this->file_content.begin() + next_nl_pos);
                            // Erase the possible '\t' or ' ' before the erased content
                            while (i > 0 && (FILE_CONTENT(i - 1) == static_cast<local_char_type>('\t') || FILE_CONTENT(i - 1) == static_cast<local_char_type>(' ')))
                            {
                                this->file_content_raw.get()->erase(i - 1, 1);
                                this->file_content.erase(this->file_content.begin() + i - 1);
                                i--;
                            }                                
                        }
                        else // erase until the end of the file
                        {
                            this->file_content_raw.get()->erase(i, this->file_content_raw.get()->size() - i);
                            this->file_content.erase(this->file_content.begin() + i, this->file_content.end());
                        }
                        i--;
                    }
                    else if (FILE_CONTENT(i + 1) == static_cast<local_char_type>('*'))
                    {
                        auto to_find = any_string<T>("*/");
                        local_size_type end_comment_pos = this->file_content_raw.get()->find(to_find.data(), i);
                        if (end_comment_pos == std::basic_string<T>::npos)
                        {
                            // Put the line, column and line content in the exception
                            size_t line = 1;
                            size_t column = 1;
                            for (size_t j = 0; j < i; j++)
                            {
                                if (FILE_CONTENT(j) == static_cast<local_char_type>('\n'))
                                {
                                    line++;
                                    column = 1;
                                }
                                else
                                    column++;
                            }
                            throw Exception<T, std::filesystem::path>(ExcType::SYNTAX_ERROR, "Unterminated comment\n", this->file_path, line, column, this->lines_raw.at(line - 1));
                        }
                        this->file_content_raw.get()->erase(i, end_comment_pos - i + 2);
                        this->file_content.erase(this->file_content.begin() + i, this->file_content.begin() + end_comment_pos + 2);
                        // Add a space instead, as specified in the standard
                        this->file_content_raw.get()->insert(i, any_string<T>(" ").data());
                        this->file_content.insert(this->file_content.begin() + i, MK_CHAR(std::get<0>(this->file_content.at(i)), std::get<1>(this->file_content.at(i)), static_cast<local_char_type>(' ')));
                        i--;
                    }
                }
            }
        }
    }

    if (this->file_content_raw.get()->size() != this->file_content.size())
        throw Exception<T, std::filesystem::path>(ExcType::INTERNAL_ERROR, "File content and file content raw are not the same size\n");
    for (string_size_type<T> i = 0; i < this->file_content_raw.get()->size(); ++i)
    {
        if (FILE_CONTENT(i) != std::get<2>(this->file_content.at(i)))
            throw Exception<T, std::filesystem::path>(ExcType::INTERNAL_ERROR, "File content and file content raw are not the same\n");
    }

#if 0 //SUPDEF_REMOVE_IF_0_BLOCKS

    auto get_current_pp_line = [&](const local_size_type pos) -> std::basic_string<T>
    {
        std::basic_string<T> line;
        for (auto i = pos; i < this->file_content.get()->size(); ++i)
        {
            if (FILE_CONTENT(i) == static_cast<local_char_type>('\\')) // Line continuation
            {
                if (i + 1 < this->file_content.get()->size())
                {
                    if (FILE_CONTENT(i + 1) == static_cast<local_char_type>('\n'))
                    {
                        i++;
                        continue;
                    }
                    else
                    {} // C/C++ syntax error or we are in a string literal
                }
                else break;
            }
            else if (FILE_CONTENT(i) == static_cast<local_char_type>('\n'))
                break;
            line += FILE_CONTENT(i);
        }
        return line;
    };

    auto get_current_pp_line_nb = [&](const local_size_type pos) -> std::tuple<local_size_type, local_size_type> // < current line nb, start of line pos >
    {
        local_size_type line_nb = 1;
        local_size_type line_start_pos = 0;
        for (local_size_type i = 0; i < pos; ++i)
        {
            if (FILE_CONTENT(i) == static_cast<local_char_type>('\n'))
            {
                line_nb++;
                line_start_pos = i + 1;
            }
        }
        return std::make_tuple(line_nb, line_start_pos);
    };
    
    auto skip_wsp = [&](const local_size_type pos) -> local_size_type
    {
        local_size_type i = pos;
        while (i < this->file_content.get()->size() && (FILE_CONTENT(i) == static_cast<local_char_type>(' ') || FILE_CONTENT(i) == static_cast<local_char_type>('\t')))
            i++;
        return i;
    };
    // Note: this function doesn't search for `c` in string literals
    // Returns a tuple containing a boolean and an offset
    auto is_in_pp_line = [&](const local_size_type pos,
                             const typename std::basic_string<T>::value_type c,
                             bool in_str) -> std::tuple<bool, local_size_type>
    {
        if (c == static_cast<decltype(c)>('"')) return std::make_tuple(false, 0);            
        local_size_type i = pos;
        while (i < this->file_content.get()->size())
        {
            if (FILE_CONTENT(i) == static_cast<decltype(c)>('"'))
            {
                if (i > 0 && FILE_CONTENT(i - 1) != static_cast<decltype(c)>('\\'))
                    in_str = !in_str;
            }
            if (FILE_CONTENT(i) == static_cast<decltype(c)>('\\') && !in_str) // Line continuation
            {
                if (i + 1 < this->file_content.get()->size())
                {
                    if (FILE_CONTENT(i + 1) == static_cast<decltype(c)>('\n'))
                    {
                        i++;
                        continue;
                    }
                    else
                    {} // C/C++ syntax error here but don't do anything for now
                }
                else break;
            }
            else if (FILE_CONTENT(i) == static_cast<decltype(c)>('\n'))
                break;
            else if (FILE_CONTENT(i) == c && !in_str)
                return std::make_tuple(true, i - pos);
            i++;
        }
        return std::make_tuple(false, 0);
    };

    auto is_str_in_pp_line = [&](local_size_type pos,
                                 const std::basic_string<T>& str,
                                 bool in_str) -> std::tuple<bool, local_size_type>
    {
        if (str.empty())
            return std::make_tuple(false, 0);
        else if (str.size() == 1)
            return is_in_pp_line(pos, str[0], in_str);

        // Calling `is_in_pp_line` ensures that we do not mess with string literals
        auto [is_in, offset] = is_in_pp_line(pos, str[0], in_str);
        if (!is_in)
            return std::make_tuple(false, 0);
        for (local_value_type c : str.substr(1)) // minus first char because already checked
        {
            if (FILE_CONTENT(pos + offset) != c)
                return std::make_tuple(false, 0);
            ++pos;
        }
        return std::make_tuple(true, offset);
    };
    // Now, remove `#if 0` blocks
    local_size_type first_lvl_if_0 = std::basic_string<T>::npos;
    in_str_lit = false;
    for (local_size_type i = 0; i < this->file_content.get()->size(); ++i)
    {
        if (FILE_CONTENT(i) == static_cast<local_char_type>('"'))
        {
            if (i > 0 && FILE_CONTENT(i - 1) != static_cast<local_char_type>('\\'))
                in_str_lit = !in_str_lit;
        }
        else if (!in_str_lit)
        {
            if (FILE_CONTENT(i) == static_cast<local_char_type>('#'))
            {
                auto t = is_str_in_pp_line(i, any_string<T>("if").data(), in_str_lit);
                bool is_in = std::get<0>(t);
                local_size_type offset = std::get<1>(t);
                // Verify that the beginning of the `if` is after whitespaces only
                if (is_in && (offset == skip_wsp(i) - i))
                {
                    auto [is_in, offset] = is_str_in_pp_line(i, any_string<T>("0").data(), in_str_lit);
                    if (is_in && (offset == skip_wsp(i) - i))
                    {
                        if (first_lvl_if_0 == std::basic_string<T>::npos)
                            first_lvl_if_0 = i;
                        pp_if_level++;
                    }
                    goto end_if;
                }
                t = is_str_in_pp_line(i, any_string<T>("endif").data(), in_str_lit);
                is_in = std::get<0>(t);
                offset = std::get<1>(t);
                if (is_in && (offset == skip_wsp(i) - i))
                {
                    if (pp_if_level == 0 || first_lvl_if_0 == std::basic_string<T>::npos)
                        throw Exception<T, std::filesystem::path>(
                            ExcType::SYNTAX_ERROR,
                            "Unexpected `#endif`",
                            this->file_path,
                            std::get<0>(get_current_pp_line_nb(i)),
                            i - std::get<1>(get_current_pp_line_nb(i)) + offset, // point to the `endif` keyword, not the `#`, in case it's written `# endif`
                            get_current_pp_line(i)
                        );
                    pp_if_level--;
                    if (pp_if_level == 0)
                    {
                        this->file_content.get()->erase(first_lvl_if_0, i - first_lvl_if_0);
                        i = first_lvl_if_0;
                        first_lvl_if_0 = std::basic_string<T>::npos;
                    }
                    goto end_if;
                }
                t = is_str_in_pp_line(i, any_string<T>("else").data(), in_str_lit);
                is_in = std::get<0>(t);
                offset = std::get<1>(t);
                if (is_in && (offset == skip_wsp(i) - i))
                {
                    if (pp_if_level == 0 || first_lvl_if_0 == std::basic_string<T>::npos)
                        throw Exception<T, std::filesystem::path>(
                            ExcType::SYNTAX_ERROR,
                            "Unexpected `#else`",
                            this->file_path,
                            std::get<0>(get_current_pp_line_nb(i)),
                            i - std::get<1>(get_current_pp_line_nb(i)) + offset,
                            get_current_pp_line(i)
                        );
                    goto end_if;
                }
end_if:
                if (pp_if_level == 0 && first_lvl_if_0 != std::basic_string<T>::npos)
                {
                    this->file_content.get()->erase(first_lvl_if_0, i - first_lvl_if_0);
                    i = first_lvl_if_0;
                    first_lvl_if_0 = std::basic_string<T>::npos;
                }
            }
        }
    }
    if (pp_if_level != 0)
        throw Exception<T, std::filesystem::path>(
            ExcType::SYNTAX_ERROR,
            "Unterminated `#if 0` block",
            this->file_path,
            std::get<0>(get_current_pp_line_nb(first_lvl_if_0)),
            first_lvl_if_0 - std::get<1>(get_current_pp_line_nb(first_lvl_if_0)),
            get_current_pp_line(first_lvl_if_0)
        );
#endif

    this->lines_raw.clear();
#if defined(__cpp_lib_containers_ranges) && __cpp_lib_containers_ranges >= 202202L && 0 // For now, disable this
    // Not even sure this works
    this->lines_raw.assign_range(split_string(*(this->file_content_raw.get()), '\n'));
#else
    this->lines_raw = split_string(*(this->file_content_raw.get()), static_cast<T>('\n'));
#endif
    // Also resplit the lines
    this->lines.clear();
    string_size_type<T> linecount = 0;
    string_size_type<T> colcount = 0;
    typedef typename std::tuple<string_size_type<T>, string_size_type<T>, T> character_t;
    std::vector<character_t> line;
    for (character_t c : this->file_content)
    {
        if (std::get<2>(c) == static_cast<T>('\n'))
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
    if (this->file_content_raw == nullptr)
        throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "File content is null\n");
    if (this->file_content_raw.get()->empty() || this->file_content.empty())
        throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "File content is empty\n");

    std::basic_string<T> res;
    res.reserve(this->file_content_raw.get()->size());
    bool in_str_lit = false;
    for (size_t i = 0; i < this->file_content_raw.get()->size(); ++i)
    {
        if (FILE_CONTENT(i) == '"')
        {
            if (i > 0 && FILE_CONTENT(i - 1) != '\\')
                in_str_lit = !in_str_lit;
        }
        else if (!in_str_lit)
            res += FILE_CONTENT(i);
    }
    return res;
}

template <typename T>
    requires CharacterType<T>
std::shared_ptr<std::basic_string<T>> Parser<T>::reassemble_lines(void)
{
    if (this->lines_raw.empty())
        throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "Lines are empty\n");
    std::basic_string<T> res;
    res.reserve(this->file_content_raw.get()->size());
    for (auto& line : this->lines_raw)
        res += line + static_cast<T>('\n');
    return std::make_shared<std::basic_string<T>>(res);
}


#if SUPDEF_WORKAROUND_GCC_INTERNAL_ERROR
// Just do the same but as a friend function
template <typename T>
    requires CharacterType<T>
Coro<Result<std::shared_ptr<std::basic_string<T>>, Error<T, std::filesystem::path>>> search_includes(Parser<T>& parser)
{
    auto this_ = &parser;
    if (this_->file_content.empty() || this_->lines.empty())
        co_return { Error<T, std::filesystem::path>(ExcType::INTERNAL_ERROR, "File content is empty or lines are empty\n") };

    typedef typename std::basic_regex<T> regex_t;
    typedef std::match_results<typename std::basic_string<T>::const_iterator> match_res_t;
    typedef typename std::vector<std::basic_string<T>>::size_type line_num_t;
    typedef typename std::basic_string<T>::size_type pos_t;

    for (line_num_t i = 0; i < this_->lines.size(); ++i)
    {
        std::vector<std::tuple<string_size_type<T>,string_size_type<T>,T>> line_ = this_->lines[i];
        std::basic_string<T> line;
        line.reserve(line_.size());
        for (auto& c : line_)
            line += std::get<2>(c);
        const regex_t inc_regex(ANY_STRING(T, SUPDEF_PRAGMA_INCLUDE_REGEX), std::regex_constants::ECMAScript);
        const regex_t inc_regex_angle_brackets(ANY_STRING(T, SUPDEF_PRAGMA_INCLUDE_REGEX_ANGLE_BRACKETS), std::regex_constants::ECMAScript);
        const regex_t inc_regex_no_quotes(ANY_STRING(T, SUPDEF_PRAGMA_INCLUDE_REGEX_NO_QUOTES), std::regex_constants::ECMAScript);
        // This is invalid and must be reported as a syntax error
        const regex_t inc_regex_no_path(ANY_STRING(T, SUPDEF_PRAGMA_INCLUDE_REGEX_NO_PATH), std::regex_constants::ECMAScript);
        // Invalid too
        const regex_t inc_regex_with_anything(ANY_STRING(T, "^\\s*#\\s*pragma\\s+" SUPDEF_PRAGMA_NAME "\\s+" SUPDEF_PRAGMA_INCLUDE ".*$"), std::regex_constants::ECMAScript);
        match_res_t match_res;
        if (std::regex_match(line, match_res, inc_regex))
        {
            pos_t l_bracket_pos = line.find(static_cast<T>('<'));
            pos_t r_bracket_pos = line.find(static_cast<T>('>'));
            if (l_bracket_pos != std::basic_string<T>::npos || r_bracket_pos != std::basic_string<T>::npos)
            {
                pos_t err_line = std::get<0>(line_.at(0));
                pos_t err_col = (l_bracket_pos != std::basic_string<T>::npos) ? std::get<1>(line_.at(l_bracket_pos)) : std::get<1>(line_.at(r_bracket_pos));
                co_yield Result<std::shared_ptr<std::basic_string<T>>, Error<T, std::filesystem::path>>{ Error<T, std::filesystem::path>(
                    ExcType::SYNTAX_ERROR,
                    "You cannot mix `<>` with `\"\"` in include pragmas",
                    this_->file_path,
                    err_line + 1,
                    err_col + 1,
                    line
                ) };
                continue;
            }
#if SUPDEF_DEBUG
            std::string converted = std::string(match_res[1].first, match_res[1].second);
            std::cerr << "[ DEBUG LOG ]  Found include: " << converted.c_str() << std::endl;
#endif
            std::basic_string<T> inc_path = remove_whitespaces(match_res[1].str());
            if (inc_path.empty())
            {
                pos_t err_line = std::get<0>(line_.at(0));
                pos_t err_col = std::get<1>(line_.at(line.find(static_cast<T>('"'))));
                co_yield Result<std::shared_ptr<std::basic_string<T>>, Error<T, std::filesystem::path>>{ Error<T, std::filesystem::path>(
                    ExcType::SYNTAX_ERROR,
                    "Empty include path",
                    this_->file_path,
                    err_line + 1,
                    err_col + 1,
                    line
                ) };
                continue;
            }
            co_yield Result<std::shared_ptr<std::basic_string<T>>, Error<T, std::filesystem::path>>{ std::make_shared<std::basic_string<T>>(inc_path) };
            // Delete the full line from this_->lines (since the pragma is supposed to take the full line)
            this_->lines_raw.erase(this_->lines_raw.begin() + i);
            this_->lines.erase(this_->lines.begin() + i);
            i--;
        }
        else if (std::regex_match(line, match_res, inc_regex_angle_brackets))
        {
            pos_t quote_pos = line.find(static_cast<T>('"'));
            if (quote_pos != std::basic_string<T>::npos)
            {
                pos_t err_line = std::get<0>(line_.at(0));
                pos_t err_col = std::get<1>(line_.at(quote_pos));
                co_yield Result<std::shared_ptr<std::basic_string<T>>, Error<T, std::filesystem::path>>{ Error<T, std::filesystem::path>(
                    ExcType::SYNTAX_ERROR,
                    "You cannot mix `\"\"` with `<>` in include pragmas",
                    this_->file_path,
                    err_line + 1,
                    err_col + 1,
                    line
                ) };
                continue;
            }
#if SUPDEF_DEBUG
            std::string converted = std::string(match_res[1].first, match_res[1].second);
            std::cerr << "[ DEBUG LOG ]  Found include: " << converted.c_str() << std::endl;
#endif
            std::basic_string<T> inc_path = remove_whitespaces(match_res[1].str());
            if (inc_path.empty())
            {
                pos_t err_line = std::get<0>(line_.at(0));
                pos_t err_col = std::get<1>(line_.at(line.find(static_cast<T>('"'))));
                co_yield Result<std::shared_ptr<std::basic_string<T>>, Error<T, std::filesystem::path>>{ Error<T, std::filesystem::path>(
                    ExcType::SYNTAX_ERROR,
                    "Empty include path",
                    this_->file_path,
                    err_line + 1,
                    err_col + 1,
                    line
                ) };
                continue;
            }
            co_yield Result<std::shared_ptr<std::basic_string<T>>, Error<T, std::filesystem::path>>{ std::make_shared<std::basic_string<T>>(inc_path) };
            // Delete the full line from this_->lines (since the pragma is supposed to take the full line)
            this_->lines_raw.erase(this_->lines_raw.begin() + i);
            this_->lines.erase(this_->lines.begin() + i);
            i--;
        }
        else if (std::regex_match(line, match_res, inc_regex_no_quotes))
        {
            // Error if there are some `<>"` in the line
            if (line.find(static_cast<T>('"')) != std::basic_string<T>::npos ||
                line.find(static_cast<T>('<')) != std::basic_string<T>::npos ||
                line.find(static_cast<T>('>')) != std::basic_string<T>::npos)
            {
                pos_t err_line = std::get<0>(line_.at(0));
                pos_t err_col = std::get<1>(line_.at(
                    (line.find(static_cast<T>('"')) != std::basic_string<T>::npos) ? line.find(static_cast<T>('"')) :
                    (line.find(static_cast<T>('<')) != std::basic_string<T>::npos) ? line.find(static_cast<T>('<')) :
                    line.find(static_cast<T>('>'))
                ));
                co_yield Result<std::shared_ptr<std::basic_string<T>>, Error<T, std::filesystem::path>>{ Error<T, std::filesystem::path>(
                    ExcType::SYNTAX_ERROR,
                    "Invalid include path",
                    this_->file_path,
                    err_line + 1,
                    err_col + 1,
                    line
                ) };
                continue;
            }
#if SUPDEF_DEBUG
            std::string converted = std::string(match_res[1].first, match_res[1].second);
            std::cerr << "[ DEBUG LOG ]  Found include: " << converted.c_str() << std::endl;
#endif
            std::basic_string<T> inc_path = remove_whitespaces(match_res[1].str());
            if (inc_path.empty())
                SupDef::Util::unreachable();
            co_yield Result<std::shared_ptr<std::basic_string<T>>, Error<T, std::filesystem::path>>{ std::make_shared<std::basic_string<T>>(inc_path) };
            // Delete the full line from this_->lines (since the pragma is supposed to take the full line)
            this_->lines_raw.erase(this_->lines_raw.begin() + i);
            this_->lines.erase(this_->lines.begin() + i);
            i--;
        }
        else if (std::regex_match(line, match_res, inc_regex_no_path))
        {
            auto include_keyword_pos = line.find(ANY_STRING(T, SUPDEF_PRAGMA_INCLUDE));
            if (include_keyword_pos == std::basic_string<T>::npos)
                SupDef::Util::unreachable();
            pos_t err_line = std::get<0>(line_.at(0));
            pos_t err_col = std::get<1>(line_.at(include_keyword_pos));
            co_yield Result<std::shared_ptr<std::basic_string<T>>, Error<T, std::filesystem::path>>{ Error<T, std::filesystem::path>(
                ExcType::SYNTAX_ERROR,
                "Include pragmas must have a path",
                this_->file_path,
                err_line + 1,
                err_col + 1,
                line
            ) };
            continue;
        }
        else if (std::regex_match(line, match_res, inc_regex_with_anything))
        {
            auto include_keyword_pos = line.find(ANY_STRING(T, SUPDEF_PRAGMA_INCLUDE));
            if (include_keyword_pos == std::basic_string<T>::npos)
                SupDef::Util::unreachable();
            pos_t err_line = std::get<0>(line_.at(0));
            pos_t err_col = std::get<1>(line_.at(include_keyword_pos));
            co_yield Result<std::shared_ptr<std::basic_string<T>>, Error<T, std::filesystem::path>>{ Error<T, std::filesystem::path>(
                ExcType::SYNTAX_ERROR,
                "Include pragmas must have a path",
                this_->file_path,
                err_line + 1,
                err_col + 1,
                line
            ) };
            continue;
        }
    }
    this_->file_content_raw = this_->reassemble_lines();
    // Do the same for this_->file_content
    this_->file_content.clear();
    this_->file_content.push_back(MK_CHAR(0, 0, static_cast<T>('\n'))); // dummy line
    for (size_t counter = 0; counter < this_->lines.size(); ++counter)
    {
        auto& line = this_->lines[counter];
        for (auto& c : line)
            this_->file_content.push_back(c);
        this_->file_content.push_back(MK_CHAR(std::get<0>(this_->file_content.at(this_->file_content.size() - 1)), std::get<1>(this_->file_content.at(this_->file_content.size() - 1)), static_cast<T>('\n')));
    }
    co_return Result<std::shared_ptr<std::basic_string<T>>, Error<T, std::filesystem::path>>(nullptr);
}
#else
// The same as above but as a member function
template <typename T>
    requires CharacterType<T>
Coro<Result<typename Parser<T>::pragma_loc_type, Error<T, std::filesystem::path>>> Parser<T>::search_includes(void)
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

    if (this->file_content.empty() || this->lines.empty())
    {
        ret = Result<Parser<T>::pragma_loc_type, Error<T, std::filesystem::path>>{ Error<T, std::filesystem::path>(ExcType::INTERNAL_ERROR, "File content is empty or lines are empty\n") };
        co_return ret;
    }

    typedef typename std::basic_regex<T> regex_t;
    typedef std::match_results<typename std::basic_string<T>::const_iterator> match_res_t;
    typedef typename std::vector<std::basic_string<T>>::size_type line_num_t;
    typedef typename std::basic_string<T>::size_type pos_t;

    for (line_num_t i = 0; i < this->lines.size(); ++i)
    {
        std::vector<std::tuple<string_size_type<T>,string_size_type<T>,T>> line_ = this->lines[i];
        std::basic_string<T> line;
        line.reserve(line_.size());
        if (line_.empty())
            continue;
        else
            curr_line = std::get<0>(line_.at(0)) + 1;
        for (auto& c : line_)
            line += std::get<2>(c);
        const regex_t inc_regex(ANY_STRING(T, SUPDEF_PRAGMA_INCLUDE_REGEX), std::regex_constants::ECMAScript);
        const regex_t inc_regex_angle_brackets(ANY_STRING(T, SUPDEF_PRAGMA_INCLUDE_REGEX_ANGLE_BRACKETS), std::regex_constants::ECMAScript);
        const regex_t inc_regex_no_quotes(ANY_STRING(T, SUPDEF_PRAGMA_INCLUDE_REGEX_NO_QUOTES), std::regex_constants::ECMAScript);
        // This is invalid and must be reported as a syntax error
        const regex_t inc_regex_no_path(ANY_STRING(T, SUPDEF_PRAGMA_INCLUDE_REGEX_NO_PATH), std::regex_constants::ECMAScript);
        // Invalid too
        const regex_t inc_regex_with_anything(ANY_STRING(T, "^\\s*#\\s*pragma\\s+" SUPDEF_PRAGMA_NAME "\\s+" SUPDEF_PRAGMA_INCLUDE ".*$"), std::regex_constants::ECMAScript);
        match_res_t match_res;
        if (std::regex_match(line, match_res, inc_regex))
        {
            pos_t l_bracket_pos = line.find(static_cast<T>('<'));
            pos_t r_bracket_pos = line.find(static_cast<T>('>'));
            if (l_bracket_pos != std::basic_string<T>::npos || r_bracket_pos != std::basic_string<T>::npos)
            {
                pos_t err_line = std::get<0>(line_.at(0));
                pos_t err_col = (l_bracket_pos != std::basic_string<T>::npos) ? std::get<1>(line_.at(l_bracket_pos)) : std::get<1>(line_.at(r_bracket_pos));
                ret = Result<Parser<T>::pragma_loc_type, Error<T, std::filesystem::path>>{ Error<T, std::filesystem::path>(
                    ExcType::SYNTAX_ERROR,
                    "You cannot mix `<>` with `\"\"` in include pragmas",
                    this->file_path,
                    err_line + 1,
                    err_col + 1,
                    line
                ) };
                co_yield ret;
                continue;
            }
#if SUPDEF_DEBUG
            std::string converted = std::string(match_res[1].first, match_res[1].second);
            std::cerr << "[ DEBUG LOG ]  Found include: " << converted.c_str() << std::endl;
#endif
            std::basic_string<T> inc_path = remove_whitespaces(match_res[1].str());
            if (inc_path.empty())
            {
                pos_t err_line = std::get<0>(line_.at(0));
                pos_t err_col = std::get<1>(line_.at(line.find(static_cast<T>('"'))));
                ret = Result<Parser<T>::pragma_loc_type, Error<T, std::filesystem::path>>{ Error<T, std::filesystem::path>(
                    ExcType::SYNTAX_ERROR,
                    "Empty include path",
                    this->file_path,
                    err_line + 1,
                    err_col + 1,
                    line
                ) };
                co_yield ret;
                continue;
            }
            ret = Result<Parser<T>::pragma_loc_type, Error<T, std::filesystem::path>>{ mk_valid_pragmaloc(inc_path, curr_line) };
            // Delete the full line from this->lines (since the pragma is supposed to take the full line)
            this->lines_raw.erase(this->lines_raw.begin() + i);
            this->lines.erase(this->lines.begin() + i);
            i--;
            co_yield ret;
            continue;
        }
        else if (std::regex_match(line, match_res, inc_regex_angle_brackets))
        {
            pos_t quote_pos = line.find(static_cast<T>('"'));
            if (quote_pos != std::basic_string<T>::npos)
            {
                pos_t err_line = std::get<0>(line_.at(0));
                pos_t err_col = std::get<1>(line_.at(quote_pos));
                ret = Result<Parser<T>::pragma_loc_type, Error<T, std::filesystem::path>>{ Error<T, std::filesystem::path>(
                    ExcType::SYNTAX_ERROR,
                    "You cannot mix `\"\"` with `<>` in include pragmas",
                    this->file_path,
                    err_line + 1,
                    err_col + 1,
                    line
                ) };
                co_yield ret;
                continue;
            }
#if SUPDEF_DEBUG
            std::string converted = std::string(match_res[1].first, match_res[1].second);
            std::cerr << "[ DEBUG LOG ]  Found include: " << converted.c_str() << std::endl;
#endif
            std::basic_string<T> inc_path = remove_whitespaces(match_res[1].str());
            if (inc_path.empty())
            {
                pos_t err_line = std::get<0>(line_.at(0));
                pos_t err_col = std::get<1>(line_.at(line.find(static_cast<T>('"'))));
                ret = Result<Parser<T>::pragma_loc_type, Error<T, std::filesystem::path>>{ Error<T, std::filesystem::path>(
                    ExcType::SYNTAX_ERROR,
                    "Empty include path",
                    this->file_path,
                    err_line + 1,
                    err_col + 1,
                    line
                ) };
                co_yield ret;
                continue;
            }
            ret = Result<Parser<T>::pragma_loc_type, Error<T, std::filesystem::path>>{ mk_valid_pragmaloc(inc_path, curr_line) };
            // Delete the full line from this->lines (since the pragma is supposed to take the full line)
            this->lines_raw.erase(this->lines_raw.begin() + i);
            this->lines.erase(this->lines.begin() + i);
            i--;
            co_yield ret;
            continue;
        }
        else if (std::regex_match(line, match_res, inc_regex_no_quotes))
        {
            // Error if there are some `<>"` in the line
            if (line.find(static_cast<T>('"')) != std::basic_string<T>::npos ||
                line.find(static_cast<T>('<')) != std::basic_string<T>::npos ||
                line.find(static_cast<T>('>')) != std::basic_string<T>::npos)
            {
                pos_t err_line = std::get<0>(line_.at(0));
                pos_t err_col = std::get<1>(line_.at(
                    (line.find(static_cast<T>('"')) != std::basic_string<T>::npos) ? line.find(static_cast<T>('"')) :
                    (line.find(static_cast<T>('<')) != std::basic_string<T>::npos) ? line.find(static_cast<T>('<')) :
                    line.find(static_cast<T>('>'))
                ));
                ret = Result<Parser<T>::pragma_loc_type, Error<T, std::filesystem::path>>{ Error<T, std::filesystem::path>(
                    ExcType::SYNTAX_ERROR,
                    "Invalid include path",
                    this->file_path,
                    err_line + 1,
                    err_col + 1,
                    line
                ) };
                co_yield ret;
                continue;
            }
#if SUPDEF_DEBUG
            std::string converted = std::string(match_res[1].first, match_res[1].second);
            std::cerr << "[ DEBUG LOG ]  Found include: " << converted.c_str() << std::endl;
#endif
            std::basic_string<T> inc_path = remove_whitespaces(match_res[1].str());
            if (inc_path.empty())
                SupDef::Util::unreachable();
            ret = Result<Parser<T>::pragma_loc_type, Error<T, std::filesystem::path>>{ mk_valid_pragmaloc(inc_path, curr_line) };
            // Delete the full line from this->lines (since the pragma is supposed to take the full line)
            this->lines_raw.erase(this->lines_raw.begin() + i);
            this->lines.erase(this->lines.begin() + i);
            i--;
            co_yield ret;
            continue;
        }
        else if (std::regex_match(line, match_res, inc_regex_no_path))
        {
            auto include_keyword_pos = line.find(ANY_STRING(T, SUPDEF_PRAGMA_INCLUDE));
            if (include_keyword_pos == std::basic_string<T>::npos)
                SupDef::Util::unreachable();
            pos_t err_line = std::get<0>(line_.at(0));
            pos_t err_col = std::get<1>(line_.at(include_keyword_pos));
            ret = Result<Parser<T>::pragma_loc_type, Error<T, std::filesystem::path>>{ Error<T, std::filesystem::path>(
                ExcType::SYNTAX_ERROR,
                "Include pragmas must have a path",
                this->file_path,
                err_line + 1,
                err_col + 1,
                line
            ) };
            co_yield ret;
            continue;
        }
        else if (std::regex_match(line, match_res, inc_regex_with_anything))
        {
            auto include_keyword_pos = line.find(ANY_STRING(T, SUPDEF_PRAGMA_INCLUDE));
            if (include_keyword_pos == std::basic_string<T>::npos)
                SupDef::Util::unreachable();
            pos_t err_line = std::get<0>(line_.at(0));
            pos_t err_col = std::get<1>(line_.at(include_keyword_pos));
            ret = Result<Parser<T>::pragma_loc_type, Error<T, std::filesystem::path>>{ Error<T, std::filesystem::path>(
                ExcType::SYNTAX_ERROR,
                "Include pragmas must have a path",
                this->file_path,
                err_line + 1,
                err_col + 1,
                line
            ) };
            co_yield ret;
            continue;
        }
    }
    this->file_content_raw = this->reassemble_lines();
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
    ret = Result<Parser<T>::pragma_loc_type, Error<T, std::filesystem::path>>(nullptr);
    co_return ret;
}
#endif

#undef FILE_CONTENT
#undef MK_CHAR