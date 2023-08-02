#include <sup_def/common/sup_def.hpp>

int main(int argc, char const *argv[])
{
    SupDef::CmdLine cmd_line(argc, argv);
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
