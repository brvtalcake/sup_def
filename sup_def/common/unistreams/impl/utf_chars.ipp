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

            constexpr explicit utf16_char(endianness e) noexcept
                : value(0, std::nullopt), endian(e)
            { }
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

    struct alignas(4) utf32_char
    {
        public:
            using code_unit = char32_t;

            template <typename CU>
            using value_type = CU;

            constexpr utf32_char() = default;

            constexpr explicit utf32_char(endianness e) noexcept
                : value(0), endian(e)
            { }
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
#include <sup_def/common/unistreams/static_test/utf_chars.ipp>
}