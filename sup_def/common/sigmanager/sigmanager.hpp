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

#ifndef SIGMANAGER_HPP
#define SIGMANAGER_HPP

#include <sup_def/common/util/util.hpp>

namespace sigmgr
{
    typedef decltype(::gettid()) tid_t;

    void init() noexcept;

    bool is_rtsig_usable() noexcept;
    bool is_rtsig_usable(const int sig) noexcept;

    bool is_sigusr_usable() noexcept;
    bool is_sigusr_usable(const int sig) noexcept;

    std::pair<bool, int> register_rtsig_use(const std::string& id) noexcept;
    warn_usage_suggest_alternative("register_rtsig_use(const std::string&)")
    bool register_rtsig_use(const std::string& id, const int sig) noexcept;

    void unregister_rtsig_use(const std::string& id) noexcept;
    void unregister_rtsig_use(const std::string& id, const int sig) noexcept;
    warn_usage_suggest_alternative(
        "unregister_rtsig_use(const std::string&, const int)",
        "unregister_rtsig_use(const std::string&)"
    )
    void unregister_rtsig_use(const int sig) noexcept;


    std::pair<bool, int> register_sigusr_use(const std::string& id) noexcept;
    warn_usage_suggest_alternative("register_sigusr_use(const std::string&)")
    bool register_sigusr_use(const std::string& id, const int sig) noexcept;

    void unregister_sigusr_use(const std::string& id) noexcept;
    void unregister_sigusr_use(const std::string& id, const int sig) noexcept;
    warn_usage_suggest_alternative(
        "unregister_sigusr_use(const std::string&, const int)",
        "unregister_sigusr_use(const std::string&)"
    )
    void unregister_sigusr_use(const int sig) noexcept;

    std::set<int> uses(const std::string& id) noexcept;

    tid_t get_signaled_thread_tid() noexcept;
    // Needs to be called from main thread
    warn_usage_suggest_alternative("sigmgr::init()")
    void start_signaled_thread() noexcept;
    void stop_signaled_thread() noexcept;

    /* namespace detail
    {
        void assert_preconditions() noexcept;
    } */
}

#endif
