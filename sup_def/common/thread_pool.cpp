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
    #define TRUE_VAL 1
#endif

#if NEED_TPP_INC(ThreadPool) == 0 || TRUE_VAL

#undef TRUE_VAL
#include <sup_def/common/sup_def.hpp>

namespace SupDef
{

    // TODO: Modify this to use ThreadSafeQueue::StopRequired
    ThreadPool::ThreadPool(const size_t nb_threads)
    {
        using namespace std::string_literals;
        using task_queue_stop_required = task_queue_t::StopRequired;

        if (nb_threads == 0)
            throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "Cannot create a thread pool of 0 thread");

        std::lock_guard<std::mutex> lock(this->threads_mtx);

        ThreadPool*& this_ptr = const_cast<ThreadPool*>(this);

        for (size_t i = 0; i < nb_threads; ++i)
        {
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
            // Create the task queue for the thread
            using pair_type = typename decltype(this->task_queues)::value_type;
            const std::jthread::id id = this->threads.at(i).first.get_id();
            pair_type pair{std::move(id), std::move(task_queue_t())};
            if (auto&& [_ignore, inserted] = this->task_queues.insert(
                    std::move(pair)
                ) ; !inserted
            )
                throw Exception<char, std::filesystem::path>(ExcType::INTERNAL_ERROR, "Failed to create task queue for thread "s + std::to_string(i));
        }
        // Say go to the threads
        for (size_t i = 0; i < nb_threads; ++i)
            this->threads.at(i).second.store(true, std::memory_order::release);
    }

    // TODO: Modify this to use ThreadSafeQueue::request_stop
    ThreadPool::~ThreadPool()
    {
        std::lock_guard<std::mutex> lock1(this->threads_mtx);
        for (auto& [thread, go] : this->threads)
        {
            thread.request_stop();
        }
        std::unique_lock<std::shared_mutex> lock2(this->map_mtx); // Get WRITE access to the map
        for (auto& [thread, go] : this->threads)
        {
            this->task_queues[thread.get_id()].push([](){});
        }
        lock2.unlock();
        for (auto& [thread, go] : this->threads)
        {
            thread.join();
        }
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

    void ThreadPool::add_threads(size_t nb_threads);
    void ThreadPool::remove_threads(size_t nb_threads);
}

#else

template <typename FuncType, typename... Args, typename ReturnType /* = std::invoke_result_t<FuncType&&, Args&&...> */>
    requires std::invocable<FuncType, Args...> && (!IsCoro<ReturnType>)
std::future<ReturnType> ThreadPool::enqueue(FuncType&& func, Args&&... args)
{
    std::promise<ReturnType> promise;
    auto future = promise.get_future();
    auto task =
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
    this->task_queues[this->get_least_busy_thread(true)].push(std::move(task));
    return future;
}

#endif