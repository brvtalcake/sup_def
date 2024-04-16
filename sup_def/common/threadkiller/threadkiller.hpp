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

#ifndef THREADKILLER_HPP
#define THREADKILLER_HPP

#include <sup_def/common/util/util.hpp>
#include <csignal>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

namespace threadkiller
{
    namespace detail
    {
        enum class implementation
        {
            linux_gettid_sigaction_eh, // gettid + sigaction + tgkill + throw + catch
            linux_pthreadkill_sigaction_eh, // sigaction + pthread_kill + throw + catch
            linux_pthreadcancel, // pthread_cancel
            linux_gettid_sigaction_setjmp_longjmp, // gettid + sigaction + tgkill + setjmp + longjmp

            portable_atomicselection_signal_eh, // raise + signal + throw when thread selected + catch
            portable_atomicselection_signal_setjmp_longjmp, // raise + signal + setjmp + longjmp when thread selected

            other /* TODO */
        };

        template <typename = void>
        concept has_gettid_func = requires {
            { ::gettid() } -> std::same_as<pid_t>;
        };

        template <typename = void>
        concept has_syscall_gettid = requires {
            { ::syscall(SYS_gettid) } -> std::convertible_to<pid_t>;
        };
        
        template <typename = void>
        concept has_pthread_cancel = requires {
            { ::pthread_cancel(std::declval<pthread_t>()) } -> std::same_as<int>;
        };

        template <typename = void>
        concept has_pthread_kill = requires {
            { ::pthread_kill(std::declval<pthread_t>(), std::declval<int>()) } -> std::same_as<int>;
        };

        template <typename = void>
        concept has_struct_sigaction = requires(struct sigaction sa) {
        };

        template <typename = void>
        concept has_sigaction_func = requires {
            { ::sigaction(std::declval<int>(), std::declval<const struct sigaction*>(), std::declval<struct sigaction*>()) } -> std::same_as<int>;
        };

        template <typename = void>
        concept has_syscall_sigaction = requires {
            {
                ::syscall(
                    SYS_rt_sigaction,
                    std::declval<int>(),
                    std::declval<const struct sigaction*>(),
                    std::declval<struct sigaction*>(),
                    std::declval<size_t>()
                )
            } -> std::convertible_to<int>;
        };

        template <typename = void>
        concept has_tgkill_func = requires {
            { ::tgkill(std::declval<pid_t>(), std::declval<pid_t>(), std::declval<int>()) } -> std::same_as<int>;
        };

        template <typename = void>
        concept has_syscall_tgkill = requires {
            { ::syscall(SYS_tgkill, std::declval<pid_t>(), std::declval<pid_t>(), std::declval<int>()) } -> std::convertible_to<int>;
        };

        struct thread_handle_type
            : public std::type_identity<std::thread::native_handle_type>
        { };
        using thread_handle_t = thread_handle_type::type;

#if 0
        static_assert(std::bool_constant<has_gettid_func<>>::value);
        static_assert(std::bool_constant<has_syscall_gettid<>>::value);
        static_assert(std::bool_constant<has_pthread_cancel<>>::value);
        static_assert(std::bool_constant<has_pthread_kill<>>::value);
        static_assert(std::bool_constant<has_struct_sigaction<>>::value);
        static_assert(std::bool_constant<has_sigaction_func<>>::value);
        static_assert(std::bool_constant<has_syscall_sigaction<>>::value);
        static_assert(std::bool_constant<has_tgkill_func<>>::value);
        static_assert(std::bool_constant<has_syscall_tgkill<>>::value);
        static_assert(std::bool_constant<std::same_as<thread_handle_t, pthread_t>>::value);
#endif

        template <implementation Impl>
        static constexpr bool use_impl;

        template <>
        static constexpr bool use_impl<implementation::linux_gettid_sigaction_eh> = bool(
            ( has_gettid_func<>    || has_syscall_gettid<> )    &&
             has_struct_sigaction<>                             &&
            ( has_sigaction_func<> || has_syscall_sigaction<> ) &&
            ( has_tgkill_func<>    || has_syscall_tgkill<> )    &&
            (__cpp_exceptions >= 199711L)
        );

        template <>
        static constexpr bool use_impl<implementation::linux_pthreadkill_sigaction_eh> = bool(
            has_struct_sigaction<>                              &&
            ( has_sigaction_func<> || has_syscall_sigaction<> ) &&
            std::same_as<thread_handle_t, pthread_t>            &&
            has_pthread_kill<>                                  &&
            (__cpp_exceptions >= 199711L)
        );

        template <>
        static constexpr bool use_impl<implementation::linux_pthreadcancel> = bool(
            std::same_as<thread_handle_t, pthread_t> &&
            has_pthread_cancel<>
        );

        template <>
        static constexpr bool use_impl<implementation::linux_gettid_sigaction_setjmp_longjmp> = bool(
            /* TODO */
        );

        static constexpr inline implementation select_implementation_helper = [] constexpr {
            return implementation::linux_gettid_sigaction_eh;
        }();
        struct select_implementation
            : public std::integral_constant<implementation, select_implementation_helper>
        { };
    }
    void kill(std::thread::id id)
    {
        
    }
}

#endif
