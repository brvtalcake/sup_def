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