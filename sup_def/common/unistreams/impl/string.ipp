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

#if !UNISTREAMS_STRING_USE_STD_BASIC_STRING
namespace uni
{
    template <
        typename CharT,
        typename Traits,
        typename Allocator
    >
    constexpr string<CharT, Traits, Allocator>::string() noexcept(noexcept(allocator_type()))
        : string<CharT, Traits, Allocator>::string(allocator_type())
    { }

    template <
        typename CharT,
        typename Traits,
        typename Allocator
    >
    constexpr /* explicit */ string<CharT, Traits, Allocator>::string(const allocator_type& alloc) noexcept
        : allocator(alloc), ptr(nullptr), len(0), cap(0)
    { }

    template <
        typename CharT,
        typename Traits,
        typename Allocator
    >
    template <typename>
    constexpr string<CharT, Traits, Allocator>::string(
        size_type count,
        value_type ch,
        const allocator_type& alloc
    ) : allocator(alloc), ptr(nullptr), len(0), cap(0)
    {
        this->reserve(count);
        for (size_type i = 0; i < count; ++i)
        {
            this->push_back(ch);
        }
    }
}
#else
namespace uni
{
    template <
        typename CharT,
        typename Traits,
        typename Allocator
    >
    constexpr bool string<CharT, Traits, Allocator>::verify_endianness() const
        requires detail::has_endianness<value_type, traits_type>
    {
        if (this->size() == 0)
            return true;
        return this->verify_endianness(
            traits_type::get_endianness(this->at(0))
        );
    }

    template <
        typename CharT,
        typename Traits,
        typename Allocator
    >
    constexpr bool string<CharT, Traits, Allocator>::verify_endianness(endianness end) const
        requires detail::has_endianness<value_type, traits_type>
    {
        if (this->size() == 0)
            return true;
        for (size_type i = 0; i < this->size(); ++i)
        {
            if (traits_type::get_endianness(this->at(i)) != end)
                return false;
        }
        return true;
    }

    template <
        typename CharT,
        typename Traits,
        typename Allocator
    >
    constexpr bool string<CharT, Traits, Allocator>::verify_endianness(size_type until) const
        requires detail::has_endianness<value_type, traits_type>
    {
        if (this->size() == 0)
            return true;
        return this->verify_endianness(
            0,
            until
        );
    }

    template <
        typename CharT,
        typename Traits,
        typename Allocator
    >
    constexpr bool string<CharT, Traits, Allocator>::verify_endianness(size_type until, endianness end) const
        requires detail::has_endianness<value_type, traits_type>
    {
        if (this->size() == 0)
            return true;
        return this->verify_endianness(
            0,
            until,
            end
        );
    }

    template <
        typename CharT,
        typename Traits,
        typename Allocator
    >
    constexpr bool string<CharT, Traits, Allocator>::verify_endianness(size_type start, size_type end) const
        requires detail::has_endianness<value_type, traits_type>
    {
        if (this->size() == 0)
            return true;
        for (size_type i = start; i < end; ++i)
        {
            if (traits_type::get_endianness(this->at(i)) != traits_type::get_endianness(this->at(start)))
                return false;
        }
        return true;
    }

    template <
        typename CharT,
        typename Traits,
        typename Allocator
    >
    constexpr bool string<CharT, Traits, Allocator>::verify_endianness(size_type start, size_type end, endianness endian) const
        requires detail::has_endianness<value_type, traits_type>
    {
        if (this->size() == 0)
            return true;
        for (size_type i = start; i < end; ++i)
        {
            if (traits_type::get_endianness(this->at(i)) != endian)
                return false;
        }
        return true;
    }

    template <
        typename CharT,
        typename Traits,
        typename Allocator
    >
    constexpr endianness string<CharT, Traits, Allocator>::get_endianness() const
        requires detail::has_endianness<value_type, traits_type>
    {
        if (this->size() == 0 || !this->verify_endianness())
            return endianness::undefined;
        return traits_type::get_endianness(this->at(0));
    }

    template <
        typename CharT,
        typename Traits,
        typename Allocator
    >
    constexpr void string<CharT, Traits, Allocator>::set_endianness(endianness end)
        requires detail::has_endianness<value_type, traits_type>
    {
        for (size_type i = 0; i < this->size(); ++i)
            traits_type::set_endianness(this->at(i), end);
    }
}
#endif