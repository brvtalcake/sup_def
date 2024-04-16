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
#define TESTFILE_NAME supdef/tests/common/convert_macro.ipp

#if !BOOST_TEST_ALREADY_INCLUDED
    #undef BOOST_TEST_MODULE
    #define BOOST_TEST_MODULE convert_macro_tests
    #include <boost/test/included/unit_test.hpp>
#endif

#include <sup_def/common/sup_def.hpp>
#include <sup_def/tests/tests.h>

#line SUPDEF_TEST_FILE_POS

BOOST_AUTO_TEST_SUITE(convert_macro,
    * BoostTest::description("Tests for `CONVERT` macro")
)

BOOST_AUTO_TEST_CASE(test_utf8_utf16,
    * BoostTest::description("Tests for the conversion from UTF-8 to UTF-16 and vice versa")
    * BoostTest::timeout(SUPDEF_TEST_DEFAULT_TIMEOUT)
    * BoostTest::disabled()
)
{
    using namespace std::string_literals;
    using ::SupDef::Util::convert;

    char8_t u8_buf[] = u8"😍";
    char16_t u16_buf[] = u"😍";
    
    constexpr size_t u8_buf_elemcount = sizeof(u8_buf) / sizeof(char8_t);
    constexpr size_t u16_buf_elemcount = sizeof(u16_buf) / sizeof(char16_t);
    
    static_assert(u8_buf_elemcount == 5);
    static_assert(u16_buf_elemcount == 3);
    static_assert((u16_buf_elemcount - 1) % 2 == 0);
    
    uint8_t expected_utf8_from[] = { 0xF0, 0x9F, 0x98, 0x8D };
    uint8_t expected1_utf16_from[] = { 0xD8, 0x3D, 0xDE, 0x0D }; // Assuming "classic" big endian
    uint16_t expected2_utf16_from[] = { 0xD83D, 0xDE0D };
    
    std::u8string utf8_from(u8_buf);
    std::u16string utf16_from(u16_buf);

    for (size_t i = 0; i < u8_buf_elemcount - 1; ++i)
    {
        using traits_type = std::char_traits<char8_t>;

        BOOST_TEST(
            traits_type::eq(
                u8_buf[i],
                utf8_from.at(i)
            )
        );

        BOOST_TEST((
            static_cast<uint8_t>(u8_buf[i]) ==
            expected_utf8_from[i]
        ));
        BOOST_TEST((
            static_cast<uint8_t>(utf8_from.at(i)) ==
            expected_utf8_from[i]
        ));
    }

    for (size_t i = 0; i < u16_buf_elemcount - 1; ++i)
    {
        using traits_type = std::char_traits<char16_t>;

        BOOST_TEST(
            traits_type::eq(
                u16_buf[i],
                utf16_from.at(i)
            )
        );
        
        BOOST_TEST((
            (static_cast<uint16_t>(u16_buf[i]) | 0xFF00) ==
            expected1_utf16_from[2 * i]
        ));
        BOOST_TEST((
            (static_cast<uint16_t>(u16_buf[i]) | 0x00FF) ==
            expected1_utf16_from[(2 * i) + 1]
        ));
        BOOST_TEST((
            (static_cast<uint16_t>(utf16_from.at(i)) | 0xFF00) ==
            expected1_utf16_from[2 * i]
        ));
        BOOST_TEST((
            (static_cast<uint16_t>(utf16_from.at(i + 1)) | 0x00FF) ==
            expected1_utf16_from[(2 * i) + 1]
        ));

        BOOST_TEST((
            static_cast<uint16_t>(u16_buf[i]) ==
            expected2_utf16_from[i]
        ));
        BOOST_TEST((
            static_cast<uint16_t>(utf16_from.at(i)) ==
            expected2_utf16_from[i]
        ));
    }

    auto utf8 = convert<char8_t>(utf16_from);
    auto utf16 = convert<char16_t>(utf8_from);

    BOOST_TEST(( std::same_as<decltype(utf8), std::u8string> ));
    BOOST_TEST(( std::same_as<decltype(utf16), std::u16string> ));

    BOOST_TEST(utf8.size() == utf8_from.size());
    BOOST_TEST(utf16.size() == utf16_from.size());

    for (size_t i = 0; i < u8_buf_elemcount - 1; ++i)
    {
        using traits_type = std::char_traits<char8_t>;

        BOOST_TEST(
            traits_type::eq(
                u8_buf[i],
                utf8.at(i)
            )
        );

        BOOST_TEST((
            static_cast<uint8_t>(utf8.at(i)) ==
            expected_utf8_from[i]
        ));
    }

    for (size_t i = 0; i < u16_buf_elemcount - 1; ++i)
    {
        using traits_type = std::char_traits<char16_t>;

        BOOST_TEST(
            traits_type::eq(
                u16_buf[i],
                utf16.at(i)
            )
        );

        BOOST_TEST((
            (static_cast<uint16_t>(utf16.at(i)) | 0xFF00) ==
            expected1_utf16_from[2 * i]
        ));
        BOOST_TEST((
            (static_cast<uint16_t>(utf16.at(i + 1)) | 0x00FF) ==
            expected1_utf16_from[(2 * i) + 1]
        ));

        BOOST_TEST((
            static_cast<uint16_t>(utf16.at(i)) ==
            expected2_utf16_from[i]
        ));
    }

    BOOST_TEST(( utf8 == utf8_from ));
    BOOST_TEST(( utf16 == utf16_from ));

    BOOST_TEST(( utf8 == convert<char8_t>(utf16) ));
    BOOST_TEST(( utf16 == convert<char16_t>(utf8) ));
}

BOOST_AUTO_TEST_SUITE_END()