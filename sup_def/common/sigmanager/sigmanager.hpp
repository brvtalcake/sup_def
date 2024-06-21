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

#include <sup_def/common/start_header.h>

#include <sup_def/common/util/util.hpp>
#include <function2/function2.hpp>

namespace sigmgr
{
    typedef ::pid_t pid_t;
    typedef ::pid_t tid_t;

    template <typename Signature>
    using function      = fu2::function_base<true,  true, fu2::capacity_default, true, false, Signature>;
    template <typename Signature>
    using function_view = fu2::function_base<false, true, fu2::capacity_default, true, false, Signature>;

    typedef function<void(int) noexcept> simple_handler_t;
    typedef function<void(int, siginfo_t*, ucontext_t*) noexcept> handler_t;
    typedef std::variant<simple_handler_t, handler_t> generic_handler_t;

    typedef function<void(const siginfo_t*, const ucontext_t*) noexcept> callback_t;
    
    using callback_iterator       = std::list<callback_t>::iterator;
    using const_callback_iterator = std::list<callback_t>::const_iterator;

    using callback_id_t = std::tuple<int, std::string, callback_iterator>;

    void init() noexcept;

    warn_unused_result()
    std::optional<callback_id_t> register_callback(const int sig, const std::string& id, const callback_t& callback) noexcept;
    warn_unused_result()
    bool unregister_callback(const callback_id_t& id) noexcept;
    void reset_callbacks(const int sig, const std::string& id) noexcept;
}

#endif

#include <sup_def/common/end_header.h>