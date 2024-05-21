#include <sup_def/common/config.h>
#include <sup_def/common/util/util.hpp>

namespace SupDef
{
    namespace Util
    {
        namespace
        {
            template <typename Fn, Fn... fns>
            no_return
            static void adapt_fns() noexcept
            {
                static constexpr std::array fns_array = { fns... };
                try
                {
                    for (auto& fn : fns_array)
                    {
                        fn();
                    }
                }
                catch (...)
                {
                    std::abort();
                }
            }
        }

        symbol_unused
        symbol_keep
        static_init_priority(101)
        static std::mutex destructors_mutex;

        symbol_unused
        symbol_keep
        static_init_priority(101)
        static std::vector<std::function<void()>> destructors = {
            []() { }
        };

        symbol_unused
        symbol_keep
        static_init_priority(101)
        static std::atomic<bool> destructors_called = false;

        void call_destructors()
        {
            std::unique_lock lock(destructors_mutex, std::defer_lock);
            if (destructors_called.load(std::memory_order::acquire) == true)
                return;
            lock.lock();
            for (auto& destructor : destructors)
            {
                destructor();
            }
            destructors_called.store(true, std::memory_order::release);
        }

        void add_destructor(const std::function<void()>& destructor) noexcept
        {
            std::unique_lock lock(destructors_mutex);
            destructors.push_back(destructor);
        }

        void set_cleanup_handlers() noexcept
        {
            // For "normal" exit (actually not needed)
            /* std::atexit(adapt_fns<decltype(&call_destructors), &call_destructors>); */

            // For "abnormal" exit
            std::at_quick_exit(&adapt_fns<decltype(&call_destructors), &call_destructors>);
            std::set_terminate(&adapt_fns<decltype(&call_destructors), &call_destructors, &stdgnu::__verbose_terminate_handler, &std::abort>);
        }
    }
};