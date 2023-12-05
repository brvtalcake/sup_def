
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
auto identity(T&& t) -> decltype(t) 
{ return std::forward<T>(t); };

using SDU::demangle;

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
            auto unwrap_lambda = [](SD::Error<char, std::filesystem::path> err) -> SD::Parser<char>::pragma_loc_type
            {
                err.report();
                return { "", 0, 0 };
            };
            auto inc = inc_or_err.unwrap_or_else<decltype(unwrap_lambda)>(std::move(unwrap_lambda));
            if (!std::get<0>(inc).empty())
                std::cout << "Found import: " << std::get<0>(inc) << " starting at line " << std::get<1>(inc) << " and ending at line " << std::get<2>(inc) << std::endl;
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
            auto unwrap_lambda = [](SD::Error<char, std::filesystem::path> err) -> SD::Parser<char>::pragma_loc_type
            {
                err.report();
                return { "", 0, 0 };
            };
            auto get_first_line = [](const std::string& str) -> std::string
            {
                std::istringstream iss(str);
                std::string line;
                std::getline(iss, line);
                return line;
            };
            auto supdef = supdef_or_err.unwrap_or_else<decltype(unwrap_lambda)>(std::move(unwrap_lambda));
            if (!std::get<0>(supdef).empty())
                std::cout << "Found super define: " << "\n" << get_first_line(std::get<0>(supdef)) << "\n" << " starting at line " << std::get<1>(supdef) << " and ending at line " << std::get<2>(supdef) << std::endl;
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
        return SDE::main_ret();
    }
    catch(...)
    {
        std::cerr << "Unknown exception" << std::endl;
        return SDE::main_ret();
    }


#if 0
    std::cout << SD::is_ident_char<char, false>('a') << std::endl;
    std::cout << SD::is_ident_char<wchar_t, false>(L'a') << std::endl;
    std::cout << SD::is_ident_char<char8_t, false>(u8'a') << std::endl;
    std::cout << SD::is_ident_char<char16_t, false>(u'a') << std::endl;
    std::cout << SD::is_ident_char<char32_t, false>(U'a') << std::endl << std::endl;

    std::cout << SD::is_ident_char<char, false>('$') << std::endl;
    std::cout << SD::is_ident_char<wchar_t, false>(L'$') << std::endl;
    std::cout << SD::is_ident_char<char8_t, false>(u8'6') << std::endl;
    std::cout << SD::is_ident_char<char16_t, false>(u'6') << std::endl;
    std::cout << SD::is_ident_char<char32_t, false>(U'6') << std::endl << std::endl;

    std::cout << SD::is_ident_char<char, true>('$') << std::endl;
    std::cout << SD::is_ident_char<wchar_t, true>(L'$') << std::endl;
    std::cout << SD::is_ident_char<char8_t, true>(u8'6') << std::endl;
    std::cout << SD::is_ident_char<char16_t, true>(u'6') << std::endl;
    std::cout << SD::is_ident_char<char32_t, true>(U'6') << std::endl << std::endl;
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
        std::cout << demangle(typeid(i).name()) << std::endl; // i
    for (auto&& i : testarr1)
        std::cout << demangle(typeid(i).name()) << std::endl; // i
    Array<int, 1> testarr2(1);
    for (auto i : testarr2)
        std::cout << demangle(typeid(i).name()) << std::endl; // Some complicated scheisse
    for (auto&& i : testarr2)
        std::cout << demangle(typeid(i).name()) << std::endl;// Some complicated scheisse again
    //for (decltype(testarr2)::reference i : testarr2)
    //    std::cout << demangle(typeid(i).name()) << std::endl; // Some complicated scheisse again
    std::cout << demangle(typeid(arr1[0] = 1).name()) << std::endl;
    std::cout << demangle(typeid(arr1[0]).name()) << std::endl;
    std::cout << demangle(typeid(decltype(arr1)::reference).name()) << std::endl;
    std::cout << demangle(typeid(*(arr1.begin())).name()) << std::endl;
    std::cout << demangle(typeid(*(testarr1.begin())).name()) << std::endl;
    std::cout << demangle(typeid(*(testarr2.begin())).name()) << std::endl;
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
    std::cout << demangle(typeid(pch).name()) << std::endl;
    std::cout << pch.val() << std::endl;
    std::cout << test1.c_str().get() << std::endl;
    std::cout << pos << std::endl;
#endif
    SD::ParsedChar<char> pch('a');
    const SD::ParsedChar<char> const_pch('a');
    static_assert(std::same_as<char&, decltype(pch.val())>);
    static_assert(std::same_as<char, decltype(const_pch.val())>);
    std::cout << demangle(typeid(pch.val()).name()) << std::endl;
    std::cout << demangle(typeid(const_pch.val()).name()) << std::endl;
    std::cout << demangle(typeid(pch).name()) << std::endl;
    std::cout << demangle(typeid(const_pch).name()) << "\n" << std::endl;

    std::cout.imbue(std::locale(""));
    std::cout << CONVERT(char, L"test 六書 1.0231") << std::endl;
    std::cout << CONVERT(char, u8"test 六書 1.0231") << std::endl;
    std::cout << CONVERT(char, u"test 六書 1.0231") << std::endl;
    std::cout << CONVERT(char, U"test 六書 1.0231") << std::endl;

    std::wcout.imbue(std::locale(""));
    std::wcout << CONVERT(wchar_t, L"test 六書 1.0231") << std::endl;
    std::wcout << CONVERT(wchar_t, u8"test 六書 1.0231") << std::endl;
    std::wcout << CONVERT(wchar_t, u"test 六書 1.0231") << std::endl;
    std::wcout << CONVERT(wchar_t, U"test 六書 1.0231") << std::endl;
    std::wcout << L"test 六書 1.0231" << "\n" << std::endl;

    struct teststruct
    {
        int a;
        int b;
        int c;
    };
    teststruct test{1, 2, 3};
    teststruct& testref = test;
    std::cout << COMPARE_ANY(test, testref) << std::endl; // 0
    teststruct* testptr = &testref;
    std::cout << COMPARE_ANY(test, testptr) << std::endl; // 1
    std::cout << COMPARE_ANY(testref, testptr) << std::endl; // 1
    std::cout << COMPARE_ANY(test, *testptr) << std::endl; // 0
    std::cout << COMPARE_ANY(testref, *testptr) << std::endl; // 0
    teststruct test2{1, 2, 4};
    std::cout << COMPARE_ANY(test, test2) << std::endl; // memcmp(&test, &test2, sizeof(teststruct)) == -1

    SD::File<std::ofstream> file("blah.txt", std::ios_base::out);
    file << "test";
    return SDE::main_ret();
}
/*
*/