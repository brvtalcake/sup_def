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
    void ThreadPool::wrap_thread_main(std::stop_token stoken, size_t index)
    {
        this->thread_main(stoken, index);
    }
    
    ThreadPool::ThreadPool(const size_t nb_threads)
    {
        using namespace std::string_literals;

        if (nb_threads == 0)
            throw InternalError("Cannot create a thread pool of 0 thread"s);

        LOCK_GUARD(READ_WRITE) threads_lock(this, this->threads);
        LOCK_GUARD(READ_WRITE) task_queues_lock(this, this->task_queues);
        LOCK_GUARD(READ_WRITE) waiting_locations_lock(this, this->waiting_locations);

        symbol_unused
        auto real_thread_main = std::bind_front(
            std::mem_fn(&ThreadPool::wrap_thread_main),
            this
        );
        static_assert(
            std::is_invocable<
                std::decay_t<decltype(std::move(real_thread_main))>,
                std::stop_token,
                std::decay_t<size_t>
            >::value
        );

        for (size_t i = 0; i < nb_threads; ++i)
        {
            try
            {
                this->threads.emplace_back(
                    std::piecewise_construct,
                    std::forward_as_tuple(
                        std::move(
                            std::jthread(
#ifndef __STRICT_ANSI__
                                std::move(&ThreadPool::wrap_thread_main),
                                this,
#else
                                std::move(real_thread_main),
#endif
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

            // Create the task queue for the thread
            using pair_type = typename decltype(this->task_queues)::value_type;

            const std::jthread::id id = this->threads.at(i).first.get_id();
            pair_type pair{id, std::move(task_queue_t())};
            if (auto&& [_ignore, inserted] = this->task_queues.insert(
                    std::move(pair)
                ) ; !inserted
            )
                throw InternalError("Failed to create task queue for thread "s + std::to_string(i));

            // Create the waiting location for the thread
            if (auto&& [_ignore, inserted] = this->waiting_locations.insert(
                    std::pair<std::jthread::id, std::jthread::id>(
                        id,
                        std::move(id)
                    )
                ) ; !inserted
            )
                throw InternalError("Failed to create waiting location for thread "s + std::to_string(i));
        }
        // Say go to the threads
        for (size_t i = 0; i < nb_threads; ++i)
            this->threads.at(i).second.store(true, std::memory_order::release);
    }

    ThreadPool::~ThreadPool()
    {
        std::vector<std::jthread::id> ids;
        
        LOCK_GUARD(READ_WRITE) lock(this, this->threads);

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
            std::jthread* thread_ptr = this->get_thread_from_id(id);
            if (thread_ptr == nullptr)
                throw InternalError("Failed to stop thread: thread not found");

            std::jthread& thread = *thread_ptr;
            bool ret_bool = thread.request_stop();
            thread.join();
            hard_assert(ret_bool);
    
            LOCK_GUARD(READ_WRITE) waiting_locations_lock(this, this->waiting_locations, std::defer_lock);
            LOCK_GUARD(READ_WRITE) task_queues_lock(this, this->task_queues, std::defer_lock);
            LOCK_GUARD(READ_WRITE) threads_lock(this, this->threads, std::defer_lock);

            waiting_locations_lock.lock();
            std::jthread::id& waiting_where = this->waiting_locations.at(id);
            size_t ret_size_t = this->waiting_locations.erase(id);
            hard_assert(ret_size_t == 1);
            waiting_locations_lock.unlock();

            task_queues_lock.lock();
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
            task_queues_lock.unlock();

            threads_lock.lock();
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
        std::jthread::id copy = id;
        this->request_thread_stop(std::move(copy));
    }

    void ThreadPool::request_thread_stop(size_t index)
    {
        LOCK_GUARD(READ) threads_lock(this, this->threads);
        std::jthread::id id = this->threads.at(index).first.get_id();
        threads_lock.unlock();
        this->request_thread_stop(std::move(id));
    }

    std::jthread* ThreadPool::get_thread_from_id(std::jthread::id&& id)
    {
        LOCK_GUARD(READ) threads_lock(this, this->threads);
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
        LOCK_GUARD(READ) threads_lock(this, this->threads);
        for (auto& [thread, go] : this->threads)
        {
            if (thread.get_id() == id)
            {
                return &thread;
            }
        }
        return nullptr;
    }

    std::jthread::id ThreadPool::get_most_busy_thread(void)
    {
        using namespace std::string_literals;
        LOCK_GUARD(READ) task_queues_lock(this, this->task_queues);
        LOCK_GUARD(READ) threads_lock(this, this->threads);

        try
        {
            std::jthread::id most_busy_thread_id = this->threads.at(0).first.get_id();
            for (auto& [id, task_queue] : this->task_queues)
            {
                if (task_queue.size() > this->task_queues.at(most_busy_thread_id).size())
                {
                    most_busy_thread_id = id;
                }
            }
            return most_busy_thread_id;
        }
        catch (const std::exception& e)
        {
            throw InternalError("Failed to get most busy thread: "s + e.what());
        }
    }

    std::jthread::id ThreadPool::get_least_busy_thread(void)
    {
        using namespace std::string_literals;
        LOCK_GUARD(READ) task_queues_lock(this, this->task_queues);
        LOCK_GUARD(READ) threads_lock(this, this->threads);

        try
        {
            std::jthread::id least_busy_thread_id = this->threads.at(0).first.get_id();
            for (auto& [id, task_queue] : this->task_queues)
            {
                if (task_queue.size() < this->task_queues.at(least_busy_thread_id).size())
                {
                    least_busy_thread_id = id;
                }
            }
            return least_busy_thread_id;
        }
        catch (const std::exception& e)
        {
            throw InternalError("Failed to get least busy thread: "s + e.what());
        }
    }

    size_t ThreadPool::size(void) const noexcept
    {
        LOCK_GUARD(READ) threads_lock(this, this->threads);
        return this->threads.size();
    }

    bool ThreadPool::try_remove_threads(size_t nb_threads)
    {
        LOCK_GUARD(READ) task_queues_lock(this, this->task_queues, std::defer_lock);
        LOCK_GUARD(READ) threads_lock(this, this->threads);
        if (this->threads.size() < nb_threads)
            return false;
        threads_lock.unlock();
        for (size_t i = 0; i < nb_threads; ++i)
        {
            std::jthread::id id = this->get_least_busy_thread();
            task_queues_lock.lock();
            if (this->task_queues.at(id).empty())
            {
                task_queues_lock.unlock();
                this->request_thread_stop(id);
                continue;
            }
            else
                return false;
        }
        return true;
    }

    void ThreadPool::add_threads(size_t nb_threads)
    {
        using namespace std::string_literals;
        LOCK_GUARD(READ_WRITE) threads_lock(this, this->threads);
        LOCK_GUARD(READ_WRITE) task_queues_lock(this, this->task_queues);
        LOCK_GUARD(READ_WRITE) waiting_locations_lock(this, this->waiting_locations);
        
        symbol_unused
        auto real_thread_main = std::bind_front(
            std::mem_fn(&ThreadPool::wrap_thread_main),
            this
        );
        static_assert(
            std::is_invocable<
                std::decay_t<decltype(std::move(real_thread_main))>,
                std::stop_token,
                std::decay_t<size_t>
            >::value
        );

        for (size_t i = this->threads.size(); i < this->threads.size() + nb_threads; ++i)
        {
            try
            {
                this->threads.emplace_back(
                    std::piecewise_construct,
                    std::forward_as_tuple(
                        std::move(
                            std::jthread(
#ifndef __STRICT_ANSI__
                                std::move(&ThreadPool::wrap_thread_main),
                                this,
#else
                                std::move(real_thread_main),
#endif
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

            // Create the task queue for the thread
            using pair_type = typename decltype(this->task_queues)::value_type;

            const std::jthread::id id = this->threads.at(i).first.get_id();
            pair_type pair{id, std::move(task_queue_t())};
            if (auto&& [_ignore, inserted] = this->task_queues.insert(
                    std::move(pair)
                ) ; !inserted
            )
                throw InternalError("Failed to create task queue for thread "s + std::to_string(i));

            // Create the waiting location for the thread
            if (auto&& [_ignore, inserted] = this->waiting_locations.insert(
                    std::pair<std::jthread::id, std::jthread::id>(
                        id,
                        std::move(id)
                    )
                ) ; !inserted
            )
                throw InternalError("Failed to create waiting location for thread "s + std::to_string(i));
        }
        // Say go to the threads
        for (size_t i = this->threads.size() - nb_threads; i < this->threads.size(); ++i)
            this->threads.at(i).second.store(true, std::memory_order::release);
    }

    void ThreadPool::remove_threads(size_t nb_threads)
    {
        using namespace std::string_literals;
        LOCK_GUARD(READ) task_queues_lock(this, this->task_queues, std::defer_lock);
        for (size_t i = 0; i < nb_threads; ++i)
        {
            std::jthread::id id = this->get_least_busy_thread();
            task_queues_lock.lock();
            if (this->task_queues.at(id).empty())
            {
                task_queues_lock.unlock();
                this->request_thread_stop(id);
                continue;
            }
            else
                throw InternalError("Cannot remove thread "s + std::to_string(i) + ": it is not idle");
        }
    }
}

#else

template <typename ThreadPoolType, typename ThreadPoolRequiredAliases>
    requires ThreadPoolBaseRequireClause<ThreadPoolType, ThreadPoolRequiredAliases>
template <typename T>
    requires std::negation_v<std::is_reference<T>>
RecursiveSharedMutex& ThreadPoolBase<ThreadPoolType, ThreadPoolRequiredAliases>::get_mtx(const T& resource)
{
    const uintptr_t addr = reinterpret_cast<uintptr_t>(std::addressof(resource));
    const uintptr_t tq_addr = reinterpret_cast<uintptr_t>(std::addressof(this->task_queues));
    const uintptr_t th_addr = reinterpret_cast<uintptr_t>(std::addressof(this->threads));
    const uintptr_t wl_addr = reinterpret_cast<uintptr_t>(std::addressof(this->waiting_locations));
    
    hard_assert(tq_addr != th_addr);
    hard_assert(tq_addr != wl_addr);
    hard_assert(th_addr != wl_addr);

    if (addr == tq_addr)
        return this->task_queues_mtx;
    else if (addr == th_addr)
        return this->threads_mtx;
    else if (addr == wl_addr)
        return this->waiting_locations_mtx;
    else
        throw InternalError("Unknown ThreadPool-resource requested for access");
    UNREACHABLE();
}

template <typename ThreadPoolType, typename ThreadPoolRequiredAliases>
    requires ThreadPoolBaseRequireClause<ThreadPoolType, ThreadPoolRequiredAliases>
template <typename T>
    requires std::negation_v<std::is_reference<T>>
const RecursiveSharedMutex& ThreadPoolBase<ThreadPoolType, ThreadPoolRequiredAliases>::get_mtx(const T& resource) const
{
    const uintptr_t addr = reinterpret_cast<uintptr_t>(std::addressof(resource));
    const uintptr_t tq_addr = reinterpret_cast<uintptr_t>(std::addressof(this->task_queues));
    const uintptr_t th_addr = reinterpret_cast<uintptr_t>(std::addressof(this->threads));
    const uintptr_t wl_addr = reinterpret_cast<uintptr_t>(std::addressof(this->waiting_locations));
    
    hard_assert(tq_addr != th_addr);
    hard_assert(tq_addr != wl_addr);
    hard_assert(th_addr != wl_addr);

    if (addr == tq_addr)
        return this->task_queues_mtx;
    else if (addr == th_addr)
        return this->threads_mtx;
    else if (addr == wl_addr)
        return this->waiting_locations_mtx;
    else
        throw InternalError("Unknown ThreadPool-resource requested for access");
    UNREACHABLE();
}

template <typename ThreadPoolType, typename ThreadPoolRequiredAliases>
    requires ThreadPoolBaseRequireClause<ThreadPoolType, ThreadPoolRequiredAliases>
template <uint8_t AccessType, typename T>
    requires std::negation_v<std::is_reference<T>>
inline void ThreadPoolBase<ThreadPoolType, ThreadPoolRequiredAliases>::get_access(const T& resource) const
{
    const uintptr_t addr = reinterpret_cast<uintptr_t>(std::addressof(resource));
    const uintptr_t tq_addr = reinterpret_cast<uintptr_t>(std::addressof(this->task_queues));
    const uintptr_t th_addr = reinterpret_cast<uintptr_t>(std::addressof(this->threads));
    const uintptr_t wl_addr = reinterpret_cast<uintptr_t>(std::addressof(this->waiting_locations));

    hard_assert(tq_addr != th_addr);
    hard_assert(tq_addr != wl_addr);
    hard_assert(th_addr != wl_addr);

    if (addr == tq_addr)
        this->get_mtx(this->task_queues).lock(AccessType);
    else if (addr == th_addr)
        this->get_mtx(this->threads).lock(AccessType);
    else if (addr == wl_addr)
        this->get_mtx(this->waiting_locations).lock(AccessType);
    else
        throw InternalError("Unknown ThreadPool-resource requested for access");
    UNREACHABLE();
}

template <typename ThreadPoolType, typename ThreadPoolRequiredAliases>
    requires ThreadPoolBaseRequireClause<ThreadPoolType, ThreadPoolRequiredAliases>
template <uint8_t AccessType, typename T>
    requires std::negation_v<std::is_reference<T>>
inline bool ThreadPoolBase<ThreadPoolType, ThreadPoolRequiredAliases>::try_get_access(const T& resource) const
{
    const uintptr_t addr = reinterpret_cast<uintptr_t>(std::addressof(resource));
    const uintptr_t tq_addr = reinterpret_cast<uintptr_t>(std::addressof(this->task_queues));
    const uintptr_t th_addr = reinterpret_cast<uintptr_t>(std::addressof(this->threads));
    const uintptr_t wl_addr = reinterpret_cast<uintptr_t>(std::addressof(this->waiting_locations));

    hard_assert(tq_addr != th_addr);
    hard_assert(tq_addr != wl_addr);
    hard_assert(th_addr != wl_addr);

    if (addr == tq_addr)
        return this->get_mtx(this->task_queues).try_lock(AccessType);
    else if (addr == th_addr)
        return this->get_mtx(this->threads).try_lock(AccessType);
    else if (addr == wl_addr)
        return this->get_mtx(this->waiting_locations).try_lock(AccessType);
    else
        throw InternalError("Unknown ThreadPool-resource requested for access");
    UNREACHABLE();
}

template <typename ThreadPoolType, typename ThreadPoolRequiredAliases>
    requires ThreadPoolBaseRequireClause<ThreadPoolType, ThreadPoolRequiredAliases>
template <uint8_t AccessType, typename T>
    requires std::negation_v<std::is_reference<T>>
inline void ThreadPoolBase<ThreadPoolType, ThreadPoolRequiredAliases>::release_access(const T& resource) const
{
    const uintptr_t addr = reinterpret_cast<uintptr_t>(std::addressof(resource));
    const uintptr_t tq_addr = reinterpret_cast<uintptr_t>(std::addressof(this->task_queues));
    const uintptr_t th_addr = reinterpret_cast<uintptr_t>(std::addressof(this->threads));
    const uintptr_t wl_addr = reinterpret_cast<uintptr_t>(std::addressof(this->waiting_locations));

    hard_assert(tq_addr != th_addr);
    hard_assert(tq_addr != wl_addr);
    hard_assert(th_addr != wl_addr);

    if (addr == tq_addr)
        this->get_mtx(this->task_queues).unlock(AccessType);
    else if (addr == th_addr)
        this->get_mtx(this->threads).unlock(AccessType);
    else if (addr == wl_addr)
        this->get_mtx(this->waiting_locations).unlock(AccessType);
    else
        throw InternalError("Unknown ThreadPool-resource requested for access");
    UNREACHABLE();
}

template <typename ThreadPoolType, typename ThreadPoolRequiredAliases>
    requires ThreadPoolBaseRequireClause<ThreadPoolType, ThreadPoolRequiredAliases>
ThreadPoolBase<ThreadPoolType, ThreadPoolRequiredAliases>::get_next_task_return_type
    ThreadPoolBase<ThreadPoolType, ThreadPoolRequiredAliases>::get_next_task(
        std::stop_token stoken
    )
{
    // If current thread's queue is empty, try to steal a task from another thread
    // else, return `this->task_queues[std::this_thread::get_id()].wait_for_next()`
    using task_queue_stop_required = typename task_queue_t::StopRequired;

    LOCK_GUARD(READ) task_queues_lock(this, this->task_queues);
    LOCK_GUARD(WRITE) waiting_locations_lock(this, this->waiting_locations, std::defer_lock);

    static const auto stoken_stop_requested_pred = [](const std::stop_token& stoken) -> bool
    {
        return stoken.stop_requested();
    };
    static const auto to_invoke_if_true = []() -> void
    {
        throw task_queue_stop_required();
    };
    if (this->task_queues.at(std::this_thread::get_id()).empty())
    {
        std::jthread::id most_busy_thread_id = static_cast<ThreadPoolType*>(this)->get_most_busy_thread();
        if (this->task_queues.at(most_busy_thread_id).empty())
            goto just_wait_ours;
        else
        {
            waiting_locations_lock.lock();
            this->waiting_locations[std::this_thread::get_id()] = most_busy_thread_id;
            waiting_locations_lock.unlock();
            task_queue_t* const tq = std::addressof(this->task_queues.at(most_busy_thread_id));
            task_queues_lock.unlock();
            return tq->wait_for_next_or(stoken_stop_requested_pred, to_invoke_if_true, stoken);
        }
    }
just_wait_ours:
    waiting_locations_lock.lock();
    this->waiting_locations[std::this_thread::get_id()] = std::this_thread::get_id();
    waiting_locations_lock.unlock();
    task_queue_t* const tq = std::addressof(this->task_queues.at(std::this_thread::get_id()));
    task_queues_lock.unlock();
    return tq->wait_for_next_or(stoken_stop_requested_pred, to_invoke_if_true, stoken);
}

template <typename ThreadPoolType, typename ThreadPoolRequiredAliases>
    requires ThreadPoolBaseRequireClause<ThreadPoolType, ThreadPoolRequiredAliases>
void ThreadPoolBase<ThreadPoolType, ThreadPoolRequiredAliases>::thread_main(std::stop_token stoken, size_t index)
{
    using task_queue_stop_required = task_queue_t::StopRequired;
#if 0
    const ThreadPoolType& const_ref = *const_this_ptr;
    ThreadPoolType& ref = const_cast<ThreadPoolType&>(const_ref);
    ThreadPoolType* const this_ptr = std::addressof(ref);
#elif 0
    ThreadPoolType* const this_ptr = std::addressof(const_cast<ThreadPoolType&>(*const_this_ptr));
#endif

    /* delctype(auto) this_ptr = dynamic_cast<ThreadPoolType* const>(this); */
    this->get_access<READ>(this->threads);
    while (this->threads.at(index).second.load(std::memory_order::acquire) != true); // Wait for the go signal
    this->release_access<READ>(this->threads);
    do
    {
        try
        {
            // `get_next_task()` will throw `task_queue_stop_required` if the thread should stop
            auto&& task = this->get_next_task(stoken);
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

template <
    typename FuncType, typename... Args,
    typename ReturnType /* = std::invoke_result_t<FuncType&&, Args&&...> */
>
    requires std::invocable<FuncType, Args...> && (!IsCoro<ReturnType>) && (std::is_copy_constructible_v<std::remove_reference_t<Args>> && ...)
std::future<ReturnType> ThreadPool::enqueue(FuncType&& func, Args&&... args)
{
#if 0
    struct WrapperFunctor
    {
        std::promise<ReturnType> promise;
        FuncType func;
        std::tuple<Args...> args;

        WrapperFunctor(std::promise<ReturnType>&& p, FuncType&& f, Args&&... a)
            : promise(std::move(p)), func(std::forward<FuncType>(f)), args(std::forward<Args>(a)...)
        {}
        WrapperFunctor(const WrapperFunctor& other) = delete;
/*             : WrapperFunctor(std::move(other))
        {} */
        WrapperFunctor(WrapperFunctor&& other)
            : promise(std::move(other.promise)), func(std::forward<FuncType>(other.func)), args(std::move(other.args))
        {}

        void operator()(void)
        {
            try
            {
                if constexpr (std::same_as<std::remove_cv_t<ReturnType>, void>)
                {
                    std::apply(std::move(this->func), std::move(this->args));
                    promise.set_value();
                }
                else
                {
                    promise.set_value(std::apply(std::move(this->func), std::move(this->args)));
                }
            }
            catch (...)
            {
                promise.set_exception(std::current_exception());
            }
        }
    };
#endif
    std::shared_ptr<std::promise<ReturnType>> promise = std::make_shared<std::promise<ReturnType>>();
    auto future = promise->get_future();
#if 1
    function_type task =
        [p = promise, f = std::forward<FuncType>(func), ... a = std::forward<Args>(args)]() mutable
        {
            try
            {
                if constexpr (std::same_as<std::remove_cv_t<ReturnType>, void>)
                {
                    std::invoke(std::move(f), std::move(a)...);
                    p->set_value();
                }
                else
                {
                    p->set_value(std::invoke(std::move(f), std::move(a)...));
                }
            }
            catch (...)
            {
                p->set_exception(std::current_exception());
            }
        };
#else
    function_type task(
        WrapperFunctor(
            std::move(promise),
            std::forward<FuncType>(func),
            std::forward<Args>(args)...
        )
    );
#endif
    LOCK_GUARD(READ_WRITE) waiting_locations_lock(this, this->waiting_locations);
    LOCK_GUARD(READ_WRITE) task_queues_lock(this, this->task_queues);
    LOCK_GUARD(READ_WRITE) threads_lock(this, this->threads);
    this->task_queues[this->get_least_busy_thread()].push(std::move(task));
    return future;
}

template <
    typename Rep, typename Period,
    typename FuncType, typename... Args,
    typename ReturnType /* = std::invoke_result_t<FuncType&&, Args&&...> */
>
    requires std::invocable<FuncType, Args...> && (!IsCoro<ReturnType>) && (std::is_copy_constructible_v<std::remove_reference_t<Args>> && ...)
std::future<ReturnType> ThreadPool::enqueue(std::chrono::duration<Rep, Period>&& timeout, FuncType&& func, Args&&... args)
{
    using duration_t = std::chrono::duration<Rep, Period>;

    std::shared_ptr<std::promise<ReturnType>> promise = std::make_shared<std::promise<ReturnType>>();
    auto future = promise->get_future();
    auto real_task = std::move(
        TimedTask(
            std::forward<duration_t>(timeout),
            std::forward<FuncType>(func)
        )
    );
    using real_task_t = decltype(real_task);

#if 0
    struct WrapperFunctor
    {
        std::promise<ReturnType> promise;
        real_task_t real_task;
        std::tuple<Args...> args;

        WrapperFunctor(std::promise<ReturnType>&& p, real_task_t&& f, Args&&... a)
            : promise(std::move(p)), real_task(std::move(f)), args(std::forward<Args>(a)...)
        {}
        WrapperFunctor(const WrapperFunctor& other) = delete;
/*             : WrapperFunctor(std::move(other))
        {} */
        WrapperFunctor(WrapperFunctor&& other)
            : promise(std::move(other.promise)), real_task(std::move(other.real_task)), args(std::move(other.args))
        {}

        void operator()(void)
        {
            try
            {
                if constexpr (std::same_as<std::remove_cv_t<ReturnType>, void>)
                {
                    std::apply(std::move(this->real_task), std::move(this->args));
                    promise.set_value();
                }
                else
                {
                    promise.set_value(std::apply(std::move(this->real_task), std::move(this->args)));
                }
            }
            catch (const typename real_task_t::TimedOut& e)
            {
                promise.set_exception(std::make_exception_ptr(TaskTimeoutError()));
            }
            catch (...)
            {
                promise.set_exception(std::current_exception());
            }
        }
    };
#endif

#if 1
    function_type task =
        [
            p = promise,
            f = std::move(real_task),
            ... a = std::forward<Args>(args)
        ]() mutable
        {
            try
            {
                if constexpr (std::same_as<std::remove_cv_t<ReturnType>, void>)
                {
                    std::invoke(std::move(f), std::move(a)...);
                    p->set_value();
                }
                else
                {
                    p->set_value(std::invoke(std::move(f), std::move(a)...));
                }
            }
            catch (const typename real_task_t::TimedOut& e)
            {
                p->set_exception(std::make_exception_ptr(TaskTimeoutError()));
            }
            catch (...)
            {
                p->set_exception(std::current_exception());
            }
        };
#else
    function_type task(
        WrapperFunctor(
            std::move(promise),
            std::move(real_task),
            std::forward<Args>(args)...
        )
    );
#endif
    LOCK_GUARD(READ_WRITE) waiting_locations_lock(this, this->waiting_locations);
    LOCK_GUARD(READ_WRITE) task_queues_lock(this, this->task_queues);
    LOCK_GUARD(READ_WRITE) threads_lock(this, this->threads);
    this->task_queues[this->get_least_busy_thread()].push(std::move(task));
    return future;
}

#endif