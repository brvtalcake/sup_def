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

#if !INCLUDED_FROM_UNISTREAMS_SOURCE
    #error "This file shall not be included from outside the unistreams library"
#endif

namespace uni
{
#if 0
    namespace detail
    {
        template <typename Fn, Fn fn>
        static void adapt_sigaction_handler(int signum, siginfo_t* info, void* context)
        {
            std::invoke(fn, signum, info, context);
        }
    }
#endif

    char* detail::mmaped_file::to_chars(mapped_t ptr)
    {
        return reinterpret_cast<char*>(ptr);
    }
    
    auto detail::mmaped_file::page_size() -> decltype(::sysconf(_SC_PAGESIZE))
    {
        return ::sysconf(_SC_PAGESIZE);
    }
    
    size_t detail::mmaped_file::get_file_size(file_descriptor_t fd)
    {
        struct stat st{};
        if (::fstat(fd, &st) == -1)
            throw InternalError("mmaped_file::get_file_size(file_descriptor_t): Failed to get file size");
        return st.st_size;
    }
    
    detail::mmaped_file::mapped_t detail::mmaped_file::map_file(file_descriptor_t fd, size_t size)
    {
        return static_cast<mapped_t>(
            ::mmap(
                nullptr,
                size,
                PROT_READ | PROT_WRITE,
                MAP_SHARED_VALIDATE,
                fd,
                0
            )
        );
    }
    
    int detail::mmaped_file::unmap_file(mapped_t ptr, size_t size)
    {
        return ::munmap(ptr, size);
    }

    void detail::mmaped_file::track_file_changes()
    {
        this->inot_fd = ::inotify_init1(IN_NONBLOCK);
    }
    
    void detail::mmaped_file::lock_file_part(off_t start, off_t len, file_lock_mode mode)
    {
        // Verify that we don't already have a lock on this part of the file
        if (
            std::any_of(
                this->locked_parts.cbegin(),
                this->locked_parts.cend(),
                [start, len](const std::tuple<off_t, off_t, file_lock_mode>& tuple) -> bool
                {
                    // Return true if at least part of the interval [ start, start + len ] is in [ std::get<0>(tuple), std::get<1>(tuple) ]
                    off_t tuple_start = std::get<0>(tuple),
                          tuple_end   = std::get<1>(tuple),
                          end         = start + len;
                    if (tuple_start <= start && start <= tuple_end)
                        return true;
                    if (tuple_start <= end && end <= tuple_end)
                        return true;
                    if (start <= tuple_start && tuple_start <= end)
                        return true;
                    return false;
                }
            )
        )
            throw InternalError("mmaped_file::lock_file_part(file_descriptor_t, off_t, off_t, file_lock_mode): Already locked");
        
        struct flock fl{
            .l_type = (mode == file_lock_mode::shared_lock ? F_RDLCK : F_WRLCK),
            .l_whence = SEEK_SET,
            .l_start = start,
            .l_len = len,
            .l_pid = ::getpid()
        };
        if (::fcntl(this->fd, F_SETLKW, &fl) == -1)
            throw InternalError("mmaped_file::lock_file_part(file_descriptor_t, off_t, off_t, file_lock_mode): Failed to lock file");
        this->locked_parts.push_back({start, start + len, mode});
    }
    
    void detail::mmaped_file::unlock_file_part(off_t start, off_t len)
    {
        auto it = std::find_if(
            this->locked_parts.cbegin(),
            this->locked_parts.cend(),
            [start, len](const std::tuple<off_t, off_t, file_lock_mode>& tuple) -> bool
            {
                return std::get<0>(tuple) == start && std::get<1>(tuple) == start + len;
            }
        );
        if (it == this->locked_parts.end())
            throw InternalError("mmaped_file::unlock_file_part(file_descriptor_t, off_t, off_t): Not locked");
        struct flock fl{
            .l_type = F_UNLCK,
            .l_whence = SEEK_SET,
            .l_start = start,
            .l_len = len,
            .l_pid = ::getpid()
        };
        if (::fcntl(this->fd, F_SETLKW, &fl) == -1)
            throw InternalError("mmaped_file::unlock_file_part(file_descriptor_t, off_t, off_t): Failed to unlock file");
        this->locked_parts.erase(it);
    }
}