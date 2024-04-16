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

#ifdef TESTFILE_NAME
    #undef TESTFILE_NAME
#endif
#define TESTFILE_NAME supdef/tests/common/unistreams_std_basic_string.ipp

#if !BOOST_TEST_ALREADY_INCLUDED
    #undef BOOST_TEST_MODULE
    #define BOOST_TEST_MODULE unistreams_std_basic_string_tests
    #include <boost/test/included/unit_test.hpp>
#endif

#include <sup_def/common/sup_def.hpp>
#include <sup_def/tests/tests.h>

#line SUPDEF_TEST_FILE_POS

BOOST_AUTO_TEST_SUITE(unistreams_std_basic_string,
    * BoostTest::description("Tests for `std::basic_string<::uni::*unistream char types*>`")
)

BOOST_AUTO_TEST_CASE(test_construction_destruction,
    * BoostTest::description("Tests for the construction and destruction of `std::basic_string<::uni::*unistream char types*>`")
    * BoostTest::timeout(SUPDEF_TEST_DEFAULT_TIMEOUT)
)
{
    using namespace uni::literals;

    using u8_string = std::basic_string<uni::utf8_char, uni::char_traits<uni::utf8_char>>;
    using u16_string = std::basic_string<uni::utf16_char, uni::char_traits<uni::utf16_char>>;
    using u32_string = std::basic_string<uni::utf32_char, uni::char_traits<uni::utf32_char>>;

    u8_string utf8;
    u16_string utf16;
    u32_string utf32;

    BOOST_TEST(utf8.empty());
    BOOST_TEST(utf16.empty());
    BOOST_TEST(utf32.empty());
}

BOOST_AUTO_TEST_CASE(test_push_back,
    * BoostTest::description("Tests for the `push_back` method of `std::basic_string<::uni::*unistream char types*>`")
    * BoostTest::timeout(SUPDEF_TEST_DEFAULT_TIMEOUT)
)
{
    using namespace uni::literals;

    using utf8_traits = uni::char_traits<uni::utf8_char>;
    using utf16_traits = uni::char_traits<uni::utf16_char>;
    using utf32_traits = uni::char_traits<uni::utf32_char>;

    using u8_string = std::basic_string<uni::utf8_char, utf8_traits>;
    using u16_string = std::basic_string<uni::utf16_char, utf16_traits>;
    using u32_string = std::basic_string<uni::utf32_char, utf32_traits>;


    u8_string utf8;
    u16_string utf16;
    u32_string utf32;

    utf8.push_back(u8'a'uni);
    utf16.push_back(u'a'uni);
    utf32.push_back(U'a'uni);

    BOOST_TEST(utf8_traits::length(utf8.data()) == 1);
    BOOST_TEST(utf16_traits::length(utf16.data()) == 1);
    BOOST_TEST(utf32_traits::length(utf32.data()) == 1);

    BOOST_TEST(utf8_traits::eq(utf8.c_str()[0], { u8'a' }));
    BOOST_TEST(utf16_traits::eq(utf16.c_str()[0], { u'a', uni::detail::get_literal_endianness<char16_t>() }));
    BOOST_TEST(utf32_traits::eq(utf32.c_str()[0], { U'a', uni::detail::get_literal_endianness<char32_t>() }));

    BOOST_TEST(utf8_traits::is_null(utf8.c_str()[1]));
    BOOST_TEST(utf16_traits::is_null(utf16.c_str()[1]));
    BOOST_TEST(utf32_traits::is_null(utf32.c_str()[1]));
}

BOOST_AUTO_TEST_SUITE_END()