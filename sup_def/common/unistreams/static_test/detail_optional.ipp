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