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
#include <concepts>
#include <compare>
#if SUPDEF_COMPILER == 3
    #include <intrin.h>
#endif
#if __STDC_VERSION__ >= 201710L
    #ifdef __has_include
        #if __has_include(<stdckdint.h>)
            #include <stdckdint.h>
        #endif
    #endif
#else
#endif

#ifdef __has_include
    
    #if __has_include(<experimental/simd>)
        #include <experimental/simd>
    #endif
    
    #if __has_include(<experimental/scope>)
        #include <experimental/scope>
        #ifdef SUPDEF_HAS_SCOPE_EXIT
            #undef SUPDEF_HAS_SCOPE_EXIT
        #endif
        #if __cpp_lib_experimental_scope && __cpp_lib_experimental_scope >= 201902L
            #define SUPDEF_HAS_SCOPE_EXIT 1
        #endif
    #endif

#endif

namespace SupDef 
{
    template<typename Tp, typename... Types>
    concept IsOneOf = std::disjunction_v<std::is_same<Tp, Types>...>;
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

        static_assert(IsOneOf<int, int, char, float>);
        static_assert(IsOneOf<int, char, float, int>);
        static_assert(IsOneOf<int, int>);
        static_assert(!IsOneOf<int, char, float>);
        static_assert(!IsOneOf<int, char, float, double>);
        static_assert(IsOneOf<int, char, float, double, int>);

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

#if !DEFINED(SpecializationOf)
        template <typename T, template <typename...> typename Template>
        struct SpecializationOf : std::false_type { };

        template <template <typename...> typename Template, typename... Args>
        struct SpecializationOf<Template<Args...>, Template> : std::true_type { };

#if defined(SPECIALIZATION_OF)
    #undef SPECIALIZATION_OF
#endif
#define SPECIALIZATION_OF(T, Template) SupDef::Util::SpecializationOf<T, Template>::value
#define SpecializationOf_DEFINED 1
#endif

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
#define VARIADIC_COUNT(...) ::SupDef::Util::variadic_count(__VA_ARGS__)

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
            std::terminate(); // TODO: Replace with std::abort() or something else?
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
        
        namespace stdx = ::std::experimental;

        // TODO: Test for multiple dimensions and improve
        template <typename T, size_t N, size_t D = 1, class ABI = stdx::simd_abi::deduce_t<T, N>>
            requires std::is_arithmetic_v<T> && stdx::is_abi_tag_v<ABI>
        class Array : public stdx::simd<T, ABI>
        {
            private:
                typedef typename stdx::simd<T, ABI> simd_type;
                // Is this 1D, 2D, 3D, ... ?
                size_t dim = D;
                // Dimensions of the array in x, y, z, ...
                std::vector<size_t> dims = std::vector<size_t>(D, N);
                size_t elem_count = D * N;
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

                template <typename Arg>
                    requires std::convertible_to<Arg, size_t> && std::is_unsigned_v<Arg>
                void set_dim(Arg d)
                {
                    this->dim = size_t(d);
                }

                template <typename... Args>
                    requires (std::convertible_to<Args, size_t> && ...) && (std::is_unsigned_v<Args> && ...)
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

        // To pass as a deleter to a std::shared_ptr<void> that stores a pointer to a type T
        template <typename T>
        static void shared_deleter(void* ptr)
        {
            delete static_cast<T*>(ptr);
        }

        // To pass as a deleter to a std::unique_ptr<void> that stores a pointer to a type T
        template <typename T>
        static void unique_deleter(void* ptr)
        {
            delete static_cast<T*>(ptr);
        }

        template <bool C>
        static constexpr size_t bool_to_size_t = C ? 1 : 0;
        
        template <typename Type, template <typename> typename Template>
        concept HasValueField = requires { Template<Type>::value; };

        template <typename Type, template <typename> typename Template>
        concept ConvertsToBool = std::convertible_to<Template<Type>, bool>;

        template <typename... Types>
        struct TypeContainer
        {
            static constexpr size_t size = sizeof...(Types);
            
            using types = std::tuple<Types...>;
            template <size_t N>
                requires (N < sizeof...(Types))
            using get = GetNthType<N, Types...>;
            template <typename T>
            static constexpr size_t count = (bool_to_size_t<std::same_as<T, Types>> + ...);
            template <typename T>
            static constexpr bool contains = (std::same_as<T, Types> || ...);
            // For each type in Types, apply predicate template T to it
            template <template <typename> typename T, size_t N>
            static constexpr bool apply_predicate_to = false;

#if GCC_VERSION_AT_LEAST(14, 0, 0)
            template <template <typename> typename T, size_t N>
                requires HasValueField<GetNthType<N, Types...>, T> && (!ConvertsToBool<GetNthType<N, Types...>, T>)
            static constexpr bool apply_predicate_to<T, N> = T<GetNthType<N, Types...>>::value;

            template <template <typename> typename T, size_t N>
                requires ConvertsToBool<GetNthType<N, Types...>, T>
            static constexpr bool apply_predicate_to<T, N> = static_cast<bool>(T<GetNthType<N, Types...>>());
#endif

#if GCC_VERSION_AT_LEAST(14, 0, 0)
            template <template <typename> typename T, size_t N = 0>
            static constexpr bool apply_predicate_conjunction_impl = (apply_predicate_to<T, N> && apply_predicate_conjunction_impl<T, N + 1>);

            template <template <typename> typename T>
            static constexpr bool apply_predicate_conjunction_impl<T, size> = true;

            template <template <typename> typename T, size_t N = 0>
            static constexpr bool apply_predicate_disjunction_impl = (apply_predicate_to<T, N> || apply_predicate_disjunction_impl<T, N + 1>);

            template <template <typename> typename T>
            static constexpr bool apply_predicate_disjunction_impl<T, size> = false;

            template <template <typename> typename T>
            static constexpr bool apply_predicate_conjunction = apply_predicate_conjunction_impl<T, 0>;

            template <template <typename> typename T>
            static constexpr bool apply_predicate_disjunction = apply_predicate_disjunction_impl<T, 0>;
#else
            template <template <typename> typename T, size_t N = 0>
            static consteval bool apply_predicate_conjunction_impl()
            {
                if constexpr (N == size)
                    return true;
                else
                    return apply_predicate_to<T, N> && apply_predicate_conjunction_impl<T, N + 1>();
            }

            template <template <typename> typename T, size_t N = 0>
            static consteval bool apply_predicate_disjunction_impl()
            {
                if constexpr (N == size)
                    return false;
                else
                    return apply_predicate_to<T, N> || apply_predicate_disjunction_impl<T, N + 1>();
            }

            template <template <typename> typename T>
            static constexpr bool apply_predicate_conjunction = apply_predicate_conjunction_impl<T, 0>();

            template <template <typename> typename T>
            static constexpr bool apply_predicate_disjunction = apply_predicate_disjunction_impl<T, 0>();
#endif
        };

#if !GCC_VERSION_AT_LEAST(14, 0, 0)
        // Specialize apply_predicate_to etc. out of the class context
        template <typename... Types>
        template <template <typename> typename T, size_t N>
            requires HasValueField<GetNthType<N, Types...>, T> && (!ConvertsToBool<GetNthType<N, Types...>, T>)
        constexpr bool TypeContainer<Types...>::apply_predicate_to<T, N> = T<GetNthType<N, Types...>>::value;

        template <typename... Types>
        template <template <typename> typename T, size_t N>
            requires ConvertsToBool<GetNthType<N, Types...>, T>
        constexpr bool TypeContainer<Types...>::apply_predicate_to<T, N> = static_cast<bool>(T<GetNthType<N, Types...>>());
#endif



        template <>
        struct TypeContainer<>
        {
            static constexpr size_t size = 0;
            template <size_t N>
            using get = void;
            template <typename T>
            static constexpr size_t count = 0;
            template <typename T>
            static constexpr bool contains = false;
            template <template <typename> typename T, size_t N = 0>
            static constexpr bool apply_predicate_conjunction_impl = false;
            template <template <typename> typename T, size_t N = 0>
            static constexpr bool apply_predicate_disjunction_impl = false;
            template <template <typename> typename T, size_t N = 0>
            static constexpr bool apply_predicate_to = false;
            template <template <typename> typename T>
            static constexpr bool apply_predicate_conjunction = false;
            template <template <typename> typename T>
            static constexpr bool apply_predicate_disjunction = false;
        };

        // Implement GetTemplateArgs, a TypeContainer alias storing the types which specialize an unknown template `Template`
        template <typename...>
        struct GetTemplateArgs
        { };

        template <template <typename...> typename Template, typename... Args>
        struct GetTemplateArgs<Template<Args...>>
        {
            using type = TypeContainer<Args...>;
            static constexpr size_t size = sizeof...(Args);

            static_assert(size == type::size);
        };

        static_assert(std::same_as<GetTemplateArgs<std::vector<int>>::type, TypeContainer<int, std::allocator<int>>>);
        static_assert(std::same_as<GetTemplateArgs<std::list<int>>::type, TypeContainer<int, std::allocator<int>>>);
        static_assert(std::same_as<GetTemplateArgs<std::vector<int, std::allocator<int>>>::type, TypeContainer<int, std::allocator<int>>>);
        static_assert(std::same_as<GetTemplateArgs<std::list<int, std::allocator<int>>>::type, TypeContainer<int, std::allocator<int>>>);

        template <typename...>
        struct IsSpecializedTemplate : std::false_type
        { };
        
        /* 
        template <template <typename...> class Template>
        struct IsSpecializedTemplate : std::false_type
        { };
        */
        
        template <template <typename...> class Template, typename... Args>
        struct IsSpecializedTemplate<Template<Args...>> : std::true_type
        { };

        template <template <auto...> class Template, auto... Args>
        struct IsSpecializedTemplate<Template<Args...>> : std::true_type
        { };

        /*
        template <typename...>
        struct IsUnspecializedTemplate : std::false_type
        { };

        template <template <typename...> class Template>
        struct IsUnspecializedTemplate : std::true_type
        { };

        template <typename... Args>
        struct IsTemplate : std::conditional_t<std::disjunction_v<IsSpecializedTemplate<Args...>, IsUnspecializedTemplate<Args...>>, std::true_type, std::false_type>
        { };
        */

        static_assert(IsSpecializedTemplate<std::vector<int>>::value);
        static_assert(IsSpecializedTemplate<std::list<int>>::value);
        static_assert(IsSpecializedTemplate<std::vector<int, std::allocator<int>>>::value);
        static_assert(IsSpecializedTemplate<std::list<int, std::allocator<int>>>::value);
        static_assert(!IsSpecializedTemplate<int>::value);
        /* static_assert(!IsSpecializedTemplate<std::vector>::value);
        static_assert(!IsSpecializedTemplate<std::list>::value); */

        /* static_assert(IsUnspecializedTemplate<std::vector>::value);
        static_assert(IsUnspecializedTemplate<std::list>::value);
        static_assert(!IsUnspecializedTemplate<std::vector<int>>::value);
        static_assert(!IsUnspecializedTemplate<std::list<int>>::value);
        static_assert(!IsUnspecializedTemplate<std::vector<int, std::allocator<int>>>::value);
        static_assert(!IsUnspecializedTemplate<std::list<int, std::allocator<int>>>::value);
        static_assert(!IsUnspecializedTemplate<int>::value);

        static_assert(IsTemplate<std::vector<int>>::value);
        static_assert(IsTemplate<std::list<int>>::value);
        static_assert(IsTemplate<std::vector<int, std::allocator<int>>>::value);
        static_assert(IsTemplate<std::list<int, std::allocator<int>>>::value);
        static_assert(IsTemplate<std::vector>::value);
        static_assert(IsTemplate<std::list>::value);
        static_assert(!IsTemplate<int>::value);
 */
        template <typename Template, size_t N>
            requires (IsSpecializedTemplate<Template>::value)
        // Use `TypeContainer`'s `get` template to get the Nth type of the template `Template`
        using GetTemplateArgN = typename GetTemplateArgs<Template>::type::template get<N>;

        static_assert(std::same_as<GetTemplateArgN<std::vector<int>, 0>, int>);
        static_assert(std::same_as<GetTemplateArgN<std::list<int>, 0>, int>);
        static_assert(std::same_as<GetTemplateArgN<std::vector<int, std::allocator<int>>, 0>, int>);
        static_assert(std::same_as<GetTemplateArgN<std::list<int, std::allocator<int>>, 0>, int>);
        static_assert(std::same_as<GetTemplateArgN<std::vector<int, std::allocator<int>>, 1>, std::allocator<int>>);
        static_assert(std::same_as<GetTemplateArgN<std::list<int, std::allocator<int>>, 1>, std::allocator<int>>);

        /**
         * @struct CanConstructFromImpl
         * @brief Check if all types in a type container `T1` are in the second type container `T2` (condition 1),
         *        and that each type in `T1` appears at least the same amount of times in `T2` (condition 2).
         * 
         * @tparam T1 The type container to check
         * @tparam T2 The type container to check against
         */
        template <typename T1, typename T2>
            requires (SPECIALIZATION_OF(T1, TypeContainer) && SPECIALIZATION_OF(T2, TypeContainer))
        class CanConstructFromImpl
        {
            private:
                template <typename T>
                struct CanConstructFromImplHelperCondition1
                {
                    static consteval bool check()
                    {
                        return T2::template contains<T>;
                    }
                    static constexpr bool value = check();

                    consteval operator bool()
                    {
                        return value;
                    }
                };
                template <typename T>
                struct CanConstructFromImplHelperCondition2
                {
                    static consteval bool check()
                    {
                        return T2::template count<T> >= T1::template count<T>;
                    }
                    static constexpr bool value = check();

                    consteval operator bool()
                    {
                        return value;
                    }
                };
            public:
                static constexpr bool value = T1::template apply_predicate_conjunction<CanConstructFromImplHelperCondition1> &&
                                              T1::template apply_predicate_conjunction<CanConstructFromImplHelperCondition2>;
        };

#ifdef CAN_CONSTRUCT_FROM
    #undef CAN_CONSTRUCT_FROM
#endif
/**
 * @brief Usage: `CAN_CONSTRUCT_FROM((type1, type2, type3), (type4, type5, type6))`
 * 
 */
#define CAN_CONSTRUCT_FROM(T1, T2) (::SupDef::Util::CanConstructFromImpl<TypeContainer<ID T1>, TypeContainer<ID T2>>::value)

        static_assert(CAN_CONSTRUCT_FROM((int, char, float), (int, char, float)));
        static_assert(CAN_CONSTRUCT_FROM((int, char, float), (float, int, char)));
        static_assert(!CAN_CONSTRUCT_FROM((int, int, float), (float, int, char)));
        static_assert(CAN_CONSTRUCT_FROM((int, int, float), (float, int, char, int)));

        /**
         * @brief Check if the `std::codecvt<C1, C2, std::mbstate_t>` class is valid, i.e. garanteed by the standard to be implemented,
         *        and not deprecated.
         * 
         * @tparam C1 The first character type
         * @tparam C2 The second character type
         */
        template <typename C1, typename C2>
        concept IsValidCodeCvt = CAN_CONSTRUCT_FROM((C1, C2), (char   , char    ))            ||
                                 CAN_CONSTRUCT_FROM((C1, C2), (char   , wchar_t ))            ||
                                 CAN_CONSTRUCT_FROM((C1, C2), (char8_t, char16_t, char32_t));

        static_assert(IsValidCodeCvt<char, char>);
        static_assert(IsValidCodeCvt<char, wchar_t>);
        static_assert(IsValidCodeCvt<wchar_t, char>);
        static_assert(IsValidCodeCvt<char8_t, char16_t>);
        static_assert(IsValidCodeCvt<char16_t, char8_t>);
        static_assert(IsValidCodeCvt<char8_t, char32_t>);
        static_assert(IsValidCodeCvt<char32_t, char8_t>);
        static_assert(IsValidCodeCvt<char16_t, char32_t>);
        static_assert(IsValidCodeCvt<char32_t, char16_t>);
        static_assert(!IsValidCodeCvt<char, char8_t>);
        static_assert(!IsValidCodeCvt<char8_t, char>);
        static_assert(!IsValidCodeCvt<char, char16_t>);
        static_assert(!IsValidCodeCvt<char16_t, char>);
        static_assert(!IsValidCodeCvt<char, char32_t>);
        static_assert(!IsValidCodeCvt<char32_t, char>);
        static_assert(!IsValidCodeCvt<wchar_t, char8_t>);
        static_assert(!IsValidCodeCvt<char8_t, wchar_t>);
        static_assert(!IsValidCodeCvt<wchar_t, char16_t>);
        static_assert(!IsValidCodeCvt<char16_t, wchar_t>);
        static_assert(!IsValidCodeCvt<wchar_t, char32_t>);
        static_assert(!IsValidCodeCvt<char32_t, wchar_t>);

        // Base template
#if 0
        template <typename C1, typename C2>
            requires (CharacterType<C1> || std::same_as<C1, std::filesystem::path>) && CharacterType<C2>
        inline std::basic_string<C1> convert(const std::basic_string<C2>& str /* External */);
#else
        template <typename C1, typename C2>
            requires CharacterType<C1> && CharacterType<C2>
        inline std::basic_string<C1> convert(const std::basic_string<C2>& str /* External */) = delete;
#endif

                /* Internal */ /* External */
        template <typename C1, typename C2>
            requires IsValidCodeCvt<C1, C2> && (!(std::same_as<C1, C2> && (std::same_as<char, C1> || std::same_as<char, C2>))) /* No `<char, char>` */ &&
                     (CharacterType<C1> && CharacterType<C2>) // double check, even if garanteed by `IsValidCodeCvt`, in case I fucked up
        inline std::basic_string<C1> convert(const std::basic_string<C2>& str /* External */)
        {
            using facet_type = typename std::codecvt<C1, C2, std::mbstate_t>;
            const facet_type& f = std::use_facet<std::codecvt<C1, C2, std::mbstate_t>>(std::locale());
            std::mbstate_t mb = std::mbstate_t();
            std::codecvt_base::result res;
            std::size_t factor = 4;
            /* Internal */
            std::basic_string<C1> ret(str.size() * factor, static_cast<C1>('\0'));
            C1* internal_next = nullptr;
            const C2* external_next = nullptr;
            do
            {
                mb = std::mbstate_t();
                ret.clear();
                internal_next = nullptr;
                external_next = nullptr;
                res = f.in(
                    mb, &str[0], &str[str.size()], external_next, &ret[0], &ret[ret.size()], internal_next
                );
                factor++;
            } while (res == std::codecvt_base::partial);
            if (res == std::codecvt_base::error)
                throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "convert(const std::basic_string<C2>&): std::codecvt_base::error");
            ret.resize(internal_next - &ret[0]);
            return ret;
        }

        // Assuming UTF-8 locale and that exec encoding for `char` is UTF-8
        template <>
        inline std::basic_string<char> convert<char, char8_t>(const std::basic_string<char8_t>& str /* External */)
        {
            std::basic_string<char> ret;
            ret.reserve(str.size());
            for (auto& c : str)
                ret += reinterpret_cast<const char&>(c);
            return ret;
        }

        template <>
        inline std::basic_string<char8_t> convert<char8_t, char>(const std::basic_string<char>& str /* External */)
        {
            std::basic_string<char8_t> ret;
            ret.reserve(str.size());
            for (auto& c : str)
                ret += reinterpret_cast<const char8_t&>(c);
            return ret;
        }

        template <>
        inline std::basic_string<wchar_t> convert<wchar_t, char8_t>(const std::basic_string<char8_t>& str /* External */)
        {
            return convert<wchar_t>(convert<char>(str));
        }

        template <>
        inline std::basic_string<char8_t> convert<char8_t, wchar_t>(const std::basic_string<wchar_t>& str /* External */)
        {
            return convert<char8_t>(convert<char>(str));
        }

        template <typename C1, typename C2>
            requires CAN_CONSTRUCT_FROM((C2), (char16_t, char32_t)) && std::same_as<char, C1> && (CharacterType<C1> && CharacterType<C2>)
        inline std::basic_string<C1> convert(const std::basic_string<C2>& str /* External */)
        {
            return convert<C1>(convert<char8_t>(str));
        }

        template <typename C1, typename C2>
            requires CAN_CONSTRUCT_FROM((C1), (char16_t, char32_t)) && std::same_as<char, C2> && (CharacterType<C1> && CharacterType<C2>)
        inline std::basic_string<C1> convert(const std::basic_string<C2>& str /* External */)
        {
            return convert<C1>(convert<char8_t>(str));
        }

        template <typename C1, typename C2>
            requires CAN_CONSTRUCT_FROM((C2), (char16_t, char32_t)) && std::same_as<wchar_t, C1> && (CharacterType<C1> && CharacterType<C2>)
        inline std::basic_string<C1> convert(const std::basic_string<C2>& str /* External */)
        {
            return convert<C1>(convert<char>(convert<char8_t>(str)));
        }

        template <typename C1, typename C2>
            requires CAN_CONSTRUCT_FROM((C1), (char16_t, char32_t)) && std::same_as<wchar_t, C2> && (CharacterType<C1> && CharacterType<C2>)
        inline std::basic_string<C1> convert(const std::basic_string<C2>& str /* External */)
        {
            return convert<C1>(convert<char8_t>(convert<char>(str)));
        }

        template <typename C1, typename C2>
            requires (CharacterType<C1> && CharacterType<C2>) && std::same_as<C1, C2>
        constexpr inline std::basic_string<C1> convert(const std::basic_string<C2>& str /* External */)
        {
            return std::basic_string<C1>(str);
        }
        
        // Base template
        template <typename C1>
            requires CharacterType<C1> || std::same_as<C1, std::filesystem::path>
        inline auto convert(const std::filesystem::path& path) = delete;
        
        template <CharacterType C1>
        inline std::basic_string<C1> convert(const std::filesystem::path& path)
        {
            return convert<C1>(path.string());
        }

        template <std::same_as<std::filesystem::path> C1>
        inline C1 convert(const std::filesystem::path& path)
        {
            return path;
        }

        // Base template
        template <typename C1, typename C2>
            requires std::same_as<C1, std::filesystem::path> && CharacterType<C2>
        inline std::filesystem::path convert(const std::basic_string<C2>& str /* External */)
        {
            return std::filesystem::path(convert<char>(str));
        }

#if 0
        // Base template
        template <typename C1, typename C2>
        inline C1 convert(const std::basic_string<C2>& str /* External */) = delete;

        // Base template
        template <typename C1, typename C2>
        inline std::basic_string<C1> convert(const C2& c) = delete;
#endif

        template <typename C1, typename C2>
            requires CharacterType<C1> && CharacterType<C2>
        inline std::basic_string<C1> convert(const C2& c)
        {
            return convert<C1>(std::basic_string<C2>(1, c));
        }

        template <typename C1, typename C2>
            requires CharacterType<C1> && CharacterType<C2>
        inline std::basic_string<C1> convert(const C2&& c)
        {
            return convert<C1>(std::basic_string<C2>(1, c));
        }

        // TODO: Make this a convert overload rather than a convert_num overload
        /* template <typename C1, typename C2>
            requires CharacterType<C1> && std::is_arithmetic_v<C2> && (!CharacterType<C2>)
        inline std::basic_string<C1> convert_num(const C2& c)
        {
            std::stringstream ss(std::ios_base::in | std::ios_base::out | std::ios_base::binary);
            ss << c;
            return convert<C1>(ss.str());
        } */

        template <typename C1, typename C2>
            requires std::is_arithmetic_v<C1> && (!CharacterType<C1>) && CharacterType<C2>
        inline C1 convert_num(const std::basic_string<C2>& str /* External */)
        {
            std::stringstream ss(convert<char>(str), std::ios_base::in | std::ios_base::out | std::ios_base::binary);
            C1 ret;
            ss >> ret;
            return ret;
        }

        template <typename C1, typename C2>
            requires std::is_arithmetic_v<C1> && (!CharacterType<C1>) && CharacterType<C2>
        inline C1 convert_num(const C2& c)
        {
            return convert_num<C1>(std::basic_string<C2>(1, c));
        }

#ifdef CONVERT
    #undef CONVERT
#endif
#define CONVERT(TYPE, STR_OR_CHAR) ::SupDef::Util::convert<TYPE>(STR_OR_CHAR)

#ifdef CONVERT_NUM
    #undef CONVERT_NUM
#endif
#define CONVERT_NUM(TYPE, STR_OR_NUM) ::SupDef::Util::convert_num<TYPE>(STR_OR_NUM)

        template <typename C1, typename C2>
            requires CharacterType<C1> && CharacterType<C2>
        inline bool same(const std::basic_string<C1>& s1, const std::basic_string<C2>& s2)
        {
            return CONVERT(char8_t, s1) == CONVERT(char8_t, s2);
        }

        template <typename C1, typename C2>
            requires CharacterType<C1> && CharacterType<C2>
        inline bool same(const std::basic_string<C1>& s1, const C2* s2)
        {
            return CONVERT(char8_t, s1) == CONVERT(char8_t, std::basic_string<C2>(s2));
        }

        template <typename C1, typename C2>
            requires CharacterType<C1> && CharacterType<C2>
        inline bool same(const C1* s1, const std::basic_string<C2>& s2)
        {
            return CONVERT(char8_t, std::basic_string<C1>(s1)) == CONVERT(char8_t, s2);
        }

        template <typename C1, typename C2>
            requires CharacterType<C1> && CharacterType<C2>
        inline bool same(const C1 c1, const C2 c2)
        {
            return CONVERT(char8_t, c1) == CONVERT(char8_t, c2);
        }

#ifdef SAME
    #undef SAME
#endif
#define SAME(S1, S2) ::SupDef::Util::same(S1, S2)

#ifdef DIFFERENT
    #undef DIFFERENT
#endif
#define DIFFERENT(S1, S2) (!SAME(S1, S2))

        template <typename T>
            requires CharacterType<T>
        static inline auto remove_whitespaces(const std::basic_string<T>& s, bool rm_nl = false, bool only_trailing_and_leading = false) -> std::basic_string<T>
        {
            if (!only_trailing_and_leading)
            {
                std::basic_string<T> res;
                res.reserve(s.size());
                for (auto& c : s)
                {
                    if (DIFFERENT(c, ' ') && DIFFERENT(c, '\t') && (rm_nl ? DIFFERENT(c, '\n') : true))
                        res += c;
                }
                return res;
            }
            else
            {
                size_t start = 0;
                size_t end = s.size() - 1;
                while (start < s.size() && (SAME(s[start], ' ') || SAME(s[start], '\t') || (rm_nl ? SAME(s[start], '\n') : false)))
                    ++start;
                while (end > 0 && (SAME(s[end], ' ') || SAME(s[end], '\t') || (rm_nl ? SAME(s[end], '\n') : false)))
                    --end;
                return s.substr(start, end - start + 1);
            }
        }

        static inline std::filesystem::path remove_whitespaces(const std::filesystem::path& path, bool rm_nl = false, bool only_trailing_and_leading = false)
        {
            return std::filesystem::path(remove_whitespaces(path.string(), rm_nl, only_trailing_and_leading));
        }

        /**
         * @fn inline constexpr std::array<C, S> any_string(const char (&literal)[S])
         * @brief Utility function to ease manipulating strings.
         * 
         * @tparam C The character type of the returned array
         * @tparam S The size of the returned array (deduced from the string literal)
         * @param[in] literal The string literal to convert to an `std::array<C, S>`. Must be of the form: `char[S]` (possibly cv-qualified)
         * @return Returns an `std::array<C, S>` (where `C` is a character type) from a string literal of character type `char`
         * @todo Put it in `SupDef::Util` namespace
         */
        template <typename C, size_t S>
            requires CharacterType<C>
        inline constexpr auto any_string(const char (&literal)[S]) -> std::array<C, S>
        {
            std::basic_string<C> str(CONVERT(C, std::string(literal)));
            std::array<C, S> arr{};
            std::copy(str.begin(), str.end(), arr.begin());
            return arr;
        }

#if defined(ANY_STRING)
    #undef ANY_STRING
#endif
/**
 * @def ANY_STRING(TYPE, LIT)
 * @brief A wrapper equivalent to `std::basic_string<TYPE>(any_string<TYPE>(LIT).data())`
 * @param TYPE The character type of the string
 * @param LIT The string literal to convert to a `std::basic_string<TYPE>`
 */
#define ANY_STRING(TYPE, LIT) (std::basic_string<TYPE>(::SupDef::Util::any_string<TYPE>(LIT).data()))

        enum class DeferType : uint8_t
        {
            SCOPE_EXIT = 0,
            SCOPE_FAIL = 1,
            SCOPE_SUCCESS = 2
        };

#if SUPDEF_HAS_SCOPE_EXIT
        template <DeferType T, typename F>
        using DeferImplBase = std::conditional_t<
                                (T == DeferType::SCOPE_EXIT),
                                std::experimental::scope_exit<F>,
                                std::conditional_t<
                                    (T == DeferType::SCOPE_FAIL),
                                    std::experimental::scope_fail<F>,
                                    std::experimental::scope_success<F>
                                >
                            >;

        // TODO: Test this
        template <DeferType T, typename... F>
        class DeferImpl
        {
            private:
                std::tuple<DeferImplBase<T, F>...> deferals;
                std::tuple<F...> deferals_fns;
                bool released = false;

                // Execute and release
                void execute_and_release() noexcept((std::is_nothrow_invocable_v<F> && ...))
                {
                    std::apply([](auto&&... args) { ((args()), ...); }, this->deferals_fns);
                    this->release();
                }
            public:
                DeferImpl() = default;
                template <typename... Fn>
                    requires (sizeof...(Fn) == sizeof...(F)) &&
                             (std::constructible_from<DeferImplBase<T, F>, Fn> && ...)
                DeferImpl(Fn&&... fn) noexcept((std::is_nothrow_constructible_v<DeferImplBase<T, F>, Fn> && ...))
                    : deferals(std::forward<Fn>(fn)...)
                    , deferals_fns(std::forward<Fn>(fn)...)
                { }
                
                DeferImpl(const DeferImpl&) = delete;
                DeferImpl(DeferImpl&&) = default;

                DeferImpl& operator=(const DeferImpl&) = delete;
                DeferImpl& operator=(DeferImpl&&) = delete;
                
                ~DeferImpl() = default;

                void release() noexcept
                {
                    std::apply([](auto&&... args) { ((args.release()), ...); }, this->deferals);
                    this->released = true;
                }

                operator bool() const noexcept
                {
                    return !this->released;
                }

                void operator()() noexcept(noexcept(this->execute_and_release()))
                {
                    this->execute_and_release();
                }
        };
#else
    #error "Defer not implemented yet"
#endif

#ifdef DEFER
    #undef DEFER
#endif
#define DEFER(...)                                                      \
    PP_IF(                                                              \
        BOOST_PP_AND(                                                   \
            BOOST_PP_GREATER(VA_COUNT(__VA_ARGS__), 1),                 \
            IS_ARG_DEFER_SCOPE_TYPE(FIRST_ARG(__VA_ARGS__))             \
        )                                                               \
    )                                                                   \
    (                                                                   \
        ::SupDef::Util::DeferImpl<                                      \
            ::SupDef::Util::DeferType::FIRST_ARG(__VA_ARGS__)           \
        > BOOST_PP_LPAREN() DROP_FIRST(__VA_ARGS__) BOOST_PP_RPAREN()   \
    )                                                                   \
    (                                                                   \
        ::SupDef::Util::DeferImpl<                                      \
            ::SupDef::Util::DeferType::SCOPE_EXIT                       \
        > BOOST_PP_LPAREN() __VA_ARGS__ BOOST_PP_RPAREN()               \
    )

#ifdef PP_DETECTOR_SCOPE_EXIT_SCOPE_TYPE_ARG
    #undef PP_DETECTOR_SCOPE_EXIT_SCOPE_TYPE_ARG
#endif
#define PP_DETECTOR_SCOPE_EXIT_SCOPE_TYPE_ARG ~, ~

#ifdef PP_DETECTOR_SCOPE_FAIL_SCOPE_TYPE_ARG
    #undef PP_DETECTOR_SCOPE_FAIL_SCOPE_TYPE_ARG
#endif
#define PP_DETECTOR_SCOPE_FAIL_SCOPE_TYPE_ARG ~, ~

#ifdef PP_DETECTOR_SCOPE_SUCCESS_SCOPE_TYPE_ARG
    #undef PP_DETECTOR_SCOPE_SUCCESS_SCOPE_TYPE_ARG
#endif
#define PP_DETECTOR_SCOPE_SUCCESS_SCOPE_TYPE_ARG ~, ~

#ifdef IS_ARG_DEFER_SCOPE_TYPE
    #undef IS_ARG_DEFER_SCOPE_TYPE
#endif
#define IS_ARG_DEFER_SCOPE_TYPE(ARG) PP_DETECT(ARG, SCOPE_TYPE_ARG)

        template <typename T, size_t N>
            requires CharacterType<T>
        consteval size_t cstr_len(const T (&literal)[N])
        {
            size_t i = 0;
            while (i < N && literal[i] != T(0))
                ++i;
            return i;
        }

        static_assert(cstr_len("Hello")   == 5);
        static_assert(cstr_len(L"Hello")  == 5);
        static_assert(cstr_len(u8"Hello") == 5);
        static_assert(cstr_len(u"Hello")  == 5);
        static_assert(cstr_len(U"Hello")  == 5);

#ifdef CSTR_LEN
    #undef CSTR_LEN
#endif
#define CSTR_LEN(LIT) ::SupDef::Util::cstr_len(LIT)

        template <typename T1, typename T2>
            requires (!std::is_lvalue_reference_v<T1> && !std::is_lvalue_reference_v<T2>)
        constexpr inline int compare_any(const T1& s1, const T2& s2)
        {
            if (sizeof(T1) != sizeof(T2))
                return sizeof(T1) < sizeof(T2) ? -1 : 1;
            return memcmp(std::addressof(s1), std::addressof(s2), sizeof(T1));
        }

        template <typename T1, typename T2>
            requires (!std::is_lvalue_reference_v<T1> && !std::is_lvalue_reference_v<T2>)
        constexpr inline bool same_any(const T1& s1, const T2& s2)
        {
            return compare_any(s1, s2) == 0;
        }

        template <typename T1, typename T2>
            requires (!std::is_lvalue_reference_v<T1> && !std::is_lvalue_reference_v<T2>)
        constexpr inline bool different_any(const T1& s1, const T2& s2)
        {
            return compare_any(s1, s2) != 0;
        }

#ifdef COMPARE_ANY
    #undef COMPARE_ANY
#endif
#define COMPARE_ANY(S1, S2) ::SupDef::Util::compare_any(S1, S2)

#ifdef SAME_ANY
    #undef SAME_ANY
#endif
#define SAME_ANY(S1, S2) ::SupDef::Util::same_any(S1, S2)

#ifdef DIFFERENT_ANY
    #undef DIFFERENT_ANY
#endif
#define DIFFERENT_ANY(S1, S2) ::SupDef::Util::different_any(S1, S2)

        template <typename T>
            requires (!std::is_lvalue_reference_v<T>)
        constexpr inline bool is_null(const T& t)
        {
            return COMPARE_ANY(t, T()) == 0;
        }

#ifdef IS_NULL
    #undef IS_NULL
#endif
#define IS_NULL(T) ::SupDef::Util::is_null(T)

        template <typename CharType>
            requires CharacterType<CharType>
        using pragma_loc_type = typename std::tuple<typename std::basic_string<CharType>, string_size_type<CharType>, string_size_type<CharType>>;

#ifdef DECL_TEST_FUNC_CALLABLE_WITH
    #undef DECL_TEST_FUNC_CALLABLE_WITH
#endif
#define DECL_TEST_FUNC_CALLABLE_WITH(FUNC_NAME)                             \
    template<typename = void, typename... Args>                             \
    struct FUNC_NAME##CallableWithImpl : std::false_type {};                \
                                                                            \
    template<typename... Args>                                              \
    struct FUNC_NAME##CallableWithImpl<                                     \
        std::void_t<decltype(FUNC_NAME(std::declval<Args>()...))>, Args...  \
    >                                                                       \
    : std::true_type {};                                                    \
                                                                            \
    template<typename... Args>                                              \
    inline constexpr bool FUNC_NAME##CallableWith =                         \
                                         FUNC_NAME##CallableWithImpl<       \
                                            void, Args...                   \
                                         >::value;

#ifdef DECL_TEST_FUNC_CALLABLE_WITH_IN_STRUCT
    #undef DECL_TEST_FUNC_CALLABLE_WITH_IN_STRUCT
#endif
#define DECL_TEST_FUNC_CALLABLE_WITH_IN_STRUCT(FUNC_NAME)                       \
    struct FUNC_NAME##CallableWithImpl                                          \
    {                                                                           \
        template<typename = void, typename... Args>                             \
        struct CallableWithImpl : std::false_type {};                           \
                                                                                \
        template<typename... Args>                                              \
        struct CallableWithImpl<                                                \
            std::void_t<decltype(FUNC_NAME(std::declval<Args>()...))>, Args...  \
        >                                                                       \
        : std::true_type {};                                                    \
                                                                                \
        template<typename... Args>                                              \
        static inline constexpr bool CallableWith =                             \
                                             CallableWithImpl<                  \
                                                void, Args...                   \
                                             >::value;                          \
    };

#ifdef STD_DECLVAL
    #undef STD_DECLVAL
#endif
#define STD_DECLVAL(TYPE) std::declval<TYPE>()

#ifdef CALLABLE_WITH
    #undef CALLABLE_WITH
#endif
#define CALLABLE_WITH(MAC, FUNC_NAME, ...) (requires { FUNC_NAME(MAP_LIST(MAC, __VA_ARGS__)); })

#ifdef CALLABLE_WITH2
    #undef CALLABLE_WITH2
#endif
#define CALLABLE_WITH2(FUNC_NAME, ...) FUNC_NAME##CallableWith<__VA_ARGS__>

#ifdef CALLABLE_WITH3
    #undef CALLABLE_WITH3
#endif
#define CALLABLE_WITH3(FUNC_NAME, ...) FUNC_NAME##CallableWith<__VA_ARGS__>

        template <typename T, typename U>
            requires (!std::is_array_v<T> && !std::is_array_v<U>)
        struct IsImpl
        {
            constexpr bool operator()(const T& t, const U& u) const
            {
                return SAME_ANY(t, u) && std::same_as<T, U> && std::addressof(t) == std::addressof(u);
            }

            constexpr bool operator()(T&& t, U&& u) const
            {
                return SAME_ANY(t, u) && std::same_as<T, U>;
            }

            constexpr bool operator()(const T& t, U&& u) const
            {
                return false;
            }

            constexpr bool operator()(T&& t, const U& u) const
            {
                return false;
            }
        };

#ifdef IS
    #undef IS
#endif
#define IS(OBJ1, OBJ2) (::SupDef::Util::IsImpl<std::remove_cvref_t<decltype(OBJ1)>, std::remove_cvref_t<decltype(OBJ2)>>()(OBJ1, OBJ2))

        // TODO: Add tests for this
        template <typename T>
        constexpr std::string_view type_name(bool use_demangler)
        {
            std::string_view name;
            if (use_demangler)
                name = ::SupDef::Util::demangle(typeid(T).name());
            else // Use compiler-specific name "printing"
            {
                std::string_view prefix, suffix;
#ifdef __clang__
                name = __PRETTY_FUNCTION__;
                prefix = "std::string_view SupDef::Util::type_name(bool)";
                suffix = "]";
#elif defined(__GNUC__)
                name = __PRETTY_FUNCTION__;
                prefix = "constexpr std::string_view SupDef::Util::type_name(bool) [with T = ";
                suffix = "; std::string_view = std::basic_string_view<char>]";
#elif defined(_MSC_VER) && !defined(__clang__)
                name = __FUNCSIG__;
                prefix = "class std::basic_string_view<char,struct std::char_traits<char> > __cdecl SupDef::Util::type_name<";
                suffix = ">(bool)";
#else
                return ::SupDef::Util::demangle(typeid(T).name());
                UNREACHABLE();                
#endif
                name.remove_prefix(prefix.size());
                name.remove_suffix(suffix.size());
            }
            return name;
        }

#ifdef hard_assert
    #undef hard_assert
#endif
#define hard_assert(COND) (static_cast<bool>((COND)) ? (void)0 : ::SupDef::Util::hard_assert_fail((COND), PP_STRINGIZE((COND)), __FILE__, __LINE__, __PRETTY_FUNCTION__))

#ifdef hard_assert_msg
    #undef hard_assert_msg
#endif
#define hard_assert_msg(COND, MSG) (static_cast<bool>((COND)) ? (void)0 : ::SupDef::Util::hard_assert_fail((COND), PP_STRINGIZE((COND)), __FILE__, __LINE__, __PRETTY_FUNCTION__, MSG))

        [[noreturn]]
        constexpr inline void hard_assert_fail(bool cond, const char* cond_str, const char* file, int line, const char* func)
        {
            if (!cond)
            {
                std::cerr << "Hard assertion failed: " << cond_str << "\n"
                          << "File: " << file << "\n"
                          << "Line: " << line << "\n"
                          << "Function: " << func << "\n";
                SUPDEF_EXIT();
            }
        }

        template <typename T>
            requires std::convertible_to<T, std::string_view>
        [[noreturn]]
        constexpr inline void hard_assert_fail(bool cond, const char* cond_str, const char* file, int line, const char* func, const T& msg)
        {
            if (!cond)
            {
                std::cerr << "Hard assertion failed: " << cond_str << " (" << msg << ")\n"
                          << "File: " << file << "\n"
                          << "Line: " << line << "\n"
                          << "Function: " << func << "\n";
                SUPDEF_EXIT();
            }
        }

#ifdef GEN_EXC_HANDLER_IMPL
    #undef GEN_EXC_HANDLER_IMPL
#endif
/* 
 * Use like :
 *   GEN_EXC_HANDLER_IMPL((<exception type>, {<code to execute>}))
 */
#define GEN_EXC_HANDLER_IMPL(TUPLE)        \
    catch(const TUPLE_FIRST_ARG(TUPLE)& e) \
    TUPLE_SECOND_ARG(TUPLE)

#ifdef GEN_EXC_HANDLER_ALT
    #undef GEN_EXC_HANDLER_ALT
#endif
/* 
 * Use like :
 *   GEN_EXC_HANDLER_ALT({<code to execute by default>}, (<exception type n°1>, {<code to execute n°1>}), (<exception type n°2>, {<code to execute n°2>}), ...)
 */
#define GEN_EXC_HANDLER_ALT(DEFAULT_CASE_BODY, ...) \
    ([](std::exception_ptr eptr) -> void            \
    {                                               \
        try                                         \
        {                                           \
            std::rethrow_exception(eptr);           \
        }                                           \
        MAP(GEN_EXC_HANDLER_IMPL, __VA_ARGS__)      \
        catch(...)                                  \
            DEFAULT_CASE_BODY                       \
                                                    \
    })                                              \
/**/

#ifdef GEN_EXC_HANDLER
    #undef GEN_EXC_HANDLER
#endif
/* 
 * Use like :
 *   GEN_EXC_HANDLER((<exception type n°1>, {<code to execute n°1>}), (<exception type n°2>, {<code to execute n°2>}), ...)
 */
#define GEN_EXC_HANDLER(...)                                            \
    GEN_EXC_HANDLER_ALT(                                                \
        {                                                               \
            std::cerr << "Unhandled exception of type `" <<             \
            ::SupDef::Util::demangle(                                   \
                eptr ?                                                  \
                eptr.__cxa_exception_type()->name() :                   \
                std::current_exception().__cxa_exception_type()->name() \
            ) << "`" <<                                                 \
            std::endl;                                                  \
            std::terminate();                                           \
        },                                                              \
        __VA_ARGS__                                                     \
    )                                                                   \
/**/

        template <typename R, typename T>
        concept ContainerCompatibleRange = std::ranges::input_range<R> &&
                                           std::convertible_to<std::ranges::range_reference_t<R>, T>;

        static_assert(ContainerCompatibleRange<std::vector<int>, int>);

        template <typename... Args>
        struct ThreadSafeQueueStopRequired : public std::exception
        {
            ThreadSafeQueueStopRequired() = default;
            ThreadSafeQueueStopRequired(const ThreadSafeQueueStopRequired&) = default;
            ThreadSafeQueueStopRequired(ThreadSafeQueueStopRequired&&) = default;
            ThreadSafeQueueStopRequired& operator=(const ThreadSafeQueueStopRequired&) = default;
            ThreadSafeQueueStopRequired& operator=(ThreadSafeQueueStopRequired&&) = default;
            ~ThreadSafeQueueStopRequired() = default;
        
            const char* what() const noexcept override
            {
#if 0
                static const char* msg = "This wait was interrupted because the associated ThreadSafeQueue was requested to stop";
                return msg;
#else
                static std::array<std::string, sizeof...(Args)> type_names = { std::string(type_name<Args>(true))... };
                static std::string msg;
                if constexpr (sizeof...(Args) == 1)
                {
                    msg = "This wait was interrupted because the associated ThreadSafeQueue<" + type_names.at(0) + "> was requested to stop";
                    return msg.c_str();
                }
                else if constexpr (sizeof...(Args) > 1)
                {
                    msg = "This wait was interrupted because the associated ThreadSafeQueue<" + std::accumulate(type_names.begin() + 1, type_names.end(), type_names.at(0), [](const std::string& acc, const std::string& s) { return acc + ", " + s; }) + "> was requested to stop";
                    return msg.c_str();
                }
                else
                    return "This wait was interrupted because the associated ThreadSafeQueue was requested to stop";
#endif
            }
        };

        template <typename Tp, typename Container = std::deque<Tp>>
        class ThreadSafeQueue
        {
            public:
                using QueueType = std::queue<Tp, Container>;

                using container_type = typename QueueType::container_type;
                using value_type = typename QueueType::value_type;
                using size_type = typename QueueType::size_type;
                using reference = typename QueueType::reference;
                using const_reference = typename QueueType::const_reference;

            private:
                QueueType queue;
                mutable std::mutex mutex{};
                mutable std::atomic_flag not_empty = ATOMIC_FLAG_INIT;
                mutable std::atomic<bool> stop_required = false;

            public:
                using StopRequired = ThreadSafeQueueStopRequired<Tp, Container>;

                ThreadSafeQueue() : queue()
                {
                    if (!this->queue.empty())
                    {
                        std::ignore = this->not_empty.test_and_set(std::memory_order::relaxed);
                        this->not_empty.notify_all();
                    }
                    else
                        this->not_empty.clear(std::memory_order::relaxed);
                }

                ThreadSafeQueue(const ThreadSafeQueue&) = delete;
                ThreadSafeQueue(ThreadSafeQueue&& other) : queue(std::move(other.queue))
                {
                    if (!this->queue.empty())
                    {
                        std::ignore = this->not_empty.test_and_set(std::memory_order::relaxed);
                        this->not_empty.notify_all();
                    }
                    else
                        this->not_empty.clear(std::memory_order::relaxed);
                }

                template <typename... Args>
                    requires std::constructible_from<QueueType, Args...>
                ThreadSafeQueue(Args&&... args)
                    : queue(std::forward<Args>(args)...)
                {
                    if (!this->queue.empty())
                    {
                        std::ignore = this->not_empty.test_and_set(std::memory_order::relaxed);
                        this->not_empty.notify_all();
                    }
                    else
                        this->not_empty.clear(std::memory_order::relaxed);
                }
                template <typename T>
                    requires std::convertible_to<T, value_type> || std::constructible_from<value_type, T>
                ThreadSafeQueue(std::initializer_list<T> il)
                    : queue()
                {
                    for (auto& e : il)
                    {
                        this->queue.push(value_type(e));
                        std::ignore = this->not_empty.test_and_set(std::memory_order::relaxed);
                        this->not_empty.notify_one();
                    }
                }

                ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;
                ThreadSafeQueue& operator=(ThreadSafeQueue&&) = delete;

                ~ThreadSafeQueue() = default;

                value_type front() const
                {
                    std::lock_guard<std::mutex> lock(this->mutex);
                    if (this->queue.empty())
                        throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "ThreadSafeQueue::front(): queue is empty");
                    return this->queue.front();
                }

                value_type back() const
                {
                    std::lock_guard<std::mutex> lock(this->mutex);
                    if (this->queue.empty())
                        throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "ThreadSafeQueue::back(): queue is empty");
                    return this->queue.back();
                }

                [[nodiscard]]
                bool empty() const
                {
                    std::lock_guard<std::mutex> lock(this->mutex);
                    return this->queue.empty();
                }

                size_type size() const
                {
                    std::lock_guard<std::mutex> lock(this->mutex);
                    return this->queue.size();
                }

                void push(const value_type& t)
                {
                    std::lock_guard<std::mutex> lock(this->mutex);
                    this->queue.push(t);
                    std::ignore = this->not_empty.test_and_set(std::memory_order::release);
                    this->not_empty.notify_one();
                }

                void push(value_type&& t)
                {
                    std::lock_guard<std::mutex> lock(this->mutex);
                    this->queue.push(std::move(t));
                    std::ignore = this->not_empty.test_and_set(std::memory_order::release);
                    this->not_empty.notify_one();
                }

                template <typename... Args>
                decltype(auto) emplace(Args&&... args)
                {
                    std::lock_guard<std::mutex> lock(this->mutex);
                    decltype(auto) ret = this->queue.emplace(std::forward<Args>(args)...);
                    std::ignore = this->not_empty.test_and_set(std::memory_order::release);
                    this->not_empty.notify_one();
                    return ret;
                }

                void pop()
                {
                    std::lock_guard<std::mutex> lock(this->mutex);
                    if (this->queue.empty())
                        throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "ThreadSafeQueue::pop(): queue is empty");
                    this->queue.pop();
                    if (this->queue.empty())
                        this->not_empty.clear(std::memory_order::release);
                }

                void swap(ThreadSafeQueue& other) noexcept(std::is_nothrow_swappable_v<QueueType>)
                {
                    if (this != std::addressof(other))
                    {
                        std::scoped_lock locker(this->mutex, other.mutex);
                        std::swap(this->queue, other.queue);
                    }
                }

                QueueType& to_unsafe() noexcept
                {
                    return this->queue;
                }

                const QueueType& to_unsafe() const noexcept
                {
                    return this->queue;
                }

                value_type next()
                {
                    std::lock_guard<std::mutex> lock(this->mutex);
                    if (this->queue.empty())
                        throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "ThreadSafeQueue::next(): queue is empty");
                    value_type ret = this->queue.front();
                    this->queue.pop();
                    if (this->queue.empty())
                        this->not_empty.clear(std::memory_order::release);
                    return ret;
                }

                // Wait until another thread pushes something (and pray for the implementation to use Linux futexes so it's efficient)
                value_type wait_for_next()
                {
                    if (this->stop_required.load(std::memory_order::acquire))
                        throw StopRequired();
                    this->not_empty.wait(false, std::memory_order::acquire);
                    if (this->stop_required.load(std::memory_order::acquire))
                        throw StopRequired();
                    return this->next();
                }

                void request_stop() noexcept
                {
                    this->stop_required.store(true, std::memory_order::release);
                    this->not_empty.notify_all();
                }
        };

        template <typename Tp>
        static constexpr inline bool is_restricted_ptr_impl = false;

        template <typename Tp>
        static constexpr inline bool is_restricted_ptr_impl<Tp* restrict> = true;

        template <typename Tp>
        static constexpr inline bool is_restricted_ptr_impl<Tp* restrict const> = true;

        template <typename Tp>
        static constexpr inline bool is_restricted_ptr_impl<Tp* restrict volatile> = true;

        template <typename Tp>
        static constexpr inline bool is_restricted_ptr_impl<Tp* restrict const volatile> = true;

        static_assert(is_restricted_ptr_impl<int* restrict>);
        static_assert(is_restricted_ptr_impl<int* restrict const>);
        static_assert(is_restricted_ptr_impl<int* restrict volatile>);
        static_assert(is_restricted_ptr_impl<int* restrict const volatile>);
        static_assert(is_restricted_ptr_impl<const int* restrict volatile const>);
        static_assert(!is_restricted_ptr_impl<int*>);
        static_assert(!is_restricted_ptr_impl<int* const>);
        static_assert(!is_restricted_ptr_impl<int* volatile>);
        static_assert(!is_restricted_ptr_impl<int* const volatile>);

        template <typename Tp>
        concept RestrictedPtr = is_restricted_ptr_impl<Tp>;

#ifdef IS_RESTRICTED_PTR
    #undef IS_RESTRICTED_PTR
#endif
#define IS_RESTRICTED_PTR(ptr) ::SupDef::Util::RestrictedPtr<decltype(ptr)>

#ifdef IS_RESTRICTED_PTRTYPE
    #undef IS_RESTRICTED_PTRTYPE
#endif
#define IS_RESTRICTED_PTRTYPE(type) ::SupDef::Util::RestrictedPtr<std::type_identity_t<type>>

        template <typename Tp>
        static constexpr inline bool is_restricted_ref_impl = false;

        template <typename Tp>
        static constexpr inline bool is_restricted_ref_impl<Tp& restrict> = true;

        static_assert(is_restricted_ref_impl<int& restrict>);
        static_assert(is_restricted_ref_impl<const int& restrict>);
        static_assert(is_restricted_ref_impl<volatile int& restrict>);
        static_assert(is_restricted_ref_impl<const volatile int& restrict>);
        static_assert(!is_restricted_ref_impl<int&>);
        static_assert(!is_restricted_ref_impl<const int&>);
        static_assert(!is_restricted_ref_impl<volatile int&>);
        static_assert(!is_restricted_ref_impl<const volatile int&>);
        static_assert(!is_restricted_ref_impl<int* restrict>);
        static_assert(!is_restricted_ref_impl<int* restrict const>);
        static_assert(!is_restricted_ref_impl<int* restrict volatile>);
        static_assert(!is_restricted_ref_impl<int* restrict const volatile>);


        template <typename Tp>
        concept RestrictedRef = is_restricted_ref_impl<Tp>;

#ifdef IS_RESTRICTED_REF
    #undef IS_RESTRICTED_REF
#endif
#define IS_RESTRICTED_REF(ref) ::SupDef::Util::RestrictedRef<decltype(ref)>

#ifdef IS_RESTRICTED_REFTYPE
    #undef IS_RESTRICTED_REFTYPE
#endif
#define IS_RESTRICTED_REFTYPE(type) ::SupDef::Util::RestrictedRef<std::type_identity_t<type>>

#ifdef IS_RESTRICT
    #undef IS_RESTRICT
#endif
#define IS_RESTRICT(ptr_or_ref) (IS_RESTRICTED_PTR(ptr_or_ref) || IS_RESTRICTED_REF(ptr_or_ref))

#ifdef IS_RESTRICT_TYPE
    #undef IS_RESTRICT_TYPE
#endif
#define IS_RESTRICT_TYPE(type) (IS_RESTRICTED_PTRTYPE(type) || IS_RESTRICTED_REFTYPE(type))

        template <typename From, typename To, bool ExactCopy = false>
        struct CopyStdQualifiersImpl
        {
            private:
                static constexpr inline bool is_from_const    = std::is_const_v<From>;
                static constexpr inline bool is_from_volatile = std::is_volatile_v<From>;

                template <typename Tp>
                using treat_const = std::conditional_t<
                    is_from_const,
                    std::add_const_t<Tp>,
                    std::conditional_t<
                        ExactCopy,
                        std::remove_const_t<Tp>,
                        Tp
                    >
                >;

                template <typename Tp>
                using treat_volatile = std::conditional_t<
                    is_from_volatile,
                    std::add_volatile_t<Tp>,
                    std::conditional_t<
                        ExactCopy,
                        std::remove_volatile_t<Tp>,
                        Tp
                    >
                >;

                template <typename Tp>
                using treat_cv = treat_const<treat_volatile<Tp>>;

            public:
                using type = treat_cv<To>;
        };

        template <typename From, typename To, bool ExactCopy = false>
        using CopyStdQualifiers = typename CopyStdQualifiersImpl<From, To, ExactCopy>::type;

        static_assert(std::same_as<CopyStdQualifiers<int, float>, float>);
        static_assert(std::same_as<CopyStdQualifiers<const int, float>, const float>);
        static_assert(std::same_as<CopyStdQualifiers<int, const float>, const float>);
        static_assert(std::same_as<CopyStdQualifiers<int, const float, true>, float>);
        static_assert(std::same_as<CopyStdQualifiers<const int, const float>, const float>);
        static_assert(std::same_as<CopyStdQualifiers<const int, const volatile float, true>, const float>);
        static_assert(std::same_as<CopyStdQualifiers<const int, const volatile float>, const volatile float>);
        static_assert(std::same_as<CopyStdQualifiers<const int, volatile float>, const volatile float>);
        static_assert(std::same_as<CopyStdQualifiers<const int, volatile float, true>, const float>);

        template <typename Tp>
        struct MakeRestrictImpl
        {
            using type = Tp;
        };

        template <typename Tp>
            requires (std::is_pointer_v<Tp> || std::is_reference_v<Tp>) && (!IS_RESTRICT_TYPE(Tp))
        struct MakeRestrictImpl<Tp>
        {
            using type = Tp restrict;
        };

        template <typename Tp>
        using MakeRestrict = typename MakeRestrictImpl<Tp>::type;

        static_assert(std::same_as<MakeRestrict<int>, int>);

        static_assert(!std::same_as<int*, int* restrict>);
        static_assert(std::same_as<MakeRestrict<int*>, int* restrict>);

        static_assert(!std::same_as<int&, int& restrict>);
        static_assert(std::same_as<MakeRestrict<int&>, int& restrict>);

        static_assert(!std::same_as<const int*, const int* restrict>);
        static_assert(std::same_as<MakeRestrict<const int*>, const int* restrict>);

        static_assert(!std::same_as<const int&, const int& restrict>);
        static_assert(std::same_as<MakeRestrict<const int&>, const int& restrict>);

        static_assert(!std::same_as<volatile int* const, volatile int* const restrict>);
        static_assert(std::same_as<MakeRestrict<volatile int* const>, volatile int* const restrict>);

        static_assert(!std::same_as<const volatile int&, volatile const int& restrict>);
        static_assert(std::same_as<MakeRestrict<volatile const int&>, volatile const int& restrict>);

#ifdef MAKE_RESTRICT
    #undef MAKE_RESTRICT
#endif
#define MAKE_RESTRICT(type) ::SupDef::Util::MakeRestrict<std::type_identity_t<type>>

#ifdef MAKE_RESTRICT_TYPEOF
    #undef MAKE_RESTRICT_TYPEOF
#endif
#define MAKE_RESTRICT_TYPEOF(ptr_or_ref) ::SupDef::Util::MakeRestrict<decltype(ptr_or_ref)>        

#ifdef UNALIASED
    #undef UNALIASED
#endif
#define UNALIASED(type, name) ::SupDef::Util::MakeRestrict<std::type_identity_t<type>> name

#ifdef UNALIASED_TYPEOF
    #undef UNALIASED_TYPEOF
#endif
#define UNALIASED_TYPEOF(ptr_or_ref, name) ::SupDef::Util::MakeRestrict<decltype(ptr_or_ref)> name

STATIC_TODO("Write more tests for `restrict` keyword support")

        template <typename Sig>
        static constexpr Sig* select_overload(Sig* func)
        {
          return func;
        }

        template <typename Sig, typename ClassType>
        static constexpr auto select_overload(Sig (ClassType::*func)) -> decltype(func)
        {
            return func;
        }

        namespace
        {
            template <typename Int>
                requires std::integral<Int>
            constexpr static inline Int saturated_add_fallback(Int a, Int b)
            {
                if (a > 0 && b > std::numeric_limits<Int>::max() - a)
                    return std::numeric_limits<Int>::max();
                else if (a < 0 && b < std::numeric_limits<Int>::min() - a)
                    return std::numeric_limits<Int>::min();
                else
                    return a + b;
            }
        }

        template <typename Int>
            requires std::integral<Int>
        constexpr static inline Int saturated_add(Int a, Int b)
        {
#if SUPDEF_COMPILER == 1 || SUPDEF_COMPILER == 2 // GCC or Clang
            Int ret;
            if (__builtin_add_overflow(a, b, &ret))
                return std::numeric_limits<Int>::max();
            else
                return ret;
#elif SUPDEF_COMPILER == 3 // MSVC
            // Use _add_overflow_i[8|16|32|64] intrinsic when signed
            // Use _addcarry_u[8|16|32|64] intrinsic when unsigned
            using namespace std::string_view_literals;
            if constexpr (std::unsigned_integral<Int>)
            {
                if constexpr (sizeof(Int) == 1)
                {
                    hard_assert_msg(std::same_as<Int, uint8_t> || std::same_as<Int, char>, "Unsupported type "sv + type_name<Int>(true) + " for saturated_add()"sv);
                    Int ret;
                    if (_addcarry_u8(0, a, b, &ret))
                        return std::numeric_limits<Int>::max();
                    else
                        return ret;
                }
                else if constexpr (sizeof(Int) == 2)
                {
                    hard_assert_msg(std::same_as<Int, uint16_t>, "Unsupported type "sv + type_name<Int>(true) + " for saturated_add()"sv);
                    Int ret;
                    if (_addcarry_u16(0, a, b, &ret))
                        return std::numeric_limits<Int>::max();
                    else
                        return ret;
                }
                else if constexpr (sizeof(Int) == 4)
                {
                    hard_assert_msg(std::same_as<Int, uint32_t>, "Unsupported type "sv + type_name<Int>(true) + " for saturated_add()"sv);
                    Int ret;
                    if (_addcarry_u32(0, a, b, &ret))
                        return std::numeric_limits<Int>::max();
                    else
                        return ret;
                }
                else if constexpr (sizeof(Int) == 8)
                {
                    hard_assert_msg(std::same_as<Int, uint64_t>, "Unsupported type "sv + type_name<Int>(true) + " for saturated_add()"sv);
                    Int ret;
                    if (_addcarry_u64(0, a, b, &ret))
                        return std::numeric_limits<Int>::max();
                    else
                        return ret;
                }
                else
                    return saturated_add_fallback(a, b);
            }
#if _MSC_VER >= 1937
            else if constexpr (std::signed_integral<Int>)
            {
                if constexpr (sizeof(Int) == 1)
                {
                    hard_assert_msg(std::same_as<Int, int8_t> || std::same_as<Int, char>, "Unsupported type "sv + type_name<Int>(true) + " for saturated_add()"sv);
                    Int ret;
                    if (_add_overflow_i8(0, a, b, &ret))
                        return std::numeric_limits<Int>::max();
                    else
                        return ret;
                }
                else if constexpr (sizeof(Int) == 2)
                {
                    hard_assert_msg(std::same_as<Int, int16_t>, "Unsupported type "sv + type_name<Int>(true) + " for saturated_add()"sv);
                    Int ret;
                    if (_add_overflow_i16(0, a, b, &ret))
                        return std::numeric_limits<Int>::max();
                    else
                        return ret;
                }
                else if constexpr (sizeof(Int) == 4)
                {
                    hard_assert_msg(std::same_as<Int, int32_t>, "Unsupported type "sv + type_name<Int>(true) + " for saturated_add()"sv);
                    Int ret;
                    if (_add_overflow_i32(0, a, b, &ret))
                        return std::numeric_limits<Int>::max();
                    else
                        return ret;
                }
                else if constexpr (sizeof(Int) == 8)
                {
                    hard_assert_msg(std::same_as<Int, int64_t>, "Unsupported type "sv + type_name<Int>(true) + " for saturated_add()"sv);
                    Int ret;
                    if (_add_overflow_i64(0, a, b, &ret))
                        return std::numeric_limits<Int>::max();
                    else
                        return ret;
                }
                else
                    return saturated_add_fallback(a, b);
            }
#else
            // Perform unsigned addition then check if the sign bit is the same as the two inputs' signs
            else if constexpr (std::signed_integral<Int>)
            {
                return saturated_add_fallback(a, b);
                TODO(
                    "Implement this for real"
                );
            }
#endif
            else
                return saturated_add_fallback(a, b);
#else
            return saturated_add_fallback(a, b);
#endif
            return saturated_add_fallback(a, b);
        }
    }

    // All values here are based on Boost.Contract library failure handlers
    enum class ContractType : uint8_t
    {
        PRECONDITION  = 1 << 0,
        POSTCONDITION = 1 << 1,
        EXCEPT        = 1 << 2,
        INVARIANT     = 1 << 3,
        OLD           = 1 << 4,
        CHECK         = 1 << 5,

        OTHER = 1 << 6
    };

    class ContractViolation : public InternalError
    {
        private:
            ContractType type;            

            [[nodiscard]]
            std::string get_contract_type_str(ContractType type) const noexcept
            {
                using namespace std::string_literals;
                switch (type)
                {
                    case ContractType::PRECONDITION:
                        return "Precondition violation"s;
                    
                    case ContractType::POSTCONDITION:
                        return "Postcondition violation"s;
                    
                    case ContractType::EXCEPT:
                        return "Exception guarantee violation"s;
                    
                    case ContractType::INVARIANT:
                        return "Invariant violation"s;
                    
                    case ContractType::OLD:
                        return "Old value violation"s;
                    
                    case ContractType::CHECK:
                        return "Check violation"s;
                    
                    case ContractType::OTHER:
                        return "Contract violation"s;
                    
                    default:
                        return "Unknown contract violation"s;
                }
            }

        public:
            ContractViolation()
                : InternalError(this->get_contract_type_str(ContractType::OTHER) + " detected.", false, false)
            {
                this->trace = CURRENT_STACKTRACE(1);
                this->init_msg();
            }
            ContractViolation(ContractType type)
                : InternalError(this->get_contract_type_str(type) + " detected.", false, false)
            {
                this->trace = CURRENT_STACKTRACE(1);
                this->init_msg();
            }

            ContractViolation(const ContractViolation&) = default;
            ContractViolation(ContractViolation&&) = default;

            ContractViolation& operator=(const ContractViolation&) = default;
            ContractViolation& operator=(ContractViolation&&) = default;
    };

    STATIC_TODO(
        "Maybe make this an __attribute__((constructor)) function ?"
    );
    static inline void configure_boost_contract(void)
    {
        auto precondition_handler = [](boost::contract::from where)
        {
            if (where == boost::contract::from_destructor)
            {
                decltype(auto) e = SUPDEF_CONTRACT_VIOLATION(PRECONDITION);
                e.report();
                SUPDEF_EXIT();
            }
            else
                throw SUPDEF_CONTRACT_VIOLATION(PRECONDITION);
        };
        
        auto postcondition_handler = [](boost::contract::from where)
        {
            if (where == boost::contract::from_destructor)
            {
                decltype(auto) e = SUPDEF_CONTRACT_VIOLATION(POSTCONDITION);
                e.report();
                SUPDEF_EXIT();
            }
            else
                throw SUPDEF_CONTRACT_VIOLATION(POSTCONDITION);
        };
        
        auto exception_garantee_handler = [](boost::contract::from)
        {
            SUPDEF_CONTRACT_VIOLATION(EXCEPT).report();
            SUPDEF_EXIT();
        };

        auto invariant_handler = [](boost::contract::from where)
        {
            if (where == boost::contract::from_destructor)
            {
                decltype(auto) e = SUPDEF_CONTRACT_VIOLATION(INVARIANT);
                e.report();
                SUPDEF_EXIT();
            }
            else
                throw SUPDEF_CONTRACT_VIOLATION(INVARIANT);
        };

        auto old_value_handler = [](boost::contract::from where)
        {
            if (where == boost::contract::from_destructor)
            {
                decltype(auto) e = SUPDEF_CONTRACT_VIOLATION(OLD);
                e.report();
                SUPDEF_EXIT();
            }
            else
                throw SUPDEF_CONTRACT_VIOLATION(OLD);
        };

        auto check_handler = []()
        {
            throw SUPDEF_CONTRACT_VIOLATION(CHECK);
        };

        std::ignore = boost::contract::set_precondition_failure(precondition_handler);
        std::ignore = boost::contract::set_postcondition_failure(postcondition_handler);
        std::ignore = boost::contract::set_except_failure(exception_garantee_handler);
        std::ignore = boost::contract::set_invariant_failure(invariant_handler);
        std::ignore = boost::contract::set_old_failure(old_value_handler);
        std::ignore = boost::contract::set_check_failure(check_handler);
    }

    using Util::remove_whitespaces;
}

template <class T, class Container>
void std::swap(SupDef::Util::ThreadSafeQueue<T, Container>& lhs, SupDef::Util::ThreadSafeQueue<T, Container>& rhs) noexcept(noexcept(lhs.swap(rhs)))
{
    lhs.swap(rhs);
}

#endif // UTIL_HPP

#include <sup_def/common/end_header.h>