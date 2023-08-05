#include <sup_def/common/sup_def.hpp>
#include <sup_def/external/external.hpp>

int main(int argc, char const *argv[])
{
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

    SupDef::exit_program(0);
    return 0;
}
