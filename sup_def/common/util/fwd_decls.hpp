#include <exception>

#include <bits/c++config.h>

#if defined(__INTELLISENSE__)
    #define SUPDEF_USE_BOOST_STACKTRACE 1
#endif

#if SUPDEF_USE_CXX23_STACKTRACE
    #if !_GLIBCXX_HAVE_STACKTRACE
        #error "Your compiler does not support C++23 stacktrace"
    #endif
    #include <stacktrace>
    #ifdef SUPDEF_STACKTRACE_TYPE
        #undef SUPDEF_STACKTRACE_TYPE
    #endif
    #define SUPDEF_STACKTRACE_TYPE std::stacktrace
    #ifdef CURRENT_STACKTRACE
        #undef CURRENT_STACKTRACE
    #endif
    // Extra parameter to skip the SupDef::Exception constructor
    #define CURRENT_STACKTRACE std::stacktrace::current(1)
#elif SUPDEF_USE_BOOST_STACKTRACE
    #include <boost/stacktrace.hpp>
    #ifdef SUPDEF_STACKTRACE_TYPE
        #undef SUPDEF_STACKTRACE_TYPE
    #endif
    #define SUPDEF_STACKTRACE_TYPE boost::stacktrace::stacktrace
    #ifdef CURRENT_STACKTRACE
        #undef CURRENT_STACKTRACE
    #endif
    // Extra parameters to skip the SupDef::Exception constructor
    #define CURRENT_STACKTRACE boost::stacktrace::stacktrace(1 , static_cast<std::size_t>(-1))
#else
    #error "No stacktrace library defined"
#endif

#include <filesystem>
#include <string>
#include <type_traits>

namespace SupDef
{
    namespace Util
    {
        SD_COMMON_API
        int reg_error(void);

        SD_COMMON_API
        int get_errcount(void);

        SD_COMMON_API
        int reg_warning(void);

        SD_COMMON_API
        int get_warncount(void);
    };
};

namespace SupDef
{
    typedef SUPDEF_STACKTRACE_TYPE StackTrace;

#define INCLUDED_FROM_SUPDEF_SOURCE 1
#include <sup_def/common/util/concepts.hpp>
#undef INCLUDED_FROM_SUPDEF_SOURCE

    template <typename T>
        requires CharacterType<T>
    using string_size_type = typename std::basic_string<T>::size_type;

#if !DEFINED(ExcType)
    enum class ExcType : uint8_t
    {
        NO_ERROR = 0,
        INTERNAL_ERROR = 1,
        INVALID_PATH_ERROR = 2,
        NO_INPUT_FILE_ERROR = 3,
        SYNTAX_ERROR = 4,
        UNSPECIFIED_ERROR = 255
    };
#define ExcType_DEFINED 1
#endif

    /**
     * @fn std::string format_error(const std::string& error_msg, std::optional<std::string> error_type, std::optional<string_size_type<char>> line, std::optional<string_size_type<char>> col, std::optional<std::string> context)
     * @brief Format an error message
     * 
     * @param type The type of the error
     * @param error_msg The main error message
     * @param error_type Additional error type information
     * @param line Line number where the error occured, if any
     * @param col Column number where the error occured, if any
     * @param context Line context where the error occured, if any
     * @tparam T The character type of the parsed file
     * @return The formatted error message
     */
    template <typename T, typename U>
        requires CharacterType<T> && FilePath<U>
    static 
    std::string 
    format_error(
        ExcType type,
        const std::string& error_msg,
        std::optional<std::string> error_type,
        std::optional<U> filepath,
        std::optional<string_size_type<T>> line,
        std::optional<string_size_type<T>> col,
        std::optional<std::basic_string<T>> context
    ) noexcept;

#define INCLUDED_FROM_SUPDEF_SOURCE 1
#include <sup_def/common/util/exception.hpp>
#undef INCLUDED_FROM_SUPDEF_SOURCE

    namespace Util
    {
        
    };
};