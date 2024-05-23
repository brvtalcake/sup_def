/* 
 * MIT License
 * 
 * Copyright (c) 2023-2024 Axel PASCON
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

#if !INCLUDED_FROM_UNISTREAMS_SOURCE
    #error "This file shall not be included from outside the unistreams library"
#endif

namespace uni
{
    namespace detail
    {

#if 1
        template <typename...>
        struct variadic_container;

        template <typename T, typename... Ts>
        struct variadic_container<T, Ts...>
        {
            private:
                optimize_space
                alignas(T) T head;

                optimize_space
                variadic_container<Ts...> tail;

                template <typename...>
                friend struct variadic_container;

                template <typename...>
                friend struct tuple;

            public:
                constexpr variadic_container() = default;
                
                template <typename U, typename... Us>
                    requires (sizeof...(Us) > 0)
                constexpr variadic_container(U&& h, Us&&... t)
                    : head(std::forward<U>(h)), tail(std::forward<Us>(t)...)
                { }
                template <typename U>
                constexpr variadic_container(U&& h)
                    : head(std::forward<U>(h)), tail()
                { }

                constexpr variadic_container(const variadic_container&) = default;
                constexpr variadic_container(variadic_container&&) = default;

                constexpr variadic_container& operator=(const variadic_container&) = default;
                constexpr variadic_container& operator=(variadic_container&&) = default;

                template <size_t I>
                constexpr auto& get() noexcept
                {
                    if constexpr (I == 0)
                        return head;
                    else
                        return tail.template get<I - 1>();
                }
                template <size_t I>
                constexpr const auto& get() const noexcept
                {
                    if constexpr (I == 0)
                        return head;
                    else
                        return tail.template get<I - 1>();
                }
        };

        template <typename T>
        struct variadic_container<T>
        {
            private:
                optimize_space
                alignas(T) T head;

                template <typename...>
                friend struct variadic_container;

                template <typename...>
                friend struct tuple;

            public:
                constexpr variadic_container() = default;

                template <typename U>
                constexpr variadic_container(U&& h)
                    : head(std::forward<U>(h))
                { }

                constexpr variadic_container(const variadic_container&) = default;
                constexpr variadic_container(variadic_container&&) = default;

                constexpr variadic_container& operator=(const variadic_container&) = default;
                constexpr variadic_container& operator=(variadic_container&&) = default;

                template <size_t I>
                constexpr auto& get() noexcept
                {
                    return head;
                }
                template <size_t I>
                constexpr const auto& get() const noexcept
                {
                    return head;
                }
        };

        template <>
        struct variadic_container<>
        {
            template <typename...>
            friend struct variadic_container;

            template <typename...>
            friend struct tuple;

            constexpr variadic_container() = default;
        };

        template <typename... Ts>
        struct tuple
        {
            private:
                optimize_space
                variadic_container<Ts...> values;

                template <typename...>
                friend struct variadic_container;

                template <typename...>
                friend struct tuple;

            public:
                constexpr tuple() = default;

                template <typename... Us>
                constexpr tuple(Us&&... t)
                    : values(std::forward<Us>(t)...)
                { }

                constexpr tuple(const tuple&) = default;
                constexpr tuple(tuple&&) = default;

                constexpr tuple& operator=(const tuple&) = default;
                constexpr tuple& operator=(tuple&&) = default;

                template <size_t I>
                    requires (I < sizeof...(Ts))
                constexpr auto& get() noexcept
                {
                    if constexpr (I == 0)
                        return values.head;
                    else
                        return values.tail.template get<I - 1>();
                }
                template <size_t I>
                    requires (I < sizeof...(Ts))
                constexpr const auto& get() const noexcept
                {
                    if constexpr (I == 0)
                        return values.head;
                    else
                        return values.tail.template get<I - 1>();
                }
        };

        template <typename... Ts>
        tuple(Ts&&...) -> tuple<Ts...>;

        static_assert(
            std::is_trivially_copyable<variadic_container<>>::value &&
            std::is_trivial<variadic_container<>>::value &&
            std::is_standard_layout<variadic_container<>>::value &&

            std::is_trivially_copyable<variadic_container<char8_t>>::value &&
            std::is_trivial<variadic_container<char8_t>>::value &&
            std::is_standard_layout<variadic_container<char8_t>>::value &&

            std::is_trivially_copyable<variadic_container<char8_t, char8_t>>::value &&
            std::is_trivial<variadic_container<char8_t, char8_t>>::value &&
            std::is_standard_layout<variadic_container<char8_t, char8_t>>::value &&

            std::is_trivially_copyable<variadic_container<char8_t, optional<char8_t>>>::value &&
            std::is_trivial<variadic_container<char8_t, optional<char8_t>>>::value &&
            std::is_standard_layout<variadic_container<char8_t, optional<char8_t>>>::value
        );
#else

        template <typename...>
        struct tuple;

        template <typename T0, typename T1>
        struct tuple<T0, T1>
        {
            private:
                T0 first;
                T1 second;

            public:
                constexpr tuple() = default;
                constexpr ~tuple() = default;

                constexpr tuple(const T0& f, const T1& s)
                    : first(f), second(s)
                { }
                constexpr tuple(T0&& f, T1&& s)
                    : first(std::move(f)), second(std::move(s))
                { }

                constexpr tuple(const tuple&) = default;
                constexpr tuple(tuple&&) = default;

                constexpr tuple& operator=(const tuple&) = default;
                constexpr tuple& operator=(tuple&&) = default;

                template <size_t I>
                    requires (I <= 1)
                constexpr auto& get() noexcept
                {
                    if constexpr (I == 0)
                        return first;
                    else
                        return second;
                }
                template <size_t I>
                    requires (I <= 1)
                constexpr const auto& get() const noexcept
                {
                    if constexpr (I == 0)
                        return first;
                    else
                        return second;
                }
        };

        template <typename T0, typename T1, typename T2, typename T3>
        struct tuple<T0, T1, T2, T3>
        {
            private:
                T0 first;
                T1 second;
                T2 third;
                T3 fourth;

            public:
                constexpr tuple() = default;
                constexpr ~tuple() = default;

                constexpr tuple(const T0& f, const T1& s, const T2& t, const T3& fo)
                    : first(f), second(s), third(t), fourth(fo)
                { }
                constexpr tuple(T0&& f, T1&& s, T2&& t, T3&& fo)
                    : first(std::move(f)), second(std::move(s)), third(std::move(t)), fourth(std::move(fo))
                { }

                constexpr tuple(const tuple&) = default;
                constexpr tuple(tuple&&) = default;

                constexpr tuple& operator=(const tuple&) = default;
                constexpr tuple& operator=(tuple&&) = default;

                template <size_t I>
                    requires (I <= 3)
                constexpr auto& get() noexcept
                {
                    if constexpr (I == 0)
                        return first;
                    else if constexpr (I == 1)
                        return second;
                    else if constexpr (I == 2)
                        return third;
                    else
                        return fourth;
                }
                template <size_t I>
                    requires (I <= 3)
                constexpr const auto& get() const noexcept
                {
                    if constexpr (I == 0)
                        return first;
                    else if constexpr (I == 1)
                        return second;
                    else if constexpr (I == 2)
                        return third;
                    else
                        return fourth;
                }
        };
#endif

#include <sup_def/common/unistreams/static_test/detail_tuple.ipp>
    }
}

template <size_t I, typename... Ts>
constexpr auto& std::get(::uni::detail::tuple<Ts...>& t) noexcept(noexcept(t.template get<I>()))
{
    return t.template get<I>();
}
template <size_t I, typename... Ts>
constexpr const auto& std::get(const ::uni::detail::tuple<Ts...>& t) noexcept(noexcept(t.template get<I>()))
{
    return t.template get<I>();
}

template <typename... Ts>
struct std::tuple_size<::uni::detail::tuple<Ts...>> : std::integral_constant<size_t, sizeof...(Ts)> { };
template <typename... Ts>
struct std::tuple_size<const ::uni::detail::tuple<Ts...>> : std::integral_constant<size_t, sizeof...(Ts)> { };

template <size_t I, typename... Ts>
struct std::tuple_element<I, ::uni::detail::tuple<Ts...>>
{
    using type = std::tuple_element_t<I, std::tuple<Ts...>>;
};
template <size_t I, typename... Ts>
struct std::tuple_element<I, const ::uni::detail::tuple<Ts...>>
{
    using type = std::tuple_element_t<I, std::tuple<Ts...>>;
};