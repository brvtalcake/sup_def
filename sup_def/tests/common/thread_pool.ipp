/* 
 * MIT License
 * 
 * Copyright (c) 2023-2024 Axel PASCON
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
#define TESTFILE_NAME supdef/tests/common/thread_pool.ipp

#if !BOOST_TEST_ALREADY_INCLUDED
    #undef BOOST_TEST_MODULE
    #define BOOST_TEST_MODULE thread_pool_tests
    #include <boost/test/included/unit_test.hpp>
#endif

#include <sup_def/common/sup_def.hpp>
#include <sup_def/tests/tests.h>

#line SUPDEF_TEST_FILE_POS

BOOST_AUTO_TEST_SUITE(thread_pool,
    * BoostTest::description("Tests for `SupDef::ThreadPool`")
)

BOOST_AUTO_TEST_CASE(test_thread_pool1,
    * BoostTest::description("First test case for `SupDef::ThreadPool`")
    * BoostTest::timeout(SUPDEF_TEST_DEFAULT_TIMEOUT)
)
{
    using ::SupDef::ThreadPool;
    using namespace std::chrono_literals;

    std::atomic<size_t> counter = 0;

    ThreadPool pool;
    BOOST_TEST(pool.size() == std::jthread::hardware_concurrency());

    auto task = [&counter]() {
        std::this_thread::sleep_for(1s);
        return counter++;
    };
    std::vector<std::future<size_t>> futures;
    std::priority_queue<size_t> results;
    for (size_t i = 0; i < 10; i++)
        futures.push_back(pool.enqueue(task));
    for (size_t i = 0; i < 10; i++)
        results.push(futures[i].get());
    for (size_t i = 0; i < 10; i++)
    {
        BOOST_TEST(results.top() == i);
        results.pop();
    }

    BOOST_TEST(counter == 10);
}

BOOST_AUTO_TEST_SUITE_END()