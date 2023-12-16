#ifndef INCLUDED_FROM_SUPDEF_SOURCE
    #error "This file may only be included from a C++ SupDef source file, and may not be compiled directly."
#endif

#include <sup_def/common/config.h>

#if 0
template <typename T>
    requires CharacterType<T>
template <typename... Args>
    requires (std::convertible_to<Args, std::basic_string<T>> && ...)
std::basic_string<T> PragmaDef<T>::substitute(Args&&... args_parm) noexcept(__cpp_lib_unreachable >= 202202L)
{
    auto argc = this->get_argc();
    if (argc == 0)
    {
        UNREACHABLE();
        return this->get_body();
    }

    auto cut_whitespaces = [](std::basic_string<T>& str) -> std::basic_string<T>&
    {
        auto pos = str.find(CONVERT(T, ' '));
        while (pos != std::basic_string<T>::npos)
        {
            str.erase(pos, 1);
            pos = str.find(CONVERT(T, ' '));
        }
        return str;
    };
    std::vector<std::basic_string<T>> args(argc + 1);
    args[0] = std::move(*(this->get_name()));
    args[0] = cut_whitespaces(args[0]);
    std::vector<std::any> unpacked_args{std::forward<Args>(args_parm)...};
    for (decltype(argc) i = 0; i < argc; ++i)
        args[i + 1] = std::any_cast<std::basic_string<T>>(unpacked_args[i]);
    
    std::basic_string<T> body = this->get_body();
    for (decltype(argc) i = 0; i < argc + 1; ++i)
    {
        std::basic_string<T> arg = CONVERT(T, '$') + CONVERT(T, std::to_string(i));
        std::basic_regex<T> arg_regex(arg);
        body = std::regex_replace(body, arg_regex, args[i]);
    }
    return body;
}
#endif