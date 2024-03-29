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

#ifdef TESTFILE_NAME
    #undef TESTFILE_NAME
#endif
#define TESTFILE_NAME supdef/tests/external/is.ipp

#if !BOOST_TEST_ALREADY_INCLUDED
    #undef BOOST_TEST_MODULE
    #define BOOST_TEST_MODULE is_macro_tests
    #include <boost/test/included/unit_test.hpp>
#endif

#include <sup_def/common/sup_def.hpp>
#include <sup_def/tests/tests.h>

#line SUPDEF_TEST_FILE_POS

namespace SupDef
{
    namespace Tests
    {
        namespace IsMacro
        {
            template <typename T, typename U>
            bool is(T&& a, U&& b)
            {
                return IS(std::forward<T>(a), std::forward<U>(b));
            }

            template <typename T, typename U>
            bool not_is(T&& a, U&& b)
            {
                return !is(std::forward<T>(a), std::forward<U>(b));
            }
        }
    }
}

BOOST_AUTO_TEST_SUITE(is_macro,
    * BoostTest::description("Tests for `IS` macro")
)

BOOST_AUTO_TEST_CASE(is_macro_rvalue_refs,
    * BoostTest::description("Tests for `IS` macro, with rvalue refs")
    * BoostTest::timeout(SUPDEF_TEST_DEFAULT_TIMEOUT)
)
{
    using namespace ::SupDef::Tests::IsMacro;

    struct A { int a; };
    struct B { int a; };

    BOOST_TEST(    is(std::move(A{1}), std::move(A{1})));
    BOOST_TEST(not_is(std::move(A{1}), std::move(A{2})));
    BOOST_TEST(not_is(std::move(A{1}), std::move(B{1})));

    BOOST_TEST(    is(std::move(1), std::move(1)));
    BOOST_TEST(not_is(std::move(1), std::move(2)));
    BOOST_TEST(not_is(std::move(1), std::move(1.0)));
    BOOST_TEST(not_is(std::move(1), std::move(1.0f)));
    BOOST_TEST(not_is(std::move(1), std::move(1u)));
    BOOST_TEST(not_is(std::move(1), std::move(1ul)));
    BOOST_TEST(not_is(std::move(1), std::move(1ull)));
    BOOST_TEST(not_is(std::move(1), std::move(1l)));
    BOOST_TEST(not_is(std::move(1), std::move(1ll)));
}

BOOST_AUTO_TEST_CASE(is_macros_lvalue_refs,
    * BoostTest::description("Tests for `IS` macro, with lvalue refs")
    * BoostTest::timeout(SUPDEF_TEST_DEFAULT_TIMEOUT)
)
{
    using namespace ::SupDef::Tests::IsMacro;

    std::string  s1 = "Hello";
    std::string  s2 = "Hello";
    std::string& s3 = s1;

    BOOST_TEST(not_is(s1, s2)); // Not the same address and not rvalue refs
    BOOST_TEST(    is(s1, s1)); // Same address
    BOOST_TEST(    is(s1, s3)); // Same address
}

BOOST_AUTO_TEST_SUITE_END()