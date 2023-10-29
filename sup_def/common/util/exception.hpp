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

#if !INCLUDED_FROM_SUPDEF_SOURCE
    #error "Don't include this yourself"
#endif

#if !DEFINED(Exception)

template <typename T, typename U>
    requires CharacterType<T> && FilePath<U>
class Exception : public std::exception
{
    private:
        ExcType type;
        std::string msg;
        std::optional<U> filepath;
        std::optional<string_size_type<T>> line;
        std::optional<string_size_type<T>> col;
        std::optional<std::basic_string<T>> context;
        std::optional<StackTrace> trace;

        inline std::optional<std::string> get_type_str(void) const noexcept
        {
            switch (this->type)
            {
                case ExcType::INTERNAL_ERROR:
                    return std::string("internal error");
                case ExcType::INVALID_FILE_PATH_ERROR:
                    return std::string("invalid file path error");
                case ExcType::NO_INPUT_FILE_ERROR:
                    return std::string("no input file error");
                case ExcType::SYNTAX_ERROR:
                    return std::string("syntax error");

                case ExcType::UNSPECIFIED_ERROR:
                    [[fallthrough]];
                case ExcType::NO_ERROR:
                    [[fallthrough]];
                default:
                    return std::nullopt;
            }
        }
    public:
        Exception() = default;
        Exception(ExcType type) : type(type), filepath(std::nullopt), line(std::nullopt), col(std::nullopt), context(std::nullopt)
        {
            this->msg = "An error occured";
            if (type == ExcType::INTERNAL_ERROR)
                this->trace = CURRENT_STACKTRACE;
            else
                this->trace = std::nullopt;
        }
        Exception(std::string msg) : msg(msg), filepath(std::nullopt), line(std::nullopt), col(std::nullopt), context(std::nullopt), trace(std::nullopt)
        { this->type = ExcType::UNSPECIFIED_ERROR; }
        Exception(ExcType type, std::string msg) : type(type), msg(msg), filepath(std::nullopt), line(std::nullopt), col(std::nullopt), context(std::nullopt)
        {
            if (type == ExcType::INTERNAL_ERROR)
                this->trace = CURRENT_STACKTRACE;
            else
                this->trace = std::nullopt;
        }
        Exception(ExcType type, std::string msg, string_size_type<T> line, string_size_type<T> col, std::basic_string<T> context) : type(type), msg(msg), filepath(std::nullopt), line(line), col(col), context(context)
        {
            if (type == ExcType::INTERNAL_ERROR)
                this->trace = CURRENT_STACKTRACE;
            else
                this->trace = std::nullopt;
        }
        Exception(ExcType type, std::string msg, U filepath, string_size_type<T> line, string_size_type<T> col, std::basic_string<T> context) : type(type), msg(msg), filepath(filepath), line(line), col(col), context(context)
        {
            if (type == ExcType::INTERNAL_ERROR)
                this->trace = CURRENT_STACKTRACE;
            else
                this->trace = std::nullopt;
        }

        ~Exception() = default;

        constexpr inline const char* what() const noexcept override { return format_error<T, U>(this->type, this->msg, this->get_type_str(), this->filepath, this->line, this->col, this->context).c_str(); }
        constexpr inline ExcType get_type() const noexcept { return this->type; }
        constexpr inline Exception& set_type(ExcType type) noexcept { this->type = type; return *this; }
        inline void report(std::ostream& os = std::cerr) const noexcept
        {
            if (this->type != ExcType::NO_ERROR)
                SupDef::Util::reg_error();
            else
                SupDef::Util::reg_warning();
            os << format_error<T, U>(this->type, this->msg, this->get_type_str(), this->filepath, this->line, this->col, this->context);
            if (this->trace.has_value())
                os << "Current stack trace:\n" << this->trace.value() << std::endl;
        }

        inline operator std::string() const noexcept { return format_error<T, U>(this->type, this->msg, this->get_type_str(), this->filepath, this->line, this->col, this->context); }
};

#define Exception_DEFINED 1
#endif