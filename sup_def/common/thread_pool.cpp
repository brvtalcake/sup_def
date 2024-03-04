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

#include <sup_def/common/config.h>

#if defined(__INTELLISENSE__)
    #define BOOL_VAL 0
#endif

#if NEED_TPP_INC(ThreadPool) == 0 || BOOL_VAL

#undef BOOL_VAL
#include <sup_def/common/sup_def.hpp>

namespace SupDef
{

    ThreadPool::ThreadPool(const size_t nb_threads)
    {
        using namespace std::string_literals;

        if (nb_threads == 0)
            throw InternalError("Cannot create a thread pool of 0 thread"s);

        std::lock_guard<std::mutex> lock1(this->threads_mtx);
        std::lock_guard<std::shared_mutex> lock2(this->map_mtx);
SAVE_MACRO(SUPDEF_THREADPOOL_USE_LAMBDAS)
#undef SUPDEF_THREADPOOL_USE_LAMBDAS
#if SUPDEF_THREADPOOL_USE_LAMBDAS
        using task_queue_stop_required = task_queue_t::StopRequired;
        ThreadPool*& this_ptr = const_cast<ThreadPool*>(this);
#endif

        for (size_t i = 0; i < nb_threads; ++i)
        {
#if SUPDEF_THREADPOOL_USE_LAMBDAS
            TODO("Make both the main thread function and `get_next_task` function a private static member function of `ThreadPool`, instead of a lambda");
            this->threads.emplace_back(std::piecewise_construct, 
            std::forward_as_tuple(std::move(std::jthread(
                [&this_ptr, i](std::stop_token stoken)
                {
                    while (this_ptr->threads.at(i).second.load(std::memory_order::acquire) != true); // Wait for the go signal
                    do
                    {
                        auto get_next_task = 
                            [&this_ptr]() -> task_queue_t::value_type&&
                            {
                                // If current thread's queue is empty, try to steal a task from another thread
                                // else, return `this_ptr->task_queues[std::this_thread::get_id()].wait_for_next()`
                                TODO(
                                    "Instead of `wait_for_next`, use `wait_for_next_or` with the predicate `!stoken.stop_requested()`"
                                );
                                std::shared_lock<std::shared_mutex> lock(this_ptr->map_mtx); // Get READ access to the map
                                if (this_ptr->task_queues[std::this_thread::get_id()].empty())
                                {
                                    std::jthread::id most_busy_thread_id = this_ptr->get_most_busy_thread(true);
                                    if (this_ptr->task_queues[most_busy_thread_id].empty())
                                        goto just_wait_ours;
                                    else
                                    {
                                        std::unique_lock<std::shared_mutex> lock2(this_ptr->waiting_locations_mtx);
                                        this_ptr->waiting_locations[std::this_thread::get_id()] = most_busy_thread_id;
                                        lock2.unlock();
                                        lock.unlock();
                                        return std::move(this_ptr->task_queues[most_busy_thread_id].wait_for_next()); // Should be instant
                                    }
                                }
                            just_wait_ours:
                                std::unique_lock<std::shared_mutex> lock2(this_ptr->waiting_locations_mtx);
                                waiting_locations[std::this_thread::get_id()] = std::this_thread::get_id();
                                lock2.unlock();
                                lock.unlock();
                                return std::move(this_ptr->task_queues[std::this_thread::get_id()].wait_for_next());
                            };
                        try
                        {
                            // `get_next_task()` will throw `task_queue_stop_required` if the thread should stop
                            auto&& task = get_next_task();
                            std::invoke(std::move(task));
                        }
                        catch (const task_queue_stop_required& e)
                        {
                            continue;
                            // It will stop if the stop token is requested
                            // If we were waiting on another thread's queue, it will just reenter the loop
                            // and retry to steal a task
                        }
                        catch (...)
                        {
                            UNREACHABLE("Unhandled exception of type `", ::SupDef::Util::demangle(std::current_exception().__cxa_exception_type()->name()), "`");
                        }
                    } while (!stoken.stop_requested());
                }
            ))), std::forward_as_tuple(false));
#else
            try
            {
                this->threads.emplace_back(
                    std::piecewise_construct,
                    std::forward_as_tuple(
                        std::move(
                            std::jthread(
                                std::addressof(thread_main),
                                this,
                                i
                            )
                        )
                    ),
                    std::forward_as_tuple(false)
                );
            }
            catch (const std::exception& e)
            {
                throw InternalError("Failed to create thread "s + std::to_string(i) + ": "s + e.what());
            }
#endif
#undef SUPDEF_THREADPOOL_USE_LAMBDAS
RESTORE_MACRO(SUPDEF_THREADPOOL_USE_LAMBDAS)
            // Create the task queue for the thread
            using pair_type = typename decltype(this->task_queues)::value_type;
            const std::jthread::id id = this->threads.at(i).first.get_id();
            pair_type pair{std::move(id), std::move(task_queue_t())};
            if (auto&& [_ignore, inserted] = this->task_queues.insert(
                    std::move(pair)
                ) ; !inserted
            )
                throw InternalError("Failed to create task queue for thread "s + std::to_string(i));
        }
        // Say go to the threads
        for (size_t i = 0; i < nb_threads; ++i)
            this->threads.at(i).second.store(true, std::memory_order::release);
    }

    // TODO: Modify this to use ThreadSafeQueue::request_stop
    ThreadPool::~ThreadPool()
    {
        std::vector<std::jthread::id> ids;
        std::unique_lock<std::mutex> lock(this->threads_mtx);

#if 0
        TODO(
            "Is `std::transform` ok here?"
        );
#endif
        std::transform(
            std::begin(this->threads),
            std::end(this->threads),
            std::back_inserter(ids),
            [](const auto& pair)
            {
                return pair.first.get_id();
            }
        );

        lock.unlock();

        std::for_each(
            std::execution::par,
            std::begin(ids),
            std::end(ids),
            [this](const auto& id)
            {
                this->request_thread_stop(id);
            }
        );
    }

    void ThreadPool::request_thread_stop(std::jthread::id&& id)
    {
        using namespace std::string_literals;
        try
        {
            std::jthread* thread_ptr = this->get_thread_from_id(std::move(id));
            if (thread_ptr == nullptr)
                throw InternalError("Failed to stop thread: thread not found");

            std::jthread& thread = *thread_ptr;
            bool ret_bool = thread.request_stop();
            hard_assert(ret_bool);
    
            std::unique_lock<std::shared_mutex> waiting_locations_lock(this->waiting_locations_mtx, std::defer_lock);
            std::unique_lock<std::shared_mutex> map_lock(this->map_mtx, std::defer_lock);
            std::unique_lock<std::mutex> threads_lock(this->threads_mtx, std::defer_lock);

            waiting_locations_lock.lock();
            std::jthread::id& waiting_where = this->waiting_locations.at(std::move(id));
            size_t ret_size_t = this->waiting_locations.erase(std::move(id));
            hard_assert(ret_size_t == 1);
            waiting_locations_lock.unlock();

            map_lock.lock();
            std::optional<std::reference_wrapper<task_queue_t>> task_queue = std::nullopt;
            try
            {
                task_queue = std::ref(this->task_queues.at(waiting_where));
            }
            catch (const std::out_of_range& e)
            {
                /*
                 * Perhaps the thread to which the queue we were waiting for belonged to
                 * was stopped while we were executing a task from its queue.
                 * In that case, it's just ok to not throw anything.
                 */
                task_queue = std::nullopt;
            }
            catch (...)
            {
                throw;
            }
            this->task_queues.at(std::move(id)).request_stop();
            if (waiting_where != std::move(id) && task_queue.has_value())
            {
                task_queue_t& real_tq = task_queue->get();
                real_tq.push([](){});
                real_tq.notify_all();
            }
            ret_size_t = this->task_queues.erase(std::move(id));
            hard_assert(ret_size_t == 1);
            map_lock.unlock();

            threads_lock.lock();
            thread.join();
            ret_size_t = std::erase_if(
                this->threads,
                [&id](const auto& pair)
                {
                    return pair.first.get_id() == id;
                }
            );
            hard_assert(ret_size_t == 1);
            threads_lock.unlock();
        }
        catch (const InternalError& e)
        {
            throw;
        }
        catch (const std::exception& e)
        {
            throw InternalError("Failed to stop thread: "s + e.what());
        }
    }

    void ThreadPool::request_thread_stop(const std::jthread::id& id)
    {
        using namespace std::string_literals;
        try
        {
            std::jthread* thread_ptr = this->get_thread_from_id(id);
            if (thread_ptr == nullptr)
                throw InternalError("Failed to stop thread: thread not found");

            std::jthread& thread = *thread_ptr;
            bool ret_bool = thread.request_stop();
            hard_assert(ret_bool);
    
            std::unique_lock<std::shared_mutex> waiting_locations_lock(this->waiting_locations_mtx);
            std::unique_lock<std::shared_mutex> map_lock(this->map_mtx);
            std::unique_lock<std::mutex> threads_lock(this->threads_mtx);

            std::jthread::id& waiting_where = this->waiting_locations.at(id);
            size_t ret_size_t = this->waiting_locations.erase(id);
            hard_assert(ret_size_t == 1);

            std::optional<std::reference_wrapper<task_queue_t>> task_queue = std::nullopt;
            try
            {
                task_queue = std::ref(this->task_queues.at(waiting_where));
            }
            catch (const std::out_of_range& e)
            {
                /*
                 * Perhaps the thread to which the queue we were waiting for belonged to
                 * was stopped while we were executing a task from its queue.
                 * In that case, it's just ok to not throw anything.
                 */
                task_queue = std::nullopt;
            }
            catch (...)
            {
                throw;
            }
            this->task_queues.at(id).request_stop();
            if (waiting_where != id && task_queue.has_value())
            {
                task_queue_t& real_tq = task_queue->get();
                real_tq.push([](){});
                real_tq.notify_all();
            }
            ret_size_t = this->task_queues.erase(id);
            hard_assert(ret_size_t == 1);

            thread.join();

            ret_size_t = std::erase_if(
                this->threads,
                [&id](const auto& pair)
                {
                    return pair.first.get_id() == id;
                }
            );
            hard_assert(ret_size_t == 1);
        }
        catch (const InternalError& e)
        {
            throw;
        }
        catch (const std::exception& e)
        {
            throw InternalError("Failed to stop thread: "s + e.what());
        }
    }

    void ThreadPool::request_thread_stop(size_t index)
    {
        std::unique_lock<std::mutex> lock(this->threads_mtx);
        std::jthread::id id = this->threads.at(index).first.get_id();
        lock.unlock();
        this->request_thread_stop(std::move(id));
    }

    std::jthread* ThreadPool::get_thread_from_id(std::jthread::id&& id)
    {
        std::lock_guard<std::mutex> lock(this->threads_mtx);
        for (auto& [thread, go] : this->threads)
        {
            if (thread.get_id() == id)
            {
                return &thread;
            }
        }

        return nullptr;
    }

    std::jthread* ThreadPool::get_thread_from_id(const std::jthread::id& id)
    {
        std::lock_guard<std::mutex> lock(this->threads_mtx);
        for (auto& [thread, go] : this->threads)
        {
            if (thread.get_id() == id)
            {
                return &thread;
            }
        }

        return nullptr;
    }

    std::jthread::id ThreadPool::get_most_busy_thread(bool already_locked)
    {
        if (!already_locked)
        {
            std::jthread::id most_busy_thread_id = this->threads[0].first.get_id();
            std::shared_lock<std::shared_mutex> lock(this->map_mtx); // Get READ access to the map
            for (auto& [id, task_queue] : this->task_queues)
            {
                if (task_queue.size() > this->task_queues[most_busy_thread_id].size())
                {
                    most_busy_thread_id = id;
                }
            }
            return most_busy_thread_id;
        }
        else
        {
            std::jthread::id most_busy_thread_id = this->threads[0].first.get_id();
            for (auto& [id, task_queue] : this->task_queues)
            {
                if (task_queue.size() > this->task_queues[most_busy_thread_id].size())
                {
                    most_busy_thread_id = id;
                }
            }
            return most_busy_thread_id;
        }
        UNREACHABLE();
    }

    std::jthread::id ThreadPool::get_least_busy_thread(bool already_locked)
    {
        if (!already_locked)
        {
            std::jthread::id least_busy_thread_id = this->threads[0].first.get_id();
            std::shared_lock<std::shared_mutex> lock(this->map_mtx); // Get READ access to the map
            for (auto& [id, task_queue] : this->task_queues)
            {
                if (task_queue.size() < this->task_queues[least_busy_thread_id].size())
                {
                    least_busy_thread_id = id;
                }
            }
            return least_busy_thread_id;
        }
        else
        {
            std::jthread::id least_busy_thread_id = this->threads[0].first.get_id();
            for (auto& [id, task_queue] : this->task_queues)
            {
                if (task_queue.size() < this->task_queues[least_busy_thread_id].size())
                {
                    least_busy_thread_id = id;
                }
            }
            return least_busy_thread_id;
        }
        UNREACHABLE();
    }

    size_t ThreadPool::size(void) const noexcept
    {
        return this->threads.size();
    }

    bool ThreadPool::try_remove_threads(size_t nb_threads)
    {
        std::lock_guard<std::mutex> lock(this->threads_mtx);
        if (this->threads.size() < nb_threads)
            return false;
        for (size_t i = 0; i < nb_threads; ++i)
        {
            this->threads.pop_back();
        }
        return true;
    }

    void ThreadPool::add_threads(size_t nb_threads)
    {
        TODO(
            "Implement `ThreadPool::add_threads`"
        );
    }

    void ThreadPool::remove_threads(size_t nb_threads)
    {
        for (size_t i = 0; i < nb_threads; ++i)
        {
            std::jthread::id id = this->get_least_busy_thread(false);
        }
    }
}

#else

template <
    typename FuncType, typename... Args,
    typename ReturnType /* = std::invoke_result_t<FuncType&&, Args&&...> */
>
    requires std::invocable<FuncType, Args...> && (!IsCoro<ReturnType>)
std::future<ReturnType> ThreadPool::enqueue(FuncType&& func, Args&&... args)
{
    std::promise<ReturnType> promise;
    auto future = promise.get_future();
    function_type task =
        [p = std::move(promise), f = std::forward<FuncType>(func), ... a = std::forward<Args>(args)]() mutable
        {
            try
            {
                if constexpr (std::same_as<ReturnType, void>)
                {
                    std::invoke(std::move(f), std::move(a)...);
                    p.set_value();
                }
                else
                {
                    p.set_value(std::invoke(std::move(f), std::move(a)...));
                }
            }
            catch (...)
            {
                p.set_exception(std::current_exception());
            }
        };
    std::unique_lock<std::shared_mutex> lock(this->map_mtx); // Get WRITE access to the map
    std::unique_lock<std::mutex> lock2(this->threads_mtx);
    this->task_queues[this->get_least_busy_thread(true)].push(std::move(task));
    return future;
}

template <
    typename Rep, typename Period,
    typename FuncType, typename... Args,
    typename ReturnType /* = std::invoke_result_t<FuncType&&, Args&&...> */
>
    requires std::invocable<FuncType, Args...> && (!IsCoro<ReturnType>)
std::future<ReturnType> ThreadPool::enqueue(std::chrono::duration<Rep, Period>&& timeout, FuncType&& func, Args&&... args)
{
    using duration_t = std::chrono::duration<Rep, Period>;

    std::promise<ReturnType> promise;
    auto future = promise.get_future();
    auto real_task = std::move(
        TimedTask(
            std::forward<duration_t>(timeout),
            std::forward<FuncType>(func)
        )
    );
    using real_task_t = decltype(real_task);
    function_type task =
        [
            p = std::move(promise),
            f = std::move(real_task),
            ... a = std::forward<Args>(args)
        ]() mutable
        {
            try
            {
                if constexpr (std::same_as<ReturnType, void>)
                {
                    std::invoke(std::move(f), std::move(a)...);
                    p.set_value();
                }
                else
                {
                    p.set_value(std::invoke(std::move(f), std::move(a)...));
                }
            }
            catch (const typename real_task_t::TimedOut& e)
            {
                p.set_exception(std::make_exception_ptr(TaskTimeoutError()));
            }
            catch (...)
            {
                p.set_exception(std::current_exception());
            }
        };
    std::unique_lock<std::shared_mutex> lock(this->map_mtx); // Get WRITE access to the map
    std::unique_lock<std::mutex> lock2(this->threads_mtx);
    this->task_queues[this->get_least_busy_thread(true)].push(std::move(task));
    return future;
}

#endif