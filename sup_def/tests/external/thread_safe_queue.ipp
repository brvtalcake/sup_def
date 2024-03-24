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
#define TESTFILE_NAME supdef/tests/external/thread_safe_queue.ipp

#if !BOOST_TEST_ALREADY_INCLUDED
    #undef BOOST_TEST_MODULE
    #define BOOST_TEST_MODULE thread_safe_queue_tests
    #include <boost/test/included/unit_test.hpp>
#endif

#include <sup_def/common/sup_def.hpp>
#include <sup_def/tests/tests.h>

#line SUPDEF_TEST_FILE_POS

BOOST_AUTO_TEST_SUITE(thread_safe_queue,
    * BoostTest::description("Tests for `SupDef::Util::ThreadSafeQueue`")
)

BOOST_AUTO_TEST_CASE(test_thread_safe_queue1,
    * BoostTest::description("First test case for `SupDef::Util::ThreadSafeQueue` : tests basic operations in single-threaded environment")
    * BoostTest::timeout(SUPDEF_TEST_DEFAULT_TIMEOUT)
)
{
    using ::SupDef::Util::ThreadSafeQueue;

    ThreadSafeQueue<int> queue;
    const std::vector<int> vec{ 1, 2, 3, 4, 5 };

    BOOST_TEST(queue.empty());
    BOOST_TEST(queue.size() == 0);

    for (auto&& i : vec)
        queue.push(i);
    
    BOOST_TEST(queue.size() == vec.size());

    for (auto&& i : vec)
        BOOST_TEST(queue.wait_for_next() == i);

    BOOST_TEST(queue.empty());
    BOOST_TEST(queue.size() == 0);
}

BOOST_AUTO_TEST_CASE(test_thread_safe_queue2,
    * BoostTest::description("Second test case for `SupDef::Util::ThreadSafeQueue` : tests basic operations in consumer-producer environment")
    * BoostTest::timeout(SUPDEF_TEST_DEFAULT_TIMEOUT)
    * BoostTest::depends_on("thread_safe_queue/test_thread_safe_queue1")
)
{
    using ::SupDef::Util::ThreadSafeQueue;
    using namespace std::chrono_literals;

    std::atomic_flag go_flag = ATOMIC_FLAG_INIT;
    go_flag.clear(std::memory_order::relaxed);
    ThreadSafeQueue<int> queue;
    const std::vector<int> vec{ 1, 2, 3, 4, 5 };

    BOOST_TEST(queue.empty());
    BOOST_TEST(queue.size() == 0);

    std::jthread producer([&queue, &vec, &go_flag]([[maybe_unused]] std::stop_token stoken)
    {
        // Wait for the go signal
        go_flag.wait(false, std::memory_order::acquire);
        for (auto&& i : vec)
        {
            queue.push(i);
            std::this_thread::sleep_for(10ms);
        }
    });

    std::jthread consumer([&queue, &vec, &go_flag]([[maybe_unused]] std::stop_token stoken)
    {
        // Wait for the go signal
        go_flag.wait(false, std::memory_order::acquire);
        for (auto&& i : vec)
        {
            BOOST_TEST(queue.wait_for_next() == i);
            std::this_thread::sleep_for(10ms);
        }
    });

    go_flag.test_and_set(std::memory_order::release);
    go_flag.notify_all();

    producer.request_stop();
    consumer.request_stop();

    producer.join();
    consumer.join();

    BOOST_TEST(queue.empty());
    BOOST_TEST(queue.size() == 0);
}

BOOST_AUTO_TEST_CASE(test_thread_safe_queue3,
    * BoostTest::description("Third test case for `SupDef::Util::ThreadSafeQueue` : tests queue.request_stop()")
    * BoostTest::timeout(SUPDEF_TEST_DEFAULT_TIMEOUT)
    * BoostTest::depends_on("thread_safe_queue/test_thread_safe_queue2")
)
{
    using ::SupDef::Util::ThreadSafeQueue;
    using namespace std::chrono_literals;

    std::atomic_flag go_flag = ATOMIC_FLAG_INIT;
    go_flag.clear(std::memory_order::relaxed);
    ThreadSafeQueue<int> queue;
    
    auto&& dummy_lambda = [&queue, &go_flag]([[maybe_unused]] std::stop_token stoken)
    {
        // Wait for the go signal
        go_flag.wait(false, std::memory_order::acquire);
        while (!stoken.stop_requested())
        {
            try
            {
                std::ignore = queue.wait_for_next();
            }
            catch (const ThreadSafeQueue<int>::StopRequired& e)
            {
                break;
            }
            catch (const std::exception& e)
            {
#ifdef ARR_SIZE
    PUSH_MACRO(ARR_SIZE)
    #undef ARR_SIZE
#endif
#define ARR_SIZE (256 * 2)
                char msg[ARR_SIZE] = { 0 };
                snprintf(msg, ARR_SIZE, "Unexpected exception thrown : %s", e.what());
                BOOST_TEST(false, msg);
                break;
#undef ARR_SIZE
POP_MACRO(ARR_SIZE)
            }
        }
    };

    std::vector<std::jthread> test_threads;
    for (size_t i = 0; i < 5; ++i)
        test_threads.emplace_back(dummy_lambda);

    go_flag.test_and_set(std::memory_order::release);
    go_flag.notify_all();

    std::this_thread::sleep_for(10ms);

    for (auto&& thread : test_threads)
        thread.request_stop();
    queue.request_stop();
    for (auto&& thread : test_threads)
        thread.join();
}

BOOST_AUTO_TEST_CASE(test_thread_safe_queue4,
    * BoostTest::description("Fourth test case for `SupDef::Util::ThreadSafeQueue` : tests basic operations in multiple-consumer-producer environment")
    * BoostTest::timeout(SUPDEF_TEST_DEFAULT_TIMEOUT)
    * BoostTest::depends_on("thread_safe_queue/test_thread_safe_queue3")
)
{
    constexpr size_t NB_CONSUMERS = 5;
    using ::SupDef::Util::ThreadSafeQueue;
    using namespace std::chrono_literals;

    std::atomic_flag go_flag = ATOMIC_FLAG_INIT;
    go_flag.clear(std::memory_order::relaxed);
    std::mutex res_mtx;
    std::vector<int> res;
    const std::vector<int> vec{ 1, 2, 3, 4, 5 };
    ThreadSafeQueue<int> queue;

    BOOST_TEST(queue.empty());
    BOOST_TEST(queue.size() == 0);

    std::jthread producer([&queue, &vec, &go_flag, &NB_CONSUMERS]([[maybe_unused]] std::stop_token stoken)
    {
        // Wait for the go signal
        go_flag.wait(false, std::memory_order::acquire);
        for (auto&& i : vec)
        {
            queue.push(i);
            std::this_thread::sleep_for(1ms / NB_CONSUMERS);
        }
    });

    auto&& consumer_lambda = [&queue, &res, &res_mtx, &go_flag]([[maybe_unused]] std::stop_token stoken)
    {
        // Wait for the go signal
        go_flag.wait(false, std::memory_order::acquire);
        while (!stoken.stop_requested())
        {
            using val_type = decltype(std::declval<ThreadSafeQueue<int>>().wait_for_next());
            val_type val;
            try
            {
                val = queue.wait_for_next();
            }
            catch (const ThreadSafeQueue<int>::StopRequired& e)
            {
                continue;
            }
            catch (const std::exception& e)
            {
#ifdef ARR_SIZE
    PUSH_MACRO(ARR_SIZE)
    #undef ARR_SIZE
#endif
#define ARR_SIZE (256 * 2)
                char msg[ARR_SIZE] = { 0 };
                snprintf(msg, ARR_SIZE, "Unexpected exception thrown : %s", e.what());
                BOOST_TEST(false, msg);
                break;
#undef ARR_SIZE
POP_MACRO(ARR_SIZE)
            }
            std::unique_lock<std::mutex> lock(res_mtx);
            res.push_back(val);
            lock.unlock();
            std::this_thread::sleep_for(1ms);
        }
    };

    std::vector<std::jthread> consumers;
    for (size_t i = 0; i < NB_CONSUMERS; ++i)
        consumers.emplace_back(consumer_lambda);

    go_flag.test_and_set(std::memory_order::release);
    go_flag.notify_all();

    std::this_thread::sleep_for(10ms);

    producer.request_stop();
    producer.join();

    for (auto&& consumer : consumers)
        consumer.request_stop();
    queue.request_stop();
    for (auto&& consumer : consumers)
        consumer.join();

    BOOST_TEST(queue.empty());
    BOOST_TEST(queue.size() == 0);
    for (auto&& i : vec)
    {
        auto&& pos = std::find(res.begin(), res.end(), i);
        bool found = pos != res.end();
        BOOST_TEST(found);
        bool any_other = std::find(pos + 1, res.end(), i) != res.end();
        BOOST_TEST(!any_other);
    }
    BOOST_TEST(res.size() == vec.size());
}

BOOST_AUTO_TEST_CASE(test_thread_safe_queue5,
    * BoostTest::description("Fifht test case for `SupDef::Util::ThreadSafeQueue` : tests wait_for_next_or()")
    * BoostTest::timeout(SUPDEF_TEST_DEFAULT_TIMEOUT)
    * BoostTest::depends_on("thread_safe_queue/test_thread_safe_queue4")
)
{
    using ::SupDef::Util::ThreadSafeQueue;
    using namespace std::chrono_literals;

    ThreadSafeQueue<std::byte> queue;
    auto lambda = [&queue](std::stop_token stoken)
    {
        try
        {
            std::ignore = queue.wait_for_next_or([&stoken](){ return stoken.stop_requested(); }, [](){ throw int(-1); });
        }
        catch (const int& e)
        {
            BOOST_TEST(e == -1);
        }
        catch (const ThreadSafeQueue<std::byte>::StopRequired& e)
        {
            return;
        }
        catch (const std::exception& e)
        {
            BOOST_TEST_ERROR(e.what());
        }
    };
    std::vector<std::jthread> threads;
    for (size_t i = 0; i < 5; ++i)
        threads.emplace_back(lambda);
        
    std::this_thread::sleep_for(2s);

    for (auto&& thread : threads)
        thread.request_stop();
    queue.notify_all();
    for (auto&& thread : threads)
        thread.join();
    queue.request_stop();

    BOOST_TEST(queue.empty());
    BOOST_TEST(queue.size() == 0);
}

BOOST_AUTO_TEST_SUITE_END()