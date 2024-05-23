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

#include <sup_def/common/unistreams/static_test/detail_optional.ipp>
    }
}