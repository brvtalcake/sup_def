#define SUPDEF_DEBUG 1
#include <sup_def/common/sup_def.hpp>
#include <sup_def/external/external.hpp>

int main(int argc, char const *argv[])
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
    SupDef::Parser<char> parser("./sup_def/external/main.cpp");
    parser.slurp_file();
    parser.strip_comments();
    parser.print_content(std::cout);
    try
    {
        throw SupDef::Exception<char, std::filesystem::path>(SupDef::ExcType::SYNTAX_ERROR, "Unkown SupDef pragma", __FILE__, 0, sizeof("#pragma supdef") - 1, "#pragma supdef blablabla bla");
    }
    catch(const SupDef::Exception<char, std::filesystem::path>& e)
    {
        e.report();
    }
    
#endif
    /* SupDef::Util::exit_program(0); */
    return SupDef::External::main_ret();
}
