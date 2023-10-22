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
    concept CStrType = std::is_pointer_v<T>                                                         &&
                       CharacterType<std::remove_cv_t<std::remove_pointer_t<std::remove_cv_t<T>>>>;
    
    template <typename T>
    concept StdStringType = requires(T)
    {
        typename std::remove_cvref_t<T>::value_type;
        typename std::remove_cvref_t<T>::size_type;
        typename std::remove_cvref_t<T>::traits_type;
        typename std::remove_cvref_t<T>::allocator_type;
    };

    template <typename T>
    concept StringType = CStrType<T>                                                ||
                         ( StdStringType<std::remove_cv_t<T>>                       &&
                         CharacterType<typename std::remove_cv_t<T>::value_type> );

    template <typename T>
    concept FilePath = std::same_as<std::remove_cvref_t<std::filesystem::path>, T>   || 
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
            typedef T type;
        };

        template <size_t N, typename T, typename... types>
            requires (N > 0)
        struct GetNthTypeImpl<N, T, types...>
        {
            typedef typename GetNthTypeImpl<N - 1, types...>::type type;
        };

        template <size_t N, typename... types>
        using GetNthType = typename GetNthTypeImpl<N, types...>::type;

        static_assert(std::same_as<GetNthType<0, int, char, float>, int>);
        static_assert(std::same_as<GetNthType<1, int, char, float>, char>);
        static_assert(std::same_as<GetNthType<2, int, char, float>, float>);
        static_assert(std::same_as<GetNthType<0, int>, int>);
        static_assert(std::same_as<GetNthType<10, int, int, int, int, int, int, int, int, int, int, float>, float>);

#if 0
        template <size_t N, typename... types>
        struct GetNthTypeFromLastImpl
        { };

        template <typename T>
        struct GetNthTypeFromLastImpl<0, T>
        {
            typedef T type;
        };

        template <size_t N, typename... types, typename T, typename = void>
            requires (N > 0)
        struct GetNthTypeFromLastImpl<N, types..., T>
        {
            typedef typename GetNthTypeFromLastImpl<N - 1, types...>::type type;
        };

        template <size_t N, typename... types>
        using GetNthTypeFromLast = typename GetNthTypeFromLastImpl<N, types...>::type;

        static_assert(std::same_as<GetNthTypeFromLast<0, int, char, float>, float>);
        static_assert(std::same_as<GetNthTypeFromLast<1, int, char, float>, char>);
        static_assert(std::same_as<GetNthTypeFromLast<2, int, char, float>, int>);
        static_assert(std::same_as<GetNthTypeFromLast<0, int>, int>);
        static_assert(std::same_as<GetNthTypeFromLast<10, int, int, int, int, int, int, int, int, int, int, float>, int>);

#endif

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
#define GetNthArg(N, ...) SupDef::Util::get_nth_arg<N>(__VA_ARGS__)

        static_assert(GetNthArg(0, 1, 2, 3, 4, 5, 6, 7, 8, 9) == 1);
        static_assert(GetNthArg(1, 1, 2, 3, 4, 5, 6, 7, 8, 9) == 2);
        static_assert(GetNthArg(0, 1.5f, 2.5, 956L, 42ULL) == 1.5f);

        template <bool B, typename C, C T, C F>
        struct Choose_num : std::conditional_t<B, std::integral_constant<C, T>, std::integral_constant<C, F>>
        { };

        template <typename T, template <typename...> typename Template>
        struct SpecializationOf : std::false_type { };

        template <template <typename...> typename Template, typename... Args>
        struct SpecializationOf<Template<Args...>, Template> : std::true_type { };

#if defined(SPECIALIZATION_OF)
    #undef SPECIALIZATION_OF
#endif
#define SPECIALIZATION_OF(T, Template) SupDef::Util::SpecializationOf<T, Template>::value

        static_assert(SPECIALIZATION_OF(std::vector<int>, std::vector));
        static_assert(SPECIALIZATION_OF(std::list<int>, std::list));
        static_assert(!SPECIALIZATION_OF(std::vector<int>, std::list));
        static_assert(!SPECIALIZATION_OF(std::list<int>, std::vector));

        template <typename... Types>
        consteval size_t variadic_count(Types...)
        { return sizeof...(Types); }

        static_assert(variadic_count(1, 2, 3, 4, 5, 6, 7, 8, 9) == 9);
        static_assert(variadic_count(1, 2, 3, 4, 5, 6, 7, 8, 9, 10) == 10);
        static_assert(variadic_count(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11) == 11);
        static_assert(variadic_count() == 0);

#if defined(VARIADIC_COUNT)
    #undef VARIADIC_COUNT
#endif
#define VARIADIC_COUNT(...) SupDef::Util::variadic_count(__VA_ARGS__)

        template <typename... Args>
        [[noreturn]] inline void unreachable(Args&&... args)
        {
            std::cerr << "\033[1;31m" << "FATAL ERROR:" << "\033[0m" << "\033[31m" << " Unreachable code reached!" << "\n";
            if (sizeof...(args) > 0)
            {
                std::cerr << "\033[31m" << "(";
                std::tuple<Args...> tuple = std::make_tuple(std::forward<Args>(args)...);
                std::apply([](auto&&... args) { ((std::cerr << args), ...); }, tuple);
                std::cerr << ")\033[0m" << "\n";
            }
#if defined(__cpp_lib_unreachable) && __cpp_lib_unreachable >= 202202L
            std::unreachable();
#elif (SUPDEF_COMPILER == 1 || SUPDEF_COMPILER == 2)
            __builtin_unreachable();
#elif (SUPDEF_COMPILER == 3)
            __assume(false); // https://learn.microsoft.com/ka-ge/cpp/intrinsics/assume?view=msvc-150
#endif
            assert(false); // Fall-back
        }

#if defined(UNREACHABLE_EMPTY_TEST)
    #undef UNREACHABLE_EMPTY_TEST
#endif
/*UNREACHABLE_(ID(MAP_LIST(decltype, __VA_ARGS__)))(__VA_ARGS__)*/
#define UNREACHABLE_EMPTY_TEST(...) ID(PP_IF(ISEMPTY(__VA_ARGS__))(SupDef::Util::unreachable<>())(ID(UNREACHABLE_(ID(MAP_LIST(decltype, __VA_ARGS__)))(__VA_ARGS__))))

#if defined(UNREACHABLE)
    #undef UNREACHABLE
#endif
#define UNREACHABLE(...) UNREACHABLE_EMPTY_TEST(__VA_ARGS__)

#if defined(MAKE_DECAY)
    #undef MAKE_DECAY
#endif
#define MAKE_DECAY(...) MAKE_DECAY_(ID(__VA_ARGS__))

#if defined(MAKE_DECAY_)
    #undef MAKE_DECAY_
#endif
#define MAKE_DECAY_(...) std::decay_t<__VA_ARGS__>

#if defined(UNREACHABLE_)
    #undef UNREACHABLE_
#endif
#define UNREACHABLE_(...) SupDef::Util::unreachable<MAP_LIST(MAKE_DECAY, ID(__VA_ARGS__))>

        template <typename T, size_t N, typename Allocator = std::allocator<T>>
            requires std::is_arithmetic_v<T>
#if SUPDEF_COMPILER == 1
        class Vector
        {
            private:
                typedef __attribute__((__vector_size__(N * sizeof(T)))) T vector_type;
                
                vector_type data;
                Allocator allocator;
                const size_t max_size = N;

            public:
                Vector() = default;
                Vector(const Vector&) = default;
                Vector(Vector&&) = default;
                Vector& operator=(const Vector&) = default;
                Vector& operator=(Vector&&) = default;
                ~Vector() = default;

                Vector(std::initializer_list<T> init)
                {
                    if (init.size() > N)
                        throw std::length_error("Vector::Vector(std::initializer_list<T>): initializer list too long");
                    std::copy(init.begin(), init.end(), data);
                }

                Vector& operator=(std::initializer_list<T> init)
                {
                    if (init.size() > N)
                        throw std::length_error("Vector::operator=(std::initializer_list<T>): initializer list too long");
                    std::copy(init.begin(), init.end(), data);
                    return *this;
                }

                T& operator[](size_t index)
                {
                    return data[index];
                }

                const T& operator[](size_t index) const
                {
                    return data[index];
                }
        };
#else
        class Vector : public std::vector<T>
        { };
#endif

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

        template <typename C1, typename C2>
            requires CharacterType<C1> && CharacterType<C2>
        inline std::basic_string<C1> convert_string(std::basic_string<C2> str)
        {
            std::basic_string<C1> ret;
            for (auto& c : str)
                ret += static_cast<C1>(c);
            return ret;
        }

        template <typename C1>
            requires CharacterType<C1>
        inline std::basic_string<C1> convert_string(std::filesystem::path path)
        {
            return convert_string<C1>(path.string());
        }
    }
}
#endif // UTIL_HPP

#include <sup_def/common/end_header.h>