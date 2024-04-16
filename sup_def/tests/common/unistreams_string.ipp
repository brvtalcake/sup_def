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
#define TESTFILE_NAME supdef/tests/common/unistreams_string.ipp

#if !BOOST_TEST_ALREADY_INCLUDED
    #undef BOOST_TEST_MODULE
    #define BOOST_TEST_MODULE unistreams_string_tests
    #include <boost/test/included/unit_test.hpp>
#endif

#include <sup_def/common/sup_def.hpp>
#include <sup_def/tests/tests.h>

#line SUPDEF_TEST_FILE_POS

BOOST_AUTO_TEST_SUITE(unistreams_string,
    * BoostTest::description("Tests for `std::basic_string<::uni::*unistream char types*>`")
)

template <typename CharT>
decltype(auto) getval(const CharT& c)
{
    if constexpr (
        std::same_as<char, CharT> ||
        std::same_as<wchar_t, CharT> ||
        std::same_as<utf8_char, CharT>
    )
        return CharT(c);
    else
        return c.value;
}

BOOST_AUTO_TEST_CASE(test_construction_destruction,
    * BoostTest::description("Tests for the construction and destruction of `uni::string<::uni::*unistream char types*>`")
    * BoostTest::timeout(SUPDEF_TEST_DEFAULT_TIMEOUT)
)
{
    using namespace uni::literals;

    auto as_uint8 = [](const auto& c, size_t i = 0) -> uint8_t
    {
        using c_type = std::remove_cvref_t<decltype(c)>;
        using val_type = decltype(getval<c_type>(c));
        if (i >= sizeof(val_type))
            throw std::out_of_range("Index out of range");
        size_t shift = (sizeof(val_type) - i - 1) * 8;
        val_type tmp = ( getval<c_type>(c) >> shift ) & val_type(0xFF);
        return static_cast<uint8_t>(tmp);
    };

    auto char_str = "😀"uni;

    auto wchar_str = L"😀"uni;
    
    auto utf8_str = u8"😀"uni;
    
    auto utf16be_str = u"😀"uni;
    
    auto utf16le_str = u"😀"uni;
    utf16le_str.set_endianness(uni::endianness::little);
    
    auto utf32be_str = U"😀"uni;
    
    auto utf32le_str = U"😀"uni;
    utf32le_str.set_endianness(uni::endianness::little);

    BOOST_TEST(( std::same_as<decltype(char_str), char_string> ));
    BOOST_TEST(( std::same_as<decltype(wchar_str), wchar_string> ));
    BOOST_TEST(( std::same_as<decltype(utf8_str), utf8_string> ));
    BOOST_TEST(( std::same_as<decltype(utf16be_str), utf16_string> ));
    BOOST_TEST(( std::same_as<decltype(utf16le_str), utf16_string> ));
    BOOST_TEST(( std::same_as<decltype(utf32be_str), utf32_string> ));
    BOOST_TEST(( std::same_as<decltype(utf32le_str), utf32_string> ));

    BOOST_TEST((
        !uni::detail::has_endianness<
            typename uni::detail::uni_string_traits_helper<char_string>::char_type,
            typename uni::detail::uni_string_traits_helper<char_string>::traits_type
        >
    ));
    BOOST_TEST((
        !uni::detail::has_endianness<
            typename uni::detail::uni_string_traits_helper<wchar_string>::char_type,
            typename uni::detail::uni_string_traits_helper<wchar_string>::traits_type
        >
    ));
    BOOST_TEST((
        !uni::detail::has_endianness<
            typename uni::detail::uni_string_traits_helper<utf8_string>::char_type,
            typename uni::detail::uni_string_traits_helper<utf8_string>::traits_type
        >
    ));
    BOOST_TEST((
        uni::detail::has_endianness<
            typename uni::detail::uni_string_traits_helper<utf16_string>::char_type,
            typename uni::detail::uni_string_traits_helper<utf16_string>::traits_type
        > &&
        utf16be_str.verify_endianness(uni::endianness::big) &&
        utf16le_str.verify_endianness(uni::endianness::little)
    ));
    BOOST_TEST((
        uni::detail::has_endianness<
            typename uni::detail::uni_string_traits_helper<utf32_string>::char_type,
            typename uni::detail::uni_string_traits_helper<utf32_string>::traits_type
        > &&
        utf32be_str.verify_endianness(uni::endianness::big) &&
        utf32le_str.verify_endianness(uni::endianness::little)
    ));

    BOOST_TEST(char_str.size() == 4);
    // 0xf0 0x9f 0x98 0x80
    BOOST_TEST(as_uint8(char_str[0], 0) == 0xF0);
    BOOST_TEST(as_uint8(char_str[1], 0) == 0x9F);
    BOOST_TEST(as_uint8(char_str[2], 0) == 0x98);
    BOOST_TEST(as_uint8(char_str[3], 0) == 0x80);

    BOOST_TEST(( wchar_str.size() == 2 || wchar_str.size() == 1 ));

    BOOST_TEST(utf8_str.size() == 4);
    // 0xf0 0x9f 0x98 0x80
    BOOST_TEST(as_uint8(utf8_str[0], 0) == 0xF0);
    BOOST_TEST(as_uint8(utf8_str[1], 0) == 0x9F);
    BOOST_TEST(as_uint8(utf8_str[2], 0) == 0x98);
    BOOST_TEST(as_uint8(utf8_str[3], 0) == 0x80);

    BOOST_TEST(utf16be_str.size() == 2);
    // 0xd8 0x3d 0xde 0x00
    BOOST_TEST(as_uint8(utf16be_str[0], 0) == 0xD8);
    BOOST_TEST(as_uint8(utf16be_str[0], 1) == 0x3D);
    BOOST_TEST(as_uint8(utf16be_str[1], 0) == 0xDE);
    BOOST_TEST(as_uint8(utf16be_str[1], 1) == 0x00);

    BOOST_TEST(utf16le_str.size() == 2);
    // 0x3d 0xd8 0x00 0xde
    BOOST_TEST(as_uint8(utf16le_str[0], 0) == 0x3D);
    BOOST_TEST(as_uint8(utf16le_str[0], 1) == 0xD8);
    BOOST_TEST(as_uint8(utf16le_str[1], 0) == 0x00);
    BOOST_TEST(as_uint8(utf16le_str[1], 1) == 0xDE);

    BOOST_TEST(utf32be_str.size() == 1);
    // 0x00 0x01 0xf6 0x00
    BOOST_TEST(as_uint8(utf32be_str[0], 0) == 0x00);
    BOOST_TEST(as_uint8(utf32be_str[0], 1) == 0x01);
    BOOST_TEST(as_uint8(utf32be_str[0], 2) == 0xF6);
    BOOST_TEST(as_uint8(utf32be_str[0], 3) == 0x00);

    BOOST_TEST(utf32le_str.size() == 1);
    // 0x00 0xf6 0x01 0x00
    BOOST_TEST(as_uint8(utf32le_str[0], 0) == 0x00);
    BOOST_TEST(as_uint8(utf32le_str[0], 1) == 0xF6);
    BOOST_TEST(as_uint8(utf32le_str[0], 2) == 0x01);
    BOOST_TEST(as_uint8(utf32le_str[0], 3) == 0x00);

}

BOOST_AUTO_TEST_CASE(test_conversions,
    * BoostTest::description("Tests for the conversion of `uni::string<::uni::*unistream char types*>`")
    * BoostTest::timeout(SUPDEF_TEST_DEFAULT_TIMEOUT)
    * BoostTest::depends_on("unistreams_string/test_construction_destruction")
)
{
    using namespace uni::literals;

    auto as_uint8 = [](const auto& c, size_t i = 0) -> uint8_t
    {
        using c_type = std::remove_cvref_t<decltype(c)>;
        using val_type = decltype(getval<c_type>(c));
        if (i >= sizeof(val_type))
            throw std::out_of_range("Index out of range");
        size_t shift = (sizeof(val_type) - i - 1) * 8;
        val_type tmp = ( getval<c_type>(c) >> shift ) & val_type(0xFF);
        return static_cast<uint8_t>(tmp);
    };

    char_string char_str = "😀"uni;
    wchar_string wchar_str = L"😀"uni;
    utf8_string utf8_str = u8"😀"uni;
    utf16_string utf16be_str = u"😀"uni;
    utf16_string utf16le_str = u"😀"uni;
    utf16le_str.set_endianness(uni::endianness::little);
    utf32_string utf32be_str = U"😀"uni;
    utf32_string utf32le_str = U"😀"uni;
    utf32le_str.set_endianness(uni::endianness::little);

    try
    {
        std::cout << "Converting from char_string to char_string\n";
        ::SupDef::Util::breakpoint();
        char_string char_to_char = char_str.convert_to<char_string>();

        std::cout << "Converting from char_string to wchar_string\n";
        ::SupDef::Util::breakpoint();
        wchar_string char_to_wchar = char_str.convert_to<wchar_string>();

        std::cout << "Converting from char_string to utf8_string\n";
        ::SupDef::Util::breakpoint();
        utf8_string char_to_utf8 = char_str.convert_to<utf8_string>();

        std::cout << "Converting from char_string to utf16_string (big endian)\n";
        ::SupDef::Util::breakpoint();
        utf16_string char_to_utf16be = char_str.convert_to<utf16_string>(uni::endianness::big);

        std::cout << "Converting from char_string to utf16_string (little endian)\n";
        ::SupDef::Util::breakpoint();
        utf16_string char_to_utf16le = char_str.convert_to<utf16_string>(uni::endianness::little);

        std::cout << "Converting from char_string to utf32_string (big endian)\n";
        ::SupDef::Util::breakpoint();
        utf32_string char_to_utf32be = char_str.convert_to<utf32_string>(uni::endianness::big);

        std::cout << "Converting from char_string to utf32_string (little endian)\n";
        ::SupDef::Util::breakpoint();
        utf32_string char_to_utf32le = char_str.convert_to<utf32_string>(uni::endianness::little);


        std::cout << "Converting from wchar_string to char_string\n";
        ::SupDef::Util::breakpoint();
        char_string wchar_to_char = wchar_str.convert_to<char_string>();

        std::cout << "Converting from wchar_string to wchar_string\n";
        ::SupDef::Util::breakpoint();
        wchar_string wchar_to_wchar = wchar_str.convert_to<wchar_string>();
        
        utf8_string wchar_to_utf8 = wchar_str.convert_to<utf8_string>();
        utf16_string wchar_to_utf16be = wchar_str.convert_to<utf16_string>(uni::endianness::big);
        utf16_string wchar_to_utf16le = wchar_str.convert_to<utf16_string>(uni::endianness::little);
        utf32_string wchar_to_utf32be = wchar_str.convert_to<utf32_string>(uni::endianness::big);
        utf32_string wchar_to_utf32le = wchar_str.convert_to<utf32_string>(uni::endianness::little);

        std::cout << "Converting from utf8_string to other types\n";
        char_string utf8_to_char = utf8_str.convert_to<char_string>();
        wchar_string utf8_to_wchar = utf8_str.convert_to<wchar_string>();
        utf8_string utf8_to_utf8 = utf8_str.convert_to<utf8_string>();
        utf16_string utf8_to_utf16be = utf8_str.convert_to<utf16_string>(uni::endianness::big);
        utf16_string utf8_to_utf16le = utf8_str.convert_to<utf16_string>(uni::endianness::little);
        utf32_string utf8_to_utf32be = utf8_str.convert_to<utf32_string>(uni::endianness::big);
        utf32_string utf8_to_utf32le = utf8_str.convert_to<utf32_string>(uni::endianness::little);

        std::cout << "Converting from utf16_string (big endian) to other types\n";
        char_string utf16be_to_char = utf16be_str.convert_to<char_string>();
        wchar_string utf16be_to_wchar = utf16be_str.convert_to<wchar_string>();
        utf8_string utf16be_to_utf8 = utf16be_str.convert_to<utf8_string>();
        utf16_string utf16be_to_utf16be = utf16be_str.convert_to<utf16_string>(uni::endianness::big);
        utf16_string utf16be_to_utf16le = utf16be_str.convert_to<utf16_string>(uni::endianness::little);
        utf32_string utf16be_to_utf32be = utf16be_str.convert_to<utf32_string>(uni::endianness::big);
        utf32_string utf16be_to_utf32le = utf16be_str.convert_to<utf32_string>(uni::endianness::little);

        std::cout << "Converting from utf16_string (little endian) to other types\n";
        char_string utf16le_to_char = utf16le_str.convert_to<char_string>();
        wchar_string utf16le_to_wchar = utf16le_str.convert_to<wchar_string>();
        utf8_string utf16le_to_utf8 = utf16le_str.convert_to<utf8_string>();
        utf16_string utf16le_to_utf16be = utf16le_str.convert_to<utf16_string>(uni::endianness::big);
        utf16_string utf16le_to_utf16le = utf16le_str.convert_to<utf16_string>(uni::endianness::little);
        utf32_string utf16le_to_utf32be = utf16le_str.convert_to<utf32_string>(uni::endianness::big);
        utf32_string utf16le_to_utf32le = utf16le_str.convert_to<utf32_string>(uni::endianness::little);

        std::cout << "Converting from utf32_string (big endian) to other types\n";
        char_string utf32be_to_char = utf32be_str.convert_to<char_string>();
        wchar_string utf32be_to_wchar = utf32be_str.convert_to<wchar_string>();
        utf8_string utf32be_to_utf8 = utf32be_str.convert_to<utf8_string>();
        utf16_string utf32be_to_utf16be = utf32be_str.convert_to<utf16_string>(uni::endianness::big);
        utf16_string utf32be_to_utf16le = utf32be_str.convert_to<utf16_string>(uni::endianness::little);
        utf32_string utf32be_to_utf32be = utf32be_str.convert_to<utf32_string>(uni::endianness::big);
        utf32_string utf32be_to_utf32le = utf32be_str.convert_to<utf32_string>(uni::endianness::little);

        std::cout << "Converting from utf32_string (little endian) to other types\n";
        char_string utf32le_to_char = utf32le_str.convert_to<char_string>();
        wchar_string utf32le_to_wchar = utf32le_str.convert_to<wchar_string>();
        utf8_string utf32le_to_utf8 = utf32le_str.convert_to<utf8_string>();
        utf16_string utf32le_to_utf16be = utf32le_str.convert_to<utf16_string>(uni::endianness::big);
        utf16_string utf32le_to_utf16le = utf32le_str.convert_to<utf16_string>(uni::endianness::little);
        utf32_string utf32le_to_utf32be = utf32le_str.convert_to<utf32_string>(uni::endianness::big);
        utf32_string utf32le_to_utf32le = utf32le_str.convert_to<utf32_string>(uni::endianness::little);

        BOOST_TEST(( char_to_char == char_str ));
        BOOST_TEST(( wchar_to_char == char_str ));
        BOOST_TEST(( utf8_to_char == char_str ));
        BOOST_TEST(( utf16be_to_char == char_str ));
        BOOST_TEST(( utf16le_to_char == char_str ));
        BOOST_TEST(( utf32be_to_char == char_str ));
        BOOST_TEST(( utf32le_to_char == char_str ));

        BOOST_TEST(( char_to_wchar == wchar_str ));
        BOOST_TEST(( wchar_to_wchar == wchar_str ));
        BOOST_TEST(( utf8_to_wchar == wchar_str ));
        BOOST_TEST(( utf16be_to_wchar == wchar_str ));
        BOOST_TEST(( utf16le_to_wchar == wchar_str ));
        BOOST_TEST(( utf32be_to_wchar == wchar_str ));
        BOOST_TEST(( utf32le_to_wchar == wchar_str ));

        BOOST_TEST(( char_to_utf8 == utf8_str ));
        BOOST_TEST(( wchar_to_utf8 == utf8_str ));
        BOOST_TEST(( utf8_to_utf8 == utf8_str ));
        BOOST_TEST(( utf16be_to_utf8 == utf8_str ));
        BOOST_TEST(( utf16le_to_utf8 == utf8_str ));
        BOOST_TEST(( utf32be_to_utf8 == utf8_str ));
        BOOST_TEST(( utf32le_to_utf8 == utf8_str ));

        BOOST_TEST(( char_to_utf16be == utf16be_str ));
        BOOST_TEST(( wchar_to_utf16be == utf16be_str ));
        BOOST_TEST(( utf8_to_utf16be == utf16be_str ));
        BOOST_TEST(( utf16be_to_utf16be == utf16be_str ));
        BOOST_TEST(( utf16le_to_utf16be == utf16be_str ));
        BOOST_TEST(( utf32be_to_utf16be == utf16be_str ));
        BOOST_TEST(( utf32le_to_utf16be == utf16be_str ));

        BOOST_TEST(( char_to_utf16le == utf16le_str ));
        BOOST_TEST(( wchar_to_utf16le == utf16le_str ));
        BOOST_TEST(( utf8_to_utf16le == utf16le_str ));
        BOOST_TEST(( utf16be_to_utf16le == utf16le_str ));
        BOOST_TEST(( utf16le_to_utf16le == utf16le_str ));
        BOOST_TEST(( utf32be_to_utf16le == utf16le_str ));
        BOOST_TEST(( utf32le_to_utf16le == utf16le_str ));

        BOOST_TEST(( char_to_utf32be == utf32be_str ));
        BOOST_TEST(( wchar_to_utf32be == utf32be_str ));
        BOOST_TEST(( utf8_to_utf32be == utf32be_str ));
        BOOST_TEST(( utf16be_to_utf32be == utf32be_str ));
        BOOST_TEST(( utf16le_to_utf32be == utf32be_str ));
        BOOST_TEST(( utf32be_to_utf32be == utf32be_str ));
        BOOST_TEST(( utf32le_to_utf32be == utf32be_str ));

        BOOST_TEST(( char_to_utf32le == utf32le_str ));
        BOOST_TEST(( wchar_to_utf32le == utf32le_str ));
        BOOST_TEST(( utf8_to_utf32le == utf32le_str ));
        BOOST_TEST(( utf16be_to_utf32le == utf32le_str ));
        BOOST_TEST(( utf16le_to_utf32le == utf32le_str ));
        BOOST_TEST(( utf32be_to_utf32le == utf32le_str ));
        BOOST_TEST(( utf32le_to_utf32le == utf32le_str ));
    }
    catch (const ::SupDef::InternalError& e)
    {
        e.report();
        std::terminate();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        std::terminate();
    }
    catch (...)
    {
        std::cerr << "Caught unknown exception" << std::endl;
        std::terminate();
    }
}

BOOST_AUTO_TEST_SUITE_END()