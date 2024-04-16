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

#include <sup_def/common/util/util.hpp>
#include <cstdlib>
#include <memory>
#include <cxxabi.h>
#include <boost/container/flat_set.hpp>

namespace SupDef
{
    namespace Util
    {
        //symbol_unused
        //symbol_keep
        //static InitDeinit _init_deinit;

        warn_unused_result()
        std::string demangle(std::string s)
        {
            int status = -1;
            std::unique_ptr<char, decltype(&std::free)> res{
                abi::__cxa_demangle(s.c_str(), nullptr, nullptr, &status),
                std::free
            };
            return (status == 0) ? res.get() : s;
        }

        namespace Detail
        {
            namespace mutex_impl
            {
                using boost::container::flat_set;

                static thread_local std::unordered_map<MutexId, uint_fast64_t> lock_counts;
                static flat_set<MutexId> ids;
                static std::mutex ids_mtx;

                static void init_thread_local(MutexId id)
                {
                    lock_counts[id] = uint_fast64_t(0);
                }

                static void destroy_thread_local(const MutexId& id)
                {
                    size_t ret = std::erase_if(lock_counts, [id](const auto& pair)
                    {
                        auto const& [key, value] = pair;
                        unlikely_if (key == id && value != 0)
                            throw InternalError("Attempted to destroy a locked mutex");
                        return key == id;
                    });
                    hard_assert(ret >= 1);
                }

                static void destroy_thread_local(const MutexId&& id)
                {
                    size_t ret = std::erase_if(lock_counts, [id](const auto& pair)
                    {
                        auto const& [key, value] = pair;
                        unlikely_if (key == id && value != 0)
                            throw InternalError("Attempted to destroy a locked mutex");
                        return key == id;
                    });
                    hard_assert(ret >= 1);
                }

                MutexId gen_new_id(void)
                {
                    auto setup_id = [](MutexId id) -> MutexId
                    {
                        ids.insert(id);
                        init_thread_local(id);
                        return id;
                    };

                    std::unique_lock<std::mutex> lock(ids_mtx);
                    if (ids.empty())
                        return setup_id(MutexId(std::numeric_limits<MutexId>::lowest()));
                    MutexId prev_id = *(ids.begin()); 
                    for (const auto& id : ids)
                    {
                        if (id != prev_id + 1)
                            return setup_id(prev_id + 1);
                        prev_id = id;
                    }
                    return setup_id(prev_id + 1);
                }

                void release_id(const MutexId& id) noexcept
                {
                    try
                    {
                        std::unique_lock<std::mutex> ids_lock(ids_mtx);
                        ids.erase(id);
                        ids_lock.unlock();
                        destroy_thread_local(id);
                    }
                    catch (const InternalError& e)
                    {
                        e.report();
                    }
                    catch (const std::exception& e)
                    {
                        std::cerr << "An exception occurred while releasing a mutex id: " << e.what() << std::endl;
                    }
                    catch (...)
                    {
                        std::cerr << "An unknown exception occurred while releasing a mutex id" << std::endl;
                    }
                }

                void release_id(const MutexId&& id) noexcept
                {
                    try
                    {
                        std::unique_lock<std::mutex> ids_lock(ids_mtx);
                        ids.erase(id);
                        ids_lock.unlock();
                        destroy_thread_local(std::move(id));
                    }
                    catch (const InternalError& e)
                    {
                        e.report();
                    }
                    catch (const std::exception& e)
                    {
                        std::cerr << "An exception occurred while releasing a mutex id: " << e.what() << std::endl;
                    }
                    catch (...)
                    {
                        std::cerr << "An unknown exception occurred while releasing a mutex id" << std::endl;
                    }
                }
                
                void release_ids(const std::vector<MutexId>& to_release) noexcept
                {
                    for (const auto& id : to_release)
                        ::SupDef::Util::Detail::mutex_impl::release_id(id);
                }

                void release_ids(std::vector<MutexId>&& to_release) noexcept
                {
                    for (auto&& id : std::move(to_release))
                        ::SupDef::Util::Detail::mutex_impl::release_id(std::move(id));
                }
                
                bool is_existing_id(const MutexId& id)
                {
                    std::unique_lock<std::mutex> lock(ids_mtx);
                    return ids.contains(id);
                }

                bool is_existing_id(const MutexId&& id)
                {
                    std::unique_lock<std::mutex> lock(ids_mtx);
                    return ids.contains(std::move(id));
                }

                namespace rec_shared
                {
                    void lock_impl(
                        const MutexId&  id,
                        std::shared_mutex& mutex,
                        std::atomic<std::underlying_type_t<enum LockingMode>>& current_mode
                    )
                    {
                        if (lock_counts[id] == 0)
                        {
                            mutex.lock();
                            current_mode.store(
                                std::to_underlying(LockingMode::Exclusive),
                                std::memory_order::release
                            );
                        }
                        unlikely_else_if (
                            current_mode.load(std::memory_order::acquire) ==
                            std::to_underlying(LockingMode::Shared)
                        )
                        {
#if __cpp_lib_formatters >= 202302L
                            std::string thrd_id_string = std::format("{}", std::this_thread::get_id());
#else
                            std::stringstream ss;
                            ss << std::this_thread::get_id();
                            std::string thrd_id_string = ss.str();
#endif
                            using namespace std::string_literals;
                            throw InternalError("Thread "s + thrd_id_string + " attempted to lock a mutex in exclusive mode while holding it in shared mode");
                        }
                        ++lock_counts[id];
                    }
                    void lock_impl(
                        const MutexId&& id,
                        std::shared_mutex& mutex,
                        std::atomic<std::underlying_type_t<enum LockingMode>>& current_mode
                    )
                    {
                        if (lock_counts[id] == 0)
                        {
                            mutex.lock();
                            current_mode.store(
                                std::to_underlying(LockingMode::Exclusive),
                                std::memory_order::release
                            );
                        }
                        unlikely_else_if (
                            current_mode.load(std::memory_order::acquire) ==
                            std::to_underlying(LockingMode::Shared)
                        )
                        {
#if __cpp_lib_formatters >= 202302L
                            std::string thrd_id_string = std::format("{}", std::this_thread::get_id());
#else
                            std::stringstream ss;
                            ss << std::this_thread::get_id();
                            std::string thrd_id_string = ss.str();
#endif
                            using namespace std::string_literals;
                            throw InternalError("Thread "s + thrd_id_string + " attempted to lock a mutex in exclusive mode while holding it in shared mode");
                        }
                        ++lock_counts[std::move(id)];
                    }

                    bool try_lock_impl(
                        const MutexId&  id,
                        std::shared_mutex& mutex,
                        std::atomic<std::underlying_type_t<enum LockingMode>>& current_mode
                    )
                    {
                        if (lock_counts[id] == 0)
                        {
                            unless (mutex.try_lock())
                                return false;
                            current_mode.store(
                                std::to_underlying(LockingMode::Exclusive),
                                std::memory_order::release
                            );
                        }
                        probably_else_if (
                            current_mode.load(std::memory_order::acquire) ==
                            std::to_underlying(LockingMode::Shared),
                            20 /* % probability */,
                            CHAOS /* Backend used internally by probably_* macros */
                        )
                            return false;
                        ++lock_counts[id];
                        return true;
                    }
                    bool try_lock_impl(
                        const MutexId&& id,
                        std::shared_mutex& mutex,
                        std::atomic<std::underlying_type_t<enum LockingMode>>& current_mode
                    )
                    {
                        if (lock_counts[id] == 0)
                        {
                            unless (mutex.try_lock())
                                return false;
                            current_mode.store(
                                std::to_underlying(LockingMode::Exclusive),
                                std::memory_order::release
                            );
                        }
                        probably_else_if (
                            current_mode.load(std::memory_order::acquire) ==
                            std::to_underlying(LockingMode::Shared),
                            20 /* % probability */,
                            CHAOS /* Backend used internally by probably_* macros */
                        )
                            return false;
                        ++lock_counts[std::move(id)];
                        return true;
                    }

                    void unlock_impl(
                        const MutexId&  id,
                        std::shared_mutex& mutex,
                        std::atomic<std::underlying_type_t<enum LockingMode>>& current_mode
                    )
                    {
                        unlikely_if (lock_counts[id] == 0)
                            throw InternalError("Attempted to unlock a mutex that is not locked");
                        --lock_counts[id];
                        probably_if (
                            lock_counts[id] == 0,
                            75 /* % probability */,
                            CHAOS /* Backend used internally by probably_* macros */
                        )
                        {
                            mutex.unlock();
                            current_mode.store(
                                std::to_underlying(LockingMode::None),
                                std::memory_order::release
                            );
                        }
                    }
                    void unlock_impl(
                        const MutexId&& id,
                        std::shared_mutex& mutex,
                        std::atomic<std::underlying_type_t<enum LockingMode>>& current_mode
                    )
                    {
                        unlikely_if (lock_counts[id] == 0)
                            throw InternalError("Attempted to unlock a mutex that is not locked");
                        --lock_counts[id];
                        probably_if (
                            lock_counts[std::move(id)] == 0,
                            75 /* % probability */,
                            CHAOS /* Backend used internally by probably_* macros */
                        )
                        {
                            mutex.unlock();
                            current_mode.store(
                                std::to_underlying(LockingMode::None),
                                std::memory_order::release
                            );
                        }
                    }

                    void lock_shared_impl(
                        const MutexId&  id,
                        std::shared_mutex& mutex,
                        std::atomic<std::underlying_type_t<enum LockingMode>>& current_mode
                    )
                    {
                        if (lock_counts[id] == 0)
                        {
                            mutex.lock_shared();
                            current_mode.store(
                                std::to_underlying(LockingMode::Shared),
                                std::memory_order::release
                            );
                        }
                        ++lock_counts[id];
                    }
                    void lock_shared_impl(
                        const MutexId&& id,
                        std::shared_mutex& mutex,
                        std::atomic<std::underlying_type_t<enum LockingMode>>& current_mode
                    )
                    {
                        if (lock_counts[id] == 0)
                        {
                            mutex.lock_shared();
                            current_mode.store(
                                std::to_underlying(LockingMode::Shared),
                                std::memory_order::release
                            );
                        }
                        ++lock_counts[std::move(id)];
                    }

                    bool try_lock_shared_impl(
                        const MutexId&  id,
                        std::shared_mutex& mutex,
                        std::atomic<std::underlying_type_t<enum LockingMode>>& current_mode
                    )
                    {
                        if (lock_counts[id] == 0)
                        {
                            unless (mutex.try_lock_shared())
                                return false;
                            current_mode.store(
                                std::to_underlying(LockingMode::Shared),
                                std::memory_order::release
                            );
                        }
                        ++lock_counts[id];
                        return true;
                    }
                    bool try_lock_shared_impl(
                        const MutexId&& id,
                        std::shared_mutex& mutex,
                        std::atomic<std::underlying_type_t<enum LockingMode>>& current_mode
                    )
                    {
                        if (lock_counts[id] == 0)
                        {
                            unless (mutex.try_lock_shared())
                                return false;
                            current_mode.store(
                                std::to_underlying(LockingMode::Shared),
                                std::memory_order::release
                            );
                        }
                        ++lock_counts[std::move(id)];
                        return true;
                    }

                    void unlock_shared_impl(
                        const MutexId&  id,
                        std::shared_mutex& mutex,
                        std::atomic<std::underlying_type_t<enum LockingMode>>&
                    )
                    {
                        unlikely_if (lock_counts[id] == 0)
                            throw InternalError("Attempted to unlock a mutex that is not locked");
                        --lock_counts[id];
                        probably_if (
                            lock_counts[id] == 0,
                            75 /* % probability */,
                            CHAOS /* Backend used internally by probably_* macros */
                        )
                            mutex.unlock_shared();
                        /*
                         * No need to update current_mode here, since it may still be
                         * in shared mode if another thread is still holding it in shared mode
                         */
                    }
                    void unlock_shared_impl(
                        const MutexId&& id,
                        std::shared_mutex& mutex,
                        std::atomic<std::underlying_type_t<enum LockingMode>>&
                    )
                    {
                        unlikely_if (lock_counts[id] == 0)
                            throw InternalError("Attempted to unlock a mutex that is not locked");
                        --lock_counts[id];
                        probably_if (
                            lock_counts[std::move(id)] == 0,
                            75 /* % probability */,
                            CHAOS /* Backend used internally by probably_* macros */
                        )
                            mutex.unlock_shared();
                        /*
                         * No need to update current_mode here, since it may still be
                         * in shared mode if another thread is still holding it in shared mode
                         */
                    }

                    bool is_thread_holding_impl(const MutexId& id)
                    {
                        return lock_counts[id] > 0;
                    }
                    bool is_thread_holding_impl(const MutexId&& id)
                    {
                        return lock_counts[std::move(id)] > 0;
                    }
                }

                namespace test
                {
                    bool test_flat_set(void)
                    {
                        std::unique_lock<std::mutex> lock(ids_mtx);
                        
                        auto&& gen_random_id = []() -> MutexId
                        {
                            std::random_device rd;
                            std::mt19937 gen(rd());

                            std::uniform_int_distribution<MutexId> dis(
                                std::numeric_limits<MutexId>::lowest(),
                                std::numeric_limits<MutexId>::max()
                            );
                            MutexId id;
                            while (ids.contains(id = dis(gen)));
                            return id;
                        };
                        
                        std::vector<MutexId> random_ids(100);
                        std::generate(random_ids.begin(), random_ids.end(), gen_random_id);

                        for (const auto& id : random_ids)
                        {
                            ids.insert(id);
                        }
                        
                        for (const auto& id : random_ids)
                        {
                            unless (ids.contains(id))
                            {
                                return false;
                            }
                        }
                        
                        MutexId prev_id = *(ids.begin());
                        for (const auto& id : ids)
                        {
                            if (id < prev_id)
                            {
                                return false;
                            }
                            prev_id = id;
                        }
                        
                        for (const auto& id : random_ids)
                        {
                            ids.erase(id);
                        }
                        
                        for (const auto& id : random_ids)
                        {
                            if (ids.contains(id))
                            {
                                return false;
                            }
                        }
                        
                        return true;
                    }
                }
            }
        }

        void breakpoint()
        {
            if (is_under_debugger())
                asm("int3");
        }

        static bool has_ending(std::string const &fullString, std::string const &ending)
        {
            if (fullString.length() >= ending.length())
                return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
            else
                return false;
        }

        warn_unused_result()
        bool is_under_debugger()
        {
            thread_local static bool result = false;
            thread_local static bool already_executed = false;
            if (already_executed)
                return result;

            /*
            * /proc/self/stat has PID of parent process as fourth parameter.
            */
            std::string stat;
            std::ifstream file("/proc/self/stat");

            for(int i = 0; i < 4; ++i)
                file >> stat;

            std::string parent_path = std::string("/proc/") + stat + "/exe";
            char path[PATH_MAX + 1];
            ::memset(path, 0, PATH_MAX + 1);
            ::readlink(parent_path.c_str(), path, PATH_MAX);

            std::vector<std::string> debuggers = {"gdb", "lldb-server"};

            for (auto& p: debuggers)
            {
                if (has_ending(std::string(path), p))
                {
                    result = true;
                    break;
                }
            }

            already_executed = true;
            return result;
        }
    }
}