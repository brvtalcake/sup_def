#if 0

#include <concepts>
#include <vector>
#include <iostream>

/* void f(int i)
{
    [[assume(i > 0)]];
    std::cout << i << '\n';
} */

#if 1

template <typename T, typename Op>
concept TotalOrder = requires(T a, T b, Op op)
{
    { op(a, b) } -> std::convertible_to<bool>;
    // TODO
};

#else

template <typename Op, typename T>
concept TotalOrdering = requires{
bool operator()(Op, T, T);
axiom Antisymmetry(Op op, T x, T y) {
if (op(x, y) && op(y, x))
x <=> y;
}
axiom Transitivity(Op op, T x, T y, T z) {
if (op(x, y) && op(y, z))
op(x, z);
}
axiom Totality(Op op, T x, T y) {
op(x, y) || op(y, x);
}
};

#endif

int main()
{
}
#endif

#include <iostream>
#include <cstdlib>

struct StartUp_CleanUp
{
    StartUp_CleanUp()
    {
        std::cout << "StartUp_CleanUp()\n";
    }
    ~StartUp_CleanUp()
    {
        std::cout << "~StartUp_CleanUp()\n";
    }
};

__attribute__((__constructor__))
void start_up()
{
    std::cout << "start_up()\n";
}

__attribute__((__destructor__))
void clean_up()
{
    std::cout << "clean_up()\n";
}

__attribute__((__used__))
static StartUp_CleanUp sucu;

void term_handler()
{
    std::cout << "term_handler()\n";
}

int main()
{
    std::cout << "main()\n";
    auto t = std::get_terminate();
    if (t != __gnu_cxx::__verbose_terminate_handler)
        std::cout << "t != __gnu_cxx::__verbose_terminate_handler\n";
    std::set_terminate(term_handler);
    t = std::get_terminate();
    if (t != __gnu_cxx::__verbose_terminate_handler)
        std::cout << "t != __gnu_cxx::__verbose_terminate_handler\n";
    std::abort();
    std::exit(0);
}