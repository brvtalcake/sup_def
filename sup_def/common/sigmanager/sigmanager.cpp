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

#include <sup_def/common/config.h>
#include <sup_def/common/sigmanager/sigmanager.hpp>
#include <boost/callable_traits.hpp>
#include <chaos/preprocessor/repetition/enum_from_to.h>
#include <chaos/preprocessor/arithmetic/add.h>
#include <chaos/preprocessor/arithmetic/sub.h>
#include <chaos/preprocessor/comparison/max.h>
#include <chaos/preprocessor/comparison/min.h>
#include <csignal>
#include <list>
#include <map>
#include <mutex>
#include <set>
#include <thread>
#include <variant>


#undef MAX_SIGUSR
#define MAX_SIGUSR CHAOS_PP_MAX(SIGUSR1, SIGUSR2)

#undef MIN_SIGUSR
#define MIN_SIGUSR CHAOS_PP_MIN(SIGUSR1, SIGUSR2)

#undef SIGUSR1_SIGUSR2_ARE_FOLLOWING
#define SIGUSR1_SIGUSR2_ARE_FOLLOWING   \
    CHAOS_PP_EQUAL(                     \
        CHAOS_PP_SUB(                   \
            MAX_SIGUSR,                 \
            MIN_SIGUSR                  \
        ),                              \
        1                               \
    )

#undef IN_BEWTEEN_SIGUSR1_SIGUSR2_COUNT
#define IN_BEWTEEN_SIGUSR1_SIGUSR2_COUNT    \
    CHAOS_PP_DEC(                           \
        CHAOS_PP_SUB(                       \
            MAX_SIGUSR,                     \
            MIN_SIGUSR                      \
        )                                   \
    )

// From GLIBC source code:
//  sysdeps/unix/sysv/linux/internal-signals.h
#if __SIGRTMIN > 0 && __SIGRTMAX > 0
    #undef RESERVED_FOR_PTHREADS
    #define RESERVED_FOR_PTHREADS 2

    #ifndef SIGCANCEL
        #define SIGCANCEL __SIGRTMIN
    #endif

    #ifndef SIGTIMER
        #define SIGTIMER SIGCANCEL
    #endif

    #ifndef SIGSETXID
        #define SIGSETXID (__SIGRTMIN + 1)
    #endif
#endif

namespace
{
    template<typename...>
    concept have_tgkill = requires(sigmgr::pid_t pid, sigmgr::tid_t tid, int sig)
    {
        { ::tgkill(pid, tid, sig) } -> std::same_as<int>;
    };

    template<typename...>
    concept have_pthread_kill = requires(pthread_t pthread, int sig)
    {
        { ::pthread_kill(pthread, sig) } -> std::same_as<int>;
    };
}

namespace sigmgr
{
    class client_callbacks
    {
        public:
            client_callbacks() = delete;

            constexpr client_callbacks(const std::string& id) noexcept
                : id(id)
            { }
            constexpr client_callbacks(std::string&& id) noexcept
                : id(std::move(id))
            { }

            client_callbacks(const client_callbacks&) = delete;
            client_callbacks(client_callbacks&& other) noexcept
                : id(std::move(other.id)), callbacks(std::move(other.callbacks))
            { }

            client_callbacks& operator=(const client_callbacks&) = delete;
            client_callbacks& operator=(client_callbacks&& other) noexcept
            {
                this->id = std::move(other.id);
                this->callbacks = std::move(other.callbacks);
                return *this;
            }

            ~client_callbacks() noexcept
            {
                this->reset_callbacks();
            }

            callback_iterator add_callback(const callback_t& callback) noexcept
            {
                return this->callbacks.insert(this->callbacks.cend(), callback);
            }
            callback_iterator add_callback(callback_t&& callback) noexcept
            {
                return this->callbacks.insert(this->callbacks.cend(), std::move(callback));
            }
            void remove_callback(const callback_iterator& it) noexcept
            {
                this->callbacks.erase(it);
            }
            void reset_callbacks() noexcept
            {
                this->callbacks.clear();
            }

            constexpr const std::string& get_id() const noexcept
            {
                return this->id;
            }

            constexpr std::strong_ordering operator<=>(const client_callbacks& other) const noexcept
            {
                return this->id <=> other.id;
            }
            constexpr std::strong_ordering operator<=>(const std::string& other) const noexcept
            {
                return this->id <=> other;
            }

            constexpr callback_iterator begin() noexcept
            {
                return this->callbacks.begin();
            }
            constexpr const_callback_iterator begin() const noexcept
            {
                return this->cbegin();
            }
            constexpr const_callback_iterator cbegin() const noexcept
            {
                return this->callbacks.cbegin();
            }

            constexpr callback_iterator end() noexcept
            {
                return this->callbacks.end();
            }
            constexpr const_callback_iterator end() const noexcept
            {
                return this->cend();
            }
            constexpr const_callback_iterator cend() const noexcept
            {
                return this->callbacks.cend();
            }
            
        private:
            std::string id;
            std::list<callback_t> callbacks;
    };

#undef SIGMGR_DEFINE_SIGNAL_ENTRY
#define SIGMGR_DEFINE_SIGNAL_ENTRY(sig, ...)    \
    CHAOS_PP_COMMA_IF(                          \
        PP_IF(                                  \
            ISEMPTY(__VA_ARGS__)                \
        )                                       \
        (1)                                     \
        (FIRST_ARG(__VA_ARGS__))                \
    ) (sig)

    static std::set<int> allsigs{
#ifdef SIGABRT
        SIGMGR_DEFINE_SIGNAL_ENTRY(SIGABRT, 0)
#endif

#ifdef SIGALRM
        SIGMGR_DEFINE_SIGNAL_ENTRY(SIGALRM)
#endif

#ifdef SIGBUS
        SIGMGR_DEFINE_SIGNAL_ENTRY(SIGBUS)
#endif

#ifdef SIGCHLD
        SIGMGR_DEFINE_SIGNAL_ENTRY(SIGCHLD)
#endif

#ifdef SIGCLD
        SIGMGR_DEFINE_SIGNAL_ENTRY(SIGCLD)
#endif

#ifdef SIGCONT
        SIGMGR_DEFINE_SIGNAL_ENTRY(SIGCONT)
#endif

#ifdef SIGEMT
        SIGMGR_DEFINE_SIGNAL_ENTRY(SIGEMT)
#endif

#ifdef SIGFPE
        SIGMGR_DEFINE_SIGNAL_ENTRY(SIGFPE)
#endif

#ifdef SIGHUP
        SIGMGR_DEFINE_SIGNAL_ENTRY(SIGHUP)
#endif

#ifdef SIGILL
        SIGMGR_DEFINE_SIGNAL_ENTRY(SIGILL)
#endif

#ifdef SIGINFO
        SIGMGR_DEFINE_SIGNAL_ENTRY(SIGINFO)
#endif

#ifdef SIGINT
        SIGMGR_DEFINE_SIGNAL_ENTRY(SIGINT)
#endif

#ifdef SIGIO
        SIGMGR_DEFINE_SIGNAL_ENTRY(SIGIO)
#endif

#ifdef SIGIOT
        SIGMGR_DEFINE_SIGNAL_ENTRY(SIGIOT)
#endif

#ifdef SIGLOST
        SIGMGR_DEFINE_SIGNAL_ENTRY(SIGLOST)
#endif

#ifdef SIGPIPE
        SIGMGR_DEFINE_SIGNAL_ENTRY(SIGPIPE)
#endif

#ifdef SIGPOLL
        SIGMGR_DEFINE_SIGNAL_ENTRY(SIGPOLL)
#endif

#ifdef SIGPROF
        SIGMGR_DEFINE_SIGNAL_ENTRY(SIGPROF)
#endif

#ifdef SIGPWR
        SIGMGR_DEFINE_SIGNAL_ENTRY(SIGPWR)
#endif

#ifdef SIGQUIT
        SIGMGR_DEFINE_SIGNAL_ENTRY(SIGQUIT)
#endif

#ifdef SIGSEGV
        SIGMGR_DEFINE_SIGNAL_ENTRY(SIGSEGV)
#endif

#ifdef SIGSTKFLT
        SIGMGR_DEFINE_SIGNAL_ENTRY(SIGSTKFLT)
#endif

#ifdef SIGTSTP
        SIGMGR_DEFINE_SIGNAL_ENTRY(SIGTSTP)
#endif

#ifdef SIGSYS
        SIGMGR_DEFINE_SIGNAL_ENTRY(SIGSYS)
#endif

#ifdef SIGTERM
        SIGMGR_DEFINE_SIGNAL_ENTRY(SIGTERM)
#endif

#ifdef SIGTRAP
        SIGMGR_DEFINE_SIGNAL_ENTRY(SIGTRAP)
#endif

#ifdef SIGTTIN
        SIGMGR_DEFINE_SIGNAL_ENTRY(SIGTTIN)
#endif

#ifdef SIGTTOU
        SIGMGR_DEFINE_SIGNAL_ENTRY(SIGTTOU)
#endif

#ifdef SIGUNUSED
        SIGMGR_DEFINE_SIGNAL_ENTRY(SIGUNUSED)
#endif

#ifdef SIGURG
        SIGMGR_DEFINE_SIGNAL_ENTRY(SIGURG)
#endif

#ifdef SIGUSR1
        SIGMGR_DEFINE_SIGNAL_ENTRY(SIGUSR1)
#endif

#ifdef SIGUSR2
        SIGMGR_DEFINE_SIGNAL_ENTRY(SIGUSR2)
#endif

#ifdef SIGVTALRM
        SIGMGR_DEFINE_SIGNAL_ENTRY(SIGVTALRM)
#endif

#ifdef SIGXCPU
        SIGMGR_DEFINE_SIGNAL_ENTRY(SIGXCPU)
#endif

#ifdef SIGXFSZ
        SIGMGR_DEFINE_SIGNAL_ENTRY(SIGXFSZ)
#endif

#ifdef SIGWINCH
        SIGMGR_DEFINE_SIGNAL_ENTRY(SIGWINCH)
#endif

    };
#undef SIGMGR_DEFINE_SIGNAL_ENTRY

    enum class management_type : uint_fast8_t
    {
        DEFAULT = 0, // Default signal management (i.e. SIG_DFL)
        USERDEFINED, // User provides a custom signal handler
        CALLBACKS    // User provides callbacks that we call when the signal is received
    };

    static std::atomic<bool> initialized = false;

    static std::atomic<tid_t> signaled_thread_tid;
    static std::atomic<pthread_t> signaled_thread_pthread;
    static std::thread signaled_thread;

    static std::multimap<int, client_callbacks> signal_callbacks;
    static std::mutex signal_callbacks_mutex;

    static std::map<int, generic_handler_t> signal_handlers;
    static std::mutex signal_handlers_mutex;

    static std::map<int, management_type> managed;
    static std::mutex managed_mutex;

    namespace
    {
        const std::set<int>& get_all_signals() noexcept
        {
            return allsigs;
        }
    }
    namespace detail
    {
        namespace
        {
            static void call_client_callbacks(int sig, siginfo_t* info, ucontext_t* context) noexcept
            {
                auto range = signal_callbacks.equal_range(sig);
                for (auto it = range.first; it != range.second; ++it)
                {
                    for (callback_t& callback : it->second)
                    {
                        if (callback)
                            callback(info, context);
                    }
                }
            }

            static void generic_handler(int sig, siginfo_t* info, ucontext_t* context) noexcept
            {
                unlikely_if (!allsigs.contains(sig))
                    std::terminate(); // Shall not be installed for this signal

                std::scoped_lock lock(signal_handlers_mutex, managed_mutex, signal_callbacks_mutex);

                switch (managed[sig])
                {
                    case management_type::DEFAULT:
                        std::terminate(); // Shall not be called
                        break;
                    case management_type::USERDEFINED:
                        std::visit(
                            [&](auto&& handler)
                            {
                                if constexpr (std::same_as<
                                    std::remove_cvref_t<decltype(handler)>,
                                    simple_handler_t
                                >) { handler(sig); }
                                else
                                {
                                    static_assert(std::same_as<
                                        std::remove_cvref_t<decltype(handler)>,
                                        handler_t
                                    >);
                                    handler(sig, info, context);
                                }
                            },
                            signal_handlers[sig]
                        );
                        break;
                    case management_type::CALLBACKS:
                        call_client_callbacks(sig, info, context);
                        break;
                }
            }
    
            symbol_cold
            static void assert_preconditions() noexcept
            {
                hard_assert(allsigs.size() == 31);
                hard_assert(::getpid() == ::gettid());

                hard_assert(       0 < SIGRTMIN);
                hard_assert(       0 < SIGRTMAX);
                hard_assert(SIGRTMIN < SIGRTMAX);

                const int& curr_max = *stdrg::max_element(allsigs);

#if __SIGRTMIN > 0 && __SIGRTMAX > 0
                hard_assert(__SIGRTMIN < SIGRTMIN);
                hard_assert(SIGRTMIN == __SIGRTMIN + RESERVED_FOR_PTHREADS);
                hard_assert(SIGRTMAX == __SIGRTMAX);
                hard_assert(curr_max < __SIGRTMIN);
#endif
                hard_assert(curr_max < SIGRTMAX);
                hard_assert(curr_max < SIGRTMIN);
            }
        }

        symbol_cold
        static void init() noexcept
        {
            assert_preconditions();

            for (int s = SIGRTMIN; s <= SIGRTMAX; ++s)
                allsigs.insert(s);

            for (const int& sig : allsigs)
                managed[sig] = management_type::DEFAULT;
        }

        
        /// @pre `signal_handlers_mutex` is locked
        static void install_signal_handler_for(
            const int sig,
            const bool default_handler = true,
            generic_handler_t&& handler = generic_handler_t(&generic_handler)
        ) noexcept
        {
            struct ::sigaction sa{0};

            if (!default_handler)
                signal_handlers[sig] = std::move(handler);

            static const auto adapter = [](int sig, siginfo_t* info, void* context) static noexcept -> void
            {
                generic_handler(sig, info, static_cast<ucontext_t*>(context));
            };
            sa.sa_sigaction = static_cast<void (*)(int, siginfo_t*, void*)>(adapter);
            sa.sa_flags = SA_SIGINFO;

            TRY_SYSCALL_WHILE_EINTR(::sigaction, (sig, &sa, nullptr));
        }

        /// @pre `signal_handlers_mutex` is locked
        static void uninstall_signal_handler_for(const int sig) noexcept
        {
            struct ::sigaction sa{0};

            signal_handlers.erase(sig);

            sa.sa_handler = SIG_DFL;

            TRY_SYSCALL_WHILE_EINTR(::sigaction, (sig, &sa, nullptr));
        }

        static void mask_all_signals(bool while_init) noexcept
        {
            sigset_t mask{0};

            TRY_SYSCALL_WHILE_EINTR(::sigemptyset, (&mask));

            for (const int& sig : get_all_signals())
                TRY_SYSCALL_WHILE_EINTR(::sigaddset, (&mask, sig));

            if (while_init)
                TRY_SYSCALL_WHILE_EINTR(::sigprocmask, (SIG_BLOCK, &mask, nullptr));
            else
                TRY_SYSCALL_WHILE_EINTR(::pthread_sigmask, (SIG_BLOCK, &mask, nullptr));
        }

        static void unmask_all_signals() noexcept
        {
            sigset_t mask{0};

            TRY_SYSCALL_WHILE_EINTR(::sigemptyset, (&mask));

            for (const int& sig : get_all_signals())
                TRY_SYSCALL_WHILE_EINTR(::sigaddset, (&mask, sig));

            TRY_SYSCALL_WHILE_EINTR(::pthread_sigmask, (SIG_UNBLOCK, &mask, nullptr));
        }
    }

    warn_unused_result()
    std::optional<callback_id_t> register_callback(const int sig, const std::string& id, const callback_t& callback) noexcept
    {
        unlikely_if (!get_all_signals().contains(sig))
            return std::nullopt;

        std::scoped_lock lock(signal_callbacks_mutex, managed_mutex, signal_handlers_mutex);

        switch (managed[sig])
        {
            case management_type::DEFAULT:
                managed[sig] = management_type::CALLBACKS;
                detail::install_signal_handler_for(sig);
                break;
            case management_type::USERDEFINED:
                return std::nullopt;
            case management_type::CALLBACKS:
                break;
        }

        // With libstdc++: mm_iter_t = std::_Rb_tree_iterator<std::pair<const int, sigmgr::client_callbacks>>
        using mm_iter_t = decltype(signal_callbacks)::iterator;
        std::pair<mm_iter_t, mm_iter_t> eqrange = signal_callbacks.equal_range(sig);
        // First element equal to sig, or end() if key not found
        mm_iter_t sigfirst = std::move(eqrange.first);
        // One past the last element equal to sig, or end() if key not found
        mm_iter_t siglast  = std::move(eqrange.second);

        mm_iter_t found    = sigfirst;
        while (
            found != signal_callbacks.end() &&
            found != siglast                &&
            found->second.get_id() != id
        ) { ++found; }
        
        if (found == signal_callbacks.end() || found == siglast || found->second.get_id() != id)
        {
            auto ret = signal_callbacks.emplace(sig, client_callbacks(id));
            return std::make_optional(callback_id_t(sig, id, ret->second.add_callback(callback)));
        }
        else
        {
            // Verify we do not have another one matching the same id
            // Normally should be either:
            // - a callback record for the same sig but different id if found was not the last of the callbacks for sig
            // - end of the multimap if no other callback for the same sig and no callbacks registered for signals past sig
            // - a callback record for next sig in the multimap if found was the last of the callbacks for sig
            mm_iter_t next = found;
            ++next;

            unlikely_if (
                next != signal_callbacks.end() &&
                next != siglast                &&
                next->second.get_id() == id
            )
            {
                // Shall not happen if we do our job correctly and append the
                // potentially new callbacks to the end of the list of the
                // corresponding client_callbacks object
                std::cerr << "Callback with id " << id << " exists multiple times for signal " << sig << std::endl;
                std::terminate();
            }
            return std::make_optional(callback_id_t(sig, id, found->second.add_callback(callback)));
        }
    }

    warn_unused_result()
    bool unregister_callback(const callback_id_t& cbid) noexcept
    {
        const int& sig                            = std::get<0>(cbid);
        const std::string& id                     = std::get<1>(cbid);
        const std::list<callback_t>::iterator& it = std::get<2>(cbid);

        unlikely_if (!get_all_signals().contains(sig))
            return false;

        std::scoped_lock lock(signal_callbacks_mutex, managed_mutex, signal_handlers_mutex);

        using mm_iter_t    = decltype(signal_callbacks)::iterator;
        
        std::pair<mm_iter_t, mm_iter_t>
            eqrange        = signal_callbacks.equal_range(sig);

        mm_iter_t sigfirst = std::move(eqrange.first);
        mm_iter_t siglast  = std::move(eqrange.second);

        mm_iter_t found = sigfirst;
        while (
            found != signal_callbacks.end() &&
            found != siglast                &&
            found->second.get_id() != id
        ) { ++found; }

        unlikely_if (found == signal_callbacks.end() || found == siglast || found->second.get_id() != id)
            return false;

        found->second.remove_callback(it);

        if (found->second.begin() == found->second.end())
        {
            signal_callbacks.erase(found);

            eqrange  = signal_callbacks.equal_range(sig);
            sigfirst = std::move(eqrange.first);
            siglast  = std::move(eqrange.second);
            
            if (std::distance(sigfirst, siglast) < 1)
            {
                managed[sig] = management_type::DEFAULT;
                detail::uninstall_signal_handler_for(sig);
            }
        }

        return true;
    }
    void reset_callbacks(const int sig, const std::string& id) noexcept
    {
        unlikely_if (!get_all_signals().contains(sig))
            return;

        std::scoped_lock lock(signal_callbacks_mutex, managed_mutex, signal_handlers_mutex);

        using mm_iter_t = decltype(signal_callbacks)::iterator;

        std::pair<mm_iter_t, mm_iter_t> eqrange = signal_callbacks.equal_range(sig);
        mm_iter_t sigfirst = std::move(eqrange.first);
        mm_iter_t siglast  = std::move(eqrange.second);

        mm_iter_t found = sigfirst;
        while (
            found != signal_callbacks.end() &&
            found != siglast                &&
            found->second.get_id() != id
        ) { ++found; }

        unlikely_if (found == signal_callbacks.end() || found == siglast || found->second.get_id() != id)
            return;

        found->second.reset_callbacks();

        signal_callbacks.erase(found);
        managed[sig] = management_type::DEFAULT;
        detail::uninstall_signal_handler_for(sig);
    }
    void reset_callbacks(const std::string& id) noexcept
    {
        std::scoped_lock lock(signal_callbacks_mutex, managed_mutex, signal_handlers_mutex);

        using mm_iter_t = decltype(signal_callbacks)::iterator;
        mm_iter_t found = signal_callbacks.begin();
        auto fdif = [&id](const mm_iter_t::reference pair) noexcept -> bool
        {
            return pair.second.get_id() == id;
        };
        while (
            (found = std::find_if(found, signal_callbacks.end(), fdif)) != signal_callbacks.end()
        )
        {
            int sig = found->first;
            found->second.reset_callbacks();
            found = signal_callbacks.erase(found);
            if (signal_callbacks.count(sig) < 1)
            {
                managed[sig] = management_type::DEFAULT;
                detail::uninstall_signal_handler_for(sig);
            }
        }
    }

    warn_unused_result()
    bool set_signal_handler(const int sig, generic_handler_t&& handler) noexcept
    {
        unlikely_if (!get_all_signals().contains(sig))
            return false;

        std::scoped_lock lock(signal_handlers_mutex, managed_mutex);
        if (managed[sig] != management_type::DEFAULT)
            return false;
        managed[sig] = management_type::USERDEFINED;
        detail::install_signal_handler_for(sig, false, std::move(handler));
        return true;
    }
    
    warn_unused_result()
    bool remove_signal_handler(const int sig) noexcept
    {
        unlikely_if (!get_all_signals().contains(sig))
            return false;

        std::scoped_lock lock(signal_handlers_mutex, managed_mutex);
        if (managed[sig] != management_type::USERDEFINED)
            return false;
        managed[sig] = management_type::DEFAULT;
        detail::uninstall_signal_handler_for(sig);
        return true;
    }

    namespace
    {
        no_return
        static void signaled_thread_func() noexcept
        {
            tid_t tid = TRY_SYSCALL_WHILE_EINTR(::gettid, ());
            signaled_thread_tid.store(tid, std::memory_order::release);

            signaled_thread_pthread.store(::pthread_self(), std::memory_order::release);

            detail::unmask_all_signals();

            while (true)
            {
                ::pause();
            }
        }
    }

    void init() noexcept
    {
        detail::init();

        detail::mask_all_signals(true);
        
        signaled_thread = std::thread(signaled_thread_func);
        signaled_thread.detach();

        initialized.store(true, std::memory_order::release);
    }
}
