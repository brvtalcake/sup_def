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
#define TESTFILE_NAME supdef/tests/common/pragloc_comparator.ipp

#if !BOOST_TEST_ALREADY_INCLUDED
    #undef BOOST_TEST_MODULE
    #define BOOST_TEST_MODULE pragloc_comparator_tests
    #include <boost/test/included/unit_test.hpp>
#endif

#include <sup_def/common/sup_def.hpp>
#include <sup_def/tests/tests.h>

#line SUPDEF_TEST_FILE_POS

BOOST_AUTO_TEST_SUITE(pragloc_comparator,
    * BoostTest::description("Tests for `SupDef::PragmaLocCompare`")
)

BOOST_AUTO_TEST_CASE(test_pragloc_comparator_fn,
    * BoostTest::description("Tests for `PRAGLOC_COMPARATOR_FN` macro")
    * BoostTest::timeout(SUPDEF_TEST_DEFAULT_TIMEOUT)
)
{
    auto comp1 = PRAGLOC_COMPARATOR_FN(char, int, [](auto&& lhs, auto&& rhs, uint8_t flags) { return PRAGLOC_COMPARE_REL(char, eq, std::forward<decltype(lhs)>(lhs), std::forward<decltype(rhs)>(rhs), flags); });
    BOOST_TEST(comp1(std::make_tuple(0, 0), std::make_tuple(0, 0)));
    BOOST_TEST(!comp1(std::make_tuple(0, 1), std::make_tuple(0, 0)));
    
    // TODO: Add more tests
}

BOOST_AUTO_TEST_CASE(test_pragloc_comparator_rel,
    * BoostTest::description("Tests for `PRAGLOC_COMPARATOR_REL` macro")
    * BoostTest::timeout(SUPDEF_TEST_DEFAULT_TIMEOUT)
    * BoostTest::depends_on("pragloc_comparator/test_pragloc_comparator_fn")
)
{
    auto comp1 = PRAGLOC_COMPARATOR_REL(char, int, eq);
    BOOST_TEST(comp1(std::make_tuple(0, 0), std::make_tuple(0, 0)));
    BOOST_TEST(!comp1(std::make_tuple(0, 1), std::make_tuple(0, 0)));
    
    auto comp2 = PRAGLOC_COMPARATOR_REL(char, int, lt);
    BOOST_TEST(comp2(std::make_tuple(0, 0), std::make_tuple(2, 3)));
    BOOST_TEST(!comp2(std::make_tuple(0, 1), std::make_tuple(0, 0)));
    
    auto comp3 = PRAGLOC_COMPARATOR_REL(char, int, gt);
    BOOST_TEST(comp3(std::make_tuple(1, 1), std::make_tuple(0, 0)));
    BOOST_TEST(!comp3(std::make_tuple(0, 0), std::make_tuple(2, 3)));
    
    // TODO: Add more tests
}

BOOST_AUTO_TEST_SUITE_END()