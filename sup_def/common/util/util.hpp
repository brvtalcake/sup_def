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
#include <sup_def/common/config.h>

#ifndef UTIL_HPP
#define UTIL_HPP

#include <cxxabi.h>
#include <filesystem>
#include <iostream>
#include <string>
#include <type_traits>
#ifdef __has_include
    #if __has_include(<experimental/simd>)
        #include <experimental/simd>
    #endif
#endif

namespace SupDef 
{
};

#if defined(COMPILING_EXTERNAL)
    SAVE_MACRO(SUPDEF_WANT_ONLY_DECLS)
    #undef SUPDEF_WANT_ONLY_DECLS
    #define SUPDEF_WANT_ONLY_DECLS 1
    #include <sup_def/external/external.hpp>
    RESTORE_MACRO(SUPDEF_WANT_ONLY_DECLS)
#endif

namespace SupDef
{
    namespace Util
    {
        std::string demangle(std::string s);

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

#if 0
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
        { using std::vector<T>::vector; };
#endif

#elif __cpp_lib_experimental_parallel_simd >= 201803L
        
        namespace stdx = std::experimental;

        // TODO: Test for multiple dimensions and improve
        template <typename T, size_t N, class ABI = stdx::simd_abi::deduce_t<T, N>>
            requires std::is_arithmetic_v<T> && stdx::is_abi_tag_v<ABI>
        class Array : public stdx::simd<T, ABI>
        {
            private:
                typedef typename stdx::simd<T, ABI> simd_type;
                // Is this 1D, 2D, 3D, ... ?
                size_t dim = 1;
                // Dimensions of the array in x, y, z, ...
                std::vector<size_t> dims = { N };
            public:

                /* Member functions */
                using simd_type::copy_from;
                using simd_type::copy_to;
                using simd_type::operator[];
                using simd_type::operator++;
                using simd_type::operator--;
                using simd_type::operator!;
                using simd_type::operator~;
                using simd_type::operator+;
                using simd_type::operator-;
                using simd_type::size;

                /* Member types */
                using typename simd_type::value_type;
                using typename simd_type::reference;
                using typename simd_type::abi_type;
                using typename simd_type::simd_type;
                using typename simd_type::mask_type;

                Array(std::initializer_list<T> init)
                {
                    if (init.size() > N)
                        throw Exception<char, std::filesystem::path>(SupDef::ExcType::INTERNAL_ERROR, "Array::Array(std::initializer_list<T>): initializer list too long");
                    for (size_t i = 0; i < init.size(); ++i)
                        this->operator[](i) = init.begin()[i];
                }

                template <typename... Args>
                    requires (std::same_as<T, Args> && ...)
                Array(Args... args)
                {
                    if (sizeof...(args) > N)
                        throw Exception<char, std::filesystem::path>(SupDef::ExcType::INTERNAL_ERROR, "Array::Array(T...): initializer list too long");
                    size_t i = 0;
                    for (auto& arg : { args... })
                        this->operator[](i++) = arg;
                }


                void set_dim(size_t d) noexcept
                {
                    this->dim = d;
                }

                template <typename... Args>
                    requires (std::same_as<size_t, Args> && ...)
                void set_dims(Args... args)
                {
                    if (sizeof...(args) != this->dim)
                        throw Exception<char, std::filesystem::path>(SupDef::ExcType::INTERNAL_ERROR, "Array::set_dims(Args...): number of dimensions must be equal to this->dim");
                    size_t sum = 0;
                    for (auto& arg : { args... })
                        sum += arg;
                    if (sum != N)
                        throw Exception<char, std::filesystem::path>(SupDef::ExcType::INTERNAL_ERROR, "Array::set_dims(Args...): sum of dimensions must be equal to N");
                    this->dims.clear();
                    this->dims.reserve(sizeof...(args));
                    (this->dims.push_back(args), ...);
                }

#if 0 // Not supported by GCC 13.2
                template <typename... Args>
                    requires (sizeof...(Args) > 1) && (std::same_as<size_t, Args> && ...)
                decltype(auto) operator[](this auto& self /* C++23 */, Args... indexes)
                {
                    if (sizeof...(indexes) != self.dim)
                        throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "Array::operator[](Args...): number of indexes must be equal to this->dim");
                    size_t index = 0;
                    size_t i = 0;
                    for (auto& arg : { indexes... })
                    {
                        if (arg >= self.dims[i])
                            throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "Array::operator[](Args...): index out of bounds");
                        index += arg * self.dims[i];
                        ++i;
                    }
                    return self[index];
                }
#else
                template <typename... Args>
                    requires (sizeof...(Args) > 1) && (std::same_as<size_t, Args> && ...)
                reference operator[](Args... indexes)
                {
                    if (sizeof...(indexes) != this->dim)
                        throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "Array::operator[](Args...): number of indexes must be equal to this->dim");
                    size_t index = 0;
                    size_t i = 0;
                    for (auto& arg : { indexes... })
                    {
                        if (arg >= this->dims[i])
                            throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "Array::operator[](Args...): index out of bounds");
                        index += arg * this->dims[i];
                        ++i;
                    }
                    return this->operator[](index);
                }

                template <typename... Args>
                    requires (sizeof...(Args) > 1) && (std::same_as<size_t, Args> && ...)
                const value_type operator[](Args... indexes) const
                {
                    if (sizeof...(indexes) != this->dim)
                        throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "Array::operator[](Args...): number of indexes must be equal to this->dim");
                    size_t index = 0;
                    size_t i = 0;
                    for (auto& arg : { indexes... })
                    {
                        if (arg >= this->dims[i])
                            throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "Array::operator[](Args...): index out of bounds");
                        index += arg * this->dims[i];
                        ++i;
                    }
                    return this->operator[](index);
                }
#endif
                template <class A>
                    requires std::same_as<Array, std::remove_cv_t<A>>
                class Iterator
                {
                    private:
                        A& arr;
                        size_t index;

                    public:
                        using iterator_category = std::random_access_iterator_tag;
                        using value_type = typename A::value_type;
                        using difference_type = ptrdiff_t;
                        using pointer = value_type*;
                        using reference = typename A::reference;

                        class SmartWrapper
                        {
                            private:
                                A& arr;
                                size_t index = 0;

                            public:
                                SmartWrapper(A& arr, size_t index) : arr(arr), index(index) { }

                                operator value_type() const
                                {
                                    return this->arr[this->index];
                                }

                                SmartWrapper& operator=(const value_type& value)
                                {
                                    this->arr[this->index] = value;
                                    return *this;
                                }

                                SmartWrapper& operator=(value_type&& value)
                                {
                                    this->arr[this->index] = std::move(value);
                                    return *this;
                                }

                                SmartWrapper& operator=(const SmartWrapper& other)
                                {
                                    this->arr[this->index] = other.arr[other.index];
                                    return *this;
                                }
                        };

                        Iterator(A& arr, size_t index) : arr(arr), index(index) { }

                        Iterator& operator++()
                        {
                            ++this->index;
                            return *this;
                        }

                        Iterator operator++(int)
                        {
                            Iterator tmp = *this;
                            ++(*this);
                            return tmp;
                        }

                        bool operator==(const Iterator& other) const
                        {
                            return this->index == other.index;
                        }

                        bool operator!=(const Iterator& other) const
                        {
                            return !(*this == other);
                        }

                        template <typename Ret = const value_type>
                            requires std::is_const_v<A>
                        Ret operator*()
                        {
                            return this->arr[this->index];
                        }

                        template <typename Ret = SmartWrapper>
                            requires std::negation_v<std::is_const<A>>
                        Ret operator*()
                        {
                            return { this->arr, this->index };
                        }
                };

                template <class A>
                using iterator = Iterator<A>;

                iterator<Array> begin()
                {
                    return iterator<Array>(*this, 0);
                }

                iterator<const Array> begin() const
                {
                    return iterator<const Array>(*this, 0);
                }

                iterator<Array> end()
                {
                    return iterator<Array>(*this, this->size());
                }

                iterator<const Array> end() const
                {
                    return iterator<const Array>(*this, this->size());
                }
        };
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
        inline std::basic_string<C1> convert_string(const std::basic_string<C2>& str)
        {
            std::basic_string<C1> ret;
            for (auto& c : str)
                ret += static_cast<C1>(c);
            return ret;
        }

        template <typename C1>
            requires CharacterType<C1>
        inline std::basic_string<C1> convert_string(const std::filesystem::path& path)
        {
            return convert_string<C1>(path.string());
        }

        template <typename T>
            requires CharacterType<T>
        auto remove_whitespaces(const std::basic_string<T>& s) -> std::basic_string<T>
        {
            std::basic_string<T> res;
            res.reserve(s.size());
            for (auto& c : s)
            {
                if (c != static_cast<T>(' ') && c != static_cast<T>('\t'))
                    res += c;
            }
            return res;
        }

        // TODO: Finish this
        class DeferImpl
        {
            private:
                std::function<void()> func;

            public:
                DeferImpl(std::function<void()> func) : func(func) { }
                ~DeferImpl() { (this->func)(); }
        };

    }
    using Util::remove_whitespaces;
}
#endif // UTIL_HPP

#include <sup_def/common/end_header.h>