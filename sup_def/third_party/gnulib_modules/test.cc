#include <config.h>
#include <gnulib_memset_explicit.h>
#include <gnulib_stdckdint.h>

#if 0
#include <iostream>
#include <tuple>
#endif

#include <stddef.h>
int main(int argc, char const *argv[])
{
    std::ignore = argc;
    std::ignore = argv;

    char buff[10] = { 0 };
    for (char& c : buff)
    {
        c = 69;
    }
    for (size_t i = 0; i < sizeof(buff); ++i)
    {
        std::cout << int(buff[i]) << std::endl;
    }
    gnulibcxx::memset_explicit(buff, 0, sizeof(buff));
    for (size_t i = 0; i < sizeof(buff); ++i)
    {
        std::cout << int(buff[i]) << std::endl;
    }

    int res = 0;
    bool overflow = false;
    std::cout << std::endl << std::boolalpha;
    overflow = ckd_add(&res, 1, 2);
    std::cout << overflow << ", " << res << std::endl;
    overflow = ckd_add(&res, 1, INT_MAX);
    std::cout << overflow << ", " << res << std::endl;
    overflow = ckd_add(&res, 1, INT_MIN);
    std::cout << overflow << ", " << res << std::endl;

    unreachable();
    /* overflow = gnulibcxx::ckd_add(&res, 1, 2);
    std::cout << overflow << ", " << res << std::endl; */

    return 0;
}

#if 1
__STDC__
__STDC_VERSION__
__cplusplus
#endif