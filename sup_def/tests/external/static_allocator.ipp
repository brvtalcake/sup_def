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
#define TESTFILE_NAME supdef/tests/external/static_allocator.ipp

#if !BOOST_TEST_ALREADY_INCLUDED
    #undef BOOST_TEST_MODULE
    #define BOOST_TEST_MODULE static_allocator_tests
    #include <boost/test/included/unit_test.hpp>
#endif

#include <sup_def/common/sup_def.hpp>
#include <sup_def/tests/tests.h>

#line SUPDEF_TEST_FILE_POS

BOOST_AUTO_TEST_SUITE(static_allocator,
    * BoostTest::description("Tests for `SupDef::Util::ThreadSafeQueue`")
)

BOOST_AUTO_TEST_CASE(test_static_allocator1,
    * BoostTest::description("First test case for `SupDef::Util::` static allocation functions")
    * BoostTest::timeout(SUPDEF_TEST_DEFAULT_TIMEOUT)
)
{
    using ::SupDef::Util::static_alloc;
    using ::SupDef::Util::static_realloc;
    using ::SupDef::Util::static_dealloc;
    using ::SupDef::Util::Test::static_dump_hdrs;

    static_dump_hdrs();
    long double* storage = static_cast<long double*>(static_alloc(sizeof(long double) * 10));
    std::cout << "Allocated storage" << '\n';
    
    BOOST_TEST(storage != nullptr);
    BOOST_TEST(reinterpret_cast<uintptr_t>(storage) % alignof(long double) == 0);
    BOOST_TEST(reinterpret_cast<uintptr_t>(storage) % __STDCPP_DEFAULT_NEW_ALIGNMENT__ == 0);

    static_dump_hdrs();
    for (size_t i = 0; i < 10; i++)
        storage[i] = i;
    for (size_t i = 0; i < 10; i++)
    {
        BOOST_TEST(static_cast<size_t>(storage[i]) == i);
    }
    std::cout << "Filled storage" << '\n';

    storage = static_cast<long double*>(static_realloc(storage, sizeof(long double) * 20));
    std::cout << "Reallocated storage" << '\n';

    BOOST_TEST(storage != nullptr);
    BOOST_TEST(reinterpret_cast<uintptr_t>(storage) % alignof(long double) == 0);
    BOOST_TEST(reinterpret_cast<uintptr_t>(storage) % __STDCPP_DEFAULT_NEW_ALIGNMENT__ == 0);

    static_dump_hdrs();
    for (size_t i = 10; i < 20; i++)
        storage[i] = i;
    for (size_t i = 0; i < 20; i++)
    {
        BOOST_TEST(static_cast<size_t>(storage[i]) == i);
    }
    std::cout << "Filled reallocated storage" << '\n';

    static_dealloc(storage);
    std::cout << "Deallocated storage" << '\n';
    static_dump_hdrs();
}

BOOST_AUTO_TEST_SUITE_END()