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