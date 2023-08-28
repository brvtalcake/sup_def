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
#include <sup_def/common/start_header.h>

#ifndef UTIL_HPP
#define UTIL_HPP

#include <sup_def/common/config.h>

#include <filesystem>
#include <iostream>
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

    template <typename T>
    concept CharacterType = std::same_as<char, std::remove_cv_t<T>>       ||
                            std::same_as<wchar_t, std::remove_cv_t<T>>    ||
                            std::same_as<char8_t, std::remove_cv_t<T>>    ||
                            std::same_as<char16_t, std::remove_cv_t<T>>   ||
                            std::same_as<char32_t, std::remove_cv_t<T>>;
    
    template <typename T>
    concept StdStringType = requires(T)
    {
        typename T::value_type;
        typename T::size_type;
        typename T::traits_type;
        typename T::allocator_type;
    };

    template <typename T>
    concept StringType = ( std::is_pointer_v<T>                                                         &&
                         CharacterType<std::remove_cv_t<std::remove_pointer_t<std::remove_cv_t<T>>>> )  ||
                         ( StdStringType<std::remove_cv_t<T>>                                           &&
                         CharacterType<typename std::remove_cv_t<T>::value_type> );

    template <typename T>
    concept FilePath = std::same_as<std::filesystem::path, T>   || 
                       StringType<T>;

    template <typename T>
        requires CharacterType<T>
    using string_size_type = std::basic_string<T>::size_type;
};

#if defined(COMPILING_EXTERNAL)
    #include <sup_def/external/external.hpp>
#endif

namespace SupDef
{
    namespace Util
    {

        template <size_t N, typename... types>
        struct GetNthTypeImpl
        { };

        template <typename T, typename... types>
        struct GetNthTypeImpl<0, T, types...>
        {
            using type = T;
        };

        template <size_t N, typename T, typename... types>
            requires (N > 0)
        struct GetNthTypeImpl<N, T, types...>
        {
            using type = typename GetNthTypeImpl<N - 1, types...>::type;
        };

        template <size_t N, typename... types>
        using GetNthType = typename GetNthTypeImpl<N, types...>::type;

        static_assert(std::same_as<GetNthType<0, int, char, float>, int>);
        static_assert(std::same_as<GetNthType<1, int, char, float>, char>);
        static_assert(std::same_as<GetNthType<2, int, char, float>, float>);
        static_assert(std::same_as<GetNthType<0, int>, int>);
        static_assert(std::same_as<GetNthType<10, int, int, int, int, int, int, int, int, int, int, float>, float>);

        template <size_t N, typename... types>
        struct GetNthArgImpl
        { };

        template <typename T, typename... types>
        struct GetNthArgImpl<0, T, types...>
        {
            static consteval T get(T arg, types...)
            {
                return arg;
            }
        };

        template <size_t N, typename T, typename... types>
            requires (N > 0)
        struct GetNthArgImpl<N, T, types...>
        {
            static consteval auto get(T, types... args)
            {
                return GetNthArgImpl<N - 1, types...>::get(args...);
            }
        };

        template <size_t N, typename... types>
        consteval auto get_nth_arg(types... args)
        {
            return GetNthArgImpl<N, types...>::get(args...);
        }

#if defined(GetNthArg)
    #undef GetNthArg
#endif
#define GetNthArg(N, ...) get_nth_arg<N>(__VA_ARGS__)

        static_assert(GetNthArg(0, 1, 2, 3, 4, 5, 6, 7, 8, 9) == 1);
        static_assert(GetNthArg(1, 1, 2, 3, 4, 5, 6, 7, 8, 9) == 2);
        static_assert(GetNthArg(0, 1.5f, 2.5, 956L, 42ULL) == 1.5f);

        inline void exit_program(int exit_code)
        {
            // Set std::cerr to bright-white
            /* std::cerr << "\033[1;97m"; */
            std::cerr << "\033[97m";
#if defined(COMPILING_EXTERNAL)
            std::cerr << SupDef::External::get_program_name() << " exited with code " << exit_code << std::endl;
#else
            std::cerr << "Program exited with code " << exit_code << std::endl;
#endif
            // Reset std::cerr to default
            std::cerr << "\033[0m";
            std::exit(exit_code);
        }

        /**
         * @brief Get the normalized path object
         * @details This function returns the normalized path of a file, i.e. the canonical path of the file if it exists, or an empty path otherwise
         */
        inline std::filesystem::path get_normalized_path(std::filesystem::path file_path)
        {
            if (!std::filesystem::exists(file_path))
                return std::filesystem::path();
            return std::filesystem::canonical(file_path);
        }
    }
}
#endif

#include <sup_def/common/end_header.h>