
/*
 * main.cpp
 * 
 * 
 */

// #pragma supdef include // error */

#include <sup_def/common/sup_def.hpp>
#include <sup_def/external/external.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <regex>

#include <iomanip> // 

#pragma supdef include <blabla.sd>
#pragma supdef include blabla2.sd

#pragma supdef include "test.sd"

#pragma supdef include "test2.sd"

#pragma supdef include test3.sd

// Fixme
//#pragma/* */ supdef include "test4.sd"> // error

/* #pragma supdef include <te"st"5.sd> // error */

//#pragma supdef include "<test5.sd>" // error

/* test             
    
    */

//
// #pragma supdef include // error */

template <typename T>
auto identity(T&& t) -> decltype(t) 
{ return std::forward<T>(t); };

using SupDef::Util::demangle;

int main/**/(int argc, char/**/const *argv[/*])*/])
{
    SupDef::External::init(argc, argv);
#if 0
    SupDef::External::CmdLine cmd_line(argc, argv);
    try
    {
        cmd_line.parse();
    }
    catch (SupDef::Exception<char>& e)
    {
        e.report();
        SupDef::exit_program(1);
        return 1;
    }
    cmd_line.update_engine();
#else
    try
    {
        SupDef::Parser<char> parser("./sup_def/external/main.cpp");
        parser.slurp_file();
        parser.strip_comments();
        parser.print_content(std::cout);
        for (auto inc_or_err : parser.search_includes())
        {
            if (inc_or_err.is_null())
                continue;
            auto unwrap_lambda = [](SupDef::Error<char, std::filesystem::path> err) -> std::tuple<std::string, std::size_t, std::size_t>
            {
                err.report();
                return std::make_tuple("", 0, 0);
            };
            auto inc = inc_or_err.unwrap_or_else<decltype(unwrap_lambda)>(std::move(unwrap_lambda));
            if (!std::get<0>(inc).empty())
                std::cout << "Found include: " << std::get<0>(inc) << " starting at line " << std::get<1>(inc) << " and ending at line " << std::get<2>(inc) << std::endl;
        }
        std::filesystem::remove("./sup_def/external/main.stripped.cc");
        std::ofstream out_file("./sup_def/external/main.stripped.cc");
        parser.print_content(out_file);
        throw SupDef::Exception<char, std::filesystem::path>(SupDef::ExcType::INTERNAL_ERROR, "Test exception");
    }
    catch(const SupDef::Exception<char, std::filesystem::path>& e)
    {
        e.report();
    }
    catch(const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << '\n';
        return SupDef::External::main_ret();
    }
    catch(...)
    {
        std::cerr << "Unknown exception" << std::endl;
        return SupDef::External::main_ret();
    }


#if 0
    std::cout << SupDef::is_ident_char<char, false>('a') << std::endl;
    std::cout << SupDef::is_ident_char<wchar_t, false>(L'a') << std::endl;
    std::cout << SupDef::is_ident_char<char8_t, false>(u8'a') << std::endl;
    std::cout << SupDef::is_ident_char<char16_t, false>(u'a') << std::endl;
    std::cout << SupDef::is_ident_char<char32_t, false>(U'a') << std::endl << std::endl;

    std::cout << SupDef::is_ident_char<char, false>('$') << std::endl;
    std::cout << SupDef::is_ident_char<wchar_t, false>(L'$') << std::endl;
    std::cout << SupDef::is_ident_char<char8_t, false>(u8'6') << std::endl;
    std::cout << SupDef::is_ident_char<char16_t, false>(u'6') << std::endl;
    std::cout << SupDef::is_ident_char<char32_t, false>(U'6') << std::endl << std::endl;

    std::cout << SupDef::is_ident_char<char, true>('$') << std::endl;
    std::cout << SupDef::is_ident_char<wchar_t, true>(L'$') << std::endl;
    std::cout << SupDef::is_ident_char<char8_t, true>(u8'6') << std::endl;
    std::cout << SupDef::is_ident_char<char16_t, true>(u'6') << std::endl;
    std::cout << SupDef::is_ident_char<char32_t, true>(U'6') << std::endl << std::endl;
#endif
#endif

#if __cpp_lib_experimental_parallel_simd >= 201803L && 0
    SupDef::Util::Array<int, 5> arr1{1, 2, 3, 4, 5};
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

    using SupDef::Util::Array;
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
        std::cout << demangle(typeid(i).name()) << std::endl; // Some complicated scheisse again
    //for (decltype(testarr2)::reference i : testarr2)
    //    std::cout << demangle(typeid(i).name()) << std::endl; // Some complicated scheisse again
    std::cout << demangle(typeid(arr1[0] = 1).name()) << std::endl;
    std::cout << demangle(typeid(arr1[0]).name()) << std::endl;
    std::cout << demangle(typeid(decltype(arr1)::reference).name()) << std::endl;
    std::cout << demangle(typeid(*(arr1.begin())).name()) << std::endl;
    std::cout << demangle(typeid(*(testarr1.begin())).name()) << std::endl;
    std::cout << demangle(typeid(*(testarr2.begin())).name()) << std::endl;
    std::cout << "\n";
#endif
    /* SupDef::Util::exit_program(0); */
#if 0
    using SupDef::ParsedCharString;
    ParsedCharString<char> test1("test1\nblah\n");
    auto pos = test1.find("blah");
    auto pch = test1.at(0);
    std::cout << demangle(typeid(pch).name()) << std::endl;
    std::cout << pch.val() << std::endl;
    std::cout << test1.c_str().get() << std::endl;
    std::cout << pos << std::endl;
#endif
    return SupDef::External::main_ret();
}
