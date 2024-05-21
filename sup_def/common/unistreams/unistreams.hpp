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

#ifndef UNISTREAMS_HPP
#define UNISTREAMS_HPP


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

#include <boost/callable_traits.hpp>
#include <mpark/patterns.hpp>

#include <sup_def/common/unistreams/types.hpp>

namespace uni
{
    namespace detail
    {
        static inline bool handles_unicode(const std::locale& loc)
        {
            if (loc.name().contains("UTF-8"))
                return true;
            else
                return false;
        }

        static std::locale user_preferred_unicode_aware_locale()
        {
#if defined(_WIN32) || defined(_WIN64)
            TODO("Find a way to correctly handle unicode on goddamn Windows");
#else
            auto def = std::locale("");
            auto loc = handles_unicode(def)         ?
                       def                          :
                       std::locale("en_US.UTF-8");
            return (loc);
#endif
        }

        template <typename CharT>
            requires std::same_as<CharT, char16_t> || std::same_as<CharT, char32_t>
        consteval endianness get_literal_endianness()
        {
            if constexpr (std::same_as<CharT, char16_t>)
            {
                constexpr char16_t value[] = u"😀";
                static_assert(
                    ( sizeof(value) / sizeof(char16_t) ) - 1 == 2
                );
                static_assert(
                    sizeof(char16_t) * 2 == 4
                );
                uint8_t buf[4] = {
                    [ 0 ] = static_cast<uint8_t>(( value[0] >> 8 ) & 0xFF),
                    [ 1 ] = static_cast<uint8_t>(value[0] & 0xFF),
                    [ 2 ] = static_cast<uint8_t>(( value[1] >> 8 ) & 0xFF),
                    [ 3 ] = static_cast<uint8_t>(value[1] & 0xFF)
                };                    
                if (
                    buf[0] == 0xD8 &&
                    buf[1] == 0x3D &&
                    buf[2] == 0xDE &&
                    buf[3] == 0x00
                )
                    return endianness::big;
                else if (
                    buf[0] == 0x3D &&
                    buf[1] == 0xD8 &&
                    buf[2] == 0x00 &&
                    buf[3] == 0xDE
                )
                    return endianness::little;
                else
                    return endianness::undefined;
            }
            else
            {
                constexpr char32_t value[] = U"😀";
                static_assert(
                    ( sizeof(value) / sizeof(char32_t) ) - 1 == 1
                );
                static_assert(
                    sizeof(char32_t) == 4
                );
                uint8_t buf[4] = {
                    [ 0 ] = static_cast<uint8_t>(( value[0] >> 24 ) & 0xFF),
                    [ 1 ] = static_cast<uint8_t>(( value[0] >> 16 ) & 0xFF),
                    [ 2 ] = static_cast<uint8_t>(( value[0] >> 8 ) & 0xFF),
                    [ 3 ] = static_cast<uint8_t>(value[0] & 0xFF)
                };
                if (
                    buf[0] == 0x00 &&
                    buf[1] == 0x01 &&
                    buf[2] == 0xF6 &&
                    buf[3] == 0x00
                )
                    return endianness::big;
                else if (
                    buf[0] == 0x00 &&
                    buf[1] == 0xF6 &&
                    buf[2] == 0x01 &&
                    buf[3] == 0x00
                )
                    return endianness::little;
                else
                    return endianness::undefined;
            }
        }
/*
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
*/
        static_assert(
            std::integral_constant<
                endianness,
                get_literal_endianness<char16_t>()
            >::value == endianness::big
        );
        static_assert(
            std::integral_constant<
                endianness,
                get_literal_endianness<char32_t>()
            >::value == endianness::big
        );
/*
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        static_assert(
            std::integral_constant<
                endianness,
                get_literal_endianness<char16_t>()
            >::value == endianness::little
        );
        static_assert(
            std::integral_constant<
                endianness,
                get_literal_endianness<char32_t>()
            >::value == endianness::little
        );
#else
    #error "Unsupported endianness"
#endif
*/

    }


#if !UNISTREAMS_STRING_USE_STD_BASIC_STRING
    template <
        typename CharT,
        typename Traits,
        typename Allocator
    >
    class string
    {
        public:
            typedef CharT value_type;
            typedef Traits traits_type;
            typedef Allocator allocator_type;
            
            typedef size_t size_type;
            typedef ptrdiff_t difference_type;
            
            typedef CharT& reference;
            typedef const CharT& const_reference;
            
            typedef CharT* pointer;
            typedef const CharT* const_pointer;
            
            typedef pointer iterator;
            typedef const_pointer const_iterator;
            
            typedef std::reverse_iterator<iterator> reverse_iterator;
            typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

        private:
            /* Private methods */
        public:
            // Constructors
            constexpr string() noexcept(noexcept(allocator_type()));
            
            constexpr explicit string(const allocator_type& alloc) noexcept;
            
            template <typename = std::_RequireAllocator<allocator_type>> // TODO: Find a portable way to require this
            constexpr string(size_type count, value_type ch, const allocator_type& alloc = allocator_type());
            
            string(std::nullptr_t) = delete;
        private:
            pointer ptr;
            size_type len;
            size_type cap;

            optimize_space
            allocator_type allocator;
    };
#else
    template <
        typename CharT,
        typename Traits,
        typename Allocator
    >
    class string : public std::basic_string<CharT, Traits, Allocator>
    {
        using base_type = std::basic_string<CharT, Traits, Allocator>;

        public:
            using value_type = base_type::value_type;
            using traits_type = base_type::traits_type;
            using allocator_type = base_type::allocator_type;
            
            using size_type = base_type::size_type;
            using difference_type = base_type::difference_type;
            
            using reference = base_type::reference;
            using const_reference = base_type::const_reference;
            
            using pointer = base_type::pointer;
            using const_pointer = base_type::const_pointer;
            
            using iterator = base_type::iterator;
            using const_iterator = base_type::const_iterator;
            
            using reverse_iterator = base_type::reverse_iterator;
            using const_reverse_iterator = base_type::const_reverse_iterator;

        private:
            using string_type = ::uni::string<CharT, Traits, Allocator>;

        public:
            using base_type::base_type;
            using base_type::operator=;

            constexpr bool verify_endianness() const
                requires detail::has_endianness<value_type, traits_type>;
            constexpr bool verify_endianness(endianness end) const
                requires detail::has_endianness<value_type, traits_type>;
            
            constexpr bool verify_endianness(size_type until) const
                requires detail::has_endianness<value_type, traits_type>;
            constexpr bool verify_endianness(size_type until, endianness end) const
                requires detail::has_endianness<value_type, traits_type>;

            constexpr bool verify_endianness(size_type start, size_type end) const
                requires detail::has_endianness<value_type, traits_type>;
            constexpr bool verify_endianness(size_type start, size_type end, endianness endian) const
                requires detail::has_endianness<value_type, traits_type>;

            constexpr endianness get_endianness() const
                requires detail::has_endianness<value_type, traits_type>;

            constexpr void set_endianness(endianness end)
                requires detail::has_endianness<value_type, traits_type>;

            template <typename Allocator2 = allocator_type>
            constexpr explicit operator char_string_ex<Allocator2>() const
                requires (!std::same_as<string_type, char_string>)
            {
                using end_type = char_string_ex<Allocator2>;

                return end_type(
                    detail::string_conversions<value_type, char>()(
                        *(this)
                    ).c_str()
                );
            }

            template <typename Allocator2 = allocator_type>
            constexpr explicit operator wchar_string_ex<Allocator2>() const
                requires (!std::same_as<string_type, wchar_string>)
            {
                using end_type = wchar_string_ex<Allocator2>;

                return end_type(
                    detail::string_conversions<value_type, wchar_t>()(
                        *(this)
                    ).c_str()
                );
            }

            template <typename Allocator2 = allocator_type>
            constexpr explicit operator utf8_string_ex<Allocator2>() const
                requires (!std::same_as<string_type, utf8_string>)
            {
                using end_type = utf8_string_ex<Allocator2>;

                return end_type(
                    detail::string_conversions<value_type, utf8_char>()(
                        *(this)
                    ).c_str()
                );
            }
            
            template <typename Allocator2 = allocator_type>
            constexpr explicit operator utf16_string_ex<Allocator2>() const
                requires (!std::same_as<string_type, utf16_string>)
            {
                using end_type = utf16_string_ex<Allocator2>;

                return end_type(
                    detail::string_conversions<value_type, utf16_char>()(
                        *(this),
                        endianness::big
                    ).c_str()
                );
            }

            template <typename Allocator2 = allocator_type>
            constexpr explicit operator utf32_string_ex<Allocator2>() const
                requires (!std::same_as<string_type, utf32_string>)
            {
                using end_type = utf32_string_ex<Allocator2>;

                return end_type(
                    detail::string_conversions<value_type, utf32_char>()(
                        *(this),
                        endianness::big
                    ).c_str()
                );
            }

            template <typename StrTo>
                requires ::uni::supported_uni_string<StrTo>
            constexpr StrTo convert_to(
                [[maybe_unused]] endianness end = endianness::big
            ) const
            {
                if constexpr (
                    detail::has_endianness<typename StrTo::value_type, typename StrTo::traits_type>
                )
                {
                    if constexpr (
                        std::same_as<
                            typename StrTo::value_type,
                            value_type
                        >
                    )
                    {
                        StrTo ret = *(this);
                        ret.set_endianness(end);
                        return ret;
                    }
                    else
                    {
                        auto tmp = detail::string_conversions<value_type, typename StrTo::value_type>()(
                            *(this),
                            end
                        );
                        return StrTo(tmp.c_str(), tmp.length());
                    }
                }
                else
                {
                    if constexpr (
                        std::same_as<
                            typename StrTo::value_type,
                            value_type
                        >
                    )
                    {
                        return *(this);
                    }
                    else
                    {
                        auto tmp = detail::string_conversions<value_type, typename StrTo::value_type>()(
                            *(this)
                        );
                        return StrTo(tmp.c_str(), tmp.length());
                    }
                }
            }
    };
#endif

#if 1
    namespace detail
    {
        using file_descriptor_t = boost::callable_traits::return_type_t<decltype(&::open)>;

        class tracked_fds
        {
            static constexpr std::string signal_id = "uni::detail::tracked_fds";

            static void install_sig_handler() noexcept;

            public:
                static void add_tracked(file_descriptor_t fd, ...) noexcept;
                static void rm_tracked(file_descriptor_t fd, ...) noexcept;

                static bool is_tracked(file_descriptor_t fd) noexcept;

            private:
                static std::shared_mutex mtx;
                static std::set<file_descriptor_t> tracked;
                static std::atomic<int> signal_used;
                /* struct fanotify_event_metadata* event; */
        };

        class file_descriptor_wrapper : protected tracked_fds
        {
            static constexpr int default_open_flags = O_RDONLY;

            using base_type = tracked_fds;

            public:
                file_descriptor_wrapper() noexcept;

                file_descriptor_wrapper(
                    const std::filesystem::path& p,
                    int open_flags = default_open_flags
                );
                file_descriptor_wrapper(
                    std::filesystem::path&& p,
                    int open_flags = default_open_flags
                );

            private:
                std::filesystem::path path;
                file_descriptor_t fd;
                int flags;
        };
        class mmaped_file
        {
            using mapped_t = boost::callable_traits::return_type_t<decltype(&::mmap)>;

            protected:
                enum class file_lock_mode : int_fast8_t
                {
                    shared_lock,    // Read
                    exclusive_lock  // Write
                };

                static char* to_chars(mapped_t ptr);
                static auto page_size() -> decltype(::sysconf(_SC_PAGESIZE));
                static size_t get_file_size(file_descriptor_t fd);

                static mapped_t map_file(file_descriptor_t fd, size_t size);
                static int unmap_file(mapped_t ptr, size_t size);

                void track_file_changes();
                void untrack_file_changes();
                void lock_file_part(off_t start, off_t len, file_lock_mode mode);
                void unlock_file_part(off_t start, off_t len);
            
            public:
                using char_type = base_type::char_type;
                using traits_type = base_type::traits_type;
                using int_type = base_type::int_type;
                using pos_type = base_type::pos_type;
                using off_type = base_type::off_type;

                mmaped_file();
                mmaped_file(const std::filesystem::path& p);

                mmaped_file(const mmaped_file&) = delete;
                mmaped_file(mmaped_file&&) = delete;

                mmaped_file& operator=(const mmaped_file&) = delete;
                mmaped_file& operator=(mmaped_file&&) = delete;

                ~mmaped_file();
                
            private:
                std::vector<std::tuple<off_t, off_t, file_lock_mode>> locked_parts;
                std::filesystem::path path;
                mapped_t ptr;
                /* file_descriptor_t fd;
                file_descriptor_t inot_fd; */
        };
    }
#endif

    enum class open_mode : uint_fast8_t
    {
        read = 1 << 0,
        write = 1 << 1,
        read_write = read | write
    };
    ENUM_CLASS_OPERATORS(open_mode);

    enum class encoding_mode : uint_fast32_t
    {
        guess = uint_fast32_t(0),
        
        uni_utf8 = uint_fast32_t(1) << 0,          // use uni::utfX_char
        uni_utf16 = uint_fast32_t(1) << 1,         // use uni::utfX_char
        uni_utf32 = uint_fast32_t(1) << 2,         // use uni::utfX_char

        char_utf8 = uint_fast32_t(1) << 3,         // use char
        wchar = uint_fast32_t(1) << 4,             // use wchar_t

        big_endian = uint_fast32_t(1) << 30,       // modifier for uni::utfX_char
        little_endian = uint_fast32_t(1) << 31,    // modifier for uni::utfX_char

        uni_utf16be = uni_utf16 | big_endian,
        uni_utf16_big = uni_utf16be,

        uni_utf16le = uni_utf16 | little_endian,
        uni_utf16_little = uni_utf16le,

        uni_utf32be = uni_utf32 | big_endian,
        uni_utf32_big = uni_utf32be,

        uni_utf32le = uni_utf32 | little_endian,
        uni_utf32_little = uni_utf32le
    };
    ENUM_CLASS_OPERATORS(encoding_mode);

    namespace detail
    {
        class filebuf_wrapper : public std::filebuf
        {
            using base_type = std::filebuf;
            public:
                using char_type = base_type::char_type;
                using traits_type = base_type::traits_type;
                using int_type = base_type::int_type;
                using pos_type = base_type::pos_type;
                using off_type = base_type::off_type;

                using base_type::base_type;
                using base_type::operator=;

                ~filebuf_wrapper() = default;                

                using base_type::eback;
                using base_type::gptr;
                using base_type::egptr;
                using base_type::pbase;
                using base_type::pptr;
                using base_type::epptr;
#if 0
                using base_type::setg;
                using base_type::setp;
#endif
        };

        template <typename CharT>
        static constexpr encoding_mode get_file_encoding([[maybe_unused]] endianness end = endianness::big)
        {
            if constexpr (std::same_as<CharT, utf8_char>)
                return encoding_mode::uni_utf8;
            else if constexpr (std::same_as<CharT, utf16_char>)
                return encoding_mode::uni_utf16 | (end == endianness::big ? encoding_mode::big_endian : encoding_mode::little_endian);
            else if constexpr (std::same_as<CharT, utf32_char>)
                return encoding_mode::uni_utf32 | (end == endianness::big ? encoding_mode::big_endian : encoding_mode::little_endian);
            else if constexpr (std::same_as<CharT, char>)
                return encoding_mode::char_utf8;
            else if constexpr (std::same_as<CharT, wchar_t>)
                return encoding_mode::wchar;
            else
                return encoding_mode::guess;
        }

        template <typename CharT>
        struct file_base
        {
            protected:
                using char_type = CharT;
                using traits_type = ::uni::char_traits<char_type>;
                using int_type = typename traits_type::int_type;
                using pos_type = typename traits_type::pos_type;
                using off_type = typename traits_type::off_type;

                using filebuf_type = detail::filebuf_wrapper;

                file_base()
                    : file_encoding(encoding_mode(0))
                    , loc(detail::user_preferred_unicode_aware_locale())
                { }
                file_base(encoding_mode enc, const std::locale& l)
                    : file_encoding(enc)
                    , loc(l)
                { }
                
                file_base(const file_base&) = delete;
                file_base(file_base&&) = delete;
                file_base& operator=(const file_base&) = delete;
                file_base& operator=(file_base&&) = delete;
                
                ~file_base() = default;
            
            public:
                
                encoding_mode encoding() const
                { return this->file_encoding; }
                
                endianness get_endianness() const
                    requires detail::has_endianness<char_type, traits_type>
                {
                    if (std::to_underlying(this->file_encoding & (encoding_mode::uni_utf16 | encoding_mode::uni_utf32)))
                    {
                        if (std::to_underlying(this->file_encoding & encoding_mode::big_endian))
                            return endianness::big;
                        else if (std::to_underlying(this->file_encoding & encoding_mode::little_endian))
                            return endianness::little;
                        return endianness::undefined;
                    }
                    UNREACHABLE();
                }

                warn_usage_suggest_alternative(
                    "<file-type>_file(const std::filesystem::path&, endianness)",
                    "<file-type>_file(const std::filesystem::path&, std::ios_base::openmode, endianness)"
                )
                constexpr bool set_endianness(endianness end)
                    requires detail::has_endianness<char_type, traits_type>;

            protected:
                encoding_mode file_encoding;
                std::locale loc;

                

                constexpr const char* find_next(const char* s, const char* const last)
                    requires (!std::same_as<CharT, wchar_t>);

                constexpr const char* find_prev(const char* s, const char* const first)
                    requires (!std::same_as<CharT, wchar_t>);

                constexpr std::pair<CharT, std::optional<CharT>> create_char(const char* s, size_t byte_count)
                    requires std::same_as<CharT, utf16_char>;

                constexpr CharT create_char(const char* s, size_t byte_count = 4)
                    requires std::same_as<CharT, utf32_char>;
        };
    }


    template <::uni::supported_uni_char CharT>
    class input_file
        : virtual public detail::file_base<CharT>
    {
        STATIC_TODO();
    };

    template <::uni::supported_uni_char CharT>
    class output_file : virtual public detail::file_base<CharT>
    {
        STATIC_TODO();
    };

    template <::uni::supported_uni_char CharT>
    class file : virtual public detail::file_base<CharT>
    {
        STATIC_TODO();
    };

    using file_wrapper = std::variant<
        input_file<utf8_char>,
        input_file<utf16_char>,
        input_file<utf32_char>,
        input_file<char>,
        input_file<wchar_t>,
        
        output_file<utf8_char>,
        output_file<utf16_char>,
        output_file<utf32_char>,
        output_file<char>,
        output_file<wchar_t>,
        
        file<utf8_char>,
        file<utf16_char>,
        file<utf32_char>,
        file<char>,
        file<wchar_t>
    >;

    using ifile = std::variant<
        input_file<utf8_char>,
        input_file<utf16_char>,
        input_file<utf32_char>,
        input_file<char>,
        input_file<wchar_t>
    >;
    using ofile = std::variant<
        output_file<utf8_char>,
        output_file<utf16_char>,
        output_file<utf32_char>,
        output_file<char>,
        output_file<wchar_t>
    >;
    using iofile = std::variant<
        file<utf8_char>,
        file<utf16_char>,
        file<utf32_char>,
        file<char>,
        file<wchar_t>
    >;

    using string_wrapper = std::variant<
        utf8_string,
        utf16_string,
        utf32_string,
        char_string,
        wchar_string
    >;

    namespace detail
    {
        static encoding_mode guess_encoding(const std::filesystem::path& path)
        {
            std::ifstream file(path, std::ios_base::in | std::ios_base::binary);
            if (!file.is_open())
                throw InternalError("guess_encoding(const std::filesystem::path&): Failed to open file");
            char buf[4];
            uint8_t* bytes = reinterpret_cast<uint8_t*>(buf);
            file.read(buf, 4);
            if (file.gcount() < 4)
                return encoding_mode::char_utf8;
            if (bytes[0] == 0xFF && bytes[1] == 0xFE)
                return encoding_mode::uni_utf16le;
            else if (bytes[0] == 0xFE && bytes[1] == 0xFF)
                return encoding_mode::uni_utf16be;
            else if (bytes[0] == 0xEF && bytes[1] == 0xBB && bytes[2] == 0xBF)
                return encoding_mode::uni_utf8;
            else if (bytes[0] == 0 && bytes[1] == 0 && bytes[2] == 0xFE && bytes[3] == 0xFF)
                return encoding_mode::uni_utf32be;
            else if (bytes[0] == 0xFF && bytes[1] == 0xFE && bytes[2] == 0 && bytes[3] == 0)
                return encoding_mode::uni_utf32le;
            else
                return encoding_mode::char_utf8;
        }

        static constexpr encoding_mode get_encoding_mode(const std::filesystem::path& path, encoding_mode mode)
        {
            if (mode == encoding_mode::guess)
                return guess_encoding(path);
            else
            {
                if (std::to_underlying(mode & (encoding_mode::uni_utf16 | encoding_mode::uni_utf32)))
                    mode |= encoding_mode::big_endian;
                return mode;
            }
        }

        static file_wrapper create_file(const std::filesystem::path& path, open_mode mode, encoding_mode retencoding)
        {
            switch (retencoding)
            {
                case encoding_mode::uni_utf8:
                    if (mode & open_mode::read)
                    {
                        if (mode & open_mode::write)
                            return file_wrapper(file<utf8_char>(path));
                        else
                            return file_wrapper(input_file<utf8_char>(path));
                    }
                    else if (mode & open_mode::write)
                        return file_wrapper(output_file<utf8_char>(path));
                    else
                        throw InternalError("create_file(const std::filesystem::path&, open_mode, encoding_mode): Invalid mode");
                    break;
                case encoding_mode::uni_utf16be:
                    if (mode & open_mode::read)
                    {
                        if (mode & open_mode::write)
                        {
                            file<utf16_char> f(path, endianness::big);
                            return file_wrapper(std::move(f));
                        }
                        else
                        {
                            input_file<utf16_char> f(path, endianness::big);
                            return file_wrapper(std::move(f));
                        }
                    }
                    else if (mode & open_mode::write)
                    {
                        output_file<utf16_char> f(path, endianness::big);
                        return file_wrapper(std::move(f));
                    }
                    else
                        throw InternalError("create_file(const std::filesystem::path&, open_mode, encoding_mode): Invalid mode");
                    break;
                case encoding_mode::uni_utf16le:
                    if (mode & open_mode::read)
                    {
                        if (mode & open_mode::write)
                        {
                            file<utf16_char> f(path, endianness::little);
                            return file_wrapper(std::move(f));
                        }
                        else
                        {
                            input_file<utf16_char> f(path, endianness::little);
                            return file_wrapper(std::move(f));
                        }
                    }
                    else if (mode & open_mode::write)
                    {
                        output_file<utf16_char> f(path, endianness::little);
                        return file_wrapper(std::move(f));
                    }
                    else
                        throw InternalError("create_file(const std::filesystem::path&, open_mode, encoding_mode): Invalid mode");
                    break;
                case encoding_mode::uni_utf32be:
                    if (mode & open_mode::read)
                    {
                        if (mode & open_mode::write)
                        {
                            file<utf32_char> f(path, endianness::big);
                            return file_wrapper(std::move(f));
                        }
                        else
                        {
                            input_file<utf32_char> f(path, endianness::big);
                            return file_wrapper(std::move(f));
                        }
                    }
                    else if (mode & open_mode::write)
                    {
                        output_file<utf32_char> f(path, endianness::big);
                        return file_wrapper(std::move(f));
                    }
                    else
                        throw InternalError("create_file(const std::filesystem::path&, open_mode, encoding_mode): Invalid mode");
                    break;
                case encoding_mode::uni_utf32le:
                    if (mode & open_mode::read)
                    {
                        if (mode & open_mode::write)
                        {
                            file<utf32_char> f(path, endianness::little);
                            return file_wrapper(std::move(f));
                        }
                        else
                        {
                            input_file<utf32_char> f(path, endianness::little);
                            return file_wrapper(std::move(f));
                        }
                    }
                    else if (mode & open_mode::write)
                    {
                        output_file<utf32_char> f(path, endianness::little);
                        return file_wrapper(std::move(f));
                    }
                    else
                        throw InternalError("create_file(const std::filesystem::path&, open_mode, encoding_mode): Invalid mode");
                    break;
                case encoding_mode::char_utf8:
                    if (mode & open_mode::read)
                    {
                        if (mode & open_mode::write)
                            return file_wrapper(file<char>(path));
                        else
                            return file_wrapper(input_file<char>(path));
                    }
                    else if (mode & open_mode::write)
                        return file_wrapper(output_file<char>(path));
                    else
                        throw InternalError("create_file(const std::filesystem::path&, open_mode, encoding_mode): Invalid mode");
                    break;
                case encoding_mode::wchar:
                    if (mode & open_mode::read)
                    {
                        if (mode & open_mode::write)
                            return file_wrapper(file<wchar_t>(path));
                        else
                            return file_wrapper(input_file<wchar_t>(path));
                    }
                    else if (mode & open_mode::write)
                        return file_wrapper(output_file<wchar_t>(path));
                    else
                        throw InternalError("create_file(const std::filesystem::path&, open_mode, encoding_mode): Invalid mode");
                    break;
                default:
                    throw InternalError("create_file(const std::filesystem::path&, open_mode, encoding_mode): Invalid encoding");
            }

            UNREACHABLE();
            return file_wrapper();
        }
    }

    static constexpr inline encoding_mode get_corrected_encoding_mode(const std::filesystem::path& path, encoding_mode encoding)
    {
        return detail::get_encoding_mode(path, encoding);
    }

    static std::pair<file_wrapper, encoding_mode> open_file(const std::filesystem::path& path, open_mode mode, encoding_mode encoding)
    {
        file_wrapper retfile;
        encoding_mode retencoding;

        retencoding = detail::get_encoding_mode(path, encoding);

        retfile = detail::create_file(path, mode, retencoding);

        return std::pair<file_wrapper, encoding_mode>(std::move(retfile), retencoding);
    }

    template <
        typename FnUtf8,
        typename FnUtf16,
        typename FnUtf32,
        typename FnChar,
        typename FnWchar,
        typename... Args
    >
    struct overload_visitor
    {
        mutable FnUtf8 fn_utf8;
        mutable FnUtf16 fn_utf16;
        mutable FnUtf32 fn_utf32;
        mutable FnChar fn_char;
        mutable FnWchar fn_wchar;
        mutable std::tuple<Args...> extra_args;

        overload_visitor(
            FnUtf8 fu8,
            FnUtf16 fu16,
            FnUtf32 fu32,
            FnChar fc,
            FnWchar fw,
            Args&&... args
        )
            : fn_utf8(fu8),
              fn_utf16(fu16),
              fn_utf32(fu32),
              fn_char(fc),
              fn_wchar(fw),
              extra_args(std::forward<Args>(args)...)
        { }
        overload_visitor(const overload_visitor&) = default;
        overload_visitor(overload_visitor&&) = default;

        overload_visitor& operator=(const overload_visitor&) = default;
        overload_visitor& operator=(overload_visitor&&) = default;

        template <typename FileT>
        constexpr decltype(auto) operator()(FileT&& file) const
        {
            using char_type = typename std::remove_cvref_t<FileT>::char_type;
            
            if constexpr (std::same_as<char_type, utf8_char>)
            {
                return std::apply(
                    fn_utf8,
                    std::tuple_cat(
                        std::forward_as_tuple(file),
                        extra_args
                    )
                );
            }
            else if constexpr (std::same_as<char_type, utf16_char>)
            {
                return std::apply(
                    fn_utf16,
                    std::tuple_cat(
                        std::forward_as_tuple(file),
                        extra_args
                    )
                );
            }
            else if constexpr (std::same_as<char_type, utf32_char>)
            {
                return std::apply(
                    fn_utf32,
                    std::tuple_cat(
                        std::forward_as_tuple(file),
                        extra_args
                    )
                );
            }
            else if constexpr (std::same_as<char_type, char>)
            {
                return std::apply(
                    fn_char,
                    std::tuple_cat(
                        std::forward_as_tuple(file),
                        extra_args
                    )
                );
            }
            else if constexpr (std::same_as<char_type, wchar_t>)
            {
                return std::apply(
                    fn_wchar,
                    std::tuple_cat(
                        std::forward_as_tuple(file),
                        extra_args
                    )
                );
            }
            UNREACHABLE();
        }
    };

    template <typename Fn, typename... Args>
    static constexpr decltype(auto) use_file(file_wrapper& file, Fn&& fn, Args&&... extra_args)
    {
        return std::visit(
            [&fn, &extra_args...](auto&& f) -> decltype(auto)
            {
                return std::invoke(
                    std::forward<Fn>(fn),
                    f,
                    std::forward<Args>(extra_args)...
                );
            },
            file
        );
    }

    template <
        typename FnUtf8,
        typename FnUtf16,
        typename FnUtf32,
        typename FnChar,
        typename FnWchar,
        typename... Args
    >
    static constexpr decltype(auto) use_file(file_wrapper& file, const overload_visitor<FnUtf8, FnUtf16, FnUtf32, FnChar, FnWchar, Args...>& visitor)
    {
        return std::visit(visitor, file);
    }

    template <
        typename FnUtf8,
        typename FnUtf16,
        typename FnUtf32,
        typename FnChar,
        typename FnWchar,
        typename... Args
    >
    static constexpr decltype(auto) use_file(file_wrapper& file, const overload_visitor<FnUtf8, FnUtf16, FnUtf32, FnChar, FnWchar, Args...>&& visitor)
    {
        return std::visit(std::move(visitor), file);
    }
        

}

#define INCLUDED_FROM_UNISTREAMS_SOURCE 1

#include <sup_def/common/unistreams/impl/string.ipp>
#include <sup_def/common/unistreams/impl/mmaped_file.ipp>
#include <sup_def/common/unistreams/impl/file.ipp>

#undef INCLUDED_FROM_UNISTREAMS_SOURCE

namespace uni
{
    /* template <typename CharT>
        requires supported_uni_char<CharT>
    bool isspace(CharT c)
    {
        return std::isspace(c, detail::str_conv_base::getloc());
    } */
    namespace literals
    {
        static constexpr inline char operator ""uni(const char c)
        {
            return c;
        }
        static constexpr inline wchar_t operator ""uni(const wchar_t c)
        {
            return c;
        }
#if 0
        static constexpr inline utf8_char operator ""uni(const char c)
        {
            return utf8_char{reinterpret_cast<const char8_t&>(c)};
        }
#endif
        static constexpr inline utf8_char operator ""uni(const char8_t c)
        {
            return utf8_char{c};
        }
        static constexpr inline utf16_char operator ""uni(const char16_t c)
        {
            return utf16_char{c, detail::get_literal_endianness<char16_t>()};
        }
        static constexpr inline utf32_char operator ""uni(const char32_t c)
        {
            return utf32_char{c, detail::get_literal_endianness<char32_t>()};
        }


        static constexpr inline char_string operator ""uni(const char* str, std::size_t len)
        {
            return char_string(str, len);
        }
        static constexpr inline wchar_string operator ""uni(const wchar_t* str, std::size_t len)
        {
            return wchar_string(str, len);
        }
#if 0
        static constexpr inline utf8_string operator ""uni(const char* str, std::size_t len)
        {
            return utf8_string{reinterpret_cast<const char8_t*>(str), len};
        }
#endif
        static constexpr inline utf8_string operator ""uni(const char8_t* str, std::size_t len)
        {
            return utf8_string(str, len);
        }
        static constexpr inline utf16_string operator ""uni(const char16_t* str, std::size_t len)
        {
            using allocator_type = utf16_string::allocator_type;
            using allocator_traits = std::allocator_traits<allocator_type>;

            allocator_type alloc{};
            
            utf16_char* buf = allocator_traits::allocate(alloc, len + 1);
            
            for (std::size_t i = 0; i < len; i++)
                buf[i] = utf16_char{str[i], detail::get_literal_endianness<char16_t>()};
            buf[len] = utf16_char{};
            utf16_string ret = std::move(utf16_string(buf, len));
            
            allocator_traits::deallocate(alloc, buf, len + 1);

            return ret;
        }
        static constexpr inline utf32_string operator ""uni(const char32_t* str, std::size_t len)
        {
            using allocator_type = utf32_string::allocator_type;
            using allocator_traits = std::allocator_traits<allocator_type>;

            allocator_type alloc{};
            
            utf32_char* buf = allocator_traits::allocate(alloc, len + 1);
            
            for (std::size_t i = 0; i < len; i++)
                buf[i] = utf32_char{str[i], detail::get_literal_endianness<char32_t>()};
            buf[len] = utf32_char{};
            utf32_string ret = std::move(utf32_string(buf, len));
            
            allocator_traits::deallocate(alloc, buf, len + 1);

            return ret;
        }
    }
}

using ::uni::utf8_char;
using ::uni::utf16_char;
using ::uni::utf32_char;

using ::uni::char_string;
using ::uni::wchar_string;
using ::uni::utf8_string;
using ::uni::utf16_string;
using ::uni::utf32_string;

using ::uni::char_string_ex;
using ::uni::wchar_string_ex;
using ::uni::utf8_string_ex;
using ::uni::utf16_string_ex;
using ::uni::utf32_string_ex;

namespace uni::detail
{
    static_assert(
        !uni_string_allocdiff<utf8_string>::value  &&
        !uni_string_allocdiff<utf16_string>::value &&
        !uni_string_allocdiff<utf32_string>::value
    ); // Are proper utfX_string's but Allocator doesn't differ

    static_assert(
        !uni_string_allocdiff<
            ::uni::string<
                utf8_char,
                ::uni::char_traits<utf8_char>,
                std::allocator<utf8_char>
            >
        >::value
    ); // Is a proper utfX_string but Allocator doesn't differ

    static_assert(
        !uni_string_allocdiff<
            ::uni::string<
                utf16_char,
                ::uni::char_traits<utf32_char>,
                std::allocator<utf16_char>
            >
        >::value
    ); // Isn't a proper utfX_string and Allocator doesn't differ

    static_assert(
        !uni_string_allocdiff<
            ::uni::string<
                utf32_char,
                ::uni::char_traits<utf8_char>,
                std::pmr::polymorphic_allocator<utf32_char>
            >
        >::value
    ); // Allocator differs but isn't a proper utfX_string

    static_assert(
        uni_string_allocdiff<
            ::uni::string<
                utf8_char,
                ::uni::char_traits<utf8_char>,
                std::pmr::polymorphic_allocator<utf8_char>
            >
        >::value
    ); // Is a proper utfX_string and Allocator differs
}

#endif
