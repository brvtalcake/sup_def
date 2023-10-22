
 



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

#include <iomanip>





 

 



 




int main (int argc, char const *argv[ ])
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
        std::vector<std::shared_ptr<std::basic_string<char>>> includes = parser.search_includes();
        for (size_t i = 0; i < includes.size(); i++)
        {
            std::cout << includes[i]->c_str() << std::endl;
        }
        std::filesystem::remove("./sup_def/external/main.stripped.cc");
        std::ofstream out_file("./sup_def/external/main.stripped.cc");
        parser.print_content(out_file);
    }
    catch(const SupDef::Exception<char, std::filesystem::path>& e)
    {
        e.report();
    }
    catch(const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << '\n';
    }
    catch(...)
    {
        std::cerr << "Unknown exception" << std::endl;
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
     
    return SupDef::External::main_ret();
}
