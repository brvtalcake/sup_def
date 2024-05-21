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

#ifndef TYPES_HPP
#define TYPES_HPP

#include <sup_def/common/util/util.hpp>

namespace SupDef
{
    void set_app_locale(void);
}

namespace uni
{
    using ::SupDef::InternalError;

    namespace detail
    {
        typedef uint_fast32_t unicode_code_point;
        typedef int_fast64_t  unicode_code_point_signed;

        template <auto... B>
            requires std::conjunction<
                std::bool_constant<
                    std::convertible_to<decltype(B), bool>
                >...
            >::value
        using sfinae_require_all_of = typename std::enable_if<
            std::conjunction<
                std::bool_constant<static_cast<bool>(B)>...
            >::value
        >::type;

        template <typename... BC>
        using sfinae_require_all_of_types = typename std::enable_if<
            std::conjunction<BC...>::value
        >::type;

        template <auto... B>
            requires std::disjunction<
                std::bool_constant<
                    std::convertible_to<decltype(B), bool>
                >...
            >::value
        using sfinae_require_any_of = typename std::enable_if<
            std::disjunction<
                std::bool_constant<static_cast<bool>(B)>...
            >::value
        >::type;

        template <typename... BC>
        using sfinae_require_any_of_types = typename std::enable_if<
            std::disjunction<BC...>::value
        >::type;

        template <typename T, size_t Align = alignof(T)>
            requires std::default_initializable<T>
                  && std::is_move_assignable<T>::value
                  && (Align >= alignof(T))
        struct optional
        {
            using value_type = T;

            private:
                alignas(Align) value_type payload;
                bool engaged;

                friend consteval bool test_optional();

                static constexpr bool equality_comparable = std::equality_comparable<value_type>;
                static constexpr bool has_three_way_comparison = std::three_way_comparable<value_type>;

                using three_way_result = std::conditional_t<
                    has_three_way_comparison,
                    std::compare_three_way_result_t<value_type>,
                    std::partial_ordering
                >;
            public:
                constexpr optional() noexcept(std::is_nothrow_default_constructible<value_type>::value) = default;
                constexpr ~optional() noexcept(std::is_nothrow_destructible<value_type>::value) = default;

                template <typename = sfinae_require_all_of<std::copy_constructible<value_type>>>
                constexpr optional(const value_type& v) noexcept(std::is_nothrow_copy_constructible<value_type>::value)
                    : payload(v), engaged(true)
                { }
                template <typename = sfinae_require_all_of<std::move_constructible<value_type>>>
                constexpr optional(value_type&& v) noexcept(std::is_nothrow_move_constructible<value_type>::value)
                    : payload(std::move(v)), engaged(true)
                { }
                constexpr optional(std::nullopt_t) noexcept(std::is_nothrow_default_constructible<value_type>::value)
                    : payload(), engaged(false)
                { }

#if 0
                template <typename = sfinae_require_all_of<std::copy_constructible<value_type>>>
#endif
                constexpr optional(const optional&) noexcept(std::is_nothrow_copy_constructible<value_type>::value)
                    requires std::copy_constructible<value_type> = default;
#if 0
                template <typename = sfinae_require_all_of<std::move_constructible<value_type>>>
#endif
                constexpr optional(optional&&) noexcept(std::is_nothrow_move_constructible<value_type>::value)
                    requires std::move_constructible<value_type> = default;

#if 0
                template <typename = sfinae_require_all_of_types<std::is_copy_assignable<value_type>>>
#endif
                constexpr optional& operator=(const optional&) noexcept(std::is_nothrow_copy_assignable<value_type>::value)
                    requires std::is_copy_assignable<value_type>::value = default;
                constexpr optional& operator=(optional&&) noexcept(std::is_nothrow_move_assignable<value_type>::value) = default;

                template <typename = sfinae_require_all_of_types<std::is_copy_assignable<value_type>>>
                constexpr optional& operator=(const value_type& v) noexcept(std::is_nothrow_copy_assignable<value_type>::value)
                {
                    this->payload = v;
                    this->engaged = true;
                    return *this;
                }
                constexpr optional& operator=(value_type&& v) noexcept(std::is_nothrow_move_assignable<value_type>::value)
                {
                    this->payload = std::move(v);
                    this->engaged = true;
                    return *this;
                }

                constexpr optional& operator=(std::nullopt_t) noexcept(std::is_nothrow_move_assignable<value_type>::value)
                {
                    this->payload = value_type();
                    this->engaged = false;
                    return *this;
                }

                constexpr bool has_value() const noexcept
                {
                    return this->engaged;
                }
                constexpr value_type& value()
                {
                    if (!this->engaged)
                        throw InternalError("optional::value(): value not set");
                    return this->payload;
                }
                constexpr const value_type& value() const
                {
                    if (!this->engaged)
                        throw InternalError("optional::value(): value not set");
                    return this->payload;
                }
                
                constexpr operator bool() const noexcept
                {
                    return this->engaged;
                }

                constexpr value_type& operator*()
                {
                    if (!this->engaged)
                        throw InternalError("optional::operator*(): value not set");
                    return this->payload;
                }
                constexpr const value_type& operator*() const
                {
                    if (!this->engaged)
                        throw InternalError("optional::operator*(): value not set");
                    return this->payload;
                }

                constexpr value_type* operator->()
                {
                    if (!this->engaged)
                        throw InternalError("optional::operator->(): value not set");
                    return std::addressof(this->payload);
                }
                constexpr const value_type* operator->() const
                {
                    if (!this->engaged)
                        throw InternalError("optional::operator->(): value not set");
                    return std::addressof(this->payload);
                }

                template <typename = sfinae_require_any_of<equality_comparable>>
                constexpr std::partial_ordering operator<=>(const value_type& rhs) const noexcept
                {
                    if (!this->engaged)
                        return std::partial_ordering::unordered;
                    if constexpr (has_three_way_comparison)
                        return std::partial_order(this->payload, rhs);
                    else
                        return std::compare_partial_order_fallback(this->payload, rhs);
                }
                template <typename = sfinae_require_any_of<equality_comparable>>
                constexpr std::partial_ordering operator<=>(const optional& rhs) const noexcept
                {
                    if (!this->engaged && !rhs.engaged)
                        return std::partial_ordering::equivalent;
                    if (!this->engaged)
                        return std::partial_ordering::unordered;
                    if (!rhs.engaged)
                        return std::partial_ordering::unordered;
                    if constexpr (has_three_way_comparison)
                        return std::partial_order(this->payload, rhs.payload);
                    else
                        return std::compare_partial_order_fallback(this->payload, rhs.payload);
                }

                constexpr bool operator==(std::nullopt_t) const noexcept
                {
                    return !this->engaged;
                }

                template <typename O>
                    requires std::convertible_to<value_type, O>
                constexpr explicit(!std::is_layout_compatible<value_type, O>::value)
                operator optional<O>() const noexcept(std::is_nothrow_convertible<value_type, O>::value)
                {
                    if (!this->engaged)
                        return optional<O>{std::nullopt};
                    return optional<O>{
                        static_cast<O>(this->payload)
                    };
                }
        };

        consteval bool test_optional()
        {
            return  optional<char8_t>{}.engaged == false    &&
                    optional<char8_t>{}.payload == 0        &&
                    optional<char16_t>{}.engaged == false   &&
                    optional<char16_t>{}.payload == 0       &&
                    optional<char32_t>{}.engaged == false   &&
                    optional<char32_t>{}.payload == 0;
        }

        static_assert(
            std::is_trivially_copyable<optional<char8_t>>::value &&
            std::is_trivial<optional<char8_t>>::value &&
            std::is_standard_layout<optional<char8_t>>::value &&

            std::is_trivially_copyable<optional<char16_t>>::value &&
            std::is_trivial<optional<char16_t>>::value &&
            std::is_standard_layout<optional<char16_t>>::value &&

            std::is_trivially_copyable<optional<char32_t>>::value &&
            std::is_trivial<optional<char32_t>>::value &&
            std::is_standard_layout<optional<char32_t>>::value &&

            std::bool_constant<test_optional()>::value
        );

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

        consteval bool test_tuples()
        {
            tuple<
                char8_t,
                optional<char8_t>,
                optional<char8_t>,
                optional<char8_t>
            > t{0, std::nullopt, std::nullopt, std::nullopt };
            if (t.get<0>() != 0)
                return false;
            if (t.get<1>().has_value())
                return false;
            if (t.get<2>().has_value())
                return false;
            if (t.get<3>().has_value())
                return false;

            t.get<0>() = 1;
            t.get<1>() = 2;
            t.get<2>() = 3;
            t.get<3>() = 4;

            if (t.get<0>() != 1)
                return false;
            if (!t.get<1>().has_value() || t.get<1>().value() != 2)
                return false;
            if (!t.get<2>().has_value() || t.get<2>().value() != 3)
                return false;
            if (!t.get<3>().has_value() || t.get<3>().value() != 4)
                return false;

            t.get<0>() = 0;
            t.get<1>() = std::nullopt;
            t.get<2>() = std::nullopt;
            t.get<3>() = std::nullopt;

            if (t.get<0>() != 0)
                return false;
            if (t.get<1>().has_value())
                return false;
            if (t.get<2>().has_value())
                return false;
            if (t.get<3>().has_value())
                return false;

            return true;
        }

        static_assert(
            std::is_trivially_copyable<tuple<char8_t, char8_t>>::value &&
            std::is_trivial<tuple<char8_t, char8_t>>::value &&
            std::is_standard_layout<tuple<char8_t, char8_t>>::value &&

            std::is_trivially_copyable<tuple<char8_t, char8_t, char8_t, char8_t>>::value &&
            std::is_trivial<tuple<char8_t, char8_t, char8_t, char8_t>>::value &&
            std::is_standard_layout<tuple<char8_t, char8_t, char8_t, char8_t>>::value
        );

        static_assert(
            std::is_trivially_copyable<tuple<char8_t, optional<char8_t>>>::value &&
            std::is_trivial<tuple<char8_t, optional<char8_t>>>::value &&
            std::is_standard_layout<tuple<char8_t, optional<char8_t>>>::value
        );

        static_assert(
            std::is_trivially_copyable<tuple<char8_t, optional<char8_t>, optional<char8_t>, optional<char8_t>>>::value &&
            std::is_trivial<tuple<char8_t, optional<char8_t>, optional<char8_t>, optional<char8_t>>>::value &&
            std::is_standard_layout<tuple<char8_t, optional<char8_t>, optional<char8_t>, optional<char8_t>>>::value &&

            std::bool_constant<test_tuples()>::value
        );

        template <typename T>
        using string_default_allocator = std::allocator<T>;

        template <
            typename CharT,
            typename T,
            typename RealCharT = std::remove_cvref_t<CharT>,
            typename RealT = std::remove_cvref_t<T>
        >
        struct similar_types
        : public std::bool_constant<
            std::same_as<RealCharT, RealT> ||
            (
                ( sizeof(RealCharT) == sizeof(RealT) ) &&
                ( std::unsigned_integral<RealCharT> == std::unsigned_integral<RealT> ) &&
                ( alignof(RealCharT) == alignof(RealT) )
            )
        > { };
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

namespace uni
{
    template <typename CharT>
    struct char_traits;

    template <
        typename CharT,
        typename Traits = ::uni::char_traits<CharT>,
        typename Allocator = detail::string_default_allocator<CharT>
    >
    class string;

    enum class endianness : int_fast8_t
    {
        little = -1,
        big = 1,

        undefined = 0
    };

    using byte = uint8_t;

    struct utf8_char;
    struct utf16_char;
    struct utf32_char;

    using local_char = char;
    using local_wchar = wchar_t;

    template <
        typename CharT,
        typename RealCharT = std::remove_cvref_t<CharT>
    >
    concept uni_char = std::same_as<RealCharT, utf8_char> ||
        std::same_as<RealCharT, utf16_char>               ||
        std::same_as<RealCharT, utf32_char>;


    struct alignas(4) utf8_char
    {
        public:
            using code_unit = char8_t;

            template <typename CU>
            using value_type = detail::tuple<
                CU,
                detail::optional<CU>,
                detail::optional<CU>,
                detail::optional<CU>
            >;

            constexpr utf8_char() = default;

            constexpr explicit utf8_char(
                code_unit b0,
                std::optional<code_unit> b1 = std::nullopt,
                std::optional<code_unit> b2 = std::nullopt,
                std::optional<code_unit> b3 = std::nullopt
            ) noexcept
                : value(b0, b1, b2, b3)
            { }

            constexpr utf8_char(const utf8_char&) = default;
            constexpr utf8_char(utf8_char&&) = default;

            constexpr utf8_char& operator=(const utf8_char&) = default;
            constexpr utf8_char& operator=(utf8_char&&) = default;

            constexpr utf8_char& operator=(std::initializer_list<code_unit> il) noexcept
            {
                switch (il.size())
                {
                    case 4:
                        this->value = value_type<code_unit>(
                            il.begin()[0],
                            il.begin()[1],
                            il.begin()[2],
                            il.begin()[3]
                        );
                        break;
                    case 3:
                        this->value = value_type<code_unit>(
                            il.begin()[0],
                            il.begin()[1],
                            il.begin()[2],
                            std::nullopt
                        );
                        break;
                    case 2:
                        this->value = value_type<code_unit>(
                            il.begin()[0],
                            il.begin()[1],
                            std::nullopt,
                            std::nullopt
                        );
                        break;
                    case 1:
                        this->value = value_type<code_unit>(
                            il.begin()[0],
                            std::nullopt,
                            std::nullopt,
                            std::nullopt
                        );
                        break;
                    default:
                        this->value = value_type<code_unit>(
                            0,
                            std::nullopt,
                            std::nullopt,
                            std::nullopt
                        );
                        break;
                }
                return *this;
            }

            template <size_t I>
            constexpr auto get() const noexcept
            {
                return this->value.template get<I>();
            }

        private:
            optimize_space
            value_type<code_unit> value;
    };
    static_assert(
        ::uni::detail::similar_types<
            char8_t,
            uint_least8_t
        >::value &&
        std::unsigned_integral<char8_t> &&
        sizeof(char8_t) == 1 &&

        std::is_trivial<utf8_char>::value &&
        std::is_standard_layout<utf8_char>::value &&
        
        utf8_char{}.get<0>() == 0 &&
        utf8_char{}.get<1>() == std::nullopt &&
        utf8_char{}.get<2>() == std::nullopt &&
        utf8_char{}.get<3>() == std::nullopt &&

        std::integral_constant<size_t, alignof(utf8_char)>::value == 4 &&
        std::integral_constant<size_t, sizeof(utf8_char) >::value == 8        
    );
    
    struct alignas(4) utf16_char
    {
        public:
            using code_unit = char16_t;
        
        private:
            using surrogate_pair = detail::tuple<code_unit, detail::optional<code_unit>>;

        public:
            template <typename CU>
            using value_type = detail::tuple<CU, detail::optional<CU>>;

            constexpr utf16_char() = default;

            constexpr explicit utf16_char(code_unit c, endianness e = endianness::undefined) noexcept
                : value(c, std::nullopt), endian(e)
            { }
            constexpr explicit utf16_char(code_unit c1, code_unit c2, endianness e = endianness::undefined) noexcept
                : value(c1, c2), endian(e)
            { }

            constexpr utf16_char(const utf16_char&) = default;
            constexpr utf16_char(utf16_char&&) = default;

            constexpr utf16_char& operator=(const utf16_char&) = default;
            constexpr utf16_char& operator=(utf16_char&&) = default;

            constexpr utf16_char& operator=(std::initializer_list<code_unit> il)
            {
                if (this->endian == endianness::undefined)
                    throw InternalError("utf16_char::operator=(): endianness not set. Please assign a new utf16_char with the endianness set.");
                switch (il.size())
                {
                    case 2:
                        this->value = value_type<code_unit>(
                            il.begin()[0],
                            il.begin()[1]
                        );
                        break;
                    case 1:
                        this->value = value_type<code_unit>(
                            il.begin()[0],
                            std::nullopt
                        );
                        break;
                    default:
                        this->value = value_type<code_unit>(
                            0,
                            std::nullopt
                        );
                        break;
                }
                return *this;
            }

            template <size_t I>
            constexpr auto get() const noexcept
            {
                return this->value.template get<I>();
            }
            constexpr endianness get_endianness() const noexcept
            {
                return this->endian;
            }
            constexpr void set_endianness(endianness e)
            {
                if (this->endian == e)
                    return;
                if (this->endian == endianness::undefined)
                    throw InternalError("utf16_char::set_endianness(): endianness not set. Please assign a new utf16_char with the endianness set.");

                this->endian = e;

                if (e == endianness::undefined)
                    this->value = value_type<code_unit>(
                        0,
                        std::nullopt
                    );
                else
                {
                    std::get<0>(this->value) = std::byteswap(std::get<0>(this->value));
                    if (std::get<1>(this->value).has_value())
                        std::get<1>(this->value) = std::byteswap(std::get<1>(this->value).value());
                }
            }
            constexpr void swap_endianness()
            {
                if (this->endian == endianness::undefined)
                    throw InternalError("utf16_char::swap_endianness(): endianness not set. Please assign a new utf16_char with the endianness set.");
                this->set_endianness(
                    (this->endian == endianness::little) ? endianness::big : endianness::little
                );
            }
        
        private:
            optimize_space
            value_type<code_unit> value;
            optimize_space
            endianness endian;
    };
    static_assert(
        ::uni::detail::similar_types<
            char16_t,
            uint_least16_t
        >::value &&
        std::unsigned_integral<char16_t> &&
        sizeof(char16_t) == 2 &&

        std::is_trivial<utf16_char>::value &&
        std::is_standard_layout<utf16_char>::value &&

        utf16_char{}.get<0>() == 0 &&
        utf16_char{}.get<1>() == std::nullopt &&
        utf16_char{}.get_endianness() == endianness::undefined &&

        std::integral_constant<size_t, alignof(utf16_char)>::value == 4 &&
        std::integral_constant<size_t, sizeof(utf16_char) >::value == 8
    );

    struct alignas(4) utf32_char
    {
        public:
            using code_unit = char32_t;

            template <typename CU>
            using value_type = CU;

            constexpr utf32_char() = default;

            constexpr explicit utf32_char(code_unit c, endianness e = endianness::undefined) noexcept
                : value(c), endian(e)
            { }

            constexpr utf32_char(const utf32_char&) = default;
            constexpr utf32_char(utf32_char&&) = default;

            constexpr utf32_char& operator=(const utf32_char&) = default;
            constexpr utf32_char& operator=(utf32_char&&) = default;

            constexpr utf32_char& operator=(std::initializer_list<code_unit> il)
            {
                if (il.size() != 1)
                    throw InternalError("utf32_char::operator=(): initializer_list must contain exactly one element.");
                this->value = *il.begin();
                return *this;
            }

            constexpr code_unit get() const noexcept
            {
                return this->value;
            }
            constexpr endianness get_endianness() const noexcept
            {
                return this->endian;
            }
            constexpr void set_endianness(endianness e)
            {
                if (this->endian == e)
                    return;
                if (this->endian == endianness::undefined)
                    throw InternalError("utf32_char::set_endianness(): endianness not set. Please assign a new utf32_char with the endianness set.");

                this->endian = e;
                
                if (e == endianness::undefined)
                    this->value = 0;
                else
                    this->value = std::byteswap(this->value);
            }
            constexpr void swap_endianness()
            {
                if (this->endian == endianness::undefined)
                    throw InternalError("utf32_char::swap_endianness(): endianness not set. Please assign a new utf32_char with the endianness set.");
                this->set_endianness(
                    (this->endian == endianness::little) ? endianness::big : endianness::little
                );
            }

        private:
            optimize_space
            value_type<code_unit> value;
            optimize_space
            endianness endian;
    };
    static_assert(
        ::uni::detail::similar_types<
            char32_t,
            uint_least32_t
        >::value &&
        std::unsigned_integral<char32_t> &&
        sizeof(char32_t) == 4 &&
        
        std::is_trivial<utf32_char>::value &&
        std::is_standard_layout<utf32_char>::value &&

        utf32_char{}.get() == 0 &&
        utf32_char{}.get_endianness() == endianness::undefined &&

        std::integral_constant<size_t, alignof(utf32_char)>::value == 4 &&
        std::integral_constant<size_t, sizeof(utf32_char) >::value == 8
    );

    typedef ::uni::string<local_char, ::uni::char_traits<local_char>> char_string;
    typedef ::uni::string<local_wchar, ::uni::char_traits<local_wchar>> wchar_string;

    typedef ::uni::string<utf8_char, ::uni::char_traits<utf8_char>> utf8_string;
    typedef ::uni::string<utf16_char, ::uni::char_traits<utf16_char>> utf16_string;
    typedef ::uni::string<utf32_char, ::uni::char_traits<utf32_char>> utf32_string;

    template <typename Allocator = detail::string_default_allocator<local_char>>
    using char_string_ex = ::uni::string<local_char, ::uni::char_traits<local_char>, Allocator>;
    template <typename Allocator = detail::string_default_allocator<local_wchar>>
    using wchar_string_ex = ::uni::string<local_wchar, ::uni::char_traits<local_wchar>, Allocator>;

    template <typename Allocator = detail::string_default_allocator<utf8_char>>
    using utf8_string_ex = ::uni::string<utf8_char, ::uni::char_traits<utf8_char>, Allocator>;
    template <typename Allocator = detail::string_default_allocator<utf16_char>>
    using utf16_string_ex = ::uni::string<utf16_char, ::uni::char_traits<utf16_char>, Allocator>;
    template <typename Allocator = detail::string_default_allocator<utf32_char>>
    using utf32_string_ex = ::uni::string<utf32_char, ::uni::char_traits<utf32_char>, Allocator>;

    namespace detail
    {
        template <typename...>
        struct uni_string_traits_helper;

        template <
            typename CharT,
            typename Traits,
            typename Allocator
        >
        struct uni_string_traits_helper<
            ::uni::string<CharT, Traits, Allocator>
        >
        {
            typedef ::uni::string<CharT, Traits, Allocator> string_type;

            typedef CharT char_type;
            typedef typename string_type::value_type real_char_type;
            
            typedef Traits traits_type;
            typedef typename string_type::traits_type real_traits_type;
            
            typedef Allocator allocator_type;
            typedef typename string_type::allocator_type real_allocator_type;
        };

        // RealStrT is a utfX_string, but Allocator differs
        template <
            typename StrT,
            typename RealStrT = std::remove_cvref_t<StrT>
        >
        struct uni_string_allocdiff
        : public std::bool_constant<
            ::uni::uni_char<
                typename uni_string_traits_helper<RealStrT>::char_type
            > &&
            std::same_as<
                typename uni_string_traits_helper<RealStrT>::traits_type,
                ::uni::char_traits<typename uni_string_traits_helper<RealStrT>::char_type>
            > &&
            !std::same_as<
                typename uni_string_traits_helper<RealStrT>::real_allocator_type,
                typename uni_string_traits_helper<
                    ::uni::string<
                        typename uni_string_traits_helper<RealStrT>::char_type,
                        typename uni_string_traits_helper<RealStrT>::traits_type
                    >
                >::real_allocator_type
            >
        > { };
    }

    template <
        typename StrT,
        typename RealStrT = std::remove_cvref_t<StrT>
    >
    concept uni_string = std::same_as<RealStrT, utf8_string>    ||
        std::same_as<RealStrT, utf16_string>                    ||
        std::same_as<RealStrT, utf32_string>                    ||
        detail::uni_string_allocdiff<RealStrT>::value;

    template <
        typename StrT,
        typename RealStrT = std::remove_cvref_t<StrT>
    >
    concept supported_uni_string = uni_string<RealStrT> ||
        (
            SPECIALIZATION_OF( RealStrT, ::uni::string ) &&
            ::SupDef::IsOneOf<
                typename uni::detail::uni_string_traits_helper<RealStrT>::char_type,
                char, wchar_t
            > &&
            ::SupDef::IsOneOf<
                typename uni::detail::uni_string_traits_helper<RealStrT>::traits_type,
                ::uni::char_traits<
                    typename uni::detail::uni_string_traits_helper<RealStrT>::char_type
                >
            >
        );

    template <
        typename CharT,
        typename RealCharT = std::remove_cvref_t<CharT>
    >
    concept supported_uni_char = uni_char<RealCharT> ||
        std::same_as<RealCharT, char>                ||
        std::same_as<RealCharT, wchar_t>;
          

    namespace detail
    {
        template <
            typename CharT,
            typename Traits = ::uni::char_traits<CharT>
        >
        concept has_endianness = requires(const CharT& c1, CharT& c2, endianness e)
        {
            { c1.get_endianness() } -> std::same_as<endianness>;
            { c2.get_endianness() } -> std::same_as<endianness>;
            { c2.set_endianness(e) } -> std::same_as<void>;

            { Traits::get_endianness(c1) } -> std::same_as<endianness>;
            { Traits::get_endianness(c2) } -> std::same_as<endianness>;
            { Traits::set_endianness(c2, e) } -> std::same_as<void>;
        };

        template <
            ::uni::supported_uni_char CharTFrom,
            ::uni::supported_uni_char CharTTo
        >
        struct string_conversions;

        template <typename Allocator, typename CharTTo>
        concept can_rebind_alloc = requires {
            typename std::allocator_traits<Allocator>::template rebind_alloc<CharTTo>;
        };
        

        template <typename...>
        struct bind_string_impl;

        template <typename...>
        struct bind_string_ex_impl;

        template <
            typename CharTFrom,
            typename AllocatorFrom,
            typename CharTTo
        > requires !can_rebind_alloc<AllocatorFrom, CharTTo>
        struct bind_string_impl<
            ::uni::string<CharTFrom, ::uni::char_traits<CharTFrom>, AllocatorFrom>,
            CharTTo
        >
        {
            typedef ::uni::string<CharTTo, ::uni::char_traits<CharTTo>, AllocatorFrom> type;
        };

        template <
            typename CharTFrom,
            typename AllocatorFrom,
            typename CharTTo
        > requires can_rebind_alloc<AllocatorFrom, CharTTo>
        struct bind_string_impl<
            ::uni::string<CharTFrom, ::uni::char_traits<CharTFrom>, AllocatorFrom>,
            CharTTo
        >
        {
            typedef ::uni::string<CharTTo, ::uni::char_traits<CharTTo>, typename std::allocator_traits<AllocatorFrom>::template rebind_alloc<CharTTo>> type;
        };

        template <
            typename CharTFrom,
            typename AllocatorFrom,
            typename CharTTo,
            typename AllocatorTo
        >
        struct bind_string_ex_impl<
            ::uni::string<CharTFrom, ::uni::char_traits<CharTFrom>, AllocatorFrom>,
            CharTTo,
            AllocatorTo
        >
        {
            typedef ::uni::string<CharTTo, ::uni::char_traits<CharTTo>, AllocatorTo> type;
        };

        template <typename StrT, typename CharT>
        using bind_string = typename bind_string_impl<StrT, CharT>::type;

        // If possible, default to the same as bind_string if Allocator is not specified
        template <
            typename StrT,
            typename CharT,
            typename Allocator = std::enable_if_t<
                can_rebind_alloc<
                    typename uni::detail::uni_string_traits_helper<StrT>::allocator_type,
                    CharT
                >,
                typename std::allocator_traits<
                    typename uni::detail::uni_string_traits_helper<StrT>::allocator_type
                >::template rebind_alloc<CharT>
            >
        >                
        using bind_string_ex = typename bind_string_ex_impl<StrT, CharT, Allocator>::type;
    }
}

#define INCLUDED_FROM_UNISTREAMS_SOURCE 1

#include <sup_def/common/unistreams/impl/char_traits.ipp>
#include <sup_def/common/unistreams/impl/string_conversions.ipp>

#undef INCLUDED_FROM_UNISTREAMS_SOURCE

#endif
