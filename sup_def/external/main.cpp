#define SUPDEF_DEBUG 1
#include <sup_def/common/sup_def.hpp>
#include <sup_def/external/external.hpp>

int main(int argc, char const *argv[])
{
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
#endif
    SupDef::exit_program(0);
    return 0;
}
