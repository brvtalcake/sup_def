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

#ifndef FS_WATCH_HPP
#define FS_WATCH_HPP

#include <sup_def/common/util/util.hpp>

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/inotify.h>
#include <sys/fanotify.h>
#include <sys/ioctl.h>
#include <sys/signalfd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>

#include <bits/stdc++.h>
#include <bits/extc++.h>

namespace fswatch
{
    namespace detail
    {
        namespace fanotify_impl
        {
            using event_t = struct ::fanotify_event_metadata;
            using event_info_header_t = struct ::fanotify_event_info_header;
            using event_info_fid_t = struct ::fanotify_event_info_fid;
            using event_info_pidfd_t = struct ::fanotify_event_info_pidfd;
            using event_info_error_t = struct ::fanotify_event_info_error;
            using response_t = struct ::fanotify_response;
            using response_info_header_t = struct ::fanotify_response_info_header;
            using response_info_audit_rule_t = struct ::fanotify_response_info_audit_rule;
        }
    }

    class fanotify_watcher
    {
        using file_descriptor_t = int;
        using fanotify_group_t = int;

        using namespace detail::fanotify_impl;

        template <bool is_self_const, bool is_self_volatile>
        using self_t = ::SupDef::Util::MakeCvIf<fanotify_watcher, is_self_const, is_self_volatile>;

        template <bool is_self_const, bool is_self_volatile>
        using self_ptr_t = ::SupDef::Util::pointer_t<self_t<is_self_const, is_self_volatile>>;

        template <bool is_self_const, bool is_self_volatile>
        using self_lref_t = ::SupDef::Util::lref_t<self_t<is_self_const, is_self_volatile>>;

        template <bool is_self_const, bool is_self_volatile>
        using self_rref_t = ::SupDef::Util::rref_t<self_t<is_self_const, is_self_volatile>>;

        public:
            fanotify_watcher();
            ~fanotify_watcher();

            self_lref_t<false, false> watch(file_descriptor_t fd);
            self_lref_t<false, false> unwatch(file_descriptor_t fd);
        
        private:
            fanotify_group_t m_fanotify_queue;

            std::set<file_descriptor_t> m_watched_fds;
            std::mutex m_watched_fds_mutex;

            static std::set<file_descriptor_t> s_watched_fds;
            static std::mutex s_watched_fds_mutex;
    };
}

#endif
