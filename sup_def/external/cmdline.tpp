/* 
 * MIT License
 * 
 * Copyright (c) 2023 Axel PASCON
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

#ifndef INCLUDED_FROM_SUPDEF_SOURCE
    #error "This file may only be included from a C++ SupDef source file, and may not be compiled directly."
#endif

#if 0
template <typename T, std::enable_if_t<StdStringType<T>>, int> = 0>
    requires FilePath<T>
inline void CmdLine<T>::parse(void)
{
    int c;
    while ((c = getopt(this->argc, this->argv, "I:o:")) != -1)
    {
        switch (c)
        {
            case 'I':
                if (!std::filesystem::exists(SupDef::Util::get_normalized_path(std::filesystem::path(optarg))))
                    throw Exception<char, std::filesystem::path>(ExcType::INVALID_PATH_ERROR, "Path \"" + std::string(optarg) + "\" does not exist");
                this->include_paths.push_back(SupDef::Util::get_normalized_path(std::filesystem::path(optarg)).string());
                break;
            case 'o':
                this->output_file = SupDef::Util::get_normalized_path(std::filesystem::path(optarg)).string();
                break;
            default:
                break;
        }
    }

    if (optind < this->argc && this->argv[optind] != nullptr 
            && std::filesystem::exists(SupDef::Util::get_normalized_path(std::filesystem::path(this->argv[optind]))))
        this->input_file = SupDef::Util::get_normalized_path(std::filesystem::path(this->argv[optind])).string();
    else if (optind < this->argc && this->argv[optind] != nullptr)
        throw Exception<char, std::filesystem::path>(ExcType::INVALID_PATH_ERROR, "Path \"" + std::string(this->argv[optind]) + "\" does not exist");
    else
        throw Exception<char, std::filesystem::path>(ExcType::NO_INPUT_FILE_ERROR, "Please specify an input file");
}


template <typename T, std::enable_if_t<std::is_same_v<std::remove_cvref_t<T>, std::filesystem::path>, int> = 0>
    requires FilePath<T>
inline void CmdLine<T>::parse(void)
{
    int c;
    while ((c = getopt(this->argc, this->argv, "I:o:")) != -1)
    {
        switch (c)
        {
            case 'I':
                if (!std::filesystem::exists(SupDef::Util::get_normalized_path(std::filesystem::path(optarg))))
                    throw Exception<char, std::filesystem::path>(ExcType::INVALID_PATH_ERROR, "Path \"" + std::string(optarg) + "\" does not exist");
                this->include_paths.push_back(SupDef::Util::get_normalized_path(std::filesystem::path(optarg)));
                break;
            case 'o':
                this->output_file = SupDef::Util::get_normalized_path(std::filesystem::path(optarg));
                break;
            default:
                break;
        }
    }

    if (optind < this->argc && this->argv[optind] != nullptr 
            && std::filesystem::exists(SupDef::Util::get_normalized_path(std::filesystem::path(this->argv[optind]))))
        this->input_file = SupDef::Util::get_normalized_path(std::filesystem::path(this->argv[optind]));
    else if (optind < this->argc && this->argv[optind] != nullptr)
        throw Exception<char, std::filesystem::path>(ExcType::INVALID_PATH_ERROR, "Path \"" + std::string(this->argv[optind]) + "\" does not exist");
    else
        throw Exception<char, std::filesystem::path>(ExcType::NO_INPUT_FILE_ERROR, "Please specify an input file");
}

template <typename T, std::enable_if_t<CStrType<T>>, int> = 0>
    requires FilePath<T>
inline void CmdLine<T>::parse(void)
{
    int c;
    while ((c = getopt(this->argc, this->argv, "I:o:")) != -1)
    {
        switch (c)
        {
            case 'I':
                if (!std::filesystem::exists(SupDef::Util::get_normalized_path(std::filesystem::path(optarg))))
                    throw Exception<char, std::filesystem::path>(ExcType::INVALID_PATH_ERROR, "Path \"" + std::string(optarg) + "\" does not exist");
                this->include_paths.push_back(SupDef::Util::get_normalized_path(std::filesystem::path(optarg)).c_str());
                break;
            case 'o':
                this->output_file = SupDef::Util::get_normalized_path(std::filesystem::path(optarg)).c_str();
                break;
            default:
                break;
        }
    }

    if (optind < this->argc && this->argv[optind] != nullptr 
            && std::filesystem::exists(SupDef::Util::get_normalized_path(std::filesystem::path(this->argv[optind]))))
        this->input_file = SupDef::Util::get_normalized_path(std::filesystem::path(this->argv[optind])).c_str();
    else if (optind < this->argc && this->argv[optind] != nullptr)
        throw Exception<char, std::filesystem::path>(ExcType::INVALID_PATH_ERROR, "Path \"" + std::string(this->argv[optind]) + "\" does not exist");
    else
        throw Exception<char, std::filesystem::path>(ExcType::NO_INPUT_FILE_ERROR, "Please specify an input file");
}

#endif


template <typename T>
    requires FilePath<T>
inline constexpr void CmdLine<T>::update_engine(void)
{
    CLR_INC_PATH();
    for (CmdLine<T>::path_type inc : this->include_paths)
    {
        if constexpr (CStrFilePath<T>)
        {
            ADD_INC_PATH(inc.c_str());
        }
        else
        {
            ADD_INC_PATH(inc);
        }
    }
}

template <typename T>
    requires FilePath<T>
inline constexpr void CmdLine<T>::parse(void)
{
    int c;
    while ((c = getopt(this->argc, this->argv, "I:o:")) != -1)
    {
        switch (c)
        {
            case 'I':
                if (!std::filesystem::exists(SupDef::Util::get_normalized_path(std::filesystem::path(optarg))))
                    throw Exception<char, std::filesystem::path>(ExcType::INVALID_PATH_ERROR, "Path \"" + std::string(optarg) + "\" does not exist");
                if constexpr (CStrFilePath<T>)
                {
                    auto const_cstr_path = SupDef::Util::get_normalized_path(std::filesystem::path(optarg)).c_str();
                    path_type to_push;
                    std::memcpy(to_push.data(), const_cstr_path, std::strlen(const_cstr_path));
                    this->include_paths.push_back(to_push);
                }
                else if constexpr (StdStrFilePath<T>)
                    this->include_paths.push_back(SupDef::Util::get_normalized_path(std::filesystem::path(optarg)).string());
                else
                    this->include_paths.push_back(SupDef::Util::get_normalized_path(std::filesystem::path(optarg)));
                break;
            case 'o':
                if constexpr (CStrFilePath<T>)
                {
                    auto const_cstr_path = SupDef::Util::get_normalized_path(std::filesystem::path(optarg)).c_str();
                    std::memcpy(this->output_file.data(), const_cstr_path, std::strlen(const_cstr_path));
                }
                else if constexpr (StdStrFilePath<T>)
                    this->output_file = SupDef::Util::get_normalized_path(std::filesystem::path(optarg)).string();
                else
                    this->output_file = SupDef::Util::get_normalized_path(std::filesystem::path(optarg));
                break;
            default:
                break;
        }
    }
    if (optind < this->argc && this->argv[optind] != nullptr 
            && std::filesystem::exists(SupDef::Util::get_normalized_path(std::filesystem::path(this->argv[optind]))))
    {
        if constexpr (CStrFilePath<T>)
        {
            auto const_cstr_path = SupDef::Util::get_normalized_path(std::filesystem::path(this->argv[optind])).c_str();
            std::memcpy(this->input_file.data(), const_cstr_path, std::strlen(const_cstr_path));
        }
        else if constexpr (StdStrFilePath<T>)
            this->input_file = SupDef::Util::get_normalized_path(std::filesystem::path(this->argv[optind])).string();
        else
            this->input_file = SupDef::Util::get_normalized_path(std::filesystem::path(this->argv[optind]));
    }
    else if (optind < this->argc && this->argv[optind] != nullptr)
        throw Exception<char, std::filesystem::path>(ExcType::INVALID_PATH_ERROR, "Path \"" + std::string(this->argv[optind]) + "\" does not exist");
    else
        throw Exception<char, std::filesystem::path>(ExcType::NO_INPUT_FILE_ERROR, "Please specify an input file");
}