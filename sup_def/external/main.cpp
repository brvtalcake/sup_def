
/*
 * main.cpp
 * 
 * 
 */

// #pragma supdef import // error */

#include <sup_def/common/sup_def.hpp>
#include <sup_def/external/external.hpp>
#include <experimental/scope>
namespace SD = ::SupDef;
namespace SDU = ::SupDef::Util;
namespace SDE = ::SupDef::External;

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <regex>

#include <iomanip> // 

#pragma supdef import <blabla.sd>
#pragma supdef import blabla2.sd

#pragma supdef import "test.sd"

#pragma supdef import "test2.sd"

// #pragma/* */ supdef import "test4.sd"> // error

#pragma supdef import test3.sd


/* #pragma supdef import <te"st"5.sd> // error */

//#pragma supdef import "<test5.sd>" // error

/* test             
    
    */

//
// #pragma supdef import // error */

#if 0

#pragma supdef begin TEST_MACRO1
"#undef $1"
"#define $1 $2"
#pragma supdef end TEST_MACRO1

#pragma supdef begin TEST_MACRO2
"#undef $1"
"#define $1 blah"
#pragma supdef end TEST_MACRO5 // Error

#endif

template <typename T>//test
auto identity(T&& t) -> decltype(auto)
{ return std::forward<T>(t); };

using SDU::demangle;

#if 0
int main/**/(int argc, char/**/const *argv[/*])*/])
{
    SDE::init(argc, argv);
#if 0
    SDE::CmdLine cmd_line(argc, argv);
    try
    {
        cmd_line.parse();
    }
    catch (SD::Exception<char>& e)
    {
        e.report();
        SD::exit_program(1);
        return 1;
    }
    cmd_line.update_engine();
#else
    try
    {
        SD::Parser<char> parser("./sup_def/external/main.cpp");
        parser.slurp_file();
        parser.strip_comments();
        parser.print_content(std::cout);
        for (auto&& inc_or_err : parser.search_imports())
        {
            if (inc_or_err.is_null())
                continue;
#if 0
            auto unwrap_lambda = [](SD::Error<char, std::filesystem::path> err) -> SD::Parser<char>::pragma_loc_type
            {
                err.report();
                return { "", 0, 0 };
            };
            auto inc = inc_or_err.unwrap_or_else<decltype(unwrap_lambda)>(std::move(unwrap_lambda));
#else
            else if (inc_or_err.is_err())
            {
                inc_or_err.error().report();
                continue;
            }
            auto& inc = inc_or_err.unwrap();
#endif
            if (!std::get<0>(inc).empty())
                std::cout << "Found import: " << std::get<0>(inc) << " starting at line " << std::get<1>(inc) << " and ending at line " << std::get<2>(inc) << "\n";
        }
        std::filesystem::remove("./sup_def/external/main.stripped.imports.cc");
        std::ofstream out_file("./sup_def/external/main.stripped.imports.cc");
        parser.print_content(out_file);
        out_file.close();
#if 1
        for (auto&& supdef_or_err : parser.search_super_defines())
        {
            if (supdef_or_err.is_null())
                continue;
#if 0
            auto unwrap_lambda = [](SD::Error<char, std::filesystem::path> err) -> SD::Parser<char>::pragma_loc_type
            {
                err.report();
                return { "", 0, 0 };
            };
            auto supdef = supdef_or_err.unwrap_or_else<decltype(unwrap_lambda)>(std::move(unwrap_lambda));
#else
            else if (supdef_or_err.is_err())
            {
                supdef_or_err.error().report();
                continue;
            }
            auto& supdef = supdef_or_err.unwrap();
#endif
            auto get_first_line = [](const std::string& str) -> std::string
            {
                std::istringstream iss(str);
                std::string line;
                std::getline(iss, line);
                return line;
            };
            if (!std::get<0>(supdef).empty())
                std::cout << "Found super define: " << "\n" << get_first_line(std::get<0>(supdef)) << "\n" << " starting at line " << std::get<1>(supdef) << " and ending at line " << std::get<2>(supdef) << "\n";
        }
#endif
        std::filesystem::remove("./sup_def/external/main.stripped.supdefs.cc");
        out_file.open("./sup_def/external/main.stripped.supdefs.cc");
        parser.print_content(out_file);
        out_file.close();
    }
    catch(const SD::Exception<char, std::filesystem::path>& e)
    {
        e.report();
    }
    catch(const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << '\n';
        return SDU::main_ret();
    }
    catch(...)
    {
        std::cerr << "Unknown exception" << "\n";
        return SDU::main_ret();
    }


#if 0
    std::cout << SD::is_ident_char<char, false>('a') << "\n";
    std::cout << SD::is_ident_char<wchar_t, false>(L'a') << "\n";
    std::cout << SD::is_ident_char<char8_t, false>(u8'a') << "\n";
    std::cout << SD::is_ident_char<char16_t, false>(u'a') << "\n";
    std::cout << SD::is_ident_char<char32_t, false>(U'a') << "\n" << "\n";

    std::cout << SD::is_ident_char<char, false>('$') << "\n";
    std::cout << SD::is_ident_char<wchar_t, false>(L'$') << "\n";
    std::cout << SD::is_ident_char<char8_t, false>(u8'6') << "\n";
    std::cout << SD::is_ident_char<char16_t, false>(u'6') << "\n";
    std::cout << SD::is_ident_char<char32_t, false>(U'6') << "\n" << "\n";

    std::cout << SD::is_ident_char<char, true>('$') << "\n";
    std::cout << SD::is_ident_char<wchar_t, true>(L'$') << "\n";
    std::cout << SD::is_ident_char<char8_t, true>(u8'6') << "\n";
    std::cout << SD::is_ident_char<char16_t, true>(u'6') << "\n";
    std::cout << SD::is_ident_char<char32_t, true>(U'6') << "\n" << "\n";
#endif
#endif

#if __cpp_lib_experimental_parallel_simd >= 201803L && 0
    SDU::Array<int, 5> arr1{1, 2, 3, 4, 5};
    std::cout << arr1.size() << "\n";
    std::cout << arr1[0] << "\n";
    std::cout << arr1[1] << "\n";
    std::cout << arr1[2] << "\n";
    std::cout << arr1[3] << "\n";
    std::cout << arr1[4] << "\n";
    for (size_t i = 0; i < arr1.size(); ++i)
    {
        //std::cout << demangle(typeid(arr1[i]).name()) << "\n";
#if 1
        arr1[i] = 0;
#else
        decltype(arr1)::reference&& ref = identity(arr1[i]);
        identity(ref) = 0;
#endif
    }
    //int assign = 1;
    //for (auto&& i : arr1)
    //{
    //    std::cout << demangle(typeid(i).name()) << "\n";
    //}
    for (auto i : arr1)
        std::cout << i << "\n";
    std::cout << "\n";

    using SDU::Array;
    Array<int, 6> arr2(1, 2, 3, 4, 5, 6);
    std::cout << arr2.size() << "\n";
    for (auto&& i : arr2)
    {
        std::cout << i << "\n";
        i = 123456;
    }
    for (auto i : arr2)
        std::cout << i << "\n";
    std::cout << "\n";

    const Array<int, 1> testarr1(1);
    for (auto i : testarr1)
        std::cout << demangle(typeid(i).name()) << "\n"; // i
    for (auto&& i : testarr1)
        std::cout << demangle(typeid(i).name()) << "\n"; // i
    Array<int, 1> testarr2(1);
    for (auto i : testarr2)
        std::cout << demangle(typeid(i).name()) << "\n"; // Some complicated scheisse
    for (auto&& i : testarr2)
        std::cout << demangle(typeid(i).name()) << "\n";// Some complicated scheisse again
    //for (decltype(testarr2)::reference i : testarr2)
    //    std::cout << demangle(typeid(i).name()) << "\n"; // Some complicated scheisse again
    std::cout << demangle(typeid(arr1[0] = 1).name()) << "\n";
    std::cout << demangle(typeid(arr1[0]).name()) << "\n";
    std::cout << demangle(typeid(decltype(arr1)::reference).name()) << "\n";
    std::cout << demangle(typeid(*(arr1.begin())).name()) << "\n";
    std::cout << demangle(typeid(*(testarr1.begin())).name()) << "\n";
    std::cout << demangle(typeid(*(testarr2.begin())).name()) << "\n";
    std::cout << "\n";

    [[gnu::unused]]
    std::string test_str1 = "/* test */", test_str2 = "// test";
#endif
    /* SDU::exit_program(0); */
#if 0
    using SD::ParsedCharString;
    ParsedCharString<char> test1("test1\nblah\n");
    auto pos = test1.find("blah");
    auto pch = test1.at(0);
    std::cout << demangle(typeid(pch).name()) << "\n";
    std::cout << pch.val() << "\n";
    std::cout << test1.c_str().get() << "\n";
    std::cout << pos << "\n";
#endif
    SD::ParsedChar<char> pch('a');
    const SD::ParsedChar<char> const_pch('a');
    static_assert(std::same_as<char&, decltype(pch.val())>);
    static_assert(std::same_as<char, decltype(const_pch.val())>);
    std::cout << demangle(typeid(pch.val()).name()) << "\n";
    std::cout << demangle(typeid(const_pch.val()).name()) << "\n";
    std::cout << demangle(typeid(pch).name()) << "\n";
    std::cout << demangle(typeid(const_pch).name()) << "\n" << "\n";

    std::cout.imbue(std::locale(""));
    std::cout << CONVERT(char, L"test 六書 1.0231") << "\n";
    std::cout << CONVERT(char, u8"test 六書 1.0231") << "\n";
    std::cout << CONVERT(char, u"test 六書 1.0231") << "\n";
    std::cout << CONVERT(char, U"test 六書 1.0231") << "\n";

    std::wcout.imbue(std::locale(""));
    std::wcout << CONVERT(wchar_t, L"test 六書 1.0231") << "\n";
    std::wcout << CONVERT(wchar_t, u8"test 六書 1.0231") << "\n";
    std::wcout << CONVERT(wchar_t, u"test 六書 1.0231") << "\n";
    std::wcout << CONVERT(wchar_t, U"test 六書 1.0231") << "\n";
    std::wcout << L"test 六書 1.0231" << "\n" << "\n";

    struct teststruct
    {
        int a;
        int b;
        int c;
    };
    teststruct test{1, 2, 3};
    teststruct& testref = test;
    std::cout << COMPARE_ANY(test, testref) << "\n"; // 0
    teststruct* testptr = &testref;
    std::cout << COMPARE_ANY(test, testptr) << "\n"; // 1
    std::cout << COMPARE_ANY(testref, testptr) << "\n"; // 1
    std::cout << COMPARE_ANY(test, *testptr) << "\n"; // 0
    std::cout << COMPARE_ANY(testref, *testptr) << "\n"; // 0
    teststruct test2{1, 2, 4};
    std::cout << COMPARE_ANY(test, test2) << "\n"; // memcmp(&test, &test2, sizeof(teststruct)) == -1

    SD::File<std::ofstream> file("blah.txt", std::ios_base::out);
    file << "test" << 1 << "\n" << std::string("test") << 2 << "\n";
    file.close();
    SD::File<std::ifstream> file2("blah.txt", std::ios_base::in);
    std::string line;
    while (std::getline(file2, line))
        std::cout << line << "\n";
    file2.close();
    std::cout << "\n";

    /* std::cout << std::boolalpha;
    std::cout << SD::test_pragloc_comparator_fn() << "\n" << SD::test_pragloc_comparator_rel() << "\n";
    std::cout << std::noboolalpha; */

    std::cerr.flush();
    std::clog.flush();
    std::cout.flush();
    return SDU::main_ret();
}
#else
int main(int argc, char const *argv[])
{
    SDE::init(argc, argv);

    using namespace uni::literals;
    
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
        std::cerr << "Converting from char_string to char_string\n";
        char_string char_to_char = char_str.convert_to<char_string>();
        std::cerr << "Converting from char_string to wchar_string\n";
        wchar_string char_to_wchar = char_str.convert_to<wchar_string>();
        std::cerr << "Converting from char_string to utf8_string\n";
        utf8_string char_to_utf8 = char_str.convert_to<utf8_string>();
        std::cerr << "Converting from char_string to utf16_string (big endian)\n";
        utf16_string char_to_utf16be = char_str.convert_to<utf16_string>(uni::endianness::big);
        std::cerr << "Converting from char_string to utf16_string (little endian)\n";
        utf16_string char_to_utf16le = char_str.convert_to<utf16_string>(uni::endianness::little);
        std::cerr << "Converting from char_string to utf32_string (big endian)\n";
        utf32_string char_to_utf32be = char_str.convert_to<utf32_string>(uni::endianness::big);
        std::cerr << "Converting from char_string to utf32_string (little endian)\n";
        utf32_string char_to_utf32le = char_str.convert_to<utf32_string>(uni::endianness::little);

        std::cerr << "Converting from wchar_string to char_string\n";
        char_string wchar_to_char = wchar_str.convert_to<char_string>();
        std::cerr << "Converting from wchar_string to wchar_string\n";
        wchar_string wchar_to_wchar = wchar_str.convert_to<wchar_string>();
        std::cerr << "Converting from wchar_string to utf8_string\n";        
        utf8_string wchar_to_utf8 = wchar_str.convert_to<utf8_string>();
        std::cerr << "Converting from wchar_string to utf16_string (big endian)\n";
        utf16_string wchar_to_utf16be = wchar_str.convert_to<utf16_string>(uni::endianness::big);
        std::cerr << "Converting from wchar_string to utf16_string (little endian)\n";
        utf16_string wchar_to_utf16le = wchar_str.convert_to<utf16_string>(uni::endianness::little);
        std::cerr << "Converting from wchar_string to utf32_string (big endian)\n";
        utf32_string wchar_to_utf32be = wchar_str.convert_to<utf32_string>(uni::endianness::big);
        std::cerr << "Converting from wchar_string to utf32_string (little endian)\n";
        utf32_string wchar_to_utf32le = wchar_str.convert_to<utf32_string>(uni::endianness::little);

        std::cerr << "Converting from utf8_string to char_string\n";
        char_string utf8_to_char = utf8_str.convert_to<char_string>();
        std::cerr << "Converting from utf8_string to wchar_string\n";
        wchar_string utf8_to_wchar = utf8_str.convert_to<wchar_string>();
        std::cerr << "Converting from utf8_string to utf8_string\n";
        utf8_string utf8_to_utf8 = utf8_str.convert_to<utf8_string>();
        std::cerr << "Converting from utf8_string to utf16_string (big endian)\n";
        utf16_string utf8_to_utf16be = utf8_str.convert_to<utf16_string>(uni::endianness::big);
        std::cerr << "Converting from utf8_string to utf16_string (little endian)\n";
        utf16_string utf8_to_utf16le = utf8_str.convert_to<utf16_string>(uni::endianness::little);
        std::cerr << "Converting from utf8_string to utf32_string (big endian)\n";
        utf32_string utf8_to_utf32be = utf8_str.convert_to<utf32_string>(uni::endianness::big);
        std::cerr << "Converting from utf8_string to utf32_string (little endian)\n";
        utf32_string utf8_to_utf32le = utf8_str.convert_to<utf32_string>(uni::endianness::little);

        std::cerr << "Converting from utf16_string (big endian) to char_string\n";
        char_string utf16be_to_char = utf16be_str.convert_to<char_string>();
        std::cerr << "Converting from utf16_string (big endian) to wchar_string\n";
        wchar_string utf16be_to_wchar = utf16be_str.convert_to<wchar_string>();
        std::cerr << "Converting from utf16_string (big endian) to utf8_string\n";
        utf8_string utf16be_to_utf8 = utf16be_str.convert_to<utf8_string>();
        std::cerr << "Converting from utf16_string (big endian) to utf16_string (big endian)\n";
        utf16_string utf16be_to_utf16be = utf16be_str.convert_to<utf16_string>(uni::endianness::big);
        std::cerr << "Converting from utf16_string (big endian) to utf16_string (little endian)\n";
        utf16_string utf16be_to_utf16le = utf16be_str.convert_to<utf16_string>(uni::endianness::little);
        std::cerr << "Converting from utf16_string (big endian) to utf32_string (big endian)\n";
        utf32_string utf16be_to_utf32be = utf16be_str.convert_to<utf32_string>(uni::endianness::big);
        std::cerr << "Converting from utf16_string (big endian) to utf32_string (little endian)\n";
        utf32_string utf16be_to_utf32le = utf16be_str.convert_to<utf32_string>(uni::endianness::little);
        std::cerr << "Converting from utf16_string (little endian) to char_string\n";
        char_string utf16le_to_char = utf16le_str.convert_to<char_string>();

        std::cerr << "Converting from utf16_string (little endian) to wchar_string\n";
        wchar_string utf16le_to_wchar = utf16le_str.convert_to<wchar_string>();
        std::cerr << "Converting from utf16_string (little endian) to utf8_string\n";
        utf8_string utf16le_to_utf8 = utf16le_str.convert_to<utf8_string>();
        std::cerr << "Converting from utf16_string (little endian) to utf16_string (big endian)\n";
        utf16_string utf16le_to_utf16be = utf16le_str.convert_to<utf16_string>(uni::endianness::big);
        std::cerr << "Converting from utf16_string (little endian) to utf16_string (little endian)\n";
        utf16_string utf16le_to_utf16le = utf16le_str.convert_to<utf16_string>(uni::endianness::little);
        std::cerr << "Converting from utf16_string (little endian) to utf32_string (big endian)\n";
        utf32_string utf16le_to_utf32be = utf16le_str.convert_to<utf32_string>(uni::endianness::big);
        std::cerr << "Converting from utf16_string (little endian) to utf32_string (little endian)\n";
        utf32_string utf16le_to_utf32le = utf16le_str.convert_to<utf32_string>(uni::endianness::little);

        std::cerr << "Converting from utf32_string (big endian) to char_string\n";
        char_string utf32be_to_char = utf32be_str.convert_to<char_string>();
        std::cerr << "Converting from utf32_string (big endian) to wchar_string\n";
        wchar_string utf32be_to_wchar = utf32be_str.convert_to<wchar_string>();
        std::cerr << "Converting from utf32_string (big endian) to utf8_string\n";
        utf8_string utf32be_to_utf8 = utf32be_str.convert_to<utf8_string>();
        std::cerr << "Converting from utf32_string (big endian) to utf16_string (big endian)\n";
        utf16_string utf32be_to_utf16be = utf32be_str.convert_to<utf16_string>(uni::endianness::big);
        std::cerr << "Converting from utf32_string (big endian) to utf16_string (little endian)\n";
        utf16_string utf32be_to_utf16le = utf32be_str.convert_to<utf16_string>(uni::endianness::little);
        std::cerr << "Converting from utf32_string (big endian) to utf32_string (big endian)\n";
        utf32_string utf32be_to_utf32be = utf32be_str.convert_to<utf32_string>(uni::endianness::big);
        std::cerr << "Converting from utf32_string (big endian) to utf32_string (little endian)\n";
        utf32_string utf32be_to_utf32le = utf32be_str.convert_to<utf32_string>(uni::endianness::little);

        std::cerr << "Converting from utf32_string (little endian) to char_string\n";
        char_string utf32le_to_char = utf32le_str.convert_to<char_string>();
        std::cerr << "Converting from utf32_string (little endian) to wchar_string\n";
        wchar_string utf32le_to_wchar = utf32le_str.convert_to<wchar_string>();
        std::cerr << "Converting from utf32_string (little endian) to utf8_string\n";
        utf8_string utf32le_to_utf8 = utf32le_str.convert_to<utf8_string>();
        std::cerr << "Converting from utf32_string (little endian) to utf16_string (big endian)\n";
        utf16_string utf32le_to_utf16be = utf32le_str.convert_to<utf16_string>(uni::endianness::big);
        std::cerr << "Converting from utf32_string (little endian) to utf16_string (little endian)\n";
        utf16_string utf32le_to_utf16le = utf32le_str.convert_to<utf16_string>(uni::endianness::little);
        std::cerr << "Converting from utf32_string (little endian) to utf32_string (big endian)\n";
        utf32_string utf32le_to_utf32be = utf32le_str.convert_to<utf32_string>(uni::endianness::big);
        std::cerr << "Converting from utf32_string (little endian) to utf32_string (little endian)\n";
        utf32_string utf32le_to_utf32le = utf32le_str.convert_to<utf32_string>(uni::endianness::little);
        
#undef BOOST_TEST
#define BOOST_TEST(x) if (!(x)) { std::cerr << "Test failed: " << #x << std::endl; std::terminate(); }

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

    return SDU::main_ret();
}
#endif
/*
*/