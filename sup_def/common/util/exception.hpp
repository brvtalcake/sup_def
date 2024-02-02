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

class InternalException;
class ContractViolation;

template <typename T, typename U>
    requires CharacterType<T> && FilePath<U>
class Exception : public std::exception
{
    friend class ::SupDef::InternalException;
    friend class ::SupDef::ContractViolation;

    private:
        ExcType type;
        std::string msg;
        std::optional<U> filepath;
        std::optional<string_size_type<T>> line;
        std::optional<string_size_type<T>> col;
        std::optional<std::basic_string<T>> context;
        std::optional<StackTrace> trace;
        mutable char* what_msg = nullptr;

        inline std::optional<std::string> get_type_str(void) const noexcept
        {
            switch (this->type)
            {
                case ExcType::INTERNAL_ERROR:
                    return std::string("internal error");
                case ExcType::INVALID_PATH_ERROR:
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
        inline void init_msg(void) noexcept
        {
            std::stringstream ss;

            int err_or_warn_count = 0;
            if (this->type != ExcType::NO_ERROR)
                err_or_warn_count = SupDef::Util::reg_error();
            else
                err_or_warn_count = SupDef::Util::reg_warning();

            ss << ::SupDef::format_error<T, U>(this->type, this->msg, this->get_type_str(), this->filepath, this->line, this->col, this->context, err_or_warn_count);

            if (this->trace.has_value())
                ss << "Current stack trace:\n" << this->trace.value() << std::endl;

            if (this->what_msg != nullptr)
                delete[] this->what_msg;
            try
            {
                this->what_msg = new char[ss.str().size() + 1];
            }
            catch (const std::bad_alloc& e)
            {
                std::cerr << "Failed to allocate memory for exception message: " << e.what() << std::endl;
                SUPDEF_EXIT();
            }
            catch (const std::exception& e)
            {
                std::cerr << "Failed to allocate memory for exception message: " << e.what() << std::endl;
                SUPDEF_EXIT();
            }
            catch (...)
            {
                std::cerr << "Failed to allocate memory for exception message: unknown exception" << std::endl;
                SUPDEF_EXIT();
            }

#if 0
            std::strcpy(this->what_msg, ss.str().c_str());
#else
            std::copy(std::begin(ss.str()), std::end(ss.str()), this->what_msg);
#endif
            this->what_msg[ss.str().size()] = '\0';
        }
    public:
        Exception() = default;

        Exception(ExcType type, bool gen_trace = true, bool gen_msg = true) : type(type), filepath(std::nullopt), line(std::nullopt), col(std::nullopt), context(std::nullopt)
        {
            this->msg = "An error occured";
            if (type == ExcType::INTERNAL_ERROR && gen_trace)
                this->trace = CURRENT_STACKTRACE(1);
            else
                this->trace = std::nullopt;
            if (gen_msg)
                this->init_msg();
        }

        Exception(std::string msg)
            : msg(msg), filepath(std::nullopt), line(std::nullopt), col(std::nullopt), context(std::nullopt), trace(std::nullopt)
        {
            this->type = ExcType::UNSPECIFIED_ERROR;
            this->init_msg();
        }

        Exception(ExcType type, std::string msg, bool gen_trace = true, bool gen_msg = true) : type(type), msg(msg), filepath(std::nullopt), line(std::nullopt), col(std::nullopt), context(std::nullopt)
        {
            if (type == ExcType::INTERNAL_ERROR && gen_trace)
                this->trace = CURRENT_STACKTRACE(1);
            else
                this->trace = std::nullopt;
            if (gen_msg)
                this->init_msg();
        }

        Exception(ExcType type, std::string msg, string_size_type<T> line, string_size_type<T> col, std::basic_string<T> context, bool gen_trace = true, bool gen_msg = true) : type(type), msg(msg), filepath(std::nullopt), line(line), col(col), context(context)
        {
            if (type == ExcType::INTERNAL_ERROR && gen_trace)
                this->trace = CURRENT_STACKTRACE(1);
            else
                this->trace = std::nullopt;
            if (gen_msg)
                this->init_msg();
        }

        Exception(ExcType type, std::string msg, U filepath, string_size_type<T> line, string_size_type<T> col, std::basic_string<T> context, bool gen_trace = true, bool gen_msg = true) : type(type), msg(msg), filepath(filepath), line(line), col(col), context(context)
        {
            if (type == ExcType::INTERNAL_ERROR && gen_trace)
                this->trace = CURRENT_STACKTRACE(1);
            else
                this->trace = std::nullopt;
            if (gen_msg)
                this->init_msg();
        }

        ~Exception() noexcept override
        {
            if (this->what_msg != nullptr)
                delete[] this->what_msg;
            this->what_msg = nullptr;
        }

        constexpr inline const char* what() const noexcept override 
        {
            return this->what_msg;
        }
        constexpr inline ExcType get_type() const noexcept { return this->type; }
        constexpr inline Exception& set_type(ExcType type) noexcept { this->type = type; return *this; }
        inline void report(std::ostream& os = std::cerr) const noexcept
        {
            os << this->what() << std::endl;
        }

        inline operator std::string() const noexcept 
        {
            return std::string(this->what());
        }
};

#define Exception_DEFINED 1
#endif

#if !DEFINED(InternalException)

class InternalException : public Exception<char, std::string>
{
    public:
        InternalException(bool gen_trace = true, bool gen_msg = true)
            : Exception<char, std::string>(ExcType::INTERNAL_ERROR, false, false)
        {
            if (gen_trace)
                this->trace = CURRENT_STACKTRACE(1);
            if (gen_msg)
                this->init_msg();
        }
        
        InternalException(std::string msg, bool gen_trace = true, bool gen_msg = true)
            : Exception<char, std::string>(ExcType::INTERNAL_ERROR, msg, false, false)
        {
            if (gen_trace)
                this->trace = CURRENT_STACKTRACE(1);
            if (gen_msg)
                this->init_msg();
        }
};

#define InternalException_DEFINED 1
#endif
