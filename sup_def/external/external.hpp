#ifndef EXTERNAL_HPP
#define EXTERNAL_HPP

#include <sup_def/common/sup_def.hpp>

namespace SupDef
{
    namespace External
    {
        /**
         * @class CmdLine
         * @brief A class representing the command line arguments, and used to parse them
         * @details A typical invocation of SupDef is: @code sup_def [-I "<include_paths>"]* [-o <output_file>] <input_file> @endcode, with:
         * - @code -I <include_paths> @endcode being an array of include paths, separated by a colon (:) (and as one argument, i.e. surrounded by quotes).
         * There may be multiple -I arguments
         * - @code -o <output_file> @endcode being the output file
         * - @code <input_file> @endcode being the input file being processed
         * 
         * Example:
         * @code sup_def -I "$HOME/my/project/path/include:../../other/project/include" -o ./output_file.c ./input_file.c @endcode
         */
        class CmdLine
        {
            public:
                std::vector<std::filesystem::path> include_paths;
                std::filesystem::path input_file;
                std::filesystem::path output_file;

                CmdLine(int argc, char** argv);
                CmdLine(int argc, const char* argv[]);
                ~CmdLine() = default;

                void parse(void);
                void update_engine(void);

            private:
                int argc;
                char** argv;
        };
    }
}

#endif
