// "classic" supdef
#pragma supdef begin test
"#if !defined($1)" 
    "#define $1 $2"
"#endif"
#pragma supdef end

// C runnable supdef (unimplemented)
#pragma supdef runnable C begin test2

int main(int argc, char const *argv[])
{
    SUPDEF_RETURN($1);
}

#pragma supdef end

// includes
#pragma supdef include "supdefinitions.sd"

#pragma supdef include "supdefinitions2.sd"

// conditionals (unimplemented)
#pragma supdef if test2(0) != 0
    #include <when_error.h>
#pragma supdef else
    #include <when_ok.h>
#pragma supdef end
// (Expand to `#include <when_ok.h>`)

test2(1) // (Expand to `1`)

// C++ runnable supdef (unimplemented)
#pragma supdef runnable CXX begin test3
#include <string>
int main(int argc, char const *argv[])
{
    std::string returned_str = "constant string";
    SUPDEF_RETURN(returned_str + " returned from a runnable");
}
#pragma supdef end

#warning test3()
// (Expand to `#warning "constant string returned from a runnable"`)